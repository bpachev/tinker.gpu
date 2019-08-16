#include "box.h"
#include "elec.h"
#include "mathfunc.h"
#include "md.h"
#include "pme.h"

TINKER_NAMESPACE_BEGIN
template <int DO_V>
void pme_conv_tmpl(PMEUnit pme_u, real* gpu_vir9) {
  auto& st = *pme_u;
  auto* dptr = pme_u.deviceptr();

  const int nfft1 = st.nfft1;
  const int nfft2 = st.nfft2;
  const int nfft3 = st.nfft3;
  const int nff = nfft1 * nfft2;
  const int ntot = nfft1 * nfft2 * nfft3;

  const real f = electric / dielec;
  real aewald = st.aewald;
  real pterm = pi / aewald;
  pterm *= pterm;

  #pragma acc parallel loop independent\
              deviceptr(gpu_vir9,dptr,box)
  for (int i = 0; i < ntot; ++i) {
    const real volterm = pi * box->volbox;

    if (i == 0) {
      dptr->qgrid[2 * i] = 0;
      dptr->qgrid[2 * i + 1] = 0;
      continue;
    }

    int k3 = i / nff;
    int j = i - k3 * nff;
    int k2 = j / nfft1;
    int k1 = j - k2 * nfft1;

    int r1 = (k1 < (nfft1 + 1) / 2) ? k1 : (k1 - nfft1);
    int r2 = (k2 < (nfft2 + 1) / 2) ? k2 : (k2 - nfft2);
    int r3 = (k3 < (nfft3 + 1) / 2) ? k3 : (k3 - nfft3);

    real h1 =
        box->recip[0][0] * r1 + box->recip[1][0] * r2 + box->recip[2][0] * r3;
    real h2 =
        box->recip[0][1] * r1 + box->recip[1][1] * r2 + box->recip[2][1] * r3;
    real h3 =
        box->recip[0][2] * r1 + box->recip[1][2] * r2 + box->recip[2][2] * r3;
    real hsq = h1 * h1 + h2 * h2 + h3 * h3;
    real term = -pterm * hsq;
    real expterm = 0;
    if (term > -50) {
      // TODO: if .not. use_bounds; if octahedron; 2/hsq
      real denom = volterm * hsq * dptr->bsmod1[k1] * dptr->bsmod1[k2] *
          dptr->bsmod1[k3];
      expterm = REAL_EXP(term) / denom;

      if_constexpr(DO_V) {
        real gridx = dptr->qgrid[2 * i];
        real gridy = dptr->qgrid[2 * i + 1];

        real struc2 = gridx * gridx + gridy * gridy;
        real eterm = 0.5f * f * expterm * struc2;
        real vterm = (2 / hsq) * (1 - term) * eterm;

        real vxx = (h1 * h1 * vterm - eterm);
        real vxy = h1 * h2 * vterm;
        real vxz = h1 * h3 * vterm;
        real vyy = (h2 * h2 * vterm - eterm);
        real vyz = h2 * h3 * vterm;
        real vzz = (h3 * h3 * vterm - eterm);

        #pragma acc atomic update
        gpu_vir9[0] += vxx;
        #pragma acc atomic update
        gpu_vir9[3] += vxy;
        #pragma acc atomic update
        gpu_vir9[6] += vxz;
        #pragma acc atomic update
        gpu_vir9[1] += vxy;
        #pragma acc atomic update
        gpu_vir9[4] += vyy;
        #pragma acc atomic update
        gpu_vir9[7] += vyz;
        #pragma acc atomic update
        gpu_vir9[2] += vxz;
        #pragma acc atomic update
        gpu_vir9[5] += vyz;
        #pragma acc atomic update
        gpu_vir9[8] += vzz;
      }
    }

    // complete the transformation of the PME grid

    dptr->qgrid[2 * i] *= expterm;
    dptr->qgrid[2 * i + 1] *= expterm;
  }
}

void pme_conv0(PMEUnit pme_u) { pme_conv_tmpl<0>(pme_u, nullptr); }

void pme_conv1(PMEUnit pme_u, real* gpu_vir9) {
  pme_conv_tmpl<1>(pme_u, gpu_vir9);
}
TINKER_NAMESPACE_END
#include "add.h"
#include "epolar.h"
#include "glob.spatial.h"
#include "image.h"
#include "launch.h"
#include "md.h"
#include "pme.h"
#include "seq_pair_field.h"
#include "switch.h"
#include "tool/cudalib.h"


namespace tinker {
#define DFIELDPARAS                                                            \
   real(*restrict field)[3], real(*restrict fieldp)[3],                        \
      const real *restrict thole, const real *restrict pdamp,                  \
      const real(*restrict rpole)[10], TINKER_IMAGE_PARAMS, real off2


template <class ETYP>
__launch_bounds__(BLOCK_DIM) __global__
void dfield_cu1(DFIELDPARAS, int n, const Spatial::SortedAtom* restrict sorted,
                int niak, const int* restrict iak, const int* restrict lst,
                real aewald)
{
   const int iwarp = (threadIdx.x + blockIdx.x * blockDim.x) / WARP_SIZE;
   const int nwarp = blockDim.x * gridDim.x / WARP_SIZE;
   const int ilane = threadIdx.x & (WARP_SIZE - 1);


   struct Data
   {
      real3 fkd, fkp;
      real3 rk;
      real ck;
      real dkx, dky, dkz;
      real qkxx, qkxy, qkxz, qkyy, qkyz, qkzz;
      real pdk, ptk;
   };
   __shared__ Data data[BLOCK_DIM];


   for (int iw = iwarp; iw < niak; iw += nwarp) {
      real3 fid = make_real3(0, 0, 0);
      real3 fip = make_real3(0, 0, 0);
      int atomi = min(iak[iw] * WARP_SIZE + ilane, n - 1);
      real3 ri = make_real3(sorted[atomi].x, sorted[atomi].y, sorted[atomi].z);
      int i = sorted[atomi].unsorted;
      real ci = rpole[i][mpl_pme_0];
      real dix = rpole[i][mpl_pme_x];
      real diy = rpole[i][mpl_pme_y];
      real diz = rpole[i][mpl_pme_z];
      real qixx = rpole[i][mpl_pme_xx];
      real qixy = rpole[i][mpl_pme_xy];
      real qixz = rpole[i][mpl_pme_xz];
      real qiyy = rpole[i][mpl_pme_yy];
      real qiyz = rpole[i][mpl_pme_yz];
      real qizz = rpole[i][mpl_pme_zz];
      real pdi = pdamp[i];
      real pti = thole[i];


      data[threadIdx.x].fkd = make_real3(0, 0, 0);
      data[threadIdx.x].fkp = make_real3(0, 0, 0);
      int shatomk = lst[iw * WARP_SIZE + ilane];
      data[threadIdx.x].rk =
         make_real3(sorted[shatomk].x, sorted[shatomk].y, sorted[shatomk].z);
      int shk = sorted[shatomk].unsorted;
      data[threadIdx.x].ck = rpole[shk][mpl_pme_0];
      data[threadIdx.x].dkx = rpole[shk][mpl_pme_x];
      data[threadIdx.x].dky = rpole[shk][mpl_pme_y];
      data[threadIdx.x].dkz = rpole[shk][mpl_pme_z];
      data[threadIdx.x].qkxx = rpole[shk][mpl_pme_xx];
      data[threadIdx.x].qkxy = rpole[shk][mpl_pme_xy];
      data[threadIdx.x].qkxz = rpole[shk][mpl_pme_xz];
      data[threadIdx.x].qkyy = rpole[shk][mpl_pme_yy];
      data[threadIdx.x].qkyz = rpole[shk][mpl_pme_yz];
      data[threadIdx.x].qkzz = rpole[shk][mpl_pme_zz];
      data[threadIdx.x].pdk = pdamp[shk];
      data[threadIdx.x].ptk = thole[shk];


      for (int j = 0; j < WARP_SIZE; ++j) {
         int srclane = (ilane + j) & (WARP_SIZE - 1);
         int klane = srclane + threadIdx.x - ilane;
         int atomk = __shfl_sync(ALL_LANES, shatomk, srclane);
         real3 dr = data[klane].rk - ri;


         real r2 = image2(dr.x, dr.y, dr.z);
         if (atomi < atomk && r2 <= off2) {
            if CONSTEXPR (eq<ETYP, EWALD>()) {
               pair_dfield<EWALD>(
                  r2, dr.x, dr.y, dr.z, 1, 1, ci, dix, diy, diz, qixx, qixy,
                  qixz, qiyy, qiyz, qizz, pdi, pti, data[klane].ck,
                  data[klane].dkx, data[klane].dky, data[klane].dkz,
                  data[klane].qkxx, data[klane].qkxy, data[klane].qkxz,
                  data[klane].qkyy, data[klane].qkyz, data[klane].qkzz,
                  data[klane].pdk, data[klane].ptk, aewald, fid, fip,
                  data[klane].fkd, data[klane].fkp);
            }
            if CONSTEXPR (eq<ETYP, NON_EWALD>()) {
               pair_dfield<NON_EWALD>(
                  r2, dr.x, dr.y, dr.z, 1, 1, ci, dix, diy, diz, qixx, qixy,
                  qixz, qiyy, qiyz, qizz, pdi, pti, data[klane].ck,
                  data[klane].dkx, data[klane].dky, data[klane].dkz,
                  data[klane].qkxx, data[klane].qkxy, data[klane].qkxz,
                  data[klane].qkyy, data[klane].qkyz, data[klane].qkzz,
                  data[klane].pdk, data[klane].ptk, 0, fid, fip,
                  data[klane].fkd, data[klane].fkp);
            }
         } // end if (include)
      }


      atomic_add(fid.x, &field[i][0]);
      atomic_add(fid.y, &field[i][1]);
      atomic_add(fid.z, &field[i][2]);
      atomic_add(fip.x, &fieldp[i][0]);
      atomic_add(fip.y, &fieldp[i][1]);
      atomic_add(fip.z, &fieldp[i][2]);
      atomic_add(data[threadIdx.x].fkd.x, &field[shk][0]);
      atomic_add(data[threadIdx.x].fkd.y, &field[shk][1]);
      atomic_add(data[threadIdx.x].fkd.z, &field[shk][2]);
      atomic_add(data[threadIdx.x].fkp.x, &fieldp[shk][0]);
      atomic_add(data[threadIdx.x].fkp.y, &fieldp[shk][1]);
      atomic_add(data[threadIdx.x].fkp.z, &fieldp[shk][2]);
   } // end for (iw)
}


__global__
void dfield_cu2(DFIELDPARAS, const real* restrict x, const real* restrict y,
                const real* restrict z, int ndpexclude,
                const int (*restrict dpexclude)[2],
                const real (*restrict dpexclude_scale)[2])
{
   for (int ii = threadIdx.x + blockIdx.x * blockDim.x; ii < ndpexclude;
        ii += blockDim.x * gridDim.x) {
      int i = dpexclude[ii][0];
      int k = dpexclude[ii][1];
      real dscale = dpexclude_scale[ii][0];
      real pscale = dpexclude_scale[ii][1];


      real xi = x[i];
      real yi = y[i];
      real zi = z[i];
      real ci = rpole[i][mpl_pme_0];
      real dix = rpole[i][mpl_pme_x];
      real diy = rpole[i][mpl_pme_y];
      real diz = rpole[i][mpl_pme_z];
      real qixx = rpole[i][mpl_pme_xx];
      real qixy = rpole[i][mpl_pme_xy];
      real qixz = rpole[i][mpl_pme_xz];
      real qiyy = rpole[i][mpl_pme_yy];
      real qiyz = rpole[i][mpl_pme_yz];
      real qizz = rpole[i][mpl_pme_zz];
      real pdi = pdamp[i];
      real pti = thole[i];


      real xr = x[k] - xi;
      real yr = y[k] - yi;
      real zr = z[k] - zi;


      real r2 = image2(xr, yr, zr);
      if (r2 <= off2) {
         real3 fid = make_real3(0, 0, 0);
         real3 fip = make_real3(0, 0, 0);
         real3 fkd = make_real3(0, 0, 0);
         real3 fkp = make_real3(0, 0, 0);
         pair_dfield<NON_EWALD>(
            r2, xr, yr, zr, dscale, pscale, ci, dix, diy, diz, qixx, qixy, qixz,
            qiyy, qiyz, qizz, pdi, pti, rpole[k][mpl_pme_0],
            rpole[k][mpl_pme_x], rpole[k][mpl_pme_y], rpole[k][mpl_pme_z],
            rpole[k][mpl_pme_xx], rpole[k][mpl_pme_xy], rpole[k][mpl_pme_xz],
            rpole[k][mpl_pme_yy], rpole[k][mpl_pme_yz], rpole[k][mpl_pme_zz],
            pdamp[k], thole[k], 0, fid, fip, fkd, fkp);


         atomic_add(fid.x, &field[i][0]);
         atomic_add(fid.y, &field[i][1]);
         atomic_add(fid.z, &field[i][2]);
         atomic_add(fip.x, &fieldp[i][0]);
         atomic_add(fip.y, &fieldp[i][1]);
         atomic_add(fip.z, &fieldp[i][2]);


         atomic_add(fkd.x, &field[k][0]);
         atomic_add(fkd.y, &field[k][1]);
         atomic_add(fkd.z, &field[k][2]);
         atomic_add(fkp.x, &fieldp[k][0]);
         atomic_add(fkp.y, &fieldp[k][1]);
         atomic_add(fkp.z, &fieldp[k][2]);
      } // end if (include)
   }
}


void dfield_ewald_real_cu(real (*field)[3], real (*fieldp)[3])
{
   const auto& st = *mspatial_unit;
   const real off = switch_off(switch_ewald);
   const real off2 = off * off;


   const PMEUnit pu = ppme_unit;
   const real aewald = pu->aewald;
   if (st.niak > 0) {
      launch_k1s(nonblk, WARP_SIZE * st.niak, dfield_cu1<EWALD>, field, fieldp,
                 thole, pdamp, rpole, TINKER_IMAGE_ARGS, off2, n, st.sorted,
                 st.niak, st.iak, st.lst, aewald);
   }
   if (ndpexclude > 0) {
      launch_k1s(nonblk, ndpexclude, dfield_cu2, field, fieldp, thole, pdamp,
                 rpole, TINKER_IMAGE_ARGS, off2, x, y, z, ndpexclude, dpexclude,
                 dpexclude_scale);
   }
}


void dfield_nonewald_cu(real (*field)[3], real (*fieldp)[3])
{
   const auto& st = *mspatial_unit;
   const real off = switch_off(switch_mpole);
   const real off2 = off * off;


   darray::zero(PROCEED_NEW_Q, n, field, fieldp);
   if (st.niak > 0) {
      launch_k1s(nonblk, WARP_SIZE * st.niak, dfield_cu1<NON_EWALD>, field,
                 fieldp, thole, pdamp, rpole, TINKER_IMAGE_ARGS, off2, n,
                 st.sorted, st.niak, st.iak, st.lst, 0);
   }
   if (ndpexclude > 0) {
      launch_k1s(nonblk, ndpexclude, dfield_cu2, field, fieldp, thole, pdamp,
                 rpole, TINKER_IMAGE_ARGS, off2, x, y, z, ndpexclude, dpexclude,
                 dpexclude_scale);
   }
}


#define UFIELDPARAS                                                            \
   const real(*restrict uind)[3], const real(*restrict uinp)[3],               \
      real(*restrict field)[3], real(*restrict fieldp)[3],                     \
      const real *restrict thole, const real *restrict pdamp,                  \
      TINKER_IMAGE_PARAMS, real off2


template <class ETYP>
__launch_bounds__(BLOCK_DIM) __global__
void ufield_cu1(UFIELDPARAS, int n, const Spatial::SortedAtom* restrict sorted,
                int niak, const int* restrict iak, const int* restrict lst,
                real aewald)
{
   const int iwarp = (threadIdx.x + blockIdx.x * blockDim.x) / WARP_SIZE;
   const int nwarp = blockDim.x * gridDim.x / WARP_SIZE;
   const int ilane = threadIdx.x & (WARP_SIZE - 1);


   struct Data
   {
      real3 fkd, fkp, ukd, ukp, rk;
      real pdk, ptk;
   };
   __shared__ Data data[BLOCK_DIM];


   for (int iw = iwarp; iw < niak; iw += nwarp) {
      real3 fid = make_real3(0, 0, 0);
      real3 fip = make_real3(0, 0, 0);
      int atomi = min(iak[iw] * WARP_SIZE + ilane, n - 1);
      real3 ri = make_real3(sorted[atomi].x, sorted[atomi].y, sorted[atomi].z);
      int i = sorted[atomi].unsorted;
      real3 uid = make_real3(uind[i][0], uind[i][1], uind[i][2]);
      real3 uip = make_real3(uinp[i][0], uinp[i][1], uinp[i][2]);
      real pdi = pdamp[i];
      real pti = thole[i];


      data[threadIdx.x].fkd = make_real3(0, 0, 0);
      data[threadIdx.x].fkp = make_real3(0, 0, 0);
      int shatomk = lst[iw * WARP_SIZE + ilane];
      data[threadIdx.x].rk =
         make_real3(sorted[shatomk].x, sorted[shatomk].y, sorted[shatomk].z);
      int shk = sorted[shatomk].unsorted;
      data[threadIdx.x].ukd =
         make_real3(uind[shk][0], uind[shk][1], uind[shk][2]);
      data[threadIdx.x].ukp =
         make_real3(uinp[shk][0], uinp[shk][1], uinp[shk][2]);
      data[threadIdx.x].pdk = pdamp[shk];
      data[threadIdx.x].ptk = thole[shk];


      for (int j = 0; j < WARP_SIZE; ++j) {
         int srclane = (ilane + j) & (WARP_SIZE - 1);
         int klane = srclane + threadIdx.x - ilane;
         int atomk = __shfl_sync(ALL_LANES, shatomk, srclane);
         real3 dr = data[klane].rk - ri;


         real r2 = image2(dr.x, dr.y, dr.z);
         if (atomi < atomk && r2 <= off2) {
            if CONSTEXPR (eq<ETYP, EWALD>()) {
               pair_ufield<EWALD>(
                  r2, dr.x, dr.y, dr.z, 1, uid.x, uid.y, uid.z, uip.x, uip.y,
                  uip.z, pdi, pti, data[klane].ukd.x, data[klane].ukd.y,
                  data[klane].ukd.z, data[klane].ukp.x, data[klane].ukp.y,
                  data[klane].ukp.z, data[klane].pdk, data[klane].ptk, aewald,
                  fid, fip, data[klane].fkd, data[klane].fkp);
            }
            if CONSTEXPR (eq<ETYP, NON_EWALD>()) {
               pair_ufield<NON_EWALD>(
                  r2, dr.x, dr.y, dr.z, 1, uid.x, uid.y, uid.z, uip.x, uip.y,
                  uip.z, pdi, pti, data[klane].ukd.x, data[klane].ukd.y,
                  data[klane].ukd.z, data[klane].ukp.x, data[klane].ukp.y,
                  data[klane].ukp.z, data[klane].pdk, data[klane].ptk, 0, fid,
                  fip, data[klane].fkd, data[klane].fkp);
            }
         } // end if (include)
      }


      atomic_add(fid.x, &field[i][0]);
      atomic_add(fid.y, &field[i][1]);
      atomic_add(fid.z, &field[i][2]);
      atomic_add(fip.x, &fieldp[i][0]);
      atomic_add(fip.y, &fieldp[i][1]);
      atomic_add(fip.z, &fieldp[i][2]);
      atomic_add(data[threadIdx.x].fkd.x, &field[shk][0]);
      atomic_add(data[threadIdx.x].fkd.y, &field[shk][1]);
      atomic_add(data[threadIdx.x].fkd.z, &field[shk][2]);
      atomic_add(data[threadIdx.x].fkp.x, &fieldp[shk][0]);
      atomic_add(data[threadIdx.x].fkp.y, &fieldp[shk][1]);
      atomic_add(data[threadIdx.x].fkp.z, &fieldp[shk][2]);
   } // end for (iw)
}


__global__
void ufield_cu2(UFIELDPARAS, const real* restrict x, const real* restrict y,
                const real* restrict z, int nuexclude,
                const int (*restrict uexclude)[2],
                const real* restrict uexclude_scale)
{
   for (int ii = threadIdx.x + blockIdx.x * blockDim.x; ii < nuexclude;
        ii += blockDim.x * gridDim.x) {
      int i = uexclude[ii][0];
      int k = uexclude[ii][1];
      real uscale = uexclude_scale[ii];


      real xi = x[i];
      real yi = y[i];
      real zi = z[i];
      real3 uid = make_real3(uind[i][0], uind[i][1], uind[i][2]);
      real3 uip = make_real3(uinp[i][0], uinp[i][1], uinp[i][2]);
      real pdi = pdamp[i];
      real pti = thole[i];


      real xr = x[k] - xi;
      real yr = y[k] - yi;
      real zr = z[k] - zi;


      real r2 = image2(xr, yr, zr);
      if (r2 <= off2) {
         real3 fid = make_real3(0, 0, 0);
         real3 fip = make_real3(0, 0, 0);
         real3 fkd = make_real3(0, 0, 0);
         real3 fkp = make_real3(0, 0, 0);
         pair_ufield<NON_EWALD>(
            r2, xr, yr, zr, uscale, uid.x, uid.y, uid.z, uip.x, uip.y, uip.z,
            pdi, pti, uind[k][0], uind[k][1], uind[k][2], uinp[k][0],
            uinp[k][1], uinp[k][2], pdamp[k], thole[k], 0, fid, fip, fkd, fkp);


         atomic_add(fid.x, &field[i][0]);
         atomic_add(fid.y, &field[i][1]);
         atomic_add(fid.z, &field[i][2]);
         atomic_add(fip.x, &fieldp[i][0]);
         atomic_add(fip.y, &fieldp[i][1]);
         atomic_add(fip.z, &fieldp[i][2]);


         atomic_add(fkd.x, &field[k][0]);
         atomic_add(fkd.y, &field[k][1]);
         atomic_add(fkd.z, &field[k][2]);
         atomic_add(fkp.x, &fieldp[k][0]);
         atomic_add(fkp.y, &fieldp[k][1]);
         atomic_add(fkp.z, &fieldp[k][2]);
      } // end if (include)
   }
}


void ufield_ewald_real_cu(const real (*uind)[3], const real (*uinp)[3],
                          real (*field)[3], real (*fieldp)[3])
{
   const auto& st = *mspatial_unit;
   const real off = switch_off(switch_ewald);
   const real off2 = off * off;


   const PMEUnit pu = ppme_unit;
   const real aewald = pu->aewald;


   if (st.niak > 0) {
      launch_k1s(nonblk, WARP_SIZE * st.niak, ufield_cu1<EWALD>, uind, uinp,
                 field, fieldp, thole, pdamp, TINKER_IMAGE_ARGS, off2, n,
                 st.sorted, st.niak, st.iak, st.lst, aewald);
   }
   if (nuexclude) {
      launch_k1s(nonblk, nuexclude, ufield_cu2, uind, uinp, field, fieldp,
                 thole, pdamp, TINKER_IMAGE_ARGS, off2, x, y, z, nuexclude,
                 uexclude, uexclude_scale);
   }
}


void ufield_nonewald_cu(const real (*uind)[3], const real (*uinp)[3],
                        real (*field)[3], real (*fieldp)[3])
{
   const auto& st = *mspatial_unit;
   const real off = switch_off(switch_mpole);
   const real off2 = off * off;


   darray::zero(PROCEED_NEW_Q, n, field, fieldp);
   if (st.niak > 0) {
      launch_k1s(nonblk, WARP_SIZE * st.niak, ufield_cu1<NON_EWALD>, uind, uinp,
                 field, fieldp, thole, pdamp, TINKER_IMAGE_ARGS, off2, n,
                 st.sorted, st.niak, st.iak, st.lst, 0);
   }
   if (nuexclude) {
      launch_k1s(nonblk, nuexclude, ufield_cu2, uind, uinp, field, fieldp,
                 thole, pdamp, TINKER_IMAGE_ARGS, off2, x, y, z, nuexclude,
                 uexclude, uexclude_scale);
   }
}
}

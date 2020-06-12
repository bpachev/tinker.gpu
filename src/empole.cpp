#include "empole.h"
#include "md.h"
#include "nblist.h"
#include "potent.h"
#include "tool/host_zero.h"
#include <tinker/detail/couple.hh>
#include <tinker/detail/mplpot.hh>
#include <tinker/detail/sizes.hh>


namespace tinker {
void empole_data(rc_op op)
{
   if (!use_potent(mpole_term))
      return;

   if (op & rc_dealloc) {
      nmexclude = 0;
      darray::deallocate(mexclude, mexclude_scale);

      if (rc_flag & calc::analyz) {
         buffer_deallocate(calc::analyz, nem);
      }
      buffer_deallocate(rc_flag | calc::analyz, em, vir_em);
      buffer_deallocate(rc_flag | calc::analyz, demx, demy, demz);
   }

   if (op & rc_alloc) {
      m2scale = mplpot::m2scale;
      m3scale = mplpot::m3scale;
      m4scale = mplpot::m4scale;
      m5scale = mplpot::m5scale;

      std::vector<int> exclik;
      std::vector<real> excls;
      // see also attach.f
      const int maxn13 = 3 * sizes::maxval;
      const int maxn14 = 9 * sizes::maxval;
      const int maxn15 = 27 * sizes::maxval;
      for (int i = 0; i < n; ++i) {
         int nn;
         int bask;

         if (m2scale != 1) {
            nn = couple::n12[i];
            for (int j = 0; j < nn; ++j) {
               int k = couple::i12[i][j];
               k -= 1;
               if (k > i) {
                  exclik.push_back(i);
                  exclik.push_back(k);
                  excls.push_back(m2scale - 1);
               }
            }
         }

         if (m3scale != 1) {
            nn = couple::n13[i];
            bask = i * maxn13;
            for (int j = 0; j < nn; ++j) {
               int k = couple::i13[bask + j];
               k -= 1;
               if (k > i) {
                  exclik.push_back(i);
                  exclik.push_back(k);
                  excls.push_back(m3scale - 1);
               }
            }
         }

         if (m4scale != 1) {
            nn = couple::n14[i];
            bask = i * maxn14;
            for (int j = 0; j < nn; ++j) {
               int k = couple::i14[bask + j];
               k -= 1;
               if (k > i) {
                  exclik.push_back(i);
                  exclik.push_back(k);
                  excls.push_back(m4scale - 1);
               }
            }
         }

         if (m5scale != 1) {
            nn = couple::n15[i];
            bask = i * maxn15;
            for (int j = 0; j < nn; ++j) {
               int k = couple::i15[bask + j];
               k -= 1;
               if (k > i) {
                  exclik.push_back(i);
                  exclik.push_back(k);
                  excls.push_back(m5scale - 1);
               }
            }
         }
      }
      nmexclude = excls.size();
      darray::allocate(nmexclude, &mexclude, &mexclude_scale);
      darray::copyin(WAIT_NEW_Q, nmexclude, mexclude, exclik.data());
      darray::copyin(WAIT_NEW_Q, nmexclude, mexclude_scale, excls.data());

      if (rc_flag & calc::analyz) {
         buffer_allocate(calc::analyz, &nem);
      }
      buffer_allocate(rc_flag | calc::analyz, &em, &vir_em);
      buffer_allocate(rc_flag | calc::analyz, &demx, &demy, &demz);
   }

   if (op & rc_init) {
   }
}


void empole(int vers)
{
   bool do_a = vers & calc::analyz;
   bool do_e = vers & calc::energy;
   bool do_v = vers & calc::virial;
   bool do_g = vers & calc::grad;


   host_zero(energy_em, virial_em);
   auto bsize = buffer_size();
   if (do_a)
      darray::zero(PROCEED_NEW_Q, bsize, nem);
   if (do_e)
      darray::zero(PROCEED_NEW_Q, bsize, em);
   if (do_v)
      darray::zero(PROCEED_NEW_Q, bsize, vir_em);
   if (do_g)
      darray::zero(PROCEED_NEW_Q, n, demx, demy, demz);


   mpole_init(vers);
   if (use_ewald())
      empole_ewald(vers);
   else
      empole_nonewald(vers);
   torque(vers, demx, demy, demz);


   if (do_e) {
      energy_buffer u = em;
      energy_prec e = energy_reduce(u);
      energy_elec += e;
      if (do_a) {
         energy_em = e;
      }
   }
   if (do_v) {
      virial_buffer u1 = vir_em;
      virial_buffer u2 = vir_trq;
      virial_prec v1[9];
      virial_prec v2[9];
      virial_reduce(v1, u1);
      virial_reduce(v2, u2);
      for (int iv = 0; iv < 9; ++iv) {
         virial_em[iv] = v1[iv] + v2[iv];
         virial_elec[iv] += virial_em[iv];
      }
   }
}


void empole_nonewald(int vers)
{
#if TINKER_CUDART
   if (mlist_version() & NBL_SPATIAL)
      empole_nonewald_cu(vers);
   else
#endif
      empole_nonewald_acc(vers);
}


void empole_ewald(int vers)
{
   empole_ewald_real_self(vers);
   empole_ewald_recip(vers);
}


void empole_ewald_real_self(int vers)
{
#if TINKER_CUDART
   if (mlist_version() & NBL_SPATIAL)
      empole_ewald_real_self_cu(vers);
   else
#endif
      empole_ewald_real_self_acc(vers);
}


void empole_ewald_recip(int vers)
{
   empole_ewald_recip_acc(vers);
}
}

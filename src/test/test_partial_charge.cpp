#include "files.h"
#include "test.h"
#include "test_rt.h"


using namespace TINKER_NAMESPACE;


TEST_CASE("PartialCharge-Trpcage", "[ff][ec][ewald][nonewald][trpcage]")
{
   rc_flag = calc::xyz | calc::vmask;


   const char* kname = "test_pchg.key";
   std::string k0 = trpcage_charmm19_key;
   k0 += "\nCHARGETERM ONLY\n";
   const char* xname = "test_pchg.xyz";
   const char* x0 = trpcage_charmm19_xyz;
   const char* pname = "charmm19.prm";
   const char* p0 = commit_11e84c69::charmm19_prm;


   const double eps_e = 0.0001;
   const double eps_g = 0.0001;
   const double eps_v = 0.001;
   const char* argv[] = {"dummy", xname};
   int argc = 2;


   /*
   SECTION("  - ec -- no pbc, no cutoff, non-ewald, taper")
   {
      TestFile fxy(xname, x0);
      TestFile fke(kname, k0);
      TestFile fpr(pname, p0);


      test_begin_with_args(argc, argv);
      initialize();


      energy(calc::v0);
      COMPARE_REALS(esum, ref_e, eps_e);


      energy(calc::v1);
      COMPARE_REALS(esum, ref_e, eps_e);
      COMPARE_GRADIENT_(ref_g, eps_g);
      for (int i = 0; i < 3; ++i)
         for (int j = 0; j < 3; ++j)
            COMPARE_REALS(vir[i * 3 + j], ref_v[i][j], eps_v);


      energy(calc::v3);
      COMPARE_REALS(esum, ref_e, eps_e);
      COMPARE_INTS(count_reduce(nec), ref_count);


      energy(calc::v4);
      COMPARE_REALS(esum, ref_e, eps_e);
      COMPARE_GRADIENT_(ref_g, eps_g);


      energy(calc::v5);
      COMPARE_GRADIENT_(ref_g, eps_g);


      energy(calc::v6);
      COMPARE_GRADIENT_(ref_g, eps_g);
      for (int i = 0; i < 3; ++i)
         for (int j = 0; j < 3; ++j)
            COMPARE_REALS(vir[i * 3 + j], ref_v[i][j], eps_v);


      finish();
      test_end();
   }
   */


   SECTION("  - ec -- pbc, cutoff, ewald")
   {
      std::string k1 = k0 +
         "\nEWALD"
         "\nNEIGHBOR-LIST"
         "\nCUTOFF           9.0"
         "\nLIST-BUFFER      0.5"
         "\nA-AXIS            25"
         "\nB-AXIS            30"
         "\nC-AXIS            20"
         "\n";


      TestFile fxy(xname, x0);
      TestFile fke(kname, k1);
      TestFile fpr(pname, p0);


      const double ref_e = -784.1710;
      const double ref_v[][3] = {{308.950, -10.859, 21.517},
                                 {-10.859, 307.362, 18.690},
                                 {21.517, 18.690, 167.439}};
      const int ref_count = 5039;
      const double ref_g[][3] = {
         {-6.3995, -2.5833, -0.4125},  {4.8138, -0.0886, -0.6376},
         {-0.4733, -0.3038, 7.6132},   {-3.7105, -5.2366, -10.4420},
         {2.9116, 0.9762, -0.0820},    {2.5393, 0.2261, -2.6482},
         {4.9295, 4.3764, 0.1128},     {0.0000, 0.0000, 0.0000},
         {7.9166, 6.3644, 11.5815},    {-12.5198, -11.3178, -2.5188},
         {-8.0233, -1.2761, -7.0385},  {3.3719, 0.5245, 1.7613},
         {1.5926, 1.5385, -0.2954},    {7.5590, 1.6277, -8.3062},
         {-0.7940, -0.7155, 1.2007},   {-4.9722, -0.3685, -0.3767},
         {7.8776, 6.4253, -7.1049},    {-6.2501, -1.0322, 4.8204},
         {0.0000, 0.0000, 0.0000},     {0.0000, 0.0000, 0.0000},
         {0.0000, 0.0000, 0.0000},     {0.0000, 0.0000, 0.0000},
         {9.1237, 2.9152, 2.4268},     {-1.2671, -1.1015, 0.1807},
         {-1.8125, 0.8945, 2.6889},    {2.1684, 5.5381, 7.8823},
         {-7.3062, -0.6177, -0.3326},  {0.0000, 0.0000, 0.0000},
         {0.0000, 0.0000, 0.0000},     {0.0000, 0.0000, 0.0000},
         {0.0000, 0.0000, 0.0000},     {0.0000, 0.0000, 0.0000},
         {0.0000, 0.0000, 0.0000},     {-0.5846, -0.9480, -0.1569},
         {0.3568, 2.1530, 0.0337},     {0.2321, -1.6145, -0.1575},
         {0.6402, -4.0318, 1.4799},    {-0.3288, -0.2108, -0.5910},
         {-3.8690, -1.8491, 1.0620},   {-6.6213, -0.1331, 1.7188},
         {-4.6376, 5.1875, 1.6221},    {0.0000, 0.0000, 0.0000},
         {0.0000, 0.0000, 0.0000},     {0.0000, 0.0000, 0.0000},
         {0.0000, 0.0000, 0.0000},     {-0.0511, -0.6701, -3.1683},
         {0.5121, 0.1900, 0.0070},     {-4.4823, 1.4666, -5.3065},
         {6.9142, -5.7499, -5.6787},   {0.2923, -1.1038, 0.0840},
         {0.0000, 0.0000, 0.0000},     {0.0000, 0.0000, 0.0000},
         {-1.5637, -6.8193, -0.6247},  {0.0976, 11.3942, 6.9627},
         {-0.2747, 3.2293, -2.1280},   {-1.0628, 1.3523, 0.6487},
         {-1.4490, 0.9570, -0.6072},   {3.8208, 1.5057, -1.1500},
         {0.2855, -0.1338, 1.0165},    {3.4808, 1.3800, 3.3817},
         {3.9844, 4.4358, -7.6436},    {-1.6525, -1.4045, 2.4543},
         {0.0000, 0.0000, 0.0000},     {0.0075, -0.3554, -0.0803},
         {-0.4333, 0.1108, 0.1165},    {-0.0452, 0.3195, 0.1511},
         {3.8550, -0.9420, -2.1175},   {0.2142, -0.1123, -0.3052},
         {-0.0148, -0.0403, 0.0865},   {0.0000, 0.0000, 0.0000},
         {0.0000, 0.0000, 0.0000},     {0.0000, 0.0000, 0.0000},
         {-7.9068, 1.8916, 0.5282},    {-1.0578, 1.9530, 2.5683},
         {0.0066, -1.1928, 0.4695},    {1.3910, -1.4207, 4.9049},
         {-5.1371, 0.1549, 7.7066},    {1.4474, -3.5537, -1.8195},
         {0.0000, 0.0000, 0.0000},     {0.0000, 0.0000, 0.0000},
         {0.0000, 0.0000, 0.0000},     {0.0000, 0.0000, 0.0000},
         {-4.0835, -0.5189, -0.7965},  {-0.2576, -0.5587, -0.6093},
         {-7.2808, -1.6207, -1.2389},  {1.1893, -3.7299, -0.6938},
         {3.2413, -2.9208, 0.5973},    {0.0000, 0.0000, 0.0000},
         {0.0000, 0.0000, 0.0000},     {0.0000, 0.0000, 0.0000},
         {-4.4284, -0.6934, -0.8147},  {1.8955, 2.0950, 2.4551},
         {-2.9244, -1.6344, -2.0763},  {-1.9404, -7.0835, -2.4914},
         {-1.6184, -0.3879, -4.6515},  {3.4175, 0.8141, -8.5550},
         {1.4410, -0.0972, 0.7599},    {10.7054, 1.5758, -0.0515},
         {-15.0805, -10.4150, 2.7023}, {-2.4804, 1.3168, 3.3689},
         {-0.1563, -0.9410, 1.3428},   {-1.4301, 2.0798, -2.7313},
         {6.0244, -0.1864, 4.5232},    {-4.6297, -3.9630, 2.1922},
         {-3.3431, -3.5105, 1.4298},   {1.3562, 0.8398, 1.4307},
         {4.1985, -0.1834, 6.6826},    {-7.7565, -10.2163, -1.0849},
         {3.8325, 0.8058, -2.2282},    {-1.4705, 1.8695, -2.4027},
         {1.0036, 0.9403, 0.6444},     {2.2299, 4.1667, -2.4451},
         {-7.5370, 6.0983, -0.3211},   {-0.5059, -0.8801, 2.3900},
         {-0.6146, 0.5961, 0.7436},    {1.2896, -0.2464, 0.0368},
         {9.1679, 4.1583, -1.4404},    {1.0512, 3.7415, 6.2578},
         {0.0000, 0.0000, 0.0000},     {0.0000, 0.0000, 0.0000},
         {0.0883, 0.6264, 0.8320},     {-5.1512, -4.5436, 1.9619},
         {0.8223, -0.5077, -0.1401},   {3.8039, -2.8057, 2.6117},
         {-2.5083, -6.5460, 1.1178},   {3.4904, 5.6774, 0.2184},
         {0.6150, 1.1295, 2.2331},     {-0.3188, -7.1464, -4.4932},
         {0.4050, 2.2333, 0.9317},     {-4.2253, -1.0363, 0.5454},
         {0.5630, 1.2925, -1.6899},    {-0.2761, 4.1152, -3.4465},
         {-6.3940, -9.4469, 1.4356},   {2.6575, 0.4637, -0.1292},
         {1.6660, 3.5426, -1.2804},    {-6.8394, -11.4315, 8.1088},
         {18.0634, 14.4005, -11.4197}, {1.2513, -3.6800, -0.8143},
         {0.6414, 1.4201, -0.1941},    {6.4236, 4.4419, -1.7383},
         {1.1339, -4.0587, -1.4444},   {0.8867, 1.4394, 0.3845},
         {-2.4835, 0.8507, 1.3614},    {-0.4655, 0.9618, 0.2780},
         {2.1198, -5.9372, 2.1574},    {2.6309, -0.2625, -0.0906},
         {1.6112, -2.1380, -0.6004},   {0.0000, 0.0000, 0.0000},
         {0.0000, 0.0000, 0.0000},     {1.4974, 1.3409, -1.6476},
         {-6.1073, -8.0325, 5.8321},   {3.8686, 8.0199, -4.4192},
         {-1.1480, 0.7414, 1.7233},    {-4.7097, -9.4909, 4.9567},
         {6.4894, 6.0295, -6.3395},    {0.8391, 0.1031, -0.5582},
         {0.3237, -5.0984, -1.3056},   {4.7073, 6.7249, -2.5960},
         {3.1942, 9.4073, -3.4511},    {-1.6957, 2.0285, -0.0429},
         {0.0992, 0.3493, 0.1318},     {2.9244, -2.3350, -2.8422},
         {-1.1640, 1.9673, 1.3126},    {0.0000, 0.0000, 0.0000},
         {0.0000, 0.0000, 0.0000},     {1.4881, 0.1910, -0.5277},
         {-1.1867, 1.5882, 0.1095},    {-0.5110, 0.3450, 0.6354},
         {3.0902, -1.0725, 3.2449},    {-1.2584, 2.7590, -0.0643},
         {0.0000, 0.0000, 0.0000},     {0.0000, 0.0000, 0.0000},
         {0.7556, -0.7434, 1.0678},    {-0.8954, 1.5556, 0.0158},
         {-0.7570, -0.5264, -0.2553},  {-3.6869, -14.1074, 1.7782},
         {-1.2395, 12.4818, -1.8606},  {0.0000, 0.0000, 0.0000},
         {0.0000, 0.0000, 0.0000},     {-0.7676, -1.6245, -0.2954},
         {-1.3451, 4.7630, -3.0331},   {0.5659, 0.5278, 0.4619},
         {0.2571, -0.0193, -0.3632},   {-0.4900, 5.3899, 1.0783},
         {3.0613, -4.6257, -2.8146},   {3.4054, -0.5440, 0.1161},
         {-8.3359, 4.6215, 0.4214},    {5.3244, -1.1341, 2.2995},
         {1.0532, 1.8806, 2.0643}};


      test_begin_with_args(argc, argv);
      initialize();


      energy(calc::v0);
      COMPARE_REALS(esum, ref_e, eps_e);


      energy(calc::v1);
      COMPARE_REALS(esum, ref_e, eps_e);
      COMPARE_GRADIENT_(ref_g, eps_g);
      for (int i = 0; i < 3; ++i)
         for (int j = 0; j < 3; ++j)
            COMPARE_REALS(vir[i * 3 + j], ref_v[i][j], eps_v);


      energy(calc::v3);
      COMPARE_REALS(esum, ref_e, eps_e);
      COMPARE_INTS(count_reduce(nec), ref_count);


      energy(calc::v4);
      COMPARE_REALS(esum, ref_e, eps_e);
      COMPARE_GRADIENT_(ref_g, eps_g);


      energy(calc::v5);
      COMPARE_GRADIENT_(ref_g, eps_g);


      energy(calc::v6);
      COMPARE_GRADIENT_(ref_g, eps_g);
      for (int i = 0; i < 3; ++i)
         for (int j = 0; j < 3; ++j)
            COMPARE_REALS(vir[i * 3 + j], ref_v[i][j], eps_v);


      finish();
      test_end();
   }
}
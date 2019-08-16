#ifndef TINKER_MOD_QMSTUF_HH_
#define TINKER_MOD_QMSTUF_HH_

#include "macro.h"

TINKER_NAMESPACE_BEGIN namespace qmstuf {
extern int& ngatom;
extern double& egau;
extern double*& gx;
extern double*& gy;
extern double*& gz;
extern double*& gfreq;
extern double*& gforce;
extern double*& gh;

#ifdef TINKER_MOD_CPP_
extern "C" int TINKER_MOD(qmstuf, ngatom);
extern "C" double TINKER_MOD(qmstuf, egau);
extern "C" double* TINKER_MOD(qmstuf, gx);
extern "C" double* TINKER_MOD(qmstuf, gy);
extern "C" double* TINKER_MOD(qmstuf, gz);
extern "C" double* TINKER_MOD(qmstuf, gfreq);
extern "C" double* TINKER_MOD(qmstuf, gforce);
extern "C" double* TINKER_MOD(qmstuf, gh);

int& ngatom = TINKER_MOD(qmstuf, ngatom);
double& egau = TINKER_MOD(qmstuf, egau);
double*& gx = TINKER_MOD(qmstuf, gx);
double*& gy = TINKER_MOD(qmstuf, gy);
double*& gz = TINKER_MOD(qmstuf, gz);
double*& gfreq = TINKER_MOD(qmstuf, gfreq);
double*& gforce = TINKER_MOD(qmstuf, gforce);
double*& gh = TINKER_MOD(qmstuf, gh);
#endif
} TINKER_NAMESPACE_END

#endif
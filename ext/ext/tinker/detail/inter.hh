#ifndef TINKER_MOD_INTER_HH_
#define TINKER_MOD_INTER_HH_

#include "macro.h"

TINKER_NAMESPACE_BEGIN namespace inter {
extern double& einter;

#ifdef TINKER_MOD_CPP_
extern "C" double TINKER_MOD(inter, einter);

double& einter = TINKER_MOD(inter, einter);
#endif
} TINKER_NAMESPACE_END

#endif
#pragma once
#include "macro.h"


namespace tinker {
TINKER_EXTERN real* polarity;
TINKER_EXTERN real* thole;
TINKER_EXTERN real* pdamp;


//====================================================================//


TINKER_EXTERN real* polarity_inv;


TINKER_EXTERN real (*ufld)[3];
TINKER_EXTERN real (*dufld)[6];


TINKER_EXTERN real (*work01_)[3];
TINKER_EXTERN real (*work02_)[3];
TINKER_EXTERN real (*work03_)[3];
TINKER_EXTERN real (*work04_)[3];
TINKER_EXTERN real (*work05_)[3];
TINKER_EXTERN real (*work06_)[3];
TINKER_EXTERN real (*work07_)[3];
TINKER_EXTERN real (*work08_)[3];
TINKER_EXTERN real (*work09_)[3];
TINKER_EXTERN real (*work10_)[3];
}
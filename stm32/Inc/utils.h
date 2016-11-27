#pragma once

#define STR(x) _STR(x)
#define _STR(x) #x
#define ARR_LEN(a) (sizeof(a)/sizeof(*a))
#define STRZ_LEN(str) (sizeof(str)-1)

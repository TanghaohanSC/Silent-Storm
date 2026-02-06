#ifndef _ODE_CONFIG_H_
#define _ODE_CONFIG_H_
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h> 
#include <malloc.h> 
#define PENTIUM 1
typedef char int8;
typedef unsigned char uint8;

#ifndef _MAPEDIT
typedef int int32;
typedef unsigned int uint32;
#endif

/* an integer type that we can safely cast a pointer to and
 * from without loss of bits.
 */
typedef unsigned int intP;
/*select the base floating point type*/
#define dSINGLE 1
/*the floating point infinity*/
extern float dInfinityValue;
#define dInfinity dInfinityValue
#define DINFINITY_DECL float dInfinityValue = 1e20f;
#ifndef _DEBUG
#define dNODEBUG
#endif
#endif

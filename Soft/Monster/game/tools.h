#ifndef __TOOLS_H__
#define __TOOLS_H__
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <math.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// square root of the 2 and 3
const double sqrt_2 = 1.4142135623730950488016887242096980785696718753769;
const double sqrt_3 = sqrt( 3 );
const float FP_SQRT_2 = static_cast<float>( sqrt_2 );
const float FP_SQRT_3 = static_cast<float>( sqrt_3 );
// different constants with 'pi'
const double pi = 3.1415926535897932384626433832795028841971693993751;
const float FP_PI = static_cast<float>( pi );
const float FP_2PI = FP_PI * 2.0f;
const float FP_4PI = FP_PI * 4.0f;
const float FP_8PI = FP_PI * 8.0f;
const float FP_PI2 = FP_PI / 2.0f;
const float FP_PI4 = FP_PI / 4.0f;
const float FP_PI8 = FP_PI / 8.0f;
const float FP_INV_PI = 1.0f / FP_PI;
const float FP_EPSILON = 1e-12f;
const float FP_EPSILON2 = FP_EPSILON*FP_EPSILON;
// size of the static array
#define ARRAY_SIZE( a ) ( sizeof( a ) / sizeof( (a)[0] ) )
// access float as DWORD
#define FP_BITS( fp ) ( *reinterpret_cast<DWORD*>( &(fp) ) )
// clear MSb
#define FP_ABS_BITS( fp ) ( FP_BITS( fp ) & 0x7FFFFFFF )
// get MSb
#define FP_SIGN_BIT( fp ) ( FP_BITS( fp ) & 0x80000000 )
// floating pt 1.0
#define FP_ONE_BITS 0x3F800000
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// standard is long long but msvc does not support this name
typedef __int64 int64;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ************************************************************************************************************************ //
// трюки с битами
// ************************************************************************************************************************ //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// получить старший включенный бит
inline int GetMSB( DWORD n )
{
  int k = 0;
	if ( n & 0xFFFF0000 ) k = 16, n >>= 16;
  if ( n & 0x0000FF00 ) k += 8, n >>= 8;
  if ( n & 0x000000F0 ) k += 4, n >>= 4;
  if ( n & 0x0000000C ) k += 2, n >>= 2;
  if ( n & 0x00000002 ) k += 1;
	return k;
}
inline int GetMSB( WORD n )
{
  int k = 0;
  if ( n & 0xFF00 ) k  = 8, n >>= 8;
  if ( n & 0x00F0 ) k += 4, n >>= 4;
  if ( n & 0x000C ) k += 2, n >>= 2;
  if ( n & 0x0002 ) k += 1;
	return k;
}
inline int GetMSB( BYTE n )
{
  int k = 0;
  if ( n & 0xF0 ) k  = 4, n >>= 4;
  if ( n & 0x0C ) k += 2, n >>= 2;
  if ( n & 0x02 ) k += 1;
	return k;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// получить младший включенный бит
inline int GetLSB( DWORD n )
{
  int k = 0;
  if ( (n & 0x0000FFFF) == 0 ) k = 16, n >>= 16;
  if ( (n & 0x000000FF) == 0 ) k += 8, n >>= 8;
  if ( (n & 0x0000000F) == 0 ) k += 4, n >>= 4;
  if ( (n & 0x00000003) == 0 ) k += 2, n >>= 2;
  if ( (n & 0x00000001) == 0 ) k += 1;
	return k;
}
inline int GetLSB( WORD n )
{
  int k = 0;
  if ( (n & 0x00FF) == 0 ) k  = 8, n >>= 8;
  if ( (n & 0x000F) == 0 ) k += 4, n >>= 4;
  if ( (n & 0x0003) == 0 ) k += 2, n >>= 2;
  if ( (n & 0x0001) == 0 ) k += 1;
	return k;
}
inline int GetLSB( BYTE n )
{
  int k = 0;
  if ( (n & 0x0F) == 0 ) k  = 4, n >>= 4;
  if ( (n & 0x03) == 0 ) k += 2, n >>= 2;
  if ( (n & 0x01) == 0 ) k += 1;
	return k;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// подсчёт колличества ненулевых бит в числе
// 0x49249249ul // = 0100_1001_0010_0100_1001_0010_0100_1001
// 0x381c0e07ul // = 0011_1000_0001_1100_0000_1110_0000_0111
inline DWORD GetNumBits( DWORD v )
{
  v = (v & 0x49249249ul) + ((v >> 1) & 0x49249249ul) + ((v >> 2) & 0x49249249ul);
  v = ((v + (v >> 3)) & 0x381c0e07ul) + ((v >> 6) & 0x381c0e07ul);
  return (v + (v >> 9) + (v >> 18) + (v >> 27)) & 0x3f;
}
inline DWORD GetNumBits( BYTE v )
{
  v = (v & 0x55) + ((v >> 1) & 0x55);
  v = (v & 0x33) + ((v >> 2) & 0x33);
  return (v & 0x0f) + ((v >> 4) & 0x0f);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ************************************************************************************************************************ //
// обнуление памяти по типу переменной
// ************************************************************************************************************************ //
template <class TYPE>
inline void Zero( TYPE& val )
{
	memset( &val, 0, sizeof(val) );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ************************************************************************************************************************ //
// побитовое приведение одного типа к другому
// ************************************************************************************************************************ //
template <class TYPE_OUT, class TYPE_IN>
inline TYPE_OUT& bit_cast( TYPE_IN &val )
{
	return *reinterpret_cast<TYPE_OUT*>( &val );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// square and triple functions
template <class TYPE>
inline TYPE square( const TYPE x )
{
	return x*x;
}
template <class TYPE>
inline TYPE sqr( const TYPE x )
{
	return x*x;
}
template <class TYPE>
inline TYPE triple( const TYPE x )
{
	return x*x*x;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ************************************************************************************************************************ //
// работа со знаком числа
// ************************************************************************************************************************ //
// signum function
template <class TYPE>
inline TYPE Sign( const TYPE x )
{
	if ( x < 0 )
		return -1;
	else if ( x > 0 )
		return +1;
	else
		return 0;
}
// calculates sign for integer variable, returns -1, 0, 1. template specialization
//#pragma warning( disable: 4035 ) // compiler can and does produce wrong code in this case with optimisations turned on
template <>
inline int Sign<int>( const int nVal )
{
	int nRes;
	_asm
	{
		xor ecx, ecx
		mov eax, nVal
		test eax, 0x7FFFFFFF
		setne cl
		sar eax, 31
		or eax, ecx
		mov nRes, eax
	}
	return nRes;
}
template <>
inline short int Sign<short int>( const short int nVal )
{
	short int nRes;
	_asm
	{
		xor ecx,ecx
		mov ax, nVal
		test ax, 0x7FFF
		setne cl
		sar ax, 15
		or ax, cx
		mov nRes, ax
	}
	return nRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ************************************************************************************************************************ //
// radian <=> degree conversion functions
// ************************************************************************************************************************ //
template <class TYPE>
inline TYPE ToRadian( const TYPE angle )
{
	return static_cast<TYPE>( angle * (pi/180.0) );
}
template <class TYPE>
inline TYPE ToDegree( const TYPE angle )
{
	return static_cast<TYPE>( angle * (180.0/pi) );
}
inline float NormalizeAngleInDegree( const float angle )
{
	return static_cast<float>( fmod( angle, 360.0 ) );
}
inline int NormalizeAngleInDegree( const int angle )
{
	return angle % 360;
}
inline float SignumNormalizeAngleInDegree( const float angle )
{
	return static_cast<float>( fmod( angle + 180*Sign(angle),  360 ) - 180 * Sign( angle ) );
}
inline float NormalizeAngleInRadian( const float angle )
{
	return static_cast<float>( fmod( angle, (2.0*pi) ) );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ************************************************************************************************************************ //
// обнуление участка памяти, состоящего из DWORD'ов
// ************************************************************************************************************************ //
inline void MemSetDWord( DWORD* lpData, const DWORD value, const int nCount )
{
	_asm
	{
		mov ecx, nCount
		mov edi, lpData
		mov eax, value
		rep stosd
	}
}

inline void MemSetInt( int* lpData, const int value, const int nCount )
{
	_asm
	{
		mov ecx, nCount
		mov edi, lpData
		mov eax, value
		rep stosd
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ************************************************************************************************************************ //
// ** float-to-int преобразование с текущим состоянием процессора
// ************************************************************************************************************************ //
// very fast float-to-int conversion. uses current FPU rounding state
inline int Float2Int( const float fpVar )
{
	int nRet;
	__asm {
		fld dword ptr fpVar
		fistp nRet
	};
	return nRet;
}
inline int MINT( const float f ) { return Float2Int(f); }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ************************************************************************************************************************ //
// ** всевозможные бастрые функции типа 'x' @ 'y' ? 'val1' : 'val2'
// ************************************************************************************************************************ //
// very fast comparison: 'x' < 'y' ? val1 : val2
inline float select_lt( const float x, const float y, const float val1, const float val2 )
{
	float z;
	_asm
	{
		// loading 'x' and compare with 'y'
		xor					eax, eax
		fld         dword ptr [x]
		fcomp       dword ptr [y]
		// store compare flags
		fnstsw      ax
		// test comparison result and set '1' or '0'
		// load 'val1' and 'val2'
		mov					ebx, [val1]
		mov					ecx, [val2]
		// create mask for merging
		and					eax, 0100h
		shl					eax, 23
		sar					eax, 31
		// merging: (val1 & mask) | (val2 & ~mask)
		and					ebx, eax
		not					eax
		and					ecx, eax
		or					ebx, ecx

		mov					[z], ebx
	}
	return z;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// very fast comparison: 'x' > 'y' ? val1 : val2
inline float select_gt( const float x, const float y, const float val1, const float val2 )
{
	float z;
	_asm
	{
		// loading 'x' and compare with 'y'
		xor					eax, eax
		fld         dword ptr [x]
		fcomp       dword ptr [y]
		// store compare flags
		fnstsw      ax
		// test comparison result and set '1' or '0'
		// load 'val1' and 'val2'
		mov					ebx, [val1]
		mov					ecx, [val2]
		// create mask for merging
		test        ah, 41h
		sete				al
		shl					eax, 31
		sar					eax, 31
		// merging: (val1 & mask) | (val2 & ~mask)
		and					ebx, eax
		not					eax
		and					ecx, eax
		or					ebx, ecx

		mov					[z], ebx
	}
	return z;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// very fast comparison: 'x' <= 'y' ? val1 : val2
inline float select_le( const float x, const float y, const float val1, const float val2 )
{
	float z;
	_asm
	{
		// loading 'x' and compare with 'y'
		xor					eax, eax
		fld         dword ptr [x]
		fcomp       dword ptr [y]
		// store compare flags
		fnstsw      ax
		// test comparison result and set '1' or '0'
		// load 'val1' and 'val2'
		mov					ebx, [val1]
		mov					ecx, [val2]
		// create mask for merging
		test        ah, 41h
		setne				al
		shl					eax, 31
		sar					eax, 31
		// merging: (val1 & mask) | (val2 & ~mask)
		and					ebx, eax
		not					eax
		and					ecx, eax
		or					ebx, ecx

		mov					[z], ebx
	}
	return z;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// very fast comparison: 'x' >= 'y' ? val1 : val2
inline float select_ge( const float x, const float y, const float val1, const float val2 )
{
	float z;
	_asm
	{
		// loading 'x' and compare with 'y'
		xor					eax, eax
		fld         dword ptr [x]
		fcomp       dword ptr [y]
		// store compare flags
		fnstsw      ax
		// test comparison result and set '1' or '0'
		// load 'val1' and 'val2'
		mov					ebx, [val1]
		mov					ecx, [val2]
		// create mask for merging
		and					eax, 0100h
		xor					eax, 0100h
		shl					eax, 23
		sar					eax, 31
		// merging: (val1 & mask) | (val2 & ~mask)
		and					ebx, eax
		not					eax
		and					ecx, eax
		or					ebx, ecx

		mov					[z], ebx
	}
	return z;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// very fast comparison: 'x' == 'y' ? val1 : val2
inline float select_eq( const float x, const float y, const float val1, const float val2 )
{
	float z;
	_asm
	{
		// loading 'x' and compare with 'y'
		xor					eax, eax
		mov ebx, [x]
		cmp ebx, [y]
		// test comparison result and set '1' or '0'
		// load 'val1' and 'val2'
		// create mask for merging
		sete				al
		mov					ebx, [val1]
		shl					eax, 31
		mov					ecx, [val2]
		sar					eax, 31
		// merging: (val1 & mask) | (val2 & ~mask)
		and					ebx, eax
		not					eax
		and					ecx, eax
		or					ebx, ecx

		mov					[z], ebx
	}
	return z;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// very fast comparison: 'x' != 'y' ? val1 : val2
inline float select_ne( const float x, const float y, const float val1, const float val2 )
{
	float z;
	_asm
	{
		// loading 'x' and compare with 'y'
		xor					eax, eax
		mov ebx, [x]
		cmp ebx, [y]
		// test comparison result and set '1' or '0'
		// load 'val1' and 'val2'
		// create mask for merging
		setne				al
		mov					ebx, [val1]
		shl					eax, 31
		mov					ecx, [val2]
		sar					eax, 31
		// merging: (val1 & mask) | (val2 & ~mask)
		and					ebx, eax
		not					eax
		and					ecx, eax
		or					ebx, ecx

		mov					[z], ebx
	}
	return z;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// performs the next check:
//			( (xp <= -wp)      ) |
//			( (xp >   wp) << 1 ) |
//			( (yp <= -wp) << 2 ) |
//			( (yp >   wp) << 3 ) |
//			( (zp <=  0 ) << 4 ) |
//			( (zp >   wp) << 5 );
inline BYTE CheckForViewingFrustum( float xp, float yp, float zp, float wp )
{
	float wp2 = wp + wp;
	BYTE value;

	xp += wp;
	yp += wp;

	_asm
	{
 		// xp <= 0, yp <= 0, zp <= 0
 		mov 	edx, xp
 		mov 	eax, yp
 		shr 	edx, 31
 		mov 	ecx, zp
 		shr 	eax, 31
 		shr 	ecx, 31
 		shl 	eax, 2
 		shl 	ecx, 4
 		or 		edx, eax
 		// xp > wp2, yp > wp2, zp > wp
 		fld 	[wp2]
 		fcomp [xp]
 		or 		edx, ecx
 		fnstsw ax
 		fld 	[wp2]
 		and 	eax, 0100h
 		fcomp [yp]
 		shr 	eax, 7
 		or 		edx, eax
 		fnstsw ax
 		fld 	[wp]
 		and 	eax, 0100h
 		fcomp [zp]
 		shr 	eax, 5
 		or 		edx, eax
 		fnstsw ax
 		and 	eax, 0100h
 		shr 	eax, 3
		// form return value in eax and move it to 'value'
		or 		eax, edx
		mov [value], al
	};

	return value;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// min/max functions
template <class TYPE>
inline TYPE Min( const TYPE val1, const TYPE val2 )
{
	return (val1 < val2 ? val1 : val2);
}
template <class TYPE>
inline TYPE Max( const TYPE val1, const TYPE val2 )
{
	return (val1 > val2 ? val1 : val2);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// returns minimum of two float values
template<>
inline float Min<float>( const float a, const float b )
{
	float fpRet;
	_asm
	{
		// comparing
		fld     dword ptr [b]
		fcomp		dword ptr [a]
		fnstsw  ax
		mov			ecx, dword ptr [b]
		shl			eax, 23
		sar			eax, 31
		// merging: (val1 & mask) | (val2 & ~mask)
		and			ecx, eax
		not			eax
		and			eax, dword ptr [a]
		or			eax, ecx
		mov			[fpRet], eax
	}
	return fpRet;
}
// returns minimum of two float values
template<>
inline float Max<float>( const float a, const float b )
{
	float fpRet;
	_asm
	{
		// comparing
		fld     dword ptr [a]
		fcomp		dword ptr [b]
		fnstsw  ax
		mov			ecx, dword ptr [b]
		shl			eax, 23
		sar			eax, 31
		// merging: (val1 & mask) | (val2 & ~mask)
		and			ecx, eax
		not			eax
		and			eax, dword ptr [a]
		or			eax, ecx
		mov			[fpRet], eax
	}
	return fpRet;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Calc normalized canonical line equation 'ax + by + c = 0'
inline void GetLineEq( const float x1, const float y1, const float x2, const float y2, float &a, float &b, float &c )
{
	float ta = y1 - y2;
	float tb = x2 - x1;
	float tc = -x1*ta - y1*tb;
	float fMod = ta*ta + tb*tb;
	if ( fMod < 1e-7 )
	{
		a = 0.7071f;
		b = -0.7071f;
		c = 0;
		return;
	}
	float sq = static_cast<float>( sqrt( fMod ) );
	a = ta / sq;
	b = tb / sq;
	c = tc / sq;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ************************************************************************************************************************ //
// получение модуля от разных величин
// ************************************************************************************************************************ //
inline float fabs2( const float x, const float y, const float z, const float w )
{
	return ( x*x + y*y + z*z + w*w );
}
inline float fabs( const float x, const float y, const float z, const float w )
{
	return static_cast<float>( sqrt( fabs2(x, y, z, w) ) );
}
inline float fabs2( const float x, const float y, const float z )
{
	return ( x*x + y*y + z*z );
}
inline float fabs( const float x, const float y, const float z )
{
	return static_cast<float>( sqrt( fabs2(x, y, z) ) );
}
inline float fabs2( const float x, const float y )
{
	return ( x*x + y*y );
}
inline float fabs( const float x, const float y )
{
	return static_cast<float>( sqrt( fabs2(x, y) ) );
}
inline float fabs2( const float x )
{
	return x*x;
}
inline float fabs( float x )
{
	return *(float*)( &( (*((int*)&x)) &= 0x7fffFFFF ) );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TYPE> 
inline bool Normalize( TYPE &x, TYPE &y )
{
  TYPE u = fabs2( x, y );
  if ( fabs(u - TYPE(1)) < FP_EPSILON )
    return true;                        // already normalized
  if ( u < FP_EPSILON2 )
    return false;                       // can't normalize
  u = static_cast<TYPE>( TYPE(1) / sqrt( u ) );
  x *= u;
  y *= u;
  return true;
}
template <class TYPE> 
inline bool Normalize( TYPE &x, TYPE &y, TYPE &z )
{
  TYPE u = fabs2( x, y, z );
  if ( fabs(u - TYPE(1)) < FP_EPSILON )
    return true;                        // already normalized
  if ( u < FP_EPSILON2 )
    return false;                       // can't normalize
  u = static_cast<TYPE>( TYPE(1) / sqrt( u ) );
  x *= u;
  y *= u;
  z *= u;
  return true;
}
template <class TYPE> 
inline bool Normalize( TYPE &x, TYPE &y, TYPE &z, TYPE &w )
{
  TYPE u = fabs2( x, y, z, w );
  if ( fabs(u - TYPE(1)) < FP_EPSILON )
    return true;                        // already normalized
  if ( u < FP_EPSILON2 )
    return false;                       // can't normalize
  u = static_cast<TYPE>( TYPE(1) / sqrt( u ) );  
  x *= u;
  y *= u;
  z *= u;
  w *= u;
  return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline float cos( float fVal ) { return static_cast<float>( cos( double(fVal) ) ); }
inline float sin( float fVal ) { return static_cast<float>( sin( double(fVal) ) ); }
inline float acos( float fVal ) { return static_cast<float>( acos( double(fVal) ) ); }
inline float asin( float fVal ) { return static_cast<float>( asin( double(fVal) ) ); }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MINIMIZE_INT( nToMin, nHow )  \
	_asm mov ecx, nToMin                \
	_asm cmp ecx, nHow                  \
	_asm setl al                        \
	_asm shl eax, 31                    \
	_asm sar eax, 31                    \
	_asm and ecx, eax                   \
	_asm not eax                        \
	_asm and eax, nHow                  \
	_asm or ecx, eax                    \
	_asm mov nToMin, ecx

#define MINIMIZE_UINT( nToMin, nHow ) \
	_asm mov ecx, nToMin                \
	_asm cmp ecx, nHow                  \
	_asm setb al                        \
	_asm shl eax, 31                    \
	_asm sar eax, 31                    \
	_asm and ecx, eax                   \
	_asm not eax                        \
	_asm and eax, nHow                  \
	_asm or ecx, eax                    \
	_asm mov nToMin, ecx
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TYPE>
const TYPE Clamp( const TYPE tVal, const TYPE tMin, const TYPE tMax )
{
  return Max( tMin, Min(tVal, tMax) );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Copy8Bytes( void* fpDst, const void* fpSrc )
{
	_asm
	{
		mov ecx, [ fpSrc ]
		mov edx, [ fpDst ]
		fild qword ptr [ ECX ]
		fistp qword ptr [ EDX ]
	}
}

inline void Copy16Bytes( void* fpDst, const void* fpSrc )
{
	_asm
	{
		mov ecx, [ fpSrc ]
		mov edx, [ fpDst ]
		fild qword ptr [ ECX ]
		fistp qword ptr [ EDX ]
		fild qword ptr [ ECX + 8 ]
		fistp qword ptr [ EDX + 8 ]
	}
}

inline void Copy32Bytes( void* fpDst, const void* fpSrc )
{
	_asm
	{
		mov ecx, [ fpSrc ]
		mov edx, [ fpDst ]
		fild qword ptr [ ECX ]
		fistp qword ptr [ EDX ]
		fild qword ptr [ ECX + 8 ]
		fistp qword ptr [ EDX + 8 ]
		fild qword ptr [ ECX + 16 ]
		fistp qword ptr [ EDX + 16 ]
		fild qword ptr [ ECX + 24 ]
		fistp qword ptr [ EDX + 24 ]
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const DWORD CPUID_MMX_FEATURE_PRESENT = 0x00800000;
const DWORD CPUID_SSE_FEATURE_PRESENT = 0x02000000;
#define GET_CPUID __asm _emit 0x0f __asm _emit 0xa2
inline void GetCPUID( char str[13], DWORD& dwRes )
{
	static char szRez[13];
	static DWORD dwRes1;
	szRez[12] = 0;
	_asm
	{
		pusha                               // keep compiler happy
		pushfd 															// get extended flags
		pop eax 														// store extended flags in eax
		mov ebx, eax 												// save current flags
		xor eax, 200000h 										// toggle bit 21
		push eax 														// put new flags on stack
		popfd 															// flags updated now in flags
		pushfd 															// get extended flags
		pop eax 														// store extended flags in eax
		xor eax, ebx 												// if bit 21 r/w then eax <> 0
		je q  															// can't toggle id bit (21) no cpuid here

		mov	eax, 1
		GET_CPUID                           // perform CPUID command
		mov dwRes1, edx                     // store CPUID in dwRes1
		mov dword ptr[szRez], eax           // store ASCII vendor name
		mov dword ptr[szRez]+4, ebx         // in the szRez
		mov dword ptr[szRez]+8, ecx         //
	q:
		popa
	}
	strcpy( str, szRez );
	dwRes = dwRes1;
}
#undef GET_CPUID
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // __TOOLS_H__

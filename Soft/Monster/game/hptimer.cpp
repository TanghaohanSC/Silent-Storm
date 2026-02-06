#include "StdAfx.h"
#include "HPTimer.h"
/////////////////////////////////////////////////////////////////////////////////////
using namespace NHPTimer;
static double fpProcFreq1 = 1;
/////////////////////////////////////////////////////////////////////////////////////
double NHPTimer::GetSeconds( const STime &a )
{
	return (double(a)) * fpProcFreq1;
}
/////////////////////////////////////////////////////////////////////////////////////
// Time counters
/////////////////////////////////////////////////////////////////////////////////////
static inline void GetCounter( int64 *pTime )
{
	__asm
	{
		rdtsc
		mov esi, pTime
		mov [esi], eax
		mov [esi+4], edx
	}
}
/////////////////////////////////////////////////////////////////////////////////////
double NHPTimer::GetClockRate()
{
	return 1 / fpProcFreq1;
}
/////////////////////////////////////////////////////////////////////////////////////
void NHPTimer::GetTime( STime *pTime )
{
	GetCounter( pTime );
}
/////////////////////////////////////////////////////////////////////////////////////
double NHPTimer::GetTimePassed( STime *pTime )
{
	STime old(*pTime );
	GetTime( pTime );
	return GetSeconds( *pTime - old );
}
/////////////////////////////////////////////////////////////////////////////////////
static void InitHPTimer()
{
	int64 freq, start, fin;
	QueryPerformanceFrequency( (_LARGE_INTEGER*) &freq );
	double fpTStart, fpTFinish;
	STime t;
	GetTime( &t );
	
	QueryPerformanceCounter( (_LARGE_INTEGER*) &start );

	Sleep( 100 );

	double fpPassed = GetTimePassed( &t );
	QueryPerformanceCounter( (_LARGE_INTEGER*) &fin );
	fpTStart = start;
	fpTFinish = fin;
	
	double fpProcFreq = (fpPassed) / (fpTFinish-fpTStart) * (double( freq ));
	fpProcFreq1 = 1 / fpProcFreq;
	//cout << "freq = " << fpProcFreq / 1000000 <<  "Mhz" << endl;
}
/////////////////////////////////////////////////////////////////////////////////////
// это вспомогательная структура для автоматической инициализации HP timer'а
struct SHPTimerInit
{
	SHPTimerInit() { InitHPTimer(); }
};
static SHPTimerInit hptInit;
/////////////////////////////////////////////////////////////////////////////////////

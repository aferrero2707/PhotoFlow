#ifndef _CORE_HELPERS_H_
#define _CORE_HELPERS_H_

//#include <Windows.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <math.h>

#define restrict(x)
#define __int64 long long int
#define BOOL bool

#define ASSERT_ALWAYS(expression) ALWAYS_ASSERT_RAW(expression,__FILE__,__LINE__,__FUNCTION__,#expression)

void ALWAYS_ASSERT_RAW(bool cond, const char fileName[], const int lineNum, const char funcName[], const char expression[]);

#define MY_SAFE_RELEASE(p) { if (p!=NULL) { p->Release(); p = NULL; } }

inline float SafeInv(float x) restrict(amp) restrict(cpu)
{
	return x == 0 ? 0.0f : 1.0f/x;
}

inline float FastSqr(float x) restrict(amp) restrict(cpu)
{
	return x*x;
}

inline float MaxFloat(float x, float y) restrict(amp) restrict(cpu)
{
	return x > y ? x : y;
}

inline float MinFloat(float x, float y) restrict(amp) restrict(cpu)
{
	return x < y ? x : y;
}

inline int MaxInt(int x, int y) restrict(amp) restrict(cpu)
{
	return x > y ? x : y;
}

inline int MinInt(int x, int y) restrict(amp) restrict(cpu)
{
	return x < y ? x : y;
}

inline float RandNorm()
{
	//return float(rand()%10001)/10000.0f;
	return 0;
}

inline float Abs(float f) restrict(amp) restrict(cpu)
{
	return f >= 0.0f ? f : -f;
}

inline float Saturate(float x) restrict(amp) restrict(cpu)
{
	return MaxFloat(0.0f,MinFloat(1.0f,x));
}

inline float Saturate255(float x) restrict(amp) restrict(cpu)
{
	return MaxFloat(0.0f,MinFloat(255.0f,x));
}

inline unsigned char ColorToChar(float x)
{
	return (unsigned char)Saturate255(x*256.0f);
}

template <class A>
inline void Swap(A & lhs, A & rhs) restrict(amp) restrict(cpu)
{
	A temp = lhs;
	lhs = rhs;
	rhs = temp;
}


inline std::string FourDigitString(int i)
{
	char camName[2048];
	sprintf(camName,"%04d",i);
	return std::string(camName);
}


inline unsigned __int64 GetQualityTimeMicroSec()
{
/*
  LARGE_INTEGER freq;
	LARGE_INTEGER currTime;
	BOOL bRet;
	bRet = QueryPerformanceFrequency(&freq);
	ASSERT_ALWAYS(bRet);

	bRet = QueryPerformanceCounter(&currTime);
	ASSERT_ALWAYS(bRet);

	double numSec = ((double)currTime.QuadPart)/((double)freq.QuadPart);
	unsigned __int64 microSec = (unsigned __int64)(numSec * 1000000.0);
	return microSec;
*/
  return 0;
}




template <class A, class B>
inline A AlignSize(A x, B size)
{
	return (((x+size)-1)/size)*size;
/*
	A extra = (x%A(size));
	A ret = x;
	if (extra != 0)
		ret += A(size)-extra;
	return ret;
	*/
}

inline unsigned __int64 AlignSize64(unsigned __int64 x, unsigned __int64 size)
{
	return (((x+size)-1)/size)*size;
}

inline unsigned int AlignSize32(unsigned int x, unsigned int size)
{
	return (((x+size)-1)/size)*size;
}


std::string GetNextLineFromFile(FILE * fin, bool & isEof);

std::string LocalTimeAsString();


#endif


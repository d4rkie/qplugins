//-----------------------------------------------------------------------------
// 
// File:	AsioOutputConvert.h
//
// About:	ASIO host output conversion routines
//
// Author:	Ted Hess
//
//	QMP multimedia player application Software Development Kit Release 5.0.
//
//	This code is free.  If you redistribute it in any form, leave this notice 
//	here.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//-----------------------------------------------------------------------------

#define	ISCALER8			0x7f
#define	ISCALER16			0x7fff
#define	ISCALER24			0x7fffff
#define	ISCALER32			0x7fffffff

#define	FSCALER8			static_cast<double>(ISCALER8)
#define	FSCALER16			static_cast<double>(ISCALER16)
#define	FSCALER24			static_cast<double>(ISCALER24)
#define	FSCALER32			static_cast<double>(ISCALER32)

//
// Utilities as inline functions (code speed most important)
//

// Covert 3 bytes (24-bit) into long (sign extended)
inline long __fastcall Cvt24To32(BYTE *val)
{
	BYTE retval[4];

	*(WORD *)&retval[0] = *(WORD *)(val);
	*(WORD *)&retval[2] = *(char *)(val + 2);

	return *((long *)retval);
}

// Copy 3 bytes (24-bit data)
inline void	__fastcall Copy3(BYTE *dest, BYTE *src)
{
	*(WORD *)(dest) = *(WORD *)(src);
	*(dest + 2) = *(src + 2);
}

//
// Byte swappers for big-endian devices
//
inline short __fastcall SwapShort(short val)
{
	BYTE retval[2];
	
	retval[0] = *((BYTE *)&val + 1);
	retval[1] = *((BYTE *)&val);

	return *((short *)retval);
}

inline long __fastcall SwapLong(long val)
{
	BYTE retval[4];

	retval[0] = *((BYTE *)&val + 3);
	retval[1] = *((BYTE *)&val + 2);
	retval[2] = *((BYTE *)&val + 1);
	retval[3] = *((BYTE *)&val + 0);

	return *((long *)retval);
}

inline __int64 __fastcall SwapLongLong(__int64 val)
{
	BYTE retval[8];

	retval[0] = *((BYTE *)&val + 7);
	retval[1] = *((BYTE *)&val + 6);
	retval[2] = *((BYTE *)&val + 5);
	retval[3] = *((BYTE *)&val + 4);
	retval[4] = *((BYTE *)&val + 3);
	retval[5] = *((BYTE *)&val + 2);
	retval[6] = *((BYTE *)&val + 1);
	retval[7] = *((BYTE *)&val + 0);

	return *((__int64 *)&retval);
}

// Convert double to int (rounded)
inline __int64 __fastcall RInt64(double Num)
{
	__int64 intNum;

	_asm
	{	
		fld Num
		fistp intNum
	};

	return intNum;
}

// Convert double to int (rounded)
inline int __fastcall RInt32(double Num)
{
	int intNum;

	_asm
	{	
		fld Num
		fistp intNum
	};

	return intNum;
}

//
// LSB normalization
//

// Normalize Float32
inline void __fastcall ConvertFloatToFloat32(float Data, float *wrptr)
{
	if (Data > 1.0)
		Data = 1.0;
	else 
		if (Data < -1.0)
			Data = -1.0;

	*wrptr = Data;
}

// Normalize 16-bit
inline void __fastcall ConvertFloatToInt16(double Data, short *wrptr)
{
	int v = RInt32(Data);

	if (v > 0x7fff)
		v = 0x7fff;
	else
		if (v < -0x8000)
			v = -0x8000;

	*wrptr = v;
}


// Normalize 24-bit
inline void __fastcall ConvertFloatToInt24(double Data, BYTE *wrptr)
{
	__int64	v = RInt64(Data);

	if (v > 0x7fffffLL)
		v = 0x7fffffLL;
	else
		if (v < -0x800000LL)
			v = -0x800000LL;

	Copy3(wrptr, (BYTE *)&v);
}
// Normalize 32-bit aligned 16-bit
inline void __fastcall ConvertFloatToInt32b16(double Data, long *wrptr)
{
	int		v = RInt32(Data);

	if (v > 0x7fff)
		v = 0x7fff;
	else
		if (v < -0x8000)
			v = -0x8000;
	
	*wrptr = v;
}

// Normalize 32-bit aligned 24-bit

inline void __fastcall ConvertFloatToInt32b24(double Data, long *wrptr)
{
	__int64	v = RInt64(Data);

	if (v > 0x7fffffLL)
		v = 0x7fffffLL;
	else
		if (v < -0x800000LL)
			v = -0x800000LL;

	*wrptr = (long)v;
}

//
// MSB normalization
//

// Normalize Float32
inline void __fastcall ConvertFloatToFloat32_S(float Data, float *wrptr)
{
	if (Data > 1.0)
		Data = 1.0;
	else 
		if (Data < -1.0)
			Data = -1.0;

	*(long *)wrptr = SwapLong((long)Data);
}

// Normalize 16-bit
inline void __fastcall ConvertFloatToInt16_S(double Data, short *wrptr)
{
	int v = RInt32(Data);

	if (v > 0x7fff)
		v = 0x7fff;
	else
		if (v < -0x8000)
			v = -0x8000;

	*wrptr = SwapShort(v);
}


// Normalize 24-bit
inline void __fastcall ConvertFloatToInt24_S(double Data, BYTE *wrptr)
{
	__int64	v = RInt64(Data);

	if (v > 0x7fffffLL)
		v = 0x7fffffLL;
	else
		if (v < -0x800000LL)
			v = -0x800000LL;

	*(wrptr + 0) = *((BYTE *)&v + 2);
	*(wrptr + 1) = *((BYTE *)&v + 1);
	*(wrptr + 2) = *((BYTE *)&v + 0);
}

// Normalize 32-bit aligned 16-bit
inline void __fastcall ConvertFloatToInt32b16_S(double Data, long *wrptr)
{
	int		v = RInt32(Data);

	if (v > 0x7fff)
		v = 0x7fff;
	else
		if (v < -0x8000)
			v = -0x8000;
	
	*wrptr = SwapLong(v);
}

// Normalize 32-bit aligned 24-bit

inline void __fastcall ConvertFloatToInt32b24_S(double Data, long *wrptr)
{
	__int64	v = RInt64(Data);

	if (v > 0x7fffffLL)
		v = 0x7fffffLL;
	else
		if (v < -0x800000LL)
			v = -0x800000LL;

	*wrptr = SwapLong((long)v);
}

//
// Generic output converter function template
//
typedef	void (__fastcall *CONVERTER_FUNC)(void *dest, void *src, int count);

//
// Map ASIO types to conversion function
//
void *GetConvertToAsioFunc(ASIOSampleType Type, DWORD WaveFormat, DWORD BytesPerSample);

//
// LSB converters
//
void __fastcall ConvertInt8ToInt16(short *dest, BYTE *src, int count);
void __fastcall	ConvertInt8ToInt24(BYTE *dest, BYTE *src, int count);
void __fastcall ConvertInt8ToInt32(long *dest, BYTE *src, int count);
void __fastcall ConvertInt8ToInt32b16(long *dest, BYTE *src, int count);
void __fastcall ConvertInt8ToInt32b24(long *dest, BYTE *src, int count);
void __fastcall ConvertInt8ToFloat32(float *dest, BYTE *src, int count);

void __fastcall ConvertInt16ToInt16(short *dest, short *src, int count);
void __fastcall	ConvertInt16ToInt24(BYTE *dest, short *src, int count);
void __fastcall ConvertInt16ToInt32(long *dest, short *src, int count);
void __fastcall ConvertInt16ToInt32b16(long *dest, short *src, int count);
void __fastcall ConvertInt16ToInt32b24(long *dest, short *src, int count);
void __fastcall ConvertInt16ToFloat32(float *dest, short *src, int count);

void __fastcall ConvertInt24ToInt16(short *dest, BYTE *src, int count);
void __fastcall	ConvertInt24ToInt24(BYTE *dest, BYTE *src, int count);
void __fastcall ConvertInt24ToInt32(long *dest, BYTE *src, int count);
void __fastcall ConvertInt24ToInt32b16(long *dest, BYTE *src, int count);
void __fastcall ConvertInt24ToInt32b24(long *dest, BYTE *src, int count);
void __fastcall ConvertInt24ToFloat32(float *dest, BYTE *src, int count);

void __fastcall ConvertInt32ToInt16(short *dest, long *src, int count);
void __fastcall ConvertInt32ToInt24(BYTE *dest, long *src, int count);
void __fastcall ConvertInt32ToInt32(long *dest, long *src, int count);
void __fastcall ConvertInt32ToInt32b16(long *dest, long *src, int count);
void __fastcall ConvertInt32ToInt32b24(long *dest, long *src, int count);
void __fastcall ConvertInt32ToFloat32(float *dest, long *src, int count);

void __fastcall ConvertFloat32ToInt16(short *dest, float *src, int count);
void __fastcall ConvertFloat32ToInt24(BYTE *dest, float *src, int count);
void __fastcall ConvertFloat32ToInt32(long *dest, float *src, int count);
void __fastcall ConvertFloat32ToInt32b16(long *dest, float *src, int count);
void __fastcall ConvertFloat32ToInt32b24(long *dest, float *src, int count);
void __fastcall ConvertFloat32ToFloat32(float *dest, float *src, int count);

//
// MSB converters
//
void __fastcall ConvertInt8ToInt16_S(short *dest, BYTE *src, int count);
void __fastcall	ConvertInt8ToInt24_S(BYTE *dest, BYTE *src, int count);
void __fastcall ConvertInt8ToInt32_S(long *dest, BYTE *src, int count);
void __fastcall ConvertInt8ToInt32b16_S(long *dest, BYTE *src, int count);
void __fastcall ConvertInt8ToInt32b24_S(long *dest, BYTE *src, int count);
void __fastcall ConvertInt8ToFloat32_S(float *dest, BYTE *src, int count);

void __fastcall ConvertInt16ToInt16_S(short *dest, short *src, int count);
void __fastcall	ConvertInt16ToInt24_S(BYTE *dest, short *src, int count);
void __fastcall ConvertInt16ToInt32_S(long *dest, short *src, int count);
void __fastcall ConvertInt16ToInt32b16_S(long *dest, short *src, int count);
void __fastcall ConvertInt16ToInt32b24_S(long *dest, short *src, int count);
void __fastcall ConvertInt16ToFloat32_S(float *dest, short *src, int count);

void __fastcall ConvertInt24ToInt16_S(short *dest, BYTE *src, int count);
void __fastcall	ConvertInt24ToInt24_S(BYTE *dest, BYTE *src, int count);
void __fastcall ConvertInt24ToInt32_S(long *dest, BYTE *src, int count);
void __fastcall ConvertInt24ToInt32b16_S(long *dest, BYTE *src, int count);
void __fastcall ConvertInt24ToInt32b24_S(long *dest, BYTE *src, int count);
void __fastcall ConvertInt24ToFloat32_S(float *dest, BYTE *src, int count);

void __fastcall ConvertInt32ToInt16_S(short *dest, long *src, int count);
void __fastcall ConvertInt32ToInt24_S(BYTE *dest, long *src, int count);
void __fastcall ConvertInt32ToInt32_S(long *dest, long *src, int count);
void __fastcall ConvertInt32ToInt32b16_S(long *dest, long *src, int count);
void __fastcall ConvertInt32ToInt32b24_S(long *dest, long *src, int count);
void __fastcall ConvertInt32ToFloat32_S(float *dest, long *src, int count);

void __fastcall ConvertFloat32ToInt16_S(short *dest, float *src, int count);
void __fastcall ConvertFloat32ToInt24_S(BYTE *dest, float *src, int count);
void __fastcall ConvertFloat32ToInt32_S(long *dest, float *src, int count);
void __fastcall ConvertFloat32ToInt32b16_S(long *dest, float *src, int count);
void __fastcall ConvertFloat32ToInt32b24_S(long *dest, float *src, int count);
void __fastcall ConvertFloat32ToFloat32_S(float *dest, float *src, int count);

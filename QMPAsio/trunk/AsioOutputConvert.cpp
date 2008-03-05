//-----------------------------------------------------------------------------
// 
// File:	AsioOutputConvert.cpp
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

#include "stdafx.h"
#include "QMPAsio.h"

//-------------------------------------------------------------------------------------------------------------------------------------------------------

//
// Map input sample size & wave format to approriate ASIO sample type
//
void *GetConvertToAsioFunc(ASIOSampleType Type, DWORD WaveFormat, DWORD BitsPerSample)
{
	switch (BitsPerSample)
	{
	case 8:
		switch (Type)
		{
		case ASIOSTInt16LSB:
			return ConvertInt8ToInt16;
		case ASIOSTInt24LSB:
			return ConvertInt8ToInt24;
		case ASIOSTInt32LSB:
			return ConvertInt8ToInt32;
		case ASIOSTInt32LSB16:
			return ConvertInt8ToInt32b16;
		case ASIOSTInt32LSB24:
			return ConvertInt8ToInt32b24;
		case ASIOSTFloat32LSB:
			return ConvertInt8ToFloat32;
		case ASIOSTInt16MSB:
			return ConvertInt8ToInt16_S;
		case ASIOSTInt24MSB:
			return ConvertInt8ToInt24_S;
		case ASIOSTInt32MSB:
			return ConvertInt8ToInt32_S;
		case ASIOSTInt32MSB16:
			return ConvertInt8ToInt32b16_S;
		case ASIOSTInt32MSB24:
			return ConvertInt8ToInt32b24_S;
		case ASIOSTFloat32MSB:
			return ConvertInt8ToFloat32_S;
		default:
			OutputDebugString(_T("*** Unsupported conversion of 8bit data to ASIO\n"));
			_ASSERT(FALSE);
		}
		break;
	case 16:
		switch (Type)
		{
		case ASIOSTInt16LSB:
			return ConvertInt16ToInt16;
		case ASIOSTInt24LSB:
			return ConvertInt16ToInt24;
		case ASIOSTInt32LSB:
			return ConvertInt16ToInt32;
		case ASIOSTInt32LSB16:
			return ConvertInt16ToInt32b16;
		case ASIOSTInt32LSB24:
			return ConvertInt16ToInt32b24;
		case ASIOSTFloat32LSB:
			return ConvertInt16ToFloat32;
		case ASIOSTInt16MSB:
			return ConvertInt16ToInt16_S;
		case ASIOSTInt24MSB:
			return ConvertInt16ToInt24_S;
		case ASIOSTInt32MSB:
			return ConvertInt16ToInt32_S;
		case ASIOSTInt32MSB16:
			return ConvertInt16ToInt32b16_S;
		case ASIOSTInt32MSB24:
			return ConvertInt16ToInt32b24_S;
		case ASIOSTFloat32MSB:
			return ConvertInt16ToFloat32_S;
		default:
			OutputDebugString(_T("*** Unsupported conversion of 16bit data to ASIO\n"));
			_ASSERT(FALSE);
		}
		break;
	case 24:
		switch(Type)
		{
		case ASIOSTInt16LSB:
			return ConvertInt24ToInt16;
		case ASIOSTInt24LSB:
			return ConvertInt24ToInt24;
		case ASIOSTInt32LSB:
			return ConvertInt24ToInt32;
		case ASIOSTInt32LSB16:
			return ConvertInt24ToInt32b16;
		case ASIOSTInt32LSB24:
			return ConvertInt24ToInt32b24;
		case ASIOSTFloat32LSB:
			return ConvertInt24ToFloat32;
		case ASIOSTInt16MSB:
			return ConvertInt24ToInt16_S;
		case ASIOSTInt24MSB:
			return ConvertInt24ToInt24_S;
		case ASIOSTInt32MSB:
			return ConvertInt24ToInt32_S;
		case ASIOSTInt32MSB16:
			return ConvertInt24ToInt32b16_S;
		case ASIOSTInt32MSB24:
			return ConvertInt24ToInt32b24_S;
		case ASIOSTFloat32MSB:
			return ConvertInt24ToFloat32_S;
		default:
			OutputDebugString(_T("*** Unsupported conversion of 24bit data to ASIO\n"));
			_ASSERT(FALSE);
		}
		break;
	case 32:
		switch(WaveFormat)
		{
		case WAVE_FORMAT_PCM:
			switch (Type)
			{
			case ASIOSTInt16LSB:
				return ConvertInt32ToInt16;
			case ASIOSTInt24LSB:
				return ConvertInt32ToInt24;
			case ASIOSTInt32LSB:
				return ConvertInt32ToInt32;
			case ASIOSTInt32LSB16:
				return ConvertInt32ToInt32b16;
			case ASIOSTInt32LSB24:
				return ConvertInt32ToInt32b24;
			case ASIOSTFloat32LSB:
				return ConvertInt32ToFloat32;
			case ASIOSTInt16MSB:
				return ConvertInt32ToInt16_S;
			case ASIOSTInt24MSB:
				return ConvertInt32ToInt24_S;
			case ASIOSTInt32MSB:
				return ConvertInt32ToInt32_S;
			case ASIOSTInt32MSB16:
				return ConvertInt32ToInt32b16_S;
			case ASIOSTInt32MSB24:
				return ConvertInt32ToInt32b24_S;
			case ASIOSTFloat32MSB:
				return ConvertInt32ToFloat32_S;
			default:
				OutputDebugString(_T("*** Unsupported conversion of 32bit data to ASIO\n"));
				_ASSERT(FALSE);
			}
			break;
		case WAVE_FORMAT_IEEE_FLOAT:
			switch (Type)
			{
			case ASIOSTInt16LSB:
				return ConvertFloat32ToInt16;
			case ASIOSTInt24LSB:
				return ConvertFloat32ToInt24;
			case ASIOSTInt32LSB:
				return ConvertFloat32ToInt32;
			case ASIOSTInt32LSB16:
				return ConvertFloat32ToInt32b16;
			case ASIOSTInt32LSB24:
				return ConvertFloat32ToInt32b24;
			case ASIOSTFloat32LSB:
				return ConvertFloat32ToFloat32;
			case ASIOSTInt16MSB:
				return ConvertFloat32ToInt16_S;
			case ASIOSTInt24MSB:
				return ConvertFloat32ToInt24_S;
			case ASIOSTInt32MSB:
				return ConvertFloat32ToInt32_S;
			case ASIOSTInt32MSB16:
				return ConvertFloat32ToInt32b16_S;
			case ASIOSTInt32MSB24:
				return ConvertFloat32ToInt32b24_S;
			case ASIOSTFloat32MSB:
				return ConvertFloat32ToFloat32_S;
				default:
				OutputDebugString(_T("*** Unsupported conversion of IEEE float data to ASIO\n"));
				_ASSERT(FALSE);
			}
			break;
		}
	default:
		OutputDebugString(_T("*** Unsupported ASIO sample type conversion\n"));
		_ASSERT(FALSE);
	}

	return NULL;
}

//////////////////////////
// LSB block converters //
//////////////////////////

//
// 8-bit
//

void __fastcall ConvertInt8ToInt16(short *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = (*src++ - 0x80) << 8;
	}
}

void __fastcall	ConvertInt8ToInt24(BYTE *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*(short *)dest = 0;
		*(dest + 2) = *src++ - 0x80;
		dest += 3;
	}
}

void __fastcall ConvertInt8ToInt32(long *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = (*src++ - 0x80) << 24;
	}
}

void __fastcall ConvertInt8ToInt32b16(long *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = (*src++ - 0x80) << 8;
	}
}

void __fastcall ConvertInt8ToInt32b24(long *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = (*src++ - 0x80) << 16;
	}
}

void __fastcall ConvertInt8ToFloat32(float *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToFloat32((float)((*src++ - 0x80) / FSCALER8), dest++);
	}
}

//
// 16-bit
//
void __fastcall ConvertInt16ToInt16(short *dest, short *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = *src++;
	}
}

void __fastcall	ConvertInt16ToInt24(BYTE *dest, short *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = 0;
		*(short *)(dest) = *src++;
		dest += 2;
	}
}

void __fastcall ConvertInt16ToInt32(long *dest, short *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = *src++ << 16;
	}
}

void __fastcall ConvertInt16ToInt32b16(long *dest, short *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = *src++;
	}
}

void __fastcall ConvertInt16ToInt32b24(long *dest, short *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = *src++ << 8;
	}
}

void __fastcall ConvertInt16ToFloat32(float *dest, short *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToFloat32((float)((*src++) / FSCALER16), dest++);
	}
}

//
// 24-bit
//
void __fastcall ConvertInt24ToInt16(short *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt16((double)(Cvt24To32(src)) / (1 << 8), dest++);
		src += 3;
	}
}

void __fastcall	ConvertInt24ToInt24(BYTE *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		Copy3(dest, src);
		dest += 3;
		src += 3;
	}
}

void __fastcall ConvertInt24ToInt32(long *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = Cvt24To32(src) << 8;
		src += 3;
	}
}

void __fastcall ConvertInt24ToInt32b16(long *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt32b16((double)(Cvt24To32(src)) / (1 << 8), dest++);
		src += 3;
	}
}

void __fastcall ConvertInt24ToInt32b24(long *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = Cvt24To32(src);
		src += 3;
	}
}

void __fastcall ConvertInt24ToFloat32(float *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToFloat32((float)(Cvt24To32(src) / FSCALER24), dest++);
		src += 3;
	}
}

//
// 32-bit
//
void __fastcall ConvertInt32ToInt16(short *dest, long *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt16((double)(*src++) / (1 << 16), dest++);
	}
}

void __fastcall ConvertInt32ToInt24(BYTE *dest, long *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt24((double)(*src++) / (1 << 8), dest);
		dest += 3;
	}
}

void __fastcall ConvertInt32ToInt32 (long *dest, long *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = *src++;
	}
}

void __fastcall ConvertInt32ToInt32b16(long *dest, long *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt32b16((double)(*src++) / (1 << 16), dest++);
	}
}

void __fastcall ConvertInt32ToInt32b24(long *dest, long *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt32b24((double)(*src++) / (1 << 8), dest++);
	}
}

void __fastcall ConvertInt32ToFloat32(float *dest, long *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToFloat32((float)((*src++) / FSCALER32), dest++);
	}
}

//
// Float32
//
void __fastcall ConvertFloat32ToInt16(short *dest, float *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt16(*src++ * FSCALER16, dest++);
	}
}

void __fastcall ConvertFloat32ToInt24(BYTE *dest, float *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt24(*src++ * FSCALER24, dest);
		dest += 3;
	}
}

void __fastcall ConvertFloat32ToInt32(long *dest, float *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = (long)(*src++ * FSCALER32);
	}
}

void __fastcall ConvertFloat32ToInt32b16(long *dest, float *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt32b16(*src++ * FSCALER16, dest++);
	}
}

void __fastcall ConvertFloat32ToInt32b24(long *dest, float *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt32b24(*src++ * FSCALER24, dest++);
	}
}

void __fastcall ConvertFloat32ToFloat32(float *dest, float *src, int count)
{
	while (--count >= 0)
	{
		// Normalize
		ConvertFloatToFloat32(*src++, dest++);
	}
}

//////////////////////////
// MSB block converters //
//////////////////////////

//
// 8-bit
//

void __fastcall ConvertInt8ToInt16_S(short *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = SwapShort((*src++ - 0x80) << 8);
	}
}

void __fastcall	ConvertInt8ToInt24_S(BYTE *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*dest = *src++ - 0x80;
		*(short *)(dest + 1) = 0;
		dest += 3;
	}
}

void __fastcall ConvertInt8ToInt32_S(long *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = SwapLong((*src++ - 0x80) << 24);
	}
}

void __fastcall ConvertInt8ToInt32b16_S(long *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = SwapLong((*src++ - 0x80) << 8);
	}
}

void __fastcall ConvertInt8ToInt32b24_S(long *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = SwapLong((*src++ - 0x80) << 16);
	}
}

void __fastcall ConvertInt8ToFloat32_S(float *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToFloat32_S((float)((*src++ - 0x80) / FSCALER8), dest++);
	}
}

//
// 16-bit
//
void __fastcall ConvertInt16ToInt16_S(short *dest, short *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = SwapShort(*src++);
	}
}

void __fastcall	ConvertInt16ToInt24_S(BYTE *dest, short *src, int count)
{
	while (--count >= 0)
	{
		*(short *)(dest) = SwapShort(*src++);
		*(dest + 2) = 0;
		dest += 3;
	}
}

void __fastcall ConvertInt16ToInt32_S(long *dest, short *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = SwapLong(*src++ << 16);
	}
}

void __fastcall ConvertInt16ToInt32b16_S(long *dest, short *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = SwapLong(*src++);
	}
}

void __fastcall ConvertInt16ToInt32b24_S(long *dest, short *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = SwapLong(*src++ << 8);
	}
}

void __fastcall ConvertInt16ToFloat32_S(float *dest, short *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToFloat32_S((float)((*src++) / FSCALER16), dest++);
	}
}

//
// 24-bit
//
void __fastcall ConvertInt24ToInt16_S(short *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt16_S((double)(Cvt24To32(src)) / (1 << 8), dest++);
		src += 3;
	}
}

void __fastcall	ConvertInt24ToInt24_S(BYTE *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*(dest + 0) = *(src + 2);
		*(dest + 1) = *(src + 1);
		*(dest + 2) = *(src + 0);
		dest += 3;
		src += 3;
	}
}

void __fastcall ConvertInt24ToInt32_S(long *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = SwapLong(Cvt24To32(src) << 8);
		src += 3;
	}
}

void __fastcall ConvertInt24ToInt32b16_S(long *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt32b16_S((double)(Cvt24To32(src)) / (1 << 8), dest++);
		src += 3;
	}
}

void __fastcall ConvertInt24ToInt32b24_S(long *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = SwapLong(Cvt24To32(src));
		src += 3;
	}
}

void __fastcall ConvertInt24ToFloat32_S(float *dest, BYTE *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToFloat32_S((float)(Cvt24To32(src) / FSCALER24), dest++);
		src += 3;
	}
}

//
// 32-bit
//
void __fastcall ConvertInt32ToInt16_S(short *dest, long *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt16_S((double)(*src++) / (1 << 16), dest++);
	}
}

void __fastcall ConvertInt32ToInt24_S(BYTE *dest, long *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt24_S((double)(*src++) / (1 << 8), dest);
		dest += 3;
	}
}

void __fastcall ConvertInt32ToInt32_S(long *dest, long *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = SwapLong(*src++);
	}
}

void __fastcall ConvertInt32ToInt32b16_S(long *dest, long *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt32b16_S((double)(*src++) / (1 << 16), dest++);
	}
}

void __fastcall ConvertInt32ToInt32b24_S(long *dest, long *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt32b24_S((double)(*src++) / (1 << 8), dest++);
	}
}

void __fastcall ConvertInt32ToFloat32_S(float *dest, long *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToFloat32_S((float)((*src++) / FSCALER32), dest++);
	}
}

//
// Float32
//
void __fastcall ConvertFloat32ToInt16_S(short *dest, float *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt16_S(*src++ * FSCALER16, dest++);
	}
}

void __fastcall ConvertFloat32ToInt24_S(BYTE *dest, float *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt24_S(*src++ * FSCALER24, dest);
		dest += 3;
	}
}

void __fastcall ConvertFloat32ToInt32_S(long *dest, float *src, int count)
{
	while (--count >= 0)
	{
		*dest++ = SwapLong((long)(*src++ * FSCALER32));
	}
}

void __fastcall ConvertFloat32ToInt32b16_S(long *dest, float *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt32b16_S(*src++ * FSCALER16, dest++);
	}
}

void __fastcall ConvertFloat32ToInt32b24_S(long *dest, float *src, int count)
{
	while (--count >= 0)
	{
		ConvertFloatToInt32b24_S(*src++ * FSCALER24, dest++);
	}
}

void __fastcall ConvertFloat32ToFloat32_S(float *dest, float *src, int count)
{
	while (--count >= 0)
	{
		// Normalize
		ConvertFloatToFloat32_S(*src++, dest++);
	}
}


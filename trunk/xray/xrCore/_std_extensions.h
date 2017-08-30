#pragma once

#if defined(DEBUG) && !defined(_EDITOR)
#	define	BREAK_AT_STRCMP
#endif

#ifdef abs
#undef abs
#endif

#ifdef  _EDITOR
IC void strcpy_s(char* strDestination,   size_t sizeInBytes,   const char *strSource)
{
	strcpy(strDestination, strSource);
}

IC void strcpy_s(char* strDestination,   const char *strSource)
{
	strcpy(strDestination, strSource);
}

IC void _strlwr_s(char* strDestination, size_t sizeInBytes)
{
    strlwr(strDestination);
}

IC void strcat_s(char* strDestination,   size_t sizeInBytes,   const char *strSource)
{
	strcat(strDestination, strSource);
}

IC int sprintf_s(char* dest, size_t sizeOfBuffer, const char* format, ...)
{
	va_list 	mark;
	va_start	(mark, format );
	int sz		= _vsnprintf(dest, sizeOfBuffer, format, mark );
    dest		[sizeOfBuffer-1]=0;
    va_end		(mark);
}
#endif

// token type definition
struct XRCORE_API xr_token
{
	LPCSTR	name;
	int 	id;
};

IC LPCSTR get_token_name(xr_token* tokens, int key)
{
    for (int k=0; tokens[k].name; k++)
    	if (key==tokens[k].id) return tokens[k].name;
    return "";
}

IC int get_token_id(xr_token* tokens, LPCSTR key)
{
    for (int k=0; tokens[k].name; k++)
    	if ( stricmp(tokens[k].name,key)==0 ) 
			return tokens[k].id;
    return -1;
}

struct XRCORE_API xr_token2
{
	LPCSTR	name;
	LPCSTR	info;
	int 	id;
};

// generic
template <class T>	IC T		_sqr	(T a)		{ return a*a;		}

// float
IC float	_abs	(float x)		{ return fabsf(x); }
IC float	_sqrt	(float x)		{ return sqrtf(x); }
IC float	_sin	(float x)		{ return sinf(x); }
IC float	_cos	(float x)		{ return cosf(x); }
IC float    _log	(float x)		{ return logf(x); }
IC BOOL		_valid	(const float x)
{
	// check for: Signaling NaN, Quiet NaN, Negative infinity ( –INF), Positive infinity (+INF), Negative denormalized, Positive denormalized
	int			cls			= _fpclass		(double(x));
	if (cls&(_FPCLASS_SNAN+_FPCLASS_QNAN+_FPCLASS_NINF+_FPCLASS_PINF+_FPCLASS_ND+_FPCLASS_PD))	
		return	false;	

	/*	*****other cases are*****
	_FPCLASS_NN Negative normalized non-zero 
	_FPCLASS_NZ Negative zero ( – 0) 
	_FPCLASS_PZ Positive 0 (+0) 
	_FPCLASS_PN Positive normalized non-zero 
	*/
	return		true;
}


// double
IC double	_abs	(double x)		{ return fabs(x); }
IC double	_sqrt	(double x)		{ return sqrt(x); }
IC double	_sin	(double x)		{ return sin(x); }
IC double	_cos	(double x)		{ return cos(x); }
IC double    _log	(double x)		{ return log(x); }
IC BOOL		_valid	(const double x)
{
	// check for: Signaling NaN, Quiet NaN, Negative infinity ( –INF), Positive infinity (+INF), Negative denormalized, Positive denormalized
	int			cls			= _fpclass		(x);
	if (cls&(_FPCLASS_SNAN+_FPCLASS_QNAN+_FPCLASS_NINF+_FPCLASS_PINF+_FPCLASS_ND+_FPCLASS_PD))	
		return false;	

	/*	*****other cases are*****
	_FPCLASS_NN Negative normalized non-zero 
	_FPCLASS_NZ Negative zero ( – 0) 
	_FPCLASS_PZ Positive 0 (+0) 
	_FPCLASS_PN Positive normalized non-zero 
	*/
	return		true;
}

// int8
IC s8		_abs	(s8  x)			{ return (x>=0)? x : s8(-x); }
// unsigned int8
IC u8		_abs	(u8 x)			{ return x; }
// int16
IC s16		_abs	(s16 x)			{ return (x>=0)? x : s16(-x); }
// unsigned int16
IC u16		_abs	(u16 x)			{ return x; }
// int32
IC s32		_abs	(s32 x)			{ return (x>=0)? x : s32(-x); }
// int64
IC s64		_abs	(s64 x)			{ return (x>=0)? x : s64(-x); }

IC u32 xr_strlen( const char* S );

#ifndef  _EDITOR
// string management
IC LPCSTR						strconcat				( int dest_sz, char* dest, const char* S1, const char* S2)
{
	u32 l1 = xr_strlen(S1);
	strcpy_s(dest,dest_sz,S1);
	strcat_s(dest,dest_sz-l1,S2);
	return dest;
//.	return strcat(strcpy(dest,S1),S2);
}

// dest = S1+S2+S3
IC LPCSTR						strconcat				( int dest_sz, char* dest, const char* S1, const char* S2, const char* S3)
{
	u32 l1 = xr_strlen(S1);
	u32 l2 = xr_strlen(S2);
	strcpy_s(dest,dest_sz,S1);
	strcat_s(dest,dest_sz-l1,S2);
	strcat_s(dest,dest_sz-l1-l2,S3);

	return dest;
//.	return strcat(strcat(strcpy(dest,S1),S2),S3);
}

// dest = S1+S2+S3+S4
IC LPCSTR						strconcat				( int dest_sz, char* dest, const char* S1, const char* S2, const char* S3, const char* S4)
{
	u32 l1 = xr_strlen(S1);
	u32 l2 = xr_strlen(S2);
	u32 l3 = xr_strlen(S3);
	strcpy_s(dest,dest_sz,S1);
	strcat_s(dest,dest_sz-l1,S2);
	strcat_s(dest,dest_sz-l1-l2,S3);
	strcat_s(dest,dest_sz-l1-l2-l3,S4);

	return dest;
//.	return strcat(strcat(strcat(strcpy(dest,S1),S2),S3),S4);
}

// dest = S1+S2+S3+S4+S5
IC LPCSTR						strconcat				( int dest_sz, char* dest, const char* S1, const char* S2, const char* S3, const char* S4, const char* S5)
{
	u32 l1 = xr_strlen(S1);
	u32 l2 = xr_strlen(S2);
	u32 l3 = xr_strlen(S3);
	u32 l4 = xr_strlen(S4);
	strcpy_s(dest,dest_sz,S1);
	strcat_s(dest,dest_sz-l1,S2);
	strcat_s(dest,dest_sz-l1-l2,S3);
	strcat_s(dest,dest_sz-l1-l2-l3,S4);
	strcat_s(dest,dest_sz-l1-l2-l3-l4,S5);

	return dest;
//.	return strcat(strcat(strcat(strcat(strcpy(dest,S1),S2),S3),S4),S5);
}

// dest = S1+S2+S3+S4+S5+S6
IC LPCSTR						strconcat				( int dest_sz, char* dest, const char* S1, const char* S2, const char* S3, const char* S4, const char* S5, const char* S6)
{
	u32 l1 = xr_strlen(S1);
	u32 l2 = xr_strlen(S2);
	u32 l3 = xr_strlen(S3);
	u32 l4 = xr_strlen(S4);
	u32 l5 = xr_strlen(S5);
	strcpy_s(dest,dest_sz,S1);
	strcat_s(dest,dest_sz-l1,S2);
	strcat_s(dest,dest_sz-l1-l2,S3);
	strcat_s(dest,dest_sz-l1-l2-l3,S4);
	strcat_s(dest,dest_sz-l1-l2-l3-l4,S5);
	strcat_s(dest,dest_sz-l1-l2-l3-l4-l5,S6);

	return dest;
	//.	return strcat(strcat(strcat(strcat(strcat(strcpy(dest,S1),S2),S3),S4),S5),S6);
}

#else

IC char*						strconcat				( int dest_sz,  char* dest, const char* S1, const char* S2)
{	return strcat(strcpy(dest,S1),S2); }

// dest = S1+S2+S3
IC char*						strconcat				( int dest_sz,  char* dest, const char* S1, const char* S2, const char* S3)
{	return strcat(strcat(strcpy(dest,S1),S2),S3); }

// dest = S1+S2+S3+S4
IC char*						strconcat				( int dest_sz,  char* dest, const char* S1, const char* S2, const char* S3, const char* S4)
{	return strcat(strcat(strcat(strcpy(dest,S1),S2),S3),S4); }

// dest = S1+S2+S3+S4+S5
IC char*						strconcat				( int dest_sz,  char* dest, const char* S1, const char* S2, const char* S3, const char* S4, const char* S5)
{	return strcat(strcat(strcat(strcat(strcpy(dest,S1),S2),S3),S4),S5); }

// dest = S1+S2+S3+S4+S5+S6
IC char*						strconcat				( int dest_sz,  char* dest, const char* S1, const char* S2, const char* S3, const char* S4, const char* S5, const char* S6)
{	return strcat(strcat(strcat(strcat(strcat(strcpy(dest,S1),S2),S3),S4),S5),S6); }

#endif
// return pointer to ".ext"
IC char*						strext					( const char* S )
{	return (char*) strrchr(S,'.');	}

IC u32							xr_strlen				( const char* S )
{	return (u32)strlen(S);			}

IC char*						xr_strlwr				(char* S)
{	return strlwr(S);				}

#ifdef BREAK_AT_STRCMP
XRCORE_API	int					xr_strcmp				( const char* S1, const char* S2 );
#else
IC int							xr_strcmp				( const char* S1, const char* S2 )
{	return strcmp(S1,S2);  }
#endif

XRCORE_API	char*				timestamp				(string64& dest);

extern XRCORE_API u32			crc32					(const void* P, u32 len);

//KRodin: Âñ¸ ÷òî íèæå - âçÿòî èç ÇÏ.
IC int xr_strcpy(LPSTR destination, size_t const destination_size, LPCSTR source)
{
	return strncpy_s(destination, destination_size, source, destination_size);
}

template <int count>
IC int xr_strcpy(char(&destination)[count], LPCSTR source)
{
	return xr_strcpy(destination, count, source);
}

IC int xr_strcat(LPSTR destination, size_t const buffer_size, LPCSTR source)
{
	size_t const destination_length = xr_strlen(destination);
	LPSTR i = destination + destination_length;
	LPSTR const e = destination + buffer_size - 1;
	if (i > e)
		return 0;

	for (LPCSTR j = source; *j && (i != e); ++i, ++j)
		*i = *j;

	*i = 0;
	return 0;
}

template <int count>
IC int xr_strcat(char(&destination)[count], LPCSTR source)
{
	return xr_strcat(destination, count, source);
}

IC int __cdecl xr_sprintf(LPSTR destination, size_t const buffer_size, LPCSTR format_string, ...)
{
	va_list args;
	va_start(args, format_string);
	return vsnprintf_s(destination, buffer_size, buffer_size - 1, format_string, args);
}

template <int count>
IC int __cdecl xr_sprintf(char(&destination)[count], LPCSTR format_string, ...)
{
	va_list args;
	va_start(args, format_string);
	return vsnprintf_s(destination, count, count - 1, format_string, args);
}

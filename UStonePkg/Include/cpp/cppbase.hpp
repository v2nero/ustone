
#pragma once

#if __cplusplus <= 199711L
#define nullptr NULL
#endif

typedef void(__cdecl *PF)(void);
int atexit_cpp(PF pf);
void initial_objs_cpp();
void destroy_objs_cpp();

#pragma warning(disable : 4075)
//init_seg
// Linker puts constructors between these sections, and we use them to locate constructor pointers.
#pragma section(".mine$a",read)
#pragma section(".mine$z",read)
// by default, goes into a read only section
#pragma init_seg(".mine$m", atexit_cpp)

#include <cpp/uefi.hpp>

#pragma push_macro("new")
  #undef new

#if !defined(MDEPKG_NDEBUG)

void * operator new(size_t size, const char *pFileName, int nLine);
void * operator new[](size_t size, const char *pFileName, int nLine);
void operator delete(void *);
void operator delete[](void *);

#undef __PLACEMENT_NEW_INLINE
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline void * operator new(size_t, void *_Where, const char *pFileName, int nLine)
{	// construct array with placement at _Where
	return (_Where);
}

inline void * operator new[](size_t, void *_Where, const char *pFileName, int nLine)
{	// construct array with placement at _Where
	return (_Where);
}

inline void operator delete(void *, void *)
{	// delete if placement new fails
}
inline void operator delete[](void *, void *)
{	// delete if placement new fails
}
#endif

#else

void * operator new(size_t);
void operator delete(void *);

#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline void * operator new(size_t, void *_Where)
{	// construct array with placement at _Where
	return (_Where);
}

inline void * operator new[](size_t, void *_Where)
{	// construct array with placement at _Where
	return (_Where);
}

inline void operator delete(void *, void *)
{	// delete if placement new fails
}
inline void operator delete[](void *, void *)
{	// delete if placement new fails
}
#endif

#endif

#pragma pop_macro("new")

#ifdef new
#undef new
#endif
#ifdef delete
#undef delete
#endif

#if (!defined(MDEPKG_NDEBUG)) && !defined(__CPP_BASE_H__)
//#define PLACEMENT_NEW(p) new(p, __FILE__, __LINE__)
#define NEW new(__FILE__, __LINE__)
#define PLACEMENT_NEW(p) new(p, __FILE__, __LINE__)
#define DELETE delete
#else
#define NEW new
#define PLACEMENT_NEW(p) new(p)
#define DELETE delete
#endif

//void FreeLeftMemPools(bool bAlert = true);


extern "C" int _purecall(void);

template <class T>
inline T math_max(T a, T b)
{
	return a > b ? a : b;
}

template <typename T>
inline T math_min(T a, T b)
{
	return a > b ? b : a;
}

//inline int math_max(int a, int b);

//inline int math_min(int, int);

/*
 * Copyright (c) 2016 by Nero Liu .  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 *
 */

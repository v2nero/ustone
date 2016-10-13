#include <Uefi.h>
#include <Library/UefiLib.h>

typedef void (__cdecl* _PVFV)(void);
typedef int  (__cdecl* _PIFV)(void);

#define	_CRTALLOC(x) __declspec(allocate(x))

// Linker puts constructors between these sections, and we use them to locate constructor pointers.
#pragma section(".CRT$XIA",long,read)
#pragma section(".CRT$XIZ",long,read)
#pragma section(".CRT$XCA",long,read)
#pragma section(".CRT$XCZ",long,read)
#pragma section(".CRT$XPA",long,read)
#pragma section(".CRT$XPZ",long,read)
#pragma section(".CRT$XTA",long,read)
#pragma section(".CRT$XTZ",long,read)


// Pointers surrounding constructors
extern _CRTALLOC(".CRT$XIA") _PIFV __xi_a[] = { NULL }; // C initializers (first)
extern _CRTALLOC(".CRT$XIZ") _PIFV __xi_z[] = { NULL }; // C initializers (last)
extern _CRTALLOC(".CRT$XCA") _PVFV __xc_a[] = { NULL }; // C++ initializers (first)
extern _CRTALLOC(".CRT$XCZ") _PVFV __xc_z[] = { NULL }; // C++ initializers (last)
extern _CRTALLOC(".CRT$XPA") _PVFV __xp_a[] = { NULL }; // C pre-terminators (first)
extern _CRTALLOC(".CRT$XPZ") _PVFV __xp_z[] = { NULL }; // C pre-terminators (last)
extern _CRTALLOC(".CRT$XTA") _PVFV __xt_a[] = { NULL }; // C terminators (first)
extern _CRTALLOC(".CRT$XTZ") _PVFV __xt_z[] = { NULL }; // C terminators (last)

// Put .CRT data into .rdata section
#pragma comment(linker, "/merge:.CRT=.rdata")

static int _initterm_e (
        _PIFV * pfbegin,
        _PIFV * pfend
        )
{
        int ret = 0;
        /*
         * walk the table of function pointers from the bottom up, until
         * the end is encountered.  Do not skip the first entry.  The initial
         * value of pfbegin points to the first valid entry.  Do not try to
         * execute what pfend points to.  Only entries before pfend are valid.
         */
        while ( pfbegin < pfend && ret == 0)
        {
            /*
             * if current table entry is non-NULL, call thru it.
             */
            if ( *pfbegin != NULL )
                ret = (**pfbegin)();
            ++pfbegin;
        }

        return ret;
}

static void _initterm (
        _PVFV * pfbegin,
        _PVFV * pfend
        )
{
        /*
         * walk the table of function pointers from the bottom up, until
         * the end is encountered.  Do not skip the first entry.  The initial
         * value of pfbegin points to the first valid entry.  Do not try to
         * execute what pfend points to.  Only entries before pfend are valid.
         */
        while ( pfbegin < pfend )
        {
            /*
             * if current table entry is non-NULL, call thru it.
             */
            if ( *pfbegin != NULL )
                (**pfbegin)();
            ++pfbegin;
        }
}

static int g_crt_init_done = 0;

int global_crt_init()
{
	if (g_crt_init_done) {
		return 0;
	}
	g_crt_init_done = 1;

	if (_initterm_e(__xi_a, __xi_z) != 0) {
		Print(L"Fail to initialize CRT XIA-XIZ section\n");
		return -1;
	}

	_initterm(__xc_a, __xc_z);
	return 0;
}

static int g_crt_uninit_done = 0;

void global_crt_uninit()
{
	if (g_crt_uninit_done) {
		return;
	}
	g_crt_uninit_done = 1;

	_initterm(__xp_a, __xp_z);
	_initterm(__xt_a, __xt_z);
}

/*
 * Copyright (c) 2016 by Nero Liu .  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 *
 */

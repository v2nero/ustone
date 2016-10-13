/*
 * Copyright (c) 1996-1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef __SGI_STL_INTERNAL_ALLOC_H
#define __SGI_STL_INTERNAL_ALLOC_H
#include <cpp/cppbase.hpp>
#include "stl_config.h"

// This implements some standard node allocators.  These are
// NOT the same as the allocators in the C++ draft standard or in
// in the original STL.  They do not encapsulate different pointer
// types; indeed we assume that there is only one pointer type.
// The allocation primitives are intended to allocate individual objects,
// not larger arenas as with the original STL allocators.


//#define __THROW_BAD_ALLOC cerr << "out of memory" << endl; exit(1)
#define __THROW_BAD_ALLOC { fprintf(stderr, "%s:%d out of memory\n", \
			  __FILE__, __LINE__); abort(); }

#ifndef __ALLOC
#   define __ALLOC alloc
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

#ifndef __RESTRICT
#  define __RESTRICT
#endif

#if !defined(_NOTHREADS)
#   define _NOTHREADS
#endif

# ifdef _NOTHREADS
//  Thread-unsafe
#   define __NODE_ALLOCATOR_LOCK
#   define __NODE_ALLOCATOR_UNLOCK
#   define __NODE_ALLOCATOR_THREADS false
#   define __VOLATILE
# endif

__STL_BEGIN_NAMESPACE

//================__malloc_alloc_template=====================
template <int inst>
class __malloc_alloc_template {

private:

	static void *oom_malloc(size_t);

	static void *oom_realloc(void *, size_t);

	static void (* __malloc_alloc_oom_handler)();

public:

	static void * allocate(size_t n)
	{
	    void *result = malloc(n);
	    if (0 == result) result = oom_malloc(n);
	    return result;
	}

	static void deallocate(void *p, size_t /* n */)
	{
	    free(p);
	}

	static void * reallocate(void *p, size_t /* old_sz */, size_t new_sz)
	{
	    void * result = realloc(p, new_sz);
	    if (0 == result) result = oom_realloc(p, new_sz);
	    return result;
	}

	static void (* set_malloc_handler(void (*f)()))()
	{
	    void (* old)() = __malloc_alloc_oom_handler;
	    __malloc_alloc_oom_handler = f;
	    return(old);
	}

};

// malloc_alloc out-of-memory handling
template <int inst>
void (* __malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;

template <int inst>
void * __malloc_alloc_template<inst>::oom_malloc(size_t n)
{
    void (* my_malloc_handler)();
    void *result;

    for (;;) {
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*my_malloc_handler)();
        result = malloc(n);
        if (result) return(result);
    }
}

template <int inst>
void * __malloc_alloc_template<inst>::oom_realloc(void *p, size_t n)
{
    void (* my_malloc_handler)();
    void *result;

    for (;;) {
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*my_malloc_handler)();
        result = realloc(p, n);
        if (result) return(result);
    }
}

//-------------------------__malloc_alloc_template instance-----------------------------
typedef __malloc_alloc_template<0> malloc_alloc;

template<class T, class Alloc>
class simple_alloc {

public:
    static T *allocate(size_t n)
                { return 0 == n? 0 : (T*) Alloc::allocate(n * sizeof (T)); }
    static T *allocate(void)
                { return (T*) Alloc::allocate(sizeof (T)); }
    static void deallocate(T *p, size_t n)
                { if (0 != n) Alloc::deallocate(p, n * sizeof (T)); }
    static void deallocate(T *p)
                { Alloc::deallocate(p, sizeof (T)); }
};

// Allocator adaptor to check size arguments for debugging.
// Reports errors using assert.  Checking can be disabled with
// NDEBUG, but it's far better to just use the underlying allocator
// instead when no checking is desired.
// There is some evidence that this can confuse Purify.
template <class Alloc>
class debug_alloc {

private:

	enum {extra = 8};       // Size of space used to store size.  Note
				// that this must be large enough to preserve
				// alignment.

public:

	static void * allocate(size_t n)
	{
	    char *result = (char *)Alloc::allocate(n + extra);
	    *(size_t *)result = n;
	    return result + extra;
	}

	static void deallocate(void *p, size_t n)
	{
	    char * real_p = (char *)p - extra;
	    assert(*(size_t *)real_p == n);
	    Alloc::deallocate(real_p, n + extra);
	}

	static void * reallocate(void *p, size_t old_sz, size_t new_sz)
	{
	    char * real_p = (char *)p - extra;
	    assert(*(size_t *)real_p == old_sz);
	    char * result = (char *)
			  Alloc::reallocate(real_p, old_sz + extra, new_sz + extra);
	    *(size_t *)result = new_sz;
	    return result + extra;
	}
};


# ifdef __USE_MALLOC

typedef malloc_alloc alloc;
typedef malloc_alloc single_client_alloc;

# else

//================__default_alloc_template=====================

// Default node allocator.
// With a reasonable compiler, this should be roughly as fast as the
// original STL class-specific allocators, but with less fragmentation.
// Default_alloc_template parameters are experimental and MAY
// DISAPPEAR in the future.  Clients should just use alloc for now.
//
// Important implementation properties:
// 1. If the client request an object of size > __MAX_BYTES, the resulting
//    object will be obtained directly from malloc.
// 2. In all other cases, we allocate an object of size exactly
//    ROUND_UP(requested_size).  Thus the client has enough size
//    information that we can return the object to the proper free list
//    without permanently losing part of the object.
//

// The first template parameter specifies whether more than one thread
// may use this allocator.  It is safe to allocate an object from
// one instance of a default_alloc and deallocate it with another
// one.  This effectively transfers its ownership to the second one.
// This may have undesirable effects on reference locality.
// The second parameter is unreferenced and serves only to allow the
// creation of multiple default_alloc instances.
// Node that containers built on different allocator instances have
// different types, limiting the utility of this approach.
template <bool threads, int inst>
class __default_alloc_template {

private:
	// Really we should use static const int x = N
	// instead of enum { x = N }, but few compilers accept the former.
	enum {__ALIGN = 8};
	enum {__MAX_BYTES = 128};
	enum {__NFREELISTS = __MAX_BYTES/__ALIGN};

	static size_t ROUND_UP(size_t bytes) {
		return (((bytes) + __ALIGN-1) & ~(__ALIGN - 1));
	}
private:
	union obj {
		union obj * free_list_link;
		char client_data[1];    /* The client sees this.        */
	};

private:
	static obj * __VOLATILE free_list[__NFREELISTS];

	static  size_t FREELIST_INDEX(size_t bytes) {
		return (((bytes) + __ALIGN-1)/__ALIGN - 1);
	}

	// Returns an object of size n, and optionally adds to size n free list.
	static void *refill(size_t n);
	// Allocates a chunk for nobjs of size "size".  nobjs may be reduced
	// if it is inconvenient to allocate the requested number.
	static char *chunk_alloc(size_t size, int &nobjs);

	// Chunk allocation state.
	static char *start_free;
	static char *end_free;
	static size_t heap_size;

public:

	/* n must be > 0      */
	static void * allocate(size_t n)
	{
		obj * __VOLATILE * my_free_list;
		obj * __RESTRICT result;

		if (n > (size_t) __MAX_BYTES) {
			return(malloc_alloc::allocate(n));
		}
		my_free_list = free_list + FREELIST_INDEX(n);
		result = *my_free_list;
		if (result == 0) {
			void *r = refill(ROUND_UP(n));
			return r;
		}
		*my_free_list = result -> free_list_link;
		return (result);
	};

	/* p may not be 0 */
	static void deallocate(void *p, size_t n)
	{
		obj *q = (obj *)p;
		obj * __VOLATILE * my_free_list;

		if (n > (size_t) __MAX_BYTES) {
			malloc_alloc::deallocate(p, n);
			return;
		}
		my_free_list = free_list + FREELIST_INDEX(n);
		q -> free_list_link = *my_free_list;
		*my_free_list = q;
	}

	static void * reallocate(void *p, size_t old_sz, size_t new_sz);

} ;

typedef __default_alloc_template<__NODE_ALLOCATOR_THREADS, 0> alloc;
typedef __default_alloc_template<false, 0> single_client_alloc;



/* We allocate memory in large chunks in order to avoid fragmenting     */
/* the malloc heap too much.                                            */
/* We assume that size is properly aligned.                             */
/* We hold the allocation lock.                                         */
template <bool threads, int inst>
char*
__default_alloc_template<threads, inst>::chunk_alloc(size_t size, int& nobjs)
{
    char * result;
    size_t total_bytes = size * nobjs;
    size_t bytes_left = end_free - start_free;

    if (bytes_left >= total_bytes) {
        result = start_free;
        start_free += total_bytes;
        return(result);
    } else if (bytes_left >= size) {
        nobjs = (int)(bytes_left/size);
        total_bytes = size * nobjs;
        result = start_free;
        start_free += total_bytes;
        return(result);
    } else {
        size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
        // Try to make use of the left-over piece.
        if (bytes_left > 0) {
            obj * __VOLATILE * my_free_list =
                        free_list + FREELIST_INDEX(bytes_left);

            ((obj *)start_free) -> free_list_link = *my_free_list;
            *my_free_list = (obj *)start_free;
        }
        start_free = (char *)malloc(bytes_to_get);
        if (0 == start_free) {
            size_t i;
            obj * __VOLATILE * my_free_list, *p;
            // Try to make do with what we have.  That can't
            // hurt.  We do not try smaller requests, since that tends
            // to result in disaster on multi-process machines.
            for (i = size; i <= __MAX_BYTES; i += __ALIGN) {
                my_free_list = free_list + FREELIST_INDEX(i);
                p = *my_free_list;
                if (0 != p) {
                    *my_free_list = p -> free_list_link;
                    start_free = (char *)p;
                    end_free = start_free + i;
                    return(chunk_alloc(size, nobjs));
                    // Any leftover piece will eventually make it to the
                    // right free list.
                }
            }
	    end_free = 0;	// In case of exception.
            start_free = (char *)malloc_alloc::allocate(bytes_to_get);
            // This should either throw an
            // exception or remedy the situation.  Thus we assume it
            // succeeded.
        }
        heap_size += bytes_to_get;
        end_free = start_free + bytes_to_get;
        return(chunk_alloc(size, nobjs));
    }
}


/* Returns an object of size n, and optionally adds to size n free list.*/
/* We assume that n is properly aligned.                                */
/* We hold the allocation lock.                                         */
template <bool threads, int inst>
void* __default_alloc_template<threads, inst>::refill(size_t n)
{
    int nobjs = 20;
    char * chunk = chunk_alloc(n, nobjs);
    obj * __VOLATILE * my_free_list;
    obj * result;
    obj * current_obj, * next_obj;
    int i;

    if (1 == nobjs) return(chunk);
    my_free_list = free_list + FREELIST_INDEX(n);

    /* Build free list in chunk */
      result = (obj *)chunk;
      *my_free_list = next_obj = (obj *)(chunk + n);
      for (i = 1; ; i++) {
        current_obj = next_obj;
        next_obj = (obj *)((char *)next_obj + n);
        if (nobjs - 1 == i) {
            current_obj -> free_list_link = 0;
            break;
        } else {
            current_obj -> free_list_link = next_obj;
        }
      }
    return(result);
}

template <bool threads, int inst>
void*
__default_alloc_template<threads, inst>::reallocate(void *p,
                                                    size_t old_sz,
                                                    size_t new_sz)
{
    void * result;
    size_t copy_sz;

    if (old_sz > (size_t) __MAX_BYTES && new_sz > (size_t) __MAX_BYTES) {
        return(realloc(p, new_sz));
    }
    if (ROUND_UP(old_sz) == ROUND_UP(new_sz)) return(p);
    result = allocate(new_sz);
    copy_sz = new_sz > old_sz? old_sz : new_sz;
    memcpy(result, p, copy_sz);
    deallocate(p, old_sz);
    return(result);
}


template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::start_free = 0;

template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::end_free = 0;

template <bool threads, int inst>
size_t __default_alloc_template<threads, inst>::heap_size = 0;

template <bool threads, int inst>
typename __default_alloc_template<threads, inst>::obj * __VOLATILE
__default_alloc_template<threads, inst> ::free_list[
    __default_alloc_template<threads, inst>::__NFREELISTS
] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
// The 16 zeros are necessary to make version 4.1 of the SunPro
// compiler happy.  Otherwise it appears to allocate too little
// space for the array.

#endif /* ! __USE_MALLOC */

__STL_END_NAMESPACE

#endif /* __SGI_STL_INTERNAL_ALLOC_H */

// Local Variables:
// mode:C++
// End:

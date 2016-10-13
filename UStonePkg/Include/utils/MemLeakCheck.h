#ifndef	__MEMLEAK_CHECK_H__
#define	__MEMLEAK_CHECK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <Uefi.h>

#define	MEMCHECK_FLAG_NONE		0
//No additional data attached
#define MEMCHECK_FLAG_DIRECT		BIT0
//Do not show any message when free or leak check at the end
#define MEMCHECK_FLAG_NOMSG		BIT1
//Alert when leak check
#define	MEMCHECK_FLAG_ASSERT		BIT2

VOID *
EFIAPI
MemCheck_AllocatePool (
	IN UINTN  AllocationSize,
	IN UINT32 flags,
	IN const char *pFileName,
	IN UINT32 nLine );

VOID *
EFIAPI
MemCheck_AllocateZeroPool (
	IN UINTN  AllocationSize,
	IN UINT32 flags,
	IN const char *pFileName,
	IN UINT32 nLine );

VOID *
EFIAPI
MemCheck_AllocateCopyPool (
	IN UINTN       AllocationSize,
	IN CONST VOID  *Buffer,
	IN UINT32 flags,
	const char *pFileName,
	UINT32 nLine );

VOID *
EFIAPI
MemCheck_ReallocatePool (
	IN UINTN  OldSize,
	IN UINTN  NewSize,
	IN VOID   *OldBuffer  OPTIONAL,
	IN const char *pFileName,
	IN UINT32 nLine);

VOID
EFIAPI
MemCheck_FreePool (
	IN VOID *Buffer,
	IN const char *pFileName,
	IN UINT32 nLine );

void *MemCheck_calloc(
	size_t Num, size_t Size,
	UINT32 flags,
	const char *pFileName,
	UINT32 nLine);

void MemCheck_free(
	void *Ptr,
	const char *pFileName,
	UINT32 nLine);

void *MemCheck_malloc(
	size_t Size,
	UINT32 flags,
	const char *pFileName,
	UINT32 nLine);

void *MemCheck_realloc(
	void *Ptr,
	size_t NewSize,
	const char *pFileName,
	UINT32 nLine);

#if !defined(MDEPKG_NDEBUG) && !defined(__MEMLEAK_CHECK_NOMACRO__)
#define AllocatePool(...) MemCheck_AllocatePool(__VA_ARGS__, MEMCHECK_FLAG_NONE, __FILE__, __LINE__)
#define AllocatePool_Direct(...) MemCheck_AllocatePool(__VA_ARGS__, MEMCHECK_FLAG_DIRECT, __FILE__, __LINE__)
#define AllocateZeroPool(...) MemCheck_AllocateZeroPool(__VA_ARGS__, MEMCHECK_FLAG_NONE, __FILE__, __LINE__)
#define AllocateZeroPool_Direct(...) MemCheck_AllocateZeroPool(__VA_ARGS__, MEMCHECK_FLAG_DIRECT, __FILE__, __LINE__)
#define AllocateCopyPool(...) MemCheck_AllocateCopyPool(__VA_ARGS__, MEMCHECK_FLAG_NONE, __FILE__, __LINE__)
#define AllocateCopyPool_Direct(...) MemCheck_AllocateCopyPool(__VA_ARGS__, MEMCHECK_FLAG_DIRECT, __FILE__, __LINE__)
#define ReallocatePool(...) MemCheck_ReallocatePool(__VA_ARGS__, __FILE__, __LINE__)
#define FreePool(...) MemCheck_FreePool(__VA_ARGS__, __FILE__, __LINE__)
#define malloc(...) MemCheck_malloc(__VA_ARGS__, MEMCHECK_FLAG_NONE, __FILE__, __LINE__)
#define malloc_direct(...) MemCheck_malloc(__VA_ARGS__, MEMCHECK_FLAG_DIRECT, __FILE__, __LINE__)
#define realloc(...) MemCheck_realloc(__VA_ARGS__, __FILE__, __LINE__)
#define calloc(...) MemCheck_calloc(__VA_ARGS__, MEMCHECK_FLAG_NONE, __FILE__, __LINE__)
#define calloc_direct(...) MemCheck_calloc(__VA_ARGS__, MEMCHECK_FLAG_DIRECT, __FILE__, __LINE__)
#define free(...) MemCheck_free(__VA_ARGS__, __FILE__, __LINE__)
#else
#define AllocatePool_Direct(...) AllocatePool(__VA_ARGS__)
#define AllocateZeroPool_Direct(...) AllocateZeroPool(__VA_ARGS__)
#define AllocateCopyPool_Direct(...) AllocateCopyPool(__VA_ARGS__)
#define malloc_direct(...) malloc(__VA_ARGS__)
#define calloc_direct(...) calloc(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif

/*
 * Copyright (c) 2016 by Nero Liu .  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 *
 */

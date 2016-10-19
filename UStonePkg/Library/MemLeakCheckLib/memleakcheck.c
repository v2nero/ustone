#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <stdio.h>
#include <stdlib.h>
#define __MEMLEAK_CHECK_NOMACRO__
#include <utils/MemLeakCheck.h>

//=============standard POOL
#define UEFIPOOL_HEAD_SIGNATURE   SIGNATURE_32('p','h','d','0')
typedef struct {
  UINT32          Signature;
  UINT32          Reserved;
  EFI_MEMORY_TYPE Type;
  UINTN           Size;
  CHAR8           Data[1];
} UEFIPOOL_HEAD;
#define SIZE_OF_UEFIPOOL_HEAD OFFSET_OF(UEFIPOOL_HEAD,Data)


//=========== C malloc header
#define CPOOL_HEAD_SIGNATURE   SIGNATURE_32('C','p','h','d')

/** The UEFI functions do not provide a way to determine the size of an
    allocated region of memory given just a pointer to the start of that
    region.  Since this is required for the implementation of realloc,
    the memory head structure, CPOOL_HEAD, containing the necessary
    information is prepended to the requested space.

    The order of members is important.  This structure is 8-byte aligned,
    as per the UEFI specification for memory allocation functions.  By
    specifying Size as a 64-bit value and placing it immediately before
    Data, it ensures that Data will always be 8-byte aligned.

    On IA32 systems, this structure is 24 bytes long, excluding Data.
    On X64  systems, this structure is 32 bytes long, excluding Data.
**/
typedef struct {
  LIST_ENTRY      List;
  UINT32          Signature;
  UINT64          Size;
  CHAR8           Data[1];
} CPOOL_HEAD;
#define SIZE_OF_CPOOL_HEAD OFFSET_OF(CPOOL_HEAD,Data)

//==============================
#define LEAKCHAECK_FREE_SIGNATURE   SIGNATURE_32('L','f','r','0')
#define LEAKCHAECK_UP_SIGNATURE   SIGNATURE_32('L','u','p','0')
#define LEAKCHAECK_HEAD_SIGNATURE   SIGNATURE_32('L','h','d','0')
#define LEAKCHAECK_TAIL_SIGNATURE   SIGNATURE_32('L','t','l','0')

#if 0
#define	MEMCHECK_FLAG_NONE		0
//No additional data attached
#define MEMCHECK_FLAG_DIRECT		BIT0
//Do not show any message when free or leak check at the end
#define MEMCHECK_FLAG_NOMSG		BIT1
//Alert when leak check
#define	MEMCHECK_FLAG_ASSERT		BIT2
#endif

typedef struct {
	UINT32 signature;
	UINT32 reserved;
	LIST_ENTRY list;
	UINTN total_size;
	UINT32 checksum;
	UINT32 index;
	UINT32 pool_offset;
	UINT32 tail_offset;
	UINT32 line;
	UINT32 flags;
} MemCheckHeapHeader;

typedef struct {
	//char filepath[]; 8 bytes align
	UINTN pool_size;
	UINT32 pool_offset;
	UINT32 checksum;
	UINT32 line_num;
	UINT32 flags;
	UINT32 signature;
	UINT32 reserved;
} MemCheckHeapMid;

typedef struct {
	UINT32 signature;
	UINT32 tail_offset;
} MemCheckHeapTail;

#define align8(x) (((x)+7)/8*8)
#define to_head(ptr) ((MemCheckHeapHeader *)(ptr))
#define to_heapsize(alloc_size, fname_len) (sizeof(MemCheckHeapHeader) + align8((fname_len) + 1) + sizeof(MemCheckHeapMid) + (alloc_size) + sizeof(MemCheckHeapTail))
#define realpool_offset(fname_len) (sizeof(MemCheckHeapHeader) + align8((fname_len) + 1) + sizeof(MemCheckHeapMid))
#define to_realpool(p, fname_len) ((VOID *)((CHAR8*)(p) + realpool_offset(fname_len)))
#define to_mid(p, fname_len) ((MemCheckHeapMid *)((CHAR8*)(p) + sizeof(MemCheckHeapHeader) + align8((fname_len) + 1)))
#define tail_offset(alloc_size, fname_len) ((alloc_size) + realpool_offset(fname_len))
#define	to_tail(p, alloc_size, fname_len) ((MemCheckHeapTail *)((CHAR8*)(p) + tail_offset(alloc_size, fname_len)))

#define MEMLEAK_TRACE_FLAG	"MemLeakCheck: "
#define TRACE_FREEPOS(f, l) DEBUG((EFI_D_ERROR, MEMLEAK_TRACE_FLAG"%a(%d): <-- free position\n", (f), (l)))
#define TRACE_ALLOCPOS(f, l) DEBUG((EFI_D_ERROR, MEMLEAK_TRACE_FLAG"%a(%d): <-- allocate position\n", (f), (l)))
#define TRACE_OVERRIDE_BY_OTHERS DEBUG((EFI_D_ERROR, MEMLEAK_TRACE_FLAG"memory override by others\n"))
#define TRACE_OVERRIDE_BY_ITSELF DEBUG((EFI_D_ERROR, MEMLEAK_TRACE_FLAG"memory override by itself\n"))
#define TRACE_LEAK DEBUG((EFI_D_ERROR, MEMLEAK_TRACE_FLAG"Memory Leak\n"))
#define TRACE_FREE_NULL DEBUG((EFI_D_ERROR, MEMLEAK_TRACE_FLAG"Free NULL\n"))
#define TRACE_COPY_NULL DEBUG((EFI_D_ERROR, MEMLEAK_TRACE_FLAG"Copy NULL\n"))
#define TRACE_ASSERT DEBUG((EFI_D_ERROR, MEMLEAK_TRACE_FLAG"ASSERT\n"))

static LIST_ENTRY gMemListHead = INITIALIZE_LIST_HEAD_VARIABLE(gMemListHead);
static UINT32 g_index = 0;

static UINT32 CalcCheckSum(void* pData, size_t len)
{
	//calculate checksum
	UINT32 checksum = 0;
	UINT32 size32 = (UINT32)len/4;
	UINT32 *pPack32 = (UINT32*)pData;
	UINT32 size8 = 0;
	UINT8 *pPack8 = 0;
	UINT32 i = 0;

	while(size32) {
		checksum += *pPack32;
		++pPack32;
		--size32;
	}

	size8 = len%4;
	pPack8 = (UINT8*)pPack32;
	for (i = 0; i < size8; i++)
	{
		checksum += ((UINT32)*pPack8)<<(i*8);
		++pPack8;
	}

	return 0-checksum;
}

static void _my_checkoverride(LIST_ENTRY *pNode)
{
	MemCheckHeapHeader *pHead = NULL;
	MemCheckHeapMid *pMid = NULL;
	MemCheckHeapTail *pTail = NULL;
	
	pHead = BASE_CR(pNode, MemCheckHeapHeader, list);
	if (pHead->signature != LEAKCHAECK_HEAD_SIGNATURE) {
		TRACE_OVERRIDE_BY_OTHERS;
		ASSERT(pHead->signature == LEAKCHAECK_HEAD_SIGNATURE);
	}
	
	if (CalcCheckSum(
		((CHAR8*)pHead) + OFFSET_OF(MemCheckHeapHeader, total_size),
		(UINTN)sizeof(MemCheckHeapHeader) - OFFSET_OF(MemCheckHeapHeader, total_size)) != 0) {
		TRACE_OVERRIDE_BY_OTHERS;
		ASSERT(CalcCheckSum(
			((CHAR8*)pHead) + OFFSET_OF(MemCheckHeapHeader, total_size),
			(UINTN)sizeof(MemCheckHeapHeader) - OFFSET_OF(MemCheckHeapHeader, total_size)) == 0);
	}
	
	pMid = (MemCheckHeapMid*)(((CHAR8*)pHead)+pHead->pool_offset - sizeof(MemCheckHeapMid));
	pTail = (MemCheckHeapTail *) (((CHAR8*)pHead)+pHead->tail_offset);
	
	if (pMid->signature != LEAKCHAECK_UP_SIGNATURE ||
	    pTail->tail_offset != pHead->tail_offset ||
	    CalcCheckSum(pMid, sizeof(MemCheckHeapMid)) != 0) {
		TRACE_OVERRIDE_BY_OTHERS;
		TRACE_ALLOCPOS((char *)(pHead+1), pMid->line_num);
		ASSERT(pMid->signature == LEAKCHAECK_UP_SIGNATURE &&
		    pTail->tail_offset == pHead->tail_offset && 
		    CalcCheckSum(pMid, sizeof(MemCheckHeapMid)) == 0);
	}
	
	if (pTail->signature != LEAKCHAECK_TAIL_SIGNATURE) {
		TRACE_OVERRIDE_BY_ITSELF;
		TRACE_ALLOCPOS((char *)(pHead+1), pMid->line_num);
		ASSERT(pTail->signature == LEAKCHAECK_TAIL_SIGNATURE);
	}
}

static
VOID _my_TraceLeftPools(void)
{
	MemCheckHeapHeader *pHead = NULL;
	LIST_ENTRY *pNode;

	DEBUG((EFI_D_ERROR, MEMLEAK_TRACE_FLAG"%a Start****\n", __FUNCTION__));
	for (pNode = gMemListHead.ForwardLink; pNode != &gMemListHead; pNode = pNode->ForwardLink) {
		_my_checkoverride(pNode);
		pHead = BASE_CR(pNode, MemCheckHeapHeader, list);
		if (!(pHead->flags & MEMCHECK_FLAG_NOMSG)) {
			TRACE_LEAK;
			TRACE_ALLOCPOS((char *)pHead + sizeof(MemCheckHeapHeader), pHead->line);
		}
		if (pHead->flags & MEMCHECK_FLAG_ASSERT) {
			TRACE_ASSERT;
			ASSERT(0);
		}
	}
	DEBUG((EFI_D_ERROR, MEMLEAK_TRACE_FLAG"%a End****\n", __FUNCTION__));
}

RETURN_STATUS
EFIAPI
__memleak_deconstruct (VOID)
{
#if !defined(MDEPKG_NDEBUG)
	_my_TraceLeftPools();
#endif
	return EFI_SUCCESS;
}

static
VOID fill_data(
	CHAR8 *ptr,
	UINTN alloc_size,
	UINT32 flags,
	const char *filename,
	UINT32 linenum)
{
	UINTN str_len = AsciiStrLen(filename);
	MemCheckHeapHeader *pHead = to_head(ptr);
	MemCheckHeapMid *pMid = to_mid(ptr, str_len);
	MemCheckHeapTail *pTail = to_tail(ptr, alloc_size, str_len);

	pMid->signature = LEAKCHAECK_UP_SIGNATURE;
	pMid->line_num = linenum;
	pMid->pool_offset = (UINT32)realpool_offset(str_len);
	pMid->flags = flags;
	pMid->pool_size = alloc_size;
	pMid->reserved = 0;
	pMid->checksum = 0;
	pMid->checksum = CalcCheckSum(pMid, sizeof(MemCheckHeapMid));

	pTail->signature = LEAKCHAECK_TAIL_SIGNATURE;
	pTail->tail_offset = (UINT32)tail_offset(alloc_size, str_len);

	pHead->signature = LEAKCHAECK_HEAD_SIGNATURE;
	pHead->pool_offset = pMid->pool_offset;
	pHead->tail_offset = pTail->tail_offset;
	pHead->total_size = to_heapsize(alloc_size, str_len);
	pHead->index = g_index;
	g_index++;
	pHead->reserved = 0;
	pHead->checksum = 0;
	pHead->line = linenum;
	pHead->flags = flags;
	pHead->checksum = CalcCheckSum(
			ptr + OFFSET_OF(MemCheckHeapHeader, total_size),
			(UINTN)sizeof(MemCheckHeapHeader) - OFFSET_OF(MemCheckHeapHeader, total_size));
	AsciiStrCpy((CHAR8*)(pHead + 1), filename);

	InsertTailList(&gMemListHead, &pHead->list);
}

BOOLEAN is_uefipool(VOID *ptr)
{
	UEFIPOOL_HEAD *pHead = (UEFIPOOL_HEAD *)((CHAR8*)ptr - SIZE_OF_UEFIPOOL_HEAD);
	if (pHead->Signature == UEFIPOOL_HEAD_SIGNATURE) {
		return TRUE;
	}
	return FALSE;
}

BOOLEAN is_cpool(VOID *ptr)
{
	CPOOL_HEAD *pHead = (CPOOL_HEAD *)((CHAR8*)ptr - SIZE_OF_CPOOL_HEAD);
	if (pHead->Signature == CPOOL_HEAD_SIGNATURE) {
		return TRUE;
	}

	return FALSE;
}

BOOLEAN is_mcpool(VOID *ptr)
{
	MemCheckHeapMid *pMid = (MemCheckHeapMid *)((CHAR8*)ptr - sizeof(MemCheckHeapMid));
	if (pMid->signature == LEAKCHAECK_UP_SIGNATURE) {
		return TRUE;
	}
	return FALSE;
}

VOID *
EFIAPI
MemCheck_AllocatePool (
	IN UINTN  size,
	IN UINT32 flags,
	IN const char *filename,
	IN UINT32 linenum )
{
	static CHAR16 c16Resean[] = L"Out of memory";
	CHAR8 *ptr = NULL;
	UINTN str_len = AsciiStrLen(filename);

	if (size == 0) {
		return NULL;
	}
	if (flags & MEMCHECK_FLAG_DIRECT) {
		return AllocatePool(size);
	}

	ptr = AllocatePool(to_heapsize(size, str_len));
	ASSERT(ptr != NULL);
	if (ptr != NULL) {
		fill_data(ptr, size, flags, filename, linenum);
		return to_realpool(ptr, str_len);
	}
	return NULL;
}

VOID *
EFIAPI
MemCheck_AllocateZeroPool (
	IN UINTN  AllocationSize,
	IN UINT32 flags,
	IN const char *pFileName,
	IN UINT32 nLine )
{
	static CHAR16 c16Resean[] = L"C++ runtime error: Out of memory";
	CHAR8 *ptr = NULL;
	UINTN str_len = AsciiStrLen(pFileName);

	if (AllocationSize == 0) {
		return NULL;
	}
	if (flags & MEMCHECK_FLAG_DIRECT) {
		return AllocateZeroPool(AllocationSize);
	}

	ptr = AllocateZeroPool(to_heapsize(AllocationSize, str_len));
	ASSERT(ptr != NULL);
	if (ptr != NULL) {
		fill_data(ptr, AllocationSize, flags, pFileName, nLine);
		return to_realpool(ptr, str_len);
	}
	return NULL;
}

VOID *
EFIAPI
MemCheck_AllocateCopyPool (
	IN UINTN       AllocationSize,
	IN CONST VOID  *Buffer,
	IN UINT32 flags,
	const char *pFileName,
	UINT32 nLine )
{
	  VOID  *Memory;

	  if (Buffer == NULL) {
		  TRACE_COPY_NULL;
		  TRACE_ALLOCPOS(pFileName, nLine);
	  }
	  ASSERT (Buffer != NULL);

	  Memory = MemCheck_AllocatePool(AllocationSize, flags, pFileName, nLine);
	  if (Memory != NULL) {
	     Memory = CopyMem (Memory, Buffer, AllocationSize);
	  }
	  return Memory;
}

VOID *
EFIAPI
MemCheck_ReallocatePool (
	IN UINTN  OldSize,
	IN UINTN  NewSize,
	IN VOID   *OldBuffer  OPTIONAL,
	IN const char *pFileName,
	IN UINT32 nLine)
{
	VOID  *NewBuffer;
	UINT32 flags = 0;
	MemCheckHeapMid *pMid = NULL;

	if (OldBuffer != NULL && is_uefipool(OldBuffer)) {
		return ReallocatePool(OldSize, NewSize, OldBuffer);
	}

	if (OldBuffer != NULL) {
		if (is_cpool(OldBuffer)) {
			DEBUG((EFI_D_ERROR, MEMLEAK_TRACE_FLAG"not append ReallocatePool to malloc\n"));
			TRACE_ALLOCPOS(pFileName, nLine);
		}
		ASSERT(!is_cpool(OldBuffer));

		if (!is_mcpool(OldBuffer)) {
			TRACE_OVERRIDE_BY_OTHERS;
			TRACE_ALLOCPOS(pFileName, nLine);
		}
		ASSERT(is_mcpool(OldBuffer));

		pMid =(MemCheckHeapMid*) ((CHAR8*)OldBuffer - sizeof(MemCheckHeapMid));
		if (CalcCheckSum(pMid, sizeof(MemCheckHeapMid)) != 0) {
			TRACE_OVERRIDE_BY_OTHERS;
			TRACE_FREEPOS(pFileName, nLine);
			ASSERT(0);
		}
		flags = pMid->flags;
	}

	NewBuffer = MemCheck_AllocateZeroPool(NewSize, flags, pFileName, nLine);
	if (NewBuffer != NULL && OldBuffer != NULL) {
		CopyMem (NewBuffer, OldBuffer, MIN (OldSize, NewSize));
		MemCheck_FreePool(OldBuffer, pFileName, nLine);
	}
	return NewBuffer;
}

void MemCheck_CheckOverride()
{
	LIST_ENTRY *pNode;

	DEBUG((EFI_D_ERROR, "%a Start****\n", __FUNCTION__));
	for (pNode = gMemListHead.ForwardLink; pNode != &gMemListHead; pNode = pNode->ForwardLink) {
		_my_checkoverride(pNode);
	}
	DEBUG((EFI_D_ERROR, "%a End****\n", __FUNCTION__));
}

static
VOID __myfree(
	IN VOID *ptr,
	IN const char *filename,
	IN UINT32 nline )
{
	MemCheckHeapMid *pMid = (MemCheckHeapMid *)((CHAR8*)(ptr) - sizeof(MemCheckHeapMid));
	MemCheckHeapHeader *pHead = NULL;
	MemCheckHeapTail *pTail = NULL;

	if (CalcCheckSum(pMid, sizeof(MemCheckHeapMid)) != 0) {
		TRACE_OVERRIDE_BY_OTHERS;
		TRACE_FREEPOS(filename, nline);
		return;
	}

	pHead = (MemCheckHeapHeader *)((CHAR8*)(ptr) - pMid->pool_offset);
	if (pHead->signature != LEAKCHAECK_HEAD_SIGNATURE) {
		TRACE_OVERRIDE_BY_OTHERS;
		TRACE_FREEPOS(filename, nline);
		return;
	}

	if (CalcCheckSum(
		(CHAR8*)pHead + OFFSET_OF(MemCheckHeapHeader, total_size),
		(UINTN)sizeof(MemCheckHeapHeader) - OFFSET_OF(MemCheckHeapHeader, total_size)) != 0) {
		TRACE_OVERRIDE_BY_OTHERS;
		TRACE_FREEPOS(filename, nline);
		return;
	}
	pTail = (MemCheckHeapTail *)((CHAR8*)pHead + pHead->tail_offset);
	if (pTail->signature != LEAKCHAECK_TAIL_SIGNATURE ||
	    pTail->tail_offset != pHead->tail_offset) {
		TRACE_OVERRIDE_BY_OTHERS;
		TRACE_ALLOCPOS((char *)(pHead+1), pMid->line_num);
		TRACE_FREEPOS(filename, nline);
		RemoveEntryList(&pHead->list);
		return;
	}
	RemoveEntryList(&pHead->list);
	FreePool(pHead);
}

VOID
EFIAPI
MemCheck_FreePool (
	IN VOID *Buffer,
	IN const char *pFileName,
	IN UINT32 nLine )
{
	if (Buffer == NULL) {
		TRACE_FREE_NULL;
		TRACE_FREEPOS(pFileName, nLine);
		ASSERT(Buffer);
		return;
	}

	if (is_uefipool(Buffer)) {
		FreePool(Buffer);
		return;
	}

	if (is_cpool(Buffer)) {
		DEBUG((EFI_D_ERROR, MEMLEAK_TRACE_FLAG"not append FreePool to malloc\n"));
		TRACE_FREEPOS(pFileName, nLine);
	}
	ASSERT(!is_cpool(Buffer));

	if (!is_mcpool(Buffer)) {
		TRACE_OVERRIDE_BY_OTHERS;
		TRACE_FREEPOS(pFileName, nLine);
		ASSERT(is_mcpool(Buffer));
		return;
	}
	__myfree(Buffer, pFileName, nLine);
}

void *MemCheck_calloc(
	size_t Num, size_t Size,
	UINT32 flags,
	const char *pFileName,
	UINT32 nLine)
{
	return MemCheck_malloc(Num*Size, flags, pFileName, nLine);
}

void MemCheck_free(
	void *Ptr,
	const char *pFileName,
	UINT32 nLine)
{
	if (Ptr == NULL) {
		TRACE_FREE_NULL;
		TRACE_FREEPOS(pFileName, nLine);
		ASSERT(Ptr);
		return;
	}

	if (is_uefipool(Ptr)) {
		FreePool(Ptr);
		return;
	}

	if (is_cpool(Ptr)) {
		free(Ptr);
		return;
	}

	if (!is_mcpool(Ptr)) {
		TRACE_OVERRIDE_BY_OTHERS;
		TRACE_FREEPOS(pFileName, nLine);
		ASSERT(is_mcpool(Ptr));
		return;
	}
	__myfree(Ptr, pFileName, nLine);
}

void *MemCheck_malloc(
	size_t size,
	UINT32 flags,
	const char *filename,
	UINT32 linenum)
{
	static CHAR16 c16Resean[] = L"Out of memory";
	CHAR8 *ptr = NULL;
	UINTN str_len = AsciiStrLen(filename);

	if (size == 0) {
		return NULL;
	}
	if (flags & MEMCHECK_FLAG_DIRECT) {
		return malloc(size);
	}

	ptr = AllocatePool(to_heapsize(size, str_len));
	ASSERT(ptr != NULL);
	if (ptr != NULL) {
		fill_data(ptr, size, flags, filename, linenum);
		return to_realpool(ptr, str_len);
	}
	return NULL;
}

void *MemCheck_realloc(
	void *Ptr,
	size_t NewSize,
	const char *pFileName,
	UINT32 nLine)
{
	VOID  *NewBuffer;
	UINT32 flags = 0;
	MemCheckHeapMid *pMid = NULL;

	if (Ptr != NULL && is_uefipool(Ptr)) {
		UEFIPOOL_HEAD *pUHead = (UEFIPOOL_HEAD *)((CHAR8*)Ptr - SIZE_OF_UEFIPOOL_HEAD);
		if (pUHead->Signature != UEFIPOOL_HEAD_SIGNATURE) {
			TRACE_OVERRIDE_BY_OTHERS;
			TRACE_ALLOCPOS(pFileName, nLine);
		}
		ASSERT(pUHead->Signature == UEFIPOOL_HEAD_SIGNATURE);
		return ReallocatePool(pUHead->Size, NewSize, Ptr);
	}
	if (Ptr != NULL && is_cpool(Ptr)) {
		return realloc(Ptr, NewSize);
	}

	if (Ptr != NULL) {
		if (!is_mcpool(Ptr)) {
			TRACE_OVERRIDE_BY_OTHERS;
			TRACE_ALLOCPOS(pFileName, nLine);
		}
		ASSERT(is_mcpool(Ptr));

		pMid = (MemCheckHeapMid *) ((CHAR8*)Ptr - sizeof(MemCheckHeapMid));
		if (CalcCheckSum(pMid, sizeof(MemCheckHeapMid)) != 0) {
			TRACE_OVERRIDE_BY_OTHERS;
			TRACE_FREEPOS(pFileName, nLine);
			ASSERT(0);
		}
		pMid = (MemCheckHeapMid *)((CHAR8*)Ptr - sizeof(MemCheckHeapMid));
		ASSERT(pMid->signature == LEAKCHAECK_UP_SIGNATURE);
		flags = pMid->flags;
	}

	NewBuffer = MemCheck_AllocateZeroPool(NewSize, flags, pFileName, nLine);
	if (NewBuffer != NULL && Ptr != NULL) {
		CopyMem (NewBuffer, Ptr, MIN (pMid->pool_size, NewSize));
		MemCheck_FreePool(Ptr, pFileName, nLine);
	}
	return NewBuffer;
}

#if 0
/**
  Allocates a buffer of type EfiBootServicesData.

  Allocates the number bytes specified by AllocationSize of type EfiBootServicesData and returns a
  pointer to the allocated buffer.  If AllocationSize is 0, then a valid buffer of 0 size is
  returned.  If there is not enough memory remaining to satisfy the request, then NULL is returned.

  @param  AllocationSize        The number of bytes to allocate.

  @return A pointer to the allocated buffer or NULL if allocation fails.

**/
VOID *
EFIAPI
AllocatePool (
  IN UINTN  AllocationSize
  )
{
  VOID  *Buffer;

  Buffer = InternalAllocatePool (EfiBootServicesData, AllocationSize);
  if (Buffer != NULL) {
    MemoryProfileLibRecord (
      (PHYSICAL_ADDRESS) (UINTN) RETURN_ADDRESS(0),
      MEMORY_PROFILE_ACTION_LIB_ALLOCATE_POOL,
      EfiBootServicesData,
      Buffer,
      AllocationSize,
      NULL
      );
  }
  return Buffer;
}

/**
  Allocates and zeros a buffer of type EfiBootServicesData.

  Allocates the number bytes specified by AllocationSize of type EfiBootServicesData, clears the
  buffer with zeros, and returns a pointer to the allocated buffer.  If AllocationSize is 0, then a
  valid buffer of 0 size is returned.  If there is not enough memory remaining to satisfy the
  request, then NULL is returned.

  @param  AllocationSize        The number of bytes to allocate and zero.

  @return A pointer to the allocated buffer or NULL if allocation fails.

**/
VOID *
EFIAPI
AllocateZeroPool (
  IN UINTN  AllocationSize
  )
{
  VOID  *Buffer;

  Buffer = InternalAllocateZeroPool (EfiBootServicesData, AllocationSize);
  if (Buffer != NULL) {
    MemoryProfileLibRecord (
      (PHYSICAL_ADDRESS) (UINTN) RETURN_ADDRESS(0),
      MEMORY_PROFILE_ACTION_LIB_ALLOCATE_ZERO_POOL,
      EfiBootServicesData,
      Buffer,
      AllocationSize,
      NULL
      );
  }
  return Buffer;
}


/**
  Copies a buffer to an allocated buffer of type EfiBootServicesData.

  Allocates the number bytes specified by AllocationSize of type EfiBootServicesData, copies
  AllocationSize bytes from Buffer to the newly allocated buffer, and returns a pointer to the
  allocated buffer.  If AllocationSize is 0, then a valid buffer of 0 size is returned.  If there
  is not enough memory remaining to satisfy the request, then NULL is returned.

  If Buffer is NULL, then ASSERT().
  If AllocationSize is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

  @param  AllocationSize        The number of bytes to allocate and zero.
  @param  Buffer                The buffer to copy to the allocated buffer.

  @return A pointer to the allocated buffer or NULL if allocation fails.

**/
VOID *
EFIAPI
AllocateCopyPool (
  IN UINTN       AllocationSize,
  IN CONST VOID  *Buffer
  )
{
  VOID  *NewBuffer;

  NewBuffer = InternalAllocateCopyPool (EfiBootServicesData, AllocationSize, Buffer);
  if (NewBuffer != NULL) {
    MemoryProfileLibRecord (
      (PHYSICAL_ADDRESS) (UINTN) RETURN_ADDRESS(0),
      MEMORY_PROFILE_ACTION_LIB_ALLOCATE_COPY_POOL,
      EfiBootServicesData,
      NewBuffer,
      AllocationSize,
      NULL
      );
  }
  return NewBuffer;
}


/**
  Reallocates a buffer of type EfiBootServicesData.

  Allocates and zeros the number bytes specified by NewSize from memory of type
  EfiBootServicesData.  If OldBuffer is not NULL, then the smaller of OldSize and
  NewSize bytes are copied from OldBuffer to the newly allocated buffer, and
  OldBuffer is freed.  A pointer to the newly allocated buffer is returned.
  If NewSize is 0, then a valid buffer of 0 size is  returned.  If there is not
  enough memory remaining to satisfy the request, then NULL is returned.

  If the allocation of the new buffer is successful and the smaller of NewSize and OldSize
  is greater than (MAX_ADDRESS - OldBuffer + 1), then ASSERT().

  @param  OldSize        The size, in bytes, of OldBuffer.
  @param  NewSize        The size, in bytes, of the buffer to reallocate.
  @param  OldBuffer      The buffer to copy to the allocated buffer.  This is an optional
                         parameter that may be NULL.

  @return A pointer to the allocated buffer or NULL if allocation fails.

**/
VOID *
EFIAPI
ReallocatePool (
  IN UINTN  OldSize,
  IN UINTN  NewSize,
  IN VOID   *OldBuffer  OPTIONAL
  )
{
  VOID  *Buffer;

  Buffer = InternalReallocatePool (EfiBootServicesData, OldSize, NewSize, OldBuffer);
  if (Buffer != NULL) {
    MemoryProfileLibRecord (
      (PHYSICAL_ADDRESS) (UINTN) RETURN_ADDRESS(0),
      MEMORY_PROFILE_ACTION_LIB_REALLOCATE_POOL,
      EfiBootServicesData,
      Buffer,
      NewSize,
      NULL
      );
  }
  return Buffer;
}

/**
  Frees a buffer that was previously allocated with one of the pool allocation functions in the
  Memory Allocation Library.

  Frees the buffer specified by Buffer.  Buffer must have been allocated on a previous call to the
  pool allocation services of the Memory Allocation Library.  If it is not possible to free pool
  resources, then this function will perform no actions.

  If Buffer was not allocated with a pool allocation function in the Memory Allocation Library,
  then ASSERT().

  @param  Buffer                Pointer to the buffer to free.

**/
VOID
EFIAPI
FreePool (
  IN VOID   *Buffer
  )
{
  EFI_STATUS    Status;

  Status = CoreFreePool (Buffer);
  ASSERT_EFI_ERROR (Status);
}
#endif

/*
 * Copyright (c) 2016 by Nero Liu .  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 *
 */

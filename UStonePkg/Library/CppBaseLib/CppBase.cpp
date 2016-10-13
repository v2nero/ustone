#define __CPP_BASE_H__
#include <cpp/cppbase.hpp>
#include <utils/MemLeakCheck.h>

#if 0
static LIST_ENTRY gMemListHead = INITIALIZE_LIST_HEAD_VARIABLE(gMemListHead);
#endif

#if defined(MDEPKG_NDEBUG)

#if 0
void FreeLeftMemPools(bool bAlert)
{
	LIST_ENTRY *pNode;

	while (FALSE == IsListEmpty(&gMemListHead)) {
		pNode = GetFirstNode(&gMemListHead);
		RemoveEntryList(pNode);
		FreePool(pNode);
	}
}
#endif

void * operator new(size_t size)
{
#if 0
	static CHAR16 c16Resean[] = L"C++ runtime error: Out of memory";
	void *ptr = AllocatePool((UINTN)(size + sizeof(LIST_ENTRY)));
	ASSERT(ptr != nullptr);
	if (ptr != nullptr) {
		InsertTailList(&gMemListHead, (LIST_ENTRY *)ptr);
		return ((CHAR8 *)ptr)+sizeof(LIST_ENTRY);
	}

	FreeLeftMemPools();
	gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(c16Resean), c16Resean);

	return nullptr;
#endif
	static CHAR16 c16Resean[] = L"C++ runtime error: Out of memory";
	void *ptr = AllocatePool(size);
	if (ptr != nullptr) {
		return ptr;
	}
	gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(c16Resean), c16Resean);
	return nullptr;
}

void * operator new[](size_t size)
{
#if 0
	static CHAR16 c16Resean[] = L"C++ runtime error: Out of memory";
	void *ptr = AllocatePool((UINTN)(size + sizeof(LIST_ENTRY)));
	ASSERT(ptr != nullptr);
	if (ptr != nullptr) {
		InsertTailList(&gMemListHead, (LIST_ENTRY *)ptr);
		return ((CHAR8 *)ptr)+sizeof(LIST_ENTRY);
	}

	FreeLeftMemPools();
	gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(c16Resean), c16Resean);

	return nullptr;
#endif
	static CHAR16 c16Resean[] = L"C++ runtime error: Out of memory";
	void *ptr = AllocatePool(size);
	if (ptr != nullptr) {
		return ptr;
	}
	gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(c16Resean), c16Resean);
	return nullptr;
}

void operator delete(void *ptr)
{
	if (ptr != nullptr) {
		FreePool(((CHAR8 *)ptr) - sizeof(LIST_ENTRY));
	}
}

void operator delete[](void *ptr)
{
	if (ptr != nullptr) {
		FreePool(((CHAR8 *)ptr) - sizeof(LIST_ENTRY));
	}
}

#else
#if 0
//#pragma pack(1)
typedef struct {
	LIST_ENTRY list;
	UINTN size;
	int file_line;
	//char FilePath;
} HeapHeader;

typedef struct {
	UINTN mid_offset;
	UINT32 dumy;
} HeapMid;

typedef struct {
	UINT32 dumy;
} HeapTail;
//#pragma pack()

void FreeLeftMemPools(bool bAlert)
{
	HeapHeader *pHead = nullptr;

	LIST_ENTRY *pNode;

	while (FALSE == IsListEmpty(&gMemListHead)) {
		pNode = GetFirstNode(&gMemListHead);
		RemoveEntryList(pNode);
		if (bAlert) {
			pHead = (HeapHeader *)pNode;
			DEBUG((EFI_D_ERROR, "%a(%d): Memory Leak\n", (char *)pHead + sizeof(HeapHeader), pHead->file_line));
		}
		FreePool(pNode);
	}
}

extern "C"
RETURN_STATUS
EFIAPI
__cppbaselib_deconstruct ( VOID	)
{
#if !defined(MDEPKG_NDEBUG)
	FreeLeftMemPools(true);
#endif
	return EFI_SUCCESS;
}
#endif

void * operator new(size_t size, const char *pFileName, int nLine)
{
#if 0
	static CHAR16 c16Resean[] = L"C++ runtime error: Out of memory";
	HeapMid *pMid = nullptr;
	HeapTail *pTail = nullptr;
	char *pMyFile = nullptr;
	UINTN unStrLen = AsciiStrLen(pFileName) + 1;
	HeapHeader *ptr = (HeapHeader *)AllocatePool(
			(UINTN)(size + sizeof(HeapHeader) + unStrLen + sizeof(HeapMid) + sizeof(HeapTail)));
	ASSERT(ptr != nullptr);
	if (ptr != nullptr) {
		pMyFile = (char *)ptr + sizeof(HeapHeader);
		*pMyFile = 0;
		AsciiStrCpy(pMyFile, pFileName);
		ptr->size = size;
		ptr->file_line = nLine;
		pMid = (HeapMid *) ((char *)ptr + sizeof(HeapHeader) + unStrLen);
		pMid->mid_offset = sizeof(HeapHeader) + unStrLen;
		pMid->dumy = 0x6c6c6c6c;
		pTail = (HeapTail *) ((char *)pMid + sizeof(HeapMid) + size);
		pTail->dumy = 0x6c6c6c6c;
		InsertTailList(&gMemListHead, &ptr->list);
		return (char *)ptr + sizeof(HeapHeader) + unStrLen + sizeof(HeapMid);
	}

	FreeLeftMemPools();
	gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(c16Resean), c16Resean);

	return nullptr;
#endif
	static CHAR16 c16Resean[] = L"C++ runtime error: Out of memory";
	void *ptr = MemCheck_AllocatePool(size, MEMCHECK_FLAG_NONE, pFileName, nLine);
	if (ptr != nullptr) {
		return ptr;
	}
	gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(c16Resean), c16Resean);
	return nullptr;
}

void * operator new[](size_t size, const char *pFileName, int nLine)
{
#if 0
	static CHAR16 c16Resean[] = L"C++ runtime error: Out of memory";
	HeapMid *pMid = nullptr;
	HeapTail *pTail = nullptr;
	char *pMyFile = nullptr;
	UINTN unStrLen = AsciiStrLen(pFileName) + 1;
	HeapHeader *ptr = (HeapHeader *)AllocatePool(
			(UINTN)(size + sizeof(HeapHeader) + unStrLen + sizeof(HeapMid) + sizeof(HeapTail)));
	ASSERT(ptr != nullptr);
	if (ptr != nullptr) {
		pMyFile = (char *)ptr + sizeof(HeapHeader);
		*pMyFile = 0;
		AsciiStrCpy(pMyFile, pFileName);
		ptr->size = size;
		ptr->file_line = nLine;
		pMid = (HeapMid *) ((char *)ptr + sizeof(HeapHeader) + unStrLen);
		pMid->mid_offset = sizeof(HeapHeader) + unStrLen;
		pMid->dumy = 0x6c6c6c6c;
		pTail = (HeapTail *) ((char *)pMid + sizeof(HeapMid) + size);
		pTail->dumy = 0x6c6c6c6c;
		InsertTailList(&gMemListHead, &ptr->list);
		return (char *)ptr + sizeof(HeapHeader) + unStrLen + sizeof(HeapMid);
	}

	FreeLeftMemPools();
	gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(c16Resean), c16Resean);

	return nullptr;
#endif
	static CHAR16 c16Resean[] = L"C++ runtime error: Out of memory";
	void *ptr = MemCheck_AllocatePool(size, MEMCHECK_FLAG_NONE, pFileName, nLine);
	if (ptr != nullptr) {
		return ptr;
	}
	gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(c16Resean), c16Resean);
	return nullptr;
}

void operator delete(void *ptr)
{
#if 0
	HeapHeader *pHead = nullptr;
	HeapMid *pMid = nullptr;
	HeapTail *pTail = nullptr;
	if (ptr != nullptr) {
		pMid = (HeapMid *)((char *)ptr - sizeof(HeapMid));
		if (pMid->dumy != 0x6c6c6c6c) {
			DEBUG((EFI_D_ERROR, "Memory Overlap ~~!!\n"));
			ASSERT(pMid->dumy == 0x6c6c6c6c);
		}
		pHead = (HeapHeader *) ((char *)pMid - pMid->mid_offset);
		pTail = (HeapTail *)((char *)ptr + pHead->size);
		if (pTail->dumy != 0x6c6c6c6c) {
			DEBUG((EFI_D_ERROR, "%a(%d):Memory Overlap\n", (char *)pHead + sizeof(HeapHeader), pHead->file_line));
			ASSERT(pTail->dumy == 0x6c6c6c6c);
		}

		RemoveEntryList(&pHead->list);
		FreePool(pHead);
	}
#endif
	if (ptr == nullptr) {
		return;
	}
	MemCheck_FreePool(ptr, __FILE__, __LINE__);
	return;
}

void operator delete[](void *ptr)
{
#if 0
	HeapHeader *pHead = nullptr;
	HeapMid *pMid = nullptr;
	HeapTail *pTail = nullptr;
	if (ptr != nullptr) {
		pMid = (HeapMid *)((char *)ptr - sizeof(HeapMid));
		if (pMid->dumy != 0x6c6c6c6c) {
			DEBUG((EFI_D_ERROR, "Memory Overlap ~~!!\n"));
			ASSERT(pMid->dumy == 0x6c6c6c6c);
		}
		pHead = (HeapHeader *) ((char *)pMid - pMid->mid_offset);
		pTail = (HeapTail *)((char *)ptr + pHead->size);
		if (pTail->dumy != 0x6c6c6c6c) {
			DEBUG((EFI_D_ERROR, "%a(%d):Memory Overlap\n", (char *)pHead + sizeof(HeapHeader), pHead->file_line));
			ASSERT(pTail->dumy == 0x6c6c6c6c);
		}

		RemoveEntryList(&pHead->list);
		FreePool(pHead);
	}
#endif
	if (ptr == nullptr) {
		return;
	}
	MemCheck_FreePool(ptr, __FILE__, __LINE__);
	return;
}

#endif


extern "C" int _purecall(void)
{
	static CHAR16 c16Resean[] = L"C++ runtime error: Pure Call";

	//FreeLeftMemPools();
	gBS->Exit(gImageHandle, EFI_ABORTED, sizeof(c16Resean), c16Resean);
	return 0;
}

#if 0
extern "C" void *memset(void *s,unsigned char c,size_t n)
{
	SetMem(s, (UINTN)n, (UINT8)c);
	return s;
}
#endif


//==================================================================
//User defined object initialize and destroy support

__declspec(allocate(".mine$a")) const PF InitSegStart = (PF)1;
__declspec(allocate(".mine$z")) const PF InitSegEnd = (PF)1;
// The comparison for 0 is important.
// For now, each section is 256 bytes. When they
// are merged, they are padded with zeros. You
// can't depend on the section being 256 bytes, but
// you can depend on it being padded with zeros.
void initial_objs_cpp() {
	const PF *x = &InitSegStart;
	for (++x; x < &InitSegEnd; ++x)
		if (*x) (*x)();
}

typedef struct {
	LIST_ENTRY link;
	PF pf;
} atexit_cpp_node;

static LIST_ENTRY g_atexit_list = INITIALIZE_LIST_HEAD_VARIABLE(g_atexit_list);

/** The atexit function registers the function pointed to by func, to be
    called without arguments at normal program termination.

    The implementation shall support the registration of
    at least 32 functions.

    @return   The atexit function returns zero if the registration succeeds,
              nonzero if it fails.
**/
int atexit_cpp(PF pf) {
	atexit_cpp_node *pNode = (atexit_cpp_node *) AllocatePool(sizeof(atexit_cpp_node));
	if (pNode == nullptr) {
		return 1;
	}
	pNode->pf = pf;
	//Insert to header
	InsertHeadList(&g_atexit_list, &pNode->link);
	return 0;
}

void destroy_objs_cpp() {
	while (IsListEmpty(&g_atexit_list) == FALSE) {
		atexit_cpp_node *pNode = BASE_CR(GetFirstNode(&g_atexit_list), atexit_cpp_node, link);
		RemoveEntryList(&pNode->link);
		pNode->pf();
		FreePool(pNode);
	}
}

/*
 * Copyright (c) 2016 by Nero Liu .  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 *
 */

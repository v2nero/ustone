//#include <cpp/uefi.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cpp/cppbase.hpp>
#include <utils/MemLeakCheck.h>

void test_Allocate()
{
	char *p1 = (char*)AllocatePool(10);
	*p1 = 0;
	char *p2 =(char*) AllocatePool(10);
	memset(p2, 0, 11);
	FreePool(p2);
	void *p3 = AllocatePool(10);
	p3 = ReallocatePool(10, 15, p3);
	FreePool(p3);

	char *p4 = (char*)AllocatePool(10);
	*p4 = '1';
	char *p5 = (char *)AllocateCopyPool(13, p4);
	if (*p5 != '1') {
		printf("%d\n", __LINE__);
	}
	FreePool(p4);
	FreePool(p5);

	void *p6 = AllocatePool_Direct(10);
	FreePool(p6);

	char *p7 = (char *)AllocateZeroPool(1);
	if (*p7 != 0) {
		printf("%d\n", __LINE__);
	}
	FreePool(p7);
	char *p8 = (char *)AllocateZeroPool_Direct(1);
	if (*p8 != 0) {
		printf("%d\n", __LINE__);
	}
	FreePool(p8);
}

void test_malloc()
{
	char *p1 = (char*)malloc(10);
	*p1 = 0;
	char *p2 = (char*)malloc(10);
	memset(p2, 0, 11);
	free(p2);
	void *p3 = malloc(10);
	p3 = realloc(p3, 15);
	free(p3);

	char *p4 = (char*)calloc(10, 20);
	memset(p4, 0, 10*20);
	free(p4);

	char *p5 = (char*)malloc_direct(10);
	free(p5);
	void *p6 = AllocatePool_Direct(10);
	free(p6);
}

extern "C"
int main()
{
	test_Allocate();
	test_malloc();
}

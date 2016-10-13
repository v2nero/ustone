//#include <cpp/uefi.hpp>
#include <cpp/cppbase.hpp>
#include <utils/LightRefBase.hpp>
#include <utils/StrongRefBase.hpp>
#include <stdlib.h>

using namespace ustone;

class TestClass : public LightRefBase<TestClass>
{

public:
	TestClass() {
		Print((CHAR16*)L"%a\n", __FUNCTION__);
	}

	virtual ~TestClass() {
		Print((CHAR16*)L"%a\n", __FUNCTION__);
	}

};

class TestClassB : public TestClass
{
public:
	TestClassB() {
		Print((CHAR16*)L"%a\n", __FUNCTION__);
	}

	~TestClassB() {
		Print((CHAR16*)L"%a\n", __FUNCTION__);
	}
};


class TestClassL : public StrongRefBase
{
public:
	TestClassL() {
		Print((CHAR16*)L"%a\n", __FUNCTION__);
	}

	~TestClassL() {
		Print((CHAR16*)L"%a\n", __FUNCTION__);
	}

	void echo() {
		Print((CHAR16*)L"I am TestClassL\n");
	}
};

class TestClassM : public TestClassL
{
public:
	TestClassM() {
		Print((CHAR16*)L"%a\n", __FUNCTION__);
	}

	~TestClassM() {
		Print((CHAR16*)L"%a\n", __FUNCTION__);
	}
};

extern "C"
int main()
{
	//sp<TestClass> a(new TestClass());
	sp<TestClassB> b(NEW TestClassB());
	sp<TestClassM> m(NEW TestClassM());

//#undef new

	void *p = malloc(sizeof(TestClassM));
	TestClassM *pT = PLACEMENT_NEW(p)TestClassM();
	pT->~TestClassM();
	free(p);

	m->echo();
}

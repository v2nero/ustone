/*
 * test_string.cpp
 *
 *  Created on: 2016Äê9ÔÂ17ÈÕ
 *      Author: v2nero
 */


#include <stdio.h>
#include <string>
using namespace std;

void test_string()
{
	string str = "112222";
	printf("str=%s\n", str.c_str());

	wstring wstr = L"1122";
	wprintf(L"wstr=%S\n", wstr);

	str += "addtion1";
	printf("str=%s\n", str.c_str());

	wstr += L"waddtion";
	wprintf(L"wstr=%S\n", wstr);
}

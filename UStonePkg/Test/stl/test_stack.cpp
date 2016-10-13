/*
 * test_stack.cpp
 *
 *  Created on: 2016Äê9ÔÂ17ÈÕ
 *      Author: v2nero
 */


#include <stdio.h>
#include <list>
#include <algorithm>
#include <stack>
using namespace std;

void test_stack()
{
	printf("%s\n", __FUNCTION__);
	stack<int> istack;
	istack.push(1);
	istack.push(3);
	istack.push(5);
	istack.push(7);

	printf("size = %d top = %d\n", istack.size(), istack.top());

	istack.pop();
	printf("%d ", istack.top());

	istack.pop();
	printf("%d ", istack.top());

	istack.pop();
	printf("%d \n", istack.top());
	printf("size = %d\n", istack.size());
}

void test_stack_list()
{
	printf("%s\n", __FUNCTION__);
	stack<int, list<int>> istack;
	istack.push(1);
	istack.push(3);
	istack.push(5);
	istack.push(7);

	printf("size = %d top = %d\n", istack.size(), istack.top());

	istack.pop();
	printf("%d ", istack.top());

	istack.pop();
	printf("%d ", istack.top());

	istack.pop();
	printf("%d\n", istack.top());
	printf("size = %d\n", istack.size());
}

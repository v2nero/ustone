/*
 * test_queue.cpp
 *
 *  Created on: 2016Äê9ÔÂ17ÈÕ
 *      Author: v2nero
 */

#include <stdio.h>
#include <queue>
#include <list>
#include <algorithm>

using namespace std;


void test_queue()
{
	printf("%s\n", __FUNCTION__);

	queue<int, list<int>> iqueue;

	iqueue.push(1);
	iqueue.push(3);
	iqueue.push(5);
	iqueue.push(7);

	printf("size = %d\n", iqueue.size());
	printf("front = %d\n", iqueue.front());

	iqueue.pop(); printf("front = %d\n", iqueue.front());
	iqueue.pop(); printf("front = %d\n", iqueue.front());
	iqueue.pop(); printf("front = %d\n", iqueue.front());
	printf("size = %d\n", iqueue.size());
}

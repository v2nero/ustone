#include <stdio.h>
#include <list>
#include <algorithm>
#include <stack>
using namespace std;

void test_list()
{
	printf("%s\n", __FUNCTION__);
	list<int> ilist;

	printf("size=%d\n", ilist.size());

	ilist.push_back(0);
	ilist.push_back(1);
	ilist.push_back(2);
	ilist.push_back(3);
	ilist.push_back(4);
	printf("size=%d\n", ilist.size());

	list<int>::iterator it;
	for (it = ilist.begin(); it != ilist.end(); ++it) {
		printf("%d ", *it);
	}
	printf("\n");

	it = find(ilist.begin(), ilist.end(), 3);
	if (it != ilist.end()) {
		ilist.insert(it, 99);
	}

	printf("size=%d\n", ilist.size());
	for (it = ilist.begin(); it != ilist.end(); ++it) {
		printf("%d ", *it);
	}
	printf("\n");

	it = find(ilist.begin(), ilist.end(), 1);
	if (it != ilist.end()) {
		it = ilist.erase(it);
		printf("%d\n", *it);
	}

	for (it = ilist.begin(); it != ilist.end(); ++it) {
		printf("%d ", *it);
	}
	printf("\n");
}

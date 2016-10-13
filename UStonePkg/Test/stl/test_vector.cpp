#include <stdio.h>
#include <algorithm>
#include <vector>

using namespace std;

void test_vector()
{
	size_t i;
	vector<int> iv(2, 9);
	printf("size=%d capacity=%d\n", iv.size(), iv.capacity());
	iv.push_back(1);
	iv.push_back(2);
	iv.push_back(3);
	iv.push_back(4);

	for (i = 0; i < iv.size(); ++i) {
		printf("%d ", iv[i]);
	}
	printf("\n");

	iv.push_back(5);
	for (i = 0; i < iv.size(); ++i) {
		printf("%d ", iv[i]);
	}
	printf("\n");
	printf("size=%d capacity=%d\n", iv.size(), iv.capacity());

	iv.pop_back();
	iv.pop_back();

	for (i = 0; i < iv.size(); ++i) {
		printf("%d ", iv[i]);
	}
	printf("\n");
	printf("size=%d capacity=%d\n", iv.size(), iv.capacity());

	iv.pop_back();

	vector<int>::iterator ivite = find(iv.begin(), iv.end(), 1);
	if (ivite != iv.end()) iv.erase(ivite);

	for (i = 0; i < iv.size(); ++i) {
		printf("%d ", iv[i]);
	}
	printf("\n");
	printf("size=%d capacity=%d\n", iv.size(), iv.capacity());

	ivite = find(iv.begin(), iv.end(), 2);
	if (ivite != iv.end()) iv.insert(ivite, 3, 7);

	for (i = 0; i < iv.size(); ++i) {
		printf("%d ", iv[i]);
	}
	printf("\n");
	printf("size=%d capacity=%d\n", iv.size(), iv.capacity());

	iv.clear();
	printf("size=%d capacity=%d\n", iv.size(), iv.capacity());
}

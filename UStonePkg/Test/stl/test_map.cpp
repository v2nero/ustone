/*
 * test_map.cpp
 *
 *  Created on: 2016Äê9ÔÂ17ÈÕ
 *      Author: v2nero
 */

#include <stdio.h>
#include <map>

using namespace std;

void test_map()
{
	map<int, int> imap;

	imap[3] = 4;
	imap[9] = 10;
	imap[1] = 100;

	map<int, int>::iterator it;
	for (it = imap.begin(); it != imap.end(); ++it) {
		printf("imap[%d] = %d\n", (*it).first, (*it).second);
	}

}

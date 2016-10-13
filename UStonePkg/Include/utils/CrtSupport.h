#ifndef	__CRT_SUPPORT_H__
#define	__CRT_SUPPORT_H__

//return 0 for success, otherwise, fail
int global_crt_init();

//this function is not quit required.
//All destructors will be invoked by atexit() function.
void global_crt_uninit();

#endif

/*
 * Copyright (c) 2016 by Nero Liu .  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 *
 */

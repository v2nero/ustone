#pragma once

#include "StrongPointer.hpp"

class StrongRefBase
{
public:
	inline StrongRefBase() : mCount(0) { }
	inline void incStrong(const void* id) const {
		++mCount;
	}
	inline void decStrong(const void* id) const {
		if ((mCount--) == 1) {
			delete this;
		}
	}
	//! DEBUGGING ONLY: Get current strong ref count.
	inline int getStrongCount() const {
		return mCount;
	}

protected:
	virtual ~StrongRefBase() { }

private:
	mutable volatile int mCount;
};

/*
 * Copyright (c) 2016 by Nero Liu .  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 *
 */

#ifndef USTONE_LIGHT_REFBASE_H
#define USTONE_LIGHT_REFBASE_H

#include "StrongPointer.hpp"

// ---------------------------------------------------------------------------
namespace ustone {

// ---------------------------------------------------------------------------
class ReferenceMover;
class ReferenceConverterBase {
public:
	virtual size_t getReferenceTypeSize() const = 0;
	virtual void* getReferenceBase(void const*) const = 0;
	inline virtual ~ReferenceConverterBase() { }
};

// ---------------------------------------------------------------------------

template <class T>
class LightRefBase
{
public:
	inline LightRefBase() : mCount(0) { }
	inline void incStrong(const void* id) const {
		++mCount;
	}
	inline void decStrong(const void* id) const {
		if ((mCount--) == 1) {
			delete static_cast<const T*>(this);
		}
	}
	//! DEBUGGING ONLY: Get current strong ref count.
	inline int getStrongCount() const {
		return mCount;
	}

	typedef LightRefBase<T> basetype;

protected:
	inline ~LightRefBase() { }

private:
	friend class ReferenceMover;
	inline static void moveReferences(void* d, void const* s, size_t n,
	    const ReferenceConverterBase& caster) { }

private:
	mutable volatile int mCount;
};

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// this class just serves as a namespace so TYPE::moveReferences can stay
// private.

class ReferenceMover {
	// StrongReferenceCast and WeakReferenceCast do the impedance matching
	// between the generic (void*) implementation in Refbase and the strongly typed
	// template specializations below.

	template <typename TYPE>
	struct StrongReferenceCast : public ReferenceConverterBase {
		virtual size_t getReferenceTypeSize() const { return sizeof( sp<TYPE> ); }
		virtual void* getReferenceBase(void const* p) const {
			sp<TYPE> const* sptr(reinterpret_cast<sp<TYPE> const*>(p));
			return static_cast<typename TYPE::basetype *>(sptr->get());
		}
	};

public:
	template<typename TYPE> static inline
	void move_references(sp<TYPE>* d, sp<TYPE> const* s, size_t n) {
		memmove(d, s, n*sizeof(sp<TYPE>));
		StrongReferenceCast<TYPE> caster;
		TYPE::moveReferences(d, s, n, caster);
	}
};

// specialization for moving sp<> types.
// these are used by the [Sorted|Keyed]Vector<> implementations
// sp<> need to be handled specially, because they do not
// have trivial copy operation in the general case (see RefBase.cpp
// when DEBUG ops are enabled), but can be implemented very
// efficiently in most cases.

template<typename TYPE> inline
void move_forward_type(sp<TYPE>* d, sp<TYPE> const* s, size_t n) {
	ReferenceMover::move_references(d, s, n);
}

template<typename TYPE> inline
void move_backward_type(sp<TYPE>* d, sp<TYPE> const* s, size_t n) {
	ReferenceMover::move_references(d, s, n);
}

}; // namespace ustone

// ---------------------------------------------------------------------------

#endif // USTONE_LIGHT_REFBASE_H

/*
 * Copyright (c) 2016 by Nero Liu .  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 *
 */

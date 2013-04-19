/*
 * NSMRef.h
 *
 *  Created on: Mar 4, 2013
 *      Author: maierm
 */

#ifndef NSMREF_H_
#define NSMREF_H_

#include "NonSequentialMemoryAccessor.h"

namespace dash
{

template<typename T>
class NSMRef
{

public:
	NSMRef(NonSequentialMemoryAccessor<T> acc) :
			m_acc(acc)
	{
	}

	virtual ~NSMRef()
	{
	}

	operator T() const
	{
		T t;
		m_acc.get_value(&t);
		return t;
	}

	NSMRef<T>& operator=(const T i)
	{
		m_acc.put_value(i);
		return *this;
	}

	NSMRef<T>& operator=(const NSMRef<T>& ref)
	{
		return *this = T(ref);
	}

	NonSequentialMemoryAccessor<T> get_accessor() const
	{
		return m_acc;
	}

private:
	NonSequentialMemoryAccessor<T> m_acc;

};

}

#endif /* NSMREF_H_ */

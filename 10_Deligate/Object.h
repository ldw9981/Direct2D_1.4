#pragma once
#include "ObjectHandleTable.h"

class Object
{
	ObjectHandle m_handle;
public:
	Object()
	{
		m_handle = ObjectHandleTable::Instance().CreateHandle(this);
	}
	virtual ~Object()
	{
		ObjectHandleTable::Instance().DestroyHandle(m_handle);
	}
	ObjectHandle GetHandle() { return m_handle; }
};


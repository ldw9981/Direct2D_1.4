#pragma once
#include "ObjectHandleTable.h"
#include "ObjectTable.h"

class Object
{
	
public:
	Object()
	{		
		ObjectTable::Instance().Register(this); // ���̺� ���
	}
	virtual ~Object()
	{
		ObjectTable::Instance().Unregister(this); // ���̺��� ����
	}
	
};


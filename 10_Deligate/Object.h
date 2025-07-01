#pragma once
#include "ObjectHandleTable.h"
#include "ObjectTable.h"

class Object
{
	
public:
	Object()
	{		
		ObjectTable::Instance().Register(this); // 테이블에 등록
	}
	virtual ~Object()
	{
		ObjectTable::Instance().Unregister(this); // 테이블에서 제거
	}
	
};


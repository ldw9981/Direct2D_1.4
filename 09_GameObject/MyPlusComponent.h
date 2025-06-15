#pragma once
#include "Component.h"
#include <iostream>
#include "MySystem.h"
#include "MyComponent.h"


class MyPlusComponent :
	public MyComponent
{
public:
	MyPlusComponent()
		:MyComponent(1)
	{
		std::cout << this << " " << __FUNCTION__ << "\n";
	}
	~MyPlusComponent() override
	{
		std::cout << this << " " << __FUNCTION__ << "\n";
	}

	void DoSomeThing() override
	{
		std::cout << this << " " << __FUNCTION__  << "\n";
	}
};


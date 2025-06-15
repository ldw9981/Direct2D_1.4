#pragma once
#include "Component.h"
#include <iostream>
#include "MySystem.h"
#include "MyComponent.h"
#include "GameObject.h"

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
	
	MyComponent* Other=nullptr;

	void DoSomeThing() override
	{
		std::cout << this << " " << __FUNCTION__  << "\n";
	}

	void OnEnable() override
	{
		std::cout << this << " " << __FUNCTION__ << "\n";
		Other = owner->GetComponent<MyComponent>();
		//추가 순서에 따라 없을수도 있다.
	};
	void OnDestroy() override
	{
		std::cout << this << " " << __FUNCTION__ << "\n";
	};
};


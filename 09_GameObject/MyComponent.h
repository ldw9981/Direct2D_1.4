#pragma once
#include "Component.h"
#include <iostream>
#include "MySystem.h"

class IMyComponent
{
public:
    virtual void DoSomeThing() = 0;
};

class MyComponent :
    public Component , public IMyComponent
{
public:
    MyComponent(int value)
        :m_Value(value)
    {
        MySystem::Instance->Regist(this);
    }
    ~MyComponent() override
    {
        std::cout << __FUNCTION__  << "\n";
        MySystem::Instance->Unregist(this);
    }

    int m_Value;


    void DoSomeThing() override
    {
        std::cout << __FUNCTION__ << m_Value << "\n";
    }
};


#pragma once
#include "Component.h"
#include <iostream>
#include "MySystem.h"



class MyComponent :
    public Component
{
public:
    MyComponent(int value)
        :m_Value(value)
    {
        std::cout << this << " " << __FUNCTION__ << "\n";
        MySystem::Instance->Regist(this);
    }
    ~MyComponent() override
    {
        std::cout << this << " " << __FUNCTION__  << "\n";
        MySystem::Instance->Unregist(this);
    }

    int m_Value;


    virtual void DoSomeThing()
    {
        std::cout << this << " " << __FUNCTION__ << m_Value << "\n";
    }
};


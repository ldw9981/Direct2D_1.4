#pragma once
#include "Component.h"
#include <iostream>

class MyComponent :
    public Component
{
public:
    MyComponent(int value)
        :m_Value(value)
    {

    }
    ~MyComponent() override
    {
        std::cout << __FUNCTION__  << "\n";
    }

    int m_Value;

};


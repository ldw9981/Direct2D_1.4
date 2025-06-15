#pragma once

#include <iostream>

class GameObject;
class Component
{
public:
    virtual ~Component() = default;
    GameObject* owner = nullptr;

    virtual void OnEnable() = 0;
    virtual void OnDestroy() = 0;
};


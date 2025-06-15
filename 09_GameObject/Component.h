#pragma once

class GameObject;
class IComponent
{
public:
};
class Component
{
public:
    virtual ~Component() = default;
    GameObject* owner = nullptr;
};


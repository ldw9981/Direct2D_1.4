#pragma once
#include <vector>
#include "Component.h"

class Component;
class GameObject {
private:
    std::vector<Component*> components;

public:
    ~GameObject() 
    {
		for (Component* comp : components) {
			comp->OnDestroy();
			delete comp;
		}
		components.clear();
    }

    // Class T를 생성하는 함수 , 인자까지 전달한다.
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) 
    {
        // 컴파일 시점에 T가 Component를 상속받은 클래스 인지 확인
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        // 인자까지 전달하면서 생성
        T* comp = new T(std::forward<Args>(args)...);
        comp->owner = this;
        components.push_back(comp);
        comp->OnEnable();
        return comp;
    }

    template<typename T>
    std::vector<T*> GetComponents() {
        std::vector<T*> result;
        for (Component* comp : components) {
            // 실행도중(Runtime)에 comp가 가르키는 인스턴스가 RTTI정보 이용하여 
            // T이거나 T의 자식 클래스 이면 주소를 리턴한다. 클래스가 가상 함수를 하나라도 가지면,
			// 컴파일러는 해당 클래스에 대해 vtable + RTTI(Run-Time Type Information, 런타임 타입 정보)
            // 정보 블록을 생성합니다.
            if (auto casted = dynamic_cast<T*>(comp))
                result.push_back(casted);
        }
        return result;
    }

	template<typename T>
	T* GetComponent() {
		for (Component* comp : components) {
			if (typeid(*comp) == typeid(T)) //완전히 동일한 타입만
				return static_cast<T*>(comp); //안전하게 static_cast
		}
		return nullptr;
	}

    template<typename T>
    bool RemoveComponent(T* target) {
        
        for (auto it = components.begin(); it != components.end(); ++it) {
            if (*it == target) 
            {
				(*it)->OnDestroy();
				delete* it;
                components.erase(it);
                return true;
            }
        }
        return false;
    }
};
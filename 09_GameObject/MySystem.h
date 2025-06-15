#pragma once
#include <vector>


class MyComponent;
class MySystem
{
public:
	static MySystem* Instance;
	MySystem()
	{
		MySystem::Instance = this;
	}

	std::vector<MyComponent*> m_Components;	

	void Update();

	void Regist(MyComponent* comp)
	{
		m_Components.push_back(comp);
	}
	void Unregist(MyComponent* comp)
	{
		for (auto it = m_Components.begin(); it != m_Components.end(); ++it) {
			if (*it == comp) {				
				m_Components.erase(it);
				return;
			}
		}
	}
};


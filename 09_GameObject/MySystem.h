#pragma once
#include <vector>

class IMyComponent;
class MyComponent;
class MySystem
{
public:
	static MySystem* Instance;
	MySystem()
	{
		MySystem::Instance = this;
	}

	std::vector<IMyComponent*> m_Components;	

	void Update();

	void Regist(IMyComponent* comp)
	{
		m_Components.push_back(comp);
	}
	void Unregist(IMyComponent* comp)
	{
		for (auto it = m_Components.begin(); it != m_Components.end(); ++it) {
			if (*it == comp) {				
				m_Components.erase(it);
				return;
			}
		}
	}
};


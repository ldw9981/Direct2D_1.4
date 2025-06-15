
#include "MyComponent.h"
#include "MySystem.h"

MySystem* MySystem::Instance;

void MySystem::Update()
{
	for (auto it = m_Components.begin(); it != m_Components.end(); ++it) 
	{
		(*it)->DoSomeThing();
	}
}

void MySystem::Regist(MyComponent* comp)
{
	m_Components.push_back(comp);
}

void MySystem::Unregist(MyComponent* comp)
{
	for (auto it = m_Components.begin(); it != m_Components.end(); ++it) {
		if (*it == comp) {
			m_Components.erase(it);
			return;
		}
	}
}


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

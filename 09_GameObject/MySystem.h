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

	void Regist(MyComponent* comp);
	void Unregist(MyComponent* comp);
};


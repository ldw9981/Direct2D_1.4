#pragma once


class IRender
{
public:
	virtual void Render()=0;
};

class RenderSystem
{
public:
	std::vector<IRender*> m_Components;
	

};


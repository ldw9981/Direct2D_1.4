#include "Transform.h"

void Transform::AddTranslation(float x, float y)
{
	Dirty = true;
	Translation.x += x;
	Translation.y += y;
}

void Transform::AddRotation(float degree)
{
	Dirty = true;
	Rotation += degree;
}

void Transform::AddScale(float x,float y)
{
	Dirty = true;
	Scale.x += x;
	Scale.y += y;
}

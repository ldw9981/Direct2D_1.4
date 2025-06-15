#include "Transform.h"
#include <assert.h>

void Transform::SetParent(Transform* Target)
{
	if (Parent == Target) // 이미 같으면 리턴
		return;

	if (Parent != nullptr) // 이미 부모가 있으면 부모에서 제거
		Parent->RemoveChild(this);		
	
	Parent = Target; // 부모가 바뀌엇다.
	
	if (Parent != nullptr )
		Parent->AddChild(this);	//부모가 있으면 추가

	// 부모가 없어지거나 새로 생겼으니 자식에게 World바꾸라고 전파
	MarkWorldDirty();
}

void Transform::AddTranslation(float x, float y)
{
	LocalDirty = true;
	Translation.x += x;
	Translation.y += y;

	MarkWorldDirty();
}

void Transform::AddRotation(float degree)
{
	LocalDirty = true;
	Rotation += degree;
	
	MarkWorldDirty();
}

void Transform::AddScale(float x,float y)
{
	LocalDirty = true;
	Scale.x += x;
	Scale.y += y;
	
	MarkWorldDirty();
}

void Transform::MakeLocalMatrix()
{
	float s = sinf(Rotation * DEG2RAD);
	float c = cosf(Rotation * DEG2RAD);

	CachedLocal = D2D1::Matrix3x2F(
		c * Scale.x, -s * Scale.y,
		s * Scale.x, c * Scale.y,
		Translation.x, Translation.y
	);
}

void Transform::AddChild(Transform* target)
{
	Children.push_back(target);
}

void Transform::RemoveChild(Transform* target)
{
	for (auto it = Children.begin(); it != Children.end(); ++it)
	{
		if (*it != target)
			continue;

		it = Children.erase(it);
		break;
	}
}

void Transform::MarkWorldDirty()
{
	WorldDirty = true;
	for (auto it = Children.begin(); it != Children.end(); ++it)
	{
		(*it)->MarkWorldDirty();
	}
}

#include "Transform.h"
#include <assert.h>



void Transform::SetTranslation(float x, float y)
{
	Dirty = true;
	Translation.x = x; Translation.y = y;
	MarkDirty();
}

void Transform::SetRotation(float InRotation)
{
	Dirty = true;  Rotation = InRotation;
	MarkDirty();
}

void Transform::SetScale(float scaleX, float scaleY)
{
	Dirty = true;  Scale.x = scaleX; Scale.y = scaleY;
	MarkDirty();
}

// 복사없이 참조로 리턴하면서 수정은 불가
const D2D1::Matrix3x2F& Transform::GetWorldMatrix()
{
	if (Dirty )
	{
		D2D1::Matrix3x2F LocalMatrix = MakeLocalMatrix(); // 로컬행렬 계산
		CachedWorld = Parent ? LocalMatrix * Parent->GetWorldMatrix() : LocalMatrix;
		Dirty = false;
	}
	return CachedWorld;
}

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
	MarkDirty();
}

void Transform::Reset()
{
	Scale = { 1.0f, 1.0f };
	Rotation = { 0.0f };
	Translation = { 0.0f, 0.0f };
	Dirty = true;
	MarkDirty();
}

void Transform::AddTranslation(float x, float y)
{
	Dirty = true;
	Translation.x += x;
	Translation.y += y;
	MarkDirty();
}

void Transform::AddRotation(float degree)
{
	Dirty = true;
	Rotation += degree;
	
	MarkDirty();
}

void Transform::AddScale(float x,float y)
{
	Dirty = true;
	Scale.x += x;
	Scale.y += y;
	
	MarkDirty();
}

D2D1::Matrix3x2F Transform::MakeLocalMatrix()
{
	float s = sinf(Rotation * DEG2RAD);
	float c = cosf(Rotation * DEG2RAD);

	return D2D1::Matrix3x2F(
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

void Transform::MarkDirty()
{
	Dirty = true;
	for (auto it = Children.begin(); it != Children.end(); ++it)
	{
		(*it)->MarkDirty();
	}
}

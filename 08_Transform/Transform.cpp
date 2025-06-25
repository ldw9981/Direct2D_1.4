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

// ������� ������ �����ϸ鼭 ������ �Ұ�
const D2D1::Matrix3x2F& Transform::GetWorldMatrix()
{
	if (Dirty )
	{
		D2D1::Matrix3x2F LocalMatrix = MakeLocalMatrix(); // ������� ���
		CachedWorld = Parent ? LocalMatrix * Parent->GetWorldMatrix() : LocalMatrix;
		Dirty = false;
	}
	return CachedWorld;
}

void Transform::SetParent(Transform* Target)
{
	if (Parent == Target) // �̹� ������ ����
		return;

	if (Parent != nullptr) // �̹� �θ� ������ �θ𿡼� ����
		Parent->RemoveChild(this);		
	
	Parent = Target; // �θ� �ٲ����.
	
	if (Parent != nullptr )
		Parent->AddChild(this);	//�θ� ������ �߰�

	// �θ� �������ų� ���� �������� �ڽĿ��� World�ٲٶ�� ����
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

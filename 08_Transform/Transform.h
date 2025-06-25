#pragma once
#include <d2d1_1.h>
#include <math.h>
#include <vector>

constexpr float DEG2RAD = 3.14159265f / 180.0f;
constexpr float RAD2DEG = 180.0f / 3.14159265f;

struct Transform
{
public:
    D2D1_VECTOR_2F Translation;  // 위치
    float Rotation;     // 회전 (degree)    
    D2D1_VECTOR_2F Scale;  // 스케일 
	bool Dirty = true; // 월드행렬을 변경해야하는지
   
    D2D1::Matrix3x2F CachedWorld;
    Transform* Parent = nullptr;
    std::vector<Transform*> Children;

    Transform()
        : Translation{ 0.0f, 0.0f }, Rotation(0.0f), Scale{ 1.0f, 1.0f }
    {    
        CachedWorld = D2D1::Matrix3x2F::Identity();
    }
    D2D1_VECTOR_2F GetTranslation() { return Translation; }
    void SetTranslation(float x, float y);
    float GetRotation() {   return Rotation; }
    void  SetRotation(float InRotation);
    D2D1_VECTOR_2F GetScale() {     return Scale; }
    void SetScale(float scaleX, float scaleY);

    // 복사없이 참조로 리턴하면서 수정은 불가
    const D2D1::Matrix3x2F& GetWorldMatrix();
    void SetParent(Transform* parent);
    void Reset();

    void AddTranslation(float x, float y);
    void AddRotation(float degree);
    void AddScale(float x, float y);

    D2D1::Matrix3x2F MakeLocalMatrix();
    void AddChild(Transform* target);
    void RemoveChild(Transform* target);
    void MarkDirty();
};



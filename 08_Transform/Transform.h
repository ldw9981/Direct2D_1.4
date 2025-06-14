#pragma once
#include <d2d1_1.h>

struct Transform
{
public:
    D2D1_VECTOR_2F Translation;  // 위치
    float Rotation;     // 회전 (degree)    
    D2D1_VECTOR_2F Scale;  // 스케일 
    bool Dirty = true;
    D2D1::Matrix3x2F Cached;
    Transform* Parent = nullptr;
    Transform()
        : Translation{ 0.0f, 0.0f }, Rotation(0.0f), Scale{ 1.0f, 1.0f }
    {
        Cached = D2D1::Matrix3x2F::Identity();
    }
    D2D1_VECTOR_2F GetTranslation() { return Translation; }
    void SetTranslation(float x, float y) { Dirty = true;  Translation.x = x; Translation.y = y; }
    float GetRotation() {   return Rotation; }
    void  SetRotation(float InRotation) { Dirty = true;  Rotation = InRotation; }
    D2D1_VECTOR_2F GetScale() {     return Scale; }
    void SetScale(float scaleX, float scaleY) { Dirty = true;  Scale.x = scaleX; Scale.y = scaleY; }

    // 행렬로 변환
    D2D1::Matrix3x2F GetLocalMatrix()
    {
        if (Dirty) 
        {           
            Cached = D2D1::Matrix3x2F::Scale(Scale.x, Scale.y) *
                D2D1::Matrix3x2F::Rotation(Rotation) *
                D2D1::Matrix3x2F::Translation(Translation.x, Translation.y);          
            Dirty = false;
        }            
        return Cached;
    }
    D2D1::Matrix3x2F GetWorldMatrix()
    {
        auto result = GetLocalMatrix();
        result = Parent ? result * Parent->GetWorldMatrix() : result;
        return result;
    }
    void Reset()
    {
        Scale = { 1.0f, 1.0f };
        Rotation = { 0.0f };
        Translation = { 0.0f, 0.0f };
        Dirty = true;
    }

    void AddTranslation(float x, float y);
    void AddRotation(float degree);
    void AddScale(float x, float y);
};



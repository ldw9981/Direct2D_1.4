#pragma once
#include <d2d1_1.h>

struct Transform
{
public:
    D2D1_VECTOR_2F Translation;  // 위치
    float Rotation;     // 회전 (degree)    
    D2D1_VECTOR_2F Scale;  // 스케일

    // 기본 생성자: Identity Transform
    Transform()
        : Translation{ 0.0f, 0.0f }, Rotation(0.0f), Scale{ 1.0f, 1.0f }
    {
    }
    D2D1_VECTOR_2F GetTranslation() const { return Translation; }
    void SetTranslation(float x, float y) { Translation.x = x; Translation.y = y; }
    float GetRotation() const { return Rotation; }
    void  SetRotation(float InRotation) { Rotation = InRotation; }
    D2D1_VECTOR_2F GetScale() const { return Scale; }
    void        SetScale(float scaleX, float scaleY) { Scale.x = scaleX; Scale.y = scaleY; }

    // 행렬로 변환
    D2D1::Matrix3x2F ToMatrix()
    {
        return D2D1::Matrix3x2F::Scale(Scale.x, Scale.y) *
            D2D1::Matrix3x2F::Rotation(Rotation) *
            D2D1::Matrix3x2F::Translation(Translation.x, Translation.y);
    }
    void Reset()
    {
        SetScale(1.0f, 1.0f);
        SetRotation(0.0f);
        SetTranslation(0.0f, 0.0f);
    }
};



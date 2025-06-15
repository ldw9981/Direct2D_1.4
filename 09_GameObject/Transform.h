#pragma once
struct Transform
{
    D2D1_VECTOR_2F Translation;  // 위치
    float Rotation;     // 회전 (degree)    
    D2D1_VECTOR_2F Scale;  // 스케일 
    bool LocalDirty = true;
    D2D1::Matrix3x2F CachedLocal;
    Transform* Parent = nullptr;
};


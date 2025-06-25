#pragma once

#include <wrl.h>  // ComPtr 사용을 위한 헤더
#include <d2d1_3.h> //ID2D1Factory8,ID2D1DeviceContext7

class BitmapResource
{
public:
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_Bitmap;
};


#pragma once

#include <wrl.h>  // ComPtr 사용을 위한 헤더
#include <unordered_map>
#include <string>
#include <d2d1_3.h>

#include "AnimationClip.h"


#include <memory>
#include "BitmapResource.h"


using ResourceKey = std::string;
using namespace Microsoft::WRL;

class ResourceManager
{
public:
	std::unordered_map< ResourceKey, std::weak_ptr<BitmapResource>> m_mapBitmap;
	std::unordered_map< ResourceKey,std::weak_ptr<AnimationClip>> m_mapAnimationClip;


};


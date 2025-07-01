#pragma once
using json = nlohmann::json;

struct Sprite
{
	std::string name; // sprite 이름
	float x, y; // sprite x 좌표
	float width, height; // sprite 너비
	float pivotX, pivotY; // sprite 피벗 좌표 (중심점)
};

struct FrameInfo	// 애니메이션의 프레임은 sprite뿐만 아니라 다른 정보도 포함될 수 있다.
{
	std::string sprite; // sprite 이름
	int spriteSheetIndex; // sprite 인덱스
	float duration; // 프레임 지속 시간	
};

struct EventInfo {
	std::string function; // 호출할 함수 이름
	std::string parameter; // 문자열 파라미터
	float time; // 이벤트가 발생하는 시간 (초 단위)
};

struct AnimationClip {
	std::string clipName; // 애니메이션 클립 이름
	std::string texturePath;// 애니메이션 클립에 사용되는 텍스처 경로
	bool loop; // 애니메이션이 반복되는지 여부
	float duration; // 전체
	std::vector<FrameInfo> frames; // 프레임 데이터
	std::vector<EventInfo> events; // 이벤트 데이터
};

struct SpriteSheet {
	std::string texture; // 스프라이트 시트 이름
	float textureWidth, textureHeight; // 스프라이트 시트의 너비와 높이
	std::vector<Sprite> sprites; // Sprite 목록
	std::unordered_map<std::string,int> spriteIndexMap; // 이름으로 index 를 찾기 위한 인덱스 맵
};


// SpriteShheet JSON -> SpriteSheet Class
void LoadSpriteSheet(const std::string& filePath, SpriteSheet& spriteSheet);

// AnimationClip JSON -> AnimationClip Class ,   FrameInfo의 이름으로 Index를 설정하기위해 SpriteSheet 가 필요하다.
void LoadAnimationClip(const std::string& filePath, AnimationClip& clip , const SpriteSheet& SpriteSheet);
#pragma once
using json = nlohmann::json;

struct Sprite
{
	std::string name; // sprite �̸�
	float x, y; // sprite x ��ǥ
	float width, height; // sprite �ʺ�
	float pivotX, pivotY; // sprite �ǹ� ��ǥ (�߽���)
};

struct FrameInfo	// �ִϸ��̼��� �������� sprite�Ӹ� �ƴ϶� �ٸ� ������ ���Ե� �� �ִ�.
{
	std::string sprite; // sprite �̸�
	int spriteSheetIndex; // sprite �ε���
	float duration; // ������ ���� �ð�	
};

struct EventInfo {
	std::string function; // ȣ���� �Լ� �̸�
	std::string parameter; // ���ڿ� �Ķ����
	float time; // �̺�Ʈ�� �߻��ϴ� �ð� (�� ����)
};

struct AnimationClip {
	std::string clipName; // �ִϸ��̼� Ŭ�� �̸�
	std::string texturePath;// �ִϸ��̼� Ŭ���� ���Ǵ� �ؽ�ó ���
	bool loop; // �ִϸ��̼��� �ݺ��Ǵ��� ����
	float duration; // ��ü
	std::vector<FrameInfo> frames; // ������ ������
	std::vector<EventInfo> events; // �̺�Ʈ ������
};

struct SpriteSheet {
	std::string texture; // ��������Ʈ ��Ʈ �̸�
	float textureWidth, textureHeight; // ��������Ʈ ��Ʈ�� �ʺ�� ����
	std::vector<Sprite> sprites; // Sprite ���
	std::unordered_map<std::string,int> spriteIndexMap; // �̸����� index �� ã�� ���� �ε��� ��
};


// SpriteShheet JSON -> SpriteSheet Class
void LoadSpriteSheet(const std::string& filePath, SpriteSheet& spriteSheet);

// AnimationClip JSON -> AnimationClip Class ,   FrameInfo�� �̸����� Index�� �����ϱ����� SpriteSheet �� �ʿ��ϴ�.
void LoadAnimationClip(const std::string& filePath, AnimationClip& clip , const SpriteSheet& SpriteSheet);
// Copyright (c) 2025 Dongwon Lee. All rights reserved.

#include <string>
#include <vector>
#include <fstream>
#include "json.hpp"
#include <stdexcept>
#include "SpriteLoader.h"

/*
from_json은 JSON 데이터를 C++ 객체(Sprite)로 역으로 주입하는 매핑 함수
nlohmann::json::get<T>() 호출 시 자동으로 사용됨
반대로 JSON으로 직렬화하려면 to_json(json&, const Sprite&)도 필요
*/
void from_json(const nlohmann::json& j, Sprite& sprite) {
	j.at("name").get_to(sprite.name);
	j.at("x").get_to(sprite.x);
	j.at("y").get_to(sprite.y);
	j.at("width").get_to(sprite.width);
	j.at("height").get_to(sprite.height);
	j.at("pivotX").get_to(sprite.pivotX);
	j.at("pivotY").get_to(sprite.pivotY);
}


void LoadSpriteSheet(const std::string& filePath, SpriteSheet& spriteSheet)
{
	std::ifstream inFile(filePath);
	if (inFile.is_open()) {
		json j;
		inFile >> j;
		inFile.close();
		// JSON 객체에서 데이터 추출
		spriteSheet.texture = j["texture"];
		spriteSheet.textureWidth = j["textureWidth"];
		spriteSheet.textureHeight = j["textureHeight"];
		spriteSheet.sprites = j["sprites"].get<std::vector<Sprite>>();

		// 스프라이트 이름으로 인덱스 맵 생성
		for (int i = 0; i < spriteSheet.sprites.size(); ++i) {
			spriteSheet.spriteIndexMap[spriteSheet.sprites[i].name] = i;
		}
	}
	else {
		throw std::runtime_error("Failed to open animation clip file: " + filePath);
	}
}


void from_json(const nlohmann::json& j, FrameInfo& frameinfo) {
	j.at("sprite").get_to(frameinfo.sprite);
	j.at("time").get_to(frameinfo.duration);
}

void from_json(const nlohmann::json& j, EventInfo& eventinfo) {
	j.at("function").get_to(eventinfo.function);
	j.at("parameter").get_to(eventinfo.parameter);
	j.at("time").get_to(eventinfo.time);
}

void LoadAnimationClip(const std::string& filePath, AnimationClip& clip, const SpriteSheet& SpriteSheet)
{
	std::ifstream inFile(filePath);
	if (inFile.is_open()) {
		json j;
		inFile >> j;
		inFile.close();
		// JSON 객체에서 데이터 추출
		clip.clipName = j["clipName"];
		clip.texturePath = j["texturePath"];
		clip.loop = j["loop"];
		clip.duration = j["duration"];
		clip.frames = j["frames"].get<std::vector<FrameInfo>>();
		clip.events = j["events"].get<std::vector<EventInfo>>();

		// 프레임의 sprite 이름을 인덱스로 변환
		for (auto& frame : clip.frames) {
			auto it = SpriteSheet.spriteIndexMap.find(frame.sprite);
			if (it != SpriteSheet.spriteIndexMap.end()) {
				frame.spriteSheetIndex = it->second; // sprite 이름으로 인덱스 찾기
			}
			else {
				throw std::runtime_error("Sprite not found in sprite sheet: " + frame.sprite);
			}
		}
	}
	else {
		throw std::runtime_error("Failed to open animation clip file: " + filePath);
	}
}

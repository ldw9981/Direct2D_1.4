// 12_AnimationClip.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <fstream>
#include <string>

#include "json.hpp"

#include "SpriteLoader.h"

using json = nlohmann::json;

struct Person {
	std::string name;
	int age;
	std::vector<std::string> hobbies;
};

int main()
{
	// JSON 라이브러리 사용 예

	// 1. JSON파일 저장하기
	Person person;
	person.name = "John Doe";
	person.age = 30;
	person.hobbies = { "Reading", "Gaming", "Hiking" };
	json j;
	j["name"] = person.name;
	j["age"] = person.age;
	j["hobbies"] = person.hobbies;
	// JSON 객체를 문자열로 변환
	std::string jsonString = j.dump(4); // 4는 들여쓰기 공백 수
	std::cout << "JSON Output:\n" << jsonString << std::endl;

	std::ofstream outFile("../Resource/person.json");
	if (outFile.is_open()) {
		outFile << jsonString;
		outFile.close();
		std::cout << "JSON saved to person.json\n" << std::endl;
	}
	else {
		std::cerr << "Error opening file for writing.\n" << std::endl;
	}

	//2. JSON 파일에서 읽기
	{
		std::ifstream inFile("../Resource/person.json");
		if (inFile.is_open()) {
			json jFromFile;
			inFile >> jFromFile;
			inFile.close();

			// JSON 객체에서 데이터 추출
			Person personFromFile;
			personFromFile.name = jFromFile["name"];
			personFromFile.age = jFromFile["age"];
			personFromFile.hobbies = jFromFile["hobbies"].get<std::vector<std::string>>();

			std::cout << "Read from JSON file:\n";
			std::cout << "Name: " << personFromFile.name << "\n";
			std::cout << "Age: " << personFromFile.age << "\n";
			std::cout << "Hobbies: ";
			for (const auto& hobby : personFromFile.hobbies) {
				std::cout << hobby << " ";
			}
			std::cout << std::endl;
		}
	}
			

	SpriteSheet spriteSheet;
	AnimationClip clip;
	
	LoadSpriteSheet("../Resource/ken_sprites.json", spriteSheet);
	LoadAnimationClip("../Resource/ken_kick_anim.json", clip,spriteSheet);

}

#pragma once

#include <vector>
#include <queue>
#include <cstdint>
#include <unordered_set>
#include "Singleton.h"

class Object;
/*
	이것은 이해를 돕기위한 예시 클래스 입니다.
	실제 게임에서는 ObjectTable을 Scene이나 GameManager와 같은곳에서 포인터를 검증할수 있도록 합니다.
*/
class ObjectTable : public Singleton<ObjectTable>
{
	//ObjectTable의 생성자 / 소멸자를 private으로 선언했기 때문에, 
	// 일반 코드는 new MyManager()나 delete를 직접 호출할 수 없습니다.
	friend class Singleton<ObjectTable>;// ← 없으면 아래 Create()에서 컴파일 에러
private:
	ObjectTable() = default;
	~ObjectTable() = default;

	std::unordered_set<Object*> m_objects;      // 생성된 인스턴스 주소 저장
public:
	
	// 인스턴스 주소가 존재하는지 검사
	bool IsValid(Object* instance) {
		return m_objects.find(instance) != m_objects.end();
	}
	void Register(Object* instance) {
		m_objects.insert(instance);
	}
	void Unregister(Object* instance) {
		m_objects.erase(instance);
	}
};
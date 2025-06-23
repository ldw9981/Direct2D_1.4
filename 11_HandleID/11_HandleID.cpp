#include <vector>
#include <queue>
#include <cstdint>
#include <iostream>
#include <assert.h>

// 예시용 GameObject
class GameObject {
public:
	int value = 0;
};

// 오브젝트를 가리키는 핸들
struct GameObjectHandle {
	uint32_t id = 0;
	uint32_t generation = 0;
};
	

class ObjectManager {
	std::vector<GameObject*> objects;      // 실제 객체 저장
	std::vector<uint32_t> generations;     // 각 객체의 Generation
	std::queue<uint32_t> freeIDs;          // 파괴된 객체의 id를 재사용
		
	
public:
	// 오브젝트 생성
	GameObjectHandle Create() {
		uint32_t id;
		if (freeIDs.empty()) {	//재활용 가능한 인덱스가 없다면
			id = static_cast<uint32_t>(objects.size());// 추가전 사이즈가 인덱스가됨.
			generations.push_back(0);				// 새로운 인덱스에 세대 값0
			objects.push_back(new GameObject());	// 뒤쪽에 인스턴스 포인터 저장
		}
		else { // 재활용 가능한 인덱스가 있다면
			id = freeIDs.front();			// 보관중인 ID 재사용 
			freeIDs.pop();					// 큐에서 제거
			generations[id]++;				// 세대 증가 (새 오브젝트 의미)		
			assert(objects[id] == nullptr);
			objects[id] = new GameObject();		// 재활용하는 인덱스에 저장
		}
		return GameObjectHandle{ id, generations[id] };
	}

	// 오브젝트 파괴
	void Destroy(const GameObjectHandle& h) {
		if (IsValid(h)) {
			delete objects[h.id];
			objects[h.id] = nullptr;
			freeIDs.push(h.id); // id 재사용 가능하게 큐에 저장
			// generation은 다음 생성에서 증가
		}
	}

	// 핸들이 가리키는 오브젝트가 유효한지 검사
	bool IsValid(const GameObjectHandle& h) const {
		return h.id < objects.size()
			&& objects[h.id] != nullptr
			&& generations[h.id] == h.generation;
	}

	// 오브젝트 포인터 얻기
	GameObject* Get(const GameObjectHandle& h) {
		if (IsValid(h)) return objects[h.id];
		return nullptr;
	}
};

int main() {
	ObjectManager mgr;

	// 오브젝트 생성
	GameObjectHandle h1 = mgr.Create();
	GameObjectHandle h2 = mgr.Create();

	// 오브젝트 사용
	if (mgr.IsValid(h1)) {
		GameObject* obj1 = mgr.Get(h1);
		obj1->value = 42;
		std::cout << "h1 value: " << obj1->value << std::endl;
	}

	// 오브젝트 파괴
	mgr.Destroy(h1);
		

	// 파괴 후 접근 (유효성 검사, nullptr 반환)
	if (mgr.Get(h1) == nullptr)
		std::cout << "h1 is destroyed, cannot use anymore." << std::endl;

	// 같은 ID에 새로운 Generation으로 재생성될 수 있음
	GameObjectHandle h3 = mgr.Create();
	if (!mgr.IsValid(h1) && mgr.IsValid(h3))
		std::cout << "h1 is invalid, but h3 is new valid object." << std::endl;

	return 0;
}

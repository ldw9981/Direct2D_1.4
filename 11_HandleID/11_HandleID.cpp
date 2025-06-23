#include <vector>
#include <queue>
#include <cstdint>
#include <iostream>
#include <assert.h>




// 오브젝트를 가리키는 핸들
struct GameObjectHandle {
	uint32_t index = 0;
	uint32_t generation = 0;
};

// 예시용 GameObject
class GameObject {
public:
	int value = 0;
	GameObjectHandle handle;
};

class ObjectManager {
	std::vector<GameObject*> objects;      // 생성된 인스턴스 주소 저장
	std::vector<uint32_t> generations;     // 각 객체의 Generation
	std::queue<uint32_t> freeIndexes;          // 파괴된 객체의 id를 재사용
		
	
public:
	// 오브젝트 생성
	template<typename T>
	GameObjectHandle Create() {
		
		GameObjectHandle hRet;
		uint32_t index = 0;
		if (freeIndexes.empty()) {	//재활용 가능한 인덱스가 없다면
			index = static_cast<uint32_t>(objects.size());// 추가전 사이즈가 인덱스가됨.
			generations.push_back(0);				// 새로운 인덱스에 세대 값0

			// 컴파일 시점에 T가 GameObject base인지 확인
			static_assert(std::is_base_of<GameObject, T>::value, "T must derive from GameObject");
			objects.push_back(new T());	// 뒤쪽에 인스턴스 포인터 저장
		}
		else { // 재활용 가능한 인덱스가 있다면
			index = freeIndexes.front();			// 보관중인 ID 재사용 
			freeIndexes.pop();					// 큐에서 제거
			generations[index]++;				// 세대 증가 (새 오브젝트 의미)		
			assert(objects[index] == nullptr);
			objects[index] = new T();		// 재활용하는 인덱스에 저장
		}	
		
		hRet.index = index;
		hRet.generation = generations[index];
		objects[index]->handle = hRet;	// 생성한 객체내부에 핸들값 저장
		return hRet;
	}

	// 오브젝트 파괴
	void Destroy(const GameObjectHandle& h) {
		if (IsValid(h)) {
			delete objects[h.index];
			objects[h.index] = nullptr;
			freeIndexes.push(h.index); // id 재사용 가능하게 큐에 저장
			// generation은 다음 생성에서 증가
		}
	}

	// 핸들이 가리키는 오브젝트가 유효한지 검사
	bool IsValid(const GameObjectHandle& h) const {
		return h.index < objects.size()
			&& objects[h.index] != nullptr
			&& generations[h.index] == h.generation;
	}

	// 오브젝트 포인터 얻기
	GameObject* Get(const GameObjectHandle& h) {
		if (IsValid(h)) return objects[h.index];
		return nullptr;
	}
};

int main() {
	ObjectManager mgr;

	// 오브젝트 생성
	GameObjectHandle h1 = mgr.Create<GameObject>();
	GameObjectHandle h2 = mgr.Create<GameObject>();

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
	GameObjectHandle h3 = mgr.Create<GameObject>();
	if (!mgr.IsValid(h1) && mgr.IsValid(h3))
		std::cout << "h1 is invalid, but h3 is new valid object." << std::endl;

	return 0;
}

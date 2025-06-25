#pragma once
#include <vector>
#include <queue>
#include <cstdint>
#include "Singleton.h"

class Object;
struct ObjectHandle {
	uint32_t index = 0;
	uint32_t generation = 0;
};

class ObjectHandleTable : public Singleton<ObjectHandleTable>
{
	//ObjectHandleTable의 생성자 / 소멸자를 private으로 선언했기 때문에, 
	// 일반 코드는 new MyManager()나 delete를 직접 호출할 수 없습니다.
	friend class Singleton<ObjectHandleTable>;// ← 없으면 아래 Create()에서 컴파일 에러
private:
	ObjectHandleTable() = default;
	~ObjectHandleTable() = default;

	std::vector<Object*> objects;      // 생성된 인스턴스 주소 저장
	std::vector<uint32_t> generations;     // 각 객체의 Generation
	std::queue<uint32_t> freeIndexes;          // 파괴된 객체의 index를 재사용
public:
	template<typename T>
	T* GetTypePtr(const ObjectHandle& h) {
		Object* base = GetObjectPtr(h);
		return dynamic_cast<T*>(base);  // typeid 기반 RTTI 다운캐스팅
	}

	Object* GetObjectPtr(const ObjectHandle& h) {
		if (h.index >= objects.size() || !objects[h.index]) return nullptr;
		if (generations[h.index] != h.generation) return nullptr;
		return objects[h.index];
	}
	ObjectHandle CreateHandle(Object* allocated) {
		uint32_t index = 0;
		if (freeIndexes.empty()) {	//재활용 가능한 인덱스가 없다면
			index = static_cast<uint32_t>(objects.size());// 추가전 사이즈가 인덱스가됨.
			generations.push_back(0);				// 새로운 인덱스에 세대 값0
			objects.push_back(allocated);	// 뒤쪽에 인스턴스 포인터 저장
		}
		else { // 재활용 가능한 인덱스가 있다면
			index = freeIndexes.front();			// 보관중인 index 재사용 
			freeIndexes.pop();					// 큐에서 제거
			generations[index]++;				// 세대 증가 (새 오브젝트 의미)		
			assert(objects[index] == nullptr);
			objects[index] = allocated;		// 재활용하는 인덱스에 저장
		}
		return ObjectHandle{ index, generations[index] };
	}
	void DestroyHandle(const ObjectHandle& h) {
		if (IsValid(h)) {
			objects[h.index] = nullptr;
			freeIndexes.push(h.index); // index 재사용 가능하게 큐에 저장
			// generation은 다음 생성에서 증가
		}
	}
	// 핸들이 가리키는 오브젝트가 유효한지 검사
	bool IsValid(const ObjectHandle& h) const {
		return h.index < objects.size()
			&& objects[h.index] != nullptr
			&& generations[h.index] == h.generation;
	}

	// 오브젝트 포인터 얻기
	Object* Get(const ObjectHandle& h) {
		if (IsValid(h)) return objects[h.index];
		return nullptr;
	}
};

template<typename T>
class WeakObjectPtr {
public:
	T* ptr=nullptr;
	ObjectHandle handle;
	
	WeakObjectPtr()=default;
	WeakObjectPtr(T* instance)
	{
		Set(instance);
	}

	bool IsValid() const {
		return ptr != nullptr && ObjectHandleTable::Instance().IsValid(handle);
	}

	void Set(T* instance) {  ptr = instance; handle = instance->GetHandle(); 	}
	T* Get() const {
		return ObjectHandleTable::Instance().IsValid(handle) ? ptr : nullptr;
	}

	T* operator->() const {
		return ptr; // 호출 전에 반드시 유효성 검사할 것
	}	
};
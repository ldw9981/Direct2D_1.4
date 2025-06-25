#include <vector>
#include <queue>
#include <cstdint>
#include <iostream>
#include <assert.h>




// 오브젝트를 가리키는 핸들
struct ObjectHandle {
	uint32_t index = 0;
	uint32_t generation = 0;
};

// 예시용 GameObject
class Object 
{
public:
	Object()
	{

	}
	int m_value = 0;
	ObjectHandle m_handle;
};

class GameObject : public Object
{
public:
};

class Component : public Object
{
public:
};


class ObjectHandleTable {
	std::vector<Object*> objects;      // 생성된 인스턴스 주소 저장
	std::vector<uint32_t> generations;     // 각 객체의 Generation
	std::queue<uint32_t> freeIndexes;          // 파괴된 객체의 index를 재사용
public:
	template<typename T>
	T* Get(const ObjectHandle& h) {
		Object* base = GetRaw(h);
		return dynamic_cast<T*>(base);  // typeid 기반 RTTI 다운캐스팅
	}

	Object* GetRaw(const ObjectHandle& h) {
		if (h.index >= objects.size() || !objects[h.index]) return nullptr;
		if (generations[h.index] != h.generation) return nullptr;
		return objects[h.index];
	}
	ObjectHandle Create(Object* allocated) {		
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
	void Destroy(const ObjectHandle& h) {
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

class Scene
{
public:
	std::vector<GameObject*> m_Objects;	
	ObjectHandleTable m_Table;
public:
	Scene() = default;
	template<typename T>
	T* CreateGameObject()
	{
		T* pObject = new T();
		ObjectHandle handle = m_Table.Create(pObject);		
		pObject->m_handle = handle;
		m_Objects.push_back(pObject);
		return pObject;
	}
	void DestroyGameObject(GameObject* pGameObject)
	{
		m_Table.Destroy(pGameObject->m_handle);
		auto it = std::find(m_Objects.begin(), m_Objects.end(), pGameObject);
		if (it != m_Objects.end())
			m_Objects.erase(it);

		delete pGameObject; // 실제 메모리 해제도 잊지 말기
	}
	bool IsValid(ObjectHandle handle)
	{
		return m_Table.IsValid(handle);
	}
};

int main() 
{
	bool result;
	Scene scene;

	GameObject* pObject = scene.CreateGameObject<GameObject>();
	ObjectHandle handle = pObject->m_handle;
	result = scene.IsValid(handle);


	scene.DestroyGameObject(pObject);
	result = scene.IsValid(handle);

	// 같은 Index에 새로운 Generation으로 재생성될 수 있음
	GameObjectHandle h3 = mgr.Create<GameObject>();
	if (!mgr.IsValid(h1) && mgr.IsValid(h3))
		std::cout << "h1 is invalid, but h3 is new valid object." << std::endl;

	return 0;
}

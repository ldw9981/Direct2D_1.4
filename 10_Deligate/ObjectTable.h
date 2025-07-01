#pragma once

#include <vector>
#include <queue>
#include <cstdint>
#include <unordered_set>
#include "Singleton.h"

class Object;
class ObjectTable : public Singleton<ObjectTable>
{
	//ObjectTable�� ������ / �Ҹ��ڸ� private���� �����߱� ������, 
	// �Ϲ� �ڵ�� new MyManager()�� delete�� ���� ȣ���� �� �����ϴ�.
	friend class Singleton<ObjectTable>;// �� ������ �Ʒ� Create()���� ������ ����
private:
	ObjectTable() = default;
	~ObjectTable() = default;

	std::unordered_set<Object*> m_objects;      // ������ �ν��Ͻ� �ּ� ����
public:
	
	// �ڵ��� ����Ű�� ������Ʈ�� ��ȿ���� �˻�
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
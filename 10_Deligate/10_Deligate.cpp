#include <vector>
#include <queue>
#include <cstdint>
#include <iostream>
#include <assert.h>
#include <functional>
#include <math.h>


#include "ObjectHandleTable.h"
#include "GameObject.h"
#include "Component.h"

template<typename... Args>
class MultiDelegate {
	struct Slot {
		ObjectHandle handle; // 콜백 구분용 포인터(주로 this)
		std::function<void(Args...)> func;
	};
	std::vector<Slot> slots;

public:
	// Handle을 같이 받는다.
	void Add(ObjectHandle handle, const std::function<void(Args...)>& f) {
		slots.push_back({ handle, f });
	}
	// tag(포인터)로 삭제
	void Remove(ObjectHandle handle) {
		slots.erase(
			std::remove_if(slots.begin(), slots.end(),
				[handle](const Slot& s) { return s.handle == handle; }),
			slots.end());
	}
	void Clear() { slots.clear(); }
	void BroadCast(Args... args) const {
		for (const auto& s : slots)
		{
			// Handle로 테이블에서 유효한지 검사
			if (ObjectHandleTable::Instance().IsValid(s.handle))
				s.func(args...);
		}
			
	}
};

class OtherComponent : public Component
{
public:
	void OnChangeHealth(int prev, int curr)
	{
		std::cout << "OnChange HP " << prev << "->" << curr << "\n";
	}
};

class HealthComponent : public Component 
{
public:
	int HP = 100;
	MultiDelegate<int, int> onChangeHealth;	// Prev,Curr
	void TakeDamage(int value)
	{
		int result = std::max<int>(0, HP - value);
		SetHP(result);
	}
	void SetHP(int value)
	{
		if (HP != value) {
			int prev = HP;
			HP = value;
			onChangeHealth.BroadCast(prev, HP);
			// 체력의 변화를 받는 함수를 모두 호출
		}
	}
};

class Player : public GameObject
{
public:
	OtherComponent other;
	HealthComponent health;
	Player* Target=nullptr;
	ObjectHandle hTarget;
	WeakObjectPtr<Player> wptrTarget;

	// 공격할 GameObject의 insance와 Handle을 보관한다.
	void SetTarget(Player* instance)
	{
		Target = instance;
		hTarget = instance->GetHandle();
		wptrTarget.Set(instance); // 클래스 타입으로도 테스트
	}
	void Attack()
	{
		// 타겟이 있고 Handle도 유효한지 확인
		if (Target != nullptr && ObjectHandleTable::Instance().IsValid(hTarget))
		{
			Target->health.TakeDamage(10);
		}		
	
		// 사용하기 편하게 클래스로 같은작업 도와주는 WeakObjectPtr
		if (wptrTarget.IsValid())
		{
			wptrTarget->health.TakeDamage(10);
		}
		
	}

	void Start()
	{
		health.onChangeHealth.Add(other.GetHandle(), // 컨테이너에서 검색 키로 사용할 값(주소)
			std::bind(&OtherComponent::OnChangeHealth,  // 실행할 클래스 멤버함수
			&other,  // 실행할 클래스 인스턴스 주소			
			std::placeholders::_1,	//BroadCast 호출에서 OnChangeHealth에 넘겨주는 자리
			std::placeholders::_2));//BroadCast 호출에서 OnChangeHealth에 넘겨주는 자리
	}
};

int main() 
{
	ObjectHandleTable::Create();

	Player* player1 = new Player;
	Player* player2 = new Player;
	player1->Start();
	//외부에서 TakeDamage를 호출한다.
	player1->health.TakeDamage(20);  // HP가 80이 됨 → onChangeHealth 브로드캐스트
	player1->health.TakeDamage(20);  // HP가 60이 됨 → onChangeHealth 브로드캐스트

	player2->SetTarget(player1);
	player2->Attack();
	delete player1;


	player2->Attack();
	delete player2;

	ObjectHandleTable::Destroy();
	return 0;
}

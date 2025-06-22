#include <vector>
#include <algorithm>
#include <functional>

template<typename... Args>
class FMulticastDelegate
{
	using FHandlerType = std::function<void(Args...)>;
	struct FSlot
	{
		void* InstancePtr;
		FHandlerType Handler;
	};
	std::vector<FSlot> Slots;

public:
	// 바인딩 (언리얼은 핸들 반환, 여기선 단순화)
	void Add(void* InInstance, FHandlerType InHandler)
	{
		Slots.push_back({ InInstance, InHandler });
	}

	// 인스턴스 단위로 전부 제거
	void Remove(void* InInstance)
	{
		Slots.erase(
			std::remove_if(Slots.begin(), Slots.end(),
				[InInstance](const FSlot& s) { return s.InstancePtr == InInstance; }),
			Slots.end());
	}

	// 모두 호출
	void Broadcast(Args... args)
	{
		for (const auto& slot : Slots)
			if (slot.Handler) slot.Handler(args...);
	}

	// 전체 제거
	void Clear()
	{
		Slots.clear();
	}
};

/////////////////////////////////////////////////////////
// 사용 예시

class FMyListener
{
public:
	void OnDamage(int prev, int curr)
	{
		printf("Listener: Damage changed %d → %d\n", prev, curr);
	}
};

class FHealthComponent
{
public:
	int HP = 100;
	FMulticastDelegate<int, int> OnChangeHealth; // Prev,Curr

	void TakeDamage(int value)
	{
		int prev = HP;
		HP = std::max(0, HP - value);
		OnChangeHealth.Broadcast(prev, HP);
	}
};

/////////////////////////////////////////////////////////

int main()
{
	FHealthComponent Health;
	FMyListener Listener;

	// 멤버 함수 바인딩 (C++11 std::bind 사용)
	Health.OnChangeHealth.Add(&Listener,
		std::bind(&FMyListener::OnDamage, &Listener, std::placeholders::_1, std::placeholders::_2));

	Health.TakeDamage(10); // Listener: Damage changed 100 → 90
	Health.TakeDamage(30); // Listener: Damage changed 90 → 60

	// 바인딩 해제
	Health.OnChangeHealth.Remove(&Listener);
	Health.TakeDamage(50); // 아무 출력 없음

	return 0;
}

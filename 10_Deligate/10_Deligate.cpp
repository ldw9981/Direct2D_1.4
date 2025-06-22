#include <vector>
#include <functional>
#include <iostream>
#include <math.h>

template<typename... Args>
class MultiDelegate {
	struct Slot {
		void* instance; // 콜백 구분용 포인터(주로 this)
		std::function<void(Args...)> func;
	};
	std::vector<Slot> slots;

public:
	void Add(void* instance, const std::function<void(Args...)>& f) {
		slots.push_back({ instance, f });
	}
	// tag(포인터)로 삭제
	void Remove(void* instance) {
		slots.erase(
			std::remove_if(slots.begin(), slots.end(),
				[instance](const Slot& s) { return s.instance == instance; }),
			slots.end());
	}
	void Clear() { slots.clear(); }
	void BroadCast(Args... args) const {
		for (const auto& s : slots)
			if (s.func) s.func(args...);
	}
};

class OtherComponent {
public:
	void OnChangeHealth(int prev, int curr)
	{
		std::cout << "OnChange HP " << prev << "->" << curr << "\n";
	}
};

class HealthComponent {
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

class Player {
public:
	OtherComponent other;
	HealthComponent health;
	void Start()
	{
		health.onChangeHealth.Add(&other,
			std::bind(&OtherComponent::OnChangeHealth, &other,
				std::placeholders::_1, std::placeholders::_2));
	}
};

int main() {
	Player player;
	player.Start();
	//외부에서 TakeDamage를 호출한다.
	player.health.TakeDamage(20);  // HP가 80이 됨 → onChangeHealth 브로드캐스트
	player.health.TakeDamage(20);  // HP가 60이 됨 → onChangeHealth 브로드캐스트

	return 0;
}

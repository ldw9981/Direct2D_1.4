#include <vector>
#include <functional>
#include <iostream>

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


class Handler {
public:
	void OnEvent(int v) { std::cout << "Handler(" << this << "): " << v << "\n"; }
};

class Player {
public:
	MultiDelegate<int> onEvent;
	void OnPlayerEvent(int v) { std::cout << "Player(" << this << "): " << v << "\n"; }
};

int main() {
	
	Handler h;
	Player p;

	// 멤버함수 등록 
	p.onEvent.Add(&h, std::bind(&Handler::OnEvent, &h, std::placeholders::_1));
	p.onEvent.BroadCast(100); 

	// h의 콜백만 해제
	p.onEvent.Remove(&h);

	p.onEvent.BroadCast(200); // p만 호출

	// 모두 삭제
	p.onEvent.Clear();
	p.onEvent.BroadCast(300); // 아무것도 호출되지 않음

	return 0;
}

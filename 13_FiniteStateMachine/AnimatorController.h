#pragma once


struct Parameter
{
	std::string name; // 파라미터 이름
	std::string type; // 파라미터 타입 (예: Float, Int, Bool)
	float defaultFloat = 0.0f; // 기본값 (Float 타입일 때 사용)
	int defaultInt = 0; // 기본값 (Int 타입일 때 사용)
	bool defaultBool = false; // 기본값 (Bool 타입일 때 사용)
};

struct Condition
{
	std::string parameter; // 조건 파라미터 이름
	std::string mode; // 조건 모드 (예: Greater, IfNot 등)
	float threshold = 0.0f; // 조건 임계값
};

struct Transition
{
	std::string fromState; // 전이 시작 상태 이름
	std::string toState; // 전이 종료 상태 이름	
	bool hasExitTime = false; // 종료 시간이 있는지 여부
	float exitTime = -1.0f; // 1.0이면 100% 완료 후 전이, -1.0이면 종료 시간 없음
	std::vector<Condition> conditions;
};

struct AnyStateTransition
{
	std::string toState; // 전이 종료 상태 이름
	std::vector<Condition> conditions;
};

struct State
{
	int index = -1; // 상태 인덱스 (필요시 사용)
	std::string name; // 상태 이름
	std::string motionName; // 모션 이름 (애니메이션 클립 이름)
	float clipLength = 1.0f; //초 단위 Todo!!AnimationClip찾아 수정 필요
	bool loop = false; // 루프 여부 Todo!!AnimationClip찾아 수정 필요
	std::vector<Transition> transitions; // 전이 목록
};


struct AnimatorController
{
	std::string controllerName; // 컨트롤러 이름
	std::vector<Parameter> parameters; // 컨트롤러 파라미터 목록
	
	std::string defaultState; // 기본 상태 이름	
	std::vector<State> states;
	std::unordered_map<std::string, int> stateNameToIndex;
	std::vector<AnyStateTransition> anyStateTransitions; // Any State 전이 목록

	State* GetState(const std::string& stateName)
	{
		auto it = stateNameToIndex.find(stateName);
		if (it != stateNameToIndex.end()) {
			return &states[it->second]; // 상태가 존재하면 해당 상태 반환
		}
		return nullptr; // 상태가 존재하지 않으면 nullptr 반환
	}
};

void LoadAnimatorController(const std::string& filePath, AnimatorController& clip);

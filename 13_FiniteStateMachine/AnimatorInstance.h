#pragma once
#include "AnimatorController.h"

class IStateNotify
{
public:
	virtual void OnEnterState(const std::string& stateName) = 0;
	virtual void OnExitState(const std::string& stateName) = 0;
	virtual void OnUpdateState(const std::string& stateName) = 0;
};
struct State;
struct AnimatorController;


class AnimatorInstance
{
private:
	enum class ParameterType { Int, Float, Bool,Trigger };
	std::unordered_map<std::string, ParameterType> m_ParameterTypes;
	std::unordered_map<std::string, int> m_IntParameters;
	std::unordered_map<std::string, float> m_FloatParameters;
	std::unordered_map<std::string, bool> m_BoolParameters;
	std::unordered_map<std::string, bool> m_TriggerParameters;

	AnimatorController* m_Controller = nullptr; // 현재 애니메이터 컨트롤러
	float m_elapsedTime = 0.0f; // 프레임 간 시간 차이	
	int m_CurrentStateIndex = -1; // 현재 상태 인덱스
	int m_PrevStateIndex = -1; // 이전 상태 인덱스
	std::vector<IStateNotify*> m_StateNotifiers; // 상태 알림 인터페이스
public:
	void RegisterParameter(const std::string& name, ParameterType type);
	void SetInt(const std::string& name, int value);
	void SetFloat(const std::string& name, float value);
	void SetBool(const std::string& name, bool value);
	void SetTrigger(const std::string& name, bool value);
	
	int GetInt(const std::string& name) const;
	bool GetBool(const std::string& name) const;
	bool GetTrigger(const std::string& name) const;
	float GetFloat(const std::string& name) const;
	bool CheckCondition(const std::string& param, const std::string& mode, float threshold);

	bool EvaluateConditions(const std::vector<Condition>& conditions);

	void SetAnimatorController(AnimatorController* controller);

	void ChangeState(const std::string& stateName);
	
	void Update(float deltaTime);	

	void OnStart();
	void OnExit();
};


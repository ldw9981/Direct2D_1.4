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
	
	std::unordered_map<std::string, ParameterType> m_ParameterTypes;
	std::unordered_map<std::string, int> m_IntParameters;
	std::unordered_map<std::string, float> m_FloatParameters;
	std::unordered_map<std::string, bool> m_BoolParameters;
	std::unordered_map<std::string, bool> m_TriggerParameters;

	AnimatorController* m_Controller = nullptr; // ���� �ִϸ����� ��Ʈ�ѷ�
	float m_elapsedTime = 0.0f; // �ִϸ��̼� �ð� ������
	int m_CurrentStateIndex = -1; // ���� ���� �ε���
	int m_PrevStateIndex = -1; // ���� ���� �ε���
	std::vector<IStateNotify*> m_StateNotifiers; // ���� �˸� �������̽�
private:
	bool GetTrigger(const std::string& name) const;
	void ResetTrigger(const std::string& name);
public:
	void RegisterParameterInt(const std::string& name, int value);
	void RegisterParameterFloat(const std::string& name, float value);
	void RegisterParameterBool(const std::string& name, bool value);
	void RegisterParameterTrigger(const std::string& name);


	void SetInt(const std::string& name, int value);
	void SetFloat(const std::string& name, float value);
	void SetBool(const std::string& name, bool value);
	void SetTrigger(const std::string& name);
	
	int GetInt(const std::string& name) const;
	bool GetBool(const std::string& name) const;	
	float GetFloat(const std::string& name) const;
	bool CheckCondition(const Condition& condition);
	bool EvaluateConditions(const std::vector<Condition>& conditions);
	void SetAnimatorController(AnimatorController* controller);
	void ChangeState(const std::string& stateName);	
	void Update(float deltaTime);	

	void OnStart();
	void OnExit();
};


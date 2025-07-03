#include <unordered_map>
#include <vector>
#include <string>
#include <assert.h>
#include <iostream>

#include "AnimatorInstance.h"
#include "AnimatorController.h"

void AnimatorInstance::RegisterParameterInt(const std::string& name, int value)
{
	m_IntParameters[name] = value;
}

void AnimatorInstance::RegisterParameterFloat(const std::string& name, float value)
{
	m_FloatParameters[name] = value;
}

void AnimatorInstance::RegisterParameterBool(const std::string& name, bool value)
{
	m_BoolParameters[name] = value;
}

void AnimatorInstance::RegisterParameterTrigger(const std::string& name)
{
	m_TriggerParameters[name] = false; // 트리거는 기본값이 false로 설정됨
}

void AnimatorInstance::SetInt(const std::string& name, int value)
{
	auto it = m_IntParameters.find(name);
	if (it == m_IntParameters.end())
	{
		std::cerr << "Error: Parameter " << name << " is not registered as Int type." << std::endl;
		return;
	}

	it->second = value;
}

void AnimatorInstance::SetFloat(const std::string& name, float value)
{
	auto it = m_FloatParameters.find(name);
	if (it == m_FloatParameters.end())
	{
		std::cerr << "Error: Parameter " << name << " is not registered as float type." << std::endl;
		return;
	}
	it->second = value;
}

void AnimatorInstance::SetBool(const std::string& name, bool value)
{
	auto it = m_BoolParameters.find(name);
	if (it == m_BoolParameters.end())
	{
		std::cerr << "Error: Parameter " << name << " is not registered as bool type." << std::endl;
		return;
	}
	it->second = value;
}
void AnimatorInstance::SetTrigger(const std::string& name)
{
	auto it = m_TriggerParameters.find(name);
	if (it == m_TriggerParameters.end())
	{
		std::cerr << "Error: Parameter " << name << " is not registered as Trigger type." << std::endl;
		return;
	}
	
	it->second = true;
}
void AnimatorInstance::ResetTrigger(const std::string& name)
{
	auto it = m_TriggerParameters.find(name);
	if (it == m_TriggerParameters.end())
	{
		std::cerr << "Error: Parameter " << name << " is not registered as Trigger type." << std::endl;
		return;
	}

	it->second = false;
}

int AnimatorInstance::GetInt(const std::string& name) const
{
	auto it = m_IntParameters.find(name);
	assert(it != m_IntParameters.end());
	if (it != m_IntParameters.end()) {
		return it->second;
	}
	return 0; // 기본값
}

bool AnimatorInstance::GetBool(const std::string& name) const
{
	auto it = m_BoolParameters.find(name);
	assert(it != m_BoolParameters.end());
	if (it != m_BoolParameters.end()) {
		return it->second;
	}
	return false; // 기본값
}

float AnimatorInstance::GetFloat(const std::string& name) const
{
	auto it = m_FloatParameters.find(name);
	assert(it != m_FloatParameters.end());
	if (it != m_FloatParameters.end()) {
		return it->second;
	}
	return 0.0f; // 기본값
}

bool AnimatorInstance::GetTrigger(const std::string& name) const
{
	auto it = m_TriggerParameters.find(name);
	assert(it != m_TriggerParameters.end());
	if (it != m_TriggerParameters.end()) 
	{
		return it->second;
	}
	return false; // 기본값
}

bool AnimatorInstance::CheckCondition(const Condition& condition)
{	
	std::string param = condition.parameter;
	std::string mode = condition.mode;
	float threshold = condition.threshold;

	switch (condition.type)
	{
	case ParameterType::Bool:
	{
		bool val = GetBool(param);
		if (mode == "If") return val;
		if (mode == "IfNot") return !val;
		break;
	}
	case ParameterType::Int:
	{
		int val = GetInt(param);
		if (mode == "Greater") return val > threshold;
		if (mode == "Less") return val < threshold;
		if (mode == "Equals") return val == static_cast<int>(threshold);
		if (mode == "NotEqual") return val != static_cast<int>(threshold);
		break;
	}
	case ParameterType::Float:
	{
		float val = GetFloat(param);
		if (mode == "Greater") return val > threshold;
		if (mode == "Less") return val < threshold;
		if (mode == "Equals") return val == threshold;
		if (mode == "NotEqual") return val != threshold;
		break;
	}
	case ParameterType::Trigger:
	{
		bool val = GetTrigger(param);
		if (mode == "If") return val;
		if (mode == "IfNot") return !val;
		break;
	}
	default:
		return false;
	}
	return false;
}

bool AnimatorInstance::EvaluateConditions(const std::vector<Condition>& conditions)
{
	for (const auto& condition : conditions) {
		if (!CheckCondition(condition)) {
			return false; // 하나라도 조건이 만족하지 않으면 false
		}
	}
	return true; // 모든 조건이 만족하면 true
}

void AnimatorInstance::SetAnimatorController(AnimatorController* controller)
{
	m_Controller = controller;
	// Parameter 초기화
	m_FloatParameters.clear();
	m_IntParameters.clear();
	m_BoolParameters.clear();
	for (const auto& param : controller->parameters) 
	{
		if (param.type == "Int") {				
			RegisterParameterInt(param.name, param.defaultInt);
		}
		else if (param.type == "Float") {			
			RegisterParameterFloat(param.name, param.defaultFloat);
		}
		else if (param.type == "Bool") {			
			RegisterParameterBool(param.name, param.defaultBool);
		}
		else if (param.type == "Trigger") {			
			RegisterParameterTrigger(param.name);
		}
	}

	// 상태별로 인터페이스 주소를 보관할 공간을 확보한다.
	m_StateNotifiers.clear();
	for (auto& state : m_Controller->states) {	
		m_StateNotifiers.push_back(nullptr);
	}

	
	// ToDo: 전체 State를 순회하면서 사용하는 AnimaitionClip의 길이를 가져와 
	// State의 clipLength에 저장해야 한다.

	
}

void AnimatorInstance::ChangeState(const std::string& stateName)
{
	assert(m_Controller != nullptr);
	
	State* pNext = m_Controller->GetState(stateName);
	if (pNext == nullptr)
		return;
	
	m_PrevStateIndex = m_CurrentStateIndex;
	m_CurrentStateIndex = pNext->index;	
	m_elapsedTime = 0.0f; // ← 전이 후 시간 초기화

	if (m_PrevStateIndex != -1 && m_StateNotifiers[m_PrevStateIndex] != nullptr)
	{
		
		m_StateNotifiers[m_PrevStateIndex]->OnExitState(
			m_Controller->states[m_PrevStateIndex].name);
	}

	if (m_StateNotifiers[m_CurrentStateIndex] != nullptr) {
		m_StateNotifiers[m_CurrentStateIndex]->OnEnterState(
			m_Controller->states[m_CurrentStateIndex].name);
	}

	// ToDo:: 모션이름으로 현재 사용할 Animation Clip 참조 연결해야합니다. 
	std::cout << "ChangeState " 
		<< m_Controller->states[m_CurrentStateIndex].motionName << std::endl;
}


void AnimatorInstance::Update(float deltaTime)
{
	if (m_CurrentStateIndex < 0)
		return;

	m_elapsedTime += deltaTime;
	// 재생 비율 계산 0.0~1.0
	float clipLength = m_Controller->states[m_CurrentStateIndex].clipLength;
	float elapsedRate = m_elapsedTime / clipLength;
	
	if (m_elapsedTime > clipLength && m_Controller->states[m_CurrentStateIndex].loop)
	{
		m_elapsedTime = fmod(m_elapsedTime, clipLength); // 나머지 시간
	}

	// Todo: SpriteRender에 현재시간에 맞는 Sprite정보를 설정해야합니다.	
	//
	//
	
	std ::cout << "State "
		<< m_Controller->states[m_CurrentStateIndex].name 
		<< ", Elapsed Time: " << m_elapsedTime << std::endl;
	
	if (m_StateNotifiers[m_CurrentStateIndex] != nullptr) {
		m_StateNotifiers[m_CurrentStateIndex]->OnUpdateState(
			m_Controller->states[m_CurrentStateIndex].name);
	}

	const auto& currentStateDef = m_Controller->states.at(m_CurrentStateIndex);

	for (const auto& trans : currentStateDef.transitions) {
		// 해당 트랜지션이 exitTime을 사용하는 경우 재생율 비교하여 클때만 검사   
		if (trans.hasExitTime && elapsedRate < trans.exitTime)
			continue;
		
		// false이면 매번 프레임 평가
		if (EvaluateConditions(trans.conditions)) {
			std::cout << "Transition "
				<< m_Controller->states[m_CurrentStateIndex].name << " -> "
				<< trans.toState << " " << m_elapsedTime << std::endl;
			ChangeState(trans.toState);

			for (auto& condition : trans.conditions) {				
				if( condition.type == ParameterType::Trigger) {
					// 트리거는 한번만 평가되므로 Reset
					ResetTrigger(condition.parameter);
				}
			}
			return;
		}
	}

	// 상태와 상관없이 Any State 전이 검사
	for (const auto& trans : m_Controller->anyStateTransitions) {
		if (EvaluateConditions(trans.conditions)) {
			std::cout << "AnyStateTransition "
				<< m_Controller->states[m_CurrentStateIndex].name << " -> " 
				<< trans.toState <<  " " << m_elapsedTime << std::endl;
			ChangeState(trans.toState);

			for (auto& condition : trans.conditions) {
				if (condition.type == ParameterType::Trigger) {
					// 트리거는 한번만 평가되므로 Reset
					ResetTrigger(condition.parameter);
				}
			}
			return;
		}
	}
}

void AnimatorInstance::OnStart()
{
	if (m_Controller) 
		ChangeState(m_Controller->defaultState); // 기본 상태로 전이
}

void AnimatorInstance::OnExit()
{

}


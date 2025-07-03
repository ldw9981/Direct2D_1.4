#include <unordered_map>
#include <vector>
#include <string>
#include <assert.h>
#include <iostream>

#include "AnimatorInstance.h"
#include "AnimatorController.h"

void AnimatorInstance::RegisterParameter(const std::string& name, ParameterType type)
{
	m_ParameterTypes[name] = type;
}

void AnimatorInstance::SetInt(const std::string& name, int value)
{
	m_IntParameters[name] = value;
}

void AnimatorInstance::SetFloat(const std::string& name, float value)
{
	m_FloatParameters[name] = value;
}

void AnimatorInstance::SetBool(const std::string& name, bool value)
{
	m_BoolParameters[name] = value;
}

int AnimatorInstance::GetInt(const std::string& name) const
{
	auto it = m_IntParameters.find(name);
	assert(it != m_IntParameters.end());
	if (it != m_IntParameters.end()) {
		return it->second;
	}
	return 0; // �⺻��
}

bool AnimatorInstance::GetBool(const std::string& name) const
{
	auto it = m_BoolParameters.find(name);
	assert(it != m_BoolParameters.end());
	if (it != m_BoolParameters.end()) {
		return it->second;
	}
	return false; // �⺻��
}

float AnimatorInstance::GetFloat(const std::string& name) const
{
	auto it = m_FloatParameters.find(name);
	assert(it != m_FloatParameters.end());
	if (it != m_FloatParameters.end()) {
		return it->second;
	}
	return 0.0f; // �⺻��
}

bool AnimatorInstance::CheckCondition(const std::string& param, const std::string& mode, float threshold)
{
	auto it = m_ParameterTypes.find(param);
	if (it == m_ParameterTypes.end())
		return false;

	switch (it->second)
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
	default:
		return false;
	}
	return false;
}

bool AnimatorInstance::EvaluateConditions(const std::vector<Condition>& conditions)
{
	for (const auto& condition : conditions) {
		if (!CheckCondition(condition.parameter, condition.mode, condition.threshold)) {
			return false; // �ϳ��� ������ �������� ������ false
		}
	}
	return true; // ��� ������ �����ϸ� true
}

void AnimatorInstance::SetAnimatorController(AnimatorController* controller)
{
	m_Controller = controller;
	// Parameter �ʱ�ȭ
	m_FloatParameters.clear();
	m_IntParameters.clear();
	m_BoolParameters.clear();
	for (const auto& param : controller->parameters) 
	{
		if (param.type == "Int") {	
			RegisterParameter(param.name, ParameterType::Int);
			SetInt(param.name, param.defaultInt);
		}
		else if (param.type == "Float") {
			RegisterParameter(param.name, ParameterType::Float);
			SetFloat(param.name, param.defaultFloat);
		}
		else if (param.type == "Bool") {
			RegisterParameter(param.name, ParameterType::Bool);
			SetBool(param.name, param.defaultBool);
		}
	}

	// ���º��� �������̽� �ּҸ� ������ ������ Ȯ���Ѵ�.
	m_StateNotifiers.clear();
	for (auto& state : m_Controller->states) {	
		m_StateNotifiers.push_back(nullptr);
	}

	
	// ToDo: ��ü State�� ��ȸ�ϸ鼭 ����ϴ� AnimaitionClip�� ���̸� ������ 
	// State�� clipLength�� �����ؾ� �Ѵ�.

	
}

void AnimatorInstance::ChangeState(const std::string& stateName)
{
	assert(m_Controller != nullptr);
	
	State* pNext = m_Controller->GetState(stateName);
	if (pNext == nullptr || pNext->index == m_CurrentStateIndex )
		return;
	
	m_PrevStateIndex = m_CurrentStateIndex;
	m_CurrentStateIndex = pNext->index;	
	m_elapsedTime = 0.0f; // �� ���� �� �ð� �ʱ�ȭ

	if (m_PrevStateIndex != -1 && m_StateNotifiers[m_PrevStateIndex] != nullptr)
	{
		
		m_StateNotifiers[m_PrevStateIndex]->OnExitState(
			m_Controller->states[m_PrevStateIndex].name);
	}

	if (m_StateNotifiers[m_CurrentStateIndex] != nullptr) {
		m_StateNotifiers[m_CurrentStateIndex]->OnEnterState(
			m_Controller->states[m_CurrentStateIndex].name);
	}

	// ToDo:: ����̸����� ���� ����� Animation Clip ���� �����ؾ��մϴ�. 
	std::cout << "ChangeState " 
		<< m_Controller->states[m_CurrentStateIndex].motionName << std::endl;
}


void AnimatorInstance::Update(float deltaTime)
{
	if (m_CurrentStateIndex < 0)
		return;

	m_elapsedTime += deltaTime;
	// ��� ���� ��� 0.0~1.0
	float clipLength = m_Controller->states[m_CurrentStateIndex].clipLength;
	float elapsedRate = m_elapsedTime / clipLength;
	
	if (m_elapsedTime > clipLength && m_Controller->states[m_CurrentStateIndex].loop)
	{
		m_elapsedTime = fmod(m_elapsedTime, clipLength); // ������ �ð�
	}

	// Todo: SpriteRender�� ����ð��� �´� Sprite������ �����ؾ��մϴ�.	
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
		// �ش� Ʈ�������� exitTime�� ����ϴ� ��� ����� ���Ͽ� Ŭ���� �˻�   
		if (trans.hasExitTime && elapsedRate < trans.exitTime)
			continue;
		
		// false�̸� �Ź� ������ ��
		if (EvaluateConditions(trans.conditions)) {
			std::cout << "Transition "
				<< m_Controller->states[m_CurrentStateIndex].name << " -> "
				<< trans.toState << " " << m_elapsedTime << std::endl;
			ChangeState(trans.toState);
			return;
		}
	}

	// ���¿� ������� Any State ���� �˻�
	for (const auto& trans : m_Controller->anyStateTransitions) {
		if (EvaluateConditions(trans.conditions)) {
			std::cout << "AnyStateTransition "
				<< m_Controller->states[m_CurrentStateIndex].name << " -> " 
				<< trans.toState <<  " " << m_elapsedTime << std::endl;
			ChangeState(trans.toState);
			return;
		}
	}
}

void AnimatorInstance::OnStart()
{
	if (m_Controller) 
		ChangeState(m_Controller->defaultState); // �⺻ ���·� ����
}

void AnimatorInstance::OnExit()
{

}


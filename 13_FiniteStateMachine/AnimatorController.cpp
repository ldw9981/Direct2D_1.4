// Copyright (c) 2025 Dongwon Lee. All rights reserved.

#include <string>
#include <vector>
#include <fstream>
#include "json.hpp"
#include <stdexcept>
#include "AnimatorController.h"


using json = nlohmann::json;


void from_json(const nlohmann::json& in, Parameter& out) {
	in.at("name").get_to(out.name);
	in.at("type").get_to(out.type);
	in.at("defaultFloat").get_to(out.defaultFloat);
	in.at("defaultInt").get_to(out.defaultInt);
	in.at("defaultBool").get_to(out.defaultBool);
}

void from_json(const nlohmann::json& in, Condition& out) {
	in.at("parameter").get_to(out.parameter);
	in.at("mode").get_to(out.mode);
	in.at("threshold").get_to(out.threshold);
}

void from_json(const nlohmann::json& in, Transition& out) {
	in.at("fromState").get_to(out.fromState);
	in.at("toState").get_to(out.toState);

	if (in.contains("conditions")) {
		out.conditions = in.at("conditions").get<std::vector<Condition>>();
	}
	in.at("exitTime").get_to(out.exitTime);	
	in.at("hasExitTime").get_to(out.hasExitTime);
}

void from_json(const nlohmann::json& in, AnyStateTransition& out) {
	in.at("toState").get_to(out.toState);	
	if (in.contains("conditions")) {
		out.conditions = in.at("conditions").get<std::vector<Condition>>();
	}
}

void from_json(const nlohmann::json& in, State& out) {
	in.at("name").get_to(out.name);
	in.at("motionName").get_to(out.motionName);
	in.at("clipLength").get_to(out.clipLength);
	in.at("loop").get_to(out.loop);
	if (in.contains("transitions")) {
		out.transitions = in.at("transitions").get<std::vector<Transition>>();
	}
}

void from_json(const nlohmann::json& in, AnimatorController& out) {
	in.at("controllerName").get_to(out.controllerName);
	out.parameters = in.at("parameters").get<std::vector<Parameter>>();
	
	// 첫번째 Layer의 상태머신만 처리
	in.at("defaultState").get_to(out.defaultState);
	if (in.contains("states")) {
		out.states = in.at("states").get<std::vector<State>>();
	}
	if (in.contains("anyStateTransitions")) {
		out.anyStateTransitions = in.at("anyStateTransitions").get<std::vector<AnyStateTransition>>();
	}
}

ParameterType StringToParameterType(const std::string& type)
{
	if (type == "Int") return ParameterType::Int;
	else if (type == "Float") return ParameterType::Float;
	else if (type == "Bool") return ParameterType::Bool;
	else if (type == "Trigger") return ParameterType::Trigger;
	return ParameterType::Int; // 기본값
}

void LoadAnimatorController(const std::string& filePath, AnimatorController& out)
{
	std::ifstream inFile(filePath);
	if (inFile.is_open()) {
		json in;
		inFile >> in;
		inFile.close();
		// JSON 객체에서 데이터 추출
		out.controllerName = in["controllerName"];
		out.parameters = in["parameters"].get<std::vector<Parameter>>();
				
		for (const auto& param : out.parameters) {
			// key:파라메터이름, value: type  추가 
			out.paramTypes[param.name] = StringToParameterType(param.type);
		}

		out.defaultState = in["defaultState"];
		if (in.contains("states")) {
			out.states = in["states"].get<std::vector<State>>();						
		}

		
		for (int i = 0; i < out.states.size(); ++i) {

			// 상태이름으로 Index 생성
			out.stateNameToIndex[out.states[i].name] = i;
			out.states[i].index = i; // 상태 인덱스 설정

			for (auto& trans : out.states[i].transitions)	{
				for (auto& condi : trans.conditions)	{
					// 파라메터이름 -> type 변환
					condi.type = out.paramTypes[condi.parameter];
				}
			}			
		}
	
		if (in.contains("anyStateTransitions")) {
			out.anyStateTransitions = in["anyStateTransitions"].get<std::vector<AnyStateTransition>>();
		
			for (auto& trans : out.anyStateTransitions) {
				for (auto& condi : trans.conditions) {
					// 파라메터이름 -> type 변환
					condi.type = out.paramTypes[condi.parameter];
				}
			}
		}	
	}
	else {
		throw std::runtime_error("Failed to open animation clip file: " + filePath);
	}
}


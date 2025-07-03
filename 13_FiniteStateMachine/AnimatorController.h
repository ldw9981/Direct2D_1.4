#pragma once


struct Parameter
{
	std::string name; // �Ķ���� �̸�
	std::string type; // �Ķ���� Ÿ�� (��: Float, Int, Bool)
	float defaultFloat = 0.0f; // �⺻�� (Float Ÿ���� �� ���)
	int defaultInt = 0; // �⺻�� (Int Ÿ���� �� ���)
	bool defaultBool = false; // �⺻�� (Bool Ÿ���� �� ���)
};

struct Condition
{
	std::string parameter; // ���� �Ķ���� �̸�
	std::string mode; // ���� ��� (��: Greater, IfNot ��)
	float threshold = 0.0f; // ���� �Ӱ谪
};

struct Transition
{
	std::string fromState; // ���� ���� ���� �̸�
	std::string toState; // ���� ���� ���� �̸�	
	bool hasExitTime = false; // ���� �ð��� �ִ��� ����
	float exitTime = -1.0f; // 1.0�̸� 100% �Ϸ� �� ����, -1.0�̸� ���� �ð� ����
	std::vector<Condition> conditions;
};

struct AnyStateTransition
{
	std::string toState; // ���� ���� ���� �̸�
	std::vector<Condition> conditions;
};

struct State
{
	int index = -1; // ���� �ε��� (�ʿ�� ���)
	std::string name; // ���� �̸�
	std::string motionName; // ��� �̸� (�ִϸ��̼� Ŭ�� �̸�)
	float clipLength = 1.0f; //�� ���� Todo!!AnimationClipã�� ���� �ʿ�
	bool loop = false; // ���� ���� Todo!!AnimationClipã�� ���� �ʿ�
	std::vector<Transition> transitions; // ���� ���
};


struct AnimatorController
{
	std::string controllerName; // ��Ʈ�ѷ� �̸�
	std::vector<Parameter> parameters; // ��Ʈ�ѷ� �Ķ���� ���
	
	std::string defaultState; // �⺻ ���� �̸�	
	std::vector<State> states;
	std::unordered_map<std::string, int> stateNameToIndex;
	std::vector<AnyStateTransition> anyStateTransitions; // Any State ���� ���

	State* GetState(const std::string& stateName)
	{
		auto it = stateNameToIndex.find(stateName);
		if (it != stateNameToIndex.end()) {
			return &states[it->second]; // ���°� �����ϸ� �ش� ���� ��ȯ
		}
		return nullptr; // ���°� �������� ������ nullptr ��ȯ
	}
};

void LoadAnimatorController(const std::string& filePath, AnimatorController& clip);

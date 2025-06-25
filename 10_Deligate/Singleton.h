#pragma once
#include <cassert>

template <typename T>
class Singleton
{
public:
	// ������� �ν��Ͻ� ����
	static void Create()
	{
		if (!instance_)
			instance_ = new T();
	}

	// �ν��Ͻ� ��ȯ
	static T& Instance()
	{
		assert(instance_ && "Singleton not created! Call Create() first.");
		return *instance_;
	}

	// ������� �ν��Ͻ� �ı�
	static void Destroy()
	{
		delete instance_;
		instance_ = nullptr;
	}

protected:
	Singleton() = default;
	~Singleton() = default;

private:
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

	static T* instance_;
};

template <typename T>
T* Singleton<T>::instance_ = nullptr;



#include <iostream>
#include <fstream>
#include <string>
#include <windows.h> 

#include "json.hpp"

#include "AnimatorController.h"
#include "AnimatorInstance.h"

using json = nlohmann::json;

int main()
{
	AnimatorController animController;
	LoadAnimatorController("../Resource/Player_AnimController.json", animController);


	AnimatorInstance animInstance;
	animInstance.SetAnimatorController(&animController);

	animInstance.SetBool("grounded", true);
	animInstance.OnStart();

	float elapedTime = 0;
	float deltaTime = 0.16f; // 0.1초 단위로 업데이트
	float testTime = 60.0f;
	for (elapedTime = 0 ;  elapedTime < testTime; elapedTime += deltaTime)
	{
		Sleep(deltaTime*1000); // 대기 
		animInstance.Update(deltaTime); // 0.1초 단위로 업데이트
		
		if (GetAsyncKeyState('A') & 0x8000) { 
			animInstance.SetFloat("velocityX", 1.0f);
		}
		else 
		{
			animInstance.SetFloat("velocityX", 0.0f);
		}

		if (GetAsyncKeyState('H') & 0x8000) {
			animInstance.SetTrigger("hurt"); // Trigger테스트
		}
	}	

	animInstance.OnExit();
}
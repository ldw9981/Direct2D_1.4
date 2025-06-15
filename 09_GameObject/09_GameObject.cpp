// 09_GameObject.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include "GameObject.h"
#include "Component.h"
#include "MyComponent.h"
int main()
{
    std::cout << "Hello World!\n";


    MySystem system;
    
    // Scene에서 생성한다.
    GameObject* object = new GameObject;

    MyComponent* ret = object->AddComponent<MyComponent>(1); 

    // Scene에서 여러  싱글톤 시스템들을 Update한다.
    MySystem::Instance->Update();


    delete object;


}

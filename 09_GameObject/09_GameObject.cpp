// 09_GameObject.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include "GameObject.h"
#include "Component.h"
#include "MyComponent.h"
int main()
{
    std::cout << "Hello World!\n";

    GameObject test;

    MyComponent* ret = test.AddComponent<MyComponent>(1);
    ret->Update();\

}

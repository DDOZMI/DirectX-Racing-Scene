////////////////////////////////////////////////////////////////////////////////
// Filename: inputclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "inputclass.h"


InputClass::InputClass()
{
}


InputClass::InputClass(const InputClass& other)
{
}


InputClass::~InputClass()
{
}


void InputClass::Initialize()
{
	int i;
	

	// Initialize all the keys to being released and not pressed.
	for(i=0; i<256; i++)
	{
		m_keys[i] = false;
	}

	return;
}


void InputClass::KeyDown(unsigned int input)
{
	// If a key is pressed then save that state in the key array.
	//m_keys[input] = true;
	return;
}


void InputClass::KeyUp(unsigned int input)
{
	// If a key is released then clear that state in the key array.
	//m_keys[input] = false;
	return;
}


bool InputClass::IsKeyDown(unsigned int key)
{
	// Return what state the key is in (pressed/not pressed).
	//return m_keys[key];

	// Windows api 함수
	// 현재 시점에서의 특정 Virtual Key 상태를 직접 확인한다.
	// 키보드의 물리적 상태를 실시간으로 확인 가능.
	// 인자로는 확인하려는 key의 코드를 받는다.
	// SHORT 값을 반환, 0x8000: 현재 키가 눌려있으면 1, 아니면 0 반환
	return (GetAsyncKeyState(key) & 0x8000) != 0;
}
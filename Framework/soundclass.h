///////////////////////////////////////////////////////////////////////////////
// Filename: soundclass.h (FMOD Version)
///////////////////////////////////////////////////////////////////////////////
#ifndef _SOUNDCLASS_H_
#define _SOUNDCLASS_H_

//////////////
// INCLUDES //
//////////////
#include <windows.h>
#include <vector>
#include <string>
#include "fmod.hpp"
#include "fmod_errors.h"

///////////////////////////////////////////////////////////////////////////////
// Class name: SoundClass
///////////////////////////////////////////////////////////////////////////////
class SoundClass
{
public:
	// 사운드 타입 열거형
	enum SoundType
	{
		SOUND_ENGINE = 0 ,
		SOUND_CRASH ,
		MAX_SOUNDS
	};

private:
	struct SoundBuffer
	{
		FMOD::Sound* sound;
		FMOD::Channel* channel;
		std::string filename;
		bool isLoaded;
		bool isLooping;

		SoundBuffer() : sound(nullptr) , channel(nullptr) , isLoaded(false) , isLooping(false) {}
	};

public:
	SoundClass();
	SoundClass(const SoundClass&);
	~SoundClass();

	bool Initialize(HWND);
	void Shutdown();
	void Update(); // FMOD 시스템 업데이트 (매 프레임 호출 필요)

	// 특정 사운드 재생/정지
	bool PlaySound(SoundType soundType , bool loop = false);
	bool StopSound(SoundType soundType);
	bool IsPlaying(SoundType soundType) const;

	// 엔진 사운드 제어 (속도 기반)
	bool UpdateEngineSound(float currentSpeed , float maxSpeed , bool isAccelerating , float deltaTime);
	bool StartEngineSound();
	bool StopEngineSound();

	// 사운드 로딩 상태 확인
	bool IsSoundLoaded(SoundType soundType) const;

	// 볼륨 설정 (0.0f - 1.0f)
	bool SetVolume(SoundType soundType , float volume);
	bool SetMasterVolume(float volume);

	// 피치 설정 (0.5f - 2.0f, 1.0f = 원본)
	bool SetPitch(SoundType soundType , float pitch);

private:
	bool InitializeFMOD(HWND hwnd);
	void ShutdownFMOD();
	bool LoadAllSounds();
	bool LoadSound(const char* filename , FMOD::Sound** sound , bool loop = false);
	void CheckFMODError(FMOD_RESULT result , const char* context);

	// 엔진 사운드 계산 헬퍼 함수
	float CalculateEnginePitch(float currentSpeed , float maxSpeed) const;
	float CalculateEngineVolume(float currentSpeed , float maxSpeed) const;

private:
	FMOD::System* m_fmodSystem;
	std::vector<SoundBuffer> m_soundBuffers;

	// 엔진 사운드 제어 파라미터
	float m_engineIdlePitch;      // 공회전 피치 (0.5f)
	float m_engineMaxPitch;       // 최대 속도 피치 (2.0f)
	float m_engineIdleVolume;     // 공회전 볼륨 (0.3f)
	float m_engineMaxVolume;      // 최대 속도 볼륨 (1.0f)
	float m_pitchSmoothSpeed;     // 피치 변경 부드러움 (5.0f)
	float m_volumeSmoothSpeed;    // 볼륨 변경 부드러움 (3.0f)

	float m_currentEnginePitch;   // 현재 엔진 피치
	float m_targetEnginePitch;    // 목표 엔진 피치
	float m_currentEngineVolume;  // 현재 엔진 볼륨
	float m_targetEngineVolume;   // 목표 엔진 볼륨

	float m_masterVolume;         // 마스터 볼륨
};

#endif
///////////////////////////////////////////////////////////////////////////////
// Filename: soundclass.cpp (FMOD Version)
///////////////////////////////////////////////////////////////////////////////
#include "soundclass.h"
#include <algorithm>

SoundClass::SoundClass()
{
	m_fmodSystem = nullptr;

	// 사운드 버퍼 초기화
	m_soundBuffers.resize(MAX_SOUNDS);
	m_soundBuffers[ SOUND_ENGINE ].filename = "./data/car1.wav";
	m_soundBuffers[ SOUND_ENGINE ].isLooping = true;  // 엔진 사운드는 루프
	m_soundBuffers[ SOUND_CRASH ].filename = "./data/crash.wav";
	m_soundBuffers[ SOUND_CRASH ].isLooping = false;

	// 엔진 사운드 파라미터 초기화
	m_engineIdlePitch = 0.5f;
	m_engineMaxPitch = 2.5f;
	m_engineIdleVolume = 1.0f;
	m_engineMaxVolume = 2.0f;
	m_pitchSmoothSpeed = 5.0f;
	m_volumeSmoothSpeed = 3.0f;

	m_currentEnginePitch = m_engineIdlePitch;
	m_targetEnginePitch = m_engineIdlePitch;
	m_currentEngineVolume = m_engineIdleVolume;
	m_targetEngineVolume = m_engineIdleVolume;

	m_masterVolume = 0.8f;
}

SoundClass::SoundClass(const SoundClass& other)
{
}

SoundClass::~SoundClass()
{
}

bool SoundClass::Initialize(HWND hwnd)
{
	bool result;

	// FMOD 시스템 초기화
	result = InitializeFMOD(hwnd);
	if ( !result )
	{
		return false;
	}

	// 모든 사운드 로드
	result = LoadAllSounds();
	if ( !result )
	{
		// 일부 사운드 로딩 실패 시에도 계속 진행
		// 경고만 표시하고 진행
	}

	return true;
}

void SoundClass::Shutdown()
{
	// 모든 사운드 정지 및 해제
	for ( int i = 0; i < MAX_SOUNDS; i++ )
	{
		if ( m_soundBuffers[ i ].channel )
		{
			m_soundBuffers[ i ].channel->stop();
			m_soundBuffers[ i ].channel = nullptr;
		}

		if ( m_soundBuffers[ i ].sound )
		{
			m_soundBuffers[ i ].sound->release();
			m_soundBuffers[ i ].sound = nullptr;
		}
	}
	m_soundBuffers.clear();

	// FMOD 시스템 종료
	ShutdownFMOD();
}

void SoundClass::Update()
{
	if ( m_fmodSystem )
	{
		m_fmodSystem->update();
	}
}

bool SoundClass::InitializeFMOD(HWND hwnd)
{
	FMOD_RESULT result;

	// FMOD 시스템 생성
	result = FMOD::System_Create(&m_fmodSystem);
	if ( result != FMOD_OK )
	{
		CheckFMODError(result , "FMOD System_Create");
		return false;
	}

	// FMOD 버전 확인
	unsigned int version;
	result = m_fmodSystem->getVersion(&version);
	if ( result != FMOD_OK )
	{
		CheckFMODError(result , "FMOD getVersion");
		return false;
	}

	if ( version < FMOD_VERSION )
	{
		return false;
	}

	// FMOD 시스템 초기화 (32개 채널)
	result = m_fmodSystem->init(32 , FMOD_INIT_NORMAL , 0);
	if ( result != FMOD_OK )
	{
		CheckFMODError(result , "FMOD init");
		return false;
	}

	return true;
}

void SoundClass::ShutdownFMOD()
{
	if ( m_fmodSystem )
	{
		m_fmodSystem->close();
		m_fmodSystem->release();
		m_fmodSystem = nullptr;
	}
}

bool SoundClass::LoadAllSounds()
{
	bool allLoaded = true;

	for ( int i = 0; i < MAX_SOUNDS; i++ )
	{
		bool result = LoadSound(
			m_soundBuffers[ i ].filename.c_str() ,
			&m_soundBuffers[ i ].sound ,
			m_soundBuffers[ i ].isLooping
		);

		if ( result )
		{
			m_soundBuffers[ i ].isLoaded = true;
		}
		else
		{
			m_soundBuffers[ i ].isLoaded = false;
			allLoaded = false;
		}
	}

	return allLoaded;
}

bool SoundClass::LoadSound(const char* filename , FMOD::Sound** sound , bool loop)
{
	if ( !m_fmodSystem )
	{
		return false;
	}

	FMOD_RESULT result;
	FMOD_MODE mode = FMOD_DEFAULT;

	if ( loop )
	{
		mode |= FMOD_LOOP_NORMAL;
	}
	else
	{
		mode |= FMOD_LOOP_OFF;
	}

	result = m_fmodSystem->createSound(filename , mode , 0 , sound);
	if ( result != FMOD_OK )
	{
		CheckFMODError(result , "createSound");
		return false;
	}

	return true;
}

bool SoundClass::PlaySound(SoundType soundType , bool loop)
{
	if ( soundType < 0 || soundType >= MAX_SOUNDS )
	{
		return false;
	}

	if ( !m_soundBuffers[ soundType ].isLoaded || !m_soundBuffers[ soundType ].sound )
	{
		return false;
	}

	// 이미 재생 중이면 정지
	if ( m_soundBuffers[ soundType ].channel )
	{
		bool playing = false;
		m_soundBuffers[ soundType ].channel->isPlaying(&playing);
		if ( playing )
		{
			m_soundBuffers[ soundType ].channel->stop();
		}
	}

	// 사운드 재생
	FMOD_RESULT result = m_fmodSystem->playSound(
		m_soundBuffers[ soundType ].sound ,
		nullptr ,
		false ,
		&m_soundBuffers[ soundType ].channel
	);

	if ( result != FMOD_OK )
	{
		CheckFMODError(result , "playSound");
		return false;
	}

	// 볼륨 설정
	if ( m_soundBuffers[ soundType ].channel )
	{
		m_soundBuffers[ soundType ].channel->setVolume(m_masterVolume);
	}

	return true;
}

bool SoundClass::StopSound(SoundType soundType)
{
	if ( soundType < 0 || soundType >= MAX_SOUNDS )
	{
		return false;
	}

	if ( m_soundBuffers[ soundType ].channel )
	{
		FMOD_RESULT result = m_soundBuffers[ soundType ].channel->stop();
		m_soundBuffers[ soundType ].channel = nullptr;
		return ( result == FMOD_OK );
	}

	return true;
}

bool SoundClass::IsPlaying(SoundType soundType) const
{
	if ( soundType < 0 || soundType >= MAX_SOUNDS )
	{
		return false;
	}

	if ( m_soundBuffers[ soundType ].channel )
	{
		bool playing = false;
		m_soundBuffers[ soundType ].channel->isPlaying(&playing);
		return playing;
	}

	return false;
}

bool SoundClass::StartEngineSound()
{
	if ( !PlaySound(SOUND_ENGINE , true) )
	{
		return false;
	}

	// 초기 엔진 사운드 설정 (공회전 상태)
	m_currentEnginePitch = m_engineIdlePitch;
	m_targetEnginePitch = m_engineIdlePitch;
	m_currentEngineVolume = m_engineIdleVolume;
	m_targetEngineVolume = m_engineIdleVolume;

	if ( m_soundBuffers[ SOUND_ENGINE ].channel )
	{
		m_soundBuffers[ SOUND_ENGINE ].channel->setPitch(m_currentEnginePitch);
		m_soundBuffers[ SOUND_ENGINE ].channel->setVolume(m_currentEngineVolume * m_masterVolume);
	}

	return true;
}

bool SoundClass::StopEngineSound()
{
	return StopSound(SOUND_ENGINE);
}

bool SoundClass::UpdateEngineSound(float currentSpeed , float maxSpeed , bool isAccelerating , float deltaTime) // 수정된 코드
{
	if ( !IsPlaying(SOUND_ENGINE) || !m_soundBuffers[ SOUND_ENGINE ].channel )
	{
		return false;
	}

	// 속도 비율 계산 (0.0 ~ 1.0)
	float speedRatio = ( maxSpeed > 0.0f ) ? ( currentSpeed / maxSpeed ) : 0.0f;
	speedRatio = max(0.0f , min(1.0f , speedRatio));

	// 목표 피치 계산
	m_targetEnginePitch = CalculateEnginePitch(currentSpeed , maxSpeed);

	// 목표 볼륨 계산
	m_targetEngineVolume = CalculateEngineVolume(currentSpeed , maxSpeed);

	// 가속 중일 때는 더 빠르게 피치 증가
	float pitchSpeed = isAccelerating ? m_pitchSmoothSpeed * 1.5f : m_pitchSmoothSpeed;

	// 감속 중일 때는 피치를 더 빠르게 감소
	if ( !isAccelerating && currentSpeed < maxSpeed * 0.5f )
	{
		pitchSpeed *= 2.0f;
	}

	// 부드럽게 피치 변경 (선형 보간)
	//float deltaTime = 0.016f; // 약 60 FPS 가정
	m_currentEnginePitch += ( m_targetEnginePitch - m_currentEnginePitch ) * pitchSpeed * deltaTime;
	m_currentEngineVolume += ( m_targetEngineVolume - m_currentEngineVolume ) * m_volumeSmoothSpeed * deltaTime;

	// 범위 제한
	m_currentEnginePitch = max(m_engineIdlePitch , min(m_engineMaxPitch , m_currentEnginePitch));
	m_currentEngineVolume = max(m_engineIdleVolume , min(m_engineMaxVolume , m_currentEngineVolume));

	// FMOD에 적용
	FMOD_RESULT result;
	result = m_soundBuffers[ SOUND_ENGINE ].channel->setPitch(m_currentEnginePitch);
	if ( result != FMOD_OK )
	{
		return false;
	}

	result = m_soundBuffers[ SOUND_ENGINE ].channel->setVolume(m_currentEngineVolume * m_masterVolume);
	if ( result != FMOD_OK )
	{
		return false;
	}

	return true;
}

float SoundClass::CalculateEnginePitch(float currentSpeed , float maxSpeed) const
{
	if ( maxSpeed <= 0.0f )
	{
		return m_engineIdlePitch;
	}

	// 속도 비율 (0.0 ~ 1.0)
	float speedRatio = currentSpeed / maxSpeed;
	speedRatio = max(0.0f , min(1.0f , speedRatio));

	// 비선형 곡선을 사용하여 더 자연스러운 엔진 사운드
	// 저속에서는 천천히 증가, 고속에서는 빠르게 증가
	float pitchCurve = speedRatio * speedRatio * 0.5f + speedRatio * 0.5f;

	// 공회전(idle)과 최대 피치 사이를 보간
	float pitch = m_engineIdlePitch + ( m_engineMaxPitch - m_engineIdlePitch ) * pitchCurve;

	return pitch;
}

float SoundClass::CalculateEngineVolume(float currentSpeed , float maxSpeed) const
{
	if ( maxSpeed <= 0.0f )
	{
		return m_engineIdleVolume;
	}

	// 속도 비율 (0.0 ~ 1.0)
	float speedRatio = currentSpeed / maxSpeed;
	speedRatio = max(0.0f , min(1.0f , speedRatio));

	// 선형 보간
	float volume = m_engineIdleVolume + ( m_engineMaxVolume - m_engineIdleVolume ) * speedRatio;

	return volume;
}

bool SoundClass::IsSoundLoaded(SoundType soundType) const
{
	if ( soundType < 0 || soundType >= MAX_SOUNDS )
	{
		return false;
	}

	return m_soundBuffers[ soundType ].isLoaded;
}

bool SoundClass::SetVolume(SoundType soundType , float volume)
{
	if ( soundType < 0 || soundType >= MAX_SOUNDS )
	{
		return false;
	}

	if ( !m_soundBuffers[ soundType ].channel )
	{
		return false;
	}

	// 볼륨 범위 제한 (0.0 ~ 1.0)
	volume = max(0.0f , min(1.0f , volume));

	FMOD_RESULT result = m_soundBuffers[ soundType ].channel->setVolume(volume * m_masterVolume);
	return ( result == FMOD_OK );
}

bool SoundClass::SetMasterVolume(float volume)
{
	// 볼륨 범위 제한 (0.0 ~ 1.0)
	m_masterVolume = max(0.0f , min(1.0f , volume));

	// 모든 현재 재생 중인 사운드의 볼륨 업데이트
	for ( int i = 0; i < MAX_SOUNDS; i++ )
	{
		if ( m_soundBuffers[ i ].channel )
		{
			bool playing = false;
			m_soundBuffers[ i ].channel->isPlaying(&playing);
			if ( playing )
			{
				float channelVolume;
				m_soundBuffers[ i ].channel->getVolume(&channelVolume);
				m_soundBuffers[ i ].channel->setVolume(channelVolume);
			}
		}
	}

	return true;
}

bool SoundClass::SetPitch(SoundType soundType , float pitch)
{
	if ( soundType < 0 || soundType >= MAX_SOUNDS )
	{
		return false;
	}

	if ( !m_soundBuffers[ soundType ].channel )
	{
		return false;
	}

	// 피치 범위 제한 (0.5 ~ 2.0)
	pitch = max(0.5f , min(2.0f , pitch));

	FMOD_RESULT result = m_soundBuffers[ soundType ].channel->setPitch(pitch);
	return ( result == FMOD_OK );
}

void SoundClass::CheckFMODError(FMOD_RESULT result , const char* context)
{
	if ( result != FMOD_OK )
	{
		// 에러 로깅 (실제 구현에서는 로깅 시스템 사용)
		// printf("FMOD error! (%d) %s - %s\n", result, FMOD_ErrorString(result), context);
	}
}
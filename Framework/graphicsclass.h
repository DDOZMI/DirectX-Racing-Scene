#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>

#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "textureshaderclass.h"
#include "fpsclass.h"
#include "cpuclass.h"
#include "fontclass.h"
#include "lightclass.h"
#include "jsonSettingsLoader.h"
#include "inputclass.h"
#include "pickingclass.h"
#include "driveclass.h"
#include "timerclass.h"

constexpr bool	FULL_SCREEN = false;
constexpr bool	VSYNC_ENABLED = true;
constexpr float SCREEN_DEPTH = 1000.0f;
constexpr float SCREEN_NEAR = 0.1f;

class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame(InputClass*);

	void SetRenderMode(bool);
	void SetBackFaceCulling(bool);
	void SetTextVisibility();
	void SetTextureFilter(int) const;

	CameraClass* GetCamera() const;

	void CycleLightingMode(int) const;

	void ToggleAmbientLight();
	void ToggleDiffuseLight();
	void ToggleSpecularLight();

	bool IsLoadingComplete() const;

private:
	bool Render(float, float);
	bool loadModel(int);
	void SetupLighting() const;
	bool LoadWithThreadPool(HWND);

	// 운전 관련 함수
	void ProcessDriving(InputClass* input, float deltaTime) const;
	void HandleCarPicking(InputClass* input);
	XMFLOAT3 GetCarInitialPosition(int carIndex) const;
	XMFLOAT3 GetCarInitialRotation(int carIndex) const;

	// 로딩 화면 함수
	bool RenderLoadingScreen() const;
	void RenderLoadingText(float progress) const;

private:
	static constexpr unsigned int NUM_OF_MODELS = 17; // 사용하는 총 모델의 개수

	// 클래스 포인터 //
	D3DClass* m_D3D;
	CameraClass* m_Camera;
	TextureShaderClass* m_TextureShader;
	CpuClass* m_Cpu;
	FpsClass* m_Fps;
	FontClass* m_Font;
	LightClass* m_Light;
	JsonSettingsLoader* m_JsonLoader;
	TimerClass* m_Timer;
	DriveClass* m_Drive;
	PickingClass* m_Picking;

	// 모델, 텍스쳐 컨테이너 //
	std::vector<ModelClass*> m_Model;
	std::vector<ModelConfig> m_modelConfigs;
	std::vector<std::vector<const WCHAR*>> textureFilenames;
	std::vector<std::vector<ModelClass::InstanceType>> instanceInfo;
	std::vector<unsigned int> instanceCount;
	std::vector<std::string> m_modelNames;

	// 변경 가능한 설정 변수 //
	bool m_backFaceCulling;
	bool m_wireframeMode;
	bool m_showText;
	bool m_ambientEnabled;
	bool m_diffuseEnabled;
	bool m_specularEnabled;

	// 멀티쓰레드 관련 변수 //
	std::mutex m_modelMutex;
	std::atomic<unsigned int> m_totalPolygons;
	std::atomic<unsigned int> m_objectCount;

	// 운전 관련 변수들
	bool m_previousPickingState;
	XMFLOAT3 m_originalCameraPos;
	XMFLOAT3 m_originalCameraRot;

	// 로딩 화면 관련 변수들
	std::atomic<int> m_loadedModels;
	std::atomic<bool> m_isLoading;
	bool m_loadingComplete;
};

#endif
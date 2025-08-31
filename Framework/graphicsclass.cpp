#pragma warning(disable: 4018) 
#include "graphicsclass.h"


GraphicsClass::GraphicsClass()
{
	m_D3D = nullptr;
	m_Camera = nullptr;
	m_TextureShader = nullptr;
	m_Cpu = nullptr;
	m_Fps = nullptr;
	m_Font = nullptr;
	m_Light = nullptr;
	m_JsonLoader = nullptr;
	m_Drive = nullptr;
	m_Timer = nullptr;

	m_Model.resize(NUM_OF_MODELS);
	textureFilenames.resize(NUM_OF_MODELS);
	instanceInfo.resize(NUM_OF_MODELS);
	instanceCount.resize(NUM_OF_MODELS, 1);

	m_totalPolygons = 0;
	m_objectCount = 0;
	m_loadedModels = 0;

	m_backFaceCulling = true;
	m_wireframeMode = false;

	m_ambientEnabled = true;
	m_diffuseEnabled = true;
	m_specularEnabled = true;

	m_showText = true;

	m_isLoading = true;
	m_loadingComplete = false;

	// 운전 관련 초기화
	m_previousPickingState = false;
	m_originalCameraPos = XMFLOAT3(20.0f, 20.0f, -130.0f);
	m_originalCameraRot = XMFLOAT3(0.0f, 0.0f, 0.0f);
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;

	// JSON 로더 생성
	m_JsonLoader = new JsonSettingsLoader;
	if (!m_JsonLoader)
	{
		return false;
	}

	// JSON 설정 파일 로드
	result = m_JsonLoader->LoadModelsConfig("./data/Settings.json", m_modelConfigs);
	if (!result)
	{
		MessageBox(hwnd, L"Could not load models configuration file.", L"Error", MB_OK);
		return false;
	}

	// Create the Direct3D object.
	m_D3D = new D3DClass;
	if (!m_D3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;
	if (!m_Camera)
	{
		return false;
	}

	// Set the initial position of the camera.
	m_Camera->SetPosition(20.0f, 20.0f, -130.0f);

	m_Font = new FontClass;
	if (!m_Font)
	{
		return false;
	}
	result = m_Font->Initialize(m_D3D->GetDevice(), m_D3D, hwnd, screenWidth, screenHeight);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the font object.", L"Error", MB_OK);
		return false;
	}

	for (int i = 0; i < NUM_OF_MODELS; i++)
	{
		m_Model[i] = new ModelClass();
		if (!m_Model[i])
		{
			return false;
		}
	}

	m_isLoading = true;
	m_loadedModels = 0;

	// Load Models using Thread Pooling.
	std::thread loadingThread(
		[this, hwnd]() 
		{
			bool loadResult = this->LoadWithThreadPool(hwnd);
			if (loadResult) 
			{
				this->m_loadingComplete = true;
			}
			this->m_isLoading = false;
		}
	);
	loadingThread.detach();

	// Create the color shader object.
	m_TextureShader = new TextureShaderClass;
	if (!m_TextureShader)
	{
		return false;
	}

	// Initialize the texture shader object.
	result = m_TextureShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	m_Light = new LightClass;
	if (!m_Light)
	{
		return false;
	}
	SetupLighting();

	m_Fps = new FpsClass;
	if (!m_Fps)
	{
		return false;
	}
	m_Fps->Initialize();

	m_Cpu = new CpuClass;
	if (!m_Cpu)
	{
		return false;
	}
	m_Cpu->Initialize();

	m_Picking = new PickingClass;
	if (!m_Picking)
	{
		return false;
	}

	result = m_Picking->Initialize(screenWidth, screenHeight);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the picking object.", L"Error", MB_OK);
		return false;
	}

	// 타이머 클래스 초기화 추가
	m_Timer = new TimerClass;
	if (!m_Timer)
	{
		return false;
	}
	result = m_Timer->Initialize();
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the timer object.", L"Error", MB_OK);
		return false;
	}

	// 운전 클래스 초기화 추가
	m_Drive = new DriveClass;
	if (!m_Drive)
	{
		return false;
	}

	result = m_Drive->Initialize();
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the drive object.", L"Error", MB_OK);
		return false;
	}

	m_modelNames.resize(NUM_OF_MODELS);
	for (int i = 0; i < m_modelConfigs.size() && i < NUM_OF_MODELS; i++)
	{
		m_modelNames[i] = m_modelConfigs[i].name;
	}

	return true;
}


void GraphicsClass::Shutdown()
{
	// Release the texture shader object.
	if(m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = nullptr;
	}

	for (auto& model : m_Model)
	{
		if (model)
		{
			model->Shutdown();
			delete model;
			model = nullptr;
		}
	}
	m_Model.clear();
	textureFilenames.clear();
	instanceInfo.clear();
	instanceCount.clear();

	// Release the camera object.
	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = nullptr;
	}

	// Release the D3D object.
	if(m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = nullptr;
	}

	if (m_Light)
	{
		delete m_Light;
		m_Light = nullptr;
	}

	if (m_Fps)
	{
		delete m_Fps;
		m_Fps = nullptr;
	}

	if (m_Cpu)
	{
		m_Cpu->Shutdown();
		delete m_Cpu;
		m_Cpu = nullptr;
	}

	if (m_Font)
	{
		m_Font->Shutdown();
		delete m_Font;
		m_Font = nullptr;
	}

	if (m_JsonLoader)
	{
		delete m_JsonLoader;
		m_JsonLoader = nullptr;
	}

	if (m_Picking)
	{
		m_Picking->Shutdown();
		delete m_Picking;
		m_Picking = nullptr;
	}

	if (m_Drive)
	{
		m_Drive->Shutdown();
		delete m_Drive;
		m_Drive = nullptr;
	}

	if (m_Timer)
	{
		delete m_Timer;
		m_Timer = nullptr;
	}

	return;
}

bool GraphicsClass::LoadWithThreadPool(HWND hwnd)
{
	std::mutex mtx;
	std::atomic<int> modelsFailed(0);
	std::condition_variable cv;
	std::atomic<int> remainingTasks(NUM_OF_MODELS);

	// 멀티 스레드 풀 설정
	std::queue<int> taskQueue;
	const int threadCount = std::thread::hardware_concurrency();
	std::vector<std::thread> threadPool(threadCount);
	std::atomic<bool> stopThreads(false);

	// 모든 작업을 큐에 追가
	for (int i = 0; i < NUM_OF_MODELS; i++) 
	{
		taskQueue.push(i);
	}

	// 워커 함수 정의
	auto workerFunction = [this, &taskQueue, &mtx, &cv, &stopThreads, &modelsFailed,
		&remainingTasks, hwnd]()
		{
			while (!stopThreads)
			{
				int modelIndex;

				// 작업 가져오기
				{
					std::unique_lock<std::mutex> lock(mtx);
					if (!taskQueue.empty())
					{
						modelIndex = taskQueue.front();
						taskQueue.pop();
					}
					else {
						// 더 이상 작업이 없으면 조건 변수 대기
						cv.wait_for(lock, std::chrono::milliseconds(100),
							[&stopThreads, &taskQueue]()
							{
								return stopThreads || !taskQueue.empty();
							});
						continue;
					}
				}

				if (modelIndex >= 0)
				{
					// 모델 로딩 작업 수행
					bool loadResult = loadModel(modelIndex);

					if (!loadResult)
					{
						++modelsFailed;
					}

					// 남은 작업 수 감소
					if (--remainingTasks == 0)
					{
						// 모든 작업 완료 시 조건 변수 알림
						cv.notify_all();
					}
				}
			}
		};

	// 스레드 풀 시작
	for (int i = 0; i < threadCount; i++)
	{
		threadPool[i] = std::thread(workerFunction);
	}

	// 모든 작업이 완료될 때까지 대기
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [&remainingTasks]() { return remainingTasks == 0; });
	}

	// 스레드 풀 종료
	stopThreads = true;
	cv.notify_all();
	for (auto& t : threadPool)
	{
		if (t.joinable())
		{
			t.join();
		}
	}

	// 모델 로딩 실패 확인
	if (modelsFailed > 0)
	{
		MessageBox(hwnd, L"One or more models failed to initialize.", L"Error", MB_OK);
		return false;
	}

	return true;
}


bool GraphicsClass::loadModel(int idx)
{
	bool result = true;

	// JSON에서 로드된 설정이 있는지 확인
	if (idx >= m_modelConfigs.size())
	{
		return false;
	}

	const ModelConfig& config = m_modelConfigs[idx];

	// 기존 벡터에 설정 복사
	textureFilenames[idx] = config.textureFiles;
	instanceInfo[idx] = config.instances;
	instanceCount[idx] = config.instanceCount;

	// 모델 초기화
	result = m_Model[idx]->Initialize(m_D3D->GetDevice(),
		config.modelFile.c_str(),
		textureFilenames[idx],
		instanceInfo[idx],
		config.instanceCount);

	if (result)
	{
		m_objectCount.fetch_add(1, memory_order_relaxed);
		m_totalPolygons.fetch_add(static_cast<unsigned int>(m_Model[idx]->GetIndexCount()) / 3, memory_order_relaxed);
		m_loadedModels.fetch_add(1, memory_order_relaxed);
	}

	return result;
}


void GraphicsClass::SetBackFaceCulling(bool enable)
{
	m_backFaceCulling = enable;
	if (m_D3D)
	{
		m_D3D->SetBackFaceCulling(enable);
	}
}

void GraphicsClass::SetRenderMode(bool wireframe)
{
	m_wireframeMode = wireframe;

	if (m_D3D)
	{
		m_D3D->SetRenderMode(wireframe);
	}
}

void GraphicsClass::ToggleAmbientLight()
{
	m_ambientEnabled = !m_ambientEnabled;
}

void GraphicsClass::ToggleDiffuseLight()
{
	m_diffuseEnabled = !m_diffuseEnabled;
}

void GraphicsClass::ToggleSpecularLight()
{
	m_specularEnabled = !m_specularEnabled;
}

void GraphicsClass::SetTextVisibility()
{
	m_showText = !m_showText;
}

void GraphicsClass::SetTextureFilter(int filterMode) const
{
	m_TextureShader->SetTextureFilter(m_D3D->GetDevice(), filterMode);
}

CameraClass* GraphicsClass::GetCamera() const
{
	return m_Camera;
}

void GraphicsClass::SetupLighting() const
{
	CycleLightingMode(0);

	// 신호등 Point Light 1
	m_Light->SetPointLightPosition(0, 169.0f, 11.4f, -107.8f);
	m_Light->SetPointLightColor(0, 1.0f, 0.0f, 0.0f, 30.0f);  // 빨간불
	m_Light->SetPointLightRange(0, 8.0f);

	// 신호등 Point Light 2
	m_Light->SetPointLightPosition(1, 198.8f, 10.8f, -107.8f);
	m_Light->SetPointLightColor(1, 1.0f, 1.0f, 0.0f, 30.0f);  // 노란불
	m_Light->SetPointLightRange(1, 8.0f);

	// 신호등 Point Light 3
	m_Light->SetPointLightPosition(2, 228.8f, 10.2f, -107.8f);
	m_Light->SetPointLightColor(2, 0.0f, 1.0f, 0.0f, 30.0f);  // 초록불
	m_Light->SetPointLightRange(2, 8.0f);
}

void GraphicsClass::CycleLightingMode(int lightingMode) const
{
	switch (lightingMode)
	{
	case 0: // 낮
		m_Light->SetAmbientColor(0.4f, 0.4f, 0.4f, 1.0f);
		m_Light->SetDiffuseColor(1.0f, 0.95f, 0.8f, 1.0f);
		m_Light->SetDirection(0.2f, -0.9f, 0.2f);
		break;

	case 1: // 해질녁
		m_Light->SetAmbientColor(0.2f, 0.2f, 0.2f, 1.0f);
		m_Light->SetDiffuseColor(1.0f, 0.7f, 0.4f, 1.0f);
		m_Light->SetDirection(0.8f, -0.3f, 0.2f);
		break;

	case 2: // 야간
		m_Light->SetAmbientColor(0.05f, 0.05f, 0.1f, 1.0f);
		m_Light->SetDiffuseColor(0.3f, 0.2f, 0.5f, 1.0f);
		m_Light->SetDirection(0.0f, -1.0f, 0.0f);
		break;
	}

	m_Light->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetSpecularPower(64.0f);
}

bool GraphicsClass::IsLoadingComplete() const
{
	return m_loadingComplete && !m_isLoading;
}

bool GraphicsClass::RenderLoadingScreen() const
{
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	float progress = static_cast<float>(m_loadedModels.load()) / static_cast<float>(NUM_OF_MODELS);
	progress = std::min(progress, 1.0f);

	if (m_Font)
	{
		RenderLoadingText(progress);
	}

	m_D3D->EndScene();

	return true;
}

void GraphicsClass::RenderLoadingText(float progress) const
{
	if (m_Font)
	{
		m_Font->RenderLoadingScreen(progress);
	}
}


bool GraphicsClass::Frame(InputClass* input)
{
	static float rotation = 0.0f;

	m_Timer->Frame();
	float frameTime = m_Timer->GetTime() / 1000.0f; // convert to sec from milisec

	// frame limit for speed calculate (60fps)
	frameTime = std::min(frameTime, 0.0167f);

	const XMFLOAT3 cameraRotation = m_Camera->GetRotation();
	float normalRotation = cameraRotation.y + XM_PI; // normal rotation value against camera

	// normalRotation을 0~2π 범위로 정규화
	while (normalRotation > 2.0f * XM_PI)
	{
		normalRotation -= 2.0f * XM_PI;
	}
	while (normalRotation < 0.0f)
	{
		normalRotation += 2.0f * XM_PI;
	}

	rotation += static_cast<float>(XM_PI) * 0.01f;
	if (rotation > 360.0f)
	{
		rotation -= 360.0f;
	}

	const bool leftMousePressed = input->IsKeyDown(VK_LBUTTON);
	const bool rightMousePressed = input->IsKeyDown(VK_RBUTTON);

	if (m_isLoading || !m_loadingComplete)
	{
		return RenderLoadingScreen();
	}

	// Additional matrix system for driving with car models //
	XMMATRIX worldMatrix[NUM_OF_MODELS];
	for (int i = 0; i < NUM_OF_MODELS; i++)
	{
		m_D3D->GetWorldMatrix(worldMatrix[i]);
	}

	// World matrix setting by json settings
	for (int i = 0; i < NUM_OF_MODELS && i < m_modelConfigs.size(); i++)
	{
		const ModelConfig& config = m_modelConfigs[i];

		// Check the model is driving.
		if (m_Drive->IsDriving() && m_Drive->GetDrivingCarIndex() == i)
		{
			// car models
			XMFLOAT3 carScale = XMFLOAT3(config.worldMatrix.scale.x,
				config.worldMatrix.scale.y,
				config.worldMatrix.scale.z);
			worldMatrix[i] = m_Drive->GetCarWorldMatrix(carScale);
		}
		else
		{
			// other models
			worldMatrix[i] =
				XMMatrixScaling(config.worldMatrix.scale.x, config.worldMatrix.scale.y, config.worldMatrix.scale.z) *
				XMMatrixRotationX(config.worldMatrix.rotation.x) *
				XMMatrixRotationY(config.worldMatrix.rotation.y) *
				XMMatrixRotationZ(config.worldMatrix.rotation.z) *
				XMMatrixTranslation(config.worldMatrix.translation.x, config.worldMatrix.translation.y, config.worldMatrix.translation.z);
		}
	}

	// Select car (picking)
	HandleCarPicking(input);

	// Basic picking mode while not driving
	if (!m_Drive->IsDriving())
	{
		std::vector<XMMATRIX> worldMatrixVector(worldMatrix, worldMatrix + NUM_OF_MODELS);
		m_Picking->ProcessPicking(m_Camera, m_Model, worldMatrixVector, m_modelNames,
			leftMousePressed, rightMousePressed);
	}
	else
	{
		// Enable deselect while car is driving
		std::vector<XMMATRIX> worldMatrixVector(worldMatrix, worldMatrix + NUM_OF_MODELS);
		m_Picking->ProcessPicking(m_Camera, m_Model, worldMatrixVector, m_modelNames,
			false, rightMousePressed);
	}

	// Update driving
	ProcessDriving(input, frameTime);

	m_Fps->Frame();
	m_Cpu->Frame();

	if (!Render(rotation, normalRotation))
	{
		return false;
	}

	return true;
}


bool GraphicsClass::Render(float rotation, float normalRotation)
{
	XMMATRIX viewMatrix, projectionMatrix;
	XMMATRIX worldMatrix[NUM_OF_MODELS];
	bool result;

	// Clear the buffers to begin the scene.
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Camera->GetViewMatrix(viewMatrix);
	for (int i = 0; i < NUM_OF_MODELS; i++) { m_D3D->GetWorldMatrix(worldMatrix[i]); }
	m_D3D->GetProjectionMatrix(projectionMatrix);

	XMFLOAT3 cameraPos = m_Camera->GetPosition();

	// JSON에서 로드된 설정을 사용하여 월드 매트릭스 설정
	for (int i = 0; i < NUM_OF_MODELS && i < m_modelConfigs.size(); i++)
	{
		const ModelConfig& config = m_modelConfigs[i];

		// 운전 중인 차량인지 확인
		if (m_Drive->IsDriving() && m_Drive->GetDrivingCarIndex() == i)
		{
			// 운전 중인 차량은 DriveClass에서 제공하는 매트릭스 사용 (JSON 스케일 적용)
			XMFLOAT3 carScale = XMFLOAT3(config.worldMatrix.scale.x,
				config.worldMatrix.scale.y,
				config.worldMatrix.scale.z);
			worldMatrix[i] = m_Drive->GetCarWorldMatrix(carScale);
		}
		else
		{
			// 일반 모델들은 JSON 설정 사용
			worldMatrix[i] =
				XMMatrixScaling(config.worldMatrix.scale.x, config.worldMatrix.scale.y, config.worldMatrix.scale.z) *
				XMMatrixRotationX(config.worldMatrix.rotation.x) *
				XMMatrixRotationY(config.worldMatrix.rotation.y) *
				XMMatrixRotationZ(config.worldMatrix.rotation.z) *
				XMMatrixTranslation(config.worldMatrix.translation.x, config.worldMatrix.translation.y, config.worldMatrix.translation.z);
		}
	}

	m_D3D->TurnZBufferOff();

	// Sky sphere model (background) - 운전 중에도 월드 매트릭스 그대로 사용
	m_Model[NUM_OF_MODELS - 1]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[NUM_OF_MODELS - 1]->GetIndexCount(),
		instanceCount[NUM_OF_MODELS - 1], worldMatrix[NUM_OF_MODELS - 1], viewMatrix, projectionMatrix,
		m_Model[NUM_OF_MODELS - 1]->GetTexture(), 0, rotation, cameraPos, NUM_OF_MODELS - 1, m_Light, m_ambientEnabled, m_diffuseEnabled, m_specularEnabled);
	if (!result)
	{
		return false;
	}

	m_D3D->TurnZBufferOn();

	m_D3D->TurnOnAlphaBlending();

	for (int i = 0; i < NUM_OF_MODELS - 1; i++)
	{
		m_Model[i]->Render(m_D3D->GetDeviceContext());

		if (i == 11 || i == 12) // billboarding rotation (tree models)
		{
			result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[i]->GetIndexCount(), instanceCount[i],
				worldMatrix[i], viewMatrix, projectionMatrix, m_Model[i]->GetTexture(), 0,
				normalRotation, cameraPos, i, m_Light,
				m_ambientEnabled, m_diffuseEnabled, m_specularEnabled);
		}
		else
		{
			result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[i]->GetIndexCount(), instanceCount[i],
				worldMatrix[i], viewMatrix, projectionMatrix, m_Model[i]->GetTexture(), 0,
				rotation, cameraPos, i, m_Light,
				m_ambientEnabled, m_diffuseEnabled, m_specularEnabled);
		}

		if (!result)
		{
			return false;
		}
	}

	m_D3D->TurnOffAlphaBlending();

	if (m_showText)
	{
		m_Font->Render(static_cast<int>(m_totalPolygons),
			m_Fps->GetFps(),
			m_Cpu->GetCpuPercentage(),
			m_objectCount);

		m_Font->RenderTitle();

		if (m_Picking)
		{
			m_Font->RenderPickingStatus(m_Picking->GetPickedModelName(),
				m_Picking->IsPickingActive());
		}
	}

	// Present the rendered scene to the screen.
	m_D3D->EndScene();

	return true;
}

void GraphicsClass::ProcessDriving(InputClass* input, float deltaTime) const
{
	if (!m_Drive)
		return;

	// 운전 업데이트
	m_Drive->UpdateDriving(input, deltaTime);

	// 카메라 업데이트
	if (m_Drive->IsDriving())
	{
		m_Drive->UpdateCamera(m_Camera);
	}
}

void GraphicsClass::HandleCarPicking(InputClass* input)
{
	if (!m_Picking || !m_Drive)
		return;

	bool currentPickingState = m_Picking->IsPickingActive();

	// 피킹 상태가 활성화되었을 때 (차량이 선택됨)
	if (currentPickingState && !m_previousPickingState)
	{
		int pickedIndex = m_Picking->GetPickedModelIndex();

		// 차량 모델인지 확인 (인덱스 1~6)
		if (pickedIndex >= 1 && pickedIndex <= 6)
		{
			// 원래 카메라 위치/회전 저장
			m_originalCameraPos = m_Camera->GetPosition();
			m_originalCameraRot = m_Camera->GetRotation();

			// 차량의 초기 위치와 회전 가져오기
			XMFLOAT3 carPos = GetCarInitialPosition(pickedIndex);
			XMFLOAT3 carRot = GetCarInitialRotation(pickedIndex);

			// 운전 시작
			m_Drive->StartDriving(pickedIndex, carPos, carRot);
		}
	}
	// 피킹 상태가 비활성화되었을 때 (차량 선택 해제)
	else if (!currentPickingState && m_previousPickingState)
	{
		if (m_Drive->IsDriving())
		{
			// 운전 중지
			m_Drive->StopDriving();

			// 원래 카메라 위치/회전 복원
			m_Camera->SetPosition(m_originalCameraPos.x, m_originalCameraPos.y, m_originalCameraPos.z);
			m_Camera->SetRotation(m_originalCameraRot.x, m_originalCameraRot.y, m_originalCameraRot.z);
		}
	}

	m_previousPickingState = currentPickingState;
}

XMFLOAT3 GraphicsClass::GetCarInitialPosition(int carIndex) const
{
	if (carIndex >= 0 && carIndex < m_modelConfigs.size())
	{
		const auto& config = m_modelConfigs[carIndex];
		return XMFLOAT3(config.worldMatrix.translation.x,
			config.worldMatrix.translation.y,
			config.worldMatrix.translation.z);
	}
	return XMFLOAT3(0.0f, 0.0f, 0.0f);
}

XMFLOAT3 GraphicsClass::GetCarInitialRotation(int carIndex) const
{
	if (carIndex >= 0 && carIndex < m_modelConfigs.size())
	{
		const auto& config = m_modelConfigs[carIndex];
		return XMFLOAT3(config.worldMatrix.rotation.x,
			config.worldMatrix.rotation.y,
			config.worldMatrix.rotation.z);
	}
	return XMFLOAT3(0.0f, 0.0f, 0.0f);
}
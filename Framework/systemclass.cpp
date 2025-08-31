////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "systemclass.h"


SystemClass::SystemClass()
{
	m_Input = 0;
	m_Graphics = 0;
	m_Fps = 0;
	m_Cpu = 0;
	m_Timer = 0;
	m_firstMouse = true;
}


SystemClass::SystemClass(const SystemClass& other)
{
}


SystemClass::~SystemClass()
{
}


bool SystemClass::Initialize()
{
	int screenWidth, screenHeight;
	bool result;


	// Initialize the width and height of the screen to zero before sending the variables into the function.
	screenWidth = 0;
	screenHeight = 0;

	// Initialize the windows api.
	InitializeWindows(screenWidth, screenHeight);

	// Create the input object.  This object will be used to handle reading the keyboard input from the user.
	m_Input = new InputClass;
	if(!m_Input)
	{
		return false;
	}

	// Initialize the input object.
	m_Input->Initialize();

	// Create the graphics object.  This object will handle rendering all the graphics for this application.
	m_Graphics = new GraphicsClass;
	if(!m_Graphics)
	{
		return false;
	}

	// Initialize the graphics object.
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
	if(!result)
	{
		return false;
	}

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

	m_Timer = new TimerClass;
	if (!m_Timer)
	{
		return false;
	}

	result = m_Timer->Initialize();
	if (!result)
	{
		MessageBox(m_hwnd, L"Could not initialize the Timer object.", L"Error", MB_OK);
		return false;
	}
	
	return true;
}


void SystemClass::Shutdown()
{
	// Release the graphics object.
	if(m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	// Release the input object.
	if(m_Input)
	{
		delete m_Input;
		m_Input = 0;
	}

	if (m_Cpu)
	{
		delete m_Cpu;
		m_Cpu = nullptr;
	}

	if (m_Fps)
	{
		delete m_Fps;
		m_Fps = 0;
	}

	if (m_Timer)
	{
		delete m_Timer;
		m_Timer = 0;
	}

	// Shutdown the window.
	ShutdownWindows();
	
	return;
}


void SystemClass::Run()
{
	MSG msg;
	bool done, result;


	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));
	
	// Loop until there is a quit message from the window or the user.
	done = false;
	while(!done)
	{
		// Handle the windows messages.
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if(msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Otherwise do the frame processing.
			result = Frame();
			if(!result)
			{
				done = true;
			}
		}

	}

	return;
}


bool SystemClass::Frame()
{
	bool result;
	static bool cKeyPressed = false;	// culling toggle flag
	static bool tKeyPressed = false;	// text show toggle flag
	static bool backFaceCulling = true; // backface culling status
	static bool iKeyPressed = false;  // ambient light toggle flag
	static bool oKeyPressed = false;  // diffuse light toggle flag  
	static bool pKeyPressed = false;  // specular light toggle flag

	float deltaTime = m_Timer->GetTime();
	float cameraSpeed = 0.1f;

	if (m_Graphics && !m_Graphics->IsLoadingComplete())
	{
		// ESC 키로 종료만 허용
		if (m_Input->IsKeyDown(VK_ESCAPE))
		{
			return false;
		}

		// 로딩 화면만 렌더링
		bool result = m_Graphics->Frame(m_Input);
		return result;
	}

	// Check if the user pressed escape and wants to exit the application.
	if (m_Input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	//------------------------------------------------------------------
	HWND activeWindow = GetForegroundWindow();
	if (activeWindow == m_hwnd)
	{
		if (m_Input->IsKeyDown('W'))
		{
			m_Graphics->GetCamera()->MoveForward(cameraSpeed * deltaTime);
		}
		if (m_Input->IsKeyDown('S'))
		{
			m_Graphics->GetCamera()->MoveForward(-cameraSpeed * deltaTime);
		}
		if (m_Input->IsKeyDown('A'))
		{
			m_Graphics->GetCamera()->MoveRight(-cameraSpeed * deltaTime);
		}
		if (m_Input->IsKeyDown('D'))
		{
			m_Graphics->GetCamera()->MoveRight(cameraSpeed * deltaTime);
		}

		POINT currentMousePos;
		GetCursorPos(&currentMousePos);

		if (m_firstMouse)
		{
			m_lastMousePos = currentMousePos;
			m_firstMouse = false;
		}

		float deltaX = (float)(currentMousePos.x - m_lastMousePos.x);
		float deltaY = (float)(currentMousePos.y - m_lastMousePos.y);

		m_Graphics->GetCamera()->RotateByMouse(deltaX, deltaY);

		// Only center the cursor when the window is active
		RECT windowRect;
		GetWindowRect(m_hwnd, &windowRect);
		int centerX = (windowRect.right + windowRect.left) / 2;
		int centerY = (windowRect.bottom + windowRect.top) / 2;
		SetCursorPos(centerX, centerY);
		m_lastMousePos.x = centerX;
		m_lastMousePos.y = centerY;
	}
	//---------------------------------------------------------------------


	// Culling Mode Key
	if (m_Input->IsKeyDown('C'))
	{
		if (!cKeyPressed)
		{
			backFaceCulling = !backFaceCulling;
			m_Graphics->SetBackFaceCulling(backFaceCulling);
			cKeyPressed = true;
		}
	}
	else
	{
		cKeyPressed = false;
	}

	// Ambient toggle
	if (m_Input->IsKeyDown('I'))
	{
		if (!iKeyPressed)
		{
			m_Graphics->ToggleAmbientLight();
			iKeyPressed = true;
		}
	}
	else
	{
		iKeyPressed = false;
	}

	// Diffuse toggle
	if (m_Input->IsKeyDown('O'))
	{
		if (!oKeyPressed)
		{
			m_Graphics->ToggleDiffuseLight();
			oKeyPressed = true;
		}
	}
	else
	{
		oKeyPressed = false;
	}

	// Specular toggle
	if (m_Input->IsKeyDown('P'))
	{
		if (!pKeyPressed)
		{
			m_Graphics->ToggleSpecularLight();
			pKeyPressed = true;
		}
	}
	else
	{
		pKeyPressed = false;
	}


	// Text show key
	if (m_Input->IsKeyDown('T'))
	{
		if (!tKeyPressed)
		{
			m_Graphics->SetTextVisibility();
			tKeyPressed = true;
		}
	}
	else
	{
		tKeyPressed = false;
	}


	if (m_Input->IsKeyDown('1'))
	{
		m_Graphics->SetTextureFilter(0);
	}
	else if (m_Input->IsKeyDown('2'))
	{
		m_Graphics->SetTextureFilter(1);
	}
	else if (m_Input->IsKeyDown('3'))
	{
		m_Graphics->SetTextureFilter(2);
	}
	else if (m_Input->IsKeyDown('4'))
	{
		m_Graphics->SetRenderMode(true);
	}
	else if (m_Input->IsKeyDown('5'))
	{
		m_Graphics->SetRenderMode(false);
	}
	else if (m_Input->IsKeyDown('6'))
	{
		m_Graphics->CycleLightingMode(0);
	}
	else if (m_Input->IsKeyDown('7'))
	{
		m_Graphics->CycleLightingMode(1);
	}
	else if (m_Input->IsKeyDown('8'))
	{
		m_Graphics->CycleLightingMode(2);
	}


	m_Cpu->Frame();
	m_Timer->Frame();
	m_Fps->Frame();


	// Do the frame processing for the graphics object
	result = m_Graphics->Frame(m_Input);
	if (!result)
	{
		return false;
	}

	return true;
}


LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch(umsg)
	{
		// Check if a key has been pressed on the keyboard.
		case WM_KEYDOWN:
		{
			// If a key is pressed send it to the input object so it can record that state.
			m_Input->KeyDown((unsigned int)wparam);
			return 0;
		}

		// Check if a key has been released on the keyboard.
		case WM_KEYUP:
		{
			// If a key is released then send it to the input object so it can unset the state for that key.
			m_Input->KeyUp((unsigned int)wparam);
			return 0;
		}

		// Any other messages send to the default message handler as our application won't make use of them.
		default:
		{
			return DefWindowProc(hwnd, umsg, wparam, lparam);
		}
	}
}


void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;


	// Get an external pointer to this object.	
	ApplicationHandle = this;

	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	// Give the application a name.
	m_applicationName = L"Engine";

	// Setup the windows class with default settings.
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hinstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize        = sizeof(WNDCLASSEX);
	
	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	screenWidth  = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if(FULL_SCREEN)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth  = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;			
		dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		screenWidth  = 1600;
		screenHeight = 900;

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth)  / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, 
						    WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
						    posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Hide the mouse cursor.
	ShowCursor(false);

	return;
}


void SystemClass::ShutdownWindows()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if(FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;

	return;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch(umessage)
	{
		// Check if the window is being destroyed.
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		// Check if the window is being closed.
		case WM_CLOSE:
		{
			PostQuitMessage(0);		
			return 0;
		}

		// All other messages pass to the message handler in the system class.
		default:
		{
			return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}
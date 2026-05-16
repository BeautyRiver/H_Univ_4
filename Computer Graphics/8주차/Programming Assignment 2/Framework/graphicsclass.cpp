////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"


GraphicsClass::GraphicsClass()
{
	m_D3D = 0;
	m_Camera = 0;
	
	// Initialize model array
	for (int i = 0; i < MAX_MODEL_COUNT; i++)
	{
		m_Model[i] = 0;
	}

	m_ColorShader = 0;
	m_rotation = 0.0f;
	
	m_bgR = 0.0f;
	m_bgG = 0.0f;
	m_bgB = 0.0f;
	m_prevKeyC = false;
	m_prevKey1 = false;
	m_prevKey2 = false;
	m_brightness = 1.0f; // Default brightness: 100%
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


	// Create the Direct3D object.
	m_D3D = new D3DClass;
	if(!m_D3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;
	if(!m_Camera)
	{
		return false;
	}

	// Set the initial position of the camera.
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	
	// Create the model object.
	for (int i = 0; i < MAX_MODEL_COUNT; i++)
	{
		m_Model[i] = new ModelClass;
		if (!m_Model[i]) { return false; }

		// Initialize the model object.
		result = m_Model[i]->Initialize(m_D3D->GetDevice(), i);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}

	
	// Create the color shader object.
	m_ColorShader = new ColorShaderClass;
	if(!m_ColorShader)
	{
		return false;
	}

	// Initialize the color shader object.
	result = m_ColorShader->Initialize(m_D3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
		return false;
	}

	return true;
}


void GraphicsClass::Shutdown()
{
	// Release the color shader object.
	if(m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = 0;
	}

	// Release the model object.
	for (int i = 0; i < MAX_MODEL_COUNT; i++)
	{
		if (m_Model[i])
		{
			m_Model[i]->Shutdown();
			delete m_Model[i];
			m_Model[i] = 0;
		}
	}
	

	// Release the camera object.
	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the D3D object.
	if(m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}

	return;
}


bool GraphicsClass::Frame(bool keyR, bool keyG, bool keyB, bool keyW, bool keyS, bool keyC, bool key1, bool key2)
{
	bool result;

	if (keyR) { m_bgR = 1.0f; m_bgG = 0.0f; m_bgB = 0.0f; }
	if (keyG) { m_bgR = 0.0f; m_bgG = 1.0f; m_bgB = 0.0f; }
	if (keyB) { m_bgR = 0.0f; m_bgG = 0.0f; m_bgB = 1.0f; }
	if (keyW) m_D3D->SetWireFrame(true);
	if (keyS) m_D3D->SetWireFrame(false);
	if (keyC && !m_prevKeyC) m_D3D->ToggleCulling();
	m_prevKeyC = keyC;

	// Key 1: increase brightness by 10%, Key 2: decrease brightness by 10% (edge detection)
	if (key1 && !m_prevKey1) { m_brightness += 0.1f; if (m_brightness > 2.0f) m_brightness = 2.0f; }
	if (key2 && !m_prevKey2) { m_brightness -= 0.1f; if (m_brightness < 0.0f) m_brightness = 0.0f; }
	m_prevKey1 = key1;
	m_prevKey2 = key2;

	m_rotation += 0.007f;
	if (m_rotation > XM_2PI) m_rotation -= XM_2PI;

	result = Render();

	if(!result)
	{
		return false;
	}

	return true;
}


bool GraphicsClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;


	// Clear the buffers to begin the scene.
	m_D3D->BeginScene(m_bgR, m_bgG, m_bgB, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	

	for (int i = 0; i < MAX_MODEL_COUNT; i++)
	{
		m_D3D->GetWorldMatrix(worldMatrix);

		if (i == 0) worldMatrix *= XMMatrixRotationX(m_rotation); // rotate on x-axis
		if (i == 1) worldMatrix *= XMMatrixRotationY(m_rotation); // rotate on y-axis
		if (i == 2) worldMatrix *= XMMatrixRotationZ(m_rotation); // rotate on z-axis

		if (i == 0) worldMatrix *= XMMatrixTranslation(-2.0f, 0.0f,  0.0f); // translate on x-axis
		if (i == 1) worldMatrix *= XMMatrixTranslation( 0.0f, 2.0f,  0.0f); // translate on y-axis
		if (i == 2) worldMatrix *= XMMatrixTranslation( 0.0f, 0.0f, -4.0f); // translate on z-axis


		m_Model[i]->Render(m_D3D->GetDeviceContext());
		// Render the model using the color shader.
		result = m_ColorShader->Render(m_D3D->GetDeviceContext(), m_Model[i]->GetIndexCount(),
			worldMatrix, viewMatrix, projectionMatrix, m_brightness);
		if (!result)
		{
			return false;
		}
	}

	// Present the rendered scene to the screen.
	m_D3D->EndScene();

	return true;
}
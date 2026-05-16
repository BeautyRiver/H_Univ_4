////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "colorshaderclass.h"


/////////////
// GLOBALS //
/////////////
const int MAX_MODEL_COUNT = 3;
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


////////////////////////////////////////////////////////////////////////////////
// Class name: GraphicsClass
////////////////////////////////////////////////////////////////////////////////
class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame(bool keyR, bool keyG, bool keyB, bool keyW, bool keyS, bool keyC, bool key1, bool key2);
private:
	bool Render();

private:
	D3DClass* m_D3D;
	CameraClass* m_Camera;
	ModelClass* m_Model[MAX_MODEL_COUNT];
	ColorShaderClass* m_ColorShader;
	float m_rotation; // rotation angle

	float m_bgR, m_bgG, m_bgB;
	bool m_prevKeyC;
	bool m_prevKey1;
	bool m_prevKey2;

	float m_brightness; // Brightness value for pixel shader (0.0 ~ 2.0)
};

#endif
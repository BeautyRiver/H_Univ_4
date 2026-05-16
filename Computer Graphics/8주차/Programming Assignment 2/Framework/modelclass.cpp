////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "modelclass.h"
#include <cmath>


ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
}


bool ModelClass::Initialize(ID3D11Device* device, int shapeType)
{
	bool result;

	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device, shapeType);
	if(!result)
	{
		return false;
	}

	return true;
}


void ModelClass::Shutdown()
{
	// Shutdown the vertex and index buffers.
	ShutdownBuffers();

	return;
}


void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}


int ModelClass::GetIndexCount()
{
	return m_indexCount;
}


bool ModelClass::InitializeBuffers(ID3D11Device* device, int shapeType)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// Set the number of vertices and indices based on shape type.
	if      (shapeType == 0) { m_vertexCount = 6;  m_indexCount = 15; } // Pentagon
	else if (shapeType == 1) { m_vertexCount = 11; m_indexCount = 30; } // Star
	else if (shapeType == 2) { m_vertexCount = 8; m_indexCount = 15; } // Arrow
	else return false;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if(!vertices) return false;

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if(!indices) { delete[] vertices; return false; }

	// Load the vertex array and index array with data.
	if (shapeType == 0) // Pentagon
	{
		vertices[0].position = XMFLOAT3(0.0f, 0.2f, 0.0f); // center
		vertices[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f); // top
		vertices[2].position = XMFLOAT3(-1.0f, 0.2f, 0.0f); // left top
		vertices[3].position = XMFLOAT3(1.0f, 0.2f, 0.0f); // right top
		vertices[4].position = XMFLOAT3(-0.6f, -1.0f, 0.0f); // left bottom 
		vertices[5].position = XMFLOAT3(0.6f, -1.0f, 0.0f); // right bottom

		for (int i = 0; i < 6; i++)
			vertices[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); 

		indices[0] = 1; indices[1] = 0; indices[2] = 2; 
		indices[3] = 1; indices[4] = 3; indices[5] = 0; 		
		indices[6] = 2; indices[7] = 0; indices[8] = 4; 

		indices[9] = 0; indices[10] = 5; indices[11] = 4; 
		indices[12] = 0; indices[13] = 3; indices[14] = 5;

	}
	else if (shapeType == 1) // Star
	{
		vertices[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f);

		for (int i = 0; i < 5; i++)
		{
			float aOuter = XMConvertToRadians(90.0f + i * 72.0f);
			float aInner = XMConvertToRadians(90.0f + 36.0f + i * 72.0f);

			vertices[i*2+1].position = XMFLOAT3(cosf(aOuter), sinf(aOuter), 0.0f);

			vertices[i*2+2].position = XMFLOAT3(0.4f*cosf(aInner), 0.4f*sinf(aInner), 0.0f);
		}

		for (int i = 0; i < 11; i++)
			vertices[i].color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

		for (int i = 0; i < 5; i++)
		{
			int outer     = i*2+1;
			int inner     = i*2+2;
			int nextOuter = ((i+1)%5)*2+1;

			indices[i*6+0] = 0;  indices[i*6+1] = inner;     indices[i*6+2] = outer;
			indices[i*6+3] = 0;  indices[i*6+4] = nextOuter; indices[i*6+5] = inner;
		}
	}
	else if (shapeType == 2)
	{
		vertices[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f);	 // center
		vertices[1].position = XMFLOAT3( 0.0f,  1.0f, 0.0f); // tip
		vertices[2].position = XMFLOAT3(-1.0f,  0.0f, 0.0f); // head left
		vertices[3].position = XMFLOAT3( 1.0f,  0.0f, 0.0f); // head right
		vertices[4].position = XMFLOAT3(-0.4f,  0.0f, 0.0f); // body top-left
		vertices[5].position = XMFLOAT3( 0.4f,  0.0f, 0.0f); // body top-right
		vertices[6].position = XMFLOAT3(-0.4f, -1.0f, 0.0f); // body bottom-left
		vertices[7].position = XMFLOAT3( 0.4f, -1.0f, 0.0f); // body bottom-right

		for (int i = 0; i < 8; i++)
			vertices[i].color = XMFLOAT4(0.0f, 0.5f, 0.5f, 1.0f); // blue

		indices[0]  = 1; indices[1]  = 0; indices[2]  = 2; // left wing
		indices[3]  = 1; indices[4]  = 3; indices[5]  = 0; // right wing
		indices[6] = 4; indices[7] = 0; indices[8] = 6; // left body
		indices[9]  = 0; indices[10] = 5; indices[11] = 7; // right upper		
		indices[12] = 0; indices[13] = 7; indices[14] = 6; // center body
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth           = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags      = 0;
	vertexBufferDesc.MiscFlags           = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem          = vertices;
	vertexData.SysMemPitch      = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result)) { delete[] vertices; delete[] indices; return false; }

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth           = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags      = 0;
	indexBufferDesc.MiscFlags           = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem          = indices;
	indexData.SysMemPitch      = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result)) { delete[] vertices; delete[] indices; return false; }

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}


void ModelClass::ShutdownBuffers()
{
	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}

// This sets the vertex buffer and index buffer as active on the input assembler in the GPU.
// Once the GPU has an active vertex buffer, it can then use the shader to render that buffer.
// This function also defines how those buffers should be drawn such as triangles, lines, fans,
// and etc using the IASetPrimitiveTopology DirectX function.
void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

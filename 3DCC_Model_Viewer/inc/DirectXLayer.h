#pragma once
#include <windows.h>
#include <D3D11.h>
#include <directxmath.h>
#include <vector>
#include <string>
#include "inc\VertexTypes.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DirectXTK.lib")
using namespace DirectX;

struct Vertex
{
	//XMFLOAT3 position;
	float x, y, z;
	float r, g, b, a;
	float i, j, k;
	float u, v;
};

enum VertexFormat { PosColor = 0, PosColorNormal, PosNormalTexture, NUM_VERTEX_FORMATS };

struct BufferInfo
{
	VertexFormat bufferID;		// Which vertex and index buffer to use.
	unsigned int indexCount;	// How many triangles to draw (polygons * 3)
	unsigned int indexStart;
	unsigned int vertexStart;
	BufferInfo()
	{
		bufferID = PosNormalTexture;
		indexCount = indexStart = vertexStart = 0;
	}
};

extern std::string mainApplicationDirectory;

class DirectXLayer
{
private:
	int m_panelWidth;
	int m_panelHeight;
	XMMATRIX m_view, m_projection;
	
	void CreateRasterizerState(void);
	void Update(void);
	void Render(void);

public:
	DirectXLayer(void);
	~DirectXLayer(void);

	HRESULT Initialize(HWND window, int width, int height);
	void Cleanup();
	void ResizeDevice(int width, int height);
	void CreateDepthBuffer(void);
	void CreateProjectionMatrix(float fov);

	void Clear(const float clearColor[4]);
	void Present();

	void CreateCameraMatrix(XMVECTOR position, XMVECTOR target);
	void LoadTexture(const char *textureName);
	
	void LoadVertexAndIndexData_PosColor(const VertexPositionColor *vertexData, unsigned vCount, const unsigned int *indexData, unsigned int iCount, BufferInfo *info);
	void LoadVertexAndIndexData_PosColorNormal(const VertexPositionNormalColor *vertexData, unsigned vCount, const unsigned int *indexData, unsigned int iCount, BufferInfo *info);
	void LoadVertexAndIndexData_PosNormalTexture(const VertexPositionNormalTexture *vertexData, unsigned vCount, const unsigned int *indexData, unsigned int iCount, BufferInfo *info);

	void SetWorldMatrix(XMMATRIX *world);
	void SetTexture(const char *textureName);	//Get a texture from map<string, ID3D11Texture2D>
	void DrawMesh(const BufferInfo *info);
};
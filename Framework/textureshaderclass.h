#ifndef _TEXTURESHADERCLASS_H_
#define _TEXTURESHADERCLASS_H_


#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>

#include <fstream>
#include <vector>

#include "lightclass.h"

using namespace std;
using namespace DirectX;

class TextureShaderClass
{
private:
	// Here is the definition of the cBuffer type that will be used with the vertex 
	// shader. This typedef must be exactly the same as the one in the vertex shader 
	// as the model data needs to match the typedefs in the shader for proper rendering.
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct TextureInfoBufferType
	{
		int textureCount;
		XMFLOAT3 padding;
	};

	struct RotationBufferType
	{
		float globalTime;
		XMFLOAT3 cameraPosition;
		int currentModelIndex;
		XMFLOAT3 padding;
	};

	struct LightBufferType
	{
		XMFLOAT4 ambientColor;
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float specularPower;
		XMFLOAT4 specularColor;
		int ambientEnabled;
		int diffuseEnabled;
		int specularEnabled;
		int padding2;
	};

	struct PointLightBufferType
	{
		XMFLOAT4 pointLightPosition[3];
		XMFLOAT4 pointLightColor[3];
		float pointLightRange[3];
		float padding1;
	};

public:
	TextureShaderClass();
	TextureShaderClass(const TextureShaderClass&);
	~TextureShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, unsigned int, unsigned int, XMMATRIX, XMMATRIX, XMMATRIX,
		std::vector<ID3D11ShaderResourceView*>, unsigned int startIndexLocation = 0,
		float globalTime = 0.0f, XMFLOAT3 cameraPos = XMFLOAT3(0, 0, 0), int modelIndex = 0,
		LightClass* light = nullptr, bool ambientEnabled = true,
		bool diffuseEnabled = true, bool specularEnabled = true);

	int SetMaxTextureNum(int);
	int GetMaxTextureNum() const;

	bool SetTextureFilter(ID3D11Device*, int);

private:
	bool InitializeShader(ID3D11Device*, HWND, const WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, const WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX,
		std::vector<ID3D11ShaderResourceView*>, float, XMFLOAT3, int,
		LightClass*, bool, bool, bool);
	void RenderShader(ID3D11DeviceContext*, unsigned int, unsigned int, unsigned int startIndexLocation = 0) const;

	bool CreateSamplerState(ID3D11Device*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_textureInfoBuffer;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_rotationBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_pointLightBuffer;

	int maxTextureNum;
	int m_currentFilterMode;
};

#endif
#include "textureshaderclass.h"


TextureShaderClass::TextureShaderClass()
{
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_layout = nullptr;
	m_matrixBuffer = nullptr;
	m_textureInfoBuffer = nullptr;
	m_sampleState = nullptr;
	m_rotationBuffer = nullptr;
	m_lightBuffer = nullptr;
	m_pointLightBuffer = nullptr;

	maxTextureNum = 0;
	m_currentFilterMode = 2; // default = anisotropic
}


TextureShaderClass::TextureShaderClass(const TextureShaderClass& other)
{
}


TextureShaderClass::~TextureShaderClass()
{
}


bool TextureShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	// Initialize the vertex and pixel shaders.
	result = InitializeShader(device, hwnd, L"./data/textureShader.hlsl");
	if(!result)
	{
		return false;
	}

	return true;
}


void TextureShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

// This first sets the parameters inside the shader, and then draws the triangle 
// using the HLSL shader.
bool TextureShaderClass::Render(ID3D11DeviceContext* deviceContext, unsigned int indexCount, unsigned int instanceCount,
	XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, std::vector<ID3D11ShaderResourceView*> textures,
	unsigned int startIndexLocation, float globalTime, XMFLOAT3 cameraPos, int modelIndex, LightClass* light,
	bool ambientEnabled, bool diffuseEnabled, bool specularEnabled)
{
	bool result;

	// Shader parameters setter
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, textures,
		globalTime, cameraPos, modelIndex, light, ambientEnabled, diffuseEnabled, specularEnabled);
	if (!result)
	{
		return false;
	}

	RenderShader(deviceContext, indexCount, instanceCount, startIndexLocation);

	return true;
}
// This loads the shader files and makes it usable to DirectX and the GPU. 
// This also sets up the layout and how the vertex buffer data is going to 
// look on the graphics pipeline in the GPU. The layout will need the match 
// the VertexType in the modelclass.h file as well as the one defined in the 
// color.vs file.
bool TextureShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR *fileName)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[5] = {};
	D3D11_BUFFER_DESC matrixBufferDesc = {};
	D3D11_BUFFER_DESC textureInfoBufferDesc = {};

	// Initialize the pointers this function will use to null.
	errorMessage = nullptr;
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer = nullptr;

    // Compile the vertex shader code.
	result = D3DCompileFromFile(fileName, nullptr, nullptr, "TextureVertexShader", "vs_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);

	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, fileName);
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, fileName, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    // Compile the pixel shader code.
	result = D3DCompileFromFile(fileName, nullptr, nullptr, "TexturePixelShader", "ps_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, fileName);
		}
		// If there was nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, fileName, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), 
		vertexShaderBuffer->GetBufferSize(), nullptr, &m_vertexShader);
	if(FAILED(result))
	{
		return false;
	}

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), 
		pixelShaderBuffer->GetBufferSize(), nullptr, &m_pixelShader);
	if(FAILED(result))
	{
		return false;
	}

	// Create the vertex input layout description that will be processed by the shader.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	// The AlignedByteOffset indicates how the data is spaced in the buffer: 12 bytes for 
	// position and the next 16 bytes for color. Use D3D11_APPEND_ALIGNED_ELEMENT which figures
	// out the spacing.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	polygonLayout[3].SemanticName = "TEXCOORD";
	polygonLayout[3].SemanticIndex = 1;
	polygonLayout[3].Format = DXGI_FORMAT_R32_SINT;
	polygonLayout[3].InputSlot = 0;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[3].InstanceDataStepRate = 0;

	polygonLayout[4].SemanticName = "TEXCOORD";
	polygonLayout[4].SemanticIndex = 2;
	polygonLayout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[4].InputSlot = 1;
	polygonLayout[4].AlignedByteOffset = 0;
	polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
	polygonLayout[4].InstanceDataStepRate = 1;


	// Get a count of the elements in the layout.
    //unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);
	constexpr unsigned int numElements = std::size(polygonLayout);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, 
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if(FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;				// The buffer is updated each frame.
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// Writing data by CPU
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, nullptr, &m_matrixBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// textureInfoBuffer 설정
	textureInfoBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureInfoBufferDesc.ByteWidth = sizeof(TextureInfoBufferType);
	textureInfoBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	textureInfoBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	textureInfoBufferDesc.MiscFlags = 0;
	textureInfoBufferDesc.StructureByteStride = 0;

	// textureInfoBuffer 생성
	result = device->CreateBuffer(&textureInfoBufferDesc, nullptr, &m_textureInfoBuffer);
	if (FAILED(result))
	{
		return false;
	}

	if (!CreateSamplerState(device, m_currentFilterMode))
	{
		return false;
	}

	// RotationBuffer 설정
	D3D11_BUFFER_DESC rotationBufferDesc;
	rotationBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	rotationBufferDesc.ByteWidth = sizeof(RotationBufferType);
	rotationBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	rotationBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	rotationBufferDesc.MiscFlags = 0;
	rotationBufferDesc.StructureByteStride = 0;

	// RotationBuffer 생성
	result = device->CreateBuffer(&rotationBufferDesc, nullptr, &m_rotationBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Light buffer 설정
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&lightBufferDesc, nullptr, &m_lightBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Point Light buffer 설정
	D3D11_BUFFER_DESC pointLightBufferDesc;
	pointLightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pointLightBufferDesc.ByteWidth = sizeof(PointLightBufferType);
	pointLightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pointLightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pointLightBufferDesc.MiscFlags = 0;
	pointLightBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&pointLightBufferDesc, nullptr, &m_pointLightBuffer);
	if (FAILED(result))
	{
		return false;
	}


	return true;
}


void TextureShaderClass::ShutdownShader()
{
	// Release the sampler state.
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = nullptr;
	}

	// Release the matrix constant buffer.
	if(m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = nullptr;
	}

	// 텍스쳐 정보 버퍼 해제
	if (m_textureInfoBuffer)
	{
		m_textureInfoBuffer->Release();
		m_textureInfoBuffer = nullptr;
	}

	// Release the layout.
	if(m_layout)
	{
		m_layout->Release();
		m_layout = nullptr;
	}

	// Release the pixel shader.
	if(m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = nullptr;
	}

	// Release the vertex shader.
	if(m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = nullptr;
	}

	if (m_rotationBuffer)
	{
		m_rotationBuffer->Release();
		m_rotationBuffer = nullptr;
	}

	if (m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = nullptr;
	}

	if (m_pointLightBuffer)
	{
		m_pointLightBuffer->Release();
		m_pointLightBuffer = nullptr;
	}

	return;
}


void TextureShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const WCHAR* shaderFilename)
{
	ofstream fout;

	// Get a pointer to the error message text buffer.
	char* compileErrors = static_cast<char*>(errorMessage->GetBufferPointer());

	// Get the length of the message.
	unsigned long bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for(unsigned long i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

// The sets up the global variables in the shader. The transfom matrices are sent into the 
// vertex shader during the Render function call.
bool TextureShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext,
	XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
	std::vector<ID3D11ShaderResourceView*> textures, float globalTime, XMFLOAT3 cameraPos,
	int modelIndex, LightClass* light, bool ambientEnabled, bool diffuseEnabled, bool specularEnabled)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	TextureInfoBufferType* textureInfoPtr;
	RotationBufferType* rotationPtr;
	unsigned int bufferNumber;

	SetMaxTextureNum(9);

	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	deviceContext->Unmap(m_matrixBuffer, 0);
	bufferNumber = 0;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	if (light)
	{
		// Directional Light Buffer Settings
		result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		LightBufferType* lightPtr = static_cast<LightBufferType*>(mappedResource.pData);
		lightPtr->ambientColor = light->GetAmbientColor();
		lightPtr->diffuseColor = light->GetDiffuseColor();
		lightPtr->lightDirection = light->GetDirection();
		lightPtr->specularColor = light->GetSpecularColor();
		lightPtr->specularPower = light->GetSpecularPower();
		lightPtr->ambientEnabled = ambientEnabled ? 1 : 0;
		lightPtr->diffuseEnabled = diffuseEnabled ? 1 : 0;
		lightPtr->specularEnabled = specularEnabled ? 1 : 0;
		lightPtr->padding2 = 0;

		deviceContext->Unmap(m_lightBuffer, 0);
		bufferNumber = 1;
		deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

		// Point Light Buffer Settings
		result = deviceContext->Map(m_pointLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		PointLightBufferType* pointLightPtr = static_cast<PointLightBufferType*>(mappedResource.pData);
		for (int i = 0; i < 3; i++) {
			pointLightPtr->pointLightPosition[i] = XMFLOAT4(
				light->GetPointLightPosition(i).x,
				light->GetPointLightPosition(i).y,
				light->GetPointLightPosition(i).z,
				1.0f);
			pointLightPtr->pointLightColor[i] = light->GetPointLightColor(i);
			pointLightPtr->pointLightRange[i] = light->GetPointLightRange(i);
		}
		pointLightPtr->padding1 = 0.0f;

		deviceContext->Unmap(m_pointLightBuffer, 0);
		bufferNumber = 3;
		deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_pointLightBuffer);
	}

	result = deviceContext->Map(m_rotationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	rotationPtr = static_cast<RotationBufferType*>(mappedResource.pData);
	rotationPtr->globalTime = globalTime;
	rotationPtr->cameraPosition = cameraPos;
	rotationPtr->currentModelIndex = modelIndex;

	deviceContext->Unmap(m_rotationBuffer, 0);
	bufferNumber = 2;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_rotationBuffer);

	result = deviceContext->Map(m_textureInfoBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	textureInfoPtr = static_cast<TextureInfoBufferType*>(mappedResource.pData);
	textureInfoPtr->textureCount = static_cast<int>(textures.size());

	deviceContext->Unmap(m_textureInfoBuffer, 0);
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_textureInfoBuffer);

	// textures setting
	const int maxTextures = min(GetMaxTextureNum(), static_cast<int>(textures.size()));
	if (maxTextures > 0)
		deviceContext->PSSetShaderResources(0, maxTextures, textures.data());

	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	return true;
}

// First, this sets the input layout to be active in the input assembler. This lets the GPU know
// the format of the data in the vertex buffer. 
// Second, this sets the vertex and pixel shaders to render the vertex buffer by calling the 
// DrawIndexed DirectX 11 function using the D3D device context.
void TextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, 
	unsigned int indexCount, unsigned int instanceCount, unsigned int startIndexLocation) const
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// 인스턴스 개수에 따라 렌더링 방식 결정
	if (instanceCount > 1)
	{
		// 인스턴싱을 사용한 렌더링
		deviceContext->DrawIndexedInstanced(indexCount, instanceCount, startIndexLocation, 0, 0);
	}
	else
	{
		// 일반 렌더링
		deviceContext->DrawIndexed(indexCount, startIndexLocation, 0);
	}

	return;
}

int TextureShaderClass::SetMaxTextureNum(int max)
{
	maxTextureNum = max;
	return maxTextureNum;
}

int TextureShaderClass::GetMaxTextureNum() const
{
	return maxTextureNum;
}

// manges the sampler state creation based on the filter mode.
bool TextureShaderClass::CreateSamplerState(ID3D11Device* device, int filterMode)
{
	HRESULT result;
	D3D11_SAMPLER_DESC samplerDesc = {};

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 16;  // anisotropic filtering level
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	switch (filterMode)
	{
	case 0:
		// Point
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;
	case 1:
		// Linear
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	case 2:
		// Anisotropic
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		break;
	default:
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}

	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		return false;
	}

	m_currentFilterMode = filterMode;

	return true;
}

bool TextureShaderClass::SetTextureFilter(ID3D11Device* device, int filterMode)
{
	if (filterMode == m_currentFilterMode)
		return true;

	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = nullptr;
	}

	return CreateSamplerState(device, filterMode);
}
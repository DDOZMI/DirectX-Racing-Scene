#include "modelclass.h"


ModelClass::ModelClass()
{
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_vertices = nullptr;
	m_indices = nullptr;
	m_instanceBuffer = nullptr;

	m_vertexCount = 0;
	m_indexCount = 0;
	m_faceCount = 0;
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
}


bool ModelClass::Initialize(ID3D11Device* device, const WCHAR* modelFilename, vector<const WCHAR*> textureFilename, vector<ModelClass::InstanceType>& instanceInfo, unsigned int instanceCount)
{
	bool result;

	m_instanceCount = instanceCount;

	// Load the model data
	result = LoadModel(modelFilename, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	if (!result)
		return false;

	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device, instanceInfo, instanceCount);
	if (!result)
	{
		return false;
	}

	result = LoadTexture(device, textureFilename);
	if (!result)
	{
		return false;
	}

	return true;
}


void ModelClass::Shutdown()
{
	ReleaseTexture();

	// Shutdown the vertex and index buffers.
	ShutdownBuffers();

	// Release the model data.
	ReleaseModel();

	return;
}


void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}


unsigned int ModelClass::GetIndexCount()
{
	return m_indexCount;
}


unsigned int ModelClass::GetTextureCount()
{
	return static_cast<int>(m_Texture.size());
}

vector<ID3D11ShaderResourceView*> ModelClass::GetTexture()
{
	// �ؽ��ĸ� ���� ���� ����
	vector<ID3D11ShaderResourceView*> textures;

	// �� �ؽ��ĸ� ���Ϳ� �߰�
	for (auto texture : m_Texture)
		textures.push_back(texture->GetTexture());

	// �ؽ��İ� ��� ���� ��ȯ
	return textures;
}


bool ModelClass::InitializeBuffers(ID3D11Device* device, vector<ModelClass::InstanceType>& instanceInfo, unsigned int instanceCount)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	if (!m_vertices || !m_indices)
		return false;
	
	// Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = m_vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = m_indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Instance buffer ���� (instanceInfo�� ������ �׻� ����)
	D3D11_BUFFER_DESC instanceBufferDesc;
	D3D11_SUBRESOURCE_DATA instanceData;

	// vector�� ������� ������ Ȯ��
	if (!instanceInfo.empty())
	{
		unsigned int instanceCount = static_cast<unsigned int>(instanceInfo.size());

		instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		instanceBufferDesc.ByteWidth = sizeof(InstanceType) * instanceCount;
		instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		instanceBufferDesc.CPUAccessFlags = 0;
		instanceBufferDesc.MiscFlags = 0;
		instanceBufferDesc.StructureByteStride = 0;

		// vector�� data() �޼���� ������ ���
		instanceData.pSysMem = instanceInfo.data();
		instanceData.SysMemPitch = 0;
		instanceData.SysMemSlicePitch = 0;

		// �ν��Ͻ� ���۸� ����ϴ�.
		result = device->CreateBuffer(&instanceBufferDesc, &instanceData, &m_instanceBuffer);
		if (FAILED(result))
		{
			return false;
		}

		// �ν��Ͻ� ī��Ʈ�� ��� ������ ���� (���������� ���)
		m_instanceCount = instanceCount;
	}
	else
	{
		// �ν��Ͻ��� ���� ���
		m_instanceBuffer = nullptr;
		m_instanceCount = 0;
	}

	return true;
}


void ModelClass::ShutdownBuffers()
{
	// Release the index buffer.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}

	// Release the vertex buffer.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}


	// Release the instance buffer.
	if (m_instanceBuffer)
	{
		m_instanceBuffer->Release();
		m_instanceBuffer = nullptr;
	}

	return;
}


void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferPointers[2];

	if (m_instanceCount > 1 && m_instanceBuffer != nullptr)
	{
		// �ν��Ͻ��� ����ϴ� ���
		strides[0] = sizeof(VertexType);
		strides[1] = sizeof(InstanceType);

		offsets[0] = 0;
		offsets[1] = 0;

		bufferPointers[0] = m_vertexBuffer;
		bufferPointers[1] = m_instanceBuffer;

		// Set the vertex buffers to active in the input assembler so it can be rendered.
		deviceContext->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
	}
	else
	{
		// �Ϲ� �������� ���
		unsigned int stride = sizeof(VertexType);
		unsigned int offset = 0;

		// Set the vertex buffer to active in the input assembler so it can be rendered.
		deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	}

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

bool ModelClass::LoadTexture(ID3D11Device* device, vector<const WCHAR*> filenames)
{
	bool result;

	// �� �ؽ��� ���� �ε�
	for (const auto& filename : filenames)
	{
		// �ؽ��� ��ü ����
		TextureClass* texture = new TextureClass();
		if (!texture)
		{
			return false;
		}

		// �ؽ��� ��ü �ʱ�ȭ
		result = texture->Initialize(device, filename);
		if (!result)
		{
			delete texture;
			return false;
		}

		// �ؽ��� ���Ϳ� �ε��� �ؽ��ĵ��� �߰�
		m_Texture.push_back(texture);
	}

	return !m_Texture.empty();
}

void ModelClass::ReleaseTexture()
{
	for (auto texture : m_Texture)
	{
		if (texture)
		{
			texture->Shutdown();
			delete texture;
			texture = nullptr;
		}
	}

	m_Texture.clear();

	return;
}

bool ModelClass::LoadModel(CString filename, UINT flag)
{

	// Load model using ASSIMP
	Assimp::Importer importer;
	string strPath = std::string(CT2CA(filename.operator LPCWSTR()));
	const aiScene* pScene = importer.ReadFile(strPath, flag);

	if (!pScene) return false;

	// ��Ƽ���� �� ����
	m_materialMap.clear();
	for (unsigned int i = 0; i < pScene->mNumMaterials; i++)
	{
		const aiMaterial* material = pScene->mMaterials[i];
		aiString name;
		if (material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
		{
			m_materialMap[name.C_Str()] = i; // ��Ƽ���� �̸��� �ε��� ����
		}
	}

	// ����޽ú� ���� �� �ε��� ī��Ʈ ���
	m_vertexCount = 0;
	m_indexCount = 0;
	m_faceCount = 0;
	m_meshes.clear();

	for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
	{
		const aiMesh* mesh = pScene->mMeshes[i];

		MeshInfo meshInfo;
		meshInfo.indexStart = m_indexCount;
		meshInfo.indexCount = mesh->mNumFaces * 3;
		meshInfo.materialIndex = mesh->mMaterialIndex;

		m_meshes.push_back(meshInfo);

		m_vertexCount += mesh->mNumVertices;
		m_indexCount += meshInfo.indexCount;
		m_faceCount += mesh->mNumFaces;
	}

	if (m_vertexCount == 0 || m_indexCount == 0)
		return false;

	// ���� �� �ε��� ���� ����
	m_vertices = new VertexType[m_vertexCount];
	m_indices = new unsigned long[m_indexCount];

	unsigned int vertexOffset = 0;
	unsigned int indexOffset = 0;

	// �� �޽��� ������ ó��
	for (unsigned int meshIndex = 0; meshIndex < pScene->mNumMeshes; meshIndex++)
	{
		const aiMesh* mesh = pScene->mMeshes[meshIndex];
		int materialIndex = mesh->mMaterialIndex;

		// �޽ú� ���� ������ ó��
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			unsigned int vertexIdx = vertexOffset + i;

			// ��ġ ����
			m_vertices[vertexIdx].position.x = mesh->mVertices[i].x;
			m_vertices[vertexIdx].position.y = mesh->mVertices[i].y;
			m_vertices[vertexIdx].position.z = mesh->mVertices[i].z;

			// UV ��ǥ ����
			if (mesh->HasTextureCoords(0)) {
				m_vertices[vertexIdx].texture.x = mesh->mTextureCoords[0][i].x;
				m_vertices[vertexIdx].texture.y = mesh->mTextureCoords[0][i].y;
			}
			else {
				m_vertices[vertexIdx].texture.x = 0;
				m_vertices[vertexIdx].texture.y = 0;
			}

			// ��� ����
			if (mesh->HasNormals()) {
				m_vertices[vertexIdx].normal.x = mesh->mNormals[i].x;
				m_vertices[vertexIdx].normal.y = mesh->mNormals[i].y;
				m_vertices[vertexIdx].normal.z = mesh->mNormals[i].z;
			}

			// ��Ƽ����/�ؽ�ó �ε��� ����
			m_vertices[vertexIdx].materialIndex = materialIndex;
		}

		// �޽ú� �ε��� ������ ó��
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];

			// �ε��� ���� (�ﰢ��)
			m_indices[indexOffset + i * 3 + 0] = vertexOffset + face.mIndices[0];
			m_indices[indexOffset + i * 3 + 1] = vertexOffset + face.mIndices[1];
			m_indices[indexOffset + i * 3 + 2] = vertexOffset + face.mIndices[2];
		}

		vertexOffset += mesh->mNumVertices;
		indexOffset += mesh->mNumFaces * 3;
	}

	return true;
}

void ModelClass::ReleaseModel()
{
	if (m_vertices)
	{
		delete[] m_vertices;
		m_vertices = 0;
	}

	if (m_indices)
	{
		delete[] m_indices;
		m_indices = 0;
	}

	return;
}

int ModelClass::GetTextureIndexByMaterialName(const string& materialName)
{
	if (m_materialMap.find(materialName) != m_materialMap.end())
	{
		return m_materialMap[materialName];
	}

	return 0;
}

bool ModelClass::GetVertexDataForPicking(vector<PickingVertexType>& outVertices, vector<unsigned long>& outIndices)
{
	if (!m_vertices || !m_indices || m_vertexCount == 0 || m_indexCount == 0)
		return false;

	// ���� �ʱ�ȭ
	outVertices.clear();
	outIndices.clear();

	outVertices.reserve(m_vertexCount);
	outIndices.reserve(m_indexCount);

	// ���� ������ ����
	for (unsigned int i = 0; i < m_vertexCount; i++)
	{
		PickingVertexType vertex;
		vertex.position = m_vertices[i].position;
		vertex.texture = m_vertices[i].texture;
		vertex.normal = m_vertices[i].normal;
		vertex.materialIndex = m_vertices[i].materialIndex;
		outVertices.push_back(vertex);
	}

	// �ε��� ������ ����
	for (unsigned int i = 0; i < m_indexCount; i++)
	{
		outIndices.push_back(m_indices[i]);
	}

	return true;
}
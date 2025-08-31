#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#pragma comment(lib, "lib/assimp-vc142-mtd.lib")

#include <d3d11.h>
#include <directxmath.h>

using namespace DirectX;

#include <fstream>
#include <atlstr.h>
#include <vector>
#include <map>

using namespace std;

// ASSIMP library
#include "include/assimp/Importer.hpp"
#include "include/assimp/scene.h"
#include "include/assimp/postprocess.h"

#include "textureclass.h"

class ModelClass
{
public:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
		int materialIndex;
	};

	struct MeshInfo
	{
		unsigned int indexStart;
		unsigned int indexCount;
		int materialIndex;
	};

	struct InstanceType
	{
		XMFLOAT3 position;
	};

	struct PickingVertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
		int materialIndex;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device*, const WCHAR*, vector<const WCHAR*>, vector<InstanceType>&, unsigned int);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	unsigned int GetIndexCount();
	unsigned int GetTextureCount();

	vector<ID3D11ShaderResourceView*> GetTexture();
	vector<MeshInfo> GetMeshes() const { return m_meshes; }

	bool LoadModel(CString, UINT flag);
	void ReleaseModel();

	int GetTextureIndexByMaterialName(const string&);

	VertexType* GetVertices() const { return m_vertices; }
	unsigned long* GetIndices() const { return m_indices; }
	unsigned int GetVertexCount() const { return m_vertexCount; }

	bool GetVertexDataForPicking(vector<PickingVertexType>&, vector<unsigned long>&);

private:
	bool InitializeBuffers(ID3D11Device*, vector<InstanceType>&, unsigned int);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, vector<const WCHAR*>);
	void ReleaseTexture();

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer, *m_instanceBuffer;
	unsigned int m_vertexCount, m_indexCount, m_faceCount, m_instanceCount;

	VertexType *m_vertices;
	unsigned long *m_indices;

	vector<TextureClass*> m_Texture;
	vector<MeshInfo> m_meshes;
	map<string, int> m_materialMap;
};

#endif
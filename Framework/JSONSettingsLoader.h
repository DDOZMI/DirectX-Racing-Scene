#ifndef _JSONSETTINGSLOADER_H_
#define _JSONSETTINGSLOADER_H_

#include <vector>
#include <string>
#include <DirectXMath.h>

// RapidJSON includes
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

#include "modelclass.h"

using namespace DirectX;

struct ModelConfig
{
	int id;
	std::string name;
	std::wstring modelFile;
	unsigned int instanceCount;
	std::vector<ModelClass::InstanceType> instances;
	std::vector<const WCHAR*> textureFiles;
	struct {
		XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
		XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
		XMFLOAT3 translation = { 0.0f, 0.0f, 0.0f };
	} worldMatrix;
};

class JsonSettingsLoader
{
public:
	JsonSettingsLoader();
	~JsonSettingsLoader();

	// JSON ���Ͽ��� �� ���� �ε�
	bool LoadModelsConfig(const char* configFile, std::vector<ModelConfig>& modelConfigs);

private:
	// ���ڿ� ��ȯ ��ƿ��Ƽ �Լ���
	std::string WStringToString(const std::wstring& wstr);
	std::wstring StringToWString(const std::string& str);
};

#endif
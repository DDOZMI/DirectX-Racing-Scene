#define _CRT_SECURE_NO_WARNINGS
#include "jsonSettingsLoader.h"
#include <Windows.h>

JsonSettingsLoader::JsonSettingsLoader()
{
}

JsonSettingsLoader::~JsonSettingsLoader()
{
}

bool JsonSettingsLoader::LoadModelsConfig(const char* configFile, std::vector<ModelConfig>& modelConfigs)
{
	// JSON ���� �б�
	FILE* fp = fopen(configFile, "rb");
	if (!fp)
	{
		return false;
	}

	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	rapidjson::Document document;
	document.ParseStream(is);
	fclose(fp);

	if (document.HasParseError())
	{
		return false;
	}

	// JSON �Ľ�
	if (!document.HasMember("models") || !document["models"].IsArray())
	{
		return false;
	}

	const rapidjson::Value& models = document["models"];
	modelConfigs.clear();
	modelConfigs.reserve(models.Size());

	for (rapidjson::SizeType i = 0; i < models.Size(); i++)
	{
		const rapidjson::Value& model = models[i];
		ModelConfig config;

		// �⺻ ���� �б�
		if (model.HasMember("id") && model["id"].IsInt())
			config.id = model["id"].GetInt();

		if (model.HasMember("name") && model["name"].IsString())
			config.name = model["name"].GetString();

		if (model.HasMember("modelFile") && model["modelFile"].IsString())
			config.modelFile = StringToWString(model["modelFile"].GetString());

		if (model.HasMember("instanceCount") && model["instanceCount"].IsUint())
			config.instanceCount = model["instanceCount"].GetUint();

		// �ν��Ͻ� ���� �б�
		if (model.HasMember("instances") && model["instances"].IsArray())
		{
			const rapidjson::Value& instances = model["instances"];
			config.instances.clear();
			config.instances.reserve(instances.Size());

			for (rapidjson::SizeType j = 0; j < instances.Size(); j++)
			{
				const rapidjson::Value& instance = instances[j];
				ModelClass::InstanceType instanceData;

				if (instance.HasMember("position") && instance["position"].IsArray())
				{
					const rapidjson::Value& pos = instance["position"];
					if (pos.Size() >= 3)
					{
						instanceData.position.x = static_cast<float>(pos[0].GetDouble());
						instanceData.position.y = static_cast<float>(pos[1].GetDouble());
						instanceData.position.z = static_cast<float>(pos[2].GetDouble());
					}
				}

				config.instances.push_back(instanceData);
			}
		}

		// �ؽ�ó ���� ���� �б�
		if (model.HasMember("textureFiles") && model["textureFiles"].IsArray())
		{
			const rapidjson::Value& textures = model["textureFiles"];
			config.textureFiles.clear();
			config.textureFiles.reserve(textures.Size());

			for (rapidjson::SizeType j = 0; j < textures.Size(); j++)
			{
				if (textures[j].IsString())
				{
					std::wstring textureFile = StringToWString(textures[j].GetString());
					// wstring�� WCHAR*�� ��ȯ�Ͽ� ���� (����: �޸� ���� �ʿ�)
					WCHAR* texturePtr = new WCHAR[textureFile.length() + 1];
					wcscpy_s(texturePtr, textureFile.length() + 1, textureFile.c_str());
					config.textureFiles.push_back(texturePtr);
				}
			}
		}

		// ���� ��Ʈ���� ���� �б�
		if (model.HasMember("worldMatrix") && model["worldMatrix"].IsObject())
		{
			const rapidjson::Value& wm = model["worldMatrix"];

			if (wm.HasMember("scale") && wm["scale"].IsArray())
			{
				const rapidjson::Value& scale = wm["scale"];
				if (scale.Size() >= 3)
				{
					config.worldMatrix.scale.x = static_cast<float>(scale[0].GetDouble());
					config.worldMatrix.scale.y = static_cast<float>(scale[1].GetDouble());
					config.worldMatrix.scale.z = static_cast<float>(scale[2].GetDouble());
				}
			}

			if (wm.HasMember("rotation") && wm["rotation"].IsArray())
			{
				const rapidjson::Value& rotation = wm["rotation"];
				if (rotation.Size() >= 3)
				{
					config.worldMatrix.rotation.x = static_cast<float>(rotation[0].GetDouble());
					config.worldMatrix.rotation.y = static_cast<float>(rotation[1].GetDouble());
					config.worldMatrix.rotation.z = static_cast<float>(rotation[2].GetDouble());
				}
			}

			if (wm.HasMember("translation") && wm["translation"].IsArray())
			{
				const rapidjson::Value& translation = wm["translation"];
				if (translation.Size() >= 3)
				{
					config.worldMatrix.translation.x = static_cast<float>(translation[0].GetDouble());
					config.worldMatrix.translation.y = static_cast<float>(translation[1].GetDouble());
					config.worldMatrix.translation.z = static_cast<float>(translation[2].GetDouble());
				}
			}
		}

		modelConfigs.push_back(config);
	}

	return true;
}

std::string JsonSettingsLoader::WStringToString(const std::wstring& wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

std::wstring JsonSettingsLoader::StringToWString(const std::string& str)
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
#ifndef _LIGHTCLASS_H_
#define _LIGHTCLASS_H_

#include <directxmath.h>

using namespace DirectX;

class LightClass
{
public:
	LightClass();
	LightClass(const LightClass&);
	~LightClass();

	void SetAmbientColor(float, float, float, float);
	void SetDiffuseColor(float, float, float, float);
	void SetDirection(float, float, float);
	void SetSpecularColor(float, float, float, float);
	void SetSpecularPower(float);

	XMFLOAT4 GetAmbientColor();
	XMFLOAT4 GetDiffuseColor();
	XMFLOAT3 GetDirection();
	XMFLOAT4 GetSpecularColor();
	float GetSpecularPower();

	void SetPointLightPosition(int index, float x, float y, float z);
	void SetPointLightColor(int index, float r, float g, float b, float intensity);
	void SetPointLightRange(int index, float range);
	XMFLOAT3 GetPointLightPosition(int index);
	XMFLOAT4 GetPointLightColor(int index);
	float GetPointLightRange(int index);

private:
	XMFLOAT4 m_ambientColor;
	XMFLOAT4 m_diffuseColor;
	XMFLOAT3 m_direction;
	XMFLOAT4 m_specularColor;
	float m_specularPower;

	XMFLOAT3 m_pointLightPosition[3];
	XMFLOAT4 m_pointLightColor[3];
	float m_pointLightRange[3];

};

#endif
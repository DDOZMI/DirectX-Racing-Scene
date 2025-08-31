////////////////////////////////////////////////////////////////////////////////
// Filename: lightclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "lightclass.h"


LightClass::LightClass()
{
    for (int i = 0; i < 3; i++)
    {
        m_pointLightPosition[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);
        m_pointLightColor[i] = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        m_pointLightRange[i] = 0.0f;
    }
}


LightClass::LightClass(const LightClass& other)
{
}


LightClass::~LightClass()
{
}

void LightClass::SetAmbientColor(float red, float green, float blue, float alpha)
{
    m_ambientColor = XMFLOAT4(red, green, blue, alpha);
    return;
}

void LightClass::SetDiffuseColor(float red, float green, float blue, float alpha)
{
    m_diffuseColor = XMFLOAT4(red, green, blue, alpha);
    return;
}

void LightClass::SetDirection(float x, float y, float z)
{
    m_direction = XMFLOAT3(x, y, z);
    return;
}

void LightClass::SetSpecularColor(float red, float green, float blue, float alpha)
{
    m_specularColor = XMFLOAT4(red, green, blue, alpha);
    return;
}

void LightClass::SetSpecularPower(float power)
{
    m_specularPower = power;
    return;
}

XMFLOAT4 LightClass::GetAmbientColor()
{
    return m_ambientColor;
}

XMFLOAT4 LightClass::GetDiffuseColor()
{
    return m_diffuseColor;
}

XMFLOAT3 LightClass::GetDirection()
{
    return m_direction;
}

XMFLOAT4 LightClass::GetSpecularColor()
{
    return m_specularColor;
}

float LightClass::GetSpecularPower()
{
    return m_specularPower;
}

void LightClass::SetPointLightPosition(int index, float x, float y, float z)
{
    if (index >= 0 && index < 3)
    {
        m_pointLightPosition[index] = XMFLOAT3(x, y, z);
    }
}

void LightClass::SetPointLightColor(int index, float r, float g, float b, float intensity)
{
    if (index >= 0 && index < 3)
    {
        m_pointLightColor[index] = XMFLOAT4(r, g, b, intensity);
    }
}

void LightClass::SetPointLightRange(int index, float range)
{
    if (index >= 0 && index < 3)
    {
        m_pointLightRange[index] = range;
    }
}

XMFLOAT3 LightClass::GetPointLightPosition(int index)
{
    if (index >= 0 && index < 3)
    {
        return m_pointLightPosition[index];
    }
    return XMFLOAT3(0.0f, 0.0f, 0.0f);
}

XMFLOAT4 LightClass::GetPointLightColor(int index)
{
    if (index >= 0 && index < 3)
    {
        return m_pointLightColor[index];
    }
    return XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
}

float LightClass::GetPointLightRange(int index)
{
    if (index >= 0 && index < 3)
    {
        return m_pointLightRange[index];
    }
    return 0.0f;
}
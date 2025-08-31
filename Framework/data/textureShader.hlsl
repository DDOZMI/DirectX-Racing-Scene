Texture2D textures[9];
SamplerState SampleType;

cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer TextureInfoBuffer
{
    int textureCount;
    float3 padding;
};

cbuffer LightBuffer : register(b1)
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float specularPower;
    float4 specularColor;
	int ambientEnabled;
	int diffuseEnabled;
	int specularEnabled;
	int padding2;
};

cbuffer RotationBuffer : register(b2)
{
    float globalTime;
    float3 cameraPosition;
    int currentModelIndex;
    float3 rotationPadding;
};

cbuffer PointLightBuffer : register(b3)
{
    float4 pointLightPosition[3];
    float4 pointLightColor[3];
    float pointLightRange[3];
    float padding1;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    int materialIndex : TEXCOORD1;
    float3 instancePos : TEXCOORD2;
    uint instanceID : SV_InstanceID;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD3;
    float3 viewDirection : TEXCOORD4;
    int materialIndex : TEXCOORD1;
};

matrix CreateRotationY(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    return matrix(
        c, 0, s, 0,
        0, 1, 0, 0,
        -s, 0, c, 0,
        0, 0, 0, 1
    );
}

PixelInputType TextureVertexShader(VertexInputType input)
{
    PixelInputType output;

    float4 localPos = input.position;
    localPos.w = 1.0f;

    matrix rotationMatrix = matrix(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    float3 worldInstancePos = input.instancePos;

    if (currentModelIndex == 8) // Flag model
    {
        float rotationSpeed = 0.3f;
        float rotationAngle = globalTime * rotationSpeed;
        rotationMatrix = CreateRotationY(-rotationAngle);
        localPos = mul(localPos, rotationMatrix);
    }
    else if (currentModelIndex == 11 || currentModelIndex == 12) // Tree models
    {
        float rotationAngle = globalTime;
        rotationMatrix = CreateRotationY(-rotationAngle);
        localPos = mul(localPos, rotationMatrix);
    }
    else if (currentModelIndex == 14) // Balloon model
    {
        float orbitSpeed = 0.01f; // 공전 속도
        float orbitRadius = 2000.0f; // 공전 반지름
        float orbitAngle = globalTime * orbitSpeed;

		float3 orbitOffset = float3(
			orbitRadius * cos(orbitAngle),
			orbitRadius * sin(orbitAngle),
			0.0f
		);

        worldInstancePos += orbitOffset;
    }

    // apply instance location
    float4 worldPos = localPos;
    worldPos.xyz += worldInstancePos;

    // world position for point light
    output.worldPos = mul(worldPos, worldMatrix).xyz;

	// 
    output.position = mul(worldPos, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.tex = input.tex;

    // calculate normal vector with rotation
    float3 worldNormal = input.normal;
    if (currentModelIndex == 8 || currentModelIndex == 11 || currentModelIndex == 12 || currentModelIndex == 14)
    {
        worldNormal = mul(worldNormal, (float3x3)rotationMatrix);
    }
    output.normal = normalize(mul(worldNormal, (float3x3)worldMatrix));

    // view vector
    output.viewDirection = normalize(cameraPosition.xyz - output.worldPos);

    output.materialIndex = input.materialIndex;

    return output;
}

float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor;

	// Texture sampling by material index
	if (input.materialIndex == 0)
	    textureColor = textures[0].Sample(SampleType, input.tex);
	else if (input.materialIndex == 1)
	    textureColor = textures[1].Sample(SampleType, input.tex);
	else if (input.materialIndex == 2)
	    textureColor = textures[2].Sample(SampleType, input.tex);
	else if (input.materialIndex == 3)
	    textureColor = textures[3].Sample(SampleType, input.tex);
	else if (input.materialIndex == 4)
	    textureColor = textures[4].Sample(SampleType, input.tex);
	else if (input.materialIndex == 5)
	    textureColor = textures[5].Sample(SampleType, input.tex);
	else if (input.materialIndex == 6)
	    textureColor = textures[6].Sample(SampleType, input.tex);
	else if (input.materialIndex == 7)
	    textureColor = textures[7].Sample(SampleType, input.tex);
	else if (input.materialIndex == 8)
	    textureColor = textures[8].Sample(SampleType, input.tex);
	else
	    textureColor = textures[0].Sample(SampleType, input.tex);

	if (textureColor.a < 0.1f)
	    discard;

	// Lighting Process Start //
    float4 color = float4(0, 0, 0, 1); // 초기값 검정색
    float4 specularTotal = float4(0, 0, 0, 0);

    // Ambient Light 적용
    if (ambientEnabled)
    {
        color += ambientColor;
    }

    // specular X models : map, tree1, 2
    //bool applySpecular = !(currentModelIndex == 0 || currentModelIndex == 11 || currentModelIndex == 12);

    // Calculate Directional light //
    float3 lightDir = -lightDirection;
    float lightIntensity = saturate(dot(input.normal, lightDir));

    if (lightIntensity > 0.0f)
    {
        // Diffuse Light 적용
        if (diffuseEnabled)
        {
            color += diffuseColor * lightIntensity;
        }

        // Specular Light 계산
        if (specularEnabled)// && applySpecular)
        {
            float3 reflection = normalize(2 * lightIntensity * input.normal - lightDir);
            specularTotal += specularColor * pow(saturate(dot(reflection, input.viewDirection)), specularPower);
        }
    }

	// Calculate Point Light //
	float4 pointLightTotal = float4(0, 0, 0, 0);

	// 0번
	{
	    float3 lightToPixel = pointLightPosition[0].xyz - input.worldPos;
	    float distance = length(lightToPixel);
	    float3 pointLightDir = normalize(lightToPixel);
	    float pointLightIntensity = saturate(dot(input.normal, pointLightDir));

	    float attenuation = 1.0f / (1.0f + 0.01f * distance + 0.1f * distance * distance);

	    if (pointLightIntensity > 0.0f)
	    {
	        // Diffuse
	        pointLightTotal += pointLightColor[0] * pointLightIntensity * attenuation;

	        // Specular
	        float3 reflection = normalize(2 * pointLightIntensity * input.normal - pointLightDir);
	        specularTotal += pointLightColor[0] * pow(saturate(dot(reflection, input.viewDirection)), specularPower) * attenuation;
	    }
	}

	// 1번
	{
	    float3 lightToPixel = pointLightPosition[1].xyz - input.worldPos;
	    float distance = length(lightToPixel);
	    float3 pointLightDir = normalize(lightToPixel);
	    float pointLightIntensity = saturate(dot(input.normal, pointLightDir));

	    float attenuation = 1.0f / (1.0f + 0.01f * distance + 0.1f * distance * distance);

	    if (pointLightIntensity > 0.0f)
	    {
	        // Diffuse
	        pointLightTotal += pointLightColor[1] * pointLightIntensity * attenuation;

	        // Specular
	        float3 reflection = normalize(2 * pointLightIntensity * input.normal - pointLightDir);
	        specularTotal += pointLightColor[1] * pow(saturate(dot(reflection, input.viewDirection)), specularPower) * attenuation;
	    }
	}

	// 2번
	{
	    float3 lightToPixel = pointLightPosition[2].xyz - input.worldPos;
	    float distance = length(lightToPixel);
	    float3 pointLightDir = normalize(lightToPixel);
	    float pointLightIntensity = saturate(dot(input.normal, pointLightDir));

	    float attenuation = 1.0f / (1.0f + 0.01f * distance + 0.1f * distance * distance);

	    if (pointLightIntensity > 0.0f)
	    {
	        // Diffuse
	        pointLightTotal += pointLightColor[2] * pointLightIntensity * attenuation;

	        // Specular
	        float3 reflection = normalize(2 * pointLightIntensity * input.normal - pointLightDir);
	        specularTotal += pointLightColor[2] * pow(saturate(dot(reflection, input.viewDirection)), specularPower) * attenuation;
	    }
	}

	// Mix up lighting effects //
    color = saturate(color + pointLightTotal);
    color = color * textureColor;

    if (specularEnabled)
    {
        color = saturate(color + specularTotal);
    }

	return color;
}
cbuffer basicPSBuffer : register(b0)
{
    float4 lightDirection;
    bool useTexture;
    bool useNormalMap;
};

Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState samplerState : register(s0);
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};
float4 PS(PS_INPUT input) : SV_Target0
{
    float4 texColor = float4(1.0, 1.0, 1.0, 1.0);
    if (useTexture)
    {
        texColor = diffuseTexture.Sample(samplerState, input.TexCoords);
    }
    if (useNormalMap)
    {
        float3 normalMap = normalTexture.Sample(samplerState, input.TexCoords).xyz * 2.0 - 1.0;
        float3 T = normalize(input.Tangent);
        float3 N = normalize(input.Normal);
        float3 B = normalize(cross(N, T));
        float3x3 TBN = float3x3(T, B, N);
        float3 modifiedNormal = normalize(mul(normalMap, TBN));
        float lightIntensity = saturate(dot(modifiedNormal, -lightDirection.xyz));
        texColor.rgb *= lightIntensity * 1.2 + 0.2;
    }
    else
    {
        float3 N = normalize(input.Normal);
        float lightIntensity = saturate(dot(N, -lightDirection.xyz));
        texColor.rgb *= lightIntensity * 1.2 + 0.2;
    }
    if (texColor.a < 0.5)
    {
        discard;
    }
    return texColor;
}

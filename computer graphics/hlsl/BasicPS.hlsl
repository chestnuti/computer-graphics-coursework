Texture2D diffuseTexture : register(t0);
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
    float4 texColor = diffuseTexture.Sample(samplerState, input.TexCoords);
    if (texColor.a < 0.5)
    {
        discard;
    }
    return texColor;
}

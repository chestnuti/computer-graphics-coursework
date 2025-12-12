cbuffer UIBuffer : register(b0)
{
    float2 uioffset;
    float2 uiscale;
};

struct VS_INPUT
{
    float2 Pos : POSITION;
    float2 TexCoords : TEXCOORD;
};
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 TexCoords : TEXCOORD;
};
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.Pos = float4(input.Pos * uiscale + uioffset, 0.0, 1.0);
    output.TexCoords = input.TexCoords;
    return output;
}

Texture2D diffuseTexture : register(t0);
SamplerState samplerState : register(s0);
float4 PS(PS_INPUT input) : SV_Target0
{
    float4 texColor = diffuseTexture.Sample(samplerState, input.TexCoords);
    if (texColor.a < 0.5)
    {
        discard;
    }
    return texColor;
}

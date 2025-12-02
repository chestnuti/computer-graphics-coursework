cbuffer staticMeshBuffer
{
    float4x4 W;
    float4x4 VP;
};
struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.Pos = mul(W, input.Pos);
    output.Pos = mul(VP, output.Pos);
    output.Normal = mul((float3x3) W, input.Normal);
    output.Tangent = mul((float3x3) W, input.Tangent);
    output.TexCoords = input.TexCoords;
    return output;
}
float4 PS(PS_INPUT input) : SV_Target0
{
    return float4(abs(normalize(input.Normal)) * 0.9f, 1.0);
}

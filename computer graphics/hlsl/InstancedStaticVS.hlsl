cbuffer staticMeshBuffer : register(b0)
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
    
    float4 world0 : WORLD0;
    float4 world1 : WORLD1;
    float4 world2 : WORLD2;
    float4 world3 : WORLD3;
};
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};

float remap(float value, float inputMin, float inputMax, float outputMin, float outputMax)
{
    return outputMin + (value - inputMin) * (outputMax - outputMin) / (inputMax - inputMin);
}

PS_INPUT VS(VS_INPUT input)
{
    float4x4 world =
    {
        input.world0,
        input.world1,
        input.world2,
        input.world3
    };
    PS_INPUT output;
    output.Pos = mul(input.Pos, world);
    output.Pos = mul(output.Pos, W);
    output.Pos = mul(output.Pos, VP);
    
    output.Normal = mul(input.Normal, (float3x3) world);
    output.Normal = mul(output.Normal, (float3x3) W);
    output.Tangent = mul(input.Tangent, (float3x3) world);
    output.Tangent = mul(output.Tangent, (float3x3) W);
    output.TexCoords = input.TexCoords;
    return output;
}
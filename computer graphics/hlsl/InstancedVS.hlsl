cbuffer staticMeshBuffer : register(b0)
{
    float4x4 W;
    float4x4 VP;
    float3 playerPosition;
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
    float3 toPlayer = playerPosition - output.Pos.xyz + float3(0, 2, 0);
    float len = length(toPlayer) + 0.3;
    len *= len;
    toPlayer /= len;
    float4x4 translation = float4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        -toPlayer.x, 0, -toPlayer.z, 1
    );
    output.Pos = mul(output.Pos, W);
    output.Pos = mul(output.Pos, translation);
    output.Pos = mul(output.Pos, VP);
    
    output.Normal = mul(input.Normal, (float3x3) world);
    output.Normal = mul(output.Normal, (float3x3) W);
    output.Tangent = mul(input.Tangent, (float3x3) world);
    output.Tangent = mul(output.Tangent, (float3x3) W);
    output.TexCoords = input.TexCoords;
    return output;
}
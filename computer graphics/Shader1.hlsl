struct VS_INPUT
{
    float4 Pos : POS;
    float3 Colour : COLOUR;
};
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Colour : COLOUR;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.Pos = input.Pos;
    output.Colour = input.Colour;
    return output;
}

float4 PS(PS_INPUT input) : SV_Target0
{
    return float4(input.Colour, 1.0);
}


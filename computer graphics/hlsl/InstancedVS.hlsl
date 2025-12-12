cbuffer staticMeshBuffer : register(b0)
{
    float4x4 W;
    float4x4 VP;
    float3 playerPosition;
    float time;
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


// ------------------------------------
// 32-bit integer hash (Wang-like)
// ------------------------------------
uint wang_hash(uint x)
{
    x = (x ^ 61u) ^ (x >> 16);
    x *= 9u;
    x = x ^ (x >> 4);
    x *= 0x27d4eb2du;
    x = x ^ (x >> 15);
    return x;
}

// Hash an int3 coordinate into a single uint
uint hash_int3(int3 p)
{
    // Mix 3D ints with big primes
    uint h = asuint(p.x) * 73856093u ^
             asuint(p.y) * 19349663u ^
             asuint(p.z) * 83492791u;

    return wang_hash(h);
}

float u01(uint v)
{
    return (float) v / 4294967296.0;
} // [0,1)

// Return a pseudo-random feature point inside integer 3D cell
float3 sampleFeaturePoint(int3 cell, float jitter)
{
    uint h = hash_int3(cell);

    // decorrelate by hashing with xor constants
    float rx = u01(wang_hash(h));
    float ry = u01(wang_hash(h ^ 0x9e3779b9u));
    float rz = u01(wang_hash(h ^ 0x85ebca6bu));

    return float3(rx, ry, rz) * jitter;
}


// ------------------------------------
// worley3D: returns float2(F1, F2)
// uvw : 3D position * frequency
// jitter: 0..1 (1 = fully random inside cell)
// useManhattan: optional distance metric
// ------------------------------------
float2 worley3D(float3 uvw, float jitter, bool useManhattan)
{
    int3 cell = (int3) floor(uvw);
    float3 f = frac(uvw);

    float F1 = 1e20;
    float F2 = 1e20;

    // Check 27 neighbor cells
    [unroll]
    for (int z = -1; z <= 1; z++)
    {
        [unroll]
        for (int y = -1; y <= 1; y++)
        {
            [unroll]
            for (int x = -1; x <= 1; x++)
            {
                int3 nc = cell + int3(x, y, z);

                float3 feature = sampleFeaturePoint(nc, jitter) + float3(x, y, z);
                float3 dv = feature - f;

                float d = useManhattan ? (abs(dv.x) + abs(dv.y) + abs(dv.z)) : length(dv);

                if (d < F1)
                {
                    F2 = F1;
                    F1 = d;
                }
                else if (d < F2)
                {
                    F2 = d;
                }
            }
        }
    }

    return float2(F1, F2);
}

// Convenience wrapper (euclidean, jitter=1)
float worley3D_F1(float3 p, float scale)
{
    return worley3D(float3(p.x * scale, p.y * scale + time * 0.2, p.z * scale), 1.0, false).
    x;
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
        -toPlayer.x * exp(-toPlayer.y / 3), 0, -toPlayer.z * exp(-toPlayer.y / 3), 1
    );
    
    float4x4 worleyTranslation = float4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        remap(worley3D_F1(output.Pos.xyz * 0.1, 3.0), 0.0, 1.732, -0.5, 0.5),
        remap(worley3D_F1(output.Pos.yzx * 0.1 + float3(5.2, 1.3, 7.7), 3.0), 0.0, 1.732, -0.5, 0.5),
        remap(worley3D_F1(output.Pos.zxy * 0.1 + float3(3.1, 4.4, 2.2), 3.0), 0.0, 1.732, -0.5, 0.5),
        1
    );
    
    output.Pos = mul(output.Pos, W);
    output.Pos = mul(output.Pos, translation);
    output.Pos = mul(output.Pos, worleyTranslation);
    output.Pos = mul(output.Pos, VP);
    
    output.Normal = mul(input.Normal, (float3x3) world);
    output.Normal = mul(output.Normal, (float3x3) W);
    output.Tangent = mul(input.Tangent, (float3x3) world);
    output.Tangent = mul(output.Tangent, (float3x3) W);
    output.TexCoords = input.TexCoords;
    return output;
}
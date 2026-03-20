// ═══════════════════════════════════════════════════════════════════
//  Default.hlsl
// ═══════════════════════════════════════════════════════════════════

cbuffer ObjectCB : register(b0)
{
    float4x4 gWorld;
};

cbuffer PassCB : register(b1)
{
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewProj;
    float3   gEyePos;
    float    gPadding1;
    float4   gAmbientLight;
};

struct VSInput
{
    float3 PosL    : POSITION;    
    float3 NormalL : NORMAL;      
    float2 TexC    : TEXCOORD;
};

struct VSOutput
{
    float4 PosH    : SV_POSITION; 
    float3 PosW    : POSITION;    
    float3 NormalW : NORMAL;      
    float2 TexC    : TEXCOORD;
};

// ═══════════════════════════════════════════════════════════════════
//  Vertex Shader
// ═══════════════════════════════════════════════════════════════════
VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float4 posW = mul(float4(input.PosL, 1.0f), gWorld);
    output.PosW = posW.xyz;
    output.PosH = mul(posW, gViewProj);

    output.NormalW = mul(input.NormalL, (float3x3)gWorld);

    output.TexC = input.TexC;

    return output;
}

// ═══════════════════════════════════════════════════════════════════
//  Pixel Shader
// ═══════════════════════════════════════════════════════════════════
float4 PSMain(VSOutput input) : SV_TARGET
{
    float3 normal = normalize(input.NormalW);

    // Directional Light
    float3 lightDir = normalize(float3(0.5f, 1.0f, 0.3f));
    float  NdotL    = max(dot(normal, lightDir), 0.0f);

    float3 baseColor = float3(0.7f, 0.7f, 0.7f);

    // Ambient + Diffuse
    float3 ambient = gAmbientLight.rgb * baseColor;
    float3 diffuse = NdotL * baseColor;

    float3 finalColor = ambient + diffuse;

    return float4(finalColor, 1.0f);
}

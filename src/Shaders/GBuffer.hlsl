// ═══════════════════════════════════════════════════════════════════
//  GBuffer.hlsl — Deferred Rendering Geometry Pass
//  씬 오브젝트를 G-Buffer 3장에 기록 (조명 계산 없음)
//
//  Root Parameters:
//    [0] b0 ObjectCB
//    [1] b1 PassCB
//    [2] t0 Albedo Texture (SamplerState s0)
//
//  Outputs:
//    SV_TARGET0 : Albedo   (R8G8B8A8_UNORM)
//    SV_TARGET1 : Normal   (R16G16B16A16_FLOAT, world space [0,1] packed)
//    SV_TARGET2 : WorldPos (R16G16B16A16_FLOAT, world space position)
// ═══════════════════════════════════════════════════════════════════

cbuffer ObjectCB : register(b0)
{
    float4x4 gWorld;
};

struct DirectionalLightData
{
    float3 Direction;
    float  Padding1;
    float3 Color;
    float  Intensity;
};

cbuffer PassCB : register(b1)
{
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewProj;
    float3   gEyePos;
    float    gPadding1;
    float4   gAmbientLight;

    DirectionalLightData gDirLight;
    float4x4 gLightViewProj;
};

Texture2D    gAlbedoTex  : register(t0);
SamplerState gTexSampler : register(s0);

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

struct GBufferOutput
{
    float4 Albedo   : SV_TARGET0;
    float4 Normal   : SV_TARGET1;
    float4 WorldPos : SV_TARGET2;
};

// ═══════════════════════════════════════════════════════════════════
//  Vertex Shader
// ═══════════════════════════════════════════════════════════════════
VSOutput VSMain(VSInput input)
{
    VSOutput o;
    float4 posW = mul(float4(input.PosL, 1.0f), gWorld);
    o.PosW    = posW.xyz;
    o.PosH    = mul(posW, gViewProj);
    o.NormalW = mul(input.NormalL, (float3x3)gWorld);
    o.TexC    = input.TexC;
    return o;
}

// ═══════════════════════════════════════════════════════════════════
//  Pixel Shader — G-Buffer 기록
// ═══════════════════════════════════════════════════════════════════
GBufferOutput PSMain(VSOutput input)
{
    GBufferOutput o;

    float3 albedo = gAlbedoTex.Sample(gTexSampler, input.TexC).rgb;
    float3 N      = normalize(input.NormalW);

    o.Albedo   = float4(albedo, 1.0f);
    o.Normal   = float4(N * 0.5f + 0.5f, 0.0f);  // [-1,1] → [0,1] 패킹
    o.WorldPos = float4(input.PosW, 1.0f);

    return o;
}

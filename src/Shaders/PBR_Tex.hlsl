// ═══════════════════════════════════════════════════════════════════
//  PBR_Tex.hlsl — Albedo Texture + Normal Map + Blinn-Phong + Shadow Map
//
//  Root Parameter :
//    [0] b0 ObjectCB
//    [1] b1 PassCB (FPCB)
//    [2] t0 Shadow Map     (SamplerComparisonState s0)
//    [3] t1 Albedo Texture (SamplerState s1)
//    [4] t2 Normal Map     (SamplerState s1)
// ═══════════════════════════════════════════════════════════════════

cbuffer ObjectCB : register(b0)
{
    float4x4 gWorld;
};

// C++ MAX_POINT_LIGHTS / MAX_SPOT_LIGHTS 상수와 반드시 일치해야 함
#define MAX_POINT_LIGHTS 8
#define MAX_SPOT_LIGHTS  4

struct DirectionalLightData
{
    float3 Direction;
    float  Padding1;
    float3 Color;
    float  Intensity;
};

struct PointLightData
{
    float3 Position;
    float  Radius;
    float3 Color;
    float  Intensity;
};

struct SpotLightData
{
    float3 Position;
    float  Radius;
    float3 Direction;
    float  InnerCosAngle;
    float3 Color;
    float  Intensity;
    float  OuterCosAngle;
    float3 Padding;
};

cbuffer FPCB : register(b1)
{
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewProj;
    float3   gEyePos;
    float    gPadding1;
    float4   gAmbientLight;

    DirectionalLightData gDirLight;
    float4x4 gLightViewProj;

    PointLightData gPointLights[MAX_POINT_LIGHTS];
    int            gPointLightCount;
    float3         gPaddingPL;

    SpotLightData  gSpotLights[MAX_SPOT_LIGHTS];
    int            gSpotLightCount;
    float3         gPaddingSL;
};

// t0 : Shadow Map
Texture2D               gShadowMap     : register(t0);
SamplerComparisonState  gShadowSampler : register(s0);

// t1 : Albedo Texture, t2 : Normal Map
Texture2D               gAlbedoTex     : register(t1);
Texture2D               gNormalMapTex  : register(t2);
SamplerState            gTexSampler    : register(s1);

struct VSInput
{
    float3 PosL     : POSITION;
    float3 NormalL  : NORMAL;
    float2 TexC     : TEXCOORD;
    float3 TangentL : TANGENT;
};

struct VSOutput
{
    float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
    float2 TexC     : TEXCOORD;
    float3 TangentW : TANGENT;
};

// ═══════════════════════════════════════════════════════════════════
//  Vertex Shader
// ═══════════════════════════════════════════════════════════════════
VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float4 posW      = mul(float4(input.PosL, 1.0f), gWorld);
    output.PosW      = posW.xyz;
    output.PosH      = mul(posW, gViewProj);
    output.NormalW   = mul(input.NormalL,  (float3x3)gWorld);
    output.TangentW  = mul(input.TangentL, (float3x3)gWorld);
    output.TexC      = input.TexC;

    return output;
}

// ═══════════════════════════════════════════════════════════════════
//  Shadow 계산
// ═══════════════════════════════════════════════════════════════════
float CalcShadow(float3 posW)
{
    float4 lightClip = mul(float4(posW, 1.0f), gLightViewProj);
    lightClip.xyz /= lightClip.w;

    float2 shadowUV;
    shadowUV.x =  lightClip.x * 0.5f + 0.5f;
    shadowUV.y = -lightClip.y * 0.5f + 0.5f;

    if (shadowUV.x < 0 || shadowUV.x > 1 || shadowUV.y < 0 || shadowUV.y > 1)
        return 1.0f;

    float currentDepth = lightClip.z;

    // PCF 3x3
    // TODO : 상수로 분리
    float shadow = 0.0f;
    float2 texelSize = float2(1.0f / 2048.0f, 1.0f / 2048.0f);

    [unroll]
    for (int x = -1; x <= 1; x++)
    {
        [unroll]
        for (int y = -1; y <= 1; y++)
        {
            shadow += gShadowMap.SampleCmpLevelZero(
                gShadowSampler,
                shadowUV + float2(x, y) * texelSize,
                currentDepth);
        }
    }
    return shadow / 9.0f;
}

// ═══════════════════════════════════════════════════════════════════
//  Pixel Shader
// ═══════════════════════════════════════════════════════════════════
float4 PSMain(VSOutput input) : SV_TARGET
{
    float3 albedo = gAlbedoTex.Sample(gTexSampler, input.TexC).rgb;

    // TBN 보간 오차 보정
    float3 N = normalize(input.NormalW);
    float3 T = normalize(input.TangentW - dot(input.TangentW, N) * N);
    float3 B = cross(N, T);

    // 노말맵 샘플링
    float3 normalT = gNormalMapTex.Sample(gTexSampler, input.TexC).rgb * 2.0f - 1.0f;
    float3x3 TBN   = float3x3(T, B, N);
    N = normalize(mul(normalT, TBN));

    float3 V = normalize(gEyePos - input.PosW);

    // Directional Light
    float3 Ld   = normalize(-gDirLight.Direction);
    float3 Hd   = normalize(Ld + V);
    float  NdLd = max(dot(N, Ld), 0.0f);
    float  NdHd = max(dot(N, Hd), 0.0f);

    float  shadow  = CalcShadow(input.PosW);
    float3 dirDiff = NdLd * gDirLight.Color * gDirLight.Intensity;
    float3 dirSpec = pow(NdHd, 64.0f) * gDirLight.Color * 0.3f;  // TODO : GGX로 교체

    float3 result = gAmbientLight.rgb * albedo
                  + shadow * (dirDiff * albedo + dirSpec);

    // Point Lights
    for (int i = 0; i < gPointLightCount; i++)
    {
        float3 toLight = gPointLights[i].Position - input.PosW;
        float  dist    = length(toLight);
        float3 Lp      = toLight / dist;
        float3 Hp      = normalize(Lp + V);

        float  atten = saturate(1.0f - dist / gPointLights[i].Radius);
        atten = atten * atten;

        float  NdLp = max(dot(N, Lp), 0.0f);
        float  NdHp = max(dot(N, Hp), 0.0f);

        float3 ptDiff = NdLp * gPointLights[i].Color * gPointLights[i].Intensity;
        float3 ptSpec = pow(NdHp, 64.0f) * gPointLights[i].Color * 0.3f;

        result += atten * (ptDiff * albedo + ptSpec);
    }

    // Spot Lights
    for (int j = 0; j < gSpotLightCount; j++)
    {
        float3 toLight = gSpotLights[j].Position - input.PosW;
        float  dist    = length(toLight);
        float3 Ls      = toLight / dist;
        float3 Hs      = normalize(Ls + V);

        float distAtten = saturate(1.0f - dist / gSpotLights[j].Radius);
        distAtten = distAtten * distAtten;

        float cosTheta  = dot(-Ls, normalize(gSpotLights[j].Direction));
        float epsilon   = gSpotLights[j].InnerCosAngle - gSpotLights[j].OuterCosAngle;
        float coneAtten = saturate((cosTheta - gSpotLights[j].OuterCosAngle) / epsilon);

        float  atten = distAtten * coneAtten;

        float  NdLs = max(dot(N, Ls), 0.0f);
        float  NdHs = max(dot(N, Hs), 0.0f);

        float3 stDiff = NdLs * gSpotLights[j].Color * gSpotLights[j].Intensity;
        float3 stSpec = pow(NdHs, 64.0f) * gSpotLights[j].Color * 0.3f;

        result += atten * (stDiff * albedo + stSpec);
    }

    return float4(result, 1.0f);
}

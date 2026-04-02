// ═══════════════════════════════════════════════════════════════════
//  Lighting.hlsl — Deferred Rendering Lighting Pass
//
//  Root Parameters:
//    [0] b0 PassCB
//    [1] t0 Shadow Map    (SamplerComparisonState s0)
//    [2] t1 GBuffer Albedo, t2 Normal, t3 WorldPos (SamplerState s1)
// ═══════════════════════════════════════════════════════════════════

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
    float3 Position;   // 월드 공간 위치
    float  Radius;     // 감쇠 반경
    float3 Color;
    float  Intensity;
};

struct SpotLightData
{
    float3 Position;
    float  Radius;
    float3 Direction;
    float  InnerCosAngle;  // 이 각도 안쪽은 최대 강도
    float3 Color;
    float  Intensity;
    float  OuterCosAngle;  // 이 각도 밖은 강도 0
    float3 Padding;
};

cbuffer PassCB : register(b0)
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
Texture2D              gShadowMap    : register(t0);
SamplerComparisonState gShadowSampler : register(s0);

// t1~t3 : G-Buffer
Texture2D    gGBufAlbedo   : register(t1);
Texture2D    gGBufNormal   : register(t2);
Texture2D    gGBufWorldPos : register(t3);
SamplerState gPointSampler : register(s1);

struct VSOutput
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD;
};

// ═══════════════════════════════════════════════════════════════════
//  Vertex Shader — SV_VertexID로 풀스크린 삼각형 생성
//  DrawInstanced(3, 1, 0, 0) 호출, 버텍스 버퍼 불필요
// ═══════════════════════════════════════════════════════════════════
VSOutput VSMain(uint vertexID : SV_VertexID)
{
    float2 uv = float2((vertexID << 1) & 2, vertexID & 2);
    VSOutput o;
    o.PosH = float4(uv * 2.0f - 1.0f, 0.0f, 1.0f);
    o.TexC = float2(uv.x, 1.0f - uv.y);
    return o;
}

// ═══════════════════════════════════════════════════════════════════
//  Shadow PCF 3x3
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
                lightClip.z);
        }
    }
    return shadow / 9.0f;
}

// ═══════════════════════════════════════════════════════════════════
//  Pixel Shader — G-Buffer 읽기 + Blinn-Phong 조명
// ═══════════════════════════════════════════════════════════════════
float4 PSMain(VSOutput input) : SV_TARGET
{
    float3 albedo = gGBufAlbedo.Sample(gPointSampler, input.TexC).rgb;
    float3 N      = gGBufNormal.Sample(gPointSampler, input.TexC).xyz * 2.0f - 1.0f;  // [0,1] → [-1,1]
    float3 posW   = gGBufWorldPos.Sample(gPointSampler, input.TexC).xyz;

    N = normalize(N);
    float3 V = normalize(gEyePos - posW);

    // Directional Light (+ Shadow)
    float3 Ld   = normalize(-gDirLight.Direction);
    float3 Hd   = normalize(Ld + V);
    float  NdLd = max(dot(N, Ld), 0.0f);
    float  NdHd = max(dot(N, Hd), 0.0f);

    float  shadow  = CalcShadow(posW);
    float3 dirDiff = NdLd * gDirLight.Color * gDirLight.Intensity;
    float3 dirSpec = pow(NdHd, 64.0f) * gDirLight.Color * 0.3f;

    float3 result = gAmbientLight.rgb * albedo
                  + shadow * (dirDiff * albedo + dirSpec);


    // Point Lights
    for (int i = 0; i < gPointLightCount; i++)
    {
        float3 toLight = gPointLights[i].Position - posW;
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
        float3 toLight = gSpotLights[j].Position - posW;
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

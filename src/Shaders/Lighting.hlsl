// ═══════════════════════════════════════════════════════════════════
//  Lighting.hlsl — Deferred Rendering Lighting Pass
//  풀스크린 삼각형으로 G-Buffer를 읽고 Blinn-Phong + PCF Shadow 계산
//
//  Root Parameters:
//    [0] b0 PassCB
//    [1] t0 Shadow Map    (SamplerComparisonState s0)
//    [2] t1 GBuffer Albedo, t2 Normal, t3 WorldPos (SamplerState s1)
// ═══════════════════════════════════════════════════════════════════

struct DirectionalLightData
{
    float3 Direction;
    float  Padding1;
    float3 Color;
    float  Intensity;
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
    float3 L = normalize(-gDirLight.Direction);
    float3 V = normalize(gEyePos - posW);
    float3 H = normalize(L + V);

    // Diffuse
    float  NdotL   = max(dot(N, L), 0.0f);
    float3 diffuse = NdotL * gDirLight.Color * gDirLight.Intensity;

    // Specular (Blinn-Phong)
    float  NdotH    = max(dot(N, H), 0.0f);
    float3 specular = pow(NdotH, 64.0f) * gDirLight.Color * 0.3f;

    // Ambient
    float3 ambient = gAmbientLight.rgb;

    // Shadow
    float shadow = CalcShadow(posW);

    float3 result = ambient * albedo + shadow * (diffuse * albedo + specular);

    return float4(result, 1.0f);
}

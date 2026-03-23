// ═══════════════════════════════════════════════════════════════════
//  Default.hlsl — Blinn-Phong + Shadow Map
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
};

Texture2D gShadowMap : register(t0);
SamplerComparisonState gShadowSampler : register(s0);

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
//  Shadow 계산
// ═══════════════════════════════════════════════════════════════════
float CalcShadow(float3 posW)
{
    float4 lightClip = mul(float4(posW, 1.0f), gLightViewProj);
    lightClip.xyz /= lightClip.w;

    float2 shadowUV;
    shadowUV.x = lightClip.x * 0.5f + 0.5f;
    shadowUV.y = -lightClip.y * 0.5f + 0.5f;

    float currentDepth = lightClip.z;

    if (shadowUV.x < 0 || shadowUV.x > 1 || shadowUV.y < 0 || shadowUV.y > 1)
        return 1.0f;

    // PCF 3x3
    float shadow = 0.0f;
    float2 texelSize = float2(1.0f / 2048.0f, 1.0f / 2048.0f);

    [unroll]
    for (int x = -1; x <= 1; x++)
    {
        [unroll]
        for (int y = -1; y <= 1; y++)
        {
            shadow += gShadowMap.SampleCmpLevelZero( gShadowSampler
                                                    , shadowUV + float2(x, y) * texelSize
                                                    , currentDepth);
        }
    }

    return shadow / 9.0f;
}

// ═══════════════════════════════════════════════════════════════════
//  Pixel Shader — Blinn-Phong + Shadow
// ═══════════════════════════════════════════════════════════════════
float4 PSMain(VSOutput input) : SV_TARGET
{
    float3 N = normalize(input.NormalW);            // Normal
    float3 L = normalize(-gDirLight.Direction);     // Light (Directional)
    float3 V = normalize(gEyePos - input.PosW);     // View
    float3 H = normalize(L + V);                    // Half

    // Diffuse
    float NdotL = max(dot(N, L), 0.0f);
    float3 diffuse = NdotL * gDirLight.Color * gDirLight.Intensity;

    // Specular (Blinn-Phong)
    float NdotH = max(dot(N, H), 0.0f);
    float spec = pow(NdotH, 64.0f);
    float3 specular = spec * gDirLight.Color * 0.5f;

    // Ambient
    float3 ambient = gAmbientLight.rgb;

    // Shadow
    float shadow = CalcShadow(input.PosW);

    float3 baseColor = float3(0.7f, 0.7f, 0.7f);

    float3 result = ambient * baseColor + shadow * (diffuse * baseColor + specular);

    return float4(result, 1.0f);
}

// ═══════════════════════════════════════════════════════════════════
//  Default.hlsl
// ═══════════════════════════════════════════════════════════════════

// ── Constant Buffers ──
// register(b0) = Root Parameter [0]
cbuffer ObjectCB : register(b0)
{
    float4x4 gWorld;
};

// register(b1) = Root Parameter [1]
cbuffer PassCB : register(b1)
{
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewProj;
    float3   gEyePos;
    float    gPadding1;
    float4   gAmbientLight;
};

// 정점 입출력
struct VSInput
{
    float3 PosL    : POSITION;    // 로컬 좌표
    float3 NormalL : NORMAL;      // 로컬 노멀
    float2 TexC    : TEXCOORD;
};

struct VSOutput
{
    float4 PosH    : SV_POSITION; // 클립 좌표
    float3 PosW    : POSITION;    // 월드 좌표
    float3 NormalW : NORMAL;      // 월드 노멀
    float2 TexC    : TEXCOORD;
};

// ═══════════════════════════════════════════════════════════════════
//  Vertex Shader
// ═══════════════════════════════════════════════════════════════════
VSOutput VSMain(VSInput input)
{
    VSOutput output;

    // 로컬 -> 월드 변환
    float4 posW = mul(float4(input.PosL, 1.0f), gWorld);
    output.PosW = posW.xyz;

    // 월드 -> 클립 변환
    output.PosH = mul(posW, gViewProj);

    // 노멀 변환
    // 비균일 스케일링이 없다고 가정, 추후 World 역전치 추가
    output.NormalW = mul(input.NormalL, (float3x3)gWorld);

    output.TexC = input.TexC;

    return output;
}

// ═══════════════════════════════════════════════════════════════════
//  Pixel Shader
// ═══════════════════════════════════════════════════════════════════
float4 PSMain(VSOutput input) : SV_TARGET
{
    // 노멀 정규화
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

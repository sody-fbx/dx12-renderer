// ═══════════════════════════════════════════════════════════════════
//  Shadow.hlsl — Shadow Map 생성
// ═══════════════════════════════════════════════════════════════════

cbuffer ObjectCB : register(b0)
{
    float4x4 gWorld;
};

cbuffer SPCB : register(b1)
{
    float4x4 gLightViewProj;
};

struct VSInput
{
    float3 PosL   : POSITION;
    float3 Normal : NORMAL;
    float2 TexC   : TEXCOORD;
};

float4 VSMain(VSInput input) : SV_POSITION
{
    float4 posW = mul(float4(input.PosL, 1.0f), gWorld);
    return mul(posW, gLightViewProj);
}

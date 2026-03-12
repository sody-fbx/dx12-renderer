// ═══════════════════════════════════════════════════════════════════
//  Triangle.hlsl
// ═══════════════════════════════════════════════════════════════════

// 정점 셰이더 입력
// C++ 코드의 Input Layout과 semantic이 일치해야 함.
struct VSInput
{
    float3 position : POSITION;
    float4 color    : COLOR;
};

// 정점 셰이더 → 픽셀 셰이더 전달 구조체
struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

// ═══════════════════════════════════════════════════════════════════
//  Vertex Shader
// ═══════════════════════════════════════════════════════════════════
VSOutput VSMain(VSInput input)
{
    VSOutput output;

    output.position = float4(input.position, 1.0f);
    output.color    = input.color;

    return output;
}

// ═══════════════════════════════════════════════════════════════════
//  Pixel Shader
// ═══════════════════════════════════════════════════════════════════
float4 PSMain(VSOutput input) : SV_TARGET
{
    return input.color;
}

cbuffer CircleParametersConstantBuffer : register(b0)
{
    float3 centerPosition;
    float radius;
}

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    if (pow(abs(input.pos.x - centerPosition.x), 2) + pow(abs(input.pos.y - centerPosition.y), 2) - pow(radius, 2) > 0)
    {
        discard;
    }

    return float4(input.color, 1.0f);
}
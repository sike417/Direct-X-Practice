cbuffer CombinedViewProjectionConstantBuffer : register(b0)
{
    matrix vpMatrix;
}

cbuffer ModelConstantBuffer : register(b1)
{
    matrix model;
};

struct VertexShaderInput
{
    float3 pos : POSITION;
    float3 color : COLOR0;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.pos, 1.0f);

    // Transform the vertex position into projected space.
    pos = mul(pos, model);
    pos = mul(pos, vpMatrix);

    output.pos = pos;

    // Pass the color through without modification.
    output.color = input.color;

    return output;
}
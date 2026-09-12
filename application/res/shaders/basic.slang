struct VSInput{
    float2 inPosition;
    float3 inColor;
    float2 inTexCoord;
}

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;
Sampler2D texture;

struct VertexOutput {
    float4 pos: SV_Position;
    float3 color;
    float2 fragTexCoord;
};

[shader("vertex")]
VertexOutput vertMain(VSInput input) {
    VertexOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPosition, 0.0, 1.0))));
    output.color = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    return output;
};


[shader("fragment")]
float4 fragMain(VertexOutput inVert): SV_Target {
    return texture.Sample(inVert.fragTexCoord);
}
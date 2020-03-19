struct VS_INPUT
{
    float3 inAttribute0_vec3 : IN_ATTRIBUTE0;
    float2 inAttribute1_vec2 : IN_ATTRIBUTE1;
    float4 inAttribute3_vec4 : IN_ATTRIBUTE2;
};

struct VS_OUTPUT 
{
   float2 outAttribute0_vec2: OUT_ATTRIBUTE0;
   float4 outAttribute1_vec4: OUT_ATTRIBUTE1;
};

struct UBO01_size192
{
    float4x4 projectionMatrix0;
    float4x4 modelMatrix0;
    float4x4 viewMatrix0;
};

struct UBO00_size64
{
    float4x4 projectionMatrix1;
};

struct UBO11_size128
{
    float4x4 modelMatrix2;
    float4x4 viewMatrix2;
};

ConstantBuffer<UBO01_size192> ubo01_size192 : register(b1, space0);
ConstantBuffer<UBO00_size64>  ubo00_size64 	: register(b0, space0);
ConstantBuffer<UBO11_size128> ubo11_size128 : register(b1, space1);

VS_OUTPUT main(VS_INPUT Input)
{
   VS_OUTPUT Output;
   
   Output.outAttribute0_vec2 = Input.inAttribute1_vec2;
   
   float4 vertex = mul(ubo01_size192.modelMatrix0, float4(Input.inAttribute0_vec3, 1.0));
   Output.outAttribute1_vec4 = mul(ubo00_size64.projectionMatrix1, vertex);
   Output.outAttribute1_vec4 = mul(ubo11_size128.viewMatrix2, Output.outAttribute1_vec4);

   return Output;
}
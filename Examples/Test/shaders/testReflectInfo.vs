struct VS_INPUT
{
    float3 inAttribute0_vec3 : IN_ATTRIBUTE0;
    float4 inAttribute1_vec4 : IN_ATTRIBUTE1;
    float2 inAttribute2_vec2 : IN_ATTRIBUTE2;
};

struct UBO01_size192
{
    float4x4 projectionMatrix;
    float4x4 modelMatrix;
    float4x4 viewMatrix;
};

struct UBO00_size64
{
    float4x4 projectionMatrix;
};

struct UBO11_size256
{
    float4x4 modelMatrix[2];
    float4x4 viewMatrix[2];
};

ConstantBuffer<UBO01_size192> ubo01_size192   : register(b1, space0);
ConstantBuffer<UBO00_size64>  ubo00_size64    : register(b0, space0);
ConstantBuffer<UBO11_size256> ubo11_size256   : register(b1, space1);


struct VS_OUTPUT 
{
   float4 Position: SV_POSITION;
   float4 outAttribute0_vec4: OUT_ATTRIBUTE0;
   float2 outAttribute1_vec2: OUT_ATTRIBUTE1;
};

VS_OUTPUT main(VS_INPUT Input)
{
   VS_OUTPUT Output;
   
   Output.outAttribute1_vec2 = Input.inAttribute2_vec2;
   
   float4 vertex = mul(ubo01_size192.modelMatrix, float4(Input.inAttribute0_vec3, 1.0));
   Output.outAttribute0_vec4 = mul(ubo00_size64.projectionMatrix, vertex);
   Output.outAttribute0_vec4 = mul(ubo11_size256.viewMatrix[1], Output.outAttribute0_vec4);

   Output.Position = Output.outAttribute0_vec4;

   return Output;
}
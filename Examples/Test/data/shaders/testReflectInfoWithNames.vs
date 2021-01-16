struct VS_INPUT
{
    float3 inAttribute0_vec3 : IN_ATTRIBUTE0;
    float4 inAttribute1_vec4 : IN_ATTRIBUTE1;
    float2 inAttribute2_vec2 : IN_ATTRIBUTE2;
};

struct UBO01_size192
{
    float4x4 matrix0_01;
    float4x4 matrix1_01;
    float4x4 matrix2_01;
};

struct UBO00_size64
{
    float4x4 matrix0_00;
};

struct UBO11_size256
{
    float4x4 matrix0_11[2];
    float4x4 matrix1_11[2];
};

ConstantBuffer<UBO01_size192> ubo01_size192 : register(b1, space0);
ConstantBuffer<UBO00_size64>  ubo00_size64 : register(b0, space0);
ConstantBuffer<UBO11_size256> ubo11_size256 : register(b1, space1);

struct PS_INPUT 
{
   float4 Position : SV_POSITION;
   
   float4 attribute0_vec4 : ATTRIBUTE0;
   float2 attribute1_vec2 : ATTRIBUTE1;
   float3 attribute2_vec3 : ATTRIBUTE2;  
};

PS_INPUT main(VS_INPUT Input)
{
    PS_INPUT Output;
   
    float4 vertex = mul(ubo01_size192.matrix1_01, float4(Input.inAttribute0_vec3, 1.0));
    Output.attribute0_vec4 = vertex;
    Output.attribute1_vec2 = Input.inAttribute2_vec2;
    Output.attribute2_vec3 = mul((float3x3)ubo01_size192.matrix0_01, Input.inAttribute1_vec4.xyz);
    
    Output.Position = mul(ubo11_size256.matrix1_11[1], mul(ubo00_size64.matrix0_00, vertex));

    return Output;
}
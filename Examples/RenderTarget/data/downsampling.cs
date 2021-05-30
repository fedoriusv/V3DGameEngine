RWTexture2D<float4> inputImage : register(u0);
RWTexture2D<float4> resultImage : register(u1);

float3 image2DInterpolation(uint2 texCoord)
{
    uint2 size;
    inputImage.GetDimensions(size.x, size.y);
    const float2 texelSize = float2(1.0, 1.0) / size;
    
    float4 p0q0 = inputImage[texCoord];
    float4 p1q0 = inputImage[texCoord + uint2(1.0, 0.0)];
    float4 p0q1 = inputImage[texCoord + uint2(0.0, 1.0)];
    float4 p1q1 = inputImage[texCoord + uint2(1.0 , 1.0)];

    //float2 a = fract(texCoord * texelSize);
    float2 a = float2(0.5, 0.5);

    float4 interpq0 = lerp(p0q0, p1q0, a.x);
    float4 interpq1 = lerp(p0q1, p1q1, a.x);

    return lerp(interpq0, interpq1, a.y).xyz;
}

[numthreads( 4, 4, 1 )]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float3 color = image2DInterpolation(DTid.xy);
    resultImage[DTid.xy / 2] = float4(color, 1.0);
}
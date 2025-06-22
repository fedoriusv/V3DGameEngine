#ifndef _GLOBAL_HLSL_
#define _GLOBAL_HLSL_

#define HLSL 1
#define GLSL 0

#define REVERSED_DEPTH 1


float linearize_depth(in float d, in float zNear, in float zFar)
{
    return zNear * zFar / (zNear + d * (zFar - zNear));
}

#endif //_GLOBAL_HLSL_
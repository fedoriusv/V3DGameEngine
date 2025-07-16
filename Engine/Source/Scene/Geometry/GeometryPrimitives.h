#pragma once

#include "Common.h"

namespace v3d
{
namespace renderer
{
    class Device;
    class CmdListRender;
} //namespace renderer
namespace scene
{
    class Mesh;

    class Primitives
    {
    public:

        static Mesh* createCube(renderer::Device* device, renderer::CmdListRender* cmdList, f32 extent = 1.0f);
        static Mesh* createSphere(renderer::Device* device, renderer::CmdListRender* cmdList, f32 radius, u32 stacks = 64, u32 slices = 64);
        static Mesh* createCone(renderer::Device* device, renderer::CmdListRender* cmdList, f32 radius, f32 height, u32 segments = 64);
        static Mesh* createPlane(renderer::Device* device, renderer::CmdListRender* cmdList, f32 width, f32 depth, u32 xSegments = 64, u32 zSegments = 64);
        static Mesh* createGrid(renderer::Device* device, renderer::CmdListRender* cmdList, f32 cellSize, u32 cellCountX = 64, u32 cellCountZ = 64);
    };

} //namespace scene
} //namespace v3d
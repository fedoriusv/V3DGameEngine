#pragma once

#include <vulkan/vulkan.h>

#ifndef VK_QCOM_render_pass_transform
#   define VK_QCOM_render_pass_transform                   1
#   define VK_QCOM_RENDER_PASS_TRANSFORM_SPEC_VERSION      1
#   define VK_QCOM_RENDER_PASS_TRANSFORM_EXTENSION_NAME    "VK_QCOM_render_pass_transform"

#   define VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM_                        1000282000
#   define VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM_   1000282001
#   define VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM                                        0x00000002
    typedef struct VkRenderPassTransformBeginInfoQCOM {
    VkStructureType                sType;
    void*                          pNext;
    VkSurfaceTransformFlagBitsKHR  transform;
    } VkRenderPassTransformBeginInfoQCOM;

    typedef struct VkCommandBufferInheritanceRenderPassTransformInfoQCOM {
    VkStructureType                sType;
    void*                          pNext;
    VkSurfaceTransformFlagBitsKHR  transform;
    VkRect2D                       renderArea;
    } VkCommandBufferInheritanceRenderPassTransformInfoQCOM;
#else
#   define VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM_                        1000282000
#   define VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM_   1000282001
#endif //VK_QCOM_render_pass_transform
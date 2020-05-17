#pragma once

#include "Common.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DFence;
    class D3DCommandList;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DRenderResource final class. DirectX Render side
    */
    class D3DRenderResource
    {

    public:

        explicit D3DRenderResource() noexcept;
        ~D3DRenderResource();

        bool isUsed() const;
        void waitToComplete();

    private:

        bool attachFence(D3DFence* fence, u64 frame = 0);
        bool detachFence(D3DFence* fence);
        void releaseFences();
    private:

        friend D3DCommandList;

        std::set<D3DFence*> m_fences;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DResourceDeleter class. Vulkan Render side
    */
    class D3DResourceDeleter final
    {
    public:

        D3DResourceDeleter() = default;
        D3DResourceDeleter(const D3DResourceDeleter&) = delete;
        ~D3DResourceDeleter();

        void requestToDelete(D3DRenderResource* resource, const std::function<void(void)>& deleter);
        void update(bool wait = false);

    private:

        void garbageCollect();

        std::queue<std::pair<D3DRenderResource*, std::function<void(void)>>> m_delayedList;
        std::queue<std::pair<D3DRenderResource*, std::function<void(void)>>> m_deleterList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER

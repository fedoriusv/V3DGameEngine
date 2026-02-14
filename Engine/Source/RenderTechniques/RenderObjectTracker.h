#pragma once

#include "Common.h"
#include "Object.h"
#include "Thread/Spinlock.h"
#include "Utils/StringID.h"

#include "Renderer/Texture.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderObjectTracker class
    */
    class RenderObjectTracker
    {
    public:

        RenderObjectTracker() = default;
        ~RenderObjectTracker() = default;

        void bind(const utils::StringID& id, ObjectHandle handle);
        ObjectHandle get(const utils::StringID& id) const;

        void cleanup();
        bool empty() const;

        struct Hash
        {
            constexpr u64 operator()(const utils::StringID& obj) const
            {
                return obj.value();
            }
        };

    private:

        mutable thread::Spinlock m_mutex;
        std::unordered_map<utils::StringID, ObjectHandle, Hash> m_resources;
    };

    inline void RenderObjectTracker::bind(const utils::StringID& id, ObjectHandle handle)
    {
        std::scoped_lock lock(m_mutex);

        auto found = m_resources.emplace(id, handle);
        if (!found.second)
        {
            found.first->second = handle;
        }
    }

    inline ObjectHandle RenderObjectTracker::get(const utils::StringID& id) const
    {
        std::scoped_lock lock(m_mutex);

        if (auto found = m_resources.find(id); found != m_resources.cend())
        {
            return found->second;
        }

        return ObjectHandle();
    }

    inline void RenderObjectTracker::cleanup()
    {
        std::scoped_lock lock(m_mutex);

        m_resources.clear();
    }

    inline bool RenderObjectTracker::empty() const
    {
        return m_resources.empty();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
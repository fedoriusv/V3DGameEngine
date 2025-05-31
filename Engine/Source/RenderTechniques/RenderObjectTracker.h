#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/StringID.h"

#include "Renderer/Texture.h"

namespace v3d
{
namespace renderer
{

    class RenderObjectTracker
    {
    public:

        RenderObjectTracker() = default;
        ~RenderObjectTracker() = default;

        void bind(const utils::StringID& id, ObjectHandle handle);
        ObjectHandle get(const utils::StringID& id) const;

    private:

        std::map<utils::StringID, ObjectHandle> m_resources;
    };

    inline void RenderObjectTracker::bind(const utils::StringID& id, ObjectHandle handle)
    {
        auto found = m_resources.emplace(id, handle);
        if (!found.second)
        {
            found.first->second = handle;
        }
    }

    inline ObjectHandle RenderObjectTracker::get(const utils::StringID& id) const
    {
        if (auto found = m_resources.find(id); found != m_resources.cend())
        {
            return found->second;
        }

        return ObjectHandle();
    }

} //namespace renderer
} //namespace v3d
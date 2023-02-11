#pragma once

#include "Common.h"
#include "Object.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TRenderObject>
    class ObjectTracker;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @bief RenderObject class
    */
    template<class TRenderObject>
    class RenderObject
    {
    public:

        RenderObject()
            : m_countLinks(0)
        {
        }

        virtual ~RenderObject() = default;

        bool linked() const
        {
            return m_countLinks > 0;
        }

    private:

        void link()
        {
            ++m_countLinks;
        }

        void unlink()
        {
            --m_countLinks;
            ASSERT(m_countLinks >= 0, "less zero");
        }

        s32 count() const
        {
            return m_countLinks;
        }
        friend ObjectTracker<TRenderObject>;

        std::atomic<s32> m_countLinks;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ObjectTracker class. Collecting RenderObject
    */
    template<class TRenderObject>
    class ObjectTracker
    {
    public:

        ObjectTracker(Object* handle, std::function<void(const std::vector<TRenderObject*>&)> deleteCallback) noexcept
            : m_handle(handle)
            , m_deleteCallback(deleteCallback)
        {
        }

        bool attach(TRenderObject* object) noexcept
        {
            static_assert(std::is_base_of<RenderObject<TRenderObject>, TRenderObject>(), "wrong type");
            std::lock_guard<std::recursive_mutex> lock(m_mutex);

            auto iter = m_list.insert(object);
            if (iter.second)
            {
                object->link();
            }
            return iter.second;
        }

        void release() noexcept
        {
            std::lock_guard<std::recursive_mutex> lock(m_mutex);

            std::vector<TRenderObject*> deleteList;
            for (auto& iter : m_list)
            {
                RenderObject<TRenderObject>* object = iter;
                object->unlink();
                if (!object->count())
                {
                    deleteList.push_back((TRenderObject*)object);
                }
            }

            if (!deleteList.empty())
            {
                m_deleteCallback(deleteList);
            }
        }

    private:

        std::recursive_mutex m_mutex;
        std::set<RenderObject<TRenderObject>*> m_list;

        Object* m_handle;
        std::function<void(const std::vector<TRenderObject*>&)> m_deleteCallback;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d

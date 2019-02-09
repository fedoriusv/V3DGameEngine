#pragma once

#include "Common.h"
#include "Object.h"

namespace v3d
{
namespace renderer
{
    template<class TRenderObject>
    class ObjectTracker;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * RenderObject class
    */
    template<class TRenderObject>
    class RenderObject
    {
    public:

        RenderObject()
            : m_couter(0)
        {
        }

        virtual ~RenderObject() {}

        bool linked() const
        {
            return m_couter > 0;
        }

    private:
        void link()
        {
            ++m_couter;
        }

        void unlink()
        {
            --m_couter;
            ASSERT(m_couter >= 0, "less zero");
        }

        s32 count() const
        {
            return m_couter;
        }
        friend ObjectTracker<TRenderObject>;

        std::atomic<s32> m_couter;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ObjectTracker class. Collecting RenderObject
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
            static_assert(std::is_base_of<RenderObject<TRenderObject>, TRenderObject>());
            std::lock_guard<std::recursive_mutex> lock(m_mutex);

            auto iter = m_list.emplace(object);
            object->link();
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

#pragma once

#include "Common.h"

namespace v3d
{
namespace task
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    struct ThreadSafeNode
    {
        std::atomic<T*> _next = nullptr;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    class ThreadSafeStack
    {
    public:

        void push(T* elem)
        {
            T* old_head = m_head.load(std::memory_order_acquire);
            T* new_head = elem;

            do
            {
                new_head->_next.store(old_head, std::memory_order_release);
            }
            while (!m_head.compare_exchange_weak(old_head, new_head));
        }

        T* pop()
        {
            T* old_head = m_head.load(std::memory_order_acquire);
            T* new_head = nullptr;

            do
            {
                if (!old_head)
                {
                    return nullptr;
                }
                new_head = old_head->_next.load(std::memory_order_acquire);
            }
            while (!m_head.compare_exchange_weak(old_head, new_head));

            return old_head;
        }

    private:

        std::atomic<T*> m_head = nullptr;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    class ThreadSafeQueue
    {
    public:

        void enqueue(T* elem)
        {
            T* new_node = elem;

            while (true)
            {
                T* tail_node = m_tail.load(std::memory_order_acquire);
                if (!tail_node)
                {
                    if (m_head.compare_exchange_weak(tail_node, new_node, std::memory_order_release, std::memory_order_relaxed))
                    {
                        m_tail.store(new_node, std::memory_order_release);
                        return;
                    }
                }
                else
                {
                    T* next = nullptr;
                    if (tail_node->_next.compare_exchange_weak(next, new_node, std::memory_order_release, std::memory_order_relaxed))
                    {
                        m_tail.store(new_node, std::memory_order_release);
                        return;
                    }
                }
            }
        }

        T* dequeue()
        {
            T* head_node = nullptr;

            while (true)
            {
                head_node = m_head.load(std::memory_order_acquire);
                if (!head_node)
                {
                    return nullptr;
                }

                T* next_node = head_node->_next.load(std::memory_order_acquire);
                if (m_head.compare_exchange_weak(head_node, next_node))
                {
                    return head_node;
                }
            }
        }

    private:

        std::atomic<T*> m_head = nullptr;
        std::atomic<T*> m_tail = nullptr;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    class ThreadSafeList
    {
        //TODO
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namesapce task
} // namespace v3d
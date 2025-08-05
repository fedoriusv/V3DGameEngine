#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename Type>
    class Reporter;

    /**
    * @brief Observer interface.
    */
    template<typename Type>
    class V3D_API Observer
    {
    public:

        virtual void handleNotify(const Reporter<Type>* reporter, const Type& data) = 0;

    protected:

        Observer() = default;
        ~Observer() = default;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////


    /**
    * @brief Reporter class.
    */
    template<typename Type>
    class V3D_API Reporter
    {
    public:

        void registerNotify(Observer<Type>* object)
        {
            std::lock_guard lock(m_mutex);
            m_observers.insert(object);
        }

        void unregisterNotify(Observer<Type>* object)
        {
            std::lock_guard lock(m_mutex);
            m_observers.erase(object);
        }

        void unregisterAll()
        {
            std::lock_guard lock(m_mutex);
            m_observers.clear();
        }

        void notify(const Type& data) const
        {
            std::set<Observer<Type>*> copy_observers;
            {
                std::lock_guard lock(m_mutex);
                copy_observers = m_observers;
            }

            for (Observer<Type>* observer : copy_observers)
            {
                observer->handleNotify(this, data);
            }
        }

    protected:

        Reporter() = default;
        ~Reporter() = default;

    private:

        mutable std::recursive_mutex    m_mutex;
        std::set<Observer<Type>*>       m_observers;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////


} //namespace utils
} //namespace v3d

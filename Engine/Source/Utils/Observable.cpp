#include "Observable.h"

namespace v3d
{
namespace utils
{

void Observable::registerNotify(Observer* object)
{
    std::lock_guard lock(m_mutex);
    m_observers.insert(object);
}

void Observable::unregisterNotify(Observer* object)
{
    std::lock_guard lock(m_mutex);
    m_observers.erase(object);
}

void Observable::unregisterAll()
{
    std::lock_guard lock(m_mutex);
    m_observers.clear();
}

void Observable::notifyObservers(void* msg) const
{
    std::set<Observer*> copy_observers;
    {
        std::lock_guard lock(m_mutex);
        copy_observers = m_observers;
    }

    for (auto& observer : copy_observers)
    {
        observer->handleNotify(this, msg);
    }
}

} //namespace utils
} //namespace v3d

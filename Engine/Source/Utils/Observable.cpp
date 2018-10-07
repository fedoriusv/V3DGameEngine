#include "Observable.h"

namespace v3d
{
namespace utils
{

//TODO: set thread safe
void Observable::registerNotify(Observer* obj)
{
    m_observers.insert(obj);
}

void Observable::unregisterNotify(Observer* obj)
{
    m_observers.erase(obj);
}

void Observable::unregisterAll()
{
    m_observers.clear();
}

void Observable::notifyObservers()
{
    for (auto& observer : m_observers)
    {
        observer->handleNotify(this);
    }
}

} //namespace utils
} //namespace v3d

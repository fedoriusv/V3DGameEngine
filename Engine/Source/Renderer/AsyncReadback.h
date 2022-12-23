#pragma once

#include "Common.h"

namespace v3d
{
namespace renderer
{

    template<class Callback>
    class AsyncReadback
    {
    public:
        AsyncReadback() {};

        std::function<Callback> m_callback;
    };

} //namespace renderer
} //namespace v3d
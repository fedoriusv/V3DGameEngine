#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class IndexGenerator
    {
    public:

        IndexGenerator();
        ~IndexGenerator();

        u64 index() const;

    private:

        u64 m_index;

        static void generateteIndex(u64& index);
        static u64 s_indexGenerator;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d

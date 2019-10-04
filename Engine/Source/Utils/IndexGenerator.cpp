#include "IndexGenerator.h"

namespace v3d
{
namespace utils
{
    u64 IndexGenerator::s_indexGenerator = 0x0;

    IndexGenerator::IndexGenerator()
        : m_index(0U) 
    {
        generateteIndex(m_index);
    }

    IndexGenerator::~IndexGenerator()
    {
    }

    u64 IndexGenerator::index() const
    {
        return m_index;
    }

    void IndexGenerator::generateteIndex(u64& index)
    {
        ASSERT(s_indexGenerator != std::numeric_limits<u64>::max(), "max value");
        ++s_indexGenerator;

        index = s_indexGenerator;
    }

} //namespace utils
} //namespace v3d

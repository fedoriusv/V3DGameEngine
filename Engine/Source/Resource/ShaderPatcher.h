#pragma once

#include "Common.h"

#ifdef USE_SPIRV
#include "ShaderPatchSpirV.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class PatchDriverBugOptimization final : public PatchSpirvCode
    {
    public:

        explicit PatchDriverBugOptimization(u32 flags = 0) noexcept;
        ~PatchDriverBugOptimization() = default;

        bool patch(std::vector<u32>& spirv, u32 flags = 0) override;

    private:

        bool composePatch(std::function<bool(std::vector<u32>& subSpirv)> func, std::vector<u32>& subSpirv);
        [[maybe_unused]] u32 m_flags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class PatchRemoveUnusedLocations final : public PatchSpirvCode
    {
    public:

        PatchRemoveUnusedLocations() = default;
        ~PatchRemoveUnusedLocations() = default;

        bool collectDataFromSpirv(const std::vector<u32>& spirv);

        bool patch(std::vector<u32>& spirv, u32 flags = 0) override;

    private:

        struct Location
        {
            u32 bindID;
            u32 typePointerID;
            u32 typeID;
            u32 executionModel;
            u32 storageClass;
            u32 location;
        };

        struct Type
        {
            u32 bindID = (u32)~0;
            u32 typeID = (u32)~0;
            u32 type = (u32)~0;
            u32 components = 0;
        };

        void extractUnusedLocations(std::vector<Location>& locations);

        std::vector<Location> m_locations;
        std::array<Type, 4> m_vector;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV

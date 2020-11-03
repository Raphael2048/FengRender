#pragma once

#include "util/defines.hpp"
#include "scene/static_mesh.hpp"
#undef max
#undef min

namespace feng
{
    class AssimpMeshLoader
    {
    public:
        static class std::vector<std::shared_ptr<Mesh>> LoadModel(const std::string& path);
    };
} // namespace feng

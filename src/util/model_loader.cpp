
#include "util/model_loader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace feng
{
    std::vector<std::shared_ptr<Mesh>> AssimpMeshLoader::LoadModel(const std::string& path)
    {
        
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path.data(),
			aiProcess_Triangulate |
			aiProcess_CalcTangentSpace |
			aiProcess_JoinIdenticalVertices |
			aiProcess_OptimizeMeshes |
			aiProcess_ImproveCacheLocality | 
			aiProcess_MakeLeftHanded);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			FMSG(std::string("Loading model ") +
				path.data() +
				std::string(" failed with error ") +
				importer.GetErrorString());
			return std::vector<std::shared_ptr<Mesh>>();
		}
        std::vector<std::shared_ptr<Mesh>> mesh_vector{};

        for (int i = 0; i < scene->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[i];

        }

        return mesh_vector;

    }
}
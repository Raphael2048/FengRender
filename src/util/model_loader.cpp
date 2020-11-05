
#include "util/model_loader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace feng
{
	void CV3(Vector3& v, aiVector3D& v2)
	{
		std::memcpy(&v, &v2, sizeof(float) * 3);
	}

	void CV2(Vector2& v, aiVector3D& v2)
	{
		std::memcpy(&v, &v2, sizeof(float) * 2);
	}
	
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

        std::vector<std::shared_ptr<Mesh>> mesh_vector{};
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			FMSG(std::string("Loading model ") +
				path.data() +
				std::string(" failed with error ") +
				importer.GetErrorString());
			return mesh_vector;
		}

        for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[i];
			Vertex* vertex_array = (Vertex*)std::malloc(sizeof(Vertex) * mesh->mNumVertices);
			//std::vector<Vertex> vertex_array;
			//vertex_array.resize(mesh->mNumVertices);
			for(unsigned int j = 0; j < mesh->mNumVertices; j++)
			{
				CV3(vertex_array[j].pos, mesh->mVertices[j]);
				CV3(vertex_array[j].normal, mesh->mNormals[j]);
				CV3(vertex_array[j].tangent, mesh->mTangents[j]);
				CV2(vertex_array[j].uv, mesh->mTextureCoords[0][j]);
			}

			uint32_t* index_array = (uint32_t*)std::malloc(sizeof(uint32_t) * mesh->mNumFaces * 3);
			for(unsigned int j = 0; j < mesh->mNumFaces; j++)
			{
				std::memcpy(index_array + j * 3, mesh->mFaces[j].mIndices, sizeof(uint32_t) * 3);
			}

			mesh_vector.emplace_back(
				std::make_shared<Mesh>(
					vertex_array, mesh->mNumVertices, index_array, mesh->mNumFaces * 3
					)
			);

        }

        return mesh_vector;

    }
}
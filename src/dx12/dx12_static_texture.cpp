#include "dx12/dx12_static_texture.hpp"
#include "dx12/dx12_device.hpp"
#include "DDSTextureLoader.h"
#include "ResourceUploadBatch.h"
#include "DirectXHelpers.h"

namespace feng
{
    StaticTexture::StaticTexture(Device &device, DirectX::ResourceUploadBatch &uploader, const std::wstring &path)
    {
        device_ = &device;
        DirectX::CreateDDSTextureFromFile(
            device.GetDevice(), uploader, path.data(), buffer_.GetAddressOf());
        srv_heap_index_ = device.GetSTSRVAllocIndex();
        DirectX::CreateShaderResourceView(device.GetDevice(), buffer_.Get(), device.GetSTSRVHeap().GetCpuHandle(srv_heap_index_));
    }

    std::unordered_map<std::wstring, std::shared_ptr<StaticTexture>> StaticMaterial::textures_{};

    StaticMaterial::StaticMaterial(const std::wstring &base_color, const std::wstring &normal, const std::wstring &roughness_, const std::wstring &metallic)
        : base_color_path_(base_color), normal_path_(normal), roughness_path_(roughness_), metallic_path_(metallic)
    {
    }

    void StaticMaterial::Init(Device &device, DirectX::ResourceUploadBatch &uploader)
    {
        if (inited_)
            return;
        inited_ = true;
        auto it = textures_.find(base_color_path_);
        if (it != textures_.end())
        {
            base_color_ = it->second;
        }
        else
        {
            base_color_ = std::make_shared<StaticTexture>(device, uploader, base_color_path_);
            textures_.emplace(base_color_path_, base_color_);
        }

        it = textures_.find(normal_path_);
        if (it != textures_.end())
        {
            normal_ = it->second;
        }
        else
        {
            normal_ = std::make_shared<StaticTexture>(device, uploader, normal_path_);
            textures_.emplace(normal_path_, normal_);
        }

        it = textures_.find(roughness_path_);
        if (it != textures_.end())
        {
            roughness_ = it->second;
        }
        else
        {
            roughness_ = std::make_shared<StaticTexture>(device, uploader, roughness_path_);
            textures_.emplace(roughness_path_, roughness_);
        }

        it = textures_.find(metallic_path_);
        if (it != textures_.end())
        {
            metallic_ = it->second;
        }
        else
        {
            metallic_ = std::make_shared<StaticTexture>(device, uploader, metallic_path_);
            textures_.emplace(metallic_path_, metallic_);
        }
    }

} // namespace feng
#include "dx12/dx12_static_texture.hpp"
#include "dx12/dx12_device.hpp"
#include "DDSTextureLoader.h"
#include "ResourceUploadBatch.h"
#include "DirectXHelpers.h"

namespace feng
{
    StaticTexture::StaticTexture(Device &device, DirectX::ResourceUploadBatch &uploader, const std::wstring &path, bool srgb, bool cubemap)
    {
        device_ = &device;
        DirectX::DDS_LOADER_FLAGS flags = DirectX::DDS_LOADER_DEFAULT;
        if (srgb)
        {
            flags = flags | DirectX::DDS_LOADER_FORCE_SRGB;
        }
        DirectX::CreateDDSTextureFromFileEx(
            device.GetDevice(), uploader, path.data(), 0, D3D12_RESOURCE_FLAG_NONE, flags,
            buffer_.GetAddressOf(), nullptr, nullptr);
        srv_heap_index_ = device.GetSRVAllocIndex();
        DirectX::CreateShaderResourceView(device.GetDevice(), buffer_.Get(), device.GetSRVHeap().GetCpuHandle(srv_heap_index_), cubemap);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE StaticTexture::GetCPUSRV()
    {
        return device_->GetSRVHeap().GetCpuHandle(srv_heap_index_);
    }
    D3D12_GPU_DESCRIPTOR_HANDLE StaticTexture::GetGPUSRV()
    {
        return device_->GetSRVHeap().GetGpuHandle(srv_heap_index_);
    }
    //std::unordered_map<std::wstring, std::shared_ptr<StaticTexture>> StaticMaterial::textures_{};

    StaticMaterial::StaticMaterial(const std::wstring &base_color, const std::wstring &normal, const std::wstring &roughness_, const std::wstring &metallic)
        : base_color_path_(base_color), normal_path_(normal), roughness_path_(roughness_), metallic_path_(metallic)
    {
    }

    void StaticMaterial::Init(Device &device, DirectX::ResourceUploadBatch &uploader)
    {
        if (inited_)
            return;
        inited_ = true;
        // Coherent SRVS
        base_color_ = std::make_shared<StaticTexture>(device, uploader, base_color_path_, true);
        first_index_ = base_color_->GetSRVIndex();
        normal_ = std::make_shared<StaticTexture>(device, uploader, normal_path_);
        roughness_ = std::make_shared<StaticTexture>(device, uploader, roughness_path_);
        metallic_ = std::make_shared<StaticTexture>(device, uploader, metallic_path_);
        // auto it = textures_.find(base_color_path_);
        // if (it != textures_.end())
        // {
        //     base_color_ = it->second;
        // }
        // else
        // {
        //     base_color_ = std::make_shared<StaticTexture>(device, uploader, base_color_path_);
        //     textures_.emplace(base_color_path_, base_color_);
        // }

        // it = textures_.find(normal_path_);
        // if (it != textures_.end())
        // {
        //     normal_ = it->second;
        // }
        // else
        // {
        //     normal_ = std::make_shared<StaticTexture>(device, uploader, normal_path_);
        //     textures_.emplace(normal_path_, normal_);
        // }

        // it = textures_.find(roughness_path_);
        // if (it != textures_.end())
        // {
        //     roughness_ = it->second;
        // }
        // else
        // {
        //     roughness_ = std::make_shared<StaticTexture>(device, uploader, roughness_path_);
        //     textures_.emplace(roughness_path_, roughness_);
        // }

        // it = textures_.find(metallic_path_);
        // if (it != textures_.end())
        // {
        //     metallic_ = it->second;
        // }
        // else
        // {
        //     metallic_ = std::make_shared<StaticTexture>(device, uploader, metallic_path_);
        //     textures_.emplace(metallic_path_, metallic_);
        // }
    }

} // namespace feng
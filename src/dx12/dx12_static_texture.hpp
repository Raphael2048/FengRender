#pragma once
#include "dx12_defines.hpp"
#include "DDSTextureLoader.h"
#include <unordered_map>
namespace feng
{
    class Device;
    class StaticTexture
    {
    public:
        StaticTexture(Device &device, DirectX::ResourceUploadBatch &uplaoder, const std::wstring &path);
        D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return gpu_address_; };

    protected:
        // DXGI_FORMAT format_;
        // UINT width;
        // UINT height;
        ComPtr<ID3D12Resource> buffer_;
        D3D12_GPU_VIRTUAL_ADDRESS gpu_address_;
    };

    class StaticMaterial
    {
    public:
        StaticMaterial(const std::wstring& base_color, const std::wstring& normal, const std::wstring& roughness_, const std::wstring& metallic);
        void Init(Device &device, DirectX::ResourceUploadBatch &uploader);
    private:
        std::wstring base_color_path_, normal_path_, roughness_path_, metallic_path_;
        std::shared_ptr<StaticTexture> base_color_, normal_, roughness_, metallic_;
        static std::unordered_map<std::wstring, std::shared_ptr<StaticTexture>> textures_;
        bool inited_ = false;
    };
} // namespace feng

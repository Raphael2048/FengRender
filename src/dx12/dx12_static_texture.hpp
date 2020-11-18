#pragma once
#include "dx12_defines.hpp"
#include "DDSTextureLoader.h"
// #include <unordered_map>
namespace feng
{
    class Device;
    class StaticTexture
    {
    public:
        StaticTexture(Device &device, DirectX::ResourceUploadBatch &uplaoder, const std::wstring &path, bool srgb = false, bool cubemap = false);
        size_t GetSRVIndex() {return srv_heap_index_; };
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSRV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSRV();
        ID3D12Resource* GetBuffer() {return buffer_.Get(); }
    protected:
        ComPtr<ID3D12Resource> buffer_;
        size_t srv_heap_index_;
        Device* device_;
    };

    class StaticMaterial
    {
    public:
        StaticMaterial(const std::wstring& base_color, const std::wstring& normal, const std::wstring& roughness, const std::wstring& metallic);
        void Init(Device &device, DirectX::ResourceUploadBatch &uploader);
        std::shared_ptr<StaticTexture> base_color_, normal_, roughness_, metallic_;
        size_t first_index_;
    private:
        std::wstring base_color_path_, normal_path_, roughness_path_, metallic_path_;
        // static std::unordered_map<std::wstring, std::shared_ptr<StaticTexture>> textures_;
        bool inited_ = false;
    };
} // namespace feng

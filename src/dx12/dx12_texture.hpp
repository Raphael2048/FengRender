#pragma once
#include "dx12_defines.hpp"
#include "DDSTextureLoader.h"
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

    class DynamicTexture : Uncopyable
    {
    public:
        DynamicTexture(Device &device, UINT64 width, UINT64 height, DXGI_FORMAT typeless_format, DXGI_FORMAT read_format, DXGI_FORMAT write_format);
        DynamicTexture(Device &device, UINT64 width, UINT64 height, DXGI_FORMAT unified_format);
        void TransitionState(ID3D12GraphicsCommandList* command, D3D12_RESOURCE_STATES state);
        ID3D12Resource *GetResource() { return buffer_.Get(); }
        int GetSRVHeapIndex() {return srv_heap_index_; }
        int GetRTVHeapIndex() {return rtv_heap_index_; }
        int GetDSVHeapIndex() {return dsv_heap_index_; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSRV();
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPURTV();
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDSV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSRV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPURTV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDSV();
    private:
        Device* device_;
        D3D12_RESOURCE_STATES current_state_;
        ComPtr<ID3D12Resource> buffer_;
        int srv_heap_index_ = -10000;
        union
        {
            int rtv_heap_index_ = -10000;
            int dsv_heap_index_;
        };
    };
} // namespace feng
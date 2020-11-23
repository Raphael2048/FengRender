#pragma once
#include "dx12_defines.hpp"
#include "DDSTextureLoader.h"
namespace feng
{
    class Device;

    class StaticTexture : public Uncopyable
    {
    public:
        StaticTexture(Device &device, DirectX::ResourceUploadBatch &uplaoder, const std::wstring &path, bool srgb = false, bool cubemap = false);
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSRV();
        ID3D12Resource *GetBuffer() { return buffer_.Get(); }

    protected:
        ComPtr<ID3D12Resource> buffer_;
        size_t srv_heap_index_;
        Device *device_;
    };

    class StaticMaterial : public Uncopyable
    {
    public:
        StaticMaterial(const std::wstring &base_color, const std::wstring &normal, const std::wstring &roughness, const std::wstring &metallic);
        void Init(Device &device, DirectX::ResourceUploadBatch &uploader);
        std::shared_ptr<StaticTexture> base_color_, normal_, roughness_, metallic_;

    private:
        std::wstring base_color_path_, normal_path_, roughness_path_, metallic_path_;
        // static std::unordered_map<std::wstring, std::shared_ptr<StaticTexture>> textures_;
        bool inited_ = false;
    };

    class DynamicPlainTexture : public Uncopyable
    {
    public:
        DynamicPlainTexture(Device &device, UINT64 width, UINT64 height, DXGI_FORMAT format, bool need_rtv = true, bool need_uav = false);
        void TransitionState(ID3D12GraphicsCommandList *command, D3D12_RESOURCE_STATES state);
        ID3D12Resource *GetResource() { return buffer_.Get(); }
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPURTV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSRV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUUAV();

    private:
        Device *device_;
        D3D12_RESOURCE_STATES current_state_;
        ComPtr<ID3D12Resource> buffer_;
        int srv_heap_index_ = -1;
        int rtv_heap_index_ = -1;
        int uav_heap_index_ = -1;
    };

    class DynamicTexture : public Uncopyable
    {
    public:
        DynamicTexture(Device &device, UINT64 width, UINT64 height, DXGI_FORMAT typeless_format, DXGI_FORMAT read_format, DXGI_FORMAT write_format);
        DynamicTexture(Device &device, UINT64 width, UINT64 height, DXGI_FORMAT unified_format);
        void TransitionState(ID3D12GraphicsCommandList *command, D3D12_RESOURCE_STATES state);
        ID3D12Resource *GetResource() { return buffer_.Get(); }
        // CPU HANDELE: 设置RT, DS. CLEAR; GPU HANDLE 设置shader读取地址
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPURTV();
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDSV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSRV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUUAV();

    private:
        Device *device_;
        D3D12_RESOURCE_STATES current_state_;
        ComPtr<ID3D12Resource> buffer_;
        int srv_heap_index_ = -1;
        union
        {
            int rtv_heap_index_ = -1;
            int dsv_heap_index_;
        };
    };
} // namespace feng
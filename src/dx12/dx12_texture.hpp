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
        StaticMaterial(const std::wstring &albedo_metallic_path, const std::wstring &normal_roughness_path);
        void Init(Device &device, DirectX::ResourceUploadBatch &uploader);
        std::shared_ptr<StaticTexture> albedo_metallic_, normal_roughness_;

    private:
        std::wstring albedo_metallic_path_, normal_roughness_path_;
        // static std::unordered_map<std::wstring, std::shared_ptr<StaticTexture>> textures_;
        bool inited_ = false;
    };

    class DynamicTexture : public Uncopyable
    {
    public:
        void TransitionState(ID3D12GraphicsCommandList *command, D3D12_RESOURCE_STATES state);
        DynamicTexture(Device &device, UINT64 width, UINT64 height) : device_(&device), width_(width), height_(height) {}
        DynamicTexture() = default;
        ID3D12Resource *GetResource() { return buffer_.Get(); }
        UINT64 GetWidth() { return width_; };
        UINT64 GetHeight() { return height_; };

    protected:
        Device *device_;
        D3D12_RESOURCE_STATES current_state_;
        ComPtr<ID3D12Resource> buffer_;
        UINT64 width_;
        UINT64 height_;
    };

    class DynamicPlainTexture : public DynamicTexture
    {
    public:
        DynamicPlainTexture(Device &device, UINT64 width, UINT64 height, DXGI_FORMAT format, bool need_rtv = true, bool need_uav = false);
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPURTV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSRV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUUAV();

    private:
        int srv_heap_index_ = -1;
        int rtv_heap_index_ = -1;
        int uav_heap_index_ = -1;
    };

    class DynamicPlainTextureMips : public DynamicTexture
    {
    public:
        DynamicPlainTextureMips(Device &device, UINT64 width, UINT64 height, uint8_t mips, DXGI_FORMAT format, bool need_rtv, bool need_uav);
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSRV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSRVAt(UINT mip);
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPURTVAt(UINT mip);
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUUAVAt(UINT mip);

    private:
        uint8_t mips_;
        int srv_heap_index_;
        std::vector<int> srv_heap_index_each_;
        std::vector<int> uav_heap_index_each_;
        std::vector<int> rtv_heap_index_each_;
    };

    class DynamicPlain3DTexture : public DynamicTexture
    {
    public:
        DynamicPlain3DTexture(Device &device, UINT64 width, UINT64 height, UINT64 depth, DXGI_FORMAT format);
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSRV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUUAV();
    private:
        UINT64 depth_;
        int srv_heap_index_ = -1;
        int uav_heap_index_ = -1;
    };

    class DynamicDepthTexture : public DynamicTexture
    {
    public:
        DynamicDepthTexture(Device &device, UINT64 width, UINT64 height, DXGI_FORMAT typeless_format, DXGI_FORMAT read_format, DXGI_FORMAT write_format);
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDSV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSRV();

    protected:
        DynamicDepthTexture() = default;
        int srv_heap_index_ = -1;
        int dsv_heap_index_ = -1;
    };

    class DynamicDepthTextureCube : public DynamicDepthTexture
    {
    public:
        DynamicDepthTextureCube(Device &device, UINT64 width, DXGI_FORMAT typeless_format, DXGI_FORMAT read_format, DXGI_FORMAT write_format);
    };
} // namespace feng
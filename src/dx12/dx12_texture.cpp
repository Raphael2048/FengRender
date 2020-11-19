#include "dx12/dx12_texture.hpp"
#include "dx12/dx12_device.hpp"
#include "d3dx12.h"
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
    }

    DynamicTexture::DynamicTexture(Device &device, UINT64 width, UINT64 height, DXGI_FORMAT typeless_format, DXGI_FORMAT read_format, DXGI_FORMAT write_format)
    :device_(&device)
    {
        bool is_depth = write_format == DXGI_FORMAT_D32_FLOAT || write_format == DXGI_FORMAT_D24_UNORM_S8_UINT ||
                        write_format == DXGI_FORMAT_D16_UNORM || write_format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        D3D12_RESOURCE_DESC texDesc;
        ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Alignment = 0;
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.DepthOrArraySize = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = typeless_format;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags = is_depth ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_CLEAR_VALUE optClear;
        optClear.Format = write_format;
        if (is_depth)
        {
            optClear.DepthStencil.Depth = 0.0f;
            optClear.DepthStencil.Stencil = 0;
        }
        else
        {
            optClear.Color[0] = 0.0f;
            optClear.Color[1] = 0.0f;
            optClear.Color[2] = 0.0f;
            optClear.Color[3] = 0.0f;
        }

        auto HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        TRY(device.GetDevice()->CreateCommittedResource(
            &HeapProp,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            &optClear,
            IID_PPV_ARGS(&buffer_)));

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = read_format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        srvDesc.Texture2D.PlaneSlice = 0;
        srv_heap_index_ = device.GetSRVAllocIndex();
        device.GetDevice()->CreateShaderResourceView(
            buffer_.Get(),
            &srvDesc,
            device.GetSRVHeap().GetCpuHandle(srv_heap_index_));

        if (is_depth)
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Format = write_format;
            dsvDesc.Texture2D.MipSlice = 0;
            dsv_heap_index_ = device.GetDSVAllocIndex();
            device.GetDevice()->CreateDepthStencilView(
                buffer_.Get(),
                &dsvDesc,
                device.GetDSVHeap().GetCpuHandle(dsv_heap_index_));
        }
        else
        {
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Format = write_format;
            rtvDesc.Texture2D.MipSlice = 0;
            rtvDesc.Texture2D.PlaneSlice = 0;
            rtv_heap_index_ = device.GetRTVAllocIndex();
            device.GetDevice()->CreateRenderTargetView(
                buffer_.Get(),
                &rtvDesc,
                device.GetRTVHeap().GetCpuHandle(rtv_heap_index_));
        }

        current_state_ = D3D12_RESOURCE_STATE_GENERIC_READ;
    }

    DynamicTexture::DynamicTexture(Device& device, UINT64 width, UINT64 height, DXGI_FORMAT unified_format):
        DynamicTexture(device, width, height, unified_format, unified_format, unified_format){}



    void DynamicTexture::TransitionState(ID3D12GraphicsCommandList* command, D3D12_RESOURCE_STATES state)
    {
        if(state == current_state_) return;
        auto transition = CD3DX12_RESOURCE_BARRIER::Transition(buffer_.Get(), current_state_, state);
        command->ResourceBarrier(1, &transition);
        current_state_ = state;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DynamicTexture::GetCPUSRV()
    {
        return device_->GetSRVHeap().GetCpuHandle(srv_heap_index_);
    }
    D3D12_CPU_DESCRIPTOR_HANDLE DynamicTexture::GetCPURTV()
    {
        return device_->GetRTVHeap().GetCpuHandle(rtv_heap_index_);
    }
    D3D12_CPU_DESCRIPTOR_HANDLE DynamicTexture::GetCPUDSV()
    {
        return device_->GetDSVHeap().GetCpuHandle(dsv_heap_index_);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DynamicTexture::GetGPUSRV()
    {
        return device_->GetSRVHeap().GetGpuHandle(srv_heap_index_);
    }
    D3D12_GPU_DESCRIPTOR_HANDLE DynamicTexture::GetGPURTV()
    {
        return device_->GetRTVHeap().GetGpuHandle(rtv_heap_index_);
    }
    D3D12_GPU_DESCRIPTOR_HANDLE DynamicTexture::GetGPUDSV()
    {
        return device_->GetDSVHeap().GetGpuHandle(dsv_heap_index_);
    }
} // namespace feng

#include "dx12/dx12_dyncmic_texture.hpp"
#include "dx12/dx12_device.hpp"
#include "d3dx12.h"
namespace feng
{
    DynamicTexture::DynamicTexture(Device &device, UINT64 width, UINT64 height, DXGI_FORMAT typeless_format, DXGI_FORMAT read_format, DXGI_FORMAT write_format)
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

        TRY(device.GetDevice()->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
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
        command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            buffer_.Get(), current_state_, state));
        current_state_ = state;
    }
} // namespace feng

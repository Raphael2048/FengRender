#include "dx12/dx12_static_texture.hpp"
#include "dx12/dx12_device.hpp"
#include "DDSTextureLoader.h"
#include "ResourceUploadBatch.h"
#include "DirectXHelpers.h"

namespace feng
{
    StaticTexture::StaticTexture(const Device& device, DirectX::ResourceUploadBatch& uploader, const std::wstring path)
    {

        // DirectX::Descti
        DirectX::CreateDDSTextureFromFile(
            device.GetDevice(), uploader, path.data(), buffer_.GetAddressOf()
            );
        // DirectX::CreateShaderResourceView(device.GetDevice(), buffer_.Get(), )
    }
}
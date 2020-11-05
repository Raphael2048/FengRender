#include "dx12/dx12_buffer.hpp"
#include "dx12/dx12_device.hpp"
#include "d3dx12.h"
#include "ResourceUploadBatch.h"
#include "BufferHelpers.h"
namespace feng
{
	Buffer::Buffer(ID3D12Device* device, DirectX::ResourceUploadBatch& uploader, void* data, size_t count, size_t stride)
		: size_(count * stride)
	{
		DirectX::CreateStaticBuffer(
			device, uploader, data, count, stride, D3D12_RESOURCE_STATE_GENERIC_READ, buffer_.GetAddressOf()
		);
		NAME_D3D12RESOURCE(buffer_)

		gpu_address_ = buffer_->GetGPUVirtualAddress();

	}
} // namespace feng
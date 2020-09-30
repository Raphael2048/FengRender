

#include "dx12/dx12_buffer.hpp"

#include "dx12/dx12_device.hpp"
#include "d3dx12.h"

namespace feng
{
	Buffer::Buffer(const Device& device, void* data, UINT size, ID3D12GraphicsCommandList* command)
		: size_(size)
	{
		CD3DX12_RESOURCE_DESC buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(size);

		TRY(device.GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&buffer_desc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(buffer_.GetAddressOf())));

		NAME_D3D12RESOURCE(buffer_)


			//ComPtr<ID3D12Resource> staging_;
		TRY(device.GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&buffer_desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&staging_)));

		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = data;
		subResourceData.RowPitch = size;
		subResourceData.SlicePitch = subResourceData.RowPitch;

		command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer_.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

		UpdateSubresources<1>(command, buffer_.Get(), staging_.Get(), 0, 0, 1, &subResourceData);

		command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer_.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

		gpu_address_ = buffer_->GetGPUVirtualAddress();

	}
} // namespace feng
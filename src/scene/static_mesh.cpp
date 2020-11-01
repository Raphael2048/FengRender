#include "scene/static_mesh.hpp"
#include <d3dcompiler.h>
namespace feng {

    D3D12_INPUT_LAYOUT_DESC StaticMesh::InputLayout()
    {
        static std::vector<D3D12_INPUT_ELEMENT_DESC> layout = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };
        return {layout.data(), (UINT)layout.size()};
    }

    StaticMesh::StaticMesh(const Vector3& position, const Vector3& rotation, const Vector3 scale,
        // const Device& device, ID3D12GraphicsCommandList* command,
        void* vertex_data, UINT vertex_size, void* index_data, UINT index_size):
        Node(position, rotation, scale), vertex_count_(vertex_size), index_count_(index_size)
    {

        TRY(D3DCreateBlob(sizeof(Vertex) * vertex_count_, &vertex_buffer_cpu_));
        CopyMemory(vertex_buffer_cpu_->GetBufferPointer(), vertex_data, sizeof(Vertex) * vertex_count_);

        TRY(D3DCreateBlob(sizeof(uint32_t) * index_count_, &index_buffer_cpu_));
        CopyMemory(index_buffer_cpu_->GetBufferPointer(), index_data, sizeof(uint32_t) * index_count_);

    }

    void StaticMesh::Init(const Device& device, ID3D12GraphicsCommandList* command)
    {
        vertex_buffer_.reset(new Buffer(device, vertex_buffer_cpu_->GetBufferPointer(), sizeof(Vertex) * vertex_count_, command));

        index_buffer_.reset(new Buffer(device, index_buffer_cpu_->GetBufferPointer(), sizeof(uint32_t) * index_count_, command));
    }

    D3D12_VERTEX_BUFFER_VIEW StaticMesh::GetVertexBufferView()
    {
            D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = vertex_buffer_->GetGPUAddress();
		vbv.StrideInBytes = sizeof(Vertex);
		vbv.SizeInBytes = sizeof(Vertex) * vertex_count_;

		return vbv;
    }
    D3D12_INDEX_BUFFER_VIEW StaticMesh::GetIndexBufferView()
    {
        D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = index_buffer_->GetGPUAddress();
		ibv.Format = DXGI_FORMAT_R32_UINT;
		ibv.SizeInBytes = sizeof(uint32_t) * index_count_;
		return ibv;
    }
}
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

        TRY(D3DCreateBlob(sizeof(Vector3) * vertex_size, &vertex_buffer_cpu_));
        CopyMemory(vertex_buffer_cpu_->GetBufferPointer(), vertex_data, sizeof(Vector3) * vertex_size);

        TRY(D3DCreateBlob(sizeof(Vector3) * vertex_size, &index_buffer_cpu_));
        CopyMemory(vertex_buffer_cpu_->GetBufferPointer(), vertex_data, sizeof(Vector3) * vertex_size);
        // vertex_buffer_.reset(new Buffer(device, vertex_data, sizeof(Vector3) * vertex_count_, command));

        // if (index_data != nullptr)
        // {
        //     index_buffer_.reset(new Buffer(device, index_data, sizeof(Vector2) * index_count_, command));
        // }
    }

    void StaticMesh::Init(const Device& device, ID3D12GraphicsCommandList* commnad)
    {

    }
}
#include "scene/static_mesh.hpp"
#include "dx12/dx12_device.hpp"
#include <d3dcompiler.h>
namespace feng
{

    Mesh::Mesh(void *vertex_data, UINT vertex_count, void *index_data, UINT index_count):vertex_count_(vertex_count), index_count_(index_count)
    {
        TRY(D3DCreateBlob(sizeof(Vertex) * vertex_count_, &vertex_buffer_cpu_));
        CopyMemory(vertex_buffer_cpu_->GetBufferPointer(), vertex_data, sizeof(Vertex) * vertex_count_);

        TRY(D3DCreateBlob(sizeof(uint32_t) * index_count_, &index_buffer_cpu_));
        CopyMemory(index_buffer_cpu_->GetBufferPointer(), index_data, sizeof(uint32_t) * index_count_);
    }

    void Mesh::Init(const Device& device, DirectX::ResourceUploadBatch& uploader)
    {
        if (!inited_)
        {
            vertex_buffer_.reset(new Buffer(device.GetDevice(), uploader,  vertex_buffer_cpu_->GetBufferPointer(), vertex_count_, sizeof(Vertex)));
            index_buffer_.reset(new Buffer(device.GetDevice(),uploader,  index_buffer_cpu_->GetBufferPointer(), index_count_, sizeof(uint32_t)));
            inited_ = true;
        }
    }

    D3D12_INPUT_LAYOUT_DESC StaticMesh::InputLayout()
    {
        static std::vector<D3D12_INPUT_ELEMENT_DESC> layout = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };
        return {layout.data(), (UINT)layout.size()};
    }

    StaticMesh::StaticMesh(const Vector3 &position, const Vector3 &rotation, const Vector3 scale, std::shared_ptr<Mesh> mesh) : Node(position, rotation, scale), mesh_(mesh)
    {
    }

    void StaticMesh::Init(const Device& device, DirectX::ResourceUploadBatch& uploader)
    {
        mesh_->Init(device, uploader);
    }

    D3D12_VERTEX_BUFFER_VIEW StaticMesh::GetVertexBufferView()
    {
        D3D12_VERTEX_BUFFER_VIEW vbv;
        vbv.BufferLocation = mesh_->vertex_buffer_->GetGPUAddress();
        vbv.StrideInBytes = sizeof(Vertex);
        vbv.SizeInBytes = sizeof(Vertex) * mesh_->vertex_count_;

        return vbv;
    }
    D3D12_INDEX_BUFFER_VIEW StaticMesh::GetIndexBufferView()
    {
        D3D12_INDEX_BUFFER_VIEW ibv;
        ibv.BufferLocation = mesh_->index_buffer_->GetGPUAddress();
        ibv.Format = DXGI_FORMAT_R32_UINT;
        ibv.SizeInBytes = sizeof(uint32_t) * mesh_->index_count_;
        return ibv;
    }

    void StaticMesh::Update([[maybe_unused]] float deltatime)
    {
        if (dirty_)
        {
            dirty_ = false;
            MatrixWorld = Matrix::CreateScale(scale_);
            MatrixWorld *= Matrix::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(rotation_.y),
                                                          DirectX::XMConvertToRadians(rotation_.x), DirectX::XMConvertToRadians(rotation_.z));

            MatrixWorld *= Matrix::CreateTranslation(position_);

            MatrixInvWorld = MatrixWorld.Invert();

            cb_dirty_ = BACK_BUFFER_SIZE;
        };
    }
} // namespace feng
#pragma once

#include "scene/node.hpp"
#include "dx12/dx12_buffer.hpp"
namespace feng
{
    struct Vertex
    {
        Vector3 pos;
        Vector3 normal;
        Vector3 tangent;
        Vector2 uv;
    };
    class Mesh
    {
    public:
        Mesh(void* vertex_data, UINT vertex_count, void* index_data, UINT index_count);
        void Init(const Device& device, ID3D12GraphicsCommandList* command);
    public:
        ComPtr<ID3DBlob> vertex_buffer_cpu_;
        ComPtr<ID3DBlob> index_buffer_cpu_;
        std::unique_ptr<Buffer> vertex_buffer_;
        std::unique_ptr<Buffer> index_buffer_;
        UINT vertex_count_;
        UINT index_count_;
        bool inited_ = false;
    };
    class StaticMesh : public Node
    {
    public:

        StaticMesh(const Vector3& position, const Vector3& rotation, const Vector3 scale, std::shared_ptr<Mesh> mesh);

        void Init(const Device& device, ID3D12GraphicsCommandList* commnad);

        D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();
        D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();

        virtual void Update([[maybe_unused]]float deltatime) override;
        static D3D12_INPUT_LAYOUT_DESC InputLayout();

    public:
        std::shared_ptr<Mesh> mesh_;
        // ComPtr<ID3DBlob> vertex_buffer_cpu_;
        // ComPtr<ID3DBlob> index_buffer_cpu_;
        // std::unique_ptr<Buffer> vertex_buffer_;
        // std::unique_ptr<Buffer> index_buffer_;
        // UINT vertex_count_;
        // UINT index_count_;
    };
} // namespace feng
#pragma once

#include "util/math.hpp"
#include <stack>
#include <cmath>
namespace feng
{

    class OctreeChildNodeRef
    {
    public:
        union
        {
            struct
            {
                uint32_t x : 1;
                uint32_t y : 1;
                uint32_t z : 1;
                uint32_t null : 1;
            };
            uint32_t index : 3;
        };

        OctreeChildNodeRef(uint32_t InX, uint32_t InY, uint32_t InZ)
            : x(InX), y(InY), z(InZ), null(false) {}

        OctreeChildNodeRef(uint32_t index = 0) : index(index) { null = 0; }

        void Advance()
        {
            if (index < 7)
                ++index;
            else
                null = true;
        }
    };

    class OctreeChildNodeSubset
    {
    public:
        union
        {
            struct
            {
                uint32_t bPositiveX : 1;
                uint32_t bPositiveY : 1;
                uint32_t bPositiveZ : 1;
                uint32_t bNegativeX : 1;
                uint32_t bNegativeY : 1;
                uint32_t bNegativeZ : 1;
            };

            struct
            {
                /** Only the bits for the children on the positive side of the splits. */
                uint32_t PositiveChildBits : 3;

                /** Only the bits for the children on the negative side of the splits. */
                uint32_t NegativeChildBits : 3;
            };

            /** All the bits corresponding to the child bits. */
            uint32_t ChildBits : 6;

            /** All the bits used to store the subset. */
            uint32_t AllBits;
        };

        OctreeChildNodeSubset() : AllBits(0) {}

        OctreeChildNodeSubset(OctreeChildNodeRef ref) : AllBits(0)
        {
            PositiveChildBits = ref.index;
            NegativeChildBits = ~ref.index;
        }

        bool Contains(OctreeChildNodeRef ref) const
        {
            const OctreeChildNodeSubset subset(ref);
            return (ChildBits & subset.ChildBits) == subset.ChildBits;
        }
    };

    class OctreeNodeContext
    {
    public:
        // tight bounds, real loose bounds is twice size
        Box Bounds;

        float ChildExtent;

        uint32_t InCullBits;

        uint32_t OutCullBits;

        OctreeNodeContext() {}

        OctreeNodeContext(uint32_t InCull, uint32_t OutCull) : InCullBits(InCull), OutCullBits(OutCull) {}

        OctreeNodeContext(const Box &InBounds) : Bounds(InBounds)
        {
            ChildExtent = Bounds.Extents.x * 0.5f;
        }

        OctreeNodeContext(const Box &InBounds, uint32_t InCull, uint32_t OutCull) : Bounds(InBounds), InCullBits(InCull), OutCullBits(OutCull)
        {
            ChildExtent = Bounds.Extents.x * 0.5f;
        }

        OctreeNodeContext GetChildContext(OctreeChildNodeRef ChildRef) const
        {
            return OctreeNodeContext(
                Box(
                    Vector3(
                        Bounds.Center.x + ChildExtent * ChildRef.x == 0 ? -1 : 1,
                        Bounds.Center.y + ChildExtent * ChildRef.y == 0 ? -1 : 1,
                        Bounds.Center.z + ChildExtent * ChildRef.z == 0 ? -1 : 1),
                    Vector3(
                        ChildExtent,
                        ChildExtent,
                        ChildExtent)));
        }

        OctreeChildNodeRef GetContaingChild(const Box &box) const
        {
            OctreeChildNodeRef result;
            float max_border = (std::max)({box.Extents.x, box.Extents.y, box.Extents.z});

            // 其中一边很长, 无法放在当前某个子格子中
            if (max_border > Bounds.Extents.x)
            {
                result.null = true;
            }
            // 检测中心点位置, 即可确定应该放置子格子的位置
            else
            {
                result.x = box.Center.x > Bounds.Center.x;
                result.y = box.Center.y > Bounds.Center.y;
                result.z = box.Center.z > Bounds.Center.z;
            }
            return result;
        }

        OctreeChildNodeSubset GetIntersectingChildren(const Box& box) const
        {
            Vector3 center = box.Center;
            Vector3 extent = box.Extents;
            Vector3 MaxP = center + extent;
            Vector3 MinP = center - extent;
            OctreeChildNodeSubset result;
            result.bNegativeX = MinP.x <= Bounds.Center.x + ChildExtent;
            result.bNegativeY = MinP.y <= Bounds.Center.y + ChildExtent;
            result.bNegativeZ = MinP.z <= Bounds.Center.z + ChildExtent;

            result.bPositiveX = MaxP.x >= Bounds.Center.x - ChildExtent;
            result.bPositiveY = MaxP.y >= Bounds.Center.y - ChildExtent;
            result.bPositiveZ = MaxP.z >= Bounds.Center.z - ChildExtent;
            return result;
        }
    };

    template <typename T>
    class Octree
    {
    public:
        typedef std::vector<T> ElementArrayType;
        // typedef typename ElementArrayType::const_iterator ElementConstIt;

        class Node
        {
        private:
            ElementArrayType Elements;
            const Node *Parent;

            std::unique_ptr<Node> Children[8];
            // elements count by the node and it's child nodes
            uint32_t InclusiveNumElements : 31;

            uint32_t IsLeaf : 1;

        public:
            friend class Octree;
            explicit Node(const Node *parent) : Parent(parent), IsLeaf(true), InclusiveNumElements(0) {}

            const ElementArrayType &GetElements() const { return Elements; }

            int32_t GetInclusiveElementCount() const { return InclusiveNumElements; }

            Node *GetChild(OctreeChildNodeRef ref) const { return Children[ref.index].get(); }

            bool HasChild(OctreeChildNodeRef ref) const { return Children[ref.index] && Children[ref.index]->InclusiveNumElements > 0; }
        };

        class NodeReference
        {
        public:
            Node *TNode = nullptr;
            OctreeNodeContext Context;

            NodeReference() {}
            NodeReference(const NodeReference&) = default;
            NodeReference& operator=(const NodeReference&) = default;
            NodeReference(NodeReference&&) = default;
            NodeReference& operator=(NodeReference&&) = default;

            NodeReference(Node *node, const OctreeNodeContext &context) : TNode(node), Context(context) {}
        };

        class NodeIterator
        {
        private:
            NodeReference CurrentNode;
            std::stack<NodeReference> NodeStack;

        public:
            NodeIterator(Octree &tree)
                : CurrentNode(NodeReference(&tree.RootNode, tree.RootNodeContext)) {}

            NodeIterator(Node &Node, const OctreeNodeContext &Context)
                : CurrentNode(NodeReference(&Node, Context)) {}

            Node &GetCurrentNode() const { return *CurrentNode.TNode; }

            const OctreeNodeContext &GetCurrentContext() const { return CurrentNode.Context; }

            void Advance()
            {
                if (NodeStack.empty())
                    CurrentNode = NodeReference();
                else
                {
                    CurrentNode = NodeStack.top();
                    NodeStack.pop();
                }
            }

            bool HasPendingNodes() const { return CurrentNode.TNode != nullptr; }

            void PushChild(OctreeChildNodeRef ChildRef)
            {
                NodeStack.emplace(
                    CurrentNode.TNode->GetChild(ChildRef), CurrentNode.Context.GetChildContext(ChildRef)
                );
            }
        };

        Octree(const Vector3 &origin, float extent):RootNode(nullptr),
            RootNodeContext(Box(origin, Vector3(extent, extent, extent))),
            MinLeafExtent(extent * pow(0.5f, T::MaxNodeDepth)) 
        {
        }

        void AddElement(const T &element)
        {
            AddElementToNode(element, RootNode, RootNodeContext);
        }

        Box GetRootBounds() const
        {
            return RootNodeContext.Bounds;
        }

    private:
        Node RootNode;

        OctreeNodeContext RootNodeContext;

        float MinLeafExtent;

        void AddElementToNode(const T &element, Node &node, const OctreeNodeContext &context)
        {
            const Box ElementBox(element.GetBoundingBox());

            for (NodeIterator NodeIt(node, context); NodeIt.HasPendingNodes(); NodeIt.Advance())
            {
                Node &n = NodeIt.GetCurrentNode();
                const OctreeNodeContext &context = NodeIt.GetCurrentContext();
                const bool isLeaf = n.IsLeaf;

                bool addToThisNode = false;

                n.InclusiveNumElements++;

                if (isLeaf)
                {
                    if (n.Elements.size() + 1 > T::MaxElementsPerLeaf && context.Bounds.Extents.x > MinLeafExtent)
                    {
                        ElementArrayType Children = std::move(n.Elements);
                        n.InclusiveNumElements = 0;
                        n.IsLeaf = false;

                        for (const auto &eleIt : Children)
                        {
                            AddElementToNode(eleIt, n, context);
                        }

                        AddElementToNode(element, n, context);
                        return;
                    }
                    else
                    {
                        addToThisNode = true;
                    }
                }
                else
                {
                    const OctreeChildNodeRef childRef = context.GetContaingChild(ElementBox);
                    if (childRef.null)
                    {
                        addToThisNode = true;
                    }
                    else
                    {
                        if(!n.Children[childRef.index])
                        {
                            n.Children[childRef.index].reset( new Node(&n) );
                        }
                        NodeIt.PushChild(childRef);
                    }
                }

                if(addToThisNode)
                {
                    n.Elements.emplace_back(element);
                    return;
                }
            }

            FMSG("Can't inset!!!");
        }
    };
} // namespace feng
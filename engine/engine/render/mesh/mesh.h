#ifndef INC_MESH_H_
#define INC_MESH_H_

#include "mcv_platform.h"
#include "utils/itemsByName.h"
#include "components/AABB.h"

#include "../shader/vertex_declarations.h"

namespace render {

class CMesh;

class Mesh
{
    public:
        static const unsigned CURRENT_VERSION = 5;
        friend CMesh;

        typedef utils::ItemsByName< Mesh > Manager;
        
        static const unsigned TAG_begin     = 0x33440000;
        static const unsigned TAG_vertices  = 0x33441111;
        static const unsigned TAG_indices   = 0x33441122;
        static const unsigned TAG_groups    = 0x33442211;
        static const unsigned TAG_end       = 0x3344FFFF;
        
        typedef unsigned short index_t;
        typedef uint32_t groupId_t;
        union group_t {
            struct {
                int firstIndex;
                int nIndices;
            };
            struct {
                int firstVertex;
                int nVertices;
            };
            group_t()=default;
            group_t(int first, int n) :
                //which anonymous struct is irrelevant since they are both the same
                firstVertex(first), nVertices(n) {}
        };

        struct header_t {
            unsigned magic;
            unsigned version;
            unsigned nvertexs;
            unsigned nidxs;
            unsigned primitive_type;
            unsigned bytes_per_vertex;
            unsigned bytes_per_index;
            unsigned vertex_type;
            float bbmin[3];
            float bbmax[3];
            unsigned magic_tail;


            inline bool isValid() const {
              assert(version >= CURRENT_VERSION); // Fails with older versions (need to export again)
              assert(version <= CURRENT_VERSION); // Fails with newer versions (need to update code)
              return magic == TAG_begin
                && magic_tail == TAG_begin
                && version == CURRENT_VERSION
                && bytes_per_index == 2
                ;
            }
        };

        struct chunkHeader_t {
            unsigned tag;
            unsigned nbytes;
        };

    private:
        static Manager mesh_manager;
        struct fileData_t {
            header_t header;
            std::vector<uint8_t> vertices;
            std::vector<index_t> indices;
            std::vector<group_t> groups;
            const VertexDecl* vertexDecl;
            bool success;
        };

    public:
        static fileData_t Mesh::loadFileData(utils::DataProvider& dp);
        static Manager& getManager() {return mesh_manager;}
        static const Mesh*    current_active_mesh;

    private:
        ID3D11Buffer*          vb = nullptr;
        ID3D11Buffer*          ib = nullptr;

        unsigned               nvertexs = 0;
        unsigned               nindices = 0;
        D3D_PRIMITIVE_TOPOLOGY topology = D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED;
        const VertexDecl*      vtxs_decl = nullptr;
        unsigned               stream=-1; //inputSlot in vertex declaration
        unsigned               bytesPerVertex=0;
  
        XMVECTOR bbmin = utils::zero_v;
        XMVECTOR bbmax = utils::zero_v;

        std::vector<group_t> groups;

    public:

        enum ePrimitiveType {
            POINTS = 4000,
            LINE_LIST,
            TRIANGLE_LIST,
            TRIANGLE_STRIP,
        };

        bool create(
            unsigned anvertexs
            , const void* the_vertexs       // Can't be null
            , unsigned anindices
            , const index_t* the_indices     // Can be null
            , ePrimitiveType primitive_type
            , const VertexDecl* avtxs_decl
            , unsigned stream = 0
            , XMVECTOR bbmin = utils::zero_v
            , XMVECTOR bbmax = utils::zero_v
            , bool can_be_updated = false
            );

        // -------------------- Create using a vertex type
        template< class TVertex >
        bool create(
            unsigned anvertexs
            , const TVertex* the_vertexs       // Can't be null
            , unsigned anindices
            , const index_t* the_indices     // Can be null
            , ePrimitiveType primitive_type
            , XMVECTOR bbmin = utils::zero_v
            , XMVECTOR bbmax = utils::zero_v
            , bool can_be_updated = false
            ) {
            return create(
                anvertexs
                , the_vertexs       // Can't be null
                , anindices
                , the_indices     // Can be null
                , primitive_type
                , getVertexDecl<TVertex>()
                , 0
                , bbmin
                , bbmax
                , can_be_updated
                );
        }

        void destroy();

        void activate() const;
        void render() const;
        void renderGroups(unsigned groupId0, unsigned groupIdf) const;

        void activateAndRender() const {
            activate();
            render();
        }

        bool load(utils::DataProvider& dp);
        bool load(const char* name, bool defaults=true);

        component::AABB getAABB() const;
        inline void setAABB(const component::AABB& aabb) {
            bbmin=aabb.getMin(), bbmax=aabb.getMax();
        }

        bool isValid() const { return vb != nullptr;  }

        void updateFromCPU(const void *new_cpu_data, size_t num_bytes_to_update = 0);
        void renderInstanced(const Mesh& instancedData, size_t nInstances) const;
        void renderInstanced(unsigned groupId0, unsigned groupIdf, const Mesh& instanedData, size_t nInstances) const;

        inline unsigned getBytesPerVertex() const {return bytesPerVertex;}
};

}
#endif

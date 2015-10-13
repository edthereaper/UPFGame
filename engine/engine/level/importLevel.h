#ifndef LEVEL_IMPORT
#define LEVEL_IMPORT

#include "utils/XMLParser.h"
#include "handles/entity.h"
#include "render/mesh/CInstancedMesh.h"

#include "gameElements/module.h"
using namespace gameElements;

namespace level {

class LevelImport : private utils::XMLParser {
    public:
        static const uint32_t TAG_SCENE = 0x2CE7A210;
    private:
        static component::Handle previousLevel_h;
        static component::Handle currentLevel_h;
        static component::Handle currentEntity_h;
        static component::Handle playerEntity_h;
        static component::Handle bossEntity_h;
        static int spatialIndex;
        typedef std::map<std::string, component::Handle> instancedPieces_t;
        typedef std::map<std::string, utils::MKeyValue> specialCollisions_t;
        static instancedPieces_t instancedPieces;
        static specialCollisions_t specialCollisions;

        struct wildcard_t {
            component::Transform transform;
            std::string tag1;
            std::string tag2;
            std::string tag3;
            XMVECTOR size;
        
            wildcard_t() = default;
            wildcard_t(component::Transform transform, 
                std::string tag1, std::string tag2, std::string tag3,
                XMVECTOR size = utils::zero_v) :
                transform(transform), tag1(tag1), tag2(tag2), tag3(tag3), size(size){}
        };

        struct pieceData_t {
            bool valid = false;
            bool instanced;
            int spatialIndex;
            std::string meshName;
            std::string transformation;
            std::string collision;
            utils::MKeyValue special;
            int hits;
            XMVECTOR pos;
            XMVECTOR rot;

            pieceData_t()=default;
            pieceData_t(bool instanced, std::string meshName, int spatialIndex,
                std::string transformation, std::string collision, int hits,
                XMVECTOR pos, XMVECTOR rot, utils::MKeyValue&& special) :
                valid(true),
                instanced(instanced), meshName(meshName), spatialIndex(spatialIndex),
                transformation(transformation), collision(collision), hits(hits),
                pos(pos), rot(rot), special(std::move(special)) {}
        };
        static std::vector<pieceData_t> pieces;
        static std::vector<wildcard_t> wildcards;
        static std::map<std::string, unsigned> instancedPieceCount;

        static gameElements::CBoss* setupBoss(Entity*, const pieceData_t&);
        static void setupWeakSpot(Entity*, const pieceData_t&);
        static void setupHammer(Entity*, const pieceData_t&);

    private:
        static component::Entity* createPiece(const pieceData_t& p);

        static component::Entity* createNonInstancedPiece(const pieceData_t&);
        static component::Entity* createNonInstancedNonTransformablePiece(const pieceData_t&);
        static component::Entity* createNonInstancedTransformablePiece(const pieceData_t&);

        static component::Entity* getInstancedMesh(
            std::string instanceName, std::string meshName, bool transformable);
        static component::Entity* createInstancedNonTransformablePiece(const pieceData_t&);
        static component::Entity* createInstancedTransformablePiece(const pieceData_t&);

        static void generateLava(const MKeyValue& atts, const wildcard_t& wc);

    private:
	    void onStartElement (const std::string &elem, utils::MKeyValue &atts);
	    void onEndElement (const std::string &elem);
        LevelImport()=default;

    public:
        static component::Handle load(const char* newName, component::Handle playerEntity_h);
};


}

#endif
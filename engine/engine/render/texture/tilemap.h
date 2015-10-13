#ifndef RENDER_TILEMAP_H_
#define RENDER_TILEMAP_H_

#include "../mesh/mesh.h"

#include "utils/XMLParser.h"
#include "utils/itemsByName.h"

namespace render {


struct framedTileMap : private utils::XMLParser {
    public:
        static framedTileMap load(std::string name) {
            framedTileMap ret;
            ret.xmlParseFile(name);
            return ret;
        }

    public:
        unsigned short top=-1,left=-1,right=-1,bottom=-1;
        unsigned short textureSize=1024;
        float worldSize=8;

    private:
        void onStartElement(const std::string &elem, utils::MKeyValue &atts);
};

/*
xl := left
zf := up
xr := right
zb := bottom
*/
bool createFramedPlanePUNT(Mesh& mesh, const framedTileMap& frame,
    float xl, float zf, float xr, float zb, float y=0);

}

#endif
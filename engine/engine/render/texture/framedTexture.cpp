#include "mcv_platform.h"
#include "tilemap.h"

#include "../render_utils.h"

#define FILL_FIT        1
#define FILL_PARTIAL    2
#define FILL_RESIZE     3
#define FILL_SHRINK_W   4
#define FILL_SHRINK_H   5
#define FILL_METHOD  FILL_SHRINK_W

#define DOUBLE_FACED

using namespace DirectX;

namespace render {

void framedTileMap::onStartElement(const std::string &elem, utils::MKeyValue &atts)
{
    left   = atts.getInt("left", left);
    right  = atts.getInt("right", right);
    top    = atts.getInt("top", top);
    bottom = atts.getInt("bottom", bottom);
    textureSize = atts.getInt("textureSize", textureSize);
    worldSize = atts.getFloat("worldSize", worldSize);
}

void pushQuadIndices(Mesh::index_t& baseIndex, std::vector<Mesh::index_t>& indices)
{
    for (Mesh::index_t i : {0, 1, 2, 1, 3, 2}) {
        indices.push_back(baseIndex+i);
    }
#ifdef DOUBLE_FACED
    for (Mesh::index_t i : {2, 1, 0, 2, 3, 1}) {
        indices.push_back(baseIndex+i);
    }
#endif
    baseIndex+=4;
}

bool createFramedPlanePUNT(Mesh& mesh, const framedTileMap& frame, float xl, float zf, float xr, float zb, float y)
{
    float width     = xl+xr;
    float length    = zb+zf;

    if(frame.worldSize == 0 || frame.textureSize == 0) {return false;}

    //Factors
    float wf                = 1/float(frame.worldSize);
    float tf                = 1/float(frame.textureSize);
    float t2w               = tf/wf;
    float w2t               = 1/t2w;

    //Frame coordinates in texture
    float textureWidth      = frame.textureSize*t2w;
    float textureHeight     = textureWidth;
    float textureTop        = float(frame.top<0?0:frame.top);
    float textureLeft       = float(frame.left<0?0:frame.left);
    float textureRight      = float(frame.right<0?frame.textureSize:frame.right);
    float textureBottom     = float(frame.bottom<0?frame.textureSize:frame.bottom);
    float cTextureWidth     = float(textureRight - textureLeft);
    float cTextureLength    = float(textureBottom - textureTop);

    //Frame coordinates in [0,1]
    float topN              = textureTop * tf;
    float leftN             = textureLeft * tf;
    float rightN            = textureRight * tf;
    float bottomN           = textureBottom * tf;
    float tileWN            = rightN-leftN;
    float tileLN            = bottomN-topN;

    //World coordinate baselines
    float tBL               = -zf + textureHeight * topN;
    float lBL               = -xl + textureWidth  * leftN;
    float rBL               =  xr - textureWidth  * (1-rightN);
    float bBL               =  zb - textureHeight * (1-bottomN);

    //World coordinate sizes
    float left              = leftN/wf;
    float right             = (1-rightN)/wf;
    float top               = topN/wf;
    float bottom            = (1-bottomN)/wf;
    float cWidth            = width - left - right;
    float cLength           = length - top - bottom;
    float cTileWidth        = tileWN/wf;
    float cTileLength       = tileLN/wf;

    // Number of tiles in each coordinate
    float nWTilesF = cWidth/cTileWidth;
    float nLTilesF = cLength/cTileLength;
    nWTilesF = std::round(nWTilesF*10000)/10000; //round decimal places to avoid loops due to precission 
    nLTilesF = std::round(nLTilesF*10000)/10000; //round decimal places to avoid loops due to precission 
    int nWTiles = static_cast<int>(std::floor(nWTilesF));
    int nLTiles = static_cast<int>(std::floor(nLTilesF));

#if FILL_METHOD == FILL_SHRINK_W
    if (nWTilesF > nWTiles) {
        //We want the world size to fit in nWTiles+1
        //For that, we shrink the texture's world size to fit the current width
        //So we solve the following equation
        //  l*f + (floor(tiles)+1)*c*f = w  => f = w/(l+(floor(tiles)+1)*c+r)
        auto f = width / (left + (nWTiles+1)*cTileWidth + right);
        auto frameCopy = frame;
        frameCopy.worldSize *= f;
        return createFramedPlanePUNT(mesh, frameCopy, xl, zf, xr, zb, y);
    }
#elif FILL_METHOD == FILL_SHRINK_H
    if (nLTilesF > nLTiles) {
        //Same as above, but in length
        auto f = length / (top + (nLTiles+1)*cTileLength + bottom);
        auto frameCopy = frame;
        frameCopy.worldSize *= f;
        return createFramedPlanePUNT(mesh, frameCopy, xl, zf, xr, zb, y);
    }
#endif

#if FILL_METHOD == FILL_RESIZE || FILL_METHOD == FILL_SHRINK_W || FILL_METHOD == FILL_SHRINK_H
    if (nLTilesF > nLTiles || nWTilesF > nWTiles) {
        float dW = (nWTilesF>nWTiles?1:0) * (1-(nWTilesF-nWTiles))*cTileWidth;
        float dL = (nLTilesF>nLTiles?1:0) * (1-(nLTilesF-nLTiles))*cTileLength;
        return createFramedPlanePUNT(mesh, frame, xl+dW/2, zf+dL/2, xr+dW/2, zb+dL/2, y);
    }
#endif

    //Invert for some reason. Don't question it. I have the feeling everything else is backwards.
    tBL = -tBL;
    lBL = -lBL;
    rBL = -rBL;
    bBL = -bBL;

    //For readability
    typedef VertexPosUVNormalTangent vertex;
    typedef XMFLOAT3 Pos;
    typedef XMFLOAT2 UV;
    
    Mesh::index_t baseIndex = 0;
    
    std::vector< VertexPosUVNormalTangent > vertices;
    std::vector< Mesh::index_t > indices;
    vertices.reserve(16 + 8*nWTiles + 8*nLTiles + 4*nWTiles*nLTiles); //An approximation
    indices.reserve(24 + 12*nWTiles + 12*nLTiles + 6*nWTiles*nLTiles); //An approximation

    // UL corner
	vertices.push_back(vertex(Pos(+xl, y, +zf), UV(0,     0)));
	vertices.push_back(vertex(Pos(lBL, y, +zf), UV(leftN, 0)));
	vertices.push_back(vertex(Pos(+xl, y, tBL), UV(0,     topN)));
	vertices.push_back(vertex(Pos(lBL, y, tBL), UV(leftN, topN)));
    pushQuadIndices(baseIndex, indices);
    
    // UR corner
	vertices.push_back(vertex(Pos(rBL, y, +zf), UV(rightN, 0)));
	vertices.push_back(vertex(Pos(-xr, y, +zf), UV(1,      0)));
	vertices.push_back(vertex(Pos(rBL, y, tBL), UV(rightN, topN)));
	vertices.push_back(vertex(Pos(-xr, y, tBL), UV(1,      topN)));
    pushQuadIndices(baseIndex, indices);

    // DL corner
	vertices.push_back(vertex(Pos(+xl, y, bBL), UV(0,     bottomN)));
	vertices.push_back(vertex(Pos(lBL, y, bBL), UV(leftN, bottomN)));
	vertices.push_back(vertex(Pos(+xl, y, -zb), UV(0,     1)));
	vertices.push_back(vertex(Pos(lBL, y, -zb), UV(leftN, 1)));
    pushQuadIndices(baseIndex, indices);
    
    // DR corner
	vertices.push_back(vertex(Pos(rBL, y, bBL), UV(rightN, bottomN)));
	vertices.push_back(vertex(Pos(-xr, y, bBL), UV(1,      bottomN)));
	vertices.push_back(vertex(Pos(rBL, y, -zb), UV(rightN, 1)));
	vertices.push_back(vertex(Pos(-xr, y, -zb), UV(1,      1)));
    pushQuadIndices(baseIndex, indices);

    // TOP
    {
        float wl = lBL;
        for (int t = 0; t<nWTiles; t++) {
	        vertices.push_back(vertex(Pos(wl,            y, +zf), UV(leftN,  0)));
	        vertices.push_back(vertex(Pos(wl-cTileWidth, y, +zf), UV(rightN, 0)));
	        vertices.push_back(vertex(Pos(wl,            y, tBL), UV(leftN,  topN)));
	        vertices.push_back(vertex(Pos(wl-cTileWidth, y, tBL), UV(rightN, topN)));
            wl -= cTileWidth;
            pushQuadIndices(baseIndex, indices);
        }
        if (wl > rBL) {
            float wr = rBL;
            #if FILL_METHOD == FILL_PARTIAL
                auto right = leftN+tileWN*(wl-rBL)/cTileWidth;
            #else
                auto right = rightN;
            #endif
	        vertices.push_back(vertex(Pos(wl, y, +zf), UV(leftN, 0)));
	        vertices.push_back(vertex(Pos(wr, y, +zf), UV(right, 0)));
	        vertices.push_back(vertex(Pos(wl, y, tBL), UV(leftN, topN)));
	        vertices.push_back(vertex(Pos(wr, y, tBL), UV(right, topN)));
            pushQuadIndices(baseIndex, indices);
        }
    }

    // BOTTOM
    {
        float wl = lBL;
        for (int t = 0; t<nWTiles; t++) {
	        vertices.push_back(vertex(Pos(wl,            y, bBL), UV(leftN,  bottomN)));
	        vertices.push_back(vertex(Pos(wl-cTileWidth, y, bBL), UV(rightN, bottomN)));
	        vertices.push_back(vertex(Pos(wl,            y, -zb), UV(leftN,  1)));
	        vertices.push_back(vertex(Pos(wl-cTileWidth, y, -zb), UV(rightN, 1)));
            wl -= cTileWidth;
            pushQuadIndices(baseIndex, indices);
        }
        if (wl > rBL) {
            float wr = rBL;
            #if FILL_METHOD == FILL_PARTIAL
                auto right = leftN+tileWN*(wl-rBL)/cTileWidth;
            #else
                auto right = rightN;
            #endif
	        vertices.push_back(vertex(Pos(wl, y, bBL), UV(leftN, bottomN)));
	        vertices.push_back(vertex(Pos(wr, y, bBL), UV(right, bottomN)));
	        vertices.push_back(vertex(Pos(wl, y, -zb), UV(leftN, 1)));
	        vertices.push_back(vertex(Pos(wr, y, -zb), UV(right, 1)));
            pushQuadIndices(baseIndex, indices);
        }
    }

    // LEFT
    {
        float wt = tBL;
        for (int t = 0; t<nLTiles; t++) {
	        vertices.push_back(vertex(Pos( xl, y, wt),             UV(0,     topN)));
	        vertices.push_back(vertex(Pos(lBL, y, wt),             UV(leftN, topN)));
	        vertices.push_back(vertex(Pos( xl, y, wt-cTileLength), UV(0,     bottomN)));
	        vertices.push_back(vertex(Pos(lBL, y, wt-cTileLength), UV(leftN, bottomN)));
            wt -= cTileLength;
            pushQuadIndices(baseIndex, indices);
        }
        if (wt > bBL) {
            #if FILL_METHOD == FILL_PARTIAL
                auto bottom = topN+tileLN*(wt-bBL)/cTileLength;
            #else
                auto bottom = bottomN;
            #endif
            float wb = bBL;
	        vertices.push_back(vertex(Pos( xl, y, wt),  UV(0,     topN)));
	        vertices.push_back(vertex(Pos(lBL, y, wt),  UV(leftN, topN)));
	        vertices.push_back(vertex(Pos( xl, y, wb),  UV(0,     bottom)));
	        vertices.push_back(vertex(Pos(lBL, y, wb),  UV(leftN, bottom)));
            pushQuadIndices(baseIndex, indices);
        }
    }
    
    // RIGHT
    {
        float wt = tBL;
        for (int t = 0; t<nLTiles; t++) {
	        vertices.push_back(vertex(Pos(rBL, y, wt),             UV(rightN, topN)));
	        vertices.push_back(vertex(Pos(-xr, y, wt),             UV(1,      topN)));
	        vertices.push_back(vertex(Pos(rBL, y, wt-cTileLength), UV(rightN, bottomN)));
	        vertices.push_back(vertex(Pos(-xr, y, wt-cTileLength), UV(1,      bottomN)));
            wt -= cTileLength;
            pushQuadIndices(baseIndex, indices);
        }
        if (wt > bBL) {
            #if FILL_METHOD == FILL_PARTIAL
                auto bottom = topN+tileLN*(wt-bBL)/cTileLength;
            #else
                auto bottom = bottomN;
            #endif
            float wb = bBL;
	        vertices.push_back(vertex(Pos(rBL, y, wt),  UV(rightN, topN)));
	        vertices.push_back(vertex(Pos(-xr, y, wt),  UV(1,      topN)));
	        vertices.push_back(vertex(Pos(rBL, y, wb),  UV(rightN, bottom)));
	        vertices.push_back(vertex(Pos(-xr, y, wb),  UV(1,      bottom)));
            pushQuadIndices(baseIndex, indices);
        }
    }

    // CENTER
    float wt = tBL;
    for (int tj = 0; tj<nLTiles; tj++) {
        float wl = lBL;
        for (int ti = 0; ti<nWTiles; ti++) {
	        vertices.push_back(vertex(Pos(wl,            y, wt),             UV(leftN,  topN)));
	        vertices.push_back(vertex(Pos(wl-cTileWidth, y, wt),             UV(rightN, topN)));
	        vertices.push_back(vertex(Pos(wl,            y, wt-cTileLength), UV(leftN,  bottomN)));
	        vertices.push_back(vertex(Pos(wl-cTileWidth, y, wt-cTileLength), UV(rightN, bottomN)));
            wl -= cTileWidth;
            pushQuadIndices(baseIndex, indices);
        }
        if (wl > rBL) {
            float wr = rBL;
            #if FILL_METHOD == FILL_PARTIAL
                auto right = leftN+tileWN*(wl-rBL)/cTileWidth;
            #else
                auto right = rightN;
            #endif
	        vertices.push_back(vertex(Pos(wl, y, wt),             UV(leftN, topN)));
	        vertices.push_back(vertex(Pos(wr, y, wt),             UV(right, topN)));
	        vertices.push_back(vertex(Pos(wl, y, wt-cTileLength), UV(leftN, bottomN)));
	        vertices.push_back(vertex(Pos(wr, y, wt-cTileLength), UV(right, bottomN)));
            pushQuadIndices(baseIndex, indices);
        }
        wt -= cTileLength;
    }
    if (wt > bBL) {
        #if FILL_METHOD == FILL_PARTIAL
            auto bottom = topN+tileLN*(wt-bBL)/cTileLength;
        #else
            auto bottom = bottomN;
        #endif
        float wb = bBL;
        float wl = lBL;
        for (int ti = 0; ti<nWTiles; ti++) {
	        vertices.push_back(vertex(Pos(wl,            y, wt), UV(leftN,  topN)));
	        vertices.push_back(vertex(Pos(wl-cTileWidth, y, wt), UV(rightN, topN)));
	        vertices.push_back(vertex(Pos(wl,            y, wb), UV(leftN,  bottom)));
	        vertices.push_back(vertex(Pos(wl-cTileWidth, y, wb), UV(rightN, bottom)));
            wl -= cTileWidth;
            pushQuadIndices(baseIndex, indices);
        }
        if (wl > rBL) {
            float wr = rBL;
            #if FILL_METHOD == FILL_PARTIAL
                auto right = leftN+tileWN*(wl-rBL)/cTileWidth;
            #else
                auto right = rightN;
            #endif
	        vertices.push_back(vertex(Pos(wl, y, wt), UV(leftN, topN)));
	        vertices.push_back(vertex(Pos(wr, y, wt), UV(right, topN)));
	        vertices.push_back(vertex(Pos(wl, y, wb), UV(leftN, bottom)));
	        vertices.push_back(vertex(Pos(wr, y, wb), UV(right, bottom)));
            pushQuadIndices(baseIndex, indices);
        }
    }

    computeTangentSpace(vertices, indices);
    
	vertices.push_back(vertex(Pos(+xl, y, +zf), UV(0, 0)));
	vertices.push_back(vertex(Pos(-xr, y, -zb), UV(1, 1)));

    auto ul = XMVectorSet(+xl, y, +zf, 1);
    auto dr = XMVectorSet(-xr, y, -zb, 1);

    return mesh.create(
        (unsigned)vertices.size(), vertices.data(),
        (unsigned)indices.size(), indices.data(), Mesh::TRIANGLE_LIST,
        XMVectorMin(ul,dr), XMVectorMax(ul,dr));
}

}
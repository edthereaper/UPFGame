#ifndef _RENDER_UTILS_H_
#define _RENDER_UTILS_H_

#include "mcv_platform.h"
#include "camera/camera.h"
#include "mesh/mesh.h"
#include "texture/material.h"
#include "texture/texture.h"
#include "texture/textureCube.h"

#include "shader/vertex_declarations.h"
#include "shader/shaders.h"
#include "shader/buffer.h"

#include "components/color.h"

namespace gameElements {
    class CPaintGroup;
}

namespace render {

/* Init submodule */
bool renderUtilsCreate();
/* Deinit submodule */
void renderUtilsDestroy();

// Procedural meshes
bool createGrid(Mesh& mesh, int nsamples);
bool createAxis(Mesh& mesh, float length = 1.0);
bool createStar(Mesh& mesh, float length = 1.0);
bool createWiredUnitCube(Mesh& mesh);
bool createWiredCube(Mesh& mesh);
bool createWiredCubeSplit(Mesh& mesh);
bool createPlanePUNT(Mesh& mesh, float xl, float zf, float xr, float zb, float y=0, float texSize=1); //VertexPosUVNormalTangent
bool createPlanePUNTWithUV(Mesh& mesh,
    float xl, float zf, float xr, float zb, float y,
    float ul=0, float vf=0, float ur=1, float vb=1);
bool createPlane(Mesh& mesh);
bool createTexturedQuadXZCentered(Mesh& mesh);
bool createTexturedQuadXYCentered(Mesh& mesh);
bool createStackedQuadXZCentered(Mesh& mesh);
bool createTexturedQuadXYCenteredX(Mesh& mesh);
bool createVolumeCube(Mesh &mesh);
bool createDownPointer(Mesh &mesh, float w, float h);
bool createRay(Mesh& meshRay, float longitude);
bool createPUNTBox(Mesh& mesh, XMVECTOR max, XMVECTOR min= utils::zero_v);
void createSphere(Mesh& mesh, float radus, unsigned latitudes, unsigned longitudes);
bool createViewVolumeWire(Mesh& mesh);
bool createViewVolume(Mesh& mesh);
bool createIcosahedron(Mesh& mesh, float size=1.f);
bool createIcosahedronWireFrame(Mesh& mesh, float size=1.f);
bool createCyllinder(Mesh& mesh, float r, unsigned divisions, float h);

void drawViewVolume(const render::Camera& camera, const XMVECTOR& color = utils::zero_v);
void drawLine(XMVECTOR src, XMVECTOR target);

void updateGlobalConstants(float elapsed);

void setObjectConstants(XMMATRIX world, component::Color tint = 0,
    component::Color selfIllumination = 0, const CMesh* mesh = nullptr,
    unsigned bone0 = 0, const Material* material = nullptr);
void activateObjectConstants();
void activateCamera(const Camera& camera);
void activateLight(const Camera& light);
            
class Camera;
class CMesh;
class CPtLight; 
class CDirLight;
class CVolPtLight;
class CMist;
class CCubeShadow;
class CShadow;

void activatePtLightCB();
void activatePtLight(const CPtLight* light, XMVECTOR pos, CCubeShadow* shadow);
void activateDirLightCB();
void activateDirLight(const CDirLight* light, const Camera* camera, const CShadow* shadow);
void activateVolPtLightCB();
void activateVolPtLight(const CVolPtLight* light, const XMVECTOR& pos);
void activateMistCB();
void activateMist(const CMist* mist, unsigned level);
void activatePaintCB();
void activatePaintGroup(const gameElements::CPaintGroup*);

struct RenderConstsMirror {
    private:
        RenderConstsMirror()=delete;
        RenderConstsMirror(const RenderConstsMirror&)=delete;
    public:
        static bool dirty;
        //Ambient
        static XMVECTOR GlobalLightDirection;
        static float GlobalLightStrength;
	    //Skybox
        static float  SkyBoxBlend;
        static float  SkyBoxBright;
	    //Resolve
        static float  AmbientMax;
        static float  AmbientMin;
        static float  ResolveAlbedo;
        static float  ResolveAlbedoLight;
        static float  ResolveLight;
        static float  ResolveSelfIll;
        //Specular
        static float  PowerSpecular;
        static float  ResolveSpecular;

        static float ResolveSaturation;
        static float ResolveBrightness;
        static float ResolveContrast;

        static void update();
};

void setSkyBoxConstants(float blend, float bright);

void updateSeldomConstants();

void activateTextureSamplers();

struct pixelRect {
    int x=0, y=0, w=256, h=256;

    pixelRect()=default;
    pixelRect(int w, int h) : w(w), h(h) {}
    pixelRect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
};

void drawTexture2DAnim(pixelRect src, pixelRect dst, const Texture* texture, 
	bool alpha, float delta_secs);
void drawTexture2D(pixelRect src, pixelRect dst, const Texture* texture,
    const Technique* tech = nullptr, bool alpha = false,
    component::Color tint=component::Color::WHITE);
void drawText(pixelRect src, pixelRect dst, std::string score);
void drawEnergyBar(pixelRect src, pixelRect dst, float energy);
void drawLeafHUD(pixelRect src, pixelRect dst, float rot, const Texture* texture);
void drawTextureFadeOut(pixelRect src, pixelRect dst, const Texture* texture, float value);
void setTextureData(uint32_t nFrames, uint32_t framesPerRow, float elapsed = 0.0f);
void updateTextureData();

size_t getCurrentBoneIndex();
void addBoneToBuffer(XMMATRIX);
void uploadBonesToGPU();
void activateBoneBuffer();
void rewindBoneBuffer();

enum zConfig_e {
    ZCFG_DEFAULT,
    ZCFG_ALL,
    ZCFG_LT,
    ZCFG_LE,
    ZCFG_EQ,
    ZCFG_GT,
    ZCFG_GE,
    ZCFG_NE,
    ZCFG_TEST_LT,
    ZCFG_TEST_LE,
    ZCFG_TEST_EQ,
    ZCFG_TEST_GT,
    ZCFG_TEST_GE,
    ZCFG_TEST_NE,
    ZCFG_TEST_ALL,
    
    ZCFG_COUNT
};
void activateZConfig(enum zConfig_e cfg);

enum RSConfig {
    RSCFG_DEFAULT,
    RSCFG_REVERSE_CULLING,
    RSCFG_DISABLE_CULLING,
    RSCFG_SHADOWS,

    RSCFG_COUNT
};
void activateRSConfig(enum RSConfig cfg);

enum BlendConfig {
    BLEND_CFG_DEFAULT,
    BLEND_CFG_COMBINATIVE,
    BLEND_CFG_ADDITIVE,
    BLEND_CFG_ADDITIVE_BY_SRC_ALPHA,
    BLEND_GLASS,
    BLEND_PARTICLES,
    BLEND_MIST,
    BLEND_CFG_PAINT,

    BLEND_CFG_COUNT,
};
void activateBlendConfig(enum BlendConfig cfg);

// Shared meshes
extern Mesh mesh_grid;
extern Mesh mesh_axis;
extern Mesh mesh_star;
extern Mesh mesh_icosahedron;
extern Mesh mesh_icosahedron_wire;
extern Mesh mesh_line;
extern Mesh mesh_cube_wire;
extern Mesh mesh_cube_wire_split;
extern Mesh mesh_view_volume;
extern Mesh mesh_view_volume_wire;
extern Mesh mesh_textured_quad_xy;
extern Mesh mesh_textured_quad_xy_centered;
extern Mesh mesh_textured_quad_xz_centered;
extern Mesh mesh_stacked_quad_xz_centered;
extern Mesh mesh_cube_wire_unit;
extern Mesh mesh_cyllinder;
extern Mesh mesh_textured_quad_xy_bottomcentered;

extern Texture*     whiteTexture;
extern TextureCube* whiteTextureCube;

XMVECTOR calculateTangent(XMVECTOR tan1, XMVECTOR tan2, XMFLOAT3 nf3);
template <class Vertex_T>
void computeTangentSpace(std::vector<Vertex_T>& vertices, const std::vector<Mesh::index_t>& indices)
{
    std::vector<XMVECTOR> tan1(vertices.size(), utils::zero_v);
    std::vector<XMVECTOR> tan2(vertices.size(), utils::zero_v);
    for (unsigned a=0; a<indices.size(); a+=3) {
		Mesh::index_t i1 = indices[a+0];
		Mesh::index_t i2 = indices[a+1];
		Mesh::index_t i3 = indices[a+2];
		Vertex_T v1 = vertices[i1];
		Vertex_T v2 = vertices[i2];
		Vertex_T v3 = vertices[i3];
		
		// Positions in the previous array
		XMFLOAT3 vPos1 = v1.Pos;
		XMFLOAT3 vPos2 = v2.Pos;
		XMFLOAT3 vPos3 = v3.Pos;
		
		// texture coords
		XMFLOAT2 w1 = v1.UV;
		XMFLOAT2 w2 = v2.UV;
		XMFLOAT2 w3 = v3.UV;
		
		float x1 = vPos2.x - vPos1.x;
		float x2 = vPos3.x - vPos1.x;
		float y1 = vPos2.y - vPos1.y;
		float y2 = vPos3.y - vPos1.y;
		float z1 = vPos2.z - vPos1.z;
		float z2 = vPos3.z - vPos1.z;
		
		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;
		
		float r = 1.0f/(s1*t2 - s2*t1);
		
		XMVECTOR sdir = DirectX::XMVectorSet(
            (t2*x1 - t1*x2)*r, (t2*y1 - t1*y2)*r, (t2*z1 - t1*z2)*r, 0);
		XMVECTOR tdir = DirectX::XMVectorSet(
            (s1*x2 - s2*x1)*r, (s1*y2 - s2*y1)*r, (s1*z2 - s2*z1)*r, 0);
		
		DirectX::operator+=(tan1[i1], sdir);
		DirectX::operator+=(tan1[i2], sdir);
		DirectX::operator+=(tan1[i3], sdir);
		DirectX::operator+=(tan2[i1], tdir);
		DirectX::operator+=(tan2[i2], tdir);
		DirectX::operator+=(tan2[i3], tdir);
    }

    for (unsigned a=0; a<vertices.size(); a++) {
        XMVECTOR tangent = calculateTangent(
            tan1[a], tan2[a], vertices[a].Normal);
        vertices[a].Tangent.x = DirectX::XMVectorGetX(tangent);
        vertices[a].Tangent.y = DirectX::XMVectorGetY(tangent);
        vertices[a].Tangent.z = DirectX::XMVectorGetZ(tangent);
        vertices[a].Tangent.w = DirectX::XMVectorGetW(tangent);
    }
}

}
#endif

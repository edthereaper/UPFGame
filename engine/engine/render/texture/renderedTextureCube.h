#ifndef RENDER_RENDERED_TEXTURECUBE_H_
#define RENDER_RENDERED_TEXTURECUBE_H_

#include "../texture/texture.h"
#include "../texture/textureCube.h"
#include "renderedTexture.h"

namespace render {

class RenderedTextureCube : private Texture {
    public:
        enum zBufferType_e {
            USE_BACK_ZBUFFER,
            USE_SIX_ZBUFFER,
            USE_ONE_ZBUFFER /* This one is currently a lie and actually creates six zbuffers */,
            NO_ZBUFFER
        };

    private:
        DXGI_FORMAT colorFormat = DXGI_FORMAT_UNKNOWN;
        DXGI_FORMAT depthFormat = DXGI_FORMAT_UNKNOWN;
        unsigned xRes = 0;
        unsigned yRes = 0;
        std::string name;
        zBufferType_e zBufferType = NO_ZBUFFER;

        ID3D11RenderTargetView* renderTargetView[6];
        ID3D11DepthStencilView* depthStencilView[6];
        TextureCube*            zTexture =nullptr;

        bool createColorBuffer(bool owned);
        bool createDepthBuffer(bool owned);

    public:
        RenderedTextureCube() {
            for (auto& i:renderTargetView) {i=nullptr;}
            for (auto& i:depthStencilView) {i=nullptr;}
        }
        bool create(const char* name, int newXRes, int newYRes,
            DXGI_FORMAT newColorFormat, DXGI_FORMAT newDepthFormat,
            zBufferType_e zbuffer_type = zBufferType_e::USE_SIX_ZBUFFER, bool owned = false);
        void activateFace(size_t i);
        void activateViewport();
        void clearRenderTargetViews(const float color[4]);
        void clearDepthBuffers();
        void clearRenderTargetView(size_t i, const float color[4]);
        void clearDepthBuffer(size_t i);
        
        inline ID3D11DepthStencilView* getDepthStencilView(size_t i) const {
            assert(i<6);
            return depthStencilView[i];
        }
        inline ID3D11RenderTargetView* getRenderTargetView(size_t i) const {
            assert(i<6);
            return renderTargetView[i];
        }

        inline unsigned getXRes() const {return xRes;}
        inline unsigned getYRes() const {return yRes;}
        inline TextureCube* getZTextureCube() const { return zTexture; }

        using Texture::activate;

        inline void destroy() {
            Texture::getManager().destroy(name.c_str());
            Texture::getManager().destroy((name+"_Z").c_str());
            for(auto& v : renderTargetView) {SAFE_RELEASE(v);}
            if (zBufferType == USE_SIX_ZBUFFER) {
                for(auto& v : depthStencilView) {SAFE_RELEASE(v);}
            } else if (zBufferType == USE_ONE_ZBUFFER) {
                for(auto& v : depthStencilView) {SAFE_RELEASE(v);}
                //SAFE_RELEASE(depthStencilView[0]);
            }
        }
};

}

#endif

#ifndef RENDER_RENDERED_TEXTURE_H_
#define RENDER_RENDERED_TEXTURE_H_

#include "../texture/texture.h"

namespace render {

class RenderedTexture : public Texture {
    public:
        enum zBufferType_e { USE_BACK_ZBUFFER, USE_OWN_ZBUFFER, NO_ZBUFFER };

    private:
        DXGI_FORMAT colorFormat = DXGI_FORMAT_UNKNOWN;
        DXGI_FORMAT depthFormat = DXGI_FORMAT_UNKNOWN;
        unsigned xRes = 0;
        unsigned yRes = 0;
        zBufferType_e zBufferType = NO_ZBUFFER;
        std::string name;

        ID3D11RenderTargetView* renderTargetView = nullptr;
        ID3D11DepthStencilView* depthStencilView = nullptr;
        Texture*                zTexture;

    private:
        bool createColorBuffer(bool owned);
        bool createDepthBuffer(bool owned);

    public:
        bool create(const char* name, int newXRes, int newYRes,
            DXGI_FORMAT newColorFormat, DXGI_FORMAT newDepthFormat,
            zBufferType_e zbuffer_type = USE_OWN_ZBUFFER, bool owned = false);
        void activate() const;
        void activateViewport() const;
        void clearRenderTargetView(const float color[4]);
        void clearDepthBuffer();

        inline ID3D11RenderTargetView* getRenderTargetView() const {return renderTargetView;}

        inline unsigned getXRes() const {return xRes;}
        inline unsigned getYRes() const {return yRes;}
        inline Texture* getZTexture() const { return zTexture; }
        
        inline void destroy() {
            Texture::getManager().destroy(name.c_str());
            Texture::getManager().destroy((name+"_Z").c_str());
            SAFE_RELEASE(renderTargetView);
            if (zBufferType == USE_OWN_ZBUFFER) {SAFE_RELEASE(depthStencilView);}
        }
};

}

#endif

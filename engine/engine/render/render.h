#ifndef INC_RENDER_H_
#define INC_RENDER_H_

#include <d3d11.h>
#include <d3dcompiler.h>

#ifdef _DEBUG
    #define _DEBUG_RENDER
#endif

#ifdef _DEBUG_RENDER
    #include <d3d9.h> // D3DPERF_*
#endif

namespace render {

class Render {
    private:
        static unsigned width;
        static unsigned height;
        static ID3D11Device*           device;
        static ID3D11DeviceContext*    ctx;
        static IDXGISwapChain*         swap_chain;
        static ID3D11RenderTargetView* render_target_view;
        static ID3D11Texture2D*        depth_stencil;
        static ID3D11DepthStencilView* depth_stencil_view;

        static bool createRenderTargetView();
        static bool createDepthStencils();

        Render() = delete;
    public:
        static inline ID3D11DeviceContext*      getContext() {return ctx;}
        static inline ID3D11Device*             getDevice() {return device;}
        static inline IDXGISwapChain*           getSwapChain() {return swap_chain;}
        static inline ID3D11RenderTargetView*   getRenderTargetView() {return render_target_view;}
        static inline ID3D11DepthStencilView*   getDepthStencilView() {return depth_stencil_view;}

        static inline unsigned getHeight() {return height;}
        static inline unsigned getWidth()  {return width;}

        static bool createDevice();
        static void destroyDevice();

        static bool activateBackBuffer();

        static bool compileShaderFromFile(const char* szFileName, const char* szEntryPoint,
            const char* szShaderModel, ID3DBlob** ppBlobOut);
};

#define SAFE_RELEASE(x) {if((x) != nullptr) {(x)->Release(); (x) = nullptr;}}

#if defined(_DEBUG_RENDER) || defined(PROFILE)
#define setDbgName(obj,name) \
    (obj)->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT) strlen( name ), name );

struct DbgTrace {
    public:
        wchar_t wname[64];
        DbgTrace(const char *name) {
            setName(name);
        }
        inline void setName(const char *name) {
            mbstowcs(wname, name, 64);
        }
        inline void begin() {
            D3DPERF_BeginEvent(D3DCOLOR_XRGB(255, 0, 255), wname);
        }
        inline void end() {
            D3DPERF_EndEvent();
        }
};

struct TraceScoped : public DbgTrace {
    public:
        TraceScoped(const char *name) : DbgTrace(name) {begin();}
        ~TraceScoped() {end();}
};

#else
#define setDbgName(obj,name)
struct TraceScoped {TraceScoped(const char *name) {}};

#endif
}

#endif

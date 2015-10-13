#ifndef RENDER_BUFFER_H_
#define RENDER_BUFFER_H_

#include "../render.h"

namespace render {

class ShaderCteBase {
    protected:
        ID3D11Buffer*  buffer;
        bool           dirty;

    public:
        ShaderCteBase() : buffer(nullptr), dirty(false) {}
        inline void destroy() {
            SAFE_RELEASE(buffer);
        }

        inline void activateInVS(unsigned index_slot) const {
            assert(buffer);
            assert(!dirty);
            Render::getContext()->VSSetConstantBuffers(index_slot, 1, &buffer);
        }
        inline void activateInPS(unsigned index_slot) const {
            assert(buffer);
            assert(!dirty);
            Render::getContext()->PSSetConstantBuffers(index_slot, 1, &buffer);
        }
};

template< class TParams >
class ShaderCte : public ShaderCteBase {
    private:
        TParams params;
    public:
        inline bool create( const char* name ) {
            assert(buffer == nullptr);

            // Create the constant buffer
            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(bd));
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(TParams);
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0;
            HRESULT hr = Render::getDevice()->CreateBuffer(&bd, NULL, &buffer);
            setDbgName( buffer, name );
            return !FAILED(hr);

        }

        //Get in rw mode
        inline TParams& get() {
            dirty = true;
            return params;
        }

        //Get in read only mode - doesn't make it dirty
        inline const TParams& get() const {
            return params;
        }

        void uploadToGPU() {
            assert(buffer);
            dirty = false;
            Render::getContext()->UpdateSubresource(buffer, 0, NULL, &params, 0, 0);
        }

        inline bool isDirty() const {return dirty;}

};

//An HLSL Buffer<T>
template<class T>
struct Buffer {
    public:
        typedef std::vector<T> container_t;

        static_assert(sizeof(T) <= 16, "Buffer width is limited to 16.");
    private:
        ID3D11Buffer*             buffer = NULL;
        ID3D11ShaderResourceView* bufferView = NULL;
        unsigned maxSize;
    public:
        container_t data;
    public:
        bool create(size_t size, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT, std::string name = "");
        void uploadToGPU();
        inline void activateInVS(unsigned slot) {
            assert(buffer != nullptr && bufferView != nullptr);
            Render::getContext()->VSSetShaderResources(slot, 1, &bufferView);
        }
        inline void activateInPS(unsigned slot) {
            assert(buffer != nullptr && bufferView != nullptr);
            Render::getContext()->PSSetShaderResources(slot, 1, &bufferView);
        }
        inline void destroy() {
            SAFE_RELEASE(buffer);
            SAFE_RELEASE(bufferView);
        } 
};

template<class T>
bool Buffer<T>::create(size_t newMaxSize, DXGI_FORMAT format, std::string name)
{
    maxSize = unsigned(newMaxSize);
    unsigned bufferSize = maxSize * sizeof(T);
    {
        D3D11_BUFFER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = bufferSize;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        auto r = Render::getDevice()->CreateBuffer(&desc, nullptr, &buffer);
        if (FAILED(r)) {return false;}
        setDbgName(buffer, name.c_str());
    }
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Format = format;
        desc.Buffer.ElementOffset = 0;
        desc.Buffer.ElementWidth = maxSize;
        desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        auto r = Render::getDevice()->CreateShaderResourceView(buffer, &desc, &bufferView);
        if (FAILED(r)) {return false;}
        setDbgName(bufferView, (name+"SRV").c_str());
    }
    return true;
}

template<class T>
void Buffer<T>::uploadToGPU()
{
    //si peta aquí, ve a render::renderUtilsCreate() y asígnale más espacio a boneBuffer
    //(múltiplos de 16). más adelante posiblemente haya varios buffers (mucho más espacio, fraccionado).
    assert(data.size() <= maxSize); 

    D3D11_MAPPED_SUBRESOURCE mappedBuffer;
    Render::getContext()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
    memcpy(mappedBuffer.pData, data.data(), data.size()*sizeof(T));
    Render::getContext()->Unmap(buffer, 0);
}

}

#endif
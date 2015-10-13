#ifndef RENDER_TEXTURE_H_
#define RENDER_TEXTURE_H_

#include "mcv_platform.h"
#include "utils/itemsByName.h"

namespace render {

class Texture {
    public:
        typedef utils::ItemsByName<Texture> Manager;
        static inline Manager& getManager() {return manager;}
        friend class RenderedTexture;

        static const std::string PATH;
    protected:
        static Manager manager;

    protected:
        ID3D11ShaderResourceView* resourceView = nullptr;
        ID3D11Texture2D*          resource = nullptr;

    public:
        virtual bool load(const char* name);
        bool load2D(const char *name);

        void destroy();
        void activate(int slot) const;
        static void deactivate(int slot, int nslots = 1 );

		inline void setResourceView(ID3D11ShaderResourceView* res){ resourceView = res; }
		inline void setResource(ID3D11Texture2D* tex){ resource = tex; }
		inline ID3D11Texture2D* getResource() const { return resource; }
};

}

#endif

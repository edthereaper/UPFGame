#ifndef RENDER_TEXTURE_CUBE_H_
#define RENDER_TEXTURE_CUBE_H_

#include "mcv_platform.h"
#include "texture.h"

namespace render {

class RenderedTextureCube;

class TextureCube : protected Texture {
    public:
        static const XMVECTOR faceDir[6];
        static const XMVECTOR faceDirFix[6];
        static const XMVECTOR faceUp[6];
        friend RenderedTextureCube;
    public:
        using Texture::activate;
        using Texture::destroy;
        bool load(const char* name);
        inline bool load(std::string str) {return load(str.c_str());}
};

}

#endif

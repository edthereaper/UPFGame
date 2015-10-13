#ifndef INC_SHADERS_H_
#define INC_SHADERS_H_

#include "../render.h"
#include "vertex_declarations.h"

#include "utils/XMLParser.h"
#include "utils/itemsByName.h"

#include "buffer.h"

namespace render {

class Technique;

class VertexShader {
    public:
        typedef utils::ItemsByName<VertexShader> Manager;
        static Manager& getManager() {return manager;}
        friend Technique;
        static void deactivate();
    private:
        static Manager manager;

    private:
        ID3D11VertexShader*   vs = nullptr;
        ID3D11InputLayout*    vtx_layout = nullptr;

    public:
        bool compile(const char* szFileName
          , const char* szEntryPoint
          , const VertexDecl &decl );
        void destroy();
        void activate() const;
        void setCte(unsigned index_slot, const ShaderCteBase& cte);
        inline bool load(const char* s) {return true;}
};

class PixelShader {
    public:
        typedef utils::ItemsByName<PixelShader> Manager;
        static Manager& getManager() {return manager;}
        friend Technique;
        static void deactivate();
    private:
        static Manager manager;

    private:
        ID3D11PixelShader*   ps = nullptr;
    public:
        bool compile(const char* szFileName, const char* szEntryPoint);
        void destroy();
        void activate() const;
        inline bool load(const char* s) {return true;}
};

class Technique : private utils::XMLParser {
    public:
        typedef utils::ItemsByName<Technique> Manager;
        static inline Manager& getManager() {return manager;}
        static inline const Technique*const getCurrent() {return current;}
        static const VertexDecl* getVertexDeclaration(std::string vdName);
    private:
        static Manager manager;
        static bool     reloading;
        static Technique* current;
    private:
        std::string vsName;
        std::string vdName;
        std::string vsfxName;
        std::string psName;
        std::string psfxName;
        VertexShader*   vs;
        PixelShader*    ps;
        bool            uses_bones;
        void onStartElement(const std::string &elem, utils::MKeyValue &atts);
        bool loadVS(std::string fxName, std::string vsName, const VertexDecl* decl);
        bool loadPS(std::string fxName, std::string psName);


    public:
        Technique()=default;
        Technique(std::string vsfxName, std::string _vdName, std::string vsName,
            std::string psfxName, std::string psName);

        bool load(const char* name);
        void activate() const;
        void destroy();
        inline bool usesBones() const {return uses_bones;}
        inline bool reload(const std::string& name) { 
            auto prevReload = reloading;
            reloading = true;
                auto decl = getVertexDeclaration(vdName);
            bool is_ok=true;
            is_ok &= loadVS(vsfxName, vsName, decl);
            is_ok &= loadPS(psfxName, psName);
            reloading = prevReload;
            return is_ok;
        }

        inline const std::string& getVSFileName() const {return vsfxName;}
        inline const std::string& getVSName() const {return vsName;}
        inline const std::string& getVDName() const {return vdName;}
        inline const std::string& getPSFileName() const {return psfxName;}
        inline const std::string& getPSName() const {return psName;}
};

}

#endif

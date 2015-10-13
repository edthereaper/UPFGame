#ifndef COMPONENT_IMPORT_XML_H_
#define COMPONENT_IMPORT_XML_H_

#include "handle.h"
#include "utils/XMLParser.h"

namespace component {

class Importer : public utils::XMLParser {
    private:
        Handle currentEntity;
        Handle currentComponent;
        Handle rootHandle; 
        bool   prefabs = false;
        bool   initEntity = false;
    public:
	    void onStartElement(const std::string &elem, utils::MKeyValue &atts);
	    void onEndElement(const std::string &elem);

        Importer() {}
        inline void setParsingPrefabs(bool b = true) { prefabs = b; }
        inline bool getParsingPrefabs() const { return prefabs; }
        inline void setCurrentEntity(Handle h) {currentEntity = h;}
        inline Handle getRootHandle() const { return rootHandle; }

        static inline bool parse(const std::string filename) {
            return Importer().xmlParseFile(filename);
        }

		std::map<std::string, void *> getParsingParticles() const;
};


}

#endif
#ifndef UTILS_XMLPARSER_
#define UTILS_XMLPARSER_

#pragma warning (disable : 4786 )

#include <string>
#include <map>
#include "mcv_platform.h"

#define XMLCALL __cdecl

namespace utils {

class XMLParser;

class MKeyValue : protected std::map<std::string, std::string> {
    protected:
        typedef std::map<std::string, std::string> map_t;
	    void writeAttributes (std::ostream &os, std::string indent="\t", std::string separator="\n") const;
	    void setRawString(const std::string &w, const std::string s);
    public:
        friend XMLParser;

	    void put( const char *what, int value );
	    void put( const char *what, float value );
	    void put( const char *what, bool value );
	    void writeSingle (std::ostream &os, const char *what,
            std::string indent="", std::string separator=" ", std::string indentAtts="") const;
	    void writeStartElement( std::ostream &os, const char *what,
            std::string indent="", std::string separator=" ", std::string indentAtts="") const;
	    void writeEndElement( std::ostream &os, const char *what, std::string indent="") const;

        bool          has(const char *what) const;
        unsigned long getHex (const char *what, unsigned long default_value = 0) const;
	    int           getInt (const char *what, int default_value = 0) const;
	    bool          getBool (const char *what, bool default_value = false) const;
	    float         getFloat (const char *what, float default_value = 0.f) const;
	    XMVECTOR      getPoint (const char *what, XMVECTOR def=zero_v) const;
	    XMVECTOR      getQuat (const char *what, XMVECTOR def=one_q) const;
		XMVECTOR      getQuatWithoutNorm(const char *what, XMVECTOR def = one_q) const;
        inline XMVECTOR getFloat4 (const char *what, XMVECTOR def=zero_v) const {
            return getQuatWithoutNorm(what, def);
        }
	    void          getMatrix (const char *what, XMMATRIX &target) const;
	    std::string	  getString(const std::string &what, const std::string default_value="") const;
        inline std::string operator[](const std::string &what) const {return getString(what);}
        
        void          setHex (const char *what, unsigned long value = 0);
	    void          setInt (const char *what, int value = 0);
	    void          setBool (const char *what, bool value = false);
	    void          setFloat (const char *what, float value = 0.f, int precision=-1);
	    void          setPoint (const char *what, XMVECTOR value=zero_v, int precision=-1);
	    void          setQuat (const char *what, XMVECTOR value=one_q, int precision=-1);
	    inline void   setFloat4 (const char *what, XMVECTOR value=zero_v, int precision=-1) {
            setQuat(what, value, precision);
        }
	    void setString(const std::string &w, const std::string s="");

        using std::map<std::string, std::string>::empty;
};

class XMLParser {
    private:
	    // Static for interface with XML
        static void XMLCALL xmlStartElement(void *userData, const char *elem, const char **atts);
        static void XMLCALL xmlData(void *userData, const char *data, int length);
        static void XMLCALL xmlEndElement(void *userData, const char *elem);

    protected:
	    std::string  xml_error;

    public:
	    virtual void onStartElement (const std::string &elem, MKeyValue &atts);
	    virtual void onData (const std::string &data);
	    virtual void onEndElement (const std::string &elem);
	    virtual ~XMLParser( ) { }

	    bool xmlParseFile (const std::string &filename);
	    bool xmlParseStream (std::istream &is, const char *stream_name = NULL);
	    const std::string &getXMLError() const { return xml_error; }
};

}

#endif 
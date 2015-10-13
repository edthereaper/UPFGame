#pragma warning( disable : 4786 )

#include "mcv_platform.h"
#include <cstdio>
#include <cassert>
#include "XMLParser.h"
#include <sstream>
#include <fstream> 
#include <iomanip> 
#define XML_STATIC
#include "expat/expat.h"
#include "utils.h"

using namespace DirectX;

namespace utils  {

/*------------------------------------------------------------------
| 
\------------------------------------------------------------------*/
std::string decodeFromUTF8(const char *data, size_t nMax) {    
	const unsigned char *szSource = (const unsigned char *) data;

	std::string sFinal;
	sFinal.reserve( nMax );

	size_t n;    
	for (n = 0; n < nMax; ++n) {
		unsigned char z = szSource[n];        
		if (z < 127) {            
			sFinal += (TCHAR)z;        
		}        
		else if (z >= 192 && z <= 223)        
		{
			assert( n < nMax - 1);
			// character is two bytes            
			if (n >= nMax - 1)                 
				break; // something is wrong            
			unsigned char y = szSource[n+1];            
			sFinal += (TCHAR)( (z-192)*64 + (y-128) );            
			n = n + 1;        
		}
		else if (z >= 224 && z <= 239)        
		{            
			// character is three bytes            
			assert( n < nMax - 2);
			if (n >= nMax - 2)                 
				break; // something is wrong            
			unsigned char y = szSource[n+1];            
			unsigned char x = szSource[n+2];            
			sFinal += (TCHAR)( (z-224)*4096 + (y-128)*64 + (x-128) );            
			n = n + 2;        
		}        
		else if (z >= 240 && z <= 247)        
		{            
			// character is four bytes            
			assert( n < nMax - 3);
			if (n >= nMax - 3)                 
				break; // something is wrong            
			unsigned char y = szSource[n+1];            
			unsigned char x = szSource[n+2];            
			unsigned char w = szSource[n+3];            
			sFinal += (TCHAR)( (z-240)*262144 + (y-128)*4096 +
				(x-128)*64 + (w-128) );            
			n = n + 3;        
		}
    else if (z >= 248 && z <= 251)        
		{            
			// character is four bytes            
			assert( n < nMax - 4);
			if (n >= nMax - 4)                 
				break; // something is wrong            
			unsigned char y = szSource[n+1];            
			unsigned char x = szSource[n+2];            
			unsigned char w = szSource[n+3];            
			unsigned char v = szSource[n+4];            
			sFinal += (TCHAR)( (z-248)*16777216 + (y-128)*262144 + 
				(x-128)*4096 + (w-128)*64 + (v-128) );            
			n = n + 4;        
		}        
		else if (z >= 252 && z <= 253)       
		{            
			// character is five bytes            
			assert( n < nMax - 5);
			if (n >= nMax - 5)                 
				break; // something is wrong            
			unsigned char y = szSource[n+1];            
			unsigned char x = szSource[n+2];            
			unsigned char w = szSource[n+3];            
			unsigned char v = szSource[n+4];            
			unsigned char u = szSource[n+5];            
			sFinal += (TCHAR)( (z-252)*1073741824 + (y-128)*16777216 +
				(x-128)*262144 + (w-128)*4096 + (v-128)*64 + (u-128) );
            n = n + 5;        
		}    
	}    
	return sFinal;
}

/*------------------------------------------------------------------
| 
\------------------------------------------------------------------*/
std::string MKeyValue::getString(const std::string &what, const std::string default_value) const {
	const_iterator it = find (what);
	if (it == end())
		return default_value;
	return decodeFromUTF8 ( it->second.c_str(), strlen ( it->second.c_str() ) );
}

/*------------------------------------------------------------------
| 
\------------------------------------------------------------------*/
int    MKeyValue::getInt (const char *what, int default_value) const {
	const_iterator it = find (what);

	if (it == end())
		return default_value;
	return atoi ((*it).second.c_str());
}

unsigned long MKeyValue::getHex (const char *what, unsigned long default_value) const {
	const_iterator it = find (what);

	if (it == end())
		return default_value;
    try {
	    return std::stoul((*it).second.c_str(), 0, 16);
    } catch (...) {return default_value;}
}

template < class T >
void putKey( MKeyValue &k, const char *what, T value ) {
	/*std::ostrstream oss;
	oss << value << '\0';
  char *buf = oss.str( );
	k[ what ] = buf;
  delete buf;*/
	// Nos libramos del delete?
	std::ostringstream oss;
	oss << value;
	const std::string &buf = oss.str( );
	k[ what ] = buf;
}

void MKeyValue::put( const char *what, int value ) {
	putKey ( *this, what, value);
}
void MKeyValue::put( const char *what, bool value ) {
	putKey ( *this, what, value);
}
void MKeyValue::put( const char *what, float value ) {
	putKey ( *this, what, value);
}

float MKeyValue::getFloat (const char *what, float default_value) const {
	const_iterator it = find (what);

    if (it == end()) {return default_value;}
    else {
        if ((*it).second=="infinite") {return FLT_MAX;}
        else {return (float)atof ((*it).second.c_str());}
    }
}

bool MKeyValue::getBool (const char *what, bool default_value) const {
	const_iterator it = find (what);
	if (it == end())
		return default_value;
  // Check agains valid keywords
  const char *yes[] = {"1", "yes", "YES", "Yes", "true", "TRUE", "True", "on", "ON", "On"};
  int i = 0;
  for (i=0; i<sizeof (yes)/sizeof(yes[0]); ++i) 
    if ((*it).second == yes[i])
      return true;
  return false;
}

bool namedVector(XMVECTOR& ans, const std::string& name)
{
    if (name == "x" || name == "X" || name == "r" || name == "R") {
        ans = xAxis_v;
        return true;
    } else if (name == "y" || name == "Y" || name == "g" || name == "G") {
        ans = yAxis_v;
        return true;
    } else if (name == "z" || name == "Z" || name == "b" || name == "B") {
        ans = zAxis_v;
        return true;
    } else if (name == "w" || name == "W" || name == "a" || name == "A") {
        ans = wAxis_v;
        return true;
    } else if (name == "0") {
        ans = zero_v;
        return true;
    } else if (name == "1") {
        ans = one_v;
        return true;
    } else {
        return false;
    } 
}

XMVECTOR MKeyValue::getPoint (const char *what, XMVECTOR def) const {
	const_iterator it = find (what);
	if (it == end()) return def;
    XMVECTOR named;
    if (namedVector(named, it->second)) {
        return named;
    }
	const char *values = it->second.c_str();
    float x, y, z;
	int n = sscanf( values, "%f %f %f", &x, &y, &z );
    assert( n == 3 );
    return XMVectorSet( x, y, z, 1.f );
}

bool MKeyValue::has(const char *what) const {
  return find(what) != end();
}

XMVECTOR MKeyValue::getQuat (const char *what, XMVECTOR def) const {
	const_iterator it = find (what);
	if (it == end()) return def;
	const char *values = it->second.c_str();

    float x, y, z, w;
    int n = sscanf( values, "%f %f %f %f", &x, &y, &z, &w );
    assert( n == 4  );

    return XMQuaternionNormalize(XMVectorSet( x, y, z, w ));
}


XMVECTOR MKeyValue::getQuatWithoutNorm(const char *what, XMVECTOR def) const {
	const_iterator it = find(what);
	if (it == end()) return def;
    XMVECTOR named;
    if (namedVector(named, it->second)) {
        return named;
    }
	const char *values = it->second.c_str();
	float x, y, z, w;
	int n = sscanf(values, "%f %f %f %f", &x, &y, &z, &w);
	assert(n == 4);

	return XMVectorSet(x, y, z, w);
}

void MKeyValue::setHex (const char *what, unsigned long value)
{
    std::stringstream ss;
    ss<< std::hex << value;
    map_t::operator[](what)=ss.str();
}

void MKeyValue::setInt (const char *what, int value)
{
    std::stringstream ss;
    ss<< std::dec << value;
    map_t::operator[](what)=ss.str();
}

void MKeyValue::setFloat (const char *what, float value, int precision)
{
    std::stringstream ss;
    if (precision>=0) {ss << std::fixed << std::setprecision(precision);}
    ss << value;
    map_t::operator[](what)=ss.str();
}

void MKeyValue::setBool (const char *what, bool value)
{
    map_t::operator[](what)=value?"yes":"no";
}

void MKeyValue::setPoint (const char *what, XMVECTOR value, int precision)
{
    std::stringstream ss;
    if (precision>=0) {ss << std::fixed << std::setprecision(precision);}
    ss << XMVectorGetX(value) << " " << XMVectorGetY(value) << " " << XMVectorGetZ(value);
    map_t::operator[](what)=ss.str();
}

void MKeyValue::setQuat (const char *what, XMVECTOR value, int precision)
{
    std::stringstream sst;
    if (precision>=0) {sst << std::fixed << std::setprecision(precision);}
    sst << XMVectorGetX(value) << " " << XMVectorGetY(value) << " "
        << XMVectorGetZ(value) << " " << XMVectorGetW(value);
    map_t::operator[](what)=sst.str();
}

void MKeyValue::setString(const std::string &w, const std::string s)
{
    std::string o = "";
    for (const char& c : s) {
        switch (c) {
            case '\"': o += "&quot;"; break;
            case '\'': o += "&apos;"; break;
            case '<':  o += "&lt;"; break;
            case '>':  o += "&gt;"; break;
            case '&':  o += "&amp;"; break;
            default:   o += c;
        }
    }
    map_t::operator[](w)=o;
}


void MKeyValue::setRawString(const std::string &w, const std::string s)
{
    map_t::operator[](w)=s;
}

void MKeyValue::getMatrix (const char *what, XMMATRIX &target) const {
    fatal("not implemented");
  /*
	const_iterator it = find (what);
	assert( it != end() || fatal( "Can't find matrix attribute %s\n", what ));
	const char *values = it->second.c_str();
	int n = sscanf( values, 
		"%f %f %f "
		"%f %f %f "
		"%f %f %f "
		"%f %f %f"
	, &target.m[0][0]
	, &target.m[0][1]
	, &target.m[0][2]
	, &target.m[1][0]
	, &target.m[1][1]
	, &target.m[1][2]
	, &target.m[2][0]
	, &target.m[2][1]
	, &target.m[2][2]
	, &target.m[3][0]
	, &target.m[3][1]
	, &target.m[3][2]
	);
	assert( n == 12 || fatal( "Can't read 12 floats from matrix attribute %s. only %d\n", what, n ));
	target.m[0][3] = 0.0f;
	target.m[1][3] = 0.0f;
	target.m[2][3] = 0.0f;
	target.m[3][3] = 1.0f;
  */
}



void MKeyValue::writeSingle (std::ostream &os, const char *what, std::string indent, std::string separator, std::string indentAtts) const {
	os << indent << "<" << what << separator;
    writeAttributes (os, indentAtts, separator);
    os << ((separator.find('\n') == std::string::npos) ? "" : indent) << "/>\n";
}

void MKeyValue::writeAttributes (std::ostream &os, std::string indent, std::string separator) const {
  const_iterator i = begin( );
	while( i != end( ) ) {
		os << indent << i->first << "=\"" << i->second << "\"" << separator;
		++i;
	}
}

void MKeyValue::writeStartElement(std::ostream &os, const char *what, std::string indent, std::string separator, std::string indentAtts) const {
    os << indent << "<" << what << separator;
    writeAttributes(os, indentAtts, separator);
    os << ((separator.find('\n') == std::string::npos) ? "" : indent) << ">\n";
}

void MKeyValue::writeEndElement( std::ostream &os, const char *what, std::string indent) const {
	os << indent << "</" << what << ">\n";
}

/*-<==>-----------------------------------------------------------------
/ Real xml handlers
/----------------------------------------------------------------------*/
void XMLParser::onStartElement (const std::string &elem, MKeyValue &atts) {
	(void) elem;
	(void) atts;
}

void XMLParser::onData (const std::string &data) {
	(void) data;
}

void XMLParser::onEndElement (const std::string &elem) {
	(void) elem;
}


/*------------------------------------------------------------------
| 
\------------------------------------------------------------------*/
void XMLParser::xmlStartElement (void *userData, const char *element, const char **atts) {
  XMLParser *self = (XMLParser *)userData;
	// Convert the array of char pointers to a map
	MKeyValue map_atts;
	for (const char **p = atts; *p; p += 2) {
		const char *key = *p;
		const char *value = *(p+1);
		map_atts.setRawString(key, value);
  } 
  self->onStartElement(std::string (element), map_atts);
}

void XMLParser::xmlData( void *userData, const char *data, int length ) {
	XMLParser *self = (XMLParser *)userData;
	
	self->onData( decodeFromUTF8 (data, length ) );
}

void XMLParser::xmlEndElement (void *userData, const char *element) {
  XMLParser *self = (XMLParser *)userData;
  self->onEndElement(std::string (element));
}

/*------------------------------------------------------------------
| 
\------------------------------------------------------------------*/
bool XMLParser::xmlParseFile (const std::string &filename) {
	std::ifstream is( filename.c_str() );
	if ( !is.is_open() ) {
		xml_error = "XML File " + filename + " not found";
		return false;
	}
	return xmlParseStream( is, filename.c_str() );
}

/*------------------------------------------------------------------
| 
\------------------------------------------------------------------*/
#undef BUFSIZ
#define BUFSIZ 8192

bool XMLParser::xmlParseStream (std::istream &is, const char *stream_name) {
	// Clear error msg
	char buf[BUFSIZ];
	xml_error = "";

	XML_Parser parser = XML_ParserCreate(NULL);

	XML_SetUserData(parser, this);
	XML_SetElementHandler(parser, xmlStartElement, xmlEndElement);
	XML_SetCharacterDataHandler(parser, xmlData);
	bool done = false;

	do {
		is.read (buf, sizeof(buf));
		size_t len = is.gcount( );
		done = len < sizeof(buf);
		if (!XML_Parse(parser, buf, int(len), done)) {
			char msg[512];
			_snprintf (msg, sizeof(msg)-1, "XML Parser error '%s' at line %d when processing input stream( %s )\n",
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser),
				stream_name ? stream_name : "<UNNAMED>"
				);
			xml_error = std::string (msg);
			done = false;
			break;
		}
	} while (!done);

	XML_ParserFree(parser);
	return done;
}

}
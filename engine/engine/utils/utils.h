#ifndef UTILS_UTILS_H_
#define UTILS_UTILS_H_

#include <algorithm>
#include <array>
#include <string>
#include <fstream>
#include <functional>
#include <cstdlib>

#include "vectorUtils.h"
#include "random.h"

#include "numberUtils.h"

namespace utils {
   
#define USER_VK_WHEELUP   0xBA200001
#define USER_VK_WHEELDOWN 0xBA200002

//#define _PARTICLES
//#define _CINEMATIC_
//#define _LUA_CONSOLE_

#define TXT(x) (#x)

typedef struct{} empty_t;

#define SAFE_DELETE(what) {if (what != nullptr) {delete what; what = nullptr;}}
#define SAFE_DELETE_ARRAY(what) {if (what != nullptr) {delete[] what; what = nullptr;}}

bool fatal(const char* fmt, ...);
bool dbg(const char* fmt, ...);

bool dbg_release(const char* fmt, ...);

inline bool dbgXMVECTOR3(const XMVECTOR& v, bool nl=false) {
    return dbg("(%3.3f, %3.3f, %3.3f)%s",
        DirectX::XMVectorGetX(v),
        DirectX::XMVectorGetY(v),
        DirectX::XMVectorGetZ(v),
        nl?"\n":"");
}

inline bool dbgXMFLOAT3(const XMFLOAT3& v, bool nl = false) {
	return dbg("(%3.3f, %3.3f, %3.3f)%s",
		v.x,
		v.y,
		v.z,
		nl ? "\n" : "");
}

inline bool dbgXMFLOAT4(const XMFLOAT3& v, bool nl = false) {
	return dbg("(%3.3f, %3.3f, %3.3f, %3.3f)%s",
		v.x,
		v.y,
		v.z,
		v.z,
		nl ? "\n" : "");
}


inline bool dbgXMVECTOR4(const XMVECTOR& v, bool nl=false) {
    return dbg("(%3.3f, %3.3f, %3.3f, %3.3f)%s",
        DirectX::XMVectorGetX(v),
        DirectX::XMVectorGetY(v),
        DirectX::XMVectorGetZ(v),
        DirectX::XMVectorGetW(v),
        nl?"\n":"");
}

inline bool fexists(const std::string& filename) {
	std::ifstream ifile(filename.c_str());
	return ifile.is_open();
}

template<class T> struct wrapper {
    private:
        T v;
    public:
        wrapper(const T& v):v(v){}
        inline operator T() {return v;}
};

template<class T>
inline wrapper<T> wrap(const T& a) {return wrap<T>(a);}

template <unsigned N, class T>
inline T repeat(T (*f)(T), T a)
{
    for(unsigned i=0; i<N;i++) {a=f(a);}
    return a;
}

template<class Iterator_T>
struct range_t {
    Iterator_T beginIt;
    Iterator_T endIt;

    range_t(const Iterator_T& begin, const Iterator_T& end) :
        beginIt(begin), endIt(end) {}
    range_t(const std::pair<Iterator_T,Iterator_T>& range) :
        range_t(range.first, range.second) {}

    Iterator_T begin() {return beginIt;}
    Iterator_T end() {return endIt;}
};

bool isKeyPressed(int key);
const char* vector3ToString(XMVECTOR v);

const float WHITE[4]        = {1.f, 1.f, 1.f, 1.0};
const float RED[4]          = {1.f, 0.f, 0.f, 1.0};
const float GREEN[4]        = {0.f, 1.f, 0.f, 1.0};
const float BLUE[4]         = {0.f, 0.f, 1.f, 1.0};
const float CYAN[4]         = {0.f, 1.f, 1.f, 1.0};
const float MAGENTA[4]      = {1.f, 0.f, 1.f, 1.0};
const float YELLOW[4]       = {1.f, 1.f, 0.f, 1.0};
const float BLACK[4]        = {0.f, 0.f, 0.f, 1.0};
const float GRAY[4]         = {.5f, .5f, .5f, .5f};
const float DARK_BLUE[4]    = {.3f, .4f, .6f, 1.f};
const float BLACK_A[4]      = {0.f, 0.f, 0.f, 0.0};
const float RED_A[4]        = {1.f, 0.f, 0.f, 0.0};
const float GREEN_A[4]      = {0.f, 1.f, 0.f, 0.0};
const float BLUE_A[4]       = {0.f, 0.f, 1.f, 0.0};
const float WHITE_A[4]      = {1.f, 1.f, 1.f, 0.0};


template <class Enum_E>
static Enum_E enumFromString(const std::string& str, Enum_E init,
    std::function<Enum_E(std::string)> getByName) {
    Enum_E e = init;
    size_t off = 0;
    for(size_t pos = str.find('|', off);
        pos < str.size();
        pos = str.find('|', off)) {

        e = Enum_E(e | getByName(str.substr(off, pos-off)));
        off = pos+1;
    }
    e = Enum_E(e | getByName(str.substr(off)));
    return e;
}

/* Accumulator */
template <typename Count_T = int >
class Counter
{
    private:
        Count_T counter = 0;
    public:
        Counter()=default;
        Counter(Count_T init) : counter(init) {}
        inline void reset() {counter = 0;}
        inline Count_T get() const {return counter;}
        inline void set(Count_T f) {counter=f;}
        inline Count_T count(Count_T f) {return counter+=f;}
        inline operator Count_T() const {return get();}
};

/* remove a value from a container */
template <class Container_T>
inline bool erase(Container_T& c,const typename Container_T::value_type& val)
{
    auto end(c.end());
    auto it(std::find(c.begin(), end, val));
    if (it != end) {
        c.erase(end);
        return true;
    } else {
        return false;
    }
}

/* remove all instances of a value from a container */
template <class Container_T>
inline bool eraseAll(Container_T& c,const typename Container_T::value_type& val)
{
    auto end(c.end());
    auto it(std::remove_if(c.begin(), c.end(),
        [=](const typename Container_T::value_type& a) {
                return a==val;
            }
        ));
    if (it != end) {
        c.erase(it, end);
        return true;
    } else {
        return false;
    }
}

/* remove all instances of a value from a container */
template <class Container_T, class Container2_T>
inline bool eraseAllFrom(Container_T& c, const Container2_T& from)
{
    auto end(c.end());
    auto it(std::remove_if(c.begin(), end,
        [=](const typename Container_T::value_type& val) {
                for (auto a : from) {
                    if (a==val) {return true;}
                }
                return false;
            }
        ));
    if (it != end) {
        c.erase(it, end);
        return true;
    } else {
        return false;
    }
}

/* const char* comparator functor */
struct StrLess {
    inline bool operator() (const char* a, const char * b) const {
	    return strcmp(a, b) < 0;
	}
};

/* Search a key in a map, and return a default if not found*/
template <class Map_T>
inline typename Map_T::mapped_type& atOrDefault(
    Map_T& map,
    typename Map_T::key_type& key,
    typename Map_T::mapped_type& def)
{
    auto i(map.find(key));
    if (i != map.end()) {
        return i->second;
    } else {
        return def;
    }
}

/* Search a key in a map, and return a default if not found*/
template <class Map_T>
inline const typename Map_T::mapped_type& atOrDefault(
    const Map_T& map,
    const typename Map_T::key_type& key,
    const typename Map_T::mapped_type& def)
{
    auto i(map.find(key));
    if (i != map.end()) {
        return i->second;
    } else {
        return def;
    }
}

template<class T, class C>
inline T& find(C c, T v) {
    return std::find(c.begin(), c.end(), v);
}

/* Sequential search using operator== 
    (std::find requires the comparison being with the same type, this one just requires
    the operator to exist)
*/
template<class It_T, class V>
inline It_T seqFind(const It_T& begin, const It_T& end, const V& val) {
    for (auto it = begin; it != end; it++) {
        if (*it == val) {return it;}
    }
    return end;
}

template<typename T> 
inline void safe_delete(T*& a) {
	delete a;
	a = NULL;
}


template<typename T> 
inline void identity_pair(const T&, const T&) {}

XMFLOAT3 toXMFloat3(XMVECTOR v);
XMFLOAT4 toXMFloat4(XMVECTOR v);
#define CHARACTER_HEIGHT 1.65f
#define CHARACTER_HEIGHTS(x) (CHARACTER_HEIGHT*(x))

/* Implementation copied from std::partition.
   does the same thing, but instead of returning the iterator to the first element
   that doesn't satisfy the unary predicate, modifies the input begin argument,
   and returns whether the function made any change

   Also includes a swapping function. This function is not meant to do the swapping itself,
   (this function takes care of that), but to do additional changes
*/
template<class BidIt, class UPr, class Swap>
inline bool partition_swap(BidIt& begin, BidIt end, UPr p, Swap swap)
{	
    auto begin_copy = begin;
    bool changed = false;
    for (;;++begin_copy) {
        // skip in-place elements at beginning
    	for (; begin_copy != end && p(*begin_copy); ++begin_copy);
    	if (begin_copy == end) break;
    
        // skip in-place elements at end
    	for (; begin_copy != --end && !p(*end); );
    	if (begin_copy == end) break;
    
        swap(*begin_copy, *end);
    	std::iter_swap(begin_copy, end);
        changed = true;
    }
    begin = begin_copy;
    return changed;
}

//Bindable constructors
template<class T, class... Args>
inline T& construct(Args... args) {return T(args...);}
template<class T, class... Args>
inline T* allocate(Args... args) {return new T(args...);}

template<class T, class K_T, class FN_T>
struct lazy {
    K_T key;
    bool calculated = false;
    inline K_T get(const T& t, const FN_T& fn) {
        if (!calculated) {
            key = fn(t);
            calculated = true;
        }
        return key;
    }
};

}

#endif

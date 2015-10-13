#ifndef COMPONENTS_H_
#define COMPONENTS_H_

#include "utils/XMLParser.h"

namespace component {
    
/* Initialize everything from the components framework */
void init();

/* Destroy everything from the components framework */
void cleanup();


/* Special component that just holds a variable of a certain type
   Useful for temporal components
 */
template<class T> struct CComponent {
    T value;
    operator T() {return value;}
    T& operator=(const T& a) {value = a; return *this;}
    void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {}
    inline void update(float elapsed) {}
    inline void init() {}
};

}

#endif
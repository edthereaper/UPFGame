#ifndef LEVEL_STATIAL_INDEX_H_
#define LEVEL_STATIAL_INDEX_H_

#include "components/components.h"

namespace level {

class SpatiallyIndexed
{
    private:
        void findSpatialIndexAux(component::Handle);

    protected:
        int spatialIndex = -1;

    protected:
        template<class T>
        void findSpatialIndex(T* self) {
            findSpatialIndexAux(component::Handle(self));
        }
        bool isSpatiallyGood(int threshold=1) const;
    public:
        inline int getSpatialIndex() const {return spatialIndex;}
        inline void setSpatialIndex(int s) {spatialIndex = s;} 
};

}

#endif
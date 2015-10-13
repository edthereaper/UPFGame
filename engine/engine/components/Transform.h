#ifndef COMPONENT_TRANSFORM_H_
#define COMPONENT_TRANSFORM_H_

#include "mcv_platform.h"
#include "utils/XMLParser.h"
#include "utils/vectorUtils.h"

namespace component {

class Transform {
    protected:
	    XMVECTOR position   = utils::zero_v;
	    XMVECTOR rotation   = utils::one_q;
	    XMVECTOR scale      = utils::one_v;
		XMVECTOR pivot	    = utils::zero_v;

        #ifdef _LIGHTTOOL
	        XMVECTOR lookAtPos = utils::zero_v;
        #endif

    public:
        Transform()=default;
        Transform(XMVECTOR position, XMVECTOR rotation = utils::one_q,
            XMVECTOR scale = utils::one_v, XMVECTOR pivot = utils::zero_v) :
            position(position), rotation(rotation), scale(scale), pivot(pivot) {}

        inline const Transform& set(const Transform& t) {
            setPosition(t.getPosition());
            setRotation(t.getRotation());
            setScale(t.getScale());
            setPivot(t.getPivot());
            return *this;
        }
        inline const Transform& operator=(const Transform& t) {return set(t);}

        inline void setRotation(XMVECTOR newRotation)   {rotation = DirectX::XMQuaternionNormalize(newRotation);}
        inline void setPosition(XMVECTOR newPosition)   {position = newPosition; }
        inline void setScale(XMVECTOR newScale)         {scale = newScale; }
        inline void setPivot(XMVECTOR p)                {pivot = p; }

		inline XMVECTOR getRotation() const             {return rotation; }
        inline XMVECTOR getPosition() const             {return position; }
        inline XMVECTOR getScale() const                {return scale; }
        inline XMVECTOR getPivot() const                {return pivot; }

		 #ifdef _LIGHTTOOL
	        inline XMVECTOR getLookAt() const           {return lookAtPos; }
        #endif

        inline XMVECTOR& refRotation()                  {return rotation; }
        inline XMVECTOR& refPosition()                  {return position; }
        inline XMVECTOR& refScale()                     {return scale; }
        inline XMVECTOR& refPivot()                     {return pivot; }

    	XMMATRIX getWorld() const;
        XMVECTOR getFront() const;
        XMVECTOR getLeft() const;
        XMVECTOR getUp() const;

        bool isInFront(XMVECTOR loc) const;
        bool isInLeft(XMVECTOR loc) const;
        bool isInFov(XMVECTOR loc, float fov_in_rad ) const;

        void lookAt(XMVECTOR new_target, XMVECTOR upAux = utils::yAxis_v);
        inline void applyRotation(XMVECTOR rotQ) {
            setRotation(DirectX::XMQuaternionMultiply(getRotation(), rotQ));
        }

        inline bool operator==(const Transform& t) {
            return t.position == position && t.rotation == rotation
                && t.scale == scale && t.pivot == pivot;
        }
        inline bool operator!=(const Transform& t) {return !operator==(t);}
};

class CTransform : public Transform {
    public:
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
		inline void update(float elapsed) { }
        inline void init() {}
};

class CRestore : public Transform {
    private:
        int spatialIndex = -1;
    public:
		inline void update(float elapsed) { }
        inline void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {}
        void init();

        inline void setSpatialIndex(int index){
            spatialIndex = index;
        }
        inline int getSpatialIndex() const {
            return spatialIndex;
        }
};

}
#endif
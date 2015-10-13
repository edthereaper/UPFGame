#ifndef COMPONENT_AABB_H_
#define COMPONENT_AABB_H_

#include "utils/XMLParser.h"

namespace component {

class AABB {
    protected:
        XMVECTOR center = utils::zero_v;
        XMVECTOR hSize = DirectX::operator-(utils::one_v);

    protected:
        /* Read from XML */
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {
            if (atts.has("min") || atts.has("max")) {
                setCorners(atts.getPoint("min", getMin()), atts.getPoint("max", getMax()));
            }
            hSize = atts.getPoint("hSize", hSize);
            center = atts.getPoint("center", center);
        }

    public:
        AABB()=default;
        AABB(const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max);

        inline XMVECTOR getMax() const {return DirectX::operator+(center,hSize);}
        inline XMVECTOR getMin() const {return DirectX::operator-(center,hSize);}
        inline XMVECTOR getSize() const {return DirectX::operator+(hSize,hSize);}
        inline XMVECTOR getCenter() const {return center;}
        inline XMVECTOR getHSize() const {return hSize;}

        AABB skinAABB(const float& skin);
        AABB skinAABB(const float& skin) const;
        inline AABB scaleAABB(const float& scale) {
            DirectX::operator*=(hSize, scale);
            return *this;
        }
        inline AABB scaleAABB(const float& scale) const {
            AABB ret = *this;
            DirectX::operator*=(ret.hSize, scale);
            return ret;
        }

        void setCorners(const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max);
        inline void setCenter(const XMVECTOR& v) {center = v;}
        inline void setHSize(const XMVECTOR& v) {hSize = v;}

        void expand(const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max);

        inline bool contains(DirectX::XMVECTOR point) const {
            return
                DirectX::XMComparisonAllTrue(DirectX::XMVector3GreaterOrEqualR(point, getMin())) &&
                DirectX::XMComparisonAllTrue(DirectX::XMVector3GreaterOrEqualR(getMax(), point));
        }
        
        inline bool contains(const AABB& box) const {
            return
                DirectX::XMComparisonAllTrue(DirectX::XMVector3GreaterOrEqualR(box.getMin(), getMin())) &&
                DirectX::XMComparisonAllTrue(DirectX::XMVector3GreaterOrEqualR(getMax(), box.getMax()))
                ;
        }

        inline bool intersects(const AABB& box) const {
            return !(
                DirectX::XMComparisonAnyTrue(DirectX::XMVector3GreaterR(box.getMin(), getMax())) ||
                DirectX::XMComparisonAnyTrue(DirectX::XMVector3GreaterR(getMin(), box.getMax()))
                );
        }

        inline AABB operator+(const XMVECTOR& v) const {
            return AABB(
                DirectX::operator+(getMin(), v),
                DirectX::operator+(getMax(), v)
                );
        }

        inline bool operator==(const AABB& b) const {
            return center == b.center && hSize == hSize;
        }

        inline bool isInvalid() const {
            return DirectX::XMComparisonAnyTrue(DirectX::XMVector3GreaterR(utils::zero_v, hSize));
        }
};

class CAABB : public AABB {
    public:
        inline void update(float) {}
        void init();
        using AABB::loadFromProperties;
        
        CAABB() = default;
        CAABB(const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max) : AABB(min, max) {}
        CAABB(const AABB& aabb) {hSize = aabb.getHSize(); center = aabb.getCenter();}
        inline operator AABB() const {
            AABB aabb;
            aabb.setCenter(center);
            aabb.setHSize(hSize);
            return aabb;
        }

        inline CAABB operator+(const XMVECTOR& v) const {
            return AABB::operator+(v);
        }

        inline bool operator==(const CAABB& b) const {
            return center == b.center && hSize == hSize;
        }
        
        XMVECTOR getOffset() const;
        inline AABB offsetAABB() const {return *this + getOffset();}
};

}

#endif
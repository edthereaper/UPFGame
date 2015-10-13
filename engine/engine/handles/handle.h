#ifndef HANDLE_H_
#define HANDLE_H_

#include "mcv_platform.h"
#include "utils/XMLParser.h"

namespace component {

class Handle {
    private:
        static bool cleanup;
    public:
	    static const uint32_t WIDTH_TYPE    = 7;
	    static const uint32_t WIDTH_INDEX   = 12;
	    static const uint32_t WIDTH_AGE     = 32 - (WIDTH_TYPE + WIDTH_INDEX);
	    static const uint32_t MAX_TYPES     = 1 << WIDTH_TYPE;
	    static const uint32_t MAX_INDEX     = 1 << WIDTH_INDEX;
	    static const uint32_t MAX_AGE       = 1 << WIDTH_AGE;
	    static const uint32_t INVALID_TYPE  = ~0;

        static inline void setCleanup(bool b=true) {cleanup=b;}
        static inline bool onCleanup() {return cleanup;}

        //naming types to make code easier to understand
        typedef uint32_t eIndex_t;  /* Index in a manager's external table */
        typedef uint32_t typeId_t;  /* Unique for each type of component */

        template< class TObj >
	    inline static Handle create() {
		    ObjectManager<TObj>* manager = getManager<TObj>();
		    return manager->createObj();
	    }
        
        inline bool operator<(const Handle& h) const {
            return getRaw() < h.getRaw();
        }

        static inline Handle fromRaw(uintptr_t raw) {
            Handle ret;
            ret.raw = (uint32_t)(raw&0xFFFFFFFF);
            return ret;
        }

        static inline Handle fromRaw(void* raw) {
            return fromRaw((uintptr_t)raw);
        }

    private:
        union {
            struct {
                eIndex_t index      : WIDTH_INDEX;
	            typeId_t type       : WIDTH_TYPE;
	            uint32_t age        : WIDTH_AGE;    /* Used to tell handles with same eIndex apart */
            };
            uint32_t raw;
        };

    public:
	    Handle() : index(~0), type(INVALID_TYPE), age(~0) { }
	    Handle(typeId_t type, eIndex_t index, uint32_t age) :
            type(type), index(index), age(age) {}

        template< class TObj >
	    Handle(const TObj *obj) {
            TObj* noConst = const_cast<TObj*>(obj);
		    ObjectManager<TObj>* manager = getManager<TObj>();
		    *this = manager->getHandle(noConst);
	    }
	    template< class TObj >
	    Handle(TObj *obj) {
		    ObjectManager<TObj>* manager = getManager<TObj>();
		    *this = manager->getHandle(obj);
	    }


	    inline typeId_t getType() const { return type; }
	    inline eIndex_t getIndex() const { return index; }
	    inline uint32_t getAge() const { return age; }

	    bool destroy();
	    Handle clone() const;
        
        void init();
        void update(float elapsed);
        
        // Owner
	    Handle  getOwner() const;
	    bool    setOwner(Handle new_owner);

        template<class TObj>
        inline Handle getBrother() const {
            return getOwner().getSon<TObj>();
        }

        template<class TObj>
        inline Handle getSon() const {
            Entity* e(*this);
            assert(e != nullptr);
            return e->get<TObj>();
        }

        template<class TObj>
        inline bool hasBrother() const {
            return getOwner().hasSon<TObj>();
        }
        
        template<class TObj>
        inline bool hasSon() const {
            Entity* e(*this);
            assert(e != nullptr);
            return e->has<TObj>();
        }

	    // Automatic conversion from Handle to the Type
	    template<class TObj>
	    operator TObj*() const {
		    auto manager = getManager<std::remove_const<TObj>::type>();
		    assert(manager != nullptr);
            if (type == 0 || !isValid()) {return nullptr;}
		    assert(this->type == manager->getType()
			    || utils::fatal("Can't convert handle of type %s to %s\n",
                    HandleManager::mRegister.get(getType())->getTypeName(),
                    manager->getTypeName())
                );
		    return manager->get(*this);
	    }

	    inline bool operator==(const Handle& handle) const {
		    return index == handle.index
			    && type == handle.type
			    && age == handle.age;
	    }

        inline bool operator!=(const Handle& handle) const {
            return !operator==(handle);
        }

	    bool isValid() const;
	    inline uint32_t getRaw() const { return raw; }
        inline void* getRawAsVoidPtr() const {return (void*)getRaw();}

        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
        const char* getTypeName() const;
};
}

#include "handleManager.h"
#include "objectManager.h"

#endif
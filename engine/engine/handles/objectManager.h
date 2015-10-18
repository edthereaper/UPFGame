#ifndef COMPONENT_OBJECT_MANAGER_H_
#define COMPONENT_OBJECT_MANAGER_H_

#include <functional>

namespace component {

/**
 * Manager that holds the objects themselves
 *
 * Preconditions on TObj:
 *      - default-constructible
 *      - copyable
 *      - moveable
 *      - destructor should be valid for default-constructed objects without side effects
 *      - implements loadFromProperties(const std::string&, utils::MKeyValue&) : void
 *      - implements update(float elapsed) : void
 *      - implements init(void) : void
 */
template< class TObj >
class ObjectManager : public HandleManager {
    public:
        typedef TObj object_t;

    protected:
        typedef uint32_t iIndex_t;
	    object_t* objects = nullptr; /* The stored objects */
	    mutable bool updating;

    public:
	    ObjectManager(const char* typeName, uint32_t capacity) :
            HandleManager(typeName, capacity),
            objects(nullptr), updating( false ) {
        }

        ~ObjectManager() {
            if (objects != nullptr ) {
                if (!Handle::onCleanup()) {
                    for (auto i = size; i<capacity; i++) {
                        objects[i] = TObj();
                    }
                }
                delete[] objects;
                objects = nullptr;
            }
        }

	    inline void init() {
		    HandleManager::init();
		    objects = new object_t[capacity];
	    }

	    // object_t* -> Handle such as &objects[handle.index] == object_t*
	    Handle getHandle(object_t* obj) {
		    iIndex_t iIndex = iIndex_t(obj - objects); /* Pointer arithmetic */
            if (iIndex >= capacity) {return Handle();}
		    eIndex_t eIndex = i2e[iIndex];
            if (eIndex >= capacity) {return Handle();}
		    return Handle(getType(), eIndex, entries[eIndex].age);
	    }

	    object_t* get(Handle h) const {
		    assert(h.getType() == getType() && isValid(h));
		    eIndex_t index = h.getIndex();
		    const entry_t& entry(entries[index]);

		    if (entry.age != h.getAge()) {
                // Old handle
			    return nullptr;
            }

		    assert(entry.index != INVALID_INDEX);
		    return &objects[entry.index];
	    }

        object_t* getNth(unsigned nth) const {
		    assert(nth < size);
            return &objects[nth];
        }

        void loadFromProperties(Handle who, const std::string& elem, utils::MKeyValue &atts) {
		    object_t* obj = who;
            obj->loadFromProperties( elem, atts );
        }
        
	    void updateObj(Handle h, float elapsed) {
            if (!h.isValid()) {return;}
		    eIndex_t eIndex = h.getIndex();
            assert(eIndex < capacity);
            iIndex_t iIndex = entries[eIndex].index;
            objects[iIndex].update(elapsed);
        }

        void initObj(Handle h) {
            if (!h.isValid()) {return;}
		    eIndex_t eIndex = h.getIndex();
            assert(eIndex < capacity);
            iIndex_t iIndex = entries[eIndex].index;
            objects[iIndex].init();
        }

	    Handle createObj() {
		    assert(size < capacity
                || utils::fatal("No more space for components of type %s\n", getTypeName()));

		    // The real object goes at the end of the array of real objects
		    iIndex_t iIndex = size++;
		    object_t* obj = &objects[iIndex];
		    obj = ::new(obj)object_t();  //Create new object at specified address

		    // Get a free entry
		    eIndex_t eIndex = nextFree;
		    entry_t& entry(entries[eIndex]);
		    nextFree = entry.next;
		    if (nextFree == INVALID_INDEX) {
			    lastFree = INVALID_INDEX;
			    assert(size == capacity);
		    }

		    // Setup the entry from the external table
		    entry.next  = INVALID_INDEX;
		    entry.index = iIndex;
		    entry.owner = Handle();

		    i2e[iIndex] = eIndex; //update i2e

		    return Handle(getType(), eIndex, entry.age);
	    }


	    bool destroyObj(Handle h) {
            if (!isValid(h)) {return false;}
		    assert(h.getType() == getType());
		    assert(size > 0);
            assert(!updating || utils::fatal("Can't delete during update!"));

		    eIndex_t eIndex = h.getIndex();
            if (eIndex >= capacity) {return false;}
            entry_t& entry(entries[eIndex]);
		    if (h.getAge() != entry.age) {return false;}

		    object_t *obj = &objects[entry.index];
		    obj->~object_t();
            size--;

		    if (entry.index != size) {
			    // Replace deleted object with the last element in the array
			    object_t* lastObject = &objects[size];
			    *obj = std::move(*lastObject);

			    // Update the external and internal tables with the new information
			    eIndex_t lastEIndex = i2e[size];
			    entries[lastEIndex].index = entry.index;
			    i2e[entry.index] = lastEIndex;
		    }
            objects[size] = TObj();

		    i2e[size] = INVALID_INDEX;

		    // Update the linked list of free handles
		    if (nextFree != INVALID_INDEX) {
			    entries[lastFree].next = eIndex;
			    lastFree = eIndex;
		    } else {
			    // Only when all objects have been used, and we free one object
			    lastFree = eIndex;
                nextFree = eIndex;
		    }

		    entry.index = INVALID_INDEX;
		    entry.age++;

		    return true;
	    }

	    // -----------------------------------------
	    Handle cloneObj(Handle h) {
		    object_t* originalObj = get(h);
            if (originalObj == nullptr) {return Handle();}

		    // Create a new object
		    Handle newH = createObj();
		    object_t* newObj = get(newH);

		    // Copy
		    ::new(newObj)object_t(*originalObj) ;

		    return newH;
	    }

        /* Init all components */
	    void initAll() {
		    object_t* obj = objects;
		    uint32_t num = size;
		    for (; num--; obj++) {
                obj->init();
            }
	    }
        
	    void clear() {
            if (size == 0) {return;}
            delete[] objects;
		    objects = new object_t[capacity];
		    // init table contents
		    for (eIndex_t e = 0; e < capacity; ++e) {
			    entry_t& entry(entries[e]);
			    entry.age++;
			    entry.index = INVALID_INDEX;
			    entry.next = e + 1;     // Each handle is linked to the next...
			    i2e[e] = INVALID_INDEX;
		    }
            // ... except the last one
            entries[capacity-1].next = INVALID_INDEX;

		    nextFree = 0;
		    lastFree = capacity - 1;
            size = 0;
	    }

        template <class R_TYPE>
        void forall(std::function<R_TYPE(TObj*)> fn, bool isUpdating = false) {
            if (isUpdating) {updating = true;}
		    uint32_t num = size;
		    for (object_t* obj = objects; num--; obj++) {
			    fn(obj);
            }
            if (isUpdating) {updating = false;}
        }
        template <class R_TYPE>
        void forall(R_TYPE(TObj::*fn)()const, bool isUpdating = false) const {
            if (isUpdating) {updating = true;}
		    uint32_t num = size;
		    for (object_t* obj = objects; num--; obj++) {
			    (obj->*fn)();
            }
            if (isUpdating) {updating = false;}
        }

        template <class R_TYPE>
        void forall(R_TYPE(TObj::*fn)(), bool isUpdating = false) {
            if (isUpdating) {updating = true;}
		    uint32_t num = size;
		    for (object_t* obj = objects; num--; obj++) {
			    (obj->*fn)();
            }
            if (isUpdating) {updating = false;}
        }

        template <class R_TYPE, typename... Args>
        void forall(R_TYPE(TObj::*fn)(Args...)const, Args... args, bool isUpdating = false) const {
            if (isUpdating) {updating = true;}
		    uint32_t num = size;
		    for (object_t* obj = objects; num--; obj++) {
			    (obj->*fn)(args...);
            }
            if (isUpdating) {updating = false;}
        }

        template <class R_TYPE, typename... Args>
        void forall(R_TYPE(TObj::*fn)(Args...), Args... args, bool isUpdating = false) {
            if (isUpdating) {updating = true;}
		    uint32_t num = size;
		    for (object_t* obj = objects; num--; obj++) {
			    (obj->*fn)(args...);
            }
            if (isUpdating) {updating = false;}
        }

        /* Update all components */
	    void update() {
		    updating = true;
		    uint32_t num = size;
		    for (object_t* obj = objects; num--; obj++) {
			    obj->update();
            }
		    updating = false;
	    }

        /* Update all components */
	    void update(float elapsed) {
		    updating = true;
		    uint32_t num = size;
		    for (object_t* obj = objects; num--; obj++) {
			    obj->update(elapsed);
            }
		    updating = false;
	    }

        /* Fixed interval update (required by PhysX) */
	    void fixedUpdate(float elapsed) {
		    updating = true;
		    uint32_t num = size;
		    for (object_t* obj = objects; num--; obj++, num) {
			    obj->fixedUpdate(elapsed);
            }
		    updating = false;
	    }

        struct iterator {
            object_t* obj;

            iterator(object_t* obj) : obj(obj) {} 
            iterator operator++() {return iterator(obj++);}
            iterator operator--() {return iterator(obj--);}
            object_t* operator*() const {return obj;}
            bool operator==(const iterator& i) const {return obj == i.obj;}
            bool operator!=(const iterator& i) const {return obj != i.obj;}
        };
        iterator begin() const {return objects;}
        iterator end() const {return objects+size;}
};

template< class TObj > ObjectManager<TObj>* getManager();

/* Must use this within namespace component */
#define DECLARE_OBJECT_MANAGER(TObj, name, capacity) \
    ObjectManager<TObj> name##Manager__ ( #name, capacity ); \
    template<> ObjectManager<TObj>* getManager<TObj>() { return &name##Manager__; }
}

#include "handleManager.h"
#endif

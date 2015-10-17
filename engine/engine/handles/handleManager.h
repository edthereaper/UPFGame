#ifndef COMPONENT_HANDLE_MANAGER_H_
#define COMPONENT_HANDLE_MANAGER_H_

#include "mcv_platform.h"
#include "handle.h"

namespace component {

class HandleManager {
    public:
        typedef uint32_t            iIndex_t;   /* Index in the manager's internal table*/
        typedef Handle::eIndex_t    eIndex_t;   /* Index in the manager's internal table*/

        friend Handle;

    protected:
	    static const uint32_t MAX_CAPACITY  = 1 << Handle::WIDTH_INDEX;
	    static const uint32_t MAX_TYPES     = Handle::MAX_TYPES;
	    static const uint32_t INVALID_INDEX = ~0;

	    // Entries in this manager
	    struct entry_t {
		    iIndex_t    index;    
		    uint32_t    age;      /* Index in a manager's external table */
		    eIndex_t    next;     /* To link handles in the external table */
		    Handle      owner;

		    entry_t() : index(INVALID_INDEX), age(0), next(INVALID_INDEX) {}
	    };

	    entry_t*                entries;        /* eIndex -> entry_t */
	    eIndex_t*               i2e;            /* iIndex -> eIndex */
	    uint32_t const          capacity;       /* maximum amount of items */
	    uint32_t                size;           /* current amount of items */

	    Handle::typeId_t        type;
	    const char* const       typeName;

	    // To quickly find the next free handle when creating objects
	    eIndex_t                nextFree;
	    eIndex_t                lastFree;

	    void init() {
		    assert(capacity > 0 || utils::fatal("Manager size is zero!"));
		    assert(entries == nullptr && i2e == nullptr);

		    // At this point we should have a name
		    assert(typeName != nullptr);
		    mRegister.add(this);

		    entries = new entry_t[capacity];
		    i2e = new eIndex_t[capacity];
            initTable();
        }
        
	    void deleteMem() {
            delete[] entries;
            delete[] i2e;
        }

        void initTable() {
		    // init table contents
		    for (eIndex_t e = 0; e < capacity; ++e) {
			    entry_t& entry(entries[e]);
			    entry.age = 1;
			    entry.index = INVALID_INDEX;
			    entry.next = e + 1;     // Each handle is linked to the next...
			    i2e[e] = INVALID_INDEX;
		    }
            // ... except the last one
            entries[capacity-1].next = INVALID_INDEX;

		    nextFree = 0;
		    lastFree = capacity - 1;
	    }

    public:
	    // Everything is invalid at the ctor
	    HandleManager(const char* typeName, uint32_t capacity) :
            entries(nullptr),
            i2e(nullptr),
            size(0),
            capacity(capacity), 
            type(0), 
            typeName(typeName)
	        {}

	    inline const char*      getTypeName() const { return typeName; }
	    inline Handle::typeId_t getType() const { return type; }
	    inline uint32_t         getSize() const { return size; }
	    inline uint32_t         getCapacity() const { return capacity; }

	    bool isValid(Handle h) const {
            return
                (h.getType() == getType()) &&
                (h.getIndex() < capacity) &&
                (entries[h.getIndex()].age == h.getAge());
	    }

	    // Owners
	    Handle getOwner(Handle h) {
		    assert(h.getType() == getType());
		    eIndex_t index = h.getIndex();
            if (index >= capacity) {return Handle();}
		    return entries[index].owner;
	    }

	    bool setOwner(Handle h, Handle owner) {
		    assert(h.getType() == getType());
		    eIndex_t index = h.getIndex();
            if (index >= capacity) {
                return false;
            } else {
		        entries[index].owner = owner;
		        return true;
            }
	    }

	    // Can't implement these without knowing the actual type
	    virtual ~HandleManager() {
            SAFE_DELETE_ARRAY(entries);
            SAFE_DELETE_ARRAY(i2e);
        }
	    virtual bool destroyObj(Handle h) = 0;
	    virtual Handle createObj() = 0;
	    virtual Handle cloneObj(Handle h) = 0;
	    virtual void updateObj(Handle h, float elapsed) = 0;
	    virtual void initObj(Handle h) = 0;
        virtual void loadFromProperties(Handle who, const std::string& elem, utils::MKeyValue &props) = 0;


        /* Register that holds*/
	    class Register {
            private:
		        typedef std::map< const char*, HandleManager*, utils::StrLess > managersByName_t;

		        HandleManager*      managersByType[MAX_TYPES];  /*  Handler::typeId_t -> HandleManager*  */
		        managersByName_t    managersByName;             /*  const char*       -> HandleManager*  */
		        Handle::typeId_t    nextType;

		        Register() : nextType(1) {}

	        public:
		        inline HandleManager* get(const char* name) const {
                    return utils::atOrDefault(managersByName, name, nullptr);
		        }

		        inline HandleManager* get(Handle::typeId_t type) const {
			        return type < MAX_TYPES ? managersByType[type] : nullptr;
		        }

		        void add(HandleManager* manager) {
			        assert(manager->getType() == 0);      // Should not be already registered
			        assert(nextType < MAX_TYPES);
			        manager->type = nextType;

			        // Manager should have a name at this point
			        const char* name = manager->getTypeName();
			        assert(name != nullptr);
			        assert(managersByName.find(name) == managersByName.end());

			        // Check if anoter manager already exists with the same name
			        managersByType[nextType++] = manager;
			        managersByName[name]       = manager;
		        }

                friend HandleManager;
	        };
   public:
	    static Register mRegister;
};

}

#endif


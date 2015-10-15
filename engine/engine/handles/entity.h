#ifndef COMPONENT_ENTITY_H_
#define COMPONENT_ENTITY_H_

#include "handle.h"
#include "message.h"

#include <sstream>
#include <iomanip>

#include "mcv_platform.h"

namespace component {

struct MsgDeleteSelf {
    DECLARE_MSG_ID();
};

class CName {
    public:
        static std::string generate(const std::string& str);
        static Handle get(const std::string& name);

    private:
        char name[32];
    public:
        CName() = default;
        CName(const CName&) = default;
        CName(std::string s) {setName(s);}

        inline void setName(const std::string& s) {
            std::strncpy(name, s.c_str(), 31);
            name[31]='\0';
        }
		inline std::string getName() {
			std::string str(name);
			return str;
		}

        inline std::string str() const {return name;}

        inline operator std::string() const {return str();}
        inline CName& operator=(const std::string& s) {setName(s); return *this;}

        void loadFromProperties(std::string, utils::MKeyValue atts);
        inline void init() {}
        inline void update(float) {}
};

class Entity {
    public:
        /* To message an entity instead of its components */
        static const Handle::typeId_t SELF = Handle::INVALID_TYPE;
        static void initType();

    private:
	    Handle components[Handle::MAX_TYPES];
    public:

	    ~Entity() {
		    for (uint32_t i = 0; i < Handle::MAX_TYPES; i++)
			    components[i].destroy();
	    }

	    Entity() { }
	    Entity(const Entity& e);

	    void add(Handle h);

	    template< class TObj>
	    inline bool has() const {
		    return get<TObj>().isValid();
	    }

	    template< class TObj >
	    inline Handle get() const {
            return getByType(getManager<TObj>()->getType());
	    }

        inline Handle getByType( Handle::typeId_t type ) const {
            return components[type];
        }

	    template< class TObj >
	    inline bool del() {
		    return components[getObjManager<TObj>()->getType()].destroy();
	    }

        void loadFromProperties(const std::string& elem, utils::MKeyValue& atts) {}

        void sendMsg(MessageManager::msgId_t id, const void* data);
        	    
        template <class Msg_T>
	    inline void sendMsg(const Msg_T& data) {
            sendMsg(Msg_T::getId(), &data);
        }

	    template <class Msg_T>
	    void postMsg(const Msg_T& data) {
            MessageManager::post(this, data);
	    }
        
        /* Do not call during update! You can set it up for postdeletion 
           by posting a EliminateEntityMsg message instead               */
        inline void destroy() { Handle(this).destroy();}
        inline void destroy(const MsgDeleteSelf&) {destroy();}

        void update(float elapsed) {}

        std::string getDefaultName() const {
            Handle h(this);
            std::stringstream ss;
            ss << "{"<< h.getIndex() << '.' << h.getAge()<<"}";
            return ss.str();
        }
        std::string getName() const {
            return has<CName>() ? ((CName*)get<CName>())->str() : getDefaultName();
        }

        void init();
};

class EntityList {
    public:
        typedef std::vector<Handle> container_t;
    protected:
        container_t handles;
    public:
        void reserve(size_t size) {handles.reserve(size);}

        template<class Msg_T>
        void broadcast(const Msg_T& msg){
            std::vector<Handle> cleanup;
            for (auto h : handles) {
                Entity* e(h);
                if (e != nullptr) {
                    e->sendMsg(msg);
                } else {
                    cleanup.push_back(h);
                }
            }
            if (!cleanup.empty()) {
                utils::eraseAllFrom(handles, cleanup);
            }
        }
        
        template<class Msg_T>
        void broadcastPost(const Msg_T& msg){
            std::vector<Handle> cleanup;
            for (auto h : handles) {
                Entity* e(h);
                if (e != nullptr) {
                    e->postMsg(msg);
                } else {
                    cleanup.push_back(h);
                }
            }
            if (!cleanup.empty()) {
                utils::eraseAllFrom(handles, cleanup);
            }
        }

        /* FN is a function<T(Entity*)>*/
        template<class FN>
        void forall(FN fn) {
            std::vector<Handle> cleanup;
            for (auto h : handles) {
                Entity* e(h);
                if (e != nullptr) {
                    fn(e);
                } else {
                    cleanup.push_back(h);
                }
            }
            if (!cleanup.empty()) {
                utils::eraseAllFrom(handles, cleanup);
            }
        }

        inline bool add(Entity* entity) {
            Handle h(entity);
            if (h.isValid()) {
                handles.push_back(h);
                return true;
            } else {
                return false;
            }
        }

        inline void remove(const Handle& h) {
			auto i = find(handles.begin(), handles.end(), h);
			if (i != handles.end()) handles.erase(i);
        }

		inline void clear() {
			handles.clear();
		}

        container_t::iterator begin() {return handles.begin();}
        container_t::iterator end() {return handles.end();}
        container_t::const_iterator begin() const {return handles.begin();}
        container_t::const_iterator end() const {return handles.end();}
};

class EntityListManager {
    public:
        /* static class */
        EntityListManager()=delete;

        typedef uint32_t key_t;
        static const key_t INVALID_KEY = ~0;

        /* Functor that generates keys from an entity handle */
        class KeyGen {
            public:
                virtual ~KeyGen(){}
                /* h is an Entity handle */
                virtual key_t operator()(Handle h)=0;
        };

    private:
        typedef std::map<key_t, EntityList> container_t;
        static container_t lists;
        static KeyGen* keyGen;
    public:
        static inline EntityList& get(const key_t& key) {return lists[key];}

        /* keyGen must not be null! */
        static inline key_t getKey(const Handle& h) {
            if (keyGen == nullptr) {
                return INVALID_KEY;
            } else {
                return (*keyGen)(h);
            }
        }

        static inline KeyGen* getKeyGen() {return keyGen;}
        /* The class owns the keygen and will delete it on cleanup() */
        static inline void setKeyGen(KeyGen* k) {keyGen = k;}
        static inline void cleanup() {SAFE_DELETE(keyGen);}
        static inline void cleanupLists() {
            for (auto& i : lists) {
                i.second.forall([](const Handle&){});
            }
        }

};

}

#endif
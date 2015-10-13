#ifndef COMPONENT_MESSAGE_H_
#define COMPONENT_MESSAGE_H_

#include "mcv_platform.h"
#include "handle.h"
#include "utils/buffer.h"

namespace component {
    
class Entity;

/*
 * Duck typing restrictions on the message structures:
 *  - Implements static MessageManager::msgId_t getId(void)
 *      (this precondition can be satisfied with the macro
 *      DECLARE_MSG_ID with MessageManager in the scope)
 */


/* Generic functor for message reception on a handler */
struct Functor {
    virtual ~Functor() {}
    virtual void operator()(Handle handle, const void* msg) = 0;
    virtual bool operator==(const Functor*) = 0;
};

// Functor that calls a function member of TObj to process a message
template< class TObj, class MsgData_T > 
struct Member : public Functor {
    public:
        typedef MsgData_T data_t;
        typedef TObj object_t;
        typedef void (TObj::*fn_t)(const data_t& msg);
        
    public:
        fn_t member;
        
    public:
        Member(fn_t member) : member(member){}
  
        void operator()(Handle handle, const void* msg) {
	        assert( msg  != nullptr);
            TObj* obj = handle;
            if (obj != nullptr) {
                (obj->*member)(* (const data_t*) msg);
            }
        }

        bool operator==(const Functor* f) {
            auto m = dynamic_cast<const Member<TObj, MsgData_T>*>(f);
            if (m == nullptr) {
                return false;
            } else {
                return m->member == member;
            }
        }
};

struct MsgHandler {
  Handle::typeId_t  compType;
  Functor*          execute;

  MsgHandler()=default;
  MsgHandler(Handle::typeId_t compType, Functor* execute) :
      compType(compType), execute(execute) {}
};

class MessageManager {
    public:
        typedef uint32_t msgId_t;
        typedef std::multimap<msgId_t, MsgHandler> subscriptions_t;

    private:
        struct postHeader_t {
            Handle handle;
            msgId_t msgId;
            size_t size;

            postHeader_t()=default;
            postHeader_t(Handle handle, msgId_t msgId, size_t size) :
                handle(handle), msgId(msgId), size(size) {}
        };

        static subscriptions_t subscriptions;
        static utils::buffer_t postBuffer;
    public:
        MessageManager()=delete; //static class

        static msgId_t generateUniqueMsgID();

        static inline utils::range_t<subscriptions_t::const_iterator> getSubscriptions(msgId_t id) {
            return utils::range_t<subscriptions_t::const_iterator>(
                subscriptions.equal_range(id));
        }

        /* operation will be owned by the manager and deleted upon unsubscription */
        inline static void subscribe(msgId_t msgId, Handle::typeId_t compType, Functor* operation) {
            assert(operation != nullptr);
            subscriptions.emplace(msgId, MsgHandler(compType, operation));
        }

        template<class TObj>
        inline static void subscribe(msgId_t msgId, Functor* operation) {
            subscribe(msgId, getManager<TObj>()->getType(), operation);
        }
        
        static void unsubscribe(msgId_t msgId, Handle::typeId_t compType, Functor* operation) {
            auto range(getSubscriptions(msgId));
            for (auto it = range.begin(); it != range.end(); it++) {
                if ((it->second.compType == compType) &&
                    ((*it->second.execute) == operation)) {
                    if (it->second.execute != nullptr) {
                        delete it->second.execute;
                    }
                    subscriptions.erase(it);
                    break;
                }
            }
        }

        template<class TObj>
        static void unsubscribe(msgId_t msgId, Functor* operation) {
            unsubscribe(msgId, getManager<TObj>()->getType(), operation);
        }


        /* Post a message to be dispatched later.
         * You Can only post to entities
         */
        template<class Msg_T>
        static void post(Entity* entity, const Msg_T& data){
            // Put message into the buffer
            postBuffer.put(postHeader_t(Handle(entity), Msg_T::getId(), sizeof(Msg_T)));
            postBuffer.put(&data, sizeof(Msg_T));
        }

        static void dispatchPosts();

        /* delete all functors */
        static void cleanup() {
            for (auto p : subscriptions) {
                SAFE_DELETE(p.second.execute);
            }
        }
};

// Macro to declare automatically a method that returns an unique id
// To be inserted as part of each msg type
#define DECLARE_MSG_ID() \
	static inline MessageManager::msgId_t getId() {  \
		static MessageManager::msgId_t id = MessageManager::generateUniqueMsgID();  \
		return id;  \
	}

// Macro to simplify the subscribe to an msg process
#define SUBSCRIBE_MSG_TO_MEMBER(component, msg, fn) \
    MessageManager::subscribe<component>(\
        msg::getId(),\
        new Member<component,msg>(&component::fn)\
        );
}



#endif

#include "mcv_platform.h"
#include "message.h"

#include "entity.h"

namespace component {

MessageManager::msgId_t MessageManager::generateUniqueMsgID()
{
    static msgId_t id = 1;
	return id++;
}

MessageManager::subscriptions_t MessageManager::subscriptions = subscriptions_t();
utils::buffer_t MessageManager::postBuffer = utils::buffer_t(256);

void MessageManager::dispatchPosts()
{
    while(!postBuffer.isPastEnd(sizeof(postHeader_t))) {
        auto post = postBuffer.read<postHeader_t>();
        if (post.handle.isValid() && !postBuffer.isPastEnd(post.size)) {
            Entity* e = post.handle;
            e->sendMsg(post.msgId, postBuffer.ptr());   
            postBuffer.forward(post.size);
            }
        }
    postBuffer.reset();
}

}
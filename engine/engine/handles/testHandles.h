#ifndef COMPONENT_TEST_HANDLES_H_
#define COMPONENT_TEST_HANDLES_H_
#ifdef _TEST

#include "mcv_platform.h"
#include "utils/XMLParser.h"
#include "handles/message.h"
using namespace component;

namespace test {

struct _TMsgA {
    DECLARE_MSG_ID();
    int c;
};

struct _TMsgB {
    DECLARE_MSG_ID();
    int c;
};

struct _TMsgC {
    DECLARE_MSG_ID();
};

struct _TCA
{
    int a=0,b=0;
    void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
    bool operator==(const _TCA& t) {return a == t.a && b == t.b;}

    inline void receiveMsg(const _TMsgA& msg) {
        utils::dbg("a=%d, b=%d, msga.c=%d\n", a, b, msg.c);
    }
    
    inline void receiveMsg(const _TMsgC& msg) {
        utils::dbg("A:\ta=%d,\tb=%d\n", a, b);
    }

    inline void init() {}
    inline void update(float elapsed) {}
};


struct _TCB
{
    int a=0,b=0;
    void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
    bool operator==(const _TCA& t) {return a == t.a && b == t.b;}
    inline void receiveMsg(const _TMsgB& msg) {
        utils::dbg("a=%d, b=%d, msgb.c=%d\n", a, b, msg.c);
    }
    inline void receiveMsg(const _TMsgC& msg) {
        utils::dbg("B:\ta=%d,\tb=%d\n", a, b);
    }
    
    inline void init() {}
    inline void update(float elapsed) {}
};

void testHandles();

}

#endif
#endif
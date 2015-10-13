#ifndef BEHAVIOR_BT_H_
#define BEHAVIOR_BT_H_

#include "mcv_platform.h"
#include <map>

namespace behavior {

typedef uint32_t btState_t;

enum ret_e {
    STAY, DONE, DONE_QUICKSTEP
};

/* BEHAVIOR TREE

 * T is the "executor" class. It has all the data, and also declares the state execution functions
 *
 * Duck typing for T:
 *      constructor (const Bt<T>&)
 *      void init()
 *      void reset()
 *      void update(float elapsed);
 */
template<class T>
class Bt
{
    public:
        typedef T executor_t;
        typedef btState_t nodeId_t;
        static const nodeId_t INVALID_NODE = ~0;

        static void initType();
        
        typedef ret_e (T::*action_t)(float /*elapsed*/);
        typedef bool (T::*condition_t) (float /*elapsed*/) const;
    private:
        struct node {
            public:
                friend Bt<T>;
            private:
                action_t    action = nullptr;
                condition_t condition = nullptr;
                unsigned weight = 1; // For randomizing
            public:
                enum type_e {
                    LEAF,           // do action and backtrack
                    SEQUENCE,       // do all children
                    SEQUENCE_WHILE, // do all children, stop sequence if condition is no longer satisfied
                    SEQUENCE_LOOP,  // do all children, if condition is still satisfied, loops
                    SEQUENCE_LOOPW, // do all children in loop, until the condition is no longer satisfied
                    PRIORITY,       // do the leftmost children that satisfies its condition
                    RANDOM,         // do a random child
                    CHAIN,          // do something before passing the action to its child (decorator)
                    /* Some possible applications of the CHAIN node.

                      As seen in http://aigamedev.com/open/article/decorator/:
                      Filters
                        - Limit the number of times a behavior can be run.
                          (use a Counter<int> and test in condition)
                        - Prevent a behavior from firing too often with a timer.
                          (use a Counter<float> and count the elapsed time and test in condition)
                        - Temporarily deactivate a behavior by script.
                          (test if a flag is active in condition)
                        - Restrict the number of behaviors running simultaneously.
                          (test against a shared blackboard with slots in condition)
                      Meta Operations
                        - Debug (in the action, print on debug out, or place a breakpoint)
                        - Logging (record on a log in the action)

                      Other possible uses, not in Champandard's article:
                        - Initializer action (Have the initializing happen in the
                          chain's action, to set up the leaf's action)
                        - SEQUENCE_WHILE aborted on the child condition instead of parent
                          (tree is explored in depth. If any node's condition fails, it
                          will trigger the backtracking. SEQUENCE_WHILE behaves similarly,
                          but the condition is always the same - the parent's)
                    */

                } type;
                nodeId_t leftChild = INVALID_NODE;
                nodeId_t rightBrother = INVALID_NODE;
                nodeId_t parent = INVALID_NODE;

                node()=default;
                node(type_e type, action_t action=nullptr,
                    condition_t condition=nullptr, nodeId_t parent = INVALID_NODE) :
                    type(type), parent(parent), action(action), condition(condition) {}

                inline ret_e act(executor_t& executor, float elapsed) const {
                    if (action == nullptr) {return DONE;}
                    else return (executor.*action)(elapsed);
                }

                inline bool test(T& executor, float elapsed) const {
                    if (condition == nullptr) {return true;}
                    else return (executor.*condition)(elapsed);
                }
        };
    public:
        typedef typename node::type_e nodeType_e;

    private:
        typedef std::map<nodeId_t, node> container_t;
        static container_t nodes;
	    static nodeId_t rootNode;

    private:
        T executor;
	    nodeId_t currentNode = INVALID_NODE;
	    nodeId_t currentActionNode = INVALID_NODE;
	    nodeId_t previousActionNode = INVALID_NODE;
        bool currentIsTested = false;

        void backtrack(const node&, float elapsed, bool quickStep);
        void stepToChild(const node&, float elapsed, bool quickStep);
        void stepPriorityNode(const node&, float elapsed, bool quickStep);
        void stepRandomNode(const node&, float elapsed, bool quickStep);

        /* Do not call this directly, use createChild and createRoot instead */
        static void createNode(nodeId_t id, nodeId_t parent, nodeType_e,
            condition_t condition = nullptr, action_t action=nullptr);

        static void createChild(nodeId_t id, nodeId_t parent, nodeType_e,
            condition_t condition = nullptr, action_t action=nullptr);
        static void createRoot(nodeId_t id, nodeType_e,
            condition_t condition = nullptr, action_t action=nullptr);
        static void setNodeWeight(nodeId_t id, unsigned weight);

    public:
        inline void init() {executor.init(); currentNode = rootNode;}
        inline void reset() {executor.reset(); currentNode = rootNode;}
        void update(float elapsed);

        inline const btState_t getState() const {return currentNode;}
        inline const btState_t getCurrentAction() const {return currentActionNode;}

        /* has state changed in last update? */
        inline bool hasActionChanged() const {
            return currentActionNode != previousActionNode;
        }

        inline void changeState(nodeId_t push) {
            //assert (nodes.find(state) != nodes.end());
            currentIsTested = false;
            currentNode = push;
        }

        inline T& getExecutor() {return executor;}
        inline const T& getExecutor() const {return executor;}


    /* root with condition and action */
    #define BT_CREATEROOT_CA(id, type, condition, action)\
    createRoot(executor_t::id, node::type, &executor_t::condition, &executor::action)
    /* root with no condition or action */
    #define BT_CREATEROOT(id, type)\
        createRoot(executor_t::id, node::type, nullptr, nullptr)
    /* child with condition and action */
    #define BT_CREATECHILD_CA(id, parent, type, condition, action)\
        createChild(executor_t::id, executor_t::parent, node::type,\
                    &executor_t::condition, &executor_t::action)
    /* child with only an action */
    #define BT_CREATECHILD_A(id, parent, type, action)\
        createChild(executor_t::id, executor_t::parent, node::type,\
                    nullptr, &executor_t::action)
    /* child with only a condition */
    #define BT_CREATECHILD_C(id, parent, type, condition)\
        createChild(executor_t::id, executor_t::parent, node::type,\
                    &executor_t::condition, nullptr)
    /* child with no condition or action */
    #define BT_CREATECHILD(id, parent, type)\
        createChild(executor_t::id, executor_t::parent, node::type, nullptr, nullptr)
        
    /* set a node's weight */
    #define BT_SET_WEIGHT(id, weight)\
        setNodeWeight(executor_t::id, weight)
};

template<class T>
void Bt<T>::backtrack(const node& n, float elapsed, bool quickStep)
{
    const node* self = &n;
    nodeId_t parentId = self->parent;
    for(; parentId != INVALID_NODE; parentId = self->parent) {
        //Test if self is child of a sequence
        const node& parent(nodes[parentId]);
        
        if (self->rightBrother != INVALID_NODE) {
            switch (parent.type) {
                case node::SEQUENCE_LOOP:
                case node::SEQUENCE:
                    currentNode = self->rightBrother;
                    if (quickStep) {update(elapsed);}
                    return;
                    break;
                case node::SEQUENCE_LOOPW:
                case node::SEQUENCE_WHILE:
                    if (parent.test(executor, elapsed)) {
                        currentNode = self->rightBrother;
                        if (quickStep) {update(elapsed);}
                        return;
                        break;
                    }
                default: break;
            }
        } else {
            switch (parent.type) {
                case node::SEQUENCE_LOOPW:
                case node::SEQUENCE_LOOP:
                    if (parent.test(executor, elapsed)) {
                        currentNode = parent.leftChild;
                        if (quickStep) {update(elapsed);}
                        return;
                        break;
                    }
                default: break;
            }
        }
        currentNode = parentId;
        self = &parent;
    }
}

template<class T>
void Bt<T>::stepToChild(const node& n, float elapsed, bool quickStep)
{
    if (n.leftChild == INVALID_NODE) {
        backtrack(n, elapsed, quickStep);
    } else {
        currentNode = n.leftChild;
        update(elapsed);
    }
}

template<class T>
void Bt<T>::stepPriorityNode(const node& n, float elapsed, bool quickStep)
{
    nodeId_t childId(n.leftChild);
    while (childId != INVALID_NODE) {
        const node& child(nodes[childId]);
        if (child.test(executor, elapsed)) {
            //Found a children with a passing condition
            currentNode = childId;
            currentIsTested = true;
            update(elapsed);
            return;
        } else {
            childId = child.rightBrother;
        }
    }
    // No children had passing conditions
    backtrack(n, elapsed, quickStep);
}

template<class T>
void Bt<T>::stepRandomNode(const node& n, float elapsed, bool quickStep)
{
    //Get the list of children
    std::vector<nodeId_t> children;
    unsigned totalWeight = 0;
    for(nodeId_t childId = n.leftChild;
        childId != INVALID_NODE;
        childId = nodes[childId].rightBrother) {
        totalWeight += nodes[childId].weight;
        children.push_back(childId);
    }
    if (children.empty()) {backtrack(n, elapsed, quickStep);};
    //select one
    unsigned selected = utils::die(totalWeight);
    nodeId_t node = INVALID_NODE;
    for(nodeId_t childId = n.leftChild;
        childId != INVALID_NODE && node;
        childId = nodes[childId].rightBrother) {
        unsigned weight = nodes[childId].weight;
        if (selected < weight) {
            node = childId;
            break;
        } else {
            selected -= nodes[childId].weight;
        }
    }
    currentNode = node != INVALID_NODE ? node :
        children[utils::die((unsigned int)children.size())];
    update(elapsed);
}

template<class T>
void Bt<T>::update(float elapsed)
{
    executor.update(elapsed);
    assert (currentNode != INVALID_NODE);
    assert (nodes.find(currentNode) != nodes.end());
    const node& n(nodes[currentNode]);
    if (!(currentIsTested || n.test(executor, elapsed))) {
        backtrack(n, elapsed, false);
    } else {
        currentIsTested = true;
        bool quickStep = false;
        switch (n.act(executor, elapsed)) {
            case DONE_QUICKSTEP:
                quickStep = true;
                // fall thru
            case DONE:
                currentIsTested = false;
                switch (n.type) {
                    case node::CHAIN:
                    case node::SEQUENCE_LOOP: 
                    case node::SEQUENCE_LOOPW: 
                    case node::SEQUENCE_WHILE: 
                    case node::SEQUENCE: stepToChild(n, elapsed, quickStep); break;
                    case node::PRIORITY: stepPriorityNode(n, elapsed, quickStep); break;
                    case node::RANDOM: stepRandomNode(n, elapsed, quickStep); break;
                    case node::LEAF:
                        previousActionNode = currentActionNode;
                        currentActionNode = currentNode;
                        backtrack(n, elapsed, quickStep);
                        break;
                    default: assert(utils::fatal("Malformed BT node!"));
                } break;
            case STAY:
                previousActionNode = currentActionNode;
                currentActionNode = currentNode;
            default:
                break;
        }
    }
}

template<class T>
void Bt<T>::setNodeWeight(nodeId_t id, unsigned weight)
{
    assert(nodes.find(id) != nodes.end());
    nodes[id].weight = weight;
}

template<class T>
void Bt<T>::createNode(nodeId_t id, nodeId_t parent,
    nodeType_e type, condition_t condition, action_t action)
{
    assert(nodes.find(id) == nodes.end());
    nodes.emplace(id, node(type, action, condition, parent));
}

template<class T>
void Bt<T>::createChild(nodeId_t id, nodeId_t parentId, nodeType_e type,
    condition_t condition, action_t action)
{
    assert(nodes.find(parentId) != nodes.end());
    createNode(id, parentId, type, condition, action);
    node& parent(nodes[parentId]);
    assert(parent.type != node::LEAF);
    if (parent.leftChild == INVALID_NODE) {
        //First child
        parent.leftChild = id;
    } else {
        assert(parent.type != node::CHAIN);
        //Add id to the end of the linked children list
        assert(nodes.find(parent.leftChild) != nodes.end());
        node* child = &nodes[parent.leftChild];
        while (child->rightBrother != INVALID_NODE) {
            assert(nodes.find(child->rightBrother) != nodes.end());
            child = &nodes[child->rightBrother];
        } 
        child->rightBrother = id;
    }
}

template<class T>
void Bt<T>::createRoot(nodeId_t id, nodeType_e type,
    condition_t condition, action_t action)
{
    assert(rootNode == INVALID_NODE);
    createNode(id, INVALID_NODE, type, condition, action);
    rootNode = id;
}

}

#ifdef _TEST
namespace test {
void testBT();
}
#endif

#endif

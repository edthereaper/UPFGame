#ifndef COMPONENT_SLOT_H_
#define COMPONENT_SLOT_H_

#include "handles/entity.h"

namespace component {

/**/
class CSlots {
    public:
        static const unsigned MAX_N_SLOTS = 8;

    private:
        Handle subscribed[MAX_N_SLOTS];
        unsigned used = 0;

		unsigned slotOf(const Handle& h) const {
			for (unsigned i = 0; i<used; i++) {
				if (subscribed[i] == h) { return i; }
			}
			return MAX_N_SLOTS;
		}

    public:
        unsigned nSlots = MAX_N_SLOTS;

        CSlots()=default;
        CSlots(unsigned nSlots) : nSlots(nSlots) { assert(nSlots < MAX_N_SLOTS);}

        inline void update(float) {}
        inline void init() {}

        bool subscribe(const Handle& h) {
            if (used >= nSlots || slotOf(h) < nSlots) {
                return false;
            } else {
                subscribed[used++] = h;
                return true;
            }
        
        }

		int whichSlot(const Handle& h){
			return slotOf(h);
		}

        bool unsubscribe(const Handle& h) {
            unsigned slot = slotOf(h);
            if (used == 0 || slot >= nSlots) {
                return false;
            } else {
                subscribed[slot] = subscribed[--used];
                subscribed[used] = Handle();
                return true;
            }
        }

        bool canSubscribe(const Handle& h) {
            return (used < nSlots) || (slotOf(h) < used);
        }

        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {
            nSlots = atts.getInt("n", nSlots);
            assert(nSlots < MAX_N_SLOTS);
        }

        void setNSlots(unsigned n) {
            assert(n < MAX_N_SLOTS);
            nSlots = n;
        }
};

}

#endif
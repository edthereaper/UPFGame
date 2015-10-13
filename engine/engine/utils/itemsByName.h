#ifndef UTILS_ITEMS_BY_NAME_H_
#define UTILS_ITEMS_BY_NAME_H_

namespace utils {

/* Retrieves existing object or loads them
 *
 * Duck typing for item_t:
 * - implements load(const char*)
 * - implements destroy()
 */
template < class Item_T >
class ItemsByName
{
    public:
        struct item_t{
            Item_T* item = nullptr;
            bool owned = true;

            item_t()=default;
            item_t(Item_T* item, bool owned=true) : item(item), owned(owned) {}
            operator Item_T*const()const{return item;}
        };
        typedef std::map< std::string, item_t > container_t;

    private:
        container_t items;

    public:
        inline typename container_t::iterator operator[](unsigned n) {
            auto it = items.begin();
            for(unsigned i = n; it!= items.end() && i>0; --i) ++it;
            return it;
        }
        inline typename container_t::const_iterator operator[](unsigned n) const {
            auto it = items.begin();
            for(unsigned i = n; it!= items.end() && i>0; --i) ++it;
            return it;
        }

        inline std::string getFreshName(const std::string& name) {
            unsigned n = 0;
            std::string fresh(name);
            while(has(fresh)) {
                fresh = name + '_' + std::to_string(n);
                ++n;
            }
            return fresh;
        }
        
        inline bool has(const std::string& name) {return items.find(name) != items.end();}
        inline bool has(const char* name) {return has(std::string(name));}

        inline bool destroy(Item_T* item, bool del=true) {
            for(auto& it = items.begin(); it != items.end(); it++) {
                if (it->second == item) {
                    it->second.item->destroy();
                    if (del && it->second.owned) {
                        delete it->second;
                    }
                    items.erase(it);
                    return true;
                }
            }
            return false;
        }
        
        inline std::string getNameOf(Item_T* item) const {
            for(auto& it = items.begin(); it != items.end(); it++) {
                if (it->second == item) {
                    return it->first;
                }
            }
            return "";
        }
        

        inline bool destroy(const std::string& name, bool del=true) {
            auto it = items.find(name);
            if (it != items.end()) {
                it->second.item->destroy();
                if (del && it->second.owned) {
                    delete it->second;
                }
                items.erase(it);
                return true;
            } else {
                return false;
            }
        }
        
        inline bool destroy(const char* name) {return destroy(std::string(name));}

        inline void destroyAll() {
            for (auto it : items) {
                it.second.item->destroy();
                if (it.second.owned) {
                    delete it.second;
                }
            }
            items.clear();
        }

        template< typename... Args >
        inline Item_T*const getByName(const std::string& name, Args... args) {
            auto it = items.find(name);
            if (it != items.end()) {
                return it->second;
            }

            Item_T *t = new Item_T;
            if (t->load(name.c_str(), args...)) {
                items[name].item = t;
                return t;
            } else {
                delete t;
                return nullptr;
            }
        }
        inline Item_T*const getByName(const char* name) {return getByName(std::string(name));}

        inline bool registerNew(const std::string& name, Item_T* i, bool owned = true, bool override=false) {
            auto it = items.find(name);
            if (!override && it != items.end()) {
			    return false;
            } else {
                items[name] = item_t(i, owned);
                return true;
            }
        }
        
        inline bool registerNew(const char* name, Item_T* i, bool owned = false) {
            return registerNew(std::string(name), i, owned);
        }

        template <class R_TYPE>
        void forall(R_TYPE(Item_T::*fn)(const std::string&)) const {
            for(auto& item : items) {
                (item.second.item->*fn)(item.first);
            }
        }

        template <class R_TYPE>
        void forall(const std::function<R_TYPE(Item_T& fn)>& f) const {
            for(auto& item : items) {
                f(*item.second.item);
            }
        }

};

}

#endif

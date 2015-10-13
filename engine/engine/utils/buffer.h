#ifndef UTILS_BUFFER_H_
#define UTILS_BUFFER_H_

#include "mcv_platform.h"

namespace utils {

/* Buffer containing bytes */
struct buffer_t {
    private:
        std::vector<uint8_t> bytes;
    public:
        size_t incrementPadding;
        size_t cursor; //read position
        size_t size;
    public:
        buffer_t(size_t initialAlloc = 1024, size_t incrementPadding=0) :
            bytes(initialAlloc), incrementPadding(incrementPadding),
            size(0), cursor(0) {}

        inline void accomodate(size_t nBytes) {
            if (size+nBytes > bytes.size()) {
                bytes.resize(size+nBytes+incrementPadding);
            }
        }

        template<typename Data_T>
        inline void put(const Data_T& data) {
            accomodate(sizeof(Data_T));
            memcpy(&bytes[size], &data, sizeof(Data_T));
            size += sizeof(Data_T);
        }

        inline void put(const void* data, size_t nBytes) {
            accomodate(nBytes);
            memcpy(&bytes[size], data, nBytes);
            size += nBytes;
        }
        
        /* Read from the buffer. Make sure !isPastEnd(sizeof(Data_T)))
         */
        template<typename Data_T>
        inline Data_T read() {
            Data_T* data = (Data_T*)&bytes[cursor];
            forward(sizeof(Data_T));
            return *data;
        }

        /* Read from the buffer. Make sure !isPastEnd(nBytes)
         * Ret must be able to accomodate nBytes
         */
        inline void read(uint8_t* ret, size_t nBytes) {
            uint8_t* data = &bytes[cursor];
            forward(nBytes);
            memcpy(ret, data, nBytes);
        }

        inline void rewind() {cursor=0;}
        inline void forward(size_t nBytes) {cursor += nBytes;}
        inline void seek(size_t t) {cursor=std::min(size, t);}
        inline void reset() {cursor=0; size=0;}
        inline bool isPastEnd(size_t nBytes=0) {return cursor+nBytes > size;}
        inline const uint8_t* ptr() const {return &bytes[cursor];}

};

}

#endif
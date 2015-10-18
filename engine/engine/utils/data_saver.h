#ifndef UTILS_DATA_SAVER_H_
#define UTILS_DATA_SAVER_H_

#include <sys/stat.h>
#include "mcv_platform.h"

namespace utils {

class DataSaver {
public:
    virtual ~DataSaver() { }
    virtual void write(const void *data, size_t nbytes) = 0;

    template< class TPOD >
    void writePOD(const TPOD& pod) {
        write(&pod, sizeof(TPOD));
    }
};

class MemoryDataSaver : public DataSaver {
    public:
        static void ensureDirectoryExists(const char* path) {
            std::string s(path);
            std::replace(s.begin(), s.end(), '\\', '/');
            s = s.substr(0, s.find_last_of('/'));
            //Returns safely if directory already exists (GetLastError() == ERROR_ALREADY_EXISTS)
            CreateDirectory(s.c_str(), NULL);
        }

    private:
        typedef unsigned char u8;
        typedef std::vector<u8> buffer_t;
        buffer_t buffer;

    public:
        void write(const void *data, size_t nbytes) override {
            size_t old_size = buffer.size();
            buffer.resize(old_size + nbytes);
            memcpy(&buffer[old_size], data, nbytes);
        }

        template<class Writer_T, typename RET_T, typename NBYTES_T>
        NBYTES_T writeFromObject(Writer_T& writer,
            RET_T(Writer_T::*read)(void*, NBYTES_T), NBYTES_T nBytes) {
            size_t old_size = buffer.size();
            buffer.resize(old_size + nBytes);
            return writer.read(&buffer[old_size], nBytes);
        }

        inline size_t size() const { return buffer.size(); }
        inline const void* data() const { return buffer.data(); }

        bool saveToFile(const char* filename) {
            ensureDirectoryExists(filename);
            FILE *f = fopen(filename, "wb");
            if (f == nullptr) {
                return false;
            }
            size_t bytes_saved = fwrite(data(), 1, size(), f);
            assert(bytes_saved == size());
            fclose(f);
            return true;
        }
};

template< class TPOD >
void saveChunk(MemoryDataSaver& mds, unsigned chunk_type, const std::vector <TPOD> &vdata)
{   
    struct TChunkHeader {
        unsigned magic;
        unsigned nbytes;
    } chunk;
    
    chunk.magic = chunk_type;
    chunk.nbytes = static_cast< unsigned > ( vdata.size() * sizeof(TPOD) );
    mds.writePOD(chunk);
    mds.write(&vdata[0], chunk.nbytes);
}

}

#endif

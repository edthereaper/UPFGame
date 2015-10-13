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
        size_t size() const { return buffer.size(); }
        const void* data() const { return &buffer[0]; }

        bool saveToFile(const char* filename) {
            FILE *f = fopen(filename, "wb");
            if (!f)
              return false;
            size_t bytes_saved = fwrite(data(), 1, size(), f);
            assert(bytes_saved == size());
            fclose(f);
            return true;
        }
};

template< class TPOD >
void saveChunk(MemoryDataSaver& mds, unsigned chunk_type, const std::vector <TPOD> &vdata) {

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

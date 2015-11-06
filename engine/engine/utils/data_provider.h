#ifndef INC_DATA_PROVIDER_H_
#define INC_DATA_PROVIDER_H_

#include <sys/stat.h>
#include <windows.h>

namespace utils {

class DataProvider {
public:
  virtual ~DataProvider() { }
  virtual const char* getName() const = 0;
  virtual void read(void *data, size_t nbytes) = 0;
  virtual bool isValid() const = 0;

  template< class TPOD >
  void read(TPOD& pod) {
    read(&pod, sizeof(TPOD));
  }
  template< class TPOD >
  void readVal(TPOD& pod) {
    read(&pod, sizeof(TPOD));
  }
};

class FileDataProvider : public DataProvider {
    private:
        FILE *f;
        char filename[MAX_PATH];

    public:
        FileDataProvider(const char* afilename) {
            strcpy(filename, afilename);
            f = fopen(filename, "rb");
        }

        FileDataProvider(const std::string& str) {
            strcpy(filename, str.c_str());
            f = fopen(filename, "rb");
        }

        ~FileDataProvider() {
            if (f){fclose(f);}
        }

        const char* getName() const {
            return filename;
        }

        void read(void *data, size_t nbytes) {
            assert(isValid());
            auto bytes_read = fread(data, 1, nbytes, f);
            assert(bytes_read == nbytes);
        }

        bool isValid() const { return f != nullptr; }

        size_t getFileSize() const {
            struct __stat64 buffer;
            int rc = _stat64(filename, &buffer);
            return buffer.st_size;
        }
};

class DirLister {
    public:
        typedef std::vector<std::string> vector_t;
    public:
        static bool isDir(std::string path) {
            struct stat s;
            if(!stat(path.c_str(), &s)) {
                return (s.st_mode & S_IFDIR) != 0;
            } else {return false;}
        }

        static vector_t listDir(std::string path) {
            HANDLE hFind;
            WIN32_FIND_DATA data;
            vector_t ret;

            std::string dPath = path + "\\*";
            hFind = FindFirstFile(dPath.c_str(), &data);
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    if(!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        ret.push_back(data.cFileName);
                    }
                } while (FindNextFile(hFind, &data));
                FindClose(hFind);
            }
            return ret;
        }

};

}
#endif

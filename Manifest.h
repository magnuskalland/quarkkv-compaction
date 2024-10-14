#include "config.h"

class Manifest {
   public:
    Manifest(std::string filename);
    virtual int open() = 0;
    virtual int persist() = 0;

   protected:
    int parse(void* buf);
    int serialize(void* src, void* dest);

    std::string filename_;

    int nSstFiles[LEVELS];
    char* sstFilenames[LEVELS];

    struct DiskManifest {
        int nSstFiles[LEVELS];
        char* sstFilenames[LEVELS];
    } diskManifest;
};
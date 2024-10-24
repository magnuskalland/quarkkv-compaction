#include "../include/Manifest.h"

class ManifestFS : public Manifest {
   public:
    ManifestFS(Config* config) : Manifest(config) {}
    int Open() override;
    int Persist() override;
    std::string ToString() override;

   private:
    int openCurrent() override;

    /**
     * @return -1 on IO error, 0 if no error and CURRENT was read, 1 if CURRENT was empty
     * and we created a new manifest file.
     */
    int getCurrentManifestPath(char* dest);
    int createNewManifest(char* dest);
};
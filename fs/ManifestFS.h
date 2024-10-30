#include "../include/Manifest.h"

class ManifestFS : public Manifest {
   public:
    ManifestFS(Config* config) : Manifest(config, 0) {}
    int Open() override;
    int Persist() override;
    std::string ToString() override;

   protected:
   private:
    int openCurrent() override;
    int createNewManifest(char* dest);

    /**
     * @return -1 on IO error, 0 if no error and CURRENT was read, 1 if CURRENT was empty
     * and we created a new manifest file.
     */
    int getCurrentManifestPath(char* dest);
};
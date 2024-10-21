#include "../include/Manifest.h"

class ManifestFS : public Manifest {
   public:
    ManifestFS(Config* config) : Manifest(config) {}
    int Open() override;
    int Persist() override;
    std::string ToString() override;

   private:
    int openCurrent() override;
    int getCurrentManifestPath(char* dest);
    int createNewManifest(char* dest);
};
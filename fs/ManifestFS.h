#pragma once

#include "../include/Manifest.h"

class ManifestFS : public Manifest {
   public:
    ManifestFS(Config* config);
    ~ManifestFS() override;
    int Open() override;
    int Populate(uint32_t n) override;

   protected:
    int persist() override;
    int createManifest(int id) override;
    std::string getFullPath(std::string name);
};
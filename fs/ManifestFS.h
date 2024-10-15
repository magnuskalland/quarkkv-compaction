#pragma once

#include "../include/Manifest.h"

class ManifestFS : public Manifest {
   public:
    ManifestFS(Config* config);
    ~ManifestFS() override;
    int Open() override;
    int Persist() override;

   protected:
    std::string getFullPath(std::string name);
};
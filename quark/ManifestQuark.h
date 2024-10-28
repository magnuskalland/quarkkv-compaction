#pragma once

#include "../include/Manifest.h"

class ManifestQuark : public Manifest {
   public:
    ManifestQuark(Config* config);
    int Open() override;
    int Persist() override;
    std::string ToString() override;

   protected:
    int openCurrent() override;

   private:
    int getCurrentManifest();
};
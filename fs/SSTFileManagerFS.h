#pragma once

#include "../include/SSTFileManager.h"

class SSTFileManagerFS : public SSTFileManager {
   public:
    SSTFileManagerFS(Config* config);
    ~SSTFileManagerFS() override;
    int CreateNewSST(uint32_t id) override;
    int PopulateSST(uint32_t id) override;
};
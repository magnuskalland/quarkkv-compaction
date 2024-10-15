#include "../include/SST.h"

class SSTFS : public SST {
   public:
    static std::shared_ptr<SST> CreateNewEmpty(Config* config, uint32_t id);
    static std::unique_ptr<SST> OpenWithID(uint32_t id);
    int Persist() override;

    int Append(std::string key);

   protected:
    SSTFS(Config* config, uint32_t handler, int id, std::string full_name_);
    int writeIndexBlock() override;
    int writeNumberOfEntries() override;

   private:
    std::string full_name_;
};
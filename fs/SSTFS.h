#include "../include/SST.h"

class SSTFS : public SST {
   public:
    static std::shared_ptr<SST> CreateNewEmpty(Config* config, uint32_t id);
    static std::shared_ptr<SST> OpenWithID(Config* config, uint32_t id);
    int Persist() override;
    int Parse() override;
    int Close() override;
    int Get(std::string key, KVPair** dest) override;

    int Append(std::string key);

   protected:
    SSTFS(Config* config, uint32_t handler, int id, std::string full_name_);
    int writeIndexBlock() override;
    int writeNumberOfEntries() override;
    int readIndexBlock() override;
    int readNumberOfEntries() override;
    int readKVAtOffset(uint64_t off, KVPair** kv_pair) override;

   private:
    std::string full_name_;
};
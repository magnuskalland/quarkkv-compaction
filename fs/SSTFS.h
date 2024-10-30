#include "../include/SST.h"

class SSTFS : public SST {
   public:
    ~SSTFS();
    static std::shared_ptr<SST> CreateNewEmpty(Config* config, uint32_t id);
    static std::shared_ptr<SST> OpenWithID(Config* config, uint32_t id);
    int Remove() override;

   protected:
    SSTFS(Config* config, uint32_t handler, int id, std::string full_name_);
    // bool verifyPersisted() override;
    int append(char* buf, size_t size) override;
    int read(char* buf, size_t size, off_t offset) override;

   private:
    std::string fullName_;
};
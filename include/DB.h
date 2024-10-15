
#include <set>

#include "ManifestFS.h"
#include "config.h"

class DB {
   public:
    DB(Config* config);
    ~DB();
    int Open();
    int Close();
    int Get(std::string key);
    int Put(std::string key, std::string _);

    void VerifyConfig();
    int Populate(int n);

   private:
    Manifest* manifest_;
    SSTManager* manager_;
    Config* config_;
    std::vector<std::shared_ptr<SST>> ssts_;
};
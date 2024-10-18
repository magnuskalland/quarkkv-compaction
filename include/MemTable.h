
#include <map>
#include <string>

class MemTable {
   public:
    MemTable(size_t limit);
    std::string* Get(std::string key);
    void Put(std::string key, std::string val);
    bool Full();
    std::map<std::string, std::string>* GetTable();
    void Clear();

   private:
    std::map<std::string, std::string> table_;
    size_t limit_;
};
#include <cstddef>

class SortedStringTable {
   public:
    virtual int Open();

   protected:
    size_t size_;
    int handler_;

   private:
};
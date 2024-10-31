#include "SSTQuark.h"

#include <algorithm>
#include <cctype>
#include <cstring>

#include "io_quark.h"

SSTQuark::SSTQuark(Config* config, uint32_t handler, int aid) : SST(config, handler, aid)
{
}
SSTQuark::~SSTQuark()
{
    // ::AtomRelease(handler_);
}

std::shared_ptr<SST> SSTQuark::CreateNewEmpty(Config* config, uint64_t* aid)
{
    int ah, ok;
    ah = ::AtomGet(aid);
    if (ah < 0) {
        return nullptr;
    }
    std::shared_ptr<SST> sst(new SSTQuark(config, ah, *aid));
    return sst;
}

std::shared_ptr<SST> SSTQuark::OpenWithID(Config* config, uint64_t* aid)
{
    int ah, ok;
    ah = ::AtomGet(aid);
    if (ah < 0) {
        return nullptr;
    }
    std::shared_ptr<SST> sst(new SSTQuark(config, ah, *aid));
    ok = sst->Parse();
    if (ok == -1) {
        return nullptr;
    }
    return sst;
}

int SSTQuark::Remove()
{
    int ok;

    ok = ::AtomRelease(handler_);
    if (ok < 0) {
        return -1;
    }

    ok = ::AtomRemove(id_);
    if (ok < 0) {
        return -1;
    }

    return 0;
}

int SSTQuark::append(char* buf, size_t size)
{
    int ok;

    ok = ::AtomWrite(handler_, buf, size, atomSize_);
    if (ok < 0) {
        return -1;
    }

    atomSize_ = atomSize_ + size;
    return ok;
}

int SSTQuark::read(char* buf, size_t size, off_t offset)
{
    int ok;

    ok = ::AtomRead(handler_, buf, size, offset);
    if (ok < 0) {
        return -1;
    }

    return ok;
}

int SSTQuark::MoveAndAddKV(KVPair* kv)
{
    int ok;
    ok =
        ::AtomMoveAppend(handler_, kv->GetHandler(), config_->kv_size(), kv->GetOffset());
    if (ok == -1) {
        return -1;
    }

    if (entries_ == 0) {
        smallestKey_ = kv->GetKey();
    }

    indexTable_.insert({kv->GetKey(), entries_ * config_->kv_size()});
    entries_ = entries_ + 1;
    atomSize_ = atomSize_ + config_->kv_size();

    return 0;
}
#include "ManifestQuark.h"

#include "io_quark.h"

ManifestQuark::ManifestQuark(Config* config) : Manifest(config) {}

int ManifestQuark::Open()
{
    int ok;
    char buf[BLOCK_SIZE];

    ok = getCurrentManifest();
    if (ok == -1) {
        return -1;
    }

    return 0;
}

int ManifestQuark::Persist()
{
    return 0;
}

std::string ManifestQuark::ToString()
{
    return "";
}

int ManifestQuark::openCurrent()
{
    return 0;
}

int ManifestQuark::getCurrentManifest()
{
    int ok;
    char buf[BLOCK_SIZE];

    current_ = ::GetAtom(config_->quark_current_id);
    if (current_ == -1) {
        return -1;
    }
    return 0;
}
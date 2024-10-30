#include "ManifestQuark.h"

#include "io_quark.h"

ManifestQuark::ManifestQuark(Config* config) : Manifest(config) {}

int ManifestQuark::Open()
{
    int ok, manifest_aid, manifest_ah;

    ok = ::Init();
    if (ok == -1) {
        return -1;
    }

    ok = getCurrentManifest();
    if (ok == -1) {
        return -1;
    }

    manifest_aid = ok;
    printf("Got manifest ID: %d\n", manifest_aid);

    ok = AtomGet(manifest_aid);
    if (ok == -1) {
        return -1;
    }

    char buf[BLOCK_SIZE];
    manifest_ah = ok;

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
    int manifest;
    char buf[BLOCK_SIZE];

    current_ = ::AtomGet(config_->quarkstore_current_aid);
    if (current_ == -1) {
        return -1;
    }

    ok = ::AtomRead(current_, buf, BLOCK_SIZE, 0);

    // read went wrong
    if (ok == -1) {
        return -1;
    }

    // read 0 bytes, create new manifest?
    if (ok == 0) {
    }

    memcpy((void*)&manifest, buf, sizeof(manifest));
    return manifest;
}
#include "ManifestQuark.h"

#include "io_quark.h"

ManifestQuark::ManifestQuark(Config* config)
    : Manifest(config, config->quarkstore_manifest_aid_start)
{
}

int ManifestQuark::Open()
{
    int ok, manifest_aid, manifest_ah;

    ok = ::Init();
    if (ok == -1) {
        return -1;
    }

    ok = openCurrent();
    if (ok == -1) {
        return -1;
    }

    ok = getCurrentManifest();

    // I/O error
    if (ok == -1) {
        return -1;
    }

    // created new manifest, nothing to parse
    if (ok == 0) {
        return 0;
    }

    printf("Got manifest ID: %d\n", handler_);

    char buf[BLOCK_SIZE];
    ok = ::AtomRead(handler_, buf, BLOCK_SIZE, 0);
    if (ok < 0) {
        return -1;
    }

    deserializeLevels(buf, BLOCK_SIZE);
    return 0;
}

int ManifestQuark::Persist()
{
    int ok, aid;

    aid = createNewManifest();
    if (aid < 0) {
        return -1;
    }

    char buf[BLOCK_SIZE];
    serializeLevels(buf);

    ok = ::AtomWrite(handler_, buf, BLOCK_SIZE, 0);
    if (ok < 0) {
        return -1;
    }

    ok = openCurrent();
    if (ok == -1) {
        fprintf(stderr, "failed to open current\n");
        return -1;
    }

    std::memset(buf, 0, BLOCK_SIZE);
    std::memcpy(buf, &aid, sizeof(aid));
    return 0;
}

std::string ManifestQuark::ToString()
{
    std::ostringstream oss;

    oss << "Manifest: " << handler_ << "\n";

    return oss.str();
}

int ManifestQuark::openCurrent()
{
    current_ = ::AtomGet(config_->quarkstore_current_aid);
    if (current_ == -1) {
        return -1;
    }
    return 0;
}

int ManifestQuark::getCurrentManifest()
{
    int ok;
    int manifest;
    char buf[BLOCK_SIZE];

    ok = ::AtomRead(current_, buf, BLOCK_SIZE, 0);

    // read failed, create new manifest
    if (ok == -2) {
        return 1;
    }
    else if (ok < 0) {
        ok = createNewManifest();
        if (ok == -1) {
            return -1;
        }
        return 0;
    }

    memcpy((void*)&manifest, buf, sizeof(manifest));
    return manifest;
}

int ManifestQuark::createNewManifest()
{
    int id, ah;
    id = ctr_.fetch_add(1);
    ah = ::AtomGet(id);
    if (ah < 0) {
        return -1;
    }
    handler_ = ah;
    return id;
}
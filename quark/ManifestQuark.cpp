#include "ManifestQuark.h"

#include "io_quark.h"

ManifestQuark::ManifestQuark(Config* config)
    : Manifest(config, config->quarkstore_manifest_aid_start)
{
    current_ = -1;
    handler_ = -1;
}

ManifestQuark::~ManifestQuark()
{
    if (current_ != -1) {
        ::AtomRelease(current_);
    }
    if (handler_ != -1) {
        ::AtomRelease(handler_);
    }
}

int ManifestQuark::Open()
{
    int ok, aid, ah;

    ok = ::Init();
    if (ok == -1) {
        return -1;
    }
    printf("Initialized QuarkStore\n");

    ok = openCurrent();
    if (ok == -1) {
        return -1;
    }

    aid = getCurrentManifest();

    // created new manifest, nothing to parse
    if (aid == 0) {
        return 0;
    }

    printf("Returning without parsing persisted SSTs\n");
    return 0;

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
    int ok, manifest_aid;
    char *buf;

    manifest_aid = createNewManifest();
    if (manifest_aid < 0) {
        return -1;
    }

    ok = posix_memalign((void**) &buf, BLOCK_SIZE, BLOCK_SIZE);
    serializeLevels(buf);

    ok = ::AtomWrite(handler_, buf, BLOCK_SIZE, 0);
    if (ok < 0) {
        free(buf);
        return -1;
    }

    std::memset(buf, 0, BLOCK_SIZE);
    std::memcpy(buf, &manifest_aid, sizeof(manifest_aid));

    ok = ::AtomWrite(current_, buf, BLOCK_SIZE, 0);
    free(buf);
    return ok;
}

std::string ManifestQuark::ToString()
{
    std::ostringstream oss;

    oss << "Manifest: " << handler_ << "\n";

    return oss.str();
}

int ManifestQuark::openCurrent()
{
    int ok;
    ok = ::AtomGet(&config_->quarkstore_current_aid);
    if (ok == -1) {
        return -1;
    }
    current_ = ok;
    return 0;
}

int ManifestQuark::getCurrentManifest()
{
    int ok;
    uint64_t aid;
    char buf[BLOCK_SIZE];

    ok = ::AtomRead(current_, buf, BLOCK_SIZE, 0);

    // no manifest exists or read failed
    if (ok != BLOCK_SIZE) {
        return 0;
    }

    memcpy((void*)&aid, buf, sizeof(aid));
    ctr_.store(aid + 1);

    ok = ::AtomGet(&aid);
    if (ok < 0) {
        return -1;
    }
    handler_ = ok;
    return aid;
}

int ManifestQuark::createNewManifest()
{
    uint64_t id;
    int ah;
    id = ctr_.fetch_add(1);
    ah = ::AtomGet(&id);
    if (ah < 0) {
        return -1;
    }
    handler_ = ah;
    return id;
}
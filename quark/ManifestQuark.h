#pragma once

#include "../include/Manifest.h"

class ManifestQuark : public Manifest {
   public:
    ManifestQuark(Config* config);
    ~ManifestQuark();

    /**
     * Initializes QuarkStore, creates a handler
     * for CURRENT. If a manifest exists, gets
     * current manifest ID and creates a handler for
     * it, and parses its content.
     */
    int Open() override;

    /**
     *
     */
    int Persist() override;
    std::string ToString() override;

   protected:
    /**
     * Creates the atom handler for CURRENT.
     */
    int openCurrent() override;

   private:
    /**
     * REQUIRES: current_ must be initialized.
     * Reads the last manifest ID from the CURRENT,
     * then creates an atom handler of the ID and
     * assigns that to handler_.
     * @returns -1 on error, otherwise the new
     * manifest ID.
     */
    int getCurrentManifest();

    /**
     * REQUIRES: ctr_ must be initialized.
     * Generates a new manifest atom ID from ctr_ and
     * gets the atom handler from that ID.
     * @returns -1 on error, otherwise the ID of the new
     * manifest;
     */
    int createNewManifest();
};
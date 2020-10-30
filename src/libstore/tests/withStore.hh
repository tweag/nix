#pragma once

#include "store-api.hh"
#include "util.hh"
#include <gtest/gtest.h>
#include <filesystem>


using namespace nix;

namespace nix::test {

// A RAII-style class holding the path to a temporary directory that will
// be deleted once this goes out of scope
struct TmpDir {
    const Path path;

    TmpDir()
        : path(createTempDir("", "nix-test"))
    { }

    ~TmpDir() {
        deletePath(path);
    }
};

class StoreFixture : public ::testing::Test {
private:
    const TmpDir tmpDir_;

    virtual void fillWithSampleData(const Path & root) {
        createDirs(root);
        writeFile(root + "/file", sampleContent);
    }

protected:
    const Path storeUrl;
    const ref<Store> store;
    const Path tmpDir;
    const Path originalDirectory;

    const std::string sampleContent = "sample content";
    const Path sampleInputDirectory;

public:
    StoreFixture(const std::string & storeUrl = "/tmp/nix-test/default")
        : Test()
        , storeUrl(storeUrl)
        , store(openStore(storeUrl))
        , tmpDir(tmpDir_.path)
        , originalDirectory(std::filesystem::current_path())
        , sampleInputDirectory(tmpDir + "/sample-input")
    {
        std::filesystem::current_path(tmpDir);
        fillWithSampleData(sampleInputDirectory);
    }

    ~StoreFixture() {
        std::filesystem::current_path(originalDirectory);
        deletePath(storeUrl);
    }
};
}

#include "withStore.hh"

using namespace nix;

namespace nix::test {

TEST_F(StoreFixture, OpensFine) {
}

TEST_F(StoreFixture, AddToStore) {
    auto path1 = store->addTextToStore("fixed", sampleContent, {});
    EXPECT_TRUE(store->isValidPath(path1));
    EXPECT_EQ(sampleContent, readFile(store->toRealPath(store->printStorePath(path1))));

    auto path2 = store->addToStore("fixed2", sampleInputDirectory);
    EXPECT_EQ(sampleContent, readFile(store->toRealPath(store->printStorePath(path2)) + "/file"));

    auto path3 = store->addToStore("fixed2", store->toRealPath(store->printStorePath(path2)));
    EXPECT_EQ(path2, path3);

    auto path4 = ([&]() -> StorePath {
        auto path3Content = sinkToSource([&](Sink & sink) {
            store->narFromPath(path3, sink);
        });
        return store->addToStoreFromDump(*path3Content, "fixed2");
    })();
    EXPECT_EQ(path2, path4);
}

}

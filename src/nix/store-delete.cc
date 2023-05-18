#include "command.hh"
#include "common-args.hh"
#include "shared.hh"
#include "store-api.hh"
#include "store-cast.hh"
#include "gc-store.hh"

using namespace nix;

struct CmdStoreDelete : StorePathsCommand
{
    GCDelete deleteOpts{.pathsToDelete = GCPathsToDelete{}};

    CmdStoreDelete()
    {
        addFlag({
            .longName = "ignore-liveness",
            .description = "Delete all provided paths, even if they are alive (reachable from a garbage collection root).",
            .handler = {&deleteOpts.ignoreLiveness, true}
        });
        addFlag({
            .longName = "skip-alive",
            .shortName = 's',
            .description = "Skip paths that are still alive (referenced by a garbage collection root), instead of exiting with a non-zero exit code.",
            .handler = {&deleteOpts.pathsToDelete->skipAlive, true}
        });
    }

    std::string description() override
    {
        return "delete paths from the Nix store";
    }

    std::string doc() override
    {
        return
          #include "store-delete.md"
          ;
    }

    void run(ref<Store> store, StorePaths && storePaths) override
    {
        auto & gcStore = require<GcStore>(*store);

        for (auto & path : storePaths)
            deleteOpts.pathsToDelete->paths.insert(path);


        GCOptions options {GCAction{deleteOpts}};
        GCResults results;
        PrintFreed freed(true, results);
        gcStore.collectGarbage(options, results);
    }
};

static auto rCmdStoreDelete = registerCommand2<CmdStoreDelete>({"store", "delete"});

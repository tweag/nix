#include "shared.hh"
#include "eval-inline.hh"
#include "eval.hh"
#include "attr-path.hh"
#include "get-drvs.hh"
#include "withStore.hh"

using namespace nix;

namespace nix::test {

DrvInfos parseFileToDrvs(EvalState evalState, const Path srcFile) {
    auto nixExpr = evalState.parseExprFromFile(srcFile);
    Value rootVal;
    evalState.eval(nixExpr, rootVal);
    Bindings * autoArgs = evalState.allocBindings(0);
    Value& nixVal(*findAlongAttrPath(evalState, "", *autoArgs, rootVal).first);
    evalState.forceValue(nixVal);
    DrvInfos ret;
    getDerivations(evalState, nixVal, "", *autoArgs, ret, false);
    return ret;
}

struct ContentAddressed : StoreFixture {
    OnStartup<std::function<void()>> _foo;
    const Path nixFile;
    EvalState evalState;
    DrvInfos CADrvs;

    ContentAddressed()
        : _foo([]() { initNix(); initGC(); initPlugins(); })
        , nixFile(originalDirectory + "/tests/content-addressed.nix")
        , evalState({}, store)
        , CADrvs(parseFileToDrvs(evalState, nixFile))
        { }
};

#define CATEST(...) TEST_F(ContentAddressed, __VA_ARGS__)

CATEST(SimpleBuild) {
    auto drvPath = store->parseStorePath(CADrvs.begin()->queryDrvPath());
    auto outputName = CADrvs.begin()->queryOutputName();
    store->buildPaths({{drvPath, {outputName}}});
}

}

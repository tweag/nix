#include "drv-output-info.hh"
#include "store-api.hh"

namespace nix {

MakeError(InvalidDerivationOutputId, Error);

DrvOutputId DrvOutputId::parse(const std::string &strRep) {
    const auto &[rawPath, outputs] = parsePathWithOutputs(strRep);
    if (outputs.size() != 1)
        throw InvalidDerivationOutputId("Invalid derivation output id %s", strRep);

    return DrvOutputId{
        .drvPath = StorePath(rawPath),
        .outputName = *outputs.begin(),
    };
}

std::string DrvOutputId::to_string() const {
    return std::string(drvPath.to_string()) + "!" + outputName;
}

DrvInput DrvInput::parse(const std::string & strRep)
{
    try {
        return DrvInput(DrvOutputId::parse(strRep));
    } catch (InvalidDerivationOutputId) {
        return DrvInput(StorePath(strRep));
    }
}

std::string DrvInput::to_string() const {
    return std::visit(
        overloaded{
            [&](StorePath p) -> std::string { return std::string(p.to_string()); },
            [&](DrvOutputId id) -> std::string { return id.to_string(); },
        },
        static_cast<RawDrvInput>(*this));
}

std::string DrvOutputInfo::to_string() const {
    std::string res;

    res += "Deriver: " + std::string(id.to_string()) + '\n';
    res += "OutPath: " + std::string(outPath.to_string()) + '\n';
    std::set<std::string> rawDependencies;
    if (!dependencies.empty()) {
        for (auto & dep : dependencies)
            rawDependencies.insert(dep.to_string());
        res += "Dependencies: " + concatStringsSep(" ", rawDependencies) + '\n';
    }
    for (auto sig : signatures)
        res += "Sig: " + sig + "\n";

    return res;
}

DrvOutputInfo DrvOutputInfo::parse(const std::string & s, const std::string & whence)
{
    // XXX: Copy-pasted from NarInfo::NarInfo. Should be factored out
    auto corrupt = [&]() {
        return Error("Drv output info file '%1%' is corrupt", whence);
    };

    std::optional<StorePath> outPath;
    std::optional<DrvOutputId> id;
    std::set<DrvInput> dependencies;
    StringSet signatures;

    size_t pos = 0;
    while (pos < s.size()) {

        size_t colon = s.find(':', pos);
        if (colon == std::string::npos) throw corrupt();

        std::string name(s, pos, colon - pos);

        size_t eol = s.find('\n', colon + 2);
        if (eol == std::string::npos) throw corrupt();

        std::string value(s, colon + 2, eol - colon - 2);

        if (name == "Deriver")
            id = DrvOutputId::parse(value);

        if (name == "OutPath")
            outPath = StorePath(value);

        if (name == "Dependencies")
            for (auto& rawDep : tokenizeString<Strings>(value, " "))
                dependencies.insert(DrvInput::parse(rawDep));

        if (name == "Sig")
            signatures.insert(value);

        pos = eol + 1;
    }

    if (!outPath) corrupt();
    if (!id) corrupt();
    return DrvOutputInfo {
        .id = *id,
        .outPath = *outPath,
        .dependencies = dependencies,
        .signatures = signatures,
    };
}

std::string DrvOutputInfo::fingerprint() const {
    std::list<std::string> strDeps;
    for (auto& dep : dependencies)
        strDeps.push_front(dep.to_string());
    return "1;" + std::string(outPath.to_string()) + ";" +
           concatStringsSep(",", strDeps) + ";" +
           id.to_string();
}

void DrvOutputInfo::sign(Store& _store, const SecretKey& secretKey) {
    signatures.insert(secretKey.signDetached(fingerprint()));
}

bool DrvOutputInfo::checkSignature(const PublicKeys& publicKeys, const std::string& sig) const
{
    return verifyDetached(fingerprint(), sig, publicKeys);
}

size_t DrvOutputInfo::checkSignatures(const PublicKeys & publicKeys) const
{
    size_t good = 0;
    for (auto & sig : signatures)
        if (checkSignature(publicKeys, sig))
            good++;
    return good;
}

} // namespace nix

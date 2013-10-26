/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "ArchitectureRepository.h"

#include <cassert>

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/arch/intel/IntelArchitecture.h>

#include "Architecture.h"

namespace nc { namespace core { namespace arch {

ArchitectureRepository *ArchitectureRepository::instance() {
    static auto repository = []() -> ArchitectureRepository * {
        using nc::arch::intel::IntelArchitecture;

        static ArchitectureRepository result;
        result.registerArchitecture(std::make_unique<IntelArchitecture>(IntelArchitecture::REAL_MODE));
        result.registerArchitecture(std::make_unique<IntelArchitecture>(IntelArchitecture::PROTECTED_MODE));
        result.registerArchitecture(std::make_unique<IntelArchitecture>(IntelArchitecture::LONG_MODE));
        return &result;
    }();

    return repository;
}

void ArchitectureRepository::registerArchitecture(std::unique_ptr<Architecture> architecture) {
    assert(architecture != NULL);
    assert(!getArchitecture(architecture->name()) && "Cannot register two architectures with the same name.");

    architectures_.push_back(std::move(architecture));
}

const Architecture *ArchitectureRepository::getArchitecture(const QString &name) const {
    foreach (auto architecture, architectures()) {
        if (architecture->name() == name) {
            return architecture;
        }
    }
    return NULL;
}

const std::vector<const Architecture *> &ArchitectureRepository::architectures() const {
    return reinterpret_cast<const std::vector<const Architecture *> &>(architectures_);
}

}}} // namespace nc::core::arch

/* vim:set et sts=4 sw=4: */

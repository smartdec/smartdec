/* No include guards. */

/**
 * \file
 *
 * This file is to be used inside a constructor of mnemonics container. 
 * It generates the necessary boilerplate to fill the container.
 * For example usage, see <tt>IntelMnemonics.cpp</tt>.
 */

/* Including this one in the middle of a function is OK. */
#include <boost/preprocessor/stringize.hpp>

#ifndef INSTRUCTION_TABLE
#    error Define INSTRUCTION_TABLE before including this file
#endif

#ifndef MNEMONICS_NAMESPACE
#    error Define MNEMONICS_NAMESPACE before including this file
#endif

#ifdef INSTR
#    error INSTR is expected not to be defined
#endif

{
    struct MnemonicDescription {
        int mnemonic;
        const char *name;
        const char *description;
    };

    static const MnemonicDescription mnemonicDescriptions[] = {
#define INSTR(lowercase, uppercase, description)                                        \
    { MNEMONICS_NAMESPACE::uppercase, BOOST_PP_STRINGIZE(lowercase), description },
#include INSTRUCTION_TABLE
#undef INSTR
    };

    for(unsigned i = 0; i < sizeof(mnemonicDescriptions) / sizeof(MnemonicDescription); i++) {
        registerMnemonic(
            new nc::core::arch::Mnemonic(
                mnemonicDescriptions[i].mnemonic,
                QLatin1String(mnemonicDescriptions[i].name),
                QLatin1String(mnemonicDescriptions[i].description)
            )
        );
    }
}

#undef MNEMONICS_NAMESPACE
#undef INSTRUCTION_TABLE

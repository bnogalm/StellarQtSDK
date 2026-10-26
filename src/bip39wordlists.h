#ifndef BIP39WORDLISTS_H
#define BIP39WORDLISTS_H

#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * The 2048 words (UTF-8, NFKD, in index order) of a BIP-39 list other than
 * English, by Util::SupportedMnemonicLanguages value (1 = JA ... 7 =
 * ZH_HANT); nullptr for any other. Internal: use Util::mnemonicWords().
 */
const char* const* bip39Wordlist(int language);

QSTELLAR_END_NS

#endif // BIP39WORDLISTS_H

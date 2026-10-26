#ifndef SEP29_H
#define SEP29_H

#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * SEP-29 — an account that needs a memo on incoming payments says so with a
 * data entry: `config.memo_required` = "1". Horizon returns data values in
 * base64, so the value read from an AccountResponse is "MQ==".
 * Server::submitTransaction and Server::checkMemoRequired look for it; an
 * account sets it with a ManageDataOperation(DATA_NAME, "1").
 */
struct Sep29
{
    static constexpr const char* DATA_NAME = "config.memo_required";
    static constexpr const char* DATA_VALUE = "1";
    static constexpr const char* DATA_VALUE_BASE64 = "MQ==";
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Sep29)

#endif // SEP29_H

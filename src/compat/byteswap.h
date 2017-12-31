// Copyright (c) 2014-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_COMPAT_BYTESWAP_H
#define BITCOIN_COMPAT_BYTESWAP_H

#include "clientversion.h"

#include <stdint.h>

#if defined(HAVE_BYTESWAP_H)
#include <byteswap.h>
#endif

#if defined(__APPLE__)

#if !defined(bswap_16)

// Mac OS X / Darwin features; we include a check for bswap_16 because if it is already defined, protobuf has
// defined these macros for us already; if it isn't, we do it ourselves. In either case, we get the exact same
// result regardless which path was taken
#include <libkern/OSByteOrder.h>
#define bswap_16(x) OSSwapInt16(x)
#define bswap_32(x) OSSwapInt32(x)
#define bswap_64(x) OSSwapInt64(x)

#endif // !defined(bswap_16)

#else
// Non-Mac OS X / non-Darwin



#if HAVE_DECL_BSWAP_32 == 0
inline uint32_t bswap_32(uint32_t x)
{
    return (((x & 0xff000000U) >> 24) | ((x & 0x00ff0000U) >>  8) |
            ((x & 0x0000ff00U) <<  8) | ((x & 0x000000ffU) << 24));
}
#endif // HAVE_DECL_BSWAP32

#endif // defined(__APPLE__)

#endif // BITCOIN_COMPAT_BYTESWAP_H

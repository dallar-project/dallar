// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CLIENTVERSION_H
#define BITCOIN_CLIENTVERSION_H

#define UCFG_DEFINE_NDEBUG 0

#define UCFG_GRS_FAST 1	//!!!T

#define CLIENT_VERSION_MAJOR 1
#define CLIENT_VERSION_MINOR 2
#define CLIENT_VERSION_REVISION 0
#define CLIENT_VERSION_BUILD 0

#define BUILD_DESC "v1.2.0"

#define COPYRIGHT_HOLDERS "The %s developers"
#define COPYRIGHT_HOLDERS_FINAL "The Dallar Core developers"
#define COPYRIGHT_HOLDERS_SUBSTITUTION "Dallar Core"

#define COPYRIGHT_YEAR 2017

#if  !defined(RC_COMPILER) && defined(_MSC_VER)
#include <vc-inc.h>
#endif

// Apparently RC Compiling for QT on MSVC doesn't define _MSC_VER?
#if !defined(_MSC_VER) && !defined(QT_RES_SHIT)
#include "config/bitcoin-config.h"
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_URL
#undef PACKAGE_VERSION
#endif

#define PACKAGE_NAME "Dallar Core"
#define PACKAGE_STRING "Dallar Core 1.2.0"
#define PACKAGE_URL "www.dallar.org"
#define PACKAGE_VERSION "1.2.0"

#ifdef _MSC_VER
#pragma warning(disable: 4005 4503 4018 4101 4146 4242 4244 4267 4290 4334 4717 4789 4800 4804)
#endif

//! Set to true for release, false for prerelease or test build
#define CLIENT_VERSION_IS_RELEASE true

// Check that required client information is defined
#if !defined(CLIENT_VERSION_MAJOR) || !defined(CLIENT_VERSION_MINOR) || !defined(CLIENT_VERSION_REVISION) || !defined(CLIENT_VERSION_BUILD) || !defined(CLIENT_VERSION_IS_RELEASE) || !defined(COPYRIGHT_YEAR)
#error Client version information missing: version is not defined by bitcoin-config.h or in any other way
#endif

#define QT_STATICPLUGIN true


/**
 * Converts the parameter X to a string after macro replacement on X has been performed.
 * Don't merge these into one macro!
 */
#define STRINGIZE(X) DO_STRINGIZE(X)
#define DO_STRINGIZE(X) #X

//! Copyright string used in Windows .rc files
#define COPYRIGHT_STR "2017-" STRINGIZE(COPYRIGHT_YEAR) " " COPYRIGHT_HOLDERS_FINAL

/**
 * dallard-res.rc includes this file, but it cannot cope with real c++ code.
 * WINDRES_PREPROC is defined to indicate that its pre-processor is running.
 * Anything other than a define should be guarded below.
 */

#if !defined(WINDRES_PREPROC)

#include <string>
#include <vector>

static const int CLIENT_VERSION =
                           1000000 * CLIENT_VERSION_MAJOR
                         +   10000 * CLIENT_VERSION_MINOR
                         +     100 * CLIENT_VERSION_REVISION
                         +       1 * CLIENT_VERSION_BUILD;

extern const std::string CLIENT_NAME;
extern const std::string CLIENT_BUILD;


std::string FormatFullVersion();
std::string FormatSubVersion(const std::string& name, int nClientVersion, const std::vector<std::string>& comments);

#endif // WINDRES_PREPROC

#endif // BITCOIN_CLIENTVERSION_H

// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2014-2015 The Dallarcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "groestlcoin.h"


#include "hash.h"
#include "crypto/common.h"




using namespace std;

extern "C" {
#include <sphlib/sph_groestl.h>
} // "C"

namespace XCoin {

uint256 HashDallar(const ConstBuf& cbuf) {
    sph_groestl512_context ctx_gr[3];
    static unsigned char pblank[1];
    uint256 hash[6];

    sph_groestl512_init(&ctx_gr[0]);
    sph_groestl512 (&ctx_gr[0], cbuf.P ? cbuf.P : pblank, cbuf.Size);
    sph_groestl512_close(&ctx_gr[0], static_cast<void*>(&hash[0]));

	sph_groestl512_init(&ctx_gr[1]);
	sph_groestl512(&ctx_gr[1],static_cast<const void*>(&hash[0]), 64);
	sph_groestl512_close(&ctx_gr[1],static_cast<void*>(&hash[2]));

	sph_groestl512_init(&ctx_gr[2]);
	sph_groestl512(&ctx_gr[2], static_cast<const void*>(&hash[2]), 64);
	sph_groestl512_close(&ctx_gr[2], static_cast<void*>(&hash[4]));

    return hash[4];
}

uint256 HashFromTx(const ConstBuf& cbuf) {
	CSHA256 sha;
	sha.Write(cbuf.P, cbuf.Size);
	uint256 r;
	sha.Finalize((unsigned char*)&r);
	return r;
}

uint256 HashForSignature(const ConstBuf& cbuf) {
	CSHA256 sha;
	sha.Write(cbuf.P, cbuf.Size);
	uint256 r;
	sha.Finalize((unsigned char*)&r);
	return r;
}

void DallarHasher::Finalize(unsigned char h[32]) {
	auto c = (sph_groestl512_context*)ctx;
	uint256 hash[6];
	sph_groestl512_close(c, static_cast<void*>(&hash[0]));

	sph_groestl512_context  c2;
	sph_groestl512_init(&c2);
	sph_groestl512(&c2, static_cast<const void*>(&hash[0]), 64);
	sph_groestl512_close(&c2, static_cast<void*>(&hash[2]));

	sph_groestl512_context  c3;
	sph_groestl512_init(&c3);
	sph_groestl512(&c3, static_cast<const void*>(&hash[2]), 64);
	sph_groestl512_close(&c3, static_cast<void*>(&hash[4]));
	memcpy(h, static_cast<void*>(&hash[4]), 32);
}

DallarHasher& DallarHasher::Write(const unsigned char *data, size_t len) {
	auto c = (sph_groestl512_context*)ctx;
	sph_groestl512(c, data, len);
	return *this;
}

DallarHasher::DallarHasher() {
	auto c = new sph_groestl512_context;
	ctx = c;
	sph_groestl512_init(c);
}

DallarHasher::DallarHasher(DallarHasher&& x)
	:	ctx(x.ctx)
{
	x.ctx = 0;
}

DallarHasher::~DallarHasher() {
	delete (sph_groestl512_context*)ctx;
}

DallarHasher& DallarHasher::operator=(DallarHasher&& x)
{
	delete (sph_groestl512_context*)ctx;
	ctx = x.ctx;
	x.ctx = 0;
	return *this;
}

} // XCoin::


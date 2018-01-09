// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2014-2015 The Dallar developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "groestlcoin.h"

#include <boost/assign/list_of.hpp>

#include "arith_uint256.h"
#include "chain.h"
#include "chainparams.h"
#include "consensus/merkle.h"
#include "consensus/params.h"
#include "utilstrencodings.h"
#include "crypto/sha256.h"
#include "util.h"

#include "bignum.h"

#include "chainparamsseeds.h"

extern "C" {

#if !defined(UCFG_LIBEXT) && (defined(_M_IX86) || defined(_M_X64))

	static __inline void Cpuid(int a[4], int level) {
#	ifdef WIN32
		__cpuid(a, level);
#	else
		__cpuid(level, a[0], a[1], a[2], a[3]);
#	endif
	}

	char g_bHasSse2;

	static int InitBignumFuns() {
		int a[4];
		::Cpuid(a, 1);
		g_bHasSse2 = a[3] & 0x02000000;
		return 1;
	}

	static int s_initBignumFuns = InitBignumFuns();
#endif // defined(_M_IX86) || defined(_M_X64)
} // "C"

using namespace std;

static const int64_t nGenesisBlockRewardCoin = 1 * COIN;
int64_t minimumSubsidy = 8.0 * COIN;
static const int64_t nPremine = 200000 * COIN;

CAmount GetBlockSubsidy(int nHeight, const Consensus::Params& consensusParams)
{
	if (nHeight == 0)
	{
		return nGenesisBlockRewardCoin;
	}

	if (nHeight == 1)
	{
		return nPremine;
	}

	int64_t nSubsidy = 256 * COIN;

	// Subsidy is reduced by 6% every 10080 blocks, which will occur approximately every 1 week
	int exponent = (nHeight / 10080);
	for (int i = 0; i < exponent; i++)
	{
		nSubsidy = nSubsidy * 47;
		nSubsidy = nSubsidy / 50;
	}
	if (nSubsidy < minimumSubsidy)
	{
		nSubsidy = minimumSubsidy;
	}
	return nSubsidy;
}

//
// minimum amount of work that could possibly be required nTime after
// minimum work required was nBase
//
static const int64_t nTargetSpacing = 1 * 60; // dallar every 60 seconds

unsigned int static DarkGravityWave3(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params) {
    /* current difficulty formula, darkcoin - DarkGravity v3, written by Evan Duffield - evan@darkcoin.io */
    const CBlockIndex *BlockLastSolved = pindexLast;
    const CBlockIndex *BlockReading = pindexLast;
    int64_t nActualTimespan = 0;
    int64_t LastBlockTime = 0;
    int64_t PastBlocksMin = 24;
    int64_t PastBlocksMax = 24;
    int64_t CountBlocks = 0;
    CBigNum PastDifficultyAverage;
    CBigNum PastDifficultyAveragePrev;

    if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || BlockLastSolved->nHeight < PastBlocksMin) {
		LogPrintf("Diff: %s", params.powLimit.GetHex());
		return UintToArith256(params.powLimit).GetCompact();
    }

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (PastBlocksMax > 0 && i > PastBlocksMax) { break; }
        CountBlocks++;

        if(CountBlocks <= PastBlocksMin) {
            if (CountBlocks == 1) { PastDifficultyAverage.SetCompact(BlockReading->nBits); }
            else { PastDifficultyAverage = ((PastDifficultyAveragePrev * CountBlocks)+(CBigNum().SetCompact(BlockReading->nBits))) / (CountBlocks+1); }
            PastDifficultyAveragePrev = PastDifficultyAverage;
        }

        if(LastBlockTime > 0){
            int64_t Diff = (LastBlockTime - BlockReading->GetBlockTime());
			// When making a new blockchain, if we're not careful, we can skyrocket difficulty
			if (BlockReading->nHeight < PastBlocksMin && Diff < 60)
			{
				Diff = 60;
			}
            nActualTimespan += Diff;
        }
        LastBlockTime = BlockReading->GetBlockTime();

        if (BlockReading->pprev == NULL) { assert(BlockReading); break; }
        BlockReading = BlockReading->pprev;
    }

    CBigNum bnNew(PastDifficultyAverage);

    int64_t nTargetTimespan = CountBlocks*nTargetSpacing;

    if (nActualTimespan < nTargetTimespan/3)
        nActualTimespan = nTargetTimespan/3;
    if (nActualTimespan > nTargetTimespan*3)
        nActualTimespan = nTargetTimespan*3;

    // Retarget
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespan;

	if (bnNew > CBigNum(params.powLimit)) {
		bnNew = CBigNum(params.powLimit);
	}

	
	LogPrintf("PastDiffAvg: %s, TargetTime: %dll, ActualTime: %dll, LastDiff: %s, New Diff %s", PastDifficultyAverage.GetHex(), nTargetTimespan, nActualTimespan, CBigNum().SetCompact(BlockReading->nBits).GetHex(), bnNew.GetHex());
    return bnNew.GetCompact();
}

unsigned int CalculateOLDSCHOOLNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
	//if (params.fPowNoRetargeting)
	//	return pindexLast->nBits;

	// Limit adjustment step
	int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
	if (nActualTimespan < params.nPowTargetTimespan / 4)
		nActualTimespan = params.nPowTargetTimespan / 4;
	if (nActualTimespan > params.nPowTargetTimespan * 4)
		nActualTimespan = params.nPowTargetTimespan * 4;

	// Retarget
	arith_uint256 bnNew;
	arith_uint256 bnOld;
	bnNew.SetCompact(pindexLast->nBits);
	bnOld = bnNew;
	// Dallar: intermediate uint256 can overflow by 1 bit
	const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
	bool fShift = bnNew.bits() > bnPowLimit.bits() - 1;
	if (fShift)
		bnNew >>= 1;
	bnNew *= nActualTimespan;
	bnNew /= params.nPowTargetTimespan;
	if (fShift)
		bnNew <<= 1;

	if (bnNew > bnPowLimit)
		bnNew = bnPowLimit;

	return bnNew.GetCompact();
}

unsigned int GetOLDSCHOOLNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
	assert(pindexLast != nullptr);
	unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

	// Only change once per difficulty adjustment interval
	if ((pindexLast->nHeight + 1) % params.DifficultyAdjustmentInterval() != 0)
	{
		if (params.fPowAllowMinDifficultyBlocks)
		{
			// Special difficulty rule for testnet:
			// If the new block's timestamp is more than 2* 10 minutes
			// then allow mining of a min-difficulty block.
			if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing * 2)
				return nProofOfWorkLimit;
			else
			{
				// Return the last non-special-min-difficulty-rules-block
				const CBlockIndex* pindex = pindexLast;
				while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
					pindex = pindex->pprev;
				return pindex->nBits;
			}
		}
		return pindexLast->nBits;
	}

	// Dallar: This fixes an issue where a 51% attack can change difficulty at will.
	// Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
	int blockstogoback = params.DifficultyAdjustmentInterval() - 1;
	if ((pindexLast->nHeight + 1) != params.DifficultyAdjustmentInterval())
		blockstogoback = params.DifficultyAdjustmentInterval();

	const CBlockIndex* pindexFirst = pindexLast;
	for (int i = 0; pindexFirst && i < blockstogoback; i++)
		pindexFirst = pindexFirst->pprev;

	assert(pindexFirst);

	return CalculateOLDSCHOOLNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params) {
    if (params.fPowAllowMinDifficultyBlocks)  {

		 // Special difficulty rule for testnet:
		 // If the new block's timestamp is more than 2* 10 minutes
		 // then allow mining of a min-difficulty block.

		if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
			return UintToArith256(params.powLimit).GetCompact();
    }  	

	if (pindexLast->nHeight < 10000) {
		return GetOLDSCHOOLNextWorkRequired(pindexLast, pblock, params);
	}
	else {
		return DarkGravityWave3(pindexLast, pblock, params);
	}
	
}

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward) {
	CMutableTransaction txNew;
	txNew.nVersion = 1;
	txNew.vin.resize(1);
	txNew.vout.resize(1);
	txNew.vin[0].scriptSig = CScript() << 133780085 << CScriptNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
	txNew.vout[0].nValue = genesisReward;
	txNew.vout[0].scriptPubKey = genesisOutputScript;

	CBlock genesis;
	genesis.nTime    = nTime;
	genesis.nBits    = nBits;
	genesis.nNonce   = nNonce;
	genesis.nVersion = nVersion;
	genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
	genesis.hashPrevBlock.SetNull();
	genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
	return genesis;
}

/**
* Build the genesis block. Note that the output of its generation
* transaction cannot be spent since it did not originally exist in the
* database.
*
* CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
*   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
*     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
*     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
*   vMerkleTree: 4a5e1e
*/
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward) {
	const char* pszTimestamp = "Library Of Congress Will No Longer Archive Every Tweet, Allar slams face on desk.";
	const CScript genesisOutputScript = CScript() << ParseHex("0458389298289560934894578376826726890378902389078789cba49a8492g50769583b3832248ac4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
	return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

// void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
// {
//     consensus.vDeployments[d].nStartTime = nStartTime;
//     consensus.vDeployments[d].nTimeout = nTimeout;
// }


/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
		consensus.BIP34Height = 0;
		consensus.BIP34Hash = uint256S("0x000005942766af8f99efd3b13fdf5be9ef43981273430f593c669cd1bfe4f586");
		consensus.BIP66Height = 0;
		consensus.BIP65Height = INT_MAX;	//!!!?
		consensus.BIP66Height = 0;	//!!!?

        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
		consensus.nMinimumChainWork = uint256S("0x00");
        consensus.nPowTargetTimespan = 5 * 60;
        consensus.nPowTargetSpacing = 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
		consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916;
		consensus.nMinerConfirmationWindow = 2016;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

		// Deployment of BIP68, BIP112, and BIP113.
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1484956800; // Jan 21, 2017
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1517356801; // Jun 21, 2017

		// Deployment of SegWit (BIP141 and BIP143)
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1484956800; // Jan 21, 2017
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1517356801; // Jun 21, 2017

		/**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xd9;
        pchMessageStart[1] = 0xd7;
        pchMessageStart[2] = 0xc2;
        pchMessageStart[3] = 0xd4;

		nDefaultPort = 20032;
        nPruneAfterHeight = 10000000;

		genesis = CreateGenesisBlock(1514351105, 255328, 0x1e0fffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
		assert(consensus.hashGenesisBlock == uint256S("0x000005942766af8f99efd3b13fdf5be9ef43981273430f593c669cd1bfe4f586"));
		assert(genesis.hashMerkleRoot == uint256S("0x8990fd06a59713876f22148b8086e9e9858e28fa87ca2ba7c3d4e1e46ef76c73"));
		consensus.nMinimumChainWork = uint256S("00000000000000000000000000000000000000000000000000014c8c51de9cb3");

        vSeeds.push_back(CDNSSeedData("dnsseed.dallar.org", "dnsseed.dallar.org"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,30);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

#ifdef _MSC_VER //!!!
		checkpointData = CCheckpointData{
#else
		checkpointData = (CCheckpointData) {
#endif
            {
                {0, uint256S("0x000005942766af8f99efd3b13fdf5be9ef43981273430f593c669cd1bfe4f586")},
				{3, uint256S("0x00000feb89ffc4366cd024f8fc2c71b23daa6deea7bd8cccc0c3261c7a823154") },
				{15, uint256S("0x0000007db54c02d2315f101ea271700d19bd970a63988c6ace4cbe63580ca557") },
				{45, uint256S("0x000000009d1c1b094b820511bce6f6a0f6f4bd01906834be86c5411de980c64f") },
				{100, uint256S("0x00000000190d9484c1ffb568f3ad4e10d3573284cac2683fa87d11e0e47225d0") },
				{500, uint256S("0x000000000d9f057c9dd49e95036d331d48ad46134ef1d6a45a06ffc336d1a071") },
				{1000, uint256S("0x0000000004f74a7a9908d40600e166971df3d06ba2ef1a4817438e95a20bb997") },
				{2000, uint256S("0x00000000011fe91da696da44890a0e1e6d17760ef9a578b25df662e35138d2ad") },
				{3000, uint256S("0x00000000046a359c49301555c45d7cea227af794412a8a257ca9e29069fb2047") },
				{3500, uint256S("0x00000000121ddfcd0cdc5fd5763b56aa3f167f1b41286587e427aab7e8389fad") },
				{7500, uint256S("0x000000000e6d8c2872864748085dad49dfab200fba42d0f348bbd934158010d0") }
            },
			1515461571,
            9000,
            0.05
        };
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
		consensus.BIP34Height = 0;
		consensus.BIP34Hash = uint256S("0x00");
		consensus.BIP66Height = 0;
		consensus.BIP65Height = 0;	//!!!?

        consensus.nPowTargetSpacing = 60;
		consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
		consensus.nMinerConfirmationWindow = 2016;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

		// Deployment of BIP68, BIP112, and BIP113.
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1484956800; // Jan 21, 2017
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1498003200; // Jun 21, 2017

		// Deployment of SegWit (BIP141, BIP143, and BIP147)
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1484956800; // Jan 21, 2017
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1498003200; // Jun 21, 2017

		consensus.powLimit = uint256S("000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
		pchMessageStart[0] = 0xd7;
		pchMessageStart[1] = 0xc2;
		pchMessageStart[2] = 0xd9;
		pchMessageStart[3] = 0xd4;

        nDefaultPort = 20132;
        nPruneAfterHeight = 1000000;



        //! Modify the testnet genesis block so the timestamp is valid for a later start.
		genesis = CreateGenesisBlock(1440000002, 6556309, 0x1e00ffff, 3, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        //assert(consensus.hashGenesisBlock == uint256S("0x00"));

        vFixedSeeds.clear();
        vSeeds.clear();
		vSeeds.push_back(CDNSSeedData("testnet1.dallar.org", "testnet1.dallar.org"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,30);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,10);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,20);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

//!!!?        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;


#ifdef _MSC_VER
		checkpointData = CCheckpointData{
#else
		checkpointData = (CCheckpointData) {
#endif
            {
                {0, uint256S("0x000005942766af8f99efd3b13fdf5be9ef43981273430f593c669cd1bfe4f586")},
            },
            1440000002,
            0,
            10
        };
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";

        consensus.nPowTargetSpacing = 60;
		consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
		consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;
		consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].bit = 1;
		consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].nStartTime = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].nTimeout = 999999999999ULL;

		pchMessageStart[0] = 0xc1;
        pchMessageStart[1] = 0xc2;
        pchMessageStart[2] = 0x3d;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 20232;

        nPruneAfterHeight = 1000;

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

						 //!!!?        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
    }
};
//!!!T static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
		return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
		return testNetParams;
//!!!T	else if (chain == CBaseChainParams::REGTEST)
//!!!T		return regTestParams;
	else
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
	pCurrentParams = &Params(network);
}

// void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
// {
//     globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
// }

void UpdateRegtestBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
//GRS	regTestParams.UpdateBIP9Parameters(d, nStartTime, nTimeout);
}

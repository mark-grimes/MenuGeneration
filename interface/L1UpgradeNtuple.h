#ifndef L1UpgradeNtuple_h
#define L1UpgradeNtuple_h

/** @file
 *
 * @brief Copy of UserCode/L1TriggerUpgrade/macros/L1UpgradeNtuple.C with minor changes to fix memory leaks.
 *
 * I need to use L1UpgradeNtuple, but it's in the macros directory of UserCode/L1TriggerUpgrade so I
 * can't include it using the standard "use" directive in a buildfile. I was previously incorporating
 * it with an #include directive so save having to potentially out of synch copies. I found some
 * memory leaks however so needed to change it, hence made another copy here.
 *
 * I'll just have to make a special effort to keep it up to date with the version in UserCode/L1TriggerUpgrade.
 *
 * 05/Dec/2013 Mark Grimes - Added some functionality to store L1 tracks. I've just blindly copied what
 * is done for the ExtraUpgrade entries for the L1Track equivalents.
 *
 * @author Copied over into this file by Mark Grimes, but original code probably by Brian Winer.
 * @date Copied here 06/Jun/2013
 */

#include <string>
#include <vector>
#include "Rtypes.h"

//
// Forward declarations
//
class TChain;
class TFile;
namespace L1Analysis
{
	class L1AnalysisEventDataFormat;
	class L1AnalysisGCTDataFormat;
	class L1AnalysisGMTDataFormat;
	class L1AnalysisGTDataFormat;
	class L1AnalysisRCTDataFormat;
	class L1AnalysisDTTFDataFormat;
	class L1AnalysisCSCTFDataFormat;
	class L1AnalysisCaloTPDataFormat;
	class L1AnalysisEventDataFormat;
	class L1AnalysisGCTDataFormat;
	class L1AnalysisGMTDataFormat;
	class L1AnalysisGTDataFormat;
	class L1AnalysisRCTDataFormat;
	class L1AnalysisDTTFDataFormat;
	class L1AnalysisCSCTFDataFormat;
	class L1AnalysisCaloTPDataFormat;
	class L1AnalysisRecoMetDataFormat;
	class L1AnalysisRecoMuonDataFormat;
	class L1AnalysisRecoJetDataFormat;
	class L1AnalysisRecoClusterDataFormat;
	class L1AnalysisRecoClusterDataFormat;
	class L1AnalysisRecoVertexDataFormat;
	class L1AnalysisRecoTrackDataFormat;
	class L1AnalysisL1ExtraDataFormat;
	class L1AnalysisL1ExtraDataFormat;
	class L1AnalysisL1MenuDataFormat;
	class L1AnalysisL1ExtraUpgradeDataFormat;
	class L1AnalysisSimulationDataFormat;
	class L1AnalysisGeneratorDataFormat;
	class L1AnalysisL1TrackDataFormat;
}


class L1UpgradeNtuple
{
public:
	TChain *fChain;   //!pointer to the analyzed TTree or TChain
	TChain *ftreeEmu;
	TChain *ftreemuon;
	TChain *ftreereco;
	TChain *ftreeExtra;
	TChain *ftreeMenu;
	TChain *ftreeEmuExtra;
	TChain *ftreeUpgrade;
	TChain *ftreeTrack;
	Int_t fCurrent; //!current Tree number in a TChain

	bool dol1emu;
	bool doreco;
	bool domuonreco;
	bool dol1extra;
	bool dol1emuextra;
	bool dol1menu;
	bool dol1upgrade;
	bool dol1track;

// L1Tree (Standard)
	L1Analysis::L1AnalysisEventDataFormat *event_;
	L1Analysis::L1AnalysisGCTDataFormat *gct_;
	L1Analysis::L1AnalysisGMTDataFormat *gmt_;
	L1Analysis::L1AnalysisGTDataFormat *gt_;
	L1Analysis::L1AnalysisRCTDataFormat *rct_;
	L1Analysis::L1AnalysisDTTFDataFormat *dttf_;
	L1Analysis::L1AnalysisCSCTFDataFormat *csctf_;
	L1Analysis::L1AnalysisCaloTPDataFormat *calo_;

// L1Tree (ReEmulated)
	L1Analysis::L1AnalysisEventDataFormat *eventEmu_;
	L1Analysis::L1AnalysisGCTDataFormat *gctEmu_;
	L1Analysis::L1AnalysisGMTDataFormat *gmtEmu_;
	L1Analysis::L1AnalysisGTDataFormat *gtEmu_;
	L1Analysis::L1AnalysisRCTDataFormat *rctEmu_;
	L1Analysis::L1AnalysisDTTFDataFormat *dttfEmu_;
	L1Analysis::L1AnalysisCSCTFDataFormat *csctfEmu_;
	L1Analysis::L1AnalysisCaloTPDataFormat *caloEmu_;

// Reco Trees
	L1Analysis::L1AnalysisRecoMetDataFormat *recoMet_;
	L1Analysis::L1AnalysisRecoMuonDataFormat *recoMuon_;
	L1Analysis::L1AnalysisRecoJetDataFormat *recoJet_;
	L1Analysis::L1AnalysisRecoClusterDataFormat *recoBasicCluster_;
	L1Analysis::L1AnalysisRecoClusterDataFormat *recoSuperCluster_;
	L1Analysis::L1AnalysisRecoVertexDataFormat *recoVertex_;
	L1Analysis::L1AnalysisRecoTrackDataFormat *recoTrack_;

// L1 Extra Trees (Standard and ReEmulated)
	L1Analysis::L1AnalysisL1ExtraDataFormat *l1extra_;
	L1Analysis::L1AnalysisL1ExtraDataFormat *l1emuextra_;

// L1 Menu
	L1Analysis::L1AnalysisL1MenuDataFormat *l1menu_;

// L1ExtraUpgrade Tree
	L1Analysis::L1AnalysisL1ExtraUpgradeDataFormat *l1upgrade_;
	L1Analysis::L1AnalysisL1TrackDataFormat *l1track_;

// Monte Carlo Information
	L1Analysis::L1AnalysisSimulationDataFormat *sim_;
	L1Analysis::L1AnalysisGeneratorDataFormat *gen_;

	L1UpgradeNtuple();
	L1UpgradeNtuple( const std::string & fname );

	virtual ~L1UpgradeNtuple();

	bool Open( const std::string & fname );
	bool OpenWithList( const std::string & fname );
	virtual Int_t GetEntry( Long64_t entry );
	virtual Long64_t LoadTree( Long64_t entry );
	virtual void Init();
	//virtual void     Loop();
	// Don't need these for now
	//void Test();
	//void Test2();
	Long64_t GetEntries();

private:
	bool CheckFirstFile();
	bool OpenWithoutInit();
	bool OpenNtupleList( const std::string & fname );

	std::vector<std::string> listNtuples;
	Long64_t nentries_;
	TFile* rf;
};

#endif

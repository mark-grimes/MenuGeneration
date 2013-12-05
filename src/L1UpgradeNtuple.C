#include "L1UpgradeNtuple.h"

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <TFriendElement.h>
#include <TList.h>
#include <TMatrix.h>
#include <TH1D.h>
#include <TH1F.h>
#include <TH2D.h>
#include <TH2F.h>
#include <TCanvas.h>


#include "UserCode/L1TriggerDPG/interface/L1AnalysisEventDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisGCTDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisGTDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisGMTDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisGTDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisRCTDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisDTTFDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisCSCTFDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisRecoMetDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisRecoJetDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisRecoClusterDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisRecoMuonDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisL1ExtraDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisRecoVertexDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisRecoTrackDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisL1MenuDataFormat.h"
#include "UserCode/L1TriggerUpgrade/interface/L1AnalysisL1ExtraUpgradeDataFormat.h"
#include "UserCode/L1TriggerUpgrade/interface/L1AnalysisL1TrackDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisSimulationDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisGeneratorDataFormat.h"
#include "UserCode/L1TriggerDPG/interface/L1AnalysisCaloTPDataFormat.h"

using namespace std;

Long64_t L1UpgradeNtuple::GetEntries()
{
  return nentries_;
}

L1UpgradeNtuple::L1UpgradeNtuple()
	: fChain(NULL), ftreeEmu(NULL), ftreemuon(NULL), ftreereco(NULL), ftreeExtra(NULL), ftreeMenu(NULL),
	  ftreeEmuExtra(NULL), ftreeUpgrade(NULL), ftreeTrack(NULL), event_(NULL), gct_(NULL), gmt_(NULL), gt_(NULL),
	  rct_(NULL), dttf_(NULL), csctf_(NULL), calo_(NULL), eventEmu_(NULL), gctEmu_(NULL), gmtEmu_(NULL),
	  gtEmu_(NULL), rctEmu_(NULL), dttfEmu_(NULL), csctfEmu_(NULL), caloEmu_(NULL), recoMet_(NULL),
	  recoMuon_(NULL), recoJet_(NULL), recoBasicCluster_(NULL), recoSuperCluster_(NULL), recoVertex_(NULL),
	  recoTrack_(NULL), l1extra_(NULL), l1emuextra_(NULL), l1menu_(NULL), l1upgrade_(NULL), l1track_(NULL), sim_(NULL), gen_(NULL)
{
  dol1emu=true; doreco=true; domuonreco=true; dol1extra=true; dol1emuextra=true; dol1menu=true; dol1upgrade=true; dol1track=true;
}

L1UpgradeNtuple::L1UpgradeNtuple(const std::string & fname)
	: fChain(NULL), ftreeEmu(NULL), ftreemuon(NULL), ftreereco(NULL), ftreeExtra(NULL), ftreeMenu(NULL),
	  ftreeEmuExtra(NULL), ftreeUpgrade(NULL), ftreeTrack(NULL), event_(NULL), gct_(NULL), gmt_(NULL), gt_(NULL),
	  rct_(NULL), dttf_(NULL), csctf_(NULL), calo_(NULL), eventEmu_(NULL), gctEmu_(NULL), gmtEmu_(NULL),
	  gtEmu_(NULL), rctEmu_(NULL), dttfEmu_(NULL), csctfEmu_(NULL), caloEmu_(NULL), recoMet_(NULL),
	  recoMuon_(NULL), recoJet_(NULL), recoBasicCluster_(NULL), recoSuperCluster_(NULL), recoVertex_(NULL),
	  recoTrack_(NULL), l1extra_(NULL), l1emuextra_(NULL), l1menu_(NULL), l1upgrade_(NULL), l1track_(NULL), sim_(NULL), gen_(NULL)
{
  dol1emu=true; doreco=true; domuonreco=true; dol1extra=true;  dol1emuextra=true; dol1menu=true; dol1upgrade=true; dol1track=true;
  Open(fname);
}

bool L1UpgradeNtuple::OpenWithList(const std::string & fname)
{
  if (!OpenNtupleList(fname)) exit(0);
  if (!CheckFirstFile())      exit(0);
  if (!OpenWithoutInit())     exit(0);

  std::cout.flush();cout<<"Going to init the available trees..."<<std::endl;std::cout.flush();
  Init();

  return true;
}

bool L1UpgradeNtuple::Open(const std::string & fname)
{
  listNtuples.push_back(fname);

  if (!CheckFirstFile())  exit(0);
  if (!OpenWithoutInit()) exit(0);

  std::cout.flush();cout<<"Going to init the available trees..."<<std::endl;std::cout.flush();
  Init();

  return true;
}

bool L1UpgradeNtuple::OpenNtupleList(const std::string & fname)
{
  std::ifstream flist(fname.c_str());
  if (!flist)
    {
      std::cout << "File "<<fname<<" is not found !"<<std::endl;
      return false;
    }

  while(!flist.eof())
    {
      std::string str;
      getline(flist,str);
      if (!flist.fail())
  {
           if (str!="") listNtuples.push_back(str);
  }
    }

  return true;
}

bool L1UpgradeNtuple::CheckFirstFile()
{
  if (listNtuples.size()==0) return false;

  rf = TFile::Open(listNtuples[0].c_str());

  if (rf==0) return false;
  if (rf->IsOpen()==0) return false;

  TTree * myChain     = (TTree*) rf->Get("l1NtupleProducer/L1Tree");
  TTree * mytreeEmu   = (TTree*) rf->Get("l1EmulatorTree/L1Tree");
  TTree * mytreemuon  = (TTree*) rf->Get("l1MuonRecoTreeProducer/MuonRecoTree");
  TTree * mytreejets  = (TTree*) rf->Get("l1RecoTreeProducer/RecoTree");
  TTree * mytreeExtra = (TTree*) rf->Get("l1ExtraTreeProducer/L1ExtraTree");
  TTree * mytreeEmuExtra = (TTree*) rf->Get("l1EmulatorExtraTree/L1ExtraTree");
  TTree * mytreeMenu  = (TTree*) rf->Get("l1MenuTreeProducer/L1MenuTree");
  TTree * mytreeUpgrade = (TTree*) rf->Get("l1ExtraUpgradeTreeProducer/L1ExtraUpgradeTree");
  TTree * mytreeTrack = (TTree*) rf->Get("l1TrackTreeProducer/L1TrackTree");

  if (!myChain) {
    std::cout<<"L1Tree not found .... "<<std::endl;
    return false;
  } else {
    std::cout<<"Main tree is found .."<<std::endl;
  }

  if (!mytreeEmu) {
    std::cout<<"L1TreeEmu not found, it will be skipped..."<<std::endl;
    dol1emu=false;
  } else
    {
      std::cout << "L1TreeEmu is found ..."<<std::endl;
    }

  if (!mytreejets) {
    std::cout<<"RecoTree not found, it will be skipped..."<<std::endl;
    doreco=false;
  } else
    {
      std::cout << "RecoTree is found ..."<<std::endl;
    }

  if (!mytreemuon) {
    std::cout<<"MuonRecoTree not found, it will be skipped..."<<std::endl;
    domuonreco=false;
  }
else
    {
      std::cout << "MuonRecoTree is found ..."<<std::endl;
    }

  if (!mytreeExtra) {
    std::cout<<"L1ExtraTree not found, it will be skipped..."<<std::endl;
    dol1extra=false;
  }
else
    {
      std::cout << "L1ExtraTree is found ..."<<std::endl;
    }
    if(!mytreeEmuExtra){
    std::cout<<"L1EmuExtraTree not found, it will be skipped..."<<std::endl;
    dol1emuextra=false;
    }
    else{
      std::cout << "L1EmuExtraTree is found ..."<<std::endl;
    }

  if (!mytreeMenu) {
    std::cout<<"L1MenuTree not found, it will be skipped..."<<std::endl;
    dol1menu=false;
  }
else
    {
      std::cout << "L1MenuTree is found ..."<<std::endl;
    }

  if (!mytreeUpgrade) {
    std::cout<<"L1ExtraUpgradeProducer/L1ExtraUpgradeTree not found, it will be skipped..."<<std::endl;
    dol1upgrade=false;
  }
  else{
    std::cout << "L1UpgradeProducer/L1ExtraTree is found ..."<<std::endl;
  }

  if (!mytreeTrack) {
    std::cout<<"L1TrackProducer/L1TrackTree not found, it will be skipped..."<<std::endl;
    dol1track=false;
  }
  else{
    std::cout << "L1TrackProducer/L1TrackTree is found ..."<<std::endl;
  }

  return true;
}


bool L1UpgradeNtuple::OpenWithoutInit()
{
  fChain     = new TChain("l1NtupleProducer/L1Tree");
  ftreeEmu   = new TChain("l1EmulatorTree/L1Tree");
  ftreemuon  = new TChain("l1MuonRecoTreeProducer/MuonRecoTree");
  ftreereco  = new TChain("l1RecoTreeProducer/RecoTree");
  ftreeExtra = new TChain("l1ExtraTreeProducer/L1ExtraTree");
  ftreeEmuExtra = new TChain("l1EmulatorExtraTree/L1ExtraTree");
  ftreeMenu  = new TChain("l1MenuTreeProducer/L1MenuTree");
  ftreeUpgrade = new TChain("l1ExtraUpgradeTreeProducer/L1ExtraUpgradeTree");
  ftreeTrack = new TChain("l1TrackTreeProducer/L1TrackTree");

  for (unsigned int i=0;i<listNtuples.size();i++)
  {
    std::cout << " -- Adding " << listNtuples[i] << std::endl;
    fChain->Add(listNtuples[i].c_str());

    if (dol1emu)    ftreeEmu   -> Add(listNtuples[i].c_str());
    if (doreco)     ftreereco  -> Add(listNtuples[i].c_str());
    if (domuonreco) ftreemuon  -> Add(listNtuples[i].c_str());
    if (dol1extra)  ftreeExtra -> Add(listNtuples[i].c_str());
    if (dol1emuextra) ftreeEmuExtra ->Add(listNtuples[i].c_str());
    if (dol1menu)   ftreeMenu  -> Add(listNtuples[i].c_str());
    if (dol1upgrade) ftreeUpgrade  -> Add(listNtuples[i].c_str());
    if (dol1track) ftreeTrack  -> Add(listNtuples[i].c_str());

  }

  if (dol1emu)    fChain->AddFriend(ftreeEmu);
  if (doreco)     fChain->AddFriend(ftreereco);
  if (domuonreco) fChain->AddFriend(ftreemuon);
  if (dol1extra)  fChain->AddFriend(ftreeExtra);
  if (dol1emuextra) fChain->AddFriend(ftreeEmuExtra);
  if (dol1menu)   fChain->AddFriend(ftreeMenu);
  if (dol1upgrade) fChain->AddFriend(ftreeUpgrade);
  if (dol1track) fChain->AddFriend(ftreeTrack);

  return true;
}

L1UpgradeNtuple::~L1UpgradeNtuple()
{
//  if (ftreemuon)  delete ftreemuon;
//  if (ftreereco)  delete ftreereco;
//  if (ftreeExtra) delete ftreeExtra;
//  if (ftreeEmuExtra) delete ftreeEmuExtra;
//  if (ftreeMenu)  delete ftreeMenu;
//  if (ftreeUpgrade) delete ftreeUpgrade;
//  if (ftreeEmu) delete ftreeEmu;
//  if (fChain)     delete fChain;
//  if (rf)         delete rf;
  delete fChain;
  delete ftreeEmu;
  delete ftreemuon;
  delete ftreereco;
  delete ftreeExtra;
  delete ftreeEmuExtra;
  delete ftreeMenu;
  delete ftreeUpgrade;
  delete ftreeTrack;
  delete event_;
  delete gct_;
  delete gmt_;
  delete gt_;
  delete rct_;
  delete dttf_;
  delete csctf_;
  delete sim_;
  delete gen_;
  delete calo_;
  delete eventEmu_;
  delete gctEmu_;
  delete gmtEmu_;
  delete gtEmu_;
  delete rctEmu_;
  delete dttfEmu_;
  delete csctfEmu_;
  delete caloEmu_;
  delete recoMet_;
  delete recoJet_;
  delete recoBasicCluster_;
  delete recoSuperCluster_;
  delete recoVertex_;
  delete recoTrack_;
  delete recoMuon_;
  delete l1extra_;
  delete l1emuextra_;
  delete l1menu_;
  delete l1upgrade_;
  delete l1track_;
}



Int_t L1UpgradeNtuple::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}

Long64_t L1UpgradeNtuple::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);

   if (centry < 0) return centry;
   if (!fChain->InheritsFrom(TChain::Class()))  return centry;
   TChain *chain = (TChain*)fChain;
   if (chain->GetTreeNumber() != fCurrent) {
      fCurrent = chain->GetTreeNumber();
   }
   return centry;
}

void L1UpgradeNtuple::Init()
{
   if (!fChain) return;
   fCurrent = -1;
   /*
   fChain->SetMakeClass(1);
   ftreemuon->SetMakeClass(1);
   ftreereco->SetMakeClass(1);
   ftreeExtra->SetMakeClass(1); */

   std::cout << "Estimate the number of entries ..."<<std::endl;
   nentries_=fChain->GetEntries();
   std::cout << nentries_ << std::endl;
   event_ = new L1Analysis::L1AnalysisEventDataFormat();
   gct_   = new L1Analysis::L1AnalysisGCTDataFormat();
   gmt_   = new L1Analysis::L1AnalysisGMTDataFormat();
   gt_    = new L1Analysis::L1AnalysisGTDataFormat();
   rct_   = new L1Analysis::L1AnalysisRCTDataFormat();
   dttf_  = new L1Analysis::L1AnalysisDTTFDataFormat();
   csctf_ = new L1Analysis::L1AnalysisCSCTFDataFormat();
	sim_   = new L1Analysis::L1AnalysisSimulationDataFormat();
	gen_   = new L1Analysis::L1AnalysisGeneratorDataFormat();
	calo_  = new L1Analysis::L1AnalysisCaloTPDataFormat();

   std::cout<<"Setting branch addresses for L1Tree...  "<<std::flush;

   fChain->SetBranchAddress("Event", &event_ );
   fChain->SetBranchAddress("GCT",   &gct_   );
   fChain->SetBranchAddress("GMT",   &gmt_   );
   fChain->SetBranchAddress("GT",    &gt_    );
   fChain->SetBranchAddress("RCT",   &rct_   );
   fChain->SetBranchAddress("CSCTF", &csctf_ );
   fChain->SetBranchAddress("DTTF",  &dttf_  );
	fChain->SetBranchAddress("Simulation",  &sim_  );
	fChain->SetBranchAddress("Generator",  &gen_  );
	fChain->SetBranchAddress("Calo", &calo_ );


   if (dol1emu)
	{
       std::cout<<"Setting branch addresses for L1TreeEmu..."<<std::endl;

       eventEmu_ = new L1Analysis::L1AnalysisEventDataFormat();
       gctEmu_   = new L1Analysis::L1AnalysisGCTDataFormat();
       gmtEmu_   = new L1Analysis::L1AnalysisGMTDataFormat();
       gtEmu_    = new L1Analysis::L1AnalysisGTDataFormat();
       rctEmu_   = new L1Analysis::L1AnalysisRCTDataFormat();
       dttfEmu_  = new L1Analysis::L1AnalysisDTTFDataFormat();
       csctfEmu_ = new L1Analysis::L1AnalysisCSCTFDataFormat();
       caloEmu_  = new L1Analysis::L1AnalysisCaloTPDataFormat();

       std::cout<<"Setting branch addresses for L1Tree...  "<<std::flush;

       ftreeEmu->SetBranchAddress("Event", &eventEmu_ );
       ftreeEmu->SetBranchAddress("GCT",   &gctEmu_   );
       ftreeEmu->SetBranchAddress("GMT",   &gmtEmu_   );
       ftreeEmu->SetBranchAddress("GT",    &gtEmu_	 );
       ftreeEmu->SetBranchAddress("RCT",   &rctEmu_   );
       ftreeEmu->SetBranchAddress("CSCTF", &csctfEmu_ );
       ftreeEmu->SetBranchAddress("DTTF",  &dttf_  );
       ftreeEmu->SetBranchAddress("Calo", &caloEmu_ );

       fChain->AddFriend(ftreeEmu);
       std::cout << "L1TreeEmu "  << ftreeEmu->GetEntries() << endl;
	}

   if (doreco)
   {
    std::cout<<"Setting branch addresses for reco..."<<std::endl;

    recoMet_          = new L1Analysis::L1AnalysisRecoMetDataFormat();
    recoJet_          = new L1Analysis::L1AnalysisRecoJetDataFormat();
    recoBasicCluster_ = new L1Analysis::L1AnalysisRecoClusterDataFormat();
    recoSuperCluster_ = new L1Analysis::L1AnalysisRecoClusterDataFormat();
    recoVertex_       = new L1Analysis::L1AnalysisRecoVertexDataFormat();
    recoTrack_        = new L1Analysis::L1AnalysisRecoTrackDataFormat();

    ftreereco->SetBranchAddress("Jet", &recoJet_);
    ftreereco->SetBranchAddress("BasicClusters", &recoBasicCluster_);
    ftreereco->SetBranchAddress("SuperClusters", &recoSuperCluster_);
    ftreereco->SetBranchAddress("Met", &recoMet_);
    ftreereco->SetBranchAddress("Tracks", &recoTrack_);
    ftreereco->SetBranchAddress("Vertices", &recoVertex_);
    fChain->AddFriend(ftreereco);
    std::cout << "RecoTree "  << ftreereco->GetEntries() << endl;
  }

   if (domuonreco)
   {
     std::cout<<"Setting branch addresses for muons...   "<<std::endl;
     recoMuon_ = new L1Analysis::L1AnalysisRecoMuonDataFormat() ;
     ftreemuon->SetBranchAddress("Muon",&recoMuon_);
     }

   if (dol1extra)
     {
     std::cout<<"Setting branch addresses for L1Extra... "<<std::endl;
     l1extra_ = new L1Analysis::L1AnalysisL1ExtraDataFormat();
     ftreeExtra->SetBranchAddress("L1Extra",&l1extra_);
     fChain->AddFriend(ftreeExtra);
     std::cout  << "L1ExtraTree: "<< ftreeExtra->GetEntries() << std::endl;
     }
   if ( dol1emuextra){
         std::cout<<"Setting branch addresses for L1EmuExtra... "<<std::endl;
     l1emuextra_ = new L1Analysis::L1AnalysisL1ExtraDataFormat();
     ftreeEmuExtra->SetBranchAddress("L1Extra",&l1emuextra_);
     fChain->AddFriend(ftreeEmuExtra);
     std::cout  << "L1EmuExtraTree: "<< ftreeEmuExtra->GetEntries() << std::endl;
  }

   if (dol1menu)
     {
     std::cout<<"Setting branch addresses for L1Menu... "<<std::endl;
     l1menu_ = new L1Analysis::L1AnalysisL1MenuDataFormat();
     ftreeMenu->SetBranchAddress("L1Menu",&l1menu_);
     }
   if (dol1upgrade)
     {
     std::cout<<"Setting branch addresses for L1Upgrade... "<<std::endl;
     l1upgrade_ = new L1Analysis::L1AnalysisL1ExtraUpgradeDataFormat();
     ftreeUpgrade->SetBranchAddress("L1ExtraUpgrade",&l1upgrade_);
     fChain->AddFriend(ftreeUpgrade);
     std::cout << "L1ExtraUpgradeTree: "<< ftreeUpgrade->GetEntries() << std::endl;
     }
   if (dol1track)
     {
     std::cout<<"Setting branch addresses for L1Track... "<<std::endl;
     l1track_ = new L1Analysis::L1AnalysisL1TrackDataFormat();
     ftreeTrack->SetBranchAddress("L1Track",&l1track_);
     fChain->AddFriend(ftreeTrack);
     std::cout << "L1TrackTree: "<< ftreeTrack->GetEntries() << std::endl;
     }

}

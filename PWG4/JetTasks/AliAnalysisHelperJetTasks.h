#ifndef  ALIANALYSISHELPERJETTASKS_H
#define  ALIANALYSISHELPERJETTASKS_H


#include "TObject.h"
class AliMCEvent;
class AliAODJet;
class AliGenPythiaEventHeader;

// Helper Class that contains a lot of usefull static functions (i.e. for Flavor selection.

class AliAnalysisHelperJetTasks : public TObject {
 public:
  AliAnalysisHelperJetTasks() : TObject() {;}
  virtual ~AliAnalysisHelperJetTasks(){;}
  
  static AliGenPythiaEventHeader*  GetPythiaEventHeader(AliMCEvent *mcEvent);
  static void PrintStack(AliMCEvent *mcEvent,Int_t iFirst = 0,Int_t iLast = 0,Int_t iMaxPrint = 10);
  static void GetClosestJets(AliAODJet *genJets,
			     const Int_t &nGenJets,
			     AliAODJet *recJets,
			     const Int_t &nRecJets,
			     Int_t *iGenIndex,
			     Int_t *iRecIndex,
			     Int_t iDebug, Float_t maxDist = 0.5);

  static void MergeOutput(char* cFiles, char* cList = "pwg4spec"); // Merges the files in the input text file  needs the two histograms fh1PtHard_Trials, fh1Xsec and the name of the input list
  

  private:
  
  ClassDef(AliAnalysisHelperJetTasks, 1) 
};

#endif // ALIANALYSISHELPERJETTASKS_H

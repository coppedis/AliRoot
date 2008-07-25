/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
* See cxx source for full Copyright notice */
/* $Id$ */


#ifndef AliAnalysisTaskLeeYangZeros_H
#define AliAnalysisTaskLeeYangZeros_H

// AliAnalysisTaskLeeYangZeros:
// analysis task for 
// Lee Yang Zeroes method
// Author: 
// Naomi van der Kolk (kolk@nikhef.nl)

class AliESDEvent;
class AliAODEvent;
class AliCFManager;
class AliFlowAnalysisWithLeeYangZeros;
class AliFlowEventSimpleMaker;
class TFile;
class TList;

#include "TString.h"
#include "AliAnalysisTask.h"

class AliAnalysisTaskLeeYangZeros : public AliAnalysisTask {
 public:
  //AliAnalysisTaskLeeYangZeros();
  AliAnalysisTaskLeeYangZeros(const char *name = "AliAnalysisTaskLeeYangZeros", Bool_t firstrun = kTRUE);
  virtual ~AliAnalysisTaskLeeYangZeros();
  
  virtual void   ConnectInputData(Option_t *);
  virtual void   CreateOutputObjects();
  virtual void   Exec(Option_t *option);
  virtual void   Terminate(Option_t *);

  //lyz flags
  void           SetFirstRunLYZ(Bool_t kt)     { this->fFirstRunLYZ = kt ;  }
  Bool_t         GetFirstRunLYZ() const        { return this->fFirstRunLYZ ; }
  void           SetUseSumLYZ(Bool_t kt)       { this->fUseSumLYZ = kt ;  }
  Bool_t         GetUseSumLYZ() const          { return this->fUseSumLYZ ; }
  void           SetAnalysisType(TString type) {this->fAnalysisType = type ; }
  TString        GetAnalysisType()             {return this->fAnalysisType; } 
  void           SetCFManager1(AliCFManager* cfmgr) {this->fCFManager1 = cfmgr; } 
  AliCFManager*  GetCFManager1()               {return this->fCFManager1; }
  void           SetCFManager2(AliCFManager* cfmgr) {this->fCFManager2 = cfmgr; } 
  AliCFManager*  GetCFManager2()               {return this->fCFManager2; }


 private:
 
  AliAnalysisTaskLeeYangZeros(const AliAnalysisTaskLeeYangZeros& aAnalysis);
  AliAnalysisTaskLeeYangZeros& operator=(const AliAnalysisTaskLeeYangZeros& aAnalysis);

  AliESDEvent*     fESD;                  //ESD object
  AliAODEvent*     fAOD;                  //AOD object
  TString          fAnalysisType;         //string to select which kind of input to analyse: ESD, AOD or MC
  AliCFManager*    fCFManager1;           // correction framework manager
  AliCFManager*    fCFManager2;           // correction framework manager
  AliFlowAnalysisWithLeeYangZeros* fLyz;  //LYZ analysis object
  AliFlowEventSimpleMaker* fEventMaker;   //FlowEventSimple maker object

  TFile*           fFirstRunFile;         // file from the first loop over events
  TList*           fListHistos;           // collection of output

  //flags
  Bool_t fFirstRunLYZ ;    //! flag for lyz analysis 
  Bool_t fUseSumLYZ ;      //! flag for lyz analysis 
  
      
  ClassDef(AliAnalysisTaskLeeYangZeros, 1); // example of analysis
};

#endif


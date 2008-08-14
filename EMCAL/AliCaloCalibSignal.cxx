/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/
/* $Id: AliCaloCalibSignal.cxx $ */

//________________________________________________________________________
//
// A help class for monitoring and calibration tools: MOOD, AMORE etc.,
// It can be created and used a la (ctor):
/*
  //Create the object for making the histograms
  fSignals = new AliCaloCalibSignal( fDetType );
  // AliCaloCalibSignal knows how many modules we have for PHOS or EMCAL
  fNumModules = fSignals->GetModules();
*/
// fed an event:
//  fSignals->ProcessEvent(fCaloRawStream,fRawEventHeaderBase);
// get some info:
//  fSignals->GetXXX..()
// etc.
//________________________________________________________________________

#include "TProfile.h"
#include "TFile.h"

#include "AliRawReader.h"
#include "AliRawEventHeaderBase.h"
#include "AliCaloRawStream.h"

//The include file
#include "AliCaloCalibSignal.h"

ClassImp(AliCaloCalibSignal)

using namespace std;

// variables for TTree filling; not sure if they should be static or not
static int fChannelNum;
static double fAmp;
static double fAvgAmp;
static double fRMS;

// ctor; initialize everything in order to avoid compiler warnings
// put some reasonable defaults
AliCaloCalibSignal::AliCaloCalibSignal(kDetType detectorType) :  
  TObject(),
  fDetType(kNone),
  fColumns(0),
  fRows(0),
  fModules(0),
  fCaloString(),
  fMapping(NULL),
  fRunNumber(-1),
  fStartTime(0),
  fAmpCut(50),
  fReqFractionAboveAmpCutVal(0.8),
  fReqFractionAboveAmp(kTRUE),
  fHour(0),
  fLatestHour(0),
  fUseAverage(kTRUE),
  fSecInAverage(1800), 
  fNEvents(0),
  fNAcceptedEvents(0),
  fTreeAmpVsTime(NULL),
  fTreeAvgAmpVsTime(NULL)
{
  //Default constructor. First we set the detector-type related constants.
  if (detectorType == kPhos) {
    fColumns = fgkPhosCols;
    fRows = fgkPhosRows;
    fModules = fgkPhosModules;
    fCaloString = "PHOS";
  } 
  else {
    //We'll just trust the enum to keep everything in line, so that if detectorType
    //isn't kPhos then it is kEmCal. Note, however, that this is not necessarily the
    //case, if someone intentionally gives another number
    fColumns = fgkEmCalCols;
    fRows = fgkEmCalRows;
    fModules = fgkEmCalModules;
    fCaloString = "EMCAL";
  }

  fDetType = detectorType;

  ResetInfo(); // trees and counters
} 

// dtor
//_____________________________________________________________________
AliCaloCalibSignal::~AliCaloCalibSignal()
{
  DeleteTrees();
}

//_____________________________________________________________________
void AliCaloCalibSignal::DeleteTrees()
{
  // delete what was created in the ctor (TTrees)
  if (fTreeAmpVsTime) delete fTreeAmpVsTime;
  if (fTreeAvgAmpVsTime) delete fTreeAvgAmpVsTime;
  // and reset pointers
  fTreeAmpVsTime = NULL;
  fTreeAvgAmpVsTime = NULL;

  return;
}

// copy ctor
//_____________________________________________________________________
AliCaloCalibSignal::AliCaloCalibSignal(const AliCaloCalibSignal &sig) :
  TObject(sig),
  fDetType(sig.GetDetectorType()),
  fColumns(sig.GetColumns()),
  fRows(sig.GetRows()),
  fModules(sig.GetModules()),
  fCaloString(sig.GetCaloString()),
  fMapping(NULL), //! note that we are not copying the map info
  fRunNumber(sig.GetRunNumber()),
  fStartTime(sig.GetStartTime()),
  fAmpCut(sig.GetAmpCut()),
  fReqFractionAboveAmpCutVal(sig.GetReqFractionAboveAmpCutVal()),
  fReqFractionAboveAmp(sig.GetReqFractionAboveAmp()),
  fHour(sig.GetHour()),
  fLatestHour(sig.GetLatestHour()),
  fUseAverage(sig.GetUseAverage()),
  fSecInAverage(sig.GetSecInAverage()),
  fNEvents(sig.GetNEvents()),
  fNAcceptedEvents(sig.GetNAcceptedEvents()),
  fTreeAmpVsTime(NULL),
  fTreeAvgAmpVsTime(NULL)
{
  // also the TTree contents
  AddInfo(&sig);
}

// assignment operator; use copy ctor to make life easy..
//_____________________________________________________________________
AliCaloCalibSignal& AliCaloCalibSignal::operator = (const AliCaloCalibSignal &source)
{
  // assignment operator; use copy ctor
  if (&source == this) return *this;

  new (this) AliCaloCalibSignal(source);
  return *this;
}

//_____________________________________________________________________
void AliCaloCalibSignal::CreateTrees()
{
  // initialize trees
  // first, regular version
  fTreeAmpVsTime = new TTree("fTreeAmpVsTime","Amplitude vs. Time Tree Variables");

  fTreeAmpVsTime->Branch("fChannelNum", &fChannelNum, "fChannelNum/I");
  fTreeAmpVsTime->Branch("fHour", &fHour, "fHour/D");
  fTreeAmpVsTime->Branch("fAmp", &fAmp, "fAmp/D");

  // then, average version
  fTreeAvgAmpVsTime = new TTree("fTreeAvgAmpVsTime","Average Amplitude vs. Time Tree Variables");

  fTreeAvgAmpVsTime->Branch("fChannelNum", &fChannelNum, "fChannelNum/I");
  fTreeAvgAmpVsTime->Branch("fHour", &fHour, "fHour/D");
  fTreeAvgAmpVsTime->Branch("fAvgAmp", &fAvgAmp, "fAvgAmp/D");
  fTreeAvgAmpVsTime->Branch("fRMS", &fRMS, "fRMS/D");

  return;
}

//_____________________________________________________________________
void AliCaloCalibSignal::ResetInfo()
{
  Zero(); // set all counters to 0
  DeleteTrees(); // delete previous stuff
  CreateTrees(); // and create some new ones
  return;
}

//_____________________________________________________________________
void AliCaloCalibSignal::Zero()
{
  // set all counters to 0; not cuts etc. though
  fHour = 0;
  fLatestHour = 0;
  fNEvents = 0;
  fNAcceptedEvents = 0;

  // Set the number of points for each Amp vs. Time graph to 0
  memset(fNHighGain, 0, sizeof(fNHighGain));
  memset(fNLowGain, 0, sizeof(fNLowGain));

  return;
}

//_____________________________________________________________________
Bool_t AliCaloCalibSignal::CheckFractionAboveAmp(int *AmpVal, int nTotChan)
{
  int nAbove = 0;
    
  int TowerNum = 0;
  for (int i = 0; i<fModules; i++) {
    for (int j = 0; j<fColumns; j++) {
      for (int k = 0; k<fRows; k++) {
	TowerNum = GetTowerNum(i,j,k);
	if (AmpVal[TowerNum] > fAmpCut) { 
	  nAbove++;
	}
      }
    }
  }
  
  double fraction = (1.0*nAbove) / nTotChan;
  
  if (fraction > fReqFractionAboveAmpCutVal) {  
    return true;
  }
  else return false;
}

//_____________________________________________________________________
Bool_t AliCaloCalibSignal::AddInfo(const AliCaloCalibSignal *sig)
{
  // add info from sig's TTrees to ours..
  TTree *sigAmp = sig->GetTreeAmpVsTime();
  TTree *sigAvgAmp = sig->GetTreeAvgAmpVsTime();

  // we could try some merging via TList or what also as a more elegant approach
  // but I wanted with the stupid/simple and hopefully safe approach of looping
  // over what we want to add..

  // associate variables for sigAmp and sigAvgAmp:
  sigAmp->SetBranchAddress("fChannelNum",&fChannelNum);
  sigAmp->SetBranchAddress("fHour",&fHour);
  sigAmp->SetBranchAddress("fAmp",&fAmp);

  // loop over the trees.. note that since we use the same variables we should not need
  // to do any assignments between the getting and filling
  for (int i=0; i<sigAmp->GetEntries(); i++) {
    sigAmp->GetEntry(i);
    fTreeAmpVsTime->Fill();
  }

  sigAvgAmp->SetBranchAddress("fChannelNum",&fChannelNum);
  sigAvgAmp->SetBranchAddress("fHour",&fHour);
  sigAvgAmp->SetBranchAddress("fAvgAmp",&fAvgAmp);
  sigAvgAmp->SetBranchAddress("fRMS",&fRMS);

  for (int i=0; i<sigAvgAmp->GetEntries(); i++) {
    sigAvgAmp->GetEntry(i);
    fTreeAvgAmpVsTime->Fill();
  }

  return kTRUE;//We hopefully succesfully added info from the supplied object
}

//_____________________________________________________________________
Bool_t AliCaloCalibSignal::ProcessEvent(AliRawReader *rawReader)
{
  // if fMapping is NULL the rawstream will crate its own mapping
  AliCaloRawStream rawStream(rawReader, fCaloString, (AliAltroMapping**)fMapping);

  return ProcessEvent( &rawStream, (AliRawEventHeaderBase*)rawReader->GetEventHeader() );
}

//_____________________________________________________________________
Bool_t AliCaloCalibSignal::ProcessEvent(AliCaloRawStream *in, AliRawEventHeaderBase *aliHeader)
{ 
  // Method to process=analyze one event in the data stream
  if (!in) return kFALSE; //Return right away if there's a null pointer
  
  fNEvents++; // one more event

  // PHOS has more towers than EMCAL, so use PHOS numbers to set array sizes
  int AmpValHighGain[fgkMaxTowers];
  int AmpValLowGain[fgkMaxTowers];

  memset(AmpValHighGain, 0, sizeof(AmpValHighGain));
  memset(AmpValLowGain, 0, sizeof(AmpValLowGain));

  int sample, isample = 0; //The sample temp, and the sample number in current event.
  int max = fgkSampleMin, min = fgkSampleMax;//Use these for picking the signal
  int gain = 0;
  
  // Number of Low and High gain channels for this event:
  int nLowChan = 0; 
  int nHighChan = 0; 

  int TowerNum = 0; // array index for TGraphs etc.

  // loop first to get the fraction of channels with amplitudes above cut
  while (in->Next()) {
    sample = in->GetSignal(); //Get the adc signal
    if (sample < min) min = sample;
    if (sample > max) max = sample;
    isample++;
    if ( isample >= in->GetTimeLength()) {
      //If we're here then we're done with this tower
      gain = 1 - in->IsLowGain();
      
      int arrayPos = in->GetModule(); //The modules are numbered starting from 0
      if (arrayPos >= fModules) {
	//TODO: return an error message, if appopriate (perhaps if debug>0?)
	return kFALSE;
      } 
      
      //Debug
      if (arrayPos < 0 || arrayPos >= fModules) {
	printf("AliCaloCalibSignal::ProcessEvent = Oh no: arrayPos = %i.\n", arrayPos); 
      }

      // get tower number for AmpVal array
      TowerNum = GetTowerNum(arrayPos, in->GetColumn(), in->GetRow()); 

      if (gain == 0) {
	// fill amplitude into the array	   
        AmpValLowGain[TowerNum] = max - min;
	nLowChan++;
      } 
      else if (gain==1) {//fill the high gain ones
	// fill amplitude into the array
	AmpValHighGain[TowerNum] = max - min;
	nHighChan++;
      }//end if gain

      
      max = fgkSampleMin; min = fgkSampleMax;
      isample = 0;
      
    }//End if end of tower
   
  }//end while, of stream
  
  // now check if it was a led event, only use high gain (that should be sufficient)
  if (fReqFractionAboveAmp) {
    bool ok = false;
    if (nHighChan > 0) { 
      ok = CheckFractionAboveAmp(AmpValHighGain, nHighChan); 
    }
    if (!ok) return false; // skip event
  }

  fNAcceptedEvents++; // one more event accepted

  if (fStartTime == 0) { // if start-timestamp wasn't set,we'll pick it up from the first event we encounter
    fStartTime = aliHeader->Get("Timestamp");
  }

  fHour = (aliHeader->Get("Timestamp")-fStartTime)/(double)fgkNumSecInHr;
  if (fLatestHour < fHour) {
    fLatestHour = fHour; 
  }
  
  // it is a led event, now fill TTree
  for(int i=0; i<fModules; i++){
    for(int j=0; j<fColumns; j++){
      for(int k=0; k<fRows; k++){
	
	TowerNum = GetTowerNum(i, j, k); 

	if(AmpValHighGain[TowerNum]) {
	  fAmp = AmpValHighGain[TowerNum];
	  fChannelNum = GetChannelNum(i,j,k,1);
	  fTreeAmpVsTime->Fill();//fChannelNum,fHour,AmpValHighGain[TowerNum]);
	  fNHighGain[TowerNum]++;
	}
	if(AmpValLowGain[TowerNum]) {
	  fAmp = AmpValLowGain[TowerNum];
	  fChannelNum = GetChannelNum(i,j,k,0);
	  fTreeAmpVsTime->Fill();//fChannelNum,fHour,AmpValLowGain[TowerNum]);
	  fNLowGain[TowerNum]++;
	}
      }
    }
  }
  
  return kTRUE;
}

//_____________________________________________________________________
Bool_t AliCaloCalibSignal::Save(TString fileName)
{
  //Saves all the TTrees to the designated file
  
  TFile destFile(fileName, "recreate");
  
  if (destFile.IsZombie()) {
    return kFALSE;
  }
  
  destFile.cd();

  // save the trees
  fTreeAmpVsTime->Write();
  if (fUseAverage) { 
    Analyze(); // get the latest and greatest averages
    fTreeAvgAmpVsTime->Write();
  }

  destFile.Close();
  
  return kTRUE;
}

//_____________________________________________________________________
Bool_t AliCaloCalibSignal::Analyze()
{
  // Fill the tree holding the average values
  if (!fUseAverage) { return kFALSE; }

  // Reset the average TTree if Analyze has already been called earlier,
  // meaning that the TTree could have been partially filled
  if (fTreeAvgAmpVsTime->GetEntries() > 0) {
    fTreeAvgAmpVsTime->Reset();
  }

  //0: setup variables for the TProfile plots that we'll use to do the averages
  int numProfBins = 0;
  double timeMin = 0;
  double timeMax = 0;
  if (fSecInAverage > 0) {
    numProfBins = (int)( (fLatestHour*fgkNumSecInHr)/fSecInAverage + 1 ); // round-off
  }
  numProfBins += 2; // add extra buffer : first and last
  double binSize = 1.0*fSecInAverage / fgkNumSecInHr;
  timeMin = - binSize;
  timeMax = timeMin + numProfBins*binSize;

  //1: set up TProfiles for the towers that had data
  TProfile * profile[fgkMaxTowers*2]; // *2 is since we include both high and low gains
  memset(profile, 0, sizeof(profile));

  char name[200]; // for profile id and title
  int TowerNum = 0;

  for (int i = 0; i<fModules; i++) {
    for (int ic=0; ic<fColumns; ic++){
      for (int ir=0; ir<fRows; ir++) {

	TowerNum = GetTowerNum(i, ic, ir);
	// high gain
	if (fNHighGain[TowerNum] > 0) {
	  fChannelNum = GetChannelNum(i, ic, ir, 1); 
	  sprintf(name, "profileChan%d", fChannelNum);
	  profile[fChannelNum] = new TProfile(name, name, numProfBins, timeMin, timeMax, "s");
	}

	// same for low gain
	if (fNLowGain[TowerNum] > 0) {
	  fChannelNum = GetChannelNum(i, ic, ir, 0); 
	  sprintf(name, "profileChan%d", fChannelNum);
	  profile[fChannelNum] = new TProfile(name, name, numProfBins, timeMin, timeMax, "s");
	}

      } // rows
    } // columns
  } // modules

  //2: fill profiles by looping over tree
  // Set addresses for tree-readback also
  fTreeAmpVsTime->SetBranchAddress("fChannelNum", &fChannelNum);
  fTreeAmpVsTime->SetBranchAddress("fHour", &fHour);
  fTreeAmpVsTime->SetBranchAddress("fAmp", &fAmp);

  for (int ient=0; ient<fTreeAmpVsTime->GetEntries(); ient++) {
    fTreeAmpVsTime->GetEntry(ient);
    if (profile[fChannelNum]) { 
      // profile should always have been created above, for active channels
      profile[fChannelNum]->Fill(fHour, fAmp);
    }
  }

  // re-associating the branch addresses here seems to be needed for OK 'average' storage	    
  fTreeAvgAmpVsTime->SetBranchAddress("fChannelNum", &fChannelNum);
  fTreeAvgAmpVsTime->SetBranchAddress("fHour", &fHour);
  fTreeAvgAmpVsTime->SetBranchAddress("fAvgAmp", &fAvgAmp);
  fTreeAvgAmpVsTime->SetBranchAddress("fRMS", &fRMS);

  //3: fill avg tree by looping over the profiles
  for (fChannelNum = 0; fChannelNum<(fgkMaxTowers*2); fChannelNum++) {
    if (profile[fChannelNum]) { // profile was created
      if (profile[fChannelNum]->GetEntries() > 0) { // profile had some entries
	for(int it=0; it<numProfBins; it++) {
	  if (profile[fChannelNum]->GetBinEntries(it+1) > 0) {
	    fAvgAmp = profile[fChannelNum]->GetBinContent(it+1);
	    fHour = profile[fChannelNum]->GetBinCenter(it+1);
	    fRMS = profile[fChannelNum]->GetBinError(it+1);
	    fTreeAvgAmpVsTime->Fill();
	  } // some entries for this bin
	} // loop over bins
      } // some entries for this profile
    } // profile exists  
  } // loop over all possible channels

  return kTRUE;
}

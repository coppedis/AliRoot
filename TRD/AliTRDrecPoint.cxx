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

/*
$Log$
Revision 1.2  2000/06/08 18:32:58  cblume
Make code compliant to coding conventions

Revision 1.1  2000/02/28 19:02:07  cblume
Add new TRD classes

*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  TRD reconstructed point                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "AliTRDgeometry.h"
#include "AliTRDrecPoint.h"
#include "AliTRD.h"

ClassImp(AliTRDrecPoint)

//_____________________________________________________________________________
AliTRDrecPoint::AliTRDrecPoint():AliRecPoint()
{
  //
  // Standard constructor
  //

  fDetector = 0;

  AliTRD *trd;
  if ((gAlice) &&
      (trd = ((AliTRD*) gAlice->GetDetector("TRD")))) {
    fGeom = trd->GetGeometry();
  }
  else {
    fGeom = NULL;
  }

}

//_____________________________________________________________________________
AliTRDrecPoint::~AliTRDrecPoint()
{
  //
  // AliTRDrecPoint destructor
  //

}

//_____________________________________________________________________________
void AliTRDrecPoint::AddDigit(Int_t digit)
{
  //
  // Adds the index of a digit to the digits list
  //

  // First resize the list 
  // (no clusters with more than 3 digits for the TRD
  if ((fMulDigit == 0) && (fMaxDigit >= 5)) {
    fMaxDigit = 5;
    delete fDigitsList;
    fDigitsList = new int[fMaxDigit];
  }

  // Increase the size of the list if necessary
  if (fMulDigit >= fMaxDigit) { 
    fMaxDigit *= 2;
    int *tempo = new (int[fMaxDigit]); 
    Int_t index; 
    for (index = 0; index < fMulDigit; index++)
      tempo[index] = fDigitsList[index]; 
    delete fDigitsList; 
    fDigitsList = tempo; 
  }
  
  fDigitsList[fMulDigit++] = digit;

}

//_____________________________________________________________________________
void AliTRDrecPoint::SetLocalPosition(TVector3 &pos)
{
  //
  // Sets the position of the point in the local coordinate system
  // (row,col,time) and calculates the error matrix in the same
  // system.
  //

  const Float_t kSq12 = 3.464101615;

  // Set the position
  fLocPos = pos;

  // Set the error matrix
  // row:  pad-size / sqrt(12)
  // col:  not defined yet
  // time: bin-size / sqrt(12)
  fLocPosM->operator()(0,0) = ((AliTRDgeometry *) fGeom)->GetRowPadSize()  
                            / kSq12;
  fLocPosM->operator()(1,1) = 0.0;
  fLocPosM->operator()(2,2) = ((AliTRDgeometry *) fGeom)->GetTimeBinSize() 
                            / kSq12;

}


#include <iostream.h>

#include "TGClient.h"
#include "Rtypes.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TObjectTable.h"
#include "TGCanvas.h"
#include "TPad.h"
#include "TPaveText.h"
#include "TGTab.h"
#include "TGIcon.h"
#include <TExec.h>
#include "TGFrame.h"
#include "TGMenu.h"
#include "TSystem.h"
#include "TGLayout.h"
#include "TGLabel.h"
#include "TGButton.h"
#include "TGTextBuffer.h"
#include "TGTextEntry.h"
#include "TGToolTip.h"
#include "TFile.h"
#include "TTimer.h"
#include "TColor.h"
#include "TQObject.h"
#include "TRootHelpDialog.h"
#include "RQ_OBJECT.h"

#include "GreenTB.h"
#include "GreenADC.h"
#include "../cfSock/cfSock_types.h"
#include "../bmw/bmw_cf_commands.h"
//#include "GMSock.h"

#define GM_LOCAL     201

#define GM_BMW_CHANGE 2001
#define GM_BMW_CHECK  2002
#define GM_BMW_TEST_ENABLE 2005
#define GM_BMW_TEST_SET_VALUE 2006
#define GM_BMW_SET_KILL 2007
#define BMW_OBJRADIO1 2101
#define BMW_OBJRADIO2 2102
#define BMW_OBJRADIO3 2103
#define BMW_OBJRADIO4 2104
#define BMW_OBJRADIO5 2105
#define BMW_OBJRADIO6 2106
#define BMW_OBJRADIO7 2107
#define BMW_OBJRADIO8 2108

#define KILL_SERVER_1 5001
#define KILL_SERVER_2 5002
#define KILL_SERVER_3 5003
#define KILL_SERVER_4 5004


class GreenMonster : public TGMainFrame {
  RQ_OBJECT()

  ClassDef(GreenMonster, 0) 

public:
  GreenMonster(const TGWindow *p, UInt_t w, UInt_t h);
  ~GreenMonster();

  Bool_t ProcessMessage( Long_t msg, Long_t parm1, Long_t parm2);
  void BMWDelayCheck();
  void BMWTestStep();

  void Init();

private:
  Int_t mainWidth;
  Int_t mainHeight;
  //  const static ULong_t grnback = 0x8fbc8f;  // darkseagreen
  ULong_t grnback;

  Bool_t fUseCrate[4];
  TString* fCrateNames[4];
  Int_t fCrateNumbers[4];

  GreenTB* fTimeBoard[4];

  GreenADC* fADC[4];

  // objects for bmw tabpage
  Int_t fBMW_TABID;
  TGTextButton *changeStatusBtBMW;
  TGTextButton *checkStatusBtBMW;
  TGTextButton *enableTestBtBMW;
  TGTextButton *setKillBtBMW;
  TGTextButton *setValueBtBMW;

  TGLabel *switchLabelBMW;
  TGLabel *activeLabelBMW;
  TGLabel *statusLabelBMW;
  TGRadioButton* fTestObjRBtBMW[8];
  TGTextEntry  *tentSetPntBMW;
  Int_t chosenObjBMW;
  Int_t testSetpointBMW;

  TTimer* _ctimer;
  void BMWDoRadio(Int_t);
  Bool_t BMWCheckStatus();
  void BMWChangeStatus();
  void BMWActiveProbe();
  void BMWCheckActiveFlag();
  
  void BMWSetKill();
  void BMWStartTest();

  void InitGui();

};

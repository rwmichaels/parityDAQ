
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

  void Init();

private:
  Int_t mainWidth;
  Int_t mainHeight;
  //  const static ULong_t grnback = 1034848;
  //  const static ULong_t grnback = 1032288;
  //  const static ULong_t grnback = 1029728;
  //  const static ULong_t grnback = 0x0fb660;
  //  const static ULong_t grnback = 0x80a080;
  const static ULong_t grnback = 0x8fbc8f;  // darkseagreen
  //  const static ULong_t grnback = 0xbc8f8f;  // darkseagreen-mod1


  Bool_t fUseCrate[4];
  TString* fCrateNames[4];
  Int_t fCrateNumbers[4];

  GreenTB* fTimeBoard[4];

  GreenADC* fADC[4];

  // objects for bmw tabpage
  Int_t fBMW_TABID;
  TGTextButton *changeStatusBtBMW;
  TGTextButton *checkStatusBtBMW;
  TGLabel *switchLabelBMW;
  TGLabel *activeLabelBMW;
  TGLabel *statusLabelBMW;
  TTimer* _ctimer;
  Bool_t BMWCheckStatus();
  void BMWChangeStatus();
  void BMWActiveProbe();
  void BMWCheckActiveFlag();

  void InitGui();

};

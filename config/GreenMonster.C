
#include "GreenMonster.h"

ClassImp(GreenMonster)

GreenMonster::GreenMonster(const TGWindow *p, UInt_t w, UInt_t h) : 
  TGMainFrame(p, w, h), mainWidth(w), mainHeight(h)
{
  this->SetBackgroundColor(grnback);
  fUseCrate[0]= kTRUE;
  fCrateNames[0] = new TString("Counting House");
  fCrateNumbers[0] = Crate_CountingHouse;

  fUseCrate[1]= kFALSE;
  fCrateNames[1] = new TString("Left Spect");
  fCrateNumbers[1] = Crate_LeftSpect;

  fUseCrate[2]= kFALSE;
  fCrateNames[2] = new TString("Right Spect");
  fCrateNumbers[2] = Crate_RightSpect;

  fUseCrate[3]= kFALSE;
  fCrateNames[3] = new TString("Injector");
  fCrateNumbers[3] = Crate_Injector;

}

GreenMonster::~GreenMonster() {
};

void GreenMonster::Init() {
  GreenMonster::InitGui();
}

void GreenMonster::InitGui() {

  Int_t killServerCommand[4] = {KILL_SERVER_1, KILL_SERVER_2, 
				 KILL_SERVER_3, KILL_SERVER_4}; 


  TGCompositeFrame *tf;
  TGTextButton *bt;
  TGLayoutHints   *fLayout;

  TGTab *fTab = new TGTab(this, 0, 0);
  fTab->SetBackgroundColor(grnback);

  TGLayoutHints *framout = new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5);

//    // place quit button on bottom of page
  tf  = new TGHorizontalFrame(this,mainWidth-10,mainHeight-10,kFixedWidth);
  tf->SetBackgroundColor(grnback);
//   TGLayoutHints *fLbottom = new TGLayoutHints(
// 	  kLHintsBottom | kLHintsRight | kLHintsExpandX, 2, 2, 4, 2);
  TGLayoutHints *fLbottom = new TGLayoutHints(
	  kLHintsBottom | kLHintsRight, 2, 2, 4, 2);

  TGCompositeFrame* sizebtn = new TGCompositeFrame(tf,150,100,kFixedWidth);
  sizebtn->SetBackgroundColor(grnback);
  tf->AddFrame(sizebtn,fLbottom);
  bt = new TGTextButton(sizebtn,"&QUIT",".q");
  bt->SetBackgroundColor(grnback);
  //  bt->Resize(100,bt->GetDefaultHeight());
  sizebtn->AddFrame(bt,new TGLayoutHints(kLHintsBottom|kLHintsExpandX,5,10,4,2));

  TGIcon *mike = new TGIcon(tf,gClient->GetPicture("config/gm.xpm"), 0,0);
  mike->SetBackgroundColor(grnback);
  tf->AddFrame(mike,new TGLayoutHints(kLHintsBottom|kLHintsLeft,5,10,4,2));

  bt->Resize(100,bt->GetDefaultHeight());
  //  tf->Resize(tf->GetDefaultWidth(), mike->GetDefaultHeight());

  TGLayoutHints *fL_Bot_Right = new TGLayoutHints(kLHintsBottom | 
						  kLHintsRight, 2, 2, 5, 1);
  this->AddFrame(tf,fL_Bot_Right);

  //
  // create first page, Timeboard
  //

  tf = fTab->AddTab("TimeBoard");
  TGTabElement *tabel = fTab->GetTabTab(0);
  tabel->ChangeBackground(grnback);
  tf->SetLayoutManager(new TGMatrixLayout(tf, 2, 2, 10));
  tf->SetBackgroundColor(grnback);

  TGCompositeFrame* tfnocrate;
  TGCompositeFrame* tfnocrate2;
  for (Int_t i=0; i<4; i++) {
    if (fUseCrate[i]) {
      fTimeBoard[i] = new GreenTB(fCrateNumbers[i],fCrateNames[i]->Data(),
				 tf, 350, 200);
      fTimeBoard[i]->Init(grnback);
      tf->AddFrame(fTimeBoard[i],framout);
    } else {
      tfnocrate = new TGCompositeFrame(tf,350,200);
      tfnocrate->SetBackgroundColor(grnback);
      tf->AddFrame(tfnocrate,framout);
      tfnocrate2 = new TGGroupFrame(tfnocrate,fCrateNames[i]->Data(),
				   kHorizontalFrame);
      tfnocrate2->SetBackgroundColor(grnback);
      tfnocrate->AddFrame(tfnocrate2,framout);
    }
  }  
  //
  //


//    // create BMW page
  tf = fTab->AddTab("BMW");
  fBMW_TABID = fTab->GetNumberOfTabs()-1;  // set tab index for later use
  tf->SetBackgroundColor(grnback);
  tf->SetLayoutManager(new TGHorizontalLayout(tf));
  TGCompositeFrame *tcfbmw = new TGGroupFrame(tf, "Beam Modulation", 
					      kHorizontalFrame);
  tcfbmw->SetBackgroundColor(grnback);
  tf->AddFrame(tcfbmw,framout);
  tabel = fTab->GetTabTab(fBMW_TABID);
  tabel->ChangeBackground(grnback);

  // create server page
  tf = fTab->AddTab("VXWorks Server");
  tf->SetBackgroundColor(grnback);
  tf->SetLayoutManager(new TGHorizontalLayout(tf));
  TGLayoutHints *noex = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 5, 5);
  TGCompositeFrame *tcf4a = new TGCompositeFrame(tf, 300, 400, 
						 kVerticalFrame|kFixedWidth);
  tcf4a->SetBackgroundColor(grnback);
  //tcf4a->SetBackgroundColor(dgreen);
  tf->AddFrame(tcf4a,noex);
  tabel = fTab->GetTabTab(2);
  tabel->ChangeBackground(grnback);

//    // create test page
//    tf = fTab->AddTab("Test Page");
//    tf->SetBackgroundColor(grnback);
//    tf->SetLayoutManager(new TGHorizontalLayout(tf));
//    TGCompositeFrame *tcf3a = new TGCompositeFrame(tf, 300, 300, kVerticalFrame);
//    tcf3a->SetBackgroundColor(grnback);
//    tf->AddFrame(tcf3a,framout);
//    tabel = fTab->GetTabTab(3);
//    tabel->ChangeBackground(grnback);


//----------------------------------------------------------------------
  // add buttons to BMW page
  // tcfbmw has a matrix layout with 2 columns: put label and buttons
  // side by side...
  TGMatrixLayout *fMatLay = new TGMatrixLayout(tcfbmw, 2, 3, 10, 10);
  tcfbmw->SetLayoutManager(fMatLay);

  switchLabelBMW = new TGLabel(tcfbmw,"Kill switch is OFF");
  switchLabelBMW->SetBackgroundColor(grnback);
  tcfbmw->AddFrame(switchLabelBMW);
  tcfbmw->AddFrame(checkStatusBtBMW = new TGTextButton(tcfbmw,"Check Status",
							GM_BMW_CHECK));
  checkStatusBtBMW->Associate(this);
  checkStatusBtBMW->SetBackgroundColor(grnback);

  tcfbmw->AddFrame(statusLabelBMW = 
		   new TGLabel(tcfbmw,"Beam Modulation is OFF"));
  statusLabelBMW->SetBackgroundColor(grnback);
  changeStatusBtBMW = new TGTextButton(tcfbmw,"Start Beam Modulation",
  				       GM_BMW_CHANGE);
  changeStatusBtBMW->Associate(this);
  changeStatusBtBMW->SetBackgroundColor(grnback);
  tcfbmw->AddFrame(changeStatusBtBMW);

  activeLabelBMW = new TGLabel(tcfbmw,"Beam Modulation script is INACTIVE");
  activeLabelBMW->SetBackgroundColor(grnback);
  tcfbmw->AddFrame(activeLabelBMW);

  BMWCheckStatus();
  BMWActiveProbe();

//----------------------------------------------------------------------
  // add buttons to Control page
  fLayout = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY | 
			      kLHintsExpandX,2,2,5,5);
  for (Int_t i = 0; i<4; i++) {
    if (fUseCrate[i]) {
      TString tmpstr = "Kill VXWorks Server, ";
      tmpstr += fCrateNames[i]->Data();
      tcf4a->AddFrame(bt = new TGTextButton(tcf4a,tmpstr.Data(),
					    killServerCommand[i]),
		      fLayout);
      bt->SetBackgroundColor(grnback);
      bt->Associate(this);
    }
  } 

//  //----------------------------------------------------------------------
//  // Add top composite frame to the main frame (this)
  AddFrame(fTab,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,0,0,0,0));
  MapSubwindows();
  Layout();
  SetWindowName("Configuration Utility");
  SetIconName("GM");
  MapWindow();
}


Bool_t GreenMonster::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
  int junk;
  // Process events generated by the buttons in the frame
    switch (GET_MSG(msg)) {
    case kC_COMMAND:
      switch (GET_SUBMSG(msg)) {
      case kCM_BUTTON:
	switch (parm1) 
	  {
	  case 1:
	    {
	      int command = 10,par_1 =1,par_2 =2, command_type = 10;
	      char *msgReq = "One";
	      char *reply = "Y";

	      //junk = cfSockCommand(command,par_1,par_2,reply,msgReq);
	      struct greenRequest gRequest;
	      strcpy(gRequest.message,msgReq);
	      gRequest.reply = reply;
	      gRequest.command_type = command_type;
	      gRequest.command = command;
	      gRequest.par1 = par_1;
	      gRequest.par2 = par_2;
	      junk = GreenSockCommand(Crate_CountingHouse,&gRequest);

	      printf ("cfSockCommand returned :  %d \n",junk);
	      command_type = gRequest.command_type;
	      command = gRequest.command;
	      par_1 = gRequest.par1;
	      par_2 = gRequest.par2;
   	      if(*reply=='Y' || *reply=='y') {
  		printf ("GM: MESSAGE FROM SERVER:\n%s\n", msgReq);
  		printf("Server reply command: %d \n",command);
  		printf("Server reply param_1: %d \n",par_1);
  		printf("Server reply param_2: %d \n",par_2);
  	      }

	    }
	    break;
	  case 2:
	    {
	      TExec *ex = new TExec("ex",".q");
	      ex->Exec();
	    }
	    break;
	  case KILL_SERVER_1:
	  case KILL_SERVER_2:
	  case KILL_SERVER_3:
	  case KILL_SERVER_4:
	    {
	      int command = 10,par_1 =1,par_2 =2, command_type = 10;
	      char *msgReq = "Q";
	      char *reply = "N";
	      junk = cfSockCommand(fCrateNumbers[parm1-KILL_SERVER_1],
				   command_type,command,par_1,par_2,
				   reply,msgReq);
	      //	    printf ("cfSockCommand returned :  %d \n",junk);
	      msgReq = "Q";
	      reply = "N";
	      junk = cfSockCommand(fCrateNumbers[parm1-KILL_SERVER_1],
				   command_type,command,par_1,par_2,
				   reply,msgReq);
	      break;
	    }
	  case GM_BMW_CHANGE:
	    {
	      //	      printf("changing status of BMW \n");
	      BMWChangeStatus();
	      break;
	    }
	  case GM_BMW_CHECK:	
    {
	      //	      printf("checking status of BMW \n");
	      BMWCheckStatus();
	      BMWActiveProbe();
	      break;
	    }
	  default:
	    printf("text button id %ld pressed\n", parm1);
	    printf("text button context %ld \n", parm2);
	    break;
	  }
	break;
      case kCM_TAB: {
	if (parm1==fBMW_TABID) {
	  // check status of BMW every time the tab is chosen
	  //	  printf("clicked bmw tab \n");
	  BMWCheckStatus();
	  BMWActiveProbe();	  
	}	  
	break;
      }
//       case kCM_CHECKBUTTON:
// 	printf("check botton id %ld pressed\n",parm1);
// 	break;
//       case kCM_RADIOBUTTON:
// 	break;
      default:
 	break;
      }
    default:
      break;
    }
  return kTRUE;
}
    
void GreenMonster::BMWChangeStatus() {
  struct greenRequest gRequest;
  int command, par1, par2, command_type;
  char *msgReq = "BMW Status Change";
  char *reply = "Y";
  // get bmw status
  Bool_t bmw_running = BMWCheckStatus();

  command_type = COMMAND_BMW;   gRequest.command_type = command_type;
  if (bmw_running) {
    // kill bmw
    command = BMW_KILL;          gRequest.command = command;
  } else {
    // start bmw
    command = BMW_START;          gRequest.command = command;
  }
  printf("changing status of bmw client\n");
  par1 = 0;                     gRequest.par1 = par1;
  par2 = 0;                     gRequest.par2 = par2;

  strcpy(gRequest.message,msgReq);   gRequest.reply = reply;
  if (GreenSockCommand(Crate_CountingHouse,&gRequest) == SOCK_OK) {
    printf("bmw status change call is complete\n");
  } else {
    printf("ERROR accessing socket!");
    return;
  }
  // check bmw status
    BMWCheckStatus();
}

Bool_t GreenMonster::BMWCheckStatus() {
  // get bmw status
  struct greenRequest gRequest;
  int command, par1, par2, command_type;
  char *msgReq = "BMW status check";
  char *reply = "Y";
  Bool_t bmw_running,kill_switch;
  
  //  printf("BMW Status =");
  command_type = COMMAND_BMW;    gRequest.command_type = command_type;
  command = BMW_GET_STATUS;      gRequest.command = command;
  par1 = 0;                      gRequest.par1 = par1;
  par2 = 0;                      gRequest.par2 = par2;
  strcpy(gRequest.message,msgReq);   gRequest.reply = reply;
  if (GreenSockCommand(Crate_CountingHouse,&gRequest) == SOCK_OK) {
    command = gRequest.command;
    par1 = gRequest.par1;
    par2 = gRequest.par2;
    if (par1 != 0) {
      bmw_running = kTRUE;
    } else { 
      bmw_running = kFALSE;
    }
    if (par2 != 0) {
      kill_switch = kTRUE;
    } else { 
      kill_switch = kFALSE;
    }
  } else {
    printf("ERROR accessing socket!");
    return kFALSE;
  }

  //  printf("% d  %d\n",par1,par2);

  if (kill_switch) {
    switchLabelBMW->SetText("Kill Switch is ON");
  } else {
    switchLabelBMW->SetText("Kill Switch is OFF");
  }
  if (bmw_running) {
    statusLabelBMW->SetText("Beam Modulation is ON");
    changeStatusBtBMW->SetText("Set Kill Switch");
    return kTRUE;
  } else {
    statusLabelBMW->SetText("Beam Modulation is OFF");
    changeStatusBtBMW->SetText("Start Beam Modulation");
    return kFALSE;
  }
  MapSubwindows();
}

void GreenMonster::BMWDelayCheck() {
  BMWCheckActiveFlag();
}

void GreenMonster::BMWActiveProbe() {
  // get bmw status
  BMWCheckActiveFlag();
  // recheck after some delay
  //    in order to use timer, or any other QT relying tool, 
  //    one must generate a dictionary using rootcint... 
  TTimer* ctimer = new TTimer(4000,kTRUE);
  TQObject::Connect(ctimer, "Timeout()", "GreenMonster", this, 
	  "BMWDelayCheck");
  ctimer->Start(1000, kTRUE);
}

void GreenMonster::BMWCheckActiveFlag() {
  // get bmw status
  struct greenRequest gRequest;
  int command, par1, par2, command_type;
  char *msgReq = "BMW status check";
  char *reply = "Y";
  Bool_t active=kFALSE;
  
  //  printf("BMWCheckActiveFlag has been called\n");
  command_type = COMMAND_BMW;    gRequest.command_type = command_type;
  command = BMW_CHECK_ALIVE;      gRequest.command = command;
  par1 = 0;                      gRequest.par1 = par1;
  par2 = 0;                      gRequest.par2 = par2;
  strcpy(gRequest.message,msgReq);   gRequest.reply = reply;
  if (GreenSockCommand(Crate_CountingHouse,&gRequest) == SOCK_OK) {
    par2 = gRequest.par2;
    if (par2 != 0) {
      active = kTRUE;
    } else { 
      active = kFALSE;
    }
  } else {
    printf("ERROR accessing socket!"); 
  }

  if (active) {
    activeLabelBMW->SetText("Beam Modulation script is ACTIVE");
  } else {
    activeLabelBMW->SetText("Beam Modulation script is INACTIVE");
  }
  MapSubwindows();
}





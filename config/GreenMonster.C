
#include "GreenMonster.h"

ClassImp(GreenMonster)

GreenMonster::GreenMonster(const TGWindow *p, UInt_t w, UInt_t h) : 
  TGMainFrame(p, w, h), mainWidth(w), mainHeight(h)
{
  //  grnback = gROOT->GetColor(31)->GetPixel();  // maybe try this if
  // a modern version of root gets installed onto adaqcp

  gClient->GetColorByName("darkseagreen",grnback);
  ULong_t white;
  gClient->GetColorByName("white",white);
  if (grnback==white)   gClient->GetColorByName("green",grnback);

  this->SetBackgroundColor(grnback);
  fUseCrate[0]= kTRUE;
  fCrateNames[0] = new TString("Counting House");
  fCrateNumbers[0] = Crate_CountingHouse;

  fUseCrate[1]= kTRUE;
  fCrateNames[1] = new TString("Left Spect");
  fCrateNumbers[1] = Crate_LeftSpect;

  fUseCrate[2]= kTRUE;
  fCrateNames[2] = new TString("Right Spect");
  fCrateNumbers[2] = Crate_RightSpect;

  fUseCrate[3]= kTRUE;
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

  TGPictureButton* mike = new TGPictureButton(tf,
					      gClient->GetPicture("gm.xpm"));
  //  TGIcon *mike = new TGIcon(tf,gClient->GetPicture("config/gm.xpm"), 0,0);
  mike->SetBackgroundColor(grnback);
  mike->SetToolTipText("grrr...");
  //  new TGToolTip(tf,mike,"grrr...",1000);
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
  TGLayoutHints *addTBLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5);
  for (Int_t i=0; i<4; i++) {
    if (fUseCrate[i]) {
      fTimeBoard[i] = new GreenTB(fCrateNumbers[i],fCrateNames[i]->Data(),
				 tf, 350, 200);
      fTimeBoard[i]->Init(grnback);
      tf->AddFrame(fTimeBoard[i],addTBLayout);
    } else {
      tfnocrate = new TGCompositeFrame(tf,350,200,kFixedSize);
      tfnocrate->SetBackgroundColor(grnback);
      tf->AddFrame(tfnocrate,addTBLayout);
      tfnocrate2 = new TGGroupFrame(tfnocrate,fCrateNames[i]->Data(),
				   kHorizontalFrame | kFixedSize);
      tfnocrate2->SetBackgroundColor(grnback);
      tfnocrate->AddFrame(tfnocrate2,addTBLayout);
    }
  }  
  //
  //


  //
  // create BMW page
  //
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



  //
  // create third page, ADCs
  //
  char buff[15];
  for (Int_t i=0; i<4; i++) {
    if (fUseCrate[i]) {
      sprintf(buff,"ADC, Crate %d",i);
      tf = fTab->AddTab(buff);
      tabel = fTab->GetTabTab(fTab->GetNumberOfTabs()-1);  // get tab index
      tabel->ChangeBackground(grnback);
      tf->SetBackgroundColor(grnback);
      //  tf->SetLayoutManager(new TGMatrixLayout(tf, 2, 2, 10));
      //      tf->SetLayoutManager(new TGVerticalLayout(tf));
      fADC[i] = new GreenADC(fCrateNumbers[i],fCrateNames[i]->Data(),
				 tf, 350, 200);
      fADC[i]->Init(grnback);
      tf->AddFrame(fADC[i],framout);
    } 
  }  
  //
  //


  //
  // create server page
  //
  tf = fTab->AddTab("VXWorks Server");
  tabel = fTab->GetTabTab(fTab->GetNumberOfTabs()-1);  // get tab index
  tabel->ChangeBackground(grnback);
  tf->SetBackgroundColor(grnback);
  tf->SetLayoutManager(new TGHorizontalLayout(tf));
  TGLayoutHints *noex = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 5, 5);
  TGCompositeFrame *tcf4a = new TGCompositeFrame(tf, 300, 400, 
						 kVerticalFrame|kFixedWidth);
  tcf4a->SetBackgroundColor(grnback);
  //tcf4a->SetBackgroundColor(dgreen);
  tf->AddFrame(tcf4a,noex);

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

  tcfbmw->SetLayoutManager(new TGVerticalLayout(tcfbmw));

  TGCompositeFrame *tcfbmw1 = new TGGroupFrame(tcfbmw, "Beam Modulation Script", 
					      kHorizontalFrame);
  tcfbmw1->SetBackgroundColor(grnback);
  tcfbmw->AddFrame(tcfbmw1,framout);

  TGCompositeFrame *tcfbmw_t = new TGGroupFrame(tcfbmw, "Beam Modulation- TEST", 
					      kVerticalFrame);
  tcfbmw_t->SetBackgroundColor(grnback);
  tcfbmw->AddFrame(tcfbmw_t,framout);



  TGMatrixLayout *fMatLay = new TGMatrixLayout(tcfbmw1, 2, 3, 10, 10);
  tcfbmw1->SetLayoutManager(fMatLay);

  switchLabelBMW = new TGLabel(tcfbmw1,"Kill switch is OFF");
  switchLabelBMW->SetBackgroundColor(grnback);
  tcfbmw1->AddFrame(switchLabelBMW);
  tcfbmw1->AddFrame(checkStatusBtBMW = new TGTextButton(tcfbmw1,"Check Status",
							GM_BMW_CHECK));
  checkStatusBtBMW->Associate(this);
  checkStatusBtBMW->SetBackgroundColor(grnback);

  tcfbmw1->AddFrame(statusLabelBMW = 
		   new TGLabel(tcfbmw1,"Beam Modulation is OFF"));
  statusLabelBMW->SetBackgroundColor(grnback);
  changeStatusBtBMW = new TGTextButton(tcfbmw1,"Start Beam Modulation",
  				       GM_BMW_CHANGE);
  changeStatusBtBMW->Associate(this);
  changeStatusBtBMW->SetBackgroundColor(grnback);
  tcfbmw1->AddFrame(changeStatusBtBMW);

  activeLabelBMW = new TGLabel(tcfbmw1,"Beam Modulation script is INACTIVE");
  activeLabelBMW->SetBackgroundColor(grnback);
  tcfbmw1->AddFrame(activeLabelBMW);

  TGCompositeFrame *tmpfrmB = new TGHorizontalFrame(tcfbmw_t,500,100);
  tmpfrmB->SetBackgroundColor(grnback);
  tcfbmw_t->AddFrame(tmpfrmB);
  tmpfrmB->SetLayoutManager(new TGMatrixLayout(tmpfrmB, 1, 0, 10));

  tmpfrmB->AddFrame(enableTestBtBMW = 
	       new TGTextButton(tmpfrmB,"Enable BMW Test",GM_BMW_TEST_ENABLE));
  enableTestBtBMW->Associate(this);
  enableTestBtBMW->SetBackgroundColor(grnback);
  tmpfrmB->AddFrame(setKillBtBMW = 
	       new TGTextButton(tmpfrmB,"Toggle Kill Switch",GM_BMW_SET_KILL));
  setKillBtBMW->Associate(this);
  setKillBtBMW->SetBackgroundColor(grnback);

//    tmpfrmB->AddFrame(setValueBtBMW = 
//  	       new TGTextButton(tmpfrmB,"Apply Value",GM_BMW_TEST_SET_VALUE));
//    setValueBtBMW->Associate(this);
//    setValueBtBMW->SetBackgroundColor(grnback);

  TGCompositeFrame *tmpfrm = new TGHorizontalFrame(tcfbmw_t,500,100);
  tmpfrm->SetLayoutManager(new TGMatrixLayout(tmpfrm, 0, 4));
  tmpfrm->SetBackgroundColor(grnback);
  tcfbmw_t->AddFrame(tmpfrm);
  fTestObjRBtBMW[0] = new TGRadioButton(tmpfrm,"MAT1C01H ",BMW_OBJRADIO1);
  fTestObjRBtBMW[1] = new TGRadioButton(tmpfrm,"MAT1C02V ",BMW_OBJRADIO2);
  fTestObjRBtBMW[2] = new TGRadioButton(tmpfrm,"MAT1C03H ",BMW_OBJRADIO3);
  fTestObjRBtBMW[3] = new TGRadioButton(tmpfrm,"MAT1C04H ",BMW_OBJRADIO4);
  fTestObjRBtBMW[4] = new TGRadioButton(tmpfrm,"MAT1C05V ",BMW_OBJRADIO5);
  fTestObjRBtBMW[5] = new TGRadioButton(tmpfrm,"MAT1C06H ",BMW_OBJRADIO6);
  fTestObjRBtBMW[6] = new TGRadioButton(tmpfrm,"MAT1C07V ",BMW_OBJRADIO7);
  fTestObjRBtBMW[7] = new TGRadioButton(tmpfrm,"SL Zone 20",BMW_OBJRADIO8);
  for (Int_t ib = 0; ib<8; ib++) {
    fTestObjRBtBMW[ib]->SetBackgroundColor(grnback);
    fTestObjRBtBMW[ib]->Associate(this);
    tmpfrm->AddFrame(fTestObjRBtBMW[ib]);
  }
  chosenObjBMW = 0;
  fTestObjRBtBMW[0]->SetState(kButtonDown);

  TGCompositeFrame *tmpfrmv = new TGHorizontalFrame(tcfbmw_t,500,100);
  tmpfrmv->SetBackgroundColor(grnback);
  tcfbmw_t->AddFrame(tmpfrmv);

  //  tmpfrmv->SetLayoutManager(new TGMatrixLayout(tmpfrm, 0, 2));
  sprintf(buff, "0");
  TGTextBuffer *tbuf = new TGTextBuffer(6);
  tbuf->AddText(0,buff);
  tentSetPntBMW = new TGTextEntry(tmpfrmv, tbuf, 221);
  tentSetPntBMW->Resize(60, tentSetPntBMW->GetDefaultHeight());
  tentSetPntBMW->SetFont("-adobe-courier-bold-r-*-*-14-*-*-*-*-*-iso8859-1");
  tmpfrmv->AddFrame(tentSetPntBMW);
  TGLabel * lab= new TGLabel(tmpfrmv,new TGHotString("Set Point (mA or keV)"));
  tmpfrmv->AddFrame(lab);
  lab->SetBackgroundColor(grnback);

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
      bt->SetToolTipText("Kill the Socket Server running on this crate");
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
	  case GM_BMW_SET_KILL:
	    {
	      BMWSetKill();
	      break;
	    }
	  case GM_BMW_TEST_ENABLE:
	    {
	      BMWStartTest();
	      break;
	    }
	  default:
	    printf("text button id %ld pressed\n", parm1);
	    printf("text button context %ld \n", parm2);
	    break;
	  }
	break;
      case kCM_RADIOBUTTON: {
	  case BMW_OBJRADIO1:
	  case BMW_OBJRADIO2:
	  case BMW_OBJRADIO3:
	  case BMW_OBJRADIO4:
	  case BMW_OBJRADIO5:
	  case BMW_OBJRADIO6:
	  case BMW_OBJRADIO7:
	  case BMW_OBJRADIO8:
	    {
	      //	      cout << "radio button pushed " << parm1 << endl;
	      BMWDoRadio(parm1);
	      break;
	    }
      }
      case kCM_TAB: {
	if (parm1==fBMW_TABID) {
	  // check status of BMW every time the tab is chosen
	  //	  printf("clicked bmw tab \n");
	  BMWCheckStatus();
	  BMWActiveProbe();	  
	}	  
	break;
      }
      default:
 	break;
      }
    default:
      break;
    }
  return kTRUE;
}

void GreenMonster::BMWDoRadio(Int_t id) {    
  fTestObjRBtBMW[chosenObjBMW]->SetState(kButtonUp);
  chosenObjBMW = id - BMW_OBJRADIO1;
  fTestObjRBtBMW[chosenObjBMW]->SetState(kButtonDown);
  //  cout << "New BMW test object chosen : " << chosenObjBMW << endl;
  return;
}

void GreenMonster::BMWStartTest() {    
  struct greenRequest gRequest;
  int command, command_type, par1, par2;
  char *msgReq = "BMW Start Test";
  char *reply = "Y";

  //
  command_type = COMMAND_BMW;    gRequest.command_type = command_type;
  command = BMW_TEST_START;      gRequest.command = command;
  par1 = 0;                      gRequest.par1 = par1;
  par2 = 0;                      gRequest.par2 = par2;
  strcpy(gRequest.message,msgReq);   gRequest.reply = reply;
  if (GreenSockCommand(Crate_CountingHouse,&gRequest) == SOCK_OK) {
    command = gRequest.command;
  } else {
    printf("BMWStartTest::ERROR accessing socket!"); 
    return;
  }

  // do first test step
  BMWTestStep();

}

void GreenMonster::BMWTestStep() {    
  int value;
  Bool_t kill_switch;

  struct greenRequest gRequest;
  int command, command_type, par1, par2;
  char *msgReq = "BW Test Set Data";
  char *reply = "Y";

  //
  // get value from tent
  //
  value = atoi(tentSetPntBMW->GetText());
  cout << " writing new set point " << value << " to " << chosenObjBMW << endl;
  //
  // send set message for obj, value
  //
  command_type = COMMAND_BMW;    gRequest.command_type = command_type;
  command = BMW_TEST_SET_DATA;   gRequest.command = command;
  par1 = chosenObjBMW;           gRequest.par1 = par1;
  par2 = value;                  gRequest.par2 = par2;
  strcpy(gRequest.message,msgReq);   gRequest.reply = reply;

  if (GreenSockCommand(Crate_CountingHouse,&gRequest) == SOCK_OK) {
    command = gRequest.command;
    par1 = gRequest.par1;
    par2 = gRequest.par2;
    // par1 is kill switch!
    if (par1 != 0) {
      kill_switch = kTRUE;
    } else {
      kill_switch = kFALSE;
    }
  } else {
    printf("ERROR accessing socket!"); 
    return;
  }
  
//    //
//    // check for kill switch,
//    //
//    command_type = COMMAND_BMW;    gRequest.command_type = command_type;
//    command = BMW_GET_STATUS;      gRequest.command = command;
//    par1 = 0;                      gRequest.par1 = par1;
//    par2 = 0;                      gRequest.par2 = par2;
//    strcpy(gRequest.message,msgReq);   gRequest.reply = reply;
//    if (GreenSockCommand(Crate_CountingHouse,&gRequest) == SOCK_OK) {
//      command = gRequest.command;
//      par1 = gRequest.par1;
//      par2 = gRequest.par2;
//      if (par2 != 0) {
//        kill_switch = kTRUE;
//      } else { 
//        kill_switch = kFALSE;
//      }
//    } else {
//      printf("ERROR accessing socket!"); 
//      return;
//    }

  //
  //if none, set timer to self
  //
  if (!kill_switch) {    
    TTimer* ctimer = new TTimer(4000,kTRUE);
    TQObject::Connect(ctimer, "Timeout()", "GreenMonster", this, 
		      "BMWTestStep");
    ctimer->Start(4000, kTRUE);
  }

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


void GreenMonster::BMWSetKill() {
  struct greenRequest gRequest;
  int command, par1, par2, command_type;
  char *msgReq = "BMW status check";
  char *reply = "Y";
  Bool_t bmw_running,kill_switch;

  // get bmw status
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
    return;
 }

  if (!kill_switch) {
    command_type = COMMAND_BMW;   gRequest.command_type = command_type;
    char *msgReq = "BMW set kill";
    command = BMW_KILL;          gRequest.command = command;
    printf("setting kill switch on bmw\n");
    par1 = 0;                     gRequest.par1 = par1;
    par2 = 0;                     gRequest.par2 = par2;
    
    strcpy(gRequest.message,msgReq);   gRequest.reply = reply;
    if (GreenSockCommand(Crate_CountingHouse,&gRequest) == SOCK_OK) {
      printf("bmw kill switch call is complete\n");
    } else {
      printf("ERROR accessing socket!");
      return;
    }
  } else {
    command_type = COMMAND_BMW;   gRequest.command_type = command_type;
    char *msgReq = "BMW lift kill";
    command = BMW_UNKILL;          gRequest.command = command;
    printf("lifting kill switch on bmw\n");
    par1 = 0;                     gRequest.par1 = par1;
    par2 = 0;                     gRequest.par2 = par2;
    
    strcpy(gRequest.message,msgReq);   gRequest.reply = reply;
    if (GreenSockCommand(Crate_CountingHouse,&gRequest) == SOCK_OK) {
      printf("bmw un-kill switch call is complete\n");
    } else {
      printf("ERROR accessing socket!");
      return;
    }
  }

  BMWCheckStatus();  // check bmw status
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





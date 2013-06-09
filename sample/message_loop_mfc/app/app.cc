// codejock_dlg.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#include "app/app.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "base/at_exit.h"

#include "ui/main_frame.h"


BEGIN_MESSAGE_MAP(App, CWinApp)
  ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


base::AtExitManager g_atExitMamager;

App::App() {}

App theApp;

BOOL App::InitInstance() {
  // InitCommonControlsEx() is required on Windows XP if an application
  // manifest specifies use of ComCtl32.dll version 6 or later to enable
  // visual styles.  Otherwise, any window creation will fail.
  INITCOMMONCONTROLSEX InitCtrls;
  InitCtrls.dwSize = sizeof(InitCtrls);
  // Set this to include all the common control classes you want to use
  // in your application.
  InitCtrls.dwICC = ICC_WIN95_CLASSES;
  InitCommonControlsEx(&InitCtrls);

  CWinApp::InitInstance();

  // Initialize OLE libraries
  if (!AfxOleInit()) {
    AfxMessageBox(IDP_OLE_INIT_FAILED);
    return FALSE;
  }
  AfxEnableControlContainer();
  // Standard initialization
  // If you are not using these features and wish to reduce the size
  // of your final executable, you should remove from the following
  // the specific initialization routines you do not need
  // Change the registry key under which our settings are stored
  // TODO: You should modify this string to be something appropriate
  // such as the name of your company or organization
  SetRegistryKey(_T("Local AppWizard-Generated Applications"));
  // To create the main window, this code creates a new frame window
  // object and then sets it as the application's main window object
  MainFrame* pFrame = new MainFrame;
  if (!pFrame)
    return FALSE;
  m_pMainWnd = pFrame;
  // create and load the frame with its resources
  pFrame->LoadFrame(IDR_MAINFRAME);

  // The one and only window has been initialized, so show and update it
  pFrame->ShowWindow(SW_NORMAL);
  pFrame->UpdateWindow();
  // call DragAcceptFiles only if there's a suffix
  //  In an SDI app, this should occur after ProcessShellCommand

  using namespace logging;

  CommandLine::Init(0, NULL);
  InitLogging(L"xtp.log", 
    LOG_TO_BOTH_FILE_AND_SYSTEM_DEBUG_LOG,
    LOCK_LOG_FILE,
    DELETE_OLD_LOG_FILE,
    DISABLE_DCHECK_FOR_NON_OFFICIAL_RELEASE_BUILDS);

  VLOG(1) << "log init";

  return TRUE;
}

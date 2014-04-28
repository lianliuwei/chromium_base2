#include "stdafx.h"

#include "ui/main_frame.h"

#include "base/logging.h"

#include "ui/test_view.h"
#include "resources/resource.h"

namespace {
static const int frame_height = 500;
static const int frame_width = 640;
}

using namespace std;

IMPLEMENT_DYNAMIC(MainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(MainFrame, CFrameWnd)
  ON_WM_CREATE()
  ON_WM_CLOSE()
END_MESSAGE_MAP()

MainFrame::MainFrame() {}


MainFrame::~MainFrame() {}

int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) {
  if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
    return -1;

  test_view_ = new TestView();
  if (!test_view_->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, 
                          CRect(0,0,frame_width, frame_height), 
                          this, ID_VIEW_TEST, NULL)) {
      return -1;
  }

  return 0;
}

BOOL MainFrame::PreCreateWindow(CREATESTRUCT& cs) {
  if( !CFrameWnd::PreCreateWindow(cs) )
    return FALSE;

  cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE | WS_THICKFRAME
      | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU;

  cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
  cs.lpszClass = AfxRegisterWndClass(0);
  cs.style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
  cs.cx = frame_width;
  cs.cy = frame_height;
  return TRUE;
}

void MainFrame::OnClose() {
  CFrameWnd::OnClose();
}

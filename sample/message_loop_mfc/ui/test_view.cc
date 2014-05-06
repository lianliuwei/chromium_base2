#include "stdafx.h"

#include "ui/test_view.h"

#include "base/bind.h"

#include "base_ex/message_loop_mfc.h"

using namespace base;

BEGIN_MESSAGE_MAP(TestView, CFormView)
  ON_BN_CLICKED(IDC_BUTTON_START, OnButtonStart)
  ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
END_MESSAGE_MAP()

void TestView::DoDataExchange(CDataExchange* pDX) {
  CFormView::DoDataExchange(pDX);
  
  DDX_Control(pDX, IDC_LIST_RESULT, result_);
  DDX_Control(pDX, IDC_BUTTON_START, start_);
  DDX_Control(pDX, IDC_BUTTON_STOP, stop_);
  InitImpl();
}

void TestView::Init() {
  stop_.EnableWindow(FALSE);
  start_.EnableWindow(TRUE);
  count_ = 0;

  InitMainThread();
  CreateTestThread();
}

TestView::TestView()
  : FormViewEx(IDD) {}

TestView::~TestView() {
  DestoryAllThread();
}

void TestView::OnButtonStart() {
  stop_.EnableWindow(TRUE);
  start_.EnableWindow(FALSE);

  PostStartTask();
}

void TestView::OnButtonStop() {
  stop_.EnableWindow(FALSE);
  start_.EnableWindow(TRUE);
}

void TestView::PostStartTask() {
  count_ = 0;
  test_thread()->message_loop_proxy()->PostTask(FROM_HERE, 
      Bind(&TestView::IncCount, Unretained(this)));
}

bool TestView::IsContinue() {
  return !start_.IsWindowEnabled();
}

void TestView::IncCount() {
  ++count_;
  ui_thread()->message_loop_proxy()->PostTask(FROM_HERE, 
      Bind(&TestView::UpdateTestUI, Unretained(this), count_));
}

void TestView::UpdateTestUI(int count) {
  if (!IsContinue())
    return;

  CString text;
  text.Format(_T("Count: %d"), count);
  int index = result_.AddString(text);
  result_.SetTopIndex(index);

  test_thread()->message_loop_proxy()->PostDelayedTask(FROM_HERE, 
      Bind(&TestView::IncCount, Unretained(this)), 
      TimeDelta::FromMilliseconds(250));
}

void TestView::InitMainThread() {
  ui_thread_.reset(new MFCThread);
  ui_thread_->StartThread();
}

void TestView::CreateTestThread() {
  DCHECK(test_thread_.get() == NULL);
  test_thread_.reset(new Thread("test"));
  if (!test_thread_->Start())
    NOTREACHED() << "thread must create";
}

base::Thread* TestView::ui_thread() {
  DCHECK(test_thread_.get()) << "must create first";
  return ui_thread_.get();
}

base::Thread* TestView::test_thread() {
  DCHECK(test_thread_.get()) << "must create first";
  return test_thread_.get();
}

void TestView::DestoryAllThread() {
  test_thread_->Stop();
  test_thread_.reset();
  ui_thread_->StopThread();
  ui_thread_.reset();
}

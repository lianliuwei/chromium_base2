#include "ui/form_view_ex.h"

#include "base/threading/thread.h"
#include "common/mfc_thread.h"

#include "resources/resource.h"

class TestView : public FormViewEx {
public:
  TestView();
  virtual ~TestView();

private:
  enum { IDD = IDD_TESTVIEW };

  virtual void DoDataExchange(CDataExchange* pDX);

  // implement FormViewEx
  virtual void Init();

  afx_msg void OnButtonStart();
  afx_msg void OnButtonStop();

  bool IsContinue();
  void PostStartTask();
  void IncCount();
  void UpdateTestUI(int count);

  base::Thread* ui_thread();
  base::Thread* test_thread();

  void InitMainThread();
  void CreateTestThread();

  void DestoryAllThread();

  CListBox result_;
  CButton start_;
  CButton stop_;

  int count_;

  scoped_ptr<base::Thread> test_thread_;
  scoped_ptr<MFCThread> ui_thread_;
  scoped_ptr<base::MessageLoop> main_message_loop_;

  DECLARE_MESSAGE_MAP()

};

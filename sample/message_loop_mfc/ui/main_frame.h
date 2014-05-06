#pragma once

class TestView;

class MainFrame : public CFrameWnd {
public:
  MainFrame();
  virtual ~MainFrame();

private:
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnClose();
  DECLARE_MESSAGE_MAP()

private:
  TestView* test_view_;

  DECLARE_DYNAMIC(MainFrame)
};

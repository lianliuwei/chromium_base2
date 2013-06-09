#pragma once

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
  DECLARE_DYNAMIC(MainFrame)
};

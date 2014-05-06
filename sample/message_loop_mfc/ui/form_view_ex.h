#pragma once

// the right way to using DIALOG template is using the CFormView.
// the right way to using CFormView is to using the control value.
// see http://www.flounder.com/updatedata.htm for more detail.
// but CFormView lack of the support of this way. no init method to fetch
// from model and update to the control.
// the only position to init the control is in DoDataExchange(), When OnCreate()
// the control are no subclassed.
class FormViewEx : public CFormView {
public:
  FormViewEx(UINT ID)
      : CFormView(ID)
      , init_(false) {}
  virtual ~FormViewEx() {}

  // for direct create.
  // if you when life easy do not use the view-document rubbish. just create
  // views by youself.
  virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, 
    DWORD dwRequestedStyle, const RECT& rect, CWnd* pParentWnd, 
    UINT nID, CCreateContext* pContext) {
      return CFormView::Create(lpszClassName, lpszWindowName, 
        dwRequestedStyle, rect, pParentWnd, nID, pContext);
  }

protected:
  // init the gui there
  // can access the control value.
  virtual void Init() = 0;

  // call after the DDX_Control() in DoDataExchange(), It call Init only once.
  // using like:
  // void XXXView::DoDataExchange(CDataExchange* pDX) {
  //  CFormView::DoDataExchange(pDX);
  //
  //  DDX_Control(pDX, ...);
  //  DDX_Control(pDX, ...);
  //
  //  InitImpl();
  //}
  void InitImpl() {
    if (init_ == false) {
      init_ = true;
      Init();
    }
  }
  bool IsInited() const { return init_; }

private:
  // init GUI trick
  bool init_;
};

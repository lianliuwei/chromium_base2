#include "ui/form_view_ex.h"

#include "resources/resource.h"

class TestView : public FormViewEx {
public:
  TestView();
  virtual ~TestView();

private:
  enum { IDD = IDD_TESTVIEW };

  // implement FormViewEx
  virtual void Init();

  virtual void DoDataExchange(CDataExchange* pDX);

  DECLARE_MESSAGE_MAP()
};

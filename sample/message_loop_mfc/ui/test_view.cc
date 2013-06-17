#include "stdafx.h"

#include "ui/test_view.h"

BEGIN_MESSAGE_MAP(TestView, CFormView)
END_MESSAGE_MAP()

void TestView::DoDataExchange(CDataExchange* pDX) {
  CFormView::DoDataExchange(pDX);

  InitImpl();
}

void TestView::Init() {

}

TestView::TestView()
  : FormViewEx(IDD) {}

TestView::~TestView() {}

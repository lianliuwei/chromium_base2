#pragma once

#include "resources/resource.h" 
#define GDIPLUS_NO_AUTO_INIT
#include "app/GdiplusH.h"

class App : public CWinApp
{
public:
  App();

public:
  virtual BOOL InitInstance();

  Gdiplus::GdiPlusInitialize initer;
	DECLARE_MESSAGE_MAP()
};

extern App theApp;
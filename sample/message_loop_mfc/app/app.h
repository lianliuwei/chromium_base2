#pragma once

#include "resources/resource.h" 

class App : public CWinApp
{
public:
  App();

public:
  virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern App theApp;

#pragma once

#include "base/threading/thread.h"

#include "base_ex/message_loop_mfc.h"

namespace base {
  class MessageLoopProxy;
}

// Special class for the main (UI) thread and unittests. We use a dummy
// thread here since the main thread already exists.
class MFCThread : public base::Thread {
public:
  MFCThread();
  virtual ~MFCThread();

  // we just attach to the MFC ui thread and
  // WARNING do no call Thread::Start()
  // call this function to Destroy thread relate obj, like NotifyServer
  void StartThread();

  void StopThread();

private:
  //virtual void Init();
  //virtual void CleanUp();

  scoped_ptr<base_ex::MessageLoopMFC> loop_;
};

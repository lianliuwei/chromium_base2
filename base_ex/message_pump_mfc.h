#pragma once

#include "base/compiler_specific.h"
#include "base/message_pump.h"
#include "base/time.h"
#include "base/run_loop.h"

#include "base_ex/base_ex_export.h"

namespace base_ex {

// This class implements a MessagePump needed for TYPE_EMBED MessageLoops on
// Windows platform.
class BASE_EX_EXPORT MessagePumpMFC : public base::MessagePump {
public:
  MessagePumpMFC();
  virtual ~MessagePumpMFC();

  // MessagePump methods:
  virtual void Run(Delegate* delegate) OVERRIDE;
  virtual void ScheduleWork() OVERRIDE;
  virtual void ScheduleDelayedWork(
      const base::TimeTicks& delayed_work_time) OVERRIDE;
  virtual void Quit() OVERRIDE;

  virtual void Start(Delegate* delegate);

private:
  static LRESULT CALLBACK WndProcThunk(HWND hwnd, 
                                       UINT message, 
                                       WPARAM wparam, 
                                       LPARAM lparam);
  void InitMessageWnd();
  void HandleWorkMessage();
  void HandleTimerMessage();
  int GetCurrentDelay() const;

  Delegate* delegate_;

  base::RunLoop* run_loop_;

  // Instance of the module containing the window procedure.
  HMODULE instance_;

  // The time at which delayed work should run.
  base::TimeTicks delayed_work_time_;

  // A boolean value used to indicate if there is a kMsgDoWork message pending
  // in the Windows Message queue.  There is at most one such message, and it
  // can drive execution of tasks when a native message pump is running.
  LONG have_work_;

  // A hidden message-only window.
  HWND message_hwnd_;

  DISALLOW_COPY_AND_ASSIGN(MessagePumpMFC);
};

}  // namespace base_ex

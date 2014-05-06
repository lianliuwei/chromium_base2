#include "base_ex/thread_mfc.h"

#include "base/logging.h"
#include "base/lazy_instance.h"
#include "base/threading/thread_local.h"
#include "base/message_loop.h"

#include "base_ex/message_loop_mfc.h"
#include "base_ex/message_pump_mfc.h"

using namespace base;

namespace {
LazyInstance<ThreadLocalBoolean>::Leaky
    g_is_mfc_thread = LAZY_INSTANCE_INITIALIZER;

base::MessagePump* MFCMessagePumpFactory() {
  if (g_is_mfc_thread.Get().Get()) { // mfc thread.
    return new base_ex::MessagePumpMFC();
  } else { // normal thread.
    return new base::MessagePumpForUI();
  }
}
}

namespace base_ex {

void InitMFCThread() {
  MarkMFCThread();
  SetMessagePumpFactory();
}

bool IsMFCThread() {
  return g_is_mfc_thread.Get().Get();
}

void MarkMFCThread() {
  if (g_is_mfc_thread.Get().Get()) {
    LOG(WARNING) << "Mark thread more than once, Check you code";
  }
  // mark thread as mfc thread
  g_is_mfc_thread.Get().Set(true);
}

void SetMessagePumpFactory() {
  bool ret = base::MessageLoop::InitMessagePumpForUIFactory(
      MFCMessagePumpFactory);
  CHECK(ret) << "Must be only one PumpFactory.";
}

void MessageLoopStart() {
  DCHECK(IsMFCThread());
  DCHECK(MessageLoopMFC::current());
  MessageLoopMFC::current()->Start();
}

void MessageLoopQuit() {
  DCHECK(IsMFCThread());
  DCHECK(MessageLoopMFC::current());
  MessageLoopMFC::current()->Quit();
}

} //  base_ex


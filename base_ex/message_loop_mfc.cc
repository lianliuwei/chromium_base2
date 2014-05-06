#include "base_ex/message_loop_mfc.h"

#include "base_ex/message_pump_mfc.h"

namespace base_ex {

void MessageLoopMFC::Start() {
  pump_ui()->Start(this);
}

void MessageLoopMFC::Quit() {
  pump_ui()->Quit();
}

MessagePumpMFC* MessageLoopMFC::pump_ui() {
  return static_cast<MessagePumpMFC*>(pump_.get());
}

MessageLoopMFC* MessageLoopMFC::current() {
  MessageLoop* loop = MessageLoop::current();
  DCHECK(loop);
  DCHECK_EQ(MessageLoop::TYPE_UI, loop->type());
  return static_cast<MessageLoopMFC*>(loop);
}

} // namespace base_ex

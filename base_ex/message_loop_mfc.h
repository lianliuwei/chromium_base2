#pragma once

#include "base/message_loop.h"

namespace base_ex {

class MessagePumpMFC;

class MessageLoopMFC : public base::MessageLoop {
public:
  friend void MessageLoopStart();
  friend void MessageLoopQuit();

  MessageLoopMFC() : MessageLoop(TYPE_UI) {}

  // Returns the MessageLoopForUI of the current thread.
  static MessageLoopMFC* current();

  // the UI message loop is handled by be Embedding side. So Run() should
  // never be called. Instead use Start(), which will forward all the native
  // UI events to the native message loop.
  void Start();

  // match Start();
  void Quit();
protected:

  // TODO(rvargas): Make this platform independent.
  MessagePumpMFC* pump_ui();
};

// see comment in MessageLoopForUI
COMPILE_ASSERT(sizeof(base::MessageLoop) == sizeof(MessageLoopMFC),
               MessageLoopMFC_should_not_have_extra_member_variables);

} // namespace base_ex

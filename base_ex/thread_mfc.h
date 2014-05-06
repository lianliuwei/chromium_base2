#pragma once

#include "base_ex/base_ex_export.h"

namespace base_ex {

// MFC has it own message loop, and it is complex. it will be painful to  take
// over the message loop by base/message_loop. need attach to MFC thread and
// make it handle PostTask
// MFC thread mean MFC UI thread, other thread just replace with base/threading/
// call in the MFC thread, mark it for create a different MessageLoop.
// call this before create MessageLoop.
BASE_EX_EXPORT void InitMFCThread();

BASE_EX_EXPORT bool IsMFCThread();

BASE_EX_EXPORT void MarkMFCThread();

// see message pump factory to create special MessagePump for MFC thread.
// call after InitMFCThread()
BASE_EX_EXPORT void SetMessagePumpFactory();

// call this before MFC MessageLoop begin.
BASE_EX_EXPORT void MessageLoopStart();

// call this after MFC MessageLoop end.
BASE_EX_EXPORT void MessageLoopQuit();

} //  base_ex

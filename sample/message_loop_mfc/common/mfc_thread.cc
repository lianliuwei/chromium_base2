#include "common/mfc_thread.h"

using namespace base;
using namespace base_ex;

MFCThread::MFCThread()
  : Thread("") {}

MFCThread::~MFCThread() {}

void MFCThread::StartThread() {
  DCHECK(!loop_.get());
  loop_.reset(new MessageLoopMFC);
  set_message_loop(loop_.get());
  MessageLoopStart();
  Init();
}

void MFCThread::StopThread() {
  DCHECK(loop_.get());
  CleanUp();
  MessageLoopQuit();
  set_message_loop(NULL);
  loop_.reset(NULL);
}

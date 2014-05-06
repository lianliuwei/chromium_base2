#include "base/message_loop.h"
#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/memory/scoped_ptr.h"
#include "base/bind.h"
#include "base/rand_util.h"
#include "base/threading/worker_pool.h"
#include "base/task_runner_util.h"

static int64 kAddUpValue = 0;
static const int kThreadNum = base::RandInt(400, 500);
static int64 kExpectValue = 0;
static int kCallBackNum = 0;
int CalculateValue(int num) {
  int ret = 0;
  for (int i = 0; i < num; ++i)
    ++ret;
  return ret;
}

void AddUp(int num) {
  ++kCallBackNum;
  kAddUpValue += num;
  if (kCallBackNum == kThreadNum) {
    DCHECK (kExpectValue == kAddUpValue) << "Expect = " << kExpectValue 
    << "; AddUp = " << kAddUpValue;
    DLOG(INFO) << "Finish Calculate num: " << kAddUpValue;
    MessageLoop::current()->Quit();
  }
}

int main(int argc, char** argv) {
  base::AtExitManager atexit;

  using namespace logging;
  CommandLine::Init(argc, argv);
  InitLogging(L"debug.log", LOG_TO_BOTH_FILE_AND_SYSTEM_DEBUG_LOG,
    DONT_LOCK_LOG_FILE, DELETE_OLD_LOG_FILE,
    DISABLE_DCHECK_FOR_NON_OFFICIAL_RELEASE_BUILDS);

  MessageLoop messageloop(MessageLoop::TYPE_DEFAULT);

  DLOG(INFO) << "Create " << kThreadNum <<" Rand Task";
  for (int i = 0; i < kThreadNum; ++i) {
    int rand_num = base::RandInt(10000000, 40000000);
    kExpectValue += rand_num;
    base::PostTaskAndReplyWithResult(
      base::WorkerPool::GetTaskRunner(false), FROM_HERE,
      base::Bind(&CalculateValue, rand_num), base::Bind(&AddUp));
  }
  messageloop.Run();

  return 0;
}

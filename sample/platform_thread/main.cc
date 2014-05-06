#include "base/threading/platform_thread.h"
#include "base/time.h"

class MyDelegate : public base::PlatformThread::Delegate {
public:
  MyDelegate() {}
  virtual ~MyDelegate() {}
  virtual void ThreadMain() {
    base::PlatformThread::SetName("testPlatformThread");
    base::PlatformThread::Sleep(base::TimeDelta::FromSeconds(2));
  }
};

int main(int argc, char** argv) {
  MyDelegate dg;
  base::PlatformThreadHandle handle;
  base::PlatformThread::Create(0, &dg, &handle);
  base::PlatformThread::Join(handle);

  return 0;
}

#include "gtest/gtest.h"

#include "base/test/main_hook.h"
#include "base/test/test_suite.h"

static int kCount = 0;
class FooEnvironment : public testing::Environment {
public:
  FooEnvironment() : count_(kCount) { ++kCount; }

  virtual void SetUp() {
    std::cout << "Foo FooEnvironment SetUp: " << count_ << std::endl;
  }
  virtual void TearDown() {
    std::cout << "Foo FooEnvironment TearDown: " << count_ << std::endl;
  }
private:
  const int count_;
};


int main(int argc, char** argv) {
  MainHook hook(main, argc, argv);
  testing::AddGlobalTestEnvironment(new FooEnvironment());
  return base::TestSuite(argc, argv).Run();
}

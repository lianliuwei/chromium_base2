#include "base_ex/call_run_loop_hack.h"

// HACK
// reduce private change
#include "base/base_export.h"
#include "base/callback.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop.h"

#define private \
friend class Test; \
friend bool ::BeforeRun(base::RunLoop* run_loop); \
friend void ::AfterRun(base::RunLoop* run_loop); \
private

#include "base/run_loop.h"

bool BeforeRun(base::RunLoop* run_loop) {
  DCHECK(run_loop);
  return run_loop->BeforeRun();
}

void AfterRun(base::RunLoop* run_loop) {
  DCHECK(run_loop);
  run_loop->AfterRun();
}

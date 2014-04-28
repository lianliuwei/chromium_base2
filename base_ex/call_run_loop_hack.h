#pragma once

namespace base {
class RunLoop;
}

// WARNING using this only in message_pump_mfc.cc
bool BeforeRun(base::RunLoop* run_loop);
void AfterRun(base::RunLoop* run_loop);


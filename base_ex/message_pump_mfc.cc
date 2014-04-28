#include "base_ex/message_pump_mfc.h"

#include <math.h>

#include "base/debug/trace_event.h"
#include "base/message_loop.h"
#include "base/metrics/histogram.h"
#include "base/process_util.h"
#include "base/win/wrapped_window_proc.h"

#include "base_ex/call_run_loop_hack.h"

using namespace base;

namespace {

enum MessageLoopProblems {
  MESSAGE_POST_ERROR,
  COMPLETION_POST_ERROR,
  SET_TIMER_ERROR,
  MESSAGE_LOOP_PROBLEM_MAX,
};

}  // namespace

namespace base_ex {

static const wchar_t kWndClass[] = L"Chrome_MessagePumpForMFC";

// Message sent to get an additional time slice for pumping (processing) another
// task (a series of such messages creates a continuous task pump).
static const int kMsgHaveWork = WM_USER + 2;

int MessagePumpMFC::GetCurrentDelay() const {
  if (delayed_work_time_.is_null())
    return -1;

  // Be careful here.  TimeDelta has a precision of microseconds, but we want a
  // value in milliseconds.  If there are 5.5ms left, should the delay be 5 or
  // 6?  It should be 6 to avoid executing delayed work too early.
  double timeout =
    ceil((delayed_work_time_ - base::TimeTicks::Now()).InMillisecondsF());

  // If this value is negative, then we need to run delayed work soon.
  int delay = static_cast<int>(timeout);
  if (delay < 0)
    delay = 0;

  return delay;
}

MessagePumpMFC::MessagePumpMFC()
    : have_work_(0)
    , delegate_(NULL)
    , instance_(NULL) {
  InitMessageWnd();
}

MessagePumpMFC::~MessagePumpMFC() {
  DestroyWindow(message_hwnd_);
  UnregisterClass(kWndClass, instance_);
}

void MessagePumpMFC::ScheduleWork() {
  if (InterlockedExchange(&have_work_, 1))
    return;  // Someone else continued the pumping.

  // Make sure the MessagePump does some work for us.
  BOOL ret = PostMessage(message_hwnd_, kMsgHaveWork,
                         reinterpret_cast<WPARAM>(this), 0);
  if (ret)
    return;  // There was room in the Window Message queue.

  // We have failed to insert a have-work message, so there is a chance that we
  // will starve tasks/timers while sitting in a nested message loop.  Nested
  // loops only look at Windows Message queues, and don't look at *our* task
  // queues, etc., so we might not get a time slice in such. :-(
  // We could abort here, but the fear is that this failure mode is plausibly
  // common (queue is full, of about 2000 messages), so we'll do a near-graceful
  // recovery.  Nested loops are pretty transient (we think), so this will
  // probably be recoverable.
  InterlockedExchange(&have_work_, 0);  // Clarify that we didn't really insert.
  UMA_HISTOGRAM_ENUMERATION("Chrome.MessageLoopMFCProblem", MESSAGE_POST_ERROR,
                            MESSAGE_LOOP_PROBLEM_MAX);
}

void MessagePumpMFC::ScheduleDelayedWork(const TimeTicks& delayed_work_time) {
  //
  // We would *like* to provide high resolution timers.  Windows timers using
  // SetTimer() have a 10ms granularity.  We have to use WM_TIMER as a wakeup
  // mechanism because the application can enter modal windows loops where it
  // is not running our MessageLoop; the only way to have our timers fire in
  // these cases is to post messages there.
  //
  // To provide sub-10ms timers, we process timers directly from our run loop.
  // For the common case, timers will be processed there as the run loop does
  // its normal work.  However, we *also* set the system timer so that WM_TIMER
  // events fire.  This mops up the case of timers not being able to work in
  // modal message loops.  It is possible for the SetTimer to pop and have no
  // pending timers, because they could have already been processed by the
  // run loop itself.
  //
  // We use a single SetTimer corresponding to the timer that will expire
  // soonest.  As new timers are created and destroyed, we update SetTimer.
  // Getting a spurrious SetTimer event firing is benign, as we'll just be
  // processing an empty timer queue.
  //
  delayed_work_time_ = delayed_work_time;

  int delay_msec = GetCurrentDelay();
  DCHECK_GE(delay_msec, 0);
  if (delay_msec < USER_TIMER_MINIMUM)
    delay_msec = USER_TIMER_MINIMUM;

  // Create a WM_TIMER event that will wake us up to check for any pending
  // timers (in case we are running within a nested, external sub-pump).
  BOOL ret = SetTimer(message_hwnd_, reinterpret_cast<UINT_PTR>(this),
                      delay_msec, NULL);
  if (ret)
    return;
  // If we can't set timers, we are in big trouble... but cross our fingers for
  // now.
  // TODO(jar): If we don't see this error, use a CHECK() here instead.
  UMA_HISTOGRAM_ENUMERATION("Chrome.MessageLoopMFCProblem", SET_TIMER_ERROR,
                            MESSAGE_LOOP_PROBLEM_MAX);
}

void MessagePumpMFC::Run( Delegate* delegate ) {
  NOTREACHED() << "MessagePumpForEmbed should no replace the Native loop";
}

void MessagePumpMFC::Start( Delegate* delegate ) {
  CHECK(delegate_ == NULL) << "Call Start more then once.";

  run_loop_ = new RunLoop();
  // Since the RunLoop was just created above, BeforeRun should be guaranteed to
  // return true (it only returns false if the RunLoop has been Quit already).
  if (!BeforeRun(run_loop_))
    NOTREACHED();

  delegate_ = delegate;
}

void MessagePumpMFC::Quit() {
  CHECK(delegate_ != NULL) << "Call Quit without call Start.";
  delegate_ = NULL;

  if (run_loop_) {
    AfterRun(run_loop_);
    delete run_loop_;
    run_loop_ = NULL;
  }
}

void MessagePumpMFC::InitMessageWnd() {
  WNDCLASSEX wc = {0};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = base::win::WrappedWindowProc<WndProcThunk>;
  wc.hInstance = base::GetModuleFromAddress(wc.lpfnWndProc);
  wc.lpszClassName = kWndClass;
  instance_ = wc.hInstance;
  RegisterClassEx(&wc);

  message_hwnd_ =
      CreateWindow(kWndClass, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, instance_, 0);
  DCHECK(message_hwnd_);
}

// static
LRESULT CALLBACK MessagePumpMFC::WndProcThunk(
    HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case kMsgHaveWork:
      reinterpret_cast<MessagePumpMFC*>(wparam)->HandleWorkMessage();
      break;
    case WM_TIMER:
      reinterpret_cast<MessagePumpMFC*>(wparam)->HandleTimerMessage();
      break;
  }
  return DefWindowProc(hwnd, message, wparam, lparam);
}

void MessagePumpMFC::HandleWorkMessage() {
  // If we are being called outside of the context of Run, then don't try to do
  // any work.  This could correspond to a MessageBox call or something of that
  // sort.
  if (!delegate_) {
    // Since we handled a kMsgHaveWork message, we must still update this flag.
    InterlockedExchange(&have_work_, 0);
    return;
  }
  // Since we discarded a kMsgHaveWork message, we must update the flag.
  int old_have_work = InterlockedExchange(&have_work_, 0);
  DCHECK(old_have_work);

  // Now give the delegate a chance to do some work.  He'll let us know if he
  // needs to do more work.
  if (delegate_->DoWork())
    ScheduleWork();
}

void MessagePumpMFC::HandleTimerMessage() {
  KillTimer(message_hwnd_, reinterpret_cast<UINT_PTR>(this));

  // If we are being called outside of the context of Run, then don't do
  // anything.  This could correspond to a MessageBox call or something of
  // that sort.
  if (!delegate_)
    return;

  delegate_->DoDelayedWork(&delayed_work_time_);
  if (!delayed_work_time_.is_null()) {
    // A bit gratuitous to set delayed_work_time_ again, but oh well.
    ScheduleDelayedWork(delayed_work_time_);
  }
}

} // base_ex

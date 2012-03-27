#include "cond.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <limits.h>

ConditionVariable::ConditionVariable()
   : m_futx(0)
{
   #ifdef TIME_LOCKS
   _timer = TotalTimer::getTimerByStacktrace("cond@" + itostr(this));
   #endif
}

ConditionVariable::~ConditionVariable()
{
   #ifdef TIME_LOCKS
   delete _timer;
   #endif
}

void ConditionVariable::wait(Lock& lock, UInt64 timeout_ns)
{
   m_lock.acquire();

   // Wait
   m_futx = 0;

   m_lock.release();

   lock.release();

   #ifdef TIME_LOCKS
   ScopedTimer tt(*_timer);
   #endif

   struct timespec timeout = { timeout_ns / 1000000000, timeout_ns % 1000000000 };

   syscall(SYS_futex, (void*) &m_futx, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, timeout_ns > 0 ? &timeout : NULL, NULL, 0);

   lock.acquire();
}

void ConditionVariable::signal()
{
   m_lock.acquire();

   m_futx = 1;

   syscall(SYS_futex, (void*) &m_futx, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1, NULL, NULL, 0);

   m_lock.release();
}

void ConditionVariable::broadcast()
{
   m_lock.acquire();

   m_futx = 1;

   syscall(SYS_futex, (void*) &m_futx, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, INT_MAX, NULL, NULL, 0);

   m_lock.release();
}
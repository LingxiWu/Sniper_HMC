#include "log.h"
#include "inst_mode.h"

// Instrumentation modes
InstMode::inst_mode_t InstMode::inst_mode_init = InstMode::CACHE_ONLY;   // Change this into FAST_FORWARD if you don't care about    warm caches
InstMode::inst_mode_t InstMode::inst_mode_roi  = InstMode::DETAILED;
InstMode::inst_mode_t InstMode::inst_mode_end  = InstMode::FAST_FORWARD;

// Initial instrumentation mode
InstMode::inst_mode_t InstMode::inst_mode = InstMode::inst_mode_init;


__attribute__((weak)) void
InstMode::SetInstrumentationMode(InstMode::inst_mode_t new_mode)
{
   LOG_PRINT_ERROR("%s: This version of this function should not be called", __FUNCTION__);
}

InstMode::inst_mode_t
InstMode::fromString(const String str)
{
   if (str == "cache_only")
      return CACHE_ONLY;
   else if (str == "detailed")
      return DETAILED;
   else if (str == "fast_forward")
      return FAST_FORWARD;
   else
      LOG_PRINT_ERROR("Invalid instrumentation mode %s", str.c_str());
}
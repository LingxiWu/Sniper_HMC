#include "hooks_manager.h"
#include "log.h"

const char* HookType::hook_type_names[] = {
   "HOOK_PERIODIC",
   "HOOK_SIM_START",
   "HOOK_SIM_END",
   "HOOK_ROI_BEGIN",
   "HOOK_ROI_END",
   "HOOK_CPUFREQ_CHANGE",
   "HOOK_MAGIC_MARKER",
   "HOOK_MAGIC_USER",
   "HOOK_INSTR_COUNT",
   "HOOK_THREAD_STALL",
   "HOOK_THREAD_RESUME",
   "HOOK_INSTRUMENT_MODE",
};

HooksManager::HooksManager()
{
   LOG_ASSERT_ERROR(HookType::HOOK_TYPES_MAX <= sizeof(HookType::hook_type_names) / sizeof(HookType::hook_type_names[0]),
                    "Not enough values in HookType::hook_type_names");
}

void HooksManager::registerHook(HookType::hook_type_t type, HookCallbackFunc func, void* argument)
{
   m_registry[type].push_back(std::pair<HookCallbackFunc, void*>(func, argument));
}

SInt64 HooksManager::callHooks(HookType::hook_type_t type, void *arg, bool expect_return)
{
   for(std::vector<HookCallback>::iterator it = m_registry[type].begin(); it != m_registry[type].end(); ++it)
   {
      SInt64 result = (it->first)(it->second, arg);
      if (expect_return && result != -1)
         return result;
   }

   return -1;
}
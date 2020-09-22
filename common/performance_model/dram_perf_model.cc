#include "simulator.h"
#include "dram_perf_model.h"
#include "dram_perf_model_constant.h"
#include "dram_perf_model_readwrite.h"
#include "dram_perf_model_normal.h"
#include "config.hpp"
using namespace std;
DramPerfModel* DramPerfModel::createDramPerfModel(core_id_t core_id, UInt32 cache_block_size)
{
   String type = Sim()->getCfg()->getString("perf_model/dram/type");

   if (type == "constant")
   {  cout << "[LINGXI]: in /common/perf_model/dram_perf_model_base.h create a constant dram_perf_mdl" << endl;
      return new DramPerfModelConstant(core_id, cache_block_size);
   }
   else if (type == "readwrite")
   { cout << "[LINGXI]: in /common/perf_model/dram_perf_model_base.h create a readwrite dram_perf_mdl" << endl; 
      return new DramPerfModelReadWrite(core_id, cache_block_size);
   }
   else if (type == "normal")
   {  cout << "[LINGXI]: in /common/perf_model/dram_perf_model_base.h create a normal dram_perf_mdl" << endl; 
      return new DramPerfModelNormal(core_id, cache_block_size);
   }
   else
   {
      LOG_PRINT_ERROR("Invalid DRAM model type %s", type.c_str());
   }
}

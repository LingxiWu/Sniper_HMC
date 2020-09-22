#include "dram_perf_model_constant.h"
#include "simulator.h"
#include "config.h"
#include "config.hpp"
#include "stats.h"
#include "shmem_perf.h"

#include <fstream>

using namespace std;
DramPerfModelConstant::DramPerfModelConstant(core_id_t core_id,
      UInt32 cache_block_size):
   DramPerfModel(core_id, cache_block_size),
   m_queue_model(NULL),
   m_dram_bandwidth(8 * Sim()->getCfg()->getFloat("perf_model/dram/per_controller_bandwidth")), // Convert bytes to bits
   m_total_queueing_delay(SubsecondTime::Zero()),
   m_total_access_latency(SubsecondTime::Zero())
{
   m_dram_access_cost = SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(Sim()->getCfg()->getFloat("perf_model/dram/latency"))); // Operate in fs for higher precision before converting to uint64_t/SubsecondTime

   if (Sim()->getCfg()->getBool("perf_model/dram/queue_model/enabled"))
   {
	   cout << "[LINGXI]: Perf_model/dramPerfModelConstant:  " << "cache_block_size: " << 
	   to_string(cache_block_size) << 
	   " m_dram_bandwidth.getRoundedLatency(8 * cache_block_size): " << to_string(m_dram_bandwidth.getRoundedLatency(8 * cache_block_size).getNS()) << 
	   " core_id: " << to_string(core_id) << endl;

       m_queue_model = QueueModel::create("dram-queue", core_id, Sim()->getCfg()->getString("perf_model/dram/queue_model/type"),
                                         m_dram_bandwidth.getRoundedLatency(8 * cache_block_size)); // bytes to bits
   }

   registerStatsMetric("dram", core_id, "total-access-latency", &m_total_access_latency);
   registerStatsMetric("dram", core_id, "total-queueing-delay", &m_total_queueing_delay);
}

DramPerfModelConstant::~DramPerfModelConstant()
{
   if (m_queue_model)
   {
     delete m_queue_model;
      m_queue_model = NULL;
   }
}

SubsecondTime
DramPerfModelConstant::getAccessLatency(SubsecondTime pkt_time, UInt64 pkt_size, core_id_t requester, IntPtr address, DramCntlrInterface::access_t access_type, ShmemPerf *perf)
{
//   cout << "[LINGXI]: Dram_Perf_Model_Constant::getAccessLatency()\n" << endl;
   // pkt_size is in 'Bytes'
   // m_dram_bandwidth is in 'Bits per clock cycle'
   if ((!m_enabled) ||
         (requester >= (core_id_t) Config::getSingleton()->getApplicationCores()))
   {
      return SubsecondTime::Zero();
   }

   SubsecondTime processing_time = m_dram_bandwidth.getRoundedLatency(8 * pkt_size); // bytes to bits

   // Compute Queue Delay
   SubsecondTime queue_delay;
   if (m_queue_model)
   {
      queue_delay = m_queue_model->computeQueueDelay(pkt_time, processing_time, requester);
   }
   else
   {
      queue_delay = SubsecondTime::Zero();
   }

/* 
 * Somewhere here needs to model dram_access and adjust m_dram_access_cost
 * Let's try to get addr first.
 * */

   // each core has an independent history_list, indicated by handled_by_core
   // each core maintains a small rolling window of size XX, check if the arriving request can be parallelised 
   // 
/*   
   cout << " requester_core: " << to_string(requester) 
	<< " handled_by_core: " << to_string(m_queue_model->q_core_id)   
	<< " address: " << to_string(address) 
	<< " arrive at: " << to_string(pkt_time.getNS()) << endl;
*/
//   getBank(address);

/*
ofstream myfile;
myfile.open("mem_addr_trace.txt", ios::app);
myfile << to_string(address) << endl;
myfile.close();
*/

   SubsecondTime access_latency = queue_delay + processing_time + m_dram_access_cost;
/*
   cout << "[LINGXI]: in /common/perf_model/dram_perf_const::getAccessLatency" 
	<< " access_latency: " << to_string(access_latency.getNS())
	<< " q_delay: " << to_string(queue_delay.getNS()) 
	<< " processing_time: " << to_string(processing_time.getNS()) 
	<< " m_dram_access_cost: " << to_string(m_dram_access_cost.getNS()) 
	<< endl;   
*/
  
   perf->updateTime(pkt_time);
   perf->updateTime(pkt_time + queue_delay, ShmemPerf::DRAM_QUEUE);
   perf->updateTime(pkt_time + queue_delay + processing_time, ShmemPerf::DRAM_BUS);
   perf->updateTime(pkt_time + queue_delay + processing_time + m_dram_access_cost, ShmemPerf::DRAM_DEVICE);

   // Update Memory Counters
   m_num_accesses ++;
   m_total_access_latency += access_latency;
   m_total_queueing_delay += queue_delay;

   return access_latency;
}

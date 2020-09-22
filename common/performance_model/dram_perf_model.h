#ifndef __DRAM_PERF_MODEL_H__
#define __DRAM_PERF_MODEL_H__

#include "queue_model.h"
#include "fixed_types.h"
#include "subsecond_time.h"
#include "dram_cntlr_interface.h"


class ShmemPerf;

// Note: Each Dram Controller owns a single DramModel object
// Hence, m_dram_bandwidth is the bandwidth for a single DRAM controller
// Total Bandwidth = m_dram_bandwidth * Number of DRAM controllers
// Number of DRAM controllers presently = Number of Cores
// m_dram_bandwidth is expressed in GB/s
// Assuming the frequency of a core is 1GHz,
// m_dram_bandwidth is also expressed in 'Bytes per clock cycle'
// This DRAM model is not entirely correct.
// It sort of increases the queueing delay to a huge value if
// the arrival times of adjacent packets are spread over a large
// simulated time period
class DramPerfModel
{
   protected:
      bool m_enabled;
      UInt64 m_num_accesses;

   public:
      static DramPerfModel* createDramPerfModel(core_id_t core_id, UInt32 cache_block_size);

      DramPerfModel(core_id_t core_id, UInt64 cache_block_size) : m_enabled(false), m_num_accesses(0) {}
      virtual ~DramPerfModel() {}
      virtual SubsecondTime getAccessLatency(SubsecondTime pkt_time, UInt64 pkt_size, core_id_t requester, IntPtr address, DramCntlrInterface::access_t access_type, ShmemPerf *perf) = 0;
      void enable() { m_enabled = true; }
      void disable() { m_enabled = false; }

/**
 * the following add-ons handle data mapping and bandwidth/q_delay
 */
       
      /* define HMC 2.0 -> 32 vaults * 256 MB_per_vault, bank_size 16MB, partition_size 32 MB*/
      int cube_capacity = 8; // GB
      int num_quandrants = 4;
      int vaults_per_quandrant = 8; // 4/8
      int banks_per_vault= 16; 
      int dram_layers = 8;
      int banks_per_partition = 2;
      int banks_per_cube = num_quandrants * vaults_per_quandrant * dram_layers * banks_per_partition;
      int block_size = 64; // 32/64/128 bytes
     
      /* define various bandwidth */
      // the literature don't agree with each other. We use a fusion of Tesseract and Mondrian
      // according to Tesseract, 512 GB/s internal bandwidth shared by 32 vaults, each TSV 2 Gb/s, 512/(2/8) = 2048 TSV, that is, 64
      // tsv per vault.
      int ext_link_bw = 120; // 4 links * 16 lanes/link * 30 Gbps * 2 dullplex = 480 GB total ext bw
      int intra_vault_bw = 16; // GB/s. 8 in Mondrian, 10 in demystifying, 16 in Tesseract
      // not sure about this, maybe should be handled by NOC and hop_latency
//      int inter_quadrant_bw_close = 10; // diff vaults in the same quadrant GB/s
//      int inter_quadrant_bw_far = 10; // diff vaults in diff quadrant
//      int inter_cube_bw = 10; // GB/s
//      according to Micron manual, an access to a local vault in a quadrant incurs lower latency than an access to a vault in another quadrant
      int intra_quadrant_hop_latency = 2; 

      /* about topology */
      int system_mem_capacity = 32; // total memory capacity (GB)
      int num_of_cubes = system_mem_capacity / cube_capacity;

      /* maximum_parallel_request_serviceable: window size, how many concurrent memory requests we need to track. 
       * This should be the minimal of ROB size and max of concurrent mem requests that can be serviced by HMC 
       * for now just 100 -> same as Q depth: queue_model/history_list: max_list_size 
       */
      int max_parallel_requests = 100;

      static int getBank(IntPtr address){	
return 0;
      }   
      static int getVault(IntPtr address){
return 0;
      }	
      static int getCube(IntPtr address){
return 0;
      }	      
      UInt64 getTotalAccesses() { return m_num_accesses; }
};

#endif /* __DRAM_PERF_MODEL_H__ */

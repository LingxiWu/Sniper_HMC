#include "simulator.h"
#include "handle_args.h"
#include "config.hpp"
#include "trace_manager.h"
#include "magic_client.h"

int main(int argc, char* argv[])
{
   // To make sure output shows up immediately, make stdout and stderr line buffered
   // (if we're writing into a pipe to run-graphite, or redirected to a file by the job runner, the default will be block buffered)
   setvbuf(stdout, NULL, _IOLBF, 0);
   setvbuf(stderr, NULL, _IOLBF, 0);

   string_vec args;

   // Set the default config path if it isn't
   // overwritten on the command line.
   String config_path = "carbon_sim.cfg";

   parse_args(args, config_path, argc, argv);

   config::ConfigFile *cfg = new config::ConfigFile();
   cfg->load(config_path);

   handle_args(args, *cfg);

   Simulator::setConfig(cfg, Config::SimulationMode::STANDALONE);

   Simulator::allocate();
   Sim()->start();

   // config::Config shouldn't be called outside of init/fini
   // With Sim()->hideCfg(), we let Simulator know to complain when someone does call Sim()->getCfg()
   Sim()->hideCfg();


   enablePerformanceGlobal();

   LOG_ASSERT_ERROR(Sim()->getTraceManager(), "In standalone mode but there is no TraceManager!");
   Sim()->getTraceManager()->run();

   disablePerformanceGlobal();


   Simulator::release();
   delete cfg;

   return 0;
}
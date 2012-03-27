#include "instruction.h"
#include "simulator.h"
#include "core_manager.h"
#include "core.h"
#include "performance_model.h"
#include "branch_predictor.h"
#include "config.hpp"

// Instruction

Instruction::StaticInstructionCosts Instruction::m_instruction_costs;

Instruction::Instruction(InstructionType type, OperandList &operands)
   : m_type(type)
   , m_addr(0)
   , m_operands(operands)
{
}

Instruction::Instruction(InstructionType type)
   : m_type(type)
   , m_addr(0)
{
}

InstructionType Instruction::getType() const
{
    return m_type;
}

String Instruction::getTypeName() const
{
   LOG_ASSERT_ERROR(m_type < MAX_INSTRUCTION_COUNT, "Unknown instruction type: %d", m_type);
   return String( INSTRUCTION_NAMES[ getType() ] );
}

// An instruction cost is the number of cycles it takes to execute the instruction, times the
// period of the processor that we are currently running on
SubsecondTime Instruction::getCost(Core *core) const
{
   LOG_ASSERT_ERROR(m_type < MAX_INSTRUCTION_COUNT, "Unknown instruction type: %d", m_type);
   const ComponentPeriod *period = core->getDvfsDomain();
   return static_cast<SubsecondTime>(*period) * Instruction::m_instruction_costs[m_type];
}

void Instruction::initializeStaticInstructionModel()
{
   m_instruction_costs.resize(MAX_INSTRUCTION_COUNT);
   for(unsigned int i = 0; i < MAX_INSTRUCTION_COUNT; i++)
   {
       char key_name [1024];
       snprintf(key_name, 1024, "perf_model/core/static_instruction_costs/%s", INSTRUCTION_NAMES[i]);
       UInt32 instruction_cost = Sim()->getCfg()->getInt(key_name, 0);
       m_instruction_costs[i] = instruction_cost;
   }
}

// DynamicInstruction

DynamicInstruction::DynamicInstruction(SubsecondTime cost, InstructionType type)
   : Instruction(type)
   , m_cost(cost)
{
}

DynamicInstruction::~DynamicInstruction()
{
}

SubsecondTime DynamicInstruction::getCost(Core *core) const
{
   return m_cost;
}

// StringInstruction

StringInstruction::StringInstruction(OperandList &ops)
   : Instruction(INST_STRING, ops)
{
}

SubsecondTime StringInstruction::getCost(Core *core) const
{
   // dequeue mem ops until we hit the final marker, then check count
   PerformanceModel *perf = core->getPerformanceModel();
   UInt32 count = 0;
   SubsecondTime cost = SubsecondTime::Zero();
   DynamicInstructionInfo* i;

   while (true)
   {
      i = perf->getDynamicInstructionInfo(*this);
      if (!i)
         return PerformanceModel::DyninsninfoNotAvailable();

      if (i->type == DynamicInstructionInfo::STRING)
         break;

      LOG_ASSERT_ERROR(i->type == DynamicInstructionInfo::MEMORY_READ,
                       "Expected memory read in string instruction (or STRING).");

      cost += i->memory_info.latency;

      ++count;
      perf->popDynamicInstructionInfo();
   }

   LOG_ASSERT_ERROR(count == i->string_info.num_ops,
                    "Number of mem ops in queue doesn't match number in string instruction.");
   perf->popDynamicInstructionInfo();

   return cost;
}

// SpawnInstruction

SpawnInstruction::SpawnInstruction(SubsecondTime time)
   : Instruction(INST_SPAWN)
   , m_time(time)
{ }

SubsecondTime SpawnInstruction::getCost(Core *core) const
{
   LOG_ASSERT_ERROR(false, "SpawnInstruction::getCost() called, this instruction should not have made it into handleInstruction");
   return SubsecondTime::Zero();
}

SubsecondTime SpawnInstruction::getTime() const
{
   return m_time;
}

// BranchInstruction

BranchInstruction::BranchInstruction(OperandList &l)
   : Instruction(INST_BRANCH, l)
{ }

SubsecondTime BranchInstruction::getCost(Core *core) const
{
   PerformanceModel *perf = core->getPerformanceModel();
   BranchPredictor *bp = perf->getBranchPredictor();
   const ComponentPeriod *period = core->getDvfsDomain();

   DynamicInstructionInfo *i = perf->getDynamicInstructionInfo(*this);
   if (!i)
      return PerformanceModel::DyninsninfoNotAvailable();

   LOG_ASSERT_ERROR(i->type == DynamicInstructionInfo::BRANCH, "Expected branch DynInstrInfo, got %d", i->type);

   // branch prediction not modeled
   if (bp == NULL)
   {
      perf->popDynamicInstructionInfo();
      return static_cast<SubsecondTime>(*period) * 1;
   }

   bool prediction = bp->predict(getAddress(), i->branch_info.target);
   bool correct = (prediction == i->branch_info.taken);

   bp->update(prediction, i->branch_info.taken, getAddress(), i->branch_info.target);
   UInt64 cost = correct ? 1 : bp->getMispredictPenalty();

   perf->popDynamicInstructionInfo();
   return static_cast<SubsecondTime>(*period) * cost;
}
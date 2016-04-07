#ifndef CORE_COMMON_SR_ISS_INTSINSICS_INSTRINSICMANAGER_H
#define CORE_COMMON_SR_ISS_INTSINSICS_INSTRINSICMANAGER_H

#include <string>

#include "core/common/systemc.h"
#include "core/common/vmap.h"
#include "core/common/trapgen/ABIIf.hpp"
#include "core/common/trapgen/ToolsIf.hpp"
#include "core/common/trapgen/instructionBase.hpp"
#include "core/common/sr_iss/intrinsics/platformintrinsic.h"

template<class issueWidth>
class IntrinsicManager :
    public sc_core::sc_object,
    public trap::ToolsIf<issueWidth>,
    public IntrinsicBase {
  private:
    typedef typename vmap<issueWidth, PlatformIntrinsic<issueWidth> *> syscallcb_map_t;
    typename syscallcb_map_t::const_iterator syscCallbacksEnd;
    syscallcb_map_t syscCallbacks;

    unsigned int countBits(issueWidth bits) {
      unsigned int numBits = 0;
      for (unsigned int i = 0; i < sizeof (issueWidth) * 8; i++) {
        if ((bits & (0x1 << i)) != 0) {
          numBits++;
        }
      }
      return numBits;
    }

  public:
    trap::ABIIf<issueWidth> &processorInstance;
    IntrinsicManager(sc_core::sc_module_name mn, trap::ABIIf<issueWidth> &processorInstance) :
        sc_core::sc_object(mn), processorInstance(processorInstance) {
      this->syscCallbacksEnd = this->syscCallbacks.end();
      programsCount++;

      // First of all I initialize the heap pointer according to the group it belongs to
      this->heapPointer = (unsigned int)this->processorInstance.getCodeLimit() + sizeof (issueWidth);
    }

    bool register_intrinsic(issueWidth addr, PlatformIntrinsic<issueWidth> &callBack) {
      typename syscallcb_map_t::iterator foundSysc = this->syscCallbacks.find(addr);
      if (foundSysc != this->syscCallbacks.end()) {
        int numMatch = 0;
        typename syscallcb_map_t::iterator allCallIter, allCallEnd;
        for (allCallIter = this->syscCallbacks.begin(), allCallEnd = this->syscCallbacks.end();
             allCallIter != allCallEnd;
             allCallIter++) {
          if (allCallIter->second == foundSysc->second) {
            numMatch++;
          }
        }
        if (numMatch <= 1) {
          delete foundSysc->second;
        }
      }

      this->syscCallbacks[addr] = &callBack;
      this->syscCallbacksEnd = this->syscCallbacks.end();

      return true;
    }

    ///Method called at every instruction issue, it returns true in case the instruction
    ///has to be skipped, false otherwise
    bool newIssue(const issueWidth &curPC, const trap::InstructionBase *curInstr) throw() {
      // I have to go over all the registered system calls and check if there is one
      // that matches the current program counter. In case I simply call the corresponding
      // callback.
      typename syscallcb_map_t::const_iterator foundSysc = this->syscCallbacks.find(curPC);
      if (foundSysc != this->syscCallbacksEnd) {
        return (*(foundSysc->second))();
      }
      return false;
    }
    ///Method called to know if the instruction at the current address has to be skipped:
    ///if true the instruction has to be skipped, otherwise the instruction can
    ///be executed
    bool emptyPipeline(const issueWidth &curPC) const throw() {
      if (this->syscCallbacks.find(curPC) != this->syscCallbacksEnd) {
        return true;
      }
      return false;
    }
    ///Resets the whole concurrency emulator, reinitializing it and preparing it for a new simulation
    void reset() {
      this->syscCallbacks.clear();
      this->syscCallbacksEnd = this->syscCallbacks.end();
      this->env.clear();
      this->sysconfmap.clear();
      this->programArgs.clear();
      this->heapPointer = 0;
      this->groupIDs.clear();
      this->programsCount = 0;
    }
    // The destructor calls the reset method
    ~IntrinsicManager() {
      typename syscallcb_map_t::iterator allCallIter, allCallEnd;
      for (allCallIter = this->syscCallbacks.begin(), allCallEnd = this->syscCallbacks.end();
        allCallIter != allCallEnd;
        allCallIter++) {
          delete allCallIter->second;
      }
      reset();
    }
};

#endif  // CORE_COMMON_SR_ISS_INTSINSICS_INSTRINSICMANAGER_H

/***************************************************************************\
*
*   This file is part of TRAP.
*
*   TRAP is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*   or see <http://www.gnu.org/licenses/>.
*
*
*
*   (c) Luca Fossati, fossati@elet.polimi.it, fossati.l@gmail.com
*
\ ***************************************************************************/

/**
 * This file contains the methods necessary to communicate with GDB in
 * order to debug software running on simulators. Source code takes inspiration
 * from the linux kernel (sparc-stub.c) and from ac_gdb.H in the ArchC sources
 * In order to debug the remote protocol (as a help in the development of this
 * stub, issue the "set debug remote 1" command in GDB)
 * "set remotelogfile file" logs all the remote communication on the specified file
 */

//// **** TODO:  it seeems that watchpoints are completely ignored ... :-(
//// **** TODO:  sometimes segmentation fault when GDB is closed while the program is
// still running; it seems there is a race condition with the GDB thread...

#ifndef GDBSTUB_HPP
#define GDBSTUB_HPP

#include <csignal>
#ifndef SIGTRAP
#define SIGTRAP 5
#endif
#ifndef SIGQUIT
#define SIGQUIT 3
#endif

#include "core/common/systemc.h"

#include <vector>

#include "core/common/trapgen/utils/trap_utils.hpp"

#include "core/common/trapgen/ABIIf.hpp"
#include "core/common/trapgen/ToolsIf.hpp"
#include "core/common/trapgen/instructionBase.hpp"

#include "core/common/trapgen/debugger/BreakpointManager.hpp"
#include "core/common/trapgen/debugger/GDBConnectionManager.hpp"
#include "core/common/trapgen/debugger/WatchpointManager.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>

extern int exitValue;
namespace trap {
template<class issueWidth>
class GDBStub : public ToolsIf<issueWidth>, public MemoryToolsIf<issueWidth>, public sc_module {
  public:
    bool simulationPaused;
  private:
    enum stopType {BREAK_stop = 0, WATCH_stop, STEP_stop, SEG_stop, TIMEOUT_stop, PAUSED_stop, UNK_stop};
    ///Thread used to send and receive responses with the GDB debugger
    struct GDBThread {
      GDBStub<issueWidth> &gdbStub;
      GDBThread(GDBStub<issueWidth> &gdbStub) : gdbStub(gdbStub) {}
      void operator()() {
        while (!gdbStub.isKilled) {
          if (gdbStub.connManager.checkInterrupt()) {
            gdbStub.step = 2;
          } else {
            // An Error happened: First of all I have to perform some cleanup
            if (!gdbStub.isKilled) {
              boost::mutex::scoped_lock lk(gdbStub.cleanupMutex);
              gdbStub.breakManager.clearAllBreaks();
              gdbStub.watchManager.clearAllWatchs();
              gdbStub.step = 0;
              gdbStub.isConnected = false;
            }
            break;
          }
        }
      }
    };

    ///Manages connection among the GDB target stub (this class) and
    ///the GDB debugger
    GDBConnectionManager connManager;
    ///Interface for communication with the internal processor's structure
    ABIIf<issueWidth> &processorInstance;
    ///Handles the breakpoints which have been set in the system
    BreakpointManager<issueWidth> breakManager;
    ///Handles the watchpoints which have been set in the system
    WatchpointManager<issueWidth> watchManager;
    ///Determines whether the processor has to halt as a consequence of a
    ///step command
    unsigned int step;
    ///Keeps track of the last breakpoint encountered by this processor
    Breakpoint<issueWidth> *breakReached;
    ///Keeps track of the last watchpoint encountered by this processor
    Watchpoint<issueWidth> *watchReached;
    ///Specifies whether the breakpoints are enabled or not
    bool breakEnabled;
    ///Specifies whether the watchpoints are enabled or not
    bool watchEnabled;
    ///Specifies whether GDB server side killed the simulation
    bool isKilled;
    ///In case we decided to run the simulation only for a limited ammount of time
    ///this variable contains that time
    double timeToGo;
    ///In case we decided to jump onwards or backwards for a specified ammount of time,
    ///this variable contains that time
    double timeToJump;
    ///In case the simulation is run only for a specified ammount of time,  this variable
    ///contains the simulation time at that start time
    double simStartTime;
    ///Specifies that we have to stop because a timeout was encountered
    bool timeout;
    ///Event used to manage execution for a specified ammount of time
    sc_event pauseEvent;
    ///Condition used to stop processor execution until simulation is restarted
    boost::condition gdbPausedEvent;
    ///Mutex used to access the condition
    boost::mutex global_mutex;
    ///Sepecifies if GDB is connected to this stub or not
    bool isConnected;
    ///Specifies that the first run is being made
    bool firstRun;
    ///Mutex controlling the cleanup of GDB status
    boost::mutex cleanupMutex;

    /********************************************************************/
    ///Checks if a breakpoint is present at the current address and
    ///in case it halts execution
#ifndef NDEBUG
    inline void checkBreakpoint(const issueWidth &address) {
#else
    inline void checkBreakpoint(const issueWidth &address) throw() {
#endif
      if (this->breakEnabled && this->breakManager.hasBreakpoint(address)) {
        this->breakReached = this->breakManager.getBreakPoint(address);
#ifndef NDEBUG
        if (this->breakReached == NULL) {
          THROW_EXCEPTION("I stopped because of a breakpoint, but no breakpoint was found");
        }
#endif
        this->setStopped(BREAK_stop);
      }
    }
    ///If true, the system is in the step mode and, as such, execution
    ///needs to be stopped
    inline bool goingToBreak(const issueWidth &address) const throw() {
      return this->breakEnabled && this->breakManager.hasBreakpoint(address);
    }

    ///Checks if execution must be stopped because of a step command
    inline void checkStep() throw() {
      if (this->step == 1) {
        this->step++;
      } else if (this->step == 2) {
        this->step = 0;
        if (this->timeout) {
          this->timeout = false;
          this->setStopped(TIMEOUT_stop);
        } else {
          this->setStopped(STEP_stop);
        }
      }
    }
    ///If true, the system is in the step mode and, as such, execution
    ///needs to be stopped
    inline bool goingToStep() const throw() {
      return this->step == 2;
    }

    ///Starts the thread which will manage the connection with the
    ///GDB debugger
    void startThread() {
      GDBThread thread(*this);
      boost::thread th(thread);
    }

    ///This method is called when we need asynchronously halt the
    ///execution of the processor this instance of the stub is
    ///connected to; it is usually used when a processor is halted
    ///and it has to communicated to the other processor that they have
    ///to halt too; note that this method halts SystemC execution and
    ///it also starts new threads (one for each processor) which will
    ///deal with communication with the stub; when a continue or
    ///step signal is met, then the receving thread is killed. When
    ///all the threads are killed execution can be resumed (this means
    ///that all GDBs pressed the resume button)
    ///This method is also called at the beginning of the simulation by
    ///the first processor which starts execution
    void setStopped(stopType stopReason = UNK_stop) {
      // saving current simulation time
      double curSimTime = sc_time_stamp().to_double();

      // Now I have to behave differently depending on whether database support is enabled or not
      // if it is enabled I do not stop simulation,  while if it is not enabled I have to stop simulation in
      // order to be able to inspect the content of the processor - memory - etc...
      // Computing the next simulation time instant
      if (this->timeToGo > 0) {
        this->timeToGo -= (curSimTime - this->simStartTime);
        if (this->timeToGo < 0) {
          this->timeToGo = 0;
        }
        this->simStartTime = curSimTime;
      }
      // Disabling break and watch points
      this->breakEnabled = false;
      this->watchEnabled = false;
      this->awakeGDB(stopReason);
      // pausing simulation
      while (this->waitForRequest()) {
      }
    }

    ///Sends a TRAP message to GDB so that it is awaken
    void awakeGDB(stopType stopReason = UNK_stop) {
      switch (stopReason) {
      case STEP_stop: {
        GDBResponse response;
        response.type = GDBResponse::S_rsp;
        response.payload = SIGTRAP;
        this->connManager.sendResponse(response);
        break;
      }
      case BREAK_stop: {
#ifndef NDEBUG
        if (this->breakReached == NULL) {
          THROW_EXCEPTION("I stopped because of a breakpoint, but it is NULL");
        }
#endif

        GDBResponse response;
        response.type = GDBResponse::S_rsp;
        response.payload = SIGTRAP;
        this->connManager.sendResponse(response);
        break;
      }
      case WATCH_stop: {
#ifndef NDEBUG
        if (this->watchReached == NULL) {
          THROW_EXCEPTION("I stopped because of a breakpoint, but it is NULL");
        }
#endif

        GDBResponse response;
        response.type = GDBResponse::T_rsp;
        response.payload = SIGTRAP;
        std::pair<std::string, unsigned int> info;
        info.second = this->watchReached->address;
        switch (this->watchReached->type) {
        case Watchpoint<issueWidth>::WRITE_watch:
          info.first = "watch";
          break;
        case Watchpoint<issueWidth>::READ_watch:
          info.first = "rwatch";
          break;
        case Watchpoint<issueWidth>::ACCESS_watch:
          info.first = "awatch";
          break;
        default:
          info.first = "none";
          break;
        }
        response.size = sizeof (issueWidth);
        response.info.push_back(info);
        this->connManager.sendResponse(response);
        break;
      }
      case SEG_stop: {
        // An error has occurred during processor execution (illelgal instruction, reading out of memory, ...);
        GDBResponse response;
        response.type = GDBResponse::S_rsp;
        response.payload = SIGILL;
        this->connManager.sendResponse(response);
        break;
      }
      case TIMEOUT_stop: {
        // the simulation time specified has elapsed,  so simulation halted
        GDBResponse resp;
        resp.type = GDBResponse::OUTPUT_rsp;
        resp.message = "Specified Simulation time completed - Current simulation time: " + sc_time_stamp().to_string() +
                       " (ps)\n";
        this->connManager.sendResponse(resp);
        this->connManager.sendInterrupt();
        break;
      }
      case PAUSED_stop: {
        // the simulation time specified has elapsed, so simulation halted
        GDBResponse resp;
        resp.type = GDBResponse::OUTPUT_rsp;
        resp.message = "Simulation Paused - Current simulation time: " + sc_time_stamp().to_string() + " (ps)\n";
        this->connManager.sendResponse(resp);
        this->connManager.sendInterrupt();
        break;
      }
      default:
        this->connManager.sendInterrupt();
        break;
      }
    }

    ///Signals to the GDB debugger that simulation ended; the error variable specifies
    ///if the program ended with an error
    void signalProgramEnd(bool error = false) {
      if (!this->isKilled || error) {
        GDBResponse response;
        // Now I just print a message to the GDB console signaling the user that the program is ending
        if (error) {
          // I start anyway by signaling an error
          GDBResponse rsp;
          rsp.type = GDBResponse::ERROR_rsp;
          this->connManager.sendResponse(rsp);
        }
        response.type = GDBResponse::OUTPUT_rsp;
        if (error) {
          response.message = "\nProgram Ended With an Error\n";
        } else {
          response.message = "\nProgram Correctly Ended\n";
        }
        this->connManager.sendResponse(response);

        // Now I really communicate to GDB that the program ended
        response.type = GDBResponse::W_rsp;
        if (error) {
          response.payload = SIGABRT;
        } else {
          response.payload = processorInstance.getExitValue();
        }
        this->connManager.sendResponse(response);
      }
    }

    ///Waits for an incoming request by the GDB debugger and, once it
    ///has been received, it routes it to the appropriate handler
    ///Returns whether we must be listening for other incoming data or not
    bool waitForRequest() {
      this->simulationPaused = true;
      GDBRequest req = connManager.processRequest();
      this->simulationPaused = false;
      switch (req.type) {
      case GDBRequest::QUEST_req:
        // ? request: it asks the target the reason why it halted
        return this->reqStopReason();
        break;
      case GDBRequest::EXCL_req:
        // ! request: it asks if extended mode is supported
        return this->emptyAction(req);
        break;
      case GDBRequest::c_req:
        // c request: Continue command
        return this->cont(req.address);
        break;
      case GDBRequest::C_req:
        // C request: Continue with signal command, currently not supported
        return this->emptyAction(req);
        break;
      case GDBRequest::D_req:
        // D request: disconnection from the remote target
        return this->detach(req);
        break;
      case GDBRequest::g_req:
        // g request: read general register
        return this->readRegisters();
        break;
      case GDBRequest::G_req:
        // G request: write general register
        return this->writeRegisters(req);
        break;
      case GDBRequest::H_req:
        // H request: multithreading stuff, not currently supported
        return this->emptyAction(req);
        break;
      case GDBRequest::i_req:
        // i request: single clock cycle step; currently it is not supported
        // since it requires advancing systemc by a specified ammont of
        // time equal to the clock cycle (or one of its multiple) and I still
        // have to think how to know the clock cycle of the processor and
        // how to awake again all the processors after simulation stopped again
        return this->emptyAction(req);
        break;
      case GDBRequest::I_req:
        // i request: signal and single clock cycle step
        return this->emptyAction(req);
        break;
      case GDBRequest::k_req:
        // i request: kill application: I simply call the sc_stop method
        return this->killApp();
        break;
      case GDBRequest::m_req:
        // m request: read memory
        return this->readMemory(req);
        break;
      case GDBRequest::M_req:
// case GDBRequest::X_req:
        // M request: write memory
        return this->writeMemory(req);
        break;
      case GDBRequest::p_req:
        // p request: register read
        return this->readRegister(req);
        break;
      case GDBRequest::P_req:
        // P request: register write
        return this->writeRegister(req);
        break;
      case GDBRequest::q_req:
        // q request: generic query
        return this->genericQuery(req);
        break;
      case GDBRequest::s_req:
        // s request: single step
        return this->doStep(req.address);
        break;
      case GDBRequest::S_req:
        // S request: single step with signal
        return this->emptyAction(req);
        break;
      case GDBRequest::t_req:
        // t request: backward search: currently not supported
        return this->emptyAction(req);
        break;
      case GDBRequest::T_req:
        // T request: thread stuff: currently not supported
        return this->emptyAction(req);
        break;
      case GDBRequest::v_req: {
        // Note that I support only the vCont packets; in particular, the only
        // supported actions are continue and stop
        std::size_t foundCont = req.command.find("Cont");
        if (foundCont == std::string::npos) {
          return this->emptyAction(req);
        }
        if (req.command.find_last_of('?') == (req.command.size() - 1)) {
          // Query of the supported commands: I support only the
          // c, and s commands
          return this->vContQuery(req);
        } else {
          req.command = req.command.substr(foundCont + 5);
          std::vector<std::string> lineElements;
          boost::split(lineElements, req.command, boost::is_any_of(";"));
          // Actual continue/step command; note that I should have only
          // one element in the vCont command (since only one thread is supported)
          if (lineElements.size() != 1) {
            GDBResponse resp;
            resp.type = GDBResponse::ERROR_rsp;
            this->connManager.sendResponse(resp);
            return true;
          }
          // Here I check whether I have to issue a continue or a step command
          if (lineElements[0][0] == 'c') {
            return this->cont();
          } else if (lineElements[0][0] == 's') {
            return this->doStep();
          } else {
            GDBResponse resp;
            resp.type = GDBResponse::ERROR_rsp;
            this->connManager.sendResponse(resp);
            return true;
          }
        }
        break;
      }
      case GDBRequest::z_req:
        // z request: breakpoint/watch removal
        return this->removeBreakWatch(req);
        break;
      case GDBRequest::Z_req:
        // z request: breakpoint/watch addition
        return this->addBreakWatch(req);
        break;
      case GDBRequest::INTR_req:
        // received an iterrupt from GDB: I pause simulation and signal GDB that I stopped
        return this->recvIntr();
        break;
      case GDBRequest::ERROR_req:
        this->isConnected = false;
        this->resumeExecution();
        this->breakEnabled = false;
        this->watchEnabled = false;
        return false;
        break;
      default:
        return this->emptyAction(req);
        break;
      }
      return false;
    }

    ///Method used to resume execution after GDB has issued
    ///the continue or step signal
    void resumeExecution() {
      // I'm going to restart execution, so I can again re-enable watch and break points
      this->breakEnabled = true;
      this->watchEnabled = true;
      this->simStartTime = sc_time_stamp().to_double();
      if (this->timeToGo > 0) {
        this->pauseEvent.notify(sc_time(this->timeToGo, SC_PS));
      }
    }

    /** Here start all the methods to handle the different GDB requests **/

    ///It does nothing, it simply sends an empty string back to the
    ///GDB debugger
    bool emptyAction(GDBRequest &req) {
      GDBResponse resp;
      resp.type = GDBResponse::NOT_SUPPORTED_rsp;
      this->connManager.sendResponse(resp);
      return true;
    }

    /// Queries the supported vCont commands; only the c and s commands
    /// are supported
    bool vContQuery(GDBRequest &req) {
      GDBResponse resp;
      resp.type = GDBResponse::CONT_rsp;
      resp.data.push_back('c');
      resp.data.push_back('s');
      this->connManager.sendResponse(resp);
      return true;
    }

    ///Asks for the reason why the processor is stopped
    bool reqStopReason() {
      this->awakeGDB();
      return true;
    }

    ///Reads the value of a register;
    bool readRegister(GDBRequest &req) {
      GDBResponse rsp;
      rsp.type = GDBResponse::REG_READ_rsp;
      try {
        if (req.reg < this->processorInstance.nGDBRegs()) {
          issueWidth regContent = this->processorInstance.readGDBReg(req.reg);
          this->valueToBytes(rsp.data, regContent);
        } else {
          this->valueToBytes(rsp.data, 0);
        }
      } catch (...) {
        this->valueToBytes(rsp.data, 0);
      }

      this->connManager.sendResponse(rsp);
      return true;
    }

    ///Reads the value of a memory location
    bool readMemory(GDBRequest &req) {
      GDBResponse rsp;
      rsp.type = GDBResponse::MEM_READ_rsp;

      for (unsigned int i = 0; i < req.length; i++) {
        try {
          unsigned char memContent = this->processorInstance.readCharMem(req.address + i);
          this->valueToBytes(rsp.data, memContent);
        } catch (...) {
          std::cerr << "GDB Stub: error in reading memory at address " << std::hex << std::showbase << req.address +
            i << std::endl;
          this->valueToBytes(rsp.data, 0);
        }
      }

      this->connManager.sendResponse(rsp);
      return true;
    }

    bool cont(unsigned int address = 0) {
      if (address != 0) {
        this->processorInstance.setPC(address);
      }

      // Now, I have to restart SystemC, since the processor
      // has to go on; note that actually SystemC restarts only
      // after all the gdbs has issued some kind of start command
      // (either a continue, a step ...)
      this->resumeExecution();
      return false;
    }

    bool detach(GDBRequest &req) {
      boost::mutex::scoped_lock lk(this->cleanupMutex);
      // First of all I have to perform some cleanup
      this->breakManager.clearAllBreaks();
      this->watchManager.clearAllWatchs();
      this->step = 0;
      this->isConnected = false;
      // Finally I can send a positive response
      GDBResponse resp;
      resp.type = GDBResponse::OK_rsp;
      this->connManager.sendResponse(resp);
      this->resumeExecution();
      this->breakEnabled = false;
      this->watchEnabled = false;
      return false;
    }

    bool readRegisters() {
      // I have to read all the general purpose registers and
      // send their content back to GDB
      GDBResponse resp;
      resp.type = GDBResponse::REG_READ_rsp;
      for (unsigned int i = 0; i < this->processorInstance.nGDBRegs(); i++) {
        try {
          issueWidth regContent = this->processorInstance.readGDBReg(i);
          this->valueToBytes(resp.data, regContent);
        } catch (...) {
          this->valueToBytes(resp.data, 0);
        }
      }
      this->connManager.sendResponse(resp);
      return true;
    }

    bool writeRegisters(GDBRequest &req) {
      std::vector<issueWidth> regContent;
      this->bytesToValue(req.data, regContent);
      typename std::vector<issueWidth>::iterator dataIter, dataEnd;
      bool error = false;
      unsigned int i = 0;
      for (dataIter = regContent.begin(), dataEnd = regContent.end();
           dataIter != dataEnd; dataIter++) {
        try {
          this->processorInstance.setGDBReg(*dataIter, i);
        } catch (...) {
          error = true;
        }
        i++;
      }

      GDBResponse resp;

      if ((i != (unsigned int)this->processorInstance.nGDBRegs()) || error) {
        resp.type = GDBResponse::ERROR_rsp;
      } else {
        resp.type = GDBResponse::OK_rsp;
      }
      this->connManager.sendResponse(resp);
      return true;
    }

    bool writeMemory(GDBRequest &req) {
      bool error = false;
      unsigned int bytes = 0;
      std::vector<unsigned char>::iterator dataIter, dataEnd;
      for (dataIter = req.data.begin(), dataEnd = req.data.end(); dataIter != dataEnd; dataIter++) {
        try {
          this->processorInstance.writeCharMem(req.address + bytes, *dataIter);
          bytes++;
        } catch (...) {
          std::cerr << "Error in writing in memory " << std::hex << std::showbase << (unsigned int)*dataIter <<
            " at address " << std::hex << std::showbase << req.address + bytes << std::endl;
          error = true;
          break;
        }
      }

      GDBResponse resp;
      resp.type = GDBResponse::OK_rsp;

      if ((bytes != (unsigned int)req.length) || error) {
        resp.type = GDBResponse::ERROR_rsp;
      }

      this->connManager.sendResponse(resp);
      return true;
    }

    bool writeRegister(GDBRequest &req) {
      GDBResponse rsp;
      if (req.reg <= this->processorInstance.nGDBRegs()) {
        try {
          this->processorInstance.setGDBReg(req.value, req.reg);
          rsp.type = GDBResponse::OK_rsp;
        } catch (...) {
          rsp.type = GDBResponse::ERROR_rsp;
        }
      } else {
        rsp.type = GDBResponse::ERROR_rsp;
      }
      this->connManager.sendResponse(rsp);
      return true;
    }

    bool killApp() {
      std::cerr << std::endl << "Killing the program according to GDB request" << std::endl << std::endl;
      this->isKilled = true;
      sc_stop();
      wait();
      return true;
    }

    bool doStep(unsigned int address = 0) {
      if (address != 0) {
        this->processorInstance.setPC(address);
      }

      this->step = 1;
      this->resumeExecution();
      return false;
    }

    bool recvIntr() {
      boost::mutex::scoped_lock lk(this->cleanupMutex);
      this->breakManager.clearAllBreaks();
      this->watchManager.clearAllWatchs();
      this->step = 0;
      this->isConnected = false;
      return true;
    }

    bool addBreakWatch(GDBRequest &req) {
      GDBResponse resp;
      switch (req.value) {
      case 0:
      case 1:
        if (this->breakManager.addBreakpoint(Breakpoint<issueWidth>::HW_break, req.address, req.length)) {
          resp.type = GDBResponse::OK_rsp;
        } else {
          resp.type = GDBResponse::ERROR_rsp;
        }
        break;
      case 2:
        if (this->watchManager.addWatchpoint(Watchpoint<issueWidth>::WRITE_watch, req.address, req.length)) {
          resp.type = GDBResponse::OK_rsp;
        } else {
          resp.type = GDBResponse::ERROR_rsp;
        }
        break;
      case 3:
        if (this->watchManager.addWatchpoint(Watchpoint<issueWidth>::READ_watch, req.address, req.length)) {
          resp.type = GDBResponse::OK_rsp;
        } else {
          resp.type = GDBResponse::ERROR_rsp;
        }
        break;
      case 4:
        if (this->watchManager.addWatchpoint(Watchpoint<issueWidth>::ACCESS_watch, req.address, req.length)) {
          resp.type = GDBResponse::OK_rsp;
        } else {
          resp.type = GDBResponse::ERROR_rsp;
        }
        break;
      default:
        resp.type = GDBResponse::NOT_SUPPORTED_rsp;
        break;
      }
      this->connManager.sendResponse(resp);
      return true;
    }

    bool removeBreakWatch(GDBRequest &req) {
      GDBResponse resp;
      if (this->breakManager.removeBreakpoint(req.address) or this->watchManager.removeWatchpoint(req.address,
            req.length)) {
        resp.type = GDBResponse::OK_rsp;
      } else {
        resp.type = GDBResponse::ERROR_rsp;
      }
      this->connManager.sendResponse(resp);
      return true;
    }

    // Note that to add additional custom commands you simply have to extend the following chain of
    // if clauses
    bool genericQuery(GDBRequest &req) {
      // I have to determine the query packet; in case it is Rcmd I deal with it
      GDBResponse resp;
      if (req.command != "Rcmd") {
        resp.type = GDBResponse::NOT_SUPPORTED_rsp;
      } else {
        // lets see which is the custom command being sent
        std::string::size_type spacePos = req.extension.find(' ');
        std::string custComm;
        if (spacePos == std::string::npos) {
          custComm = req.extension;
        } else {
          custComm = req.extension.substr(0, spacePos);
        }
        if (custComm == "go") {
          // Ok,  finally I got the right command: lets see for
          // how many nanoseconds I have to execute the continue
          this->timeToGo = boost::lexical_cast<double>(req.extension.substr(spacePos + 1)) * 1e3;
          if (this->timeToGo < 0) {
            resp.type = GDBResponse::OUTPUT_rsp;
            resp.message = "Please specify a positive offset";
            this->connManager.sendResponse(resp);
            resp.type = GDBResponse::NOT_SUPPORTED_rsp;
            this->timeToGo = 0;
          } else {
            resp.type = GDBResponse::OK_rsp;
          }
        } else if (custComm == "go_abs") {
          // This command specify to go up to a specified simulation time; the time is specified in nanoseconds
          this->timeToGo =
            boost::lexical_cast<double>(req.extension.substr(spacePos + 1)) * 1e3 - sc_time_stamp().to_double();
          if (this->timeToGo < 0) {
            resp.type = GDBResponse::OUTPUT_rsp;
            resp.message = "Please specify a positive offset";
            this->connManager.sendResponse(resp);
            resp.type = GDBResponse::NOT_SUPPORTED_rsp;
            this->timeToGo = 0;
          } else {
            resp.type = GDBResponse::OK_rsp;
          }
        } else if (custComm == "status") {
          // Returns the current status of the STUB
          resp.type = GDBResponse::OUTPUT_rsp;
          resp.message = "Current simulation time: " +
                         boost::lexical_cast<std::string>((sc_time_stamp().to_default_time_units()) / (sc_time(1, \
                                                                                                         SC_US)
                                                                                                       .
                                                                                                       to_default_time_units()))
                         + " (us)\n";
          if (this->timeToGo != 0) {
            resp.message += "Simulating for : " + boost::lexical_cast<std::string>(this->timeToGo) + " Nanoseconds\n";
          }
          this->connManager.sendResponse(resp);
          resp.type = GDBResponse::OK_rsp;
        } else if (custComm == "time") {
          // This command is simply a query to know the current simulation time
          resp.type = GDBResponse::OUTPUT_rsp;
          resp.message = "Current simulation time: " +
                         boost::lexical_cast<std::string>((sc_time_stamp().to_default_time_units()) / (sc_time(1, \
                                                                                                         SC_US)
                                                                                                       .
                                                                                                       to_default_time_units()))
                         + " (us)\n";
          this->connManager.sendResponse(resp);
          resp.type = GDBResponse::OK_rsp;
        } else if (custComm == "hist") {
          // Now I have to print the last n executed instructions; lets first get such number n
          resp.type = GDBResponse::OUTPUT_rsp;
          this->connManager.sendResponse(resp);
          resp.type = GDBResponse::OK_rsp;
        } else if (custComm == "help") {
          // This command is simply a query to know the current simulation time
          resp.type = GDBResponse::OUTPUT_rsp;
          resp.message = "Help about the custom GDB commands available for TRAP generated simulators:\n";
          resp.message += "   monitor help:       prints the current message\n";
          resp.message += "   monitor time:       returns the current simulation time\n";
          resp.message += "   monitor status:     returns the status of the simulation\n";
          this->connManager.sendResponse(resp);
          resp.message = "   monitor hist n:     prints the last n (up to a maximum of 1000) instructions\n";
          resp.message +=
            "   monitor go n:       after the \'continue\' command is given, it simulates for n (ns) starting from the current time\n";
          resp.message +=
            "   monitor go_abs n:   after the \'continue\' command is given, it simulates up to instant n (ns)\n";
          this->connManager.sendResponse(resp);
          resp.type = GDBResponse::OK_rsp;
        } else {
          resp.type = GDBResponse::NOT_SUPPORTED_rsp;
        }
      }
      this->connManager.sendResponse(resp);
      return true;
    }

    ///Separates the bytes which form an integer value and puts them
    ///into an array of bytes
    template<class ValueType>
    void valueToBytes(std::vector<char> &byteHolder, ValueType value, bool ConvertEndian = true) {
      if (this->processorInstance.matchEndian() || !ConvertEndian) {
        for (unsigned int i = 0; i < sizeof (ValueType); i++) {
          byteHolder.push_back((char)((value & (0x0FF << 8 * i)) >> 8 * i));
        }
      } else {
        for (int i = sizeof (ValueType) - 1; i >= 0; i--) {
          byteHolder.push_back((char)((value & (0x0FF << 8 * i)) >> 8 * i));
        }
      }
    }

    ///Converts a vector of bytes into a vector of integer values
    void bytesToValue(std::vector<unsigned char> &byteHolder, std::vector<issueWidth> &values) {
      for (unsigned int i = 0; i < byteHolder.size(); i += sizeof (issueWidth)) {
        issueWidth buf = 0;
        for (unsigned int k = 0; k < sizeof (issueWidth); k++) {
          buf |= (byteHolder[i + k] << 8 * k);
        }
        values.push_back(buf);
      }
    }
  public:
    SC_HAS_PROCESS(GDBStub);
    GDBStub(ABIIf<issueWidth> &processorInstance) :
      sc_module("debugger"),
      simulationPaused(false),
      connManager(processorInstance.matchEndian()),
      processorInstance(processorInstance),
      step(0),
      breakReached(NULL),
      breakEnabled(true),
      watchEnabled(true),
      isKilled(false),
      timeToGo(0),
      timeToJump(0),
      simStartTime(0),
      timeout(false),
      isConnected(false),
      firstRun(true) {
      SC_METHOD(pauseMethod);
      sensitive << this->pauseEvent;
      dont_initialize();

      end_module();
    }

    ///Method used to pause simulation
    void pauseMethod() {
      this->step = 2;
      this->timeout = true;
    }

    ///Overloading of the end_of_simulation method; it can be used to execute methods
    ///at the end of the simulation
    void end_of_simulation() {
      if (this->isConnected) {
        this->isKilled = false;
        this->signalProgramEnd();
        this->isKilled = true;
      }
    }

    ///Starts the connection with the GDB client
    void initialize(unsigned int port = 1500) {
      this->connManager.initialize(port);
      this->isConnected = true;
      // Now I have to listen for incoming GDB messages; this will
      // be done in a new thread.
      this->startThread();
    }

    ///Method called at every cycle from the processor's main loop
    bool newIssue(const issueWidth &curPC, const InstructionBase *curInstr) throw() {
      if (!this->firstRun) {
        this->checkStep();
        this->checkBreakpoint(curPC);
      } else {
        this->firstRun = false;
        this->breakEnabled = false;
        this->watchEnabled = false;
        while (this->waitForRequest()) {
        }
      }
      return false;
    }

    ///The debugger needs the pipeline to be empty only in case it is going to be stopped
    ///because, for exmple, we hitted a breakpoint or we are in step mode
    bool emptyPipeline(const issueWidth &curPC) const throw() {
      return !this->firstRun && (this->goingToStep() || this->goingToBreak(curPC));
    }

    ///Method called whenever a particular address is written into memory
#ifndef NDEBUG
    inline void notifyAddress(issueWidth address, unsigned int size) throw() {
#else
    inline void notifyAddress(issueWidth address, unsigned int size) {
#endif
      if (this->watchEnabled && this->watchManager.hasWatchpoint(address, size)) {
        this->watchReached = this->watchManager.getWatchPoint(address, size);
#ifndef NDEBUG
        if (this->watchReached == NULL) {
          THROW_EXCEPTION("I stopped because of a watchpoint, but no watchpoint was found");
        }
#endif
        this->setStopped(WATCH_stop);
      }
    }
};
}

#endif

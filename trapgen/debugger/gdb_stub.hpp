/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     gdb_stub->hpp
* @brief    This file is part of the TRAP runtime library.
* @details  Contains the methods necessary to communicate with GDB in order to
*           debug software running on simulators. Source code takes inspiration
*           from the linux kernel (sparc-stub.c) and from ac_gdb.H in the ArchC
*           sources. In order to debug the remote protocol (as a help in the
*           development of this stub, issue the "set debug remote 1" command in
*           GDB) "set remotelogfile file" logs all the remote communication on
*           the specified file.
* @author   Luca Fossati
* @author   Lillian Tadros (Technische Universitaet Dortmund)
* @date     2008-2013 Luca Fossati
*           2015-2016 Technische Universitaet Dortmund
* @copyright
*
* This file is part of TRAP.
*
* TRAP is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
* or see <http://www.gnu.org/licenses/>.
*
* (c) Luca Fossati, fossati@elet.polimi.it, fossati.l@gmail.com
*
* @todo     It seeems that watchpoints are completely ignored ... :-(
*           Sometimes segmentation fault when GDB is closed while the program is
*           still running. It seems there is a race condition with the GDB
*           thread.
*******************************************************************************/
#ifndef TRAP_GDBSTUB_HPP
#define TRAP_GDBSTUB_HPP

#include <csignal>
#ifndef SIGTRAP
#define SIGTRAP 5
#endif
#ifndef SIGQUIT
#define SIGQUIT 3
#endif

#include "breakpoint_manager.hpp"
#include "watchpoint_manager.hpp"
#include "gdb_connection_manager.hpp"
#include "modules/abi_if.hpp"
#include "modules/instruction.hpp"
#include "common/report.hpp"
#include "common/tools_if.hpp"

#include <systemc.h>

#include <boost/algorithm/string.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>

#include <vector>

namespace trap {

/**
 * @brief GDBStub
 */
template<class IssueWidth>
class GDBStub : public ToolsIf<IssueWidth>, public MemoryToolsIf<IssueWidth>, public sc_module {
  /// @name Types
  /// @{

  private:
  enum StopType {BREAK_STOP = 0, WATCH_STOP, STEP_STOP, SEG_STOP, TIMEOUT_STOP, PAUSED_STOP, UNK_STOP};

  /// Send and receive responses with the GDB debugger.
  struct GDBThread {
    GDBStub<IssueWidth>* gdb_stub;
    GDBThread(GDBStub<IssueWidth>* gdb_stub) : gdb_stub(gdb_stub) {}
    void operator()() {
      while (!gdb_stub->is_killed) {
        if (gdb_stub->connection_manager.check_interrupt()) {
          gdb_stub->step = 2;
        } else {
          // An Error happened: First of all I have to perform some cleanup
          if (!gdb_stub->is_killed) {
            boost::mutex::scoped_lock lk(gdb_stub->cleanup_mutex);
            gdb_stub->breakpoint_manager.clear_all_breaks();
            gdb_stub->watchpoint_manager.clear_all_watchpoints();
            gdb_stub->step = 0;
            gdb_stub->is_connected = false;
          }
          break;
        }
      }
    }
  };

  /// @} Types
  /// --------------------------------------------------------------------------
  /// @name Constructors and Destructors
  /// @{

  public:
  SC_HAS_PROCESS(GDBStub);
  GDBStub(ABIIf<IssueWidth>* processor) :
    sc_module(sc_core::sc_module_name("debugger")),
    simulation_paused(false),
    connection_manager(processor->match_endian()),
    processor(processor),
    step(0),
    breakpoint_reached(NULL),
    breakpoint_enabled(true),
    watchpoint_enabled(true),
    is_killed(false),
    time_to_go(0),
    time_to_jump(0),
    sim_start_time(0),
    timeout(false),
    is_connected(false),
    first_run(true) {
    SC_METHOD(pause_method);
    sensitive << this->pause_event;
    dont_initialize();

    end_module();
  }

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Interface Methods
  /// @{

  public:
  /// Execute methods at the end of the simulation.
  void end_of_simulation() {
    if (this->is_connected) {
      this->is_killed = false;
      this->signal_program_end();
      this->is_killed = true;
    }
  }

  /// ..........................................................................

  /// Starts the connection with the GDB client.
  void initialize(unsigned port = 1500) {
    this->connection_manager.initialize(port);
    this->is_connected = true;
    // Now I have to listen for incoming GDB messages; this will
    // be done in a new thread.
    this->start_thread();
  }

  /// ..........................................................................

  /// Called at every cycle from the processor's main loop.
  bool issue(const IssueWidth& cur_PC, const InstructionBase* cur_instr) throw() {
    if (!this->first_run) {
      this->check_step();
      this->check_breakpoint(cur_PC);
    } else {
      this->first_run = false;
      this->breakpoint_enabled = false;
      this->watchpoint_enabled = false;
      while (this->wait_for_request())
        ;
    }
    return false;
  }

  /// ..........................................................................

  /// Pause simulation.
  void pause_method() {
    this->step = 2;
    this->timeout = true;
  }

  /// ..........................................................................

  /// The debugger only needs the pipeline to be empty in case it is going to be
  /// stopped because a breakpoint was hit or the debugger is in step mode e.g.
  bool is_pipeline_empty(const IssueWidth& cur_PC) const throw() {
    return !this->first_run && (this->going_to_step() || this->going_to_break(cur_PC));
  }

  /// ..........................................................................

  /// Called whenever a particular address is written into memory.
#ifndef NDEBUG
  inline void notify_address(IssueWidth address, unsigned size) throw() {
#else
  inline void notify_address(IssueWidth address, unsigned size) {
#endif
    if (this->watchpoint_enabled && this->watchpoint_manager.has_watchpoint(address, size)) {
      this->watchpoint_reached = this->watchpoint_manager.get_watchpoint(address, size);
#ifndef NDEBUG
      if (this->watchpoint_reached == NULL) {
        THROW_EXCEPTION("No watchpoint found at given watchpoint address" << address << '.');
      }
#endif
      this->set_stopped(WATCH_STOP);
    }
  }

  /// @} Interface Methods
  /// --------------------------------------------------------------------------
  /// @name Internal Methods
  /// @{

  private:
  /// Checks if a breakpoint is present at the current address and halts execution.
#ifndef NDEBUG
  inline void check_breakpoint(const IssueWidth& address) {
#else
  inline void check_breakpoint(const IssueWidth& address) throw() {
#endif
    if (this->breakpoint_enabled && this->breakpoint_manager.has_breakpoint(address)) {
      this->breakpoint_reached = this->breakpoint_manager.get_breakpoint(address);
#ifndef NDEBUG
      if (this->breakpoint_reached == NULL) {
        THROW_EXCEPTION("No breakpoint found at given breakpoint address" << address << '.');
      }
#endif
      this->set_stopped(BREAK_STOP);
    }
  }

  /// ..........................................................................

  /// If true, the system is in step mode and execution needs to be stopped.
  inline bool going_to_break(const IssueWidth& address) const throw() {
    return this->breakpoint_enabled && this->breakpoint_manager.has_breakpoint(address);
  }

  /// ..........................................................................

  /// Checks if execution must be stopped because of a step command.
  inline void check_step() throw() {
    if (this->step == 1) {
      this->step++;
    } else if (this->step == 2) {
      this->step = 0;
      if (this->timeout) {
        this->timeout = false;
        this->set_stopped(TIMEOUT_STOP);
      } else {
        this->set_stopped(STEP_STOP);
      }
    }
  }

  /// ..........................................................................

  /// If true, the system is in step mode and execution needs to be stopped.
  inline bool going_to_step() const throw() {
    return this->step == 2;
  }

  /// ..........................................................................

  /// Starts the thread which will manage the connection with the GDB debugger.
  void start_thread() {
    GDBThread thread(this);
    boost::thread th(thread);
  }

  /// ..........................................................................

  /// Called when we need to asynchronously halt the execution of the processor
  /// that this instance of the stub is connected to. Usually used when one
  /// processor is halted and it has to communicated to the other processor that
  /// it needs to halt too. Note that this method halts SystemC execution and
  /// also starts new threads (one for each processor) which will deal with the
  /// communication with the stub. When a continue or step signal is met, the
  /// receving thread is killed. When all the threads are killed execution can
  /// be resumed (i.e. all GDBs pressed the resume button).
  /// This method is also called at the beginning of the simulation by the first
  /// processor that starts executing.
  void set_stopped(StopType stop_reason = UNK_STOP) {
    // Save current simulation time.
    double cur_sim_time = sc_time_stamp().to_double();

    // Now I have to behave differently depending on whether database support is
    // enabled or not. If yes, I do not stop simulation. If not, I have to stop
    // simulation in order to be able to inspect the content of the processor,
    // memory, etc. Computing the next simulation time instant.
    if (this->time_to_go > 0) {
      this->time_to_go -= (cur_sim_time - this->sim_start_time);
      if (this->time_to_go < 0) {
        this->time_to_go = 0;
      }
      this->sim_start_time = cur_sim_time;
    }
    // Disable break- and watchpoints.
    this->breakpoint_enabled = false;
    this->watchpoint_enabled = false;
    this->wake_gdb(stop_reason);
    // Pause simulation.
    while(this->wait_for_request())
      ;
  }

  /// ..........................................................................

  /// Sends a TRAP message to GDB to wake.
  void wake_gdb(StopType stop_reason = UNK_STOP) {
    switch (stop_reason) {
    case STEP_STOP: {
      GDBResponse response;
      response.type = GDBResponse::S_RSP;
      response.payload = SIGTRAP;
      this->connection_manager.send_response(response);
      break;
    }
    case BREAK_STOP: {
#ifndef NDEBUG
      if (this->breakpoint_reached == NULL) {
        THROW_EXCEPTION("NULL breakpoint found at given breakpoint address.");
      }
#endif

      GDBResponse response;
      response.type = GDBResponse::S_RSP;
      response.payload = SIGTRAP;
      this->connection_manager.send_response(response);
    break;}
    case WATCH_STOP: {
#ifndef NDEBUG
      if (this->watchpoint_reached == NULL) {
        THROW_EXCEPTION("NULL breakpoint found at given breakpoint address.");
      }
#endif

      GDBResponse response;
      response.type = GDBResponse::T_RSP;
      response.payload = SIGTRAP;
      std::pair<std::string, unsigned> info;
      info.second = this->watchpoint_reached->address;
      switch (this->watchpoint_reached->type) {
      case Watchpoint<IssueWidth>::WRITE_WATCH:
        info.first = "watch";
        break;
      case Watchpoint<IssueWidth>::READ_WATCH:
        info.first = "rwatch";
        break;
      case Watchpoint<IssueWidth>::ACCESS_WATCH:
        info.first = "awatch";
        break;
      default:
        info.first = "none";
        break;
      }
      response.size = sizeof(IssueWidth);
      response.info.push_back(info);
      this->connection_manager.send_response(response);
      break;
    }
    case SEG_STOP: {
      // An error has occurred during processor execution (illegal instruction,
      // reading out of memory,etc.
      GDBResponse response;
      response.type = GDBResponse::S_RSP;
      response.payload = SIGILL;
      this->connection_manager.send_response(response);
      break;
    }
    case TIMEOUT_STOP: {
      // The simulation time specified has elapsed, so simulation halted.
      GDBResponse resp;
      resp.type = GDBResponse::OUTPUT_RSP;
      resp.message = "Simulation completed - current simulation time=" + sc_time_stamp().to_string() + "ps\n";
      this->connection_manager.send_response(resp);
      this->connection_manager.send_interrupt();
      break;
    }
    case PAUSED_STOP: {
      // The simulation time specified has elapsed, so simulation halted.
      GDBResponse resp;
      resp.type = GDBResponse::OUTPUT_RSP;
      resp.message = "Simulation paused - current simulation time=" + sc_time_stamp().to_string() + "ps\n";
      this->connection_manager.send_response(resp);
      this->connection_manager.send_interrupt();
      break;
    }
    default:
      this->connection_manager.send_interrupt();
      break;
    }
  }

  /// ..........................................................................

  /// Signals to the GDB debugger that simulation ended. The error variable
  /// indicates whether the program ended with an error.
  void signal_program_end(bool error = false) {
    if (!this->is_killed || error) {
      GDBResponse response;
      // Print a message to the GDB console signaling to the user that the
      // program will end.
      if (error) {
        GDBResponse rsp;
        rsp.type = GDBResponse::ERROR_RSP;
        this->connection_manager.send_response(rsp);
      }
      response.type = GDBResponse::OUTPUT_RSP;
      if (error) {
        response.message = "\nProgram exited with an error.\n";
      } else {
        response.message = "\nProgram exited successfully.\n";
      }
      this->connection_manager.send_response(response);

      // Communicate to GDB that the program has ended.
      response.type = GDBResponse::W_RSP;
      if (error) {
        response.payload = SIGABRT;
      } else {
        response.payload = processor->get_exit_value();
      }
      this->connection_manager.send_response(response);
    }
  }

  /// ..........................................................................

  /// Waits for an incoming request by the GDB debugger and then routes it to
  /// the appropriate handler. Returns whether we must be listening for other
  /// incoming data or not.
  bool wait_for_request() {
    this->simulation_paused = true;
    GDBRequest req = connection_manager.process_request();
    this->simulation_paused = false;
    switch (req.type) {
    case GDBRequest::QUEST_REQ:
      // ? request: Asks the target for the halting reason.
      return this->request_stop_reason();
      break;
    case GDBRequest::EXCL_REQ:
      // ! request: Asks if extended mode is supported.
      return this->empty_action(req);
      break;
    case GDBRequest::c_REQ:
      // c request: Continue command.
      return this->cont(req.address);
      break;
    case GDBRequest::C_REQ:
      // C request: Continue with signal command, currently unsupported.
      return this->empty_action(req);
      break;
    case GDBRequest::D_REQ:
      // D request: Disconnection from the remote target.
      return this->detach(req);
      break;
    case GDBRequest::g_REQ:
      // g request: Read general register.
      return this->read_registers();
      break;
    case GDBRequest::G_REQ:
      // G request: Write general register.
      return this->write_registers(req);
      break;
    case GDBRequest::H_REQ:
      // H request: Multithreading stuff, currently unsupported.
      return this->empty_action(req);
      break;
    case GDBRequest::i_REQ:
      // i request: Single clock cycle step, currently unsupported since it
      // requires advancing SystemC by a specified ammont of time equal to (a
      // multiple of) the clock cycle. I still have to think how to find out
      // the clock cycle of the processor and how to wake all the processors
      // once again after simulation had stopped.
      return this->empty_action(req);
      break;
    case GDBRequest::I_REQ:
      // i request: Signal and single clock cycle step.
      return this->empty_action(req);
      break;
    case GDBRequest::k_REQ:
      // i request: Kill application: Call sc_stop().
      return this->kill();
      break;
    case GDBRequest::m_REQ:
      // m request: Read memory.
      return this->read_memory(req);
      break;
    case GDBRequest::M_REQ:
    //case GDBRequest::X_REQ:
      // M request: Write memory.
      return this->write_memory(req);
      break;
    case GDBRequest::p_REQ:
      // p request: Register read.
      return this->read_register(req);
      break;
    case GDBRequest::P_REQ:
      // P request: Register write.
      return this->write_register(req);
      break;
    case GDBRequest::q_REQ:
      // q request: Generic query.
      return this->generic_query(req);
      break;
    case GDBRequest::s_REQ:
      // s request: Single step.
      return this->do_step(req.address);
      break;
    case GDBRequest::S_REQ:
      // S request: Single step with signal.
      return this->empty_action(req);
      break;
    case GDBRequest::t_REQ:
      // t request: Backward search, currently unsupported.
      return this->empty_action(req);
      break;
    case GDBRequest::T_REQ:
      // T request: Thread stuff, currently unsupported.
      return this->empty_action(req);
      break;
    case GDBRequest::v_REQ: {
      // Note that I support only the vCont packets. In particular, the only
      // supported actions are continue and stop.
      std::size_t found_cont = req.command.find("Cont");
      if (found_cont == std::string::npos) {
        return this->empty_action(req);
      }
      if (req.command.find_last_of('?') == (req.command.size() - 1)) {
        // Query supported commands: Only the c, and s commands are supported.
        return this->vcont_query(req);
      } else {
        req.command = req.command.substr(found_cont + 5);
        std::vector<std::string> line_elements;
        boost::split(line_elements, req.command, boost::is_any_of(";"));
        // Actual continue/step command. Note that I should have only one
        // element in the vCont command (since only one thread is supported).
        if (line_elements.size() != 1) {
          GDBResponse resp;
          resp.type = GDBResponse::ERROR_RSP;
          this->connection_manager.send_response(resp);
          return true;
        }
        // Check whether to issue a continue or a step command.
        if (line_elements[0][0] == 'c') {
          return this->cont();
        } else if (line_elements[0][0] == 's') {
          return this->do_step();
        } else {
          GDBResponse resp;
          resp.type = GDBResponse::ERROR_RSP;
          this->connection_manager.send_response(resp);
          return true;
        }
      }
      break;
    }
    case GDBRequest::z_REQ:
      // z request: Remove break-/watchpoint.
      return this->remove_break_watch(req);
      break;
    case GDBRequest::Z_REQ:
      // z request: Add break-/watchpoint.
      return this->add_break_watch(req);
      break;
    case GDBRequest::INTR_REQ:
      // Received an interrupt from GDB: Pause simulation and signal to GDB that
      // I stopped.
      return this->received_interrupt();
      break;
    case GDBRequest::ERROR_REQ:
      this->is_connected = false;
      this->resume();
      this->breakpoint_enabled = false;
      this->watchpoint_enabled = false;
      return false;
      break;
    default:
      return this->empty_action(req);
      break;
    }
    return false;
  }

  /// ..........................................................................

  /// Used to resume execution after GDB has issued a continue or step signal.
  void resume() {
    // Will restart execution, so re-enable break- and watchpoints.
    this->breakpoint_enabled = true;
    this->watchpoint_enabled = true;
    this->sim_start_time = sc_time_stamp().to_double();
    if (this->time_to_go > 0) {
      this->pause_event.notify(sc_time(this->time_to_go, SC_PS));
    }
  }

  /// ..........................................................................

  /// Does nothing. Simply sends an empty string back to the GDB debugger.
  bool empty_action(GDBRequest& req) {
    GDBResponse resp;
    resp.type = GDBResponse::UNSUPPORTED_RSP;
    this->connection_manager.send_response(resp);
    return true;
  }

  /// ..........................................................................

  /// Queries the supported vCont commands. Only the c and s commands are
  /// supported.
  bool vcont_query(GDBRequest& req) {
    GDBResponse resp;
    resp.type = GDBResponse::CONT_RSP;
    resp.data.push_back('c');
    resp.data.push_back('s');
    this->connection_manager.send_response(resp);
    return true;
  }

  /// ..........................................................................

  /// Asks the reason why the processor has stopped.
  bool request_stop_reason() {
    this->wake_gdb();
    return true;
  }

  /// ..........................................................................

  /// Reads the value of a register.
  bool read_register(GDBRequest& req) {
    GDBResponse rsp;
    rsp.type = GDBResponse::REG_READ_RSP;
    try {
      if (req.reg < this->processor->num_gdb_regs()) {
        IssueWidth reg_content = this->processor->read_gdb_reg(req.reg);
        this->inttobytes(rsp.data, reg_content);
      } else {
        this->inttobytes(rsp.data, 0);
      }
    } catch(...) {
      this->inttobytes(rsp.data, 0);
    }

    this->connection_manager.send_response(rsp);
    return true;
  }

  /// ..........................................................................

  /// Reads the value of a memory location.
  bool read_memory(GDBRequest& req) {
    GDBResponse rsp;
    rsp.type = GDBResponse::MEM_READ_RSP;

    for (unsigned i = 0; i < req.length; i++) {
      try {
        unsigned char mem_content = this->processor->read_char_mem(req.address + i);
        this->inttobytes(rsp.data, mem_content);
      } catch(...) {
        std::cerr << "GDB Stub: Cannot reading memory at address " << std::hex << std::showbase << req.address +
          i << ".\n";
        this->inttobytes(rsp.data, 0);
      }
    }

    this->connection_manager.send_response(rsp);
    return true;
  }

  /// ..........................................................................

  bool cont(unsigned address = 0) {
    if (address != 0) {
      this->processor->set_PC(address);
    }

    // Restart SystemC, since the processor has to go on. Note that SystemC
    // actually restarts only after all gdbs have issued some kind of start
    // command (continue, step, ...).
    this->resume();
    return false;
  }

  /// ..........................................................................

  bool detach(GDBRequest& req) {
    boost::mutex::scoped_lock lk(this->cleanup_mutex);
    // First of all I have to perform some cleanup.
    this->breakpoint_manager.clear_all_breaks();
    this->watchpoint_manager.clear_all_watchpoints();
    this->step = 0;
    this->is_connected = false;
    // Finally I can send a positive response.
    GDBResponse resp;
    resp.type = GDBResponse::OK_RSP;
    this->connection_manager.send_response(resp);
    this->resume();
    this->breakpoint_enabled = false;
    this->watchpoint_enabled = false;
    return false;
  }

  /// ..........................................................................

  bool read_registers() {
    // I have to read all the general purpose registers and send their content
    // back to GDB.
    GDBResponse resp;
    resp.type = GDBResponse::REG_READ_RSP;
    for (unsigned i = 0; i < this->processor->num_gdb_regs(); i++) {
      try {
        IssueWidth reg_content = this->processor->read_gdb_reg(i);
        this->inttobytes(resp.data, reg_content);
      } catch(...) {
        this->inttobytes(resp.data, 0);
      }
    }
    this->connection_manager.send_response(resp);
    return true;
  }

  /// ..........................................................................

  bool write_registers(GDBRequest& req) {
    std::vector<IssueWidth> reg_content;
    this->bytestoints(req.data, reg_content);
    typename std::vector<IssueWidth>::iterator data_it, data_end;
    bool error = false;
    unsigned i = 0;
    for (data_it = reg_content.begin(), data_end = reg_content.end();
    data_it != data_end; data_it++) {
      try {
        this->processor->set_gdb_reg(*data_it, i);
      } catch(...) {
        error = true;
      }
      i++;
    }

    GDBResponse resp;

    if ((i != (unsigned)this->processor->num_gdb_regs()) || error) {
      resp.type = GDBResponse::ERROR_RSP;
    } else {
      resp.type = GDBResponse::OK_RSP;
    }
    this->connection_manager.send_response(resp);
    return true;
  }

  /// ..........................................................................

  bool write_memory(GDBRequest& req) {
    bool error = false;
    unsigned bytes = 0;
    std::vector<unsigned char>::iterator data_it, data_end;
    for (data_it = req.data.begin(), data_end = req.data.end(); data_it != data_end; data_it++) {
      try {
        this->processor->write_char_mem(req.address + bytes, *data_it);
        bytes++;
      } catch(...) {
        std::cerr << "Cannot write memory " << std::hex << std::showbase << (unsigned)*data_it <<
          " at address " << std::hex << std::showbase << req.address + bytes << ".\n";
        error = true;
        break;
      }
    }

    GDBResponse resp;
    resp.type = GDBResponse::OK_RSP;

    if ((bytes != (unsigned)req.length) || error) {
      resp.type = GDBResponse::ERROR_RSP;
    }

    this->connection_manager.send_response(resp);
    return true;
  }

  /// ..........................................................................

  bool write_register(GDBRequest& req) {
    GDBResponse rsp;
    if (req.reg <= this->processor->num_gdb_regs()) {
      try {
        this->processor->set_gdb_reg(req.value, req.reg);
        rsp.type = GDBResponse::OK_RSP;
      } catch(...) {
        rsp.type = GDBResponse::ERROR_RSP;
      }
    } else {
      rsp.type = GDBResponse::ERROR_RSP;
    }
    this->connection_manager.send_response(rsp);
    return true;
  }

  /// ..........................................................................

  bool kill() {
    std::cerr << std::endl << "Killing the program according to GDB request.\n\n";
    this->is_killed = true;
    sc_stop();
    wait();
    return true;
  }

  /// ..........................................................................

  bool do_step(unsigned address = 0) {
    if (address != 0) {
      this->processor->set_PC(address);
    }

    this->step = 1;
    this->resume();
    return false;
  }

  /// ..........................................................................

  bool received_interrupt() {
    boost::mutex::scoped_lock lk(this->cleanup_mutex);
    this->breakpoint_manager.clear_all_breaks();
    this->watchpoint_manager.clear_all_watchpoints();
    this->step = 0;
    this->is_connected = false;
    return true;
  }

  /// ..........................................................................

  bool add_break_watch(GDBRequest& req) {
    GDBResponse resp;
    switch (req.value) {
    case 0:
    case 1:
      if (this->breakpoint_manager.add_breakpoint(Breakpoint<IssueWidth>::HW_BREAK, req.address, req.length)) {
        resp.type = GDBResponse::OK_RSP;
      } else {
        resp.type = GDBResponse::ERROR_RSP;
      }
      break;
    case 2:
      if (this->watchpoint_manager.add_watchpoint(Watchpoint<IssueWidth>::WRITE_WATCH, req.address, req.length)) {
        resp.type = GDBResponse::OK_RSP;
      } else {
        resp.type = GDBResponse::ERROR_RSP;
      }
      break;
    case 3:
      if (this->watchpoint_manager.add_watchpoint(Watchpoint<IssueWidth>::READ_WATCH, req.address, req.length)) {
        resp.type = GDBResponse::OK_RSP;
      } else {
        resp.type = GDBResponse::ERROR_RSP;
      }
      break;
    case 4:
      if (this->watchpoint_manager.add_watchpoint(Watchpoint<IssueWidth>::ACCESS_WATCH, req.address, req.length)) {
        resp.type = GDBResponse::OK_RSP;
      } else {
        resp.type = GDBResponse::ERROR_RSP;
      }
      break;
    default:
      resp.type = GDBResponse::UNSUPPORTED_RSP;
      break;
    }
    this->connection_manager.send_response(resp);
    return true;
  }

  /// ..........................................................................

  bool remove_break_watch(GDBRequest& req) {
    GDBResponse resp;
    if (this->breakpoint_manager.remove_breakpoint(req.address) or this->watchpoint_manager.remove_watchpoint(req.address,
          req.length)) {
      resp.type = GDBResponse::OK_RSP;
    } else {
      resp.type = GDBResponse::ERROR_RSP;
    }
    this->connection_manager.send_response(resp);
    return true;
  }

  /// ..........................................................................

  // Note that to add additional custom commands you simply have to extend the
  // following chain of if clauses.
  bool generic_query(GDBRequest& req) {
    // Determine the query packet. In case it is Rcmd I deal with it.
    GDBResponse resp;
    if (req.command != "Rcmd") {
      resp.type = GDBResponse::UNSUPPORTED_RSP;
    } else {
      // What is the custom command being sent.
      std::string::size_type space_pos = req.extension.find(' ');
      std::string cust_comm;
      if (space_pos == std::string::npos) {
        cust_comm = req.extension;
      } else {
        cust_comm = req.extension.substr(0, space_pos);
      }
      if (cust_comm == "go") {
        // Got the right command. How many nanoseconds do I have to execute the
        // continue.
        this->time_to_go = boost::lexical_cast<double>(req.extension.substr(space_pos + 1)) * 1e3;
        if (this->time_to_go < 0) {
          resp.type = GDBResponse::OUTPUT_RSP;
          resp.message = "Please specify a positive offset.";
          this->connection_manager.send_response(resp);
          resp.type = GDBResponse::UNSUPPORTED_RSP;
          this->time_to_go = 0;
        } else {
          resp.type = GDBResponse::OK_RSP;
        }
      } else if (cust_comm == "go_abs") {
        // Go up to a specified simulation time (ns).
        this->time_to_go =
          boost::lexical_cast<double>(req.extension.substr(space_pos + 1)) * 1e3 - sc_time_stamp().to_double();
        if (this->time_to_go < 0) {
          resp.type = GDBResponse::OUTPUT_RSP;
          resp.message = "Please specify a positive offset.";
          this->connection_manager.send_response(resp);
          resp.type = GDBResponse::UNSUPPORTED_RSP;
          this->time_to_go = 0;
        } else {
          resp.type = GDBResponse::OK_RSP;
        }
      } else if (cust_comm == "status") {
        // Returns the current status of the stub.
        resp.type = GDBResponse::OUTPUT_RSP;
        resp.message = "Current simulation time=" +
                        boost::lexical_cast<std::string>((sc_time_stamp().to_default_time_units())
                                                        / (sc_time(1, SC_US).to_default_time_units()))
                        + "us.\n";
        if (this->time_to_go != 0) {
          resp.message += "Simulating for " + boost::lexical_cast<std::string>(this->time_to_go) + "ns.\n";
        }
        this->connection_manager.send_response(resp);
        resp.type = GDBResponse::OK_RSP;
      } else if (cust_comm == "time") {
        // Query current simulation time.
        resp.type = GDBResponse::OUTPUT_RSP;
        resp.message = "Current simulation time=" +
                        boost::lexical_cast<std::string>((sc_time_stamp().to_default_time_units())
                                                        / (sc_time(1, SC_US).to_default_time_units()))
                        + "us.\n";
        this->connection_manager.send_response(resp);
        resp.type = GDBResponse::OK_RSP;
      } else if (cust_comm == "hist") {
        // Print the last n executed instructions. First find out n.
        resp.type = GDBResponse::OUTPUT_RSP;
#ifndef ENABLE_HISTORY
        resp.message = "\nInstruction history not enabled at compile time. Reconfigure the project with the --enable-history option.\n\n";
#else
        unsigned history_len = 0;
        try {
          history_len = boost::lexical_cast<unsigned>(req.extension.substr(space_pos + 1));
        }
        catch(...) {
          resp.message = "\nInvalid instruction history buffer length.\n\n";
        }
        if (history_len > 1000) {
          resp.message = "\nInstruction history buffer length too large, expected <=1000.\n\n";
        }
        // Print the history.
        boost::circular_buffer<HistoryInstrType>& history_queue = processor->get_history();
        std::vector<std::string> history_vec;
        boost::circular_buffer<HistoryInstrType>::const_reverse_iterator history_queue_it, history_queue_end;
        unsigned history_read = 0;
        for (history_read = 0, history_queue_it = history_queue.rbegin(), history_queue_end = history_queue.rend(); history_queue_it != history_queue_end && history_read < history_len; history_queue_it++, history_read++) {
          history_vec.push_back(history_queue_it->get_mnemonic());
        }
        resp.message += "\nAddress\t\tname\t\t\tmnemonic\t\tcycle\n\n";
        std::vector<std::string>::const_reverse_iterator history_vector_it, history_vector_end;
        unsigned sent_lines = 0;
        for (history_vector_it = history_vec.rbegin(), history_vector_end = history_vec.rend(); history_vector_it != history_vector_end; history_vector_it++) {
          resp.message += *history_vector_it + "\n";
          sent_lines++;
          if (sent_lines == 5) {
            sent_lines = 0;
            this->connection_manager.send_response(resp);
            resp.message = "";
          }
        }
#endif
        this->connection_manager.send_response(resp);
        resp.type = GDBResponse::OK_RSP;
      } else if (cust_comm == "help") {
        // Query the current simulation time.
        resp.type = GDBResponse::OUTPUT_RSP;
        resp.message = "Help about the custom GDB commands available for TRAP generated simulators:\n";
        resp.message += "   monitor help:       prints the current message\n";
        resp.message += "   monitor time:       returns the current simulation time\n";
        resp.message += "   monitor status:     returns the status of the simulation\n";
        this->connection_manager.send_response(resp);
        resp.message = "   monitor hist n:     prints the last n (up to a maximum of 1000) instructions\n";
        resp.message += "   monitor go n:       after the \'continue\' command is given, it simulates for n (ns) starting from the current time\n";
        resp.message += "   monitor go_abs n:   after the \'continue\' command is given, it simulates up to instant n (ns)\n";
        this->connection_manager.send_response(resp);
        resp.type = GDBResponse::OK_RSP;
      } else {
        resp.type = GDBResponse::UNSUPPORTED_RSP;
      }
    }
    this->connection_manager.send_response(resp);
    return true;
  }

  /// ..........................................................................

  /// Separates the bytes which form an integer value and puts them into an
  /// array of bytes.
  template<class ValueType>
  void inttobytes(std::vector<char>& byte_vec, ValueType value, bool convert_endian = true) {
    if (this->processor->match_endian() || !convert_endian) {
      for (unsigned i = 0; i < sizeof(ValueType); i++) {
        byte_vec.push_back((char)((value & (0x0FF << 8 * i)) >> 8 * i));
      }
    } else {
      for (int i = sizeof(ValueType) - 1; i >= 0; i--) {
        byte_vec.push_back((char)((value & (0x0FF << 8 * i)) >> 8 * i));
      }
    }
  }

  /// ..........................................................................

  /// Converts a vector of bytes into a vector of integer values.
  void bytestoints(std::vector<unsigned char>& byte_vec, std::vector<IssueWidth>& int_vec) {
    for (unsigned i = 0; i < byte_vec.size(); i += sizeof(IssueWidth)) {
      IssueWidth buf = 0;
      for (unsigned k = 0; k < sizeof(IssueWidth); k++) {
        buf |= (byte_vec[i + k] << 8 * k);
      }
      int_vec.push_back(buf);
    }
  }

  /// @} Internal Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  public:
  bool simulation_paused;

  private:
  /// Manages the connection between the GDB target stub (this class) and the
  /// GDB debugger.
  GDBConnectionManager connection_manager;

  /// Interface for communication with the internal processor's structure.
  ABIIf<IssueWidth>* processor;

  /// Handles the breakpoints which have been set in the system.
  BreakpointManager<IssueWidth> breakpoint_manager;

  /// Handles the watchpoints which have been set in the system.
  WatchpointManager<IssueWidth> watchpoint_manager;

  /// Determines whether the processor has to halt due to a step command.
  unsigned step;

  /// Keeps track of the last breakpoint encountered by this processor->
  Breakpoint<IssueWidth>* breakpoint_reached;

  /// Keeps track of the last watchpoint encountered by this processor->
  Watchpoint<IssueWidth>* watchpoint_reached;

  /// Specifies whether breakpoints are enabled or not.
  bool breakpoint_enabled;

  /// Specifies whether watchpoints are enabled or not.
  bool watchpoint_enabled;

  /// Specifies whether GDB server side killed the simulation.
  bool is_killed;

  /// In case we decided to run the simulation only for a given amount of
  /// time, this variable contains that time.
  double time_to_go;

  /// In case we decided to jump onwards or backwards for a given amount of
  /// time, this variable contains that time.
  double time_to_jump;

  /// In case the simulation is run only for a given amount of time,
  /// this variable contains the simulation time at that start time.
  double sim_start_time;

  /// Specifies that we have to stop because a timeout was encountered.
  bool timeout;

  /// Event used to manage execution for a given amount of time.
  sc_event pause_event;

  /// Condition used to stop processor execution until simulation is restarted.
  boost::condition gdb_paused_condition;

  /// Mutex used to access the condition.
  boost::mutex global_mutex;

  /// Specifies if GDB is connected to this stub or not.
  bool is_connected;

  /// Specifies that the first run is being made.
  bool first_run;

  /// Mutex controlling the cleanup of GDB status.
  boost::mutex cleanup_mutex;

  /// @} Data
}; // class GDBStub

} // namespace trap

/// ****************************************************************************
#endif

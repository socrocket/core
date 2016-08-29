/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     osemu.hpp
* @brief    This file is part of the TRAP runtime library.
* @details
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
*******************************************************************************/
#ifndef TRAP_OSEMULATOR_HPP
#define TRAP_OSEMULATOR_HPP

#include "osemu_base.hpp"
#include "syscall.hpp"
#include "modules/abi_if.hpp"
#include "modules/instruction.hpp"
#include "common/tools_if.hpp"

#ifndef EXTERNAL_BFD
#include "elfloader/elf_frontend.hpp"
#else
#include "bfdWrapper.hpp"
#define BFDFrontend BFDWrapper
#endif

#include <systemc.h>

#include <map>
#include <string>

#ifdef __GNUC__
#ifdef __GNUC_MINOR__
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 3)
#include <tr1/unordered_map>
#define template_map std::tr1::unordered_map
#else
#include <ext/hash_map>
#define  template_map __gnu_cxx::hash_map
#endif
#else
#include <ext/hash_map>
#define  template_map __gnu_cxx::hash_map
#endif
#else
#ifdef _WIN32
#include <hash_map>
#define  template_map stdext::hash_map
#else
#include <map>
#define  template_map std::map
#endif
#endif

namespace trap {

/**
 * @brief OSEmulator
 */
template<class IssueWidth>
class OSEmulator : public ToolsIf<IssueWidth>, public OSEmulatorBase {
  /// @name Constructors and Destructors
  /// @{

  public:
  OSEmulator(ABIIf<IssueWidth>* processor) :
    processor(processor) {
    this->syscalls_end = this->syscalls.end();
  } // OSEmulator()

  /// ..........................................................................

  ~OSEmulator() {
    reset();
  } // ~OSEmulator()

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Interface Methods
  /// @{

  public:
  /// Resets the whole concurrency emulator, reinitializing it and preparing it
  /// for a new simulation.
  void reset() {
    this->syscalls.clear();
    this->syscalls_end = this->syscalls.end();
    this->env.clear();
    this->sysconf.clear();
    this->program_args.clear();
    this->heap_ptr = 0;
    this->group_ids.clear();
    this->num_programs = 0;
  } // reset()

  /// ..........................................................................

  bool register_syscall(IssueWidth address, Syscall<IssueWidth>& callback) {
      typename template_map<IssueWidth, Syscall<IssueWidth>* >::iterator syscall_found = this->syscalls.find(address);
      if (syscall_found != this->syscalls.end()) {
        int num_match = 0;
        typename template_map<IssueWidth, Syscall<IssueWidth>* >::iterator syscall_it, syscall_end;
        for (syscall_it = this->syscalls.begin(), syscall_end = this->syscalls.end(); syscall_it != syscall_end; syscall_it++) {
          if (syscall_it->second == syscall_found->second)
            num_match++;
        }
        // Only delete the callback if the last reference to it is about to be
        // replaced.
        if (num_match <= 1) {
          delete syscall_found->second;
        }
      }

      this->syscalls[address] = &callback;
      this->syscalls_end = this->syscalls.end();

      return true;
  } // register_syscall()

  /// ..........................................................................

  bool register_syscall(std::string func_name, Syscall<IssueWidth>& callback) {
      bool valid = false;
      unsigned address = this->elf_frontend->get_sym_addr(func_name, valid);
      if (!valid) {
        return false;
      }

      typename template_map<IssueWidth, Syscall<IssueWidth>* >::iterator syscall_found = this->syscalls.find(address);
      if (syscall_found != this->syscalls.end()) {
        int num_match = 0;
        typename template_map<IssueWidth, Syscall<IssueWidth>* >::iterator syscall_it, syscall_end;
        for (syscall_it = this->syscalls.begin(), syscall_end = this->syscalls.end(); syscall_it != syscall_end; syscall_it++) {
          if (syscall_it->second == syscall_found->second)
            num_match++;
        }
        // Only delete the callback if the last reference to it is about to be
        // replaced.
        if (num_match <= 1) {
          delete syscall_found->second;
        }
      }

      this->syscalls[address] = &callback;
      this->syscalls_end = this->syscalls.end();

      return true;
  } // register_syscall()

  /// ..........................................................................

  std::set<std::string> get_registered_functions() {
    std::set<std::string> registered_functions;
    typename template_map<IssueWidth, Syscall<IssueWidth>* >::iterator syscall_it, syscall_end;
    for (syscall_it = this->syscalls.begin(), syscall_end = this->syscalls.end(); syscall_it != syscall_end; syscall_it++) {
      registered_functions.insert(this->elf_frontend->symbol_at(syscall_it->first));
    }
    return registered_functions;
  } // get_registered_functions()

  /// ..........................................................................

  void init_sys_calls(std::string exec_name, std::map<std::string, sc_time> latencies, int group = 0) {
    if (find(group_ids.begin(), group_ids.end(), group) == group_ids.end()) {
      group_ids.push_back(group);
      num_programs++;
    }

    // Initialize the heap pointer according to the group it belongs to.
    this->heap_ptr = (unsigned)this->processor->get_code_limit() + sizeof(IssueWidth);

    this->elf_frontend = &ELFFrontend::get_instance(exec_name);
    // Perform the registration of the basic system calls.
    bool registered = false;

    openSyscall<IssueWidth>* a = NULL;
    if (latencies.find("open") != latencies.end()) {
      a = new openSyscall<IssueWidth>(this->processor, this, latencies["open"]);
    } else if (latencies.find("_open") != latencies.end()) {
      a = new openSyscall<IssueWidth>(this->processor, this, latencies["_open"]);
    } else {
      a = new openSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("open", *a);
    registered |= this->register_syscall("_open", *a);
    if (!registered) {
      delete a;
    }
    creatSyscall<IssueWidth>* b = NULL;
    if (latencies.find("creat") != latencies.end()) {
      b = new creatSyscall<IssueWidth>(this->processor, this, latencies["creat"]);
    } else if (latencies.find("_creat") != latencies.end()) {
      b = new creatSyscall<IssueWidth>(this->processor, this, latencies["_creat"]);
    } else {
      b = new creatSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("creat", *b);
    registered |= this->register_syscall("_creat", *b);
    if (!registered) {
      delete b;
    }
    closeSyscall<IssueWidth>* c = NULL;
    if (latencies.find("close") != latencies.end()) {
      c = new closeSyscall<IssueWidth>(this->processor, this, latencies["close"]);
    } else if (latencies.find("_close") != latencies.end()) {
      c = new closeSyscall<IssueWidth>(this->processor, this, latencies["_close"]);
    } else {
      c = new closeSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("close", *c);
    registered |= this->register_syscall("_close", *c);
    if (!registered) {
      delete c;
    }
    readSyscall<IssueWidth>* d = NULL;
    if (latencies.find("read") != latencies.end()) {
      d = new readSyscall<IssueWidth>(this->processor, this, latencies["read"]);
    } else if (latencies.find("_read") != latencies.end()) {
      d = new readSyscall<IssueWidth>(this->processor, this, latencies["_read"]);
    } else {
      d = new readSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("read", *d);
    registered |= this->register_syscall("_read", *d);
    if (!registered) {
      delete d;
    }
    writeSyscall<IssueWidth>* e = NULL;
    if (latencies.find("write") != latencies.end()) {
      e = new writeSyscall<IssueWidth>(this->processor, this, latencies["write"]);
    } else if (latencies.find("_write") != latencies.end()) {
      e = new writeSyscall<IssueWidth>(this->processor, this, latencies["_write"]);
    } else {
      e = new writeSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("write", *e);
    registered |= this->register_syscall("_write", *e);
    if (!registered) {
      delete e;
    }
    isattySyscall<IssueWidth>* f = NULL;
    if (latencies.find("isatty") != latencies.end()) {
      f = new isattySyscall<IssueWidth>(this->processor, this, latencies["isatty"]);
    } else if (latencies.find("_isatty") != latencies.end()) {
      f = new isattySyscall<IssueWidth>(this->processor, this, latencies["_isatty"]);
    } else {
      f = new isattySyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("isatty", *f);
    registered |= this->register_syscall("_isatty", *f);
    if (!registered) {
      delete f;
    }
    sbrkSyscall<IssueWidth>* g = NULL;
    if (latencies.find("sbrk") != latencies.end()) {
      g = new sbrkSyscall<IssueWidth>(this->processor, this, latencies["sbrk"]);
    } else if (latencies.find("_sbrk") != latencies.end()) {
      g = new sbrkSyscall<IssueWidth>(this->processor, this, latencies["_sbrk"]);
    } else {
      g = new sbrkSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("sbrk", *g);
    registered |= this->register_syscall("_sbrk", *g);
    if (!registered) {
      delete g;
    }
    lseekSyscall<IssueWidth>* h = NULL;
    if (latencies.find("lseek") != latencies.end()) {
      h = new lseekSyscall<IssueWidth>(this->processor, this, latencies["lseek"]);
    } else if (latencies.find("_lseek") != latencies.end()) {
      h = new lseekSyscall<IssueWidth>(this->processor, this, latencies["_lseek"]);
    } else {
      h = new lseekSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("lseek", *h);
    registered |= this->register_syscall("_lseek", *h);
    if (!registered) {
      delete h;
    }
    fstatSyscall<IssueWidth>* i = NULL;
    if (latencies.find("fstat") != latencies.end()) {
      i = new fstatSyscall<IssueWidth>(this->processor, this, latencies["fstat"]);
    } else if (latencies.find("_fstat") != latencies.end()) {
      i = new fstatSyscall<IssueWidth>(this->processor, this, latencies["_fstat"]);
    } else {
      i = new fstatSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("fstat", *i);
    registered |= this->register_syscall("_fstat", *i);
    if (!registered) {
      delete i;
    }
    _exitSyscall<IssueWidth>* j = NULL;
    if (latencies.find("_exit") != latencies.end()) {
      j = new _exitSyscall<IssueWidth>(this->processor, this, latencies["_exit"]);
    } else {
      j = new _exitSyscall<IssueWidth>(this->processor, this);
    }
    if (!this->register_syscall("_exit", *j)) {
      delete j;
    }
    timesSyscall<IssueWidth>* k = NULL;
    if (latencies.find("times") != latencies.end()) {
      k = new timesSyscall<IssueWidth>(this->processor, this, latencies["times"]);
    } else if (latencies.find("_times") != latencies.end()) {
      k = new timesSyscall<IssueWidth>(this->processor, this, latencies["_times"]);
    } else {
      k = new timesSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("times", *k);
    registered |= this->register_syscall("_times", *k);
    if (!registered) {
      delete k;
    }
    timeSyscall<IssueWidth>* l = NULL;
    if (latencies.find("time") != latencies.end()) {
      l = new timeSyscall<IssueWidth>(this->processor, this, latencies["time"]);
    } else if (latencies.find("_time") != latencies.end()) {
      l = new timeSyscall<IssueWidth>(this->processor, this, latencies["_time"]);
    } else {
      l = new timeSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("time", *l);
    registered |= this->register_syscall("_time", *l);
    if (!registered) {
      delete l;
    }
    randomSyscall<IssueWidth>* m = NULL;
    if (latencies.find("random") != latencies.end()) {
      m = new randomSyscall<IssueWidth>(this->processor, this, latencies["random"]);
    } else if (latencies.find("_random") != latencies.end()) {
      m = new randomSyscall<IssueWidth>(this->processor, this, latencies["_random"]);
    } else {
      m = new randomSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("random", *m);
    registered |= this->register_syscall("_random", *m);
    if (!registered) {
      delete m;
    }
    getpidSyscall<IssueWidth>* n = NULL;
    if (latencies.find("getpid") != latencies.end()) {
      n = new getpidSyscall<IssueWidth>(this->processor, this, latencies["getpid"]);
    } else if (latencies.find("_getpid") != latencies.end()) {
      n = new getpidSyscall<IssueWidth>(this->processor, this, latencies["_getpid"]);
    } else {
      n = new getpidSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("getpid", *n);
    registered |= this->register_syscall("_getpid", *n);
    if (!registered) {
      delete n;
    }
    chmodSyscall<IssueWidth>* o = NULL;
    if (latencies.find("chmod") != latencies.end()) {
      o = new chmodSyscall<IssueWidth>(this->processor, this, latencies["chmod"]);
    } else if (latencies.find("_chmod") != latencies.end()) {
      o = new chmodSyscall<IssueWidth>(this->processor, this, latencies["_chmod"]);
    } else {
      o = new chmodSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("chmod", *o);
    registered |= this->register_syscall("_chmod", *o);
    if (!registered) {
      delete o;
    }
    dupSyscall<IssueWidth>* p = NULL;
    if (latencies.find("dup") != latencies.end()) {
      p = new dupSyscall<IssueWidth>(this->processor, this, latencies["dup"]);
    } else if (latencies.find("_dup") != latencies.end()) {
      p = new dupSyscall<IssueWidth>(this->processor, this, latencies["_dup"]);
    } else {
      p = new dupSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("dup", *p);
    registered |= this->register_syscall("_dup", *p);
    if (!registered) {
      delete p;
    }
    dup2Syscall<IssueWidth>* q = NULL;
    if (latencies.find("dup2") != latencies.end()) {
      q = new dup2Syscall<IssueWidth>(this->processor, this, latencies["dup2"]);
    } else if (latencies.find("_dup2") != latencies.end()) {
      q = new dup2Syscall<IssueWidth>(this->processor, this, latencies["_dup2"]);
    } else {
      q = new dup2Syscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("dup2", *q);
    registered |= this->register_syscall("_dup2", *q);
    if (!registered) {
      delete q;
    }
    getenvSyscall<IssueWidth>* r = NULL;
    if (latencies.find("getenv") != latencies.end()) {
      r = new getenvSyscall<IssueWidth>(this->processor, this, latencies["getenv"]);
    } else if (latencies.find("_getenv") != latencies.end()) {
      r = new getenvSyscall<IssueWidth>(this->processor, this, latencies["_getenv"]);
    } else {
      r = new getenvSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("getenv", *r);
    registered |= this->register_syscall("_getenv", *r);
    if (!registered) {
      delete r;
    }
    sysconfSyscall<IssueWidth>* s = NULL;
    if (latencies.find("sysconf") != latencies.end()) {
      s = new sysconfSyscall<IssueWidth>(this->processor, this, latencies["sysconf"]);
    } else {
      s = new sysconfSyscall<IssueWidth>(this->processor, this);
    }
    if (!this->register_syscall("sysconf", *s)) {
      delete s;
    }
    gettimeofdaySyscall<IssueWidth>* t = NULL;
    if (latencies.find("gettimeofday") != latencies.end()) {
      t = new gettimeofdaySyscall<IssueWidth>(this->processor, this, latencies["gettimeofday"]);
    } else if (latencies.find("_gettimeofday") != latencies.end()) {
      t = new gettimeofdaySyscall<IssueWidth>(this->processor, this, latencies["_gettimeofday"]);
    } else {
      t = new gettimeofdaySyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("gettimeofday", *t);
    registered |= this->register_syscall("_gettimeofday", *t);
    if (!registered) {
      delete t;
    }
    killSyscall<IssueWidth>* u = NULL;
    if (latencies.find("kill") != latencies.end()) {
      u = new killSyscall<IssueWidth>(this->processor, this, latencies["kill"]);
    } else if (latencies.find("_kill") != latencies.end()) {
      u = new killSyscall<IssueWidth>(this->processor, this, latencies["_kill"]);
    } else {
      u = new killSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("kill", *u);
    registered |= this->register_syscall("_kill", *u);
    if (!registered) {
      delete u;
    }
    errorSyscall<IssueWidth>* v = NULL;
    if (latencies.find("error") != latencies.end()) {
      v = new errorSyscall<IssueWidth>(this->processor, this, latencies["error"]);
    } else if (latencies.find("_error") != latencies.end()) {
      v = new errorSyscall<IssueWidth>(this->processor, this, latencies["_error"]);
    } else {
      v = new errorSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("error", *v);
    registered |= this->register_syscall("_error", *v);
    if (!registered) {
      delete v;
    }
    chownSyscall<IssueWidth>* w = NULL;
    if (latencies.find("chown") != latencies.end()) {
      w = new chownSyscall<IssueWidth>(this->processor, this, latencies["chown"]);
    } else if (latencies.find("_chown") != latencies.end()) {
      w = new chownSyscall<IssueWidth>(this->processor, this, latencies["_chown"]);
    } else {
      w = new chownSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("chown", *w);
    registered |= this->register_syscall("_chown", *w);
    if (!registered) {
      delete w;
    }
    unlinkSyscall<IssueWidth>* x = NULL;
    if (latencies.find("unlink") != latencies.end()) {
      x = new unlinkSyscall<IssueWidth>(this->processor, this, latencies["unlink"]);
    } else if (latencies.find("_unlink") != latencies.end()) {
      x = new unlinkSyscall<IssueWidth>(this->processor, this, latencies["_unlink"]);
    } else {
      x = new unlinkSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("unlink", *x);
    registered |= this->register_syscall("_unlink", *x);
    if (!registered) {
      delete x;
    }
    usleepSyscall<IssueWidth>* y = NULL;
    if (latencies.find("usleep") != latencies.end()) {
      y = new usleepSyscall<IssueWidth>(this->processor, this, latencies["usleep"]);
    } else if (latencies.find("_usleep") != latencies.end()) {
      y = new usleepSyscall<IssueWidth>(this->processor, this, latencies["_usleep"]);
    } else {
      y = new usleepSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("usleep", *y);
    registered |= this->register_syscall("_usleep", *y);
    if (!registered) {
      delete y;
    }
    statSyscall<IssueWidth>* z = NULL;
    if (latencies.find("stat") != latencies.end()) {
      z = new statSyscall<IssueWidth>(this->processor, this, latencies["stat"]);
    } else if (latencies.find("_stat") != latencies.end()) {
      z = new statSyscall<IssueWidth>(this->processor, this, latencies["_stat"]);
    } else {
      z = new statSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("stat", *z);
    registered |= this->register_syscall("_stat", *z);
    if (!registered) {
      delete z;
    }
    lstatSyscall<IssueWidth>* A = NULL;
    if (latencies.find("lstat") != latencies.end()) {
      A = new lstatSyscall<IssueWidth>(this->processor, this, latencies["lstat"]);
    } else if (latencies.find("_lstat") != latencies.end()) {
      A = new lstatSyscall<IssueWidth>(this->processor, this, latencies["_lstat"]);
    } else {
      A = new lstatSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("lstat", *A);
    registered |= this->register_syscall("_lstat", *A);
    if (!registered) {
      delete A;
    }
    utimesSyscall<IssueWidth>* B = NULL;
    if (latencies.find("utimes") != latencies.end()) {
      B = new utimesSyscall<IssueWidth>(this->processor, this, latencies["utimes"]);
    } else if (latencies.find("_utimes") != latencies.end()) {
      B = new utimesSyscall<IssueWidth>(this->processor, this, latencies["_utimes"]);
    } else {
      B = new utimesSyscall<IssueWidth>(this->processor, this);
    }
    registered = this->register_syscall("utimes", *B);
    registered |= this->register_syscall("_utimes", *B);
    if (!registered) {
      delete B;
    }

    mainSyscall<IssueWidth>* mainCallBack = new mainSyscall<IssueWidth>(this->processor, this);
    if (!this->register_syscall("main", *mainCallBack)) {
      THROW_EXCEPTION("Unable to find main function in current application.");
    }
  } // init_sys_calls()

  /// ..........................................................................

  void init_sys_calls(std::string exec_name, int group = 0) {
    std::map<std::string, sc_time> empty_lat_map;
    this->init_sys_calls(exec_name, empty_lat_map, group);
  } // init_sys_calls()

  /// ..........................................................................

  // Called at every instruction issue. Goes over all the registered system
  // calls and checks if there is one that matches the current program counter.
  // If so, call the corresponding callback, else return false.
  bool issue(const IssueWidth& cur_PC, const InstructionBase* cur_instr) throw() {

    typename template_map<IssueWidth, Syscall<IssueWidth>* >::const_iterator syscall_found = this->syscalls.find(cur_PC);
    if (syscall_found != this->syscalls_end) {
      return (*(syscall_found->second))();
    }
    return false;
  } // issue()

  /// ..........................................................................

  bool is_pipeline_empty(const IssueWidth& cur_PC) const throw() {
    if (this->syscalls.find(cur_PC) != this->syscalls_end) {
      return true;
    }
    return false;
  } // is_pipeline_empty()

  /// @} Interface Methods
  /// --------------------------------------------------------------------------
  /// @name Internal Methods
  /// @{

  private:
  unsigned count_bits(IssueWidth bits) {
    unsigned num_bits = 0;
    /*for (unsigned i = 0; i < sizeof(IssueWidth) * 8; i++) {
      if ((bits & (0x1 << i)) != 0) {
        num_bits++;
      }
    }*/
    // NOTE: Search algorithm for counting ones taken from
    // Brian W. Kernighan and Dennis M. Ritchie, "The C Programming Language," 1988.
    for (; bits; num_bits++) {
      // Clears least significant set bit.
      bits &= bits - 1;
    }
    return num_bits;
  } // count_bits()

  /// @} Internal Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  private:
  ABIIf<IssueWidth>* processor;
  ELFFrontend* elf_frontend;
  template_map<IssueWidth, Syscall<IssueWidth>* > syscalls;
  typename template_map<IssueWidth, Syscall<IssueWidth>* >::const_iterator syscalls_end;

  /// @} Data
}; // class OSEmulator

} // namespace trap

/// ****************************************************************************
#endif

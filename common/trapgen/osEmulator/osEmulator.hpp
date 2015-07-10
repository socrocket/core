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
\***************************************************************************/

#ifndef OSEMULATOR_HPP
#define OSEMULATOR_HPP

#include <string>

#include "core/common/systemc.h"
#include "core/common/vmap.h"

#include "core/common/trapgen/ABIIf.hpp"
#include "core/common/trapgen/ToolsIf.hpp"

#ifndef EXTERNAL_BFD
#include "core/common/trapgen/elfloader/elfFrontend.hpp"
#else
#include "bfdWrapper.hpp"
#define BFDFrontend BFDWrapper
#endif

#include "core/common/trapgen/instructionBase.hpp"
#include "core/common/trapgen/osEmulator/syscCallB.hpp"

namespace trap {
template<class issueWidth>
class OSEmulator : public ToolsIf<issueWidth>, public OSEmulatorBase {
  private:
    typedef typename vmap<issueWidth, SyscallCB<issueWidth> *> syscallcb_map_t;
    typename syscallcb_map_t::const_iterator syscCallbacksEnd;
    syscallcb_map_t syscCallbacks;
    ABIIf<issueWidth> &processorInstance;
    ELFFrontend *elfFrontend;

    unsigned int countBits(issueWidth bits) {
      unsigned int numBits = 0;
      for (unsigned int i = 0; i < sizeof (issueWidth) * 8; i++) {
        if ((bits & (0x1 << i)) != 0) {
          numBits++;
        }
      }
      return numBits;
    }

    bool register_syscall(issueWidth addr, SyscallCB<issueWidth> &callBack) {
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

    bool register_syscall(std::string funName, SyscallCB<issueWidth> &callBack) {
      bool valid = false;
      unsigned int symAddr = this->elfFrontend->getSymAddr(funName, valid);
      if (!valid) {
        return false;
      }
      return register_syscall(symAddr, callBack);
    }

  public:
    OSEmulator(ABIIf<issueWidth> &processorInstance) : processorInstance(processorInstance) {
      this->syscCallbacksEnd = this->syscCallbacks.end();
    }
    std::set<std::string> getRegisteredFunctions() {
      std::set<std::string> registeredFunctions;
      typename vmap<issueWidth, SyscallCB<issueWidth> *>::iterator emuIter, emuEnd;
      for (emuIter = this->syscCallbacks.begin(), emuEnd = this->syscCallbacks.end(); emuIter != emuEnd; emuIter++) {
        registeredFunctions.insert(this->elfFrontend->symbolAt(emuIter->first));
      }
      return registeredFunctions;
    }
    void initSysCalls(std::string execName, int group = 0) {
      std::map<std::string, sc_time> emptyLatMap;
      this->initSysCalls(execName, emptyLatMap, group);
    }
    void initSysCalls(std::string execName, std::map<std::string, sc_time> latencies, int group = 0) {
      if (find(groupIDs.begin(), groupIDs.end(), group) == groupIDs.end()) {
        groupIDs.push_back(group);
        programsCount++;
      }

      // First of all I initialize the heap pointer according to the group it belongs to
      this->heapPointer = (unsigned int)this->processorInstance.getCodeLimit() + sizeof (issueWidth);

      this->elfFrontend = &ELFFrontend::getInstance(execName);
      // Now I perform the registration of the basic System Calls
      bool registered = false;

      openSysCall<issueWidth> *a = NULL;
      if (latencies.find("open") != latencies.end()) {
        a = new openSysCall<issueWidth>(this->processorInstance, *this, latencies["open"]);
      } else if (latencies.find("_open") != latencies.end()) {
        a = new openSysCall<issueWidth>(this->processorInstance, *this, latencies["_open"]);
      } else {
        a = new openSysCall<issueWidth>(this->processorInstance, *this);
      }
      registered = this->register_syscall("open", *a);
      registered |= this->register_syscall("_open", *a);
      if (!registered) {
        delete a;
      }
      creatSysCall<issueWidth> *b = NULL;
      if (latencies.find("creat") != latencies.end()) {
        b = new creatSysCall<issueWidth>(this->processorInstance, latencies["creat"]);
      } else if (latencies.find("_creat") != latencies.end()) {
        b = new creatSysCall<issueWidth>(this->processorInstance, latencies["_creat"]);
      } else {
        b = new creatSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("creat", *b);
      registered |= this->register_syscall("_creat", *b);
      if (!registered) {
        delete b;
      }
      closeSysCall<issueWidth> *c = NULL;
      if (latencies.find("close") != latencies.end()) {
        c = new closeSysCall<issueWidth>(this->processorInstance, latencies["close"]);
      } else if (latencies.find("_close") != latencies.end()) {
        c = new closeSysCall<issueWidth>(this->processorInstance, latencies["_close"]);
      } else {
        c = new closeSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("close", *c);
      registered |= this->register_syscall("_close", *c);
      if (!registered) {
        delete c;
      }
      readSysCall<issueWidth> *d = NULL;
      if (latencies.find("read") != latencies.end()) {
        d = new readSysCall<issueWidth>(this->processorInstance, latencies["read"]);
      } else if (latencies.find("_read") != latencies.end()) {
        d = new readSysCall<issueWidth>(this->processorInstance, latencies["_read"]);
      } else {
        d = new readSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("read", *d);
      registered |= this->register_syscall("_read", *d);
      if (!registered) {
        delete d;
      }
      writeSysCall<issueWidth> *e = NULL;
      if (latencies.find("write") != latencies.end()) {
        e = new writeSysCall<issueWidth>(this->processorInstance, latencies["write"]);
      } else if (latencies.find("_write") != latencies.end()) {
        e = new writeSysCall<issueWidth>(this->processorInstance, latencies["_write"]);
      } else {
        e = new writeSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("write", *e);
      registered |= this->register_syscall("_write", *e);
      if (!registered) {
        delete e;
      }
      isattySysCall<issueWidth> *f = NULL;
      if (latencies.find("isatty") != latencies.end()) {
        f = new isattySysCall<issueWidth>(this->processorInstance, latencies["isatty"]);
      } else if (latencies.find("_isatty") != latencies.end()) {
        f = new isattySysCall<issueWidth>(this->processorInstance, latencies["_isatty"]);
      } else {
        f = new isattySysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("isatty", *f);
      registered |= this->register_syscall("_isatty", *f);
      if (!registered) {
        delete f;
      }
      sbrkSysCall<issueWidth> *g = NULL;
      if (latencies.find("sbrk") != latencies.end()) {
        g = new sbrkSysCall<issueWidth>(this->processorInstance, this->heapPointer, latencies["sbrk"]);
      } else if (latencies.find("_sbrk") != latencies.end()) {
        g = new sbrkSysCall<issueWidth>(this->processorInstance, this->heapPointer, latencies["_sbrk"]);
      } else {
        g = new sbrkSysCall<issueWidth>(this->processorInstance, this->heapPointer);
      }
      registered = this->register_syscall("sbrk", *g);
      registered |= this->register_syscall("_sbrk", *g);
      if (!registered) {
        delete g;
      }
      lseekSysCall<issueWidth> *h = NULL;
      if (latencies.find("lseek") != latencies.end()) {
        h = new lseekSysCall<issueWidth>(this->processorInstance, latencies["lseek"]);
      } else if (latencies.find("_lseek") != latencies.end()) {
        h = new lseekSysCall<issueWidth>(this->processorInstance, latencies["_lseek"]);
      } else {
        h = new lseekSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("lseek", *h);
      registered |= this->register_syscall("_lseek", *h);
      if (!registered) {
        delete h;
      }
      fstatSysCall<issueWidth> *i = NULL;
      if (latencies.find("fstat") != latencies.end()) {
        i = new fstatSysCall<issueWidth>(this->processorInstance, latencies["fstat"]);
      } else if (latencies.find("_fstat") != latencies.end()) {
        i = new fstatSysCall<issueWidth>(this->processorInstance, latencies["_fstat"]);
      } else {
        i = new fstatSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("fstat", *i);
      registered |= this->register_syscall("_fstat", *i);
      if (!registered) {
        delete i;
      }
      _exitSysCall<issueWidth> *j = NULL;
      if (latencies.find("_exit") != latencies.end()) {
        j = new _exitSysCall<issueWidth>(this->processorInstance, latencies["_exit"]);
      } else {
        j = new _exitSysCall<issueWidth>(this->processorInstance);
      }
      if (!this->register_syscall("_exit", *j)) {
        delete j;
      }
      timesSysCall<issueWidth> *k = NULL;
      if (latencies.find("times") != latencies.end()) {
        k = new timesSysCall<issueWidth>(this->processorInstance, latencies["times"]);
      } else if (latencies.find("_times") != latencies.end()) {
        k = new timesSysCall<issueWidth>(this->processorInstance, latencies["_times"]);
      } else {
        k = new timesSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("times", *k);
      registered |= this->register_syscall("_times", *k);
      if (!registered) {
        delete k;
      }
      timeSysCall<issueWidth> *l = NULL;
      if (latencies.find("time") != latencies.end()) {
        l = new timeSysCall<issueWidth>(this->processorInstance, latencies["time"]);
      } else if (latencies.find("_time") != latencies.end()) {
        l = new timeSysCall<issueWidth>(this->processorInstance, latencies["_time"]);
      } else {
        l = new timeSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("time", *l);
      registered |= this->register_syscall("_time", *l);
      if (!registered) {
        delete l;
      }
      randomSysCall<issueWidth> *m = NULL;
      if (latencies.find("random") != latencies.end()) {
        m = new randomSysCall<issueWidth>(this->processorInstance, latencies["random"]);
      } else if (latencies.find("_random") != latencies.end()) {
        m = new randomSysCall<issueWidth>(this->processorInstance, latencies["_random"]);
      } else {
        m = new randomSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("random", *m);
      registered |= this->register_syscall("_random", *m);
      if (!registered) {
        delete m;
      }
      getpidSysCall<issueWidth> *n = NULL;
      if (latencies.find("getpid") != latencies.end()) {
        n = new getpidSysCall<issueWidth>(this->processorInstance, latencies["getpid"]);
      } else if (latencies.find("_getpid") != latencies.end()) {
        n = new getpidSysCall<issueWidth>(this->processorInstance, latencies["_getpid"]);
      } else {
        n = new getpidSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("getpid", *n);
      registered |= this->register_syscall("_getpid", *n);
      if (!registered) {
        delete n;
      }
      chmodSysCall<issueWidth> *o = NULL;
      if (latencies.find("chmod") != latencies.end()) {
        o = new chmodSysCall<issueWidth>(this->processorInstance, latencies["chmod"]);
      } else if (latencies.find("_chmod") != latencies.end()) {
        o = new chmodSysCall<issueWidth>(this->processorInstance, latencies["_chmod"]);
      } else {
        o = new chmodSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("chmod", *o);
      registered |= this->register_syscall("_chmod", *o);
      if (!registered) {
        delete o;
      }
      dupSysCall<issueWidth> *p = NULL;
      if (latencies.find("dup") != latencies.end()) {
        p = new dupSysCall<issueWidth>(this->processorInstance, latencies["dup"]);
      } else if (latencies.find("_dup") != latencies.end()) {
        p = new dupSysCall<issueWidth>(this->processorInstance, latencies["_dup"]);
      } else {
        p = new dupSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("dup", *p);
      registered |= this->register_syscall("_dup", *p);
      if (!registered) {
        delete p;
      }
      dup2SysCall<issueWidth> *q = NULL;
      if (latencies.find("dup2") != latencies.end()) {
        q = new dup2SysCall<issueWidth>(this->processorInstance, latencies["dup2"]);
      } else if (latencies.find("_dup2") != latencies.end()) {
        q = new dup2SysCall<issueWidth>(this->processorInstance, latencies["_dup2"]);
      } else {
        q = new dup2SysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("dup2", *q);
      registered |= this->register_syscall("_dup2", *q);
      if (!registered) {
        delete q;
      }
      getenvSysCall<issueWidth> *r = NULL;
      if (latencies.find("getenv") != latencies.end()) {
        r = new getenvSysCall<issueWidth>(this->processorInstance, this->heapPointer, this->env, latencies["getenv"]);
      } else if (latencies.find("_getenv") != latencies.end()) {
        r = new getenvSysCall<issueWidth>(this->processorInstance, this->heapPointer, this->env, latencies["_getenv"]);
      } else {
        r = new getenvSysCall<issueWidth>(this->processorInstance, this->heapPointer, this->env);
      }
      registered = this->register_syscall("getenv", *r);
      registered |= this->register_syscall("_getenv", *r);
      if (!registered) {
        delete r;
      }
      sysconfSysCall<issueWidth> *s = NULL;
      if (latencies.find("sysconf") != latencies.end()) {
        s = new sysconfSysCall<issueWidth>(this->processorInstance, this->sysconfmap, latencies["sysconf"]);
      } else {
        s = new sysconfSysCall<issueWidth>(this->processorInstance, this->sysconfmap);
      }
      if (!this->register_syscall("sysconf", *s)) {
        delete s;
      }
      gettimeofdaySysCall<issueWidth> *t = NULL;
      if (latencies.find("gettimeofday") != latencies.end()) {
        t = new gettimeofdaySysCall<issueWidth>(this->processorInstance, latencies["gettimeofday"]);
      } else if (latencies.find("_gettimeofday") != latencies.end()) {
        t = new gettimeofdaySysCall<issueWidth>(this->processorInstance, latencies["_gettimeofday"]);
      } else {
        t = new gettimeofdaySysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("gettimeofday", *t);
      registered |= this->register_syscall("_gettimeofday", *t);
      if (!registered) {
        delete t;
      }
      killSysCall<issueWidth> *u = NULL;
      if (latencies.find("kill") != latencies.end()) {
        u = new killSysCall<issueWidth>(this->processorInstance, latencies["kill"]);
      } else if (latencies.find("_kill") != latencies.end()) {
        u = new killSysCall<issueWidth>(this->processorInstance, latencies["_kill"]);
      } else {
        u = new killSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("kill", *u);
      registered |= this->register_syscall("_kill", *u);
      if (!registered) {
        delete u;
      }
      errorSysCall<issueWidth> *v = NULL;
      if (latencies.find("error") != latencies.end()) {
        v = new errorSysCall<issueWidth>(this->processorInstance, latencies["error"]);
      } else if (latencies.find("_error") != latencies.end()) {
        v = new errorSysCall<issueWidth>(this->processorInstance, latencies["_error"]);
      } else {
        v = new errorSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("error", *v);
      registered |= this->register_syscall("_error", *v);
      if (!registered) {
        delete v;
      }
      chownSysCall<issueWidth> *w = NULL;
      if (latencies.find("chown") != latencies.end()) {
        w = new chownSysCall<issueWidth>(this->processorInstance, latencies["chown"]);
      } else if (latencies.find("_chown") != latencies.end()) {
        w = new chownSysCall<issueWidth>(this->processorInstance, latencies["_chown"]);
      } else {
        w = new chownSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("chown", *w);
      registered |= this->register_syscall("_chown", *w);
      if (!registered) {
        delete w;
      }
      unlinkSysCall<issueWidth> *x = NULL;
      if (latencies.find("unlink") != latencies.end()) {
        x = new unlinkSysCall<issueWidth>(this->processorInstance, latencies["unlink"]);
      } else if (latencies.find("_unlink") != latencies.end()) {
        x = new unlinkSysCall<issueWidth>(this->processorInstance, latencies["_unlink"]);
      } else {
        x = new unlinkSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("unlink", *x);
      registered |= this->register_syscall("_unlink", *x);
      if (!registered) {
        delete x;
      }
      usleepSysCall<issueWidth> *y = NULL;
      if (latencies.find("usleep") != latencies.end()) {
        y = new usleepSysCall<issueWidth>(this->processorInstance, latencies["usleep"]);
      } else if (latencies.find("_usleep") != latencies.end()) {
        y = new usleepSysCall<issueWidth>(this->processorInstance, latencies["_usleep"]);
      } else {
        y = new usleepSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("usleep", *y);
      registered |= this->register_syscall("_usleep", *y);
      if (!registered) {
        delete y;
      }
      statSysCall<issueWidth> *z = NULL;
      if (latencies.find("stat") != latencies.end()) {
        z = new statSysCall<issueWidth>(this->processorInstance, latencies["stat"]);
      } else if (latencies.find("_stat") != latencies.end()) {
        z = new statSysCall<issueWidth>(this->processorInstance, latencies["_stat"]);
      } else {
        z = new statSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("stat", *z);
      registered |= this->register_syscall("_stat", *z);
      if (!registered) {
        delete z;
      }
      lstatSysCall<issueWidth> *A = NULL;
      if (latencies.find("lstat") != latencies.end()) {
        A = new lstatSysCall<issueWidth>(this->processorInstance, latencies["lstat"]);
      } else if (latencies.find("_lstat") != latencies.end()) {
        A = new lstatSysCall<issueWidth>(this->processorInstance, latencies["_lstat"]);
      } else {
        A = new lstatSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("lstat", *A);
      registered |= this->register_syscall("_lstat", *A);
      if (!registered) {
        delete A;
      }
      utimesSysCall<issueWidth> *B = NULL;
      if (latencies.find("utimes") != latencies.end()) {
        B = new utimesSysCall<issueWidth>(this->processorInstance, latencies["utimes"]);
      } else if (latencies.find("_utimes") != latencies.end()) {
        B = new utimesSysCall<issueWidth>(this->processorInstance, latencies["_utimes"]);
      } else {
        B = new utimesSysCall<issueWidth>(this->processorInstance);
      }
      registered = this->register_syscall("utimes", *B);
      registered |= this->register_syscall("_utimes", *B);
      if (!registered) {
        delete B;
      }

      mainSysCall<issueWidth> *mainCallBack = new mainSysCall<issueWidth>(this->processorInstance,
        this->heapPointer,
        this->programArgs);
      if (!this->register_syscall("main", *mainCallBack)) {
        THROW_EXCEPTION("Fatal Error, unable to find main function in current application");
      }
    }
    ///Method called at every instruction issue, it returns true in case the instruction
    ///has to be skipped, false otherwise
    bool newIssue(const issueWidth &curPC, const InstructionBase *curInstr) throw() {
      // I have to go over all the registered system calls and check if there is one
      // that matches the current program counter. In case I simply call the corresponding
      // callback.
      typename vmap<issueWidth, SyscallCB<issueWidth> *>::const_iterator foundSysc = this->syscCallbacks.find(
        curPC);
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
    ~OSEmulator() {
      reset();
    }
};
}

#endif

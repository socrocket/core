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

#ifndef SYSCCALLB_H
#define SYSCCALLB_H

#ifdef _WIN32
#pragma warning( disable : 4244 )
#endif

#include "core/common/trapgen/utils/trap_utils.hpp"

#include "core/common/trapgen/ABIIf.hpp"
#include "core/common/systemc.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include "core/common/systemc.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <utime.h>
#ifdef __GNUC__
#include <unistd.h>
#else
#include <io.h>
#endif
#ifdef __GNUC__
#if !(defined(__MACOSX__) || defined(__DARWIN__) || defined(__APPLE__) || defined(__CYGWIN__))
#include <error.h>
#endif
#endif
#include <cerrno>
#if !defined(errno) && !defined(HAVE_ERRNO_DECL)
extern int errno;
#endif
#include <sstream>
#ifdef __GNUC__
#include <sys/times.h>
#endif
#include <ctime>

#include "core/common/trapgen/elfloader/elfFrontend.hpp"

namespace trap {
class OSEmulatorBase {
  public:

    virtual std::set<std::string> getRegisteredFunctions() = 0;
    void set_program_args(const std::vector<std::string> args);
    void correct_flags(int &val);
    void set_environ(const std::string name, const std::string value);
    void set_sysconf(const std::string name, int value);
    void reset();

    std::map<std::string, std::string> env;
    std::map<std::string, int> sysconfmap;
    std::vector<std::string> programArgs;
    unsigned int heapPointer;

    static std::vector<unsigned int> groupIDs;
    static unsigned int programsCount;
};

///Base class for each emulated system call;
///Operator () implements the behaviour of the
///emulated call
template<class wordSize>
class SyscallCB {
  protected:
    ABIIf<wordSize> &processorInstance;
    sc_time latency;
  public:
    SyscallCB(ABIIf<wordSize> &processorInstance, sc_time latency) : processorInstance(processorInstance), latency(
                                                                       latency) {}
    virtual ~SyscallCB() {}
    virtual bool operator()() = 0;
};

template<class wordSize>
class openSysCall : public SyscallCB<wordSize> {
  private:
    OSEmulatorBase &osEmu;
  public:
    openSysCall(ABIIf<wordSize> &processorInstance, OSEmulatorBase &osEmu, sc_time latency =
      SC_ZERO_TIME) : SyscallCB<wordSize>(processorInstance, latency), osEmu(osEmu) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();
      // Lets read the name of the file to be opened
      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->processorInstance.readCharMem(callArgs[0] + i);
        if (pathname[i] == '\x0') {
          break;
        }
      }
      int flags = callArgs[1];
      osEmu.correct_flags(flags);
      int mode = callArgs[2];
#ifdef __GNUC__
      int ret = ::open(pathname, flags, mode);
#else
      int ret = ::_open(pathname, flags, mode);
#endif
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class creatSysCall : public SyscallCB<wordSize> {
  public:
    creatSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                         processorInstance,
                                                                                         latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();
      // Lets read the name of the file to be opened
      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->processorInstance.readCharMem(callArgs[0] + i);
        if (pathname[i] == '\x0') {
          break;
        }
      }
      int mode = callArgs[1];
#ifdef __GNUC__
      int ret = ::creat((char *)pathname, mode);
#else
      int ret = ::_creat((char *)pathname, mode);
#endif
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class closeSysCall : public SyscallCB<wordSize> {
  public:
    closeSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                         processorInstance,
                                                                                         latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();
      int fd = callArgs[0];
      if (fd < 0) {
        THROW_EXCEPTION("File descriptor " << fd << " not valid");
      }
#ifdef __GNUC__
      if ((fd == fileno(stdin)) || (fd == fileno(stdout)) || (fd == fileno(stderr))) {
#else
      if ((fd == _fileno(stdin)) || (fd == _fileno(stdout)) || (fd == _fileno(stderr))) {
#endif
        this->processorInstance.setRetVal(0);
        this->processorInstance.returnFromCall();
      } else {
#ifdef __GNUC__
        int ret = ::close(fd);
#else
        int ret = ::_close(fd);
#endif
        this->processorInstance.setRetVal(ret);
        this->processorInstance.returnFromCall();
      }
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class readSysCall : public SyscallCB<wordSize> {
  public:
    readSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                        processorInstance,
                                                                                        latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();
      int fd = callArgs[0];
      if (fd < 0) {
        THROW_EXCEPTION("File descriptor " << fd << " not valid");
      }
      unsigned count = callArgs[2];
      unsigned char *buf = new unsigned char[count];
#ifdef __GNUC__
      int ret = ::read(fd, buf, count);
#else
      int ret = ::_read(fd, buf, count);
#endif
      // Now I have to write the read content into memory
      wordSize destAddress = callArgs[1];
      for (int i = 0; i < ret; i++) {
        this->processorInstance.writeCharMem(destAddress + i, buf[i]);
      }
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      delete[] buf;
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class writeSysCall : public SyscallCB<wordSize> {
  public:
    writeSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                         processorInstance,
                                                                                         latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();
      int fd = callArgs[0];
      if (fd < 0) {
        THROW_EXCEPTION("File descriptor " << fd << " not valid");
      }
      unsigned count = callArgs[2];
      wordSize destAddress = callArgs[1];
      unsigned char *buf = new unsigned char[count];
      for (unsigned int i = 0; i < count; i++) {
        buf[i] = this->processorInstance.readCharMem(destAddress + i);
      }
#ifdef __GNUC__
      int ret = ::write(fd, buf, count);
#else
      int ret = ::_write(fd, buf, count);
#endif
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      delete[] buf;
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class isattySysCall : public SyscallCB<wordSize> {
  public:
    isattySysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                          processorInstance,
                                                                                          latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();
      int desc = callArgs[0];
#ifdef __GNUC__
      int ret = ::isatty(desc);
#else
      int ret = ::_isatty(desc);
#endif
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class sbrkSysCall : public SyscallCB<wordSize> {
  private:
    unsigned int &heapPointer;
  public:
    sbrkSysCall(ABIIf<wordSize> &processorInstance, unsigned int &heapPointer, sc_time latency = SC_ZERO_TIME) :
      SyscallCB<wordSize>(processorInstance, latency), heapPointer(heapPointer) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();

      wordSize base = this->heapPointer;
      long long increment = callArgs[0];

      this->heapPointer += increment;

      // I try to read from meory to see if it is possible to access the just allocated address;
      // In case it is not it means that I'm out of memory and I signal the error
      try {
        this->processorInstance.readMem(this->heapPointer);
        this->processorInstance.setRetVal(base);
      } catch (...) {
        this->processorInstance.setRetVal(-1);
        std::cerr << "SBRK: tried to allocate " << increment << " bytes of memory starting at address " << std::hex <<
        std::showbase << base << std::dec << " but it seems there is not enough memory" << std::endl;
      }

      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class lseekSysCall : public SyscallCB<wordSize> {
  public:
    lseekSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                         processorInstance,
                                                                                         latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();
      int fd = callArgs[0];
      if (fd < 0) {
        THROW_EXCEPTION("File descriptor " << fd << " not valid");
      }
      int offset = callArgs[1];
      int whence = callArgs[2];
#ifdef __GNUC__
      int ret = ::lseek(fd, offset, whence);
#else
      int ret = ::_lseek(fd, offset, whence);
#endif
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class fstatSysCall : public SyscallCB<wordSize> {
  public:
    fstatSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                         processorInstance,
                                                                                         latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();
#ifdef __GNUC__
      struct stat buf_stat;
#else
      struct _stat buf_stat;
#endif
      int fd = callArgs[0];
      if (fd < 0) {
        THROW_EXCEPTION("File descriptor " << fd << " not valid");
      }
      int retAddr = callArgs[1];
#ifdef __GNUC__
      int ret = ::fstat(fd, &buf_stat);
#else
      int ret = ::_fstat(fd, &buf_stat);
#endif
      if ((ret >= 0) && (retAddr != 0)) {
        this->processorInstance.writeMem(retAddr, buf_stat.st_dev);
        this->processorInstance.writeMem(retAddr + 2, buf_stat.st_ino);
        this->processorInstance.writeMem(retAddr + 4, buf_stat.st_mode);
        this->processorInstance.writeMem(retAddr + 8, buf_stat.st_nlink);
        this->processorInstance.writeMem(retAddr + 10, buf_stat.st_uid);
        this->processorInstance.writeMem(retAddr + 12, buf_stat.st_gid);
        this->processorInstance.writeMem(retAddr + 14, buf_stat.st_rdev);
        this->processorInstance.writeMem(retAddr + 16, buf_stat.st_size);
        this->processorInstance.writeMem(retAddr + 20, buf_stat.st_atime);
        this->processorInstance.writeMem(retAddr + 28, buf_stat.st_mtime);
        this->processorInstance.writeMem(retAddr + 36, buf_stat.st_ctime);
#ifdef __GNUC__
        this->processorInstance.writeMem(retAddr + 44, buf_stat.st_blksize);
        this->processorInstance.writeMem(retAddr + 48, buf_stat.st_blocks);
#endif
      }
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class statSysCall : public SyscallCB<wordSize> {
  public:
    statSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                        processorInstance,
                                                                                        latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();

#ifdef __GNUC__
      struct stat buf_stat;
#else
      struct _stat buf_stat;
#endif

      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->processorInstance.readCharMem(callArgs[0] + i);
        if (pathname[i] == '\x0') {
          break;
        }
      }
      int retAddr = callArgs[1];
#ifdef __GNUC__
      int ret = ::stat((char *)pathname, &buf_stat);
#else
      int ret = ::_stat((char *)pathname, &buf_stat);
#endif
      if ((ret >= 0) && (retAddr != 0)) {
        this->processorInstance.writeMem(retAddr, buf_stat.st_dev);
        this->processorInstance.writeMem(retAddr + 2, buf_stat.st_ino);
        this->processorInstance.writeMem(retAddr + 4, buf_stat.st_mode);
        this->processorInstance.writeMem(retAddr + 8, buf_stat.st_nlink);
        this->processorInstance.writeMem(retAddr + 10, buf_stat.st_uid);
        this->processorInstance.writeMem(retAddr + 12, buf_stat.st_gid);
        this->processorInstance.writeMem(retAddr + 14, buf_stat.st_rdev);
        this->processorInstance.writeMem(retAddr + 16, buf_stat.st_size);
        this->processorInstance.writeMem(retAddr + 20, buf_stat.st_atime);
        this->processorInstance.writeMem(retAddr + 28, buf_stat.st_mtime);
        this->processorInstance.writeMem(retAddr + 36, buf_stat.st_ctime);
#ifdef __GNUC__
        this->processorInstance.writeMem(retAddr + 44, buf_stat.st_blksize);
        this->processorInstance.writeMem(retAddr + 48, buf_stat.st_blocks);
#endif
      }
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class _exitSysCall : public SyscallCB<wordSize> {
  public:
    _exitSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                         processorInstance,
                                                                                         latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      extern int exitValue;
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();
      exitValue = (int)callArgs[0];
      std::cout << std::endl << "Program exited with value " << exitValue << std::endl << std::endl;

      if (sc_is_running()) {
        OSEmulatorBase::programsCount--;
        if (OSEmulatorBase::programsCount > 0) { // in case there are other programs still running, block the current processor
          sc_event endEv;
          wait(endEv);
        } else {  // ok, this is the last running program, it is possible to call sc_stop()
          sc_stop();
        }
        wait(SC_ZERO_TIME);
      }

      return true;
    }
};

template<class wordSize>
class timesSysCall : public SyscallCB<wordSize> {
  public:
    timesSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                         processorInstance,
                                                                                         latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();

      unsigned int curSimTime = (unsigned int)(sc_time_stamp().to_double() / 1.0e+6);
      wordSize timesRetLoc = callArgs[0];
      if (timesRetLoc != 0) {
#ifndef __GNUC__
        struct tms {
          clock_t tms_utime;        /* user time */
          clock_t tms_stime;        /* system time */
          clock_t tms_cutime;       /* user time of children */
          clock_t tms_cstime;       /* system time of children */
        };
#endif
        struct tms buf;
        buf.tms_utime = curSimTime;
        buf.tms_stime = curSimTime;
        buf.tms_cutime = curSimTime;
        buf.tms_cstime = curSimTime;
        this->processorInstance.writeMem(timesRetLoc, buf.tms_utime);
        timesRetLoc += 4;
        this->processorInstance.writeMem(timesRetLoc, buf.tms_stime);
        timesRetLoc += 4;
        this->processorInstance.writeMem(timesRetLoc, buf.tms_cutime);
        timesRetLoc += 4;
        this->processorInstance.writeMem(timesRetLoc, buf.tms_cstime);
      }
      this->processorInstance.setRetVal(curSimTime);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class timeSysCall : public SyscallCB<wordSize> {
  private:
    int initialTime;
  public:
    timeSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                        processorInstance,
                                                                                        latency) {
      this->initialTime = time(0);
    }
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();

      int t = callArgs[0];
      int ret = this->initialTime + (int)(sc_time_stamp().to_double() / 1.0e+12);
      if (t != 0) {
        this->processorInstance.writeMem(t, ret);
      }
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class randomSysCall : public SyscallCB<wordSize> {
  public:
    randomSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                          processorInstance,
                                                                                          latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      int ret = ::rand();
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class utimesSysCall : public SyscallCB<wordSize> {
  public:
    utimesSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                          processorInstance,
                                                                                          latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();

      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->processorInstance.readCharMem(callArgs[0] + i);
        if (pathname[i] == '\x0') {
          break;
        }
      }

      int ret = -1;
      int timesAddr = callArgs[1];
      if (timesAddr == 0) {
        ret = ::utimes((char *)pathname, NULL);
      } else {
        struct timeval times[2];
        times[0].tv_sec = this->processorInstance.readMem(timesAddr);
        times[0].tv_usec = this->processorInstance.readMem(timesAddr + 4);
        times[1].tv_sec = this->processorInstance.readMem(timesAddr + 8);
        times[1].tv_usec = this->processorInstance.readMem(timesAddr + 12);
        ret = ::utimes((char *)pathname, times);
      }

      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class lstatSysCall : public SyscallCB<wordSize> {
  public:
    lstatSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                         processorInstance,
                                                                                         latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();

#ifdef __GNUC__
      struct stat buf_stat;
#else
      struct _stat buf_stat;
#endif

      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->processorInstance.readCharMem(callArgs[0] + i);
        if (pathname[i] == '\x0') {
          break;
        }
      }
      int retAddr = callArgs[1];
#ifdef __GNUC__
      int ret = ::lstat((char *)pathname, &buf_stat);
#else
      int ret = ::_lstat((char *)pathname, &buf_stat);
#endif
      if ((ret >= 0) && (retAddr != 0)) {
        this->processorInstance.writeMem(retAddr, buf_stat.st_dev);
        this->processorInstance.writeMem(retAddr + 2, buf_stat.st_ino);
        this->processorInstance.writeMem(retAddr + 4, buf_stat.st_mode);
        this->processorInstance.writeMem(retAddr + 8, buf_stat.st_nlink);
        this->processorInstance.writeMem(retAddr + 10, buf_stat.st_uid);
        this->processorInstance.writeMem(retAddr + 12, buf_stat.st_gid);
        this->processorInstance.writeMem(retAddr + 14, buf_stat.st_rdev);
        this->processorInstance.writeMem(retAddr + 16, buf_stat.st_size);
        this->processorInstance.writeMem(retAddr + 20, buf_stat.st_atime);
        this->processorInstance.writeMem(retAddr + 28, buf_stat.st_mtime);
        this->processorInstance.writeMem(retAddr + 36, buf_stat.st_ctime);
#ifdef __GNUC__
        this->processorInstance.writeMem(retAddr + 44, buf_stat.st_blksize);
        this->processorInstance.writeMem(retAddr + 48, buf_stat.st_blocks);
#endif
      }
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class getpidSysCall : public SyscallCB<wordSize> {
  public:
    getpidSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                          processorInstance,
                                                                                          latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      this->processorInstance.setRetVal(123);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class chmodSysCall : public SyscallCB<wordSize> {
  public:
    chmodSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                         processorInstance,
                                                                                         latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();

      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->processorInstance.readCharMem(callArgs[0] + i);
        if (pathname[i] == '\x0') {
          break;
        }
      }
      int mode = callArgs[1];
#ifdef __GNUC__
      int ret = ::chmod((char *)pathname, mode);
#else
      int ret = ::_chmod((char *)pathname, mode);
#endif
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class dupSysCall : public SyscallCB<wordSize> {
  public:
    dupSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                       processorInstance,
                                                                                       latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();
      int fd = callArgs[0];
      if (fd < 0) {
        THROW_EXCEPTION("File descriptor not valid");
      }
#ifdef __GNUC__
      int ret = ::dup(fd);
#else
      int ret = ::_dup(fd);
#endif
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class dup2SysCall : public SyscallCB<wordSize> {
  public:
    dup2SysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                        processorInstance,
                                                                                        latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();
      int fd = callArgs[0];
      if (fd < 0) {
        THROW_EXCEPTION("File descriptor not valid");
      }
      int newfd = callArgs[1];
#ifdef __GNUC__
      int ret = ::dup2(fd, newfd);
#else
      int ret = ::_dup2(fd, newfd);
#endif
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class getenvSysCall : public SyscallCB<wordSize> {
  private:
    unsigned int &heapPointer;
    std::map<std::string, std::string> &env;
  public:
    getenvSysCall(ABIIf<wordSize> &processorInstance,
    unsigned int &heapPointer,
    std::map<std::string, std::string> &env,
    sc_time latency = SC_ZERO_TIME) :
      SyscallCB<wordSize>(processorInstance, latency), heapPointer(heapPointer), env(env) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();

      char envname[256];
      int envNameAddr = callArgs[0];
      if (envNameAddr != 0) {
        for (int i = 0; i < 256; i++) {
          envname[i] = (char)this->processorInstance.readCharMem(envNameAddr + i);
          if (envname[i] == '\x0') {
            break;
          }
        }
        std::map<std::string, std::string>::iterator curEnv = this->env.find((std::string(envname)));
        if (curEnv == this->env.end()) {
          this->processorInstance.setRetVal(0);
          this->processorInstance.returnFromCall();
        } else {
          // I have to allocate memory for the result on the simulated memory;
          // I then have to copy the read environment variable here and return
          // the pointer to it
          unsigned int base = this->heapPointer;
          this->heapPointer += curEnv->second.size() + 1;
          for (unsigned int i = 0; i < curEnv->second.size(); i++) {
            this->processorInstance.writeCharMem(base + i, curEnv->second[i]);
          }
          this->processorInstance.writeCharMem(base + curEnv->second.size(), 0);
          this->processorInstance.setRetVal(base);
          this->processorInstance.returnFromCall();
        }
      } else {
        this->processorInstance.setRetVal(0);
        this->processorInstance.returnFromCall();
      }
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class gettimeofdaySysCall : public SyscallCB<wordSize> {
  public:
    gettimeofdaySysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                                processorInstance,
                                                                                                latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();

      int timesRetLoc = callArgs[0];
      if (timesRetLoc != 0) {
        double curSimTime = sc_time_stamp().to_double();
        unsigned int tv_sec = (unsigned int)(curSimTime / 1.0e+12);
        unsigned int tv_usec = (unsigned int)((curSimTime - tv_sec * 1.0e+12) / 1.0e+6);
        this->processorInstance.writeMem(timesRetLoc, tv_sec);
        timesRetLoc += 4;
        this->processorInstance.writeMem(timesRetLoc, tv_usec);
      }
      this->processorInstance.setRetVal(0);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class killSysCall : public SyscallCB<wordSize> {
  public:
    killSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                        processorInstance,
                                                                                        latency) {}
    bool operator()() {
      THROW_EXCEPTION("KILL SystemCall not yet implemented");

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class errorSysCall : public SyscallCB<wordSize> {
  public:
    errorSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                         processorInstance,
                                                                                         latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();

      int status = callArgs[0];
      int errnum = callArgs[1];
      char *errorString = ::strerror(errnum);
      if (status != 0) {
        std::cerr << std::endl << "Program exited with value " << status << std::endl << " Error message: " <<
        errorString << std::endl;
        if (sc_is_running()) {
          sc_stop();
        }
      } else {
        std::cerr << "An error occurred in the execution of the program: message = " << errorString << std::endl;
        this->processorInstance.setRetVal(0);
        this->processorInstance.returnFromCall();
      }
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class chownSysCall : public SyscallCB<wordSize> {
  public:
    chownSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                         processorInstance,
                                                                                         latency) {}
    bool operator()() {
      this->processorInstance.preCall();
#ifdef __GNUC__
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();

      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->processorInstance.readCharMem(callArgs[0] + i);
        if (pathname[i] == '\x0') {
          break;
        }
      }
      uid_t owner = callArgs[1];
      gid_t group = callArgs[2];
      int ret = ::chown((char *)pathname, owner, group);
#else // ifdef __GNUC__
      int ret = 0;
#endif
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class unlinkSysCall : public SyscallCB<wordSize> {
  public:
    unlinkSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                          processorInstance,
                                                                                          latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();

      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->processorInstance.readCharMem(callArgs[0] + i);
        if (pathname[i] == '\x0') {
          break;
        }
      }
#ifdef __GNUC__
      int ret = ::unlink((char *)pathname);
#else
      int ret = ::_unlink((char *)pathname);
#endif
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class usleepSysCall : public SyscallCB<wordSize> {
  public:
    usleepSysCall(ABIIf<wordSize> &processorInstance, sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(
                                                                                          processorInstance,
                                                                                          latency) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Since we have a single process this function doesn't do anything :-)
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class mainSysCall : public SyscallCB<wordSize> {
  private:
    unsigned int &heapPointer;
    std::vector<std::string> &programArgs;
  public:
    mainSysCall(ABIIf<wordSize> &processorInstance, unsigned int &heapPointer, std::vector<std::string> &programArgs) :
      SyscallCB<wordSize>(processorInstance, SC_ZERO_TIME), heapPointer(heapPointer), programArgs(programArgs) {}
    bool operator()() {
      this->processorInstance.preCall();

      std::vector<wordSize> callArgs = this->processorInstance.readArgs();
      if (callArgs[0] != 0) {
        this->processorInstance.postCall();
        return false;
      }

      std::vector<wordSize> mainArgs;

      if (this->programArgs.size() == 0) {
        mainArgs.push_back(0);
        mainArgs.push_back(0);
        this->processorInstance.setArgs(mainArgs);
        this->processorInstance.postCall();
        return false;
      }

      unsigned int argAddr = ((unsigned int)this->heapPointer) + (this->programArgs.size() + 1) * 4;
      unsigned int argNumAddr = this->heapPointer;
      std::vector<std::string>::iterator argsIter, argsEnd;
      for (argsIter = this->programArgs.begin(), argsEnd = this->programArgs.end(); argsIter != argsEnd; argsIter++) {
        this->processorInstance.writeMem(argNumAddr, argAddr);
        argNumAddr += 4;
        for (unsigned int i = 0; i < argsIter->size(); i++) {
          this->processorInstance.writeCharMem(argAddr + i, argsIter->c_str()[i]);
        }
        this->processorInstance.writeCharMem(argAddr + argsIter->size(), 0);
        argAddr += argsIter->size() + 1;
      }
      this->processorInstance.writeMem(argNumAddr, 0);

      mainArgs.push_back(this->programArgs.size());
      mainArgs.push_back(this->heapPointer);
      this->processorInstance.setArgs(mainArgs);
      this->heapPointer = argAddr;
      this->processorInstance.postCall();
      return false;
    }
};

/*
 *  sysconf values per IEEE Std 1003.1, 2004 Edition
 */
#define NEWLIB_SC_ARG_MAX                       0
#define NEWLIB_SC_CHILD_MAX                     1
#define NEWLIB_SC_CLK_TCK                       2
#define NEWLIB_SC_NGROUPS_MAX                   3
#define NEWLIB_SC_OPEN_MAX                      4
#define NEWLIB_SC_JOB_CONTROL                   5
#define NEWLIB_SC_SAVED_IDS                     6
#define NEWLIB_SC_VERSION                       7
#define NEWLIB_SC_PAGESIZE                      8
#define NEWLIB_SC_PAGE_SIZE                     NEWLIB_SC_PAGESIZE
/* These are non-POSIX values we accidentally introduced in 2000 without
   guarding them.  Keeping them unguarded for backward compatibility. */
#define NEWLIB_SC_NPROCESSORS_CONF              9
#define NEWLIB_SC_NPROCESSORS_ONLN             10
#define NEWLIB_SC_PHYS_PAGES                   11
#define NEWLIB_SC_AVPHYS_PAGES                 12
/* End of non-POSIX values. */
#define NEWLIB_SC_MQ_OPEN_MAX                  13
#define NEWLIB_SC_MQ_PRIO_MAX                  14
#define NEWLIB_SC_RTSIG_MAX                    15
#define NEWLIB_SC_SEM_NSEMS_MAX                16
#define NEWLIB_SC_SEM_VALUE_MAX                17
#define NEWLIB_SC_SIGQUEUE_MAX                 18
#define NEWLIB_SC_TIMER_MAX                    19
#define NEWLIB_SC_TZNAME_MAX                   20
#define NEWLIB_SC_ASYNCHRONOUS_IO              21
#define NEWLIB_SC_FSYNC                        22
#define NEWLIB_SC_MAPPED_FILES                 23
#define NEWLIB_SC_MEMLOCK                      24
#define NEWLIB_SC_MEMLOCK_RANGE                25
#define NEWLIB_SC_MEMORY_PROTECTION            26
#define NEWLIB_SC_MESSAGE_PASSING              27
#define NEWLIB_SC_PRIORITIZED_IO               28
#define NEWLIB_SC_REALTIME_SIGNALS             29
#define NEWLIB_SC_SEMAPHORES                   30
#define NEWLIB_SC_SHARED_MEMORY_OBJECTS        31
#define NEWLIB_SC_SYNCHRONIZED_IO              32
#define NEWLIB_SC_TIMERS                       33
#define NEWLIB_SC_AIO_LISTIO_MAX               34
#define NEWLIB_SC_AIO_MAX                      35
#define NEWLIB_SC_AIO_PRIO_DELTA_MAX           36
#define NEWLIB_SC_DELAYTIMER_MAX               37
#define NEWLIB_SC_THREAD_KEYS_MAX              38
#define NEWLIB_SC_THREAD_STACK_MIN             39
#define NEWLIB_SC_THREAD_THREADS_MAX           40
#define NEWLIB_SC_TTY_NAME_MAX                 41
#define NEWLIB_SC_THREADS                      42
#define NEWLIB_SC_THREAD_ATTR_STACKADDR        43
#define NEWLIB_SC_THREAD_ATTR_STACKSIZE        44
#define NEWLIB_SC_THREAD_PRIORITY_SCHEDULING   45
#define NEWLIB_SC_THREAD_PRIO_INHERIT          46
/* NEWLIB_SC_THREAD_PRIO_PROTECT was NEWLIB_SC_THREAD_PRIO_CEILING in early drafts */
#define NEWLIB_SC_THREAD_PRIO_PROTECT          47
#define NEWLIB_SC_THREAD_PRIO_CEILING          NEWLIB_SC_THREAD_PRIO_PROTECT
#define NEWLIB_SC_THREAD_PROCESS_SHARED        48
#define NEWLIB_SC_THREAD_SAFE_FUNCTIONS        49
#define NEWLIB_SC_GETGR_R_SIZE_MAX             50
#define NEWLIB_SC_GETPW_R_SIZE_MAX             51
#define NEWLIB_SC_LOGIN_NAME_MAX               52
#define NEWLIB_SC_THREAD_DESTRUCTOR_ITERATIONS 53
#define NEWLIB_SC_ADVISORY_INFO                54
#define NEWLIB_SC_ATEXIT_MAX                   55
#define NEWLIB_SC_BARRIERS                     56
#define NEWLIB_SC_BC_BASE_MAX                  57
#define NEWLIB_SC_BC_DIM_MAX                   58
#define NEWLIB_SC_BC_SCALE_MAX                 59
#define NEWLIB_SC_BC_STRING_MAX                60
#define NEWLIB_SC_CLOCK_SELECTION              61
#define NEWLIB_SC_COLL_WEIGHTS_MAX             62
#define NEWLIB_SC_CPUTIME                      63
#define NEWLIB_SC_EXPR_NEST_MAX                64
#define NEWLIB_SC_HOST_NAME_MAX                65
#define NEWLIB_SC_IOV_MAX                      66
#define NEWLIB_SC_IPV6                         67
#define NEWLIB_SC_LINE_MAX                     68
#define NEWLIB_SC_MONOTONIC_CLOCK              69
#define NEWLIB_SC_RAW_SOCKETS                  70
#define NEWLIB_SC_READER_WRITER_LOCKS          71
#define NEWLIB_SC_REGEXP                       72
#define NEWLIB_SC_RE_DUP_MAX                   73
#define NEWLIB_SC_SHELL                        74
#define NEWLIB_SC_SPAWN                        75
#define NEWLIB_SC_SPIN_LOCKS                   76
#define NEWLIB_SC_SPORADIC_SERVER              77
#define NEWLIB_SC_SS_REPL_MAX                  78
#define NEWLIB_SC_SYMLOOP_MAX                  79
#define NEWLIB_SC_THREAD_CPUTIME               80
#define NEWLIB_SC_THREAD_SPORADIC_SERVER       81
#define NEWLIB_SC_TIMEOUTS                     82
#define NEWLIB_SC_TRACE                        83
#define NEWLIB_SC_TRACE_EVENT_FILTER           84
#define NEWLIB_SC_TRACE_EVENT_NAME_MAX         85
#define NEWLIB_SC_TRACE_INHERIT                86
#define NEWLIB_SC_TRACE_LOG                    87
#define NEWLIB_SC_TRACE_NAME_MAX               88
#define NEWLIB_SC_TRACE_SYS_MAX                89
#define NEWLIB_SC_TRACE_USER_EVENT_MAX         90
#define NEWLIB_SC_TYPED_MEMORY_OBJECTS         91
#define NEWLIB_SC_V6_ILP32_OFF32               92
#define NEWLIB_SC_XBS5_ILP32_OFF32             NEWLIB_SC_V6_ILP32_OFF32
#define NEWLIB_SC_V6_ILP32_OFFBIG              93
#define NEWLIB_SC_XBS5_ILP32_OFFBIG            NEWLIB_SC_V6_ILP32_OFFBIG
#define NEWLIB_SC_V6_LP64_OFF64                94
#define NEWLIB_SC_XBS5_LP64_OFF64              NEWLIB_SC_V6_LP64_OFF64
#define NEWLIB_SC_V6_LPBIG_OFFBIG              95
#define NEWLIB_SC_XBS5_LPBIG_OFFBIG            NEWLIB_SC_V6_LPBIG_OFFBIG
#define NEWLIB_SC_XOPEN_CRYPT                  96
#define NEWLIB_SC_XOPEN_ENH_I18N               97
#define NEWLIB_SC_XOPEN_LEGACY                 98
#define NEWLIB_SC_XOPEN_REALTIME               99
#define NEWLIB_SC_STREAM_MAX                  100
#define NEWLIB_SC_PRIORITY_SCHEDULING         101
#define NEWLIB_SC_XOPEN_REALTIME_THREADS      102
#define NEWLIB_SC_XOPEN_SHM                   103
#define NEWLIB_SC_XOPEN_STREAMS               104
#define NEWLIB_SC_XOPEN_UNIX                  105
#define NEWLIB_SC_XOPEN_VERSION               106
#define NEWLIB_SC_2_CHAR_TERM                 107
#define NEWLIB_SC_2_C_BIND                    108
#define NEWLIB_SC_2_C_DEV                     109
#define NEWLIB_SC_2_FORT_DEV                  110
#define NEWLIB_SC_2_FORT_RUN                  111
#define NEWLIB_SC_2_LOCALEDEF                 112
#define NEWLIB_SC_2_PBS                       113
#define NEWLIB_SC_2_PBS_ACCOUNTING            114
#define NEWLIB_SC_2_PBS_CHECKPOINT            115
#define NEWLIB_SC_2_PBS_LOCATE                116
#define NEWLIB_SC_2_PBS_MESSAGE               117
#define NEWLIB_SC_2_PBS_TRACK                 118
#define NEWLIB_SC_2_SW_DEV                    119
#define NEWLIB_SC_2_UPE                       120
#define NEWLIB_SC_2_VERSION                   121

template<class wordSize>
class sysconfSysCall : public SyscallCB<wordSize> {
  private:
    std::map<std::string, int> &sysconfmap;
  public:
    sysconfSysCall(ABIIf<wordSize> &processorInstance,
    std::map<std::string, int> &sysconfmap,
    sc_time latency = SC_ZERO_TIME) : SyscallCB<wordSize>(processorInstance, latency), sysconfmap(sysconfmap) {}
    bool operator()() {
      this->processorInstance.preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->processorInstance.readArgs();

      int argId = callArgs[0];
      int ret = -1;
      switch (argId) {
      case NEWLIB_SC_NPROCESSORS_ONLN:
        if (this->sysconfmap.find("_SC_NPROCESSORS_ONLN") == this->sysconfmap.end()) {
          ret = 1;
        } else {
          ret = this->sysconfmap["_SC_NPROCESSORS_ONLN"];
        }
        break;
      case NEWLIB_SC_CLK_TCK:
        if (this->sysconfmap.find("_SC_CLK_TCK") == this->sysconfmap.end()) {
          ret = 1000000;
        } else {
          ret = this->sysconfmap["_SC_CLK_TCK"];
        }
        break;
      default:
        ret = -1;
        break;
      }
      this->processorInstance.setRetVal(ret);
      this->processorInstance.returnFromCall();
      this->processorInstance.postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};
}

#endif

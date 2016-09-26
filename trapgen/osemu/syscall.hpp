/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     syscall.hpp
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

#ifndef TRAP_SYSCALL_H
#define TRAP_SYSCALL_H

#ifdef _WIN32
#pragma warning( disable : 4244 )
#endif

#include "osemu_base.hpp"
#include "elfloader/elf_frontend.hpp"
#include "modules/abi_if.hpp"
#include "common/report.hpp"

#include <systemc.h>

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <systemc.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <sys/time.h>
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

namespace trap {

/// sysconf values per IEEE Std 1003.1, 2004 Edition
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
/// These are non-POSIX values we accidentally introduced in 2000 without
/// guarding them.  Keeping them unguarded for backward compatibility.
#define NEWLIB_SC_NPROCESSORS_CONF              9
#define NEWLIB_SC_NPROCESSORS_ONLN             10
#define NEWLIB_SC_PHYS_PAGES                   11
#define NEWLIB_SC_AVPHYS_PAGES                 12
/// End of non-POSIX values.
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
/// NEWLIB_SC_THREAD_PRIO_PROTECT was NEWLIB_SC_THREAD_PRIO_CEILING in early drafts
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

/// ****************************************************************************

/**
 * @brief Syscall
 *
 * Base class for each emulated system call. operator() implements the behavior
 * of the emulated call
 */
template<class WordSize>
class Syscall {
  public:
  Syscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    processor(processor),
    osemu(osemu),
    latency(latency) {}
  virtual ~Syscall() {}
  virtual void set_processor(trap::ABIIf<WordSize>* processor) {
    this->processor = processor;
  }
  virtual void set_emulator(OSEmulatorBase* osemu) {
    this->osemu = osemu;
  }
  virtual void set_latency(sc_time& latency) {
    this->latency = latency;
  }
  virtual bool operator()() = 0;

  protected:
  ABIIf<WordSize>* processor;
  OSEmulatorBase* osemu;
  sc_time latency;
}; // class Syscall

/// ****************************************************************************

/**
 * @brief openSyscall
 */
template<class WordSize>
class openSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  openSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();
    // Read the name of the file to be opened.
    char pathname[256];
    for (int i = 0; i < 256; i++) {
      pathname[i] = (char)this->processor->read_char_mem(call_args[0] + i);
      if (pathname[i] == '\x0') {
        break;
      }
    }
    int flags = call_args[1];
    this->osemu->correct_flags(flags);
    int mode = call_args[2];
  #ifdef __GNUC__
    int ret = ::open(pathname, flags, mode);
  #else
    int ret = ::_open(pathname, flags, mode);
  #endif
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class openSyscall

/// ****************************************************************************

/**
 * @brief creatSyscall
 */
template<class WordSize>
class creatSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  creatSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();
    // Read the name of the file to be opened.
    char pathname[256];
    for (int i = 0; i < 256; i++) {
      pathname[i] = (char)this->processor->read_char_mem(call_args[0] + i);
      if (pathname[i] == '\x0') {
        break;
      }
    }
    int mode = call_args[1];
  #ifdef __GNUC__
    int ret = ::creat((char*)pathname, mode);
  #else
    int ret = ::_creat((char*)pathname, mode);
  #endif
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class creatSyscall

/// ****************************************************************************

/**
 * @brief closeSyscall
 */
template<class WordSize>
class closeSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  closeSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();
    int fd = call_args[0];
    if (fd < 0) {
      THROW_EXCEPTION("Invalid file descriptor " << fd << '.');
    }
  #ifdef __GNUC__
    if ((fd == fileno(stdin)) || (fd == fileno(stdout)) || (fd == fileno(stderr))) {
  #else
    if ((fd == _fileno(stdin)) || (fd == _fileno(stdout)) || (fd == _fileno(stderr))) {
  #endif
      this->processor->set_return_value(0);
      this->processor->return_from_call();
    } else {
  #ifdef __GNUC__
      int ret = ::close(fd);
  #else
      int ret = ::_close(fd);
  #endif
      this->processor->set_return_value(ret);
      this->processor->return_from_call();
    }
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class closeSyscall

/// ****************************************************************************

/**
 * @brief readSyscall
 */
template<class WordSize>
class readSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  readSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();
    int fd = call_args[0];
    if (fd < 0) {
      THROW_EXCEPTION("Invalid file descriptor " << fd << '.');
    }
    unsigned count = call_args[2];
    unsigned char* buf = new unsigned char[count];
  #ifdef __GNUC__
    int ret = ::read(fd, buf, count);
  #else
    int ret = ::_read(fd, buf, count);
  #endif
    // Write the read content into memory.
    WordSize dest_addr = call_args[1];
    for (int i = 0; i < ret; i++) {
      this->processor->write_char_mem(dest_addr + i, buf[i]);
    }
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    delete[] buf;
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class readSyscall

/// ****************************************************************************

/**
 * @brief writeSyscall
 */
template<class WordSize>
class writeSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  writeSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();
    int fd = call_args[0];
    if (fd < 0) {
      THROW_EXCEPTION("Invalid file descriptor " << fd << '.');
    }
    unsigned count = call_args[2];
    WordSize dest_addr = call_args[1];
    unsigned char* buf = new unsigned char[count];
    for (unsigned i = 0; i < count; i++) {
      buf[i] = this->processor->read_char_mem(dest_addr + i);
    }
  #ifdef __GNUC__
    int ret = ::write(fd, buf, count);
  #else
    int ret = ::_write(fd, buf, count);
  #endif
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    delete [] buf;
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class writeSyscall

/// ****************************************************************************

/**
 * @brief isattySyscall
 */
template<class WordSize>
class isattySyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  isattySyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();
    int desc = call_args[0];
  #ifdef __GNUC__
    int ret = ::isatty(desc);
  #else
    int ret = ::_isatty(desc);
  #endif
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class isattySyscall

/// ****************************************************************************

/**
 * @brief sbrkSyscall
 */
template<class WordSize>
class sbrkSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  sbrkSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();

    WordSize base = this->osemu->heap_ptr;
    long long increment = call_args[0];

    this->osemu->heap_ptr += increment;

    // Try to read from memory to see if it is possible to access the address
    // that has just been allocated. In case it is not possible it is an out-
    // of-memory error.
    try {
      this->processor->read_mem(this->osemu->heap_ptr);
      this->processor->set_return_value(base);
    } catch(...) {
      this->processor->set_return_value(-1);
      std::cerr << "SBRK: Unable to allocate " << increment << " bytes of memory at address " << std::hex <<
      std::showbase << base << std::dec << ". Not enough memory.\n";
    }

    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class sbrkSyscall

/// ****************************************************************************

/**
 * @brief lseekSyscall
 */
template<class WordSize>
class lseekSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  lseekSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();
    int fd = call_args[0];
    if (fd < 0) {
      THROW_EXCEPTION("Invalid file descriptor " << fd << '.');
    }
    int offset = call_args[1];
    int whence = call_args[2];
  #ifdef __GNUC__
    int ret = ::lseek(fd, offset, whence);
  #else
    int ret = ::_lseek(fd, offset, whence);
  #endif
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class lseekSyscall

/// ****************************************************************************

/**
 * @brief fstatSyscall
 */
template<class WordSize>
class fstatSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  fstatSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();
  #ifdef __GNUC__
    struct stat buf_stat;
  #else
    struct _stat buf_stat;
  #endif
    int fd = call_args[0];
    if (fd < 0) {
      THROW_EXCEPTION("Invalid file descriptor " << fd << '.');
    }
    int ret_addr = call_args[1];
  #ifdef __GNUC__
    int ret = ::fstat(fd, &buf_stat);
  #else
    int ret = ::_fstat(fd, &buf_stat);
  #endif
    if ((ret >= 0) && (ret_addr != 0)) {
      this->processor->write_mem(ret_addr, buf_stat.st_dev);
      this->processor->write_mem(ret_addr + 2, buf_stat.st_ino);
      this->processor->write_mem(ret_addr + 4, buf_stat.st_mode);
      this->processor->write_mem(ret_addr + 8, buf_stat.st_nlink);
      this->processor->write_mem(ret_addr + 10, buf_stat.st_uid);
      this->processor->write_mem(ret_addr + 12, buf_stat.st_gid);
      this->processor->write_mem(ret_addr + 14, buf_stat.st_rdev);
      this->processor->write_mem(ret_addr + 16, buf_stat.st_size);
      this->processor->write_mem(ret_addr + 20, buf_stat.st_atime);
      this->processor->write_mem(ret_addr + 28, buf_stat.st_mtime);
      this->processor->write_mem(ret_addr + 36, buf_stat.st_ctime);
  #ifdef __GNUC__
      this->processor->write_mem(ret_addr + 44, buf_stat.st_blksize);
      this->processor->write_mem(ret_addr + 48, buf_stat.st_blocks);
  #endif
    }
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class fstatSyscall

/// ****************************************************************************

/**
 * @brief statSyscall
 */
template<class WordSize>
class statSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  statSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();

  #ifdef __GNUC__
    struct stat buf_stat;
  #else
    struct _stat buf_stat;
  #endif

    char pathname[256];
    for (int i = 0; i < 256; i++) {
      pathname[i] = (char)this->processor->read_char_mem(call_args[0] + i);
      if (pathname[i] == '\x0') {
        break;
      }
    }
    int ret_addr = call_args[1];
  #ifdef __GNUC__
    int ret = ::stat((char*)pathname, &buf_stat);
  #else
    int ret = ::_stat((char*)pathname, &buf_stat);
  #endif
    if ((ret >= 0) && (ret_addr != 0)) {
      this->processor->write_mem(ret_addr, buf_stat.st_dev);
      this->processor->write_mem(ret_addr + 2, buf_stat.st_ino);
      this->processor->write_mem(ret_addr + 4, buf_stat.st_mode);
      this->processor->write_mem(ret_addr + 8, buf_stat.st_nlink);
      this->processor->write_mem(ret_addr + 10, buf_stat.st_uid);
      this->processor->write_mem(ret_addr + 12, buf_stat.st_gid);
      this->processor->write_mem(ret_addr + 14, buf_stat.st_rdev);
      this->processor->write_mem(ret_addr + 16, buf_stat.st_size);
      this->processor->write_mem(ret_addr + 20, buf_stat.st_atime);
      this->processor->write_mem(ret_addr + 28, buf_stat.st_mtime);
      this->processor->write_mem(ret_addr + 36, buf_stat.st_ctime);
  #ifdef __GNUC__
      this->processor->write_mem(ret_addr + 44, buf_stat.st_blksize);
      this->processor->write_mem(ret_addr + 48, buf_stat.st_blocks);
  #endif
    }
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class statSyscall

/// ****************************************************************************

/**
 * @brief _exitSyscall
 */
template<class WordSize>
class _exitSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  _exitSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    std::vector<WordSize> call_args = this->processor->read_args();
    this->processor->set_exit_value((int)call_args[0]);
    std::cout << std::endl << "Program exited with value " << this->processor->get_exit_value() << std::endl << std::endl;

    if (sc_is_running()) {
      OSEmulatorBase::num_programs--;
      // If there are other programs still running block the current processor.
      if (OSEmulatorBase::num_programs > 0) {
        sc_event end_event;
        wait(end_event);
      // This is the last running program. It is possible to call sc_stop().
      } else {
        sc_stop();
      }
      wait(SC_ZERO_TIME);
    }

    return true;
  }
}; // class _exitSyscall

/// ****************************************************************************

/**
 * @brief timesSyscall
 */
template<class WordSize>
class timesSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  timesSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();

    unsigned cur_sim_time = (unsigned)(sc_time_stamp().to_double() / 1.0e+6);
    WordSize times_ret_loc = call_args[0];
    if (times_ret_loc != 0) {
  #ifndef __GNUC__
      struct tms {
        clock_t tms_utime;        /* user time */
        clock_t tms_stime;        /* system time */
        clock_t tms_cutime;       /* user time of children */
        clock_t tms_cstime;       /* system time of children */
      };
  #endif
      struct tms buf;
      buf.tms_utime = cur_sim_time;
      buf.tms_stime = cur_sim_time;
      buf.tms_cutime = cur_sim_time;
      buf.tms_cstime = cur_sim_time;
      this->processor->write_mem(times_ret_loc, buf.tms_utime);
      times_ret_loc += 4;
      this->processor->write_mem(times_ret_loc, buf.tms_stime);
      times_ret_loc += 4;
      this->processor->write_mem(times_ret_loc, buf.tms_cutime);
      times_ret_loc += 4;
      this->processor->write_mem(times_ret_loc, buf.tms_cstime);
    }
    this->processor->set_return_value(cur_sim_time);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class timesSyscall

/// ****************************************************************************

/**
 * @brief timeSyscall
 */
template<class WordSize>
class timeSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  timeSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency),
      initial_time(time(0)) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();

    int t = call_args[0];
    int ret = this->initial_time + (int)(sc_time_stamp().to_double() / 1.0e+12);
    if (t != 0) {
      this->processor->write_mem(t, ret);
    }
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }

  private:
  int initial_time;
}; // class timeSyscall

/// ****************************************************************************

/**
 * @brief randomSyscall
 */
template<class WordSize>
class randomSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  randomSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    int ret = ::rand();
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class randomSyscall

/// ****************************************************************************

/**
 * @brief utimesSyscall
 */
template<class WordSize>
class utimesSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  utimesSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();

    char pathname[256];
    for (int i = 0; i < 256; i++) {
      pathname[i] = (char)this->processor->read_char_mem(call_args[0] + i);
      if (pathname[i] == '\x0') {
        break;
      }
    }

    int ret = -1;
    int times_addr = call_args[1];
    if (times_addr == 0) {
      ret = ::utimes((char*)pathname, NULL);
    } else {
      struct timeval times[2];
      times[0].tv_sec = this->processor->read_mem(times_addr);
      times[0].tv_usec = this->processor->read_mem(times_addr + 4);
      times[1].tv_sec = this->processor->read_mem(times_addr + 8);
      times[1].tv_usec = this->processor->read_mem(times_addr + 12);
      ret = ::utimes((char*)pathname, times);
    }

    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class utimesSyscall

/// ****************************************************************************

/**
 * @brief lstatSyscall
 */
template<class WordSize>
class lstatSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  lstatSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();

  #ifdef __GNUC__
    struct stat buf_stat;
  #else
    struct _stat buf_stat;
  #endif

    char pathname[256];
    for (int i = 0; i < 256; i++) {
      pathname[i] = (char)this->processor->read_char_mem(call_args[0] + i);
      if (pathname[i] == '\x0') {
        break;
      }
    }
    int ret_addr = call_args[1];
  #ifdef __GNUC__
    int ret = ::lstat((char*)pathname, &buf_stat);
  #else
    int ret = ::_lstat((char*)pathname, &buf_stat);
  #endif
    if ((ret >= 0) && (ret_addr != 0)) {
      this->processor->write_mem(ret_addr, buf_stat.st_dev);
      this->processor->write_mem(ret_addr + 2, buf_stat.st_ino);
      this->processor->write_mem(ret_addr + 4, buf_stat.st_mode);
      this->processor->write_mem(ret_addr + 8, buf_stat.st_nlink);
      this->processor->write_mem(ret_addr + 10, buf_stat.st_uid);
      this->processor->write_mem(ret_addr + 12, buf_stat.st_gid);
      this->processor->write_mem(ret_addr + 14, buf_stat.st_rdev);
      this->processor->write_mem(ret_addr + 16, buf_stat.st_size);
      this->processor->write_mem(ret_addr + 20, buf_stat.st_atime);
      this->processor->write_mem(ret_addr + 28, buf_stat.st_mtime);
      this->processor->write_mem(ret_addr + 36, buf_stat.st_ctime);
  #ifdef __GNUC__
      this->processor->write_mem(ret_addr + 44, buf_stat.st_blksize);
      this->processor->write_mem(ret_addr + 48, buf_stat.st_blocks);
  #endif
    }
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class lstatSyscall

/// ****************************************************************************

/**
 * @brief getpidSyscall
 */
template<class WordSize>
class getpidSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  getpidSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    this->processor->set_return_value(123);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class getpidSyscall

/// ****************************************************************************

/**
 * @brief chmodSyscall
 */
template<class WordSize>
class chmodSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  chmodSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();

    char pathname[256];
    for (int i = 0; i < 256; i++) {
      pathname[i] = (char)this->processor->read_char_mem(call_args[0] + i);
      if (pathname[i] == '\x0') {
        break;
      }
    }
    int mode = call_args[1];
  #ifdef __GNUC__
    int ret = ::chmod((char*)pathname, mode);
  #else
    int ret = ::_chmod((char*)pathname, mode);
  #endif
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class chmodSyscall

/// ****************************************************************************

/**
 * @brief dupSyscall
 */
template<class WordSize>
class dupSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  dupSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();
    int fd = call_args[0];
    if (fd < 0) {
      THROW_EXCEPTION("Invalid file descriptor " << fd << '.');
    }
  #ifdef __GNUC__
    int ret = ::dup(fd);
  #else
    int ret = ::_dup(fd);
  #endif
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class dupSyscall

/// ****************************************************************************

/**
 * @brief dup2Syscall
 */
template<class WordSize>
class dup2Syscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  dup2Syscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();
    int fd = call_args[0];
    if (fd < 0) {
      THROW_EXCEPTION("Invalid file descriptor " << fd << '.');
    }
    int newfd = call_args[1];
  #ifdef __GNUC__
    int ret = ::dup2(fd, newfd);
  #else
    int ret = ::_dup2(fd, newfd);
  #endif
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class dup2Syscall

/// ****************************************************************************

/**
 * @brief getenvSyscall
 */
template<class WordSize>
class getenvSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  getenvSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();

    char env_name[256];
    int env_name_addr = call_args[0];
    if (env_name_addr != 0) {
      for (int i = 0; i < 256; i++) {
        env_name[i] = (char)this->processor->read_char_mem(env_name_addr + i);
        if (env_name[i] == '\x0') {
          break;
        }
      }
      std::map<std::string, std::string>::iterator cur_env = this->osemu->env.find((std::string(env_name)));
      if (cur_env == this->osemu->env.end()) {
        this->processor->set_return_value(0);
        this->processor->return_from_call();
      } else {
        // Allocate memory for the result in the simulated memory. Then copy the
        // read environment variable here and return a pointer to it.
        unsigned base = this->osemu->heap_ptr;
        this->osemu->heap_ptr += cur_env->second.size() + 1;
        for (unsigned i = 0; i < cur_env->second.size(); i++) {
          this->processor->write_char_mem(base + i, cur_env->second[i]);
        }
        this->processor->write_char_mem(base + cur_env->second.size(), 0);
        this->processor->set_return_value(base);
        this->processor->return_from_call();
      }
    } else {
      this->processor->set_return_value(0);
      this->processor->return_from_call();
    }
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class getenvSyscall

/// ****************************************************************************

/**
 * @brief gettimeofdaySyscall
 */
template<class WordSize>
class gettimeofdaySyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  gettimeofdaySyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();

    int times_ret_loc = call_args[0];
    if (times_ret_loc != 0) {
      double cur_sim_time = sc_time_stamp().to_double();
      unsigned tv_sec = (unsigned)(cur_sim_time / 1.0e+12);
      unsigned tv_usec = (unsigned)((cur_sim_time - tv_sec * 1.0e+12) / 1.0e+6);
      this->processor->write_mem(times_ret_loc, tv_sec);
      times_ret_loc += 4;
      this->processor->write_mem(times_ret_loc, tv_usec);
    }
    this->processor->set_return_value(0);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class gettimeofdaySyscall

/// ****************************************************************************

/**
 * @brief killSyscall
 */
template<class WordSize>
class killSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  killSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    THROW_EXCEPTION("KILL system call not yet implemented.");

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class killSyscall

/// ****************************************************************************

/**
 * @brief errorSyscall
 */
template<class WordSize>
class errorSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  errorSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();

    int status = call_args[0];
    int err_num = call_args[1];
    char* err_str = ::strerror(err_num);
    if (status != 0) {
      std::cerr << std::endl << "Program exited with value " << status << std::endl << " Error message: " <<
      err_str << std::endl;
      if (sc_is_running()) {
        sc_stop();
      }
    } else {
      std::cerr << "An error occurred in the execution of the program: message = " << err_str << std::endl;
      this->processor->set_return_value(0);
      this->processor->return_from_call();
    }
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class errorSyscall

/// ****************************************************************************

/**
 * @brief chownSyscall
 */
template<class WordSize>
class chownSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  chownSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
#ifdef __GNUC__
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();

    char pathname[256];
    for (int i = 0; i < 256; i++) {
      pathname[i] = (char)this->processor->read_char_mem(call_args[0] + i);
      if (pathname[i] == '\x0') {
        break;
      }
    }
    uid_t owner = call_args[1];
    gid_t group = call_args[2];
    int ret = ::chown((char*)pathname, owner, group);
#else // ifdef __GNUC__
    int ret = 0;
#endif
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class chownSyscall

/// ****************************************************************************

/**
 * @brief unlinkSyscall
 */
template<class WordSize>
class unlinkSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  unlinkSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();

    char pathname[256];
    for (int i = 0; i < 256; i++) {
      pathname[i] = (char)this->processor->read_char_mem(call_args[0] + i);
      if (pathname[i] == '\x0') {
        break;
      }
    }
  #ifdef __GNUC__
    int ret = ::unlink((char*)pathname);
  #else
    int ret = ::_unlink((char*)pathname);
  #endif
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class unlinkSyscall

/// ****************************************************************************

/**
 * @brief usleepSyscall
 */
template<class WordSize>
class usleepSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  usleepSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Since we have a single process this function doesn't do anything :-)
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class usleepSyscall

/// ****************************************************************************

/**
 * @brief mainSyscall
 */
template<class WordSize>
class mainSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  mainSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();

    std::vector<WordSize> call_args = this->processor->read_args();
    if (call_args[0] != 0) {
      this->processor->post_call();
      return false;
    }

    std::vector<WordSize> main_args;

    if (this->osemu->program_args.size() == 0) {
      main_args.push_back(0);
      main_args.push_back(0);
      this->processor->set_args(main_args);
      this->processor->post_call();
      return false;
    }

    unsigned arg_addr = ((unsigned)this->osemu->heap_ptr) + (this->osemu->program_args.size() + 1) * 4;
    unsigned arg_num_addr = this->osemu->heap_ptr;
    std::vector<std::string>::iterator args_it, args_end;
    for (args_it = this->osemu->program_args.begin(), args_end = this->osemu->program_args.end(); args_it != args_end; args_it++) {
      this->processor->write_mem(arg_num_addr, arg_addr);
      arg_num_addr += 4;
      for (unsigned i = 0; i < args_it->size(); i++) {
        this->processor->write_char_mem(arg_addr + i, args_it->c_str()[i]);
      }
      this->processor->write_char_mem(arg_addr + args_it->size(), 0);
      arg_addr += args_it->size() + 1;
    }
    this->processor->write_mem(arg_num_addr, 0);

    main_args.push_back(this->osemu->program_args.size());
    main_args.push_back(this->osemu->heap_ptr);
    this->processor->set_args(main_args);
    this->osemu->heap_ptr = arg_addr;
    this->processor->post_call();

    return false;
  }
}; // class mainSyscall

/// ****************************************************************************

/**
 * @brief notifySyscall
 */
template<class WordSize>
class notifySyscall : public Syscall<WordSize> {
  //using Syscall<WordSize>::Syscall;
  notifySyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    this->processor->post_call();

    return false;
  }
}; // class notifySyscall

/// ****************************************************************************

/**
 * @brief sysconfSyscall
 */
template<class WordSize>
class sysconfSyscall : public Syscall<WordSize> {
  public:
  //using Syscall<WordSize>::Syscall;
  sysconfSyscall(ABIIf<WordSize>* processor, OSEmulatorBase* osemu, sc_time latency = SC_ZERO_TIME) :
    Syscall<WordSize>(processor, osemu, latency) {}

  bool operator()() {
    this->processor->pre_call();
    // Get the system call arguments.
    std::vector<WordSize> call_args = this->processor->read_args();

    int arg_id = call_args[0];
    int ret = -1;
    switch (arg_id) {
    case NEWLIB_SC_NPROCESSORS_ONLN:
      if (this->osemu->sysconf.find("_SC_NPROCESSORS_ONLN") == this->osemu->sysconf.end()) {
        ret = 1;
      } else {
        ret = this->osemu->sysconf["_SC_NPROCESSORS_ONLN"];
      }
      break;
    case NEWLIB_SC_CLK_TCK:
      if (this->osemu->sysconf.find("_SC_CLK_TCK") == this->osemu->sysconf.end()) {
        ret = 1000000;
      } else {
        ret = this->osemu->sysconf["_SC_CLK_TCK"];
      }
      break;
    default:
      ret = -1;
      break;
    }
    this->processor->set_return_value(ret);
    this->processor->return_from_call();
    this->processor->post_call();

    if (this->latency.to_double() > 0) {
      wait(this->latency);
    }

    return true;
  }
}; // class sysconfSyscall

} // namespace trap

/// ****************************************************************************
#endif // TRAP_SYSCALL_H

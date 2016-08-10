#include "core/common/sr_iss/intrinsics/platformintrinsic.h"
#include "core/common/sr_registry.h"
#include "core/common/sr_report.h"
#include "gaisler/leon3/intunit/processor.hpp"
#include "gaisler/leon3/intunit/interface.hpp"
#include "gaisler/leon3/leon3.h"

#include <map>
#include <string>
#include <vector>

#define NEWLIB_O_RDONLY          0x0000
#define NEWLIB_O_WRONLY          0x0001
#define NEWLIB_O_RDWR            0x0002
#define NEWLIB_O_APPEND          0x0008
#define NEWLIB_O_CREAT           0x0200
#define NEWLIB_O_TRUNC           0x0400
#define NEWLIB_O_EXCL            0x0800
#define NEWLIB_O_NOCTTY          0x8000
#define NEWLIB_O_NONBLOCK        0x4000

#define CORRECT_O_RDONLY             00
#define CORRECT_O_WRONLY             01
#define CORRECT_O_RDWR               02
#define CORRECT_O_CREAT            0100
#define CORRECT_O_EXCL             0200
#define CORRECT_O_NOCTTY           0400
#define CORRECT_O_TRUNC           01000
#define CORRECT_O_APPEND          02000
#define CORRECT_O_NONBLOCK        04000

#define \
  SR_HAS_INTRINSIC_GENERATOR(type, factory, isinstance) \
  static SrModuleRegistry __sr_module_registry_##type##__("PlatformIntrinsic", #type, &factory, &isinstance, __FILE__); \
  volatile SrModuleRegistry *__sr_module_registry_##type = &__sr_module_registry_##type##__;

#define \
  SR_HAS_INTRINSIC(type) \
    sc_core::sc_object *create_##type(sc_core::sc_module_name mn) { \
      return new type(mn); \
    } \
    bool isinstance_of_##type(sc_core::sc_object *obj) { \
      return dynamic_cast<type *>(obj) != NULL; \
    } \
    SR_HAS_INTRINSIC_GENERATOR(type, create_##type, isinstance_of_##type);

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
#if defined(__GNUC__) and not defined(_WIN32)
#include <sys/times.h>
#endif
#include <ctime>

using namespace trap;

template<class wordSize>
class openIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    openIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();
      // Lets read the name of the file to be opened
      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->m_processor->readCharMem(callArgs[0] + i);
        if (pathname[i] == '\x0') {
          break;
        }
      }
      int flags = callArgs[1];
      this->m_manager->correct_flags(flags);
      int mode = callArgs[2];
#ifdef __GNUC__
      int ret = ::open(pathname, flags, mode);
#else
      int ret = ::_open(pathname, flags, mode);
#endif
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class creatIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    creatIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();
      // Lets read the name of the file to be opened
      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->m_processor->readCharMem(callArgs[0] + i);
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
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class closeIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    closeIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();
      int fd = callArgs[0];
      if (fd < 0) {
        THROW_EXCEPTION("File descriptor " << fd << " not valid");
      }
#ifdef __GNUC__
      if ((fd == fileno(stdin)) || (fd == fileno(stdout)) || (fd == fileno(stderr))) {
#else
      if ((fd == _fileno(stdin)) || (fd == _fileno(stdout)) || (fd == _fileno(stderr))) {
#endif
        this->m_processor->setRetVal(0);
        this->m_processor->returnFromCall();
      } else {
#ifdef __GNUC__
        int ret = ::close(fd);
#else
        int ret = ::_close(fd);
#endif
        this->m_processor->setRetVal(ret);
        this->m_processor->returnFromCall();
      }
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class readIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    readIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();
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
        this->m_processor->writeCharMem(destAddress + i, buf[i]);
      }
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      delete[] buf;
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class writeIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    writeIntrinsic(sc_core::sc_module_name mn) :
      PlatformIntrinsic<wordSize>(mn),
      stdout_log_file(-1) {
    }

    virtual void setProcessor(trap::ABIIf<wordSize> *processor) {
      PlatformIntrinsic<wordSize>::setProcessor(processor);
      Leon3 *cpu;
      cpu = dynamic_cast<Leon3*>(&dynamic_cast<leon3_funclt_trap::LEON3_ABIIf*>(processor)->get_data_memory());
      if (cpu != NULL) {
        if (((std::string)cpu->g_stdout_filename).length() > 0) {
          this->stdout_log_file = open(((std::string)cpu->g_stdout_filename).c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        } else {
          this->stdout_log_file = -1;
        }
      }
    }

    ~writeIntrinsic() {
      if (this->stdout_log_file > 0) {
          close(this->stdout_log_file);
      }
    }

    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();
      int fd = callArgs[0];
      if (fd < 0) {
        THROW_EXCEPTION("File descriptor " << fd << " not valid");
      }
      unsigned count = callArgs[2];
      wordSize destAddress = callArgs[1];
      unsigned char *buf = new unsigned char[count];
      for (unsigned int i = 0; i < count; i++) {
        buf[i] = this->m_processor->readCharMem(destAddress + i);
      }
#ifdef __GNUC__
      int ret = ::write(fd, buf, count);
      if ((fd == STDOUT_FILENO) && (this->stdout_log_file > 0)) {
        ::write(this->stdout_log_file, buf, count);
      }
#else
      int ret = ::_write(fd, buf, count);
      if ((fd == STDOUT_FILENO) && (this->stdout_log_file > 0)) {
        ::_write(this->stdout_log_file, buf, count);
      }
#endif
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      delete[] buf;
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }

  protected:
    int stdout_log_file;
};

#if not defined(_WIN32)
template<class wordSize>
class isattyIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    isattyIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();
      int desc = callArgs[0];
#ifdef __GNUC__
      int ret = ::isatty(desc);
#else
      int ret = ::_isatty(desc);
#endif
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class sbrkIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    sbrkIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();

      wordSize base = this->m_manager->heapPointer;
      long long increment = callArgs[0];

      this->m_manager->heapPointer += increment;

      // I try to read from meory to see if it is possible to access the just allocated address;
      // In case it is not it means that I'm out of memory and I signal the error
      try {
        this->m_processor->readMem(this->m_manager->heapPointer);
        this->m_processor->setRetVal(base);
      } catch (...) {
        this->m_processor->setRetVal(-1);
        std::cerr << "SBRK: tried to allocate " << increment << " bytes of memory starting at address " << std::hex <<
        std::showbase << base << std::dec << " but it seems there is not enough memory" << std::endl;
      }

      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class lseekIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    lseekIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();
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
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class fstatIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    fstatIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();
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
        this->m_processor->writeMem(retAddr, buf_stat.st_dev);
        this->m_processor->writeMem(retAddr + 2, buf_stat.st_ino);
        this->m_processor->writeMem(retAddr + 4, buf_stat.st_mode);
        this->m_processor->writeMem(retAddr + 8, buf_stat.st_nlink);
        this->m_processor->writeMem(retAddr + 10, buf_stat.st_uid);
        this->m_processor->writeMem(retAddr + 12, buf_stat.st_gid);
        this->m_processor->writeMem(retAddr + 14, buf_stat.st_rdev);
        this->m_processor->writeMem(retAddr + 16, buf_stat.st_size);
        this->m_processor->writeMem(retAddr + 20, buf_stat.st_atime);
        this->m_processor->writeMem(retAddr + 28, buf_stat.st_mtime);
        this->m_processor->writeMem(retAddr + 36, buf_stat.st_ctime);
#ifdef __GNUC__
        this->m_processor->writeMem(retAddr + 44, buf_stat.st_blksize);
        this->m_processor->writeMem(retAddr + 48, buf_stat.st_blocks);
#endif
      }
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class statIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    statIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();

#ifdef __GNUC__
      struct stat buf_stat;
#else
      struct _stat buf_stat;
#endif

      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->m_processor->readCharMem(callArgs[0] + i);
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
        this->m_processor->writeMem(retAddr, buf_stat.st_dev);
        this->m_processor->writeMem(retAddr + 2, buf_stat.st_ino);
        this->m_processor->writeMem(retAddr + 4, buf_stat.st_mode);
        this->m_processor->writeMem(retAddr + 8, buf_stat.st_nlink);
        this->m_processor->writeMem(retAddr + 10, buf_stat.st_uid);
        this->m_processor->writeMem(retAddr + 12, buf_stat.st_gid);
        this->m_processor->writeMem(retAddr + 14, buf_stat.st_rdev);
        this->m_processor->writeMem(retAddr + 16, buf_stat.st_size);
        this->m_processor->writeMem(retAddr + 20, buf_stat.st_atime);
        this->m_processor->writeMem(retAddr + 28, buf_stat.st_mtime);
        this->m_processor->writeMem(retAddr + 36, buf_stat.st_ctime);
#ifdef __GNUC__
        this->m_processor->writeMem(retAddr + 44, buf_stat.st_blksize);
        this->m_processor->writeMem(retAddr + 48, buf_stat.st_blocks);
#endif
      }
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

#endif
template<class wordSize>
class _exitIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    _exitIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      std::vector<wordSize> callArgs = this->m_processor->readArgs();
      this->m_processor->setExitValue((int)callArgs[0]);
      std::cout << std::endl << "Program exited with value " << this->m_processor->getExitValue() << std::endl << std::endl;

      if (sc_is_running()) {
        IntrinsicBase::programsCount--;
        if (IntrinsicBase::programsCount > 0) { // in case there are other programs still running, block the current processor
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
#if not defined(_WIN32)

template<class wordSize>
class timesIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    timesIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();

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
        this->m_processor->writeMem(timesRetLoc, buf.tms_utime);
        timesRetLoc += 4;
        this->m_processor->writeMem(timesRetLoc, buf.tms_stime);
        timesRetLoc += 4;
        this->m_processor->writeMem(timesRetLoc, buf.tms_cutime);
        timesRetLoc += 4;
        this->m_processor->writeMem(timesRetLoc, buf.tms_cstime);
      }
      this->m_processor->setRetVal(curSimTime);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class timeIntrinsic : public PlatformIntrinsic<wordSize> {
  private:
    int initialTime;
  public:
    timeIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {
      this->initialTime = time(0);
    }
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();

      int t = callArgs[0];
      int ret = this->initialTime + (int)(sc_time_stamp().to_double() / 1.0e+12);
      if (t != 0) {
        this->m_processor->writeMem(t, ret);
      }
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class randomIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    randomIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      int ret = ::rand();
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class utimesIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    utimesIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();

      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->m_processor->readCharMem(callArgs[0] + i);
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
        times[0].tv_sec = this->m_processor->readMem(timesAddr);
        times[0].tv_usec = this->m_processor->readMem(timesAddr + 4);
        times[1].tv_sec = this->m_processor->readMem(timesAddr + 8);
        times[1].tv_usec = this->m_processor->readMem(timesAddr + 12);
        ret = ::utimes((char *)pathname, times);
      }

      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class lstatIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    lstatIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();

#ifdef __GNUC__
      struct stat buf_stat;
#else
      struct _stat buf_stat;
#endif

      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->m_processor->readCharMem(callArgs[0] + i);
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
        this->m_processor->writeMem(retAddr, buf_stat.st_dev);
        this->m_processor->writeMem(retAddr + 2, buf_stat.st_ino);
        this->m_processor->writeMem(retAddr + 4, buf_stat.st_mode);
        this->m_processor->writeMem(retAddr + 8, buf_stat.st_nlink);
        this->m_processor->writeMem(retAddr + 10, buf_stat.st_uid);
        this->m_processor->writeMem(retAddr + 12, buf_stat.st_gid);
        this->m_processor->writeMem(retAddr + 14, buf_stat.st_rdev);
        this->m_processor->writeMem(retAddr + 16, buf_stat.st_size);
        this->m_processor->writeMem(retAddr + 20, buf_stat.st_atime);
        this->m_processor->writeMem(retAddr + 28, buf_stat.st_mtime);
        this->m_processor->writeMem(retAddr + 36, buf_stat.st_ctime);
#ifdef __GNUC__
        this->m_processor->writeMem(retAddr + 44, buf_stat.st_blksize);
        this->m_processor->writeMem(retAddr + 48, buf_stat.st_blocks);
#endif
      }
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class getpidIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    getpidIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      this->m_processor->setRetVal(123);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class chmodIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    chmodIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();

      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->m_processor->readCharMem(callArgs[0] + i);
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
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class dupIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    dupIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();
      int fd = callArgs[0];
      if (fd < 0) {
        THROW_EXCEPTION("File descriptor not valid");
      }
#ifdef __GNUC__
      int ret = ::dup(fd);
#else
      int ret = ::_dup(fd);
#endif
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class dup2Intrinsic : public PlatformIntrinsic<wordSize> {
  public:
    dup2Intrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();
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
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class getenvIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    getenvIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();

      char envname[256];
      int envNameAddr = callArgs[0];
      if (envNameAddr != 0) {
        for (int i = 0; i < 256; i++) {
          envname[i] = (char)this->m_processor->readCharMem(envNameAddr + i);
          if (envname[i] == '\x0') {
            break;
          }
        }
        std::map<std::string, std::string>::iterator curEnv = this->m_manager->env.find((std::string(envname)));
        if (curEnv == this->m_processor->env.end()) {
          this->m_processor->setRetVal(0);
          this->m_processor->returnFromCall();
        } else {
          // I have to allocate memory for the result on the simulated memory;
          // I then have to copy the read environment variable here and return
          // the pointer to it
          unsigned int base = this->m_manager->heapPointer;
          this->m_manager->heapPointer += curEnv->second.size() + 1;
          for (unsigned int i = 0; i < curEnv->second.size(); i++) {
            this->m_processor->writeCharMem(base + i, curEnv->second[i]);
          }
          this->m_processor->writeCharMem(base + curEnv->second.size(), 0);
          this->m_processor->setRetVal(base);
          this->m_processor->returnFromCall();
        }
      } else {
        this->m_processor->setRetVal(0);
        this->m_processor->returnFromCall();
      }
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class gettimeofdayIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    gettimeofdayIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();

      int timesRetLoc = callArgs[0];
      if (timesRetLoc != 0) {
        double curSimTime = sc_time_stamp().to_double();
        unsigned int tv_sec = (unsigned int)(curSimTime / 1.0e+12);
        unsigned int tv_usec = (unsigned int)((curSimTime - tv_sec * 1.0e+12) / 1.0e+6);
        this->m_processor->writeMem(timesRetLoc, tv_sec);
        timesRetLoc += 4;
        this->m_processor->writeMem(timesRetLoc, tv_usec);
      }
      this->m_processor->setRetVal(0);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class killIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    killIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      THROW_EXCEPTION("KILL SystemCall not yet implemented");

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class errorIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    errorIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();

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
        this->m_processor->setRetVal(0);
        this->m_processor->returnFromCall();
      }
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class chownIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    chownIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
#ifdef __GNUC__
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();

      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->m_processor->readCharMem(callArgs[0] + i);
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
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class unlinkIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    unlinkIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();

      char pathname[256];
      for (int i = 0; i < 256; i++) {
        pathname[i] = (char)this->m_processor->readCharMem(callArgs[0] + i);
        if (pathname[i] == '\x0') {
          break;
        }
      }
#ifdef __GNUC__
      int ret = ::unlink((char *)pathname);
#else
      int ret = ::_unlink((char *)pathname);
#endif
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class renameIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    renameIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();

      char pathname_old[256];
      for (int i = 0; i < 256; i++) {
        pathname_old[i] = (char)this->m_processor->readCharMem(callArgs[0] + i);
        if (pathname_old[i] == '\x0') {
          break;
        }
      }

      char pathname_new[256];
      for (int i = 0; i < 256; i++) {
        pathname_new[i] = (char)this->m_processor->readCharMem(callArgs[1] + i);
        if (pathname_new[i] == '\x0') {
          break;
        }
      }
#ifdef __GNUC__
      int ret = ::rename((char *)pathname_old, (char *)pathname_new);
#else
      int ret = ::_rename((char *)pathname_old, (char *)pathname_new);
#endif
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

template<class wordSize>
class usleepIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    usleepIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      // Since we have a single process this function doesn't do anything :-)
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};

#endif

template<class wordSize>
class mainIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    mainIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();

      std::vector<wordSize> callArgs = this->m_processor->readArgs();
      if (callArgs[0] != 0) {
        this->m_processor->postCall();
        return false;
      }

      std::vector<wordSize> mainArgs;

      if (this->m_manager->programArgs.size() == 0) {
        mainArgs.push_back(0);
        mainArgs.push_back(0);
        this->m_processor->setArgs(mainArgs);
        this->m_processor->postCall();
        return false;
      }

      unsigned int argAddr = ((unsigned int)this->m_manager->heapPointer) + (this->m_manager->programArgs.size() + 1) * 4;
      unsigned int argNumAddr = this->m_manager->heapPointer;
      std::vector<std::string>::iterator argsIter, argsEnd;
      for (argsIter = this->m_manager->programArgs.begin(), argsEnd = this->m_manager->programArgs.end(); argsIter != argsEnd; argsIter++) {
        this->m_processor->writeMem(argNumAddr, argAddr);
        argNumAddr += 4;
        for (unsigned int i = 0; i < argsIter->size(); i++) {
          this->m_processor->writeCharMem(argAddr + i, argsIter->c_str()[i]);
        }
        this->m_processor->writeCharMem(argAddr + argsIter->size(), 0);
        argAddr += argsIter->size() + 1;
      }
      this->m_processor->writeMem(argNumAddr, 0);

      mainArgs.push_back(this->m_manager->programArgs.size());
      mainArgs.push_back(this->m_manager->heapPointer);
      this->m_processor->setArgs(mainArgs);
      this->m_manager->heapPointer = argAddr;
      this->m_processor->postCall();
      return false;
    }
};

template<class wordSize>
class notifyIntrinsic : public PlatformIntrinsic<wordSize> {
  public:
    notifyIntrinsic(sc_core::sc_module_name mn) : PlatformIntrinsic<wordSize>(mn) {}
    bool operator()() {
      this->m_processor->preCall();
      srCommand("notifyIntrinsic")("command");
      this->m_processor->postCall();
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

#if not defined(_WIN32)
template<class wordSize>
class sysconfIntrinsic : public PlatformIntrinsic<wordSize> {
  private:
    std::map<std::string, int> &sysconfmap;
  public:
    sysconfIntrinsic(ABIIf<wordSize> &processorInstance,
    std::map<std::string, int> &sysconfmap,
    sc_time latency = SC_ZERO_TIME) : PlatformIntrinsic<wordSize>(processorInstance, latency), sysconfmap(sysconfmap) {}
    bool operator()() {
      this->m_processor->preCall();
      // Lets get the system call arguments
      std::vector<wordSize> callArgs = this->m_processor->readArgs();

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
      this->m_processor->setRetVal(ret);
      this->m_processor->returnFromCall();
      this->m_processor->postCall();

      if (this->latency.to_double() > 0) {
        wait(this->latency);
      }

      return true;
    }
};
#endif

typedef openIntrinsic<unsigned int> openIntrinsic32;
SR_HAS_INTRINSIC(openIntrinsic32);
typedef creatIntrinsic<unsigned int> creatIntrinsic32;
SR_HAS_INTRINSIC(creatIntrinsic32);
typedef closeIntrinsic<unsigned int> closeIntrinsic32;
SR_HAS_INTRINSIC(closeIntrinsic32);
typedef readIntrinsic<unsigned int> readIntrinsic32;
SR_HAS_INTRINSIC(readIntrinsic32);
typedef writeIntrinsic<unsigned int> writeIntrinsic32;
SR_HAS_INTRINSIC(writeIntrinsic32);
#if not defined(_WIN32)
typedef isattyIntrinsic<unsigned int> isattyIntrinsic32;
SR_HAS_INTRINSIC(isattyIntrinsic32);
typedef sbrkIntrinsic<unsigned int> sbrkIntrinsic32;
SR_HAS_INTRINSIC(sbrkIntrinsic32);
typedef lseekIntrinsic<unsigned int> lseekIntrinsic32;
SR_HAS_INTRINSIC(lseekIntrinsic32);
typedef fstatIntrinsic<unsigned int> fstatIntrinsic32;
SR_HAS_INTRINSIC(fstatIntrinsic32);
#endif
typedef _exitIntrinsic<unsigned int> _exitIntrinsic32;
SR_HAS_INTRINSIC(_exitIntrinsic32);
#if not defined(_WIN32)
typedef timesIntrinsic<unsigned int> timesIntrinsic32;
SR_HAS_INTRINSIC(timesIntrinsic32);
typedef timeIntrinsic<unsigned int> timeIntrinsic32;
SR_HAS_INTRINSIC(timeIntrinsic32);
typedef randomIntrinsic<unsigned int> randomIntrinsic32;
SR_HAS_INTRINSIC(randomIntrinsic32);
typedef getpidIntrinsic<unsigned int> getpidIntrinsic32;
SR_HAS_INTRINSIC(getpidIntrinsic32);
typedef chmodIntrinsic<unsigned int> chmodIntrinsic32;
SR_HAS_INTRINSIC(chmodIntrinsic32);
typedef dupIntrinsic<unsigned int> dupIntrinsic32;
SR_HAS_INTRINSIC(dupIntrinsic32);
typedef dup2Intrinsic<unsigned int> dup2Intrinsic32;
SR_HAS_INTRINSIC(dup2Intrinsic32);
typedef getenvIntrinsic<unsigned int> getenvIntrinsic32;
//SR_HAS_INTRINSIC(getenvIntrinsic32);
typedef sysconfIntrinsic<unsigned int> sysconfIntrinsic32;
//SR_HAS_INTRINSIC(sysconfIntrinsic32);
typedef gettimeofdayIntrinsic<unsigned int> gettimeofdayIntrinsic32;
SR_HAS_INTRINSIC(gettimeofdayIntrinsic32);
typedef killIntrinsic<unsigned int> killIntrinsic32;
SR_HAS_INTRINSIC(killIntrinsic32);
typedef errorIntrinsic<unsigned int> errorIntrinsic32;
SR_HAS_INTRINSIC(errorIntrinsic32);
typedef chownIntrinsic<unsigned int> chownIntrinsic32;
SR_HAS_INTRINSIC(chownIntrinsic32);
typedef unlinkIntrinsic<unsigned int> unlinkIntrinsic32;
SR_HAS_INTRINSIC(unlinkIntrinsic32);
typedef usleepIntrinsic<unsigned int> usleepIntrinsic32;
SR_HAS_INTRINSIC(usleepIntrinsic32);
typedef statIntrinsic<unsigned int> statIntrinsic32;
SR_HAS_INTRINSIC(statIntrinsic32);
typedef lstatIntrinsic<unsigned int> lstatIntrinsic32;
SR_HAS_INTRINSIC(lstatIntrinsic32);
typedef utimesIntrinsic<unsigned int> utimesIntrinsic32;
SR_HAS_INTRINSIC(utimesIntrinsic32);
#endif
typedef mainIntrinsic<unsigned int> mainIntrinsic32;
SR_HAS_INTRINSIC(mainIntrinsic32);
typedef notifyIntrinsic<unsigned int> notifyIntrinsic32;
SR_HAS_INTRINSIC(notifyIntrinsic32);

void IntrinsicBase::correct_flags(int &val){
    int flags = 0;

    if( val &  NEWLIB_O_RDONLY )
        flags |= CORRECT_O_RDONLY;
    if( val &  NEWLIB_O_WRONLY )
        flags |= CORRECT_O_WRONLY;
    if( val &  NEWLIB_O_RDWR )
        flags |= CORRECT_O_RDWR;
    if( val & NEWLIB_O_CREAT )
        flags |= CORRECT_O_CREAT;
    if( val & NEWLIB_O_EXCL )
        flags |= CORRECT_O_EXCL;
    if( val & NEWLIB_O_NOCTTY )
        flags |= CORRECT_O_NOCTTY;
    if( val & NEWLIB_O_TRUNC )
        flags |= CORRECT_O_TRUNC;
    if( val & NEWLIB_O_APPEND )
        flags |= CORRECT_O_APPEND;
    if( val & NEWLIB_O_NONBLOCK )
        flags |= CORRECT_O_NONBLOCK;

    val = flags;
}

void IntrinsicBase::set_environ(const std::string name, const std::string value){
    env[name] = value;
}

void IntrinsicBase::set_sysconf(const std::string name, int value){
    sysconfmap[name] = value;
}

void IntrinsicBase::set_program_args(const std::vector<std::string> args){
    programArgs = args;
}

void IntrinsicBase::reset(){
    this->programArgs.clear();
    this->sysconfmap.clear();
    this->programArgs.clear();
    this->heapPointer = 0;
}

std::vector<unsigned int> IntrinsicBase::groupIDs;
unsigned int IntrinsicBase::programsCount = 0;

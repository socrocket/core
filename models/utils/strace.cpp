// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file strace.cpp
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include <cxxabi.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

/// Print a demangled stack backtrace of the caller function to FILE* out.
static void print_stacktrace(const char *source, FILE *out = stderr, unsigned int max_frames = 63) {
  char linkname[512];   // /proc/
                        // /exe
  char buf[512];
  pid_t pid;
  int ret;

  /* Get our PID and build the name of the link in /proc */
  pid = getpid();

  snprintf(linkname, sizeof (linkname), "/proc/%i/exe", pid);

  /* Now read the symbolic link */
  ret = readlink(linkname, buf, 512);
  buf[ret] = 0;

  fprintf(out, "stack trace (%s) for process %s (PID:%d):\n", source, buf, pid);

  // storage array for stack trace address data
  void *addrlist[max_frames + 1];

  // retrieve current stack addresses
  int addrlen = backtrace(addrlist, sizeof (addrlist) / sizeof (void *));

  if (addrlen == 0) {
    fprintf(out, "  \n");
    return;
  }

  // resolve addresses into strings containing "filename(function+address)",
  // this array must be free()-ed
  char **symbollist = backtrace_symbols(addrlist, addrlen);

  // allocate string which will be filled with the demangled function name
  size_t funcnamesize = 256;
  char *funcname = reinterpret_cast<char *>(malloc(funcnamesize));

  // iterate over the returned symbol lines. skip first two,
  // (addresses of this function and handler)
  for (int i = 2; i < addrlen; i++) {
    char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

    // find parentheses and +address offset surrounding the mangled name:
    // ./module(function+0x15c) [0x8048a6d]
    for (char *p = symbollist[i]; *p; ++p) {
      if (*p == '(') {
        begin_name = p;
      } else if (*p == '+') {
        begin_offset = p;
      } else if ((*p == ')') && begin_offset) {
        end_offset = p;
        break;
      }
    }
    if (begin_name && begin_offset && end_offset && (begin_name < begin_offset)) {
      *begin_name++ = '\0';
      *begin_offset++ = '\0';
      *end_offset = '\0';

      // mangled name is now in [begin_name, begin_offset) and caller
      // offset in [begin_offset, end_offset). now apply
      // __cxa_demangle():

      int status;
      char *ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
      if (status == 0) {
        funcname = ret;         // use possibly realloc()-ed string
        fprintf(out, "  (PID:%d) %s : %s+%s\n", pid, symbollist[i], funcname, begin_offset);
      } else {
        // demangling failed. Output function name as a C function with
        // no arguments.
        fprintf(out, "  (PID:%d) %s : %s()+%s\n", pid, symbollist[i], begin_name, begin_offset);
      }
    } else {
      // couldn't parse the line? print the whole line.
      fprintf(out, "  (PID:%d) %s: ??\n", pid, symbollist[i]);
    }
  }
  free(funcname);
  free(symbollist);

  fprintf(out, "stack trace END (PID:%d)\n", pid);
}

static void segv_handler(int, siginfo_t *, void *) {
  print_stacktrace("segv_handler");
  exit(1);
}

static void abrt_handler(int, siginfo_t *, void *) {
  print_stacktrace("abrt_handler");
  exit(1);
}

void terminate_handler1() {
  print_stacktrace("terminate_handler");
}

void unexpected_handler1() {
  print_stacktrace("unexpected_handler");
}

static int install_handler() {
  printf("Install_STrace_Handlers\n");
  struct sigaction sigact;
  sigact.sa_flags = SA_SIGINFO | SA_ONSTACK;

  sigact.sa_sigaction = segv_handler;
  if (sigaction(SIGSEGV, &sigact, (struct sigaction *)NULL) != 0) {
    fprintf(stderr, "error setting signal handler for %d (%s)\n", SIGSEGV, strsignal(SIGSEGV));
  }

  sigact.sa_sigaction = abrt_handler;
  if (sigaction(SIGABRT, &sigact, (struct sigaction *)NULL) != 0) {
    fprintf(stderr, "error setting signal handler for %d (%s)\n", SIGABRT, strsignal(SIGABRT));
  }

  std::set_terminate(&terminate_handler1);
  std::set_unexpected(&unexpected_handler1);
  // qInstallMsgHandler(qt_message_handler);

  return 0;
}

static int x = install_handler();
/// @}

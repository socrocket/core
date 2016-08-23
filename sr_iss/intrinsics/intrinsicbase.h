#ifndef CORE_COMMON_SR_ISS_INTSINSICS_INSTRINSICBASE_H
#define CORE_COMMON_SR_ISS_INTSINSICS_INSTRINSICBASE_H

#include <map>
#include <set>
#include <vector>
#include <string>

class IntrinsicBase {
  public:
    void set_program_args(const std::vector<std::string> args);
    void correct_flags(int &val);
    void set_environ(const std::string name, const std::string value);
    void set_sysconf(const std::string name, int value);
    void reset();

    std::map<std::string, std::string> env;
    std::map<std::string, int> sysconfmap;
    std::vector<std::string> programArgs;
    unsigned int heapPointer;
    unsigned int exitValue;

    static std::vector<unsigned int> groupIDs;
    static unsigned int programsCount;
};


#endif  // CORE_COMMON_SR_ISS_INTSINSICS_INSTRINSICBASE_H

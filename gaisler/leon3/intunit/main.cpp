/***************************************************************************\
 *
 *   
 *         _/        _/_/_/_/    _/_/    _/      _/   _/_/_/
 *        _/        _/        _/    _/  _/_/    _/         _/
 *       _/        _/_/_/    _/    _/  _/  _/  _/     _/_/
 *      _/        _/        _/    _/  _/    _/_/         _/
 *     _/_/_/_/  _/_/_/_/    _/_/    _/      _/   _/_/_/
 *   
 *
 *
 *   
 *   This file is part of LEON3.
 *   
 *   LEON3 is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *   
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *   or see <http://www.gnu.org/licenses/>.
 *   
 *
 *
 *   (c) Luca Fossati, fossati.l@gmail.com
 *
\***************************************************************************/



#include <string>
#ifdef _WIN32
#pragma warning( disable : 4101 )
#endif
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <vector>
#include <set>
#include <signal.h>
#include <boost/program_options.hpp>
#include <boost/timer.hpp>
#include <boost/filesystem.hpp>
#include "core/common/trapgen/misc/SparseMemoryLT.hpp"
#include "gaisler/leon3/intunit/processor.hpp"
#include "gaisler/leon3/intunit/instructions.hpp"
#include "core/common/trapgen/utils/trap_utils.hpp"
#include "core/common/trapgen/elfloader/elfFrontend.hpp"
#include "core/common/trapgen/elfloader/execLoader.hpp"
#include <stdexcept>
#include "core/common/trapgen/debugger/GDBStub.hpp"
#include "core/common/trapgen/profiler/profiler.hpp"
#include "core/common/trapgen/osEmulator/osEmulator.hpp"
#include "core/common/systemc.h"

#include "gaisler/leon3/intunit/irqGenerator.hpp"

#define LEON3_STANDALONE

std::string banner = std::string("\n\
\t\n\
\t      _/        _/_/_/_/    _/_/    _/      _/   _/_/_/\n\
\t     _/        _/        _/    _/  _/_/    _/         _/\n\
\t    _/        _/_/_/    _/    _/  _/  _/  _/     _/_/\n\
\t   _/        _/        _/    _/  _/    _/_/         _/\n\
\t  _/_/_/_/  _/_/_/_/    _/_/    _/      _/   _/_/_/\n\
\t\n\
\n\n\tLuca Fossati  -    email: fossati.l@gmail.com\n\n");
GDBStub< unsigned int > * gdbStub_ref = NULL;
void stopSimFunction( int sig ){
    if(gdbStub_ref != NULL && gdbStub_ref->simulationPaused){
        std::cerr << std::endl << "Simulation is already paused; ";
        std::cerr << "please use the connected GDB debugger to controll it" << std::endl \
            << std::endl;
    }
    else{
        std::cerr << std::endl << "Interrupted the simulation" << std::endl << std::endl;
        sc_stop();
        wait(SC_ZERO_TIME);
    }
}
unsigned int toIntNum( std::string & toConvert ){
    std::map<char, unsigned int> hexMap;
    hexMap['0'] = 0;
    hexMap['1'] = 1;
    hexMap['2'] = 2;
    hexMap['3'] = 3;
    hexMap['4'] = 4;
    hexMap['5'] = 5;
    hexMap['6'] = 6;
    hexMap['7'] = 7;
    hexMap['8'] = 8;
    hexMap['9'] = 9;
    hexMap['A'] = 10;
    hexMap['B'] = 11;
    hexMap['C'] = 12;
    hexMap['D'] = 13;
    hexMap['E'] = 14;
    hexMap['F'] = 15;
    hexMap['a'] = 10;
    hexMap['b'] = 11;
    hexMap['c'] = 12;
    hexMap['d'] = 13;
    hexMap['e'] = 14;
    hexMap['f'] = 15;

    std::string toConvTemp = toConvert;
    if(toConvTemp.size() >= 2 && toConvTemp[0] == '0' && (toConvTemp[1] == 'X' || toConvTemp[1] \
        == 'x'))
    toConvTemp = toConvTemp.substr(2);

    unsigned int result = 0;
    unsigned int pos = 0;
    std::string::reverse_iterator hexIter, hexIterEnd;
    for(hexIter = toConvTemp.rbegin(), hexIterEnd = toConvTemp.rend();
    hexIter != hexIterEnd; hexIter++){
        std::map<char, unsigned int>::iterator mapIter = hexMap.find(*hexIter);
        if(mapIter == hexMap.end()){
            throw std::runtime_error(toConvert + ": wrong hex number");
        }
        result |= (mapIter->second << pos);
        pos += 4;
    }
    return result;
}
std::pair< unsigned int, unsigned int > getCycleRange( const std::string & cycles_range, \
    const std::string & application ){
    std::pair<unsigned int, unsigned int> decodedRange;
    std::size_t foundSep = cycles_range.find('-');
    if(foundSep == std::string::npos){
        THROW_EXCEPTION("ERROR: specified address range " << cycles_range << " is not valid, \
            it has to be in the form start-end");
    }
    std::string start = cycles_range.substr(0, foundSep);
    std::string end = cycles_range.substr(foundSep + 1);
    // I first try standard numbers, then hex, then, if none of them, I check if a corresponding
    // symbol exists; if none I return an error.
    try{
        decodedRange.first = boost::lexical_cast<unsigned int>(start);
    }
    catch(...){
        try{
            decodedRange.first = toIntNum(start);
        }
        catch(...){
            trap::ELFFrontend &elfFE = trap::ELFFrontend::getInstance(application);
            bool valid = true;
            decodedRange.first = elfFE.getSymAddr(start, valid);
            if(!valid){
                THROW_EXCEPTION("ERROR: start address range " << start << " does not specify a valid \
                    address or a valid symbol");
            }
        }
    }
    try{
        decodedRange.second = boost::lexical_cast<unsigned int>(end);
    }
    catch(...){
        try{
            decodedRange.second = toIntNum(end);
        }
        catch(...){
            trap::ELFFrontend &elfFE = trap::ELFFrontend::getInstance(application);
            bool valid = true;
            decodedRange.second = elfFE.getSymAddr(end, valid);
            if(!valid){
                THROW_EXCEPTION("ERROR: end address range " << end << " does not specify a valid \
                    address or a valid symbol");
            }
        }
    }

    return decodedRange;
}
int sc_main( int argc, char * * argv ){
    using namespace leon3_funclt_trap;
    using namespace trap;

    std::cerr << banner << std::endl;

    boost::program_options::options_description desc("Processor simulator for LEON3", 120);
    desc.add_options()
    ("help,h", "produces the help message")
    ("debugger,d", "activates the use of the software debugger")
    ("profiler,p", boost::program_options::value<std::string>(),
    "activates the use of the software profiler, specifying the name of the output file")
    ("prof_range,g", boost::program_options::value<std::string>(),
    "specifies the range of addresses restricting the profiler instruction statistics")
    ("disable_fun_prof,n", "disables profiling statistics for the application routines")
    ("frequency,f", boost::program_options::value<double>(),
    "processor clock frequency specified in MHz [Default 1MHz]")
    ("cycles_range,c", boost::program_options::value<std::string>(),
    "start-end addresses between which computing the execution cycles")
    ("application,a", boost::program_options::value<std::string>(),
    "application to be executed on the simulator")
    ("disassembler,i", "prints the disassembly of the application")
    ("history,y", boost::program_options::value<std::string>(),
    "prints on the specified file the instruction history")
    ("arguments,r", boost::program_options::value<std::string>(),
    "command line arguments (if any) of the application being simulated - comma separated")
    ("environment,e", boost::program_options::value<std::string>(),
    "environmental variables (if any) visible to the application being simulated - comma \
        separated")
    ("sysconf,s", boost::program_options::value<std::string>(),
    "configuration information (if any) visible to the application being simulated - \
        comma separated")
    ;

    std::cerr << std::endl;

    boost::program_options::variables_map vm;
    try{
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, \
            desc), vm);
    }
    catch(boost::program_options::invalid_command_line_syntax &e){
        std::cerr << "ERROR in parsing the command line parametrs" << std::endl << std::endl;
        std::cerr << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return -1;
    }
    catch(boost::program_options::validation_error &e){
        std::cerr << "ERROR in parsing the command line parametrs" << std::endl << std::endl;
        std::cerr << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return -1;
    }
    catch(boost::program_options::error &e){
        std::cerr << "ERROR in parsing the command line parametrs" << std::endl << std::endl;
        std::cerr << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return -1;
    }
    boost::program_options::notify(vm);

    // Checking that the parameters are correctly specified
    if(vm.count("help") != 0){
        std::cout << desc << std::endl;
        return 0;
    }
    if(vm.count("application") == 0){
        std::cerr << "It is necessary to specify the application which has to be simulated" \
            << " using the --application command line option" << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return -1;
    }
    double latency = 1; // 1us
    if(vm.count("frequency") != 0){
        latency = 1/(vm["frequency"].as<double>());
    }
    //Now we can procede with the actual instantiation of the processor
    Processor_leon3_funclt procInst("LEON3", sc_time(latency, SC_US), false);
    //Here we instantiate the memory and connect it
    //wtih the processor
    SparseMemoryLT<2, 32> mem("procMem", 10485760, sc_time(latency*0, SC_US));
    procInst.instrMem.initSocket.bind(*(mem.socket[0]));
    procInst.dataMem.initSocket.bind(*(mem.socket[1]));

    IrqGenerator irqGen("irqGen", sc_time(latency*1000, SC_US));
    irqGen.initSignal(procInst.IRQ_port.irq_signal);
    procInst.irqAck.initSignal(irqGen.targSignal);

    std::cout << std::endl << "Loading the application and initializing the tools ..." \
        << std::endl;
    //And with the loading of the executable code
    boost::filesystem::path applicationPath = boost::filesystem::system_complete(boost::filesystem::path(vm["application"].as<std::string>()));
    if ( !boost::filesystem::exists( applicationPath ) ){
        std::cerr << "ERROR: specified application " << vm["application"].as<std::string>() \
            << " does not exist" << std::endl;
        return -1;
    }
    ExecLoader loader(vm["application"].as<std::string>());
    //Lets copy the binary code into memory
    unsigned char * programData = loader.getProgData();
    unsigned int programDim = loader.getProgDim();
    unsigned int progDataStart = loader.getDataStart();
    for(unsigned int i = 0; i < programDim; i++){
        mem.write_byte_dbg(progDataStart + i, programData[i]);
    }
    if(vm.count("disassembler") != 0){
        std:cout << "Entry Point: " << std::hex << std::showbase << loader.getProgStart() \
            << std::endl << std::endl;
        for(unsigned int i = 0; i < programDim; i+= 4){
            Instruction * curInstr = procInst.decode(procInst.instrMem.read_word_dbg(loader.getDataStart() \
                + i));
            std::cout << std::hex << std::showbase << progDataStart + i << ":    " << procInst.instrMem.read_word_dbg(progDataStart \
                + i);
            if(curInstr != NULL){
                std::cout << "    " << curInstr->getMnemonic();
            }
            std::cout << std::endl;
        }
        return 0;
    }
    //Finally I can set the processor variables
    procInst.ENTRY_POINT = loader.getProgStart();
    procInst.PROGRAM_LIMIT = programDim + progDataStart;
    procInst.PROGRAM_START = progDataStart;
    // Now I check if the count of the cycles among two locations (addresses or symbols) \
        is required
    std::pair<unsigned int, unsigned int> decodedRange((unsigned int)-1, (unsigned int)-1);
    if(vm.count("cycles_range") != 0){
        // Now, the range is in the form start-end, where start and end can be both integer \
            numbers
        // (both normal and hex) or symbols of the binary file
        std::string cycles_range = vm["cycles_range"].as<std::string>();
        decodedRange = getCycleRange(cycles_range, vm["application"].as<std::string>());
        // Finally now I can initialize the processor with the given address range values
        procInst.setProfilingRange(decodedRange.first, decodedRange.second);
    }

    //Initialization of the instruction history management; note that I need to enable \
        both if the debugger is being used
    //and/or if history needs to be dumped on an output file
    if(vm.count("debugger") > 0){
        procInst.enableHistory();
    }
    if(vm.count("history") > 0){
        #ifndef ENABLE_HISTORY
        std::cout << std::endl << "Unable to initialize instruction history as it has " << \
            "been disabled at compilation time" << std::endl << std::endl;
        #endif
        procInst.enableHistory(vm["history"].as<std::string>());
    }

    //Now I initialize the tools (i.e. debugger, os emulator, ...)
    OSEmulator< unsigned int> osEmu(*(procInst.abiIf));
    GDBStub< unsigned int > gdbStub(*(procInst.abiIf));
    Profiler< unsigned int > profiler(*(procInst.abiIf), vm["application"].as<std::string>(), \
        vm.count("disable_fun_prof") > 0);

    osEmu.initSysCalls(vm["application"].as<std::string>());
    std::vector<std::string> options;
    options.push_back(vm["application"].as<std::string>());
    if(vm.count("arguments") > 0){
        //Here we have to parse the command line program arguments; they are
        //in the form option,option,option ...
        std::string packedOpts = vm["arguments"].as<std::string>();
        while(packedOpts.size() > 0){
            std::size_t foundComma = packedOpts.find(',');
            if(foundComma != std::string::npos){
                options.push_back(packedOpts.substr(0, foundComma));
                packedOpts = packedOpts.substr(foundComma + 1);
            }
            else{
                options.push_back(packedOpts);
                break;
            }
        }
    }
    osEmu.set_program_args(options);
    if(vm.count("environment") > 0){
        //Here we have to parse the environment; they are
        //in the form option=value,option=value .....
        std::string packedEnv = vm["environment"].as<std::string>();
        while(packedEnv.size() > 0){
            std::size_t foundComma = packedEnv.find(',');
            std::string curEnv;
            if(foundComma != std::string::npos){
                curEnv = packedEnv.substr(0, foundComma);
                packedEnv = packedEnv.substr(foundComma + 1);
            }
            else{
                curEnv = packedEnv;
                packedEnv = "";
            }
            // Now I have to split the current environment
            std::size_t equalPos = curEnv.find('=');
            if(equalPos == std::string::npos){
                std::cerr << "Error in the command line environmental options: " << \
                "'=' not found in option " << curEnv << std::endl;
                return -1;
            }
            osEmu.set_environ(curEnv.substr(0, equalPos), curEnv.substr(equalPos + 1));
        }
    }
    if(vm.count("sysconf") > 0){
        //Here we have to parse the environment; they are
        //in the form option=value,option=value .....
        std::string packedEnv = vm["sysconf"].as<std::string>();
        while(packedEnv.size() > 0){
            std::size_t foundComma = packedEnv.find(',');
            std::string curEnv;
            if(foundComma != std::string::npos){
                curEnv = packedEnv.substr(0, foundComma);
                packedEnv = packedEnv.substr(foundComma + 1);
            }
            else{
                curEnv = packedEnv;
                packedEnv = "";
            }
            // Now I have to split the current environment
            std::size_t equalPos = curEnv.find('=');
            if(equalPos == std::string::npos){
                std::cerr << "Error in the command line sysconf options: " << \
                "'=' not found in option " << curEnv << std::endl;
                return -1;
            }
            try{
                osEmu.set_sysconf(curEnv.substr(0, equalPos), boost::lexical_cast<int>(curEnv.substr(equalPos \
                    + 1)));
            }
            catch(...){
                std::cerr << "Error in the command line sysconf options: " << \
                "error in option " << curEnv << std::endl;
                return -1;
            }
        }
    }
    procInst.toolManager.addTool(osEmu);
    if(vm.count("debugger") != 0){
        procInst.toolManager.addTool(gdbStub);
        gdbStub.initialize();
        procInst.instrMem.setDebugger(&gdbStub);
        procInst.dataMem.setDebugger(&gdbStub);
        gdbStub_ref = &gdbStub;
    }
    if(vm.count("profiler") != 0){
        std::set<std::string> toIgnoreFuns = osEmu.getRegisteredFunctions();
        toIgnoreFuns.erase("main");
        profiler.addIgnoredFunctions(toIgnoreFuns);
        // Now I check if there is the need to restrict the use of the profiler in a specific
        // cycles range
        if(vm.count("prof_range") != 0){
            std::pair<unsigned int, unsigned int> decodedProfRange = getCycleRange(vm["prof_range"].as<std::string>(), \
                vm["application"].as<std::string>());
            // Now, the range is in the form start-end, where start and end can be both integer \
                numbers
            // (both normal and hex) or symbols of the binary file
            profiler.setProfilingRange(decodedProfRange.first, decodedProfRange.second);
        }
        procInst.toolManager.addTool(profiler);
    }

    // Lets register the signal handlers for the CTRL^C key combination
    (void) signal(SIGINT, stopSimFunction);
    (void) signal(SIGTERM, stopSimFunction);
    (void) signal(10, stopSimFunction);

    std::cout << "... tools initialized" << std::endl << std::endl;

    //Now we can start the execution
    boost::timer t;
    sc_start();
    double elapsedSec = t.elapsed();
    if(vm.count("profiler") != 0){
        profiler.printCsvStats(vm["profiler"].as<std::string>());
    }
    std::cout << std::endl << "Elapsed " << elapsedSec << " sec. (real time)" << std::endl;
    std::cout << "Executed " << procInst.numInstructions << " instructions" << std::endl;
    std::cout << "Execution Speed: " << (double)procInst.numInstructions/(elapsedSec*1e6) \
        << " MIPS" << std::endl;
    std::cout << "Simulated time: " << ((sc_time_stamp().to_default_time_units())/(sc_time(1, \
        SC_US).to_default_time_units())) << " us" << std::endl;
    std::cout << "Elapsed " << std::dec << (unsigned int)(sc_time_stamp()/sc_time(latency, \
        SC_US)) << " cycles" << std::endl;
    if(decodedRange.first != (unsigned int)-1 || decodedRange.second != (unsigned int)-1){
        if(procInst.profTimeEnd == SC_ZERO_TIME){
            procInst.profTimeEnd = sc_time_stamp();
            std::cout << "End address " << std::hex << std::showbase << decodedRange.second << \
                " not found, counting until the end" << std::endl;
        }
        std::cout << "Cycles between addresses " << std::hex << std::showbase << decodedRange.first \
            << " - " << decodedRange.second << ": " << std::dec << (unsigned int)((procInst.profTimeEnd \
            - procInst.profTimeStart)/sc_time(latency, SC_US)) << std::endl;
    }
    std::cout << std::endl;

    return 0;
}


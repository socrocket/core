/***************************************************************************\
 *
 *
 *            ___        ___           ___           ___
 *           /  /\      /  /\         /  /\         /  /\
 *          /  /:/     /  /::\       /  /::\       /  /::\
 *         /  /:/     /  /:/\:\     /  /:/\:\     /  /:/\:\
 *        /  /:/     /  /:/~/:/    /  /:/~/::\   /  /:/~/:/
 *       /  /::\    /__/:/ /:/___ /__/:/ /:/\:\ /__/:/ /:/
 *      /__/:/\:\   \  \:\/:::::/ \  \:\/:/__\/ \  \:\/:/
 *      \__\/  \:\   \  \::/~~~~   \  \::/       \  \::/
 *           \  \:\   \  \:\        \  \:\        \  \:\
 *            \  \ \   \  \:\        \  \:\        \  \:\
 *             \__\/    \__\/         \__\/         \__\/
 *
 *
 *
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

#ifndef IRQGENERATOR_HPP
#define IRQGENERATOR_HPP

#include <iostream>

#include <systemc.h>
#include <trap_utils.hpp>
#include <boost/lexical_cast.hpp>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>

//Now some stuff for generating random numbers
#include <ctime>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#ifdef BOOST_NO_STDC_NAMESPACE
namespace std {
  using ::time;
}
#endif

class IrqGenerator : public sc_module{
    private:
        sc_time latency;
        int lastIrq;
        // Random number generator to be used during algorithm execution
        boost::minstd_rand generator;

    public:
        //The initiator socket is used for sending out, raising interrupts;
        //the target socket for receiving the acknowledgement
        tlm_utils::simple_initiator_socket< IrqGenerator, 32 > initSocket;
        tlm_utils::simple_target_socket< IrqGenerator, 32 > targSocket;

        SC_HAS_PROCESS( IrqGenerator );
        IrqGenerator( sc_module_name generatorName, sc_time latency ) : sc_module(generatorName),
                            targSocket(("irq_target_" + boost::lexical_cast<std::string>(generatorName)).c_str()),
                                        latency(latency), generator(static_cast<unsigned int>(std::time(NULL))){
            this->targSocket.register_b_transport(this, &IrqGenerator::b_transport);
            this->lastIrq = -1;
            SC_THREAD(generateIrq);
            end_module();
        }

        //Method used for receiving acknowledgements of interrupts; the ack consists of
        //uninteresting data and the address corresponds to the interrupt signal to
        //be lowered
        //As a response, I lower the interrupt by sending a NULL pointer on the initSocket
        void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay){
            if(this->lastIrq < 0){
                THROW_EXCEPTION("Error, lowering an interrupt which hasn't been raised yet!!");
            }
            tlm::tlm_command cmd = trans.get_command();
            sc_dt::uint64    adr = trans.get_address();
            unsigned char*   ptr = trans.get_data_ptr();
            if(trans.get_command() == tlm::TLM_READ_COMMAND){
                THROW_EXCEPTION("Error, the read request is not currently supported by external PINs");
            }
            else if(cmd == tlm::TLM_WRITE_COMMAND){
                if(this->lastIrq != adr){
                    THROW_EXCEPTION("Error, lowering interrupt " << std::hex << std::showbase << (unsigned int)adr << " while " << std::hex << std::showbase << this->lastIrq << " was raised");
                }
                else{
                    //finally I can really lower the interrupt: I send 0 on
                    //the initSocked
                    unsigned char data = 0;
                    trans.set_data_ptr(&data);
                    trans.set_dmi_allowed(false);
                    trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
                    sc_time delay;
                    this->initSocket->b_transport(trans, delay);
                    if(trans.is_response_error()){
                        std::string errorStr("Error in b_transport of PIN, response status = " + trans.get_response_string());
                        SC_REPORT_ERROR("TLM-2", errorStr.c_str());
                    }

                    this->lastIrq = -1;
                }
            }

            trans.set_response_status(tlm::TLM_OK_RESPONSE);
        }

        //Simple systemc thread which keeps on generating interrupts;
        //the number of the interrupt is printed to the screen before sending it to
        //the processor
        void generateIrq(){
            while(true){

                //An interrupt transaction is composed of a data pointer (containing
                //0 if the interrupt has to be lowered, different if raised) and an
                //address, corrisponding to the ID of the interrupt
                if(this->lastIrq == -1){
                    unsigned char data = 1;
                    tlm::tlm_generic_payload trans;
                    boost::uniform_int<> degen_dist(0x1, 0xe);
                    boost::variate_generator<boost::minstd_rand&, boost::uniform_int<> > deg(this->generator, degen_dist);
                    this->lastIrq = deg();
                    std::cerr << "Sending out IRQ id=" << std::hex << std::showbase << this->lastIrq << std::endl;
                    trans.set_address(this->lastIrq);
                    trans.set_data_ptr(&data);
                    trans.set_data_length(0);
                    trans.set_byte_enable_ptr(0);
                    trans.set_dmi_allowed(false);
                    trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
                    sc_time delay;
                    this->initSocket->b_transport(trans, delay);

                    if(trans.is_response_error()){
                        std::string errorStr("Error in generateIrq, response status = " + trans.get_response_string());
                        SC_REPORT_ERROR("TLM-2", errorStr.c_str());
                    }
                }
                wait(this->latency);
            }
        }
};

#endif


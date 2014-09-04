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
\ ***************************************************************************/

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <csignal>
#ifndef SIGTRAP
#define SIGTRAP 5
#endif

#ifndef __PRETTY_FUNCTION__
#ifdef __FUNCDNAME__
#define __PRETTY_FUNCTION__ __FUNCDNAME__
#else
#define __PRETTY_FUNCTION__ "NONAME"
#endif
#endif

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

#include "core/common/trapgen/utils/trap_utils.hpp"

#include "core/common/trapgen/debugger/GDBConnectionManager.hpp"

trap::GDBConnectionManager::GDBConnectionManager(bool endianess) : endianess(endianess), killed(false){
    this->socket = NULL;
    this->HexMap['0'] = 0;
    this->HexMap['1'] = 1;
    this->HexMap['2'] = 2;
    this->HexMap['3'] = 3;
    this->HexMap['4'] = 4;
    this->HexMap['5'] = 5;
    this->HexMap['6'] = 6;
    this->HexMap['7'] = 7;
    this->HexMap['8'] = 8;
    this->HexMap['9'] = 9;
    this->HexMap['A'] = 10;
    this->HexMap['B'] = 11;
    this->HexMap['C'] = 12;
    this->HexMap['D'] = 13;
    this->HexMap['E'] = 14;
    this->HexMap['F'] = 15;
    this->HexMap['a'] = 10;
    this->HexMap['b'] = 11;
    this->HexMap['c'] = 12;
    this->HexMap['d'] = 13;
    this->HexMap['e'] = 14;
    this->HexMap['f'] = 15;
}

trap::GDBConnectionManager::~GDBConnectionManager(){
   if(this->socket != NULL && !this->killed){
      delete this->socket;
   }
}

///Creates a socket connection waiting on the specified port;
///this will be later used to communicate with GDB
void trap::GDBConnectionManager::initialize(unsigned int port){
    try{
        boost::asio::ip::tcp::acceptor acceptor(this->io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

        this->socket = new boost::asio::ip::tcp::socket(this->io_service);
        std::cerr << "GDB: waiting for connections on port " << port << std::endl;
        acceptor.accept(*this->socket);
        std::cerr << "GDB: connection accepted on port " << port << std::endl << std::endl;
    }
    catch(...){
        this->killed = true;
        THROW_ERROR("Error during the creation of the connection on port " + boost::lexical_cast<std::string>(port));
    }
}

///Sends the response to the GDB debugger connected
void trap::GDBConnectionManager::sendResponse(GDBResponse &response){
    //All the response packets are in the form $<packet info>#<checksum>
    std::string payload;
    //First of all I compute the payload; it depends on the particular packet
    //sent by GDB
    switch(response.type){
        case GDBResponse::S_rsp:{
            //S request: informs GDB that a signal interrupted program execution
            payload = 'S' + this->toHexString((unsigned char)response.payload, 2);
        break;}
        case GDBResponse::T_rsp:{
            //T request: informs GDB that a signal interrupted program execution;
            //more datailed information is provided
            payload = 'T' + this->toHexString((unsigned char)response.payload, 2);
            std::vector<std::pair<std::string, unsigned int> >::iterator pairsIter, pairsEnd;
            for(pairsIter = response.info.begin(), pairsEnd = response.info.end();
                                    pairsIter != pairsEnd; pairsIter++){
            if(pairsIter->first == "thread" || pairsIter->first == "watch" ||
            pairsIter->first == "rwatch" || pairsIter->first == "awatch" || pairsIter->first == "library"){
                //it is a hex number representing a register and the second part
                //represents the value of that register
                payload += pairsIter->first + ':' + this->toHexString(pairsIter->second, response.size*2);
            }
            else{
                //it is a hex number representing a register and the second part
                //represents the value of that register; TODO we should check that
                //this is really a number
                payload += pairsIter->first + ':' + this->toHexString(pairsIter->second, response.size*2);
            }
            payload += ';';
            }
        break;}
        case GDBResponse::W_rsp:{
            //Process Exited
            payload = 'W' + this->toHexString((unsigned char)response.payload, 2);
            this->killed = true;
        break;}
        case GDBResponse::X_rsp:{
            //Process Exited
            payload = 'X' + this->toHexString((unsigned char)response.payload);
        break;}
        case GDBResponse::OUTPUT_rsp:{
            //Sending output message to the GDB debugger console
            payload = 'O';
        std::string::iterator messageIter, messageEnd;
            for(messageIter = response.message.begin(), messageEnd = response.message.end();
                                                    messageIter != messageEnd; messageIter++){
            payload += this->toHexString((unsigned char)*messageIter, 2);
            }
        break;}
        case GDBResponse::OK_rsp:{
            payload = "OK";
        break;}
        case GDBResponse::ERROR_rsp:{
            payload = 'E' + this->toHexString((unsigned char)response.payload, 2);
        break;}
        case GDBResponse::REG_READ_rsp:
        case GDBResponse::MEM_READ_rsp:{
        std::vector<char>::iterator dataIter, dataEnd;
        for(dataIter = response.data.begin(), dataEnd = response.data.end(); dataIter != dataEnd; dataIter++)
            payload += this->toHexString((unsigned char)*dataIter, 2);
        break;}
        case GDBResponse::CONT_rsp:{
            std::vector<char>::iterator dataIter, dataEnd;
            for(dataIter = response.data.begin(), dataEnd = response.data.end(); dataIter != dataEnd; dataIter++){
                payload += ';' + *dataIter;
            }
        break;}
        default:{
        break;}
    }

    unsigned char ack = '\x0';
    bool retry = false;
    do{
        //Now I complete the packet with the checksum
        std::string packet = '$' + payload + '#' + this->toHexString(this->computeChecksum(payload), 2);

        //Finally I can send the packet on the network
        boost::system::error_code asioError;
        boost::asio::write(*this->socket, boost::asio::buffer(packet.c_str(), packet.size()), boost::asio::transfer_all(), asioError);

        #ifndef NDEBUG
        if(asioError){
            std::cerr << __PRETTY_FUNCTION__ << ": WriteError " << asioError.message() << std::endl;
        }
        #endif

        if(this->killed)
            break;
        //Now I have to check that the packet was correctly received, otherwise I
        //retransmitt it
        int numRetries = 0;
        retry = false;
        do{
            numRetries = 0;
            do{
                ack = this->readQueueChar();
                if(ack == '\x0'){
                    std::cerr << std::endl <<  "Connection Unexpetedly closed by the GDB Debugger" << std::endl << std::endl;
                    this->killed = true;
                    return;
                }
                numRetries++;
            }while((ack & 0x7f) != '+' && (ack & 0x7f) != '-');
            if(numRetries > 1){
                //Some random characters were received,  I signal an error
                packet = "$E00#a5";
                boost::asio::write(*this->socket, boost::asio::buffer(packet.c_str(), packet.size()), boost::asio::transfer_all(), asioError);
                retry = true;
            }
        }while(numRetries > 1);
    }while((ack & 0x7f) == '-' || retry);
}

///Waits for the sending of a packet from GDB; it then parses it and
///translates it into the correct request
trap::GDBRequest trap::GDBConnectionManager::processRequest(){
    //Ok, we have to read the request and create its high level representation;
    //Note how this operation is repeated until the packet is correctly received
    std::string payload;
    bool correctlyReceived = false;
    GDBRequest req;

    do{
        unsigned char receivedChar = '\x0';
        boost::system::error_code asioError;

        //Reading the starting character
        while((receivedChar & 0x7f) != '$'){
            receivedChar = this->readQueueChar();
            if(receivedChar == '\x0'){
                std::cerr << std::endl << "Connection Unexpetedly closed by the GDB Debugger before sending a request" << std::endl;
                std::cerr << std::endl << "There might be a problem with your GDB client" << std::endl << std::endl;
                std::cerr << std::endl << "Detaching from GDB and restarting simulation" << std::endl << std::endl;
                req.type = GDBRequest::ERROR_req;
                this->killed = true;
                return req;
            }
        }
        if(receivedChar == 0x03){
            //It means that I received an interrupt from GDB, I stop the simulation and
            //become responsive
            req.type = GDBRequest::INTR_req;
            return req;
        }

        //Now I have to start reading the payload: I go on until # is enocuntered;
        payload = "";
        while((receivedChar & 0x7f) != '#'){
            receivedChar = this->readQueueChar();
            if(receivedChar == '\x0'){
                std::cerr << std::endl << "Connection Unexpetedly closed by the GDB Debugger" << std::endl << std::endl;
                std::cerr << std::endl << "Detaching from GDB and restarting simulation" << std::endl << std::endl;
                req.type = GDBRequest::ERROR_req;
                this->killed = true;
                return req;
            }
            if((receivedChar & 0x7f) != '#')
                payload += (char)(receivedChar & 0x7f);
        }

        //Finally I read the checksum: it should be composed of two characters
        char checkSum[2];
        checkSum[0] = this->readQueueChar();
        if(checkSum[0] == '\x0'){
            std::cerr << std::endl << "Connection Unexpetedly closed by the GDB Debugger" << std::endl << std::endl;
            std::cerr << std::endl << "Detaching from GDB and restarting simulation" << std::endl << std::endl;
            req.type = GDBRequest::ERROR_req;
            this->killed = true;
            return req;
        }
        checkSum[1] = this->readQueueChar();
        if(checkSum[1] == '\x0'){
            std::cerr << std::endl << "Connection Unexpetedly closed by the GDB Debugger" << std::endl << std::endl;
            std::cerr << std::endl << "Detaching from GDB and restarting simulation" << std::endl << std::endl;
            req.type = GDBRequest::ERROR_req;
            this->killed = true;
            return req;
        }

        //Now I have to check the checksum...
        correctlyReceived = this->checkChecksum(payload, checkSum);

        //...and communicate the result of the checking to GDB server
        char checkRes;
        if(correctlyReceived){
            checkRes = '+';
        }
        else{
            checkRes = '-';
        }
        boost::asio::write(*this->socket, boost::asio::buffer(&checkRes, 1), boost::asio::transfer_all(), asioError);
        if(asioError){
            std::cerr << __PRETTY_FUNCTION__ << ": WriteError " << asioError.message() << std::endl;
            req.type = GDBRequest::ERROR_req;
            return req;
        }
    }while(!correctlyReceived);

    //Now I have do decode the payload and transform it into the real packet
    char payType = payload[0];
    payload = payload.substr(1);
    switch(payType){
        case '!':{
            req.type = GDBRequest::EXCL_req;
        break;}
        case '?':{
            req.type = GDBRequest::QUEST_req;
        break;}
        case 'c':{
            req.type = GDBRequest::c_req;
            if(payload.size() > 0){
                req.address = this->toIntNum(payload);
            }
            else
                req.address = 0;
        break;}
        case 'C':{
            req.type = GDBRequest::C_req;
            std::string::size_type sepIndex = payload.find(';');
            if(sepIndex == std::string::npos)
                req.signal = this->toIntNum(payload);
            else{
                std::string temp = payload.substr(0, sepIndex);
                req.signal = this->toIntNum(temp);
                temp = payload.substr(sepIndex + 1);
                req.address = this->toIntNum(temp);
            }
        break;}
        case 'D':{
            req.type = GDBRequest::D_req;
        break;}
        case 'g':{
            req.type = GDBRequest::g_req;
        break;}
        case 'G':{
            req.type = GDBRequest::G_req;
            std::string::iterator payIter, payIterEnd;
            for(payIter = payload.begin(), payIterEnd = payload.end();
                                        payIter != payIterEnd; payIter++){
                std::string buf(1, *payIter);
                payIter++;
                buf += *payIter;
                req.data.push_back((unsigned char)this->toIntNum(buf));
            }
        break;}
        case 'H':{
            req.type = GDBRequest::H_req;
            req.data.push_back(payload[0]);
            payload = payload.substr(1);
            req.value = boost::lexical_cast<int>(payload);
        break;}
        case 'i':{
            req.type = GDBRequest::i_req;
            if(payload.size() > 0){
                std::string::size_type sepIndex = payload.find(',');
                if(sepIndex == std::string::npos){
                    req.value = 1;
                    req.address = this->toIntNum(payload);
                }
                else{
                    std::string temp = payload.substr(0, sepIndex);
                    req.address = this->toIntNum(temp);
                    temp = payload.substr(sepIndex + 1);
                    req.value = this->toIntNum(temp);
                }
            }
            else{
                req.address = 0;
                req.value = 1;
            }
        break;}
        case 'I':{
            req.type = GDBRequest::I_req;
        break;}
        case 'k':{
            this->killed = true;
            req.type = GDBRequest::k_req;
        break;}
        case 'm':{
            req.type = GDBRequest::m_req;
            std::string::size_type sepIndex = payload.find(',');
            std::string temp = payload.substr(0, sepIndex);
            req.address = this->toIntNum(temp);
            temp = payload.substr(sepIndex + 1);
            req.length = this->toIntNum(temp);
        break;}
        case 'M':{
            req.type = GDBRequest::M_req;
            std::string::size_type sepIndex = payload.find(',');
            std::string::size_type sepIndex2 = payload.find(':');
            std::string temp = payload.substr(0, sepIndex);
            req.address = this->toIntNum(temp);
            temp = payload.substr(sepIndex + 1, sepIndex2 - sepIndex - 1);
            req.length = this->toIntNum(temp);
            temp = payload.substr(sepIndex2 + 1);
            //Now it is time to read the content of memory
            std::string::iterator dataIter, dataIterEnd;
            for(dataIter = temp.begin(), dataIterEnd = temp.end(); dataIter != dataIterEnd; dataIter++){
                std::string buf(1, *dataIter);
                dataIter++;
                buf += *dataIter;
                req.data.push_back((unsigned char)this->toIntNum(buf));
            }
            //Now I check that the length of the buffer is the specified one
            if(req.data.size() != req.length)
                std::cerr << __PRETTY_FUNCTION__ << ": error in the M message: different length of bytes" << std::endl;
        break;}
        case 'p':{
            req.type = GDBRequest::p_req;
            req.reg = this->toIntNum(payload);
        break;}
        case 'P':{
            req.type = GDBRequest::P_req;
            std::string::size_type sepIndex = payload.find('=');
            if(sepIndex == std::string::npos)
                std::cerr << __PRETTY_FUNCTION__ << ": error in the P message: no arguments given" << std::endl;
            std::string temp = payload.substr(0, sepIndex);
            req.reg = this->toIntNum(temp);
            temp = payload.substr(sepIndex + 1);
            req.value = this->toIntNum(temp);
        break;}
        case 'q':{
            req.type = GDBRequest::q_req;
            std::string::size_type sepIndex = payload.find(',');
            if(sepIndex == std::string::npos){
                req.type = GDBRequest::UNK_req;
                break;
            }
            std::string temp = payload.substr(0, sepIndex);
            req.command = temp;
            temp = payload.substr(sepIndex + 1);
            req.extension = this->toStr(temp);
        break;}
        case 's':{
            req.type = GDBRequest::s_req;
            if(payload.size() > 0)
                req.address = this->toIntNum(payload);
            else
                req.address = 0;
        break;}
        case 'S':{
            req.type = GDBRequest::S_req;
            std::string::size_type sepIndex = payload.find(';');
            if(sepIndex == std::string::npos)
                req.signal = this->toIntNum(payload);
            else{
                std::string temp = payload.substr(0, sepIndex);
                req.signal = this->toIntNum(temp);
                temp = payload.substr(sepIndex + 1);
                req.address = this->toIntNum(temp);
            }
        break;}
        case 't':{
            req.type = GDBRequest::t_req;
            std::string::size_type sepIndex = payload.find(':');
            std::string::size_type sepIndex2 = payload.find(',');
            if(sepIndex == std::string::npos || sepIndex2 == std::string::npos)
                std::cerr << __PRETTY_FUNCTION__ << ": error in the t message: no arguments given" << std::endl;
            std::string temp = payload.substr(0, sepIndex);
            req.address = this->toIntNum(temp);
            temp = payload.substr(sepIndex + 1, sepIndex2 - sepIndex - 1);
            req.value = this->toIntNum(temp);
            temp = payload.substr(sepIndex2 + 1);
            req.length = this->toIntNum(temp);
        break;}
        case 'T':{
            req.type = GDBRequest::T_req;
            req.value = this->toIntNum(payload);
        break;}
        case 'v':{
            req.type = GDBRequest::v_req;
            req.command = payload;
        break;}
/*        case 'X':{
            req.type = GDBRequest::X_req;
            std::string::size_type sepIndex = payload.find(',');
            std::string::size_type sepIndex2 = payload.find(':');
            if(sepIndex == std::string::npos || sepIndex2 == std::string::npos)
                std::cerr << __PRETTY_FUNCTION__ << ": error in the M message: no arguments given" << std::endl;
            std::string temp = payload.substr(0, sepIndex);
            req.address = this->toIntNum(temp);
            temp = payload.substr(sepIndex + 1, sepIndex2 - sepIndex - 1);
            req.length = this->toIntNum(temp);
            temp = payload.substr(sepIndex2 + 1);
            //Now it is time to read the content of memory
            std::string::iterator dataIter, dataIterEnd;
            for(dataIter = temp.begin(), dataIterEnd = temp.end(); dataIter != dataIterEnd; dataIter++){
                req.data.push_back(*dataIter);
            }
            //Now I check that the length of the buffer is the specified one
            if(req.data.size() != req.length)
                std::cerr << __PRETTY_FUNCTION__ << ": error in the X message: different length of bytes" << std::endl;
        break;}*/
        case 'z':{
            req.type = GDBRequest::z_req;
            std::string::size_type sepIndex = payload.find(',');
            std::string::size_type sepIndex2 = payload.find_last_of(',');
            if(sepIndex == std::string::npos || sepIndex2 == std::string::npos)
                std::cerr << __PRETTY_FUNCTION__ << ": error in the z message: no arguments given" << std::endl;
            std::string temp = payload.substr(0, sepIndex);
            req.value = this->toIntNum(temp);
            temp = payload.substr(sepIndex + 1, sepIndex2 - sepIndex - 1);
            req.address = this->toIntNum(temp);
            temp = payload.substr(sepIndex2 + 1);
            req.length = this->toIntNum(temp);
        break;}
        case 'Z':{
            req.type = GDBRequest::Z_req;
            std::string::size_type sepIndex = payload.find(',');
            std::string::size_type sepIndex2 = payload.find_last_of(',');
            if(sepIndex == std::string::npos || sepIndex2 == std::string::npos)
                std::cerr << __PRETTY_FUNCTION__ << ": error in the Z message: no arguments given" << std::endl;
            std::string temp = payload.substr(0, sepIndex);
            req.value = this->toIntNum(temp);
            temp = payload.substr(sepIndex + 1, sepIndex2 - sepIndex - 1);
            req.address = this->toIntNum(temp);
            temp = payload.substr(sepIndex2 + 1);
            req.length = this->toIntNum(temp);
        break;}
        default:{
            req.type = GDBRequest::UNK_req;
        break;}
    }

    return req;
}

///Keeps waiting for a character on the channel with the GDB
///debugger
bool trap::GDBConnectionManager::checkInterrupt(){
    unsigned char recivedChar = '\x0';
    boost::system::error_code asioError;
    //Reading the starting character
    do{
        this->socket->read_some(boost::asio::buffer(&recivedChar, 1), asioError);
        if(asioError == boost::asio::error::eof){
            boost::mutex::scoped_lock lock(this->queueMutex);
            this->recvdChars.push_back('\x0');
            this->emptyQueueCond.notify_all();
            this->killed = true;
            return false;
        }
        if((recivedChar & 0x7f) != 0x03 && !this->killed){
            boost::mutex::scoped_lock lock(this->queueMutex);
            this->recvdChars.push_back(recivedChar);
            this->emptyQueueCond.notify_all();
        }
        //std::cerr << recivedChar << "- hex form -" << std::hex << std::showbase << (unsigned int)recivedChar << std::endl;
    }while((recivedChar & 0x7f) != 0x03 && !this->killed);
    //std::cerr << "returned since an interrupt was encountered" << std::endl;
    if(this->killed){
        return false;
    }
    else{
        return true;
    }
}

///Reads a character from the queue of ready characters
unsigned char trap::GDBConnectionManager::readQueueChar(){
    boost::mutex::scoped_lock lock(this->queueMutex);
    while(this->recvdChars.empty()){
        this->emptyQueueCond.wait(lock);
    }
    unsigned char recvd = this->recvdChars.front();
    this->recvdChars.pop_front();
    return recvd;
}

///Sends and interrupt message to the GDB debugger signaling that
///the execution of the program halted: this way the GDB
///debugger becomes responsive and it is possible to debug the
///program under test
void trap::GDBConnectionManager::sendInterrupt(){
   GDBResponse response;
   response.type = GDBResponse::S_rsp;
   response.payload = SIGTRAP;

   this->sendResponse(response);
}

///Closes the connection with the GDB debugger
void trap::GDBConnectionManager::disconnect(){
   if(this->socket != NULL){
      if(this->socket->is_open())
         this->socket->close();
      delete this->socket;
      this->socket = NULL;
   }
}

///Computes the checksum for the data
unsigned char trap::GDBConnectionManager::computeChecksum(std::string &data){
    unsigned char sum = 0;
    std::string::iterator dataIter, dataEnd;
    for(dataIter = data.begin(), dataEnd = data.end(); dataIter != dataEnd; dataIter++)
        sum += (unsigned char)*dataIter;
    return sum;
}

///Checks that the checksum included in the packet is correct
bool trap::GDBConnectionManager::checkChecksum(std::string &data, char checkSum[2]){
    unsigned char compCheck = this->computeChecksum(data);
    unsigned char recvCheck = ((this->chToHex(checkSum[0]) & 0x0f) << 4) | (this->chToHex(checkSum[1]) & 0x0f);
    return compCheck == recvCheck;
}

///Converts a generic numeric value into a string of hex numbers;
///each hex number of the string is in the same order of the endianess
///of the processor linked to this stub
std::string trap::GDBConnectionManager::toHexString(unsigned int value, int numChars){
    std::ostringstream os;

    if(!this->endianess && ((value & 0xFFFFFF00) != 0)){
        //I have to flip the bytes of value so that the endianess is correct
        value = ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) |
                ((value & 0x00FF0000) >> 8) | ((value & 0xFF000000) >> 24);
    }

    //Conversion to hex
    if(numChars == -1)
        os << std::hex << value;
    else
        os << std::hex << std::setw(numChars) << std::setfill('0') << value;

    return os.str();
}

///Converts an hexadecimal number expressed with a string
///into its correspondent integer number
///each hex number of the string is in the same order of the endianess
///of the processor linked to this stub
unsigned int trap::GDBConnectionManager::toIntNum(std::string &toConvert){
    std::string toConvTemp = toConvert;
    if(toConvTemp.size() >= 2 && toConvTemp[0] == '0' && (toConvTemp[1] == 'X' || toConvTemp[1] == 'x'))
        toConvTemp = toConvTemp.substr(2);

    unsigned int result = 0;
    unsigned int pos = 0;
    std::string::reverse_iterator hexIter, hexIterEnd;
    for(hexIter = toConvTemp.rbegin(), hexIterEnd = toConvTemp.rend();
                    hexIter != hexIterEnd; hexIter++){
        std::map<char, unsigned int>::iterator mapIter = this->HexMap.find(*hexIter);
        if(mapIter == this->HexMap.end()){
            THROW_ERROR(toConvert << " is not a valid hexadecimal number");
        }
        result |= (mapIter->second << pos);
        pos += 4;
    }

    return result;
}

///Converts a hex character to an int representing it
int trap::GDBConnectionManager::chToHex(unsigned char ch){
    if(ch >= 'a' && ch <= 'f')
        return ch-'a'+10;
    if(ch >= '0' && ch <= '9')
        return ch-'0';
    if(ch >= 'A' && ch <= 'F')
        return ch-'A'+10;
    return -1;
}

///Converts a hexadecimal number into the corresponding character string
std::string trap::GDBConnectionManager::toStr(std::string &toConvert){
    //What I do is to read the string element in couples; then
    //I convert each couple to its integer representation:
    //that is one string character
    if((toConvert.size() % 2) != 0){
         std::cerr << __PRETTY_FUNCTION__ << ": hexadecimal string contains an odd ammount of characters " << toConvert << std::endl;
         return "";
    }
    std::string outMex = "";
    for(unsigned int i = 0; i < toConvert.size()/2; i++){
        std::string temp = toConvert.substr(i*2,  2);
        outMex += (char)(this->toIntNum(temp));
    }

    return outMex;
}

/***************************************************************************\
 *
 *   This file is part of objcodeFrontend.
 *
 *   objcodeFrontend is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
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

#include <string>
#include <map>
#include <iostream>
#include <fstream>

#include "core/common/trapgen/utils/trap_utils.hpp"

extern "C" {
#include <gelf.h>
}

#include "core/common/trapgen/elfloader/elfFrontend.hpp"
#include "core/common/trapgen/elfloader/execLoader.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

trap::ExecLoader::ExecLoader(std::string fileName, bool plainFile) : plainFile(plainFile), elfFrontend(NULL), programData(NULL) {
    if(plainFile) {
        /// Here I simply have to read the input file, putting all the bytes
        /// of its content in the programData array
        boost::filesystem::path fileNamePath = boost::filesystem::system_complete(boost::filesystem::path(fileName));
        if ( !boost::filesystem::exists( fileNamePath ) ){
            THROW_EXCEPTION("Path " << fileName << " specified in the executable loader does not exists");
        }
        this->plainExecFile.open(fileName.c_str(), std::ifstream::in | std::ifstream::binary);
        if (!this->plainExecFile.good())
            THROW_EXCEPTION("Error in opening file " << fileName);

        unsigned int progDim = this->getProgDim();
        this->plainExecFile.seekg (0, std::ios::beg);
        this->programData = new unsigned char[progDim];
        //Now I read the whole file
        this->plainExecFile.read((char *)this->programData, progDim);
    } else {
        this->elfFrontend = &ELFFrontend::getInstance(fileName);
    }
}

trap::ExecLoader::~ExecLoader(){
    if (this->programData != NULL){
        delete [] this->programData;
        this->programData = NULL;
    }
    if (this->plainExecFile.is_open()){
        this->plainExecFile.close();        
    }
}

unsigned int trap::ExecLoader::getProgStart(){
    if(this->elfFrontend == NULL && !this->plainFile){
        THROW_ERROR("The binary parser not yet correcly created");
    }
    if(this->plainFile)
        return 0;
    else
        return this->elfFrontend->getEntryPoint();
}

unsigned int trap::ExecLoader::getProgDim(){
    if(this->elfFrontend == NULL && !this->plainFile){
        THROW_ERROR("The binary parser not yet correcly created");
    }
    if(this->plainFile){
        plainExecFile.seekg (0, std::ios::end);
        return plainExecFile.tellg();
    }
    else{
        return this->elfFrontend->getBinaryEnd() - this->elfFrontend->getBinaryStart();
    }
}

unsigned char * trap::ExecLoader::getProgData(){
    if(this->elfFrontend == NULL && !this->plainFile){
        THROW_ERROR("The binary parser not yet correcly created");
    }
    if(this->plainFile){
        return this->programData;
    }
    else{
        return this->elfFrontend->getProgData();
    }
}

unsigned int trap::ExecLoader::getDataStart(){
    if(this->elfFrontend == NULL && !this->plainFile){
        THROW_ERROR("The binary parser not yet correcly created");
    }
    if(this->plainFile)
        return 0;
    else
        return this->elfFrontend->getBinaryStart();
}

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
 *   the Free Software Foundation; either version 2 of the License, or
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
 *   (c) Luca Fossati, fossati@elet.polimi.it
 *
\***************************************************************************/



#include <registers.hpp>
#include <ostream>
#include <trap_utils.hpp>

using namespace leon3_funclt_trap;
InnerField & leon3_funclt_trap::InnerField::operator =( const InnerField & other \
    ) throw(){
    *this = (unsigned int)other;
    return *this;
}

leon3_funclt_trap::InnerField::~InnerField(){

}
void leon3_funclt_trap::Register::clockCycle() throw(){

}

leon3_funclt_trap::Register::Register(){

}

leon3_funclt_trap::Register::Register( const Register & other ){

}

InnerField & leon3_funclt_trap::Reg32_0::InnerField_VER::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xf0ffffffL;
    this->value |= ((other & 0xf) << 24);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_VER::InnerField_VER( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_0::InnerField_VER::~InnerField_VER(){

}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_ICC_z::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffbfffffL;
    this->value |= ((other & 0x1) << 22);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_ICC_z::InnerField_ICC_z( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_0::InnerField_ICC_z::~InnerField_ICC_z(){

}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_ICC_v::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffdfffffL;
    this->value |= ((other & 0x1) << 21);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_ICC_v::InnerField_ICC_v( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_0::InnerField_ICC_v::~InnerField_ICC_v(){

}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_EF::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffffefffL;
    this->value |= ((other & 0x1) << 12);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_EF::InnerField_EF( unsigned int & value ) \
    : value(value){

}

leon3_funclt_trap::Reg32_0::InnerField_EF::~InnerField_EF(){

}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_EC::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffffdfffL;
    this->value |= ((other & 0x1) << 13);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_EC::InnerField_EC( unsigned int & value ) \
    : value(value){

}

leon3_funclt_trap::Reg32_0::InnerField_EC::~InnerField_EC(){

}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_ICC_n::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xff7fffffL;
    this->value |= ((other & 0x1) << 23);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_ICC_n::InnerField_ICC_n( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_0::InnerField_ICC_n::~InnerField_ICC_n(){

}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_S::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffffff7fL;
    this->value |= ((other & 0x1) << 7);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_S::InnerField_S( unsigned int & value ) : \
    value(value){

}

leon3_funclt_trap::Reg32_0::InnerField_S::~InnerField_S(){

}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_ET::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffffffdfL;
    this->value |= ((other & 0x1) << 5);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_ET::InnerField_ET( unsigned int & value ) \
    : value(value){

}

leon3_funclt_trap::Reg32_0::InnerField_ET::~InnerField_ET(){

}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_ICC_c::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffefffffL;
    this->value |= ((other & 0x1) << 20);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_ICC_c::InnerField_ICC_c( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_0::InnerField_ICC_c::~InnerField_ICC_c(){

}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_PS::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffffffbfL;
    this->value |= ((other & 0x1) << 6);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_PS::InnerField_PS( unsigned int & value ) \
    : value(value){

}

leon3_funclt_trap::Reg32_0::InnerField_PS::~InnerField_PS(){

}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_PIL::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfffff0ffL;
    this->value |= ((other & 0xf) << 8);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_PIL::InnerField_PIL( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_0::InnerField_PIL::~InnerField_PIL(){

}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_CWP::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffffffe0L;
    this->value |= other;
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_CWP::InnerField_CWP( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_0::InnerField_CWP::~InnerField_CWP(){

}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_IMPL::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfffffff;
    this->value |= ((other & 0xf) << 28);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_IMPL::InnerField_IMPL( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_0::InnerField_IMPL::~InnerField_IMPL(){

}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_Empty::operator =( const unsigned \
    int & other ) throw(){
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_Empty::InnerField_Empty(){

}

leon3_funclt_trap::Reg32_0::InnerField_Empty::~InnerField_Empty(){

}
void leon3_funclt_trap::Reg32_0::immediateWrite( const unsigned int & value ) throw(){
    this->value = value;
}

unsigned int leon3_funclt_trap::Reg32_0::readNewValue() throw(){
    return this->value;
}

unsigned int leon3_funclt_trap::Reg32_0::operator ~() throw(){
    return ~(this->value);
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator =( const unsigned int & other ) throw(){
    this->value = other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator +=( const unsigned int & other ) throw(){
    this->value += other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator -=( const unsigned int & other ) throw(){
    this->value -= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator *=( const unsigned int & other ) throw(){
    this->value *= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator /=( const unsigned int & other ) throw(){
    this->value /= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator |=( const unsigned int & other ) throw(){
    this->value |= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator &=( const unsigned int & other ) throw(){
    this->value &= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator ^=( const unsigned int & other ) throw(){
    this->value ^= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator <<=( const unsigned int & other ) throw(){
    this->value <<= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator >>=( const unsigned int & other ) throw(){
    this->value >>= other;
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_0::operator +( const Reg32_0 & other ) const \
    throw(){
    return (this->value + other.value);
}

unsigned int leon3_funclt_trap::Reg32_0::operator -( const Reg32_0 & other ) const \
    throw(){
    return (this->value - other.value);
}

unsigned int leon3_funclt_trap::Reg32_0::operator *( const Reg32_0 & other ) const \
    throw(){
    return (this->value * other.value);
}

unsigned int leon3_funclt_trap::Reg32_0::operator /( const Reg32_0 & other ) const \
    throw(){
    return (this->value / other.value);
}

unsigned int leon3_funclt_trap::Reg32_0::operator |( const Reg32_0 & other ) const \
    throw(){
    return (this->value | other.value);
}

unsigned int leon3_funclt_trap::Reg32_0::operator &( const Reg32_0 & other ) const \
    throw(){
    return (this->value & other.value);
}

unsigned int leon3_funclt_trap::Reg32_0::operator ^( const Reg32_0 & other ) const \
    throw(){
    return (this->value ^ other.value);
}

unsigned int leon3_funclt_trap::Reg32_0::operator <<( const Reg32_0 & other ) const \
    throw(){
    return (this->value << other.value);
}

unsigned int leon3_funclt_trap::Reg32_0::operator >>( const Reg32_0 & other ) const \
    throw(){
    return (this->value >> other.value);
}

bool leon3_funclt_trap::Reg32_0::operator <( const Reg32_0 & other ) const throw(){
    return (this->value < other.value);
}

bool leon3_funclt_trap::Reg32_0::operator >( const Reg32_0 & other ) const throw(){
    return (this->value > other.value);
}

bool leon3_funclt_trap::Reg32_0::operator <=( const Reg32_0 & other ) const throw(){
    return (this->value <= other.value);
}

bool leon3_funclt_trap::Reg32_0::operator >=( const Reg32_0 & other ) const throw(){
    return (this->value >= other.value);
}

bool leon3_funclt_trap::Reg32_0::operator ==( const Reg32_0 & other ) const throw(){
    return (this->value == other.value);
}

bool leon3_funclt_trap::Reg32_0::operator !=( const Reg32_0 & other ) const throw(){
    return (this->value != other.value);
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator =( const Reg32_0 & other ) throw(){
    this->value = other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator +=( const Reg32_0 & other ) throw(){
    this->value += other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator -=( const Reg32_0 & other ) throw(){
    this->value -= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator *=( const Reg32_0 & other ) throw(){
    this->value *= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator /=( const Reg32_0 & other ) throw(){
    this->value /= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator |=( const Reg32_0 & other ) throw(){
    this->value |= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator &=( const Reg32_0 & other ) throw(){
    this->value &= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator ^=( const Reg32_0 & other ) throw(){
    this->value ^= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator <<=( const Reg32_0 & other ) throw(){
    this->value <<= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator >>=( const Reg32_0 & other ) throw(){
    this->value >>= other;
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_0::operator +( const Register & other ) const \
    throw(){
    return (this->value + other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator -( const Register & other ) const \
    throw(){
    return (this->value - other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator *( const Register & other ) const \
    throw(){
    return (this->value * other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator /( const Register & other ) const \
    throw(){
    return (this->value / other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator |( const Register & other ) const \
    throw(){
    return (this->value | other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator &( const Register & other ) const \
    throw(){
    return (this->value & other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator ^( const Register & other ) const \
    throw(){
    return (this->value ^ other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator <<( const Register & other ) const \
    throw(){
    return (this->value << other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator >>( const Register & other ) const \
    throw(){
    return (this->value >> other);
}

bool leon3_funclt_trap::Reg32_0::operator <( const Register & other ) const throw(){
    return (this->value < other);
}

bool leon3_funclt_trap::Reg32_0::operator >( const Register & other ) const throw(){
    return (this->value > other);
}

bool leon3_funclt_trap::Reg32_0::operator <=( const Register & other ) const throw(){
    return (this->value <= other);
}

bool leon3_funclt_trap::Reg32_0::operator >=( const Register & other ) const throw(){
    return (this->value >= other);
}

bool leon3_funclt_trap::Reg32_0::operator ==( const Register & other ) const throw(){
    return (this->value == other);
}

bool leon3_funclt_trap::Reg32_0::operator !=( const Register & other ) const throw(){
    return (this->value != other);
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator =( const Register & other ) throw(){
    this->value = other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator +=( const Register & other ) throw(){
    this->value += other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator -=( const Register & other ) throw(){
    this->value -= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator *=( const Register & other ) throw(){
    this->value *= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator /=( const Register & other ) throw(){
    this->value /= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator |=( const Register & other ) throw(){
    this->value |= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator &=( const Register & other ) throw(){
    this->value &= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator ^=( const Register & other ) throw(){
    this->value ^= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator <<=( const Register & other ) throw(){
    this->value <<= other;
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator >>=( const Register & other ) throw(){
    this->value >>= other;
    return *this;
}

std::ostream & leon3_funclt_trap::Reg32_0::operator <<( std::ostream & stream ) const \
    throw(){
    stream << std::hex << std::showbase << this->value << std::dec;
    return stream;
}

leon3_funclt_trap::Reg32_0::Reg32_0() : field_VER(this->value), field_ICC_z(this->value), \
    field_ICC_v(this->value), field_EF(this->value), field_EC(this->value), field_ICC_n(this->value), \
    field_S(this->value), field_ET(this->value), field_ICC_c(this->value), field_PS(this->value), \
    field_PIL(this->value), field_CWP(this->value), field_IMPL(this->value){
    this->value = 0;
}

InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_28::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xefffffffL;
    this->value |= ((other & 0x1) << 28);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_28::InnerField_WIM_28( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_28::~InnerField_WIM_28(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_29::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xdfffffffL;
    this->value |= ((other & 0x1) << 29);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_29::InnerField_WIM_29( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_29::~InnerField_WIM_29(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_24::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfeffffffL;
    this->value |= ((other & 0x1) << 24);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_24::InnerField_WIM_24( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_24::~InnerField_WIM_24(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_25::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfdffffffL;
    this->value |= ((other & 0x1) << 25);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_25::InnerField_WIM_25( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_25::~InnerField_WIM_25(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_26::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfbffffffL;
    this->value |= ((other & 0x1) << 26);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_26::InnerField_WIM_26( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_26::~InnerField_WIM_26(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_27::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xf7ffffffL;
    this->value |= ((other & 0x1) << 27);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_27::InnerField_WIM_27( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_27::~InnerField_WIM_27(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_20::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffefffffL;
    this->value |= ((other & 0x1) << 20);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_20::InnerField_WIM_20( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_20::~InnerField_WIM_20(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_21::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffdfffffL;
    this->value |= ((other & 0x1) << 21);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_21::InnerField_WIM_21( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_21::~InnerField_WIM_21(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_22::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffbfffffL;
    this->value |= ((other & 0x1) << 22);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_22::InnerField_WIM_22( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_22::~InnerField_WIM_22(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_23::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xff7fffffL;
    this->value |= ((other & 0x1) << 23);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_23::InnerField_WIM_23( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_23::~InnerField_WIM_23(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_9::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfffffdffL;
    this->value |= ((other & 0x1) << 9);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_9::InnerField_WIM_9( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_9::~InnerField_WIM_9(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_8::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfffffeffL;
    this->value |= ((other & 0x1) << 8);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_8::InnerField_WIM_8( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_8::~InnerField_WIM_8(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_1::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfffffffdL;
    this->value |= ((other & 0x1) << 1);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_1::InnerField_WIM_1( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_1::~InnerField_WIM_1(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_0::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfffffffeL;
    this->value |= other;
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_0::InnerField_WIM_0( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_0::~InnerField_WIM_0(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_3::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfffffff7L;
    this->value |= ((other & 0x1) << 3);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_3::InnerField_WIM_3( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_3::~InnerField_WIM_3(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_2::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfffffffbL;
    this->value |= ((other & 0x1) << 2);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_2::InnerField_WIM_2( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_2::~InnerField_WIM_2(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_5::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffffffdfL;
    this->value |= ((other & 0x1) << 5);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_5::InnerField_WIM_5( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_5::~InnerField_WIM_5(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_4::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffffffefL;
    this->value |= ((other & 0x1) << 4);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_4::InnerField_WIM_4( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_4::~InnerField_WIM_4(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_7::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffffff7fL;
    this->value |= ((other & 0x1) << 7);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_7::InnerField_WIM_7( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_7::~InnerField_WIM_7(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_6::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffffffbfL;
    this->value |= ((other & 0x1) << 6);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_6::InnerField_WIM_6( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_6::~InnerField_WIM_6(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_11::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfffff7ffL;
    this->value |= ((other & 0x1) << 11);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_11::InnerField_WIM_11( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_11::~InnerField_WIM_11(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_10::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfffffbffL;
    this->value |= ((other & 0x1) << 10);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_10::InnerField_WIM_10( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_10::~InnerField_WIM_10(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_13::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffffdfffL;
    this->value |= ((other & 0x1) << 13);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_13::InnerField_WIM_13( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_13::~InnerField_WIM_13(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_12::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffffefffL;
    this->value |= ((other & 0x1) << 12);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_12::InnerField_WIM_12( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_12::~InnerField_WIM_12(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_15::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffff7fffL;
    this->value |= ((other & 0x1) << 15);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_15::InnerField_WIM_15( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_15::~InnerField_WIM_15(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_14::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xffffbfffL;
    this->value |= ((other & 0x1) << 14);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_14::InnerField_WIM_14( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_14::~InnerField_WIM_14(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_17::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfffdffffL;
    this->value |= ((other & 0x1) << 17);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_17::InnerField_WIM_17( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_17::~InnerField_WIM_17(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_16::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfffeffffL;
    this->value |= ((other & 0x1) << 16);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_16::InnerField_WIM_16( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_16::~InnerField_WIM_16(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_19::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfff7ffffL;
    this->value |= ((other & 0x1) << 19);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_19::InnerField_WIM_19( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_19::~InnerField_WIM_19(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_18::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfffbffffL;
    this->value |= ((other & 0x1) << 18);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_18::InnerField_WIM_18( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_18::~InnerField_WIM_18(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_31::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0x7fffffff;
    this->value |= ((other & 0x1) << 31);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_31::InnerField_WIM_31( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_31::~InnerField_WIM_31(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_30::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xbfffffffL;
    this->value |= ((other & 0x1) << 30);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_30::InnerField_WIM_30( unsigned int & \
    value ) : value(value){

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_30::~InnerField_WIM_30(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_Empty::operator =( const unsigned \
    int & other ) throw(){
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_Empty::InnerField_Empty(){

}

leon3_funclt_trap::Reg32_1::InnerField_Empty::~InnerField_Empty(){

}
void leon3_funclt_trap::Reg32_1::immediateWrite( const unsigned int & value ) throw(){
    this->value = value;
}

unsigned int leon3_funclt_trap::Reg32_1::readNewValue() throw(){
    return this->value;
}

unsigned int leon3_funclt_trap::Reg32_1::operator ~() throw(){
    return ~(this->value);
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator =( const unsigned int & other ) throw(){
    this->value = other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator +=( const unsigned int & other ) throw(){
    this->value += other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator -=( const unsigned int & other ) throw(){
    this->value -= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator *=( const unsigned int & other ) throw(){
    this->value *= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator /=( const unsigned int & other ) throw(){
    this->value /= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator |=( const unsigned int & other ) throw(){
    this->value |= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator &=( const unsigned int & other ) throw(){
    this->value &= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator ^=( const unsigned int & other ) throw(){
    this->value ^= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator <<=( const unsigned int & other ) throw(){
    this->value <<= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator >>=( const unsigned int & other ) throw(){
    this->value >>= other;
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_1::operator +( const Reg32_1 & other ) const \
    throw(){
    return (this->value + other.value);
}

unsigned int leon3_funclt_trap::Reg32_1::operator -( const Reg32_1 & other ) const \
    throw(){
    return (this->value - other.value);
}

unsigned int leon3_funclt_trap::Reg32_1::operator *( const Reg32_1 & other ) const \
    throw(){
    return (this->value * other.value);
}

unsigned int leon3_funclt_trap::Reg32_1::operator /( const Reg32_1 & other ) const \
    throw(){
    return (this->value / other.value);
}

unsigned int leon3_funclt_trap::Reg32_1::operator |( const Reg32_1 & other ) const \
    throw(){
    return (this->value | other.value);
}

unsigned int leon3_funclt_trap::Reg32_1::operator &( const Reg32_1 & other ) const \
    throw(){
    return (this->value & other.value);
}

unsigned int leon3_funclt_trap::Reg32_1::operator ^( const Reg32_1 & other ) const \
    throw(){
    return (this->value ^ other.value);
}

unsigned int leon3_funclt_trap::Reg32_1::operator <<( const Reg32_1 & other ) const \
    throw(){
    return (this->value << other.value);
}

unsigned int leon3_funclt_trap::Reg32_1::operator >>( const Reg32_1 & other ) const \
    throw(){
    return (this->value >> other.value);
}

bool leon3_funclt_trap::Reg32_1::operator <( const Reg32_1 & other ) const throw(){
    return (this->value < other.value);
}

bool leon3_funclt_trap::Reg32_1::operator >( const Reg32_1 & other ) const throw(){
    return (this->value > other.value);
}

bool leon3_funclt_trap::Reg32_1::operator <=( const Reg32_1 & other ) const throw(){
    return (this->value <= other.value);
}

bool leon3_funclt_trap::Reg32_1::operator >=( const Reg32_1 & other ) const throw(){
    return (this->value >= other.value);
}

bool leon3_funclt_trap::Reg32_1::operator ==( const Reg32_1 & other ) const throw(){
    return (this->value == other.value);
}

bool leon3_funclt_trap::Reg32_1::operator !=( const Reg32_1 & other ) const throw(){
    return (this->value != other.value);
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator =( const Reg32_1 & other ) throw(){
    this->value = other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator +=( const Reg32_1 & other ) throw(){
    this->value += other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator -=( const Reg32_1 & other ) throw(){
    this->value -= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator *=( const Reg32_1 & other ) throw(){
    this->value *= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator /=( const Reg32_1 & other ) throw(){
    this->value /= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator |=( const Reg32_1 & other ) throw(){
    this->value |= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator &=( const Reg32_1 & other ) throw(){
    this->value &= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator ^=( const Reg32_1 & other ) throw(){
    this->value ^= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator <<=( const Reg32_1 & other ) throw(){
    this->value <<= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator >>=( const Reg32_1 & other ) throw(){
    this->value >>= other;
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_1::operator +( const Register & other ) const \
    throw(){
    return (this->value + other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator -( const Register & other ) const \
    throw(){
    return (this->value - other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator *( const Register & other ) const \
    throw(){
    return (this->value * other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator /( const Register & other ) const \
    throw(){
    return (this->value / other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator |( const Register & other ) const \
    throw(){
    return (this->value | other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator &( const Register & other ) const \
    throw(){
    return (this->value & other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator ^( const Register & other ) const \
    throw(){
    return (this->value ^ other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator <<( const Register & other ) const \
    throw(){
    return (this->value << other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator >>( const Register & other ) const \
    throw(){
    return (this->value >> other);
}

bool leon3_funclt_trap::Reg32_1::operator <( const Register & other ) const throw(){
    return (this->value < other);
}

bool leon3_funclt_trap::Reg32_1::operator >( const Register & other ) const throw(){
    return (this->value > other);
}

bool leon3_funclt_trap::Reg32_1::operator <=( const Register & other ) const throw(){
    return (this->value <= other);
}

bool leon3_funclt_trap::Reg32_1::operator >=( const Register & other ) const throw(){
    return (this->value >= other);
}

bool leon3_funclt_trap::Reg32_1::operator ==( const Register & other ) const throw(){
    return (this->value == other);
}

bool leon3_funclt_trap::Reg32_1::operator !=( const Register & other ) const throw(){
    return (this->value != other);
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator =( const Register & other ) throw(){
    this->value = other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator +=( const Register & other ) throw(){
    this->value += other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator -=( const Register & other ) throw(){
    this->value -= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator *=( const Register & other ) throw(){
    this->value *= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator /=( const Register & other ) throw(){
    this->value /= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator |=( const Register & other ) throw(){
    this->value |= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator &=( const Register & other ) throw(){
    this->value &= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator ^=( const Register & other ) throw(){
    this->value ^= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator <<=( const Register & other ) throw(){
    this->value <<= other;
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator >>=( const Register & other ) throw(){
    this->value >>= other;
    return *this;
}

std::ostream & leon3_funclt_trap::Reg32_1::operator <<( std::ostream & stream ) const \
    throw(){
    stream << std::hex << std::showbase << this->value << std::dec;
    return stream;
}

leon3_funclt_trap::Reg32_1::Reg32_1() : field_WIM_28(this->value), field_WIM_29(this->value), \
    field_WIM_24(this->value), field_WIM_25(this->value), field_WIM_26(this->value), \
    field_WIM_27(this->value), field_WIM_20(this->value), field_WIM_21(this->value), \
    field_WIM_22(this->value), field_WIM_23(this->value), field_WIM_9(this->value), field_WIM_8(this->value), \
    field_WIM_1(this->value), field_WIM_0(this->value), field_WIM_3(this->value), field_WIM_2(this->value), \
    field_WIM_5(this->value), field_WIM_4(this->value), field_WIM_7(this->value), field_WIM_6(this->value), \
    field_WIM_11(this->value), field_WIM_10(this->value), field_WIM_13(this->value), \
    field_WIM_12(this->value), field_WIM_15(this->value), field_WIM_14(this->value), \
    field_WIM_17(this->value), field_WIM_16(this->value), field_WIM_19(this->value), \
    field_WIM_18(this->value), field_WIM_31(this->value), field_WIM_30(this->value){
    this->value = 0;
}

InnerField & leon3_funclt_trap::Reg32_2::InnerField_TBA::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfff;
    this->value |= ((other & 0xfffff) << 12);
    return *this;
}

leon3_funclt_trap::Reg32_2::InnerField_TBA::InnerField_TBA( unsigned int & value \
    ) : value(value){

}

leon3_funclt_trap::Reg32_2::InnerField_TBA::~InnerField_TBA(){

}
InnerField & leon3_funclt_trap::Reg32_2::InnerField_TT::operator =( const unsigned \
    int & other ) throw(){
    this->value &= 0xfffff00fL;
    this->value |= ((other & 0xff) << 4);
    return *this;
}

leon3_funclt_trap::Reg32_2::InnerField_TT::InnerField_TT( unsigned int & value ) \
    : value(value){

}

leon3_funclt_trap::Reg32_2::InnerField_TT::~InnerField_TT(){

}
InnerField & leon3_funclt_trap::Reg32_2::InnerField_Empty::operator =( const unsigned \
    int & other ) throw(){
    return *this;
}

leon3_funclt_trap::Reg32_2::InnerField_Empty::InnerField_Empty(){

}

leon3_funclt_trap::Reg32_2::InnerField_Empty::~InnerField_Empty(){

}
void leon3_funclt_trap::Reg32_2::immediateWrite( const unsigned int & value ) throw(){
    this->value = value;
}

unsigned int leon3_funclt_trap::Reg32_2::readNewValue() throw(){
    return this->value;
}

unsigned int leon3_funclt_trap::Reg32_2::operator ~() throw(){
    return ~(this->value);
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator =( const unsigned int & other ) throw(){
    this->value = other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator +=( const unsigned int & other ) throw(){
    this->value += other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator -=( const unsigned int & other ) throw(){
    this->value -= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator *=( const unsigned int & other ) throw(){
    this->value *= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator /=( const unsigned int & other ) throw(){
    this->value /= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator |=( const unsigned int & other ) throw(){
    this->value |= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator &=( const unsigned int & other ) throw(){
    this->value &= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator ^=( const unsigned int & other ) throw(){
    this->value ^= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator <<=( const unsigned int & other ) throw(){
    this->value <<= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator >>=( const unsigned int & other ) throw(){
    this->value >>= other;
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_2::operator +( const Reg32_2 & other ) const \
    throw(){
    return (this->value + other.value);
}

unsigned int leon3_funclt_trap::Reg32_2::operator -( const Reg32_2 & other ) const \
    throw(){
    return (this->value - other.value);
}

unsigned int leon3_funclt_trap::Reg32_2::operator *( const Reg32_2 & other ) const \
    throw(){
    return (this->value * other.value);
}

unsigned int leon3_funclt_trap::Reg32_2::operator /( const Reg32_2 & other ) const \
    throw(){
    return (this->value / other.value);
}

unsigned int leon3_funclt_trap::Reg32_2::operator |( const Reg32_2 & other ) const \
    throw(){
    return (this->value | other.value);
}

unsigned int leon3_funclt_trap::Reg32_2::operator &( const Reg32_2 & other ) const \
    throw(){
    return (this->value & other.value);
}

unsigned int leon3_funclt_trap::Reg32_2::operator ^( const Reg32_2 & other ) const \
    throw(){
    return (this->value ^ other.value);
}

unsigned int leon3_funclt_trap::Reg32_2::operator <<( const Reg32_2 & other ) const \
    throw(){
    return (this->value << other.value);
}

unsigned int leon3_funclt_trap::Reg32_2::operator >>( const Reg32_2 & other ) const \
    throw(){
    return (this->value >> other.value);
}

bool leon3_funclt_trap::Reg32_2::operator <( const Reg32_2 & other ) const throw(){
    return (this->value < other.value);
}

bool leon3_funclt_trap::Reg32_2::operator >( const Reg32_2 & other ) const throw(){
    return (this->value > other.value);
}

bool leon3_funclt_trap::Reg32_2::operator <=( const Reg32_2 & other ) const throw(){
    return (this->value <= other.value);
}

bool leon3_funclt_trap::Reg32_2::operator >=( const Reg32_2 & other ) const throw(){
    return (this->value >= other.value);
}

bool leon3_funclt_trap::Reg32_2::operator ==( const Reg32_2 & other ) const throw(){
    return (this->value == other.value);
}

bool leon3_funclt_trap::Reg32_2::operator !=( const Reg32_2 & other ) const throw(){
    return (this->value != other.value);
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator =( const Reg32_2 & other ) throw(){
    this->value = other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator +=( const Reg32_2 & other ) throw(){
    this->value += other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator -=( const Reg32_2 & other ) throw(){
    this->value -= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator *=( const Reg32_2 & other ) throw(){
    this->value *= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator /=( const Reg32_2 & other ) throw(){
    this->value /= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator |=( const Reg32_2 & other ) throw(){
    this->value |= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator &=( const Reg32_2 & other ) throw(){
    this->value &= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator ^=( const Reg32_2 & other ) throw(){
    this->value ^= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator <<=( const Reg32_2 & other ) throw(){
    this->value <<= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator >>=( const Reg32_2 & other ) throw(){
    this->value >>= other;
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_2::operator +( const Register & other ) const \
    throw(){
    return (this->value + other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator -( const Register & other ) const \
    throw(){
    return (this->value - other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator *( const Register & other ) const \
    throw(){
    return (this->value * other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator /( const Register & other ) const \
    throw(){
    return (this->value / other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator |( const Register & other ) const \
    throw(){
    return (this->value | other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator &( const Register & other ) const \
    throw(){
    return (this->value & other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator ^( const Register & other ) const \
    throw(){
    return (this->value ^ other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator <<( const Register & other ) const \
    throw(){
    return (this->value << other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator >>( const Register & other ) const \
    throw(){
    return (this->value >> other);
}

bool leon3_funclt_trap::Reg32_2::operator <( const Register & other ) const throw(){
    return (this->value < other);
}

bool leon3_funclt_trap::Reg32_2::operator >( const Register & other ) const throw(){
    return (this->value > other);
}

bool leon3_funclt_trap::Reg32_2::operator <=( const Register & other ) const throw(){
    return (this->value <= other);
}

bool leon3_funclt_trap::Reg32_2::operator >=( const Register & other ) const throw(){
    return (this->value >= other);
}

bool leon3_funclt_trap::Reg32_2::operator ==( const Register & other ) const throw(){
    return (this->value == other);
}

bool leon3_funclt_trap::Reg32_2::operator !=( const Register & other ) const throw(){
    return (this->value != other);
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator =( const Register & other ) throw(){
    this->value = other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator +=( const Register & other ) throw(){
    this->value += other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator -=( const Register & other ) throw(){
    this->value -= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator *=( const Register & other ) throw(){
    this->value *= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator /=( const Register & other ) throw(){
    this->value /= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator |=( const Register & other ) throw(){
    this->value |= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator &=( const Register & other ) throw(){
    this->value &= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator ^=( const Register & other ) throw(){
    this->value ^= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator <<=( const Register & other ) throw(){
    this->value <<= other;
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator >>=( const Register & other ) throw(){
    this->value >>= other;
    return *this;
}

std::ostream & leon3_funclt_trap::Reg32_2::operator <<( std::ostream & stream ) const \
    throw(){
    stream << std::hex << std::showbase << this->value << std::dec;
    return stream;
}

leon3_funclt_trap::Reg32_2::Reg32_2() : field_TBA(this->value), field_TT(this->value){
    this->value = 0;
}

InnerField & leon3_funclt_trap::Reg32_3::InnerField_Empty::operator =( const unsigned \
    int & other ) throw(){
    return *this;
}

leon3_funclt_trap::Reg32_3::InnerField_Empty::InnerField_Empty(){

}

leon3_funclt_trap::Reg32_3::InnerField_Empty::~InnerField_Empty(){

}
void leon3_funclt_trap::Reg32_3::immediateWrite( const unsigned int & value ) throw(){
    this->value = value;
}

unsigned int leon3_funclt_trap::Reg32_3::readNewValue() throw(){
    return this->value;
}

unsigned int leon3_funclt_trap::Reg32_3::operator ~() throw(){
    return ~(this->value);
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator =( const unsigned int & other ) throw(){
    this->value = other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator +=( const unsigned int & other ) throw(){
    this->value += other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator -=( const unsigned int & other ) throw(){
    this->value -= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator *=( const unsigned int & other ) throw(){
    this->value *= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator /=( const unsigned int & other ) throw(){
    this->value /= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator |=( const unsigned int & other ) throw(){
    this->value |= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator &=( const unsigned int & other ) throw(){
    this->value &= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator ^=( const unsigned int & other ) throw(){
    this->value ^= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator <<=( const unsigned int & other ) throw(){
    this->value <<= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator >>=( const unsigned int & other ) throw(){
    this->value >>= other;
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_3::operator +( const Reg32_3 & other ) const \
    throw(){
    return (this->value + other.value);
}

unsigned int leon3_funclt_trap::Reg32_3::operator -( const Reg32_3 & other ) const \
    throw(){
    return (this->value - other.value);
}

unsigned int leon3_funclt_trap::Reg32_3::operator *( const Reg32_3 & other ) const \
    throw(){
    return (this->value * other.value);
}

unsigned int leon3_funclt_trap::Reg32_3::operator /( const Reg32_3 & other ) const \
    throw(){
    return (this->value / other.value);
}

unsigned int leon3_funclt_trap::Reg32_3::operator |( const Reg32_3 & other ) const \
    throw(){
    return (this->value | other.value);
}

unsigned int leon3_funclt_trap::Reg32_3::operator &( const Reg32_3 & other ) const \
    throw(){
    return (this->value & other.value);
}

unsigned int leon3_funclt_trap::Reg32_3::operator ^( const Reg32_3 & other ) const \
    throw(){
    return (this->value ^ other.value);
}

unsigned int leon3_funclt_trap::Reg32_3::operator <<( const Reg32_3 & other ) const \
    throw(){
    return (this->value << other.value);
}

unsigned int leon3_funclt_trap::Reg32_3::operator >>( const Reg32_3 & other ) const \
    throw(){
    return (this->value >> other.value);
}

bool leon3_funclt_trap::Reg32_3::operator <( const Reg32_3 & other ) const throw(){
    return (this->value < other.value);
}

bool leon3_funclt_trap::Reg32_3::operator >( const Reg32_3 & other ) const throw(){
    return (this->value > other.value);
}

bool leon3_funclt_trap::Reg32_3::operator <=( const Reg32_3 & other ) const throw(){
    return (this->value <= other.value);
}

bool leon3_funclt_trap::Reg32_3::operator >=( const Reg32_3 & other ) const throw(){
    return (this->value >= other.value);
}

bool leon3_funclt_trap::Reg32_3::operator ==( const Reg32_3 & other ) const throw(){
    return (this->value == other.value);
}

bool leon3_funclt_trap::Reg32_3::operator !=( const Reg32_3 & other ) const throw(){
    return (this->value != other.value);
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator =( const Reg32_3 & other ) throw(){
    this->value = other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator +=( const Reg32_3 & other ) throw(){
    this->value += other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator -=( const Reg32_3 & other ) throw(){
    this->value -= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator *=( const Reg32_3 & other ) throw(){
    this->value *= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator /=( const Reg32_3 & other ) throw(){
    this->value /= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator |=( const Reg32_3 & other ) throw(){
    this->value |= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator &=( const Reg32_3 & other ) throw(){
    this->value &= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator ^=( const Reg32_3 & other ) throw(){
    this->value ^= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator <<=( const Reg32_3 & other ) throw(){
    this->value <<= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator >>=( const Reg32_3 & other ) throw(){
    this->value >>= other;
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_3::operator +( const Register & other ) const \
    throw(){
    return (this->value + other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator -( const Register & other ) const \
    throw(){
    return (this->value - other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator *( const Register & other ) const \
    throw(){
    return (this->value * other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator /( const Register & other ) const \
    throw(){
    return (this->value / other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator |( const Register & other ) const \
    throw(){
    return (this->value | other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator &( const Register & other ) const \
    throw(){
    return (this->value & other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator ^( const Register & other ) const \
    throw(){
    return (this->value ^ other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator <<( const Register & other ) const \
    throw(){
    return (this->value << other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator >>( const Register & other ) const \
    throw(){
    return (this->value >> other);
}

bool leon3_funclt_trap::Reg32_3::operator <( const Register & other ) const throw(){
    return (this->value < other);
}

bool leon3_funclt_trap::Reg32_3::operator >( const Register & other ) const throw(){
    return (this->value > other);
}

bool leon3_funclt_trap::Reg32_3::operator <=( const Register & other ) const throw(){
    return (this->value <= other);
}

bool leon3_funclt_trap::Reg32_3::operator >=( const Register & other ) const throw(){
    return (this->value >= other);
}

bool leon3_funclt_trap::Reg32_3::operator ==( const Register & other ) const throw(){
    return (this->value == other);
}

bool leon3_funclt_trap::Reg32_3::operator !=( const Register & other ) const throw(){
    return (this->value != other);
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator =( const Register & other ) throw(){
    this->value = other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator +=( const Register & other ) throw(){
    this->value += other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator -=( const Register & other ) throw(){
    this->value -= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator *=( const Register & other ) throw(){
    this->value *= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator /=( const Register & other ) throw(){
    this->value /= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator |=( const Register & other ) throw(){
    this->value |= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator &=( const Register & other ) throw(){
    this->value &= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator ^=( const Register & other ) throw(){
    this->value ^= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator <<=( const Register & other ) throw(){
    this->value <<= other;
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator >>=( const Register & other ) throw(){
    this->value >>= other;
    return *this;
}

std::ostream & leon3_funclt_trap::Reg32_3::operator <<( std::ostream & stream ) const \
    throw(){
    stream << std::hex << std::showbase << this->value << std::dec;
    return stream;
}

leon3_funclt_trap::Reg32_3::Reg32_3(){
    this->value = 0;
}

InnerField & leon3_funclt_trap::Reg32_3_const_0::InnerField_Empty::operator =( const \
    unsigned int & other ) throw(){
    return *this;
}

leon3_funclt_trap::Reg32_3_const_0::InnerField_Empty::InnerField_Empty(){

}

leon3_funclt_trap::Reg32_3_const_0::InnerField_Empty::~InnerField_Empty(){

}
void leon3_funclt_trap::Reg32_3_const_0::immediateWrite( const unsigned int & value \
    ) throw(){

}

unsigned int leon3_funclt_trap::Reg32_3_const_0::readNewValue() throw(){
    return this->value;
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator ~() throw(){
    return ~(0);
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator =( const unsigned \
    int & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator +=( const unsigned \
    int & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator -=( const unsigned \
    int & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator *=( const unsigned \
    int & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator /=( const unsigned \
    int & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator |=( const unsigned \
    int & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator &=( const unsigned \
    int & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator ^=( const unsigned \
    int & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator <<=( const unsigned \
    int & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator >>=( const unsigned \
    int & other ) throw(){
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator +( const Reg32_3_const_0 \
    & other ) const throw(){
    return (0 + other.value);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator -( const Reg32_3_const_0 \
    & other ) const throw(){
    return (0 - other.value);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator *( const Reg32_3_const_0 \
    & other ) const throw(){
    return (0 * other.value);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator /( const Reg32_3_const_0 \
    & other ) const throw(){
    return (0 / other.value);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator |( const Reg32_3_const_0 \
    & other ) const throw(){
    return (0 | other.value);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator &( const Reg32_3_const_0 \
    & other ) const throw(){
    return (0 & other.value);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator ^( const Reg32_3_const_0 \
    & other ) const throw(){
    return (0 ^ other.value);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator <<( const Reg32_3_const_0 \
    & other ) const throw(){
    return (0 << other.value);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator >>( const Reg32_3_const_0 \
    & other ) const throw(){
    return (0 >> other.value);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator <( const Reg32_3_const_0 & other \
    ) const throw(){
    return (0 < other.value);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator >( const Reg32_3_const_0 & other \
    ) const throw(){
    return (0 > other.value);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator <=( const Reg32_3_const_0 & other \
    ) const throw(){
    return (0 <= other.value);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator >=( const Reg32_3_const_0 & other \
    ) const throw(){
    return (0 >= other.value);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator ==( const Reg32_3_const_0 & other \
    ) const throw(){
    return (0 == other.value);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator !=( const Reg32_3_const_0 & other \
    ) const throw(){
    return (0 != other.value);
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator =( const Reg32_3_const_0 \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator +=( const Reg32_3_const_0 \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator -=( const Reg32_3_const_0 \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator *=( const Reg32_3_const_0 \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator /=( const Reg32_3_const_0 \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator |=( const Reg32_3_const_0 \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator &=( const Reg32_3_const_0 \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator ^=( const Reg32_3_const_0 \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator <<=( const Reg32_3_const_0 \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator >>=( const Reg32_3_const_0 \
    & other ) throw(){
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator +( const Register & other \
    ) const throw(){
    return (0 + other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator -( const Register & other \
    ) const throw(){
    return (0 - other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator *( const Register & other \
    ) const throw(){
    return (0 * other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator /( const Register & other \
    ) const throw(){
    return (0 / other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator |( const Register & other \
    ) const throw(){
    return (0 | other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator &( const Register & other \
    ) const throw(){
    return (0 & other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator ^( const Register & other \
    ) const throw(){
    return (0 ^ other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator <<( const Register & other \
    ) const throw(){
    return (0 << other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator >>( const Register & other \
    ) const throw(){
    return (0 >> other);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator <( const Register & other ) const \
    throw(){
    return (0 < other);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator >( const Register & other ) const \
    throw(){
    return (0 > other);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator <=( const Register & other ) const \
    throw(){
    return (0 <= other);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator >=( const Register & other ) const \
    throw(){
    return (0 >= other);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator ==( const Register & other ) const \
    throw(){
    return (0 == other);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator !=( const Register & other ) const \
    throw(){
    return (0 != other);
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator =( const Register \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator +=( const Register \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator -=( const Register \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator *=( const Register \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator /=( const Register \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator |=( const Register \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator &=( const Register \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator ^=( const Register \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator <<=( const Register \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator >>=( const Register \
    & other ) throw(){
    return *this;
}

std::ostream & leon3_funclt_trap::Reg32_3_const_0::operator <<( std::ostream & stream \
    ) const throw(){
    stream << std::hex << std::showbase << 0 << std::dec;
    return stream;
}

leon3_funclt_trap::Reg32_3_const_0::Reg32_3_const_0(){
    this->value = 0;
}

void leon3_funclt_trap::RegisterBankClass::setNewRegister( unsigned int numReg, Register \
    * newReg ){
    if(numReg > this->size - 1){
        THROW_EXCEPTION("Register number " << numReg << " is out of register bank boundaries");
    }
    else{
        this->registers[numReg] = newReg;
    }
}

void leon3_funclt_trap::RegisterBankClass::setSize( unsigned int size ) throw(){

    for(unsigned int i = 0; i < this->size; i++){
        if(this->registers[i] != NULL){
            delete this->registers[i];
        }
    }
    if(this->registers != NULL){
        delete [] this->registers;
    }
    this->size = size;
    this->registers = new Register *[this->size];
    for(unsigned int i = 0; i < this->size; i++){
        this->registers[i] = NULL;
    }
}

leon3_funclt_trap::RegisterBankClass::RegisterBankClass( unsigned int size ){
    this->size = size;
    this->registers = new Register *[this->size];
    for(unsigned int i = 0; i < this->size; i++){
        this->registers[i] = NULL;
    }
}

leon3_funclt_trap::RegisterBankClass::RegisterBankClass(){
    this->size = 0;
    this->registers = NULL;
}

leon3_funclt_trap::RegisterBankClass::~RegisterBankClass(){

    for(unsigned int i = 0; i < this->size; i++){
        if(this->registers[i] != NULL){
            delete this->registers[i];
        }
    }
    if(this->registers != NULL){
        delete [] this->registers;
    }
}


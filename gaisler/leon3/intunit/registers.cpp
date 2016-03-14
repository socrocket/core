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



#include "gaisler/leon3/intunit/registers.hpp"
#include <ostream>
#include "core/common/trapgen/utils/trap_utils.hpp"

using namespace leon3_funclt_trap;
InnerField & leon3_funclt_trap::InnerField::operator =( const InnerField & other \
    ) throw(){
    *this = (unsigned int)other;
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::InnerField::~InnerField(){

}
void leon3_funclt_trap::Register::clockCycle() throw(){

}

leon3_funclt_trap::Register::Register() : sc_register<unsigned int, SC_REG_RW_ACCESS>(sc_core::sc_gen_unique_name("reg"), 0) {

}

leon3_funclt_trap::Register::Register(const char *name) : sc_register<unsigned int, SC_REG_RW_ACCESS>(name, 0) {

}

leon3_funclt_trap::Register::Register( const Register & other ) : sc_register<unsigned int, SC_REG_RW_ACCESS>(sc_core::sc_gen_unique_name("reg"), 0) {

}

InnerField & leon3_funclt_trap::Reg32_0::InnerField_VER::operator =( const unsigned int & other) throw() {
    this->m_cur_val &= 0xf0ffffffL;
    this->m_cur_val |= ((other & 0xf) << 24);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_VER::InnerField_VER( unsigned int & value, Register *reg) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_0::InnerField_VER::~InnerField_VER(){
}

InnerField & leon3_funclt_trap::Reg32_0::InnerField_ICC_z::operator =( const unsigned int & other) throw() {
    this->m_cur_val &= 0xffbfffffL;
    this->m_cur_val |= ((other & 0x1) << 22);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_ICC_z::InnerField_ICC_z( unsigned int & value, Register *reg) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_0::InnerField_ICC_z::~InnerField_ICC_z(){
}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_ICC_v::operator =( const unsigned  int & other) throw() {
    this->m_cur_val &= 0xffdfffffL;
    this->m_cur_val |= ((other & 0x1) << 21);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_ICC_v::InnerField_ICC_v( unsigned int & value, Register *reg) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_0::InnerField_ICC_v::~InnerField_ICC_v() {
}

InnerField & leon3_funclt_trap::Reg32_0::InnerField_EF::operator =( const unsigned int & other ) throw() {
    this->m_cur_val &= 0xffffefffL;
    this->m_cur_val |= ((other & 0x1) << 12);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_EF::InnerField_EF( unsigned int & value, Register *reg ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_0::InnerField_EF::~InnerField_EF() {
}

InnerField & leon3_funclt_trap::Reg32_0::InnerField_EC::operator =( const unsigned  int & other) throw() {
    this->m_cur_val &= 0xffffdfffL;
    this->m_cur_val |= ((other & 0x1) << 13);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_EC::InnerField_EC( unsigned int & value, Register *reg) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_0::InnerField_EC::~InnerField_EC() {
}

InnerField & leon3_funclt_trap::Reg32_0::InnerField_ICC_n::operator =( const unsigned int & other) throw() {
    this->m_cur_val &= 0xff7fffffL;
    this->m_cur_val |= ((other & 0x1) << 23);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_ICC_n::InnerField_ICC_n( unsigned int & value, Register *reg) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_0::InnerField_ICC_n::~InnerField_ICC_n(){

}
InnerField & leon3_funclt_trap::Reg32_0::InnerField_S::operator =( const unsigned int & other ) throw() {
    this->m_cur_val &= 0xffffff7fL;
    this->m_cur_val |= ((other & 0x1) << 7);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_S::InnerField_S( unsigned int & value, Register *reg ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_0::InnerField_S::~InnerField_S() {
}

InnerField & leon3_funclt_trap::Reg32_0::InnerField_ET::operator =( const unsigned int & other ) throw() {
    this->m_cur_val &= 0xffffffdfL;
    this->m_cur_val |= ((other & 0x1) << 5);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_ET::InnerField_ET( unsigned int & value, Register *reg ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_0::InnerField_ET::~InnerField_ET() {
}

InnerField & leon3_funclt_trap::Reg32_0::InnerField_ICC_c::operator =( const unsigned int & other ) throw() {
    this->m_cur_val &= 0xffefffffL;
    this->m_cur_val |= ((other & 0x1) << 20);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_ICC_c::InnerField_ICC_c( unsigned int & value, Register *reg) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_0::InnerField_ICC_c::~InnerField_ICC_c() {
}

InnerField & leon3_funclt_trap::Reg32_0::InnerField_PS::operator =( const unsigned int & other ) throw() {
    this->m_cur_val &= 0xffffffbfL;
    this->m_cur_val |= ((other & 0x1) << 6);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_PS::InnerField_PS( unsigned int & value, Register *reg ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_0::InnerField_PS::~InnerField_PS() {
}

InnerField & leon3_funclt_trap::Reg32_0::InnerField_PIL::operator =( const unsigned int & other) throw() {
    this->m_cur_val &= 0xfffff0ffL;
    this->m_cur_val |= ((other & 0xf) << 8);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_PIL::InnerField_PIL( unsigned int & value, Register *reg) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_0::InnerField_PIL::~InnerField_PIL() {
}

InnerField & leon3_funclt_trap::Reg32_0::InnerField_CWP::operator =( const unsigned int & other ) throw() {
    this->m_cur_val &= 0xffffffe0L;
    this->m_cur_val |= other;
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_CWP::InnerField_CWP( unsigned int & value, Register *reg) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_0::InnerField_CWP::~InnerField_CWP() {
}

InnerField & leon3_funclt_trap::Reg32_0::InnerField_IMPL::operator =( const unsigned int & other ) throw() {
    this->m_cur_val &= 0xfffffff;
    this->m_cur_val |= ((other & 0xf) << 28);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_IMPL::InnerField_IMPL( unsigned int & value, Register *reg) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_0::InnerField_IMPL::~InnerField_IMPL() {
}

InnerField & leon3_funclt_trap::Reg32_0::InnerField_Empty::operator =( const unsigned int & other ) throw() {
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_0::InnerField_Empty::InnerField_Empty() {
}

leon3_funclt_trap::Reg32_0::InnerField_Empty::~InnerField_Empty() {
}

void leon3_funclt_trap::Reg32_0::immediateWrite( const unsigned int & value ) throw(){
    this->m_cur_val = value;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
}

unsigned int leon3_funclt_trap::Reg32_0::readNewValue() throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return this->m_cur_val;
}

unsigned int leon3_funclt_trap::Reg32_0::operator ~() throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return ~(this->m_cur_val);
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator =( const unsigned int & other ) throw(){
    this->m_cur_val = other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator +=( const unsigned int & other ) throw(){
    this->m_cur_val += other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator -=( const unsigned int & other ) throw(){
    this->m_cur_val -= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator *=( const unsigned int & other ) throw(){
    this->m_cur_val *= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator /=( const unsigned int & other ) throw(){
    this->m_cur_val /= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator |=( const unsigned int & other ) throw(){
    this->m_cur_val |= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator &=( const unsigned int & other ) throw(){
    this->m_cur_val &= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator ^=( const unsigned int & other ) throw(){
    this->m_cur_val ^= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator <<=( const unsigned int & other ) throw(){
    this->m_cur_val <<= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator >>=( const unsigned int & other ) throw(){
    this->m_cur_val >>= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_0::operator +( const Reg32_0 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val + other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_0::operator -( const Reg32_0 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val - other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_0::operator *( const Reg32_0 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val * other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_0::operator /( const Reg32_0 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val / other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_0::operator |( const Reg32_0 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val | other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_0::operator &( const Reg32_0 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val & other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_0::operator ^( const Reg32_0 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val ^ other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_0::operator <<( const Reg32_0 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val << other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_0::operator >>( const Reg32_0 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >> other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_0::operator <( const Reg32_0 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val < other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_0::operator >( const Reg32_0 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val > other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_0::operator <=( const Reg32_0 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val <= other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_0::operator >=( const Reg32_0 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >= other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_0::operator ==( const Reg32_0 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val == other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_0::operator !=( const Reg32_0 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val != other.m_cur_val);
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator =( const Reg32_0 & other ) throw(){
    this->m_cur_val = other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator +=( const Reg32_0 & other ) throw(){
    this->m_cur_val += other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator -=( const Reg32_0 & other ) throw(){
    this->m_cur_val -= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator *=( const Reg32_0 & other ) throw(){
    this->m_cur_val *= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator /=( const Reg32_0 & other ) throw(){
    this->m_cur_val /= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator |=( const Reg32_0 & other ) throw(){
    this->m_cur_val |= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator &=( const Reg32_0 & other ) throw(){
    this->m_cur_val &= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator ^=( const Reg32_0 & other ) throw(){
    this->m_cur_val ^= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator <<=( const Reg32_0 & other ) throw(){
    this->m_cur_val <<= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator >>=( const Reg32_0 & other ) throw(){
    this->m_cur_val >>= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_0::operator +( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val + other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator -( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val - other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator *( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val * other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator /( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val / other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator |( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val | other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator &( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val & other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator ^( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val ^ other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator <<( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val << other);
}

unsigned int leon3_funclt_trap::Reg32_0::operator >>( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >> other);
}

bool leon3_funclt_trap::Reg32_0::operator <( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val < other);
}

bool leon3_funclt_trap::Reg32_0::operator >( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val > other);
}

bool leon3_funclt_trap::Reg32_0::operator <=( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val <= other);
}

bool leon3_funclt_trap::Reg32_0::operator >=( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >= other);
}

bool leon3_funclt_trap::Reg32_0::operator ==( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val == other);
}

bool leon3_funclt_trap::Reg32_0::operator !=( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val != other);
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator =( const Register & other ) throw(){
    this->m_cur_val = other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator +=( const Register & other ) throw(){
    this->m_cur_val += other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator -=( const Register & other ) throw(){
    this->m_cur_val -= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator *=( const Register & other ) throw(){
    this->m_cur_val *= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator /=( const Register & other ) throw(){
    this->m_cur_val /= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator |=( const Register & other ) throw(){
    this->m_cur_val |= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator &=( const Register & other ) throw(){
    this->m_cur_val &= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator ^=( const Register & other ) throw(){
    this->m_cur_val ^= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator <<=( const Register & other ) throw(){
    this->m_cur_val <<= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_0 & leon3_funclt_trap::Reg32_0::operator >>=( const Register & other ) throw(){
    this->m_cur_val >>= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

std::ostream & leon3_funclt_trap::Reg32_0::operator <<( std::ostream & stream ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    stream << std::hex << std::showbase << this->m_cur_val << std::dec;
    return stream;
}

leon3_funclt_trap::Reg32_0::Reg32_0() : field_VER(this->m_cur_val, this), field_ICC_z(this->m_cur_val, this), \
    field_ICC_v(this->m_cur_val, this), field_EF(this->m_cur_val, this), field_EC(this->m_cur_val, this), field_ICC_n(this->m_cur_val, this), \
    field_S(this->m_cur_val, this), field_ET(this->m_cur_val, this), field_ICC_c(this->m_cur_val, this), field_PS(this->m_cur_val, this), \
    field_PIL(this->m_cur_val, this), field_CWP(this->m_cur_val, this), field_IMPL(this->m_cur_val, this){
    this->m_cur_val = 0;
}

leon3_funclt_trap::Reg32_0::Reg32_0(const char *name) : Register(name), field_VER(this->m_cur_val, this), field_ICC_z(this->m_cur_val, this), \
    field_ICC_v(this->m_cur_val, this), field_EF(this->m_cur_val, this), field_EC(this->m_cur_val, this), field_ICC_n(this->m_cur_val, this), \
    field_S(this->m_cur_val, this), field_ET(this->m_cur_val, this), field_ICC_c(this->m_cur_val, this), field_PS(this->m_cur_val, this), \
    field_PIL(this->m_cur_val, this), field_CWP(this->m_cur_val, this), field_IMPL(this->m_cur_val, this){
    this->m_cur_val = 0;
}

InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_28::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xefffffffL;
    this->m_cur_val |= ((other & 0x1) << 28);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_28::InnerField_WIM_28( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_28::~InnerField_WIM_28(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_29::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xdfffffffL;
    this->m_cur_val |= ((other & 0x1) << 29);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_29::InnerField_WIM_29( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_29::~InnerField_WIM_29(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_24::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfeffffffL;
    this->m_cur_val |= ((other & 0x1) << 24);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_24::InnerField_WIM_24( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_24::~InnerField_WIM_24(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_25::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfdffffffL;
    this->m_cur_val |= ((other & 0x1) << 25);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_25::InnerField_WIM_25( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_25::~InnerField_WIM_25(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_26::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfbffffffL;
    this->m_cur_val |= ((other & 0x1) << 26);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_26::InnerField_WIM_26( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_26::~InnerField_WIM_26(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_27::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xf7ffffffL;
    this->m_cur_val |= ((other & 0x1) << 27);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_27::InnerField_WIM_27( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_27::~InnerField_WIM_27(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_20::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xffefffffL;
    this->m_cur_val |= ((other & 0x1) << 20);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_20::InnerField_WIM_20( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_20::~InnerField_WIM_20(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_21::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xffdfffffL;
    this->m_cur_val |= ((other & 0x1) << 21);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_21::InnerField_WIM_21( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_21::~InnerField_WIM_21(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_22::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xffbfffffL;
    this->m_cur_val |= ((other & 0x1) << 22);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_22::InnerField_WIM_22( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_22::~InnerField_WIM_22(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_23::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xff7fffffL;
    this->m_cur_val |= ((other & 0x1) << 23);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_23::InnerField_WIM_23( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_23::~InnerField_WIM_23(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_9::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfffffdffL;
    this->m_cur_val |= ((other & 0x1) << 9);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_9::InnerField_WIM_9( unsigned int & value, Register *reg \
    ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_9::~InnerField_WIM_9(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_8::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfffffeffL;
    this->m_cur_val |= ((other & 0x1) << 8);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_8::InnerField_WIM_8( unsigned int & value, Register *reg \
    ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_8::~InnerField_WIM_8(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_1::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfffffffdL;
    this->m_cur_val |= ((other & 0x1) << 1);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_1::InnerField_WIM_1( unsigned int & value, Register *reg \
    ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_1::~InnerField_WIM_1(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_0::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfffffffeL;
    this->m_cur_val |= other;
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_0::InnerField_WIM_0( unsigned int & value, Register *reg \
    ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_0::~InnerField_WIM_0(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_3::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfffffff7L;
    this->m_cur_val |= ((other & 0x1) << 3);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_3::InnerField_WIM_3( unsigned int & value, Register *reg \
    ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_3::~InnerField_WIM_3(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_2::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfffffffbL;
    this->m_cur_val |= ((other & 0x1) << 2);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_2::InnerField_WIM_2( unsigned int & value, Register *reg \
    ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_2::~InnerField_WIM_2(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_5::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xffffffdfL;
    this->m_cur_val |= ((other & 0x1) << 5);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_5::InnerField_WIM_5( unsigned int & value, Register *reg \
    ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_5::~InnerField_WIM_5(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_4::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xffffffefL;
    this->m_cur_val |= ((other & 0x1) << 4);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_4::InnerField_WIM_4( unsigned int & value, Register *reg \
    ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_4::~InnerField_WIM_4(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_7::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xffffff7fL;
    this->m_cur_val |= ((other & 0x1) << 7);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_7::InnerField_WIM_7( unsigned int & value, Register *reg \
    ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_7::~InnerField_WIM_7(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_6::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xffffffbfL;
    this->m_cur_val |= ((other & 0x1) << 6);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_6::InnerField_WIM_6( unsigned int & value, Register *reg \
    ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_6::~InnerField_WIM_6(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_11::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfffff7ffL;
    this->m_cur_val |= ((other & 0x1) << 11);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_11::InnerField_WIM_11( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_11::~InnerField_WIM_11(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_10::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfffffbffL;
    this->m_cur_val |= ((other & 0x1) << 10);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_10::InnerField_WIM_10( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_10::~InnerField_WIM_10(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_13::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xffffdfffL;
    this->m_cur_val |= ((other & 0x1) << 13);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_13::InnerField_WIM_13( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_13::~InnerField_WIM_13(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_12::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xffffefffL;
    this->m_cur_val |= ((other & 0x1) << 12);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_12::InnerField_WIM_12( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_12::~InnerField_WIM_12(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_15::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xffff7fffL;
    this->m_cur_val |= ((other & 0x1) << 15);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_15::InnerField_WIM_15( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_15::~InnerField_WIM_15(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_14::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xffffbfffL;
    this->m_cur_val |= ((other & 0x1) << 14);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_14::InnerField_WIM_14( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_14::~InnerField_WIM_14(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_17::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfffdffffL;
    this->m_cur_val |= ((other & 0x1) << 17);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_17::InnerField_WIM_17( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_17::~InnerField_WIM_17(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_16::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfffeffffL;
    this->m_cur_val |= ((other & 0x1) << 16);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_16::InnerField_WIM_16( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_16::~InnerField_WIM_16(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_19::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfff7ffffL;
    this->m_cur_val |= ((other & 0x1) << 19);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_19::InnerField_WIM_19( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_19::~InnerField_WIM_19(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_18::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfffbffffL;
    this->m_cur_val |= ((other & 0x1) << 18);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_18::InnerField_WIM_18( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_18::~InnerField_WIM_18(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_31::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0x7fffffff;
    this->m_cur_val |= ((other & 0x1) << 31);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_31::InnerField_WIM_31( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_31::~InnerField_WIM_31(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_WIM_30::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xbfffffffL;
    this->m_cur_val |= ((other & 0x1) << 30);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_WIM_30::InnerField_WIM_30( unsigned int & \
    value, Register *reg) : m_cur_val(value) {
    parent = reg;

}

leon3_funclt_trap::Reg32_1::InnerField_WIM_30::~InnerField_WIM_30(){

}
InnerField & leon3_funclt_trap::Reg32_1::InnerField_Empty::operator =( const unsigned \
    int & other ) throw(){
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_1::InnerField_Empty::InnerField_Empty(){

}

leon3_funclt_trap::Reg32_1::InnerField_Empty::~InnerField_Empty(){

}
void leon3_funclt_trap::Reg32_1::immediateWrite( const unsigned int & value ) throw(){
    this->m_cur_val = value;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
}

unsigned int leon3_funclt_trap::Reg32_1::readNewValue() throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return this->m_cur_val;
}

unsigned int leon3_funclt_trap::Reg32_1::operator ~() throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return ~(this->m_cur_val);
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator =( const unsigned int & other ) throw(){
    this->m_cur_val = other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator +=( const unsigned int & other ) throw(){
    this->m_cur_val += other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator -=( const unsigned int & other ) throw(){
    this->m_cur_val -= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator *=( const unsigned int & other ) throw(){
    this->m_cur_val *= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator /=( const unsigned int & other ) throw(){
    this->m_cur_val /= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator |=( const unsigned int & other ) throw(){
    this->m_cur_val |= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator &=( const unsigned int & other ) throw(){
    this->m_cur_val &= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator ^=( const unsigned int & other ) throw(){
    this->m_cur_val ^= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator <<=( const unsigned int & other ) throw(){
    this->m_cur_val <<= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator >>=( const unsigned int & other ) throw(){
    this->m_cur_val >>= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_1::operator +( const Reg32_1 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val + other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_1::operator -( const Reg32_1 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val - other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_1::operator *( const Reg32_1 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val * other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_1::operator /( const Reg32_1 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val / other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_1::operator |( const Reg32_1 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val | other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_1::operator &( const Reg32_1 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val & other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_1::operator ^( const Reg32_1 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val ^ other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_1::operator <<( const Reg32_1 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val << other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_1::operator >>( const Reg32_1 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >> other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_1::operator <( const Reg32_1 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val < other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_1::operator >( const Reg32_1 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val > other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_1::operator <=( const Reg32_1 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val <= other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_1::operator >=( const Reg32_1 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >= other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_1::operator ==( const Reg32_1 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val == other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_1::operator !=( const Reg32_1 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val != other.m_cur_val);
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator =( const Reg32_1 & other ) throw(){
    this->m_cur_val = other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator +=( const Reg32_1 & other ) throw(){
    this->m_cur_val += other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator -=( const Reg32_1 & other ) throw(){
    this->m_cur_val -= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator *=( const Reg32_1 & other ) throw(){
    this->m_cur_val *= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator /=( const Reg32_1 & other ) throw(){
    this->m_cur_val /= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator |=( const Reg32_1 & other ) throw(){
    this->m_cur_val |= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator &=( const Reg32_1 & other ) throw(){
    this->m_cur_val &= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator ^=( const Reg32_1 & other ) throw(){
    this->m_cur_val ^= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator <<=( const Reg32_1 & other ) throw(){
    this->m_cur_val <<= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator >>=( const Reg32_1 & other ) throw(){
    this->m_cur_val >>= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_1::operator +( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val + other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator -( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val - other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator *( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val * other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator /( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val / other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator |( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val | other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator &( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val & other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator ^( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val ^ other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator <<( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val << other);
}

unsigned int leon3_funclt_trap::Reg32_1::operator >>( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >> other);
}

bool leon3_funclt_trap::Reg32_1::operator <( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val < other);
}

bool leon3_funclt_trap::Reg32_1::operator >( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val > other);
}

bool leon3_funclt_trap::Reg32_1::operator <=( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val <= other);
}

bool leon3_funclt_trap::Reg32_1::operator >=( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >= other);
}

bool leon3_funclt_trap::Reg32_1::operator ==( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val == other);
}

bool leon3_funclt_trap::Reg32_1::operator !=( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val != other);
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator =( const Register & other ) throw(){
    this->m_cur_val = other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator +=( const Register & other ) throw(){
    this->m_cur_val += other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator -=( const Register & other ) throw(){
    this->m_cur_val -= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator *=( const Register & other ) throw(){
    this->m_cur_val *= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator /=( const Register & other ) throw(){
    this->m_cur_val /= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator |=( const Register & other ) throw(){
    this->m_cur_val |= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator &=( const Register & other ) throw(){
    this->m_cur_val &= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator ^=( const Register & other ) throw(){
    this->m_cur_val ^= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator <<=( const Register & other ) throw(){
    this->m_cur_val <<= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_1 & leon3_funclt_trap::Reg32_1::operator >>=( const Register & other ) throw(){
    this->m_cur_val >>= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

std::ostream & leon3_funclt_trap::Reg32_1::operator <<( std::ostream & stream ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    stream << std::hex << std::showbase << this->m_cur_val << std::dec;
    return stream;
}

leon3_funclt_trap::Reg32_1::Reg32_1() : field_WIM_28(this->m_cur_val, this), field_WIM_29(this->m_cur_val, this), \
    field_WIM_24(this->m_cur_val, this), field_WIM_25(this->m_cur_val, this), field_WIM_26(this->m_cur_val, this), \
    field_WIM_27(this->m_cur_val, this), field_WIM_20(this->m_cur_val, this), field_WIM_21(this->m_cur_val, this), \
    field_WIM_22(this->m_cur_val, this), field_WIM_23(this->m_cur_val, this), field_WIM_9(this->m_cur_val, this), field_WIM_8(this->m_cur_val, this), \
    field_WIM_1(this->m_cur_val, this), field_WIM_0(this->m_cur_val, this), field_WIM_3(this->m_cur_val, this), field_WIM_2(this->m_cur_val, this), \
    field_WIM_5(this->m_cur_val, this), field_WIM_4(this->m_cur_val, this), field_WIM_7(this->m_cur_val, this), field_WIM_6(this->m_cur_val, this), \
    field_WIM_11(this->m_cur_val, this), field_WIM_10(this->m_cur_val, this), field_WIM_13(this->m_cur_val, this), \
    field_WIM_12(this->m_cur_val, this), field_WIM_15(this->m_cur_val, this), field_WIM_14(this->m_cur_val, this), \
    field_WIM_17(this->m_cur_val, this), field_WIM_16(this->m_cur_val, this), field_WIM_19(this->m_cur_val, this), \
    field_WIM_18(this->m_cur_val, this), field_WIM_31(this->m_cur_val, this), field_WIM_30(this->m_cur_val, this){
    this->m_cur_val = 0;
}

leon3_funclt_trap::Reg32_1::Reg32_1(const char *name) : Register(name), field_WIM_28(this->m_cur_val, this), field_WIM_29(this->m_cur_val, this), \
    field_WIM_24(this->m_cur_val, this), field_WIM_25(this->m_cur_val, this), field_WIM_26(this->m_cur_val, this), \
    field_WIM_27(this->m_cur_val, this), field_WIM_20(this->m_cur_val, this), field_WIM_21(this->m_cur_val, this), \
    field_WIM_22(this->m_cur_val, this), field_WIM_23(this->m_cur_val, this), field_WIM_9(this->m_cur_val, this), field_WIM_8(this->m_cur_val, this), \
    field_WIM_1(this->m_cur_val, this), field_WIM_0(this->m_cur_val, this), field_WIM_3(this->m_cur_val, this), field_WIM_2(this->m_cur_val, this), \
    field_WIM_5(this->m_cur_val, this), field_WIM_4(this->m_cur_val, this), field_WIM_7(this->m_cur_val, this), field_WIM_6(this->m_cur_val, this), \
    field_WIM_11(this->m_cur_val, this), field_WIM_10(this->m_cur_val, this), field_WIM_13(this->m_cur_val, this), \
    field_WIM_12(this->m_cur_val, this), field_WIM_15(this->m_cur_val, this), field_WIM_14(this->m_cur_val, this), \
    field_WIM_17(this->m_cur_val, this), field_WIM_16(this->m_cur_val, this), field_WIM_19(this->m_cur_val, this), \
    field_WIM_18(this->m_cur_val, this), field_WIM_31(this->m_cur_val, this), field_WIM_30(this->m_cur_val, this){
    this->m_cur_val = 0;
}

InnerField & leon3_funclt_trap::Reg32_2::InnerField_TBA::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfff;
    this->m_cur_val |= ((other & 0xfffff) << 12);
    return *this;
}

leon3_funclt_trap::Reg32_2::InnerField_TBA::InnerField_TBA( unsigned int & value, Register *reg \
    ) : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_2::InnerField_TBA::~InnerField_TBA(){

}
InnerField & leon3_funclt_trap::Reg32_2::InnerField_TT::operator =( const unsigned \
    int & other ) throw(){
    this->m_cur_val &= 0xfffff00fL;
    this->m_cur_val |= ((other & 0xff) << 4);
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_2::InnerField_TT::InnerField_TT( unsigned int & value, Register *reg ) \
    : m_cur_val(value) {
    parent = reg;
}

leon3_funclt_trap::Reg32_2::InnerField_TT::~InnerField_TT(){

}
InnerField & leon3_funclt_trap::Reg32_2::InnerField_Empty::operator =( const unsigned \
    int & other ) throw(){
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_2::InnerField_Empty::InnerField_Empty(){

}

leon3_funclt_trap::Reg32_2::InnerField_Empty::~InnerField_Empty(){

}
void leon3_funclt_trap::Reg32_2::immediateWrite( const unsigned int & value ) throw(){
    this->m_cur_val = value;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
}

unsigned int leon3_funclt_trap::Reg32_2::readNewValue() throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return this->m_cur_val;
}

unsigned int leon3_funclt_trap::Reg32_2::operator ~() throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return ~(this->m_cur_val);
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator =( const unsigned int & other ) throw(){
    this->m_cur_val = other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator +=( const unsigned int & other ) throw(){
    this->m_cur_val += other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator -=( const unsigned int & other ) throw(){
    this->m_cur_val -= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator *=( const unsigned int & other ) throw(){
    this->m_cur_val *= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator /=( const unsigned int & other ) throw(){
    this->m_cur_val /= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator |=( const unsigned int & other ) throw(){
    this->m_cur_val |= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator &=( const unsigned int & other ) throw(){
    this->m_cur_val &= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator ^=( const unsigned int & other ) throw(){
    this->m_cur_val ^= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator <<=( const unsigned int & other ) throw(){
    this->m_cur_val <<= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator >>=( const unsigned int & other ) throw(){
    this->m_cur_val >>= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_2::operator +( const Reg32_2 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val + other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_2::operator -( const Reg32_2 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val - other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_2::operator *( const Reg32_2 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val * other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_2::operator /( const Reg32_2 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val / other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_2::operator |( const Reg32_2 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val | other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_2::operator &( const Reg32_2 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val & other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_2::operator ^( const Reg32_2 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val ^ other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_2::operator <<( const Reg32_2 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val << other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_2::operator >>( const Reg32_2 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >> other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_2::operator <( const Reg32_2 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val < other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_2::operator >( const Reg32_2 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val > other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_2::operator <=( const Reg32_2 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val <= other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_2::operator >=( const Reg32_2 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >= other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_2::operator ==( const Reg32_2 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val == other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_2::operator !=( const Reg32_2 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val != other.m_cur_val);
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator =( const Reg32_2 & other ) throw(){
    this->m_cur_val = other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator +=( const Reg32_2 & other ) throw(){
    this->m_cur_val += other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator -=( const Reg32_2 & other ) throw(){
    this->m_cur_val -= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator *=( const Reg32_2 & other ) throw(){
    this->m_cur_val *= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator /=( const Reg32_2 & other ) throw(){
    this->m_cur_val /= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator |=( const Reg32_2 & other ) throw(){
    this->m_cur_val |= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator &=( const Reg32_2 & other ) throw(){
    this->m_cur_val &= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator ^=( const Reg32_2 & other ) throw(){
    this->m_cur_val ^= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator <<=( const Reg32_2 & other ) throw(){
    this->m_cur_val <<= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator >>=( const Reg32_2 & other ) throw(){
    this->m_cur_val >>= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_2::operator +( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val + other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator -( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val - other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator *( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val * other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator /( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val / other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator |( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val | other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator &( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val & other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator ^( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val ^ other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator <<( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val << other);
}

unsigned int leon3_funclt_trap::Reg32_2::operator >>( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >> other);
}

bool leon3_funclt_trap::Reg32_2::operator <( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val < other);
}

bool leon3_funclt_trap::Reg32_2::operator >( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val > other);
}

bool leon3_funclt_trap::Reg32_2::operator <=( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val <= other);
}

bool leon3_funclt_trap::Reg32_2::operator >=( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >= other);
}

bool leon3_funclt_trap::Reg32_2::operator ==( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val == other);
}

bool leon3_funclt_trap::Reg32_2::operator !=( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val != other);
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator =( const Register & other ) throw(){
    this->m_cur_val = other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator +=( const Register & other ) throw(){
    this->m_cur_val += other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator -=( const Register & other ) throw(){
    this->m_cur_val -= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator *=( const Register & other ) throw(){
    this->m_cur_val *= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator /=( const Register & other ) throw(){
    this->m_cur_val /= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator |=( const Register & other ) throw(){
    this->m_cur_val |= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator &=( const Register & other ) throw(){
    this->m_cur_val &= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator ^=( const Register & other ) throw(){
    this->m_cur_val ^= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator <<=( const Register & other ) throw(){
    this->m_cur_val <<= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_2 & leon3_funclt_trap::Reg32_2::operator >>=( const Register & other ) throw(){
    this->m_cur_val >>= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

std::ostream & leon3_funclt_trap::Reg32_2::operator <<( std::ostream & stream ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    stream << std::hex << std::showbase << this->m_cur_val << std::dec;
    return stream;
}

leon3_funclt_trap::Reg32_2::Reg32_2() : field_TBA(this->m_cur_val, this), field_TT(this->m_cur_val, this){
    this->m_cur_val = 0;
}

leon3_funclt_trap::Reg32_2::Reg32_2(const char *name) : Register(name), field_TBA(this->m_cur_val, this), field_TT(this->m_cur_val, this){
    this->m_cur_val = 0;
}

InnerField & leon3_funclt_trap::Reg32_3::InnerField_Empty::operator =( const unsigned \
    int & other ) throw(){
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

leon3_funclt_trap::Reg32_3::InnerField_Empty::InnerField_Empty(){

}

leon3_funclt_trap::Reg32_3::InnerField_Empty::~InnerField_Empty(){

}
void leon3_funclt_trap::Reg32_3::immediateWrite( const unsigned int & value ) throw(){
    this->m_cur_val = value;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
}

unsigned int leon3_funclt_trap::Reg32_3::readNewValue() throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return this->m_cur_val;
}

unsigned int leon3_funclt_trap::Reg32_3::operator ~() throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return ~(this->m_cur_val);
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator =( const unsigned int & other ) throw(){
    this->m_cur_val = other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator +=( const unsigned int & other ) throw(){
    this->m_cur_val += other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator -=( const unsigned int & other ) throw(){
    this->m_cur_val -= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator *=( const unsigned int & other ) throw(){
    this->m_cur_val *= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator /=( const unsigned int & other ) throw(){
    this->m_cur_val /= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator |=( const unsigned int & other ) throw(){
    this->m_cur_val |= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator &=( const unsigned int & other ) throw(){
    this->m_cur_val &= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator ^=( const unsigned int & other ) throw(){
    this->m_cur_val ^= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator <<=( const unsigned int & other ) throw(){
    this->m_cur_val <<= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator >>=( const unsigned int & other ) throw(){
    this->m_cur_val >>= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_3::operator +( const Reg32_3 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val + other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3::operator -( const Reg32_3 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val - other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3::operator *( const Reg32_3 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val * other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3::operator /( const Reg32_3 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val / other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3::operator |( const Reg32_3 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val | other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3::operator &( const Reg32_3 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val & other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3::operator ^( const Reg32_3 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val ^ other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3::operator <<( const Reg32_3 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val << other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3::operator >>( const Reg32_3 & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >> other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_3::operator <( const Reg32_3 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val < other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_3::operator >( const Reg32_3 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val > other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_3::operator <=( const Reg32_3 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val <= other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_3::operator >=( const Reg32_3 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >= other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_3::operator ==( const Reg32_3 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val == other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_3::operator !=( const Reg32_3 & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val != other.m_cur_val);
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator =( const Reg32_3 & other ) throw(){
    this->m_cur_val = other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator +=( const Reg32_3 & other ) throw(){
    this->m_cur_val += other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator -=( const Reg32_3 & other ) throw(){
    this->m_cur_val -= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator *=( const Reg32_3 & other ) throw(){
    this->m_cur_val *= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator /=( const Reg32_3 & other ) throw(){
    this->m_cur_val /= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator |=( const Reg32_3 & other ) throw(){
    this->m_cur_val |= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator &=( const Reg32_3 & other ) throw(){
    this->m_cur_val &= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator ^=( const Reg32_3 & other ) throw(){
    this->m_cur_val ^= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator <<=( const Reg32_3 & other ) throw(){
    this->m_cur_val <<= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator >>=( const Reg32_3 & other ) throw(){
    this->m_cur_val >>= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_3::operator +( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val + other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator -( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val - other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator *( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val * other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator /( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val / other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator |( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val | other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator &( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val & other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator ^( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val ^ other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator <<( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val << other);
}

unsigned int leon3_funclt_trap::Reg32_3::operator >>( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >> other);
}

bool leon3_funclt_trap::Reg32_3::operator <( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val < other);
}

bool leon3_funclt_trap::Reg32_3::operator >( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val > other);
}

bool leon3_funclt_trap::Reg32_3::operator <=( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val <= other);
}

bool leon3_funclt_trap::Reg32_3::operator >=( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val >= other);
}

bool leon3_funclt_trap::Reg32_3::operator ==( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val == other);
}

bool leon3_funclt_trap::Reg32_3::operator !=( const Register & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (this->m_cur_val != other);
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator =( const Register & other ) throw(){
    this->m_cur_val = other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator +=( const Register & other ) throw(){
    this->m_cur_val += other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator -=( const Register & other ) throw(){
    this->m_cur_val -= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator *=( const Register & other ) throw(){
    this->m_cur_val *= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator /=( const Register & other ) throw(){
    this->m_cur_val /= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator |=( const Register & other ) throw(){
    this->m_cur_val |= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator &=( const Register & other ) throw(){
    this->m_cur_val &= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator ^=( const Register & other ) throw(){
    this->m_cur_val ^= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator <<=( const Register & other ) throw(){
    this->m_cur_val <<= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3 & leon3_funclt_trap::Reg32_3::operator >>=( const Register & other ) throw(){
    this->m_cur_val >>= other;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

std::ostream & leon3_funclt_trap::Reg32_3::operator <<( std::ostream & stream ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    stream << std::hex << std::showbase << this->m_cur_val << std::dec;
    return stream;
}

leon3_funclt_trap::Reg32_3::Reg32_3() : Register(){
    this->m_cur_val = 0;
}

leon3_funclt_trap::Reg32_3::Reg32_3(const char *name) : Register(name){
    this->m_cur_val = 0;
}

InnerField & leon3_funclt_trap::Reg32_3_const_0::InnerField_Empty::operator =( const \
    unsigned int & other ) throw(){
    parent->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
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
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return this->m_cur_val;
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
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 + other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator -( const Reg32_3_const_0 \
    & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 - other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator *( const Reg32_3_const_0 \
    & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 * other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator /( const Reg32_3_const_0 \
    & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 / other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator |( const Reg32_3_const_0 \
    & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 | other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator &( const Reg32_3_const_0 \
    & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 & other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator ^( const Reg32_3_const_0 \
    & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 ^ other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator <<( const Reg32_3_const_0 \
    & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 << other.m_cur_val);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator >>( const Reg32_3_const_0 \
    & other ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 >> other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator <( const Reg32_3_const_0 & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 < other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator >( const Reg32_3_const_0 & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 > other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator <=( const Reg32_3_const_0 & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 <= other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator >=( const Reg32_3_const_0 & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 >= other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator ==( const Reg32_3_const_0 & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 == other.m_cur_val);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator !=( const Reg32_3_const_0 & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 != other.m_cur_val);
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator =( const Reg32_3_const_0 \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator +=( const Reg32_3_const_0 \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator -=( const Reg32_3_const_0 \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator *=( const Reg32_3_const_0 \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator /=( const Reg32_3_const_0 \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator |=( const Reg32_3_const_0 \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator &=( const Reg32_3_const_0 \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator ^=( const Reg32_3_const_0 \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator <<=( const Reg32_3_const_0 \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator >>=( const Reg32_3_const_0 \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator +( const Register & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 + other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator -( const Register & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 - other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator *( const Register & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 * other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator /( const Register & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 / other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator |( const Register & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 | other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator &( const Register & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 & other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator ^( const Register & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 ^ other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator <<( const Register & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 << other);
}

unsigned int leon3_funclt_trap::Reg32_3_const_0::operator >>( const Register & other \
    ) const throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 >> other);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator <( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 < other);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator >( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 > other);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator <=( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 <= other);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator >=( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 >= other);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator ==( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 == other);
}

bool leon3_funclt_trap::Reg32_3_const_0::operator !=( const Register & other ) const \
    throw(){
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return (0 != other);
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator =( const Register \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator +=( const Register \
    & other ) throw(){
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator -=( const Register \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator *=( const Register \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator /=( const Register \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator |=( const Register \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator &=( const Register \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator ^=( const Register \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator <<=( const Register \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

Reg32_3_const_0 & leon3_funclt_trap::Reg32_3_const_0::operator >>=( const Register \
    & other ) throw(){
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return *this;
}

std::ostream & leon3_funclt_trap::Reg32_3_const_0::operator <<( std::ostream & stream \
    ) const throw(){
    stream << std::hex << std::showbase << 0 << std::dec;
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return stream;
}

leon3_funclt_trap::Reg32_3_const_0::Reg32_3_const_0(){
    this->m_cur_val = 0;
}

leon3_funclt_trap::Reg32_3_const_0::Reg32_3_const_0(const char *name) : Register(name){
    this->m_cur_val = 0;
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


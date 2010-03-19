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


#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include <ostream>
#include <trap_utils.hpp>

#define FUNC_MODEL
#define LT_IF
#define key_VER 0
#define key_ICC_z 1
#define key_ICC_v 2
#define key_EF 3
#define key_EC 4
#define key_ICC_n 5
#define key_S 6
#define key_ET 7
#define key_ICC_c 8
#define key_PS 9
#define key_PIL 10
#define key_CWP 11
#define key_IMPL 12
#define key_WIM_28 13
#define key_WIM_29 14
#define key_WIM_24 15
#define key_WIM_25 16
#define key_WIM_26 17
#define key_WIM_27 18
#define key_WIM_20 19
#define key_WIM_21 20
#define key_WIM_22 21
#define key_WIM_23 22
#define key_WIM_9 23
#define key_WIM_8 24
#define key_WIM_1 25
#define key_WIM_0 26
#define key_WIM_3 27
#define key_WIM_2 28
#define key_WIM_5 29
#define key_WIM_4 30
#define key_WIM_7 31
#define key_WIM_6 32
#define key_WIM_11 33
#define key_WIM_10 34
#define key_WIM_13 35
#define key_WIM_12 36
#define key_WIM_15 37
#define key_WIM_14 38
#define key_WIM_17 39
#define key_WIM_16 40
#define key_WIM_19 41
#define key_WIM_18 42
#define key_WIM_31 43
#define key_WIM_30 44
#define key_TBA 45
#define key_TT 46
namespace leon3_funclt_trap{

    class InnerField{

        public:
        InnerField & operator =( const InnerField & other ) throw();
        virtual InnerField & operator =( const unsigned int & other ) throw() = 0;
        virtual operator unsigned int() const throw() = 0;
        virtual ~InnerField();
    };

};

namespace leon3_funclt_trap{

    class Register{

        public:
        Register( const Register & other );
        Register();
        virtual void immediateWrite( const unsigned int & value ) throw() = 0;
        virtual unsigned int readNewValue() throw() = 0;
        virtual void clockCycle() throw();
        virtual InnerField & operator []( int bitField ) throw() = 0;
        virtual unsigned int operator ~() throw() = 0;
        virtual unsigned int operator +( const Register & other ) const throw() = 0;
        virtual unsigned int operator -( const Register & other ) const throw() = 0;
        virtual unsigned int operator *( const Register & other ) const throw() = 0;
        virtual unsigned int operator /( const Register & other ) const throw() = 0;
        virtual unsigned int operator |( const Register & other ) const throw() = 0;
        virtual unsigned int operator &( const Register & other ) const throw() = 0;
        virtual unsigned int operator ^( const Register & other ) const throw() = 0;
        virtual unsigned int operator <<( const Register & other ) const throw() = 0;
        virtual unsigned int operator >>( const Register & other ) const throw() = 0;
        virtual bool operator <( const Register & other ) const throw() = 0;
        virtual bool operator >( const Register & other ) const throw() = 0;
        virtual bool operator <=( const Register & other ) const throw() = 0;
        virtual bool operator >=( const Register & other ) const throw() = 0;
        virtual bool operator ==( const Register & other ) const throw() = 0;
        virtual bool operator !=( const Register & other ) const throw() = 0;
        virtual Register & operator =( const unsigned int & other ) throw() = 0;
        virtual Register & operator +=( const unsigned int & other ) throw() = 0;
        virtual Register & operator -=( const unsigned int & other ) throw() = 0;
        virtual Register & operator *=( const unsigned int & other ) throw() = 0;
        virtual Register & operator /=( const unsigned int & other ) throw() = 0;
        virtual Register & operator |=( const unsigned int & other ) throw() = 0;
        virtual Register & operator &=( const unsigned int & other ) throw() = 0;
        virtual Register & operator ^=( const unsigned int & other ) throw() = 0;
        virtual Register & operator <<=( const unsigned int & other ) throw() = 0;
        virtual Register & operator >>=( const unsigned int & other ) throw() = 0;
        virtual Register & operator =( const Register & other ) throw() = 0;
        virtual Register & operator +=( const Register & other ) throw() = 0;
        virtual Register & operator -=( const Register & other ) throw() = 0;
        virtual Register & operator *=( const Register & other ) throw() = 0;
        virtual Register & operator /=( const Register & other ) throw() = 0;
        virtual Register & operator |=( const Register & other ) throw() = 0;
        virtual Register & operator &=( const Register & other ) throw() = 0;
        virtual Register & operator ^=( const Register & other ) throw() = 0;
        virtual Register & operator <<=( const Register & other ) throw() = 0;
        virtual Register & operator >>=( const Register & other ) throw() = 0;
        virtual std::ostream & operator <<( std::ostream & other ) const throw() = 0;
        virtual operator unsigned int() const throw() = 0;
    };

};

namespace leon3_funclt_trap{

    class Reg32_0 : public Register{
        public:
        class InnerField_VER : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_VER( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0xf000000) >> 24;
            }
            virtual ~InnerField_VER();
        };

        class InnerField_ICC_z : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_ICC_z( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x400000) >> 22;
            }
            virtual ~InnerField_ICC_z();
        };

        class InnerField_ICC_v : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_ICC_v( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x200000) >> 21;
            }
            virtual ~InnerField_ICC_v();
        };

        class InnerField_EF : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_EF( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x1000) >> 12;
            }
            virtual ~InnerField_EF();
        };

        class InnerField_EC : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_EC( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x2000) >> 13;
            }
            virtual ~InnerField_EC();
        };

        class InnerField_ICC_n : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_ICC_n( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x800000) >> 23;
            }
            virtual ~InnerField_ICC_n();
        };

        class InnerField_S : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_S( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x80) >> 7;
            }
            virtual ~InnerField_S();
        };

        class InnerField_ET : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_ET( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x20) >> 5;
            }
            virtual ~InnerField_ET();
        };

        class InnerField_ICC_c : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_ICC_c( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x100000) >> 20;
            }
            virtual ~InnerField_ICC_c();
        };

        class InnerField_PS : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_PS( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x40) >> 6;
            }
            virtual ~InnerField_PS();
        };

        class InnerField_PIL : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_PIL( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0xf00) >> 8;
            }
            virtual ~InnerField_PIL();
        };

        class InnerField_CWP : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_CWP( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x1f);
            }
            virtual ~InnerField_CWP();
        };

        class InnerField_IMPL : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_IMPL( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0xf0000000L) >> 28;
            }
            virtual ~InnerField_IMPL();
        };

        class InnerField_Empty : public InnerField{

            public:
            InnerField_Empty();
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return 0;
            }
            virtual ~InnerField_Empty();
        };

        private:
        InnerField_VER field_VER;
        InnerField_ICC_z field_ICC_z;
        InnerField_ICC_v field_ICC_v;
        InnerField_EF field_EF;
        InnerField_EC field_EC;
        InnerField_ICC_n field_ICC_n;
        InnerField_S field_S;
        InnerField_ET field_ET;
        InnerField_ICC_c field_ICC_c;
        InnerField_PS field_PS;
        InnerField_PIL field_PIL;
        InnerField_CWP field_CWP;
        InnerField_IMPL field_IMPL;
        InnerField_Empty field_empty;
        unsigned int value;

        public:
        Reg32_0();
        inline InnerField & operator []( int bitField ) throw(){
            switch(bitField){
                case key_VER:{
                    return this->field_VER;
                    break;
                }
                case key_ICC_z:{
                    return this->field_ICC_z;
                    break;
                }
                case key_ICC_v:{
                    return this->field_ICC_v;
                    break;
                }
                case key_EF:{
                    return this->field_EF;
                    break;
                }
                case key_EC:{
                    return this->field_EC;
                    break;
                }
                case key_ICC_n:{
                    return this->field_ICC_n;
                    break;
                }
                case key_S:{
                    return this->field_S;
                    break;
                }
                case key_ET:{
                    return this->field_ET;
                    break;
                }
                case key_ICC_c:{
                    return this->field_ICC_c;
                    break;
                }
                case key_PS:{
                    return this->field_PS;
                    break;
                }
                case key_PIL:{
                    return this->field_PIL;
                    break;
                }
                case key_CWP:{
                    return this->field_CWP;
                    break;
                }
                case key_IMPL:{
                    return this->field_IMPL;
                    break;
                }
                default:{
                    return this->field_empty;
                    break;
                }
            }
        }
        void immediateWrite( const unsigned int & value ) throw();
        unsigned int readNewValue() throw();
        unsigned int operator ~() throw();
        Reg32_0 & operator =( const unsigned int & other ) throw();
        Reg32_0 & operator +=( const unsigned int & other ) throw();
        Reg32_0 & operator -=( const unsigned int & other ) throw();
        Reg32_0 & operator *=( const unsigned int & other ) throw();
        Reg32_0 & operator /=( const unsigned int & other ) throw();
        Reg32_0 & operator |=( const unsigned int & other ) throw();
        Reg32_0 & operator &=( const unsigned int & other ) throw();
        Reg32_0 & operator ^=( const unsigned int & other ) throw();
        Reg32_0 & operator <<=( const unsigned int & other ) throw();
        Reg32_0 & operator >>=( const unsigned int & other ) throw();
        unsigned int operator +( const Reg32_0 & other ) const throw();
        unsigned int operator -( const Reg32_0 & other ) const throw();
        unsigned int operator *( const Reg32_0 & other ) const throw();
        unsigned int operator /( const Reg32_0 & other ) const throw();
        unsigned int operator |( const Reg32_0 & other ) const throw();
        unsigned int operator &( const Reg32_0 & other ) const throw();
        unsigned int operator ^( const Reg32_0 & other ) const throw();
        unsigned int operator <<( const Reg32_0 & other ) const throw();
        unsigned int operator >>( const Reg32_0 & other ) const throw();
        bool operator <( const Reg32_0 & other ) const throw();
        bool operator >( const Reg32_0 & other ) const throw();
        bool operator <=( const Reg32_0 & other ) const throw();
        bool operator >=( const Reg32_0 & other ) const throw();
        bool operator ==( const Reg32_0 & other ) const throw();
        bool operator !=( const Reg32_0 & other ) const throw();
        Reg32_0 & operator =( const Reg32_0 & other ) throw();
        Reg32_0 & operator +=( const Reg32_0 & other ) throw();
        Reg32_0 & operator -=( const Reg32_0 & other ) throw();
        Reg32_0 & operator *=( const Reg32_0 & other ) throw();
        Reg32_0 & operator /=( const Reg32_0 & other ) throw();
        Reg32_0 & operator |=( const Reg32_0 & other ) throw();
        Reg32_0 & operator &=( const Reg32_0 & other ) throw();
        Reg32_0 & operator ^=( const Reg32_0 & other ) throw();
        Reg32_0 & operator <<=( const Reg32_0 & other ) throw();
        Reg32_0 & operator >>=( const Reg32_0 & other ) throw();
        unsigned int operator +( const Register & other ) const throw();
        unsigned int operator -( const Register & other ) const throw();
        unsigned int operator *( const Register & other ) const throw();
        unsigned int operator /( const Register & other ) const throw();
        unsigned int operator |( const Register & other ) const throw();
        unsigned int operator &( const Register & other ) const throw();
        unsigned int operator ^( const Register & other ) const throw();
        unsigned int operator <<( const Register & other ) const throw();
        unsigned int operator >>( const Register & other ) const throw();
        bool operator <( const Register & other ) const throw();
        bool operator >( const Register & other ) const throw();
        bool operator <=( const Register & other ) const throw();
        bool operator >=( const Register & other ) const throw();
        bool operator ==( const Register & other ) const throw();
        bool operator !=( const Register & other ) const throw();
        Reg32_0 & operator =( const Register & other ) throw();
        Reg32_0 & operator +=( const Register & other ) throw();
        Reg32_0 & operator -=( const Register & other ) throw();
        Reg32_0 & operator *=( const Register & other ) throw();
        Reg32_0 & operator /=( const Register & other ) throw();
        Reg32_0 & operator |=( const Register & other ) throw();
        Reg32_0 & operator &=( const Register & other ) throw();
        Reg32_0 & operator ^=( const Register & other ) throw();
        Reg32_0 & operator <<=( const Register & other ) throw();
        Reg32_0 & operator >>=( const Register & other ) throw();
        inline operator unsigned int() const throw(){
            return this->value;
        }
        std::ostream & operator <<( std::ostream & stream ) const throw();
    };

};

namespace leon3_funclt_trap{

    class Reg32_1 : public Register{
        public:
        class InnerField_WIM_28 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_28( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x10000000) >> 28;
            }
            virtual ~InnerField_WIM_28();
        };

        class InnerField_WIM_29 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_29( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x20000000) >> 29;
            }
            virtual ~InnerField_WIM_29();
        };

        class InnerField_WIM_24 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_24( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x1000000) >> 24;
            }
            virtual ~InnerField_WIM_24();
        };

        class InnerField_WIM_25 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_25( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x2000000) >> 25;
            }
            virtual ~InnerField_WIM_25();
        };

        class InnerField_WIM_26 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_26( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x4000000) >> 26;
            }
            virtual ~InnerField_WIM_26();
        };

        class InnerField_WIM_27 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_27( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x8000000) >> 27;
            }
            virtual ~InnerField_WIM_27();
        };

        class InnerField_WIM_20 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_20( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x100000) >> 20;
            }
            virtual ~InnerField_WIM_20();
        };

        class InnerField_WIM_21 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_21( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x200000) >> 21;
            }
            virtual ~InnerField_WIM_21();
        };

        class InnerField_WIM_22 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_22( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x400000) >> 22;
            }
            virtual ~InnerField_WIM_22();
        };

        class InnerField_WIM_23 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_23( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x800000) >> 23;
            }
            virtual ~InnerField_WIM_23();
        };

        class InnerField_WIM_9 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_9( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x200) >> 9;
            }
            virtual ~InnerField_WIM_9();
        };

        class InnerField_WIM_8 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_8( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x100) >> 8;
            }
            virtual ~InnerField_WIM_8();
        };

        class InnerField_WIM_1 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_1( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x2) >> 1;
            }
            virtual ~InnerField_WIM_1();
        };

        class InnerField_WIM_0 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_0( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x1);
            }
            virtual ~InnerField_WIM_0();
        };

        class InnerField_WIM_3 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_3( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x8) >> 3;
            }
            virtual ~InnerField_WIM_3();
        };

        class InnerField_WIM_2 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_2( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x4) >> 2;
            }
            virtual ~InnerField_WIM_2();
        };

        class InnerField_WIM_5 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_5( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x20) >> 5;
            }
            virtual ~InnerField_WIM_5();
        };

        class InnerField_WIM_4 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_4( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x10) >> 4;
            }
            virtual ~InnerField_WIM_4();
        };

        class InnerField_WIM_7 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_7( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x80) >> 7;
            }
            virtual ~InnerField_WIM_7();
        };

        class InnerField_WIM_6 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_6( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x40) >> 6;
            }
            virtual ~InnerField_WIM_6();
        };

        class InnerField_WIM_11 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_11( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x800) >> 11;
            }
            virtual ~InnerField_WIM_11();
        };

        class InnerField_WIM_10 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_10( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x400) >> 10;
            }
            virtual ~InnerField_WIM_10();
        };

        class InnerField_WIM_13 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_13( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x2000) >> 13;
            }
            virtual ~InnerField_WIM_13();
        };

        class InnerField_WIM_12 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_12( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x1000) >> 12;
            }
            virtual ~InnerField_WIM_12();
        };

        class InnerField_WIM_15 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_15( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x8000) >> 15;
            }
            virtual ~InnerField_WIM_15();
        };

        class InnerField_WIM_14 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_14( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x4000) >> 14;
            }
            virtual ~InnerField_WIM_14();
        };

        class InnerField_WIM_17 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_17( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x20000) >> 17;
            }
            virtual ~InnerField_WIM_17();
        };

        class InnerField_WIM_16 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_16( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x10000) >> 16;
            }
            virtual ~InnerField_WIM_16();
        };

        class InnerField_WIM_19 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_19( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x80000) >> 19;
            }
            virtual ~InnerField_WIM_19();
        };

        class InnerField_WIM_18 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_18( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x40000) >> 18;
            }
            virtual ~InnerField_WIM_18();
        };

        class InnerField_WIM_31 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_31( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x80000000L) >> 31;
            }
            virtual ~InnerField_WIM_31();
        };

        class InnerField_WIM_30 : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_WIM_30( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0x40000000) >> 30;
            }
            virtual ~InnerField_WIM_30();
        };

        class InnerField_Empty : public InnerField{

            public:
            InnerField_Empty();
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return 0;
            }
            virtual ~InnerField_Empty();
        };

        private:
        InnerField_WIM_28 field_WIM_28;
        InnerField_WIM_29 field_WIM_29;
        InnerField_WIM_24 field_WIM_24;
        InnerField_WIM_25 field_WIM_25;
        InnerField_WIM_26 field_WIM_26;
        InnerField_WIM_27 field_WIM_27;
        InnerField_WIM_20 field_WIM_20;
        InnerField_WIM_21 field_WIM_21;
        InnerField_WIM_22 field_WIM_22;
        InnerField_WIM_23 field_WIM_23;
        InnerField_WIM_9 field_WIM_9;
        InnerField_WIM_8 field_WIM_8;
        InnerField_WIM_1 field_WIM_1;
        InnerField_WIM_0 field_WIM_0;
        InnerField_WIM_3 field_WIM_3;
        InnerField_WIM_2 field_WIM_2;
        InnerField_WIM_5 field_WIM_5;
        InnerField_WIM_4 field_WIM_4;
        InnerField_WIM_7 field_WIM_7;
        InnerField_WIM_6 field_WIM_6;
        InnerField_WIM_11 field_WIM_11;
        InnerField_WIM_10 field_WIM_10;
        InnerField_WIM_13 field_WIM_13;
        InnerField_WIM_12 field_WIM_12;
        InnerField_WIM_15 field_WIM_15;
        InnerField_WIM_14 field_WIM_14;
        InnerField_WIM_17 field_WIM_17;
        InnerField_WIM_16 field_WIM_16;
        InnerField_WIM_19 field_WIM_19;
        InnerField_WIM_18 field_WIM_18;
        InnerField_WIM_31 field_WIM_31;
        InnerField_WIM_30 field_WIM_30;
        InnerField_Empty field_empty;
        unsigned int value;

        public:
        Reg32_1();
        inline InnerField & operator []( int bitField ) throw(){
            switch(bitField){
                case key_WIM_28:{
                    return this->field_WIM_28;
                    break;
                }
                case key_WIM_29:{
                    return this->field_WIM_29;
                    break;
                }
                case key_WIM_24:{
                    return this->field_WIM_24;
                    break;
                }
                case key_WIM_25:{
                    return this->field_WIM_25;
                    break;
                }
                case key_WIM_26:{
                    return this->field_WIM_26;
                    break;
                }
                case key_WIM_27:{
                    return this->field_WIM_27;
                    break;
                }
                case key_WIM_20:{
                    return this->field_WIM_20;
                    break;
                }
                case key_WIM_21:{
                    return this->field_WIM_21;
                    break;
                }
                case key_WIM_22:{
                    return this->field_WIM_22;
                    break;
                }
                case key_WIM_23:{
                    return this->field_WIM_23;
                    break;
                }
                case key_WIM_9:{
                    return this->field_WIM_9;
                    break;
                }
                case key_WIM_8:{
                    return this->field_WIM_8;
                    break;
                }
                case key_WIM_1:{
                    return this->field_WIM_1;
                    break;
                }
                case key_WIM_0:{
                    return this->field_WIM_0;
                    break;
                }
                case key_WIM_3:{
                    return this->field_WIM_3;
                    break;
                }
                case key_WIM_2:{
                    return this->field_WIM_2;
                    break;
                }
                case key_WIM_5:{
                    return this->field_WIM_5;
                    break;
                }
                case key_WIM_4:{
                    return this->field_WIM_4;
                    break;
                }
                case key_WIM_7:{
                    return this->field_WIM_7;
                    break;
                }
                case key_WIM_6:{
                    return this->field_WIM_6;
                    break;
                }
                case key_WIM_11:{
                    return this->field_WIM_11;
                    break;
                }
                case key_WIM_10:{
                    return this->field_WIM_10;
                    break;
                }
                case key_WIM_13:{
                    return this->field_WIM_13;
                    break;
                }
                case key_WIM_12:{
                    return this->field_WIM_12;
                    break;
                }
                case key_WIM_15:{
                    return this->field_WIM_15;
                    break;
                }
                case key_WIM_14:{
                    return this->field_WIM_14;
                    break;
                }
                case key_WIM_17:{
                    return this->field_WIM_17;
                    break;
                }
                case key_WIM_16:{
                    return this->field_WIM_16;
                    break;
                }
                case key_WIM_19:{
                    return this->field_WIM_19;
                    break;
                }
                case key_WIM_18:{
                    return this->field_WIM_18;
                    break;
                }
                case key_WIM_31:{
                    return this->field_WIM_31;
                    break;
                }
                case key_WIM_30:{
                    return this->field_WIM_30;
                    break;
                }
                default:{
                    return this->field_empty;
                    break;
                }
            }
        }
        void immediateWrite( const unsigned int & value ) throw();
        unsigned int readNewValue() throw();
        unsigned int operator ~() throw();
        Reg32_1 & operator =( const unsigned int & other ) throw();
        Reg32_1 & operator +=( const unsigned int & other ) throw();
        Reg32_1 & operator -=( const unsigned int & other ) throw();
        Reg32_1 & operator *=( const unsigned int & other ) throw();
        Reg32_1 & operator /=( const unsigned int & other ) throw();
        Reg32_1 & operator |=( const unsigned int & other ) throw();
        Reg32_1 & operator &=( const unsigned int & other ) throw();
        Reg32_1 & operator ^=( const unsigned int & other ) throw();
        Reg32_1 & operator <<=( const unsigned int & other ) throw();
        Reg32_1 & operator >>=( const unsigned int & other ) throw();
        unsigned int operator +( const Reg32_1 & other ) const throw();
        unsigned int operator -( const Reg32_1 & other ) const throw();
        unsigned int operator *( const Reg32_1 & other ) const throw();
        unsigned int operator /( const Reg32_1 & other ) const throw();
        unsigned int operator |( const Reg32_1 & other ) const throw();
        unsigned int operator &( const Reg32_1 & other ) const throw();
        unsigned int operator ^( const Reg32_1 & other ) const throw();
        unsigned int operator <<( const Reg32_1 & other ) const throw();
        unsigned int operator >>( const Reg32_1 & other ) const throw();
        bool operator <( const Reg32_1 & other ) const throw();
        bool operator >( const Reg32_1 & other ) const throw();
        bool operator <=( const Reg32_1 & other ) const throw();
        bool operator >=( const Reg32_1 & other ) const throw();
        bool operator ==( const Reg32_1 & other ) const throw();
        bool operator !=( const Reg32_1 & other ) const throw();
        Reg32_1 & operator =( const Reg32_1 & other ) throw();
        Reg32_1 & operator +=( const Reg32_1 & other ) throw();
        Reg32_1 & operator -=( const Reg32_1 & other ) throw();
        Reg32_1 & operator *=( const Reg32_1 & other ) throw();
        Reg32_1 & operator /=( const Reg32_1 & other ) throw();
        Reg32_1 & operator |=( const Reg32_1 & other ) throw();
        Reg32_1 & operator &=( const Reg32_1 & other ) throw();
        Reg32_1 & operator ^=( const Reg32_1 & other ) throw();
        Reg32_1 & operator <<=( const Reg32_1 & other ) throw();
        Reg32_1 & operator >>=( const Reg32_1 & other ) throw();
        unsigned int operator +( const Register & other ) const throw();
        unsigned int operator -( const Register & other ) const throw();
        unsigned int operator *( const Register & other ) const throw();
        unsigned int operator /( const Register & other ) const throw();
        unsigned int operator |( const Register & other ) const throw();
        unsigned int operator &( const Register & other ) const throw();
        unsigned int operator ^( const Register & other ) const throw();
        unsigned int operator <<( const Register & other ) const throw();
        unsigned int operator >>( const Register & other ) const throw();
        bool operator <( const Register & other ) const throw();
        bool operator >( const Register & other ) const throw();
        bool operator <=( const Register & other ) const throw();
        bool operator >=( const Register & other ) const throw();
        bool operator ==( const Register & other ) const throw();
        bool operator !=( const Register & other ) const throw();
        Reg32_1 & operator =( const Register & other ) throw();
        Reg32_1 & operator +=( const Register & other ) throw();
        Reg32_1 & operator -=( const Register & other ) throw();
        Reg32_1 & operator *=( const Register & other ) throw();
        Reg32_1 & operator /=( const Register & other ) throw();
        Reg32_1 & operator |=( const Register & other ) throw();
        Reg32_1 & operator &=( const Register & other ) throw();
        Reg32_1 & operator ^=( const Register & other ) throw();
        Reg32_1 & operator <<=( const Register & other ) throw();
        Reg32_1 & operator >>=( const Register & other ) throw();
        inline operator unsigned int() const throw(){
            return this->value;
        }
        std::ostream & operator <<( std::ostream & stream ) const throw();
    };

};

namespace leon3_funclt_trap{

    class Reg32_2 : public Register{
        public:
        class InnerField_TBA : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_TBA( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0xfffff000L) >> 12;
            }
            virtual ~InnerField_TBA();
        };

        class InnerField_TT : public InnerField{
            private:
            unsigned int & value;

            public:
            InnerField_TT( unsigned int & value );
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return (this->value & 0xff0) >> 4;
            }
            virtual ~InnerField_TT();
        };

        class InnerField_Empty : public InnerField{

            public:
            InnerField_Empty();
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return 0;
            }
            virtual ~InnerField_Empty();
        };

        private:
        InnerField_TBA field_TBA;
        InnerField_TT field_TT;
        InnerField_Empty field_empty;
        unsigned int value;

        public:
        Reg32_2();
        inline InnerField & operator []( int bitField ) throw(){
            switch(bitField){
                case key_TBA:{
                    return this->field_TBA;
                    break;
                }
                case key_TT:{
                    return this->field_TT;
                    break;
                }
                default:{
                    return this->field_empty;
                    break;
                }
            }
        }
        void immediateWrite( const unsigned int & value ) throw();
        unsigned int readNewValue() throw();
        unsigned int operator ~() throw();
        Reg32_2 & operator =( const unsigned int & other ) throw();
        Reg32_2 & operator +=( const unsigned int & other ) throw();
        Reg32_2 & operator -=( const unsigned int & other ) throw();
        Reg32_2 & operator *=( const unsigned int & other ) throw();
        Reg32_2 & operator /=( const unsigned int & other ) throw();
        Reg32_2 & operator |=( const unsigned int & other ) throw();
        Reg32_2 & operator &=( const unsigned int & other ) throw();
        Reg32_2 & operator ^=( const unsigned int & other ) throw();
        Reg32_2 & operator <<=( const unsigned int & other ) throw();
        Reg32_2 & operator >>=( const unsigned int & other ) throw();
        unsigned int operator +( const Reg32_2 & other ) const throw();
        unsigned int operator -( const Reg32_2 & other ) const throw();
        unsigned int operator *( const Reg32_2 & other ) const throw();
        unsigned int operator /( const Reg32_2 & other ) const throw();
        unsigned int operator |( const Reg32_2 & other ) const throw();
        unsigned int operator &( const Reg32_2 & other ) const throw();
        unsigned int operator ^( const Reg32_2 & other ) const throw();
        unsigned int operator <<( const Reg32_2 & other ) const throw();
        unsigned int operator >>( const Reg32_2 & other ) const throw();
        bool operator <( const Reg32_2 & other ) const throw();
        bool operator >( const Reg32_2 & other ) const throw();
        bool operator <=( const Reg32_2 & other ) const throw();
        bool operator >=( const Reg32_2 & other ) const throw();
        bool operator ==( const Reg32_2 & other ) const throw();
        bool operator !=( const Reg32_2 & other ) const throw();
        Reg32_2 & operator =( const Reg32_2 & other ) throw();
        Reg32_2 & operator +=( const Reg32_2 & other ) throw();
        Reg32_2 & operator -=( const Reg32_2 & other ) throw();
        Reg32_2 & operator *=( const Reg32_2 & other ) throw();
        Reg32_2 & operator /=( const Reg32_2 & other ) throw();
        Reg32_2 & operator |=( const Reg32_2 & other ) throw();
        Reg32_2 & operator &=( const Reg32_2 & other ) throw();
        Reg32_2 & operator ^=( const Reg32_2 & other ) throw();
        Reg32_2 & operator <<=( const Reg32_2 & other ) throw();
        Reg32_2 & operator >>=( const Reg32_2 & other ) throw();
        unsigned int operator +( const Register & other ) const throw();
        unsigned int operator -( const Register & other ) const throw();
        unsigned int operator *( const Register & other ) const throw();
        unsigned int operator /( const Register & other ) const throw();
        unsigned int operator |( const Register & other ) const throw();
        unsigned int operator &( const Register & other ) const throw();
        unsigned int operator ^( const Register & other ) const throw();
        unsigned int operator <<( const Register & other ) const throw();
        unsigned int operator >>( const Register & other ) const throw();
        bool operator <( const Register & other ) const throw();
        bool operator >( const Register & other ) const throw();
        bool operator <=( const Register & other ) const throw();
        bool operator >=( const Register & other ) const throw();
        bool operator ==( const Register & other ) const throw();
        bool operator !=( const Register & other ) const throw();
        Reg32_2 & operator =( const Register & other ) throw();
        Reg32_2 & operator +=( const Register & other ) throw();
        Reg32_2 & operator -=( const Register & other ) throw();
        Reg32_2 & operator *=( const Register & other ) throw();
        Reg32_2 & operator /=( const Register & other ) throw();
        Reg32_2 & operator |=( const Register & other ) throw();
        Reg32_2 & operator &=( const Register & other ) throw();
        Reg32_2 & operator ^=( const Register & other ) throw();
        Reg32_2 & operator <<=( const Register & other ) throw();
        Reg32_2 & operator >>=( const Register & other ) throw();
        inline operator unsigned int() const throw(){
            return this->value;
        }
        std::ostream & operator <<( std::ostream & stream ) const throw();
    };

};

namespace leon3_funclt_trap{

    class Reg32_3 : public Register{
        public:
        class InnerField_Empty : public InnerField{

            public:
            InnerField_Empty();
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return 0;
            }
            virtual ~InnerField_Empty();
        };

        private:
        InnerField_Empty field_empty;
        unsigned int value;

        public:
        Reg32_3();
        inline InnerField & operator []( int bitField ) throw(){
            return this->field_empty;
        }
        void immediateWrite( const unsigned int & value ) throw();
        unsigned int readNewValue() throw();
        unsigned int operator ~() throw();
        Reg32_3 & operator =( const unsigned int & other ) throw();
        Reg32_3 & operator +=( const unsigned int & other ) throw();
        Reg32_3 & operator -=( const unsigned int & other ) throw();
        Reg32_3 & operator *=( const unsigned int & other ) throw();
        Reg32_3 & operator /=( const unsigned int & other ) throw();
        Reg32_3 & operator |=( const unsigned int & other ) throw();
        Reg32_3 & operator &=( const unsigned int & other ) throw();
        Reg32_3 & operator ^=( const unsigned int & other ) throw();
        Reg32_3 & operator <<=( const unsigned int & other ) throw();
        Reg32_3 & operator >>=( const unsigned int & other ) throw();
        unsigned int operator +( const Reg32_3 & other ) const throw();
        unsigned int operator -( const Reg32_3 & other ) const throw();
        unsigned int operator *( const Reg32_3 & other ) const throw();
        unsigned int operator /( const Reg32_3 & other ) const throw();
        unsigned int operator |( const Reg32_3 & other ) const throw();
        unsigned int operator &( const Reg32_3 & other ) const throw();
        unsigned int operator ^( const Reg32_3 & other ) const throw();
        unsigned int operator <<( const Reg32_3 & other ) const throw();
        unsigned int operator >>( const Reg32_3 & other ) const throw();
        bool operator <( const Reg32_3 & other ) const throw();
        bool operator >( const Reg32_3 & other ) const throw();
        bool operator <=( const Reg32_3 & other ) const throw();
        bool operator >=( const Reg32_3 & other ) const throw();
        bool operator ==( const Reg32_3 & other ) const throw();
        bool operator !=( const Reg32_3 & other ) const throw();
        Reg32_3 & operator =( const Reg32_3 & other ) throw();
        Reg32_3 & operator +=( const Reg32_3 & other ) throw();
        Reg32_3 & operator -=( const Reg32_3 & other ) throw();
        Reg32_3 & operator *=( const Reg32_3 & other ) throw();
        Reg32_3 & operator /=( const Reg32_3 & other ) throw();
        Reg32_3 & operator |=( const Reg32_3 & other ) throw();
        Reg32_3 & operator &=( const Reg32_3 & other ) throw();
        Reg32_3 & operator ^=( const Reg32_3 & other ) throw();
        Reg32_3 & operator <<=( const Reg32_3 & other ) throw();
        Reg32_3 & operator >>=( const Reg32_3 & other ) throw();
        unsigned int operator +( const Register & other ) const throw();
        unsigned int operator -( const Register & other ) const throw();
        unsigned int operator *( const Register & other ) const throw();
        unsigned int operator /( const Register & other ) const throw();
        unsigned int operator |( const Register & other ) const throw();
        unsigned int operator &( const Register & other ) const throw();
        unsigned int operator ^( const Register & other ) const throw();
        unsigned int operator <<( const Register & other ) const throw();
        unsigned int operator >>( const Register & other ) const throw();
        bool operator <( const Register & other ) const throw();
        bool operator >( const Register & other ) const throw();
        bool operator <=( const Register & other ) const throw();
        bool operator >=( const Register & other ) const throw();
        bool operator ==( const Register & other ) const throw();
        bool operator !=( const Register & other ) const throw();
        Reg32_3 & operator =( const Register & other ) throw();
        Reg32_3 & operator +=( const Register & other ) throw();
        Reg32_3 & operator -=( const Register & other ) throw();
        Reg32_3 & operator *=( const Register & other ) throw();
        Reg32_3 & operator /=( const Register & other ) throw();
        Reg32_3 & operator |=( const Register & other ) throw();
        Reg32_3 & operator &=( const Register & other ) throw();
        Reg32_3 & operator ^=( const Register & other ) throw();
        Reg32_3 & operator <<=( const Register & other ) throw();
        Reg32_3 & operator >>=( const Register & other ) throw();
        inline operator unsigned int() const throw(){
            return this->value;
        }
        std::ostream & operator <<( std::ostream & stream ) const throw();
    };

};

namespace leon3_funclt_trap{

    class Reg32_3_const_0 : public Register{
        public:
        class InnerField_Empty : public InnerField{

            public:
            InnerField_Empty();
            InnerField & operator =( const unsigned int & other ) throw();
            inline operator unsigned int() const throw(){
                return 0;
            }
            virtual ~InnerField_Empty();
        };

        private:
        InnerField_Empty field_empty;
        unsigned int value;

        public:
        Reg32_3_const_0();
        inline InnerField & operator []( int bitField ) throw(){
            return this->field_empty;
        }
        void immediateWrite( const unsigned int & value ) throw();
        unsigned int readNewValue() throw();
        unsigned int operator ~() throw();
        Reg32_3_const_0 & operator =( const unsigned int & other ) throw();
        Reg32_3_const_0 & operator +=( const unsigned int & other ) throw();
        Reg32_3_const_0 & operator -=( const unsigned int & other ) throw();
        Reg32_3_const_0 & operator *=( const unsigned int & other ) throw();
        Reg32_3_const_0 & operator /=( const unsigned int & other ) throw();
        Reg32_3_const_0 & operator |=( const unsigned int & other ) throw();
        Reg32_3_const_0 & operator &=( const unsigned int & other ) throw();
        Reg32_3_const_0 & operator ^=( const unsigned int & other ) throw();
        Reg32_3_const_0 & operator <<=( const unsigned int & other ) throw();
        Reg32_3_const_0 & operator >>=( const unsigned int & other ) throw();
        unsigned int operator +( const Reg32_3_const_0 & other ) const throw();
        unsigned int operator -( const Reg32_3_const_0 & other ) const throw();
        unsigned int operator *( const Reg32_3_const_0 & other ) const throw();
        unsigned int operator /( const Reg32_3_const_0 & other ) const throw();
        unsigned int operator |( const Reg32_3_const_0 & other ) const throw();
        unsigned int operator &( const Reg32_3_const_0 & other ) const throw();
        unsigned int operator ^( const Reg32_3_const_0 & other ) const throw();
        unsigned int operator <<( const Reg32_3_const_0 & other ) const throw();
        unsigned int operator >>( const Reg32_3_const_0 & other ) const throw();
        bool operator <( const Reg32_3_const_0 & other ) const throw();
        bool operator >( const Reg32_3_const_0 & other ) const throw();
        bool operator <=( const Reg32_3_const_0 & other ) const throw();
        bool operator >=( const Reg32_3_const_0 & other ) const throw();
        bool operator ==( const Reg32_3_const_0 & other ) const throw();
        bool operator !=( const Reg32_3_const_0 & other ) const throw();
        Reg32_3_const_0 & operator =( const Reg32_3_const_0 & other ) throw();
        Reg32_3_const_0 & operator +=( const Reg32_3_const_0 & other ) throw();
        Reg32_3_const_0 & operator -=( const Reg32_3_const_0 & other ) throw();
        Reg32_3_const_0 & operator *=( const Reg32_3_const_0 & other ) throw();
        Reg32_3_const_0 & operator /=( const Reg32_3_const_0 & other ) throw();
        Reg32_3_const_0 & operator |=( const Reg32_3_const_0 & other ) throw();
        Reg32_3_const_0 & operator &=( const Reg32_3_const_0 & other ) throw();
        Reg32_3_const_0 & operator ^=( const Reg32_3_const_0 & other ) throw();
        Reg32_3_const_0 & operator <<=( const Reg32_3_const_0 & other ) throw();
        Reg32_3_const_0 & operator >>=( const Reg32_3_const_0 & other ) throw();
        unsigned int operator +( const Register & other ) const throw();
        unsigned int operator -( const Register & other ) const throw();
        unsigned int operator *( const Register & other ) const throw();
        unsigned int operator /( const Register & other ) const throw();
        unsigned int operator |( const Register & other ) const throw();
        unsigned int operator &( const Register & other ) const throw();
        unsigned int operator ^( const Register & other ) const throw();
        unsigned int operator <<( const Register & other ) const throw();
        unsigned int operator >>( const Register & other ) const throw();
        bool operator <( const Register & other ) const throw();
        bool operator >( const Register & other ) const throw();
        bool operator <=( const Register & other ) const throw();
        bool operator >=( const Register & other ) const throw();
        bool operator ==( const Register & other ) const throw();
        bool operator !=( const Register & other ) const throw();
        Reg32_3_const_0 & operator =( const Register & other ) throw();
        Reg32_3_const_0 & operator +=( const Register & other ) throw();
        Reg32_3_const_0 & operator -=( const Register & other ) throw();
        Reg32_3_const_0 & operator *=( const Register & other ) throw();
        Reg32_3_const_0 & operator /=( const Register & other ) throw();
        Reg32_3_const_0 & operator |=( const Register & other ) throw();
        Reg32_3_const_0 & operator &=( const Register & other ) throw();
        Reg32_3_const_0 & operator ^=( const Register & other ) throw();
        Reg32_3_const_0 & operator <<=( const Register & other ) throw();
        Reg32_3_const_0 & operator >>=( const Register & other ) throw();
        inline operator unsigned int() const throw(){
            return 0;
        }
        std::ostream & operator <<( std::ostream & stream ) const throw();
    };

};

namespace leon3_funclt_trap{

    class RegisterBankClass{
        private:
        Register * * registers;
        unsigned int size;

        public:
        RegisterBankClass();
        RegisterBankClass( unsigned int size );
        void setNewRegister( unsigned int numReg, Register * newReg );
        void setSize( unsigned int size ) throw();
        inline Register & operator []( unsigned int numReg ) throw(){
            return *(this->registers[numReg]);
        }
        virtual ~RegisterBankClass();
    };

};



#endif

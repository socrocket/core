// ********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the
// European Space Agency or of TU-Braunschweig.
// ********************************************************************
// Title:      block_allocator.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Timo Veit
// Reviewed:
// ********************************************************************

#ifndef BLOCK_ALLOCATOR_H
#define BLOCK_ALLOCATOR_H


class block_allocator
{
private:
        struct block
        {
                size_t size;
                size_t used;
                char *buffer;
                block *next;
        };


        block *m_head;
        size_t m_blocksize;


        block_allocator(const block_allocator &);
        block_allocator &operator=(block_allocator &);


public:
        block_allocator(size_t blocksize);
        ~block_allocator();


        // exchange contents with rhs
        void swap(block_allocator &rhs);


        // allocate memory
        void *malloc(size_t size);


        // free all allocated blocks
        void free();
};


#endif
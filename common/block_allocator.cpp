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
// Title:      block_allocator.cpp
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

#include <memory.h>
#include <algorithm>
#include "block_allocator.h"


block_allocator::block_allocator(size_t blocksize): m_head(0), m_blocksize(blocksize)
{
}


block_allocator::~block_allocator()
{
        while (m_head)
        {
                block *temp = m_head->next;
                ::free(m_head);
                m_head = temp;
        }
}


void block_allocator::swap(block_allocator &rhs)
{
        std::swap(m_blocksize, rhs.m_blocksize);
        std::swap(m_head, rhs.m_head);
}


void *block_allocator::malloc(size_t size)
{
        if ((m_head && m_head->used + size > m_head->size) || !m_head)
        {
                // calc needed size for allocation
                size_t alloc_size = std::max(sizeof(block) + size, m_blocksize);


                // create new block
                char *buffer = (char *)::malloc(alloc_size);
                block *b = reinterpret_cast<block *>(buffer);
                b->size = alloc_size;
                b->used = sizeof(block);
                b->buffer = buffer;
                b->next = m_head;
                m_head = b;
        }


        void *ptr = m_head->buffer + m_head->used;
        m_head->used += size;
        return ptr;
}


void block_allocator::free()
{
        block_allocator(0).swap(*this);
}
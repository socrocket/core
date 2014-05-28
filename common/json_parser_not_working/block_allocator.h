// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file block_allocator.h
/// 
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Timo Veit
///

#ifndef BLOCK_ALLOCATOR_H
#define BLOCK_ALLOCATOR_H

#include <string>
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
/// @}
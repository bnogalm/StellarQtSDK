/*
 * CyoEncode.hpp - part of the CyoEncode library
 *
 * Copyright (c) 2009-2014, Graham Bull.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __CYOENCODE_HPP
#define __CYOENCODE_HPP

#include "CyoEncode.h"

namespace CyoEncode
{
    typedef unsigned char byte_t;
    typedef int size_t;
    class Base16
    {
    public:
        static size_t GetLength(size_t size) {
            return cyoBase16EncodeGetLength(size);
        }
        static size_t Encode(char* dest, const byte_t* src, size_t size) {
            return cyoBase16Encode(dest, src, size);
        }
        static size_t EncodeBlock(char dest[2], const byte_t src) {
            return cyoBase16EncodeBlock(dest, &src);
        }
    private:
        static size_t cyoBase16EncodeGetLength(size_t size);
        static size_t cyoBase16Encode(char *dest, const void *src, size_t size);
        static size_t cyoBase16EncodeBlock(char *dest, const void *src);
    };

    class Base32
    {
    public:
        static size_t GetLength(size_t size) {
            return cyoBase32EncodeGetLength(size);
        }
        static size_t Encode(char* dest, const byte_t* src, size_t size) {
            return cyoBase32Encode(dest, src, size);
        }
        static size_t EncodeBlock(char dest[8], const byte_t src[5]) {
            return cyoBase32EncodeBlock(dest, src);
        }
    private:
        static size_t cyoBase32EncodeGetLength(size_t size);
        static size_t cyoBase32Encode(char *dest, const void *src, size_t size);
        static size_t cyoBase32EncodeBlock(char *dest, const void *src);
    };

    class Base64
    {
    public:
        static size_t GetLength(size_t size) {
            return cyoBase64EncodeGetLength(size);
        }
        static size_t Encode(char* dest, const byte_t* src, size_t size) {
            return cyoBase64Encode(dest, src, size);
        }
        static size_t EncodeBlock(char dest[4], const byte_t src[3]) {
            return cyoBase64EncodeBlock(dest, src);
        }
    private:
        static size_t cyoBase64EncodeGetLength(size_t size);
        static size_t cyoBase64Encode(char *dest, const void *src, size_t size);
        static size_t cyoBase64EncodeBlock(char *dest, const void *src);
    };
}

#endif //__CYOENCODE_HPP

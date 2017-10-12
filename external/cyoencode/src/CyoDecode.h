/*
 * CyoDecode.hpp - part of the CyoEncode library
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

#ifndef __CYODECODE_HPP
#define __CYODECODE_HPP

namespace CyoDecode
{
    typedef unsigned char byte_t;
    typedef int size_t;
    class Base16
    {
    public:
        static int Validate(const char* src, size_t size) {
            return cyoBase16Validate(src, size);
        }
        static size_t GetLength(size_t size) {
            return cyoBase16DecodeGetLength(size);
        }
        static size_t Decode(byte_t* dest, const char* src, size_t size) {
            return cyoBase16Decode(dest, src, size);
        }
        static size_t DecodeBlock(byte_t& dest, const char src[2]) {
            return cyoBase16DecodeBlock(&dest, src);
        }
    private:
        static int cyoBase16Validate(const char *src, size_t size);
        static size_t cyoBase16DecodeGetLength(size_t size);
        static size_t cyoBase16Decode(void *dest, const char *src, size_t size);
        static size_t cyoBase16DecodeBlock(void *dest, const char *src);
    };

    class Base32
    {
    public:
        static int Validate(const char* src, size_t size) {
            return cyoBase32Validate(src, size);
        }
        static size_t GetLength(size_t size) {
            return cyoBase32DecodeGetLength(size);
        }
        static size_t Decode(byte_t* dest, const char* src, size_t size) {
            return cyoBase32Decode(dest, src, size);
        }
        static size_t DecodeBlock(byte_t dest[5], const char src[8]) {
            return cyoBase32DecodeBlock(dest, src);
        }
    private:
        static int cyoBase32Validate(const char *src, size_t size);
        static size_t cyoBase32DecodeGetLength(size_t size);
        static size_t cyoBase32Decode(void *dest, const char *src, size_t size);
        static size_t cyoBase32DecodeBlock(void *dest, const char *src);
    };

    class Base64
    {
    public:
        static int Validate(const char* src, size_t size) {
            return cyoBase64Validate(src, size);
        }
        static size_t GetLength(size_t size) {
            return cyoBase64DecodeGetLength(size);
        }
        static size_t Decode(byte_t* dest, const char* src, size_t size) {
            return cyoBase64Decode(dest, src, size);
        }
        static size_t DecodeBlock(byte_t dest[3], const char src[4]) {
            return cyoBase64DecodeBlock(dest, src);
        }
    private:
        static int cyoBase64Validate(const char *src, size_t size);
        static size_t cyoBase64DecodeGetLength(size_t size);
        static size_t cyoBase64Decode(void *dest, const char *src, size_t size);
        static size_t cyoBase64DecodeBlock(void *dest, const char *src);
    };
}

#endif //__CYODECODE_HPP

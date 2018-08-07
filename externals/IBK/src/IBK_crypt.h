﻿/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#ifndef IBK_cryptH
#define IBK_cryptH

#include <string>
#include <vector>

#ifdef _MSC_VER
	typedef __int64 int64_t;
	typedef unsigned __int64 uint64_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int16 uint16_t;
#else
	#include <stdint.h>
#endif

/*

The SuperFastHash-Functions are copyrighted under the following license:

Paul Hsieh OLD BSD license

Copyright (c) 2010, Paul Hsieh
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:

	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
	disclaimer in the documentation and/or other materials provided with the distribution.

	Neither my name, Paul Hsieh, nor the names of any other contributors to the code use may not be used to endorse or
	promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Software was obtained from http://www.azillionmonkeys.com/qed/hash.html


The BlowFish-Functions are copyrighted under the following license:

Copyright by George Anescu

Microsoft Public License (MS-PL)

This license governs use of the accompanying software. If you use the software, you
accept this license. If you do not accept the license, do not use the software.

1. Definitions
The terms "reproduce," "reproduction," "derivative works," and "distribution" have the
same meaning here as under U.S. copyright law.
A "contribution" is the original software, or any additions or changes to the software.
A "contributor" is any person that distributes its contribution under this license.
"Licensed patents" are a contributor's patent claims that read directly on its contribution.

2. Grant of Rights
(A) Copyright Grant- Subject to the terms of this license, including the license conditions and limitations in section
3, each contributor grants you a non-exclusive, worldwide, royalty-free copyright license to reproduce its contribution,
prepare derivative works of its contribution, and distribute its contribution or any derivative works that you create.
(B) Patent Grant- Subject to the terms of this license, including the license conditions and limitations in section 3,
each contributor grants you a non-exclusive, worldwide, royalty-free license under its licensed patents to make, have
made, use, sell, offer for sale, import, and/or otherwise dispose of its contribution in the software or derivative
works of the contribution in the software.

3. Conditions and Limitations
(A) No Trademark License- This license does not grant you rights to use any contributors' name, logo, or trademarks. (B)
If you bring a patent claim against any contributor over patents that you claim are infringed by the software, your
patent license from such contributor to the software ends automatically.
(C) If you distribute any portion of the
software, you must retain all copyright, patent, trademark, and attribution notices that are present in the software.
(D) If you distribute any portion of the software in source code form, you may do so only under this license by
including a complete copy of this license with your distribution. If you distribute any portion of the software in
compiled or object code form, you may only do so under a license that complies with this license.
(E) The software is
licensed "as-is." You bear the risk of using it. The contributors give no express warranties, guarantees or conditions.
You may have additional consumer rights under your local laws which this license cannot change. To the extent permitted
under your local laws, the contributors exclude the implied warranties of merchantability, fitness for a particular
purpose and non-infringement.


*/


namespace IBK {

/*! Super fast hash function, very good distribution, closed set.
	\param data The data we calculate the hash on
	\param len Length of the data field in char.
	\note Code copyrighted by Paul Hsieh, see license above.
*/
uint32_t SuperFastHash (const char * data, int len);


/*! Alternative version of above function for string arguments.
	\param str The string to calculate the hash on
	\note Code copyrighted by Paul Hsieh, see license above.
*/
uint32_t SuperFastHash ( const std::string & str );


/*! Super fast hash function, very good distribution, incremental calculation.
	\param data The data we calculate the hash on
	\param len Length of the data field in char.
	\param hash Old hash value, to calculate incremental new hash.
	\return Calculated hash value.
	\note Code copyrighted by Paul Hsieh, see license above.
*/
uint32_t SuperFastHashIncremental (const char * data, int len, uint32_t hash);

/*! Encodes to base64.
	Number of bytes in pInput should be a multiple of 4.
	This function is mainly for binary inputs
	\param pInput Input vector
	\param pOutput Output vector
	\return results false if its not possible
*/
bool base64_encode(const std::vector<unsigned char>& pInput, std::vector<unsigned char>& pOutput);

/*! Encodes a string to base64.
	Number of bytes in pInput should be a multiple of 4.
	This function is only for strings
	\param Value Input vector
	\return results the encoded string or an empty string
*/
std::string base64_encodeStr(const std::string& Value);

/*! Decodes a base64 encoded vector.
	Number of bytes in pInput should be a multiple of 4.
	This function is mainly for binary inputs
	\param Input Input vector
	\param Output Output vector
	\return results false if its not possible
*/
bool base64_decode(const std::vector<unsigned char>& Input, std::vector<unsigned char>& Output);

/*! Decodes a base64 encoded string.
	Number of bytes in pInput should be a multiple of 4.
	This function is only for strings
	\param Value Input vector
	\return results the decoded string or an empty string
*/
std::string base64_decodeStr(const std::string& Value);

/*! Creates a MD5 hash from the given string.
	\param val Input string
	\return resulting the hash as vector
*/
std::vector<unsigned int> md5(const std::string& val);

/*! Creates a MD5 hash from the given string.
	The hash will be represented as string with 32 hex numbers.
	\param val Input string
	\return resulting the hash as string
*/
std::string md5_str(const std::string& val);

/*! Creates a shortened MD5 hash from the given string with a given length beginning at random position.
	The resulting string contains begin position in first 2 chars.
	\param val Input string
	\param length Length of the md5 hash segment.
	\return resulting the hash as string
*/
std::string md5_str(const std::string& val, unsigned int length);

/*! Check the if the given string has the also give short md5 hash.
	\param val String for checking.
	\param md5 shortened md5 hash.
*/
bool checkShortMD5(const std::string& val, const std::string& md5);

/*! Encodes a vector using the blowfish algorithm.
	The result is depending the given key
	This function is mainly for binary inputs
	\param key Code key vector
	\param Input Input vector
	\param Output Output vector
	\return results false if encoding is not possible
*/
bool blowfish_encode(const std::vector<unsigned char>& key, const std::vector<unsigned char>& Input, std::vector<unsigned char>& Output);

/*! Encodes a vector using the blowfish algorithm.
	The result is depending the given key
	\param key Code key as string
	\param Value Input vector
	\return resulting the encoded string or an empty one.
*/
std::string blowfish_encodeStr(const std::string& key, const std::string& Value);

/*! Decodes a vector using the blowfish algorithm.
	The result is depending the given key (should be the same as encoding).
	This function is mainly for binary inputs
	\param key Code key vector
	\param Input Input vector
	\param Output Output vector
	\return results false if encoding is not possible
*/
bool blowfish_decode(const std::vector<unsigned char>& key, const std::vector<unsigned char>& Input, std::vector<unsigned char>& Output);

/*! Encodes a vector using the blowfish algorithm.
	The result is depending the given key (should be the same as encoding).
	\param key Code key as string
	\param Value Input vector
	\return resulting the encoded string or an empty one.
*/
std::string blowfish_decodeStr(const std::string& key, const std::string& Value);

}	// namespace IBK

/*! \file IBK_crypt.h
	\brief Contains cryptographic functions.
*/

#endif // IBK_cryptH

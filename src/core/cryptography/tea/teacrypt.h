/**
 * The QQ2003C protocol plugin
 *
 * for gaim
 *
 * Copyright (C) 2004 Puzzlebird
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 **************************************************
 * Reorganized by Minmin <csdengxm@hotmail.com>, 2005-3-27
 **************************************************
 */

#ifndef EVACRYPT_H
#define EVACRYPT_H

#define DECRYPT 0x00
#define ENCRYPT 0x01


#include <iostream>
#include <string.h>
using namespace std;


class TEACrypt
{
public:
        TEACrypt();
        ~TEACrypt();
	
	static void encrypt( unsigned char* instr, int instrlen, unsigned char* key,
				unsigned char*  outstr, int* outstrlen_ptr);
	
	static int decrypt(unsigned char* instr, int instrlen, unsigned char* key,
				unsigned char*  outstr, int* outstrlen_ptr);

private:
	static int rand(void);
	static void teaEncipher(unsigned int *const v, const unsigned int *const k, 
				unsigned int *const w);
	static void teaDecipher(unsigned int *const v, const unsigned int *const k, 
				unsigned int *const w);
	
	static int qq_crypt ( unsigned char   flag,  unsigned char*  instr,  int  instrlen,
				unsigned char*  key, unsigned char*  outstr, int* outstrlen_ptr);

//        int stringToByte(unsigned char* destination, string &source);
        void charToByte(unsigned char* destination, const char *source, int sourceLength);
        void byteToChar(char *destination, const unsigned char* source, int sourceLength);

};

#endif

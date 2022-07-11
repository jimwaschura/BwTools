//================================================================================
// BOOST SOFTWARE LICENSE
//
// Copyright 2020 BitWise Laboratories Inc.
// Author.......Jim Waschura
// Contact......info@bitwiselabs.com
//
//Permission is hereby granted, free of charge, to any person or organization
//obtaining a copy of the software and accompanying documentation covered by
//this license (the "Software") to use, reproduce, display, distribute,
//execute, and transmit the Software, and to prepare derivative works of the
//Software, and to permit third-parties to whom the Software is furnished to
//do so, all subject to the following:
//
//The copyright notices in the Software and this entire statement, including
//the above license grant, this restriction and the following disclaimer,
//must be included in all copies of the Software, in whole or in part, and
//all derivative works of the Software, unless such copies or derivative
//works are solely in the form of machine-executable object code generated by
//a source language processor.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
//SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
//FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
//ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//DEALINGS IN THE SOFTWARE.
//================================================================================

#include "pch.h"

#include <string.h>

#include "UtilTrim.h"

char * trim_string(char* buf)
{
	return trim_string_begin(trim_string_end(buf));
}

char * trim_string_begin(char* buf)
{
	int retn = (int)strlen(buf);
	while (retn > 0 && (buf[0] == '"' || buf[0] == ' '))
	{
		strncpy_s(buf, retn+1, buf + 1, retn - 1);
		retn = retn - 1;
		buf[retn] = 0;
	}
	return buf;
}

char * trim_string_end(char* buf)
{
	int retn = (int)strlen(buf);
	buf[retn] = 0;
	while (retn > 0 &&
		(buf[retn - 1] == '\n' ||
			buf[retn - 1] == '\r' ||
			buf[retn - 1] == '\t' ||
			buf[retn - 1] == ' ' ||
			buf[retn - 1] == '"'))
		buf[--retn] = 0;
	return buf;
}


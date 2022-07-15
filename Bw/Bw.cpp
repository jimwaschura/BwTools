// Bw.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


/*
 * bw.c
 *
 *  Created on: Aug 23 2016
 *      Author: Audrey Waschura
 */


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

#include <stdio.h>
#include <stdlib.h>

#include "BwDevice.h"
#include "UtilTrim.h"

static const int DFLT_PORT = 923;
static const int RESPONSE_BUF_SIZE = 65536 ;

static char responseBuffer[RESPONSE_BUF_SIZE];
static void process(BwDevice * Device, int queryFlag, char *prefix, char * buf, int buflen );

int main(int argc, char* argv[])
{
	char* IPAddress = NULL;
	int Port = DFLT_PORT;
	char* InputFileName = NULL;
	char* Prefix = NULL;
	bool helpFlag = false;
	bool verboseFlag = false;
	bool NoCheck = false;
	bool Query = false;
	bool NoPrefix = false;

	/* initialize variables for settings from getenv values */
	/*  BW_PORT, BW_IP */

	size_t len;
	char* envString;

	if (_dupenv_s(&envString, &len, "BW_PORT") == 0)
	{
		int n;
		if (envString != NULL && len>0 && sscanf_s(envString, "%d", &n) == 1)
			Port = n;
		free(envString);
	}

	if (_dupenv_s(&envString, &len, "BW_IP") == 0)
	{
		if (envString != NULL)
			IPAddress = trim_string_quotes(envString);
	}

	if (_dupenv_s(&envString, &len, "BW_PREFIX") == 0)
	{
		if (envString != NULL)
			Prefix = trim_string_quotes(envString);
	}

	/* parse command line options */
	char XmitString[2048];
	XmitString[0] = 0;

	while (*(++argv))
	{
		if (!strcmp(*argv, "-h"))              helpFlag = 1;
		else if (!strcmp(*argv, "-help"))      helpFlag = 1;
		else if (!strcmp(*argv, "-v"))         verboseFlag = 1;
		else if (!strcmp(*argv, "-port"))      Port = atoi(*(++argv));
		else if (!strcmp(*argv, "-p"))         Port = atoi(*(++argv));
		else if (!strcmp(*argv, "-ip"))        IPAddress = *(++argv);
		else if (!strcmp(*argv, "-i"))         IPAddress = *(++argv);
		else if (!strcmp(*argv, "-file"))      InputFileName = *(++argv);
		else if (!strcmp(*argv, "-f"))         InputFileName = *(++argv);
		else if (!strcmp(*argv, "-nocheck"))   NoCheck = true;
		else if (!strcmp(*argv, "-n"))		   NoCheck = true;
		else if (!strcmp(*argv, "-query"))	   Query = true;
		else if (!strcmp(*argv, "-q"))		   Query = true;
		else if (!strcmp(*argv, "-prefix"))	   Prefix = *(++argv);
		else if (!strcmp(*argv, "-x"))		   Prefix = *(++argv);
		else if (!strcmp(*argv, "-noprefix"))  NoPrefix = true;
		else if (!strcmp(*argv, "-nx"))        NoPrefix = true;
		else
		{
			strcat_s(XmitString, 2048, *argv);
			strcat_s(XmitString, 2048, " ");
		}
	}

	trim_string_quotes(XmitString);

	//if (IPAddress == NULL)
	//	IPAddress = (char*) "localhost";

	/* if -h show help and exit */

	if (helpFlag)
	{
		printf("Bw, version 2.0, (c) BitWise Laboratories, Inc.\n");
		printf("Purpose:  Sends automation query or command to BitWise Laboratories\n");
		printf("          device and displays response.\n");
		printf("Usage:    Bw [options] [query_or_command]\n");
		printf("Options:  -h .............. display this message\n");
		printf("          -v .............. display verbose messages\n");
		printf("          -port <N> ....... (or -p) port number, default is %d, or use BW_PORT env variable\n",DFLT_PORT);
		printf("          -ip <addr> ...... (or -i) ip address required, or use BW_IP env variable\n");
		printf("          -file <file> .... (or -f) specify file to read commands from\n");
		printf("          -nocheck......... (or -n) turn on fast mode to skip error checking\n");
		printf("          -query........... (or -q) force command to query a response\n");
		printf("          -prefix <str> ... (or -x) prefix for each command-line Xmit string, or use BW_PREFIX env variable\n");
		printf("          -noprefix ....... (or -nx) ignore any prefix that may be set\n");

		return 0;
	}

	/* if verbose, show current values */

	if (verboseFlag)
	{
		printf("IPAddress.......[%s]\n", (IPAddress == NULL) ? "NULL" : IPAddress);
		printf("Port............%d\n", Port);
		printf("NoCheck.........%d\n", NoCheck);
		printf("NoPrefix........%d\n", NoPrefix);
		printf("Query...........%d\n", Query);
		printf("InputFileName...[%s]\n", (InputFileName == NULL ? "stdin" : InputFileName) );
		printf("Prefix..........[%s]\n",(Prefix==NULL)?"NULL": Prefix);
		printf("XmitString......[%s]\n", XmitString);
	}

	if (IPAddress == NULL)
	{
		fprintf(stderr,"IP Address must be specified on command line or using BW_IP enviornment variable.  Use -h for help.\n");
		return 1;
	}

	/* open socket */

	BwDevice Device;

	Device.setFastMode( NoCheck);

	/* Connect to host */

	Device.Connect( IPAddress, Port);
	int retn = 1; /* error */
	FILE* fd = NULL;

	try
	{
		if (InputFileName != NULL && fopen_s(&fd, InputFileName, "r") != 0 )
			throw "[Unable_To_Open_Input_File]";

		/* have transmission string from command line    */
		/* have file descriptor from -f argument         */
		/* use stdin, and read multiple lines until EOF  */

		if (fd==NULL && XmitString[0]!=0 )
		{
			if (verboseFlag)
				printf("Transmit: %s\n", XmitString);

			char* pre = NoPrefix ? NULL : Prefix;
			process(&Device, Query, pre, XmitString, (int)strlen(XmitString));
		}
		else
		{
			if (fd == NULL)
				fd = stdin;

			char buffer[4096];
			while (fgets(buffer, 4096, fd) != NULL)
			{
				trim_string_quotes(buffer);
				if (buffer[0] != 0)
				{
					if (verboseFlag)
						printf("Transmit: %s\n", buffer);

					process(&Device, Query, NULL, buffer, (int)strlen(buffer));
				}
			}

			if (fd != stdin)
			{
				fclose(fd);
				fd = NULL;
			}
		}

		Device.Disconnect();
		retn = 0;
	}
	catch (const char* msg)
	{
		if (fd != NULL && fd != stdin)
			fclose(fd);

		fprintf(stderr,"Error: %s\n", msg);
		Device.Disconnect();
	}
	catch (...)
	{
		if (fd != NULL && fd != stdin)
			fclose(fd);

		fprintf(stderr,"Unknown error\n");
		Device.Disconnect();
	}

	return retn;
}

static void process(BwDevice * Device, int queryFlag, char* prefix, char* buf, int buflen)
{
	if (prefix == NULL)
		prefix = (char*) "";

	if (queryFlag || (buflen > 0 && buf[buflen - 1] == '?'))
	{
		Device->QueryResponse( responseBuffer, RESPONSE_BUF_SIZE, "%s%s\n", prefix, buf );
		fprintf(stdout, "%s\n", responseBuffer);
	}
	else if (buflen > 0)
	{
		Device->SendCommand( "%s%s\n", prefix,buf);
	}
	else
	{
		fprintf(stderr, "No command entered.\n");
	}
}

/* EOF */

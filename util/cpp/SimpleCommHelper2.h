/* 
 * SimpleCommHelper2.h
 *
 * This file is part of the IHMC Util Library
 * Copyright (c) 1993-2016 IHMC.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 (GPLv3) as published by the Free Software Foundation.
 *
 * U.S. Government agencies and organizations may redistribute
 * and/or modify this program under terms equivalent to
 * "Government Purpose Rights" as defined by DFARS 
 * 252.227-7014(a)(12) (February 2014).
 *
 * Alternative licenses that allow for use within commercial products may be
 * available. Contact Niranjan Suri at IHMC (nsuri@ihmc.us) for details.
 *
 * Author: Giacomo Benincasa    (gbenincasa@ihmc.us)
 * Created on December 18, 2013, 10:45 PM
 */

#ifndef INCL_BASE_COMM_HELPER2_H
#define	INCL_BASE_COMM_HELPER2_H

#include "DArray.h"
#include "Exceptions.h"
#include "LineOrientedReader.h"
#include "Logger.h"
#include "Reader.h"
#include "Writer.h"

#include <stdarg.h>

#pragma warning (disable:4290)

namespace NOMADSUtil
{
    class Socket;

    class SimpleCommHelper2
    {
        public:
            enum Error
            {
                None,
                CommError,
                ProtocolError
            };

            SimpleCommHelper2 (Logger::Level loggingLevel=Logger::L_NetDetailDebug);
            virtual ~SimpleCommHelper2 (void);
            
            // Initialize the CommHelper with the given Socket and an optional
            //     buffer size
            // Assumes that the socket is connected
            int init (Socket *pSocket, unsigned short usLineBufSize = 1024);

            // Initialize the CommHelper with the given Reader and Writer and an
            //     optional buffer size
            // NOTE: It is possible for the Reader or the Writer to be NULL, if the
            //     CommHelper will only be used for one-way operations
            int init (Reader *pReader, Writer *pWriter, unsigned short usLineBufSize = 1024);

            // Get the Socket object being used
            Socket * getSocket (void);

            // Send ui32BufSize number of bytes from pBuf
            // May send less than the requested number of bytes
            // Returns the number of bytes sent
            uint32 send (const void *pBuf, uint32 ui32BufSize, Error &error);

            // Receive upto ui32BufSize number of bytes into pBuf
            // May receive less than the requested number of bytes
            // Returns the number of bytes received
            uint32 receive (void *pBuf, uint32 ui32BufSize, Error &error);

            // Sends ui32BufSize number of bytes from pBuf
            void sendBlob (const void *pBuf, uint32 ui32BufSize, Error &error);

            // Receives ui32BufSize number of bytes into pBuf
            void receiveBlob (void *pBuf, uint32 ui32BufSize, Error &error);

            // Sends a block of binary data. The data block is preceded by the length of the
            // block sent as a 4 byte value
            void sendBlock (const void *pBuf, uint32 ui32BufSize, Error &error);

            // Sends a block of binary data. The data block is preceded by the length of the
            // block sent as a 4 byte value
            void sendBlock (const void *pBuf1, uint32 ui32Buf1Size, const void *pBuf2, uint32 ui32Buf2Size, Error &error);

            // Sends a null-terminated string block of binary data. The data block
            // is preceded by the length of the block sent as a 4 byte value.
            // It automatically checks the length of the string.
            // If pszBuf is null, only the length of the block is sent
            void sendStringBlock (const char *pszBuf, Error &error);

            // Receives a block of binary data. Assumes the other end will send the length of
            // the block as a 4 byte value followed by the block of data.
            // Returns the number of bytes that were read
            // Throws a ProtocolException if the block is bigger than the receiving buffer size
            uint32 receiveBlock (void *pBuf, uint32 ui32BufSize, Error &error);

            // Sends a line (terminated by \r\n) after performing a vsprintf() on pszMsg
            // Arguments must be printf() style and total length of line must not exceed
            //     the line buffer size (specified in init())
            void sendLine (Error &error, const char *pszMsg, ...);

            // Receives a line (terminated by \r\n) upto iBufSize-1 characters into pszBuf
            // Line termination is removed before returning
            // Returns the number of characters in the line
            int receiveLine (char *pszBuf, int iBufSize, Error &error);

            // Similar to receiveLine() above but uses the internal buffer and returns a pointer to the buffer
            const char * receiveLine (Error &error);

            // Receives a line (just like receiveLine) after truncating an initial portion
            // The initial portion to be discarded is determined by performing a vsprintf()
            //     on pszMsg which must be a printf() style format string followed by arguments
            // Throws a ProtocolException if the specified initial portion does not match the
            //     data received
            int receiveRemainingLine (Error &error, char *pszBuf, int iBufSize, const char *pszMsg, ...);

            // Receives a line and compares it with the specified string (which must be printf() style)
            // If received line does not match, throws a ProtocolException
            // Otherwise, received input is quietly discarded
            void receiveMatch (Error &error, const char *pszMsg ...);

            // Receives a line and compares it with a number of specified alternative strings
            // The first argument (usAltCount) must specify the number of alternatives (each of which
            //     must follow as a const char* argument)
            // Returns the zero-based index of the matching alternative
            // Throws a ProtocolException if none of the strings match
            int receiveMatchIndex (Error &error, unsigned short usAltCount, ...);

            // Same as above except that the alternative strings are passed in as a null-terminated array of
            //     const char* values
            int receiveMatchIndex (const char *apszAlternatives[], Error &error);

            // Same as above except that the alternative strings are passed in as an array of const char* values
            //     whose size is specified in the second argument (usCount)
            int receiveMatchIndex (const char *apszAlternatives[], unsigned short usCount, Error &error);

            // A combination of receiveRemainingLine and receiveMatchIndex
            // There must be usAltCount number of strings which are alternatives followed by
            //     a printf() style arguments for the initial string to be ignored
            int receiveRemainingMatchIndex (Error &error, unsigned short usAltCount, ...);

            // Same as above except the string alternatives are passed in as a null-terminated array
            //     of const char* values
            int receiveRemainingMatchIndex (Error &error, const char *apszAlternatives[], const char *pszMsg, ...);

            // Same as above except that the alternative strings are passed in as an array of const char* values
            //     whose size is specified in the second argument (usCount)
            int receiveRemainingMatchIndex (Error &error, const char *apszAlternatives[], unsigned short usCount, const char *pszMsg, ...);

            // Receives a line, parses the line into words, and returns a null-terminated
            //     array of strings, one for each word
            // All whitespace between words is trimmed
            // NOTE: Caller should not delete the array that has been returned
            const char ** receiveParsed (Error &error, int *pCount = NULL);

            // Same as receiveParsed, except that a custom delimiter can be be used
            // NOTE: Caller should not delete the array that has been returned
            const char ** receiveParsedDelimited (Error &error, const char *pszDelimiters, int *pCount = NULL);

            // Same as above except that the parsing is done according to the specified parse format string
            // The parse format string specifies the number of words that should be grouped into one
            //     string in the returned array
            // Specifying a negative number implies that that many words should be ignored
            // Specifying a zero implies that all words from the the current word to the end of the line
            //     should be grouped into one string
            // Any whitespace between words grouped into a single string is retained as is
            // Throws a ProtocolException if the number of words as required by the parse format string
            //     are not found
            // Example:
            //     Parse format string: "-2 1 3 -2 1 0"
            //     Received line: "The quick   brown fox  jumped over the lazy sleeping   dog in   the yard  "
            //     Returned array: [0] "brown"
            //                     [1] "fox  jumped over"
            //                     [2] "sleeping"
            //                     [3] "dog in   the yard"
            //                     [4] NULL
            const char ** receiveParsedSpecific (const char *pszParseFmt, Error &error);

            // Same as receiveParsed except that an initial portion is discarded just like receiveRemainingLine
            const char ** receiveRemainingParsed (Error &error, const char *pszMsg, ...);

            // A combination of receiveRemainingParsed and receiveParsedSpecific
            const char ** receiveRemainingParsedSpecific (Error &error, const char *pszParseFmt, const char *pszMsg, ...);
            void closeConnection (Error &error);

            // returns a reference to the underlying Reader object
            Reader * getReaderRef();

            // returns a reference to the underlying Writer object
            Writer * getWriterRef();

            // Specifies whether the CommHelper should delete the underlying Socket when the CommHelper is deleted
            void setDeleteUnderlyingSocket (bool bDelete);

            // Specifies whether the CommHelper should delete the underlying Reader when the CommHelper is deleted
            void setDeleteUnderlyingReader (bool bDelete);

            // Specifies whether the CommHelper should delete the underlying Writer when the CommHelper is deleted
            void setDeleteUnderlyingWriter (bool bDelete);

        protected:
            int receiveRemainingLine (char *pszBuf, int iBufSize, const char *pszMsg, Error &error, va_list args);
            void stripCRLF (char *pszBuf);

        protected:
            const Logger::Level _loggingLevel;
            Socket *_pSocket;
            LineOrientedReader *_pLOReader;
            Writer *_pWriter;
            bool _bDeleteSocket;    // NOTE: There is no _bDeleteReader because the _pLOReader is instantiated by this class and hence needs to be deleted always
            bool _bDeleteWriter;    //       It is still possible to control the deletion of the pReader that is given to CommHelper - see setDeleteUnderlyingReader()
            char _szErrorMsgBuf[255];
            char *_pszLineBuf;
            unsigned short _usLineBufSize;
            DArray<char*> _parsedTokens;
    };

    inline Socket * SimpleCommHelper2::getSocket (void)
    {
        return _pSocket;
    }
}

#endif	/* BASECOMMHELPER2_H */


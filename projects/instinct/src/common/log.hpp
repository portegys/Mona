/*
 * This software is provided under the terms of the GNU General
 * Public License as published by the Free Software Foundation.
 * Copyright (c) 2003 by the authors, All Rights Reserved.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 */

/*
 * Basic logging.
 */

#ifndef __LOG__
#define __LOG__

#include <stdio.h>
#include <string.h>
#ifdef UNIX
#include <unistd.h>
#endif

#define MESSAGE_SIZE 300
#define LOG_FILE_NAMESZ 100
#define NO_LOG 0
#define LOG_TO_FILE 1
#define LOG_TO_PRINT 2
#define LOG_TO_BOTH 3
#define DEFAULT_LOG_FILE_NAME "default.log"
#define TEMP_LOG_FILE_NAME "temp.log"

class Log
{
    public:

        // Logging flag.
        static int LOGGING_FLAG;

        // Log file name.
        static char logFileNameBuf[LOG_FILE_NAMESZ + 1];
        static char *logFileName;
        static void setLogFileName(char *name);

        // Message composition buffer.
        static char messageBuf[MESSAGE_SIZE + 1];

        // Log error message.
        static void logError(char *message);
        static void logError();

        // Log warning message.
        static void logWarning(char *message);
        static void logWarning();

        // Log debugging message.
        static void logDebug(char *message);
        static void logDebug();

        // Log information message.
        static void logInformation(char *message);
        static void logInformation();

        // Append temporary log.
        static void appendTempLog();

        // Close log.
        static void close();

        // Remove temporary log file.
        static void removeTempLog();

    private:

        static FILE *logfp;
        static bool logOpened;

        // Log a message.
        static void log(char *prefix, char *message);
};
#endif

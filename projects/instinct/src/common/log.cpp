/*
 * This software is provided under the terms of the GNU General
 * Public License as published by the Free Software Foundation.
 * Copyright (c) 2003 by the authors, All Rights Reserved.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 */

/*
 * Basic logging.
 */

#include "log.hpp"

// Logging flag.
int Log::LOGGING_FLAG = LOG_TO_PRINT;

// Log file name.
char Log::logFileNameBuf[LOG_FILE_NAMESZ + 1];
char *Log::logFileName = DEFAULT_LOG_FILE_NAME;

// Message composition buffer.
char Log::messageBuf[MESSAGE_SIZE + 1];

FILE *Log::logfp = NULL;
bool Log::logOpened = false;

void Log::setLogFileName(char *name)
{
    strncpy(logFileNameBuf, name, LOG_FILE_NAMESZ);
    logFileName = logFileNameBuf;
}


// Log error message.
void Log::logError(char *message)
{
    strncpy(messageBuf, message, MESSAGE_SIZE);
    logError();
}


void Log::logError()
{
    log("ERROR: ", messageBuf);
}


// Log warning message.
void Log::logWarning(char *message)
{
    strncpy(messageBuf, message, MESSAGE_SIZE);
    logWarning();
}


void Log::logWarning()
{
    log("WARNING: ", messageBuf);
}


// Log debugging message.
void Log::logDebug(char *message)
{
    strncpy(messageBuf, message, MESSAGE_SIZE);
    logDebug();
}


void Log::logDebug()
{
    log("DEBUG: ", messageBuf);
}


// Log information message.
void Log::logInformation(char *message)
{
    strncpy(messageBuf, message, MESSAGE_SIZE);
    logInformation();
}


void Log::logInformation()
{
    log(NULL, messageBuf);
}


// Append temporary log.
void Log::appendTempLog()
{
    FILE *fp;
    char buf[500];

    if ((fp = fopen(TEMP_LOG_FILE_NAME, "r")) == NULL) return;
    while (fgets(buf, 499, fp) != NULL)
    {
        log(NULL, buf);
    }
    fclose(fp);
}


// Close log.
void Log::close()
{
    if (logOpened && logfp != NULL)
    {
        fclose(logfp);
        logfp = NULL;
    }
    removeTempLog();
}


// Remove temporary log file.
void Log::removeTempLog()
{
    unlink(TEMP_LOG_FILE_NAME);
}


// Log a message.
void Log::log(char *prefix, char *message)
{
    if (LOGGING_FLAG == NO_LOG) return;

    // Open log file?
    if (LOGGING_FLAG == LOG_TO_FILE || LOGGING_FLAG == LOG_TO_BOTH)
    {
        if (!logOpened)
        {
            logOpened = true;
            if ((logfp = fopen(logFileName, "w")) == NULL)
            {
                fprintf(stderr, "Cannot open log file: %s\n", logFileName);
            }
        }
    }

    if (LOGGING_FLAG == LOG_TO_FILE || LOGGING_FLAG == LOG_TO_BOTH)
    {
        if (logfp != NULL)
        {
            if (prefix != NULL)
            {
                fprintf(logfp, "%s%s\n", prefix, message);
            }
            else
            {
                fprintf(logfp, "%s\n", message);
            }
            fflush(logfp);
        }
    }

    if (LOGGING_FLAG == LOG_TO_PRINT || LOGGING_FLAG == LOG_TO_BOTH)
    {
        if (prefix != NULL)
        {
            printf("%s%s\n", prefix, message);
        }
        else
        {
            printf("%s\n", message);
        }
        fflush(stdout);
    }
}

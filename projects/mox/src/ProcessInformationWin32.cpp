// Get process information.

#include "ProcessInformation.h"
#include <windows.h>

/*
 * A helper function for converting FILETIME to a LONGLONG [safe from memory
 * alignment point of view].
 */
static LONGLONG
fileTimeToInt64(const FILETIME *time)
{
   ULARGE_INTEGER _time;

   _time.LowPart  = time->dwLowDateTime;
   _time.HighPart = time->dwHighDateTime;

   return(_time.QuadPart);
}


/*
 * Class:     ProcessInformation
 * Method:    getProcessCPUTime
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_ProcessInformation_getProcessCPUTime(JNIEnv *env, jclass cls)
{
   FILETIME creationTime, exitTime, kernelTime, userTime;
   DWORD    errCode;
   LPVOID   lpMsgBuf;

   BOOL resultSuccessful = GetProcessTimes(GetCurrentProcess(), &creationTime,
                                           &exitTime, &kernelTime, &userTime);

   if (!resultSuccessful)
   {
      errCode = GetLastError();
      FormatMessage(
         FORMAT_MESSAGE_ALLOCATE_BUFFER |
         FORMAT_MESSAGE_FROM_SYSTEM,
         NULL,
         errCode,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
         (LPTSTR)&lpMsgBuf,
         0, NULL);

      fprintf(stderr, "Cannot get CPU time.\n  Error code: %ld\n  Error description: %s\n", errCode, lpMsgBuf);

      LocalFree(lpMsgBuf);
      return(-1);
   }

   return((jlong)((fileTimeToInt64(&kernelTime) + fileTimeToInt64(&userTime)) / 10000));
}

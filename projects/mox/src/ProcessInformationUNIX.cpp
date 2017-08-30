// Get process information.

#ifdef UNIX
typedef long long   __int64;
#endif

#include "ProcessInformation.h"
#include <sys/times.h>

/*
 * Class:     ProcessInformation
 * Method:    getProcessCPUTime
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_ProcessInformation_getProcessCPUTime(JNIEnv *env, jclass cls)
{
   struct tms proctimes;

   times(&proctimes);
   return(proctimes.tms_stime + proctimes.tms_utime);
}

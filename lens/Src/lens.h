/* lens.h: This is the interface to the Lens library */

#ifndef LENS_H
#define LENS_H

#include "system.h"
#ifdef NO_TK
extern flag startLens(char *progName);
#else
extern flag startLens(char *progName, flag batchMode);
#endif
extern flag lens(char *fmt, ...);

// Client access.
extern void     (*clientProc)();
extern int exampleTick;
extern real **netInputs;
extern real **netOutputs;
extern int suppressLensOutput;

#endif /* LENS_H */

#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "system.h"
#include "util.h"
#include "type.h"
#include "extension.h"
#include "command.h"
#include "control.h"
#include "train.h"
#include "lens.h"
#ifndef NO_TK
#include "canvRect.h"
#include <tk.h>
#endif

char Version[32];
char *RootDirBuf;

int initError(char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  beep();
  fprintf(stderr, "Error: ");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
  return TCL_ERROR;
}

int Tcl_AppInit(Tcl_Interp *interp) {

  if (Tcl_Init(interp) == TCL_ERROR)
    return initError("Tcl_Init failed:\n%s\n"
	     "Have you set the LENSDIR environment variable properly?\n"
	     "If not, see Running Lens in the online manual.",
	     Tcl_GetStringResult(interp));

#ifndef NO_TK
  if (!Batch) {
    if (Tk_Init(interp) == TCL_ERROR)
      return initError("Tk_Init failed: %s", Tcl_GetStringResult(interp));
  }
#endif

  if (NoTk == TRUE)
  {
    Root->graphics = 0;
  } else {
    Root->graphics = 1;    
  }
  Tcl_LinkVar(interp, ".NOTK", (char *) &NoTk, TCL_LINK_INT);
  Tcl_LinkVar(interp, ".GUI", (char *) &Gui, TCL_LINK_INT);
  Tcl_LinkVar(interp, ".BATCH", (char *) &Batch, TCL_LINK_INT);
  Tcl_LinkVar(interp, ".CONSOLE", (char *) &Console, TCL_LINK_INT);
#if defined(MACHINE_WINDOWS) || defined(WIN32)
  eval("set .WINDOWS 1");
#else
  eval("set .WINDOWS 0");
#endif /* MACHINE_WINDOWS */

  Tcl_SetVar(interp, ".RootDir", RootDir, TCL_GLOBAL_ONLY);

#ifdef ADVANCED
  Tcl_Eval(interp, "set .ADVANCED 1");
  sprintf(Version, "%f%sa", VERSION, VersionExt);
#else
  Tcl_Eval(interp, "set .ADVANCED 0");
  sprintf(Version, "%f%s", VERSION, VersionExt);
#endif /* ADVANCED */

#ifdef DOUBLE_REAL
  strcat(Version, " dbl");
#endif

  Tcl_SetVar(interp, ".Version", Version, TCL_GLOBAL_ONLY);

  sprintf(Buffer, "%s/Src/shell.tcl", RootDir);
  if (Tcl_EvalFile(interp, Buffer) != TCL_OK)
    return initError("%s\nwhile reading %s/Src/shell.tcl", 
		 Tcl_GetStringResult(interp), RootDir);

  registerCommands();
  registerAlgorithms();
  createObjects();
#ifndef NO_TK
  if (!Batch) createCanvRectType();
#endif
  if (userInit()) return initError("user initialization failed");
  initObjects();

#ifndef NO_TK
  if (!Batch) {
    if (Tcl_Eval(Interp, "wm withdraw ."))
      return initError("error withdrawing main window");
  }

  if (Console) createConsole(TRUE);
#endif

  eval("set _script(path) [pwd]; set _script(file) {}");
  
  Tcl_Eval(Interp, "update");

  return TCL_OK;
}

/* Returns 1 on failure, 0 on success. */
#ifdef NO_TK
flag startLens(char *progName) {
  int tty,len,i;
#else
flag startLens(char *progName, flag batchMode) {
  int tty;
  Gui = FALSE;
  Batch = batchMode;
  if (!Batch) Console = TRUE;
#endif

  if (!(RootDir = getenv("LENSDIR")))
  {
    if ((RootDir = getenv("MONA_HOME")))
	{
	  len = strlen(RootDir);
	  RootDirBuf = (char *)malloc(len + 7);
	  memset(RootDirBuf, 0, len + 7);
#ifdef WIN32
	  if (strncmp(RootDir, "/cygdrive/", 10) == 0)
	  {
	    if (len >= 11)
		{
	      RootDirBuf[0] = RootDir[10];
		  RootDirBuf[1] = ':';
		  if (len >= 12)
		  {
		    strcpy(&RootDirBuf[2], &RootDir[11]);
		  }
		} else {
          return initError("The LENSDIR/MONA_HOME environment variable was not set properly.\n"
            "See Running Lens in the online manual.");
		}
	  } else {
	    strcpy(RootDirBuf, RootDir);
	  }
	  len = strlen(RootDirBuf);
	  if (RootDirBuf[len - 1] == '/' || RootDirBuf[len - 1] == '\\')
	  {
		strcat(RootDirBuf, "lens");
	  } else {
	    strcat(RootDirBuf, "/lens");
	  }
	  len = strlen(RootDirBuf);
	  for (i = 0; i < len; i++)
	  {
		if (RootDirBuf[i] == '/') RootDirBuf[i] = '\\';
	  }
#else
	  strcpy(RootDirBuf, RootDir);
	  len = strlen(RootDirBuf);
	  if (RootDirBuf[len - 1] == '/' || RootDirBuf[len - 1] == '\\')
	  {
		strcat(RootDirBuf, "lens");
	  } else {
	    strcat(RootDirBuf, "/lens");
	  }
#endif
	  RootDir = RootDirBuf;
	} else {
#ifdef WIN32
      RootDir = ".\\lens";
#else
      RootDir = "./lens";
#endif
    }
  }
#ifdef WIN32
  else {
	  len = strlen(RootDir);
	  RootDirBuf = (char *)malloc(len + 2);
	  memset(RootDirBuf, 0, len + 2);
	  if (strncmp(RootDir, "/cygdrive/", 10) == 0)
	  {
	    if (len >= 11)
		{
	      RootDirBuf[0] = RootDir[10];
		  RootDirBuf[1] = ':';
		  if (len >= 12)
		  {
		    strcpy(&RootDirBuf[2], &RootDir[11]);
		  } else {
			  RootDirBuf[2] = '\\';
		  }
		} else {
          return initError("The LENSDIR/MONA_HOME environment variable was not set properly.\n"
            "See Running Lens in the online manual.");
		}
	  } else {
	    strcpy(RootDirBuf, RootDir);
	  }
	  len = strlen(RootDirBuf);
	  for (i = 0; i < len; i++)
	  {
		if (RootDirBuf[i] == '/') RootDirBuf[i] = '\\';
	  }
	  RootDir = RootDirBuf;
  }
#endif

  initializeSimulator();
  
  Tcl_FindExecutable(progName);
  if (!(Interp = Tcl_CreateInterp())) 
    return initError("Lens couldn't create the interpreter");
  
  /* Set the "tcl_interactive" variable. */
  tty = isatty(0);
  Tcl_SetVar(Interp, "tcl_interactive",
	     (tty) ? "1" : "0", TCL_GLOBAL_ONLY);
  
  if (Tcl_AppInit(Interp) != TCL_OK)
    return initError("Lens initialization failed");
  
  return 0;   /* Needed only to prevent compiler warning. */
}

flag lens(char *fmt, ...) {
  char *command;
  flag result;
  va_list args;
  if (fmt == Buffer) {
    error("lens() called on Buffer, report this to Doug");
    return TCL_ERROR;
  }
  va_start(args, fmt);
  vsprintf(Buffer, fmt, args);
  command = copyString(Buffer);
  result = Tcl_EvalEx(Interp, command, -1, TCL_EVAL_GLOBAL);
  va_end(args);
  FREE(command);
  if (result) 
    fprintf(stderr, "Lens Error: %s\a\n", Tcl_GetStringResult(Interp));
  return result;
}

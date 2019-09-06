// Load and store functions.

#ifndef __FILEIO__
#define __FILEIO__

#include "common.h"

#define FREAD_INT myfreadInt
#define FWRITE_INT myfwriteInt
#define FREAD_LONG myfreadLong
#define FWRITE_LONG myfwriteLong
#define FREAD_DOUBLE myfreadDouble
#define FWRITE_DOUBLE myfwriteDouble
#define FREAD_BOOL myfreadBool
#define FWRITE_BOOL myfwriteBool
#define FREAD_STRING myfreadString
#define FWRITE_STRING myfwriteString
void myfreadInt(int *, FILE *);
void myfwriteInt(int *, FILE *);
void myfreadLong(unsigned long *, FILE *);
void myfwriteLong(unsigned long *, FILE *);
void myfreadDouble(double *, FILE *);
void myfwriteDouble(double *, FILE *);
void myfreadBool(bool *, FILE *);
void myfwriteBool(bool *, FILE *);
void myfreadString(char *, int, FILE *);
void myfwriteString(char *, int, FILE *);
#endif

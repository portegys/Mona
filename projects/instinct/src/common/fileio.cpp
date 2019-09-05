// Load and save in binary or readable format.

#include "fileio.h"

void myfreadInt(int *i, FILE *fp)
{
    #ifdef BINARY_FILE_FORMAT
    assert(fread(i, sizeof(int), 1, fp) == 1);
    #else
    fscanf(fp, "%d", i);
    #endif
}


void myfwriteInt(int *i, FILE *fp)
{
    #ifdef BINARY_FILE_FORMAT
    assert(fwrite(i, sizeof(int), 1, fp) == 1);
    #else
    fprintf(fp, "%d\n", *i);
    #endif
}


void myfreadLong(unsigned long *l, FILE *fp)
{
    #ifdef BINARY_FILE_FORMAT
    assert(fread(l, sizeof(unsigned long), 1, fp) == 1);
    #else
    fscanf(fp, "%d", l);
    #endif
}


void myfwriteLong(unsigned long *l, FILE *fp)
{
    #ifdef BINARY_FILE_FORMAT
    assert(fwrite(l, sizeof(unsigned long), 1, fp) == 1);
    #else
    fprintf(fp, "%d\n", *l);
    #endif
}


void myfreadDouble(double *d, FILE *fp)
{
    #ifdef BINARY_FILE_FORMAT
    assert(fread(d, sizeof(double), 1, fp) == 1);
    #else
    char buf[100];
    fscanf(fp, "%s", buf);
    *d = atof(buf);
    #endif
}


void myfwriteDouble(double *d, FILE *fp)
{
    #ifdef BINARY_FILE_FORMAT
    assert(fwrite(d, sizeof(double), 1, fp) == 1);
    #else
    fprintf(fp, "%f\n", *d);
    #endif
}


void myfreadBool(bool *b, FILE *fp)
{
    #ifdef BINARY_FILE_FORMAT
    assert(fread(b, sizeof(bool), 1, fp) == 1);
    #else
    int v;
    fscanf(fp, "%d", &v);
    if (v == 1) *b = true; else *b = false;
    #endif
}


void myfwriteBool(bool *b, FILE *fp)
{
    #ifdef BINARY_FILE_FORMAT
    assert(fwrite(b, sizeof(bool), 1, fp) == 1);
    #else
    if (*b) fprintf(fp, "1\n"); else fprintf(fp, "0\n");
    #endif
}


void myfreadString(char *str, int size, FILE *fp)
{
    #ifdef BINARY_FILE_FORMAT
    assert(fread(str, size, 1, fp) == 1);
    #else
    char *buf = new char[size + 1];
    assert(buf != NULL);
    fscanf(fp, "%s", buf);
    for (int i = 0; i < size; i++)
    {
        if (buf[i] == '`') buf[i] = ' ';
    }
    strncpy(str, buf, size);
    str[size - 1] = '\0';
    delete buf;
    #endif
}


void myfwriteString(char *str, int size, FILE *fp)
{
    #ifdef BINARY_FILE_FORMAT
    assert(fwrite(str, size, 1, fp) == 1);
    #else
    size = strlen(str) + 1;
    char *buf = new char[size];
    assert(buf != NULL);
    for (int i = 0; i < size; i++)
    {
        buf[i] = str[i];
        if (str[i] == ' ') buf[i] = '`';
    }
    fprintf(fp, "%s\n", buf);
    delete buf;
    #endif
}

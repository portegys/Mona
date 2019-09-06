// Randomize strings.

#ifdef WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_NAMES 100
#define MAX_NAME_LENGTH 100
#define NUM_SWAPS 50

int main(int argc, char *argv[])
{
    char *names[MAX_NAMES];
    char buf[MAX_NAME_LENGTH];
    char *temp;
    int num = 0;
    int i,j,k;

    while (gets(buf) != NULL && num < MAX_NAMES)
    {
        names[num] = new char[strlen(buf)+1];
        strcpy(names[num], buf);
        num++;
    }

    srand(time(NULL));
    for (i = 0; i < NUM_SWAPS; i++)
    {
        j = rand() % num;
        k = rand() % num;
        if (j == k) continue;
        temp = names[j];
        names[j] = names[k];
        names[k] = temp;
    }

    for (i = 0; i < num; i++)
    {
        printf("%s\n", names[i]);
    }

    return 0;
}

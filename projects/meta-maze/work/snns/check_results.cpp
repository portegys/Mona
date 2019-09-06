/*

Check SNNS (Stuttgart Neural Network Simulator)
results file. Print score as: "<correct> / <total>".
A correct result is defined as a pattern of output
values that completely matches the target values.

Usage: check_results <SNNS results (.res) file>

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define LINESZ 500
#define NUMSZ 50

int main(int argc, char *argv[])
{
    int i,j,k,total,correct;
    bool ok,eof;
    int targets[5];
    float outputs[5];
    char *vals[5];
    FILE *fp;
    char buf[LINESZ + 1];
    char *resFile;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <SNNS results (.res) file>\n", argv[0]);
        return 1;
    }
    resFile = argv[1];

    if ((fp = fopen(resFile, "r")) == NULL)
    {
        fprintf(stderr, "Cannot open file %s\n", resFile);
        return 1;
    }

    // Analyze results.
    total = correct = 0;
    for (i = 0; i < 5; i++)
    {
        vals[i] = new char[NUMSZ];
    }
    eof = false;
    if (fgets(buf, LINESZ, fp) == NULL) eof = true;
    while (!eof)
    {
        // Expect beginning of a pattern.
        if (strncmp(buf, "SNNS result file", 16) != 0) break;

        for (i = 0; i < 9; i++)
        {
            if (fgets(buf, LINESZ, fp) == NULL) { eof = true; break; }
        }
        if (eof) break;

        // Assess pattern.
        ok = true;
        while (true)
        {
            if (fgets(buf, LINESZ, fp) == NULL) { eof = true; break; }
            if (strncmp(buf, "SNNS result file", 16) == 0) break;
            if (fgets(buf, LINESZ, fp) == NULL) { eof = true; break; }
            if (fgets(buf, LINESZ, fp) == NULL) { eof = true; break; }
            sscanf(buf, "%d %d %d %d %d", &targets[0], &targets[1], &targets[2], &targets[3], &targets[4]);
            if (fgets(buf, LINESZ, fp) == NULL) { eof = true; break; }
            sscanf(buf, "%s %s %s %s %s", vals[0], vals[1], vals[2], vals[3], vals[4]);
            for (i = 0; i < 5; i++)
            {
                outputs[i] = atof(vals[i]);
            }
            for (i = 0, j = -1; i < 5; i++)
            {
                if (targets[i] == 1)
                {
                    if (j == -1)
                    {
                        j = i;
                    }
                    else
                    {
                        j = -1;
                        break;
                    }
                }
            }
            if (j == -1) continue;
            for (i = 0, k = -1; i < 5; i++)
            {
                if (k == -1 || outputs[i] > outputs[k])
                {
                    k = i;
                }
            }
            if (j != k) ok = false;
        }
        if (ok) correct++;
        total++;
    }
    printf("%d / %d\n", correct, total);
    fclose(fp);
    return 0;
}

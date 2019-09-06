// Average numbers in a file.

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define LINESZ 100

main(int argc, char *argv[])
{
    int c;
    double sum;
    FILE *fp;
    char buf[LINESZ + 1];

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <numbers file>\n", argv[0]);
        return 1;
    }

    if ((fp = fopen(argv[1], "r")) == NULL)
    {
        fprintf(stderr, "Cannot open file %s\n", argv[1]);
        return 1;
    }

    c = 0;
    sum = 0.0;
    while (fgets(buf, LINESZ, fp) != NULL)
    {
        sum += atof(buf);
        c++;
    }
    fclose(fp);

    if (c > 0)
    {
        printf("%f\n", sum / (double)c);
    }
    else
    {
        printf("0.0\n");
    }

    return 0;
}

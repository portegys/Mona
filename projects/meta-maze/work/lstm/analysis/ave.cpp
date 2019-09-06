// Average numbers in an SNNS results file.

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define LINESZ 100

main(int argc, char *argv[])
{
    int i,j;
    char c;
    double correct,sum;
    FILE *fp;
    char buf[LINESZ + 1];

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <results file>\n", argv[0]);
        return 1;
    }

    if ((fp = fopen(argv[1], "r")) == NULL)
    {
        fprintf(stderr, "Cannot open file %s\n", argv[1]);
        return 1;
    }

    correct = sum = 0.0;
    while (fscanf(fp, "%d %c %d", &i, &c, &j) == 3)
    {
      correct += (double)i;
      sum += (double)j;
    }
    fclose(fp);
    
    if (sum > 0.0)
    {
      printf("%f\n", correct / sum);
    } else {
      printf("0.0\n");
    }

    return 0;
}

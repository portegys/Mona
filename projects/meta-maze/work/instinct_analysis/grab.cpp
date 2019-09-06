// Grab data from instinct evolve log.

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define LINESZ 100

main(int argc, char *argv[])
{
    int generation,i,j,k,c;
    double fitness,prevfitness;
    bool grab;
    FILE *fp;
    char buf[LINESZ + 1];

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <instinct evolve log file>\n", argv[0]);
        return 1;
    }

    if ((fp = fopen(argv[1], "r")) == NULL)
    {
        fprintf(stderr, "Cannot open file %s\n", argv[1]);
        return 1;
    }

    // Grab fitness of generations.
    generation = 0;
    grab = false;
    c = 0;
    while (fgets(buf, LINESZ, fp) != NULL)
    {
        if (strncmp(buf, "Mutate:", 6) == 0)
        {
            grab = false;
            if (i > 0)
            {
                fitness /= (double)i;
            }
            if ((c % 50) == 1 || c == 1000)
            {
                fitness -= 21.0;
                if (fitness < 0.0) fitness = 0.0;
                fitness *= 2.0;
                if (fitness > 100.0) fitness = 100.0;
                fitness /= 100.0;
                if (c > 1) {
                printf("%d %f\n", generation, (fitness + prevfitness) / 2.0);
                } else {
                printf("%d %f\n", generation, fitness);
                }
                prevfitness = fitness;
            }
            generation++;
        }

        if (grab)
        {
            for (j = 0; j < LINESZ; j++)
            {
                if (buf[j] == '=') break;
            }
            j++;
            for (k = j; k < LINESZ; k++)
            {
                if (buf[k] == ',') break;
            }
            buf[k] = '\0';
            fitness += atof(&buf[j]);
            i++;
        }

        if (strncmp(buf, "Prune:", 6) == 0 ||
            strncmp(buf, "Retain:", 7) == 0 ||
            strncmp(buf, "Select:", 7) == 0)
        {
            grab = true;
            i = 0;
            fitness = 0.0;
            c++;
        }
    }
    fclose(fp);

    return 0;
}

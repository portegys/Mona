#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#include "../Src/lens.h"
}
#endif

/*
 * Without TK graphics:
 * Usage: testLens
 * Build:
 * gcc -o testLens -I../Src -DNO_TK testLens.cpp -L../Bin -llens2.63 -ltcl -lm
 *
 * With TK:
 * Usage: testLens [batch]
 * Build:
 * gcc -o testLens -I../Src testLens.cpp -L../Bin -llens2.63 -ltk -ltcl -lm -lX11
 *
 * Note: export LENSDIR to lens root directory before running.
 */

int main(int argc, char *argv[]) 
{
  char line[1024];
#ifdef NO_TK
  if (startLens(argv[0])) {
#else
  bool batch = false;
  if (argc == 2 && strcmp(argv[1], "batch") == 0)
  {
    batch = true;
  }
  if (startLens(argv[0], batch)) {
#endif
    fprintf(stderr, "Lens Failed\n");
    return 1;
  }
  //lens("source ../Examples/digits.in");
#ifdef NO_TK
  while (fgets(line, 1024, stdin)) lens(line);
#else
  if (batch)
  {
    while (fgets(line, 1024, stdin)) lens(line);
  } else {
    lens("wait");
  }
#endif
  return 0;
}



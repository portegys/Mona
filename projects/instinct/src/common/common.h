// Common header

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <list>
#include <vector>
#include <stack>

#define DONT_CARE (-1)
#define TRUE 1
#define FALSE 0

#define NEARLY_ZERO 0.00001

#define MAX(x, y) ((x > y) ? x : y)

#include "fileio.h"
#include "random.hpp"
#include "valueset.hpp"

typedef unsigned long TIME;
#define INVALID_TIME 0xffffffffUL
TIME gettime();

using namespace std;

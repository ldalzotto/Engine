#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>

#include "sqlite3.h"

#if __DEBUG

#define __MEMLEAK 1
#define __TOKEN 1

#elif __RELEASE

#define __MEMLEAK 0
#define __TOKEN 0

#endif
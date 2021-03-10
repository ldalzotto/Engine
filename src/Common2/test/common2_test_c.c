
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>

#include "sqlite3.h"

#include "Common2/Types/types.h"
#include "Common2/Macros/macros.h"
#include "Common2/Memory/limits.h"

#include "Common2/Include/platform_include.h"

#include "Common2/Clock/clock.h"
#include "Common2/Thread/thread.h"

#include "Common2/Functional/assert.h"

#include "Common2/Memory/token.h"

#include "Common2/Memory/memory.h"
#include "Common2/Memory/slice.h"

Token_declare(uimax);
Slice_declare(uimax);
Slice_declare_functions(uimax);
// #include "Common2/common2c.h"

int main()
{
    uimax l_arr[3] = {8,23,6};
    SliceC(uimax) l_slice = SliceC_build(uimax)(10, l_arr);
    // Slice_ l_s = Slice__build(10);
}

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

SliceN_declare(uimax, 10);
SliceN_declare_functions(uimax, 10);

int main()
{
    uimax l_arr[3] = {8, 23, 6};
    SliceC(uimax) l_slice = SliceC_build(uimax)(10, l_arr);
    SliceN_uimax_10 l_s = {14, 58};
    Slice_uimax l_slice_m = SliceN_uimax_10_to_slice(&l_s);
    uimax* l_val = Slice_uimax_get(&l_slice_m, 1);
    assert_true(*l_val == *SliceN_uimax_10_get(&l_s, 1));
    SliceN_uimax_10_get(&l_s, 9);
}
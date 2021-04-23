#pragma once

#ifndef RB_ASSERT
#   include <cassert>
#   define RB_ASSERT(cond, msg) assert((cond) && msg)
#endif

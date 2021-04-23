#pragma once

#ifndef RB_ASSERT
#   include <cassert>
#   define RB_ASSERT(cond, msg) assert((cond) && msg)
#endif

#ifndef RB_MAYBE_UNUSED
#   define RB_MAYBE_UNUSED [[maybe_unused]]
#endif

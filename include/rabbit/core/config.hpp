#pragma once

#ifndef RB_ASSERT
#   include <cassert>
#   define RB_ASSERT(cond, msg) assert((cond) && msg)
#endif

#ifndef RB_MAYBE_UNUSED
#   define RB_MAYBE_UNUSED [[maybe_unused]]
#endif

#ifndef RB_NOVTABLE
#   ifdef _MSC_VER
#       define RB_NOVTABLE __declspec(novtable)
#   else
#       define RB_NOVTABLE
#   endif
#endif

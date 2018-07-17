// -*-c++-*-
#pragma once
#include <cassert>

#define _SFS_DEBUG 0

#if _SFS_DEBUG == 1
#define SFS_DEBUG 1
#define DASSERT(_X) assert(_X)
#else
#define DASSERT(_X)
#endif

#undef _SFS_DEGUG

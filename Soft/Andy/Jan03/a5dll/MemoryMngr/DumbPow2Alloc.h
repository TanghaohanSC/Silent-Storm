// silent-storm-port 2026-05-11: rewritten to defer to std <new>.
// Original Nival header guarded with _NEW_/_INC_NEW to hide the real
// <new> and redefined std::bad_alloc, nothrow_t, new_handler from scratch.
// Modern MSVC vcruntime_new.h uses different include guards, so both
// definitions ended up active and clashed (C2011 nothrow_t, C2084 op new,
// C3615 constexpr op new, C2504 bad_alloc base undefined, etc.).
//
// Modern C++ permits replacing global operator new/delete by providing
// your own definitions in a .cpp — that's what DumbPow2Alloc.cpp does
// and the standard mechanism still works. Nival's custom std types are
// not needed: the real std::bad_alloc / nothrow_t are equivalent.
#pragma once
#include <new>
#include "malloc.h"
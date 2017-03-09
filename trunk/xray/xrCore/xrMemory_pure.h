#pragma once

#ifdef XRCORE_STATIC
#	define PURE_ALLOC
#else // XRCORE_STATIC
#	ifdef DEBUG
#		define PURE_ALLOC
#	endif // DEBUG
#endif // XRCORE_STATIC

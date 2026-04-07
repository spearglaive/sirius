#pragma once

#if defined(_WIN32)
	#if defined(SIRIUS_EXPORT)
		#define SIRIUS_API [[gnu::dllexport]]
	#else
		#define SIRIUS_API [[gnu::dllimport]]
	#endif
#else 
	#define SIRIUS_API [[gnu::visibility("default")]]
#endif
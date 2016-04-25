#ifndef IBK_BuildFlagsH
#define IBK_BuildFlagsH

#if defined(_MSC_VER)

#if defined(_DEBUG)
		#define IBK_DEBUG
	#else
		#undef IBK_DEBUG
	#endif // _DEBUG

#elif defined(__BORLANDC__)

	#ifdef _DEBUG
		#define IBK_DEBUG
	#else
		#undef IBK_DEBUG
	#endif // _DEBUG

#elif defined(__GNUC__)

	#if !defined(NDEBUG)
		#define IBK_DEBUG
	#else
		#undef IBK_DEBUG
	#endif // DEBUG

#else
	#error Define this for your compiler
#endif

#ifdef IBK_DEBUG
	#undef IBK_DEPLOYMENT
#else
/*! If defined, all applications/libraries based on IBK library will build in Deployment mode.
	\warning All files accessing the IBK_DEPLOYMENT flag need to include this file!!!

	!!!! This flag must only be modified by Stefan and/or Andreas !!!!!

	This flag must not be committed in enabled state to the subversion repository.
*/

// DO NOT CHANGE THE FORMAT OF THE FOLLOWING LINE, IT IS RECOGNIZED BY THE RELEASE SCRIPTS
//#define IBK_DEPLOYMENT

#endif // IBK_DEBUG


/*! \file IBK_BuildFlags.h
	\brief Build-related defines and macros.
*/

#endif // IBK_BuildFlagsH

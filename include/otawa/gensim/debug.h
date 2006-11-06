#ifndef _DEBUG_H_
#define _DEBUG_H_


#if defined(DEBUG) || defined(GENSIM_TRACE)
#	define TRACE(x) x
#else
#	define TRACE(x)
#endif

#endif //_DEBUG_H_

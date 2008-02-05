/*
 *	$Id$
 *	Copyright (c) 2007, IRIT-UPS <casse@irit.fr>.
 *
 *	AbstractCacheDriver class interface
 */
#ifndef OTAWA_SIM_ABSTRACT_DRIVER_H
#define OTAWA_SIM_ABSTRACT_DRIVER_H

#include <otawa/sim/CacheDriver.h>

namespace otawa {

// External classes
namespace hard {
	class Cache;
} // hard

namespace sim {

// AbstractCacheDriver class
class AbstractCacheDriver: public CacheDriver {
public:	
	typedef unsigned long tag_t;
	AbstractCacheDriver(const hard::Cache *cache);
	virtual ~AbstractCacheDriver(void); 
	virtual result_t access(address_t address, size_t size, action_t action);
	inline const hard::Cache *cache(void) const { return _cache; }
	static CacheDriver *lookup(const hard::Cache *cache);

protected:
	virtual void touch(int index, int num, tag_t *line) = 0;
	virtual void replace(tag_t tag, int num, tag_t *line) = 0;

private:
	const hard::Cache *_cache;
	tag_t *lines;
};

// LRUCacheDriver class
class LRUCacheDriver: public AbstractCacheDriver {
public:
	LRUCacheDriver(const hard::Cache *cache);

protected:
	virtual void touch(int index, int num, tag_t *line);
	virtual void replace(tag_t tag, int num, tag_t *line);
};

// DirectMappedCacheDriver class
class DirectMappedCacheDriver: public AbstractCacheDriver {
public:
	DirectMappedCacheDriver(const hard::Cache *cache);

protected:
	virtual void touch(int index, int num, tag_t *line);
	virtual void replace(tag_t tag, int num, tag_t *line);
};

// FIFOCacheDriver class
class FIFOCacheDriver: public AbstractCacheDriver {
public:
	FIFOCacheDriver(const hard::Cache *cache);
	~FIFOCacheDriver(void);

protected:
	virtual void touch(int index, int num, tag_t *line);
	virtual void replace(tag_t tag, int num, tag_t *line);

private:
	int *counters;
};

} } // otawa::sim

#endif	// OTAWA_SIM_ABSTRACT_DRIVER_H

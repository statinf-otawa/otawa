/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	prog/hardware_CacheConfiguration.h -- CacheConfiguration class implementation.
 */

#include <elm/assert.h>
#include <elm/serial2/XOMUnserializer.h>

#include <otawa/hard/CacheConfiguration.h>
#include <otawa/proc/Processor.h>
#include <otawa/prog/Manager.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa { namespace hard {

/**
 * @class CacheConfiguration
 * @ingroup hard
 * This class represents the full configuration of caches of a processor.
 */


/**
 * Useful constant representing a configuration without cache.
 */
const CacheConfiguration CacheConfiguration::NO_CACHE;


/**
 * @fn CacheConfiguration::CacheConfiguration(const Cache *inst_cache, const Cache *data_cache);
 * Build a new configuration from the passed cache. For creating a no-cache
 * configuration, do not give any cache in argument. If you want to create
 * an unified cache, pass the the same cache as inst_cache and data_cache.
 * @param inst_cache	Instruction cache.
 * @param data_cache	Data cache.
 */


/**
 * @fn const Cache *CacheConfiguration::instCache(void) const;
 * Get the instruction cache.
 * @return	Instruction cache.
 */


/**
 * @fn const Cache *CacheConfiguration::dataCache(void) const;
 * Get the data cache.
 * @return	Data cache.
 */


/**
 * @fn bool CacheConfiguration::hasInstCache(void) const;
 * Check if some instruction is available (in a harvard or an unified
 * architecture).
 * @return	True if instruction cache is available, false else.
 */


/**
 * @fn bool CacheConfiguration::hasDataCache(void) const;
 * Check if some data is available (in a harvard or an unified architecture).
 * @return	True if a data cache data is available, false else.
 */


/**
 * @fn bool CacheConfiguration::isUnified(void) const;
 * Check if the cache ois unified.
 * @return True if the cache is unified, false else.
 */


/**
 * @fn bool CacheConfiguration::isHarvard(void) const;
 * Check if the cache follows the Harvard architecture.
 * @return True if the cache follows the Harvard architecture, false else.
 */


/**
 * Load a cache configuration from the given element.
 * @param element	Element to load from.
 * @return			Built cache configuration.
 */
CacheConfiguration *CacheConfiguration::load(elm::xom::Element *element) {
	elm::serial2::XOMUnserializer unserializer(element);
	CacheConfiguration *conf = new CacheConfiguration();
	try {
		unserializer >> *conf;
		return conf;
	}
	catch(elm::Exception& exn) {
		delete conf;
		throw exn;
	}
}


/**
 * Load a cache configuration from an XML file.
 * @param path	Path to the file.
 * @return		Built cache configuration.
 */
CacheConfiguration *CacheConfiguration::load(const elm::sys::Path& path) {
	elm::serial2::XOMUnserializer unserializer(path);
	CacheConfiguration *conf = new CacheConfiguration();
	try {
		unserializer >> *conf;
		return conf;
	}
	catch(elm::Exception& exn) {
		delete conf;
		throw exn;
	}
}


/**
 * Build a cache configuration.
 * @param icache	Instruction cache.
 * @param dcache	Data cache.
 * @note	To get an unified cache, use the same value for icache and dcache.
 */
CacheConfiguration::CacheConfiguration(const Cache *inst_cache, const Cache *data_cache): icache(inst_cache), dcache(data_cache) {
}


/**
 */
CacheConfiguration::~CacheConfiguration(void) {
}


/**
 * Compute name of the cache.
 * @param cache		Cache to get name for.
 * @return			Cache name.
 */
string CacheConfiguration::cacheName(const Cache *cache) const {
	int level = 1;

	// traverse cache hierarchy
	const Cache *icache = instCache(), *dcache = dataCache();
	while(icache != cache && dcache != cache) {
		if(!icache && !dcache)
			return "unknown";
		if(icache)
			icache = icache->nextLevel();
		if(dcache)
			dcache = dcache->nextLevel();
	}

	// build the name
	StringBuffer buf;
	buf << 'L' << level;
	if(!icache)
		buf << " data";
	else if(!dcache)
		buf << " instruction";
	else
		buf << " unified";
	buf << " cache";
	return buf.toString();
}


/**
 * Look for a cache configuration to provide.
 * The following items are looked in its configuration property list (in the list order):
 * @li @ref CACHE_CONFIG
 * @li @ref CACHE_CONFIG_ELEMENT (to unserialize it from the given XML element)
 * @li @ref CACHE_CONFIG_PATH (to load it from the given path).
 *
 * @par Provided Features
 * @li @ref CACHE_CONFIGURATION_FEATURE
 */
class CacheConfigurationProcessor: public otawa::Processor {
public:
	static p::declare reg;
	CacheConfigurationProcessor(p::declare& r = reg): Processor(r), caches(nullptr), xml(nullptr), to_free(false) { }

	virtual void configure(const PropList& props) {
		Processor::configure(props);
		caches = CACHE_CONFIG(props);
		if(!caches) {
			xml = CACHE_CONFIG_ELEMENT(props);
			if(!xml)
				path = CACHE_CONFIG_PATH(props);
		}
	}

	void *interfaceFor(const AbstractFeature& feature) override {
		return const_cast<CacheConfiguration *>(caches);
	}

	void destroy(WorkSpace *ws) override {
		if(to_free)
			delete caches;
		caches = nullptr;
		to_free = false;
	}

protected:
	virtual void processWorkSpace(WorkSpace *ws) {
		if(caches) {
			if(logFor(LOG_DEPS))
				log << "\tcustom cache configuration\n";
		}
		else if(xml) {
			caches = CacheConfiguration::load(xml);
			to_free = true;
			if(logFor(LOG_DEPS))
				log << "\t cache configuration from XML element\n";
		}
		else if(path) {
			if(logFor(LOG_DEPS))
				log << "\t cache configuration from \"" << path << "\"\n";
			caches = CacheConfiguration::load(path);
			to_free = true;
		}
		else if(logFor(LOG_DEPS))
			log << "\tno cache configuration\n";
	}

private:
	const CacheConfiguration *caches;
	xom::Element *xml;
	Path path;
	bool to_free;
};


p::declare CacheConfigurationProcessor::reg = p::init("otawa::CacheConfigurationProcessor", Version(1, 0, 0))
	.provide(CACHE_CONFIGURATION_FEATURE)
	.maker<CacheConfigurationProcessor>();


/**
 * This feature ensures we have obtained the cache configuration
 * of the system. This features is interfaced; this means that you can fetch
 * the cache configuration with the code:
 * <code c++>
 * const CacheConfiguration *config = CACHE_CONFIGURATION_FEATURE.get(workspace);
 * </code>
 *
 * Configuration:
 * @li @ref CACHE_CONFIG -- cache configuration object to use,
 * @li @ref CACHE_CONFIG_ELEMENT -- cache configuration as an XML element,
 * @li @ref CACHE_CONFIG_PATH -- path to the cache configuration descriptor in XML.
 */
p::interfaced_feature<const CacheConfiguration> CACHE_CONFIGURATION_FEATURE("otawa::hard::CACHE_CONFIGURATION_FEATURE", p::make<CacheConfigurationProcessor>());


/**
 * Current configuration.
 *
 * @par Hooks
 * @li @ref otawa::WorkSpace
 *
 * @par Features
 * @li @ref otawa::CACHE_CONFIGURATION_FEATURE
 *
 * @par Default Value
 * Cache configuration without any cache (never null).
 */
Identifier<const CacheConfiguration *> CACHE_CONFIGURATION("otawa::hard::CACHE_CONFIGURATION", &CacheConfiguration::NO_CACHE);


// accessors functions
static const Cache *l1_icache(WorkSpace *ws) {
	const CacheConfiguration *conf = CACHE_CONFIGURATION(ws);
	if(!ws)
		return 0;
	else
		return conf->instCache();
}

static const Cache *l1_dcache(WorkSpace *ws) {
	const CacheConfiguration *conf = CACHE_CONFIGURATION(ws);
	if(!ws)
		return 0;
	else
		return conf->dataCache();
}


/**
 * Accessor on the instruction cache L1.
 *
 * @par Feature
 * @li @ref otawa::CACHE_CONFIGURATION_FEATURE
 */
FunAccessor<const Cache *> L1_ICACHE(l1_icache, "otawa::hard::L1_ICACHE");


/**
 * Accessor on the data cache L1.
 *
 * @par Feature
 * @li @ref otawa::CACHE_CONFIGURATION_FEATURE
 */
FunAccessor<const Cache *> L1_DCACHE(l1_dcache, "otawa::hard::L1_DCACHE");;

} } // otawa::hard

SERIALIZE(otawa::hard::CacheConfiguration)

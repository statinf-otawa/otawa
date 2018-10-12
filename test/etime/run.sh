#!/bin/bash

test -d xgraphs || mkdir xgraphs

#	--add-prop otawa::GRAPHS_OUTPUT_DIRECTORY=xgraphs

operform $1 -P -v \
	require:otawa::ipet::FLOW_FACTS_FEATURE \
	require:otawa::ICACHE_ONLY_CONSTRAINT2_FEATURE \
	require:otawa::WEIGHT_FEATURE \
	process:otawa::etime::AbstractTimeBuilder \
	--add-prop otawa::PROCESSOR_PATH=op1.xml \
	--add-prop otawa::CACHE_CONFIG_PATH=cache.xml \

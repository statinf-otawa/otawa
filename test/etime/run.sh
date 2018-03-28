#!/bin/bash

test -d xgraphs || mkdir xgraphs

operform $1 -P -v \
	require:otawa::ICACHE_ONLY_CONSTRAINT2_FEATURE \
	process:otawa::etime::AbstractTimeBuilder \
	--add-prop otawa::PROCESSOR_PATH=op1.xml \
	--add-prop otawa::CACHE_CONFIG_PATH=cache.xml \
	--add-prop otawa::GRAPHS_OUTPUT_DIRECTORY=xgraphs


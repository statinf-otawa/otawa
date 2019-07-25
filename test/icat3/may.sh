#!/bin/bash

operform -v -P $1 require:otawa::icat3::MAY_ANALYSIS_FEATURE --add-prop otawa::CACHE_CONFIG_PATH=cache.xml

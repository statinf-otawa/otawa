#!/bin/bash
operform ../benchs/bs.elf -P \
	--add-prop otawa::MEMORY_PATH=mem.xml \
	require:otawa::hard::MEMORY_FEATURE \
	process:otawa::hard::Dumper

#!/usr/bin/python3
#
# This script requires SIS sources to be installed (at the same level
# as OTAWA sources).

import os
import re
import stat

map = {
	"APP":			'"OTAWA"',
	"DB_URL":		'"http://tracesgroup.net/otawa/packages/2.0"',
	"DB_CONF":		'"share/Otawa/install.xml"',
	"DEFAULT":		'["otawa"]',
	"DEFAULT_PATH":	'"otawa"'
}
path = "otawa-install.py"

out = open(path, "w")
for line in open("../../sis/sis-install.py").readlines():
	m = re.match(r"^([A-Z_]+)\s*=\s*", line)
	if m and m.group(1) in map:
		out.write("%s = %s\n" % (m.group(1), map[m.group(1)]))
	else:
		out.write(line)

os.chmod(path, stat.S_IXUSR | os.stat(path).st_mode)

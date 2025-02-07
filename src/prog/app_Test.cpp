/*
 *	Test class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2018, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <otawa/app/Test.h>
#include <elm/io/FileInput.h>
#include <elm/io/FileOutput.h>
#include <elm/sys/Path.h>
#include <elm/sys/System.h>
#include <otawa/program.h>

namespace otawa {

/**
 * @class Test
 *
 * This special type of application is dedicated to testing.
 * The process is the following:
 *   * When an analysis is set up, this application is run to generate
 * an output that is considered as the reference output. This output
 * is usually checked by hand.
 *   * When modification is performed to the analysis or OTAWA is ported
 * to a different system, the output is generated again and compared to
 * the reference output. IF they are the same, the check is ok. Else
 * it is considered as an error.
 *
 * The application has a return code corresponding to ok when the check
 * passes and corresponding to an error in the opposite case.
 *
 * The output is produced when the function Test::generate(io::Output& out)
 * is called. To customize the test, one has to create a class extendind
 * Test and to overload Test::generate() and to produce the output
 * on the given out stream.
 *
 * The reference output file names are formed by the binary file name where
 * the extension is replaced by `.ref`. They must be archived
 * with the sources in order to test analysis later. They are obtained
 * by passing option `-r` or `--ref`.
 *
 * The check output file name are also formed by the binary file name
 * where the extension is replaced by `.out` but do not require any
 * option to be generated. They can be removed at any time: they are
 * only kept to be compared with reference in case of error.
 *
 * Both reference and check output files are created in the current
 * directory.
 *
 * Notice that the application is compatible with CMake testing facility.
 *
 * @ingroup application
 */

/**
 * Build the test.
 * @param name	Name of the test.
 */
Test::Test(cstring name)
:	Application(Make(name)),
	ref(option::Switch::Make(*this).cmd("-r").cmd("--ref").description("generate the reference file"))
{ }

/**
 * @fn void Test::generate(void);
 * Override this function to generate the reference output or the output to check.
 * You can use Application functions like Application::workspace() or Application::require()
 * from this function.
 * @param out	Output stream to generate the output to.
 */

/**
 */
void Test::work(const string& entry, PropList &props) {

	// compute the paths
	sys::Path elf_path = workspace()->process()->program_name();
	sys::Path out_path = sys::Path(elf_path.withoutExt().namePart() + ".out");
	sys::Path ref_path = sys::Path(elf_path.withoutExt().namePart() + ".ref");

	// generate a reference
	if(ref) {
		FileOutput out(ref_path);
		generate(out);
		cerr << "Reference created!\n";
		sys::System::exit(0);
	}

	// perform a check
	else {

		// generate the output file
		FileOutput out(out_path);
		generate(out);
		out.flush();

		// perform the comparison
		FileInput out_in(out_path);
		FileInput ref_in(ref_path);
		while(true) {
			string out_line = out_in.scanLine();
			string ref_line = ref_in.scanLine();
			if(out_line != ref_line) {
				cerr << "Test failed!\n";
				sys::System::exit(1);
			}
			if(out_line == "")
				break;
		}
		cerr << "Test passed!\n";

	}

}

} // otawa

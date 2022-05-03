# OTAWA 2.0, code named "Reloaded"

## Information

**OTAWA** (_Open Tool for Adaptive WCET Analysis_) is a framework of C++ classes dedicated to static analyses of programs in machine code and to the computation of WCET (Worst Case Execution Time).

**OTAWA** is freely available (under the LGPL license) and is developed by the TRACES (https://www.irit.fr/TRACES/site/) team at IRIT labs (http://www.irit.fr), université Paul Sabatier, France.

**OTAWA** provides state-of-art WCET analyses like IPET (Implicit Path Enumeration Technique) and a lot of facilities to work on binary programs (control flow graphs, loop detection and so on).

**OTAWA** supports successfully many architecture as PowerPC, ARM 32, Sparc, TriCore, etc.

The official website of **OTAWA** may be found at the URL below:
	http://www.otawa.fr

You may contact us at [otawa@irit.fr](mailto:otawa@irit.fr).

## Installing from sources

Currently, **OTAWA** has two main requirements:
* GEL --  an ELF loading library (https://sourcesup.renater.fr/scm/browser.php?group_id=4453&extra=gel&scm_plugin=scmgit).
* ELM -- portable multi-feature library (https://sourcesup.renater.fr/projects/elm).

These libraries has to be built and installed or may be provided,
without installation, in the same directory as **OTAWA**.

Then the procedure is straight-forward:
```bash
$ cd otawa
$ cmake .
$ make install
```

Basically, **OTAWA** tries to install in your standard file system. To specify a directory, add option `--CMAKE_INSTALL_PREFIX=`_PATH_ to select a specific installation path.

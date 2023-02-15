# OTAWA Developer Manual

[OTAWA](http://www.otawa.fr) is dedicated to the development and experimentation
of Worst Case Execution Time (WCET) computation methods. OTAWA is a framework
composed of a main library, //libotawa//, with several sub-libraries and plugins
that may be automatically invoked by //libotawa//. This makes the framework
very versatile and easy to extend.

The goal of this manual is mainly to describe how extensions can be developed
for OTAWA.  The following extensions are described :
  * *scripts*: to provide a new computation methods or adaptation to new micro-architectures (usually matching a specific microprocessor model).
  * *loaders*: to adapt a new Instruction Set Architecture (ISA),
  * *analyzers* (or code processor in OTAWA terminology): to provide new or more specific analyzes, 
  * *ilp solvers*: to connect OTAWA with a new solver,

Before reading this manual, it is advised to master the OTAWA framework
whose details are given in the [OTAWA manual](../manual/manual.html). It is also
a good idea to have an access to the API documentation (also called autodoc):
prefer the up-to-date auto-documentation delivered with your OTAWA package
although this documentation may be also obtained from [here](http://www.otawa.fr/doku/autodoc).

For any problem or question, you may ask the OTAWA developers from the site [http://www.otawa.fr](http://www.otawa.fr).

[TOC]

# Script Development 

A script, in OTAWA, allows to configure a WCET computation.
It contains :
  * step-by-step code processor invocations,
  * a way to let the user fine-tune the computation thanks to user parameters,
  * description of the involved architecture,
  * possible documentation about the computation (allowing to inform the user about
limitations for example).

The scripts are described in [XML format](http://www.w3.org/XML/) and, therefore,
easy to read and write by a human user. They are mostly the simpler way
to extend OTAWA without the need to understand
the internal API of the framework. In addition, scripts are easy
to write because they use well-known formats based on XML like
[XInclude](http://www.w3.org/TR/2006/REC-xinclude-20061115/)
or [XSLT](http://www.w3.org/TR/xslt20/) that gives a lot of power
in the script behavior.

Scripts have basically two usage. First, they are used to configure the computation
for a particular architecture. Instead of using the main stream computation approach,
they allows to easily perform and automate specific analyzes to fine-tune
a computation. Another use of the script is to provide support in OTAWA
for a new micro-architecture: indeed, they allows to describe the components
of an architecture or a model processor (pipeline, caches, memory space) and
then to invoke all analyzes required to support the architecture. Obviously,
this way to describe a micro-architecture is bound to the feature supported
by OTAWA. For more exotic architectures, you will need to implements plugins
described in the following sections.

In addition, describing a WCET computation allows to seamlessly use it
under the OTAWA plugin inside Eclipse. The plugin has the ability
to understand the script and to translate the configuration
into a user interface form adapted to Eclipse. 

## Notation 

The description of the XML files in this document merges 
[XML](http://www.w3.org/XML/) textual format with
[EBNF](http://fr.wikipedia.org/wiki/Extended_Backus-Naur_Form).

The grammar is formed of a list of rules whose root is the first one.
Each rule is made of:
  * an XML comment giving the name of the rule,
  * the matching XML element possibly containing other elements
giving the shape of the rule (as below).

```xml
<!-- RULE_NAME ::= -->
<tag> ... </tag>
```

This description element may contain a sequence of symbols that may be :
  * other elements (that in turn contains also the same type of item that the rule element),
  * identifiers in uppercase (to represent terminal symbols, see below for the list of accepted terminals),
  * empty XML element to refer to subsequently defined rules (they are replaced in actual files
by their definition element),
  * EBNF symbols (described below).

EBNF symbols allows to repeat elements, make them optional or select alternatives.
They may be :
  * `*` -- repeat 0 or *n* times the previous symbol,
  * `+` -- repeat 1 or *n* times the previous symbol,
  * `?` -- the previous symbol is optional,
  * //sym<sub>1</sub>// `|` //sym<sub>2</sub>// `|` ... -- symbols separated by pipes are alternative, only one is selected at one time,
  * `(` symbols `)` -- parentheses allows to group symbols in order to support previous operators on a group of symbols.

The XML elements attributes are defined at their normal location, after the opening
XML tag. They conform to the usual XML notation except for their content
and their activation. An attribute definition may be followed
by a question mark `?` to denote it as optional. Otherwise, it is considered
as mandatory. The content of the attribute, between simple or double quotes,
supports the EBNF annotations for the contained text.
Finally, an optional attribute defined as an alternative
may provide a default value by appending to the alternative list the string
`; default=` //default value//.

The terminal identifier have the following meaning:
  * `ID` -- an XML identifier (any non-blank sequence of characters),
  * `INT` -- a decimal or hexadecimal (prefixed by `0[xX]`) integer,
  * `TEXT` --  any text,
  * `ADDRESS` -- an address (synonym of `INT`),
  * `BOOL` -- a boolean value (`true` ou `false`).




## Script Format 

A script is a textual XML file whose extension is usually `.osx` for
Otawa Script XML. It follows the usual rule of XML and the top-level element
is called `otawa-script`:

```xml
<!-- OTAWA-SCRIPT ::= -->
  <?xml version="1.0"?>
  <otawa-script
    xmlns:xi="http://www.w3.org/2001/XInclude"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- DESCRIPTION -->

  <!-- IDENTIFICATION -->
  
  <!-- CONFIGURATION -->?
  
  <!-- PLATFORM -->?
  
  <!-- SCRIPT -->?

```

Notice the two namespace declarations `xmlns:`: they are not mandatory
but are very useful if you to use //XInclude// or //XSLT//.

The script is made of 5 different parts detailed below:
  * //DESCRIPTION// provides various information targeting the human user,
  * //IDENTIFICATION// contains mainly identifier about the hardware (architecture, model, ABI),
  * //CONFIGURATION// provides a list of items the user may tune,
  * //PLATFORM// describes the hardware,
  * //SCRIPT// details the performed computation steps.

### Script Description 

The description is made of the following items:
```xml
<!-- DESCRIPTION ::= -->
<name> TEXT </name>
<info> XHTML </info>?
<path to="PATH"/>*
```

The `name` tag is mandatory and provide the name of the script
as displayed to the human user. The `info` element may contain a whole
documentation describing the script, its applications and its limitation.
As it is intended displayed to the human user and as it may contains
a structured documentation, it is described in
[[http://www.w3.org/TR/xhtml1/|XHTML]]. Finally, the `path` is used for
the internal work of the script inside OTAWA. The `to` attributes contain
paths that are used to retrieve plugins used in the WCET computation.
If a relative path is used, it is based on the directory containing
the script file. 

### Script Identification 

The identification part has the following structure:
```xml
<!-- IDENTIFICATION ::= -->
<id>
  <arch>TEXT</arch>
  <abi>TEXT<abi>
  <mach>TEXT</mach>
</id>
```

The `arch` tag allows to identify the progamming model, also called
the ISA (Instruction Set Architecture) of the supported hardware.
Common values include `arm`, `powerpc`, `sparc`, `x86`, etc.
The `abi` element gives the Application Binary Interface with common
values being `eabi`, `elf`, `linux`, etc. Finaly, the `mach`
element allows to precisely identify the processor model the script
is targetting. Only the `arch` element is mandatory to be able
to check if it supports the instruction set used in the processed
executable file.


### Platform Description 

The platform provide details about the hardware feature of the targeted system.

```xml
<!-- PLATFORM :: -->
<platform>
  <!-- PIPELINE -->?
  <!-- CACHES -->?
  <!-- MEMORY -->?
  <!-- BHT -->?
</platform>
```

The items found in the //PLATFORM// may described directly in the script
or in a separate file included by //XInclude//. In the latter case,
the file must be prefixed by the usual XML identification line:

```xml
<?xml version="1.0"?>
```

The example belows uses //XInclude// to get the hardware description
from three different external files:

```xml
<platform>
  <xi:include href="mpc5554/pipeline.xml"/>
  <xi:include href="mpc5554/cache.xml"/>
  <xi:include href="mpc5554/memory.xml"/>
</platform>
```

Notice that the relative paths passed in the `href` attribute
are resolved from the XML base of the document, that is, the directory
containing the script file.

Their content being very complex is described in their own parts.


### Configuration Description 

The configuration lists a set of items to let the human user parameterize
the computation:
```xml
<!-- CONFIGURATION ::= -->
  <configuration>
    <!-- CONFIGURATION-ITEM -->*
  </configuration>

<!-- CONFIGURATION-ITEM ::= -->
  <item
    name="TEXT"
    type="bool|int|string|range|enum"
    default="TEXT"?
    label="TEXT"?>
      <help> <!-- TEXT --> </help>
  </item>
```

A configuration item is made of:
  * an internal `name` used to identify the variable containing their value in //XSLT//,
  * a `label`, the name of the configuration item displayed to the user,
  * a `default` value,
  * a `type` that describes the type of value,
  * an `help` sub-element that contains human-readable information to help the user understanding the configuration item.

In addition, each type of items may have its own set for attributes and sub-elements.

#### `bool` type 

The `bool` type allows to get boolean information from the user.
Possible values are `true` or `false`. They are used to enable
or disable specific features of the script. The example below allows
to activate or not the use of prefetching from a flash memory:
```xml
<item name="flash_prefetch" type="bool" default="true" label="Flash Prefetch">
  <help>MPC5554 provides flash prefetching to improve performances.
  You may activate it or no.</help>
</item>
```

#### `int` type 

The integer type allows to get an integer configuration value from the user.
If no default value is given, it is assumed to be 0. The argument may be expressed
as a decimal integer or hexadecimal one prefixed by `0x` or `0X`.
An integer configuration item is used to pass any integer quantitive value
as a number of functional units in a pipeline description, a specific address
in the memory space or the size of any part of the architecture.

In the example below, the `intt` configuration item is isued to configure the number
of wait states used to write in the static RAM.
```xml
<item name="ramwws" type="int" default="0" label="SRAM write wait states">
<help>Defines the number of wait state for a SRAM write. One wait cycle delays one cycle.</help>
</item>
```

#### `string` type 

This configuration item type is used to pass a string to the script. If no default value
is given, an empty string is assumed. A common usage of this type of item is to pass
a path in the file system to a specific resource used in the computation but any use
of a string is supported.

#### `range` type 

A `range` configuration item is a bit like the `int` type but with bounds on the possible
given value. The bounds are inclusive and given by two additional attributes, `low` and `high`.
The default value must be in the bounds and, if not given, the `low` bound is assumed as default.

The code below shows the `range` in action to define the pre-charge time of a dynamic ram
in the range of [2, 3].
```xml
<item name="trp" type="range" default="2" label="SDRAM precharge time" low="2" high="3">
  <help>In cycles.</help>
</item>
```

#### `enum` type 

The `enum` type allows to support the selection of value from a collection of different values.

The enumerated values are declared with the syntax below:
```xml
<!-- ITEM ::= -->
<item name="TEXT" type="enum" label="TEXT">
  <value label="TEXT" value="INT"  default="BOOL"?/>+
</item>
```

Each value is defined from a `value` XML element with a `label`, to display to the user
and an integer value that represents the value handled by the script if the enumerated value
is selected. In addition, a `default` attribute may be set to indicate if the current value
is the default one. If no `default` attribute is set to true, the first enumeration value
is considered as the default.

The `range` configuration type is useful to display a choice to the user in a textual way
while keeping hidden the associated integer value. The example allows to select a multiplier
implementation in order to compute for a processor delivered as an IP. 

```xml
  <item name="multiplier" type="enum" label="multiplier" default="m32x32">
    <help>Multiplier implementation: defines the multiplier latency.</help>
    <value label="iterative" value="0" default="true"/>
    <value label="m16x16 + pipeline" value="1"/>
    <value label="m16x16" value="2"/>
    <value label="m32x8" value="3"/>
    <value label="m32x16" value="4"/>
    <value label="m32x32" value="5"/>
  </item>
```



### Script Description 


This element describes the script itself, that is, the analyzes to apply to get the WCET.
In fact, the script works directly on the code processor structure of OTAWA that is composed:
  * code processors that implements simple analyzes or may transform the program representation,
  * features are required or provided by code processors and represent information retrieved by the analyzes,
  * properties that are annotations representing results of analyzes, hooked to the program representation and grouped in features.

Code processors, features and properties are documented in the [[http://www.irit.fr/recherches/ARCHI/MARCH/OTAWA//autodoc/group__features.html|automatic documentation of OTAWA]].

In OTAWA, computing the WCET is invoking either the code processor computing the WCET, or requiring the feature provided by this code processor.
In turn, this processor may require other features that will be achieved by other code processors and so on.
The rule is that if an already-provided feature is required, it is used as is. If it is not provided,
the default processor associated to the feature is invoked.

It comes out that the order of feature requirements
or code processor invocations matters! To substitute an analysis *A* to the default analyses *B*  of a feature *F*,
the *A* analysis must be invoked first to let it providing the feature *F*. When a code processor will require
the feature *F*, it will use the feature provided by *A* as it is already available.

Yet, one may observe that, thanks to the default processor associated with a feature, it is not mandatory
to provided the whole chain f analyzes to perform the WCET computation. Instead, the script writer
has only to focus only on the particular analyzes of its script.

The script part has the syntax below and is made of a sequence of steps, possibly with configuration
items that apply to all steps:
```xml
<!-- SCRIPT ::= -->
    <script>
      <!-- CONFIG -->*
      <!-- STEP -->+
    </script>
```

A step may invoke a code processor (attribute `processor`)
or require a feature (attribute `feature`). If a step contains configuration items,
they are only applied to this step and to code processors automatically invoked from
this step.
```xml
<!-- STEP ::= -->
    <step processor="C++PATH"? require="C++PATH"?>
        <!-- CONFIG -->*
    </step>
```

The //C++PATH// used to identify a processor or a feature (but also a property)
is the full-qualified path of the object in the C++ implementation of OTAWA. For example,
if a code processor implementing class is `MyAnalysis` that is contained in namespace
`my` and `otawa`, the matching //C++PATH// is `otawa::my::MyAnalysis`.

The configuration items allows to provide important parameters or to tune the behavior
of the analysis. They matches exactly the properties in OTAWA but only some properties
provide a converter from XML text to C++ values. The syntax is below:
```xml
<!-- CONFIG ::= -->
    <config name="C++PATH" value="TEXT" add="yes|true|no|false"?/>
```

The matching property identifier is retrieved from its //C++PATH// and its `value`
is converted according to the type of the identifier and pushed in the property list
used to configure the code processor. If the `add` attribute is to `yes` or `true`,
the value is added to the property list such that several values with the same identifier
can be added to the configuration property list. 

Below is a small example for the //LPC2138// microprocessor: 
```xml
<script>
    <step require="otawa::VIRTUALIZED_CFG_FEATURE"/>
    <step processor="otawa::lpc2138::CATMAMBuilder"/>
    <step processor="otawa::lpc2138::ARM7ParamExeGraphBBTime">
      <config name="otawa::lpc2138::FLASH_MISS" value="56"/>
    </step>
    <step require="otawa::ipet::WCET_FEATURE"/>
</script>
```

This script requires first the feature `otawa::VIRTUALIZED_CFG_FEATURE` ensures that
all functions calls have been inlined. In fact, this requirement will cause the invocation
of several analyses like the flow fact loader, program text decoder, CFG building, etc.

The the `otawa::lpc2138::CATMAMBuilder` analyzes the prefetcher of the LPC2138 flash memory
and computes the execution of the blocks with `otawa::lpc2138::ARM7ParamExeGraphBBTime`.
In this step, a configuration parameter is passed to configure the time for a memory flash miss.
As will be presented below, the value is rarely a constant: it may be derived from
the configuration variables.

Finally, the WCET computation is required, `otawa::ipet::WCET_FEATURE`, that will
build the ILP system, flowfact constraints but will re-use the block timings already
provided by `otawa::lpc2138::ARM7ParamExeGraphBBTime` without invoking the default
computation of block timings.




## File Organization and XInclude 

Using the //XInclude// XML extension, a script can be made of several different files.
An //XInclude// element looks like:
```xml
  <xi:include href="PATH"/>
```

Where //PATH// is an URL pointing to the file to include. The included file must be
a valid XML file (prefixed by the `<?xml ... >` tag and the top level element
will replaced the `xi:include` element so that the application processing the resulting
XML file does not have to do any more processing.

A common use of this feature is to split the script description in several files, one for the
script, the entry point whose name is suffixed by `.osx` and one for each aspect of
the hardware (pipeline, caches, memory space). The example shows a summary of the entry point
in such an organization:

```xml
<?xml version="1.0"?>
  <otawa-script
    xmlns:xi="http://www.w3.org/2001/XInclude"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
 
    <info> ... </info>?
    <name> ... </name>
  <configuration> ... </configuration>
  <id> ... </id>

  <platform>
    <xi:include href="mpc5554/pipeline.xml"/>
      <xi:include href="mpc5554/cache.xml"/>
      <xi:include href="mpc5554/memory.xml"/>
  </platform>   

  <script> ... </script> 
</otawa-script>
```

The `PATH` attribute of `xi:include` element can contains any type of path,
absolute or relative, but the latter option allows to preserve the consistency
of the script. Indeed, the script interpreter considers that any relative path
is relative to the directory containing the script. Therefore, if the script
is moved in the file system tree, they will be correctly retrieved
if the relative position of the script file and the included file is not changed.
Such a method allows also to deliver easily a package containing a script
without to have to fix the paths of the included file.

In the example above, the script is contained in a file named `mpc5554.osx`
and the included files are found out from a directory named `mpc5554` installed
in the same directory than the entry file `mpc5554.osx`. It is also advised 
to put the included file in a sub-directory and to not suffix these files
with `.osx` in order to not confuse application using the `.osx` files
(as the OTAWA Eclipse plugin).


## Smart Scripts and XSLT 

[[http://www.w3.org/TR/xslt|XSLT]] is an XML-based language to describe templates to perform automatic transformation
on XML files. In the OTAWA scripts, we do not use its templating system but only the interpreter
of its algorithmic components, that is, its capacity to have variables and perform computation
with conditional structure allowing to insert or not XML elements. This section gives
basic commands of the //XSLT// language
but more details can be found in the [[http://www.w3.org/TR/xslt|XSLT]] documentation.

In //XSLT//, the variable are accessed by prefixing them with `$`: an element attribute whose value is "here is my $myvar !"
will get as value the given string with the variable reference `$myvar` replaced by the actual value of //myvar//.
Basically, the available variables, when the script is processed by the //XSLT// interpreter, is made of
the variables declared for the configuration items, named according to the `name` attribute `name` and
whose value is the one passed by the user or, else, the default value.

These variables may be used throughout the script file to provide more flexibility in the configuration of the script.
Using the following syntax, one may change the value stored in a script element:
```xml
<element> <xsl:value-of select="XPATH expression"/> </element>
```

In the example below, the content of the element `write_latency` is replaced
by the value computed bu the `xsl-value-of` element, that is, the sum of 2,
value of variable `trp` and value of variable `sdrcas`.
```xml
<otawa-script>
  <platform>
    <memory>
      ...
      <bank>
        <write_latency><xsl:value-of select="2+$trp+$sdrcas"/></write_latency>
      </bank>
      ...
    </meory>
  </platform>
```

The [[http://www.w3.org/TR/xpath20/|XPATH]] expression (follow the link for more details)
is very versatile and provide usually operators to perform computation:
  * `$`//name// to access a variable content,
  * `(` and `)`, parentheses,
  * usually unary and binary operators like `+`, `-`, `*`, `/`, etc
  * and a lot of common mathematic, textual and logic functions.

To set the value of an attribute of a script element, one can use the following syntax.
The `xsl:value` will be first evaluated to produce the //VALUE// and, then, 
the `xsl::attribute` takes effect to add an attribute named //NAME// to `my-element`
with the computed //VALUE//.
```xml
<my-element> <xsl:attribute name="NAME"> <xsl:value-of select="VALUE" /> </xsl:attribute> </my-element>
```

//XSLT// allows to compute values but also to add conditionally XML elements thanks to `xsl:if` and `xsl:choose`.
The former syntax allows to keep the containted XML element if an //XPath// condition evaluates to true or to remove it:
```xml
<!-- xsl::if ::= -->
  <xsl:if test="xpath-test"> <!-- contained elements --> </xsl:if>
```

The //xpath-test// is any valid //XPath// expressions evaluating to non-zero with the following operators:
  * `=`, '!=` -- equality, inequality,
  * `>`, `>=` -- greater than, greater or or equal,
  * `<`, `<=` -- less than, less or equal.
In additions, the comparisons may be combined with:
  * `and` -- logical and,
  * `or` -- logical or,
  * `not` -- logical not.

The example below shows the use of `xsl:if`. Depending of the definition of the configuration
item `$virtual`, the code processor `otawa::Virtualizer` will be launched because
its XML element will be maintained in the script or not, because its `step` element
will be removed from the script.
```xml
<script>
  <xsl:if test="$virtual!=0">
    <step processor="otawa::Virtualizer"/>
  </xsl:if>
  <step processor="otawa::ipet::WCETComputation"/>
</script>```

The latter form of conditional is `xsl:choose`. It supports several test and an //otherwise// case.
```xml
<!-- xsl:choose ::= -->
  <xsl:choose>
      <xsl:when test="xpath-test"> <!-- contained elements --> </xsl:when>+
      <xsl:otherwise> <!-- contained elements --> </xsl:when>?
  </xsl:choose>
```

In this conditional structure, the `test` attributes are evaluated sequentially, in the order of the XML file,
and the result, as an XML result, are the contained elements of the first test resulting to true. If all tests fail,
the result is the elements contained in the `xsl:otherwise` node.

The example below shows that, using the name of `$processor_model`  variable, we can select
precisely the size of ROM memory of the processed architecture.
```xml
  <bank>
    <name>ON-CHIP NON-VOLATILE MEMORY</name>
    <address>0x00000000</address>
    <xsl:choose>
      <xsl:when test="$processor_model='lpc2131'"><size>0x8000</size></xsl:when>
      <xsl:when test="$processor_model='lpc2132'"><size>0x10000</size></xsl:when>
      <xsl:when test="$processor_model='lpc2134'"><size>0x20000</size></xsl:when>
      <xsl:when test="$processor_model='lpc2136'"><size>0x40000</size></xsl:when>
      <xsl:when test="$processor_model='lpc2138'"><size>0x80000</size></xsl:when>
    </xsl:choose>
    <type>ROM</type>
  </bank>
```




## Pipeline Description 

This section provides the XML format description used to represent pipeline of microprocessor.
It is used to compute time for block of code either trivially (see otawa::ipet::TrivialBBTime analysis),
or by simulation (see otawa::ipet::BBTimeSimulator analysis), or by execution graphs (see otawa::BasicGraphBBTime.h>).

Each of these analysis uses these information in a specific way but, more often, it is insufficient
to describe the whole complexity of an actual microprocessor pipeline. The goal of such a format
could be to describe the whole complexity of the pipeline but there are so heterogeneous and complex features
that the right level should be VHDL, a pipeline description complete but indadequate to infer static analysis.
In a more realistic goal, this format provides only a big picture of the pipeline and the exotic features
of actual processors is left to a plugin implementation.

### Top-Level 

A pipeline description can be found lonely in an XML file (in this case, the first element must be
`<?xml ...>` or inside another file like scripts. The top-level element must be:

```xml
<!-- pipeline ::= -->
  <processor class="otawa::hard::Processor">
    <arch>ARCH</arch>
    <model>MODEL</model>
    <builder>BUILDER</builder>
    <stages> <!-- stage -->+ </stages>
    <queues> <!-- queue -->+ </qeues>
  </processor>
```

The //ARCH// gives the programming model (instruction set, registers, etc)
supported by the pipeline. Current values includes
`arm`, `ppc`, `sparc`, `tricore`, etc. It is mainly used to check that the processor
description supports the programming model of loaded binary program.

//MODEL// and //BUILDER// are only informative data but may help to identify a pipeline description.
//MODEL// represents the accurate model of the hardware: for example, if //ARCH// is `arm`,
usual models includes `armv5t`, `cortexa8`. The //BUILDER// item gives the name
of the microprocessor builder like `atmel`, `nxp`, etc for an `arm` architecture.

The `stages` elements gives the list of stages composing the pipeline that are detailed
in the following sections. Notice this list is ordered according to the order of stages
in the actual pipeline. The first stage must be of type `fetch` while the last stage must be
of type `commit`.

The `queues` contains the list of queues in the pipeline. The queues represents any
pipeline feature storing a set of instructions, that is, FIFO buffer, reorder buffer, etc.
As a default, if there are no queue is declared between two stages, it is considered implicitly
that there is a latch whose dimension is the width of incoming stage. Only queues that does
not match the previous description must be put here.


### Stage Description 

A stage has the following syntax:
```xml
<!-- stage ::= -->
  <stage id="ID">
    <name>STRING</name>?
    <width>INT</width>?
    <latency>INT</latency>?
    <type>fetch|lazy|commit|exec</type>?
  </stage>
```

An `id` attribute is not required but it allows to link a stage with a queue.

The `name` gives the name of the stage only used to display to a human user.
As a default, the stage has an empty string for name.

The `width` represents the number of instructions that are processed in parallel
by the stage. If not given, the default is 1.

The `latency` gives the basic latency (in cycle) of the stage in order to process as many
instructions as its width. As a default, the latency is of 1 cycle.

The `type` is the more interesting part of the stage as it provides insight
on the work of the stage. Notice that the work of a stage, from the point of view OTAWA,
is mainly its effect on the execution time. The `type` must be one the enumerated value below:
  * `fetch` -- ever the first stage of the pipeline, it is reponsible for fetching instrucions
from the memory. Consequently, its throughput depends not only on its own properties but also on the time
spent to access the memory.
  * `lazy` -- the simpler stage type whose only goal is to spent time in the instruction execution.
  * `commit` -- the last stage where the instructions go out of the pipeline.
  * `exec` -- represents the stage where an instruction is executed. It is complexe because
(a) it is often split in different function units (see below) and (b) this stage handles the data dependencies
between the instruction.

A stage of type `exec` has two more elements:
```xml
<!-- exec-stage ::= -->
  ...
  <dispatch> <!-- instruction match -->+ </dispatch>
  <fus> <!-- fu -->+ </fus>
```

The `fus` element gives the list of functional unit (described in the next section).
The `dispatch` allows to dispatch instruction for functional unit.
It is made of a list of `inst` elements:
```xml
<!-- instruction match ::= -->
  <inst>
    <type>masks</type>
    <fu ref="ID"/>
  </inst>
```

The `type` allows to select an instruction from its kind mask. The kind mask
of the `otawa::Inst` class is a set of flags describing the nature of the instruction.
Each bit is identified by mask named `IS_`//xxx// and the `type` element can supports
several of these masks separated by a pipe `|`. To be selected, an instruction must
match **all** the flags in the `type`. If no flags is given, any instruction will be selected.
The `fu` gives the functional unit that will receive the instruction matching the type.
The //ID// is one of the functional unit `id` describing in the next section.

The supported flags are:
  * `IS_COND` if the instruction is conditional,
  * `IS_CONTROL` if the instruction changes the PC,
  * `IS_CALL` if the instruction performs a sub-program call,
  * `IS_RETURN` if the instruction returns from a sub-program,
  * `IS_MEM` if the instruction performs memory accesss,
  * `IS_LOAD` if the instruction loads of data from memory,
  * `IS_STORE` if the instruction stores data to memory,
  * `IS_INT` if the instruction works on integer values,
  * `IS_FLOAT` if the instruction works on float values,
  * `IS_ALU` if the instruction performs arithmetic or logic operation (not address calculation),
  * `IS_MUL` if the instruction performs a multiplication,
  * `IS_DIV` if the instruction performs a division,
  * `IS_SHIFT` if the instruction performs a shift,
  * `IS_TRAP` if the instruction performs a trap (system call, exception raise, debugging, etc)
  * `IS_INTERN` if the instruction has an effect on the internal work of the microprocessor,
  * `IS_MULTI` if the instruction performs multiple accesses to memory,
  * `IS_SPECIAL`, other types of instructions not covered by the existing flags,
  * `IS_INDIRECT` if the instruction performs an indirect branch,
  * `IS_ATOMIC` if the instruction performs atomic access to memory (in case of parallel access to memory). 

Hexadecimal numbers are also accepted as flag masks to cope with the specificities of some
instruction sets.

Below is the example of the Leon 3 dispatch element of the execution stage:
```xml
  <dispatch>
    <inst> <type>IS_MEM</type> <fu ref="INT"/> </inst>
    <inst> <type>IS_CONTROL</type> <fu ref="INT"/> </inst>
    <inst> <type>IS_INT</type> <fu ref="INT"/> </inst>
    <inst> <type>IS_FLOAT|IS_DIV</type> <fu ref="FDIV"/> </inst>
    <inst> <type></type> <fu ref="FPU"/> </inst>
  </dispatch>
```

Any memory, control or integer instruction goes in the `INT` functional unit. The first three instructions
matches works like an disjunctive condition as they branching to the same functional unit. Otherwise,
if the instruction is working on float and performs a division, it moves to the `FDIV` functional unit.
Finally, it can be stated from the instruction set of the Leon that remaining instruction are working
on floating-point numbers and must go to the `FPU` functional unit.


### Functional Unit Description 

A functional unit is like a stage but dedicated to the execution / work of an instruction.
The syntax of functional units is given below:

```xml
<!-- fu ::= -->
  <name>STRING</name>?
  <width>INT</width>?
  <latency>INT</latency>?
  <pipelined>BOOL</pipelined> 
```

The `name` allows to associatve the functional unit with a name and is only used foer human user convenience,
or with the `dispatch` element described in the previous section.

The `width` defines the number of functional unit existing, that is, the number of instruction that may be executed
in the current functional unit. Its value is 1 instruction as a default.

The `latency` describes how many cycles is required for an instruction to traverse the functional unit.
Its value is 1 cycle as a default.

Finally,`pipelined` express a fact that a multiple-cycle functional unit is not blocked until the end
of the instruction: at each cycle, it can accept another instruction. Considered false as default.

Below is an example of a multiple ALU functional unit in a superscalar microprocessor:
```xml
<fu>
  <name>ALU</name>
  <width>4</width>
  <latency>1</latency>
</fu>
```

The coming example is a multiplication of 4-cycles supporting pipelining of operations:
```xml
<fu>
  <name>MUL</name>
  <latency>4</latency>
  <pipelined>true</pipelined>
</fu>
```

The final example represents a floating-point division functional unit of 10-cyles
but that does not support pipelinging ((Notice that the `pipelined` is not required)):
```xml
<fu>
  <name>FDIV</name>
  <latency>10</latency>
  <pipelined>false</false>
</fu>
```


### Queue Description 

A queue is a small data structure containing and buffering instruction between stages.
The syntax of queues is:
```xml
<!-- queue ::= -->
<queue>
  <name>STRING</name>?
  <size>INT</size>
  <input ref="ID"/>
  <output ref="ID"/>
  <intern>  <stage ref="ID"/>+ </intern>?
</queue>
```

The `name` element allows to associate a human-readable identifier with a queue.

The `size` gives the maximum number of instructions the queue can contain.

The `ref` attribute of `input` element represents the stage that deposits instructions
in the queue.

The `ref` attribute of `output` element represents the stage that extract instructions
from the queue.

The `intern` element is only used in the case of queues implementing a re-order buffer.
In this case, an instruction can only be extracted if it has been processed
by one or several calculation stages (usually execution stages). The list of `stage`
elements gives the list of stages that will validate an instruction (that is, execute it).

Below is the example of simple FIFO queue between a fetch stage and a decode stage:
```xml
<queue>
  <name>FETCH_QUEUE</name>
  <size>8</size>
  <input ref="FI"/>
  <output ref="DI"/>
</queue>
```

In this second example, a reorder buffer stores instructions until
they have been executed by the `EX` stage:
```xml
<queue>
  <name>ROB_QUEUE</name>
  <size>16</size>
  <input ref="DI"/>
  <intern>
    <stage ref="EX"/>
  </intern>
  <ouput ref="CM"/>
</queue>
```


## Cache Description 

A cache is a small fast memory that allows to store and access fastly little block of bytes of the main memory.
Caches are a main feature feature to speed up processor execution by allowing to avoid long-time access
to the main memory.

From the point of view of caches, the main memory is divided in blocks of same size
and the cache is divided in sets. Based on its address, each memory block is assigned to a cache set that, in turn,
may contrain one (direct-mapped cache) or several blocks (associative cache).

To be compliant with the binary encoding of address, the block size, the set number and the cache size are power of 2.


There are a lot of different way to configure the caches:
  * they may be split between instruction and data or unified,
  * they have different configurations for block size, set size, cache size,
  * there are different levels of cache from L1 (close to the core) to L2, L3 (close to the main memory),
  * there are different wayq to manage blocks stored in a set (replacement policy, write-through or write-back save model, etc).

### Cache Configuration Level 

OTAWA try to provide a consistent and versatile representation of caches as below:
```xml
<!-- CACHES ::= -->
<cache-config>
  (<icache ref="ID"/> | <icache> CACHE </icache>)
  (<dcache ref="ID"/> | <dcache> CACHE </dcache>)
  (<cache id="ID"/> | <cache> CACHE </cache>)*
<cache-config>
```

A cache configuration may:
  * have without cache -- `cache-config`  is empty,
  * contain only instruction cache -- `cache-config` contains only an `icache` element,
  * be split (Harvard architecture) -- `cache-config` contains an element `icache` and an element `dcache`,
  * or unified -- `cache-config` contains only an element named `cache`.

This describes only the first level of cache, L1. If there is an L2 cache, its description is provided inside the L1 cache
element called `next`. If the L2 is unified while the L1 is split, the first `next` element contains
the L2 unified cache description and with an `id` attribute while the second `next` element
is ampty but provides a `ref` attribute that design the identifier of the first `next` element.
Of course, the scheme may repeated as far as needed.


### Cache elements 

The content of a cache element, //CACHE//, is defined below:
```xml
<!-- CACHE ::= -->
  <block_bits>INT</block_bits>
  <row_bits>INT</row_bits>
  <set_bits>INT<set_bits>
  (<next ref="ID"/> | <next> CACHE </next>)?
    <replace>NONE|OTHER||LRU|RANDOM|FIFO|PLRU</replace>?  <!-- default: LRU -->
  <write>WRITE_THROUGH|WRITE_BACK</write>?        <!-- default: WRITE_THROUGH --> 
  <allocate>BOOL</allocate>?                <!-- default: true -->
```


`block_bits` defines the size of the cache blocks: if its value is *N*, the block size is
2^*B* and means that the *B* less significant bits of the addresses design the accessed
byte in the block.

`row_bits`((This name is relatively old and misleading.)) value, *S*, determines
the number of sets in the cache, that is, 2^*S*. In the address, the bits selecting
the set ranges from *B* to *B* + *S* - 1.

Finally, the `set_bits` *A* determines the number of blocks in each set, that is, 2^*A*.
As any block may go any way of a set, there is no matching in the address.
In the end, the cache size in bytes is 2^(*B* + *S* + *A*).

The `next` element allows to link the current cache at level L*i* to a cache at level L//i+1//.
The cache at level L//i+1// may either be described in the `next` element or just contain
a reference to an element describing this cache.

The following elements are used to describe the policy of use of the blocks stored in a set.
As a set contains several blocks in an unordered way, a policy must be applied to know
which block to wipe out when a new cache block needs to be loaded. Notice that this element
can be ignored if the number of way, *A*, is equal to 0 as the set contains only one block.

`replace` describes the replacement policy that may be:
  * `NONE` -- null value (usually unused in this format),
  * `OTHER` -- unknown policy,
  * `LRU` (Least Recently Used) -- the replaced block is the least recently used,
  * `RANDOM` -- the replaced block is selected randomly,
  * `FIFO` (First-In First-Out) -- also called Round-Robin, blocks are organized as a queue and the last block is replaced,
  * `PLRU` (Pseudo-LRU) -- this policy mimics [[http://en.wikipedia.org/wiki/PLRU|LRU]] but with a lower hardware cost.

It's likely that this list be extended in the future.

`write` and `allocate` elements are only used with data or unified caches.
`write` describes the write policy of the cache (when a block is modified):
  * `WRITE_THROUGH` -- means that a write to block is immediately propagated to the main memory
to avoid to write-back the block when it is wiped out; if the `allocate` element is set to true,
a write-through is performed but, if the block is not already in the cache, it is allocated and loaded.
  * `WRITE_BACK` -- means that a write to a block is just performed in the cache and the memory
modification will be propagated to the main memory only when the block is wiped out;
if the block is not in the cache, it is loaded; `allocate` element is not used.


### Examples 

Below is a simple configuration of an architecture with only one instruction cache
(block of 16 bytes, 4-way associative, 128 sets, 8 Kb size):

```xml
<cache-config>
  <icache>
    <block_bits>4</block_bits>
    <row_bits>7</row_bits>
    <way_bits>2</way_bits>
    <replace>LRU</replace>
  </icache>
</cache-config>
```

Below is the cache structure of the ARM9, that is, split with random replacement policy
(64 ways, 32 b per block, 15Kb size):

```xml
<cache-config>
  <icache>
    <block_bits>6</block_bits>
    <row_bits>3</row_bits>
    <way_bits>5</way_bits>
    <replace>RANDOM</replace>
  </icache>
  <dcache>
    <block_bits>6</block_bits>
    <row_bits>3</row_bits>
    <way_bits>5</way_bits>
    <replace>RANDOM</replace>
  </dcache>
</cache-config>
```

This example represents a unified cache of 16 Kb:

```xml
<cache-config>
  <cache>
    <block_bits>5</block_bits>
    <row_bits>3</row_bits>
    <way_bits>6</way_bits>
    <replace>PLRU</replace>
  </cache>
</cache-config>
```

Finally, the example shows an example of split L1 cache (16 Kb) and unified L2 (256 Kb):
```xml
<cache-config>
  <icache>
    <block_bits>5</block_bits>
    <row_bits>9</row_bits>
    <way_bits>2</way_bits>
    <replace>RANDOM</replace>
    <next id="M2">
      <block_bits>6</block_bits>
      <row_bits>8</row_bits>
      <way_bits>4</way_bits>
      <replace>RANDOM</replace>
    </next>
  </icache>
  <dcache>
    <block_bits>5</block_bits>
    <row_bits>9</row_bits>
    <way_bits>2</way_bits>
    <replace>RANDOM</replace>
    <next ref="L2"/>
  </dcache>
```


## Memory Space Description 

The memory space description defines properties of the different areas
of the address space. So, a `bank` element is mainly a memory area described
by its base address and its size.

OTAWA allows to represent
addresses over different address spaces. An address is made of two components:
  * page --identification of the address space on 32-bits,
  * offset -- identification of a byte in an address space on 32-bits.

One must observe that the address space (-1) or 0xffffffff is used to represent the null address,
that is, the abstract value representing no address. 

```xml
<!-- ADDRESS ::= -->
  INT
  | (<page>INT</page>?</page> <offset>INT</offset>)
```

If the `page` element is not given, the default address space (e.g. page) is 0.


### Memory Element 

```xml
<!-- MEMORY ::= -->
  <memory>
    <banks>
      BANK*
    </banks>
  </memory>
```

A memory is just a collection of memory areas that are called banks.


### Bank Element 

```xml
<!-- BANK ::= -->
  <bank>
    <name>TEXT</name>
      <address>ADDRESS</address>
      <size>INT</size>
      <type>DRAM|SPM|ROM|IO</type>
      <latency>INT</latency>?
      <write_latency>INT</write_latency>?
    <cached>BOOL</cached>   <!-- default: false -->
      <writable>BOOL</writable> <!-- default: true -->
  </bank>
```

`name` assign a name to a memory area and exists only for human user interaction.

`address` defines the base address of the memory area.

`size` defines the memory area size in bytes. As the representation of this value is on 32-bits
and as the 0 value does not mean anything for size, 0 size represents a full coverage of the address space,
that is, a memory of size 2^32.

The `type` gives hints on the nature of the memory. Possible values may be:
  * DRAM -- usual dynamic RAM,
  * SPM -- on-chip static RAM (usually accessed in 1 cycle),
  * ROM -- read-only memory (EEPROM or anything else) but also NAND flash memory with random access,
  * IO -- represents input/output register of peripherals (access time is usually high).

The `latency` defines the number of cycle to access the memory space to read a data item.
If there is no `write_latency`, this defines also the time in cycle to write a data item.

The `write_latency` element provides the write time in cycles.

`cached` allows to know if the memory area is accessed through the cache or directly.
IO memory areas are often not cached.

With the `writable` element, one can know if a memory area is writable or not:
ROM are often considered as not writable using the classic load / store instruction of a microprocessor.
For example, flash memories are readable word by word but are written block by block through
dedicated IO registers.


## BHT Description 

BHT (Branch Hit Table) is a current implementation of branch prediction mechanism.
Basically, a BHT can be viewed as a cache memory indexed on branch instruction addresses
(supporting direct mapping and associativity) but providing at each entry an hint to
predict the behaviour of the branch instruction: taken or not-taken. For now, we consider
only bi-modal branch prediction with saturation (2 bits for states Strongly-Taken,
Weakly-Taken, Weakly-Not-Taken, Strongly-Not-Taken) but we plan to support more branch
prediction in next versions.

The format of BHT is displayed below:
```xml
<!-- BHT ::= -->
  <bht class="otawa::hard::BHT">
  
    <block_bits>INT</block_bits>
    <row_bits>INT</row_bits>
    <way_bits>INT</way_bits>
    <replace>LRU|RANDOM|FIFO|PLRU</replace>
  
    <cond_penalty>INT</cond_penalty>?
    <indirect_penalty>INT</indirect_penalty>
    <cond_indirect_penalty>INT>/cond_indirect_penalty>
    <def_predict>PREDICT_TAKEN|PREDICT_NOT_TAKEN|PREDICT_DIRECT</def_predict>
  </bht>  
```

The first four elements describes the cache nature of the BHT:
  * `block_bits` -- if its value is *n*, the branch address is divided by 2<sup>n</sup>,
  * `row_bits` -- number of sets in the cache (if its values is *n*, there are 2<sup>n</sup> sets),
  * `way_bits` -- number of branch stored in each set (if its values is *n*, there are 2<sup>n</sup> branches in each set),
  * `replace` -- describe how the branches are managed and replaced in each set.
    * LRU -- Least Recently Used,
    * RANDOM -- Random Replacement,
    * FIFO -- First-In First-Out (also called Round-Robin),
    * PLRU -- Pseudo-Least Recently Used.

The next fields are specific to a branch predictor:
  * `cond_penalty` -- cost of a misprediction in cycles for a simple conditional branch,
  * `indirect_penalty` -- cost of a misprediction in cyles for an indirect branch,
  * `cond_indirect_penalty` -- cost of a misprediction in cyles for an conditional indirect branch,
  * `def_predict` -- default prediction to apply when a branch is not in BHT:
    * `PREDICT_TAKEN` -- branch is considered as taken,
    * `PREDICT_NOT_TAKEN` -- branch is considered as not-taken,
    * `PREDICT_DIRECT` -- backward branch considered taken, forward as not-tkabe.

# Development of Loader

OTAWA supports different API thanks to a plugin mechanism and to an //architecture abstraction layer//.
Developing a loader is mainly providing actual implementation of this layer
for a particular ISA.

## The Architecture Abstraction Layer 

###  Overview ### 

![](otawa-overview.png)

As shown in the figure above, this layer allows to make
analysis implemented on OTAWA independent of the actual
architecture of the processed program. This does not mean
that the analyzes cannot be aware of the particular properties
of the processed program but that they do not have to be tuned
for a particular instruction (ISA). Yet, the analyzes can have
as many information as required from the actual form of the
processed program.

The //architecture abstraction layer// is mainly
formed of two parts:
  * a set of objects describing program image in memory, the files, the symbols and the instructions,
  * a translator to get semantics of the actual instruction set expressed in a language of semantics instructions.


###  Abstraction Objects ### 

The UML diagram below describes the classes and the relationship of C++ objects
involved in the //architecture description layer//.

![](aal-uml.png)

The root class of the //architecture abstract layer// is the `Process` that represents the program ready
to run in memory. It contains all information about the program in memory (start instruction, platform,
file composing the memory image) with other information provided in the executable files like
debugging information (matches between source lines and memory addresses). The `Process` is
the root object and the result of a loader work.

In addition, the `Process` contains also information about the programming model of the architecture
thanks to the `Platform` object: mainly, the list of register banks with their description.

From the `Process`, one can get the list of executable file composing the program image in memory.
Although most OTAWA loader, at this time, only supports applications made of a monolithic executable file,
the framework is able to cope with processes made of several file: at least one `program` file and
possibly several library files.

The files, in turn, contains bits composing the program image in memory, called the `Segments`
but also more functional information like `Symbol`. The `Symbol` matches any object
with a name produced by the compiler, that is, function, data, labels, etc. The symbols
may occupy a place in the memory (defined by its address and its size) or not (constants values,
compiler or OS internal symbols).

The `Segment` objects represents slices of memory sharing common properties. They may be initialized
or not. Closer concepts from the ELF file format are either sections, or program headers. Whatever,
a `Segment` has usually a name, an address, a size and may be executable or writable. They are composed
of `ProgItem` object.

A `ProgItem` object represents any atomic entity in the program, mainly, instructions and data items.
The current version of OTAWA only supports instructions, `Inst` class, but next versions may be able
to recognize data and add representation objects. Whatever, a program item is identified by its address
and its size.

Specializing the `ProgItem` class, the `Inst` objects represents the actual machine instructions
of the current ISA. The role of these objects is to give an abstract description of machine instructions
as precise as possible to let higher level analyzes to work with the instruction. An `Inst` object
have the following interface:
  * `dump` - to get a textual representation (useful for debugging or for user output),
  * `kind` - information about the nature of the instruction,
  * `readRegs` - set of read registers,
  * `writtenRegs` - set of written registers,
  * `target` - when the instruction is a control, the target of the branch (if one can be determined),
  * `semInsts` - translate the instruction into semantics instructions.

The `kind` allows to identify the type of an instruction whatever the used instruction set.
It is composed of a bit vector with each bit giving a specific information:
  * `IS_COND` - set if the instruction is conditional,
  * `IS_CONTROL` - set if the instruction performs a branch, that is, changes the PC,
    * `IS_CALL` - set if the instruction is a sub-program call (ever induces `IS_CONTROL` to be set),
    * `IS_RETURN` - set if the instruction is sub-program return (ever induces `IS_CONTROL` to be set),
    * `IS_TRAP` - set if the instruction performs a system trap like exception or system call (ever induces `IS_CONTROL` to be set),
  * `IS_MEM` - set if the instruction performs memory access,
    * `IS_LOAD` - set if the instruction performs memory load (ever induces `IS_MEM` to be set),
    * `IS_STORE` - set if the instruction performs memory store (ever induces `IS_MEM` to be set),
    * `IS_MULTI` - set if the instruction performs multiple memory accesses of the same type (ever induces `IS_MEM` to be set),
  * `IS_INT` - set if the instruction handles integer,
  * `IS_FLOAT` - set if the  instruction handles floats,
  * `IS_ALU` - set if the instruction performs arithmetic or logic operations,
  * `IS_MUL` - set if the instruction performs multiplication operation,
  * `IS_DIV` - set if the instruction performs division operation,
  * `IS_SHIFT` - set if the instructions performs shift operation,
  * `IS_INTERN` - set if the instructions performs operation internal to the microprocessor (hardware driving),
  * `IS_SPECIAL` - set if the instruction is unusual (often found in old CISC ISA).

A lot of facilities provided by the //architecture abstraction layer// are optional
and the analyzes must be able to handle this: either assuming worst case configuration,
or aborting the analysis. Whatever, each information availability
is represented by a specific feature put on the `Process` and included in the `WorkSpace`:
  * `MEMORY_ACCESS_FEATURE` -- analyzes can read integer values in the process memory,
  * `FLOAT_MEMORY_ACCESS_FEATURE` -- analyzes can read float values in the process memory,
  * `REGISTER_USAGE_FEATURE` -- ensures that lists of read and written registers are available,
  * `CONTROL_DECODING_FEATURE` -- ensures that the control instruction target is decoded,
  * `SOURCE_LINE_FEATURE` -- ensures that the source/line debugging information is available,
  * `SEMANTICS_INFO` -- ensures that the semantics instruction translation is provided,
  * `DELAYED_FEATURE` -- means that information about delayed control is provided,
  * `SEMANTICS_INFO_EXTENDED` - means that the semantics instructions extension is available
(multiplication, division, binary operations),



###  Semantics Instructions ### 

Semantics instructions provides a way to cope with the functional behavior of the instructions
independently of the ISA. Basically, the semantic instruction set is a minimal regular
instruction set including usual operations with the following features:
  * the control flow is bound to a forward-branching conditional instruction (no loop can be created),
  * the memory operations are only performed by two specific instruction, `load` and `store`,
  * the instructions can either use registers, or temporaries (not involved in the program state),
  * constant values can only be processed through a specific instruction, `seti`,
  * the comparison instructions can only produces results based on the usual comparison operators,
  * as it is impossible, without adding a lot of complexity, to cope with all possible machine instructions,
the instruction, `scratch`, can inform that a result is unknown.

The goal of this language is to make possible data flow analysis of the program, that is static analyzes
like abstract interpretation without needing to specialize it to a specific ISA.
As such analyzes may have polynomial complexity, the interpretation must be as fast as possible.
Hence, the semantics instructions can not create loops (inside the machine instruction they are defining)
to avoid to have to compute fix points on the translation of the instruction. The idea is that
the machine control flow is viewable inside the instruction but the control flow of the program
is handled at an upper level (in the Control Flow Graph for example).

The only conditional instruction, `if`(`c`, `r`, `s`) allows to have several parallel interpretation paths
but no loop. It means that if the condition `c` is true in register `r`, the interpretation path continue,
else `s` instructions must be skipped. The misnamed instructions `branch` or `trap` means the instruction
is performing a branch instruction here, that is, from a CFG point of view, that the branch-taken edge is followed.
Finally `cont` semantic instruction stops the execution of the semantic instruction for the current machine
instruction. If no `branch` has been found on the current execution path, the CFG edge representing sequential
control flow is considered to be followed. Notice that `branch` does not stop the execution of the semantic
instruction: from an ISA point of view, it just changes the PC register of the machine.

For computation, the following instructions are available. Most of them works as three-operand operations
on register or variable registers with `d` the destination register, `a` the first source operand
and `b` the second source operand.
  * `set(d, a)` -- register copy,
  * `add`(d, a, b) -- addition,
  * `sub`(d, a, b) -- subtraction,
  * `shl`(d, a, b) -- logical shift left,
  * `shr`(d, a, b) -- logical shift right,
  * `asr`(d, a, b) -- arithmetic shift right,
  * `cmp`(d, a, b) -- comparison of a and b,
  * `cmpu`(d, a, b) -- unsigned comparison of a and b.

If the feature `SEMANTICS_INFO_EXTENDED` is provided by the process, the following instructions
may also be used:
  * `neg`(d, a) -- sign inversion,
  * `mul`(d, a, b) -- signed multiplication,
  * `mulu`(d, a, b) -- unsigned multiplication,
  * `div`(d, a, b) -- signed division,
  * `divu`(d, a, b) -- unsigned division,
  * `mod`(d, a, b) -- signed modulo,
  * `modu`(d, a, b) -- unsigned modulo,
  * `not`(d, a) -- bit-to-bit not,
  * `and`(d, a, b) -- bit-to-bit and,
  * `or`(d, a, b) -- bit-to-bit inclusive-or,
  * `xor`(d, a, b) -- bit-to-bit exclusive-or,


## Developing a Loader

A loader is basically a plugin whose handle implements `otawa::Loader`. The handle object is used
to load a binary file and to build the process representing the program. To illustrate the procedure,
we will implement (partially) a loader for a MIPS ISA. Let declare the loader handle:


```cpp
#include <otawa/prog/Loader.h>

namespace mips {

// loader class
static string table[] = { "elf_20" };
static elm::genstruct::Table<string> aliases(table, 1);

class Loader: public otawa::Loader {
public:
  Loader(void): otawa::Loader("mips", Version(1, 0, 0), OTAWA_LOADER_VERSION, aliases)
    { }
  virtual CString getName(void) const
    { return "mips"; }

  virtual otawa::Process *load(Manager *man, CString path, const PropList& props) {
    otawa::Process *proc = create(man, props);
    if(!proc->loadProgram(path)) {
      delete proc;
      proc = 0;
    }
    return proc;
  }

  virtual otawa::Process *create(Manager *man, const PropList& props)
    { return new Process(man, new Platform(props), props); }
};

} // mips

mips::Loader OTAWA_LOADER_HOOK;
mips::Loader& mips_plugin = OTAWA_LOADER_HOOK;
```

The more interesting part is in `create`() method that builds a platform of types `mips::Platform`
and the process itself of type `mips::Process`. The description of these object is detailed below.
When a binary is opened from an ELF file, OTAWA looks for the matching plugin using ISA field of ELF header,
install the loader and call method `load`(). This one creates a process and and load the program
in the process. If there is an error, process object is cleaned and a null is returned.

Now, we have to describe the structure of MIPS state in the platform object. First, we have to declare
the platform that is mainly composed of registers as a banks of identical register or separate registers:


```cpp
#include <otawa/hard/Platform.h>
#include <otawa/hard/Register.h>

namespace mips {

class Platform: public hard::Platform {
public:
  static const Identification ID;
  Platform(const PropList& props = PropList::EMPTY): hard::Platform(ID, props)
    { setBanks(banks); }

  static const hard::Register PC, HI, LO;
  static const hard::PlainBank GR;
  static const hard::PlainBank FPR;
  static const hard::MeltedBank MISC;
  static const elm::genstruct::Table<const hard::RegBank *> banks;

  virtual bool accept(const Identification& id)
    { return id.abi() == "elf" && id.architecture() == "mips"; }
};

const Platform::Identification Platform::ID("mips-elf-");

const hard::PlainBank Platform::GR("GR", hard::Register::INT,  32, "$%d", 32);
const hard::PlainBank Platform::FPR("FPR", hard::Register::FLOAT,  64, "$f%d", 32);
const hard::Register Platform::PC("pc", hard::Register::INT, 32);
const hard::Register Platform::HI("hi", hard::Register::INT, 32);
const hard::Register Platform::LO("lo", hard::Register::INT, 32);
const hard::MeltedBank Platform::MISC("MISC", &Platform::PC, &Platform::HI, &Platform::LO, 0);

static const hard::RegBank *banks_array[] = {
  &Platform::GR,
  &Platform::FPR,
  &Platform::MISC
};
const elm::genstruct::Table<const hard::RegBank *> Platform::banks(banks_array, 3);

} // mips
```

The main point here is that the constructor of MIPS platform **must** record the hardware register
using the `setBanks`() method. This method takes as input a `genstruct::Table`, that is, a list
of register banks. In the banks array, some may be uniforms like `GR` or `FPR`, some are a melt
of different registers like `MISC`. A uniform `PlainBank` register bank is constructed using
its name, its type (one of `INT`, `FLOAT`, `ADDR` or `BITS`), the size of its register
in bits, a format string to name bank register containing a "%d" that will be replaced by the actual
register index and the count of registers in the bank.

To declare a register alone (like `PC`, `HI` or `LO`), one has to pass the register name,
its type and its size in bits. Non-regular registers are then grouped in `MISC`. They are passed
to the bank constructor as a null-ended list of register pointers.

`ID` and `accept` are added here for backward compatibility, to identify the platform, but they
are rarely used by OTAWA.

Now the MIPS `Process` itself may be declared (this is the minimal version):

```cpp
#include <otawa/proc/Process.h>

namespace mips {

class Process: public otawa::Process {
public:
  Process(Manager *manager, hard::Platform *pf, const PropList& props = PropList::EMPTY);

  virtual hard::Platform *platform(void) { return _pf; }
  virtual Inst *start(void);
  virtual int instSize(void) const;
  virtual File *loadFile(elm::CString path);

private:
  hard::Platform *_pf;
};

} // mips
```

The method `start`() gives the first instruction of the program while `loadFile` is called
by `loadProgram`() to install the executable file in the current process (copying code and data segments).
`instSize`() provides the size of instruction in bytes if the instruction set is regular, 0 else. This declaration
is a very minimal `Process`, more customization will be added afterward in the following sections
to provide more facilities.

To go further in the description of MIPS `Process`, a loader library must be chosen and used. The following
section gives the implemetation using the GEL library (GEL is delivered with OTAWA).

###  Developing a loader with GEL ### 

GEL is the usual library used by OTAWA to handle ELF binaries. It is a C library but it is perfectly
compatible with OTAWA C++. The very first method that will use GEL is `loadFile`() so we extend our
process description with data structure useful for GEL:


```cpp
#include <gel/gel.h>
#include <gel/gel_elf.h>
#include <gel/image.h>
#include <otawa/prog/Segment.h>

namespace mips {

class Process: public otawa Process {
public:
  ...
  virtual Inst *start(void) { return _start; }
private:
  ...
  Inst *_start;
  gel_file_t *gel_file;
  gel_line_map_t *map;
};

} // mips
```

The `loadFile`() is described now:

```cpp
File *Process::loadFile(elm::CString path) {

  // (a) Check if there is not an already opened file !
  if(program())
    throw LoadException("loader cannot open multiple files !");

  // (b) create file
  File *file = new otawa::File(path);
  addFile(file);

  // (c) open the binary
  gel_file = gel_open(&path, 0, GEL_OPEN_NOPLUGINS | GEL_OPEN_QUIET);
  if(!gel_file)
    throw LoadException(_ << "cannot load \"" << path << "\".");

  // (d) build the segments
  gel_file_info_t infos;
  gel_file_infos(gel_file, &infos);
  for (int i = 0; i < infos.sectnum; i++) {
    gel_sect_info_t infos;
    gel_sect_t *sect = gel_getsectbyidx(gel_file, i);
    gel_sect_infos(sect, &infos);
    if(infos.flags & SHF_EXECINSTR) {
      Segment *seg = new Segment(*this, infos.name, infos.vaddr, infos.size);
      file->addSegment(seg);
    }
  }

  // (e) initialize symbols
  gel_enum_t *iter = gel_enum_file_symbol(gel_file);
  gel_enum_initpos(iter);
  for(char *name = (char *)gel_enum_next(iter); name; name = (char *)gel_enum_next(iter)) {

    // scan the symbol
    address_t addr = 0;
    Symbol::kind_t kind;
    gel_sym_t *sym = gel_find_file_symbol(gel_file, name);
    gel_sym_info_t infos;
    gel_sym_infos(sym, &infos);
    switch(ELF32_ST_TYPE(infos.info)) {
    case STT_FUNC:
      kind = Symbol::FUNCTION;
      addr = (address_t)infos.vaddr;
      break;
    case STT_NOTYPE:
      kind = Symbol::LABEL;
      addr = (address_t)infos.vaddr;
      break;
    default:
      continue;
    }

    // build the label if required
    if(addr) {
      String label(infos.name);
      Symbol *sym = new Symbol(*file, label, kind, addr);
      file->addSymbol(sym);
    }
  }
  gel_enum_free(iter);

  // (f) find sart point
  _start = findInstAt(Address(infos.entry));
  return file;
}

```

This method is the more complex part of the loading procedure as it opens the binary
and examine it to build the OTAWA program representation. For the sake of simplicity,
the presented version accepts in the process only one binary, the main program, but
OTAWA is designed to support shared libraries also. So the first block of code (a)
ensures there is no other program opened.Block (c) create the file itself and at it
to the process. Block (c) uses GEL to open the binary file and stores the handle in
`gel_file`. This handle will be used all along the process life and is stored
in attributes.

Block (d) looks in the section composing the executable and builds OTAWA `Segment`
for each section that represents a part of the program in memory. ELF contains
lots of sections but only some aims to be involved in the execution. Some contains
informations on the code like symbols table or debugging sections. Block (e) allows
translating symbols in ELF executable as symbols in the process. Notice that only
symbols corresponding to an address in the memory are kept. Finally, block (f)
retrieve the OTAWA instruction representing the first instruction of the program.

Each time an instruction is accessed, it must be decoded and built as an `Inst` object.
This is usually done by calling `findInstAt`(). Yet, it is not straight: OTAWA maintains
a list of decoded instruction to reduce memory footprint. In fact, the decoding is
performed by a call to the segment, containing the instruction (through `findInstAt`() ),
that manages the list of instructions. This will be shown in the next section.

Whatever, GEL provides also access to debugging information of the ELF file. So it may
be used to implement `SOURCE_LINE_FEATURE` as below:


```cpp
class Process: public otawa::Process {
public:
  ...

  Option<Pair<cstring, int> > Process::getSourceLine(Address addr) throw (UnsupportedFeatureException) {

    // build line map
    setup();
    if (!map)
      return none;

    // look for address
    const char *file;
    int line;
    if(!map || gel_line_from_address(map, addr.offset(), &file, &line) < 0)
      return none;
    return some(pair(cstring(file), line));
  }

  void Process::getAddresses(cstring file, int line, Vector<Pair<Address, Address> >& addresses) throw (UnsupportedFeatureException) {

    // build line map
    setup();

    // look for source line
    addresses.clear();
    if (!map)
      return;
    gel_line_iter_t iter;
    gel_location_t loc, ploc = { 0, 0, 0, 0 };
    for (loc = gel_first_line(&iter, map); loc.file; loc = gel_next_line(&iter)) {
    cstring lfile = loc.file;
    if (file == loc.file || lfile.endsWith(file))
    {
      if (line == loc.line)
        addresses.add(pair(Address(loc.low_addr), Address(loc.high_addr)));
      else if(loc.file == ploc.file && line > ploc.line && line < loc.line)
        addresses.add(pair(Address(ploc.low_addr), Address(ploc.high_addr)));
    }
    ploc = loc;
  }

  void Process::setup(void) {
    if(init)
      return;
    init = true;
    map = gel_new_line_map(gel_file);
  }

private:
  ...
  struct gel_line_map_t *map;
  bool init;
};
```

The first function, `getSourceLine`() returns, if any, source name and source file matching
the given address. The second, `getAddresses`() computes a set of addresses matching the
given source file and source line. Both use a call to `setup`() to obtain the line map.
The `setup`() function simply checks wether an attempt to build the line map has not
already been done otherwise it tries to build the line map, that may fail if no debugging
information is available.


### Developing a Loader with GLISS2

If GEL is used to build the program image in memory, it does not provide facilities to support
instruction set. Instead, OTAWA uses an ISS (Instruction Set Simulatpor) called GLISS.
The instruction set is described using an ADL (Architecture Description Language), SimNML for GLISS,
and the generator produces what is required to perform the simulation : memory emulator,
system call emulator, simulator and, what is specially used by OTAWA, an instruction decoder
and a disassembler. Using an ADL like SimNML and a generator like GLISS allows adding user
attributes and also generating code to access to these attributes from a decoded instruction.
This feature is exploited a lot by OTAWA to obtain information on the semantic of instructions.

Below is an excerpt from a SimNML file describing the `add` instruction of MIPS ISA:

```c
op add(rd: card(5), rs: card(5), rt: card(5))
  image = format("000000 %5b %5b %5b 00000 100000", rs, rt, rd)
  syntax = format("add $%d, $%d, $%d", rd, rs, rt)
  action = {
    temp = (GPR[rs]<31..31> :: GPR[rs]) + (GPR[rd]<31..31> :: GPR[rt]);
    if temp<32..32> != temp<31..31> then
      SignalException(IntegerOverflow);
    else
      GPR[rd] = temp;
    endif;
  }
```

The first line identify the instruction with 3 logical parameters, `rd`, `rs` and `rt`
then come attribute definitions. The `syntax` attributes specifies the opcode of the instruction
and particularly bits dedicated to encode the parameters. The `syntax` attibutes defines
the assembly form of the instruction. Both previous attributes use a-la `printf` format string
and arguments. Finally, the `action` attribute provides the semantics of the instruction
using an algorithmic-like language. Basically, the information required by OTAWA is fully
available in the SimNML but not so easy to extract, particularly about instruction behaviour.

Whatever, SimNML provides a good base for decoding instructions. The first thing to do is
to instantiate a GLISS decode inside our plugin and to build the program image in
the memory emulation of GLISS. The following modifications are done to our `Process` class:


```cpp
#include <mips/api.h>
...

class Process: public otawa::Process {
  ...
private:
  ...
  mips_platform_t *_mips_pf;
  mips_memory_t *_mips_mem;
  mips_decoder_t *_mips_dec;
};

File *Process::loadFile(elm::CString path) {
  ...

  // (g) allocate GLISS resources
  _mips_pf = mips_new_platform();
  _mips_dec = mips_new_decoder(_mips_pf);
  _mips_mem = mips_get_memory(_mips_pf, ARM_MAIN_MEMORY);
  mips_lock_platform(_mips_platform);

  // (h) build the memory
  gel_image_t *gimage = gel_image_load(_file, 0, 0);
  if(!gimage) {
    gel_close(_file);
    throw LoadException(_ << "cannot build image of \"" << path << "\": " << gel_strerror());
  }
  gel_image_info_t iinfo;
  gel_image_infos(gimage, &iinfo);
  for(t::uint32 i = 0; i < iinfo.membersnum; i++) {
    gel_cursor_t cursor;
    gel_block2cursor(iinfo.members[i], &cursor);
    mips_mem_write(_memory,
      gel_cursor_vaddr(cursor),
      gel_cursor_addr(&cursor),
      gel_cursor_avail(cursor));
  }

}
```

The block (g) obtains all required resources from MIPS GLISS to perform decoding
and disassembly while block (h) build the program image in the GLISS memory emulator.

It becomes now possible to decode instructions from memory emulator and to build
an OTAWA instruction:


```cpp
namespace mips {

class Inst: public otawa::Inst {
public:
  inline Inst(Process& process, kind_t kind, Address addr, int size)
    : proc(process), _kind(kind), _size(size), _addr(addr), isRegsDone(false) { }
  virtual void dump(io::Output& out);
  virtual kind_t kind() { return _kind; }
  virtual address_t address() const { return _addr; }
  virtual t::uint32 size() const { return _size; }
  virtual const elm::genstruct::Table<hard::Register *>& readRegs(void);
  virtual const elm::genstruct::Table<hard::Register *>& writtenRegs(void);
  virtual void semInsts (sem::Block &block);

protected:
  Process &proc;
  kind_t _kind;
  int _size;
private:
  elm::genstruct::AllocatedTable<hard::Register *> in_regs;
  elm::genstruct::AllocatedTable<hard::Register *> out_regs;
  mips_address_t _addr;
  bool isRegsDone;
};
```

The instruction class store very basic information like kind, address and size of the instruction
and a reference on the process. Very basic method like `kind`(), `address`() or `size`() or
easy to implement from information passed to the instruction constructor. Other information
are more complex as obtaining the disassembly of the instruction and required calls to the GLISS API:

```cpp
void Inst::dump(io::Output& out) {
  char out_buffer[200];
  mips_inst_t *inst = mips_decode(proc._mips_dec, mips_address_t(addr.offset()));
  mips_disasm(out_buffer, inst);
  mips_free_inst(inst);
  out << out_buffer;
}
```

In the previous code, a decoded handle of the instruction is first obtaine from the address
of the instruction. This handle is used to get the disassembled instruction in `out_buffer`
that is displayed on the `out` stream. To be clean, the handle needs then to be fried.

In fact, the decoding of an instruction works in the same way. But it is performed
in a customized `Segment` class:

```cpp
class Segment: public otawa::Segment {
public:
  Segment(Process& process, CString name, address_t address, t::uint32 size, int flags = EXECUTABLE)
    : otawa::Segment(name, address, size, flags), proc(process) { }
protected:
  virtual otawa::Inst *decode(Address addr) {
    mips_inst_t *inst = mips_decode(proc._mips_dec, mips_address_t(_addr.offset()));
    Inst::kind_t kind = 0;
    otawa::Inst *result = 0;
    kind = mips_kind(inst);
    int size = mips_get_inst_size(inst) >> 3;
    free_inst(inst);
    result = new Inst(proc, kind, addr, size);
    return result;
  }
private:
  Process& proc;
};
```

The main work is done in `decode`() function. As in the previous example, a GLISS instruction
handle is obtained and used to collect information on the instruction. The obtained information
allows building the OTAWA instruction in the end. Instruction address is part of the decoding
call cause by a call ton `findInstAt`(). Notice that an instruction is only decoded once
and then saved by OTAWA in a dedicated structure. The size of the instruction is also
a standard information provided by GLISS in function `mips_size`. It is given bits and so
must be divided by 8 to convert in bytes.

Yet, GLISS has not the concept of kind as used by OTAWA. Basically, we need to assign to each
information declared in the ADL. According to the instruction, this may be relatively complex
as not only the type of instruction is needed but also, sometimes, the value of the parameters.
For example, on ARM instruction set, as the PC is like a general purpose register, any instruction
assigning it can be considered as a control. Fortunately, GLISS provides custom attributes
to help to build kind according the full set of parameters. First, a NMP file (in SimNML)
with the definition of the kind must be written; an excerpt of `kind.nmp` for MIPS is given below:

```c
let NO_KIND   = 0x00000000
let IS_COND   = 0x00000001
let IS_CONTROL  = 0x00000002
let IS_CALL   = 0x00000004
let IS_RETURN = 0x00000008
let IS_MEM    = 0x00000010
let IS_LOAD   = 0x00000020
let IS_STORE  = 0x00000040
let IS_INT    = 0x00000080
let IS_FLOAT  = 0x00000100
let IS_ALU    = 0x00000200
let IS_MUL    = 0x00000400
let IS_DIV    = 0x00000800
let IS_SHIFT  = 0x00001000
let IS_TRAP   = 0x00002000
let IS_SPECIAL  = 0x00010000

extend add
  otawa_kind = IS_ALU | IS_INT

...
```

Then, a template file for code generation needs to be written. We name it `kind.tpl`:

```c
/* Generated by gliss-attr ($(date)) copyright (c) 2009 IRIT - UPS */
#include <$(proc)/api.h>
#include <$(proc)/id.h>
#include <$(proc)/macros.h>
#include <$(proc)/grt.h>

typedef unsigned long otawa_kind_t;
typedef otawa_kind_t (*fun_t)($(proc)_inst_t *inst);

/*** function definition ***/
static otawa_kind_t otawa_kind_UNKNOWN($(proc)_inst_t *inst) {
  return 0;
}

$(foreach instructions)
static otawa_kind_t otawa_kind_$(IDENT)($(proc)_inst_t *inst) {
$(otawa_kind)
};

$(end)


/*** function table ***/
static fun_t kind_funs[] = {
  otawa_kind_UNKNOWN$(foreach instructions),
  otawa_kind_$(IDENT)$(end)
};

otawa_kind_t $(proc)_kind($(proc)_inst_t *inst) {
  return kind_funs[inst->ident](inst);
}
```

Finally the actual C will be generated with a command like:
```sh
gliss-attr mips.irg -o kind.c -a kind -f -t kind.tpl -d "return 0\\;" -e kind.nmp
```
The only special file in the command above is `mips.irg` that is available in the
generation of the ISS for MIPS by GLISS. This approach may be used also to compute branch
address of a control instruction. Let the inconditional branch instruction of MIPS
described in SimNML with:

```c
op J(instr_index: card(26))
  image = format("000010 %26b", instr_index)
  target = __IADDR<31..28> :: instr_index :: 0b00
  syntax = format("J %l", target)
  action = { PC = target; }
```


Now, it is easy to provide a new template like:

```cpp
/* Generated by gliss-attr ($(date)) copyright (c) 2009 IRIT - UPS */
#include <$(proc)/api.h>
#include <$(proc)/id.h>
#include <$(proc)/macros.h>

typedef uint32_t otawa_target_t;
typedef otawa_target_t (*target_fun_t)($(proc)_inst_t *inst);

/* functions */
static otawa_target_t otawa_target_UNKNOWN($(proc)_inst_t *inst) {
        /* this code should also be used as default value if
        an instruction has no otawa_target field */
        return 0;
}

$(foreach instructions)
static otawa_target_t otawa_target_$(IDENT)($(proc)_inst_t *inst) {
$(otawa_target)
};

$(end)

/* function table */
static target_fun_t $(proc)_target_table[] = {
  otawa_target_UNKNOWN$(foreach instructions),
  otawa_target_$(IDENT)$(end)
};

otawa_target_t $(proc)_target($(proc)_inst_t *inst) {
        return $(proc)_target_table[inst->ident](inst);
}
```

And the generation command will be:
```sh
gliss-attr mips.irg -o target.c -a target -f -t target.tpl -d "return 0\\;"
```
As `target` attribute is already available in the original NMP file, we do not
add extension fill with option `-e`.


# Analyzer Development 

OTAWA annotation system, loader plugins and scripts provides lots of possibilities
to tune and customize the WCET computation. Yet, sometimes, the available analysis
are not enough to support a particular hardware or application. Fortunately,
OTAWA allows also to extend the performed analyzes  using its flexible plugin system.

## OTAWA Plugin System 

OTAWA is heavily built upon the plugin concept to make it as versatile as possible.
This involves several issues for which OTAWA framework proposes several solutions.

The first one is the localization of a needed plugin: OTAWA uses a system that try
to locate a plugin based on the full name of used object. This approach can be applied
to locate a plugin containing either a property identifier, a feature or a code processor.

The full-qualified name is the complete name of the entity prefixed by the C++ name spaces (separated
by the usual C++ "::"). The name spaces chain is transformed in a file system path
that allows to lookout for a plugin containing it. Once the plugin is found in the file system,
it is loaded and the looked item (property, feature and processor) becomes available.

For example, the "otawa::dcache::MUST_ACS" property identifier produces the file system path
"otawa/dcache". The, OTAWA will look for plugin matching this name in the usual directories
where plugins are fetched:
  * $PWD/.otawa/proc,
  * $HOME/.otawa/proc
  * $(OTAWA_HOME)/lib/otawa/proc

In fact, a hierarchical search is performed. First, only "otawa" is looked for a plugin.
If not found, a plugin whose path "otawa/dcache" is looked. The search continues until
the name space identifiers have been exhausted. This allows to group together in a same plugin
different namespaces.


## Using Entities defined in a Plugin 

To get an entity inside a plugin (property identifier, feature or code processor), the full-qualified
name is only required and lookup of this name is performed by OTAWA. To help the programmer in this task,
the framework propose several useful class.

For a property identifier, the class `DynIdentifier` is a good candidate. It takes as constructor
argument the full-qualified name of the identifier and, as soon it is used, will automatically,
if required, causes the linkage of the plugin. In addition, it may be used as any other identifier.
The example below shows its use:

```cpp
#include <otawa/prop/DynIdentifier.h>
using namespace otawa;

static DynIdentifier INITIAL_SP("otawa::dcache::INITIAL_SP");

PropList props;
INITIAL_SP(props) = initial_sp_address;
```

The same exists for feature with `DynFeature` class:
```cpp
#include <otawa/proc/DynFeature.h>
using namespace otawa;

static DynFeature MAY_ACS_FEATURE("otawa::dcache::MAY_ACS_FEATURE");

WorkSpace *ws;
ws->require(MAY_ACS_FEATURE);
```


And it works in the same way for code processors with class `DynProcessor`. Naturally,
this works only with registered processors. 
```cpp
#include <otawa/proc/DynProcessor.h>
using namespace otawa;

static DynProcessor clp_builder("otawa::dcache::CLPBlockBuilder");
clp_builder.process(workspace, props);
```

It must be noted that global variable is an efficient to use dynamic identifier, dynamic feature
or dynamic processors because the linkage is only performed once.

Yet, the linkage to a dynamic entities, based on plugins, does not always succeed. In this case,
an exception of class `ProcessorNotFound` or `FeatureNotFound` is raised.
It may just be ignored and application will stop and display the message attached to the exception,
letting the user understanding the problem: `app::Application` will do this automatically.
Else the exception may be caught and a fix operation started. There is no exception for
property identifier because they are supposed to be used after the requirement of their matching
feature.

The use of the `static` C++ modified aims to avoid symbols conflicts. Depending on the OS,
this may prevent the linkage because a symbol of the same name is present. In addition,
if the symbols have different types (and in our examples, they have), this could drive
to the application crash.


## Writing a Analysis Plugin 

With the `Dyn`//XXX// family of classe, it is possible to exploit entities defined
in plugins. Here is presented the way to write and to make such a plugin.

Let's for example, a wonderful analysis that counts the number of instructions in each
basic block. It is in the `useful` package that defines several plugins and particularly
the `stat` plugin. This plugin provides a feature, `COUNT_FEATURE` and a property
`INST_COUNT`. It is implemented by the code processor `InstCounter`.

First, a `stat.h` file has to be created containing this definitions in the right namespace
to provides identifier and featiures to the user of the plugin:
```cpp
namespace useful { namespace stat {
  extern p::feature COUNT_FEATURE;
  extern Identifier<int> INST_COUNT;
} }
```

The code processor, `InstCounter`, does not need to be provided in the `stat.h`
because it does not aim to be used as class but only as the producer of
`COUNT_FEATURE`. Therefore, the source `stat.cpp` might looks like:
```cpp
#include "stat.h"

namespace useful { namespace stat {

class InstCounter: public BBProcessor {
public:
  p::declare reg;
  InstCounter(p::declare& r = reg): BBProcessor(r) { }
protected:
  virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) { ... }
};

p::declare InstCounter::reg = p::init("useful::stat::InstCounter", Version(1, 0, 0))
  .base(BBProcessor::reg)
  .maker<InstCounter>()
  .provide(COUNT_FEATURE);

p::Identifier<int> INST_COUNT("useful::stat::INST_COUNT", -1);
p::feature COUNT_FEATURE("useful::stat::COUNT_FEATURE", new Maker<InstCounter>());

} }   // useful::stat
```

The code of the plugin is mostly ready. Only an object declaring the OTAWA plugin is missing.
This object allows to OTAWA to know that this is, as expected, a real plugin and to
avoid to perform an inconsistent link.

```cpp
namespace useful { namespace stat {
  class Plugin: public ProcessorPlugin {
  public:
    typedef genstruct::Table<AbstractRegistration * > procs_t;
    AbstractRegistration *regs[] = { InstCounter::reg };  // (1)  
    procs_t reg_tab(regs, 1);               // (2)
     
  Plugin(void)
    : ProcessorPlugin("useful::stat", Version(1, 0, 0), OTAWA_PROC_VERSION) { } // (3)
  virtual procs_t& processors(void) const
    { return procs_t::EMPTY; }; // (4)
};

} } useful::stat

useful::stat::Plugin OTAWA_PROC_HOOK;           // (4)
useful::stat::Plugin& useful_stat = OTAWA_PROC_HOOK;    // (5)
```

The code above class a plugin class decicated to the plugin that must inherit from
class `otawa::ProcessorPlugin`. At mark (1), a simple array
with the registration of all provided code processors is created and then wrapped, mark (2),
in a `elm::genstruct::Table`. This table is returned to the user with the virtual method
`ProcessorPlugin::processors`.

More interesting is the construction at mark (3). Arguments includes the plugin name, version
and the version of the interface, constant `OTAWA_PROC_VERSION`. This content allows
(a) to record the current version of plugin interface and (b) to allow to check compatibility
when the plugin is load.

Finally, out of the namespace of the name space of the plugin, a plugin instance must be created.
It is named according to the constant `OTAWA_PROC_HOOK` that contains the real name used by OTAWA
to lookup for the plugin entry. This definition must in the top name space else the C++ mangling
will prevent OTAWA from finding it. In the opposite, the mark (5) is optional and only useful
if static linkage is planned for this plugin.

There are different ways to build a plugin but here is presented an approach based
on [[http://www.cmake.org/|CMake]] and [[http://gcc.gnu.org|GCC]] that works on the OS we support
(Linux, Windows, MaxOSX)((On Windows, [[http://mingw.org/|MinGW]] has been used) without changing the scripts.

To compile the plugin, the //CMake// script below may be used:
```cmake
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# configuration (1)
set(PLUGIN    "stat")     # plugin name
set(NAMESPACE "useful")   # namespace
set(MODULES       )   # used modules (to pass to otawa-config=
set(SOURCES   stat.cpp)   # sources of the plugin

# script (2)
project(${PLUGIN})

# look for OTAWA (3)
if(NOT OTAWA_CONFIG)
  find_program(OTAWA_CONFIG otawa-config DOC "path to otawa-config")
  if(NOT OTAWA_CONFIG)
    message(FATAL_ERROR "ERROR: otawa-config is required !")
  endif()
endif()
message(STATUS "otawa-config at ${OTAWA_CONFIG}")
execute_process(COMMAND "${OTAWA_CONFIG}" --cflags ${MODULES} OUTPUT_VARIABLE OTAWA_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "${OTAWA_CONFIG}" --libs ${MODULES}  OUTPUT_VARIABLE OTAWA_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "${OTAWA_CONFIG}" --prefix OUTPUT_VARIABLE OTAWA_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

# plugin definition (4)
set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../../")
include_directories("${CMAKE_SOURCE_DIR}" ".")
add_library(${PLUGIN} SHARED ${SOURCES})
set_property(TARGET ${PLUGIN} PROPERTY PREFIX "")
set_property(TARGET ${PLUGIN} PROPERTY COMPILE_FLAGS "${OTAWA_CFLAGS}")
target_link_libraries(${PLUGIN} "${OTAWA_LDFLAGS}")

# installation (5)
if(NOT PREFIX)
  set(PREFIX "${OTAWA_PREFIX}")
endif()
set(PLUGIN_PATH "${PREFIX}/lib/otawa/proc/${NAMESPACE}")
if(WIN32 OR WIN64)
  install(TARGETS ${PLUGIN} RUNTIME DESTINATION ${PLUGIN_PATH})
else()
  install(TARGETS ${PLUGIN} LIBRARY DESTINATION ${PLUGIN_PATH})
endif()
```

Section (1) only defines some variables that will be used thereafter while section (2) defines the projet for //CMake//.
Section (3) looks for `otawa-config` to get details for compilation (`OTAWA_CFLAGS`, `OTAWA_LDFLAGS`).
`otawa-config` is either automatically discovered if it is on the path, or may be passed as a parameter
to the script as `-DOTAWA_CONFIG=path-to-otawa-config`.

The section (4) is specially interesting as it builds the plugin itself. The `CMAKE_INSTALL_RPATH`
allows to link back with OTAWA libraries. `$ORIGIN` represents the container directory of the plugin
and, as it may be have installed in the OTAWA standard directories `lib/otawa/proc`, the built path
designs the directory `lib` where OTAWA libraries can be find. The number of `..` mus be adjusted
according to the depth of `${NAMESPACE}` of the plugin. One must also observe that the library
prefix, usually `lib`, is removed from the name of plugin.

The installation is performed in section (5). It is not very clear but it seems that, on Windows,
the plugin must be installed as a `RUNTIME` and not as a `LIBRARY`.

The script has to be recorded as `CMakeList.txt` and can be invoked with:
```sh
$ cmake .
$ make install
```


## Management of Dependencies with Plugin 

Although the plugin techniques are widely used, their support in OS is quite irregular.
This makes the procedure to build them and load them difficult to handle and specially
for the developer. Using `${CMAKE_INSTALL_RPATH}` as in the //CMake// of previous
section is very efficient on Unix-like systems (Linux, MacOSX) but does not work on Windows.

To help a bit, OTAWA supports meta-information with the its plugins. This meta-information
is a file with the same named as the plugin but with `.eld` suffix and is formatted
as Windows initialization files. It may help to pre-load required libraries and plugins,
to mimic symbolic links of Unices, etc.

In `.eld`, OTAWA is only interested in the section named `elm-plugin`. Developers
can add other sections for their own use since they will be ignored by OTAWA.
In this section, the following definitions are considered:

  * `author` -- author of the plugin in the form "AUTHOR <EMAIL>",
  * `copyright` -- name and, optionally and space separated, link to the license,
  * `deps` -- ";" separated list of plugin names to load before the current plugin,
  * `description` -- description of the plugin for human user,
  * `libs` -- ";" separated list of library names or paths (absolute or relative) to load before the current plugin,
  * `name` -- alternative name of the plugin,
  * `path` -- absolute or relative path to the actual plugin (support for aliasing),
  * `rpaths` -- ";" separated list of paths to look for required plugins / libraries,
  * `site` -- URL to the website publising the plugin.

Lets the example of the previous to depend on plugin `useful::handy`, the following `stat.eld` file
can be created:
```ini
[elm-plugin]
author=myself
deps=handy
```

To create an alias name to `useful::stat` named `useful::old_stat`, the file `old_state.eld` below
must be generated:
```ini
[elm-plugin]
name=stat
path=stat
```

Both files must be copied in the same directory as the plugin. This is done by the //CMake// below:
```cmake
install(FILES stat.eld old_stat.eld DESTINATION ${PLUGIN_PATH})
```

# ILP Solver Development
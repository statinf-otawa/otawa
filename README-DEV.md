To compile and install **OTAWA**, follow directives of [README.md](file:README.md).

# INSTALL_TYPE
Installation of **OTAWA** may also be customized passing `INSTALL_TYPE` to CMake invocation
(or modifying in `CMakeCache.txt`). `INSTALL_TYPE` contains a comma-separated combination
of:
  * `lib` to install libraries and data files,
  * `bin` to install commands,
  * `inc` to install development files like includes,
  * `doc` to build and install automatic documentation.


# Testing

To enable the compilation of testing code, run CMake with variable `OTAWA_TEST` set to yes:
```bash
cmake . -DOTAWA_TEST=yes
```


# Source Formatting

The source format used in **OTAWA** is very close to usual C++ source formatting.

## Identifiers

The identifiers must match the following rules:
  * Class identifiers are composed of joined words starting with an upper case and continued
with lower case (eg BasicBlock)
  * Function identifiers are in lower case but, if they are composed of several words, all
words except the first one starts with an upper case and they are join as is (eg loadWorkspace). 
  * Name space, parameters and local variable identifiers are low case and if they contains
several words, are separated by `_`.
  * Member variables or attributes follows the same rules as name spaces, parameters and
variables but are prefixed by `_`.
  * Accessors of a member variable `_x` are named  `x` and _x_ must match the rule
of methods or member functions.
  * Setters of a member variable `_x`are named `setX` and _X_ must match the rule
of method or member functions.
  * Local variables or member variables, not aimed to be read by the class use, must
be as short as possible (typically 1 character or most often no more than 4 characters).
  * For other identifier, try to keep them short but readable.
  * Except for some type name suffixed by `_t`, no other type mark is accepted on the
identifiers.


## Files

	The indentation is made using 1 tabulation that must be configured, in your preferred
	text editor, to represent 4 spaces! Lines must be no longer than 80 characters!

Any header file for a class _Class_ defined in namespace _module_ must be
declared in a header file with path `include/otawa/module/Class.h` with the
following format:
```C++
/*
 *	Class interface
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
#ifndef OTAWA_OTAWA_MODULE_CLASS_H
#define OTAWA_OTAWA_MODULE_CLASS_H

namespace otawa { namespace module {

...

} }	// otawa::module

#endif	// OTAWA_MODULE_CLASS_H
```

Whatever the type of the class (template or not), a source file must exist
with name Class.cpp following the format:
```C++
/*
 *	Class implementation
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

namespace otawa { namespace module {

...

} }	// otawa::module
```

## Class Declaration

A class is declared as usual in C++ keeping the `class` declaration and
the extension as much possible on the same line. Then comes the different
parts of the class:
  * list of friends if any
  * list of private types if any
  * `public:` keyword
  * list of public types if any
  * constructor/destructors
  * function members (static or not)
  * `private:` keyword
  * private variable members
  * private function members

After the class, an overload of operators `<<` or `>>` can be provided.

For evolution purpose, it is advised to declare member function as private.
In most cases, getter and setters are easy to write and may prevent break
in future evolution. Public member variable are only permitted in very
rare and special cases to make easier the use of a class.

Inline function members must embed their body (if possible) in the class declaration.
Except for template classes, the body if an inline function must be no longer
than 1 line.


## Statement Writing

The goal of the following is to make the code as readable as possible and
therefore as shorter and light as possible.

	Braces, ``{`` or ``}``, must be avoided as much as possible because (a) they make
	longer the code and (b) they may obfuscate the reading.

	Only 1 statement per line is accepted.

Simple `if`(_c_) _s_; instructions must be formatted as:
```C++
if(c)
	s;
```

`if` with `else` must be written:
```C++
if(c)
	s1;
else
	s2;
```

Other complex statement can be written:
```C++
while(c)
	s;

do
	s;
while(c);
```

If a sub-statement is composed, requiring the use of braces, the open brace
starts on the `if`/`while`/`do` line. The content of the braces is
indented and the closing brace is on the next not-indented line.
```C++
if(c) {
	s1;
	s2;
	...
}
```

The switch must match the following format:
```C++
switch(c) {
case c1:
	...
	break;
case c2:
	...
	break;
...
}
```

If the statements of a case is very short, a one line case is also accepted:
```C++
switch(c) {
case c1: ...; break;
case c2: ...; break;
...
}
```

`return`, `break` and `continue` are permitted as they make simpler the main flow
of the algorithm. The `goto` may be permitted in `switch` in order to
factorize come code parts.

It is a good idea to structure the code in block prefixed by a line of comments
explaining the block. Formulae in UTF-8 are allowed and even encouraged as
they may play the role of assertions and avoid long explanations. 

The functions must very shorts: typically 10-15 lines shorts. The only exception
concerns very long `switch` where each case is mostly independent.



## Expression Writing

The expressions syntax must follow the usual English typography:
  * no space between unary operators and their argument,
  * 1 space between binary or ternary operator and their arguments,
  * no space after `(` or `[` or before `]` or `)`,
  * no space before a `,` but 1 space after a `,`.

When a function call is too long to fit a line width, a new line must be inserted
after the call open parenthesis and the arguments are written on the next line,
one argument per line followed by a `,` or the closing `)`.


## C++ Idiom

`#define` must be avoided as much as possible: use `const` variable instead.

If available, prefer an ELM class instead of an STL class.

Ternary operator, ... `?` ... `:` ..., must be avoided as they make the syntax
very obscure.

An overloaded virtual member function must be suffixed by ''override''.

The use of ellipsis parameter `...` is forbidden. Operator `<<` overloading
is a good idea to maintain typing, safety and genericity.

Avoid `this->` notation as much as possible (not required).


# Source Documentation

The source documentation is only done in source, `.cpp`, files and is supported
by Doxygen. Refer to  for tag details.

A class named Class is documented as follows:
```C++
/**
 * @class Class
 * ...
 */
```

A member function is documented as before the definition of the function.

A inline function named _type_ _Class_::_fun_(...) is documented by:
```C++
/**
 * @fn type Class:fun(...);
 * ...
 */
```


Private variables are usually not documented.


## Formatted documentation

Doxygen uses the Markdown wiki-notation to format text.
A non-exhaustive list of the tags is given below:
  * \n to create a new paragraph,
  * title \n ==== or # title to create a section
  * title \n ---- or ## title to create a new section
  * > text to create a blockquote
  * */+/- text for a bulleted list
  * 1. text for a numbered list
  * \t text for code block
  * - - - for horizontal rule
  * [text](url) for a link
  * *text* or _text_ for emphasized text
  * **text** or __text__ for even more emphasized text
  * \c to avoid c to be used as special character
  * `text` for code text
  * ![alternate text](url) to include an image
  * ![alternate text](url caption) to include an image with caption
  * <url> for explicit link


  
# PARALLEL OTAWA

The development of multi-cores provides an utterly important sources of computation power
but the price is the adoption of parallel way of programming. Unfortunately, this means
that an application is split thread running concurrently with the possibility
of synchronization problem leading to the use of time costly synchronization facilities
that may possibly waste lot of computation power or memory space. In OTAWA, we will follow
a lock-free approach as much as possible whose principles will be described here.


## Principles

The main parallelization problems concerns the property lists that are the main storage
of information for analyzes.

[P0] OTAWA concurrent execution only works on microprocessor ensuring atomic read or write
of pointers.

[P1] at some time t of the computation, a property list is owned by at most 1 thread.

[P2] only the owner thread can modify the property list (change a property, add a new property
or remove a property).

[P3] not-owner threads of a property can visit its property at any time without being
disturbed by any change to the properties.

[P4] Property list being a data structure too small, its changes must be done in an atomic way.

[P5] Only store of basic types are atomic. Basic types includes bool, pointer, integer o
 8, 16 and 32-bits integer, float and cstring on 32-bits machines. On 64-bits machine,
 64-bits integer and double are added to basic types.
 
 [P6] Analyzes must adopt these principles to be OTAWA parallel-compliant. If they don't,
 they will be executed in mono-thread way. The code processor will record which analyzes
 are parallel-compliant.
 
 [P7] Tasks executed on two different thread can not rely on the property for synchronization.
 If a synchronization is needed (but it is discouraged), the analyzes must provide their own
 way to implement it. As a consequence, order of update of the property list is irrelevant
 to implement an analyzes unless an external synchronization facility is provided.
 
 
## Lock-free Property Lists
 
Only the owner can modify the property list. A property list has the following structure:
```C++
 	template <class T>
 	class Property {
 		Property *next;
 		T value;
 	};
 	class PropertyList {
 		Property *head;
 	};
```

As described below, property will implement read-copy-update (RCU) approach
with the owner of the property list being the lonely updater.
 
Adding a list must be done in an atomic way, set of a pointer and, therefore,
must be done by inserting a property at the header (the faster way) or at the
end. This way, a reader may or may not show the property at insertion time
but this behavior is compliant with P7.

```C++
 	void addProp(PropertList *props, Property *prop) {
 		prop->next = props->header;
 		// property list is not modified by the owner thread that is the only modifier
 		props->head = prop;
 		// atomic modification of the list: new list includes now new property and old properties
 	}
```
 
 Changing an existing value must be done an atomic way. That is the value in Property must be of type
 T for basic types and T* for non-basic type. Whatever the case, this will be assure atomic change
 of the set value without disturbing an existing reader.
 
The deletion is trickier because a reader may be reading the removed property at removal time.
The main ideas will be (a) to change the pointer of next of the previous property in an atomic way
and (b) to preserve the removed property until all reader has left the property. This last action
is maybe the more complex one: when could we be sure that there is no more reader on a particular
property?
https://www.efficios.com/pub/rcu/urcu-supp.pdf.

  * When a synthetic method is used (get, set, remove, add), we now that the traversal will be
  short in time (less than 1 second for example and we can have a list of delayed properties
  with a kind of time but the implementation of the list must be lock-free but it may be easy
  using the atomic write of next field and one list by thread).
  * Property iterators make things more complex. In parallel analyzes, iterators should be
  replaced by collector methods that will fill a data structures with properties matching a
  specific identifier ensuring the read of the property list to be short in time.
```C++
  template <class T>
  	void collect(AbstractIdentifier& id, genstruct::Vector<T>& data);
```


## Lock-free Assignment of Tasks

As the property lists are implemented using lock-free approaches, an analysiss will typically divide
its work in jobs, that is group of data to be processed. For example, a basic block based
analysis will distribute the basic block in jobs (at least as many as available core) and lets
the threads pick tasks until all tasks are performed. This may mean that a thread that has ended
job will pick another one to complete the computation. We will discuss here several ways to implement
this job-aware lock-free mechanism.


## Practical Approach

  * Think to protect data possibly modified by different core using C "volatile" modifier.
  * Be careful with code reorganization of the compiler, put memory barriers around the atomic pointer write!
  	(http://lwn.net/Articles/262464/)


### Issues

The RCU update of ProptyList should work most of the time (while PropertyList access duration, grace time, is
under 1s but is clearly not a guarantee of soundness of the application. Is there is any non-blocking
cheap way to provide such a guarantee?

### Global barrier

For each N core, we can have a collection of bytes for each property list:
	byte M[N]
For a read lock of core _i_:
	M[i] <- 1
For a read unlock of core _i_:
	M[i] <- 0
When all M[i] = 0, no more thread is reading the list and we can free the deleted data.
As it is relatively costly, it may be implemented globally and from time to time, variables
can be looked and owned property by the current could be deleted.

This policy refine in the following way at read unlock:
	M[i] <- 0
	if all M[i] = 0 then signal all thread for deletion

Ensuring the global quiescent state is detected by last entering core
and alert other cores. This will improve the approach but we are not free
of starving (by never reaching global quiescent state). 

### Local Approach

If we could identify the owner of a Property list, let be core o, each core may have its
own set of markers and be alerted as soon there is no more reader on their property list:
	byte M[N][N];

read_lock(o) core i
	M[o][i] <- 1

read_unlock(o) core i
	M[o][i] <- 0
	if all M[o][j] = 0 then signal thread o for deletion

But it would be too costly or not flexible enough to store owner in property list.
This policy may leveraged with:

read_lock() core i
	M[j][i] <- 1 for all j in [0, N[

read_unlock() core i
	M[j][i] <- 0 for all j in [0, N[
	if there a j s.t. M[k][j] for k in [0,N[ then alert core j

It works but `read_lock()` and `read_unlock()` are much more expensive.
A better way would to let Properties cooperate with higher level code
processor to find point of deletion.


### References ====
	* https://en.wikipedia.org/wiki/Non-blocking_algorithm
	* https://en.wikipedia.org/wiki/Software_transactional_memory
	* https://en.wikipedia.org/wiki/Read-copy-update
	* http://www.cl.cam.ac.uk/research/srg/netos/projects/archive/lock-free/
	* http://kukuruku.co/hub/cpp/lock-free-data-structures-introduction

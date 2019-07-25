/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	type.h -- type class implementation.
 */

#include <elm/assert.h>
#include <elm/string.h>
#include <otawa/type.h>
 
namespace otawa {

/**
 * @class NoType
 * Null value for types.
 */


// Type::kind() overload 
Type::kind_t NoType::kind(void) const {
	return NONE;
}


// Type::print() overload
void NoType::print(elm::io::Output& output) const {
	output << "notype";
}


/**
 * @class Type
 * This class describes the type of the data in the program.
 */


/**
 * @fn Type::kind_t Type::kind(void) const;
 * Get the kind of the type.
 * @return The kind of the type.
 */


/**
 * @fn const BaseType *Type::toBase(void) const;
 * If the type is a base type, return it. Else return null.
 * @return Base type or null.
 */


// Constant types
const NoType Type::no_type;
const BaseType Type::void_type(Type::VOID);
const BaseType Type::block_type(Type::BLOCK);
const BaseType Type::bool_type(Type::BOOL);
const BaseType Type::char_type(Type::CHAR);
const BaseType Type::int8_type(Type::INT8);
const BaseType Type::uint8_type(Type::UINT8);
const BaseType Type::int16_type(Type::INT16);
const BaseType Type::uint16_type(Type::UINT16);
const BaseType Type::int32_type(Type::INT32);
const BaseType Type::uint32_type(Type::UINT32);
const BaseType Type::int64_type(Type::INT64);
const BaseType Type::uint64_type(Type::UINT64);
const BaseType Type::float32_type(Type::FLOAT32);
const BaseType Type::float64_type(Type::FLOAT64);
const BaseType Type::float128_type(Type::FLOAT128);
const BaseType Type::addr32_type(Type::ADDR32);
const BaseType Type::cstring_type(Type::CSTRING);
const BaseType Type::string_type(Type::STRING_);

// array of base types
static const BaseType *_base_types[] = {
	&Type::void_type,
	&Type::block_type,
	&Type::bool_type,
	&Type::char_type,
	&Type::int8_type,
	&Type::uint8_type,
	&Type::int16_type,
	&Type::uint16_type,
	&Type::int32_type,
	&Type::uint32_type,
	&Type::int64_type,
	&Type::uint64_type,
	&Type::float32_type,
	&Type::float64_type,
	&Type::float128_type,
	&Type::addr32_type,
	&Type::cstring_type,
	&Type::string_type
};

// array of base types displays
elm::CString _base_displays[] = {
	"void",
	"block",
	"bool",
	"char",
	"int8",
	"uint8",
	"int16",
	"uint16",
	"int32",
	"uint32",
	"int64",
	"uint64",
	"float32",
	"float64",
	"float128",
	"addr32",
	"cstring",
	"string"
};


/**
 * Get a base type from its base type value.
 * @param type	Base type.
 * @return		Matching base type value.
 */
const BaseType& Type::getBaseType(base_t type) {
	ASSERT(type >= 0 && type < BASE_TOP);
	return *_base_types[type];
};


/**
 * Print a text representation of the type.
 * @param output	Output to display to.
 */
void Type::print(elm::io::Output& output) const {
	output << "<not displayable type>";
}


/**
 * @class BaseType
 * This class provides a representation for the base type. It contains a
 * base kind value defining the base type.
 */


/**
 * @fn Type::base_t BaseType::base(void) const;
 * Get the base type kind.
 * @return Base type kind.
 */


// Type overload
void BaseType::print(elm::io::Output& output) const {
	output << _base_displays[bknd];
}

}	// namespace otawa

/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	type.h -- type class implementation.
 */

#include <otawa/type.h>
 
namespace otawa {

/**
 * @class Type
 * This class describes the type of the data in the program.
 */

/**
 * @fn type_kind_t Type::kind(void);
 * Get the kind of the type.
 * @return The kind of the type.
 */

/**
 * @fn BaseType *Type::toBase(void);
 * If the type is a base type, return it. Else return null.
 * @return Base type or null.
 */


/**
 * @class BaseType
 * This class provides a representation for the base type. It contains a
 * base kind value defining the base type.
 */

/**
 * @fn type_base_t BaseType::base(void) const;
 * Get the base type kind.
 * @return Base type kind.
 */

/**
 * Void base type unique representation.
 */
BaseType BaseType::void_type(TYPE_Void);

/**
 * Bit block base type unique representation.
 */
BaseType BaseType::block_type(TYPE_Block);

/**
 * Boolean base type unique representation.
 */
BaseType BaseType::bool_type(TYPE_Bool);

/**
 * Signed 8-bits integer base type unique representation.
 */
BaseType BaseType::int8_type(TYPE_Int8);

/**
 * Unsigned 8-bits integer base type unique representation.
 */
BaseType BaseType::uint8_type(TYPE_UInt8);

/**
 * Signed 16-bits integer base type unique representation.
 */
BaseType BaseType::int16_type(TYPE_Int16);

/**
 * Unsigned 16-bits integer base type unique representation.
 */
BaseType BaseType::uint16_type(TYPE_UInt16);

/**
 * Signed 32-bits integer base type unique representation.
 */
BaseType BaseType::int32_type(TYPE_Int32);

/**
 * Unsigned 32-bits integer base type unique representation.
 */
BaseType BaseType::uint32_type(TYPE_UInt32);

/**
 * Signed 64-bits integerbase type unique representation.
 */
BaseType BaseType::int64_type(TYPE_Int64);

/**
 * Unsigned 64-bits integer base type unique representation.
 */
BaseType BaseType::uint64_type(TYPE_UInt64);

/**
 * 32-bits float base type unique representation.
 */
BaseType BaseType::float32_type(TYPE_Float32);

/**
 * 64-bits float base type unique representation.
 */
BaseType BaseType::float64_type(TYPE_Float64);

/**
 * 128-bits floats base type unique representation.
 */
BaseType BaseType::float128_type(TYPE_Float128);

/**
 * 32-bits address base type unique representation.
 */
BaseType BaseType::addr32_type(TYPE_Addr32);


}	// namespace otawa

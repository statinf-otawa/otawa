/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	type.h -- type class interface.
 */
#ifndef OTAWA_TYPE_H
#define OTAWA_TYPE_H

namespace otawa {


// Defined class
class Type;
class BaseType;
class BlockType;


// Class kind
typedef enum type_kind_t {
	TYPE_None = 0,
	TYPE_Base,
	TYPE_Enum,
	TYPE_Array,
	TYPE_Struct,
	TYPE_Union,
	TYPE_Ptr,
	TYPE_Fun
} type_kind_t;


// Base types
typedef enum type_base_t {
	TYPE_Void = 0,
	TYPE_Block,
	TYPE_Bool,
	TYPE_Int8,
	TYPE_UInt8,
	TYPE_Int16,
	TYPE_UInt16,
	TYPE_Int32,
	TYPE_UInt32,
	TYPE_Int64,
	TYPE_UInt64,
	TYPE_Float32,
	TYPE_Float64,
	TYPE_Float128,
	TYPE_Addr32
} type_base_t;


// Type class
class Type {
public:
	virtual type_kind_t kind(void) = 0;
	virtual BaseType *toBase(void) { return 0; };
};


// BaseType class
class BaseType: public Type {
	type_base_t bknd;
	inline BaseType(type_base_t base_kind): bknd(base_kind) { };
public:

	// Methods
	inline type_base_t base(void) const { return bknd; };
	virtual type_kind_t kind(void) { return TYPE_Base; };
	virtual BaseType *toBase(void) { return this; };
	
	// Constant types
	static BaseType void_type;
	static BaseType block_type;
	static BaseType bool_type;
	static BaseType int8_type;
	static BaseType uint8_type;
	static BaseType int16_type;
	static BaseType uint16_type;
	static BaseType int32_type;
	static BaseType uint32_type;
	static BaseType int64_type;
	static BaseType uint64_type;
	static BaseType float32_type;
	static BaseType float64_type;
	static BaseType float128_type;
	static BaseType addr32_type;
};


} // namespace otawa

#endif // OTAWA_TYPE_H

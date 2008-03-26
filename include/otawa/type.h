/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	type.h -- type class interface.
 */
#ifndef OTAWA_TYPE_H
#define OTAWA_TYPE_H

#include <elm/io.h>

namespace otawa {


// Defined class
class Type;
class BaseType;
class NoType;
class BlockType;


// Type class
class Type {
protected:
	virtual ~Type(void) { }

public:

	// Kind
	typedef enum kind_t {
		NONE = 0,
		BASE,
		ENUM,
		ARRAY,
		STRUCT,
		UNION,
		PTR,
		FUN
	} kind_t;

	// Base types
	typedef enum base_t {
		VOID = 0,
		BLOCK,
		BOOL,
		CHAR,
		INT8,
		UINT8,
		INT16,
		UINT16,
		INT32,
		UINT32,
		INT64,
		UINT64,
		FLOAT32,
		FLOAT64,
		FLOAT128,
		ADDR32,
		CSTRING,
		STRING_,
		BASE_TOP
	} base_t;

	// Predefined types
	static const NoType no_type;
	static const BaseType void_type;
	static const BaseType block_type;
	static const BaseType bool_type;
	static const BaseType char_type;
	static const BaseType int8_type;
	static const BaseType uint8_type;
	static const BaseType int16_type;
	static const BaseType uint16_type;
	static const BaseType int32_type;
	static const BaseType uint32_type;
	static const BaseType int64_type;
	static const BaseType uint64_type;
	static const BaseType float32_type;
	static const BaseType float64_type;
	static const BaseType float128_type;
	static const BaseType addr32_type;
	static const BaseType cstring_type;
	static const BaseType string_type;
	
	// Constructors
	static const BaseType& getBaseType(base_t type);
	
	// Methods
	inline bool equals(const Type& type) const { return this == &type; };
	virtual kind_t kind(void) const = 0;
	virtual const BaseType *toBase(void) const { return 0; };
	virtual void print(elm::io::Output& output) const;
	
	// Operators
	inline bool operator==(const Type& type) const { return equals(type); }
	inline bool operator!=(const Type& type) const { return !equals(type); }
};


// NoType class
class NoType: public Type {
public:
	
	// Type overload
	virtual kind_t kind(void) const;
	virtual void print(elm::io::Output& output) const;
};


// BaseType class
class BaseType: public Type {
	friend class Type;
	base_t bknd;
	inline BaseType(base_t base_kind): bknd(base_kind) { };
public:
	inline base_t base(void) const { return bknd; };
	
	// Type overload
	virtual Type::kind_t kind(void) const { return Type::BASE; };
	virtual const BaseType *toBase(void) const { return this; };
	virtual void print(elm::io::Output& output) const;
};

// typeof function
template <class T> struct __type { static inline const Type& _(void) { return Type::no_type; } };

template <> struct __type<void> { static inline const Type& _(void) { return Type::void_type; } };
template <> struct __type<bool> { static inline const Type& _(void) { return Type::bool_type; } };
template <> struct __type<char> { static inline const Type& _(void) { return Type::char_type; } };
template <> struct __type<signed char> { static inline const Type& _(void) { return Type::int8_type; } };
template <> struct __type<unsigned char> { static inline const Type& _(void) { return Type::uint8_type; } };
template <> struct __type<short> { static inline const Type& _(void) { return Type::int16_type; } };
template <> struct __type<unsigned short> { static inline const Type& _(void) { return Type::uint16_type; } };
template <> struct __type<int> { static inline const Type& _(void) { return Type::int32_type; } };
template <> struct __type<unsigned int> { static inline const Type& _(void) { return Type::uint32_type; } };
template <> struct __type<long> { static inline const Type& _(void) { return Type::int32_type; } };
template <> struct __type<unsigned long> { static inline const Type& _(void) { return Type::uint32_type; } };
template <> struct __type<long long> { static inline const Type& _(void) { return Type::int64_type; } };
template <> struct __type<unsigned long long> { static inline const Type& _(void) { return Type::uint64_type; } };
template <> struct __type<float> { static inline const Type& _(void) { return Type::float32_type; } };
template <> struct __type<double> { static inline const Type& _(void) { return Type::float64_type; } };
template <> struct __type<long double> { static inline const Type& _(void) { return Type::float128_type; } };
template <> struct __type<elm::CString> { static inline const Type& _(void) { return Type::cstring_type; } };
template <> struct __type<elm::String> { static inline const Type& _(void) { return Type::string_type; } };

template <class T> inline const Type& type(void) { return __type<T>::_(); }

} // namespace otawa

#endif // OTAWA_TYPE_H

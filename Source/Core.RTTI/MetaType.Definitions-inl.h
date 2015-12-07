#ifndef DEF_METATYPE_SCALAR
#   define DEF_METATYPE_SCALAR(_Name, T, _TypeId)
#endif

// Here goes the list of all BASIC types supported by RTTI
// Use MetaTypeTraits.h to project your custom type to one of these

#ifndef DEF_METATYPE_SCALAR_ARITH
#   define DEF_METATYPE_SCALAR_ARITH_UNDEF
#   define DEF_METATYPE_SCALAR_ARITH(_Name, T, _TypeId) DEF_METATYPE_SCALAR(_Name, T, _TypeId)
#endif
#ifndef DEF_METATYPE_SCALAR_STRING
#   define DEF_METATYPE_SCALAR_STRING_UNDEF
#   define DEF_METATYPE_SCALAR_STRING(_Name, T, _TypeId) DEF_METATYPE_SCALAR(_Name, T, _TypeId)
#endif
#ifndef DEF_METATYPE_SCALAR_ATOM
#   define DEF_METATYPE_SCALAR_ATOM_UNDEF
#   define DEF_METATYPE_SCALAR_ATOM(_Name, T, _TypeId) DEF_METATYPE_SCALAR(_Name, T, _TypeId)
#endif
#ifndef DEF_METATYPE_SCALAR_OBJECT
#   define DEF_METATYPE_SCALAR_OBJECT_UNDEF
#   define DEF_METATYPE_SCALAR_OBJECT(_Name, T, _TypeId) DEF_METATYPE_SCALAR(_Name, T, _TypeId)
#endif

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DEF_METATYPE_SCALAR_ARITH   (bool,       bool,                           1)
DEF_METATYPE_SCALAR_ARITH   (i8,         i8,                             2)
DEF_METATYPE_SCALAR_ARITH   (i16,        i16,                            3)
DEF_METATYPE_SCALAR_ARITH   (i32,        i32,                            4)
DEF_METATYPE_SCALAR_ARITH   (i64,        i64,                            5)
DEF_METATYPE_SCALAR_ARITH   (u8,         u8,                             6)
DEF_METATYPE_SCALAR_ARITH   (u16,        u16,                            7)
DEF_METATYPE_SCALAR_ARITH   (u32,        u32,                            8)
DEF_METATYPE_SCALAR_ARITH   (u64,        u64,                            9)
DEF_METATYPE_SCALAR_ARITH   (float,      float,                          10)
DEF_METATYPE_SCALAR_ARITH   (double,     double,                         11)
DEF_METATYPE_SCALAR_STRING  (String,     Core::String,                   12)
DEF_METATYPE_SCALAR_STRING  (WString,    Core::WString,                  13)
DEF_METATYPE_SCALAR_ATOM    (MetaAtom,   Core::RTTI::PMetaAtom,          14)
DEF_METATYPE_SCALAR_OBJECT  (MetaObject, Core::RTTI::PMetaObject,        15)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

#ifdef DEF_METATYPE_SCALAR_ARITH_UNDEF
#   undef DEF_METATYPE_SCALAR_ARITH_UNDEF
#   undef DEF_METATYPE_SCALAR_ARITH
#endif
#ifdef DEF_METATYPE_SCALAR_STRING_UNDEF
#   undef DEF_METATYPE_SCALAR_STRING_UNDEF
#   undef DEF_METATYPE_SCALAR_STRING
#endif
#ifdef DEF_METATYPE_SCALAR_ATOM_UNDEF
#   undef DEF_METATYPE_SCALAR_ATOM_UNDEF
#   undef DEF_METATYPE_SCALAR_ATOM
#endif
#ifdef DEF_METATYPE_SCALAR_OBJECT_UNDEF
#   undef DEF_METATYPE_SCALAR_OBJECT_UNDEF
#   undef DEF_METATYPE_SCALAR_OBJECT
#endif

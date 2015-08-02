#ifndef DEF_METATYPE_SCALAR
#   define DEF_METATYPE_SCALAR(T, _TypeId)
#endif

// Here goes the list of all BASIC types supported by RTTI
// Use MetaTypeTraits.h to project your custom type to one of these

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DEF_METATYPE_SCALAR(bool,           1)
DEF_METATYPE_SCALAR(i8,             2)
DEF_METATYPE_SCALAR(i16,            3)
DEF_METATYPE_SCALAR(i32,            4)
DEF_METATYPE_SCALAR(i64,            5)
DEF_METATYPE_SCALAR(u8,             6)
DEF_METATYPE_SCALAR(u16,            7)
DEF_METATYPE_SCALAR(u32,            8)
DEF_METATYPE_SCALAR(u64,            9)
DEF_METATYPE_SCALAR(float,         10)
DEF_METATYPE_SCALAR(double,        11)
DEF_METATYPE_SCALAR(String,        12)
DEF_METATYPE_SCALAR(WString,       13)
DEF_METATYPE_SCALAR(PMetaAtom,     14)
DEF_METATYPE_SCALAR(PMetaObject,   15)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

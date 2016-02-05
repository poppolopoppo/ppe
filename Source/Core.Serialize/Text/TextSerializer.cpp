#include "stdafx.h"

#include "TextSerializer.h"

#include "Core/Container/HashSet.h"
#include "Core/Container/Vector.h"
#include "Core/Memory/MemoryStream.h"

#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaProperty.h"
#include "Core.RTTI/MetaTransaction.h"

#include "Lexer/Lexer.h"

#include "Parser/ParseExpression.h"
#include "Parser/ParseItem.h"
#include "Parser/ParseList.h"

#include "Grammar.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class TextSerialize_ {
public:
    TextSerialize_(const TextSerializer* owner) : _owner(owner), _indent(0), _newline(false) { Assert(owner); }

    TextSerialize_(const TextSerialize_& ) = delete;
    TextSerialize_& operator =(const TextSerialize_& ) = delete;

    void Append(const RTTI::MetaObject* object, bool topObject = true);
    void Append(const MemoryView<const RTTI::PMetaObject>& objects, bool topObject = true);
    void Finalize(IStreamWriter* writer);

private:
    class AtomWriter_ : public RTTI::MetaAtomWrapCopyVisitor {
    public:
        typedef RTTI::MetaAtomWrapCopyVisitor parent_type;
        AtomWriter_(TextSerialize_* owner) : _owner(owner) {}

        using parent_type::Append;
        using parent_type::Inspect;
        using parent_type::Visit;

        virtual void Inspect(const RTTI::IMetaAtomPair* ppair, const RTTI::Pair<RTTI::PMetaAtom, RTTI::PMetaAtom>& pair) override {
            UNUSED(ppair);
            _owner->Print("(");
            Append(pair.first.get());
            _owner->Print(", ");
            Append(pair.second.get());
            _owner->Print(")");
        }

        virtual void Inspect(const RTTI::IMetaAtomVector* pvector, const RTTI::Vector<RTTI::PMetaAtom>& vector) override {
            UNUSED(pvector);
            if (vector.empty()) {
                _owner->Print("[]");
            }
            else {
                _owner->Puts("[");
                _owner->IncIndent();
                for(const RTTI::PMetaAtom& atom : vector) {
                    Append(atom.get());
                    _owner->Puts(",");
                }
                _owner->DecIndent();
                _owner->Print("]");
            }
        }

        virtual void Inspect(const RTTI::IMetaAtomDictionary* pdictionary, const RTTI::Dictionary<RTTI::PMetaAtom, RTTI::PMetaAtom>& dictionary) override {
            UNUSED(pdictionary);
            if (dictionary.empty()) {
                _owner->Print("{}");
            }
            else {
                _owner->Puts("{");
                _owner->IncIndent();
                for(const RTTI::Pair<RTTI::PMetaAtom, RTTI::PMetaAtom>& pair : dictionary) {
                    _owner->Print("(");
                    Append(pair.first.get()); // do not call Visit(pair) !
                    _owner->Print(", ");
                    Append(pair.second.get());
                    _owner->Puts("),");
                }
                _owner->DecIndent();
                _owner->Print("}");
            }
        }

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
        virtual void Visit(const RTTI::MetaTypedAtom<T>* scalar) override { \
            Assert(scalar); \
            Assert(_TypeId == scalar->TypeInfo().Id); \
            WriteValue_(scalar->Wrapper()); \
            /*parent_type::Visit(scalar);*/ \
        }
        FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR

    private:
        static bool Numeric_(bool b) { return b; }
        template <typename T>
        static typename std::enable_if< std::is_integral<T>::value && std::is_signed<T>::value, i64 >::type Numeric_(const T& n) {
            return checked_cast<i64>(n);
        }
        template <typename T>
        static typename std::enable_if< std::is_integral<T>::value && false == std::is_signed<T>::value, u64 >::type Numeric_(const T& n) {
            return checked_cast<u64>(n);
        }
        template <typename T>
        static typename std::enable_if< std::is_floating_point<T>::value, double >::type Numeric_(const T& d) {
            return double(d);
        }

        template <typename T>
        void WriteValue_(const T& value) {
            _owner->PrintFormat(Numeric_(value));
        }

        void WriteValue_(const String& str) {
            static char const* const hexdig = "0123456789ABCDEF";
            _owner->Print("\"");
            for (char c : str) {
                if (' ' <= c && c <= '~' && c != '\\' && c != '"') {
                    _owner->_oss.WritePOD(c);
                }
                else {
                    _owner->_oss.WritePOD('\\');
                    switch(c) {
                        case '"':  _owner->_oss.WritePOD('"');  break;
                        case '\\': _owner->_oss.WritePOD('\\'); break;
                        case '\t': _owner->_oss.WritePOD('t');  break;
                        case '\r': _owner->_oss.WritePOD('r');  break;
                        case '\n': _owner->_oss.WritePOD('n');  break;
                        default:
                            _owner->_oss.WritePOD('x');
                            _owner->_oss.WritePOD(hexdig[c >> 4]);
                            _owner->_oss.WritePOD(hexdig[c & 0xF]);
                    }
                }
            }
            _owner->Print("\"");
        }

        void WriteValue_(const WString& wstr) {
            String str = ToString(wstr);
            WriteValue_(str);
        }

        template <typename T, size_t _Dim>
        void WriteValue_(const ScalarVector<T, _Dim>& v) {
            _owner->Print("[");
            _owner->PrintFormat(Numeric_(v._data[0]));
            forrange(i, 1, _Dim) {
                _owner->Print(", ");
                _owner->PrintFormat(Numeric_(v._data[i]));
            }
            _owner->Print("]");
        }

        template <typename T, size_t _Width, size_t _Height>
        void WriteValue_(const ScalarMatrix<T, _Width, _Height>& v) {
            _owner->Print("[");
            _owner->PrintFormat(Numeric_(v.data().raw[0]));
            forrange(i, 1, _Width*_Height) {
                _owner->Print(", ");
                _owner->PrintFormat(Numeric_(v.data().raw[i]));
            }
            _owner->Print("]");
        }

        void WriteValue_(const RTTI::PMetaAtom& atom) {
            if (atom) {
                const RTTI::MetaTypeId typeId = atom->TypeInfo().Id;
                switch(typeId)
                {
#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) case _TypeId:
                FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR
                    atom->Accept(this);
                    break;

                default:
                    throw TextSerializerException("no support for abstract RTTI atoms serialization");
                }
            }
            else
                _owner->Print("nil");
        }

        void WriteValue_(const RTTI::PMetaObject& object) {
            if (object && object->RTTI_IsExported()) {
                _owner->Print(object->RTTI_Name().MakeView());
                _owner->QueueObject_(object.get());
            }
            else {
                _owner->WriteObject_(*this, object.get());
            }
        }

        TextSerialize_* _owner;
    };

    void IncIndent() { ++_indent; }
    void DecIndent() { Assert(0 < _indent); --_indent; }
    void Indent_();

    void Print(const StringSlice& str);
    template <size_t _Dim>
    void Print(const char (&cstr)[_Dim]) { Print(MakeStringSlice(cstr)); }

    void Puts(const StringSlice& str);
    template <size_t _Dim>
    void Puts(const char (&cstr)[_Dim]) { Puts(MakeStringSlice(cstr)); }

    template <typename T>
    void PrintFormat(const T& value) {
        STACKLOCAL_OCSTRSTREAM(fmt, 2048);
        fmt << value;
        Print(fmt.MakeView());
    }

    void ProcessQueue_();
    void QueueObject_(const RTTI::MetaObject* object);
    void WriteObject_(AtomWriter_& writer, const RTTI::MetaObject* object);

    bool _newline;
    size_t _indent;
    const TextSerializer* _owner;
    MEMORYSTREAM_THREAD_LOCAL(Serialize) _oss;
    VECTOR_THREAD_LOCAL(Serialize, RTTI::PCMetaObject) _queue;
    HASHSET_THREAD_LOCAL(Serialize, RTTI::PCMetaObject) _exported;
};
//----------------------------------------------------------------------------
void TextSerialize_::Append(const RTTI::MetaObject* object, bool topObject /* = true */) {
    Assert(false == topObject || nullptr != object);
    QueueObject_(object);
    ProcessQueue_();
}
//----------------------------------------------------------------------------
void TextSerialize_::Append(const MemoryView<const RTTI::PMetaObject>& objects, bool topObject /* = true */) {
    for (const RTTI::PMetaObject& object : objects) {
        QueueObject_(object.get());
    }
    ProcessQueue_();
}
//----------------------------------------------------------------------------
void TextSerialize_::Finalize(IStreamWriter* writer) {
    Assert(writer);
    writer->WriteView(_oss.MakeView());
}
//----------------------------------------------------------------------------
void TextSerialize_::QueueObject_(const RTTI::MetaObject* object) {
    if (object) {
        if (object->RTTI_IsExported()) {
            const auto it = _exported.insert(object);
            if (false == it.second)
                return;
        }
        _queue.emplace_back(object);
    }
}
//----------------------------------------------------------------------------
void TextSerialize_::WriteObject_(AtomWriter_& writer, const RTTI::MetaObject* object) {
    if (object) {
        if (object->RTTI_IsExported()) {
            Print("export ");
            Print(object->RTTI_Name().MakeView());
            Print(" ");
        }

        const RTTI::MetaClass* metaClass = object->RTTI_MetaClass();
        Assert(metaClass);

        Print(metaClass->Name().MakeView());
        Puts("(");
        IncIndent();

        for (const RTTI::UCMetaProperty& prop : metaClass->Properties()) {
            if (prop->IsDefaultValue(object))
                continue;

            Print(prop->Name().MakeView());
            Print(" = ");

            const RTTI::PMetaAtom atom = prop->WrapCopy(object);
            writer.Append(atom.get());

            Puts("");
        }

        DecIndent();
        Print(")");
    }
    else {
        Print("nil");
    }
}
//----------------------------------------------------------------------------
void TextSerialize_::ProcessQueue_() {
    AtomWriter_ writer(this);
    while (_queue.size()) {
        RTTI::PCMetaObject object = _queue.back();
        _queue.pop_back();

        WriteObject_(writer, object.get());
        Puts("");
    }
}
//----------------------------------------------------------------------------
void TextSerialize_::Indent_() {
    if (_newline) {
        _newline = false;
        forrange(i, 0, _indent)
            _oss.WriteCStr("    ");
    }
}
//----------------------------------------------------------------------------
void TextSerialize_::Print(const StringSlice& str) {
    Indent_();
    _oss.WriteView(str);
}
//----------------------------------------------------------------------------
void TextSerialize_::Puts(const StringSlice& str) {
    Indent_();
    _oss.WriteView(str);
    _oss.WriteCStr("\r\n");
    _newline = true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TextSerializer::TextSerializer() {}
//----------------------------------------------------------------------------
TextSerializer::~TextSerializer() {}
//----------------------------------------------------------------------------
void TextSerializer::Deserialize(RTTI::MetaTransaction* transaction, const MemoryView<const u8>& input, const wchar_t *sourceName /* = nullptr */) {
    Assert(transaction);
    Assert(input.SizeInBytes());

    Lexer::Lexer lexer(input.Cast<const char>(), sourceName);

    Parser::ParseList parseList(&lexer);
    Parser::ParseContext parseContext;

    while (Parser::PCParseItem result = Grammar_Parse(parseList)) {
        const Parser::ParseExpression *expr = result->As<Parser::ParseExpression>();
        if (expr) {
            const RTTI::PMetaAtom atom = expr->Eval(&parseContext);
            const auto* object = atom->As<RTTI::PMetaObject>();
            AssertRelease(object);

            if (object->Wrapper())
                transaction->Add(object->Wrapper().get());
        }
    }
}
//----------------------------------------------------------------------------
void TextSerializer::Serialize(IStreamWriter* output, const RTTI::MetaTransaction* transaction) {
    Assert(output);
    Assert(transaction);

    TextSerialize_ serialize(this);
    serialize.Append(transaction->MakeView());
    serialize.Finalize(output);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core

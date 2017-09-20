#include "stdafx.h"

#include "TextSerializer.h"

#include "Core.RTTI/MetaTransaction.h"

#include "Lexer/Lexer.h"

#include "Parser/ParseExpression.h"
#include "Parser/ParseItem.h"
#include "Parser/ParseList.h"

#include "Grammar.h"

#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaProperty.h"

#include "Core/Container/HashSet.h"
#include "Core/Container/Vector.h"
#include "Core/IO/BufferedStreamProvider.h"
#include "Core/Memory/MemoryStream.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FTextSerialize_ {
public:
    FTextSerialize_(const FTextSerializer* owner, IBufferedStreamWriter* writer)
        : _newline(false)
        , _indent(0)
        , _owner(owner)
        , _writer(writer) {
        Assert(owner);
        Assert(writer);
    }

    FTextSerialize_(const FTextSerialize_& ) = delete;
    FTextSerialize_& operator =(const FTextSerialize_& ) = delete;

    const FTextSerializer* Owner() const { return _owner; }

    void Append(const RTTI::FMetaObject* object, bool topObject = true);
    void Append(const TMemoryView<const RTTI::PMetaObject>& objects, bool topObject = true);

private:
    class FAtomWriter_ : public RTTI::FMetaAtomWrapCopyVisitor {
    public:
        typedef RTTI::FMetaAtomWrapCopyVisitor parent_type;
        FAtomWriter_(FTextSerialize_* owner) : _owner(owner) {}

        using parent_type::Append;
        using parent_type::Inspect;
        using parent_type::Visit;

        virtual void Inspect(const RTTI::IMetaAtomPair* ppair, const RTTI::TPair<RTTI::PMetaAtom, RTTI::PMetaAtom>& pair) override {
            UNUSED(ppair);
            _owner->Print("(");
            Append(pair.first.get());
            _owner->Print(", ");
            Append(pair.second.get());
            _owner->Print(")");
        }

        virtual void Inspect(const RTTI::IMetaAtomVector* pvector, const RTTI::TVector<RTTI::PMetaAtom>& vector) override {
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

        virtual void Inspect(const RTTI::IMetaAtomDictionary* pdictionary, const RTTI::TDictionary<RTTI::PMetaAtom, RTTI::PMetaAtom>& dictionary) override {
            UNUSED(pdictionary);
            if (dictionary.empty()) {
                _owner->Print("{}");
            }
            else {
                _owner->Puts("{");
                _owner->IncIndent();
                for(const RTTI::TPair<RTTI::PMetaAtom, RTTI::PMetaAtom>& pair : dictionary) {
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
        virtual void Visit(const RTTI::TMetaTypedAtom<T>* scalar) override { \
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

        void WriteValue_(const FString& str) {
            static const char hexdig[] = "0123456789ABCDEF";
            _owner->Print("\"");
            for (char c : str) {
                if (' ' <= c && c <= '~' && c != '\\' && c != '"') {
                    _owner->_writer->WritePOD(c);
                }
                else {
                    _owner->_writer->WritePOD('\\');
                    switch(c) {
                        case '"':  _owner->_writer->WritePOD('"');  break;
                        case '\\': _owner->_writer->WritePOD('\\'); break;
                        case '\t': _owner->_writer->WritePOD('t');  break;
                        case '\r': _owner->_writer->WritePOD('r');  break;
                        case '\n': _owner->_writer->WritePOD('n');  break;
                        default:
                            _owner->_writer->WritePOD('x');
                            _owner->_writer->WritePOD(hexdig[c >> 4]);
                            _owner->_writer->WritePOD(hexdig[c & 0xF]);
                    }
                }
            }
            _owner->Print("\"");
        }

        void WriteValue_(const FWString& wstr) {
            const FString str = ToString(wstr);
            WriteValue_(str);
        }

        void WriteValue_(const RTTI::FName& name) {
            _owner->Print("Name:\"");
            _owner->Print(name.MakeView());
            _owner->Print("\"");
        }

        void WriteValue_(const RTTI::FBinaryData& rawdata) {
            if (rawdata.empty()) {
                _owner->Print("BinaryData:\"\"");
            }
            else {
                static const char hexdigit[17] = "0123456789abcdef";
                char hexbyte[4] = {'\\','x','0','0'};
                _owner->Print("BinaryData:\"");
                for (const u8& b : rawdata) {
                    if (IsAlnum((char)b)) {
                        _owner->_writer->WritePOD((char)b);
                    }
                    else {
                        hexbyte[2] = hexdigit[b>>4];
                        hexbyte[3] = hexdigit[b&15];
                        _owner->_writer->WritePOD(hexbyte);
                    }
                }
                _owner->Print("\"");
            }
        }

        void WriteValue_(const RTTI::FOpaqueData& opaqueData) {
            if (opaqueData.empty()) {
                _owner->Print("OpaqueData:{}");
            }
            else {
                _owner->Puts("OpaqueData:{");
                _owner->IncIndent();
                for(const RTTI::TPair<RTTI::FName, RTTI::PMetaAtom>& pair : opaqueData) {
                    _owner->Print("(");
                    WriteValue_(pair.first);
                    _owner->Print(", ");
                    Append(pair.second.get());
                    _owner->Puts("),");
                }
                _owner->DecIndent();
                _owner->Print("}");
            }
        }

        template <typename T, size_t _Dim>
        void WriteValue_(const TScalarVector<T, _Dim>& v) {
            const RTTI::FMetaTypeInfo type = RTTI::TypeInfo<TScalarVector<T, _Dim>>();
            _owner->Print(type.Name);
            _owner->Print(":[");
            _owner->PrintFormat(Numeric_(v._data[0]));
            forrange(i, 1, _Dim) {
                _owner->Print(", ");
                _owner->PrintFormat(Numeric_(v._data[i]));
            }
            _owner->Print("]");
        }

        template <typename T, size_t _Width, size_t _Height>
        void WriteValue_(const TScalarMatrix<T, _Width, _Height>& v) {
            const RTTI::FMetaTypeInfo type = RTTI::TypeInfo<TScalarMatrix<T, _Width, _Height>>();
            _owner->Print(type.Name);
            _owner->Print(":[");
            _owner->PrintFormat(Numeric_(v.data().raw[0]));
            forrange(i, 1, _Width*_Height) {
                _owner->Print(", ");
                _owner->PrintFormat(Numeric_(v.data().raw[i]));
            }
            _owner->Print("]");
        }

        void WriteValue_(const RTTI::PMetaAtom& atom) {
            if (atom) {
                const RTTI::FMetaTypeId typeId = atom->TypeInfo().Id;
                switch(typeId)
                {
#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) case _TypeId:
                FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR
                    atom->Accept(this);
                    break;

                default:
                    CORE_THROW_IT(FTextSerializerException("no support for abstract RTTI atoms serialization"));
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

        FTextSerialize_* _owner;
    };

    void IncIndent() { ++_indent; }
    void DecIndent() { Assert(0 < _indent); --_indent; }
    void Indent_();

    void Print(const FStringView& str);
    void Puts(const FStringView& str);

    template <typename T>
    void PrintFormat(const T& value) {
        STACKLOCAL_OCSTRSTREAM(fmt, 2048);
        fmt << value;
        Print(fmt.MakeView());
    }

    void ProcessQueue_();
    void QueueObject_(const RTTI::FMetaObject* object);
    void WriteObject_(FAtomWriter_& writer, const RTTI::FMetaObject* object);

    bool _newline;
    size_t _indent;
    const FTextSerializer* _owner;
    IBufferedStreamWriter* _writer;
    VECTOR_THREAD_LOCAL(Serialize, RTTI::PCMetaObject) _queue;
    HASHSET_THREAD_LOCAL(Serialize, RTTI::PCMetaObject) _exported;
};
//----------------------------------------------------------------------------
void FTextSerialize_::Append(const RTTI::FMetaObject* object, bool topObject /* = true */) {
    UNUSED(topObject);
    Assert(false == topObject || nullptr != object);
    QueueObject_(object);
    ProcessQueue_();
}
//----------------------------------------------------------------------------
void FTextSerialize_::Append(const TMemoryView<const RTTI::PMetaObject>& objects, bool topObject /* = true */) {
    UNUSED(topObject);
    for (const RTTI::PMetaObject& object : objects) {
        QueueObject_(object.get());
    }
    ProcessQueue_();
}
//----------------------------------------------------------------------------
void FTextSerialize_::QueueObject_(const RTTI::FMetaObject* object) {
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
void FTextSerialize_::WriteObject_(FAtomWriter_& writer, const RTTI::FMetaObject* object) {
    if (object) {
        if (object->RTTI_IsExported()) {
            Print("export ");
            Print(object->RTTI_Name().MakeView());
            Print(" ");
        }

        const RTTI::FMetaClass* metaClass = object->RTTI_MetaClass();
        Assert(metaClass);

        Print(metaClass->Name().MakeView());
        Puts("(");
        IncIndent();

        while (metaClass) {
            for (const RTTI::UCMetaProperty& prop : metaClass->Properties()) {
                if (prop->IsDefaultValue(object))
                    continue;

                Print(prop->Name().MakeView());
                Print(" = ");

                const RTTI::PMetaAtom atom = prop->WrapCopy(object);
                writer.Append(atom.get());

                Puts("");
            }

            metaClass = metaClass->Parent();
        }

        DecIndent();
        Print(")");
    }
    else {
        Print("nil");
    }
}
//----------------------------------------------------------------------------
void FTextSerialize_::ProcessQueue_() {
    FAtomWriter_ writer(this);
    while (_queue.size()) {
        RTTI::PCMetaObject object = _queue.back();
        _queue.pop_back();

        WriteObject_(writer, object.get());
        Puts("");
    }
}
//----------------------------------------------------------------------------
void FTextSerialize_::Indent_() {
    if (_newline) {
        _newline = false;
        forrange(i, 0, _indent)
            _writer->WriteView("    ");
    }
}
//----------------------------------------------------------------------------
void FTextSerialize_::Print(const FStringView& str) {
    Indent_();
    _writer->WriteView(str);
}
//----------------------------------------------------------------------------
void FTextSerialize_::Puts(const FStringView& str) {
    Indent_();
    _writer->WriteView(str);
    _writer->WriteView("\r\n");
    _newline = true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextSerializer::FTextSerializer() {}
//----------------------------------------------------------------------------
FTextSerializer::~FTextSerializer() {}
//----------------------------------------------------------------------------
void FTextSerializer::Deserialize(RTTI::FMetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName /* = nullptr */) {
    Assert(transaction);
    Assert(input);

    Parser::FParseList parseList;
    UsingBufferedStream(input, [&parseList, sourceName](IBufferedStreamReader* buffered) {
        Lexer::FLexer lexer(buffered, MakeStringView(sourceName, Meta::FForceInit{}), true);
        parseList.Parse(&lexer);
    });

    Parser::FParseContext parseContext;

    while (Parser::PCParseItem result = FGrammarStartup::Parse(parseList)) {
        const Parser::FParseExpression *expr = result->As<Parser::FParseExpression>();
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
void FTextSerializer::Serialize(IStreamWriter* output, const RTTI::FMetaTransaction* transaction) {
    Assert(output);
    Assert(transaction);

    UsingBufferedStream(output, [this, transaction](IBufferedStreamWriter* buffered) {
        FTextSerialize_ serialize(this, buffered);
        serialize.Append(transaction->MakeView());
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core

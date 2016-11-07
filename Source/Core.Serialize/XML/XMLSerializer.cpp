#include "stdafx.h"

#include "XMLSerializer.h"

#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaProperty.h"
#include "Core.RTTI/MetaTransaction.h"

#include "Core/Container/HashSet.h"
#include "Core/Container/Vector.h"
#include "Core/IO/Format.h"
#include "Core/IO/StreamProvider.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FXMLEscaped_ {
    FStringView Str;

    template <typename _OStream>
    void Serialize(_OStream& oss) const {
        for (char ch : Str) {
            if (IsAlnum(ch) || '-' == ch || '_' == ch || '.' == ch || '~' == ch) {
                oss(ch);
            }
            else if (IsSpace(ch)) {
                oss('+');
            }
            else {
                const size_t x0 = unsigned char(ch) >> 4;
                const size_t x1 = unsigned char(ch) & 15;

                oss('%');
                oss(char(x0 + (x0 > 9 ? 'A' - 10 : '0')));
                oss(char(x1 + (x1 > 9 ? 'A' - 10 : '0')));
            }
        }
    }

    inline friend void operator <<(
        IStreamWriter& output,
        const FXMLEscaped_& escaped ) {
        escaped.Serialize([&output](char  ch) { output.WritePOD(ch); });
    }

    inline friend std::basic_ostream<char>& operator <<(
        std::basic_ostream<char>& oss,
        const FXMLEscaped_& escaped ) {
        escaped.Serialize([&oss](char  ch) { oss.put(ch); });
        return oss;
    }
};
//----------------------------------------------------------------------------
class FXMLSerialize_ : private RTTI::FMetaAtomWrapCopyVisitor {
public:
    FXMLSerialize_(IStreamWriter* output, const RTTI::FMetaTransaction* transaction)
        : _output(output)
        , _transaction(transaction)
        , _indentLevel(0)
        , _newLine(false) {}

    void ProcessQueue();
    void QueueObject(const RTTI::FMetaObject* object);
    void WriteObject(const RTTI::FMetaObject* object);

private: // FMetaAtomWrapCopyVisitor
    using RTTI::FMetaAtomWrapCopyVisitor::Append;
    using RTTI::FMetaAtomWrapCopyVisitor::Inspect;
    using RTTI::FMetaAtomWrapCopyVisitor::Visit;

    virtual void Inspect(const RTTI::IMetaAtomPair* ppair, const RTTI::TPair<RTTI::PMetaAtom, RTTI::PMetaAtom>& pair) override {
        Puts("<pair>");
        IncIndent();
            Puts("<first class='{0}'>", FXMLEscaped_{ppair->FirstTypeInfo().Name} );
            IncIndent();
                Append(pair.first.get());
            DecIndent();
            Puts("</first>");
            Puts("<second class='{0}'>", FXMLEscaped_{ppair->SecondTypeInfo().Name} );
            IncIndent();
                Append(pair.second.get());
            DecIndent();
            Puts("</second>");
        DecIndent();
        Puts("</pair>");
    }

    virtual void Inspect(const RTTI::IMetaAtomVector* pvector, const RTTI::TVector<RTTI::PMetaAtom>& vector) override {
        if (vector.empty()) {
            Puts("<vector class='{0}' size='0' />", FXMLEscaped_{pvector->ValueTypeInfo().Name} );
        }
        else {
            Puts("<vector class='{0}' size='{1}'>", FXMLEscaped_{pvector->ValueTypeInfo().Name}, vector.size());
            IncIndent();
                for(const RTTI::PMetaAtom& atom : vector)
                    Append(atom.get());
            DecIndent();
            Puts("</vector>");
        }
    }

    virtual void Inspect(const RTTI::IMetaAtomDictionary* pdictionary, const RTTI::TDictionary<RTTI::PMetaAtom, RTTI::PMetaAtom>& dictionary) override {
        if (dictionary.empty()) {
            Puts("<dictionary class='{0};{1}' size='0' />",
                FXMLEscaped_{pdictionary->KeyTypeInfo().Name},
                FXMLEscaped_{pdictionary->ValueTypeInfo().Name} );
        }
        else {
            Puts("<dictionary class='{0};{1}' size='{2}'>",
                FXMLEscaped_{pdictionary->KeyTypeInfo().Name},
                FXMLEscaped_{pdictionary->ValueTypeInfo().Name},
                dictionary.size() );
            IncIndent();
                for(const RTTI::TPair<RTTI::PMetaAtom, RTTI::PMetaAtom>& pair : dictionary) {
                    Puts("<pair>");
                    IncIndent();
                        if (pair.first) {
                            Puts("<first class='{0}'>", FXMLEscaped_{pair.first->TypeInfo().Name} );
                                IncIndent();
                                Append(pair.first.get());
                                DecIndent();
                            Puts("</first>");
                        }
                        else {
                            Puts("<first />");
                        }
                        if (pair.second) {
                            Puts("<second class='{0}'>", FXMLEscaped_{pair.second->TypeInfo().Name} );
                                IncIndent();
                                Append(pair.second.get());
                                DecIndent();
                            Puts("</second>");
                        }
                        else {
                            Puts("<second />");
                        }
                    DecIndent();
                    Puts("</pair>");
                }
            DecIndent();
            Puts("</dictionary>");
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
        Puts("<numeric class='{0}' value='{1}' />", FXMLEscaped_{RTTI::TypeInfo<T>().Name}, Numeric_(value) );
    }

    void WriteValue_(const FString& str) {
        Puts("<string value='{0}' />", FXMLEscaped_{str.MakeView()} );
    }

    void WriteValue_(const FWString& wstr) {
        const FString str = ToString(wstr);
        WriteValue_(str);
    }

    void WriteValue_(const RTTI::FName& name) {
        Puts("<name value='{0}' />", FXMLEscaped_{name.MakeView()} );
    }

    void WriteValue_(const RTTI::FBinaryData& rawdata) {
        if (rawdata.size()) {
            Print("<binarydata size='{0}'>", rawdata.size());
            *_output << FXMLEscaped_{rawdata.MakeView().Cast<const char>()};
            Puts("</binarydata>");
        }
        else {
            Puts("<binarydata size='0' />");
        }
    }

    void WriteValue_(const RTTI::FOpaqueData& opaqueData) {
        if (opaqueData.size()) {
            Puts("<opaquedata size='{0}'>", opaqueData.size());
            IncIndent();
                for(const RTTI::TPair<RTTI::FName, RTTI::PMetaAtom>& pair : opaqueData) {
                    Puts("<pair>");
                    IncIndent();
                        Puts("<first class='{0}'>", FXMLEscaped_{RTTI::TypeInfo<RTTI::FName>().Name} );
                        IncIndent();
                            WriteValue_(pair.first);
                        DecIndent();
                        Puts("</first>");
                        if (pair.second) {
                            Puts("<second class='{0}'>", FXMLEscaped_{pair.second->TypeInfo().Name} );
                            IncIndent();
                                Append(pair.second.get());
                            DecIndent();
                            Puts("</second>");
                        }
                        else {
                            Puts("<second />");
                        }
                    DecIndent();
                    Puts("</pair>");
                }
            DecIndent();
            Puts("<opaquedata/>");
        }
        else {
            Puts("<opaquedata size='0' />");
        }
    }

    template <typename T>
    void WriteValue_(const TScalarVector<T, 2>& v) {
        Puts("<scalarvector class='{0}' dim='2' x='{1}' y='{2}' />", FXMLEscaped_{RTTI::TypeInfo<T>().Name}, Numeric_(v.x()), Numeric_(v.y()) );
    }
    template <typename T>
    void WriteValue_(const TScalarVector<T, 3>& v) {
        Puts("<scalarvector class='{0}' dim='3' x='{1}' y='{2}' z='{3}' />", FXMLEscaped_{RTTI::TypeInfo<T>().Name}, Numeric_(v.x()), Numeric_(v.y()), Numeric_(v.z()) );
    }

    template <typename T>
    void WriteValue_(const TScalarVector<T, 4>& v) {
        Puts("<scalarvector class='{0}' dim='4' x='{1}' y='{2}' z='{3}' w='{4}' />", FXMLEscaped_{RTTI::TypeInfo<T>().Name}, Numeric_(v.x()), Numeric_(v.y()), Numeric_(v.z()), Numeric_(v.w()) );
    }

    template <typename T, size_t _Width, size_t _Height>
    void WriteValue_(const TScalarMatrix<T, _Width, _Height>& v) {
        Puts("<scalarmatrix class='{0}' width='{1}' height='{2}'>", FXMLEscaped_{RTTI::TypeInfo<T>().Name}, _Width, _Height);
        IncIndent();
            forrange(row, 0, _Height) {
                WriteValue_(v.Row(row));
            }
        DecIndent();
        Puts("</scalarmatrix>");
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
                CORE_THROW_IT(FXMLSerializerException("no support for abstract RTTI atoms serialization"));
            }
        }
        else {
            Puts("</atom>");
        }
    }

    void WriteValue_(const RTTI::PMetaObject& object) {
        if (object) {
            if (_transaction->Contains(object.get())) {
                if (object->RTTI_IsExported()) {
                    QueueObject(object.get());
                    Puts("<object name='{0}' />", FXMLEscaped_{object->RTTI_Name().MakeView()} );
                }
                else {
                    WriteObject(object.get());
                }
            }
            else {
                const RTTI::FMetaClass* metaClass = object->RTTI_MetaClass();
                Puts("<externalreference class='{0}' name='{1}' />",
                    FXMLEscaped_{metaClass->Name().MakeView()},
                    FXMLEscaped_{object->RTTI_Name().MakeView()} );
            }
        }
        else {
            Puts("<object />");
        }
    }

private:
    void IncIndent() { ++_indentLevel; }
    void DecIndent() { Assert(0 < _indentLevel); --_indentLevel; }
    void IndentIFN() {
        if (_newLine) {
            _newLine = false;
            forrange(i, 0, _indentLevel)
                _output->WriteView("    ");
        }
    }

    void Print(char ch) {
        IndentIFN();

        _output->WritePOD(ch);
    }

    void Print(const FStringView& str) {
        IndentIFN();

        _output->WriteView(str);
    }

    void Puts(const FStringView& str) {
        IndentIFN();

        _output->WriteView(str);
        _output->WriteView("\r\n");
        _newLine = true;
    }

    template <typename _Arg0, typename... _Args>
    void Print(const FStringView& fmt, _Arg0&& arg0, _Args&&... args) {
        char buffer[1024];
        const size_t n = Format(buffer, fmt, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
        Print(FStringView(buffer, n));
    }

    template <typename _Arg0, typename... _Args>
    void Puts(const FStringView& fmt, _Arg0&& arg0, _Args&&... args) {
        char buffer[1024];
        const size_t n = Format(buffer, fmt, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
        Puts(FStringView(buffer, n));
    }

private:
    IStreamWriter* _output;
    const RTTI::FMetaTransaction* _transaction;

    size_t _indentLevel;
    bool _newLine;

    HASHSET_THREAD_LOCAL(Serialize, RTTI::PCMetaObject) _exported;
    VECTORINSITU_THREAD_LOCAL(Serialize, RTTI::PCMetaObject, 8) _queue;
};

//----------------------------------------------------------------------------
void FXMLSerialize_::QueueObject(const RTTI::FMetaObject* object) {
    AssertRelease(object);

    RTTI::PCMetaObject refObj(object);
    if (not Insert_ReturnIfExists(_exported, refObj))
        _queue.emplace_back(object);
}
//----------------------------------------------------------------------------
void FXMLSerialize_::WriteObject(const RTTI::FMetaObject* object) {
    AssertRelease(object);

    const RTTI::FMetaClass* metaClass = object->RTTI_MetaClass();
    Assert(metaClass);

    if (object->RTTI_IsExported())
        Puts("<object class='{0}' name='{1}'>",  FXMLEscaped_{metaClass->Name().MakeView()}, FXMLEscaped_{object->RTTI_Name().MakeView()} );
    else
        Puts("<object class='{0}'>",  metaClass->Name().MakeView());

    IncIndent();

    while (metaClass) {
        for (const RTTI::UCMetaProperty& prop : metaClass->Properties()) {
            if (prop->IsDefaultValue(object) )
                continue;

            Puts("<property name='{0}' class='{1}'>", FXMLEscaped_{prop->Name().MakeView()}, FXMLEscaped_{prop->TypeInfo().Name} );
            IncIndent();

                const RTTI::PMetaAtom atom = prop->WrapCopy(object);
                Append(atom.get());

            DecIndent();
            Puts("</property>");
        }

        metaClass = metaClass->Parent();
    }

    DecIndent();
    Puts("</object>");
}
//----------------------------------------------------------------------------
void FXMLSerialize_::ProcessQueue() {
    while (_queue.size()) {
        RTTI::PCMetaObject object = std::move(_queue.back());
        _queue.pop_back();

        WriteObject(object.get());
    }
}
//---------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FXMLSerializer::FXMLSerializer() {}
//----------------------------------------------------------------------------
FXMLSerializer::~FXMLSerializer() {}
//----------------------------------------------------------------------------
void FXMLSerializer::Deserialize(RTTI::FMetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName/* = nullptr */) {
    Assert(transaction);
    Assert(input);

    AssertNotImplemented(); // TODO
}
//----------------------------------------------------------------------------
void FXMLSerializer::Serialize(IStreamWriter* output, const RTTI::FMetaTransaction* transaction) {
    Assert(output);
    Assert(transaction);

    output->WriteView("<?xml version=\"1.0\"?>");

    FXMLSerialize_ serialize(output, transaction);

    for (const RTTI::PMetaObject& object : transaction->MakeView())
        serialize.QueueObject(object.get());

    serialize.ProcessQueue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core

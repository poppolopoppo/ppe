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

    inline friend void operator <<(
        IStreamWriter& output,
        const FXMLEscaped_& escaped ) {
        escaped.Serialize_([&output](char  ch) { output.WritePOD(ch); });
    }

    inline friend std::basic_ostream<char>& operator <<(
        std::basic_ostream<char>& oss,
        const FXMLEscaped_& escaped ) {
        escaped.Serialize_([&oss](char  ch) { oss.put(ch); });
        return oss;
    }

private:
    template <typename _OStream>
    void Serialize_(const _OStream& oss) const {
        for (char ch : Str) {
            if (IsAlnum(ch) || '-' == ch || '_' == ch || '.' == ch || '~' == ch) {
                oss(ch);
            }
            else if (IsSpace(ch)) {
                oss('+');
            }
            else {
                const size_t x0 = ((unsigned char)ch) >> 4;
                const size_t x1 = ((unsigned char)ch) & 15;

                oss('%');
                oss(char(x0 + (x0 > 9 ? 'A' - 10 : '0')));
                oss(char(x1 + (x1 > 9 ? 'A' - 10 : '0')));
            }
        }
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
        UNUSED(ppair);
        Puts("<Pair>");
        IncIndent();
            Puts("<First>");
            IncIndent();
                Append(pair.first.get());
            DecIndent();
            Puts("</First>");
            Puts("<Second>");
            IncIndent();
                Append(pair.second.get());
            DecIndent();
            Puts("</Second>");
        DecIndent();
        Puts("</Pair>");
    }

    virtual void Inspect(const RTTI::IMetaAtomVector* pvector, const RTTI::TVector<RTTI::PMetaAtom>& vector) override {
        if (vector.empty()) {
            Puts("<Array size='0' />");
        }
        else {
            Puts("<Array size='{0}'>", vector.size());
            IncIndent();
                for(const RTTI::PMetaAtom& atom : vector)
                    Append(atom.get());
            DecIndent();
            Puts("</Array>");
        }
    }

    virtual void Inspect(const RTTI::IMetaAtomDictionary* pdictionary, const RTTI::TDictionary<RTTI::PMetaAtom, RTTI::PMetaAtom>& dictionary) override {
        if (dictionary.empty()) {
            Puts("<Dictionary size='0' />");
        }
        else {
            Puts("<Dictionary size='{0}'>", dictionary.size() );
            IncIndent();
                for(const RTTI::TPair<RTTI::PMetaAtom, RTTI::PMetaAtom>& pair : dictionary) {
                    Puts("<Entry>");
                    IncIndent();
                        if (pair.first) {
                            Puts("<Key>");
                            IncIndent();
                                Append(pair.first.get());
                            DecIndent();
                            Puts("</Key>");
                        }
                        else {
                            Puts("<Key />");
                        }
                        if (pair.second) {
                            Puts("<Value>");
                            IncIndent();
                                Append(pair.second.get());
                            DecIndent();
                            Puts("</Value>");
                        }
                        else {
                            Puts("<Value />");
                        }
                    DecIndent();
                    Puts("</Entry>");
                }
            DecIndent();
            Puts("</Dictionary>");
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
    template <typename T, typename _Enabled = void >
    struct TNumericTraits_ {};
    template <typename T>
    struct TNumericTraits_< T, typename std::enable_if< std::is_integral<T>::value && std::is_signed<T>::value >::type > { typedef i64 type; };
    template <typename T>
    struct TNumericTraits_< T, typename std::enable_if< std::is_integral<T>::value && not std::is_signed<T>::value >::type > { typedef u64 type; };
    template <typename T>
    struct TNumericTraits_< T, typename std::enable_if< std::is_floating_point<T>::value >::type > { typedef double type; };
    template <>
    struct TNumericTraits_<bool, void> { typedef bool type; };

    template <typename T>
    void WriteValue_(const T& value) {
        typedef typename TNumericTraits_<T>::type numeric_type;
        Puts("<Numeric class='{0}'>{1}</Numeric>", FXMLEscaped_{ RTTI::TypeInfo<T>().Name }, numeric_type(value) );
    }

    void WriteValue_(const bool& value) {
        Puts("<Bool>{0:A}</Bool>", value);
    }

    void WriteValue_(const FString& str) {
        Print("<String>");
        *_output << FXMLEscaped_{str.MakeView()};
        Puts("</String>");
    }

    void WriteValue_(const FWString& wstr) {
        const FString str = ToString(wstr);
        WriteValue_(str);
    }

    void WriteValue_(const RTTI::FName& name) {
        Print("<Name>");
        *_output << FXMLEscaped_{name.MakeView()};
        Puts("</Name>");
    }

    void WriteValue_(const RTTI::FBinaryData& rawdata) {
        if (rawdata.size()) {
            Print("<BinaryData size='{0}'>", rawdata.size());
            *_output << FXMLEscaped_{rawdata.MakeView().Cast<const char>()};
            Puts("</BinaryData>");
        }
        else {
            Puts("<BinaryData size='0' />");
        }
    }

    void WriteValue_(const RTTI::FOpaqueData& opaqueData) {
        if (opaqueData.size()) {
            Puts("<OpaqueData size='{0}'>", opaqueData.size());
            IncIndent();
                for(const RTTI::TPair<RTTI::FName, RTTI::PMetaAtom>& pair : opaqueData) {
                    Puts("<Entry>");
                    IncIndent();
                        WriteValue_(pair.first);
                        if (pair.second) {
                            Puts("<Value>");
                            IncIndent();
                                Append(pair.second.get());
                            DecIndent();
                            Puts("</Value>");
                        }
                        else {
                            Puts("<Value />");
                        }
                    DecIndent();
                    Puts("</Entry>");
                }
            DecIndent();
            Puts("</OpaqueData>");
        }
        else {
            Puts("<OpaqueData size='0' />");
        }
    }

    template <typename T, size_t _Dim>
    void WriteValue_(const TScalarVector<T, _Dim>& v) {
        typedef typename TNumericTraits_<T>::type numeric_type;
        Print("<Vector class='{0}' dim='{1}'>", FXMLEscaped_{ RTTI::TypeInfo<T>().Name }, _Dim);
        Print("{0}", numeric_type(v[0]) );
        for (T num : v.MakeView().ShiftFront()) {
            Print(";{0}", numeric_type(num));
        }
        Puts("</Vector>");
    }

    template <typename T, size_t _Width, size_t _Height>
    void WriteValue_(const TScalarMatrix<T, _Width, _Height>& m) {
        typedef typename TNumericTraits_<T>::type numeric_type;
        Print("<Matrix class='{0}' width='{1}' height='{2}'>", FXMLEscaped_{ RTTI::TypeInfo<T>().Name }, _Width, _Height);
        Print("{0}", static_cast<numeric_type>(m.at(0,0)) );
        for (T num : m.MakeView().ShiftFront()) {
            Print(";{0}", numeric_type(num));
        }
        Puts("</Matrix>");
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
    }

    void WriteValue_(const RTTI::PMetaObject& object) {
        if (object) {
            if (_transaction->Contains(object.get())) {
                if (object->RTTI_IsExported()) {
                    QueueObject(object.get());
                    Puts("<Object name='{0}' />", FXMLEscaped_{object->RTTI_Name().MakeView()} );
                }
                else {
                    WriteObject(object.get());
                }
            }
            else {
                const RTTI::FMetaClass* metaClass = object->RTTI_MetaClass();
                Puts("<ExternalReference class='{0}' name='{1}' />",
                    FXMLEscaped_{metaClass->Name().MakeView()},
                    FXMLEscaped_{object->RTTI_Name().MakeView()} );
            }
        }
        else {
            Puts("<Object />");
        }
    }

private:
    void IncIndent() { ++_indentLevel; }
    void DecIndent() { Assert(0 < _indentLevel); --_indentLevel; }
    void IndentIFN() {
        if (_newLine) {
            _newLine = false;
            forrange(i, 0, _indentLevel)
                _output->WriteView("  ");
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

    template <typename... _Args>
    void Print(const FStringView& fmt, _Args&&... args) {
        char buffer[1024];
        const size_t n = Format(buffer, fmt, std::forward<_Args>(args)...);
        Print(FStringView(buffer, n));
    }

    template <typename... _Args>
    void Puts(const FStringView& fmt, _Args&&... args) {
        char buffer[1024];
        const size_t n = Format(buffer, fmt, std::forward<_Args>(args)...);
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
        Puts("<Object class='{0}' name='{1}'>",  FXMLEscaped_{metaClass->Name().MakeView()}, FXMLEscaped_{object->RTTI_Name().MakeView()} );
    else
        Puts("<Object class='{0}'>",  metaClass->Name().MakeView());

    IncIndent();

    while (metaClass) {
        for (const RTTI::UCMetaProperty& prop : metaClass->Properties()) {
            if (prop->IsDefaultValue(object) )
                continue;

            Puts("<Property name='{0}'>", FXMLEscaped_{prop->Name().MakeView()} );
            IncIndent();

                const RTTI::PMetaAtom atom = prop->WrapCopy(object);
                Append(atom.get());

            DecIndent();
            Puts("</Property>");
        }

        metaClass = metaClass->Parent();
    }

    DecIndent();
    Puts("</Object>");
}
//----------------------------------------------------------------------------
void FXMLSerialize_::ProcessQueue() {
    Puts("<Transaction>");
    IncIndent();

    while (_queue.size()) {
        RTTI::PCMetaObject object = std::move(_queue.back());
        _queue.pop_back();

        WriteObject(object.get());
    }

    DecIndent();
    Puts("</Transaction>");
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

    output->WriteView("<?xml version=\"1.0\"?>\r\n");

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

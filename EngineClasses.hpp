#pragma once

#include <set>
#include <string>

struct FPointer
{
	uintptr_t Dummy;
};

struct FQWord
{
	int A;
	int B;
};

struct FName
{
	int32_t ComparisonIndex;
	int32_t Number;
};

template<class T>
struct TArray
{
	TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	size_t Num() const
	{
		return Count;
	};

	T& operator[](size_t i)
	{
		return Data[i];
	};

	const T& operator[](size_t i) const
	{
		return Data[i];
	};

	bool IsValidIndex(size_t i) const
	{
		return i < Num();
	}

	T* Data;
	int32_t Count;
	int32_t Max;
};

template<typename KeyType, typename ValueType>
class TPair
{
public:
	KeyType   Key;
	ValueType Value;
};

struct FString : public TArray<wchar_t>
{
	std::string ToString() const
	{
		std::wstring wstr(Data);
		return std::string(wstr.begin(), wstr.end());
	}
};

class FScriptInterface
{
private:
	UObject* ObjectPointer;
	void* InterfacePointer;

public:
	UObject* GetObject() const
	{
		return ObjectPointer;
	}

	UObject*& GetObjectRef()
	{
		return ObjectPointer;
	}

	void* GetInterface() const
	{
		return ObjectPointer != nullptr ? InterfacePointer : nullptr;
	}
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface
{
public:
	InterfaceType* operator->() const
	{
		return (InterfaceType*)GetInterface();
	}

	InterfaceType& operator*() const
	{
		return *((InterfaceType*)GetInterface());
	}

	operator bool() const
	{
		return GetInterface() != nullptr;
	}
};

struct FTextData
{
	char __pad00[0x28];
	wchar_t* Data;
	int32_t Length;
};

struct FText
{
	FTextData* Data;
	unsigned char UnknownData[0x10];

	wchar_t* Get()
	{
		if (!Data) return nullptr;

		return Data->Data;
	}
};

struct FWeakObjectPtr
{
	int32_t ObjectIndex;
	int32_t ObjectSerialNumber;
};

struct FStringAssetReference
{
	FString AssetLongPathname;
};

template<typename TObjectID>
class TPersistentObjectPtr
{
public:
	FWeakObjectPtr WeakPtr;
	int32_t TagAtLastTest;
	TObjectID ObjectID;
};

class FAssetPtr : public TPersistentObjectPtr<FStringAssetReference>
{

};

struct FGuid
{
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t D;
};

struct FUniqueObjectGuid
{
	FGuid Guid;
};

class FLazyObjectPtr : public TPersistentObjectPtr<FUniqueObjectGuid>
{

};

struct FScriptDelegate
{
	unsigned char UnknownData[20];
};

struct FScriptMulticastDelegate
{
	unsigned char UnknownData[16];
};

class UClass;

class UObject
{
public:
	FPointer VTableObject;
	int32_t ObjectFlags;
	int32_t InternalIndex;
	UClass* Class;
	FName Name;
	UObject* Outer;
};

class UField : public UObject
{
public:
	UField* Next;
};

class UEnum : public UField
{
public:
	FString CppType; //0x0030 
	TArray<TPair<FName, uint64_t>> Names; //0x0040 
	__int64 CppForm; //0x0050 
};

class UStruct : public UField
{
public:
	UStruct* SuperField;
	UField* Children;
	int32_t PropertySize;
	int32_t MinAlignment;
	char pad_0x0048[0x40];
};

class UScriptStruct : public UStruct
{
public:
	char pad_0x0088[0x10]; //0x0088
};

class UFunction : public UStruct
{
public:
	__int32 FunctionFlags; //0x0088
	__int16 RepOffset; //0x008C
	__int8 NumParms; //0x008E
	char pad_0x008F[0x1]; //0x008F
	__int16 ParmsSize; //0x0090
	__int16 ReturnValueOffset; //0x0092
	__int16 RPCId; //0x0094
	__int16 RPCResponseId; //0x0096
	class UProperty* FirstPropertyToInit; //0x0098
	UFunction* EventGraphFunction; //0x00A0
	__int32 EventGraphCallOffset; //0x00A8
	char pad_0x00AC[0x4]; //0x00AC
	void* Func; //0x00B0
};

class UClass : public UStruct
{
public:
	char pad_0x0088[0x198]; //0x0088
};

class UProperty : public UField
{
public:
	__int32 ArrayDim; //0x0030 
	__int32 ElementSize; //0x0034 
	FQWord PropertyFlags; //0x0038
	__int32 PropertySize; //0x0040 
	char pad_0x0044[0xC]; //0x0044
	__int32 Offset; //0x0050 
	char pad_0x0054[0x24]; //0x0054
};

class UNumericProperty : public UProperty
{
public:
	
};

class UByteProperty : public UNumericProperty
{
public:
	UEnum*		Enum;										// 0x0088 (0x04)
};

class UUInt16Property : public UNumericProperty
{
public:

};

class UUInt32Property : public UNumericProperty
{
public:

};

class UUInt64Property : public UNumericProperty
{
public:

};

class UInt8Property : public UNumericProperty
{
public:

};

class UInt16Property : public UNumericProperty
{
public:

};

class UIntProperty : public UNumericProperty
{
public:

};

class UInt64Property : public UNumericProperty
{
public:

};

class UFloatProperty : public UNumericProperty
{
public:

};

class UDoubleProperty : public UNumericProperty
{
public:

};

class UBoolProperty : public UProperty
{
public:
	uint8_t FieldSize;
	uint8_t ByteOffset;
	uint8_t ByteMask;
	uint8_t FieldMask;
};

class UObjectPropertyBase : public UProperty
{
public:
	UClass* PropertyClass;
};

class UObjectProperty : public UObjectPropertyBase
{
public:

};

class UClassProperty : public UObjectProperty
{
public:
	UClass* MetaClass;
};

class UInterfaceProperty : public UProperty
{
public:
	UClass* InterfaceClass;
};

class UWeakObjectProperty : public UObjectPropertyBase
{
public:

};

class ULazyObjectProperty : public UObjectPropertyBase
{
public:

};

class UAssetObjectProperty : public UObjectPropertyBase
{
public:

};

class UAssetClassProperty : public UAssetObjectProperty
{
public:
	UClass* MetaClass;
};

class UNameProperty : public UProperty
{
public:

};

class UStructProperty : public UProperty
{
public:
	UScriptStruct* Struct;
};

class UStrProperty : public UProperty
{
public:

};

class UTextProperty : public UProperty
{
public:

};

class UArrayProperty : public UProperty
{
public:
	UProperty* Inner;
};

class UMapProperty : public UProperty
{
public:
	UProperty* KeyProp;
	UProperty* ValueProp;
};

class UDelegateProperty : public UProperty
{
public:
	UFunction* SignatureFunction;
};

class UMulticastDelegateProperty : public UProperty
{
public:
	UFunction* SignatureFunction;
};

class UEnumProperty : public UProperty
{
public:
	class UNumericProperty* UnderlyingProp; //0x0070
	class UEnum* Enum; //0x0078
}; //Size: 0x0080

/*
#pragma once

#include <set>

template<typename Fn>
inline Fn GetVFunction(const void *instance, std::size_t index) {
auto vtable = *reinterpret_cast<const void***>(const_cast<void*>(instance));
return reinterpret_cast<Fn>(vtable[index]);
}

// Credits to namazso <3
static unsigned fnv_hash_runtime(const char* str) {
static constexpr auto k_fnv_prime = 16777619u;
static constexpr auto k_offset_basis = 2166136261u;

auto hash = k_offset_basis;
do {
hash ^= *str++;
hash *= k_fnv_prime;
} while (*(str - 1) != 0);

return hash;
}

class UObject;

class FUObjectItem {
public:
UObject* Object;
int32_t Flags;
int32_t SerialNumber;
void* unk;

enum class EInternalObjectFlags : int32_t {
None = 0,
Native = 1 << 25,
Async = 1 << 26,
AsyncLoading = 1 << 27,
Unreachable = 1 << 28,
PendingKill = 1 << 29,
RootSet = 1 << 30,
NoStrongReference = 1 << 31
};

inline bool IsUnreachable() const {
return !!(Flags & static_cast<std::underlying_type_t<EInternalObjectFlags>>(EInternalObjectFlags::Unreachable));
}
inline bool IsPendingKill() const {
return !!(Flags & static_cast<std::underlying_type_t<EInternalObjectFlags>>(EInternalObjectFlags::PendingKill));
}
};

class TUObjectArray {
public:
inline int32_t Num() const {
return NumElements;
}

inline UObject* GetByIndex(int32_t index) const {
return Objects[index].Object;
}

inline FUObjectItem* GetItemByIndex(int32_t index) const {
if (index < NumElements) {
return &Objects[index];
}
return nullptr;
}

private:
FUObjectItem* Objects;
int32_t MaxElements;
int32_t NumElements;
};

class FUObjectArray {
public:
int32_t ObjFirstGCIndex;
int32_t ObjLastNonGCIndex;
int32_t MaxObjectsNotConsideredByGC;
int32_t OpenForDisregardForGC;
TUObjectArray ObjObjects;
};

template<class T>
struct TArray {
friend struct FString;

public:
inline TArray() {
Data = nullptr;
Count = Max = 0;
};

inline int Num() const {
return Count;
};

inline T& operator[](int i) {
return Data[i];
};

inline const T& operator[](int i) const {
return Data[i];
};

inline bool IsValidIndex(int i) const {
return i < Num();
}

private:
T* Data;
int32_t Count;
int32_t Max;
};

class FNameEntry {
public:
static const auto NAME_WIDE_MASK = 0x1;
static const auto NAME_INDEX_SHIFT = 1;

int32_t Index;
char UnknownData00[0x04];
FNameEntry* HashNext;
union {
char AnsiName[1024];
wchar_t WideName[1024];
};

inline const int32_t GetIndex() const {
return Index >> NAME_INDEX_SHIFT;
}

inline bool IsWide() const {
return Index & NAME_WIDE_MASK;
}

inline const char* GetAnsiName() const {
return AnsiName;
}

inline const wchar_t* GetWideName() const {
return WideName;
}
};

template<typename ElementType, int32_t MaxTotalElements, int32_t ElementsPerChunk>
class TStaticIndirectArrayThreadSafeRead {
public:
inline size_t Num() const {
return NumElements;
}

inline bool IsValidIndex(int32_t index) const {
return index < Num() && index >= 0;
}

inline ElementType const* const& operator[](int32_t index) const {
return *GetItemPtr(index);
}

private:
inline ElementType const* const* GetItemPtr(int32_t Index) const {
int32_t ChunkIndex = Index / ElementsPerChunk;
int32_t WithinChunkIndex = Index % ElementsPerChunk;
ElementType** Chunk = Chunks[ChunkIndex];
return Chunk + WithinChunkIndex;
}

enum {
ChunkTableSize = (MaxTotalElements + ElementsPerChunk - 1) / ElementsPerChunk
};

ElementType** Chunks[ChunkTableSize];
int32_t NumElements;
int32_t NumChunks;
};

using TNameEntryArray = TStaticIndirectArrayThreadSafeRead<FNameEntry, 2 * 1024 * 1024, 16384>;

struct FName {
union {
struct {
int32_t ComparisonIndex;
int32_t Number;
};

// DO NOT REMOVE: needed for memory alignment! biggest member is now uint64_t
uint64_t CompositeComparisonValue;
};

inline FName()
: ComparisonIndex(0),
Number(0) {
};

inline FName(int32_t i)
: ComparisonIndex(i),
Number(0) {
};

FName(const char* nameToFind)
: ComparisonIndex(0),
Number(0) {
static std::set<int> cache;

for (auto i : cache) {
if (!std::strcmp(GetGlobalNames()[i]->GetAnsiName(), nameToFind)) {
ComparisonIndex = i;

return;
}
}

for (auto i = 0; i < GetGlobalNames().Num(); ++i) {
if (GetGlobalNames()[i] != nullptr) {
if (!std::strcmp(GetGlobalNames()[i]->GetAnsiName(), nameToFind)) {
cache.insert(i);

ComparisonIndex = i;

return;
}
}
}
};

static TNameEntryArray *GNames;
static inline TNameEntryArray& GetGlobalNames() {
return *GNames;
};

inline const char* GetName() const {
return GetGlobalNames()[ComparisonIndex]->GetAnsiName();
};

inline bool operator==(const FName &other) const {
return ComparisonIndex == other.ComparisonIndex;
};
};

struct FString : private TArray<wchar_t> {
inline FString() {
};

FString(const wchar_t* other) {
Max = Count = *other ? (int32_t)std::wcslen(other) + 1 : 0;

if (Count) {
Data = const_cast<wchar_t*>(other);
}
};

inline bool IsValid() const {
return Data != nullptr;
}

inline const wchar_t* c_str() const {
return Data;
}

std::string ToString() const {
auto length = std::wcslen(Data);

std::string str(length, '\0');

std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

return str;
}
};

template<class TEnum>
class TEnumAsByte {
public:
inline TEnumAsByte() {
}

inline TEnumAsByte(TEnum _value)
: value(static_cast<uint8_t>(_value)) {
}

explicit inline TEnumAsByte(int32_t _value)
: value(static_cast<uint8_t>(_value)) {
}

explicit inline TEnumAsByte(uint8_t _value)
: value(_value) {
}

inline operator TEnum() const {
return (TEnum)value;
}

inline TEnum GetValue() const {
return (TEnum)value;
}

private:
uint8_t value;
};

class FScriptInterface {
private:
UObject* ObjectPointer;
void* InterfacePointer;

public:
inline UObject* GetObject() const {
return ObjectPointer;
}

inline UObject*& GetObjectRef() {
return ObjectPointer;
}

inline void* GetInterface() const {
return ObjectPointer != nullptr ? InterfacePointer : nullptr;
}
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface {
public:
inline InterfaceType* operator->() const {
return (InterfaceType*)GetInterface();
}

inline InterfaceType& operator*() const {
return *((InterfaceType*)GetInterface());
}

inline operator bool() const {
return GetInterface() != nullptr;
}
};

class FTextData {
public:
char pad_0x0000[0x28];  //0x0000
wchar_t* Name;          //0x0028
__int32 Length;         //0x0030

};

struct FText {
FTextData* Data;
char UnknownData[0x10];

wchar_t* Get() const {
if (Data) {
return Data->Name;
}

return nullptr;
}
};

struct FScriptDelegate {
char UnknownData[16];
};

struct FScriptMulticastDelegate {
char UnknownData[16];
};

template<typename Key, typename Value>
class TMap {
char UnknownData[0x50];
};

struct FWeakObjectPtr {
public:
inline bool SerialNumbersMatch(FUObjectItem* ObjectItem) const {
return ObjectItem->SerialNumber == ObjectSerialNumber;
}

bool IsValid() const;

UObject* Get() const;

int32_t ObjectIndex;
int32_t ObjectSerialNumber;
};

template<class T, class TWeakObjectPtrBase = FWeakObjectPtr>
struct TWeakObjectPtr : private TWeakObjectPtrBase {
public:
inline T* Get() const {
return (T*)TWeakObjectPtrBase::Get();
}

inline T& operator*() const {
return *Get();
}

inline T* operator->() const {
return Get();
}

inline bool IsValid() const {
return TWeakObjectPtrBase::IsValid();
}
};

template<class T, class TBASE>
class TAutoPointer : public TBASE {
public:
inline operator T*() const {
return TBASE::Get();
}

inline operator const T*() const {
return (const T*)TBASE::Get();
}

explicit inline operator bool() const {
return TBASE::Get() != nullptr;
}
};

template<class T>
class TAutoWeakObjectPtr : public TAutoPointer<T, TWeakObjectPtr<T>> {
public:
};

template<typename TObjectID>
class TPersistentObjectPtr {
public:
FWeakObjectPtr WeakPtr;
int64_t TagAtLastTest;
TObjectID ObjectID;
};

struct FStringAssetReference_ {
char UnknownData[0x10];
};

class FAssetPtr : public TPersistentObjectPtr<FStringAssetReference_> {

};

template<typename ObjectType>
class TAssetPtr : FAssetPtr {

};

struct FUniqueObjectGuid_ {
char UnknownData[0x10];
};

class FLazyObjectPtr : public TPersistentObjectPtr<FUniqueObjectGuid_> {

};

template<typename ObjectType>
class TLazyObjectPtr : FLazyObjectPtr {

};

*/
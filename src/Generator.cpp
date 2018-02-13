#include "IGenerator.hpp"
#include "ObjectsStore.hpp"
#include "NamesStore.hpp"

class Generator : public IGenerator
{
public:
	bool Initialize(void* module) override
	{
		alignasClasses = {
			{ "ScriptStruct CoreUObject.Plane", 16 },
			{ "ScriptStruct CoreUObject.Quat", 16 },
			{ "ScriptStruct CoreUObject.Transform", 16 },
			{ "ScriptStruct CoreUObject.Vector4", 16 },

			{ "ScriptStruct Engine.RootMotionSourceGroup", 8 }
		};

		virtualFunctionPattern["Class CoreUObject.Object"] = {
			{ "\x45\x33\xF6\x3B\x05\x00\x00\x00\x00\x4D\x8B\xE0", "xxxxx????xxx", 0x200, R"(	inline void ProcessEvent(class UFunction* function, void* parms)
	{
		return GetVFunction<void(*)(UObject*, class UFunction*, void*)>(this, %d)(this, function, parms);
	})" }
		};
		virtualFunctionPattern["Class CoreUObject.Class"] = {
			{ "\x4C\x8B\xDC\x57\x48\x81\xEC", "xxxxxxx", 0x200, R"(	inline UObject* CreateDefaultObject()
	{
		return GetVFunction<UObject*(*)(UClass*)>(this, %d)(this);
	})" }
		};

		predefinedMembers["Class CoreUObject.Object"] = {
			{ "void*", "Vtable" },
			{ "int32_t", "ObjectFlags" },
			{ "int32_t", "InternalIndex" },
			{ "class UClass*", "Class" },
			{ "FName", "Name" },
			{ "class UObject*", "Outer" }
		};
		predefinedStaticMembers["Class CoreUObject.Object"] = {
			{ "FUObjectArray*", "GObjects" }
		};
		predefinedMembers["Class CoreUObject.Field"] = {
			{ "class UField*", "Next" }
		};
		predefinedMembers["Class CoreUObject.Struct"] = {
			{ "class UStruct*", "SuperField" },
			{ "class UField*", "Children" },
			{ "int32_t", "PropertySize" },
			{ "int32_t", "MinAlignment" },
			{ "unsigned char", "UnknownData0x0048[0x40]" }
		};
		predefinedMembers["Class CoreUObject.Function"] = {
			{ "int32_t", "FunctionFlags" },
			{ "int16_t", "RepOffset" },
			{ "int8_t", "NumParms" },
			{ "int16_t", "ParmsSize" },
			{ "int16_t", "ReturnValueOffset" },
			{ "int16_t", "RPCId" },
			{ "int16_t", "RPCResponseId" },
			{ "class UProperty*", "FirstPropertyToInit" },
			{ "class UFunction*", "EventGraphFunction" },
			{ "int32_t", "EventGraphCallOffset" },
			{ "void*", "Func" }
		};

		predefinedMethods["ScriptStruct CoreUObject.Vector2D"] = {
			PredefinedMethod::Inline(R"(	inline FVector2D()
		: X(0), Y(0)
	{ })"),
			PredefinedMethod::Inline(R"(	inline FVector2D(float x, float y)
		: X(x),
		  Y(y)
	{ })")
		};
		predefinedMethods["ScriptStruct CoreUObject.LinearColor"] = {
			PredefinedMethod::Inline(R"(	inline FLinearColor()
		: R(0), G(0), B(0), A(0)
	{ })"),
			PredefinedMethod::Inline(R"(	inline FLinearColor(float r, float g, float b, float a)
		: R(r),
		  G(g),
		  B(b),
		  A(a)
	{ })")
		};

		predefinedMethods["Class CoreUObject.Object"] = {
			PredefinedMethod::Inline(R"(	static inline TUObjectArray& GetGlobalObjects()
	{
		return GObjects->ObjObjects;
	})"),
			PredefinedMethod::Default("std::string GetName() const", R"(std::string UObject::GetName() const
{
	std::string name(Name.GetName());
	if (Name.Number > 0)
	{
		name += '_' + std::to_string(Name.Number);
	}

	auto pos = name.rfind('/');
	if (pos == std::string::npos)
	{
		return name;
	}
	
	return name.substr(pos + 1);
})"),
			PredefinedMethod::Default("std::string GetFullName() const", R"(std::string UObject::GetFullName() const
{
	std::string name;

	if (Class != nullptr)
	{
		std::string temp;
		for (auto p = Outer; p; p = p->Outer)
		{
			temp = p->GetName() + "." + temp;
		}

		name = Class->GetName();
		name += " ";
		name += temp;
		name += GetName();
	}

	return name;
})"),
			PredefinedMethod::Inline(R"(	template<typename T>
	static T* FindObject(unsigned name)
	{
		for (int i = 0; i < GetGlobalObjects().Num(); ++i)
		{
			auto object = GetGlobalObjects().GetByIndex(i);
	
			if (object == nullptr)
			{
				continue;
			}
	
			if (fnv_hash_runtime(object->GetFullName().c_str()) == name)
			{
				return static_cast<T*>(object);
			}
		}
		return nullptr;
	})"),
			PredefinedMethod::Inline(R"(	static UClass* FindClass(unsigned name)
	{
		return FindObject<UClass>(name);
	})"),
			PredefinedMethod::Inline(R"(	template<typename T>
	static T* GetObjectCasted(int32_t index)
	{
		return static_cast<T*>(GetGlobalObjects().GetByIndex(index));
	})"),
			PredefinedMethod::Default("bool IsA(UClass* cmp) const", R"(bool UObject::IsA(UClass* cmp) const
{
	for (auto super = Class; super; super = static_cast<UClass*>(super->SuperField))
	{
		if (super == cmp)
		{
			return true;
		}
	}

	return false;
})")
		};
		predefinedMethods["Class CoreUObject.Class"] = {
			PredefinedMethod::Inline(R"(	template<typename T>
	inline T* CreateDefaultObject()
	{
		return static_cast<T*>(CreateDefaultObject());
	})")
		};

		predefinedMethods["ScriptStruct CoreUObject.Vector"] = {
			PredefinedMethod::Inline(R"(	inline FVector()
			: X(0), Y(0), Z(0) {
		})"),
			PredefinedMethod::Inline(R"(	inline FVector(float x, float y, float z) : X(x), Y(y),	Z(z) {})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector operator-(const FVector& V) {
		return FVector(X - V.X, Y - V.Y, Z - V.Z);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector operator+(const FVector& V) {
		return FVector(X + V.X, Y + V.Y, Z + V.Z);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector operator*(float Scale) const {
		return FVector(X * Scale, Y * Scale, Z * Scale);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector operator/(float Scale) const {
		const float RScale = 1.f / Scale;
		return FVector(X * RScale, Y * RScale, Z * RScale);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector operator+(float A) const {
		return FVector(X + A, Y + A, Z + A);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector operator-(float A) const {
		return FVector(X - A, Y - A, Z - A);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector operator*(const FVector& V) const {
		return FVector(X * V.X, Y * V.Y, Z * V.Z);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector operator/(const FVector& V) const {
		return FVector(X / V.X, Y / V.Y, Z / V.Z);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float operator|(const FVector& V) const {
		return X*V.X + Y*V.Y + Z*V.Z;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float operator^(const FVector& V) const {
		return X*V.Y - Y*V.X - Z*V.Z;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector& operator+=(const FVector& v) {
		X += v.X;
		Y += v.Y;
		Z += v.Z;
		return *this;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector& operator-=(const FVector& v) {
		X -= v.X;
		Y -= v.Y;
		Z -= v.Z;
		return *this;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector& operator*=(const FVector& v) {
		X *= v.X;
		Y *= v.Y;
		Z *= v.Z;
		return *this;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector& operator/=(const FVector& v) {
		X /= v.X;
		Y /= v.Y;
		Z /= v.Z;
		return *this;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline bool operator==(const FVector& src) const {
		return (src.X == X) && (src.Y == Y) && (src.Z == Z);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline bool operator!=(const FVector& src) const {
		return (src.X != X) || (src.Y != Y) || (src.Z != Z);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float Size() const {
		return sqrt(X*X + Y*Y + Z*Z);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float Size2D() const {
		return sqrt(X*X + Y*Y);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float SizeSquared() const {
		return X*X + Y*Y + Z*Z;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float SizeSquared2D() const {
		return X*X + Y*Y;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float Dot(const FVector& vOther) const {
		const FVector& a = *this;
		return (a.X * vOther.X + a.Y * vOther.Y + a.Z * vOther.Z);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector Normalize() {
		FVector vector;
		float length = this->Size();

		if (length != 0) {
			vector.X = X / length;
			vector.Y = Y / length;
			vector.Z = Z / length;
		} else
			vector.X = vector.Y = 0.0f;
		vector.Z = 1.0f;

		return vector;
	})")
		};

		predefinedMethods["ScriptStruct CoreUObject.Vector2D"] = {
			PredefinedMethod::Inline(R"(	inline FVector2D() : X(0), Y(0) {})"),
			PredefinedMethod::Inline(R"(	inline FVector2D(float x, float y) : X(x), Y(y) {})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector2D operator-(const FVector2D& V) {
		return FVector2D(X - V.X, Y - V.Y);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector2D operator+(const FVector2D& V) {
		return FVector2D(X + V.X, Y + V.Y);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector2D operator*(float Scale) const {
		return FVector2D(X * Scale, Y * Scale);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector2D operator/(float Scale) const {
		const float RScale = 1.f / Scale;
		return FVector2D(X * RScale, Y * RScale);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector2D operator+(float A) const {
		return FVector2D(X + A, Y + A);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector2D operator-(float A) const {
		return FVector2D(X - A, Y - A);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector2D operator*(const FVector2D& V) const {
		return FVector2D(X * V.X, Y * V.Y);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector2D operator/(const FVector2D& V) const {
		return FVector2D(X / V.X, Y / V.Y);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float operator|(const FVector2D& V) const {
		return X*V.X + Y*V.Y;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float operator^(const FVector2D& V) const {
		return X*V.Y - Y*V.X;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector2D& operator+=(const FVector2D& v) {
		X += v.X;
		Y += v.Y;
		return *this;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector2D& operator-=(const FVector2D& v) {
		X -= v.X;
		Y -= v.Y;
		return *this;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector2D& operator*=(const FVector2D& v) {
		X *= v.X;
		Y *= v.Y;
		return *this;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector2D& operator/=(const FVector2D& v) {
		X /= v.X;
		Y /= v.Y;
		return *this;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline bool operator==(const FVector2D& src) const {
		return (src.X == X) && (src.Y == Y);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline bool operator!=(const FVector2D& src) const {
		return (src.X != X) || (src.Y != Y);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float Size() const {
		return sqrt(X*X + Y*Y);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float SizeSquared() const {
		return X*X + Y*Y;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float Dot(const FVector2D& vOther) const {
		const FVector2D& a = *this;
		return (a.X * vOther.X + a.Y * vOther.Y);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector2D Normalize() {
		FVector2D vector;
		float length = this->Size();

		if (length != 0) {
			vector.X = X / length;
			vector.Y = Y / length;
		} else
			vector.X = vector.Y = 0.0f;

		return vector;
	})")
		};

		predefinedMethods["ScriptStruct CoreUObject.Rotator"] = {
			PredefinedMethod::Inline(R"(	inline FRotator() : Pitch(0), Yaw(0), Roll(0) {})"),
			PredefinedMethod::Inline(R"(	inline FRotator(float x, float y, float z) : Pitch(x), Yaw(y), Roll(z) {})"),
			PredefinedMethod::Inline(R"(	__forceinline FRotator operator+(const FRotator& V) {
		return FRotator(Pitch + V.Pitch, Yaw + V.Yaw, Roll + V.Roll);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FRotator operator-(const FRotator& V) {
		return FRotator(Pitch - V.Pitch, Yaw - V.Yaw, Roll - V.Roll);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FRotator operator*(float Scale) const {
		return FRotator(Pitch * Scale, Yaw * Scale, Roll * Scale);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FRotator operator/(float Scale) const {
		const float RScale = 1.f / Scale;
		return FRotator(Pitch * RScale, Yaw * RScale, Roll * RScale);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FRotator operator+(float A) const {
		return FRotator(Pitch + A, Yaw + A, Roll + A);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FRotator operator-(float A) const {
		return FRotator(Pitch - A, Yaw - A, Roll - A);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FRotator operator*(const FRotator& V) const {
		return FRotator(Pitch * V.Pitch, Yaw * V.Yaw, Roll * V.Roll);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FRotator operator/(const FRotator& V) const {
		return FRotator(Pitch / V.Pitch, Yaw / V.Yaw, Roll / V.Roll);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float operator|(const FRotator& V) const {
		return Pitch*V.Pitch + Yaw*V.Yaw + Roll*V.Roll;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FRotator& operator+=(const FRotator& v) {
		Pitch += v.Pitch;
		Yaw += v.Yaw;
		Roll += v.Roll;
		return *this;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FRotator& operator-=(const FRotator& v) {
		Pitch -= v.Pitch;
		Yaw -= v.Yaw;
		Roll -= v.Roll;
		return *this;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FRotator& operator*=(const FRotator& v) {
		Pitch *= v.Pitch;
		Yaw *= v.Yaw;
		Roll *= v.Roll;
		return *this;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FRotator& operator/=(const FRotator& v) {
		Pitch /= v.Pitch;
		Yaw /= v.Yaw;
		Roll /= v.Roll;
		return *this;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float operator^(const FRotator& V) const {
		return Pitch*V.Yaw - Yaw*V.Pitch - Roll*V.Roll;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline bool operator==(const FRotator& src) const {
		return (src.Pitch == Pitch) && (src.Yaw == Yaw) && (src.Roll == Roll);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline bool operator!=(const FRotator& src) const {
		return (src.Pitch != Pitch) || (src.Yaw != Yaw) || (src.Roll != Roll);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float Size() const {
		return sqrt(Pitch*Pitch + Yaw* Yaw + Roll*Roll);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float SizeSquared() const {
		return Pitch*Pitch + Yaw* Yaw + Roll*Roll;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float Dot(const FRotator& vOther) const {
		const FRotator& a = *this;
		return (a.Pitch * vOther.Pitch + a.Yaw * vOther.Yaw + a.Roll * vOther.Roll);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float ClampAxis(float Angle) {
		Angle = fmod(Angle, 360.f);

		if (Angle < 0.f) {
			Angle += 360.f;
		}

		return Angle;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline float NormalizeAxis(float Angle) {
		Angle = ClampAxis(Angle);

		if (Angle > 180.f) {
			Angle -= 360.f;
		}

		return Angle;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline void Normalize() {
		Pitch = NormalizeAxis(Pitch);
		Yaw = NormalizeAxis(Yaw);
		Roll = NormalizeAxis(Roll);
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FRotator GetNormalized() const {
		FRotator Rot = *this;
		Rot.Normalize();
		return Rot;
	})"),
			PredefinedMethod::Inline(R"(	__forceinline FVector ToVector() {
		return FVector(Pitch, Yaw, Roll);
	})")
		};

		return true;
	}

	std::string GetGameName() const override
	{
		return "PlayerUnknown's Battlegrounds";
	}

	std::string GetGameNameShort() const override
	{
		return "PUBG";
	}

	std::string GetGameVersion() const override
	{
		return "3.5.7.7";
	}

	std::string GetNamespaceName() const override
	{
		return "Classes";
	}

	std::vector<std::string> GetIncludes() const override
	{
		return { };
	}

	std::string GetBasicDeclarations() const override
	{
		return R"(template<typename Fn>
inline Fn GetVFunction(const void *instance, std::size_t index)
{
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

class FUObjectItem
{
public:
	UObject* Object;
	int32_t Flags;
	int32_t ClusterIndex;
	int32_t SerialNumber;

	enum class ObjectFlags : int32_t
	{
		None = 0,
		Native = 1 << 25,
		Async = 1 << 26,
		AsyncLoading = 1 << 27,
		Unreachable = 1 << 28,
		PendingKill = 1 << 29,
		RootSet = 1 << 30,
		NoStrongReference = 1 << 31
	};

	inline bool IsUnreachable() const
	{
		return !!(Flags & static_cast<std::underlying_type_t<ObjectFlags>>(ObjectFlags::Unreachable));
	}
	inline bool IsPendingKill() const
	{
		return !!(Flags & static_cast<std::underlying_type_t<ObjectFlags>>(ObjectFlags::PendingKill));
	}
};

class TUObjectArray
{
public:
	inline int32_t Num() const
	{
		return NumElements;
	}

	inline UObject* GetByIndex(int32_t index) const
	{
		return Objects[index].Object;
	}

	inline FUObjectItem* GetItemByIndex(int32_t index) const
	{
		if (index < NumElements)
		{
			return &Objects[index];
		}
		return nullptr;
	}

private:
	FUObjectItem* Objects;
	int32_t MaxElements;
	int32_t NumElements;
};

class FUObjectArray
{
public:
	int32_t ObjFirstGCIndex;
	int32_t ObjLastNonGCIndex;
	int32_t MaxObjectsNotConsideredByGC;
	int32_t OpenForDisregardForGC;
	TUObjectArray ObjObjects;
};

template<class T>
struct TArray
{
	friend struct FString;

public:
	inline TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	inline TArray(T* data, int32_t number)
	{
		Data = data;
		Count = Max = number;
	};

	inline int Num() const
	{
		return Count;
	};

	inline T& operator[](int i)
	{
		return Data[i];
	};

	inline const T& operator[](int i) const
	{
		return Data[i];
	};

	inline bool IsValidIndex(int i) const
	{
		return i < Num();
	}

private:
	T* Data;
	int32_t Count;
	int32_t Max;
};

class FNameEntry
{
public:
	static const auto NAME_WIDE_MASK = 0x1;
	static const auto NAME_INDEX_SHIFT = 1;

	int32_t Index;
	char UnknownData00[0x04];
	FNameEntry* HashNext;
	union
	{
		char AnsiName[1024];
		wchar_t WideName[1024];
	};

	inline const int32_t GetIndex() const
	{
		return Index >> NAME_INDEX_SHIFT;
	}

	inline bool IsWide() const
	{
		return Index & NAME_WIDE_MASK;
	}

	inline const char* GetAnsiName() const
	{
		return AnsiName;
	}

	inline const wchar_t* GetWideName() const
	{
		return WideName;
	}
};

template<typename ElementType, int32_t MaxTotalElements, int32_t ElementsPerChunk>
class TStaticIndirectArrayThreadSafeRead
{
public:
	inline size_t Num() const
	{
		return NumElements;
	}

	inline bool IsValidIndex(int32_t index) const
	{
		return index < Num() && index >= 0;
	}

	inline ElementType const* const& operator[](int32_t index) const
	{
		return *GetItemPtr(index);
	}

private:
	inline ElementType const* const* GetItemPtr(int32_t Index) const
	{
		int32_t ChunkIndex = Index / ElementsPerChunk;
		int32_t WithinChunkIndex = Index % ElementsPerChunk;
		ElementType** Chunk = Chunks[ChunkIndex];
		return Chunk + WithinChunkIndex;
	}

	enum
	{
		ChunkTableSize = (MaxTotalElements + ElementsPerChunk - 1) / ElementsPerChunk
	};

	ElementType** Chunks[ChunkTableSize];
	int32_t NumElements;
	int32_t NumChunks;
};

using TNameEntryArray = TStaticIndirectArrayThreadSafeRead<FNameEntry, 2 * 1024 * 1024, 16384>;

struct FName
{
	union
	{
		struct
		{
			int32_t ComparisonIndex;
			int32_t Number;
		};

		uint64_t CompositeComparisonValue;
	};

	inline FName()
		: ComparisonIndex(0),
		  Number(0)
	{
	};

	inline FName(int32_t i)
		: ComparisonIndex(i),
		  Number(0)
	{
	};

	FName(const char* nameToFind)
		: ComparisonIndex(0),
		  Number(0)
	{
		static std::unordered_set<int> cache;

		for (auto i : cache)
		{
			if (!std::strcmp(GetGlobalNames()[i]->GetAnsiName(), nameToFind))
			{
				ComparisonIndex = i;
				
				return;
			}
		}

		for (auto i = 0; i < GetGlobalNames().Num(); ++i)
		{
			if (GetGlobalNames()[i] != nullptr)
			{
				if (!std::strcmp(GetGlobalNames()[i]->GetAnsiName(), nameToFind))
				{
					cache.insert(i);

					ComparisonIndex = i;

					return;
				}
			}
		}
	};

	static TNameEntryArray *GNames;
	static inline TNameEntryArray& GetGlobalNames()
	{
		return *GNames;
	};

	inline const char* GetName() const
	{
		return GetGlobalNames()[ComparisonIndex]->GetAnsiName();
	};

	inline bool operator==(const FName &other) const
	{
		return ComparisonIndex == other.ComparisonIndex;
	};
};

struct FString : private TArray<wchar_t>
{
	inline FString()
	{
	};

	FString(const wchar_t* other)
	{
		Max = Count = *other ? (int32_t)std::wcslen(other) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	};

	inline bool IsValid() const
	{
		return Data != nullptr;
	}

	inline const wchar_t* c_str() const
	{
		return Data;
	}

	std::string ToString() const
	{
		std::wstring wstr(Data);
		return std::string(wstr.begin(), wstr.end());
	}
};

template<class TEnum>
class TEnumAsByte
{
public:
	inline TEnumAsByte()
	{
	}

	inline TEnumAsByte(TEnum _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit inline TEnumAsByte(int32_t _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit inline TEnumAsByte(uint8_t _value)
		: value(_value)
	{
	}

	inline operator TEnum() const
	{
		return (TEnum)value;
	}

	inline TEnum GetValue() const
	{
		return (TEnum)value;
	}

private:
	uint8_t value;
};

class FScriptInterface
{
private:
	UObject* ObjectPointer;
	void* InterfacePointer;

public:
	inline UObject* GetObject() const
	{
		return ObjectPointer;
	}

	inline UObject*& GetObjectRef()
	{
		return ObjectPointer;
	}

	inline void* GetInterface() const
	{
		return ObjectPointer != nullptr ? InterfacePointer : nullptr;
	}
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface
{
public:
	inline InterfaceType* operator->() const
	{
		return (InterfaceType*)GetInterface();
	}

	inline InterfaceType& operator*() const
	{
		return *((InterfaceType*)GetInterface());
	}

	inline operator bool() const
	{
		return GetInterface() != nullptr;
	}
};

struct FTextData 
{
	unsigned char UnknownData00[0x28];
	wchar_t* Name;
	uint32_t Length;
};

struct FText
{
	FTextData* Data;
	char UnknownData[0x10];

	wchar_t* Get()
	{
		if (!Data) return nullptr;

		return Data->Name;
	}
};

struct FScriptDelegate
{
	char UnknownData[0x10];
};

struct FScriptMulticastDelegate
{
	char UnknownData[0x10];
};

template<typename Key, typename Value>
class TMap
{
	char UnknownData[0x50];
};

struct FWeakObjectPtr
{
public:
	inline bool SerialNumbersMatch(FUObjectItem* ObjectItem) const
	{
		return ObjectItem->SerialNumber == ObjectSerialNumber;
	}

	bool IsValid() const;

	UObject* Get() const;

	int32_t ObjectIndex;
	int32_t ObjectSerialNumber;
};

template<class T, class TWeakObjectPtrBase = FWeakObjectPtr>
struct TWeakObjectPtr : private TWeakObjectPtrBase
{
public:
	inline T* Get() const
	{
		return (T*)TWeakObjectPtrBase::Get();
	}

	inline T& operator*() const
	{
		return *Get();
	}

	inline T* operator->() const
	{
		return Get();
	}

	inline bool IsValid() const
	{
		return TWeakObjectPtrBase::IsValid();
	}
};

template<class T, class TBASE>
class TAutoPointer : public TBASE
{
public:
	inline operator T*() const
	{
		return TBASE::Get();
	}

	inline operator const T*() const
	{
		return (const T*)TBASE::Get();
	}

	explicit inline operator bool() const
	{
		return TBASE::Get() != nullptr;
	}
};

template<class T>
class TAutoWeakObjectPtr : public TAutoPointer<T, TWeakObjectPtr<T>>
{
public:
};

template<typename TObjectID>
class TPersistentObjectPtr
{
public:
	FWeakObjectPtr WeakPtr;
	int32_t TagAtLastTest;
	TObjectID ObjectID;
};

struct FStringAssetReference_
{
	char UnknownData[0x10];
};

class FAssetPtr : public TPersistentObjectPtr<FStringAssetReference_>
{

};

template<typename ObjectType>
class TAssetPtr : FAssetPtr
{

};

struct FUniqueObjectGuid_
{
	char UnknownData[0x10];
};

class FLazyObjectPtr : public TPersistentObjectPtr<FUniqueObjectGuid_>
{

};

template<typename ObjectType>
class TLazyObjectPtr : FLazyObjectPtr
{

};)";
	}

	std::string GetBasicDefinitions() const override
	{
		return R"(TNameEntryArray* FName::GNames = nullptr;
FUObjectArray* UObject::GObjects = nullptr;

bool FWeakObjectPtr::IsValid() const
{
	if (ObjectSerialNumber == 0)
	{
		return false;
	}
	if (ObjectIndex < 0)
	{
		return false;
	}
	auto ObjectItem = UObject::GetGlobalObjects().GetItemByIndex(ObjectIndex);
	if (!ObjectItem)
	{
		return false;
	}
	if (!SerialNumbersMatch(ObjectItem))
	{
		return false;
	}
	return !(ObjectItem->IsUnreachable() || ObjectItem->IsPendingKill());
}

UObject* FWeakObjectPtr::Get() const
{
	if (IsValid())
	{
		auto ObjectItem = UObject::GetGlobalObjects().GetItemByIndex(ObjectIndex);
		if (ObjectItem)
		{
			return ObjectItem->Object;
		}
	}
	return nullptr;
}
)";
	}
};

Generator _generator;
IGenerator* generator = &_generator;

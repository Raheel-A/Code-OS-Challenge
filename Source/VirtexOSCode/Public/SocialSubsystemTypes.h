#pragma once

#include <CoreMinimal.h>
#include "SocialSubsystemTypes.generated.h"

USTRUCT(BlueprintType)
struct VIRTEXOSCODE_API FFriendStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	bool bOnline;

	UPROPERTY(BlueprintReadOnly)
	FString Nickname;

	friend inline bool operator==(const FFriendStruct& Lhs, const FFriendStruct& Rhs)
	{
		return Lhs.Nickname == Rhs.Nickname;
	}

	friend uint32 GetTypeHash(const FFriendStruct& A)
	{
		return GetTypeHash(A.Nickname);
	}
};

USTRUCT()
struct FFriendJson {
	GENERATED_BODY()
public:
	UPROPERTY()
	FString Name;
	UPROPERTY()
	bool Status;
};

USTRUCT()
struct FFriendResponseJson {
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<FFriendJson> Friends;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFriendsUpdated, bool, bChanged, const TArray<FFriendStruct>&, Users);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendStatusChanged, FFriendStruct, User);

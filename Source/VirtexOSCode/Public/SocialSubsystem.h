// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SocialSubsystemTypes.h"
#include "SocialSubsystem.generated.h"


/*
* This class represents the "social" system for the app. Its main goal is to provide updates on friend list
* It has two delegates OnFriendsUpdated which should be triggered every time we finished querying the friend list
* and OnFriendStatusUpdate which should be triggered only if previous status (Online/Offline) of a single friend entity have been changed
* 
* 
* 1)You need to call OnFriendsUpdated.Broadcast(...) and OnFriendStatusChanged.Broadcast(...)
* to trigger UI changes ingame.
* 
* 2)You need to implement GetFriends() function, which should syncronously return a list of friends (used in blueprints)
* It can return empty list if query have failed or havent ever been complete yet.
* 
* Extra task:
* 3) GetFriends() function should return a list of friends sorted alphabetically and Online players should always be in the beggining of the array
* 
* REQUIREMENTS:
* 
* You must not call QueryFriendList more than once in 5 seconds.
* 
* You can add members to the class and use any other systems.
* You can override any parent functions if you need to.
* You can modify any function in this class.
* 
* You cannot add/modify any blueprints.
* You can add/modify types if you see fit.
*/



UCLASS()
class VIRTEXOSCODE_API USocialSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FOnFriendStatusChanged OnFriendStatusChanged;

	UPROPERTY(BlueprintAssignable)
	FOnFriendsUpdated OnFriendsUpdated;

	UFUNCTION(BlueprintPure)
	const TArray<FFriendStruct> GetFriends();


	// -- FTickableGameObject START
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	// -- FTickableGameObject END
private:
	bool QueryFriendList();
	void OnQueryComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

};

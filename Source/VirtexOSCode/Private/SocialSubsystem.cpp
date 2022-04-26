// Fill out your copyright notice in the Description page of Project Settings.


#include "SocialSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHTTPResponse.h"
#include "JsonObjectConverter.h"

const TArray<FFriendStruct> USocialSubsystem::GetFriends()
{
	//Implement me
	return TArray<FFriendStruct>();
}

void USocialSubsystem::Tick(float DeltaTime)
{
}

TStatId USocialSubsystem::GetStatId() const
{
	return TStatId();
}

bool USocialSubsystem::IsTickable() const
{
	return true;
}

bool USocialSubsystem::QueryFriendList()
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	FString URL = TEXT("https://yt6avkmffh.execute-api.eu-west-2.amazonaws.com/default/VirtexOSCodeChallenge");
	Request->SetURL(URL);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetVerb(TEXT("GET"));
	Request->OnProcessRequestComplete().BindUObject(this, &USocialSubsystem::OnQueryComplete);
	return Request->ProcessRequest();
}
void USocialSubsystem::OnQueryComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (!bConnectedSuccessfully || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		return;
	}
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!(FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid()))
	{
		return;
	}
	FFriendResponseJson JsonResponse;
	if (!FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), FFriendResponseJson::StaticStruct(), &JsonResponse))
	{
		return;
	}
	//JsonResponse contains a fresh friendlist
	/*
	*  Example of how to construct friends array:
	* 
	* TArray<FFriendStruct> Friends;
	* for (auto& JsonFriend : JsonResponse.Friends)
	* {
	* 	Friends.Add(FFriendStruct{ JsonFriend.Status, JsonFriend.Name });
	* }
	* OnFriendsUpdated.Broadcast(true, Friends);
	*/
}

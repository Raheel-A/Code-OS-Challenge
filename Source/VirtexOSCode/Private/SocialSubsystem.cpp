// Fill out your copyright notice in the Description page of Project Settings.


#include "SocialSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHTTPResponse.h"
#include "JsonObjectConverter.h"
#include <algorithm>

const TArray<FFriendStruct> USocialSubsystem::GetFriends()
{
	QueryFriendList();
	return *MyFriendsOrdered;
}

void USocialSubsystem::Tick(float DeltaTime)
{
	MyTicker += DeltaTime;
	if (MyTicker >= TickInterval)
	{
		QueryFriendList();
		MyTicker = 0.0f;
	}
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
	if (FirstLoad)
	{
		for (int i = 0; i < JsonResponse.Friends.Num(); i++)
		{
			if (JsonResponse.Friends[i].Status)
			{
				MyFriendsOnline->Add(FFriendStruct{ JsonResponse.Friends[i].Status, JsonResponse.Friends[i].Name });
			}
			else
			{
				MyFriendsOffline->Add(FFriendStruct{ JsonResponse.Friends[i].Status, JsonResponse.Friends[i].Name });
			}
			FirstLoad = false;
		}

		MyFriendsOnline->Sort();
		for (FFriendStruct Friend : *MyFriendsOnline)
		{
			OnFriendStatusChanged.Broadcast(Friend);
		}
		MyFriendsOffline->Sort();

	}
	else
	{
		for (int i = 0; i < JsonResponse.Friends.Num(); i++)
		{
			FFriendStruct TempFriend = FFriendStruct{ JsonResponse.Friends[i].Status, JsonResponse.Friends[i].Name };

			int32 Index = MyFriendsOnline->Find(TempFriend);

			//if Index == -1, it means the friend was not found in the online list
			if (Index == -1)
			{
				//If they were not found on the online list, but are online now, that means their status has changed.
				if (JsonResponse.Friends[i].Status)
				{
					MyFriendsOffline->Remove(TempFriend);
					MyFriendsOnline->Add(TempFriend);
					OnFriendStatusChanged.Broadcast(TempFriend);
				}
			}

			//If Index != -1, it means the friend was found in the online list
			else
			{
				//If they were found on the online list, but are not online, that means their status has changed.
				if (!JsonResponse.Friends[i].Status)
				{
					MyFriendsOnline->Remove(TempFriend);
					MyFriendsOffline->Add(TempFriend);
					OnFriendStatusChanged.Broadcast(TempFriend);
				}
			}

			i++;

		}

		MyFriendsOnline->Sort();
		MyFriendsOffline->Sort();

	}

	MyFriendsOrdered = CombineArrays(MyFriendsOnline, MyFriendsOffline);

	OnFriendsUpdated.Broadcast(true, *MyFriendsOrdered);

}

TArray<FFriendStruct>* USocialSubsystem::CombineArrays(TArray<FFriendStruct>* ArrayOne, TArray<FFriendStruct>* ArrayTwo)
{
	TArray<FFriendStruct>* tempCombiner = new TArray<FFriendStruct>;

	tempCombiner->Append(*ArrayOne);
	tempCombiner->Append(*ArrayTwo);

	return tempCombiner;
}

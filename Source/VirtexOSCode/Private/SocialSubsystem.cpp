// Fill out your copyright notice in the Description page of Project Settings.


#include "SocialSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHTTPResponse.h"
#include "JsonObjectConverter.h"

// This only seems to be called once at the start of the program.
const TArray<FFriendStruct> USocialSubsystem::GetFriends()
{
	QueryFriendList();
	return *MyFriendsOrdered;
}

//TickInterval is set to 5 seconds to limit the Friends list query to a maximum of once per 5 seconds.
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

#pragma region "Raheel changes"
	
	//Because the list of Friends isn't preloaded anywhere, used a Firstload boolean to bypass any sorting or checking during the first pass.
	if (FirstLoad)
	{
		for (int i = 0; i < JsonResponse.Friends.Num(); i++)
		{
			//For each Friend in the list, sort by status into two arrays.
			if (JsonResponse.Friends[i].Status)
			{
				MyFriendsOnline->Add(FFriendStruct{ JsonResponse.Friends[i].Status, JsonResponse.Friends[i].Name });
			}
			else
			{
				MyFriendsOffline->Add(FFriendStruct{ JsonResponse.Friends[i].Status, JsonResponse.Friends[i].Name });
			}
		}

		//Sort the online friends before broadcasting so the "Friend is online" messages are also in order.
		MyFriendsOnline->Sort();
		for (FFriendStruct Friend : *MyFriendsOnline)
		{
			//We only want to broadcast the online friends on first load.
			OnFriendStatusChanged.Broadcast(Friend);
		}

		//Sort the Offline friends for later.
		MyFriendsOffline->Sort();

		//Forces this section of code to only run once.
		FirstLoad = false;
	}

	//If this isn't the first load, we have to check for changes in the friends list.
	else
	{
		FFriendStruct TempFriend;
		bool OnlineIsSmaller;
		int32 Index;
		for (int i = 0; i < JsonResponse.Friends.Num(); i++)
		{
			//Grab the Friend element at the current position of the JsonResponse.
			TempFriend = FFriendStruct{ JsonResponse.Friends[i].Status, JsonResponse.Friends[i].Name };

			//Discover which array is smaller (decreases the search). 
			//Bool asks "Online is smaller" because online is the first one being input
			OnlineIsSmaller = IsArraySmaller(MyFriendsOnline, MyFriendsOffline);

			//Then search for the friend in the smaller array.
			if (OnlineIsSmaller)
			{
				Index = MyFriendsOnline->Find(TempFriend);
			}
			else
			{
				Index = MyFriendsOffline->Find(TempFriend);
			}

			//if Index == -1, it means the friend was not found in the searched list
			if (Index == -1)
			{

				//if their current status is the same as the list in which they weren't found, then they need to be updated.
				if (OnlineIsSmaller == JsonResponse.Friends[i].Status)
				{
					//If the searched list was Online, remove from offline and add to online.
					if (OnlineIsSmaller)
					{
						MyFriendsOffline->Remove(TempFriend);
						MyFriendsOnline->Add(TempFriend);
					}
					//Otherwise remove from online and add to offline.
					else
					{
						MyFriendsOnline->Remove(TempFriend);
						MyFriendsOffline->Add(TempFriend);
					}

					// Then Broadcast the change.
					OnFriendStatusChanged.Broadcast(TempFriend);
				}
			}

			//increase the increment for the For loop
			i++;
		}

		//Sort alphabetical again.
		MyFriendsOnline->Sort();
		MyFriendsOffline->Sort();

	}

	//combine the arrays to create the full friends list
	MyFriendsOrdered = CombineArrays(MyFriendsOnline, MyFriendsOffline);

	//Last step is to broadcast OnFriendsUpdated after we are done with the friends list.
	OnFriendsUpdated.Broadcast(true, *MyFriendsOrdered);
}

//General function to append one TArray onto another
TArray<FFriendStruct>* USocialSubsystem::CombineArrays(TArray<FFriendStruct>* ArrayOne, TArray<FFriendStruct>* ArrayTwo)
{
	TArray<FFriendStruct>* tempCombiner = new TArray<FFriendStruct>;

	tempCombiner->Append(*ArrayOne);
	tempCombiner->Append(*ArrayTwo);

	return tempCombiner;
}

//Compares the size of the two TArrays.
bool USocialSubsystem::IsArraySmaller(TArray<FFriendStruct>* OnlineFriends, TArray<FFriendStruct>* OfflineFriends)
{
	return OnlineFriends->Num() < OfflineFriends->Num();
}

#pragma endregion
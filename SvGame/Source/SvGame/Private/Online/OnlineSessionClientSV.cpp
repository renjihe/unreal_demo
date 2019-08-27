// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "OnlineSessionClientSV.h"

void UOnlineSessionClientSV::OnSessionUserInviteAccepted(const bool bWasSuccess, const int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult) 
{

}

void UOnlineSessionClientSV::OnPlayTogetherEventReceived(int32 UserIndex, TArray<TSharedPtr<const FUniqueNetId>> UserIdList) 
{

}



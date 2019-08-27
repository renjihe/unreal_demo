// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "OnlineSessionClient.h"
#include "OnlineSessionClientSV.generated.h"

/**
 * 
 */
UCLASS()
class SVGAME_API UOnlineSessionClientSV : public UOnlineSessionClient
{
	GENERATED_BODY()


	virtual void OnSessionUserInviteAccepted(const bool bWasSuccess, const int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult) override;
	virtual void OnPlayTogetherEventReceived(int32 UserIndex, TArray<TSharedPtr<const FUniqueNetId>> UserIdList) override;
};

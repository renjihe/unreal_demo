// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "PlayerControllerSV.generated.h"

/**
 * 
 */
UCLASS()
class SVGAME_API APlayerControllerSV : public APlayerController
{
	GENERATED_UCLASS_BODY()

	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction);
public:
	/** notify player about started match */
	UFUNCTION(reliable, client)
	void ClientGameStarted();

	/** Starts the online game using the session name in the PlayerState */
	UFUNCTION(reliable, client)
	void ClientStartOnlineGame();

	/** Handle for efficient management of ClientStartOnlineGame timer */
	FTimerHandle TimerHandle_ClientStartOnlineGame;

	/** Ends the online game using the session name in the PlayerState */
	UFUNCTION(reliable, client)
	void ClientEndOnlineGame();

	/** notify player about finished match */
	virtual void ClientGameEnded_Implementation(class AActor* EndGameFocus, bool bIsWinner);
	
	
	float SayTimer, SayInterval;
	UFUNCTION(BlueprintCallable, server, reliable, WithValidation)
	void ServerSay(const FString &content);

	UFUNCTION(BlueprintCallable, NetMulticast, reliable)
	void NetMulticastSay(const FString &content);

	UFUNCTION(reliable, server, WithValidation)
	void ServerReplicateTrans(AActor *actor);

	UFUNCTION(reliable, NetMulticast)
	void NetMulticastReplicateTrans(AActor *actor, const FVector &relativeLocation, const FRotator &relativeRotation);

	UFUNCTION(reliable, server, WithValidation)
	void ServerTestCmd(const FString &Cmd, const FString &Arg);

	virtual void Destroyed() override;
};

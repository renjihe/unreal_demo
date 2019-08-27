// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "client/entityinterface.h"
#include "Common/GameData.h"

class AActor;
class UMeshComponent;
class UParticleSystemComponent;
class UAudioComponent;

class SPGAME_API FEntityComponent : public EntityInterface
{
public:	
	FEntityComponent(AActor *owner);
	virtual ~FEntityComponent();
public:	
	//entity interface
	virtual void EntityDestory();
	virtual void SetCurrPos(float x, float y, float z);
	virtual void SetCurrDir(float pitch, float yaw, float roll);
	virtual void GetCurrPos(float &x, float &y, float &z);
	virtual void GetCurrDir(float &x, float &y, float &z);

	virtual void AddYawInput(float yaw);
	virtual float GetYawInput();
	virtual void AddPitchInput(float pitch);
	virtual float GetPitchInput();
	virtual void Possess();

	virtual void PlayAction(int id);
	virtual void StopAction();
	virtual void PlayParticle(int id, const char *name, AvatarPosition pos, const char *socket, bool bAttached = false, bool bAutoDestroy = true);
	virtual void PlayParticle(int id, const char *name, float locX, float locY, float locZ, float normalX, float normalY, float normalZ, bool bAutoDestroy = true);
	virtual void StopPariticle(int id);
	virtual void MovePariticle(int id, AvatarPosition pos, const char *socket);
	virtual void Spray(int caster, int id, int faction, float locX, float locY, float locZ, float normalX, float normalY, float normalZ, float dirX, float dirY, float dirZ, float sizeX, float sizeY, float sizeZ);

	virtual void SetDead();
	virtual void Rebirth();
	void OnAtked(float hpRate);

	virtual void Show(bool bShow);
	virtual void Cosplay(bool bCos);

	virtual void SwithAvatar(int AvatarId, AvatarPosition AvatarPos);
	virtual void ShowEnergyBar(bool bShow);
	virtual void SetEnergyPercent(float percent);

	bool bIsDead;
	bool bCosplay;
	//component
	int GetFaction();

	void OnParticleSystemFinished(class UParticleSystemComponent* FinishedComponent);
	FTransform MakeFromTransform(AvatarPosition fromPosition, FName fromSocket, FVector relaLocation = FVector::ZeroVector, FRotator relaRotation = FRotator::ZeroRotator);
private:
	void ShowAttachedActors(bool bShow);
	virtual UMeshComponent* GetSpecMesh(AvatarPosition fromPosition);

	void GetFromActorAndMesh(AvatarPosition fromPosition, AActor *&actor, UMeshComponent *&meshComp);
	UParticleSystemComponent* PlayParticleInternal(int id, const char *name, AvatarPosition fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation, bool bAttached = false, bool bAutoDestroy = true);
	UAudioComponent* PlaySoundInternal(int soundID, AvatarPosition fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation);

private:
	AActor *Owner;
	float YawInput;
	float PitchInput;
	TMap<int, UParticleSystemComponent*> Pariticles;
};

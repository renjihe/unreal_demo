// Fill out your copyright notice in the Description page of Project Settings.

#include "EntityComponent.h"
#include "SpGame.h"
#include "SpCharacter.h"
#include "AbilityObj.h"
#include "Res/ResLibrary.h"
#include "SprayMgr.h"
#include "SpGameModeBattle.h"
#include "GUI/DlgBattleInfo.h"
#include "GUI/SpHUD.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Particles/ParticleSystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "client/dllclient.h"

FEntityComponent::FEntityComponent(AActor *owner)
{
	//PrimaryComponentTick.bCanEverTick = false;
	Owner = owner;
	YawInput = 0.0f;
	PitchInput = 0.0f;
	bIsDead = false;
	bCosplay = false;
}

FEntityComponent::~FEntityComponent()
{

}

void FEntityComponent::EntityDestory()
{
	if (Owner) {
		Owner->Destroy();
	}
}

void FEntityComponent::SetCurrPos(float x, float y, float z)
{
	if (Owner) {
		Owner->SetActorLocation(FVector(x, y, z));
		//UKismetSystemLibrary::DrawDebugSphere(Owner->GetWorld(), Owner->GetActorLocation(), 12.f, 12, FLinearColor::White, 10);
	}
}

void FEntityComponent::SetCurrDir(float pitch, float yaw, float roll)
{
	if (Owner) {
		Owner->SetActorRotation(FRotator(pitch, yaw, roll), ETeleportType::TeleportPhysics);
	}
}

void FEntityComponent::GetCurrPos(float &x, float &y, float &z)
{
	if (Owner) 
	{
		FVector location = Owner->GetActorLocation();
		x = location.X;
		y = location.Y;
		z = location.Z;
	}
}

void FEntityComponent::GetCurrDir(float &x, float &y, float &z)
{
	if (Owner)
	{
		FRotator rotation = Owner->GetActorRotation();
		rotation.Pitch -= PitchInput;
		FVector forward = rotation.Vector();
		x = forward.X;
		y = forward.Y;
		z = forward.Z;
	}
}

void FEntityComponent::AddYawInput(float yaw)
{
	float scale = 1.0f;
	if (GSpWorld) {
		APlayerController *controller = GSpWorld->GetFirstPlayerController();
		if (controller) {
			scale = controller->InputYawScale;
		}
	}

	YawInput += yaw * scale;
	ASpCharacter *character = Cast<ASpCharacter>(Owner);
	if (character && character->Controller && character->Controller->IsLocalPlayerController()) {
		character->AddControllerYawInputImpl(yaw);
	}
}

float FEntityComponent::GetYawInput()
{
	return YawInput;
}

#define PITCH_INPUT_MAX 60.0f
#define PITCH_INPUT_MIN -50.0f
void FEntityComponent::AddPitchInput(float pitch)
{
	float scale = 1.0f;
	if (GSpWorld) {
		APlayerController *controller = GSpWorld->GetFirstPlayerController();
		if (controller) {
			scale = controller->InputPitchScale;
		}
	}

	float OldInput = PitchInput;

	PitchInput += pitch * scale;
	if (PitchInput > PITCH_INPUT_MAX)
	{
		pitch = (PITCH_INPUT_MAX - OldInput) / scale;
		PitchInput = PITCH_INPUT_MAX;
	}
	else if (PitchInput < PITCH_INPUT_MIN)
	{
		pitch = (PITCH_INPUT_MIN - OldInput) / scale;
		PitchInput = PITCH_INPUT_MIN;
	}

	ASpCharacter *character = Cast<ASpCharacter>(Owner);
	if (character && character->Controller && character->Controller->IsLocalPlayerController()) {
		character->AddControllerPitchInputImpl(pitch);
	}
}

extern FVector GetSprayColor(int faction);
void FEntityComponent::Possess()
{
	ASpCharacter *pawn = Cast<ASpCharacter>(Owner);
	if (pawn)
	{
		APlayerController *controller = Owner->GetWorld()->GetFirstPlayerController();
		if (controller)
		{
			ASpHUD *hud = Cast<ASpHUD>(controller->MyHUD);
			if (hud) {
				hud->SetEnergyColor(GetSprayColor(pawn->Faction));
			}

			APawn *defaultPawn = controller->GetPawn();
			controller->UnPossess();
			defaultPawn->Destroy();
			controller->Possess(pawn);
		}
	}
}

void FEntityComponent::PlayAction(int id)
{
	if (AnimationType::None == (AnimationType::Type)id)
		return;

	ASpCharacter *character = Cast<ASpCharacter>(Owner);
	if (character)
	{
		UAnimInstance* animInstance = Cast<UAnimInstance>(character->GetMesh()->GetAnimInstance());
		if (!animInstance) {
			return;
		}

		const JobInfo &job = spclient::getJobInfo(character->JobId);
		const FLookRow &look = FLookRes::Get().GetLook(job.nLook);
		if (!look.Montages.Contains((AnimationType::Type)id)) {
			return;
		}

		bool isPlayed = false;
		if (animInstance->IsAnyMontagePlaying())
		{
			UAnimMontage* currMontage = animInstance->GetCurrentActiveMontage();
			if (currMontage == look.Montages[(AnimationType::Type)id].Montage)
			{
				FName loopSection(*FString("loop"));
				if (currMontage->IsValidSectionName(loopSection)) 
				{
					animInstance->Montage_JumpToSection(loopSection, currMontage);
					isPlayed = true;
				}
			}
		}

		if (!isPlayed) {
			animInstance->Montage_Play(look.Montages[(AnimationType::Type)id].Montage, look.Montages[(AnimationType::Type)id].Rate);
		}
	}
}

void FEntityComponent::StopAction()
{
	ASpCharacter *character = Cast<ASpCharacter>(Owner);
	if (character)
	{
		UAnimInstance* animInstance = Cast<UAnimInstance>(character->GetMesh()->GetAnimInstance());
		if (!animInstance) {
			return;
		}

		bool isPlayed = false;
		if (animInstance->IsAnyMontagePlaying())
		{
			UAnimMontage* currMontage = animInstance->GetCurrentActiveMontage();
			FName endSection(*FString("end"));
			if (currMontage && currMontage->IsValidSectionName(endSection))
			{
				animInstance->Montage_JumpToSection(endSection, currMontage);
				isPlayed = true;
			}
		}

		if (!isPlayed) {
			animInstance->Montage_Stop(0);
		}
	}
}

void FEntityComponent::PlayParticle(int id, const char *name, AvatarPosition pos, const char *socket, bool bAttached, bool bAutoDestroy)
{
	PlayParticleInternal(id, name, pos, socket, FVector(0, 0, 0), FRotator(0, 0, 0), bAttached, bAutoDestroy);
}

void FEntityComponent::Spray(int caster, int id, int faction, float locX, float locY, float locZ, float normalX, float normalY, float normalZ, float dirX, float dirY, float dirZ, float sizeX, float sizeY, float sizeZ)
{
	const FSprayMaskRow &row = UResLibrary::GetSprayMaskByID(id);
	if (row.Mask) {
		ASprayMgr::Spray(caster, FVector(locX, locY, locZ), FVector(normalX, normalY, normalZ), FVector(dirX, dirY, dirZ), faction, row.Mask, FVector(sizeX, sizeY, sizeZ), row.Range);
	}
}

void FEntityComponent::SetDead()
{
	if (Owner) 
	{
		bIsDead = true;
		Show(false);

		ASpCharacter *character = Cast<ASpCharacter>(Owner);
		if (character)
		{
			if (this == spclient::getBattleUserEntityInterface()) {
				UDlgBattleInfo::GetInstance()->ShowDeadNotice();
			}

			PlayAction(AnimationType::Die);
		}
	}
}

void FEntityComponent::Rebirth()
{
	if (Owner) 
	{
		bIsDead = false;

		Show(true);

		ASpCharacter *character = Cast<ASpCharacter>(Owner);
		if (character)
		{
			YawInput = 0;
			PitchInput = 0;
			if (character->Controller && character->Controller->IsPlayerController()) {
				character->Controller->SetControlRotation(FRotator::ZeroRotator);
			}

			character->SprayRate = 0.0f;
			character->UpdateSprayRate();

			character->AtkedRate = 1.0f;
			character->AtkedTimer = 0.0f;
			character->UpdateAtkedRate();
		}

		if (this == spclient::getBattleUserEntityInterface()) {
			UDlgBattleInfo::GetInstance()->ShowRebirthNotice();
		}

		PlayAction(AnimationType::Birth);
	}
}

void FEntityComponent::OnAtked(float hpRate)
{
	ASpCharacter *character = Cast<ASpCharacter>(Owner);
	if (character)
	{
		character->SprayRate = 1.0f;
		character->UpdateSprayRate();

		character->AtkedRate = hpRate;
		character->AtkedTimer = 3.0f;
		character->UpdateAtkedRate();
	}
}

void FEntityComponent::Show(bool bShow)
{
	if (Owner)
	{
#if 1
		Owner->SetActorHiddenInGame(!bShow);
#else
		TArray<UActorComponent*> components = Owner->GetComponentsByClass(UMeshComponent::StaticClass());
		for (UActorComponent *component : components)
		{
			UMeshComponent *mesh = Cast<UMeshComponent>(component);
			if (mesh) {
				mesh->SetHiddenInGame(!bShow);
			}
		}
#endif
		Owner->SetActorEnableCollision(bShow);

		if (bShow && bCosplay)
			return;

		ShowAttachedActors(bShow);
	}
}

void FEntityComponent::ShowAttachedActors(bool bShow)
{
	if (Owner)
	{
		TArray<AActor *> actors;
		Owner->GetAttachedActors(actors);
		for (int i = 0; i < actors.Num(); ++i) {
			actors[i]->SetActorHiddenInGame(!bShow);
		}
	}
}

void FEntityComponent::Cosplay(bool bCos)
{
	ASpCharacter *character = Cast<ASpCharacter>(Owner);
	if (character) 
	{
		bCosplay = bCos;
		//PlayAction(bCos ? AnimationType::CosplayBegin : AnimationType::CosplayEnd);
		FLookRes::Get().CreateLook(bCos ? 0 : character->LookId, character->GetMesh());
		ShowAttachedActors(!bCos);
	}
}

void FEntityComponent::SwithAvatar(int AvatarId, AvatarPosition AvatarPos)
{
	ASpCharacter *character = Cast<ASpCharacter>(Owner);
	if (character) {
		character->AttachActor(AvatarId, AvatarPos);
	}
}

void FEntityComponent::ShowEnergyBar(bool bShow)
{
	ASpCharacter *character = Cast<ASpCharacter>(Owner);
	if (character)
	{
		if (this == spclient::getBattleUserEntityInterface())
		{
			APlayerController *controller = Cast<APlayerController>(character->Controller);
			if (controller) 
			{
				ASpHUD *hud = Cast<ASpHUD>(controller->MyHUD);
				hud->ShowEnergyBar(bShow);
			}
		}
	}
}

void FEntityComponent::SetEnergyPercent(float percent)
{
	ASpCharacter *character = Cast<ASpCharacter>(Owner);
	if (character)
	{
		if (this == spclient::getBattleUserEntityInterface())
		{
			APlayerController *controller = Cast<APlayerController>(character->Controller);
			if (controller)
			{
				ASpHUD *hud = Cast<ASpHUD>(controller->MyHUD);
				hud->SetEnergyPercent(percent);
			}
		}
	}
}

//skill interface
UMeshComponent* FEntityComponent::GetSpecMesh(AvatarPosition fromPosition)
{
	UMeshComponent *meshComp = nullptr;
	switch (fromPosition)
	{
	case AvatarPosition::Actor:
	{
		ASpCharacter *character = Cast<ASpCharacter>(Owner);
		if (character) {
			meshComp = Cast<UMeshComponent>(character->GetMesh());
		}

		if (nullptr == meshComp) {
			meshComp = Cast<UMeshComponent>(Owner->GetComponentByClass(UMeshComponent::StaticClass()));
		}
	}
		break;
	default:
		ASpCharacter *character = Cast<ASpCharacter>(Owner);
		if (character) 
		{
			AActor *actor = character->Avatar[fromPosition];
			if (actor) {
				meshComp = Cast<UMeshComponent>(actor->GetComponentByClass(UMeshComponent::StaticClass()));
			}
		}
		break;
	}

	return meshComp;
}

void FEntityComponent::GetFromActorAndMesh(AvatarPosition fromPosition, AActor *&actor, UMeshComponent *&meshComp)
{
	actor = nullptr;
	meshComp = nullptr;
	switch (fromPosition)
	{
	case AvatarPosition::Actor:
		actor = Owner;
		meshComp = GetSpecMesh(fromPosition);
		break;
	default:
		meshComp = GetSpecMesh(fromPosition);
		if (meshComp) {
			actor = meshComp->GetOwner();
		}
		break;
	}
}

FTransform FEntityComponent::MakeFromTransform(AvatarPosition fromPosition, FName fromSocket, FVector relaLocation, FRotator relaRotation)
{
	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	GetFromActorAndMesh(fromPosition, actor, mesh);

	FTransform trans;
	if (actor)
	{
		if (fromSocket.IsNone()) {
			trans = actor->GetTransform();
		}
		else {
			trans = mesh->GetSocketTransform(fromSocket);
		}
	}

	FTransform rela(relaRotation, relaLocation);

	FTransform ret;
	FTransform::Multiply(&ret, &rela, &trans);
	return ret;
}

void FEntityComponent::PlayParticle(int id, const char *name, float locX, float locY, float locZ, float normalX, float normalY, float normalZ, bool bAutoDestroy)
{
	UParticleSystemComponent* psc = nullptr;
	FParticleRow &particle = ParticleRes::Get().GetParticle(name);
	if (particle.Particle == nullptr) {
		return;
	}

	psc = UGameplayStatics::SpawnEmitterAtLocation(Owner, particle.Particle, FVector(locX, locY, locZ), FVector(normalX, normalY, normalZ).Rotation(), bAutoDestroy);
	if (psc)
	{
		//init spray color
		FVector sprayColor = GetSprayColor(GetFaction());
		psc->SetVectorParameter(TEXT("SprayColor"), sprayColor);
	}

	//return psc;
}

UParticleSystemComponent* FEntityComponent::PlayParticleInternal(int id, const char *name, AvatarPosition fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation, bool bAttached, bool bAutoDestroy)
{
	UParticleSystemComponent* psc = nullptr;
	FParticleRow &particle = ParticleRes::Get().GetParticle(name);
	if (particle.Particle == nullptr) {
		return psc;
	}

	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	GetFromActorAndMesh(fromPosition, actor, mesh);

	switch (fromPosition)
	{
	case AvatarPosition::Actor:
		if (actor)
		{
			if (bAttached) {
				psc = UGameplayStatics::SpawnEmitterAttached(particle.Particle, actor->GetRootComponent(), fromSocket, relativeLocation, relativeRotation, EAttachLocation::KeepRelativeOffset, bAutoDestroy);
			}
			else
			{
				FTransform trans = MakeFromTransform(fromPosition, fromSocket, relativeLocation, relativeRotation);
				psc = UGameplayStatics::SpawnEmitterAtLocation(Owner, particle.Particle, trans.GetLocation(), trans.Rotator(), bAutoDestroy);
			}
		}
		break;
	default:
		if (mesh)
		{
			if (bAttached) {
				psc = UGameplayStatics::SpawnEmitterAttached(particle.Particle, mesh, fromSocket, relativeLocation, relativeRotation, EAttachLocation::KeepRelativeOffset, bAutoDestroy);
			}
			else
			{
				FRotator RotatorOffset = UKismetMathLibrary::ComposeRotators(particle.Rotator, relativeRotation);
				FTransform trans = MakeFromTransform(fromPosition, fromSocket, relativeLocation, RotatorOffset);
				FVector loc = trans.GetLocation();
				FRotator rot = trans.Rotator();
				psc = UGameplayStatics::SpawnEmitterAtLocation(Owner, particle.Particle, loc, rot, bAutoDestroy);
			}
		}
		break;
	}

	if (psc) 
	{
		ASpCharacter *character = Cast<ASpCharacter>(Owner);
		if (character)
		{
			if (id > 0 && !bAutoDestroy)
			{
				psc->OnSystemFinished.AddUniqueDynamic(character, &ASpCharacter::OnParticleSystemFinished);
				UParticleSystemComponent** Pariticle = Pariticles.Find(id);
				if (Pariticle)
				{
					(*Pariticle)->Complete();
					(*Pariticle)->DestroyComponent();
				}

				Pariticles.Add(id, psc);
			}
		}

		psc->CustomTimeDilation = particle.TimeDilation;
		psc->SetWorldScale3D(particle.Scale);

		//init spray color
		FVector sprayColor = GetSprayColor(GetFaction());
		psc->SetVectorParameter(TEXT("SprayColor"), sprayColor);
	}

	return psc;
}

void FEntityComponent::StopPariticle(int id)
{
	ASpCharacter *character = Cast<ASpCharacter>(Owner);
	if (character)
	{
		UParticleSystemComponent** Pariticle = Pariticles.Find(id);
		if (Pariticle)
		{
			(*Pariticle)->Complete();
			(*Pariticle)->DestroyComponent();
		}
	}
}

void FEntityComponent::MovePariticle(int id, AvatarPosition pos, const char *socket)
{
	ASpCharacter *character = Cast<ASpCharacter>(Owner);
	if (character)
	{
		UParticleSystemComponent** Pariticle = Pariticles.Find(id);
		if (Pariticle)
		{
			FTransform trans = MakeFromTransform(pos, socket, FVector::ZeroVector, FRotator::ZeroRotator);
			(*Pariticle)->SetRelativeLocation(trans.GetLocation());
			(*Pariticle)->SetRelativeRotation(trans.Rotator());
		}
	}
}

void FEntityComponent::OnParticleSystemFinished(class UParticleSystemComponent* FinishedComponent)
{
	ASpCharacter *character = Cast<ASpCharacter>(Owner);
	if (character) 
	{
		FinishedComponent->OnSystemFinished.RemoveDynamic(character, &ASpCharacter::OnParticleSystemFinished);
		for (TMap<int, UParticleSystemComponent*>::TIterator It(Pariticles); It; ++It)
		{
			const UParticleSystemComponent *v = It.Value();
			if (FinishedComponent == v) 
			{
				Pariticles.Remove(It.Key());
				break;
			}
		}
	}
}

UAudioComponent* FEntityComponent::PlaySoundInternal(int soundID, AvatarPosition fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation)
{
	UAudioComponent* ac = nullptr;
	/*FSoundRow &sound = SoundRes::Get().GetSound(soundID);
	if (sound.Sound == nullptr) {
	return ac;
	}

	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	GetFromActorAndMesh(fromActor, fromPosition, actor, mesh);

	switch (fromPosition)
	{
	case EEffectPosition::Mesh:
	case EEffectPosition::GripMesh:
	if (mesh) {
	FTransform trans = MakeFromTransform(fromActor, fromPosition, fromSocket, relativeLocation, relativeRotation);
	FVector loc = trans.GetLocation();
	FRotator rot = trans.Rotator();
	ac = UGameplayStatics::SpawnSoundAtLocation(fromActor, sound.Sound, loc, rot, sound.Volume, sound.Pitch);
	}
	break;
	case EEffectPosition::MeshAttached:
	case EEffectPosition::GripMeshAttached:
	if (mesh) {
	ac = UGameplayStatics::SpawnSoundAttached(sound.Sound, mesh, fromSocket, relativeLocation, relativeRotation, EAttachLocation::KeepRelativeOffset, true, sound.Volume, sound.Pitch);
	}
	break;
	case EEffectPosition::Actor:
	case EEffectPosition::GripActor:
	if (actor) {
	FTransform trans = MakeFromTransform(fromActor, fromPosition, fromSocket, relativeLocation, relativeRotation);
	ac = UGameplayStatics::SpawnSoundAtLocation(fromActor, sound.Sound, trans.GetLocation(), trans.Rotator(), sound.Volume, sound.Pitch);
	}
	break;
	case EEffectPosition::ActorAttached:
	case EEffectPosition::GripActorAttached:
	if (actor) {
	ac = UGameplayStatics::SpawnSoundAttached(sound.Sound, actor->GetRootComponent(), fromSocket, relativeLocation, relativeRotation, EAttachLocation::KeepRelativeOffset, true, sound.Volume, sound.Pitch);
	}
	break;
	default:
	break;
	}

	if (ac)
	{
	ac->bOverrideAttenuation = true;
	ac->AttenuationOverrides.AttenuationShapeExtents.X = sound.Radius;
	ac->AttenuationOverrides.FalloffDistance = sound.FalloffDistance;
	}
	*/
	return ac;
}

float FEntityComponent::GetPitchInput()
{
	return PitchInput;
}

int FEntityComponent::GetFaction()
{
	if (Cast<ASpCharacter>(Owner))
	{
		return Cast<ASpCharacter>(Owner)->Faction;
	}
	else if (Cast<AAbilityObj>(Owner))
	{
		return Cast<AAbilityObj>(Owner)->Faction;
	}
	return 0;
}

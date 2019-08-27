// Fill out your copyright notice in the Description page of Project Settings.
#include "SpCharacter.h"
#include "EntityComponent.h"
#include "SpGame.h"
#include "Res/ResLibrary.h"
#include "SpGameModeBattle.h"

#include "Containers/Array.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/MeshComponent.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Particles/ParticleSystemComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"

#include "client/dllclient.h"
#include "Gui/DlgSysSetting.h"

using namespace spclient;
using namespace summer;

static int InputForward = 0, InputRight = 0;
static int InputForwardPrev = 0, InputRightPrev = 0;

ASpCharacter::ASpCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetCapsuleComponent()->SetCapsuleSize(BATTLECHAR_RADIUS, BATTLECHAR_HALF_HEIGHT);
	GetMesh()->SetRelativeLocation(FVector(0, 0, -BATTLECHAR_HALF_HEIGHT));

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0, 0, BATTLECHAR_HEIGHT));
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller

	// Create a follow camera
	/*FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm*/

	//PrimaryActorTick.bCanEverTick = false;
	if (!HasAnyFlags(RF_ClassDefaultObject)) 
	{
		EntityComponent = MakeShareable(new FEntityComponent(this));
	}

	//this->GetCapsuleComponent()->SetRelativeLocation(0, 0, 100);
	this->bSimGravityDisabled = true;

	LookId = Invalid_Look;
	CurrLookId = Invalid_Look;
	JobId = 0;
	Faction = 0;
	memset(Avatar, 0, sizeof(Avatar));

	for (unsigned i = 0; i < AvatarPosition::Count; ++i) {
		CurrAvatarId[i] = -1;
	}
}

void ASpCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASpCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	EntityComponent = NULL;
}

void ASpCharacter::InitSpray()
{
	SprayRate = 0.0f;
	AtkedRate = 1.0f;
	AtkedTimer = 0.0f;

	FVector sprayColor = GetSprayColor(Faction);
	FVector atkedColor = GetSprayColor(Faction ? 0 : 1);

	TArray<UActorComponent*> meshs = GetComponentsByClass(UMeshComponent::StaticClass());
	for (UActorComponent *it : meshs)
	{
		UMeshComponent *mesh = Cast<UMeshComponent>(it);
		TArray<class UMaterialInterface *> materials = mesh->GetMaterials();
		for (int i = 0; i < materials.Num(); ++i)
		{
			if (materials[i] && nullptr == Cast<UMaterialInstanceDynamic>(materials[i]))
			{
				UMaterialInstanceDynamic *mat = UMaterialInstanceDynamic::Create(materials[i], this);
				if (mat)
				{
					mesh->SetMaterial(i, mat);
					mat->SetVectorParameterValue(TEXT("SprayColor"), sprayColor);
					mat->SetVectorParameterValue(TEXT("AtkedColor"), atkedColor);
					mat->SetScalarParameterValue(TEXT("SprayRate"), SprayRate);
					mat->SetScalarParameterValue(TEXT("AtkedRate"), AtkedRate);
				}
			}
		}
	}
}

void ASpCharacter::InitAttachmentSpray(AActor *actor)
{
	if (actor)
	{
		UMeshComponent *mesh = Cast<UMeshComponent>(actor->GetComponentByClass(UMeshComponent::StaticClass()));
		if (mesh)
		{
			FVector sprayColor = GetSprayColor(Faction);
			FVector atkedColor = GetSprayColor(Faction ? 0 : 1);

			TArray<class UMaterialInterface *> materials = mesh->GetMaterials();
			for (int i = 0; i < materials.Num(); ++i)
			{
				if (materials[i] && nullptr == Cast<UMaterialInstanceDynamic>(materials[i]))
				{
					UMaterialInstanceDynamic *mat = UMaterialInstanceDynamic::Create(materials[i], actor);
					if (mat)
					{
						mesh->SetMaterial(i, mat);
						mat->SetVectorParameterValue(TEXT("SprayColor"), sprayColor);
						mat->SetVectorParameterValue(TEXT("AtkedColor"), atkedColor);
						mat->SetScalarParameterValue(TEXT("SprayRate"), SprayRate);
						mat->SetScalarParameterValue(TEXT("AtkedRate"), AtkedRate);
					}
				}
			}
		}
	}
}

void ASpCharacter::Init(FVector Location, FRotator Rotator)
{
	bInit = true;

	int avatarId = 0;
	AvatarPosition avatarPos = AvatarPosition::RightHand;

	spclient::getSkillAvatar(EntityId, summer::MOB_SKILL_TYPE::MOB_SKILL_NORMAL, avatarId, avatarPos);
	AttachActor(avatarId, avatarPos);

	InitSpray();
}

void ASpCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bInit) {
		Init(FVector(), FRotator());
	}

	UpdateLook();

	if (spclient::getBattleResult() != 0)
		return;

	if ((InputForward != InputForwardPrev || InputRight != InputRightPrev)
		&& (Controller && Controller->IsLocalController())) 
	{
		InputForwardPrev = InputForward;
		InputRightPrev = InputRight;
		battleCmdMove(InputForward, InputRight);
	}

	float *pos, *dir, speed;
	unsigned state;
	if (spclient::getBattleCharMovement(EntityId, &pos, &dir, state, &speed))
	{
		MoveState = state;
		MoveSpeed = speed;

		SetActorLocationAndRotation(FVector(pos[0], pos[1], pos[2])
									, FRotationMatrix::MakeFromXZ(FVector(dir[0], dir[1], dir[2]), FVector(0, 0, 1)).Rotator());
	}
}

bool ASpCharacter::IsFalling() const
{
	return IS_MOVE_STATE_FALL(MoveState);
}

FVector ASpCharacter::GetVelocity() const
{
	float *dir, speed = 0;
	FVector ret = GetActorForwardVector();
	if (spclient::getBattleCharMoveDir(EntityId, &dir, &speed))
	{
		ret = FRotationMatrix::MakeFromXZ(FVector(dir[0], dir[1], dir[2]), FVector(0, 0, 1)).Rotator().Vector();
		ret.Normalize();
	}

	ret *= speed;
	return ret;
}

float ASpCharacter::GetYawInput() const
{
	if (EntityComponent.IsValid()) {
		return EntityComponent->GetYawInput();
	}

	return 0.0f;
}

float ASpCharacter::GetPitchInput() const
{
	if (EntityComponent.IsValid()) {
		return EntityComponent->GetPitchInput();
	}

	return 0.0f;
}

void ASpCharacter::PrintSocketInfo(int pos, FName fromSocket, FVector relaLocation, FRotator relaRotation)
{
#if WITH_EDITOR
	if (EntityComponent.IsValid()) 
	{
		FTransform trans = EntityComponent->MakeFromTransform((AvatarPosition)pos, fromSocket, relaLocation, relaRotation);
		trans = trans.GetRelativeTransform(GetTransform());
		FVector loc = trans.GetLocation();
		FRotator rot = trans.Rotator();

		TArray<FStringFormatArg> args;
		args.Add(loc.X);
		args.Add(loc.Y);
		args.Add(loc.Z);
		args.Add(rot.Pitch);
		args.Add(rot.Yaw);
		args.Add(rot.Roll);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Format(TEXT("loc:{0},{1},{2} rot:{3},{4},{5}"), args));

		UE_LOG(LogTemp, Log, TEXT("loc:%f,%f,%f rot:%f,%f,%f"), loc.X, loc.Y, loc.Z, rot.Pitch, rot.Yaw, rot.Roll);
	}
#endif
}

void ASpCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ASpCharacter::PossessedBy(AController *newController)
{
	Super::PossessedBy(newController);
}

USceneComponent* ASpCharacter::GetDefaultAttachComponent() const
{
	USkeletalMeshComponent *mesh = Cast<USkeletalMeshComponent>(GetComponentByClass(USkeletalMeshComponent::StaticClass()));
	if (mesh)
		return mesh;
	
	return GetRootComponent();
}

void ASpCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &ASpCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ASpCharacter::MoveRight);
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	InputComponent->BindAction("JumpBtn", IE_Pressed, this, &ASpCharacter::Jump);
	InputComponent->BindAction("JumpBtn", IE_Released, this, &ASpCharacter::StopJumping);

	//InputComponent->BindAction("Sneak", IE_Pressed, this, &ASpCharacter::Sneak);
	//InputComponent->BindAction("Sneak", IE_Released, this, &ASpCharacter::StopSneak);

	InputComponent->BindAction("Skill1", IE_Pressed, this, &ASpCharacter::KeyQPress);
	InputComponent->BindAction("Skill1", IE_Released, this, &ASpCharacter::KeyQRelease);

	InputComponent->BindAction("Skill2", IE_Pressed, this, &ASpCharacter::KeyEPress);
	InputComponent->BindAction("Skill2", IE_Released, this, &ASpCharacter::KeyERelease);

	InputComponent->BindAction("MouseLeft", IE_Pressed, this, &ASpCharacter::MouseLeftPress);
	InputComponent->BindAction("MouseRight", IE_Pressed, this, &ASpCharacter::MouseRightPress);
	InputComponent->BindAction("MouseLeft", IE_Released, this, &ASpCharacter::MouseLeftRelease);
	InputComponent->BindAction("MouseRight", IE_Released, this, &ASpCharacter::MouseRightRelease);

	if (Controller && Controller->IsLocalController()) {
		InputForward = InputRight = InputForwardPrev = InputRightPrev = 0;
	}
}

void ASpCharacter::Jump()
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (g_ClientStage == CLIENT_BATTLE) {
		battleCmdKey(BATTLE_CMD_KEY_SPACE, true, 0);
	}
	else {
		ACharacter::Jump();
	}
}

void ASpCharacter::StopJumping()
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (g_ClientStage != CLIENT_BATTLE) {
		ACharacter::StopJumping();
	}
}

/*void ASpCharacter::Sneak()
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (g_ClientStage == CLIENT_BATTLE) {
		battleCmdKey(BATTLE_CMD_KEY_ALT, true, 0);
	}
}

void ASpCharacter::StopSneak()
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (g_ClientStage == CLIENT_BATTLE) {
		battleCmdKey(BATTLE_CMD_KEY_ALT, false, 0);
	}
}*/


void ASpCharacter::KeyQPress()
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (g_ClientStage == CLIENT_BATTLE) {
		battleCmdKey(BATTLE_CMD_KEY_Q, true, 0);
	}
}

void ASpCharacter::KeyQRelease()
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (g_ClientStage == CLIENT_BATTLE) {
		battleCmdKey(BATTLE_CMD_KEY_Q, false, 0);
	}
}

void ASpCharacter::KeyEPress()
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (g_ClientStage == CLIENT_BATTLE) {
		battleCmdKey(BATTLE_CMD_KEY_E, true, 0);
	}
}

void ASpCharacter::KeyERelease()
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (g_ClientStage == CLIENT_BATTLE) {
		battleCmdKey(BATTLE_CMD_KEY_E, false, 0);
	}
}

void ASpCharacter::AddControllerPitchInput(float Val)
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (Val != 0.f && Controller && Controller->IsLocalPlayerController())
	{
		if (g_ClientStage == CLIENT_BATTLE)
		{
			battleCmdMouseMove(summer::BATTLE_CMD_MOUSE_Y, Val);
		}
		else {
			AddControllerPitchInputImpl(Val);
		}
	}
}

void ASpCharacter::AddControllerYawInput(float Val)
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (Val != 0.f && Controller && Controller->IsLocalPlayerController())
	{
		if (g_ClientStage == CLIENT_BATTLE)
		{
			battleCmdMouseMove(summer::BATTLE_CMD_MOUSE_X, Val);
		}
		else {
			AddControllerYawInputImpl(Val);
		}
	}
}

void ASpCharacter::MoveForward(float value)
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (g_ClientStage == CLIENT_BATTLE) 
	{
		if (value > 0.01f) {
			InputForward = 1;
		}
		else if (value < -0.01f) {
			InputForward = -1;
		}
		else {
			InputForward = 0;
		}			
	}
	else if (value) {
		MoveForwardImpl(value);
	}
}

void ASpCharacter::MoveRight(float value)
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (g_ClientStage == CLIENT_BATTLE)
	{
		if (value > 0.01f) {
			InputRight = 1;
		}
		else if (value < -0.01f) {
			InputRight = -1;
		}
		else {
			InputRight = 0;
		}
	}
	else if (value) {
		MoveRightImpl(value);
	}
}

void ASpCharacter::MouseLeftPress()
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (g_ClientStage == CLIENT_BATTLE) {
		battleCmdMouseButton(BATTLE_CMD_MOUSE_LEFT, true);
	}
	else {
		MouseLeftPressImpl();
	}
}

void ASpCharacter::MouseLeftRelease()
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (g_ClientStage == CLIENT_BATTLE) {
		battleCmdMouseButton(BATTLE_CMD_MOUSE_LEFT, false);
	}
	else {
		MouseLeftReleaseImpl();
	}
}

void ASpCharacter::MouseRightPress()
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (g_ClientStage == CLIENT_BATTLE) {
		battleCmdMouseButton(BATTLE_CMD_MOUSE_RIGHT, true);
	}
	else {
		MouseRightPressImpl();
	}
}

void ASpCharacter::MouseRightRelease()
{
	if (!spclient::canCtrl(EntityId)) {
		return;
	}

	if (g_ClientStage == CLIENT_BATTLE) {
		battleCmdMouseButton(BATTLE_CMD_MOUSE_RIGHT, false);
	}
	else {
		MouseRightReleaseImpl();
	}
}

void ASpCharacter::MoveForwardImpl(float value, float Yaw)
{
	if (value != 0.0f)
	{
		// 找到当前的前方向
		const FRotator YawRotation(0, (Controller != NULL) ? Controller->GetControlRotation().Yaw : Yaw, 0);
#if 0
		// 获取到前方向  
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, value);
#else 
		const FVector Direction = UKismetMathLibrary::GetForwardVector(YawRotation);
		AddMovementInput(Direction, value, true);
#endif
	}
}
void ASpCharacter::MoveRightImpl(float value, float Yaw)
{
	if (value != 0.0f)
	{
		// 找到当前的前方向  
		const FRotator YawRotation(0, (Controller != NULL) ? Controller->GetControlRotation().Yaw : Yaw, 0);
#if 0
		// 获取到右方向  
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, value);
#else 
		const FVector Direction = UKismetMathLibrary::GetRightVector(YawRotation);
		AddMovementInput(Direction, value, true);
#endif
	}
}

void ASpCharacter::AddControllerPitchInputImpl(float Val)
{
	APawn::AddControllerPitchInput(Val);
}

void ASpCharacter::AddControllerYawInputImpl(float Val)
{
	APawn::AddControllerYawInput(Val);
}

void ASpCharacter::MouseLeftPressImpl()
{
}

void ASpCharacter::MouseLeftReleaseImpl()
{
}

void ASpCharacter::MouseRightPressImpl()
{
}

void ASpCharacter::MouseRightReleaseImpl()
{
}

void ASpCharacter::SetLook(int look)
{
	FLookRes::Get().CreateLook(look, GetMesh());

	InitSpray();

	CurrLookId = LookId = look;
}

void ASpCharacter::AttachActor(AActor* childActor, AvatarPosition pos)
{
	if (Avatar[pos])
	{
		GetWorld()->DestroyActor(Avatar[pos]);
		Avatar[pos] = nullptr;
		CurrAvatarId[pos] = -1;
	}

	if (childActor)
	{
		childActor->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false), GetAvatarName(pos));
		InitAttachmentSpray(childActor);
		Avatar[pos] = childActor;
	}
}

void ASpCharacter::AttachActor(int avatarId, AvatarPosition pos)
{
	if (avatarId < 0) 
	{
		AttachActor(nullptr, pos);
		return;
	}
	
	if (CurrAvatarId[pos] == avatarId) {
		return;
	}

	FAvatarRow avatar = UResLibrary::GetAvatarByID(avatarId);
	if (avatar.Avatar && GSpWorld)
	{
		AActor *actor = GetWorld()->SpawnActor(avatar.Avatar);
		AttachActor(actor, pos);

		CurrAvatarId[pos] = avatarId;
	}
}

void ASpCharacter::UpdateLook()
{
	if (CurrLookId != LookId) {
		SetLook(LookId);
	}

	if (SprayRate < 1.0f)
	{
		SprayRate += 0.011f;

		UpdateSprayRate();
	}

	AtkedTimer -= 0.011f;
	if (AtkedRate < 1.0f && AtkedTimer <= 0.0f)
	{
		AtkedRate += 0.011f;

		UpdateAtkedRate();
	}

	if (Controller && Controller->IsPlayerController())
	{
		FVector camLoc;
		FRotator camRot;
		Cast<APlayerController>(Controller)->GetPlayerViewPoint(camLoc, camRot);
		float pitch = camRot.Pitch;

		float alpha = 1;
		if (pitch > 40) {
			alpha = FMath::Lerp<float, float>(0, 1, 1 - abs(pitch) / 90);
		}
		ForEachNeedUpdateMat([&](UMaterialInstanceDynamic* matd) {
			matd->SetScalarParameterValue("Alpha", alpha);
		});
	}
}

void ASpCharacter::UpdateSprayRate()
{
	ForEachNeedUpdateMat([&](UMaterialInstanceDynamic* matd) {
		matd->SetScalarParameterValue("SprayRate", SprayRate);
	});
}

void ASpCharacter::UpdateAtkedRate()
{
	ForEachNeedUpdateMat([&](UMaterialInstanceDynamic* matd) {
		matd->SetScalarParameterValue("AtkedRate", AtkedRate);
	});
}

void ASpCharacter::ForEachNeedUpdateMat(TFunctionRef<void(UMaterialInstanceDynamic*)> Predicate)
{
	TArray<UActorComponent*> meshs = GetComponentsByClass(UMeshComponent::StaticClass());
	for (UActorComponent *it : meshs)
	{
		UMeshComponent *mesh = Cast<UMeshComponent>(it);
		TArray<class UMaterialInterface*> materials = mesh->GetMaterials();
		for (UMaterialInterface *mat : materials)
		{
			UMaterialInstanceDynamic *matd = Cast<UMaterialInstanceDynamic>(mat);
			if (matd) {
				Predicate(matd);
			}
		}
	}

	TArray<AActor *> actors;
	GetAttachedActors(actors);
	for (AActor *actor : actors)
	{
		UMeshComponent* mesh = Cast<UMeshComponent>(actor->GetComponentByClass(UMeshComponent::StaticClass()));
		if (mesh)
		{
			TArray<class UMaterialInterface *> materials = mesh->GetMaterials();
			for (int i = 0; i < materials.Num(); ++i)
			{
				if (materials[i] && nullptr != Cast<UMaterialInstanceDynamic>(materials[i])) {
					Predicate(Cast<UMaterialInstanceDynamic>(materials[i]));
				}
			}
		}
	}
}

void ASpCharacter::OnParticleSystemFinished(UParticleSystemComponent* FinishedComponent)
{
	EntityComponent->OnParticleSystemFinished(FinishedComponent);
}

#if WITH_EDITOR
void ASpCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UpdateLook();
}
#endif

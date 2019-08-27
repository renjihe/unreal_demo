// Fill out your copyright notice in the Description page of Project Settings.

#include "SpPlayerController.h"
#include "CollisionQueryParams.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/SpCharacter.h"
#include "Character/EntityComponent.h"
#include "GUI/SpHUD.h"
#include "client/dllclient.h"

static float s_pitch = 0.0f;
static FVector s_traceStart;
void ASpPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ASpHUD *hud = Cast<ASpHUD>(MyHUD);
	ASpCharacter* pawn = Cast<ASpCharacter>(GetPawn());
	if (hud && pawn && pawn->EntityComponent.IsValid())
	{
		const JobInfo &job = spclient::getJobInfo(pawn->JobId);
		int skillId = job.wSkill[summer::MOB_SKILL_NORMAL];

		float distance = spclient::getSkillDis(skillId);

		FVector offsetFrom;
		spclient::getSkillOffsetFrom(skillId, offsetFrom.X, offsetFrom.Y, offsetFrom.Z);

		FTransform rela(FRotator(0.0f, 0.0f, 0.0f), offsetFrom);
		FTransform pawnTrans = pawn->GetTransform();
		FTransform ret;
		FTransform::Multiply(&ret, &rela, &pawnTrans);

		FVector traceStart = s_traceStart;
		s_traceStart = ret.GetLocation();

		FRotator pawnRot = pawn->GetActorRotation();
		FVector camLoc;
		FRotator camRot;
		GetPlayerViewPoint(camLoc, camRot);
		camRot.Pitch = s_pitch;
		s_pitch = pawnRot.Pitch + pawn->EntityComponent->GetPitchInput();
		//FRotator rotator = UKismetMathLibrary::ComposeRotators(EntityComponent->GetRelativeRotator(), FromerRotator);

		FVector direction = camRot.Vector();
		direction.Normalize();
		const FVector traceEnd = traceStart + (direction * distance);

		FVector2D screenPos;
		ProjectWorldLocationToScreen(traceEnd, screenPos);
		hud->UpdatePos(screenPos);

		FVector2D engeryBarPos;
		FVector2D engeryBarSize(1024, 1024);
		ProjectWorldLocationToScreen((pawn->GetActorLocation() - FVector(0, 0, pawn->GetDefaultHalfHeight())), engeryBarPos);
		hud->UpdateEnergyBarPos(engeryBarPos, engeryBarSize);

		//
		FCollisionQueryParams traceParams(FName(TEXT("TraceUsableActor")), true, this);
		traceParams.bTraceAsyncScene = true;
		traceParams.bReturnPhysicalMaterial = false;
		traceParams.bTraceComplex = true;

		//UKismetSystemLibrary::DrawDebugLine(GetWorld(), traceStart, traceEnd, FLinearColor(1, 0, 1, 1), 0.2, 2.0f);

		FHitResult Hit(ForceInit);
		if (GetWorld()->LineTraceSingleByChannel(Hit, traceStart, traceEnd, ECC_Visibility, traceParams))
		{
			FVector2D tipScreenPos;
			ProjectWorldLocationToScreen(Hit.ImpactPoint, tipScreenPos);

			hud->UpdateTipPos(tipScreenPos);
		}
		else {
			hud->ResetTipPos();
		}
	}
}



// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"

/** when you modify this, please note that this information can be saved with instances
* also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list **/
#define COLLISION_WEAPON		ECC_GameTraceChannel1
#define COLLISION_PROJECTILE	ECC_GameTraceChannel2
#define COLLISION_PICKUP		ECC_GameTraceChannel3

#define TRACE_FIRE ETraceTypeQuery::TraceTypeQuery3
#define TRACE_PROP ETraceTypeQuery::TraceTypeQuery4
#define TRACE_MELEE ETraceTypeQuery::TraceTypeQuery5
#define TRACE_BOMB_BLOCK ETraceTypeQuery::TraceTypeQuery6

#define MAX_PLAYER_NAME_LENGTH 16

// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * WhitelineNightmareGameplayTags
 *
 * Native gameplay tags for Whiteline Nightmare.
 * These tags are automatically registered and available throughout the project.
 * No manual setup in Project Settings required!
 */
namespace WhitelineNightmareGameplayTags
{
	// ===== ABILITY TAGS =====
	// Tags for gameplay abilities

	/** Lane change ability - Used by war rig to switch lanes */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_LaneChange);

	/** Turret firing ability - Used by turrets to attack enemies */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_TurretFire);

	/** Raider attack ability - Used by enemy raiders to attack the war rig */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_RaiderAttack);

	/** Game over ability - Triggered when game ends (win or lose) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_GameOver);

	// ===== STATE TAGS =====
	// Tags representing entity states

	/** Moving state - Entity is actively moving */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Moving);

	/** Dead state - Entity has been destroyed/killed */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);

	/** Lane changing state - War rig is currently changing lanes */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_LaneChanging);

	// ===== DAMAGE TAGS =====
	// Tags for damage types

	/** Direct damage - Standard damage from weapons/collisions */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Direct);

	/** Explosive damage - Area of effect damage */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Explosive);

	/** Collision damage - Damage from physical collisions */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Collision);

	// ===== EFFECT TAGS =====
	// Tags for gameplay effects

	/** Fuel drain effect - Reduces fuel over time */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_FuelDrain);

	/** Fuel restore effect - Restores fuel (from pickups) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_FuelRestore);

	/** Armor restore effect - Restores hull/armor */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_ArmorRestore);

	/** Speed boost effect - Increases lane change speed */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_SpeedBoost);

	// ===== MOUNT POINT TAGS =====
	// Tags for turret mount point filtering

	/** Mount point is on the cab - May have restricted firing angles */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mount_Cab);

	/** Mount point is on a trailer - More versatile placement */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mount_Trailer);

	/** Mount point is at the rear - Good for rear-facing defense */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mount_Rear);

	/** Mount point can support heavy turrets - Structural reinforcement */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mount_Heavy);

	/** Mount point is suitable for anti-air turrets - Elevated position */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mount_AntiAir);

	// ===== ENEMY TAGS =====
	// Tags for enemy types and behaviors

	/** Ground enemy - Approaches on the road */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Ground);

	/** Air enemy - Flies above the war rig */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Air);

	/** Boss enemy - Larger, tougher enemies */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Boss);

	// ===== PICKUP TAGS =====
	// Tags for pickup types

	/** Fuel pickup */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pickup_Fuel);

	/** Scrap/currency pickup */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pickup_Scrap);

	/** Armor repair pickup */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pickup_Armor);

	// ===== TURRET TAGS =====
	// Tags for turret types

	/** Ballistic turret - Standard projectile weapons */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Turret_Ballistic);

	/** Energy turret - Laser/energy weapons */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Turret_Energy);

	/** Explosive turret - Area damage weapons */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Turret_Explosive);

	/** Support turret - Utility/defensive turrets */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Turret_Support);
}

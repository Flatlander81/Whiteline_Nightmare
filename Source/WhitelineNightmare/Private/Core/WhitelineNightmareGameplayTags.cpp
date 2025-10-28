// Copyright Flatlander81. All Rights Reserved.

#include "Core/WhitelineNightmareGameplayTags.h"

namespace WhitelineNightmareGameplayTags
{
	// ===== ABILITY TAGS =====
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_LaneChange, "Ability.LaneChange", "Lane change ability - Used by war rig to switch lanes");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_TurretFire, "Ability.TurretFire", "Turret firing ability - Used by turrets to attack enemies");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_RaiderAttack, "Ability.RaiderAttack", "Raider attack ability - Used by enemy raiders to attack the war rig");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_GameOver, "Ability.GameOver", "Game over ability - Triggered when game ends (win or lose)");

	// ===== STATE TAGS =====
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Moving, "State.Moving", "Moving state - Entity is actively moving");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dead, "State.Dead", "Dead state - Entity has been destroyed/killed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_LaneChanging, "State.LaneChanging", "Lane changing state - War rig is currently changing lanes");

	// ===== DAMAGE TAGS =====
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Direct, "Damage.Direct", "Direct damage - Standard damage from weapons/collisions");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Explosive, "Damage.Explosive", "Explosive damage - Area of effect damage");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Collision, "Damage.Collision", "Collision damage - Damage from physical collisions");

	// ===== EFFECT TAGS =====
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_FuelDrain, "Effect.FuelDrain", "Fuel drain effect - Reduces fuel over time");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_FuelRestore, "Effect.FuelRestore", "Fuel restore effect - Restores fuel (from pickups)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_ArmorRestore, "Effect.ArmorRestore", "Armor restore effect - Restores hull/armor");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_SpeedBoost, "Effect.SpeedBoost", "Speed boost effect - Increases lane change speed");

	// ===== MOUNT POINT TAGS =====
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Mount_Cab, "Mount.Cab", "Mount point is on the cab - May have restricted firing angles");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Mount_Trailer, "Mount.Trailer", "Mount point is on a trailer - More versatile placement");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Mount_Rear, "Mount.Rear", "Mount point is at the rear - Good for rear-facing defense");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Mount_Heavy, "Mount.Heavy", "Mount point can support heavy turrets - Structural reinforcement");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Mount_AntiAir, "Mount.AntiAir", "Mount point is suitable for anti-air turrets - Elevated position");

	// ===== ENEMY TAGS =====
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Enemy_Ground, "Enemy.Ground", "Ground enemy - Approaches on the road");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Enemy_Air, "Enemy.Air", "Air enemy - Flies above the war rig");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Enemy_Boss, "Enemy.Boss", "Boss enemy - Larger, tougher enemies");

	// ===== PICKUP TAGS =====
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Pickup_Fuel, "Pickup.Fuel", "Fuel pickup");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Pickup_Scrap, "Pickup.Scrap", "Scrap/currency pickup");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Pickup_Armor, "Pickup.Armor", "Armor repair pickup");

	// ===== TURRET TAGS =====
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Turret_Ballistic, "Turret.Ballistic", "Ballistic turret - Standard projectile weapons");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Turret_Energy, "Turret.Energy", "Energy turret - Laser/energy weapons");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Turret_Explosive, "Turret.Explosive", "Explosive turret - Area damage weapons");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Turret_Support, "Turret.Support", "Support turret - Utility/defensive turrets");
}

# Whiteline Nightmare

A Mad Max-inspired tower defense game built in Unreal Engine 5.6.

## Overview

Command a war rig driving through post-apocalyptic wastelands. Defend against raiders, manage fuel and armor, collect scrap, and fight to survive. The world scrolls past your stationary rig as you mount turrets and battle endless waves of enemies.

## Development Approach

- **Code-First**: C++ primary, Blueprints for data/content only
- **Test-Driven**: Comprehensive automated testing framework
- **Defensive Programming**: Input validation, null checks, graceful error handling
- **GAS Integration**: Gameplay Ability System for all abilities and attributes

## Documentation

- **[PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md)**: Comprehensive development guide
  - Project structure and organization
  - Data table definitions
  - Testing framework usage
  - Code style guidelines
  - Development workflow

## Quick Start

1. Clone the repository
2. Open `WhitelineNightmare.uproject` in Unreal Engine 5.6
3. Compile the C++ code
4. Review `PROJECT_STRUCTURE.md` for detailed documentation

## Testing

Run automated tests:
- In-editor: Create test map with `ATestingGameMode`
- Console command: `RunTests [Category]`
- Categories: All, Movement, Combat, Economy, Spawning, ObjectPool, GAS

## Requirements

- Unreal Engine 5.6
- Visual Studio 2022 or JetBrains Rider
- Gameplay Ability System plugin (enabled)

## License

Copyright Flatlander81. All Rights Reserved.
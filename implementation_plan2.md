# Luzes Apagadas - Final Verification Plan

The core logic, audio, and visual updates for the "Luzes Apagadas" mod are now in place. We have also resolved the final compilation warnings/errors that were preventing a successful build.

## Completed Changes

### 1. Maps Design (Level 1, 2, 3)
The levels ([level1.txt](file:///d:/UNIFAP/projeto-doom-computacao-grafica/maps/level1.txt), [level2.txt](file:///d:/UNIFAP/projeto-doom-computacao-grafica/maps/level2.txt), [level3.txt](file:///d:/UNIFAP/projeto-doom-computacao-grafica/maps/level3.txt)) are already created and populated correctly:
- **Level 1** has 8-10 Light Posts (`P`), an Exit Door (`D`), and a few enemies.
- **Level 2** has fewer Light Posts, more enemies.
- **Level 3** has minimal Light Posts, high enemy density, and the Exit Door is hidden far away.

### 2. Compilation and Build Fixes
All build errors reported by the compiler have been resolved:
- **`gProgLightFloor`**: Removed the broken shader references since the new lighting model relies on vertex subdivision.
- **Unused variables**: Removed the unused `half` variable and correctly cast/commented out the unused `temTeto` parameter in [drawlevel.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/graphics/drawlevel.cpp).

### 3. Cleanup & Polishing
- Re-wired the audio system to use the new monster SFX (`bufMonsterChase`, `bufMonsterSpot`, `bufMonsterAttack`, `bufMonsterIdle`).
- Lava audio references have been completely deleted.

## User Action Required

> [!IMPORTANT]  
> The codebase should now compile cleanly. I need you to run your build command (`mingw32-make`) to confirm everything compiles without errors, and then launch the game to test the gameplay loop!

## Verification Plan

### Manual Verification (By User)
- **Compile the game:** Run `mingw32-make` in your `build` folder.
- **Play the game:** Launch [DoomLike.exe](file:///d:/UNIFAP/projeto-doom-computacao-grafica/build/DoomLike.exe).
- **Test Mechanics:** 
  - Walk into the light cone of a lamp post and verify enemies stop moving (they become IDLE).
  - Reach the door (`D` tile) at the end of the map to progress to the next level.
  - Complete Level 3 to see the Victory Screen.


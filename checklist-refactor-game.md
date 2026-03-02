# Game Improvements Task List

## 1. Remove Ammo & Gun Fire Mechanics
- [ ] Remove `ITEM_AMMO` / `ITEM_AMMO_BOX` from [entities.h](file:///d:/UNIFAP/projeto-doom-computacao-grafica/include/core/entities.h)
- [ ] Remove ammo fields (`currentAmmo`, `reserveAmmo`) from `PlayerState` in [game_state.h](file:///d:/UNIFAP/projeto-doom-computacao-grafica/include/core/game_state.h)
- [ ] Remove [WeaponAnim](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/core/player.cpp#16-21) struct and weapon-related code from [game_state.h](file:///d:/UNIFAP/projeto-doom-computacao-grafica/include/core/game_state.h) / [game_enums.h](file:///d:/UNIFAP/projeto-doom-computacao-grafica/include/core/game_enums.h)
- [ ] Delete [weapon.h](file:///d:/UNIFAP/projeto-doom-computacao-grafica/include/core/weapon.h) (or empty it)
- [ ] Gut [player.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/core/player.cpp): remove [playerTryReload()](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/core/player.cpp#6-10), [playerTryAttack()](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/core/player.cpp#11-15), [updateWeaponAnim()](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/core/player.cpp#16-21)
- [ ] Remove mouse-click attack in [input.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/input/input.cpp) ([mouseClick](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/input/input.cpp#127-132) → [playerTryAttack](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/core/player.cpp#11-15))
- [ ] Remove R-key reload binding in [input.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/input/input.cpp)
- [ ] Remove weapon anim update call in [game.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/core/game.cpp) ([updateWeaponAnim](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/core/player.cpp#16-21))
- [ ] Remove ammo reset in [gameReset()](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/core/game.cpp#169-196)
- [ ] Remove shot/reload/grunt audio buffers+sources from [audio_system.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/audio/audio_system.cpp) / [audio_system.h](file:///d:/UNIFAP/projeto-doom-computacao-grafica/include/audio/audio_system.h)
- [ ] Remove [audioPlayShot](file:///d:/UNIFAP/projeto-doom-computacao-grafica/include/audio/audio_system.h#85-87), [audioPlayReload](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/audio/audio_system.cpp#419-424), [audioOnPlayerShot](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/audio/audio_system.cpp#445-457) functions
- [ ] Remove ammo pickup logic from [entities.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/core/entities.cpp)
- [ ] Remove ammo items from level maps (A tiles)
- [ ] Remove ammo drop on enemy death in [player.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/core/player.cpp)
- [ ] Remove AMMO display from HUD ([hud.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/graphics/hud.cpp))
- [ ] Remove gun HUD textures from [assets.h](file:///d:/UNIFAP/projeto-doom-computacao-grafica/include/utils/assets.h) / [assets.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/utils/assets.cpp) (gun_fire, gun_reload, Shotgun.png)
- [ ] Remove gun-related fields from `HudTextures` / `HudState`
- [ ] Remove gun textures from `RenderAssets`
- [ ] Clean up [audio_tuning.h](file:///d:/UNIFAP/projeto-doom-computacao-grafica/include/audio/audio_tuning.h) (remove SHOT_GAIN, RELOAD_GAIN, PUMP_GAIN, GRUNT refs)

## 2. Add Key Collectible for Door Unlock
- [ ] Add `ITEM_KEY` to `ItemType` enum in [entities.h](file:///d:/UNIFAP/projeto-doom-computacao-grafica/include/core/entities.h)
- [ ] Add `hasKey` boolean to `PlayerState` in [game_state.h](file:///d:/UNIFAP/projeto-doom-computacao-grafica/include/core/game_state.h)
- [ ] Add 'L' tile for key placement in maps
- [ ] Update [level.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/level/level.cpp) to spawn key items from 'L' tiles
- [ ] Update [entities.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/core/entities.cpp) to handle key pickup
- [ ] Update door check in [game.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/core/game.cpp): require key + batteries
- [ ] Load key textures from [assets/items/Key0_Icon.png](file:///d:/UNIFAP/projeto-doom-computacao-grafica/assets/items/Key0_Icon.png) etc.
- [ ] Update HUD to show key status
- [ ] Place 'L' tiles in all 3 level maps

## 3. Fix Map Lighting (Blinking / Too Bright)
- [ ] Reduce indoor light intensity in [lighting.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/graphics/lighting.cpp) ([setupIndoorLightOnce](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/graphics/lighting.cpp#5-19))
- [ ] Reduce indoor ambient values in [drawlevel.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/graphics/drawlevel.cpp) (`kAmbientIndoor`)
- [ ] Adjust [flickerFluorescente](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/graphics/drawlevel.cpp#87-106) function to be subtle or remove flickering from non-safepost areas
- [ ] Ensure uniform ambient between indoor sections (reduce [beginIndoor](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/graphics/drawlevel.cpp#127-139) intensity multiplier)

## 4. Wall Dimensions: Thinner & Taller
- [ ] Reduce wall thickness by reducing `TILE_SIZE` or making walls render as thin slabs instead of full cubes
- [ ] Increase `WALL_H` constant in [drawlevel.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/graphics/drawlevel.cpp) (e.g., from 6.0 to 10.0+)
- [ ] Adjust ceiling height proportionally

## 5. Redesign Level 1 Map
- [ ] Design proper labyrinth layout with corridors
- [ ] Add "big open areas" for safe posts
- [ ] Place safe posts (P) with good spacing
- [ ] Place batteries (V), key (L), enemies (G/K/J), health (H)
- [ ] Place exit door (D)
- [ ] Ensure spawn point (9) is accessible

## 6. Remove Old Music/Global Textures Refs
- [ ] Remove [music.wav](file:///d:/UNIFAP/projeto-doom-computacao-grafica/assets/audio/music.wav) ambient load from [audio_system.cpp](file:///d:/UNIFAP/projeto-doom-computacao-grafica/src/audio/audio_system.cpp)
- [ ] Remove [lava_mono.wav](file:///d:/UNIFAP/projeto-doom-computacao-grafica/assets/audio/lava_mono.wav) references if any
- [ ] Clean up any unused texture loading

## 7. Audio Overhaul  
- [ ] Use [music_ambient_loop.ogg.mp3](file:///d:/UNIFAP/projeto-doom-computacao-grafica/assets/audio/music/music_ambient_loop.ogg.mp3) as main background music
- [ ] Reduce overall volume (lower `MASTER` or individual gains in [audio_tuning.h](file:///d:/UNIFAP/projeto-doom-computacao-grafica/include/audio/audio_tuning.h))
- [ ] Integrate new SFX from `assets/audio/sfx/`:
  - [ ] `sfx_door_open.flac` — when door opens
  - [ ] `sfx_door_locked.wav.wav` — when door is locked
  - [ ] `sfx_door_close.wav.wav` — door close
  - [ ] `sfx_step_concrete_01/02` — footstep sounds
  - [ ] `sfx_step_metal_01` — metal footstep
  - [ ] Monster SFX already integrated (chase, attack, idle, spot, death)
- [ ] Integrate music from `assets/audio/music/`:
  - [ ] `music_chase.ogg.mp3` — when being chased
  - [ ] `music_death.ogg.wav` — on game over
  - [ ] `music_intro.ogg.ogg` — on menu screen
  - [ ] `music_victory.ogg.mp3` — on victory


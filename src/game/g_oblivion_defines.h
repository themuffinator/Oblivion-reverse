/*
==============================================================================

  Oblivion specific defines and flags

==============================================================================
*/

#ifndef G_OBLIVION_DEFINES_H
#define G_OBLIVION_DEFINES_H

// Oblivion specific Means of Death
// Starting from 40 to avoid conflict with standard Q2 (0-33) and common mods
#define MOD_DISINTEGRATOR 40
#define MOD_DEATOMIZER 41
#define MOD_DEATOMIZER_SPLASH 42
#define MOD_PLASMA_PISTOL 43
#define MOD_PLASMA_RIFLE 44
#define MOD_DONUT 45
#define MOD_HELLFURY 46
#define MOD_LASERCANNON 47
#define MOD_DETPACK 48
#define MOD_MINE 49
#define MOD_MINE_SPLASH 50

// Spawnflags (SVF_) or Entity Flags (FL_)
// Using available bits where possible.
// SVF_NOCLIENT = 1, SVF_DEADMONSTER = 2, SVF_MONSTER = 4
#define SVF_PROJECTILE 0x00000008

// FL_ flags typically start at 0x00000001 (FL_FLY). standard goes up to
// 0x00002000 (FL_RESPAWN) Picking a high bit for safety.
#define FL_DODGE 0x00004000

// Effect Flags (EF_)
// Using a define to map to existing or unused flag
#define EF_DUALFIRE EF_ANIM_ALLFAST

#endif

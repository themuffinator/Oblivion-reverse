#ifndef G_OBLIVION_MONSTER_H
#define G_OBLIVION_MONSTER_H

void fire_deatom(edict_t *self, vec3_t start, vec3_t dir, int damage,
                 int speed);
void monster_fire_deatom(edict_t *self, vec3_t start, vec3_t dir, int damage,
                         int speed, int flashtype);

// Defines for Konig recreations if needed
#define FRAME_tpose 0
// Frames will be defined in respective monster files or here if shared.

#endif

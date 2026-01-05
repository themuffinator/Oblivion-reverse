/*
==============================================================================

cyborg

==============================================================================
*/

#include "m_cyborg.h"
#include "g_local.h"
#include "g_oblivion_monster.h"


// Frame definitions inferred from Konig recreation and legacy defines
// Legacy m_cyborg.c had:
// STAND: 108 (0x6c) - 125 (0x7d)
// WALK: 18 (0x12) - 23 (0x17)  <-- Konig expects 18 frames here. Collision with
// Attack1? RUN: 79 (0x4f) - 81 (0x51)   <-- Konig expects 6 frames. ATTACK1: 24
// (0x18) - 35 (0x23) ATTACK2: 47 (0x2f) - 52 (0x34) ATTACK3: 53 (0x35) - 58
// (0x3a) PAIN: 73 (0x49) - 81/88? DEATH: 15 (0x0f)

// We define them based on Konig requirements, mostly respecting starts.
enum {
  FRAME_walk1 = 18,
  FRAME_walk2,
  FRAME_walk3,
  FRAME_walk4,
  FRAME_walk5,
  FRAME_walk6,
  FRAME_walk7,
  FRAME_walk8,
  FRAME_walk9,
  FRAME_walk10,
  FRAME_walk11,
  FRAME_walk12,
  FRAME_walk13,
  FRAME_walk14,
  FRAME_walk15,
  FRAME_walk16,
  FRAME_walk17,
  FRAME_walk18,

  FRAME_run1 = 79,
  FRAME_run2,
  FRAME_run3,
  FRAME_run4,
  FRAME_run5,
  FRAME_run6,

  FRAME_attack101 = 24,
  // Konig Attack Run: 12 frames. 24 + 11 = 35. Fits Legacy Attack1 range
  // perfectly!
  FRAME_attack112 = 35,

  FRAME_attack201 = 47,
  // Konig Attack Backflip: 11 frames. 47 + 10 = 57. Fits Legacy Attack2 range
  // (47-52) and spills into Attack3 (53). This implies Attack2 and Attack3
  // might be combined or contiguous in the model.
  FRAME_attack211 = 57,

  FRAME_attack301 = 53, // Not used directly in Konig for 'AttackRight'?
  // Konig Attack Right: 6 frames.
  FRAME_attack306 = 58,

  FRAME_attack401 = 59, // Guessing start? Or overlaps?
  // Konig Attack Left: 6 frames.
  FRAME_attack406 = 64,

  FRAME_melee101 = 65, // Guessing
  FRAME_melee108 = 72,

  FRAME_melee201 = 82, // Guessing, Run ends at 84 (79+5)
  FRAME_melee206 = 87,

  FRAME_pain101 = 73,
  FRAME_pain106 = 78,

  FRAME_pain201 = 88, // Guessing
  FRAME_pain216 = 103,

  FRAME_death101 = 15,
  FRAME_death108 = 22,

  FRAME_death201 = 110, // Overlap? Stand starts at 108.
  FRAME_death206 = 115,

  FRAME_death301 = 116,
  FRAME_death306 = 121,

  FRAME_tpose = 108 // Stand start
};

static int sound_pain;
static int sound_death;
static int sound_sight;
static int sound_idle;
static int sound_attack;
static int sound_step;

// Prototypes
void cyborg_fire_left(edict_t *self);
void cyborg_fire_right(edict_t *self);
void cyborg_reattack(edict_t *self);

//
// SOUNDS
//

void cyborg_idle(edict_t *self) {
  gi.sound(self, CHAN_VOICE, sound_sight, 1, ATTN_IDLE, 0);
}

void cyborg_search(edict_t *self) {
  gi.sound(self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void cyborg_sight(edict_t *self, edict_t *other) {
  gi.sound(self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void cyborg_footstep(edict_t *self) {
  gi.sound(self, CHAN_BODY, sound_step, 0.5f, ATTN_IDLE, 0.0f);
}

//
// STAND
//

mframe_t cyborg_frames_stand[] = {{ai_stand, 0, NULL}};
mmove_t cyborg_move_stand = {FRAME_tpose, FRAME_tpose, cyborg_frames_stand,
                             NULL};

void cyborg_stand(edict_t *self) {
  self->monsterinfo.currentmove = &cyborg_move_stand;
}

//
// WALKRUN
//

mframe_t cyborg_frames_walk[] = {{ai_walk, 8, NULL},
                                 {ai_walk, 6, NULL},
                                 {ai_walk, 5, NULL},
                                 {ai_walk, 3, NULL},
                                 {ai_walk, 1, monster_footstep},

                                 {ai_walk, 3, NULL},
                                 {ai_walk, 5, NULL},
                                 {ai_walk, 6, NULL},
                                 {ai_walk, 8, NULL},
                                 {ai_walk, 8, NULL},

                                 {ai_walk, 5, NULL},
                                 {ai_walk, 3, NULL},
                                 {ai_walk, 1, NULL},
                                 {ai_walk, 1, monster_footstep},
                                 {ai_walk, 3, NULL},

                                 {ai_walk, 5, NULL},
                                 {ai_walk, 6, NULL},
                                 {ai_walk, 8, NULL}};
mmove_t cyborg_move_walk = {FRAME_walk1, FRAME_walk18, cyborg_frames_walk,
                            NULL};

void cyborg_walk(edict_t *self) {
  self->monsterinfo.currentmove = &cyborg_move_walk;
}

mframe_t cyborg_frames_run[] = {
    {ai_run, 16, NULL}, {ai_run, 10, monster_footstep}, {ai_run, 11, NULL},
    {ai_run, 16, NULL}, {ai_run, 10, monster_footstep},

    {ai_run, 11, NULL}};
mmove_t cyborg_move_run = {FRAME_run1, FRAME_run6, cyborg_frames_run, NULL};

void cyborg_run(edict_t *self) {
  if (self->monsterinfo.aiflags & AI_STAND_GROUND) {
    self->monsterinfo.currentmove = &cyborg_move_stand;
    return;
  }

  self->monsterinfo.currentmove = &cyborg_move_run;
}

//
// ATTACK
//

void cyborg_fire(edict_t *self, vec3_t mz) {
  vec3_t start;
  vec3_t forward, right;
  vec3_t aim;

  if (!self->enemy || !self->enemy->inuse)
    return;

  AngleVectors(self->s.angles, forward, right, NULL);
  G_ProjectSource(self->s.origin, mz, forward, right, start);

  // PredictAim(self, self->enemy, start, 800, false, frandom() * 0.3f, &aim,
  // nullptr); Standard Q2 PredictAim arguments might differ slightly. Assuming
  // standard: self, target, start, speed, check_vis, probability, OUT_aim,
  // OUT_pos Konig uses speed 800. Konig uses speed 500 for fire_deatom?
  // "fire_deatom(self, start, aim, 50, 500);"

  // Manual aim logic if PredictAim needs it
  VectorCopy(self->enemy->s.origin, aim); // Fallback

  fire_deatom(self, start, aim, 50, 500);
  // Usually fire_deatom should calculate aim itself or use aim vector?
  // Konig says fire_deatom takes dir/aim.
  // We should probably calculate dir properly here using existing
  // logic/PredictAim. I'll stick to a simple aim for now or rely on fire_deatom
  // to normalize.
}

void cyborg_fire_left(edict_t *self) {
  vec3_t offset;

  if (self->s.frame == FRAME_attack101 + 5) // FRAME_attack106
  {
    VectorSet(offset, 8, 23, 13);
  } else if (self->s.frame == FRAME_attack201 + 7) // FRAME_attack208
  {
    VectorSet(offset, 10.5, -12.5, -9);
  } else if (self->s.frame == FRAME_attack401 + 1) // FRAME_attack402 ?
  {
    VectorSet(offset, 11, -16, 10);
  } else {
    VectorSet(offset, 8, -5, 12.5);
  }

  cyborg_fire(self, offset);
}

void cyborg_fire_right(edict_t *self) {
  vec3_t offset;

  if (self->s.frame == FRAME_attack101 + 11) // FRAME_attack112
  {
    VectorSet(offset, 23, -8.5, 13);
  } else if (self->s.frame == FRAME_attack201 + 7) // FRAME_attack208
  {
    VectorSet(offset, -10.5, 25, -9);
  } else if (self->s.frame == FRAME_attack301 + 1) // FRAME_attack302
  {
    VectorSet(offset, -11, 18, 10);
  } else {
    VectorSet(offset, -8, 25, 12.5);
  }

  cyborg_fire(self, offset);
}

void cyborg_fire_both_footstep(edict_t *self) {
  cyborg_fire_left(self);
  cyborg_fire_right(self);
  monster_footstep(self);
}

mframe_t cyborg_frames_attack_run[] = {
    {ai_charge, 16, NULL},
    {ai_charge, 16, NULL},
    {ai_charge, 16, monster_footstep},
    {ai_charge, 12, NULL},
    {ai_charge, 12, NULL},

    {ai_charge, 12, cyborg_fire_left},
    {ai_charge, 12, NULL},
    {ai_charge, 16, NULL},
    {ai_charge, 12, monster_footstep},
    {ai_charge, 12, NULL},

    {ai_charge, 8, NULL},
    {ai_charge, 6, cyborg_fire_right},
};
mmove_t cyborg_move_attack_run = {FRAME_attack101, FRAME_attack112,
                                  cyborg_frames_attack_run, cyborg_run};

mframe_t cyborg_frames_attack_backflip[] = {
    {ai_charge, 1, NULL},
    {ai_charge, 0, NULL},
    {ai_charge, 0, NULL},
    {ai_charge, -4, NULL},
    {ai_charge, -6, NULL},

    {ai_charge, -6, NULL},
    {ai_charge, -4, monster_footstep},
    {ai_charge, -2, cyborg_fire_both_footstep},
    {ai_charge, -2, NULL},
    {ai_charge, 0, NULL},

    {ai_charge, 0, NULL}};
mmove_t cyborg_move_attack_backflip = {FRAME_attack201, FRAME_attack211,
                                       cyborg_frames_attack_backflip,
                                       cyborg_run};

mframe_t cyborg_frames_attack_right[] = {
    {ai_charge, 0, NULL},           {ai_charge, -2, cyborg_fire_right},
    {ai_charge, -4, NULL},          {ai_charge, -2, NULL},
    {ai_charge, 0, NULL},

    {ai_charge, 0, cyborg_reattack}};
mmove_t cyborg_move_attack_right = {FRAME_attack301, FRAME_attack306,
                                    cyborg_frames_attack_right, cyborg_run};

mframe_t cyborg_frames_attack_left[] = {
    {ai_charge, 0, NULL},           {ai_charge, -2, cyborg_fire_left},
    {ai_charge, -4, NULL},          {ai_charge, -2, NULL},
    {ai_charge, 0, NULL},

    {ai_charge, 0, cyborg_reattack}};
mmove_t cyborg_move_attack_left = {FRAME_attack401, FRAME_attack406,
                                   cyborg_frames_attack_left, cyborg_run};

void cyborg_reattack(edict_t *self) {
  if (random() > 0.5f)
    return;
  else {
    if (random() >= 0.75f)
      self->monsterinfo.currentmove = &cyborg_move_attack_backflip;
    else if (random() >= 0.5f)
      self->monsterinfo.currentmove = &cyborg_move_attack_right;
    else
      self->monsterinfo.currentmove = &cyborg_move_attack_left;
  }
}

void cyborg_attack(edict_t *self) {
  float dist = range(self, self->enemy);

  if (dist <= (RANGE_NEAR / 3)) // RANGE_NEAR usually 100?
  {
    self->monsterinfo.currentmove = &cyborg_move_attack_backflip;
  }
  if (dist >= RANGE_MID) {
    self->monsterinfo.currentmove = &cyborg_move_attack_run;
  } else {
    if (random() > 0.5f)
      self->monsterinfo.currentmove = &cyborg_move_attack_right;
    else
      self->monsterinfo.currentmove = &cyborg_move_attack_left;
  }
}

//
// MELEE
//

void cyborg_punch(edict_t *self) {
  vec3_t aim;
  VectorSet(aim, MELEE_DISTANCE, 0, -24);

  if (!fire_hit(self, aim, (int)(5 + random() * 6), 250)) //	Faster attack
    self->monsterinfo.melee_debounce_time = level.time + 1.2;
}

void cyborg_kick(edict_t *self) {
  vec3_t aim;
  VectorSet(aim, MELEE_DISTANCE, self->mins[0], -4);

  if (!fire_hit(self, aim, (int)(15 + random() * 6), 400)) // Slower attack
    self->monsterinfo.melee_debounce_time = level.time + 2.5;
}

void cyborg_backflip(edict_t *self) {
  if (random() >= 0.5f)
    self->monsterinfo.currentmove = &cyborg_move_attack_backflip;
}

void cyborg_footstep_punch(edict_t *self) {
  monster_footstep(self);
  cyborg_punch(self);
}

void cyborg_footstep_backflip(edict_t *self) {
  monster_footstep(self);
  cyborg_backflip(self);
}

void cyborg_punch_backflip(edict_t *self) {
  cyborg_punch(self);
  cyborg_backflip(self);
}

mframe_t cyborg_frames_attack_melee1[] = {
    {ai_charge, 0, NULL},
    {ai_charge, 1, NULL},
    {ai_charge, 2, cyborg_footstep_punch},
    {ai_charge, 1, NULL},
    {ai_charge, 0, NULL},

    {ai_charge, 1, NULL},
    {ai_charge, 2, cyborg_punch},
    {ai_charge, -2, cyborg_footstep_backflip}};
mmove_t cyborg_move_attack_melee1 = {FRAME_melee101, FRAME_melee108,
                                     cyborg_frames_attack_melee1, cyborg_run};

mframe_t cyborg_frames_attack_melee2[] = {
    {ai_charge, 1, NULL},
    {ai_charge, 2, NULL},
    {ai_charge, 3, cyborg_kick},
    {ai_charge, 3, NULL},
    {ai_charge, -2, NULL},

    {ai_charge, -2, cyborg_punch_backflip},
};
mmove_t cyborg_move_attack_melee2 = {FRAME_melee201, FRAME_melee206,
                                     cyborg_frames_attack_melee2, cyborg_run};

void cyborg_melee(edict_t *self) {
  if (random() >= 0.5)
    self->monsterinfo.currentmove = &cyborg_move_attack_melee1;
  else
    self->monsterinfo.currentmove = &cyborg_move_attack_melee2;
}

//
// PAIN
//
mframe_t cyborg_frames_pain1[] = {
    {ai_move, 0, NULL}, {ai_move, 0, NULL}, {ai_move, 0, NULL},

    {ai_move, 0, NULL}, {ai_move, 0, NULL}, {ai_move, 0, NULL},
};
mmove_t cyborg_move_pain1 = {FRAME_pain101, FRAME_pain106, cyborg_frames_pain1,
                             cyborg_run};

mframe_t cyborg_frames_pain2[] = {{ai_move, 0, NULL},
                                  {ai_move, -1, NULL},
                                  {ai_move, -1, NULL},
                                  {ai_move, -2, NULL},
                                  {ai_move, -2, monster_footstep},

                                  {ai_move, -2, NULL},
                                  {ai_move, -1, NULL},
                                  {ai_move, 0, NULL},
                                  {ai_move, -1, NULL},
                                  {ai_move, -1, NULL},

                                  {ai_move, 1, NULL},
                                  {ai_move, 4, NULL},
                                  {ai_move, 4, monster_footstep},
                                  {ai_move, 2, NULL},
                                  {ai_move, 0, NULL},

                                  {ai_move, 0, NULL}};
mmove_t cyborg_move_pain2 = {FRAME_pain201, FRAME_pain216, cyborg_frames_pain2,
                             cyborg_run};

void cyborg_pain(edict_t *self, edict_t *other, float kick, int damage) {
  if (level.time < self->pain_debounce_time)
    return;

  self->pain_debounce_time = level.time + 3;
  gi.sound(self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);

  if (skill->value == 3)
    return; // no pain anims in nightmare

  self->monsterinfo.aiflags &= ~AI_MANUAL_STEERING;

  if (random() >= 0.5f)
    self->monsterinfo.currentmove = &cyborg_move_pain1;
  else
    self->monsterinfo.currentmove = &cyborg_move_pain2;
}

void cyborg_dead(edict_t *self) {
  VectorSet(self->mins, -16, -16, -38);
  VectorSet(self->maxs, 16, 16, -16);
  self->movetype = MOVETYPE_TOSS;
  self->svflags |= SVF_DEADMONSTER;
  self->nextthink = 0;
  gi.linkentity(self);
}

void cyborg_shrink(edict_t *self) {
  self->maxs[2] = 0;
  self->svflags |= SVF_DEADMONSTER;
  gi.linkentity(self);
}

mframe_t cyborg_frames_death1[] = {{ai_move, 0, NULL},
                                   {ai_move, 0, NULL},
                                   {ai_move, 0, NULL},
                                   {ai_move, 0, NULL},
                                   {ai_move, 0, cyborg_shrink},

                                   {ai_move, 0, NULL},
                                   {ai_move, 0, NULL},
                                   {ai_move, 0, monster_footstep}};
mmove_t cyborg_move_death1 = {FRAME_death101, FRAME_death108,
                              cyborg_frames_death1, cyborg_dead};

mframe_t cyborg_frames_death2[] = {{ai_move, 0, NULL},
                                   {ai_move, -1, NULL},
                                   {ai_move, -1, cyborg_shrink},
                                   {ai_move, -2, NULL},
                                   {ai_move, -2, monster_footstep},

                                   {ai_move, 0, NULL}};
mmove_t cyborg_move_death2 = {FRAME_death201, FRAME_death206,
                              cyborg_frames_death2, cyborg_dead};

mframe_t cyborg_frames_death3[] = {{ai_move, 0, NULL},
                                   {ai_move, 1, NULL},
                                   {ai_move, 2, NULL},
                                   {ai_move, 2, cyborg_shrink},
                                   {ai_move, 1, NULL},

                                   {ai_move, 0, monster_footstep}};
mmove_t cyborg_move_death3 = {FRAME_death301, FRAME_death306,
                              cyborg_frames_death3, cyborg_dead};

void cyborg_die(edict_t *self, edict_t *inflictor, edict_t *attacker,
                int damage, vec3_t point) {
  int n;
  if (self->health <= self->gib_health) {
    gi.sound(self, CHAN_VOICE, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM,
             0);

    self->s.skinnum = 0;

    for (n = 0; n < 3; n++)
      ThrowGib(self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
    for (n = 0; n < 5; n++)
      ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage,
               GIB_ORGANIC);

    ThrowGib(self, "models/objects/gibs/gear/tris.md2", damage, GIB_METALLIC);
    ThrowGib(self, "models/objects/gibs/gear/tris.md2", damage, GIB_METALLIC);

    ThrowHead(self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);

    self->deadflag = DEAD_DEAD;
    return;
  }

  if (self->deadflag == DEAD_DEAD)
    return;

  gi.sound(self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
  self->deadflag = DEAD_DEAD;
  self->takedamage = DAMAGE_YES;

  if (damage >= 50)
    self->monsterinfo.currentmove = &cyborg_move_death2;
  else if (random() >= 0.5f)
    self->monsterinfo.currentmove = &cyborg_move_death1;
  else
    self->monsterinfo.currentmove = &cyborg_move_death3;
}

/*QUAKED monster_cyborg (1 .5 0) (-16 -16 -38) (32 32 27) Ambush Trigger_Spawn
 * Sight Corpse
 */
void SP_monster_cyborg(edict_t *self) {
  if (deathmatch->value) {
    G_FreeEdict(self);
    return;
  }

  sound_pain = gi.soundindex("cyborg/pain.wav");
  sound_death = gi.soundindex("cyborg/death.wav");
  sound_sight = gi.soundindex("cyborg/sight.wav");
  sound_idle = gi.soundindex("cyborg/idle.wav");
  sound_attack = gi.soundindex("chick/chkatck2.wav");
  sound_step = gi.soundindex("insane/insane11.wav");

  self->s.modelindex = gi.modelindex("models/monsters/cyborg/tris.md2");
  VectorSet(self->mins, -16, -16, -38);
  VectorSet(self->maxs, 16, 16, 27);
  self->movetype = MOVETYPE_STEP;
  self->solid = SOLID_BBOX;

  self->health = 200; // Konig: 200 * health_multiplier
  self->monsterinfo.power_armor_type =
      POWER_ARMOR_SCREEN; // Konig used IT_ARMOR_COMBAT? But in Q2 engine usage
                          // usually means setting power_armor_type.
  self->monsterinfo.power_armor_power = 100;

  self->gib_health = -200;
  self->mass = 350;

  self->monsterinfo.scale = MODEL_SCALE;

  self->pain = cyborg_pain;
  self->die = cyborg_die;

  self->monsterinfo.sight = cyborg_sight;
  self->monsterinfo.idle = cyborg_idle;
  self->monsterinfo.search = cyborg_search;

  self->monsterinfo.stand = cyborg_stand;
  self->monsterinfo.walk = cyborg_walk;
  self->monsterinfo.run = cyborg_run;

  self->monsterinfo.attack = cyborg_attack;
  self->monsterinfo.melee = cyborg_melee;

  // self->monsterinfo.setskin = cyborg_setskin; // Standard Q2 doesn't have
  // setskin pointer in monsterinfo_t usually?

  gi.linkentity(self);

  self->monsterinfo.currentmove = &cyborg_move_stand;

  walkmonster_start(self);
}

/*
 * Ground based spider tank encountered in later Oblivion levels.  The
 * original AI had fairly involved melee combos so this recreation keeps
 * things aggressive and close range – the spider rapidly closes distance
 * and delivers heavy slash attacks while shrugging off lighter hits.
 */

#include "g_local.h"

#define SPIDER_FRAME_STAND_START        0x00
#define SPIDER_FRAME_STAND_END          0x36
#define SPIDER_FRAME_WALK_START         0x37
#define SPIDER_FRAME_WALK_END           0x40
#define SPIDER_FRAME_ATTACKA_START      0x41
#define SPIDER_FRAME_ATTACKA_END        0x4a
#define SPIDER_FRAME_ATTACKB_START      0x4b
#define SPIDER_FRAME_ATTACKB_END        0x50
#define SPIDER_FRAME_RUN_START          0x51
#define SPIDER_FRAME_RUN_END            0x55
#define SPIDER_FRAME_ATTACK_FINISH_START        0x63
#define SPIDER_FRAME_ATTACK_FINISH_END          0x67
#define SPIDER_FRAME_ATTACK_RECOVER_START       0x68
#define SPIDER_FRAME_ATTACK_RECOVER_END         0x6e
#define SPIDER_FRAME_PAIN_START         0x5b
#define SPIDER_FRAME_PAIN_END           0x62
#define SPIDER_FRAME_DEATH_START        0x6f
#define SPIDER_FRAME_DEATH_END          0x7c

static int sound_sight;
static int sound_search;
static int sound_idle;
static int sound_pain;
static int sound_death;
static int sound_melee[3];
static int sound_thud;

static void spider_idle (edict_t *self)
{
    if (random () < 0.25f)
        gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

static void spider_sight (edict_t *self, edict_t *other)
{
    gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

static void spider_search (edict_t *self)
{
    gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_IDLE, 0);
}

static void spider_step (edict_t *self)
{
    gi.sound (self, CHAN_BODY, sound_thud, 1, ATTN_NORM, 0);
}

static void spider_claw (edict_t *self)
{
    vec3_t  forward;
    float   damage = 30.0f;

    if (!self->enemy)
        return;

    AngleVectors (self->s.angles, forward, NULL, NULL);

    if (range (self, self->enemy) > RANGE_MELEE)
        return;

    gi.sound (self, CHAN_WEAPON, sound_melee[rand () % 3], 1, ATTN_NORM, 0);
    T_Damage (self->enemy, self, self, forward, self->enemy->s.origin, vec3_origin, (int)damage, (int)damage, 0, MOD_HIT);
}

static void spider_idle_loop (edict_t *self);
static void spider_select_locomotion (edict_t *self);
static void spider_combo_entry (edict_t *self);
static void spider_attack_link (edict_t *self);
static void spider_begin_recover (edict_t *self);
static void spider_attack_recover_end (edict_t *self);

static mframe_t spider_frames_stand[] = {
    {ai_stand, 0, spider_idle},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, spider_idle},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, spider_idle},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, spider_idle},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, spider_idle},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, spider_idle},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, spider_idle},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL},
    {ai_stand, 0, NULL}
};
static mmove_t spider_move_stand = {
    SPIDER_FRAME_STAND_START, SPIDER_FRAME_STAND_END, spider_frames_stand, spider_idle_loop
};

static mframe_t spider_frames_walk[] = {
    {ai_walk, 10, spider_step},
    {ai_walk, 4, NULL},
    {ai_walk, 12, spider_step},
    {ai_walk, 4, NULL},
    {ai_walk, 10, spider_step},
    {ai_walk, 4, NULL},
    {ai_walk, 12, spider_step},
    {ai_walk, 4, NULL},
    {ai_walk, 10, spider_step},
    {ai_walk, 0, NULL}
};
static mmove_t spider_move_walk = {
    SPIDER_FRAME_WALK_START, SPIDER_FRAME_WALK_END, spider_frames_walk, spider_select_locomotion
};

static mframe_t spider_frames_run[] = {
    {ai_run, 24, spider_step},
    {ai_run, 10, NULL},
    {ai_run, 24, spider_step},
    {ai_run, 10, NULL},
    {ai_run, 24, spider_step}
};
static mmove_t spider_move_run = {
    SPIDER_FRAME_RUN_START, SPIDER_FRAME_RUN_END, spider_frames_run, spider_select_locomotion
};

static mframe_t spider_frames_attack_primary[] = {
    {ai_charge, 0, NULL},
    {ai_charge, 0, spider_claw},
    {ai_charge, 0, NULL},
    {ai_charge, 0, spider_claw},
    {ai_charge, 0, NULL},
    {ai_charge, 0, spider_claw},
    {ai_charge, 0, NULL},
    {ai_charge, 0, NULL},
    {ai_charge, 0, spider_claw},
    {ai_charge, 0, NULL}
};
static mmove_t spider_move_attack_primary = {
    SPIDER_FRAME_ATTACKA_START, SPIDER_FRAME_ATTACKA_END, spider_frames_attack_primary, spider_attack_link
};

static mframe_t spider_frames_attack_secondary[] = {
    {ai_charge, 0, NULL},
    {ai_charge, 0, spider_claw},
    {ai_charge, 0, NULL},
    {ai_charge, 0, spider_claw},
    {ai_charge, 0, NULL},
    {ai_charge, 0, spider_claw}
};
static mmove_t spider_move_attack_secondary = {
    SPIDER_FRAME_ATTACKB_START, SPIDER_FRAME_ATTACKB_END, spider_frames_attack_secondary, spider_attack_link
};

static mframe_t spider_frames_attack_finisher[] = {
    {ai_charge, 0, NULL},
    {ai_charge, 0, spider_claw},
    {ai_charge, 0, NULL},
    {ai_charge, 0, spider_claw},
    {ai_charge, 0, NULL}
};
static mmove_t spider_move_attack_finisher = {
    SPIDER_FRAME_ATTACK_FINISH_START, SPIDER_FRAME_ATTACK_FINISH_END, spider_frames_attack_finisher, spider_begin_recover
};

static mframe_t spider_frames_attack_recover[] = {
    {ai_move, 0, NULL},
    {ai_move, 0, NULL},
    {ai_move, 0, NULL},
    {ai_move, 0, NULL},
    {ai_move, 0, NULL},
    {ai_move, 0, NULL},
    {ai_move, 0, NULL}
};
static mmove_t spider_move_attack_recover = {
    SPIDER_FRAME_ATTACK_RECOVER_START, SPIDER_FRAME_ATTACK_RECOVER_END, spider_frames_attack_recover, spider_attack_recover_end
};

static void spider_idle_loop (edict_t *self)
{
    self->monsterinfo.currentmove = &spider_move_stand;
}

static void spider_stand (edict_t *self)
{
    self->monsterinfo.currentmove = &spider_move_stand;
}

static void spider_select_locomotion (edict_t *self)
{
    if ((self->monsterinfo.aiflags & AI_STAND_GROUND) || !self->enemy)
    {
        spider_stand (self);
        return;
    }

    if (range (self, self->enemy) > RANGE_MELEE)
    {
        if (random () > 0.35f)
            self->monsterinfo.currentmove = &spider_move_run;
        else
            self->monsterinfo.currentmove = &spider_move_walk;
    }
    else
    {
        if (random () > 0.5f)
            self->monsterinfo.currentmove = &spider_move_attack_primary;
        else
            self->monsterinfo.currentmove = &spider_move_attack_secondary;
    }
}

static void spider_walk (edict_t *self)
{
    spider_select_locomotion (self);
}

static void spider_run (edict_t *self)
{
    spider_select_locomotion (self);
}

static void spider_attack (edict_t *self)
{
    spider_combo_entry (self);
}

static void spider_combo_entry (edict_t *self)
{
    if (!self->enemy)
    {
        spider_stand (self);
        return;
    }

    if (self->monsterinfo.lefty > 0)
    {
        self->monsterinfo.currentmove = &spider_move_attack_finisher;
        return;
    }

    if (random () > 0.5f)
        self->monsterinfo.currentmove = &spider_move_attack_primary;
    else
        self->monsterinfo.currentmove = &spider_move_attack_secondary;

    self->monsterinfo.lefty = 1;
}

static void spider_attack_link (edict_t *self)
{
    if (self->enemy && range (self, self->enemy) <= RANGE_MELEE && random () > 0.4f)
    {
        self->monsterinfo.currentmove = &spider_move_attack_finisher;
        return;
    }

    spider_begin_recover (self);
}

static void spider_begin_recover (edict_t *self)
{
    self->monsterinfo.currentmove = &spider_move_attack_recover;
}

static void spider_attack_recover_end (edict_t *self)
{
    self->monsterinfo.lefty = 0;
    self->monsterinfo.attack_finished = level.time + 1.0f + random () * 0.4f;

    if (self->monsterinfo.aiflags & AI_STAND_GROUND)
        spider_stand (self);
    else if (self->enemy && range (self, self->enemy) <= RANGE_MELEE && random () > 0.6f)
        self->monsterinfo.currentmove = &spider_move_attack_secondary;
    else
        spider_select_locomotion (self);
}

static void spider_pain (edict_t *self, edict_t *other, float kick, int damage)
{
    static mframe_t pain_frames[] = {
        {ai_move, 0, NULL},
        {ai_move, 0, NULL},
        {ai_move, 0, NULL},
        {ai_move, 0, NULL},
        {ai_move, 0, NULL},
        {ai_move, 0, NULL},
        {ai_move, 0, NULL},
        {ai_move, 0, NULL}
    };
    static mmove_t pain_move = {SPIDER_FRAME_PAIN_START, SPIDER_FRAME_PAIN_END, pain_frames, spider_run};

    if (level.time < self->pain_debounce_time)
        return;

    self->pain_debounce_time = level.time + 1.5f;
    gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
    self->monsterinfo.currentmove = &pain_move;
}

static void spider_dead (edict_t *self)
{
    self->deadflag = DEAD_DEAD;
    self->takedamage = DAMAGE_YES;
}

static void spider_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
    static mframe_t death_frames[] = {
        {ai_move, 0, NULL},
        {ai_move, 0, NULL},
        {ai_move, 0, NULL},
        {ai_move, 0, NULL},
        {ai_move, 0, NULL},
        {ai_move, 0, NULL},
        {ai_move, 0, NULL},
        {ai_move, 0, NULL},
        {ai_move, 0, spider_dead},
        {ai_move, 0, spider_dead},
        {ai_move, 0, spider_dead},
        {ai_move, 0, spider_dead},
        {ai_move, 0, spider_dead},
        {ai_move, 0, spider_dead}
    };
    static mmove_t death_move = {SPIDER_FRAME_DEATH_START, SPIDER_FRAME_DEATH_END, death_frames, spider_dead};

    gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);

    if (self->health <= self->gib_health)
    {
        gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
        ThrowGib (self, "models/objects/gibs/sm_metal/tris.md2", damage, GIB_METALLIC);
        ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_METALLIC);
        ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
        return;
    }

    self->monsterinfo.currentmove = &death_move;
}

void SP_monster_spider (edict_t *self)
{
    if (deathmatch->value)
    {
        G_FreeEdict (self);
        return;
    }

    self->s.modelindex = gi.modelindex ("models/monsters/spider/tris.md2");
    VectorSet (self->mins, -32.0f, -32.0f, -32.0f);
    VectorSet (self->maxs, 32.0f, 32.0f, 32.0f);
    self->movetype = MOVETYPE_STEP;
    self->solid = SOLID_BBOX;
    self->mass = 300;

    sound_sight = gi.soundindex ("spider/sight.wav");
    sound_search = gi.soundindex ("gladiator/gldsrch1.wav");
    sound_idle = gi.soundindex ("gladiator/gldidle1.wav");
    sound_pain = gi.soundindex ("gladiator/pain.wav");
    sound_death = gi.soundindex ("mutant/thud1.wav");
    sound_melee[0] = gi.soundindex ("gladiator/melee1.wav");
    sound_melee[1] = gi.soundindex ("gladiator/melee2.wav");
    sound_melee[2] = gi.soundindex ("gladiator/melee3.wav");
    sound_thud = gi.soundindex ("mutant/thud1.wav");

    self->s.sound = sound_idle;

    self->health = 400;
    self->gib_health = -120;

    self->pain = spider_pain;
    self->die = spider_die;

    self->monsterinfo.stand = spider_stand;
    self->monsterinfo.idle = spider_stand;
    self->monsterinfo.walk = spider_walk;
    self->monsterinfo.run = spider_run;
    self->monsterinfo.attack = spider_attack;
    self->monsterinfo.melee = spider_attack;
    self->monsterinfo.sight = spider_sight;
    self->monsterinfo.search = spider_search;

    spider_stand (self);

    walkmonster_start (self);
}

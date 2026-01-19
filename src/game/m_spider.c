/*
==============================================================================

SPIDER

==============================================================================
*/

#include "m_spider.h"
#include "g_local.h"
#include "g_oblivion_monster.h"

#define MODEL_SCALE 1.0f

#define SPIDER_FRAME_STAND_START 0
#define SPIDER_FRAME_STAND_END 54
#define SPIDER_FRAME_WALK_START 55
#define SPIDER_FRAME_WALK_END 64
#define SPIDER_FRAME_RUN1_START 65
#define SPIDER_FRAME_RUN1_END 74
#define SPIDER_FRAME_RUN2_START 75
#define SPIDER_FRAME_RUN2_END 80
#define SPIDER_FRAME_ATTACK_LEFT_START 81
#define SPIDER_FRAME_ATTACK_LEFT_END 85
#define SPIDER_FRAME_ATTACK_RIGHT_START 86
#define SPIDER_FRAME_ATTACK_RIGHT_END 90
#define SPIDER_FRAME_ATTACK_DUAL_START 91
#define SPIDER_FRAME_ATTACK_DUAL_END 98
#define SPIDER_FRAME_MELEE_PRIMARY_START 99
#define SPIDER_FRAME_MELEE_PRIMARY_END 103
#define SPIDER_FRAME_MELEE_SECONDARY_START 104
#define SPIDER_FRAME_MELEE_SECONDARY_END 110
#define SPIDER_FRAME_PAIN1_START 111
#define SPIDER_FRAME_PAIN1_END 116
#define SPIDER_FRAME_PAIN2_START 117
#define SPIDER_FRAME_PAIN2_END 124
#define SPIDER_FRAME_DEATH1_START 125
#define SPIDER_FRAME_DEATH1_END 144
#define SPIDER_FRAME_DEATH2_START 145
#define SPIDER_FRAME_DEATH2_END 164

#define SPIDER_ROCKET_DAMAGE 50
#define SPIDER_ROCKET_SPEED 500
#define SPIDER_MELEE_DAMAGE_MIN 20
#define SPIDER_MELEE_DAMAGE_MAX 24
#define SPIDER_MELEE_KICK 300
#define SPIDER_CHARGE_DAMAGE_MIN 40
#define SPIDER_CHARGE_DAMAGE_MAX 49
#define SPIDER_CHARGE_SPEED 500
#define SPIDER_CHARGE_RANGE 400.0f
#define SPIDER_CHARGE_YAWSPEED 250.0f

#define SPIDER_MZ_LEFT 0x8a
#define SPIDER_MZ_RIGHT 0x8b

static int sound_step;
static int sound_pain1;
static int sound_pain2;
static int sound_sight;
static int sound_search;
static int sound_idle;
static int sound_melee1;
static int sound_melee2;
static int sound_melee3;

static void spider_idle(edict_t *self);
static void spider_search(edict_t *self);
static void spider_sight(edict_t *self, edict_t *other);
static void spider_step(edict_t *self);
static void spider_charge_think(edict_t *self);
static void spider_charge_start(edict_t *self);
static void spider_charge_end(edict_t *self);
static void spider_melee_swing(edict_t *self);
static void spider_melee_hit(edict_t *self);
static void spider_rocket_left(edict_t *self);
static void spider_rocket_right(edict_t *self);
static void spider_dead(edict_t *self);
static void spider_stand(edict_t *self);
static void spider_walk(edict_t *self);
static void spider_run(edict_t *self);
static void spider_attack(edict_t *self);
static void spider_melee(edict_t *self);
static void spider_pain(edict_t *self, edict_t *other, float kick, int damage);
static void spider_die(edict_t *self, edict_t *inflictor, edict_t *attacker,
	int damage, vec3_t point);

static mframe_t spider_frames_stand[] = {
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, spider_step},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, spider_step},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},

	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, spider_step},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, spider_step},

	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, spider_step},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, spider_step},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},

	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL},

	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}, {ai_stand, 0.0f, NULL},
	{ai_stand, 0.0f, NULL}
};

static mmove_t spider_move_stand = {
	SPIDER_FRAME_STAND_START,
	SPIDER_FRAME_STAND_END,
	spider_frames_stand,
	NULL
};

static mframe_t spider_frames_walk[] = {
	{ai_walk, 2.0f, spider_step}, {ai_walk, 5.0f, NULL},
	{ai_walk, 12.0f, spider_step}, {ai_walk, 16.0f, NULL},
	{ai_walk, 5.0f, NULL}, {ai_walk, 8.0f, spider_step},
	{ai_walk, 8.0f, NULL}, {ai_walk, 12.0f, NULL},
	{ai_walk, 9.0f, spider_step}, {ai_walk, 5.0f, NULL}
};

static mmove_t spider_move_walk = {
	SPIDER_FRAME_WALK_START,
	SPIDER_FRAME_WALK_END,
	spider_frames_walk,
	NULL
};

static mframe_t spider_frames_run1[] = {
	{ai_run, 0.0f, NULL}, {ai_run, 0.0f, NULL},
	{ai_run, 0.0f, spider_charge_start}, {ai_run, 0.0f, NULL},
	{ai_run, 0.0f, NULL}, {ai_run, 0.0f, NULL},
	{ai_run, 0.0f, NULL}, {ai_run, 0.0f, spider_charge_end},
	{ai_run, 0.0f, NULL}, {ai_run, 0.0f, NULL}
};

static mmove_t spider_move_run1 = {
	SPIDER_FRAME_RUN1_START,
	SPIDER_FRAME_RUN1_END,
	spider_frames_run1,
	spider_run
};

static mframe_t spider_frames_run2[] = {
	{ai_run, 16.0f, NULL}, {ai_run, 16.0f, NULL},
	{ai_run, 16.0f, NULL}, {ai_run, 16.0f, NULL},
	{ai_run, 16.0f, NULL}, {ai_run, 16.0f, NULL}
};

static mmove_t spider_move_run2 = {
	SPIDER_FRAME_RUN2_START,
	SPIDER_FRAME_RUN2_END,
	spider_frames_run2,
	NULL
};

static mframe_t spider_frames_attack_left[] = {
	{ai_charge, 0.0f, NULL}, {ai_charge, 0.0f, NULL},
	{ai_charge, 0.0f, spider_rocket_left}, {ai_charge, 0.0f, NULL},
	{ai_charge, 0.0f, NULL}
};

static mmove_t spider_move_attack_left = {
	SPIDER_FRAME_ATTACK_LEFT_START,
	SPIDER_FRAME_ATTACK_LEFT_END,
	spider_frames_attack_left,
	spider_run
};

static mframe_t spider_frames_attack_right[] = {
	{ai_charge, 0.0f, NULL}, {ai_charge, 0.0f, NULL},
	{ai_charge, 0.0f, spider_rocket_right}, {ai_charge, 0.0f, NULL},
	{ai_charge, 0.0f, NULL}
};

static mmove_t spider_move_attack_right = {
	SPIDER_FRAME_ATTACK_RIGHT_START,
	SPIDER_FRAME_ATTACK_RIGHT_END,
	spider_frames_attack_right,
	spider_run
};

static mframe_t spider_frames_attack_dual[] = {
	{ai_charge, 0.0f, NULL}, {ai_charge, 0.0f, spider_rocket_left},
	{ai_charge, 0.0f, NULL}, {ai_charge, 0.0f, NULL},
	{ai_charge, 0.0f, NULL}, {ai_charge, 0.0f, spider_rocket_right},
	{ai_charge, 0.0f, NULL}, {ai_charge, 0.0f, NULL}
};

static mmove_t spider_move_attack_dual = {
	SPIDER_FRAME_ATTACK_DUAL_START,
	SPIDER_FRAME_ATTACK_DUAL_END,
	spider_frames_attack_dual,
	spider_run
};

static mframe_t spider_frames_melee_primary[] = {
	{ai_charge, 0.0f, NULL}, {ai_charge, 0.0f, NULL},
	{ai_charge, 0.0f, NULL}, {ai_charge, 0.0f, spider_melee_swing},
	{ai_charge, 0.0f, spider_melee_hit}
};

static mmove_t spider_move_melee_primary = {
	SPIDER_FRAME_MELEE_PRIMARY_START,
	SPIDER_FRAME_MELEE_PRIMARY_END,
	spider_frames_melee_primary,
	spider_run
};

static mframe_t spider_frames_melee_secondary[] = {
	{ai_charge, 0.0f, NULL}, {ai_charge, 0.0f, NULL},
	{ai_charge, 0.0f, NULL}, {ai_charge, 0.0f, NULL},
	{ai_charge, 0.0f, spider_melee_swing},
	{ai_charge, 0.0f, spider_melee_hit},
	{ai_charge, 0.0f, spider_melee_hit}
};

static mmove_t spider_move_melee_secondary = {
	SPIDER_FRAME_MELEE_SECONDARY_START,
	SPIDER_FRAME_MELEE_SECONDARY_END,
	spider_frames_melee_secondary,
	spider_run
};

static mframe_t spider_frames_pain1[] = {
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL}
};

static mmove_t spider_move_pain1 = {
	SPIDER_FRAME_PAIN1_START,
	SPIDER_FRAME_PAIN1_END,
	spider_frames_pain1,
	spider_run
};

static mframe_t spider_frames_pain2[] = {
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL}
};

static mmove_t spider_move_pain2 = {
	SPIDER_FRAME_PAIN2_START,
	SPIDER_FRAME_PAIN2_END,
	spider_frames_pain2,
	spider_run
};

static mframe_t spider_frames_death1[] = {
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL}
};

static mmove_t spider_move_death1 = {
	SPIDER_FRAME_DEATH1_START,
	SPIDER_FRAME_DEATH1_END,
	spider_frames_death1,
	spider_dead
};

static mframe_t spider_frames_death2[] = {
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL},
	{ai_move, 0.0f, NULL}, {ai_move, 0.0f, NULL}
};

static mmove_t spider_move_death2 = {
	SPIDER_FRAME_DEATH2_START,
	SPIDER_FRAME_DEATH2_END,
	spider_frames_death2,
	spider_dead
};

/*
=============
spider_idle
=============
*/
static void spider_idle(edict_t *self)
{
	gi.sound(self, CHAN_VOICE, sound_idle, 1.0f, ATTN_IDLE, 0.0f);
}

/*
=============
spider_search
=============
*/
static void spider_search(edict_t *self)
{
	gi.sound(self, CHAN_VOICE, sound_search, 1.0f, ATTN_NORM, 0.0f);
}

/*
=============
spider_sight
=============
*/
static void spider_sight(edict_t *self, edict_t *other)
{
	gi.sound(self, CHAN_VOICE, sound_sight, 1.0f, ATTN_NORM, 0.0f);
}

/*
=============
spider_step
=============
*/
static void spider_step(edict_t *self)
{
	gi.sound(self, CHAN_BODY, sound_step, 1.0f, ATTN_IDLE, 0.0f);
}

/*
=============
spider_charge_think
=============
*/
static void spider_charge_think(edict_t *self)
{
	vec3_t forward;
	vec3_t point;
	int damage;

	if (self->health <= 0)
	{
		self->think = NULL;
		return;
	}

	if (self->enemy && self->enemy->takedamage)
	{
		if (VectorLength(self->velocity) > SPIDER_CHARGE_RANGE)
		{
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorMA(self->s.origin, self->maxs[0], forward, point);
			damage = SPIDER_CHARGE_DAMAGE_MIN +
				(int)(random() * (SPIDER_CHARGE_DAMAGE_MAX - SPIDER_CHARGE_DAMAGE_MIN + 1));
			T_Damage(self->enemy, self, self, forward, point, forward, damage,
				damage, 0, MOD_UNKNOWN);
		}
	}

	if (self->groundentity)
	{
		self->monsterinfo.nextframe = SPIDER_FRAME_RUN1_START + 3;
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		self->think = NULL;
		return;
	}

	if (level.time >= self->monsterinfo.attack_finished)
	{
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		self->think = NULL;
		return;
	}

	self->nextthink = level.time + FRAMETIME;
}

/*
=============
spider_charge_start
=============
*/
static void spider_charge_start(edict_t *self)
{
	vec3_t forward;

	gi.sound(self, CHAN_VOICE, sound_sight, 1.0f, ATTN_NORM, 0.0f);

	AngleVectors(self->s.angles, forward, NULL, NULL);
	self->s.angles[0] += 1.0f;
	VectorScale(forward, SPIDER_CHARGE_SPEED, self->velocity);

	self->monsterinfo.aiflags |= AI_HOLD_FRAME;
	self->yaw_speed = SPIDER_CHARGE_YAWSPEED;
	self->groundentity = NULL;
	self->monsterinfo.attack_finished = level.time + 3.0f;

	self->think = spider_charge_think;
	self->nextthink = level.time + FRAMETIME;
}

/*
=============
spider_charge_end
=============
*/
static void spider_charge_end(edict_t *self)
{
	if (!self->groundentity)
		return;

	gi.sound(self, CHAN_WEAPON, sound_step, 1.0f, ATTN_NORM, 0.0f);
	self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	self->monsterinfo.attack_finished = 0.0f;
}

/*
=============
spider_melee_swing
=============
*/
static void spider_melee_swing(edict_t *self)
{
	gi.sound(self, CHAN_WEAPON, sound_melee1, 1.0f, ATTN_NORM, 0.0f);
}

/*
=============
spider_melee_hit
=============
*/
static void spider_melee_hit(edict_t *self)
{
	vec3_t aim;
	int damage;
	qboolean hit;

	if (!self->enemy)
		return;

	VectorSet(aim, MELEE_DISTANCE, self->mins[0], -4.0f);
	damage = SPIDER_MELEE_DAMAGE_MIN +
		(int)(random() * (SPIDER_MELEE_DAMAGE_MAX - SPIDER_MELEE_DAMAGE_MIN + 1));
	hit = fire_hit(self, aim, damage, SPIDER_MELEE_KICK);
	if (hit)
		gi.sound(self, CHAN_WEAPON, sound_melee2, 1.0f, ATTN_NORM, 0.0f);
	else
		gi.sound(self, CHAN_WEAPON, sound_melee3, 1.0f, ATTN_NORM, 0.0f);
}

/*
=============
spider_rocket
=============
*/
static void spider_rocket(edict_t *self, vec3_t offset, int flashtype)
{
	vec3_t forward;
	vec3_t right;
	vec3_t start;
	vec3_t dir;

	if (!self->enemy || !self->enemy->inuse)
		return;

	AngleVectors(self->s.angles, forward, right, NULL);
	G_ProjectSource(self->s.origin, offset, forward, right, start);

	VectorSubtract(self->pos1, start, dir);
	VectorNormalize(dir);

	monster_fire_rocket(self, start, dir, SPIDER_ROCKET_DAMAGE,
		SPIDER_ROCKET_SPEED, flashtype);
}

/*
=============
spider_rocket_left
=============
*/
static void spider_rocket_left(edict_t *self)
{
	spider_rocket(self, tv(64.0f, -22.0f, 2.0f), SPIDER_MZ_LEFT);
}

/*
=============
spider_rocket_right
=============
*/
static void spider_rocket_right(edict_t *self)
{
	spider_rocket(self, tv(58.0f, 20.0f, 2.0f), SPIDER_MZ_RIGHT);
}

/*
=============
spider_dead
=============
*/
static void spider_dead(edict_t *self)
{
	VectorSet(self->mins, -32.0f, -32.0f, -30.0f);
	VectorSet(self->maxs, 32.0f, 32.0f, 0.0f);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0.0f;
	gi.linkentity(self);
}

/*
=============
spider_stand
=============
*/
static void spider_stand(edict_t *self)
{
	self->monsterinfo.currentmove = &spider_move_stand;
}

/*
=============
spider_walk
=============
*/
static void spider_walk(edict_t *self)
{
	self->monsterinfo.currentmove = &spider_move_walk;
}

/*
=============
spider_run
=============
*/
static void spider_run(edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		spider_stand(self);
		return;
	}

	if (random() < 0.2f)
		self->monsterinfo.currentmove = &spider_move_run1;
	else
		self->monsterinfo.currentmove = &spider_move_run2;
}

/*
=============
spider_attack
=============
*/
static void spider_attack(edict_t *self)
{
	float r;

	if (!self->enemy || !self->enemy->inuse)
		return;

	VectorCopy(self->enemy->s.origin, self->pos1);
	self->pos1[2] += self->enemy->viewheight;

	r = random();
	if (r < 0.33f)
		self->monsterinfo.currentmove = &spider_move_attack_left;
	else if (r < 0.66f)
		self->monsterinfo.currentmove = &spider_move_attack_right;
	else
		self->monsterinfo.currentmove = &spider_move_attack_dual;
}

/*
=============
spider_melee
=============
*/
static void spider_melee(edict_t *self)
{
	if (!self->enemy)
		return;

	gi.bprintf(PRINT_MEDIUM, "spider_melee\n");

	if (random() < 0.5f)
		self->monsterinfo.currentmove = &spider_move_melee_primary;
	else
		self->monsterinfo.currentmove = &spider_move_melee_secondary;
}

/*
=============
spider_pain
=============
*/
static void spider_pain(edict_t *self, edict_t *other, float kick, int damage)
{
	int sound_id;

	if (level.time < self->pain_debounce_time)
		return;

	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	self->pain_debounce_time = level.time + 3.0f;

	sound_id = (random() < 0.5f) ? sound_pain1 : sound_pain2;

	if (skill->value == 3 && random() < 0.1f)
	{
		gi.sound(self, CHAN_VOICE, sound_id, 1.0f, ATTN_NORM, 0.0f);
		self->monsterinfo.currentmove = &spider_move_pain1;
		return;
	}

	if (damage < 10 && random() < 0.2f)
	{
		gi.sound(self, CHAN_VOICE, sound_id, 1.0f, ATTN_NORM, 0.0f);
		self->monsterinfo.currentmove = &spider_move_pain1;
		return;
	}

	gi.sound(self, CHAN_VOICE, sound_id, 1.0f, ATTN_NORM, 0.0f);

	if (damage < 50 && random() < 0.5f)
	{
		if (random() < 0.5f)
		{
			self->monsterinfo.currentmove = &spider_move_pain1;
			return;
		}
	}

	self->monsterinfo.currentmove = &spider_move_pain2;
}

/*
=============
spider_die
=============
*/
static void spider_die(edict_t *self, edict_t *inflictor, edict_t *attacker,
	int damage, vec3_t point)
{
	if (self->health <= self->gib_health)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("misc/udeath.wav"),
			1.0f, ATTN_NORM, 0.0f);
		ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2",
			damage, GIB_ORGANIC);
		ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2",
			damage, GIB_ORGANIC);
		ThrowGib(self, "models/objects/gibs/sm_metal/tris.md2",
			damage, GIB_METALLIC);
		ThrowGib(self, "models/objects/gibs/sm_metal/tris.md2",
			damage, GIB_METALLIC);
		ThrowGib(self, "models/objects/gibs/sm_metal/tris.md2",
			damage, GIB_METALLIC);
		ThrowGib(self, "models/objects/gibs/sm_metal/tris.md2",
			damage, GIB_METALLIC);
		ThrowGib(self, "models/objects/gibs/chest/tris.md2",
			damage, GIB_ORGANIC);
		ThrowHead(self, "models/objects/gibs/head2/tris.md2",
			damage, GIB_ORGANIC);
		return;
	}

	if (self->deadflag)
		return;

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	if (random() < 0.5f)
		self->monsterinfo.currentmove = &spider_move_death1;
	else
		self->monsterinfo.currentmove = &spider_move_death2;
}

/*
=============
SP_monster_spider
=============
*/
void SP_monster_spider(edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict(self);
		return;
	}

	sound_melee1 = gi.soundindex("gladiator/melee1.wav");
	sound_melee2 = gi.soundindex("gladiator/melee2.wav");
	sound_melee3 = gi.soundindex("gladiator/melee3.wav");
	sound_step = gi.soundindex("mutant/thud1.wav");
	sound_pain1 = gi.soundindex("gladiator/pain.wav");
	sound_pain2 = gi.soundindex("gladiator/gldpain2.wav");
	sound_idle = gi.soundindex("gladiator/gldidle1.wav");
	sound_search = gi.soundindex("gladiator/gldsrch1.wav");
	sound_sight = gi.soundindex("spider/sight.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/spider/tris.md2");
	VectorSet(self->mins, -32.0f, -32.0f, -35.0f);
	VectorSet(self->maxs, 32.0f, 32.0f, 32.0f);

	self->health = 400;
	self->max_health = 400;
	self->gib_health = -175;
	self->mass = 300;

	self->pain = spider_pain;
	self->die = spider_die;

	self->monsterinfo.stand = spider_stand;
	self->monsterinfo.idle = spider_idle;
	self->monsterinfo.search = spider_search;
	self->monsterinfo.walk = spider_walk;
	self->monsterinfo.run = spider_run;
	self->monsterinfo.attack = spider_attack;
	self->monsterinfo.melee = spider_melee;
	self->monsterinfo.sight = spider_sight;

	if (self->spawnflags & 0x8)
	{
		self->health = -1;
		self->takedamage = DAMAGE_NO;
		self->deadflag = DEAD_DEAD;
		self->svflags |= SVF_DEADMONSTER;
		self->movetype = MOVETYPE_TOSS;
		self->solid = SOLID_BBOX;
		VectorSet(self->mins, -32.0f, -32.0f, -30.0f);
		VectorSet(self->maxs, 32.0f, 32.0f, 0.0f);
		self->nextthink = 0.0f;
		gi.linkentity(self);
		return;
	}

	gi.linkentity(self);
	self->monsterinfo.currentmove = &spider_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start(self);
}

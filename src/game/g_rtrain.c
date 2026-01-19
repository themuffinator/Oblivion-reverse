#include "g_local.h"

#if OBLIVION_ENABLE_ROTATE_TRAIN

#define STATE_TOP                       0
#define STATE_BOTTOM            1
#define STATE_UP                        2
#define STATE_DOWN                      3

#define RTRAIN_START_ON         1
#define RTRAIN_TOGGLE           2
#define RTRAIN_BLOCK_STOPS      4

static void rotate_train_next(edict_t *self);
static void rotate_train_wait(edict_t *self);
static void rotate_train_resume(edict_t *self);

/*
=============
RotateTrain_ApplyCornerSettings

Apply per-corner overrides to the rotate train.
=============
*/
static void RotateTrain_ApplyCornerSettings(edict_t *self, edict_t *corner, qboolean set_speed)
{
	if (!corner)
		return;

	if (corner->duration > 0)
		self->duration = corner->duration;
	else
		self->duration = 0;

	if (set_speed) {
		self->moveinfo.speed = corner->speed;
	} else if (corner->speed != 0) {
		self->moveinfo.speed = corner->speed;
	}

	if (!VectorCompare(corner->rotate, vec3_origin))
		VectorCopy(corner->rotate, self->rotate);
	else
		VectorClear(self->rotate);

	if (!VectorCompare(corner->rotate_speed, vec3_origin))
		VectorCopy(corner->rotate_speed, self->rotate_speed);
	else
		VectorClear(self->rotate_speed);
}

/*
=============
RotateTrain_SetAngleTargets

Set angle start/end targets for rotate vectors.
=============
*/
static qboolean RotateTrain_SetAngleTargets(edict_t *self)
{
	vec3_t accum;
	vec3_t temp;
	qboolean has_rotate = false;

	VectorCopy(self->s.angles, self->moveinfo.start_angles);
	VectorCopy(self->s.angles, self->moveinfo.end_angles);

	VectorClear(accum);

	if (self->rotate[0] != 0.0f) {
		VectorCopy(self->s.angles, temp);
		temp[0] += self->rotate[0];
		VectorAdd(accum, temp, accum);
		has_rotate = true;
	}

	if (self->rotate[1] != 0.0f) {
		VectorCopy(self->s.angles, temp);
		temp[1] += self->rotate[1];
		VectorAdd(accum, temp, accum);
		has_rotate = true;
	}

	if (self->rotate[2] != 0.0f) {
		VectorCopy(self->s.angles, temp);
		temp[2] += self->rotate[2];
		VectorAdd(accum, temp, accum);
		has_rotate = true;
	}

	if (has_rotate)
		VectorCopy(accum, self->moveinfo.end_angles);

	return has_rotate;
}

/*
=============
RotateTrain_SetRotateSpeed

Apply rotate_speed when no rotate vector is present.
=============
*/
static void RotateTrain_SetRotateSpeed(edict_t *self)
{
	if (!VectorCompare(self->rotate, vec3_origin))
		return;

	if (!VectorCompare(self->rotate_speed, vec3_origin))
		VectorCopy(self->rotate_speed, self->avelocity);
}

/*
=============
RotateTrain_UpdateAngularVelocity

Update angular velocity for rotate vectors.
=============
*/
static void RotateTrain_UpdateAngularVelocity(edict_t *self)
{
	vec3_t delta;
	float angle_dist;
	float move_time;
	float frames;
	float frame_time;

	VectorSubtract(self->moveinfo.end_angles, self->moveinfo.start_angles, delta);
	angle_dist = VectorLength(delta);

	if (angle_dist <= 0.0f) {
		VectorClear(self->avelocity);
		return;
	}

	if (self->duration > 0.0f) {
		move_time = self->duration;
	} else if (self->moveinfo.speed > 0.0f) {
		move_time = angle_dist / self->moveinfo.speed;
	} else {
		move_time = 0.0f;
	}

	if (move_time <= 0.0f) {
		VectorClear(self->avelocity);
		return;
	}

	frames = (float)floor(move_time / FRAMETIME);
	if (frames < 1.0f)
		frames = 1.0f;

	frame_time = frames * FRAMETIME;
	if (frame_time <= 0.0f) {
		VectorClear(self->avelocity);
		return;
	}

	VectorScale(delta, 1.0f / frame_time, self->avelocity);
}

/*
=============
RotateTrain_WrapAngle

Wrap angles using the retail rotate train behavior.
=============
*/
static float RotateTrain_WrapAngle(float angle)
{
	int int_angle = (int)angle;

	return (float)(int_angle % 360);
}

/*
=============
RotateTrain_MoveDone

Stop motion and fire the end function.
=============
*/
static void RotateTrain_MoveDone(edict_t *self)
{
	VectorClear(self->velocity);
	VectorClear(self->avelocity);

	self->s.angles[0] = RotateTrain_WrapAngle(self->s.angles[0]);
	self->s.angles[1] = RotateTrain_WrapAngle(self->s.angles[1]);
	self->s.angles[2] = RotateTrain_WrapAngle(self->s.angles[2]);

	self->moveinfo.endfunc(self);
}

/*
=============
RotateTrain_MoveFinal

Finish movement and schedule RotateTrain_MoveDone.
=============
*/
static void RotateTrain_MoveFinal(edict_t *self)
{
	vec3_t move;

	if (self->moveinfo.remaining_distance == 0) {
		RotateTrain_MoveDone(self);
		return;
	}

	VectorScale(self->moveinfo.dir, self->moveinfo.remaining_distance / FRAMETIME, self->velocity);

	if (!VectorCompare(self->rotate, vec3_origin)) {
		VectorSubtract(self->moveinfo.end_angles, self->s.angles, move);
		if (VectorCompare(move, vec3_origin))
			VectorClear(self->avelocity);
		else
			VectorScale(move, 1.0f / FRAMETIME, self->avelocity);
	}

	self->think = RotateTrain_MoveDone;
	self->nextthink = level.time + FRAMETIME;
}

/*
=============
RotateTrain_MoveBegin

Begin translation/rotation toward the current target.
=============
*/
static void RotateTrain_MoveBegin(edict_t *self)
{
	float frames;

	if ((self->moveinfo.speed * FRAMETIME) >= self->moveinfo.remaining_distance) {
		RotateTrain_MoveFinal(self);
		return;
	}

	VectorScale(self->moveinfo.dir, self->moveinfo.speed, self->velocity);
	frames = floor((self->moveinfo.remaining_distance / self->moveinfo.speed) / FRAMETIME);
	self->moveinfo.remaining_distance -= frames * self->moveinfo.speed * FRAMETIME;
	self->nextthink = level.time + (frames * FRAMETIME);
	self->think = RotateTrain_MoveFinal;

	if (!VectorCompare(self->rotate, vec3_origin))
		RotateTrain_UpdateAngularVelocity(self);
}

/*
=============
RotateTrain_MoveCalc

Initialize movement toward the destination.
=============
*/
static void RotateTrain_MoveCalc(edict_t *self, vec3_t dest, void (*func)(edict_t *))
{
	VectorClear(self->velocity);
	VectorSubtract(dest, self->s.origin, self->moveinfo.dir);
	self->moveinfo.remaining_distance = VectorNormalize(self->moveinfo.dir);
	self->moveinfo.distance = self->moveinfo.remaining_distance;
	self->moveinfo.endfunc = func;

	if (self->duration > 0)
		self->moveinfo.speed = self->moveinfo.distance / self->duration;

	RotateTrain_SetAngleTargets(self);
	RotateTrain_SetRotateSpeed(self);

	if (level.current_entity ==
	    ((self->flags & FL_TEAMSLAVE) ? self->teammaster : self)) {
		RotateTrain_MoveBegin(self);
	} else {
		self->nextthink = level.time + FRAMETIME;
		self->think = RotateTrain_MoveBegin;
	}
}

/*
=============
rotate_train_blocked

Handle entities blocking the rotate train.
=============
*/
static void rotate_train_blocked(edict_t *self, edict_t *other)
{
	if (!(other->svflags & SVF_MONSTER) && (!other->client)) {
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		if (other)
			BecomeExplosion1(other);
		return;
	}

	if (level.time < self->touch_debounce_time)
		return;

	if (!self->dmg)
		return;
	self->touch_debounce_time = level.time + 0.5f;
	T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

/*
=============
rotate_train_wait

Wait at a corner or advance to the next target.
=============
*/
static void rotate_train_wait(edict_t *self)
{
	if (self->target_ent && self->target_ent->pathtarget) {
		char *savetarget = self->target_ent->target;
		edict_t *ent = self->target_ent;
		ent->target = ent->pathtarget;
		G_UseTargets(ent, self->activator);
		ent->target = savetarget;
		if (!self->inuse)
			return;
	}

	if (self->moveinfo.wait) {
		if (self->moveinfo.wait > 0) {
			self->nextthink = level.time + self->moveinfo.wait;
			self->think = rotate_train_next;
		} else if (self->spawnflags & RTRAIN_TOGGLE) {
			rotate_train_next(self);
			self->spawnflags &= ~RTRAIN_START_ON;
			VectorClear(self->velocity);
			self->nextthink = 0;
		}

		if (!(self->flags & FL_TEAMSLAVE)) {
			if (self->moveinfo.sound_end)
				gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_end, 1, ATTN_STATIC, 0);
			self->s.sound = 0;
		}
	} else {
		rotate_train_next(self);
	}
}

/*
=============
rotate_train_next

Advance the rotate train to the next path corner.
=============
*/
static void rotate_train_next(edict_t *self)
{
	edict_t *ent;
	vec3_t dest;
	qboolean first = true;

again:
	if (!self->target)
		return;

	ent = G_PickTarget(self->target);
	if (!ent) {
		gi.dprintf("train_next: bad target %s\n", self->target);
		return;
	}

	self->target = ent->target;

	if (ent->spawnflags & 1) {
		if (!first) {
			gi.dprintf("connected teleport path_corners, see %s at %s\n", ent->classname, vtos(ent->s.origin));
			return;
		}
		first = false;
		VectorCopy(ent->s.origin, self->s.origin);
		VectorCopy(self->s.origin, self->s.old_origin);
		self->s.event = EV_OTHER_TELEPORT;
		gi.linkentity(self);
		goto again;
	}

	self->moveinfo.wait = ent->wait;
	self->target_ent = ent;

	if (!(self->flags & FL_TEAMSLAVE)) {
		if (self->moveinfo.sound_start)
			gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0);
		self->s.sound = self->moveinfo.sound_middle;
	}

	RotateTrain_ApplyCornerSettings(self, ent, false);

	VectorCopy(ent->s.origin, dest);
	self->moveinfo.state = STATE_TOP;
	VectorCopy(self->s.origin, self->moveinfo.start_origin);
	VectorCopy(dest, self->moveinfo.end_origin);

	RotateTrain_MoveCalc(self, dest, rotate_train_wait);
	self->spawnflags |= RTRAIN_START_ON;
}

/*
=============
rotate_train_resume

Resume rotation and translation toward the current target corner.
=============
*/
static void rotate_train_resume(edict_t *self)
{
	edict_t *ent = self->target_ent;
	vec3_t dest;

	if (!ent)
		return;

	VectorCopy(ent->s.origin, dest);
	self->moveinfo.state = STATE_TOP;
	VectorCopy(self->s.origin, self->moveinfo.start_origin);
	VectorCopy(dest, self->moveinfo.end_origin);

	RotateTrain_MoveCalc(self, dest, rotate_train_wait);
	self->spawnflags |= RTRAIN_START_ON;
}

/*
=============
rotate_train_find

Initialize the rotate train starting position based on its first corner.
=============
*/
static void rotate_train_find(edict_t *self)
{
	edict_t *ent;

	if (!self->target) {
		gi.dprintf("train_find: no target\n");
		return;
	}
	ent = G_PickTarget(self->target);
	if (!ent) {
		gi.dprintf("train_find: target %s not found\n", self->target);
		return;
	}
	self->target = ent->target;

	RotateTrain_ApplyCornerSettings(self, ent, true);

	VectorCopy(ent->s.origin, self->s.origin);
	gi.linkentity(self);

	if (!self->targetname)
		self->spawnflags |= RTRAIN_START_ON;

	if (self->spawnflags & RTRAIN_START_ON) {
		self->nextthink = level.time + FRAMETIME;
		self->think = rotate_train_next;
		self->activator = self;
	}
}

/*
=============
rotate_train_use

Toggle the rotate train on or off.
=============
*/
static void rotate_train_use(edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;

	if (self->spawnflags & RTRAIN_START_ON) {
		if (!(self->spawnflags & RTRAIN_TOGGLE))
			return;
		self->spawnflags &= ~RTRAIN_START_ON;
		VectorClear(self->velocity);
		self->nextthink = 0;
	} else {
		if (self->target_ent)
			rotate_train_resume(self);
		else
			rotate_train_next(self);
	}
}

/*
=============
SP_func_rotate_train

Spawn function for func_rotate_train entities.
=============
*/
void SP_func_rotate_train(edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;
	VectorClear(self->s.angles);
	self->blocked = rotate_train_blocked;

	if (self->spawnflags & RTRAIN_BLOCK_STOPS)
		self->dmg = 0;
	else if (!self->dmg)
		self->dmg = 100;

	self->solid = SOLID_BSP;
	gi.setmodel(self, self->model);

	if (st.noise)
		self->moveinfo.sound_middle = gi.soundindex(st.noise);

	if (!self->speed)
		self->speed = 100;

	self->moveinfo.speed = self->speed;
	self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;

	self->use = rotate_train_use;

	gi.linkentity(self);

	if (self->target) {
		self->nextthink = level.time + FRAMETIME;
		self->think = rotate_train_find;
	} else {
		gi.dprintf("func_rotate_train without a target at %s\n", vtos(self->absmin));
	}
}

#endif

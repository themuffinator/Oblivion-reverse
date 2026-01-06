#include "g_local.h"

static void check_dodge(edict_t *self, vec3_t start, vec3_t dir, int speed) {
  vec3_t end;
  vec3_t v;
  trace_t tr;
  float eta;

  // easy mode only ducks one quarter the time
  if (skill->value == 0) {
    if (random() > 0.25)
      return;
  }

  VectorMA(start, 8192, dir, end);
  tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);
  if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) &&
      (tr.ent->monsterinfo.dodge) && infront(tr.ent, self)) {
    VectorSubtract(tr.endpos, start, v);
    eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
    tr.ent->monsterinfo.dodge(tr.ent, self, eta);
  }
}

static void deatomizer_touch(edict_t *self, edict_t *other, cplane_t *plane,
                             csurface_t *surf) {
  vec3_t dir;

  if (other == self->owner)
    return;

  if (surf && (surf->flags & SURF_SKY)) {
    G_FreeEdict(self);
    return;
  }

  if (other->takedamage) {
    if (!VectorCompare(self->velocity, vec3_origin))
      VectorNormalize2(self->velocity, dir);
    else
      VectorClear(dir);
    T_Damage(other, self, self->owner, dir, self->s.origin,
             plane ? plane->normal : vec3_origin, self->dmg, 0, DAMAGE_ENERGY,
             MOD_DEATOMIZER);
  } else {
    gi.WriteByte(svc_temp_entity);
    gi.WriteByte(TE_PLASMA_EXPLOSION);
    gi.WritePosition(self->s.origin);
    gi.multicast(self->s.origin, MULTICAST_PVS);
  }

  if (self->dmg_radius > 0)
    T_RadiusDamage(self, self->owner, self->radius_dmg, other, self->dmg_radius,
                   MOD_DEATOMIZER_SPLASH);

  G_FreeEdict(self);
}

void fire_deatomizer(edict_t *self, vec3_t start, vec3_t dir, int damage,
                     int speed, float damage_radius, int splash_damage) {
  edict_t *bolt;

  bolt = G_Spawn();
  VectorCopy(start, bolt->s.origin);
  VectorCopy(start, bolt->s.old_origin);
  vectoangles(dir, bolt->s.angles);
  VectorScale(dir, speed, bolt->velocity);
  bolt->movetype = MOVETYPE_FLYMISSILE;
  bolt->clipmask = MASK_SHOT;
  bolt->solid = SOLID_BBOX;
  VectorClear(bolt->mins);
  VectorClear(bolt->maxs);
  bolt->s.effects = EF_BFG | EF_ANIM_ALLFAST;
  bolt->s.sound = gi.soundindex("misc/lasfly.wav");
  bolt->s.modelindex = gi.modelindex("models/objects/laser/tris.md2");
  bolt->owner = self;
  bolt->touch = deatomizer_touch;
  bolt->nextthink = level.time + 8000 / speed;
  bolt->think = G_FreeEdict;
  bolt->dmg = damage;
  bolt->radius_dmg = splash_damage;
  bolt->dmg_radius = damage_radius;
  bolt->classname = "deatomizer bolt";

  if (self->client)
    check_dodge(self, bolt->s.origin, dir, speed);

  gi.linkentity(bolt);
}

static void plasma_pistol_touch(edict_t *self, edict_t *other, cplane_t *plane,
                                csurface_t *surf) {
  vec3_t dir;

  if (other == self->owner)
    return;

  if (surf && (surf->flags & SURF_SKY)) {
    G_FreeEdict(self);
    return;
  }

  if (other->takedamage) {
    if (!VectorCompare(self->velocity, vec3_origin))
      VectorNormalize2(self->velocity, dir);
    else
      VectorClear(dir);
    T_Damage(other, self, self->owner, dir, self->s.origin,
             plane ? plane->normal : vec3_origin, self->dmg, 0, DAMAGE_ENERGY,
             MOD_PLASMA_PISTOL);
  } else {
    gi.WriteByte(svc_temp_entity);
    gi.WriteByte(TE_PLASMA_EXPLOSION);
    gi.WritePosition(self->s.origin);
    gi.multicast(self->s.origin, MULTICAST_PVS);
  }

  G_FreeEdict(self);
}

void fire_plasma_pistol(edict_t *self, vec3_t start, vec3_t dir, int damage,
                        int speed) {
  edict_t *bolt;

  bolt = G_Spawn();
  VectorCopy(start, bolt->s.origin);
  VectorCopy(start, bolt->s.old_origin);
  vectoangles(dir, bolt->s.angles);
  VectorScale(dir, speed, bolt->velocity);
  bolt->movetype = MOVETYPE_FLYMISSILE;
  bolt->clipmask = MASK_SHOT;
  bolt->solid = SOLID_BBOX;
  VectorClear(bolt->mins);
  VectorClear(bolt->maxs);
  bolt->s.effects = EF_PLASMA;
  bolt->s.sound = gi.soundindex("misc/lasfly.wav");
  bolt->s.modelindex = gi.modelindex("models/objects/laser/tris.md2");
  bolt->owner = self;
  bolt->touch = plasma_pistol_touch;
  bolt->nextthink = level.time + 8000 / speed;
  bolt->think = G_FreeEdict;
  bolt->dmg = damage;
  bolt->classname = "plasma pistol";

  if (self->client)
    check_dodge(self, bolt->s.origin, dir, speed);

  gi.linkentity(bolt);
}

static void plasma_rifle_touch(edict_t *self, edict_t *other, cplane_t *plane,
                               csurface_t *surf) {
  vec3_t dir;

  if (other == self->owner)
    return;

  if (surf && (surf->flags & SURF_SKY)) {
    G_FreeEdict(self);
    return;
  }

  if (other->takedamage) {
    if (!VectorCompare(self->velocity, vec3_origin))
      VectorNormalize2(self->velocity, dir);
    else
      VectorClear(dir);
    T_Damage(other, self, self->owner, dir, self->s.origin,
             plane ? plane->normal : vec3_origin, self->dmg, 0, DAMAGE_ENERGY,
             MOD_PLASMA_RIFLE);
  } else {
    gi.WriteByte(svc_temp_entity);
    gi.WriteByte(TE_PLASMA_EXPLOSION);
    gi.WritePosition(self->s.origin);
    gi.multicast(self->s.origin, MULTICAST_PVS);
  }

  G_FreeEdict(self);
}

void fire_plasma_rifle(edict_t *self, vec3_t start, vec3_t dir, int damage,
                       int speed) {
  edict_t *bolt;

  bolt = G_Spawn();
  VectorCopy(start, bolt->s.origin);
  VectorCopy(start, bolt->s.old_origin);
  vectoangles(dir, bolt->s.angles);
  VectorScale(dir, speed, bolt->velocity);
  bolt->movetype = MOVETYPE_FLYMISSILE;
  bolt->clipmask = MASK_SHOT;
  bolt->solid = SOLID_BBOX;
  VectorClear(bolt->mins);
  VectorClear(bolt->maxs);
  bolt->s.effects = EF_ROTATE;
  bolt->s.sound = gi.soundindex("misc/lasfly.wav");
  bolt->s.modelindex = gi.modelindex("models/objects/laser/tris.md2");
  bolt->owner = self;
  bolt->touch = plasma_rifle_touch;
  bolt->nextthink = level.time + 8000 / speed;
  bolt->think = G_FreeEdict;
  bolt->dmg = damage;
  bolt->classname = "plasma rifle";

  if (self->client)
    check_dodge(self, bolt->s.origin, dir, speed);

  gi.linkentity(bolt);
}


             void fire_donut(edict_t * self, vec3_t origin, float damage_radius,
                             int splash_damage, edict_t *ignore) {
  edict_t *attacker;

  attacker = self->owner ? self->owner : self;
  T_RadiusDamage(self, attacker, splash_damage, ignore, damage_radius,
                 MOD_DONUT);
}

static void dod_explode(edict_t *self) {
  edict_t *ignore;

  if (!self->inuse)
    return;

  ignore = self->enemy;

  self->s.sound = 0;
  gi.sound(self, CHAN_AUTO, gi.soundindex("sound/dod/DoD.wav"), 1, ATTN_NORM,
           0);

  gi.WriteByte(svc_temp_entity);
  gi.WriteByte(TE_EXPLOSION2);
  gi.WritePosition(self->s.origin);
  gi.multicast(self->s.origin, MULTICAST_PHS);

  fire_donut(self, self->s.origin, self->dmg_radius, self->radius_dmg, ignore);

  G_FreeEdict(self);
}

static void dod_touch(edict_t *self, edict_t *other, cplane_t *plane,
                      csurface_t *surf) {
  vec3_t dir;

  if (other == self->owner)
    return;

  if (surf && (surf->flags & SURF_SKY)) {
    G_FreeEdict(self);
    return;
  }

  self->enemy = NULL;

  if (other->takedamage) {
    self->enemy = other;

    if (!VectorCompare(self->velocity, vec3_origin))
      VectorNormalize2(self->velocity, dir);
    else
      VectorClear(dir);

    T_Damage(other, self, self->owner, dir, self->s.origin,
             plane ? plane->normal : vec3_origin, self->dmg, 0, DAMAGE_ENERGY,
             MOD_DONUT);
  }

  dod_explode(self);
}

void fire_dod(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed,
              float damage_radius, int splash_damage) {
  edict_t *bolt;

  bolt = G_Spawn();
  VectorCopy(start, bolt->s.origin);
  VectorCopy(start, bolt->s.old_origin);
  vectoangles(dir, bolt->s.angles);
  VectorScale(dir, speed, bolt->velocity);
  bolt->movetype = MOVETYPE_FLYMISSILE;
  bolt->clipmask = MASK_SHOT;
  bolt->solid = SOLID_BBOX;
  VectorClear(bolt->mins);
  VectorClear(bolt->maxs);
  bolt->s.effects = EF_PLASMA | EF_ANIM_ALLFAST;
  bolt->s.renderfx = RF_FULLBRIGHT;
  bolt->s.modelindex = gi.modelindex("models/objects/dod/tris.md2");
  bolt->s.sound = gi.soundindex("sound/dod/DoD_hum.wav");
  bolt->owner = self;
  bolt->enemy = NULL;
  bolt->touch = dod_touch;
  bolt->nextthink = level.time + 2.0f;
  bolt->think = dod_explode;
  bolt->dmg = damage;
  bolt->radius_dmg = splash_damage;
  bolt->dmg_radius = damage_radius;
  bolt->classname = "dod";

  if (self->client)
    check_dodge(self, bolt->s.origin, dir, speed);

  gi.linkentity(bolt);
}

static void hellfury_touch(edict_t *self, edict_t *other, cplane_t *plane,
                           csurface_t *surf) {
  vec3_t dir;

  if (other == self->owner)
    return;

  if (surf && (surf->flags & SURF_SKY)) {
    G_FreeEdict(self);
    return;
  }

  if (other->takedamage) {
    VectorNormalize2(self->velocity, dir);
    T_Damage(other, self, self->owner, dir, self->s.origin,
             plane ? plane->normal : vec3_origin, self->dmg, 0,
             DAMAGE_ENERGY | DAMAGE_RADIUS, MOD_HELLFURY);
  }

  fire_donut(self, self->s.origin, self->dmg_radius, self->radius_dmg, other);

  gi.WriteByte(svc_temp_entity);
  gi.WriteByte(TE_EXPLOSION1);
  gi.WritePosition(self->s.origin);
  gi.multicast(self->s.origin, MULTICAST_PVS);

  G_FreeEdict(self);
}

void fire_hellfury(edict_t *self, vec3_t start, vec3_t dir, int damage,
                   int speed, float damage_radius, int splash_damage) {
  edict_t *bolt;

  bolt = G_Spawn();
  VectorCopy(start, bolt->s.origin);
  VectorCopy(start, bolt->s.old_origin);
  vectoangles(dir, bolt->s.angles);
  VectorScale(dir, speed, bolt->velocity);
  bolt->movetype = MOVETYPE_FLYMISSILE;
  bolt->clipmask = MASK_SHOT;
  bolt->solid = SOLID_BBOX;
  VectorClear(bolt->mins);
  VectorClear(bolt->maxs);
  bolt->s.effects = EF_ROCKET;
  bolt->s.modelindex = gi.modelindex("models/objects/rocket/tris.md2");
  bolt->owner = self;
  bolt->touch = hellfury_touch;
  bolt->nextthink = level.time + 8000 / speed;
  bolt->think = G_FreeEdict;
  bolt->dmg = damage;
  bolt->radius_dmg = splash_damage;
  bolt->dmg_radius = damage_radius;
  bolt->classname = "hellfury";

  if (self->client)
    check_dodge(self, bolt->s.origin, dir, speed);

  gi.linkentity(bolt);
}

void fire_laser_cannon(edict_t *self, vec3_t start, vec3_t dir, int damage,
                       int kick) {
  vec3_t end;
  trace_t tr;

  VectorMA(start, 8192, dir, end);
  tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

  if (tr.ent && tr.ent->takedamage) {
    T_Damage(tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, kick,
             DAMAGE_ENERGY, MOD_LASERCANNON);
  }

  gi.WriteByte(svc_temp_entity);
  gi.WriteByte(TE_BFG_LASER);
  gi.WritePosition(start);
  gi.WritePosition(tr.endpos);
  gi.multicast(start, MULTICAST_PVS);
}

static void detpack_detonate(edict_t *self);

/*
=============
detpack_die

Explode the detpack when it is destroyed by external damage.
=============
*/
static void detpack_die(edict_t *self, edict_t *inflictor, edict_t *attacker,
                        int damage, vec3_t point) {
  detpack_detonate(self);
}

/*
=============
detpack_arm

Clear the detpack's temporary flight handlers once it has landed and armed.
=============
*/
static void detpack_arm(edict_t *self) {
  self->think = NULL;
  self->nextthink = 0;
  self->touch = NULL;
}

/*
=============
detpack_detonate

Create the explosion effect and apply the configured splash damage.
=============
*/
static void detpack_detonate(edict_t *self) {
  gi.WriteByte(svc_temp_entity);
  gi.WriteByte(TE_EXPLOSION2);
  gi.WritePosition(self->s.origin);
  gi.multicast(self->s.origin, MULTICAST_PHS);

  T_RadiusDamage(self, self->owner ? self->owner : self, self->radius_dmg, NULL,
                 self->dmg_radius, MOD_DETPACK);

  G_FreeEdict(self);
}

/*
=============
detpack_touch

Handle the detpack coming to rest on world geometry.
=============
*/
static void detpack_touch(edict_t *self, edict_t *other, cplane_t *plane,
                          csurface_t *surf) {
  if (other == self->owner)
    return;

  if (surf && (surf->flags & SURF_SKY)) {
    G_FreeEdict(self);
    return;
  }

  if (!self->groundentity) {
    VectorClear(self->velocity);
    VectorClear(self->avelocity);
    self->movetype = MOVETYPE_NONE;
    self->touch = NULL;
    self->think = detpack_arm;
    self->nextthink = level.time + 0.2f;
    self->groundentity = other;
  }
}

#define MAX_ACTIVE_DETPACKS 5

/*
=============
detpack_enforce_limit

Ensure a single owner cannot exceed the detpack count that the HLIL logic
enforces.
=============
*/
static void detpack_enforce_limit(edict_t *charge) {
  edict_t *ent;
  edict_t *oldest;
  int count;
  int i;

  if (!charge || !charge->owner)
    return;

  oldest = charge;
  count = 0;

  for (i = 1; i < globals.num_edicts; i++) {
    ent = &g_edicts[i];
    if (!ent->inuse)
      continue;
    if (!ent->classname)
      continue;
    if (strcmp(ent->classname, "detpack"))
      continue;
    if (ent->owner != charge->owner)
      continue;

    count++;

    if ((ent != charge) &&
        (oldest == charge || ent->timestamp < oldest->timestamp))
      oldest = ent;
  }

  if (count > MAX_ACTIVE_DETPACKS && oldest)
    detpack_detonate(oldest);
}

/*
=============
fire_detpack

Spawn the thrown detpack projectile and enforce the per-owner count cap.
=============
*/
edict_t *fire_detpack(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
                      int speed, float damage_radius) {
  edict_t *charge;

  charge = G_Spawn();
  VectorCopy(start, charge->s.origin);
  VectorCopy(start, charge->s.old_origin);
  vectoangles(aimdir, charge->s.angles);
  VectorScale(aimdir, speed, charge->velocity);
  charge->movetype = MOVETYPE_TOSS;
  charge->clipmask = MASK_SHOT;
  charge->solid = SOLID_BBOX;
  VectorClear(charge->mins);
  VectorClear(charge->maxs);
  charge->s.modelindex = gi.modelindex("models/objects/detpack/tris.md2");
  charge->s.effects = EF_GRENADE;
  charge->owner = self;
  charge->touch = detpack_touch;
  charge->think = detpack_arm;
  charge->nextthink = level.time + 0.2f;
  charge->dmg = damage;
  charge->radius_dmg = damage;
  charge->dmg_radius = damage_radius;
  charge->classname = "detpack";
  charge->health = 70;
  charge->max_health = 70;
  charge->takedamage = DAMAGE_YES;
  charge->die = detpack_die;
  charge->timestamp = level.time;

  gi.linkentity(charge);
  detpack_enforce_limit(charge);

  return charge;
}

/*
=============
SP_detpack

Spawn a detpack projectile from a map entity.
=============
*/
void SP_detpack(edict_t *self) {
  vec3_t forward;
  float radius;
  int damage;
  int speed;

  if (!self)
    return;

  if (self->speed <= 0)
    self->speed = 400;
  if (!self->dmg)
    self->dmg = 240;
  if (self->dmg_radius <= 0.0f)
    self->dmg_radius = 200.0f;

  damage = self->dmg;
  speed = (int)self->speed;
  radius = self->dmg_radius;

  AngleVectors(self->s.angles, forward, NULL, NULL);
  fire_detpack(&g_edicts[0], self->s.origin, forward, damage, speed, radius);

  G_FreeEdict(self);
}

/*
=============
remote_detonator_trigger

Detonate every active detpack that belongs to the specified owner.
=============
*/
void remote_detonator_trigger(edict_t *owner) {
  int i;
  edict_t *ent;

  for (i = 1; i < globals.num_edicts; i++) {
    ent = &g_edicts[i];
    if (!ent->inuse)
      continue;
    if (!ent->classname)
      continue;
    if (strcmp(ent->classname, "detpack"))
      continue;
    if (ent->owner != owner)
      continue;

    detpack_detonate(ent);
  }
}

#define MAX_ACTIVE_MINES 5

static void proximity_mine_explode(edict_t *self, edict_t *target);

/*
=============
mine_enforce_limit

Ensure a single owner cannot exceed the mine count cap from the HLIL logic.
=============
*/
static void mine_enforce_limit(edict_t *mine) {
  edict_t *ent;
  edict_t *oldest;
  int count;
  int i;

  if (!mine || !mine->owner)
    return;

  oldest = mine;
  count = 0;

  for (i = 1; i < globals.num_edicts; i++) {
    ent = &g_edicts[i];
    if (!ent->inuse)
      continue;
    if (!ent->classname)
      continue;
    if (strcmp(ent->classname, "mine"))
      continue;
    if (ent->owner != mine->owner)
      continue;

    count++;

    if ((ent != mine) && (oldest == mine || ent->timestamp < oldest->timestamp))
      oldest = ent;
  }

  if (count > MAX_ACTIVE_MINES && oldest)
    proximity_mine_explode(oldest, NULL);
}

static void proximity_mine_explode(edict_t *self, edict_t *target) {
  if (target && target->takedamage) {
    vec3_t dir;
    VectorSubtract(target->s.origin, self->s.origin, dir);
    VectorNormalize(dir);
    T_Damage(target, self, self->owner ? self->owner : self, dir,
             self->s.origin, vec3_origin, self->dmg, 0, DAMAGE_ENERGY,
             MOD_MINE);
  }

  gi.WriteByte(svc_temp_entity);
  gi.WriteByte(TE_PLASMA_EXPLOSION);
  gi.WritePosition(self->s.origin);
  gi.multicast(self->s.origin, MULTICAST_PVS);

  if (self->dmg_radius > 0) {
    self->takedamage = DAMAGE_NO;
    self->die = NULL;
    T_RadiusDamage(self, self->owner ? self->owner : self, self->radius_dmg,
                   self, self->dmg_radius, MOD_MINE_SPLASH);
  }

  G_FreeEdict(self);
}

/*
=============
proximity_mine_die

Detonate the proximity mine when it is destroyed by damage.
=============
*/
static void proximity_mine_die(edict_t *self, edict_t *inflictor,
                               edict_t *attacker, int damage, vec3_t point) {
  (void)inflictor;
  (void)damage;
  (void)point;

  proximity_mine_explode(self, attacker);
}

static void proximity_mine_think(edict_t *self) {
  edict_t *ent;

  ent = NULL;
  while ((ent = findradius(ent, self->s.origin, self->dmg_radius)) != NULL) {
    if (ent == self->owner)
      continue;
    if (!ent->takedamage)
      continue;
    if (!(ent->svflags & SVF_MONSTER) && !ent->client)
      continue;

    proximity_mine_explode(self, ent);
    return;
  }

  self->nextthink = level.time + 0.1f;
}

static void proximity_mine_arm(edict_t *self) {
  self->think = proximity_mine_think;
  self->nextthink = level.time + 0.1f;
}

static void proximity_mine_touch(edict_t *self, edict_t *other, cplane_t *plane,
                                 csurface_t *surf) {
  if (other == self->owner)
    return;

  if (surf && (surf->flags & SURF_SKY)) {
    G_FreeEdict(self);
    return;
  }

  if (!self->groundentity) {
    VectorClear(self->velocity);
    VectorClear(self->avelocity);
    self->movetype = MOVETYPE_NONE;
    self->touch = NULL;
    self->think = proximity_mine_arm;
    self->nextthink = level.time + 0.2f;
    self->groundentity = other;
  }
}

edict_t *fire_proximity_mine(edict_t *self, vec3_t start, vec3_t aimdir,
                             int damage, int speed, float damage_radius,
                             int splash_damage) {
  edict_t *mine;

  mine = G_Spawn();
  VectorCopy(start, mine->s.origin);
  VectorCopy(start, mine->s.old_origin);
  vectoangles(aimdir, mine->s.angles);
  VectorScale(aimdir, speed, mine->velocity);
  mine->movetype = MOVETYPE_TOSS;
  mine->clipmask = MASK_SHOT;
  mine->solid = SOLID_BBOX;
  VectorSet(mine->mins, -2, -2, -2);
  VectorSet(mine->maxs, 2, 2, 2);
  mine->s.effects = EF_GRENADE;
  mine->s.modelindex = gi.modelindex("models/objects/mine/tris.md2");
  mine->owner = self;
  mine->touch = proximity_mine_touch;
  mine->think = proximity_mine_arm;
  mine->nextthink = level.time + 0.2f;
  mine->dmg = damage;
  mine->radius_dmg = splash_damage;
  mine->dmg_radius = damage_radius;
  mine->classname = "mine";
  mine->takedamage = DAMAGE_YES;
  mine->health = 10;
  mine->max_health = 10;
  mine->die = proximity_mine_die;
  mine->timestamp = level.time;

  gi.linkentity(mine);
  mine_enforce_limit(mine);

  return mine;
}

/*
=============
SP_mine

Spawn a proximity mine projectile from a map entity.
=============
*/
void SP_mine(edict_t *self) {
  vec3_t forward;
  float radius;
  int damage;
  int speed;
  int splash;

  if (!self)
    return;

  if (self->speed <= 0)
    self->speed = 600;
  if (!self->dmg)
    self->dmg = 150;
  if (!self->radius_dmg)
    self->radius_dmg = 100;
  if (self->dmg_radius <= 0.0f)
    self->dmg_radius = 180.0f;

  damage = self->dmg;
  speed = (int)self->speed;
  splash = self->radius_dmg;
  radius = self->dmg_radius;

  AngleVectors(self->s.angles, forward, NULL, NULL);
  fire_proximity_mine(&g_edicts[0], self->s.origin, forward, damage, speed,
                      radius, splash);

  G_FreeEdict(self);
}

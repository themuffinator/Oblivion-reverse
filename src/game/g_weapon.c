static void deatomizer_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    vec3_t  dir;

    if (other == self->owner)
        return;

    if (surf && (surf->flags & SURF_SKY))
    {
        G_FreeEdict (self);
        return;
    }

    if (other->takedamage)
    {
        if (!VectorCompare (self->velocity, vec3_origin))
            VectorNormalize2 (self->velocity, dir);
        else
            VectorClear (dir);
        T_Damage (other, self, self->owner, dir, self->s.origin,
                plane ? plane->normal : vec3_origin, self->dmg, 0, DAMAGE_ENERGY, MOD_DEATOMIZER);
    }
    else
    {
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_PLASMA_EXPLOSION);
        gi.WritePosition (self->s.origin);
        gi.multicast (self->s.origin, MULTICAST_PVS);
    }

    if (self->dmg_radius > 0)
        T_RadiusDamage (self, self->owner, self->radius_dmg, other, self->dmg_radius, MOD_DEATOMIZER_SPLASH);

    G_FreeEdict (self);
}

void fire_deatomizer (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int splash_damage)
{
    edict_t *bolt;

    bolt = G_Spawn ();
    VectorCopy (start, bolt->s.origin);
    VectorCopy (start, bolt->s.old_origin);
    vectoangles (dir, bolt->s.angles);
    VectorScale (dir, speed, bolt->velocity);
    bolt->movetype = MOVETYPE_FLYMISSILE;
    bolt->clipmask = MASK_SHOT;
    bolt->solid = SOLID_BBOX;
    VectorClear (bolt->mins);
    VectorClear (bolt->maxs);
    bolt->s.effects = EF_BFG | EF_ANIM_ALLFAST;
    bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
    bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
    bolt->owner = self;
    bolt->touch = deatomizer_touch;
    bolt->nextthink = level.time + 8000 / speed;
    bolt->think = G_FreeEdict;
    bolt->dmg = damage;
    bolt->radius_dmg = splash_damage;
    bolt->dmg_radius = damage_radius;
    bolt->classname = "deatomizer bolt";

    if (self->client)
        check_dodge (self, bolt->s.origin, dir, speed);

    gi.linkentity (bolt);
}

static void plasma_pistol_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    vec3_t dir;

    if (other == self->owner)
        return;

    if (surf && (surf->flags & SURF_SKY))
    {
        G_FreeEdict (self);
        return;
    }

    if (other->takedamage)
    {
        if (!VectorCompare (self->velocity, vec3_origin))
            VectorNormalize2 (self->velocity, dir);
        else
            VectorClear (dir);
        T_Damage (other, self, self->owner, dir, self->s.origin,
                plane ? plane->normal : vec3_origin, self->dmg, 0, DAMAGE_ENERGY, MOD_PLASMA_PISTOL);
    }
    else
    {
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_PLASMA_EXPLOSION);
        gi.WritePosition (self->s.origin);
        gi.multicast (self->s.origin, MULTICAST_PVS);
    }

    G_FreeEdict (self);
}

void fire_plasma_pistol (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
    edict_t *bolt;

    bolt = G_Spawn ();
    VectorCopy (start, bolt->s.origin);
    VectorCopy (start, bolt->s.old_origin);
    vectoangles (dir, bolt->s.angles);
    VectorScale (dir, speed, bolt->velocity);
    bolt->movetype = MOVETYPE_FLYMISSILE;
    bolt->clipmask = MASK_SHOT;
    bolt->solid = SOLID_BBOX;
    VectorClear (bolt->mins);
    VectorClear (bolt->maxs);
    bolt->s.effects = EF_PLASMA;
    bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
    bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
    bolt->owner = self;
    bolt->touch = plasma_pistol_touch;
    bolt->nextthink = level.time + 8000 / speed;
    bolt->think = G_FreeEdict;
    bolt->dmg = damage;
    bolt->classname = "plasma pistol";

    if (self->client)
        check_dodge (self, bolt->s.origin, dir, speed);

    gi.linkentity (bolt);
}

static void plasma_rifle_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    vec3_t dir;

    if (other == self->owner)
        return;

    if (surf && (surf->flags & SURF_SKY))
    {
        G_FreeEdict (self);
        return;
    }

    if (other->takedamage)
    {
        if (!VectorCompare (self->velocity, vec3_origin))
            VectorNormalize2 (self->velocity, dir);
        else
            VectorClear (dir);
        T_Damage (other, self, self->owner, dir, self->s.origin,
                plane ? plane->normal : vec3_origin, self->dmg, 0, DAMAGE_ENERGY, MOD_PLASMA_RIFLE);
    }
    else
    {
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_PLASMA_EXPLOSION);
        gi.WritePosition (self->s.origin);
        gi.multicast (self->s.origin, MULTICAST_PVS);
    }

    G_FreeEdict (self);
}

void fire_plasma_rifle (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
    edict_t *bolt;

    bolt = G_Spawn ();
    VectorCopy (start, bolt->s.origin);
    VectorCopy (start, bolt->s.old_origin);
    vectoangles (dir, bolt->s.angles);
    VectorScale (dir, speed, bolt->velocity);
    bolt->movetype = MOVETYPE_FLYMISSILE;
    bolt->clipmask = MASK_SHOT;
    bolt->solid = SOLID_BBOX;
    VectorClear (bolt->mins);
    VectorClear (bolt->maxs);
    bolt->s.effects = EF_ROTATE;
    bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
    bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
    bolt->owner = self;
    bolt->touch = plasma_rifle_touch;
    bolt->nextthink = level.time + 8000 / speed;
    bolt->think = G_FreeEdict;
    bolt->dmg = damage;
    bolt->classname = "plasma rifle";

    if (self->client)
        check_dodge (self, bolt->s.origin, dir, speed);

    gi.linkentity (bolt);
}

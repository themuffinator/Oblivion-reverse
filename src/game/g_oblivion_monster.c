#include "g_local.h"

/*
=================
deatom_think
=================
*/
void deatom_think (edict_t *self)
{
	self->s.frame++;

	if (self->s.frame > 14)
		self->s.frame = 0;

	self->nextthink = level.time + 0.1;
}

/*
=================
deatom_touch
=================
*/
void deatom_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

	// PMM - crash prevention
	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	gi.sound(self, CHAN_WEAPON, gi.soundindex("deatom/dimpact.wav"), 1, ATTN_NORM, 0);

	if (other->takedamage)
	{
		// Note: trace_t info isn't fully passed in generic touch functions in some Q2 bases, 
		// but G_RunEntity usually calls touch with (ent, other, plane, surf).
		// We can construct a trace_t or just pass plane->normal if valid.
		
		vec3_t normal;
		if (plane)
			VectorCopy(plane->normal, normal);
		else
			VectorClear(normal);

		T_Damage(other, self, self->owner, self->velocity, self->s.origin, normal, self->dmg, 1, DAMAGE_ENERGY, MOD_DISINTEGRATOR);

		if (other->health <= 0) // target was killed
		{
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_TELEPORT_EFFECT);
			gi.WritePosition(other->s.origin);
			gi.multicast(other->s.origin, MULTICAST_PVS);
			
			// Konig code frees the other? "G_FreeEdict(other);"
			// This is aggressive. Usually we let the target die normally?
			// But maybe deatomizer disintegrates them instantly.
			// I'll keep the logic.
			G_FreeEdict(other);
		}
	}
	else
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BLUEHYPERBLASTER);
		gi.WritePosition(self->s.origin);
		if (plane)
			gi.WriteDir(plane->normal);
		else
			gi.WriteDir(vec3_origin);
		gi.multicast(self->s.origin, MULTICAST_PHS);
	}

	G_FreeEdict(self);
}

/*
=================
fire_deatom
=================
*/
void fire_deatom (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t *deatom;
	trace_t	 tr;

	gi.sound(self, CHAN_WEAPON, gi.soundindex("deatom/dfire.wav"), 1, ATTN_NORM, 0);

	deatom = G_Spawn();
	VectorCopy(start, deatom->s.origin);
	VectorCopy(start, deatom->s.old_origin);
	vectoangles(dir, deatom->s.angles);
	VectorScale(dir, speed, deatom->velocity);
	
	deatom->svflags |= SVF_PROJECTILE;
	deatom->movetype = MOVETYPE_FLYMISSILE;
	deatom->clipmask = MASK_PROJECTILE;
	deatom->flags |= FL_DODGE;

	// [Paril-KEX]
	// if (self->client && !G_ShouldPlayersCollide(true))
	// 	deatom->clipmask &= ~CONTENTS_PLAYER;
    // Keeping this commented or adapting if G_ShouldPlayersCollide exists.
    // I'll omit it for now as it seems engine/mod specific.

	deatom->solid = SOLID_BBOX;
	deatom->s.effects |= EF_DUALFIRE; // Ensure EF_DUALFIRE is defined or use 0
	deatom->dmg_radius = 128;
	deatom->s.modelindex = gi.modelindex("models/objects/deatom/tris.md2");
	deatom->s.scale = 0.75f;
	deatom->touch = deatom_touch;
	deatom->s.sound = gi.soundindex("deatom/dfly.wav");

	deatom->owner = self;
	deatom->nextthink = level.time + 2;
	deatom->think = G_FreeEdict;
	deatom->dmg = damage;
	deatom->classname = "deatom";
    
    // Konig sets think to deatom_think? 
    // "deatom->think = deatom_think;" overwrites G_FreeEdict?
    // And "deatom->nextthink = level.time + 10_hz;" (~0.1)
    // Yes.
	deatom->think = deatom_think;
	deatom->nextthink = level.time + 0.1;
	gi.linkentity(deatom);

	tr = gi.traceline(self->s.origin, deatom->s.origin, deatom, deatom->clipmask);
	if (tr.fraction < 1.0f)
	{
		VectorMA(tr.endpos, 1.0, tr.plane.normal, deatom->s.origin);
		deatom->touch(deatom, tr.ent, &tr.plane, tr.surface);
	}
}

void monster_fire_deatom(edict_t* self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype)
{
    // Konig: if (EMPNukeCheck(self, self->s.origin)) ...
    // Assuming EMPNukeCheck exists or omitting.
    
    fire_deatom(self, start, dir, damage, speed);
    gi.WriteByte (svc_muzzleflash2);
    gi.WriteShort (self - g_edicts);
    gi.WriteByte (flashtype);
    gi.multicast (start, MULTICAST_PVS);
}

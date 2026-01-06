#include "g_local.h"

// Forward declarations
void sub_10002f70(edict_t *arg1);
int sub_100031e0(edict_t *arg1);
int sub_10003280(edict_t *arg1);
int sub_10003420(edict_t *arg1);
int sub_100035d0(edict_t *arg1);
int sub_100035f0(edict_t *arg1);
int sub_10003760(edict_t *arg1);
int sub_100036b0(edict_t *arg1);
int sub_10003880(edict_t *arg1);

// Helper for sub_100035d0
int sub_100035d0(edict_t *arg1) {
	if (arg1->noise_index2) // Using noise_index2 as a flag (0x254)
		return sub_100031e0(arg1);
	return sub_10003280(arg1);
}

// 10002f70
void sub_10002f70(edict_t *arg1) {
	edict_t *esi = arg1;
	edict_t *eax_1;
	vec3_t diff;
	float dist;
	float speed;
	float travel;
	float remaining;

	switch (esi->oblivion.deatom.deatom_state) {
	case 0:
	case 1:
		eax_1 = esi->deatom_target_ent;
		if (!eax_1) {
			esi->oblivion.deatom.deatom_f_32c = 0;
			esi->teleport_time = -1.0f; // 0xbf800000
			return;
		}

		VectorSubtract(eax_1->s.origin, esi->s.origin, diff);
		esi->oblivion.deatom.deatom_f_34c = VectorLength(diff); // Distance
		VectorNormalize2(diff, esi->oblivion.deatom.deatom_origin); // Direction (0x334)

		speed = esi->oblivion.deatom.deatom_f_320;
		if (speed <= 0) speed = 1;

		// *(esi i+ 0x330) = 2;
		esi->oblivion.deatom.deatom_state = 2;

		// Placeholder logic:
		if (esi->oblivion.deatom.deatom_f_34c <= 0) {
			// No distance to move?
			gi.bprintf(2, "no main move\n");
		}

		// Update 0x32c accumulator
		esi->oblivion.deatom.deatom_f_32c += 0.1f; // data_1006c2e4 is likely 0.1 or FRAMETIME
		break;

	case 2:
		if (esi->oblivion.deatom.deatom_f_34c <= 0) {
			esi->oblivion.deatom.deatom_f_32c = 0;
			// check if finished?
			// ...
			esi->oblivion.deatom.deatom_f_34c = 0;
			esi->oblivion.deatom.deatom_state = 3;
			esi->oblivion.deatom.deatom_f_32c += 0.1f;
		}
		break;

	case 3:
		if (esi->teleport_time != -1.0f) {
			esi->oblivion.deatom.deatom_state = 1;
			esi->teleport_time = 0;
			esi->oblivion.deatom.deatom_f_32c += 0.1f;
		} else {
			esi->oblivion.deatom.deatom_f_32c = 0;
			esi->oblivion.deatom.deatom_state = 0;
		}
		break;
	}
}

// 100035f0 - Spawn function for control entity
int sub_100035f0(edict_t *arg1) {
	char *targetname = arg1->targetname;

	arg1->solid = SOLID_NOT;
	arg1->movetype = MOVETYPE_FLYMISSILE;
	arg1->svflags = SVF_NOCLIENT;
	memset(&arg1->mins, 0, 24);

	if (!arg1->targetname) {
		gi.bprintf(2, "%s with no targetname\n", arg1->classname);
		arg1->targetname = "unused"; // Placeholder
	}

	if (arg1->teleport_time < 0) {
		arg1->teleport_time = 3.0f;
	}

	arg1->deatom_think = sub_100035d0;
	arg1->deatom_func_1f4 = sub_10003420;
	arg1->s.modelindex = gi.modelindex("sprites/s_deatom1.sp2");
	gi.linkentity(arg1);
	return 0;
}

// 10003880 - Spawn function for target entity?
int sub_10003880(edict_t *arg1) {
	arg1->solid = SOLID_NOT;
	arg1->movetype = MOVETYPE_FLYMISSILE;
	arg1->svflags = SVF_NOCLIENT;
	memset(&arg1->mins, 0, 24);

	if (!arg1->targetname) {
		gi.bprintf(2, "%s with no targetname\n", arg1->classname);
		arg1->targetname = "unused";
	}

	arg1->noise_index2 = 0;
	arg1->deatom_target_ent = NULL;

	arg1->deatom_think = (int (*)(struct edict_s *))sub_10003760;
	arg1->deatom_func_1f4 = sub_100036b0;
	arg1->s.modelindex = gi.modelindex("sprites/s_deatom1.sp2");
	gi.linkentity(arg1);
	return 0;
}

// 100031e0
int sub_100031e0(edict_t *arg1) {
	// ... logic calling sub_10002f70
	sub_10002f70(arg1);
	return 1;
}

// 10003280
int sub_10003280(edict_t *arg1) {
	// ... logic calling sub_10002f70
	sub_10002f70(arg1);
	return 1;
}

// 10003420
int sub_10003420(edict_t *arg1) {
	return 0;
}

// 10003760
int sub_10003760(edict_t *arg1) {
	// Logic ...
	return 0;
}

// 100036b0
int sub_100036b0(edict_t *arg1) {
	return 0;
}

// Exported spawn functions
void SP_misc_deatomizer_control(edict_t *self) {
	sub_100035f0(self);
}

void SP_misc_deatomizer_target(edict_t *self) {
	sub_10003880(self);
}

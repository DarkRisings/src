/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@efn.org)				   *
*	    Gabrielle Taylor						   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "tables.h"

const struct flag_type  qtype_flags[] =
{
    {   "xpquest",      QTYPE_XP,       TRUE    },
    {   "skillquest",   QTYPE_SKILL,    TRUE    },
    {   "mysteryquest", QTYPE_MYSTERY,  TRUE    },
    {   NULL,           0,              FALSE   }
};

const struct flag_type mprecog_flags[] =
{
    { "A",  A, TRUE }, { "B",  B, TRUE }, { "C",  C, TRUE },
    { "D",  D, TRUE }, { "E",  E, TRUE }, { "F",  F, TRUE },
    { "G",  G, TRUE }, { "H",  H, TRUE }, { "I",  I, TRUE },
    { "J",  J, TRUE }, { "K",  K, TRUE }, { "L",  L, TRUE },
    { "M",  M, TRUE }, { "N",  N, TRUE }, { "O",  O, TRUE },
    { "P",  P, TRUE }, { "Q",  Q, TRUE }, { "R",  R, TRUE },
    { "S",  S, TRUE }, { "T",  T, TRUE }, { "U",  U, TRUE },
    { "V",  V, TRUE }, { "W",  W, TRUE }, { "X",  X, TRUE },
    { "Y",  Y, TRUE }, { "Z",  Z, TRUE }, { "!", aa, TRUE },
    { "@", bb, TRUE }, { "#", cc, TRUE }, { "%", dd, TRUE },
    { "^", ee, TRUE },
    { NULL, 0, FALSE }
};

const struct flag_type quest_flags[] =
{
    { "in_progress",      QUEST_INPROGRESS,      FALSE },
    { "open",             QUEST_OPEN,            FALSE },
    { "always_ko",        QUEST_ALWAYS_KO,        TRUE },
    { "no_loot",          QUEST_NO_LOOT,          TRUE },
    { "no_murder",        QUEST_NO_MURDER,        TRUE },
    { "no_deathrot",      QUEST_NO_DEATHROT,      TRUE },
    { "no_questrot",      QUEST_NO_QUESTROT,      TRUE },
    { "no_scoring",       QUEST_NO_SCORING,       TRUE },
    { "no_magic_score",   QUEST_NO_MAGIC_SCORE,   TRUE },
    { "no_melee_score",   QUEST_NO_MELEE_SCORE,   TRUE },
    { "no_healing_score", QUEST_NO_HEALING_SCORE, TRUE },
    { "pc_damage_only",   QUEST_PC_DAMAGE_ONLY,   TRUE },
    { "mob_damage_only",  QUEST_MOB_DAMAGE_ONLY,  TRUE },
    { NULL,               0,                     FALSE }
};

const struct flag_type limit_flags[] =
{
    { "covenance",      LIMIT_COVENANCE,     TRUE },
    { "coder",          LIMIT_CODER,         TRUE },
    { "imp",            LIMIT_IMP,           TRUE },
    { "builder",        LIMIT_BUILDER,       TRUE },
    { "gypsy",          LIMIT_GYPSY,         TRUE },
    { "inferno",        LIMIT_INFERNO,       TRUE },
    { "vermillion",     LIMIT_VERMILLION,    TRUE },
    { "dawning",        LIMIT_DAWNING,       TRUE },
    { "crimson",        LIMIT_CRIMSON,       TRUE },
    { "arcaenum",       LIMIT_ARCAENUM,      TRUE },
    { "loser",          LIMIT_LOSER,         TRUE },
    { "aequitas",       LIMIT_AEQUITAS,      TRUE },
    { "vampires",       LIMIT_VAMPIRES,      TRUE },
    { "werecreatures",  LIMIT_WERECREATURES, TRUE },
    { "brawlers",       LIMIT_BRAWLERS,      TRUE },
    { "immortals",      LIMIT_IMMORTALS,     TRUE },
    { "patriarchs",     LIMIT_PATRIARCHS,    TRUE },
    { "ravnos",         LIMIT_RAVNOS,        TRUE },
    { "lasombra",       LIMIT_LASOMBRA,      TRUE },
    { "flocrian",       LIMIT_FLOCRIAN,      TRUE },
    { "seraphs",        LIMIT_SERAPHS,       TRUE },
    { "coalition",      LIMIT_COALITION,     TRUE },
    { NULL,             0,                   FALSE }
};

/* The guild table-- WHEN ADDING A NEW GUILD, don't forget
   to add the corresponding limit to the limits table
   above AND add the functionality to do_help in act_info.c! */
struct guild_type guild_table[MAX_GUILD] =
{
    {	"",		""			},
    {   "covenance",    "{R[Covenance]{x "	},
    {	"coder",	"{c[Coder]{x "		},
    {	"imp",		"{m[IMP]{x "		},
    {	"builder",	"{c[Builder]{x "	},
    {   "gypsy",        "{M[Gypsy]{x "		},
    {   "inferno",      "{r[Inferno]{x "	},
    {   "vermillion",   "{b[Vermillion]{x "	},
    {   "dawning",      "{Y[Dawning]{x "	},
    {   "crimson rain", "{m[Crimson Rain]{x "	},
    {   "arcaenum",     "{C[Arcaenum]{x "	},
    {   "loser",        "{G[Loser]{x "		},
    {   "aequitas",     "{W[Aequitas]{x "	},
    {   "admin",        "{m[Admin]{x "          },
    {   "coalition",    "{B[Coalition]{x "	}
};


/* for position */
const struct position_type position_table[] =
{
    {	"dead",			"dead"	},
    {	"unconscious",		"uncon"	},
    {	"not used",		"none"	},
    {	"sleeping",		"sleep"	},
    {	"sprawled",		"sprawl"},
    {	"resting",		"rest"	},
    {   "sitting",		"sit"   },
    {	"fighting",		"fight"	},
    {	"standing",		"stand"	},
    {	NULL,			NULL	}
};

/* for sex */
const struct sex_type sex_table[] =
{
   {	"none"		},
   {	"male"		},
   {	"female"	},
   {	"either"	},
   {	NULL		}
};

/* for sizes */
const struct size_type size_table[] =
{ 
    {	"tiny"		},
    {	"small" 	},
    {	"medium"	},
    {	"large"		},
    {	"huge", 	},
    {	"giant" 	},
    {	NULL		}
};

/* various flag tables */
const struct flag_type act_flags[] =
{
  {	"npc",			A,	FALSE	},
  {	"sentinel",		B,	TRUE	},
  {	"scavenger",		C,	TRUE	},
  {	"nokill",		D,	TRUE	},
  {	"notseen",		E,	TRUE	},
  {	"aggressive",		F,	TRUE	},
  {	"stay_area",		G,	TRUE	},
  {	"wimpy",		H,	TRUE	},
  {	"pet",			I,	TRUE	},
  {	"train",		J,	TRUE	},
  {	"practice",		K,	TRUE	},
  {     "wizinvis",		L,	TRUE	},
  {     "nolocate",             M,      TRUE    },
  {     "unused(N)",            N,      FALSE   },
  {	"undead",		O,	TRUE	},
  {     "nogive",               P,      TRUE    },
  {	"cleric",		Q,	TRUE	},
  {	"mage",			R,	TRUE	},
  {	"thief",		S,	TRUE	},
  {	"warrior",		T,	TRUE	},
  {	"weretrainer",		U,	TRUE	},
  {	"nopurge",		V,	TRUE	},
  {	"outdoors",		W,	TRUE	},
  {     "unused(X)",            X,      FALSE   },
  {	"indoors",		Y,	TRUE	},
  {     "unused(Z)",            Z,      FALSE   },
  {	"healer",		aa,	TRUE	},
  {	"gain",			bb,	TRUE	},
  {	"update_always",	cc,	TRUE	},
  {	"changer",		dd,	TRUE	},
  {     "unused(e)",            ee,     FALSE   },
  {	NULL,			0,	FALSE	}
};

const struct flag_type plr_flags[] =
{
    {	"npc",			A,	FALSE	},
    {   "pueblo",		B,	FALSE   },
    {	"autoassist",		C,	FALSE	},
    {	"autoexit",		D,	FALSE	},
    {	"autoloot",		E,	FALSE	},
    {	"autosac",		F,	FALSE	},
    {	"autogold",		G,	FALSE	},
    {	"autosplit",		H,	FALSE	},
    {   "battlebrief",          I,      FALSE   },
    {   "suspendlevel",         J,      TRUE    },
    {   "unused(K)",            K,      FALSE   },
    {   "brawler",              L,      TRUE    },
    {   "autoafk",              M,      TRUE    },
    {	"holylight",		N,	FALSE	},
    {   "unused(O)",            O,      FALSE   },
    {	"unused(P)",		P,	FALSE	},
    {	"nosummon",		Q,	FALSE	},
    {	"nofollow",		R,	FALSE	},
    {   "unused(S)",            S,      FALSE   },
    {	"colour",		T,	FALSE	},
    {	"permit",		U,	TRUE	},
    {   "unused(V)",            V,      FALSE   },
    {	"log",			W,	FALSE	},
    {	"deny",			X,	FALSE	},
    {	"freeze",		Y,	FALSE	},
    {	"newbie",		Z,	TRUE	},
    {	"killer",		aa,	FALSE	},
    {   "quest",                bb,     FALSE   },
    {   "leader",               cc,     FALSE   },
    {   "linkdead",             dd,     FALSE   },
    {	"unapproved",		ee,	TRUE    },
    {	NULL,			0,	0	}
};

const struct flag_type affect_flags[] =
{
    {	"blind",		A,	TRUE	},
    {	"invisible",		B,	TRUE	},
    {   "bless",		C,	TRUE	},
    {	"detect_invis",		D,	TRUE	},
    {	"detect_magic",		E,	TRUE	},
    {   "detect_hidden",        F,      TRUE    },
    {   "shift_sick",		G,	FALSE	},
    {	"sanctuary",		H,	TRUE	},
    {	"faerie_fire",		I,	TRUE	},
    {	"infrared",		J,	TRUE	},
    {	"curse",		K,	TRUE	},
    {   "nighthaste",		L,	TRUE	},
    {	"poison",		M,	TRUE	},
    {	"shift_on",		N,	FALSE	},
    {	"magicward",		O,	TRUE	},
    {	"sneak",		P,	TRUE	},
    {	"hide",			Q,	TRUE	},
    {	"sleep",		R,	TRUE	},
    {	"charm",		S,	TRUE	},
    {	"flying",		T,	TRUE	},
    {	"pass_door",		U,	TRUE	},
    {	"haste",		V,	TRUE	},
    {	"calm",			W,	TRUE	},
    {	"plague",		X,	TRUE	},
    {	"weaken",		Y,	TRUE	},
    {	"dark_vision",		Z,	TRUE	},
    {	"battle",		aa,	TRUE	},
    {	"swim",			bb,	TRUE	},
    {	"regeneration",		cc,	TRUE	},
    {	"slow",			dd,	TRUE	},
    {	"shift_perm",		ee,	FALSE	},
    {	NULL,			0,	0	}
};

const struct flag_type affect2_flags[] =
{
    {	"ghost",		A,	FALSE	},
    {	"mirror",		B,	TRUE	},
    {	"inspire",		C,	TRUE	},
    {	"hsenses",		D,	FALSE	},
    {	"summonedpet",		E,	FALSE	},
    {	"headache",		F,	TRUE	},
    {	"noregen",		G,	TRUE 	},
    {	"wildaura",		H,	TRUE	},
    {	"sumanimal",		I,	TRUE	},
    {	"fury", 		J,	TRUE	},
    {	"fear",			K,	TRUE	},
    {	"web",			L,	TRUE	},
    {	"shadows",		M,	TRUE	},
    {	"bboil",		N,	TRUE	},
    {   "confuse",		O,	TRUE	},
    {	"absorb",		P,	TRUE	},
    {   "forget",               Q,      TRUE    },
    {	"wither",		R,	TRUE	},
    {	"wanted",		S,	TRUE	},
    {	"focus",		T,	TRUE	},
    {	"silence",		U,	TRUE	},
    {   "guildspell",		V,	TRUE	},
    {	"chimesick",		W,	TRUE	},
    {	"sumsick",		X,	TRUE	},
    {	"celerity",		Y,	TRUE	},
    {	"terror",		Z,	TRUE	},
    {	"detectwere",		aa,	TRUE	},
    {	"chimestry",		bb,	TRUE	},
    {	"reflect",		cc,	TRUE	},
    {   "blindsight",		dd,	TRUE	},
    {   "burning",              ee,     TRUE    },
    {	NULL,			0,	0	}
};

const struct flag_type affect3_flags[] =
{
    {   "sbond",                A,  FALSE   },
    {   "ordain",               B,  TRUE    },
    {   "solemn",               C,  TRUE    },
    {   "skulls",               D,  TRUE    },
    {   "narcolepsy",           F,  FALSE   },
    {   "dirt",                 G,  TRUE    },
    {   "veil",                 H,  TRUE    },
    {   "veilwait",             I,  TRUE    },
    {   "torpor",               J,  FALSE   },
    {   "pinch",                K,  TRUE    },
    {   "weresumsick",          L,  TRUE    },
    {   "offering",             M,  FALSE   },
    {   "frog",                 N,  FALSE   },
    {   "abjure",               O,  FALSE   },
    {   "supercast",            P,  TRUE    },
    {   "famine",               Q,  FALSE   },
    {   "aurapain",             R,  FALSE   },
    {   "profaneword",          S,  FALSE   },
    {   NULL,                   0,  0       }
};

const struct flag_type off_flags[] =
{
    {	"area_attack",		A,	TRUE	},
    {	"backstab",		B,	TRUE	},
    {	"bash",			C,	TRUE	},
    {	"battle",		D,	TRUE	},
    {	"disarm",		E,	TRUE	},
    {	"dodge",		F,	TRUE	},
    {	"fade",			G,	TRUE	},
    {	"fast",			H,	TRUE	},
    {	"kick",			I,	TRUE	},
    {	"dirt_kick",		J,	TRUE	},
    {	"parry",		K,	TRUE	},
    {	"rescue",		L,	TRUE	},
    {	"tail",			M,	TRUE	},
    {	"trip",			N,	TRUE	},
    {	"crush",		O,	TRUE	},
    {	"assist_all",		P,	TRUE	},
    {	"assist_race",		R,	TRUE	},
    {	"assist_players",	S,	TRUE	},
    {	"assist_guard",		T,	TRUE	},
    {	"assist_vnum",		U,	TRUE	},
    {	NULL,			0,	0	}
};

const struct flag_type imm_flags[] =
{
    {	"summon",		A,	TRUE	},
    {   "push",                 A,      TRUE    },
    {	"charm",		B,	TRUE	},
    {	"magic",		C,	TRUE	},
    {	"weapon",		D,	TRUE	},
    {	"bash",			E,	TRUE	},
    {	"pierce",		F,	TRUE	},
    {	"slash",		G,	TRUE	},
    {	"fire",			H,	TRUE	},
    {	"cold",			I,	TRUE	},
    {   "light",	IMM_LIGHT,	TRUE	}, /* IMM_LIGHT = U */
    {	"lightning",		J,	TRUE	},
    {	"acid",			K,	TRUE	},
    {	"poison",		L,	TRUE	},
    {	"negative",		M,	TRUE	},
    {	"holy",			N,	TRUE	},
    {	"energy",		O,	TRUE	},
    {	"mental",		P,	TRUE	},
    {	"disease",		Q,	TRUE	},
    {	"drowning",		R,	TRUE	},
    {	"lagskills",		S,	TRUE	},
    {	"sound",		T,	TRUE	},
    {   "wind",                 cc,     TRUE    },
/*       light                  U       TRUE         See above */
    {	"wood",			X,	TRUE	},
    {	"silver",		Y,	TRUE	},
    {	"iron",			Z,	TRUE	},
    {   "gate",                 aa,     TRUE    },
    {   "portal",               bb,     TRUE    },
    {	NULL,			0,	0	}
};

const struct flag_type form_flags[] =
{
    {	"edible",		FORM_EDIBLE,		TRUE	},
    {	"poison",		FORM_POISON,		TRUE	},
    {	"magical",		FORM_MAGICAL,		TRUE	},
    {	"instant_decay",	FORM_INSTANT_DECAY,	TRUE	},
    {	"other",		FORM_OTHER,		TRUE	},
    {	"animal",		FORM_ANIMAL,		TRUE	},
    {	"sentient",		FORM_SENTIENT,		TRUE	},
    {	"undead",		FORM_UNDEAD,		TRUE	},
    {	"construct",		FORM_CONSTRUCT,		TRUE	},
    {	"mist",			FORM_MIST,		TRUE	},
    {	"intangible",		FORM_INTANGIBLE,	TRUE	},
    {	"biped",		FORM_BIPED,		TRUE	},
    {	"centaur",		FORM_CENTAUR,		TRUE	},
    {	"insect",		FORM_INSECT,		TRUE	},
    {	"spider",		FORM_SPIDER,		TRUE	},
    {	"crustacean",		FORM_CRUSTACEAN,	TRUE	},
    {	"worm",			FORM_WORM,		TRUE	},
    {	"blob",			FORM_BLOB,		TRUE	},
    {	"mammal",		FORM_MAMMAL,		TRUE	},
    {	"bird",			FORM_BIRD,		TRUE	},
    {	"reptile",		FORM_REPTILE,		TRUE	},
    {	"snake",		FORM_SNAKE,		TRUE	},
    {	"dragon",		FORM_DRAGON,		TRUE	},
    {	"amphibian",		FORM_AMPHIBIAN,		TRUE	},
    {	"fish",			FORM_FISH ,		TRUE	},
    {	"cold_blood",		FORM_COLD_BLOOD,	TRUE	},
    {	NULL,			0,			0	}
};

const struct flag_type part_flags[] =
{
    {	"head",			PART_HEAD,		TRUE	},
    {	"arms",			PART_ARMS,		TRUE	},
    {	"legs",			PART_LEGS,		TRUE	},
    {	"heart",		PART_HEART,		TRUE	},
    {	"brains",		PART_BRAINS,		TRUE	},
    {	"guts",			PART_GUTS,		TRUE	},
    {	"hands",		PART_HANDS,		TRUE	},
    {	"feet",			PART_FEET,		TRUE	},
    {	"fingers",		PART_FINGERS,		TRUE	},
    {	"ear",			PART_EAR,		TRUE	},
    {	"eye",			PART_EYE,		TRUE	},
    {	"long_tongue",		PART_LONG_TONGUE,	TRUE	},
    {	"eyestalks",		PART_EYESTALKS,		TRUE	},
    {	"tentacles",		PART_TENTACLES,		TRUE	},
    {	"fins",			PART_FINS,		TRUE	},
    {	"wings",		PART_WINGS,		TRUE	},
    {	"tail",			PART_TAIL,		TRUE	},
    {	"claws",		PART_CLAWS,		TRUE	},
    {	"fangs",		PART_FANGS,		TRUE	},
    {	"horns",		PART_HORNS,		TRUE	},
    {	"scales",		PART_SCALES,		TRUE	},
    {	"tusks",		PART_TUSKS,		TRUE	},
    {	NULL,			0,			0	}
};

const struct flag_type comm_flags[] =
{
    {	"quiet",		COMM_QUIET,		TRUE	},
    {   "deaf",			COMM_DEAF,		TRUE	},
    {   "nowiz",		COMM_NOWIZ,		TRUE	},
    {   "noclangossip",		COMM_NOAUCTION,		TRUE	},
    {   "nogossip",		COMM_NOGOSSIP,		TRUE	},
    {   "noadmin",		COMM_NOADMIN,	TRUE	},
    {   "nomusic",		COMM_NOMUSIC,		TRUE	},
    {   "noguild",		COMM_NOGUILD,		TRUE	},
    {   "noquote",		COMM_NOQUOTE,		TRUE	},
    {   "shoutsoff",		COMM_SHOUTSOFF,		TRUE	},
    {   "compact",		COMM_COMPACT,		TRUE	},
    {   "brief",		COMM_BRIEF,		TRUE	},
    {   "prompt",		COMM_PROMPT,		TRUE	},
    {   "combine",		COMM_COMBINE,		TRUE	},
    {   "telnet_ga",		COMM_TELNET_GA,		TRUE	},
    {   "show_affects",		COMM_SHOW_AFFECTS,	TRUE	},
    {   "nograts",		COMM_NOGRATS,		TRUE	},
    {   "noemote",		COMM_NOEMOTE,		FALSE	},
    {   "noshout",		COMM_NOSHOUT,		FALSE	},
    {   "notell",		COMM_NOTELL,		FALSE	},
    {   "nochannels",		COMM_NOCHANNELS,	FALSE	},
    {   "drmode",               COMM_DRMODE,            TRUE    },
    {   "snoop_proof",		COMM_SNOOP_PROOF,	FALSE	},
    {   "afk",			COMM_AFK,		TRUE	},
    {   "vamp",                 COMM_VAMP,              TRUE    },
    {   "nobrawler",            COMM_NOBRAWLER,         TRUE    },
    {   "verbose",              COMM_VERBOSE,           TRUE },
    {	NULL,			0,			0	}
};

const struct flag_type mprog_flags[] =
{
    {	"act",			TRIG_ACT,		TRUE	},
    {	"bribe",		TRIG_BRIBE,		TRUE 	},
    {	"death",		TRIG_DEATH,		TRUE    },
    {	"entry",		TRIG_ENTRY,		TRUE	},
    {	"fight",		TRIG_FIGHT,		TRUE	},
    {	"give",			TRIG_GIVE,		TRUE	},
    {	"greet",		TRIG_GREET,		TRUE    },
    {	"grall",		TRIG_GRALL,		TRUE	},
    {	"kill",			TRIG_KILL,		TRUE	},
    {	"hpcnt",		TRIG_HPCNT,		TRUE    },
    {	"random",		TRIG_RANDOM,		TRUE	},
    {	"speech",		TRIG_SPEECH,		TRUE	},
    {	"exit",			TRIG_EXIT,		TRUE    },
    {	"exall",		TRIG_EXALL,		TRUE    },
    {	"delay",		TRIG_DELAY,		TRUE    },
    {	"surr",			TRIG_SURR,		TRUE    },
    {   "spawn",		TRIG_SPAWN,		TRUE	},
    {   "ppm",                  TRIG_PPM,               TRUE    },
    {   "selse",                TRIG_SELSE,             TRUE    },
    {   "gelse",                TRIG_GELSE,             TRUE    },
    {	NULL,			0,			TRUE	}
};

const struct flag_type area_flags[] =
{
    {	"none",			AREA_NONE,		FALSE	},
    {	"changed",		AREA_CHANGED,		TRUE	},
    {	"added",		AREA_ADDED,		TRUE	},
    {	"loading",		AREA_LOADING,		FALSE	},
    {	NULL,			0,			0	}
};



const struct flag_type sex_flags[] =
{
    {	"male",			SEX_MALE,		TRUE	},
    {	"female",		SEX_FEMALE,		TRUE	},
    {	"neutral",		SEX_NEUTRAL,		TRUE	},
    {   "random",               3,                      TRUE    },   /* ROM */
    {	"none",			SEX_NEUTRAL,		TRUE	},
    {	NULL,			0,			0	}
};



const struct flag_type exit_flags[] =
{
    {   "door",			EX_ISDOOR,		TRUE    },
    {	"closed",		EX_CLOSED,		TRUE	},
    {	"locked",		EX_LOCKED,		TRUE	},
    {	"pickproof",		EX_PICKPROOF,		TRUE	},
    {   "nopass",		EX_NOPASS,		TRUE	},
    {   "easy",			EX_EASY,		TRUE	},
    {   "hard",			EX_HARD,		TRUE	},
    {	"infuriating",		EX_INFURIATING,		TRUE	},
    {	"noclose",		EX_NOCLOSE,		TRUE	},
    {	"nolock",		EX_NOLOCK,		TRUE	},
    {   "secret",               EX_SECRET,              TRUE    },
    {	NULL,			0,			0	}
};



const struct flag_type door_resets[] =
{
    {	"open and unlocked",	0,		TRUE	},
    {	"closed and unlocked",	1,		TRUE	},
    {	"closed and locked",	2,		TRUE	},
    {	NULL,			0,		0	}
};



const struct flag_type room_flags[] =
{
    {	"dark",			ROOM_DARK,		TRUE	},
    {   "bank",                 ROOM_BANK,              TRUE    },
    {	"no_mob",		ROOM_NO_MOB,		TRUE	},
    {	"indoors",		ROOM_INDOORS,		TRUE	},
    {	"private",		ROOM_PRIVATE,		TRUE    },
    {	"safe",			ROOM_SAFE,		TRUE	},
    {	"solitary",		ROOM_SOLITARY,		TRUE	},
    {	"pet_shop",		ROOM_PET_SHOP,		TRUE	},
    {	"no_recall",		ROOM_NO_RECALL,		TRUE	},
    {	"imp_only",		ROOM_IMP_ONLY,		TRUE    },
    {	"gods_only",	        ROOM_GODS_ONLY,		TRUE    },
    {	"heroes_only",		ROOM_HEROES_ONLY,	TRUE	},
    {	"newbies_only",		ROOM_NEWBIES_ONLY,	TRUE	},
    {	"law",			ROOM_LAW,		TRUE	},
    {   "nowhere",		ROOM_NOWHERE,		TRUE	},
    {   "nogate",               ROOM_NO_GATE,           TRUE    },
    {   "nosummon",		ROOM_NO_SUMMON,		TRUE    },
    {   "noportal",             ROOM_NO_PORTAL,         TRUE    },
    {   "redsands",             ROOM_RED_SANDS,         TRUE    },
    {   "semisafe",             ROOM_SEMISAFE,          TRUE    },
    {	NULL,			0,			0	}
};


const struct flag_type trans_flags[] =
  {
    { "no_portal",        TRANS_NO_PORTAL,       TRUE },
    { "no_portal_in",     TRANS_NO_PORTAL_IN,    TRUE },
    { "no_portal_out",    TRANS_NO_PORTAL_OUT,   TRUE },
    { "no_gate",          TRANS_NO_GATE,         TRUE },
    { "no_gate_in",       TRANS_NO_GATE_IN,      TRUE },
    { "no_gate_out",      TRANS_NO_GATE_OUT,     TRUE },
    { "no_teleport",      TRANS_NO_TELEPORT,     TRUE },
    { "no_teleport_in",   TRANS_NO_TELEPORT_IN,  TRUE },
    { "no_teleport_out",  TRANS_NO_TELEPORT_OUT, TRUE },
    { "no_summon",        TRANS_NO_SUMMON,       TRUE },
    { "no_summon_in",     TRANS_NO_SUMMON_IN,    TRUE },
    { "no_summon_out",    TRANS_NO_SUMMON_OUT,   TRUE },
    { "no_spells",        TRANS_NO_SPELLS,       TRUE },
    { "no_spells_in",     TRANS_NO_SPELLS_IN,    TRUE },
    { "no_spells_out",    TRANS_NO_SPELLS_OUT,   TRUE },
    { "no_recall",        TRANS_NO_RECALL,       TRUE },
    { "no_bad_gates",     TRANS_NO_BAD_GATES,    TRUE },
    { NULL,               0,                     0    }
  };



const struct flag_type sector_flags[] =
{
    {	"inside",	SECT_INSIDE,		TRUE	},
    {	"city",		SECT_CITY,		TRUE	},
    {	"field",	SECT_FIELD,		TRUE	},
    {	"forest",	SECT_FOREST,		TRUE	},
    {	"hills",	SECT_HILLS,		TRUE	},
    {	"mountain",	SECT_MOUNTAIN,		TRUE	},
    {	"swim",		SECT_WATER_SWIM,	TRUE	},
    {	"noswim",	SECT_WATER_NOSWIM,	TRUE	},
    {   "cave",         SECT_CAVE,		TRUE	},
    {	"air",		SECT_AIR,		TRUE	},
    {	"desert",	SECT_DESERT,		TRUE	},
    {	"snow",         SECT_SNOW,		TRUE	},
    {	"lava",	        SECT_LAVA,		TRUE	},
    {	NULL,		0,			0	}
};

const struct flag_type ic_sector_flags[] =
{
    {	"inside",	        SECT_INSIDE,		TRUE	},
    {	"in a city",		SECT_CITY,		TRUE	},
    {	"in fields",	        SECT_FIELD,		TRUE	},
    {	"in the forest",	SECT_FOREST,		TRUE	},
    {	"in the hills",	        SECT_HILLS,		TRUE	},
    {	"in the mountains",	SECT_MOUNTAIN,		TRUE	},
    {	"in shallow water",     SECT_WATER_SWIM,	TRUE	},
    {	"in deep water",	SECT_WATER_NOSWIM,	TRUE	},
    {   "underground",          SECT_CAVE,		TRUE	},
    {	"in the air",	        SECT_AIR,		TRUE	},
    {	"in the desert",        SECT_DESERT,		TRUE	},
    {   "in the snow",          SECT_SNOW,              TRUE    },
    {   "in lava",              SECT_LAVA,              TRUE    },
    {	NULL,		        0,			0	}
};

const struct flag_type type_flags[] =
{
    {	"light",		ITEM_LIGHT,		TRUE	},
    {	"scroll",		ITEM_SCROLL,		TRUE	},
    {	"wand",			ITEM_WAND,		TRUE	},
    {	"staff",		ITEM_STAFF,		TRUE	},
    {	"weapon",		ITEM_WEAPON,		TRUE	},
    {	"treasure",		ITEM_TREASURE,		TRUE	},
    {	"armor",		ITEM_ARMOR,		TRUE	},
    {	"potion",		ITEM_POTION,		TRUE	},
    {	"furniture",		ITEM_FURNITURE,		TRUE	},
    {	"trash",		ITEM_TRASH,		TRUE	},
    {	"container",		ITEM_CONTAINER,		TRUE	},
    {	"drinkcontainer",	ITEM_DRINK_CON,		TRUE	},
    {	"key",			ITEM_KEY,		TRUE	},
    {	"food",			ITEM_FOOD,		TRUE	},
    {	"money",		ITEM_MONEY,		TRUE	},
    {	"boat",			ITEM_BOAT,		TRUE	},
    {	"npccorpse",		ITEM_CORPSE_NPC,	TRUE	},
    {	"pc corpse",		ITEM_CORPSE_PC,		FALSE	},
    {	"fountain",		ITEM_FOUNTAIN,		TRUE	},
    {	"pill",			ITEM_PILL,		TRUE	},
    {	"protect",		ITEM_PROTECT,		TRUE	},
    {	"map",			ITEM_MAP,		TRUE	},
    {   "portal",		ITEM_PORTAL,		TRUE	},
    {   "warpstone",		ITEM_WARP_STONE,	TRUE	},
    {	"roomkey",		ITEM_ROOM_KEY,		TRUE	},
    { 	"gem",			ITEM_GEM,		TRUE	},
    {	"jewelry",		ITEM_JEWELRY,		TRUE	},
    {	"jukebox",		ITEM_JUKEBOX,		TRUE	},
    {	"arrow",		ITEM_ARROW,		TRUE    },
    {   "quiver",               ITEM_QUIVER,            TRUE    },
    {   "seed",                 ITEM_SEED,              TRUE    },
    {   "tree",                 ITEM_TREE,              TRUE    },
    {   "book",                 ITEM_BOOK,              TRUE    },
    {   "plant",                ITEM_PLANT,             TRUE    },
    {	NULL,			0,			0	}
};

const struct flag_type ic_type_flags[] =
{
    {	"light",		ITEM_LIGHT,		TRUE	},
    {	"scroll",		ITEM_SCROLL,		TRUE	},
    {	"wand",			ITEM_WAND,		TRUE	},
    {	"staff",		ITEM_STAFF,		TRUE	},
    {	"weapon",		ITEM_WEAPON,		TRUE	},
    {	"amount of treasure",	ITEM_TREASURE,		TRUE	},
    {	"piece of armor",	ITEM_ARMOR,		TRUE	},
    {	"potion",		ITEM_POTION,		TRUE	},
    {	"piece of furniture",	ITEM_FURNITURE,		TRUE	},
    {	"piece of trash",	ITEM_TRASH,		TRUE	},
    {	"container",		ITEM_CONTAINER,		TRUE	},
    {	"drink container",	ITEM_DRINK_CON,		TRUE	},
    {	"key",			ITEM_KEY,		TRUE	},
    {	"amount of food",	ITEM_FOOD,		TRUE	},
    {	"amount of money",	ITEM_MONEY,		TRUE	},
    {	"boat",			ITEM_BOAT,		TRUE	},
    {	"corpse",		ITEM_CORPSE_NPC,	TRUE	},
    {	"person's corpse",	ITEM_CORPSE_PC,		FALSE	},
    {	"fountain",		ITEM_FOUNTAIN,		TRUE	},
    {	"pill",			ITEM_PILL,		TRUE	},
    {	"protected item",	ITEM_PROTECT,		TRUE	},
    {	"map",			ITEM_MAP,		TRUE	},
    {	"portal",		ITEM_PORTAL,		TRUE	},
    {	"warpstone",		ITEM_WARP_STONE,	TRUE	},
    {	"room key",		ITEM_ROOM_KEY,		TRUE	},
    {	"gem",			ITEM_GEM,		TRUE	},
    {	"piece of jewelry",	ITEM_JEWELRY,		TRUE	},
    {	"jukebox",		ITEM_JUKEBOX,		TRUE	},
    {   "arrow",                ITEM_ARROW,             TRUE    },
    {   "quiver",               ITEM_QUIVER,            TRUE    },
    {   "seed",                 ITEM_SEED,              TRUE    },
    {   "tree",                 ITEM_TREE,              TRUE    },
    {   "book",                 ITEM_BOOK,              TRUE    },
    {   "plant",                ITEM_PLANT,             TRUE    },
    {	NULL,			0,			0	}
};

const struct flag_type extra_flags[] =
{
    {	"glow",			ITEM_GLOW,		TRUE	},
    {	"hum",			ITEM_HUM,		TRUE	},
    {	"dark",			ITEM_DARK,		TRUE	},
    {	"lock",			ITEM_LOCK,		TRUE	},
/*  {   "unused",               ITEM_EVIL,              FALSE   }, */
    {	"invis",		ITEM_INVIS,		TRUE	},
    {	"magic",		ITEM_MAGIC,		TRUE	},
    {	"nodrop",		ITEM_NODROP,		TRUE	},
    {	"bless",		ITEM_BLESS,		TRUE	},
/*  {   "created",              ITEM_CREATED            FALSE   }, */
/*  {   "unused2",              ITEM_ANTI_EVIL,         FALSE   }, */
/*  {   "unused3",              ITEM_ANTI_GOOD,         FALSE   }, */
    {	"noremove",		ITEM_NOREMOVE,		TRUE	},
    {	"inventory",		ITEM_INVENTORY,		TRUE	},
    {	"nopurge",		ITEM_NOPURGE,		TRUE	},
    {	"rotdeath",		ITEM_ROT_DEATH,		TRUE	},
    {	"visdeath",		ITEM_VIS_DEATH,		TRUE	},
    {   "nonmetal",		ITEM_NONMETAL,		TRUE	},
    {   "nolocate",             ITEM_NOLOCATE,          TRUE    },
    {	"meltdrop",		ITEM_MELT_DROP,		TRUE	},
    {	"hadtimer",		ITEM_HAD_TIMER,		TRUE	},
    {	"sellextract",		ITEM_SELL_EXTRACT,	TRUE	},
    {	"burnproof",		ITEM_BURN_PROOF,	TRUE	},
    {	"nouncurse",		ITEM_NOUNCURSE,		TRUE	},
    {   "noidentify",           ITEM_NO_IDENTIFY,       TRUE    },
    {   "quest",                ITEM_KO_ROT,            TRUE    },
    {   "notseen",		ITEM_NOTSEEN,    	TRUE	},
    {   "noloot",               ITEM_NOLOOT,            TRUE    },
    {   "guildrot",             ITEM_GUILD_ROT,         TRUE    },
    {	NULL,			0,			0	}
};

const struct flag_type ic_extra_flags[] =
{
    {	"",	        	ITEM_GLOW,		TRUE	},
    {	"",	        	ITEM_HUM,		TRUE	},
    {	"",			ITEM_DARK,		TRUE	},
    {	"",	        	ITEM_LOCK,		TRUE	},
    {	"",	        	ITEM_INVIS,		TRUE	},
    {	"",		        ITEM_MAGIC,		TRUE	},
    {	"undroppable",		ITEM_NODROP,		TRUE	},
    {	"",		        ITEM_BLESS,		TRUE	},
    {	"unremoveable",		ITEM_NOREMOVE,		TRUE	},
    {	"",  			ITEM_INVENTORY,		TRUE	},
    {	"",	        	ITEM_NOPURGE,		TRUE	},
    {	"fragile",		ITEM_ROT_DEATH,		TRUE	},
    {	"",		        ITEM_VIS_DEATH,		TRUE	},
    {   "",                     ITEM_NONMETAL,		TRUE	},
    {   "unlocateable",         ITEM_NOLOCATE,          TRUE    },
    {	"ephemeral",		ITEM_MELT_DROP,		TRUE	},
    {	"",		        ITEM_HAD_TIMER,		TRUE	},
    {	"",		        ITEM_SELL_EXTRACT,	TRUE	},
    {	"burnproof",		ITEM_BURN_PROOF,	TRUE	},
    {	"'forever cursed'",     ITEM_NOUNCURSE,		TRUE	},
    {   "unidentifiable",       ITEM_NO_IDENTIFY,       TRUE    },
    {   "'very fragile'",       ITEM_KO_ROT,            TRUE    },
    {   "",	                ITEM_NOTSEEN,    	TRUE	},
    {   "",                     ITEM_NOLOOT,            TRUE    },
    {   "",                     ITEM_GUILD_ROT,         TRUE    },
    {	NULL,			0,			0	}
};



const struct flag_type wear_flags[] =
{
    {	"take",			ITEM_TAKE,		TRUE	},
    {	"finger",		ITEM_WEAR_FINGER,	TRUE	},
    {	"neck",			ITEM_WEAR_NECK,		TRUE	},
    {	"body",			ITEM_WEAR_BODY,		TRUE	},
    {	"head",			ITEM_WEAR_HEAD,		TRUE	},
    {	"legs",			ITEM_WEAR_LEGS,		TRUE	},
    {	"feet",			ITEM_WEAR_FEET,		TRUE	},
    {	"hands",		ITEM_WEAR_HANDS,	TRUE	},
    {	"arms",			ITEM_WEAR_ARMS,		TRUE	},
    {	"shield",		ITEM_WEAR_SHIELD,	TRUE	},
    {	"about",		ITEM_WEAR_ABOUT,	TRUE	},
    {	"waist",		ITEM_WEAR_WAIST,	TRUE	},
    {	"wrist",		ITEM_WEAR_WRIST,	TRUE	},
    {	"wield",		ITEM_WIELD,		TRUE	},
    {	"hold",			ITEM_HOLD,		TRUE	},
    {   "nosac",		ITEM_NO_SAC,		TRUE	},
    {	"wearfloat",		ITEM_WEAR_FLOAT,	TRUE	},
    {   "pride",                ITEM_WEAR_PRIDE,        TRUE    },
/*    {   "twohands",            ITEM_TWO_HANDS,         TRUE    }, */
    {	NULL,			0,			0	}
};

/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] =
{
    {	"none",			APPLY_NONE,		TRUE	},
    {	"strength",		APPLY_STR,		TRUE	},
    {	"dexterity",		APPLY_DEX,		TRUE	},
    {	"intelligence",		APPLY_INT,		TRUE	},
    {	"wisdom",		APPLY_WIS,		TRUE	},
    {	"constitution",		APPLY_CON,		TRUE	},
    {	"sex",			APPLY_SEX,		TRUE	},
    {	"class",		APPLY_CLASS,		TRUE	},
    {	"level",		APPLY_LEVEL,		TRUE	},
    {	"age",			APPLY_AGE,		TRUE	},
    {	"height",		APPLY_HEIGHT,		TRUE	},
    {	"weight",		APPLY_WEIGHT,		TRUE	},
    {	"mana",			APPLY_MANA,		TRUE	},
    {	"hp",			APPLY_HIT,		TRUE	},
    {	"move",			APPLY_MOVE,		TRUE	},
    {	"gold",			APPLY_GOLD,		TRUE	},
    {	"experience",		APPLY_EXP,		TRUE	},
    {	"ac",			APPLY_AC,		TRUE	},
    {	"hitroll",		APPLY_HITROLL,		TRUE	},
    {	"damroll",		APPLY_DAMROLL,		TRUE	},
    {	"saves",		APPLY_SAVES,		TRUE	},
    {	"savingpara",		APPLY_SAVING_PARA,	TRUE	},
    {	"savingrod",		APPLY_SAVING_ROD,	TRUE	},
    {	"savingpetri",		APPLY_SAVING_PETRI,	TRUE	},
    {	"savingbreath",		APPLY_SAVING_BREATH,	TRUE	},
    {	"savingspell",		APPLY_SAVING_SPELL,	TRUE	},
    {	"spellaffect",		APPLY_SPELL_AFFECT,	TRUE	},
    {   "skills",               APPLY_SKILLS,           FALSE   },
    {   "blood",                APPLY_BLOOD,            FALSE   },
    {	NULL,			0,			0	}
};

const struct flag_type ic_apply_flags[] =
{
    {   "nothing",              APPLY_NONE,             TRUE    },
    {   "strength",             APPLY_STR,              TRUE    },
    {   "dexterity",            APPLY_DEX,              TRUE    },
    {   "intelligence",         APPLY_INT,              TRUE    },
    {   "wisdom",               APPLY_WIS,              TRUE    },
    {   "constitution",         APPLY_CON,              TRUE    },
    {   "gender",               APPLY_SEX,              TRUE    },
    {   "class",                APPLY_CLASS,            TRUE    },
    {   "level",                APPLY_LEVEL,            TRUE    },
    {   "age",                  APPLY_AGE,              TRUE    },
    {   "height",               APPLY_HEIGHT,           TRUE    },
    {   "weight",               APPLY_WEIGHT,           TRUE    },
    {   "mana",                 APPLY_MANA,             TRUE    },
    {   "health",               APPLY_HIT,              TRUE    },
    {   "movement",             APPLY_MOVE,             TRUE    },
    {   "gold",                 APPLY_GOLD,             TRUE    },
    {   "experience",           APPLY_EXP,              TRUE    },
    {   "armoring",             APPLY_AC,               TRUE    },
    {   "hitting accuracy",     APPLY_HITROLL,          TRUE    },
    {   "hitting power",        APPLY_DAMROLL,          TRUE    },
    {   "spell resistance",     APPLY_SAVES,            TRUE    },
    {   "paralysis resistance", APPLY_SAVING_PARA,      TRUE    },
    {   "rod resistance",       APPLY_SAVING_ROD,       TRUE    },
    {   "petrification resistance",  APPLY_SAVING_PETRI,TRUE    },
    {	"breath resistance",	APPLY_SAVING_BREATH,	TRUE	},
    {	"spell resistance",	APPLY_SAVING_SPELL,	TRUE	},
    {	"magical effects",	APPLY_SPELL_AFFECT,	FALSE	},
    {   "skills",               APPLY_SKILLS,           FALSE   },
    {   "blood",                APPLY_BLOOD,            FALSE   },
    {	NULL,			0,			0	}
};



/*
 * What is seen.
 */
const struct flag_type wear_loc_strings[] =
{
    {	"in the inventory",	WEAR_NONE,	TRUE	},
    {	"as a light",		WEAR_LIGHT,	TRUE	},
    {	"on the left finger",	WEAR_FINGER_L,	TRUE	},
    {	"on the right finger",	WEAR_FINGER_R,	TRUE	},
    {	"around the neck (1)",	WEAR_NECK_1,	TRUE	},
    {	"around the neck (2)",	WEAR_NECK_2,	TRUE	},
    {	"on the body",		WEAR_BODY,	TRUE	},
    {	"over the head",	WEAR_HEAD,	TRUE	},
    {	"on the legs",		WEAR_LEGS,	TRUE	},
    {	"on the feet",		WEAR_FEET,	TRUE	},
    {	"on the hands",		WEAR_HANDS,	TRUE	},
    {	"on the arms",		WEAR_ARMS,	TRUE	},
    {	"as a shield",		WEAR_SHIELD,	TRUE	},
    {	"about the shoulders",	WEAR_ABOUT,	TRUE	},
    {	"around the waist",	WEAR_WAIST,	TRUE	},
    {	"on the left wrist",	WEAR_WRIST_L,	TRUE	},
    {	"on the right wrist",	WEAR_WRIST_R,	TRUE	},
    {	"wielded",		WEAR_WIELD,	TRUE	},
    {	"held in the hands",	WEAR_HOLD,	TRUE	},
    {	"floating nearby",	WEAR_FLOAT,	TRUE	},
    {   "worn with pride",      WEAR_PRIDE,     TRUE    },
    {	NULL,			0	      , 0	}
};


const struct flag_type wear_loc_flags[] =
{
    {	"none",		WEAR_NONE,	TRUE	},
    {	"light",	WEAR_LIGHT,	TRUE	},
    {	"lfinger",	WEAR_FINGER_L,	TRUE	},
    {	"rfinger",	WEAR_FINGER_R,	TRUE	},
    {	"neck1",	WEAR_NECK_1,	TRUE	},
    {	"neck2",	WEAR_NECK_2,	TRUE	},
    {	"body",		WEAR_BODY,	TRUE	},
    {	"head",		WEAR_HEAD,	TRUE	},
    {	"legs",		WEAR_LEGS,	TRUE	},
    {	"feet",		WEAR_FEET,	TRUE	},
    {	"hands",	WEAR_HANDS,	TRUE	},
    {	"arms",		WEAR_ARMS,	TRUE	},
    {	"shield",	WEAR_SHIELD,	TRUE	},
    {	"about",	WEAR_ABOUT,	TRUE	},
    {	"waist",	WEAR_WAIST,	TRUE	},
    {	"lwrist",	WEAR_WRIST_L,	TRUE	},
    {	"rwrist",	WEAR_WRIST_R,	TRUE	},
    {	"wielded",	WEAR_WIELD,	TRUE	},
    {	"hold",		WEAR_HOLD,	TRUE	},
    {	"floating",	WEAR_FLOAT,	TRUE	},
    {   "pride",        WEAR_PRIDE,     TRUE    },
    {	NULL,		0,		0	}
};

const struct flag_type container_flags[] =
{
    {	"closeable",		1,		TRUE	},
    {	"pickproof",		2,		TRUE	},
    {	"closed",		4,		TRUE	},
    {	"locked",		8,		TRUE	},
    {	"puton",		16,		TRUE	},
    {   "easy",             CONT_EASY,          TRUE    },
    {   "hard",             CONT_HARD,          TRUE    },
    {   "infuriating",      CONT_INFURIATING,   TRUE    },
    {	NULL,			0,		0	}
};

const struct flag_type ic_container_flags[] =
{
    {	"closeable",		1,		TRUE	},
    {	"'its lock cannot be picked'",		2,		TRUE	},
    {	"closed",		4,		TRUE	},
    {	"locked",		8,		TRUE	},
    {	"",                    16,		TRUE	},
    {   "'easily picked'",             CONT_EASY,          TRUE    },
    {   "'difficult to pick'",             CONT_HARD,          TRUE    },
    {   "'very difficult to pick'",      CONT_INFURIATING,   TRUE    },
    {	NULL,			0,		0	}
};


/*****************************************************************************
                      ROM - specific tables:
 ****************************************************************************/




const struct flag_type ac_type[] =
{
    {   "pierce",        AC_PIERCE,            TRUE    },
    {   "bash",          AC_BASH,              TRUE    },
    {   "slash",         AC_SLASH,             TRUE    },
    {   "exotic",        AC_EXOTIC,            TRUE    },
    {   NULL,              0,                    0       }
};


const struct flag_type size_flags[] =
{
    {   "tiny",          SIZE_TINY,            TRUE    },
    {   "small",         SIZE_SMALL,           TRUE    },
    {   "medium",        SIZE_MEDIUM,          TRUE    },
    {   "large",         SIZE_LARGE,           TRUE    },
    {   "huge",          SIZE_HUGE,            TRUE    },
    {   "giant",         SIZE_GIANT,           TRUE    },
    {   NULL,              0,                    0       },
};


const struct flag_type weapon_class[] =
{
    {   "exotic",	WEAPON_EXOTIC,		TRUE    },
    {   "sword",	WEAPON_SWORD,		TRUE    },
    {   "dagger",	WEAPON_DAGGER,		TRUE    },
    {   "spear",	WEAPON_SPEAR,		TRUE    },
    {   "mace",		WEAPON_MACE,		TRUE    },
    {   "axe",		WEAPON_AXE,		TRUE    },
    {   "flail",	WEAPON_FLAIL,		TRUE    },
    {   "whip",		WEAPON_WHIP,		TRUE    },
    {   "polearm",	WEAPON_POLEARM,		TRUE    },
    {   "staff",	WEAPON_STAFF,		TRUE    },
    {   "hand",         WEAPON_HAND,            TRUE    },
    {   "bow",		WEAPON_BOW,		TRUE	},
    {   NULL,		0,			0       }
};


const struct flag_type weapon_type2[] =
{
    {   "flaming",       WEAPON_FLAMING,       TRUE    },
    {   "frost",         WEAPON_FROST,         TRUE    },
    {   "vampiric",      WEAPON_VAMPIRIC,      TRUE    },
    {   "sharp",         WEAPON_SHARP,         TRUE    },
    {   "vorpal",        WEAPON_VORPAL,        TRUE    },
    {   "twohands",      WEAPON_TWO_HANDS,     TRUE    },
    {	"shocking",	 WEAPON_SHOCKING,      TRUE    },
    {	"poison",        WEAPON_POISON,	       TRUE    },
    {   "sticky",        WEAPON_STICKY,        TRUE    },
	{   "angelic",        WEAPON_ANGELIC,        TRUE    },
    {   NULL,              0,                    0     }
};

const struct flag_type ic_weapon_type2[] =
{
    {   "flaming",              WEAPON_FLAMING,         TRUE    },
    {   "frosty",               WEAPON_FROST,           TRUE    },
    {   "vampiric",             WEAPON_VAMPIRIC,        TRUE    },
    {   "sharp",                WEAPON_SHARP,           TRUE    },
    {   "vorpal",               WEAPON_VORPAL,          TRUE    },
    {   "two-handed",           WEAPON_TWO_HANDS,       TRUE    },
    {   "shocking",             WEAPON_SHOCKING,        TRUE    },
    {   "poisoned",             WEAPON_POISON,          TRUE    },
    {   "'unable to be disarmed'",  WEAPON_STICKY,            TRUE    },
	{   "angelic",        WEAPON_ANGELIC,        TRUE    },
    {   NULL,                   0,                      0       }
};

const struct flag_type archery_flags[] =
{
    {   "burning",	 ARROW_BURNING,		TRUE  },
    {   "freezing",	 ARROW_FREEZING,	TRUE  },
    {   "blunt",	 ARROW_KNOCKOUT,	TRUE  },
    {   "heavy",	 ARROW_SLEEP,		TRUE  },
    {   "poison",	 ARROW_POISON,		TRUE  },
    {   "shocking",	 ARROW_SHOCKING,	TRUE  },
    {   "armor piercing",ARROW_PIERCE,		TRUE  },
    {   "love",          ARROW_LOVE,            TRUE  },
    {   NULL,              0,                    0    }
};
const struct flag_type res_flags[] =
{
    {	"summon",	 RES_SUMMON,		TRUE	},
    {   "push",          RES_PUSH,              TRUE    },
    {   "charm",         RES_CHARM,             TRUE    },
    {   "magic",         RES_MAGIC,             TRUE    },
    {   "weapon",        RES_WEAPON,            TRUE    },
    {   "bash",          RES_BASH,              TRUE    },
    {   "pierce",        RES_PIERCE,            TRUE    },
    {   "slash",         RES_SLASH,             TRUE    },
    {   "fire",          RES_FIRE,              TRUE    },
    {   "cold",          RES_COLD,              TRUE    },
    {   "light",	 RES_LIGHT,		TRUE	},
    {   "lightning",     RES_LIGHTNING,         TRUE    },
    {   "acid",          RES_ACID,              TRUE    },
    {   "poison",        RES_POISON,            TRUE    },
    {   "negative",      RES_NEGATIVE,          TRUE    },
    {   "holy",          RES_HOLY,              TRUE    },
    {   "energy",        RES_ENERGY,            TRUE    },
    {   "mental",        RES_MENTAL,            TRUE    },
    {   "disease",       RES_DISEASE,           TRUE    },
    {   "drowning",      RES_DROWNING,          TRUE    },
    {	"sound",	 RES_SOUND,		TRUE	},
    {   "wind",          RES_WIND,              TRUE    },
    {	"wood",	 	 RES_WOOD,		TRUE	},
    {	"silver",	 RES_SILVER,		TRUE	},
    {	"iron",		 RES_IRON,		TRUE	},
/* RES_LAGSKILLS has no function in-game; it is a placeholder here */
    {   "lagskills",     RES_LAGSKILLS,        FALSE    },
    {   NULL,            0,          		0    }
};


const struct flag_type vuln_flags[] =
{
    {	"summon",	 VULN_SUMMON,		TRUE	},
    {   "push",          VULN_PUSH,             TRUE    },
    {	"charm", 	 VULN_CHARM,		TRUE	},
    {   "magic",         VULN_MAGIC,            TRUE    },
    {   "weapon",        VULN_WEAPON,           TRUE    },
    {   "bash",          VULN_BASH,             TRUE    },
    {   "pierce",        VULN_PIERCE,           TRUE    },
    {   "slash",         VULN_SLASH,            TRUE    },
    {   "fire",          VULN_FIRE,             TRUE    },
    {   "cold",          VULN_COLD,             TRUE    },
    {   "light",	 VULN_LIGHT,		TRUE	},
    {   "lightning",     VULN_LIGHTNING,        TRUE    },
    {   "acid",          VULN_ACID,             TRUE    },
    {   "poison",        VULN_POISON,           TRUE    },
    {   "negative",      VULN_NEGATIVE,         TRUE    },
    {   "holy",          VULN_HOLY,             TRUE    },
    {   "energy",        VULN_ENERGY,           TRUE    },
    {   "mental",        VULN_MENTAL,           TRUE    },
    {   "disease",       VULN_DISEASE,          TRUE    },
    {   "drowning",      VULN_DROWNING,         TRUE    },
    {	"sound",	 VULN_SOUND,		TRUE	},
    {   "wind",          VULN_WIND,             TRUE    },
    {   "wood",          VULN_WOOD,             TRUE    },
    {   "silver",        VULN_SILVER,           TRUE    },
    {   "iron",          VULN_IRON,             TRUE    },
/* VULN_LAGSKILLS has no function in-game; it is a placeholder here */
    {   "lagskills",     VULN_LAGSKILLS,       FALSE    },
    {   NULL,              0,                    0       }
};

const struct flag_type position_flags[] =
{
    {   "dead",           POS_DEAD,            FALSE   },
    {   "unconscious",         POS_UNCONSCIOUS,      FALSE   },
    {   "sprawled",        POS_SPRAWLED,         FALSE   },
    {   "sleeping",       POS_SLEEPING,        TRUE    },
    {   "resting",        POS_RESTING,         TRUE    },
    {   "sitting",        POS_SITTING,         TRUE    },
    {   "fighting",       POS_FIGHTING,        FALSE   },
    {   "standing",       POS_STANDING,        TRUE    },
    {   NULL,              0,                    0       }
};

const struct flag_type portal_flags[]=
{
    {   "normal_exit",	  GATE_NORMAL_EXIT,	TRUE	},
    {	"no_curse",	  GATE_NOCURSE,		TRUE	},
    {   "go_with",	  GATE_GOWITH,		TRUE	},
    {   "buggy",	  GATE_BUGGY,		TRUE	},
    {	"random",	  GATE_RANDOM,		TRUE	},
    {   "fatal",          GATE_FATAL,           TRUE    },
    {   "dead_end",       GATE_DEAD_END,        TRUE    },
    {   "drainer",        GATE_DRAINER,         TRUE    },
    {   NULL,		  0,			0	}
};

const struct flag_type furniture_flags[]=
{
    {   "stand_at",	  STAND_AT,		TRUE	},
    {	"stand_on",	  STAND_ON,		TRUE	},
    {	"stand_in",	  STAND_IN,		TRUE	},
    {	"sit_at",	  SIT_AT,		TRUE	},
    {	"sit_on",	  SIT_ON,		TRUE	},
    {	"sit_in",	  SIT_IN,		TRUE	},
    {	"rest_at",	  REST_AT,		TRUE	},
    {	"rest_on",	  REST_ON,		TRUE	},
    {	"rest_in",	  REST_IN,		TRUE	},
    {	"sleep_at",	  SLEEP_AT,		TRUE	},
    {	"sleep_on",	  SLEEP_ON,		TRUE	},
    {	"sleep_in",	  SLEEP_IN,		TRUE	},
    {	"put_at",	  PUT_AT,		TRUE	},
    {	"put_on",	  PUT_ON,		TRUE	},
    {	"put_in",	  PUT_IN,		TRUE	},
    {	"put_inside",	  PUT_INSIDE,		TRUE	},
    {	NULL,		  0,			0	}
};

const	struct	flag_type	apply_types	[]	=
{
	{	"affects",	TO_AFFECTS,	TRUE	},
	{	"object",	TO_OBJECT,	TRUE	},
	{	"immune",	TO_IMMUNE,	TRUE	},
	{	"resist",	TO_RESIST,	TRUE	},
	{	"vuln",		TO_VULN,	TRUE	},
	{	"weapon",	TO_WEAPON,	TRUE	},
	{	NULL,		0,		TRUE	}
};

const	struct	bit_type	bitvector_type	[]	=
{
	{	affect_flags,	"affect"	},
	{	apply_flags,	"apply"		},
	{	imm_flags,	"imm"		},
	{	res_flags,	"res"		},
	{	vuln_flags,	"vuln"		},
	{	weapon_type2,	"weapon"	}
};


const	struct	idscale_type	weight_scale	[]	=
{
    {	1,	"very light"			},
    {	4,	"light"				},
    {	9,	"of moderate weight"		},
    {	19,	"heavy"				},
    {	29,	"very heavy"			},
    {	100,	"extremely heavy"		},
    {   32767,	"too heavy for you"		},
    {	0,	NULL				}
};

const	struct	idscale_type	value_scale	[]	=
{
    {	0,	"worthless"			},
    {	1,	"without much value at all"	},
    {	14,	"with very little value"	},
    {	49,	"with a little value"		},
    {	84,	"somewhat valuable"		},
    {	990,	"valuable"			},
    {	136,	"quite valuable"		},
    {	179,	"incredibly valuable"		},
    {	219,	"phenomenally valuable"		},
    {	32767,	"priceless"			},
    {	0,	NULL				}
};

const	struct	idscale_type	level_scale	[]	=
{
    {	-11,	"seems entirely useless"	},
    {	-6,	"seems a bit useless"		},
    {	-2,	"seems like your kind of thing"	},
    {	2,	"seems a bit complicated"	},
    {	5,	"seems very complicated"	},
    {	9,	"is nearly incomprehensible"	},
    {	32767,	"is beyond comprehension"	},
    {	0,	NULL				}
};

const   struct  idscale_type    splevel_scale     []      =
{
    {   -20,    "very weak"                     },
    {   -11,    "weak"                          },
    {   -5,     "moderate"                      },
    {   0,      "strong"                        },
    {   5,      "very strong"                   },
    {   11,     "powerful"                      },
    {   32767,  "truly powerful"                },
    {   0,      NULL                            }
};

const   struct  idscale_type    contnum_scale     []      =
{
    {	1,	"one"		        	},
    {	10,	"a few"				},
    {   25,	"quite a few"			},
    {   50,	"a lot of"			},
    {   75,	"very many"			},
    {   32767,	"a vast amount of"		},
    {   0,      NULL                            }
};

const   struct  idscale_type    contwgt_scale     []      =
{
    {	5,	"very little"			},
    {   25,	"quite a bit of"		},
    {   50,	"a lot of"			},
    {   75,	"a whole lot of"		},
    {   32767,	"a vast amount of"		},
    {   0,      NULL                            }
};

const   struct  idscale_type    contmlt_scale     []      =
{
    {	0,	"will lighten your load completely"  },
    {   25,	"will lighten your load tremendously"},
    {   50,	"will lighten your load a lot"	     },
    {   75,	"will lighten your load a little"    },
    {   100,	"will not affect your load much"     },
    {   150,	"will increase your load a lot"	     }, 
    {   32767,	"will break your back"               },
    {   0,      NULL                                 }
};

const   struct  idscale_type    weapdam_scale     []      =
{
    {	10,	"very weak"			},
    {	20,	"weak"				},
    {	27,	"average"                       },
    {	33,	"strong"			},
    {	37,	"powerful"			},
    {	42,	"extremely powerful"		},
    {   32767,	"godly"                         },
    {   0,      NULL                            }
};

const   struct  idscale_type    armor_scale         []      =
{
    {   -1,     "vulnerability"			},
    {    0,	"no protection"			},
    {	30,	"barely any protection"		},
    {	50,	"little protection"		},
    {	85,	"fair protection"		},
    {	100,	"good protection"		},
    {	120,	"shielding"			},
    {	150,	"thorough shielding"		},
    {	190,	"tremendous shielding"		},
    {	220,	"fortification"			},
    {	270,	"good fortification"		},
    {   32767,	"tremendous fortification"	},
    {   0,      NULL                            }
};

const   struct  idscale_type    statapply_scale     []      =
{
    {	-4,	"cripples"			},
    {   -1,	"inhibits"			},
    {    3,	"increases"			},
    {    6,	"greatly increases"		},
    {   32767,	"amplifies"			},
    {   0,      NULL                            }
};

const   struct  idscale_type    hpmpapply_scale     []      =
{
    {	-9,	"cripples"			},
    {   -1,	"inhibits"			},
    {   10,	"slightly increases"            },
    {   20,	"increases"			},
    {   30,	"greatly increases"		},
    {   32767,	"amplifies"			},
    {   0,      NULL                            }
};

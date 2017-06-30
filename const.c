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
#include "magic.h"
#include "interp.h"


/* item type list */
const struct item_type		item_table	[]	=
{
    {	ITEM_LIGHT,	"light"		},
    {	ITEM_SCROLL,	"scroll"	},
    {	ITEM_WAND,	"wand"		},
    {   ITEM_STAFF,	"staff"		},
    {   ITEM_WEAPON,	"weapon"	},
    {   ITEM_TREASURE,	"treasure"	},
    {   ITEM_ARMOR,	"armor"		},
    {	ITEM_POTION,	"potion"	},
    {	ITEM_CLOTHING,	"clothing"	},
    {   ITEM_FURNITURE,	"furniture"	},
    {	ITEM_TRASH,	"trash"		},
    {	ITEM_CONTAINER,	"container"	},
    {	ITEM_DRINK_CON, "drink"		},
    {	ITEM_KEY,	"key"		},
    {	ITEM_FOOD,	"food"		},
    {	ITEM_MONEY,	"money"		},
    {	ITEM_BOAT,	"boat"		},
    {	ITEM_CORPSE_NPC,"npc_corpse"	},
    {	ITEM_CORPSE_PC,	"pc_corpse"	},
    {   ITEM_FOUNTAIN,	"fountain"	},
    {	ITEM_PILL,	"pill"		},
    {	ITEM_PROTECT,	"protect"	},
    {	ITEM_MAP,	"map"		},
    {	ITEM_PORTAL,	"portal"	},
    {	ITEM_WARP_STONE,"warp_stone"	},
    {	ITEM_ROOM_KEY,	"room_key"	},
    {	ITEM_GEM,	"gem"		},
    {	ITEM_JEWELRY,	"jewelry"	},
    {   ITEM_JUKEBOX,	"jukebox"	},
    {   ITEM_ARROW,	"arrow"		},
    {   ITEM_SEED,      "seed"          },
    {   ITEM_TREE,      "tree"          },
    {   ITEM_BOOK,      "book"          },
    {   ITEM_PLANT,     "plant"         },
    {   0,		NULL		}
};


/* weapon selection table */
const	struct	weapon_type	weapon_table	[]	=
{
   { "sword",	 OBJ_VNUM_SCHOOL_SWORD,	   WEAPON_SWORD,	&gsn_sword	  },
   { "mace",	 OBJ_VNUM_SCHOOL_MACE,	   WEAPON_MACE, 	&gsn_mace 	  },
   { "dagger",	 OBJ_VNUM_SCHOOL_DAGGER,   WEAPON_DAGGER,	&gsn_dagger	  },
   { "axe",	 OBJ_VNUM_SCHOOL_AXE,	   WEAPON_AXE,	        &gsn_axe	  },
  /* { "staff",	 OBJ_VNUM_SCHOOL_STAFF,	   WEAPON_SPEAR,	&gsn_spear	  },*/
   { "staff",    OBJ_VNUM_SCHOOL_STAFF,    WEAPON_STAFF,        &gsn_staff        },
   { "spear",    OBJ_VNUM_SCHOOL_SPEAR,    WEAPON_SPEAR,        &gsn_spear        },
   { "flail",	 OBJ_VNUM_SCHOOL_FLAIL,	   WEAPON_FLAIL,	&gsn_flail	  },
   { "whip",     OBJ_VNUM_SCHOOL_WHIP,	   WEAPON_WHIP,   	&gsn_whip	  },
   { "polearm",	 OBJ_VNUM_SCHOOL_POLEARM,  WEAPON_POLEARM,	&gsn_polearm      },
   { "knuckles", OBJ_VNUM_SCHOOL_KNUCKLES, WEAPON_HAND ,        &gsn_hand_to_hand },
   { "bow",	 OBJ_VNUM_SCHOOL_KNUCKLES, WEAPON_BOW,		&gsn_bow  },
   { NULL,	 0,			   0,	                NULL		}
};

/* If you modify this table, you have to update the definitions
   which reference the table indices in merc.h.  This is not a good
   way of managing deities, but this table should not have to be
   altered very much in the future. */
const   struct  god_type       god_table      []           = 
 {
/*      name,    rosevnum       descr                aura      */
     { "none",       1001,      "the Triad of Rhia", "n"        }, 
     { "Rhian",        28,      "Rhian",             " golden"    }, 
     { "Tyrin",        27,      "Tyrin",             " pure"  }, 
     { "Syrin",        29,      "Syrin",             " dark"    }, 
};
         
/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
   {    "on",           WIZ_ON,         IM },
   {    "prefix",	WIZ_PREFIX,	IM },
   {    "ticks",        WIZ_TICKS,      IM },
   {    "logins",       WIZ_LOGINS,     IM },
   {    "sites",        WIZ_SITES,      L4 },
   {    "links",        WIZ_LINKS,      L7 },
   {	"newbies",	WIZ_NEWBIE,	IM },
/*      Wiznet spam removed 3/22/08 gkl       */
   {	"mobalerts",	WIZ_MOBALERTS,	IM }, 
   {    "deaths",       WIZ_DEATHS,     IM },
   {    "resets",       WIZ_RESETS,     L4 },
   {    "mobdeaths",    WIZ_MOBDEATHS,  L4 },
   {    "flags",	WIZ_FLAGS,	L5 },
   {	"penalties",	WIZ_PENALTIES,	L5 },
   {	"saccing",	WIZ_SACCING,	L5 },
   {	"levels",	WIZ_LEVELS,	IM },
   {	"load",		WIZ_LOAD,	L2 },
   {	"restore",	WIZ_RESTORE,	L2 },
   {	"snoops",	WIZ_SNOOPS,	L2 },
   {	"switches",	WIZ_SWITCHES,	L2 },
   {	"secure",	WIZ_SECURE,	L1 },
   {    "multiplay",    WIZ_MULTI,      L3 },
   {	"approval",	WIZ_APPROVAL,	IM },
   {    "quest",        WIZ_QUEST,      IM },
   {    "notes",        WIZ_NOTES,      L1 },
   {    "logs",         WIZ_LOGS,       L1 },
   {    "bans",         WIZ_BANS,       IM },
   {    "commerce",	WIZ_COMMERCE,	L1 },
   {    "bugs",		WIZ_BUGS,	L2 },
   {	NULL,		0,		0  }
};

/* attack table  -- not very organized :( */
const 	struct attack_type	attack_table	[MAX_DAMAGE_MESSAGE]	=
{
    { 	"none",		"hit",               -1	             }, /* 0 */
    {	"slice",	"slice", 	     DAM_SLASH       },	
    {   "stab",		"stab",		     DAM_PIERCE      },
    {	"slash",	"slash",	     DAM_SLASH       },
    {	"whip",		"whip",		     DAM_SLASH       },
    {   "claw",		"claw",		     DAM_SLASH       }, /* 5 */  
    {	"blast",	"blast",	     DAM_BASH        },
    {   "pound",	"pound",	     DAM_BASH        },
    {	"crush",	"crush",	     DAM_BASH        },
    {   "grep",		"grep",		     DAM_SLASH       },
    {	"bite",		"bite",		     DAM_PIERCE      }, /* 10 */  
    {   "pierce",	"pierce",	     DAM_PIERCE      },
    {   "suction",	"suction",	     DAM_ENERGY      },
    {	"beating",	"beating",	     DAM_BASH        },
    {   "digestion",	"digestion",	     DAM_ACID        },
    {	"charge",	"charge",	     DAM_BASH        }, /* 15 */ 
    { 	"slap",		"slap",		     DAM_BASH        },
    {	"punch",	"punch",	     DAM_BASH        },
    {	"wrath",	"wrath",	     DAM_ENERGY      },
    {	"magic",	"magic",	     DAM_ENERGY      },
    {   "divine",	"divine power",	     DAM_HOLY        }, /* 20 */
    {	"cleave",	"cleave",	     DAM_SLASH       },
    {	"scratch",	"scratch",	     DAM_PIERCE      },
    {   "peck",		"peck",		     DAM_PIERCE      },
    {   "peckb",	"peck",		     DAM_BASH        },
    {   "chop",		"chop",		     DAM_SLASH       }, /* 25 */
    {   "sting",	"sting",	     DAM_PIERCE      },
    {   "smash",        "smash",	     DAM_BASH        },
    {   "shbite",	"shocking bite",     DAM_LIGHTNING   },
    {	"flbite",	"flaming bite",      DAM_FIRE        },
    {	"frbite",	"freezing bite",     DAM_COLD        }, /* 30 */
    {	"acbite",	"acidic bite", 	     DAM_ACID        },
    {	"chomp",	"chomp",	     DAM_PIERCE      },
    {  	"drain",	"life drain",	     DAM_NEGATIVE    },
    {   "thrust",	"thrust",	     DAM_PIERCE      },
    {   "slime",	"slime",	     DAM_ACID        }, /* 35 */
    {	"shock",	"shock",	     DAM_LIGHTNING   },
    {   "thwack",	"thwack",	     DAM_BASH        },
    {   "flame",	"flame",	     DAM_FIRE        },
    {   "chill",	"chill",	     DAM_COLD        },
    {   "stake",        "stake",             DAM_WOOD        }, /* 40 */
    {   "silbite",      "silvery bite",      DAM_SILVER      },
    {   "mwarp",        "mind warp",         DAM_MENTAL      },
    {   "srage",        "sonic rage",        DAM_SOUND       },
    {   "hrage",        "holy rage",         DAM_HOLY        },
    {   "urage",        "unholy rage",       DAM_NEGATIVE    }, /* 45 */
    {   "arrow",	"arrow",	     DAM_PIERCE      },
    {   "fbite",        "ferric bite",       DAM_IRON        },
    {   "corrosion",    "corrosion",         DAM_IRON        },
    {   "venom",        "venom",             DAM_POISON      },
    {   "rot",          "rot",               DAM_DISEASE     }, /* 50 */
    {   "watblast",     "watery blast",      DAM_DROWNING    },
    {   "ligblast",     "light beam",        DAM_LIGHT       },
    {   "lwrath",	"wrath",	     DAM_LIGHT       },
    {   "gougep",	"gouge",	     DAM_PIERCE      },
    {   "gougeb",	"gouge",	     DAM_BASH        }, /* 55 */
    {   "flurry",       "flurry",            DAM_WIND        },
    {   "bonk",         "bonk",              DAM_BASH        },
    {   "sear",         "sear",              DAM_LIGHT       },
    {   "bchop",        "burning chop",      DAM_FIRE        },
    {   "fchop",        "freezing chop",     DAM_COLD        }, /* 60 */
    {   "echop",        "electrifying chop", DAM_LIGHTNING   },
    {   "vthrust",      "venomous thrust",   DAM_POISON      },
    {   "deluge",       "deluge",            DAM_DROWNING    },
    {   NULL,		NULL,                0               }
};

/* race table */
const 	struct	race_type	race_table	[]		=
{

/*
    {
	name,		pc_race?, remort_race?, visible_race?
	act bits,	aff_by bits,	aff_by2 bits,
        aff_by3 bits,   off bits,       imm,		
        res,		vuln,          form,		
        parts 
    },
*/

    { 
      "null",        FALSE, FALSE, FALSE,
        0,             0,               0,
        0,             0,               0,
        0,             0,               0,             
        0
    },
	
    {
      "avariel",      TRUE, FALSE, TRUE,
        0,              AFF_FLYING,     0,
        0,              0,              0,              
        0,      VULN_WIND,        A|H|M|V,
        A|B|C|D|E|F|G|H|I|J|K|P
    },

    {
      "draconian",    TRUE, FALSE, TRUE,
        0,              AFF_INFRARED,  0,
        0,               0,            0,              
        RES_FIRE,      VULN_SOUND,        A|H|M|V,
        A|B|C|D|E|F|G|H|I|J|K
    },

    {   
      "drow",         TRUE, FALSE, TRUE,
        0,              AFF_INFRARED,  0,
        0,               0,            0,
        RES_CHARM,      VULN_LIGHT,        A|H|M|V,
        A|B|C|D|E|F|G|H|I|J|K
    },

    {
      "dwarf",	TRUE, FALSE, TRUE,
	0,		AFF_INFRARED,  0, 
        0,              0,             0,
	RES_MAGIC, VULN_DROWNING, A|H|M|V,
	A|B|C|D|E|F|G|H|I|J|K
    },

    {
      "half-elf",     TRUE, FALSE, TRUE,
        0,              0,0,0,             0,
        0,              RES_CHARM,         0,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },    

    {
      "werekin",      TRUE, FALSE, TRUE,
        0,              0,             0,0,0,
        0,              0,             0,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
      "kine",         TRUE, FALSE, TRUE,
        0,              0,0,0,             0,
        0,              0,             0,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
      "lich",         TRUE, FALSE, FALSE,
        0,              AFF_DETECT_INVIS|AFF_DETECT_HIDDEN|AFF_DETECT_MAGIC,0,0,
        0,              0,              RES_NEGATIVE,  
        VULN_HOLY,        A|H|M|V,
        A|B|C|D|E|F|G|H|I|J|K|O
    },

    {
      "merfolk",      TRUE, FALSE, TRUE,
        0,              AFF_SWIM,       0,0,0,
        0,              RES_DROWNING,  0,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K|O
    },

    {
      "ogre",         TRUE, FALSE, TRUE,
        0,              0,              0,0,0,
        0,              RES_FIRE|RES_COLD,  VULN_MENTAL,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K|Y
    },

    {
      "orc",	        TRUE, FALSE, TRUE,
	0,		AFF_INFRARED,	0,0,0,
	0,		RES_DISEASE|RES_POISON,	VULN_MENTAL,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|Y
    },      

    {
      "sylvan",       TRUE, FALSE, TRUE,
        0,              AFF_INFRARED,   0,0,0,
        0,              RES_CHARM|RES_MENTAL|RES_MAGIC, VULN_IRON,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
      "githyanki",    TRUE, FALSE, TRUE,
        0,              AFF_INVISIBLE, 0,0,0,
        0,              RES_LIGHTNING, VULN_HOLY,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
      "kender",       TRUE, FALSE, TRUE,
        0,              AFF_DETECT_HIDDEN, 0,0,0,
        0,              RES_MENTAL,          0,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },
	
	 {
	   "seraph",      TRUE, TRUE, FALSE,
        0,              AFF_FLYING|AFF_DETECT_HIDDEN|AFF_DETECT_INVIS|AFF_DETECT_MAGIC,     0,
        0,              0,              0,              
        RES_HOLY,      VULN_NEGATIVE,        A|H|M|V,
        A|B|C|D|E|F|G|H|I|J|K|P
    },


    {
      "troll",	FALSE, FALSE, FALSE,
	0,		0,		0,0,0,
	0,		RES_FIRE|RES_COLD,	VULN_LIGHTNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|U|V
    },

    { 
      "human",        FALSE,  FALSE, FALSE,
	0,		0, 		0,0,0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
      "elf",	        FALSE, FALSE, FALSE,
	0,		AFF_INFRARED,	0,0,0,
	0,		RES_CHARM,	VULN_IRON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
      "bat",		FALSE, FALSE, FALSE,
	0,		AFF_FLYING|AFF_DARK_VISION,0,0,	OFF_DODGE|OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|P
    },

    {
      "bear",		FALSE, FALSE, FALSE,
	0,		0,0,0,		OFF_CRUSH|OFF_DISARM|OFF_BERSERK,
	0,		RES_BASH|RES_COLD,	0,
	A|G|V,		A|B|C|D|E|F|H|J|K|U|V
    },

    {
      "cat",		FALSE, FALSE, FALSE,
	0,		AFF_DARK_VISION,0,0,	OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|U|V
    },

    {
      "centipede",	FALSE, FALSE, FALSE,
	0,		AFF_DARK_VISION,0,0,	0,
	0,		RES_PIERCE|RES_COLD,	VULN_BASH,
 	A|B|G|O,	A|C|K	
    },

    {
      "dog",		FALSE, FALSE, FALSE,
	0,		0,0,0,		OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|U|V
    },

    {
      "doll",		FALSE, FALSE, FALSE,
	0,		0,0,0,		0,
	IMM_COLD|IMM_POISON|IMM_HOLY|IMM_NEGATIVE|IMM_MENTAL|IMM_DISEASE
	|IMM_DROWNING,	RES_BASH,
	VULN_SLASH|VULN_FIRE|VULN_ACID|VULN_LIGHTNING|VULN_ENERGY,
	E|J|M|cc,	A|B|C|G|H|K
    },

    { 	"dragon", 	FALSE,  FALSE, FALSE,
	0, 			AFF_INFRARED|AFF_FLYING,0,0,	0,
	0,			RES_FIRE|RES_BASH|RES_CHARM, 
	VULN_PIERCE|VULN_COLD,
	A|H|Z,		A|C|D|E|F|G|H|I|J|K|P|Q|U|V|X
    },

    {
      "fido",		FALSE, FALSE, FALSE,
	0,		0,0,0,		OFF_DODGE|ASSIST_RACE,
	0,		0,		VULN_MAGIC,
	A|B|G|V,	A|C|D|E|F|H|J|K|Q|V
    },		
   
    {
      "fox",		FALSE, FALSE, FALSE,
	0,		AFF_DARK_VISION,0,0,	OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|V
    },

    {
      "gnome",       FALSE, FALSE, FALSE,
         0,             AFF_INFRARED,0,0,   0,
         0,             RES_MAGIC,      VULN_DROWNING,
         A|H|M|V,       A|B|C|D|E|F|G|H|I|J|K
    },

    {
      "goblin",       FALSE, FALSE, FALSE,
	0,		AFF_INFRARED,0,0,	0,
	0,		RES_DISEASE,	VULN_DROWNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { 
      "halfling",    FALSE, FALSE, FALSE,
         0,             0,0,0,              0,
         0,             0,              0,
         A|H|M|V,       A|B|C|D|E|F|G|H|I|J|K
    },


    {
      "hobgoblin",	FALSE, FALSE, FALSE,
	0,		AFF_INFRARED,0,0,	0,
	0,		RES_DISEASE|RES_POISON,	0,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K|Y
    },

    {
      "kobold",	FALSE, FALSE, FALSE,
	0,		AFF_INFRARED,0,0,	0,
	0,		RES_POISON,	VULN_MAGIC,
	A|B|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|Q
    },

    {
      "lizard",	FALSE, FALSE, FALSE,
	0,		0,0,0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|X|cc,	A|C|D|E|F|H|K|Q|V
    },

    {
      "modron",	FALSE, FALSE, FALSE,
	0,		AFF_INFRARED,0,0,	ASSIST_RACE|ASSIST_ALIGN,
	IMM_CHARM|IMM_DISEASE|IMM_MENTAL|IMM_HOLY|IMM_NEGATIVE,
			RES_FIRE|RES_COLD|RES_ACID,	0,
	H,		A|B|C|G|H|J|K
    },

    {
      "pig",		FALSE, FALSE, FALSE,
	0,		0,0,0,		0,
	0,		0,		0,
	A|G|V,	 	A|C|D|E|F|H|J|K
    },	

    {
      "rabbit",	FALSE, FALSE, FALSE,
	0,		0,0,0,		OFF_DODGE|OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K
    },
    
    {
      "school monster",	FALSE, FALSE, FALSE,
	0,		0,0,0,		0,
	IMM_CHARM|IMM_SUMMON,	0,		VULN_MAGIC,
	A|M|V,		A|B|C|D|E|F|H|J|K|Q|U
    },	

    {
      "snake",	FALSE, FALSE, FALSE,
	0,		0,0,0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|X|Y|cc,	A|D|E|F|K|L|Q|V|X
    },
 
    {
      "song bird",	FALSE, FALSE, FALSE,
	0,		AFF_FLYING,0,0,	OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|W,		A|C|D|E|F|H|K|P
    },

    {
      "giant",	FALSE, FALSE, FALSE,
	0,		AFF_REGENERATION|AFF_INFRARED|AFF_DETECT_HIDDEN,0,0,
	OFF_BERSERK,
 	0,	        RES_BASH,	VULN_FIRE|VULN_ACID,
	A|B|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
      "water fowl",	FALSE, FALSE, FALSE,
	0,		AFF_SWIM|AFF_FLYING,0,0,	0,
	0,		RES_DROWNING,		0,
	A|G|W,		A|C|D|E|F|H|K|P
    },		
  
    {
      "wolf",		FALSE, FALSE, FALSE,
	0,		AFF_DARK_VISION,0,0,	OFF_FAST|OFF_DODGE,
	0,		0,		0,	
	A|G|V,		A|C|D|E|F|J|K|Q|V
    },

    {
      "wyvern",	FALSE, FALSE, FALSE,
	0,		AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN,0,0,
	OFF_BASH|OFF_FAST|OFF_DODGE,
	IMM_POISON,	0,          0,
	A|B|G|Z,		A|C|D|E|F|H|J|K|Q|V|X
    },

    {
      "unique",	FALSE, FALSE, FALSE,
	0,		0,		0,
	0,		0,		0,		
	0,              0,              0,
        0
    },

    {
      "DONE",	FALSE, FALSE, FALSE,
	0,		0,		0,
	0,		0,		0,		
	0,		0,              0,
        0
    }

    };

const	struct	pc_race_type	pc_race_table	[]	=
{

/*
    {
	"race name", 	short name, 	points,	{ class multipliers },
	{ bonus skills },
	{ base stats },		{ max stats },		size 
    },
*/

    { 
         "null",          "    ",        0,     
         { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
         {""},
         {12, 12, 12, 12, 12 }, { 18, 18, 18, 18, 18 }, SIZE_TINY
    },
	
	
    {    
         "avariel",       "Avrl",         6,     
         { 105, 125, 125, 125, 150, 105, 110, 115, 115, 105, 105, 105, 140, 125 },
         {"sneak", "hide"},
         { 13, 15, 15, 17, 8 }, { 17, 20, 21, 22, 14 }, SIZE_MEDIUM
    },

    {
        "draconian",      "Draco",        6,
        { 110, 125, 125, 110, 115, 150, 110, 150, 115, 120, 110, 102, 145, 135 },
        { "fast healing", "meditation" },
        { 14, 14, 12, 14, 13 }, { 20, 20, 16, 19, 19 }, SIZE_HUGE
    },

    { 
         "drow",          "Drow",        6,     
         { 105, 150, 130, 110, 150, 105, 130, 150, 105, 110, 105, 102, 110, 130 },
         {"sneak", "hide", "weather"},
         { 10, 15, 12, 18, 9 }, { 16, 20, 18, 21, 16 }, SIZE_SMALL
    },

    {
	"dwarf",          "Dwarf",	 6,
        { 175, 105, 105, 105, 110, 200, 150, 115, 125, 150, 175, 120, 160, 105 },
	{ "battle focus", "haggle", "fast healing"},
	{ 14, 11, 14, 11, 17 },	{ 20, 17, 20, 17, 20 }, SIZE_MEDIUM
    },



    {
        "half-elf",       "H-Elf",       4,     
        { 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 120, 110 },
        {"sneak", "hide"},
        { 12, 14, 14, 14, 12 }, { 18, 20, 20, 20, 18 }, SIZE_LARGE
    },

    {
        "werekin",        "Wkin",        3,    
        { 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 130, 110 },
        {"lore"},
        { 14, 13, 13, 13, 13 }, { 20, 18, 18, 19, 20 }, SIZE_LARGE
    },

    {
        "kine",           "Kine",        2,      
        { 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105 },
        {"lore"},
        { 13, 13, 13, 13, 13 }, { 19, 19, 19, 19, 19 }, SIZE_LARGE
    },

    {  
        "lich",        "Lich",        8,      
        { 115, 130, 170, 200, 160, 105, 150, 150, 125, 115, 110, 105, 110, 115 },
        {"meditation", "fast healing"},
        { 13, 17, 14, 14, 13 }, { 17, 22, 20, 19, 18 }, SIZE_MEDIUM
    },
  
    {
        "merfolk",        "Merfk",        5,      
        { 105, 105, 105, 115, 105, 105, 105, 125, 105, 105, 105, 115, 160, 110 },
        {"meditation"},
        { 14, 12, 13, 12, 15 }, { 19, 20, 19, 17, 19 }, SIZE_LARGE
    },

    {
        "ogre",           "Ogre",         6,      
        { 200, 200, 110, 110, 105, 200, 200, 115, 150, 200, 200, 150, 200, 150 },
        {"enhanced damage", "bash", "battle focus"},        
        { 19, 10, 10, 10, 19 },    { 22, 16, 16, 16, 22 }, SIZE_HUGE
    },

    {
        "orc",            " Orc ",        5,      
        { 140, 115, 110, 105, 110, 150, 115, 110, 110, 125, 120, 110, 115, 125 },
        {"battle focus", "enhanced damage", "sneak"},       
        { 15, 11, 10, 16, 16 },   { 20, 17, 16, 20, 20 }, SIZE_LARGE
    },
  
    {
        "sylvan",         "Sylv",       6,      
        { 105, 105, 125, 110, 150, 105, 115, 115, 110, 105, 110, 115, 200, 110 },
        {"sneak", "hide"},
        { 10, 18, 13, 15, 9 },  { 16, 21, 18, 20, 15 }, SIZE_SMALL
    },

   {
        "githyanki",          "Gith",       7,
        { 165, 400, 145, 100, 210, 100, 100, 300, 175, 200, 205, 110, 140, 300 },
        { "meditation"},
        { 14, 17, 14, 15, 14 }, { 19, 21, 18, 18, 18 }, SIZE_LARGE
    },

   {
        "kender",           "Kend",        4,
        { 235, 205, 200, 165, 300, 210, 125, 145, 100, 115, 100, 300, 300, 175 },
        {"steal", "hide"},
        { 10, 17, 11, 16, 8 }, { 15, 20, 16, 22, 17 }, SIZE_SMALL
    },
	
	 {    
         "seraph",       "Sera",         6,     
         { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
         {"sneak", "hide"},
         { 14, 14, 14, 14, 14 }, { 20, 20, 20, 20, 20 }, SIZE_MEDIUM
    }
	


};

	
      	

/*
 * Class table.
 */
const	struct	class_type	class_table	[MAX_CLASS]	=
{
    {
	"mage",       "Mag", "r",  STAT_INT, STAT_WIS,  OBJ_VNUM_SCHOOL_STAFF,
	{ 3018, 9618 },  75,  20, 0,  8,  10, TRUE,
	"mage basics", "mage default",
        2,      &gsn_vorpalspell
    },

    {
	"cleric",     "Cle", "C",  STAT_WIS, STAT_INT,  OBJ_VNUM_SCHOOL_MACE,
	{ 3003, 9619 },  75,  20, -5,  10, 12, TRUE,
	"cleric basics", "cleric default",
        2,      &gsn_vorpalspell
    },

    {
	"monk",       "Mon", "W",  STAT_DEX, STAT_WIS,  0,
	{ 3340, 9717 },  75,  15, -24,  12, 15, FALSE,
	"monk basics", "monk default",
        3,      &gsn_vorpalspell
    },

    {
	"warrior",    "War", "R",  STAT_STR, STAT_DEX,  OBJ_VNUM_SCHOOL_SWORD,
	{ 3022, 9633 },  75,  16,  -24,  14, 16, FALSE,
	"warrior basics", "warrior default",
        4,      &gsn_vorpalspell
    },

    {
        "barbarian",  "Bar", "y",  STAT_CON, STAT_STR,  OBJ_VNUM_SCHOOL_AXE,
        { 3333, 9712 }, 75,   25,  -25,  15, 17, FALSE, 
        "barbarian basics", "barbarian default",
        5,      NULL
    },
    
    { 
        "psionicist", "Psi", "m",  STAT_INT, STAT_CON,  OBJ_VNUM_SCHOOL_WHIP,
        { 3008, 9714 }, 75,   20,    0,   9,  12,  TRUE,  
        "psionicist basics", "psionicist default",
        2,      &gsn_vorpalspell
    },

    {
        "druid",      "Dru", "G",  STAT_WIS, STAT_DEX, OBJ_VNUM_SCHOOL_POLEARM,
        { 3342, 9716 }, 75,   20,   -5,  9,  12,  TRUE,
        "druid basics",       "druid default",
        2,      &gsn_vorpalspell
    },
    
    {
        "ranger",     "Ran", "g",  STAT_WIS, STAT_STR,  OBJ_VNUM_SCHOOL_SPEAR,     
        { 3337, 9751 }, 75,   16,  -24,  14, 16,  FALSE,
        "ranger basics",      "ranger default",
        5,      &gsn_vorpalspell
    },
  
    {
        "rogue",      "Rog", "D",  STAT_DEX, STAT_CON,  OBJ_VNUM_SCHOOL_DAGGER,
        { 3028, 9639 }, 75,   18,   -18,  11,  14,  FALSE,
        "rogue basics",       "rogue default",
        3,      NULL
    },
    
    { 
        "bard",       "Brd", "M",  STAT_DEX, STAT_INT,  OBJ_VNUM_SCHOOL_FLAIL,
        { 3338, 9708 }, 75,   16,   -16,  10,  13,  TRUE,
        "bard basics",        "bard default",
        3,      &gsn_vorpalspell
    },
    
    {
        "wildmage",   "Wmg", "C",  STAT_INT, STAT_DEX,  OBJ_VNUM_SCHOOL_WHIP,
	{ 3080, 1202 },  75,  20, 6,  8,  11, TRUE,
	"wild basics", "wild default",
        2,      &gsn_vorpalspell
    },

    {
        "warlock",   "Wlk", "b",  STAT_INT, STAT_STR,  OBJ_VNUM_SCHOOL_POLEARM,
	{ 3081, 1202 },  75,  19,   -17,  10,  13, TRUE,
	"warlock basics", "warlock default",
        3,      &gsn_vorpalspell
    },
    
    {
        "necromancer", "Nec", "Y", STAT_INT, STAT_WIS, OBJ_VNUM_SCHOOL_STAFF,
        {3351, 9570}, 75, 20, 0, 8, 10, TRUE,
	"necro basics", "necro default",
        2,      &gsn_vorpalspell
    },

    {   "templar", "Tem", "w", STAT_CON, STAT_WIS, OBJ_VNUM_SCHOOL_SPEAR,
	{3058, 1226}, 75, 16, -24, 14, 16, FALSE,
	"templar basics", "templar default",
        3,      &gsn_vorpalspell
    },
};

/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[28]		=
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },  /* 1  */
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },  /* 3  */
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0, 100,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 115, 11 },
    {  0,  0, 130, 12 },
    {  0,  0, 130, 13 }, /* 13  */
    {  0,  1, 140, 14 },
    {  1,  1, 150, 15 }, /* 15  */
    {  1,  2, 165, 16 },
    {  2,  3, 180, 22 },
    {  2,  3, 200, 25 }, /* 18  */
    {  3,  4, 225, 30 },
    {  3,  5, 250, 35 }, /* 20  */
    {  4,  6, 300, 40 },
    {  4,  6, 350, 45 },
    {  5,  7, 400, 50 },
    {  5,  8, 450, 55 },
    {  6,  9, 500, 60 },  /* 25   */
    {  7, 10, 550, 65 },
    {  8, 11, 600, 70 }
};



const	struct	int_app_type	int_app		[28]		=
{
    {  3 },	/*  0 */
    {  5 },	/*  1 */
    {  7 },
    {  8 },	/*  3 */
    {  9 },
    { 10 },	/*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },	/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },	/* 15 */
    { 34 },
    { 37 },
    { 40 },	/* 18 */
    { 44 },
    { 49 },	/* 20 */
    { 55 },
    { 60 },
    { 70 },
    { 80 },
    { 85 },	/* 25 */
    { 90 },
    { 95 }
};



const	struct	wis_app_type	wis_app		[28]		=
{
    { 0 },	/*  0 */
    { 0 },	/*  1 */
    { 0 },
    { 0 },	/*  3 */
    { 0 },
    { 1 },	/*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 1 },	/* 10 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 2 },	/* 15 */
    { 2 },
    { 2 },
    { 3 },	/* 18 */
    { 3 },
    { 3 },	/* 20 */
    { 3 },
    { 4 },
    { 4 },
    { 4 },
    { 5 },	/* 25 */
    { 5 },
    { 6 }
};



const	struct	dex_app_type	dex_app		[28]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 },    /* 25 */
    { -130 },
    { -140 }
};


const	struct	con_app_type	con_app		[28]		=
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -2, 30 },
    { -2, 35 },	  /*  3 */
    { -1, 40 },
    { -1, 45 },   /*  5 */
    { -1, 50 },
    {  0, 55 },
    {  0, 60 },
    {  0, 65 },
    {  0, 70 },   /* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  0, 88 },
    {  1, 90 },   /* 15 */
    {  2, 95 },
    {  2, 97 },
    {  3, 99 },   /* 18 */
    {  3, 99 },
    {  4, 99 },   /* 20 */
    {  4, 99 },
    {  5, 99 },
    {  6, 99 },
    {  7, 99 },
    {  8, 99 },    /* 25 */
    {  9, 99 },
    { 10, 99 }
};

/*
 * Wand/Staff Table
 * for special zap/brandish flavor text
 *
 * do NOT change the ordering of the items in this table or else
 * items already built in the game WILL be messed up.  Wand/staff
 * v4 values correspond irectly to the indices of this table's
 * contents, so if you NEED to change anything other than the
 * last entry of this table, add a line into the area file loading
 * routines in db.c to convert old area wand/staff v4's into new
 * values, asave world, then that's that.
 * 
 */

const   struct wandstaff_type   wandstaff_table [MAX_WANDSTAFF]      =
{
/*
name
type
wand_zap_ch_c "You zap $N with $p."
wand_zap_ch_v "$n zaps you with $p."
wand_zap_ch_o "$n zaps $N with $p."
wand_zap_ob_c "You zap $P with $p."
wand_zap_ob_o "$n zaps $P with $p."
wand_fail_c "Your efforts with $p produce only smoke and sparks."
wand_fail_o "$n's efforts with $p produce only smoke and sparks."
wand_die_c "Your $p explodes into fragments."
wand_die_o "$n's $p explodes into fragments."
staff_brand_c "You brandish $p."
staff_brand_o "$n brandishes $p."
staff_fail_c "You fail to invoke $p."
staff_fail_o "...and nothing happens."
staff_die_c "Your $p blazes bright and is gone."
staff_die_o "$n's $p blazes bright and is gone."
*/
    {
        "default",
        "You zap $N with $p.",
        "$n zaps you with $p.",
        "$n zaps $N with $p.",
        "You zap $P with $p.",
        "$n zaps $P with $p.",
        "Your efforts with $p produce only smoke and sparks.",
        "$n's efforts with $p produce only smoke and sparks.",
        "$p explodes into fragments.",
        "$p explodes into fragments.",
        "You brandish $p.",
        "$n brandishes $p.",
        "You fail to invoke $p.",
        "...and nothing happens.",
        "$p blazes bright and is gone.",
        "$p blazes bright and is gone."
    },
    {
        "instrument",
        "You play $p for $N.",
        "$n plays $p for you.",
        "$n plays $p for $N.",
        "You play $p for $P.",
        "$n plays $p for $P.",
        "...but miss a note.",
        "...but misses a note.",
        "$p falls to pieces.",
        "$p falls apart in $n's hands.",
        "You play $p.",
        "$n plays $p.",
        "...but you miss a note.",
        "...but misses a note.",
        "$p falls to pieces.",
        "$p falls apart in $n's hands.",
    },

};

/*
 * Liquid properties.
 * Used in world.obj.
 */
const	struct	liq_type	liq_table	[MAX_LIQUID]	=
{
/*    name			color    proof, serving size DISABLED? */
    { "water",			"clear",	{   0, 16 }, 0 },
    { "beer",			"amber",	{  12, 12 }, 0 },
    { "red wine",		"burgundy",	{  30,  5 }, 0 },
    { "ale",			"brown",	{  15, 12 }, 0 },
    { "dark ale",		"dark",		{  16, 12 }, 0 },
    { "whisky",			"golden",	{ 120,  2 }, 0 },
    { "lemonade",		"pink",		{   0, 12 }, 0 },
    { "firebreather",		"boiling",	{ 190,  2 }, 0 },
    { "local specialty",	"clear",	{ 151,  2 }, 0 },
    { "slime mold juice",	"thick, green",	{   0,  2 }, 0 },

    { "milk",			"white",	{   0, 12 }, 0 },
    { "tea",			"tan",		{   0,  6 }, 0 },
    { "coffee",			"black",	{   0,  6 }, 0 },
    { "blood",			"red",		{   0,  6 }, 0 },
    { "salt water",		"clear",	{   0,  1 }, 0 },
    { "fountain pop",		"brown",	{   0, 12 }, 0 }, 
    { "root beer",		"brown",	{   0, 12 }, 0 },
    { "elvish wine",		"green",	{  35,  5 }, 0 },
    { "white wine",		"golden",	{  28,  5 }, 0 },
    { "champagne",		"golden",	{  32,  5 }, 0 },

    { "mead",			"honey-colored",{  34, 12 }, 0 },
    { "rose wine",		"pink",		{  26,  5 }, 0 },
    { "brown gravy",	        "thick, brown",	{   0,  5 }, 0 },
    { "vodka",			"clear",	{ 130,  2 }, 0 },
    { "cranberry juice",	"red",		{   0, 12 }, 0 },
    { "orange juice",		"orange",	{   0, 12 }, 0 }, 
    { "absinthe",		"green",	{ 200,  2 }, 0 },
    { "brandy",			"golden",	{  80,  4 }, 0 },
    { "gin",		        "clear",	{  80,  2 }, 0 },
    { "schnapps",		"clear",	{  90,  2 }, 0 },

    { "icewine",		"purple",	{  50,  5 }, 0 },
    { "amontillado",		"burgundy",	{  35,  5 }, 0 },
    { "sherry",			"red",		{  38,  5 }, 0 },	
    { "Gander ale",		"amber",	{  15, 12 }, 0 },
    { "rum",			"amber",	{ 151,  2 }, 0 },
    { "cordial",		"clear",	{ 100,  2 }, 0 },
    { "cocoa",		        "brown",	{   0,  6 }, 0 },
    { "apple juice",		"amber",	{   0, 12 }, 0 },
    { "strawberry banana juice", "pinkish",	{   0, 12 }, 0 },
    { "grape juice",		"purple",	{   0, 12 }, 0 },

    { "blackberry wine",	"burgundy",	{  50,  6 }, 0 },
    { "cider",		        "amber",	{   0, 10 }, 0 },
    { "chocolate milk",		"brown",	{   0, 12 }, 0 },
    { "bleach",		        "clear",	{   0,  6 }, 0 },
    { "iced tea",		"tan",	        {   0, 12 }, 0 },
    { "iced coffee",		"brown",	{   0,  8 }, 0 },    
    { "rye",                    "brown",        {  80,  5 }, 0 },
    { "bourbon",                "brown",        {  80,  5 }, 0 },
    { "scotch",                 "amber",        {  80,  5 }, 0 },
    { "light rum",              "clear",        {  80,  5 }, 0 },

    { "life essence",           "misty",        { 200,  8 }, 0 },
    { "sludge",                 "foul",         {   0,  8 }, 0 },
    { "cognac",                 "brown",        {  80,  4 }, 0 },
    { "nettle soup",      "rich, forest green", {   0, 12 }, 0 },
    { "eggnog",               "buttery yellow", {  40,  8 }, 0 },
    { "spiced wine",            "earthy brown", {  25,  5 }, 0 },
    { "Romany spiced wine",     "earthy brown", {  30,  5 }, 0 },
    { "mulled ale",                    "amber", {  15, 12 }, 0 },
    { "githyanki hooch",       "ethereal grey", {   0, 12 }, 0 },
    { "liquid flame",            "burning red", {   0, 12 }, 0 },

    { "tiger blood",		"red",		{   0,  6 }, 0 },
    { "vampire blood",          "swirling red", {   0,  6 }, 0 },
    { "creme de violette",      "purple",       {   0,  6 }, 1 },
    { "creme yvette",           "purple",       {   0,  6 }, 1 },
    { "curacao",                "orange",       {   0,  6 }, 1 },
    { "galliano",               "orange",       {   0,  6 }, 1 },
    { "godiva",                 "brown",        {   0,  6 }, 1 },
    { "goldwasser",             "clear",        {   0,  6 }, 1 },
    { "grand marnier",          "orange",       {   0,  6 }, 1 },
    { "kummel",                 "yellow",       {   0,  6 }, 1 },

    { "midori",                 "green",        {   0,  6 }, 1 },
    { "ouzo",                   "clear",        {   0,  6 }, 1 },
    { "sambuca",                "clear",        {   0,  6 }, 1 },
    { "bitter",                 "amber",        {   0,  6 }, 1 },
    { "extra special bitter",   "amber",        {   0,  6 }, 1 },
    { "extra bitter",           "amber",        {   0,  6 }, 1 },
    { "pale ale",               "amber",        {  16, 12 }, 0 },
    { "lambic",                 "amber",        {   0,  6 }, 1 },
    { "porter",                 "black",        {   0,  6 }, 1 },
    { "stout",                  "black",        {   0,  6 }, 1 },
    { "hefeweizen",             "amber",        {   0,  6 }, 1 },

    { "pilsner",                "yellow",       {   0,  6 }, 1 },
    { "bock",                   "amber",        {   0,  6 }, 1 },
    { "amber bock",             "amber",        {   0,  6 }, 1 },
    { "dubbel bock",            "brown",        {   0,  6 }, 1 },
    { "marzen",                 "amber",        {   0,  6 }, 1 },
    { "oktoberfest",            "amber",        {   0,  6 }, 1 },
    { "schwarzbier",            "black",        {   0,  6 }, 1 },
    { NULL,			NULL,		{   0,  0 }, 1 }
};



/*
 * The skill and spell table.
 * Spell and skill slots are no longer used in the Dark Risings code, so they can
 * be doubled up upon, all set to zero, or whatever else anyone may desire.
 * Instead of saving spell slot numbers into the area files as stock ROM did, DR
 * actually saves the spell's name string in files - this makes for larger area
 * files and a more CPU intensive bootup, but it is less troublesome to add new
 * spells.  5/31/07 gkl
 */
 
 /* spell name, whether or not it's selectable at creation, levels, ignore (see above), spell function name, target, position, GSN (can be NULL), mana cost, lag time, damage message, fade message,  */
#define SLOT(n)	n

const	struct	skill_type	skill_table	[MAX_SKILL]	=
{
    {
        "reserved", FALSE,
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
        0,                             TAR_IGNORE,                POS_STANDING,
        NULL,                             0,   0,                           "",
        "",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "abjuration", TRUE,
        { 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_abjuration,              TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_abjuration,                 20,  12,              "burning flesh",
        "The runes covering your skin fade and the burning stops.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "acid blast", TRUE,
        { 36, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1 },
        spell_acid_blast,              TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_acid_blast,                 20,  12,                 "acid blast",
        "!Acid Blast!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "acid breath", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  1 },
        spell_acid_breath,             TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                             9,   4,              "blast of acid",
        "!Acid Breath!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "adrenaline rush", TRUE,
        { 53, 53, 53, 53, 53, 27, 53, 53, 53, 53, 53, 28, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2, 53,  1 },
        spell_adrenaline_rush,         TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            30,  12,                           "",
        "You feel the adrenaline leaving your veins.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "animate dead", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 15, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_animate_dead,            TAR_CHAR_SELF,             POS_STANDING,
        &gsn_animate_dead,               50,  12,               "animate dead",
        "Arcane life departs, and $n crumbles into dust.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "apple goodness", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 53,  1 },
        spell_apple,                   TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            50,  12,                           "",
        "",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "armor", TRUE,
        {  3,  3,  3,  5,  6, 53,  3,  5,  7,  5,  3, 53,  1,  5 },
        {  1,  1,  2,  1,  3,  0,  1,  3,  3,  2,  1,  2,  1,  1 },
        spell_armor,                   TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                             5,  12,                           "",
        "You feel less armored.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "astral projection", TRUE,
        { 53, 53, 53, 53, 53, 24, 53, 53, 53, 53, 53, 37, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  2, 53,  1 },
        spell_astral_projection,       TAR_CHAR_SELF,             POS_STANDING,
        NULL,                            25,  12,                           "",
        "Your projections fade away.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "atrophy", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 40, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_atrophy,                 TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_atrophy,                    15,  12,                "putrid form",
        "Your body returns to normal.",
        "$n seems much healthier now.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "aura of pain", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_aura_pain,                TAR_CHAR_SELF,             POS_STANDING,
        &gsn_aura_pain,                 20,  12,                 "aura of pain",
        "Your aura of pain fades.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "aura parilitatis", FALSE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_guildspell,              TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            10,  24,                           "",
        "The glory of your guild leaves you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "awe", TRUE,
        { 53, 53, 53, 53, 53, 15, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_awe,                     TAR_IGNORE,                POS_FIGHTING,
        NULL,                            75,  12,                           "",
        "Your sense of awe leaves you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "banana ward", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 53,  1 },
        spell_banana,                  TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            50,  12,                           "",
        "You no longer feel as protected.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "battle sorrow", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53,  8, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_battle_sorrow,           TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            10,   8,            "sorrowful mourn",
        "!Battle Sorrow!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "battle wail", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 27, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_battle_wail,             TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            16,   8,                 "eerie wail",
        "!Battle Wail!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "battlecry", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 14, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_battlecry,               TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            14,   8,                  "battlecry",
        "!Battlecry!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "betray", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 24, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  3,  1,  1,  1, 53,  1 },
        spell_betray,                  TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            10,   8,                           "",
        "You feel more self-confident.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "biofeedback", TRUE,
        { 53, 53, 53, 53, 53, 35, 53, 53, 53, 53, 53, 36, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2, 53,  1 },
        spell_biofeedback,             TAR_CHAR_SELF,             POS_STANDING,
        NULL,                            75,  12,                           "",
        "Your biofeedback fades.",
        "$n's biofeedback fades.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "bless", TRUE,
        { 53,  9, 11, 53, 53, 53, 10, 12, 53, 53, 53, 53, 53,  9 },
        {  0,  1,  2,  0,  0,  0,  2,  2,  0,  0,  0,  0,  0,  2 },
        spell_bless,                   TAR_OBJ_CHAR_DEF,          POS_STANDING,
        NULL,                             5,  12,                           "",
        "You feel less righteous.",
        "$p's holy aura fades.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "blindness", TRUE,
        {  9, 10, 53, 15, 53, 10, 10, 53, 12, 11,  9, 12,  9, 53 },
        {  1,  1,  2,  3,  1,  2,  2,  1,  3,  2,  1,  2,  1,  1 },
        spell_blindness,               TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_blindness,                   5,  12,                           "",
        "You can see again.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "blister", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 10, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_blister,                 TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_blister,                    25,  12,            "blistering skin",
        "Your blisters fade.",
        "$n's blisters seem to fade.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "blood boil", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_bboil,                   TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_bboil,                      20,   8,              "boiling blood",
        "Your blood cools down.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "blood cool", TRUE,
        { 53, 43, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_blood_cool,              TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        NULL,                            20,  12,                           "",
        "!Blood Cool!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "blood rush", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 53,  1 },
        spell_bloodrush,               TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            50,  12,                           "",
        "",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "bolas", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1 },
        spell_bolas,                   TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_bolas,                      15,   8,                           "",
        "You untangle yourself from the bolas.",
        "$n is free of the bolas!",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "brand", FALSE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  2,  2,  3,  3,  0,  1,  2,  3,  2,  1,  1, 53,  1 },
        spell_wanted,                  TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                             5,  12,                           "",
        "You are no longer wanted.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "burning hands", TRUE,
        {  8, 53, 53, 14, 53, 53,  9, 53, 53, 53,  8, 53, 53,  1 },
        {  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_burning_hands,           TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            15,  12,              "burning hands",
        "!Burning Hands!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "call lightning", TRUE,
        { 14, 12, 53, 53, 53, 53, 18, 19, 53, 53, 14, 53, 53, 53 },
        {  2,  1,  1,  1,  1,  1,  1,  2,  1,  1,  2,  1, 53,  1 },
        spell_call_lightning,          TAR_IGNORE,                POS_FIGHTING,
        NULL,                            15,  12,             "lightning bolt",
        "!Call Lightning!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "call spiders", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  2,  1,  2,  2,  1,  1, 53,  1 },
        spell_call_spiders,            TAR_CHAR_SELF,             POS_FIGHTING,
        &gsn_call_spiders,               50,  12,                           "",
        "$n disperses into whatever dark crevices $e can find.",
        "",
        { 99, 99, 99, 30, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "call water spirit", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  2,  1,  2,  2,  1,  1, 53,  1 },
        spell_call_water_spirit,       TAR_CHAR_SELF,             POS_FIGHTING,
        &gsn_call_water_spirit,          50,  12,                           "",
        "$n dissolves into a puddle.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 30, 99, 99, 99, 99, 99 },
    },

    {
        "calm", TRUE,
        { 53, 28, 34, 53, 53, 53, 30, 38, 53, 53, 53, 53, 53, 28 },
        {  1,  1,  2,  1,  1,  1,  1,  2,  1,  1,  1,  1, 53,  2 },
        spell_calm,                    TAR_IGNORE,                POS_FIGHTING,
        NULL,                            30,  12,                           "",
        "You have lost your peace of mind.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "cancellation", TRUE,
        { 20, 22, 24, 26, 26, 21, 22, 25, 26, 27, 20, 26, 19, 26 },
        {  1,  1,  2,  2,  3,  1,  2,  3,  3,  2,  1,  2,  1,  2 },
        spell_cancellation,            TAR_CHAR_SELF,             POS_FIGHTING,
        &gsn_cancellation,               20,  12,                           "",
        "!cancellation!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "carnivorous tendancies", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 53,  1 },
        spell_carnivore,               TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            50,  12,                           "",
        "You feel more like a vegetarian.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "celerity", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_celerity,                TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            15,   8,                           "",
        "Your blood flows normally.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "celestial spear", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 14 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  2 },
        spell_spear,                   TAR_IGNORE,                POS_STANDING,
        NULL,                            50,  12,                           "",
        "",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "cellular purge", TRUE,
        { 53, 53, 53, 53, 53, 17, 53, 53, 53, 53, 53, 23, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2, 53,  1 },
        spell_cellular_purge,          TAR_CHAR_SELF,             POS_STANDING,
        NULL,                            25,  12,                           "",
        "!cellular purge!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "chain lightning", TRUE,
        { 40, 35, 42, 53, 53, 53, 36, 43, 53, 53, 35, 53, 53, 53 },
        {  2,  1,  1,  1,  1,  1,  1,  2,  1,  1,  2,  1, 53,  1 },
        spell_chain_lightning,         TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            25,  12,                  "lightning",
        "!Chain Lightning!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "change sex", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53, 53,  1 },
        spell_change_sex,              TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        NULL,                            15,  12,                           "",
        "Your body feels familiar again.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "chaos", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 15, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2, 53,  1 },
        spell_chaos,                   TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            10,   8,          "{rc{bh{Wa{co{Ys{x",
        "!Battle Sorrow!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "chaos demon", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 38, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1, 53,  1 },
        spell_chaosdemon,              TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        &gsn_chaosdemon,                 25,  12,                "chaos demon",
        "$n shatters, turning into tiny shards which quickly vanish.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "chaotic dispersal", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 46, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1, 53,  1 },
        spell_wild_dispel,             TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            20,  12,          "chaotic dispersal",
        "!WILD DISPEL!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "charm person", TRUE,
        { 15, 53, 53, 53, 53, 53, 18, 53, 26, 24, 15, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  3,  1,  1,  1,  1,  1 },
        spell_charm_person,            TAR_CHAR_OFFENSIVE,        POS_STANDING,
        &gsn_charm_person,               10,  12,                           "",
        "You feel more self-confident.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "chill touch", TRUE,
        {  6, 53, 53, 18, 53, 53, 13, 53, 53, 53,  6, 53, 53, 10 },
        {  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_chill_touch,             TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            15,  12,             "chilling touch",
        "You feel less cold.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "chimestry", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 53 },
        {  1,  2,  1,  1,  1,  1,  2,  3,  1,  1,  1,  1,  1,  1 },
        spell_chimestry,               TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        NULL,                            10,   8,                           "",
        "The illusions fade away",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "chocolate fortification", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 53,  1 },
        spell_chocalate,               TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            50,  12,                           "",
        "Your mouth no longer tastes like chocolate.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "circle of protection", FALSE,
        { 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1 },
        spell_circle_of_protection,    TAR_CHAR_SELF,             POS_STANDING,
        &gsn_absorb,                     15,  12,                           "",
        "Your circle of protection fades.",
        "$n's circle of protection falls.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "colour spray",  TRUE,
        { 16, 53, 53, 22, 53, 53, 53, 53, 53, 53, 16, 53, 53, 53 },
        {  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_colour_spray,            TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            15,  12,               "colour spray",
        "!Colour Spray!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "confusion", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_confusion,               TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            15,   8,                           "",
        "You feel less confused.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "conjure bats", FALSE,
        { 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_conjure_bats,            TAR_IGNORE,                POS_FIGHTING,
        &gsn_conjure_bats,               55,  12,                           "",
        "$n leaves angrily in a black cloud.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "conjure familiar", FALSE,
        { 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_conjure_familiar,        TAR_IGNORE,                POS_FIGHTING,
        &gsn_conjure_familiar,           35,  12,                           "",
        "$n disappears into a dark cloud of smoke.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "continual light", TRUE,
        { 53,  1, 53, 53, 53, 53,  1, 53, 53, 53, 53, 53, 53,  3 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_continual_light,         TAR_IGNORE,                POS_STANDING,
        NULL,                             7,  12,                           "",
        "!Continual Light!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },
	
	{
        "contrition", FALSE,
        { 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50 },
        {  1,  2,  2,  2,  1,  1,  2,  1,  3,  2,  1,  2,  1,  1 },
        spell_contrition,                TAR_CHAR_OFFENSIVE,          POS_FIGHTING,
        &gsn_contrition,                      20,  12,                      "contrition",
        "You no longer feel the anguish of your penance.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "control weather", TRUE,
        { 12, 14, 53, 53, 53, 53, 11, 20, 53, 53, 12, 53, 53, 53 },
        {  2,  1,  1,  1,  1,  1,  1,  2,  1,  1,  2,  1, 53,  1 },
        spell_control_weather,         TAR_IGNORE,                POS_STANDING,
        NULL,                            25,  12,                           "",
        "!Control Weather!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "create food", TRUE,
        { 53, 15, 53, 53, 53, 53, 15, 53, 53, 53, 53, 53, 53, 15 },
        {  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  2 },
        spell_create_food,             TAR_IGNORE,                POS_STANDING,
        NULL,                             5,  12,                           "",
        "!Create Food!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "create klaive", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_klaive,                  TAR_IGNORE,                POS_STANDING,
        NULL,                            50,  12,                           "",
        "!klave",
        "",
        { 30, 30, 30, 99, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
    },

    {
        "create rose", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,  1 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_create_rose,             TAR_IGNORE,                POS_STANDING,
        NULL,                            30,  12,                           "",
        "!Create Rose!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "create spring", TRUE,
        { 53, 17, 53, 53, 53, 53, 17, 53, 53, 53, 53, 53, 53, 17 },
        {  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  2 },
        spell_create_spring,           TAR_IGNORE,                POS_STANDING,
        NULL,                            20,  12,                           "",
        "!Create Spring!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "create water", TRUE,
        { 53,  8, 53, 53, 53, 53,  8, 53, 53, 53, 53, 53, 53,  8 },
        {  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  2 },
        spell_create_water,            TAR_OBJ_INV,               POS_STANDING,
        NULL,                             5,  12,                           "",
        "!Create Water!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "crimson tide", FALSE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_guildspell,              TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            10,  24,                           "",
        "The glory of your guild leaves you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "cure blindness", TRUE,
        { 53, 23, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_cure_blindness,          TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        &gsn_cure_blindness,              5,  12,                           "",
        "!Cure Blindness!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "cure critical", TRUE,
        { 53, 53, 29, 40, 53, 53, 24, 35, 53, 26, 53, 27, 53, 53 },
        {  1,  1,  2,  3,  1,  1,  1,  2,  1,  2,  1,  2, 53,  1 },
        spell_cure_critical,           TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        &gsn_cure_critical,              20,   8,                           "",
        "!Cure Critical!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "cure disease", TRUE,
        { 53, 24, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_cure_disease,            TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        &gsn_cure_disease,               20,  12,                           "",
        "!Cure Disease!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "cure light", TRUE,
        { 53, 53, 15, 22, 53, 53,  4,  9, 53,  7, 53,  7, 53, 53 },
        {  1,  1,  2,  3,  1,  1,  1,  2,  1,  2,  1,  2, 53,  1 },
        spell_cure_light,              TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        &gsn_cure_light,                 10,  12,                           "",
        "!Cure Light!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "cure poison", TRUE,
        { 53, 21, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_cure_poison,             TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        &gsn_cure_poison,                 5,  12,                           "",
        "!Cure Poison!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "cure serious", TRUE,
        { 53, 53, 20, 30, 53, 53, 13, 17, 53, 16, 53, 18, 53, 53 },
        {  1,  1,  2,  3,  1,  1,  1,  2,  1,  2,  1,  2, 53,  1 },
        spell_cure_serious,            TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        &gsn_cure_serious,               15,  12,                           "",
        "!Cure Serious!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "curse", TRUE,
        { 24, 16, 53, 26, 53, 53, 25, 53, 32, 28, 24, 25, 21, 53 },
        {  1,  2,  2,  2,  1,  1,  2,  1,  3,  2,  1,  2,  1,  1 },
        spell_curse,                   TAR_OBJ_CHAR_OFF,          POS_FIGHTING,
        &gsn_curse,                      20,  12,                      "curse",
        "The curse wears off.",
        "$p is no longer impure.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "cyclone", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_cyclone,                 TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            25,   8,                 "gale force",
        "!Gale Force!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "dark grimoire", FALSE,
        { 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1 },
        spell_dark_grimoire,           TAR_CHAR_SELF,             POS_STANDING,
        &gsn_dark_grimoire,               5,  12,              "burning flesh",
        "The burning pain subsides.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "dark risings", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_dark_risings,            TAR_CHAR_SELF,             POS_STANDING,
        NULL,                            10,  12,                           "",
        "You fall to the ground as the darkness abandons you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "deathsong", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 32, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_deathsong,               TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_deathsong,                  20,   8,                  "deathsong",
        "!Deathsong!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "decay", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 20, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_decay,                   TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_decay,                      30,  12,              "decaying body",
        "You stop decaying.",
        "$n seems less decayed.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "detect hidden", TRUE,
        { 15, 17, 53, 53, 53, 53, 16, 18, 19, 13, 15, 14, 12, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  1,  2,  1,  1 },
        spell_detect_hidden,           TAR_CHAR_SELF,             POS_STANDING,
        NULL,                             5,   8,                           "",
        "You feel less aware of your surroundings.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "detect invis", TRUE,
        {  5, 12, 53, 53, 53, 53,  6, 12, 16, 12,  5, 13, 17, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  1,  2,  1,  1 },
        spell_detect_invis,            TAR_CHAR_SELF,             POS_STANDING,
        NULL,                             5,   8,                           "",
        "You no longer see invisible objects.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "detect magic", TRUE,
        { 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  1,  1,  1,  1 },
        spell_detect_magic,            TAR_CHAR_SELF,             POS_STANDING,
        NULL,                             5,   8,                           "",
        "The detect magic wears off.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "detect were", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  2,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1, 53,  1 },
        spell_detect_were,             TAR_CHAR_SELF,             POS_STANDING,
        NULL,                             5,   8,                           "",
        "You can no longer sense were creatures.",
        "",
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
    },

    {
        "devotion", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 53,  1 },
        spell_devotion,                TAR_CHAR_DEFENSIVE,        POS_STANDING,
        &gsn_devotion,                   50,  12,                           "",
        "You raise your head high once again.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "dispel magic", TRUE,
        { 16, 17, 19, 20, 23, 23, 18, 18, 20, 22, 53, 20, 14, 53 },
        {  1,  1,  2,  2,  3,  1,  2,  3,  3,  2,  1,  2,  1,  2 },
        spell_dispel_magic,            TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_dispel_magic,               15,  12,                           "",
        "!Dispel Magic!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "displace", TRUE,
        { 53, 53, 53, 53, 53,  4, 53, 53, 53, 53, 53,  5, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2, 53,  1 },
        spell_displace,                TAR_CHAR_SELF,             POS_STANDING,
        NULL,                             5,  12,                           "",
        "You no longer feel out of place.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "divine essence", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_divine_essence,          TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                             5,   8,                           "",
        "!Divine Essence!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "domination", TRUE,
        { 53, 53, 53, 53, 53, 13, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_domination,              TAR_CHAR_OFFENSIVE,        POS_STANDING,
        &gsn_domination,                 20,  12,                           "",
        "You regain your mind.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "earthquake", TRUE,
        { 25, 23, 16, 53, 53, 53, 26, 34, 53, 53, 25, 53, 53, 53 },
        {  1,  1,  2,  1,  1,  1,  2,  2,  1,  1,  2,  1, 53,  1 },
        spell_earthquake,              TAR_IGNORE,                POS_FIGHTING,
        NULL,                            15,  12,                 "earthquake",
        "!Earthquake!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "echoes of power", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 21, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_echoes,                  TAR_IGNORE,                POS_STANDING,
        NULL,                            50,  12,                           "",
        "!Blade!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "elemental sphere", FALSE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_guildspell,              TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            10,  24,                           "",
        "The glory of your guild leaves you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "enchant armor", TRUE,
        { 15, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_enchant_armor,           TAR_OBJ_INV,               POS_STANDING,
        NULL,                           100,  24,                           "",
        "!Enchant Armor!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "enchant weapon", TRUE,
        { 16, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_enchant_weapon,          TAR_OBJ_INV,               POS_STANDING,
        NULL,                           100,  24,                           "",
        "!Enchant Weapon!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "energy drain", TRUE,
        { 53, 53, 53, 53, 53, 25, 53, 53, 53, 53, 53, 30, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2, 53,  1 },
        spell_energy_drain,            TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_energy_drain,               35,  12,               "energy drain",
        "!Energy Drain!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "enhanced strength", TRUE,
        { 53, 53, 53, 53, 53, 15, 53, 53, 53, 53, 53, 17, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2, 53,  1 },
        spell_enhanced_strength,       TAR_CHAR_SELF,             POS_STANDING,
        NULL,                            20,  12,                           "",
        "You no longer feel so strong.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "entrance", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_entrance,                TAR_CHAR_OFFENSIVE,        POS_STANDING,
        &gsn_entrance,                   10,   8,                           "",
        "The trance is lifted.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "epiclesis", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_epiclesis,               TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            20,  12,                           "",
        "!Epiclesis!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "faerie fire", TRUE,
        {  2,  2,  4, 53, 53, 53,  2,  6, 53, 53,  2,  6, 53, 53 },
        {  1,  1,  2,  1,  1,  1,  1,  2,  1,  1,  2,  2, 53,  1 },
        spell_faerie_fire,             TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                             5,  12,                "faerie fire",
        "The pink aura around you fades away.",
        "$n looks less pink.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "faerie fog", TRUE,
        { 18, 16, 12, 53, 53, 53, 17, 25, 53, 53, 18, 18, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  2,  2, 53,  1 },
        spell_faerie_fog,              TAR_IGNORE,                POS_STANDING,
        NULL,                            12,  12,                 "faerie fog",
        "!Faerie Fog!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "familiar link", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_familiar_link,           TAR_CHAR_SELF,             POS_FIGHTING,
        &gsn_familiar_link,               0,   0,                           "",
     "You no longer sense the tie to your familiar and your power diminishes.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "famine", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_famine,              TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                                     20,  12,                  "",
        "You feel stronger as the unnatural hunger subsides.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "fear", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  2,  2,  3,  1,  1,  2,  1,  3,  1,  1,  1, 53,  1 },
        spell_fear,                    TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            20,  12,                       "fear",
        "You are no longer afraid.",
        "",
        { 30, 30, 30, 99, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 99 },
    },

    {
        "fester", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 30, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_wilt,                    TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_wilt,                       25,  12,             "festering pain",
        "You stop hurting.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "festive spirit", FALSE,
        { 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 53 },
        {  2,  1,  1,  1,  1,  1,  1,  2,  1,  1,  2,  1, 53,  1 },
        spell_festive_spirit,          TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            25,  12,                           "",
        "You feel an emptiness as festive spirits leave you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "fire breath", FALSE,
        { 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_fire_breath,             TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                             8,   4,             "blast of flame",
        "The smoke leaves your eyes.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "fireball", TRUE,
        { 28, 53, 53, 36, 53, 53, 27, 53, 53, 53, 28, 53, 53, 53 },
        {  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1, 53, 53,  1 },
        spell_fireball,                TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_fireball,                   15,   8,                   "fireball",
        "!Fireball!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "fishy faculties", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 53,  1 },
        spell_fish,                    TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            50,  12,                           "",
        "The smell of fish leaves your breath.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "flame blade", TRUE,
        { 16, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_blade,                   TAR_IGNORE,                POS_STANDING,
        NULL,                            50,  12,                           "",
        "!Blade!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "flamestrike", TRUE,
        { 53, 20, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 23 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_flamestrike,             TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_flamestrike,                20,   8,                 "holy flame",
        "!Flamestrike!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "flashfire", TRUE,
        { 39, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_flashfire,               TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                             8,   4,            "blazing inferno",
        "!FlashFire!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "fly", TRUE,
        { 19, 21, 23, 24, 27, 53, 20, 24, 25, 19, 19, 53, 23, 24 },
        {  1,  1,  2,  2,  3,  3,  1,  3,  2,  1,  1,  1,  1,  2 },
        spell_fly,                     TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            10,   8,                           "",
        "You slowly float to the ground.",
        "$p lands nearby.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "focus", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_focus,                   TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            35,  12,                           "",
        "You lose your focus.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "forget", TRUE,
        { 53, 53, 53, 53, 53, 20, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_forget,                  TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_forget,                            12,  12,                           "",
        "The haze lifts, and you remember your true abilities.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "fortune", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_fortune,                 TAR_IGNORE,                POS_STANDING,
        NULL,                            20,  12,                           "",
        "!Fortune!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "frenzy", FALSE,
        { 53, 26, 28, 53, 53, 53, 27, 29, 53, 53, 53, 53, 53, 29 },
        {  1,  1,  2,  1,  1,  1,  2,  2,  1,  1,  1,  1, 53,  2 },
        spell_frenzy,                  TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            30,  12,                           "",
        "Your rage ebbs.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "frost breath", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_frost_breath,            TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                             7,   4,             "blast of frost",
        "!Frost Breath!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "gas breath",  TRUE,
        { 46, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_gas_breath,              TAR_IGNORE,                POS_FIGHTING,
        NULL,                            12,   4,               "blast of gas",
        "!Gas Breath!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "gate", TRUE,
        { 30, 28, 53, 53, 53, 31, 33, 53, 53, 53, 30, 53, 45, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_gate,                    TAR_IGNORE,                POS_FIGHTING,
        NULL,                            80,  12,                           "",
        "!Gate!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "general purpose", FALSE,
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 53, 53 },
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 53,  1 },
        spell_general_purpose,         TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                             0,  12,       "general purpose ammo",
        "!General Purpose Ammo!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "giant strength", TRUE,
        { 11, 53, 22, 27, 53, 53, 12, 23, 25, 18, 11, 53, 11, 53 },
        {  1,  1,  2,  2,  1,  1,  1,  3,  2,  2,  1, 53,  1,  1 },
        spell_giant_strength,          TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            20,  12,                           "",
        "You feel weaker.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "green thumb", FALSE,
        { 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12 },
        {  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1 },
        spell_green_thumb,              TAR_IGNORE,                POS_STANDING,
        NULL,                             5,   4,                           "",
        "!Green Thumb!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "gypsy curse", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_gypsy_curse,             TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            15,  20,                "gypsy curse",
        "The Gypsy curse fades and you feel clean once more.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "hand of faith", FALSE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_guildspell,              TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            10,  24,                           "",
        "The glory of your guild leaves you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "hand of law", FALSE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_guildspell,              TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            10,  24,                           "",
        "The glory of your guild leaves you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "harrow", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 43 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 },
        spell_harrow,                  TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            20,   6,                 "holy flame",
        "",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "haste", TRUE,
        { 22, 53, 25, 29, 53, 53, 23, 29, 30, 27, 22, 53, 24, 53 },
        {  1,  1,  2,  2,  1,  1,  1,  3,  2,  2,  1, 53,  1,  1 },
        spell_haste,                   TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        NULL,                            30,  12,                           "",
        "You feel yourself slow down.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "haunt", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  1 },
        spell_haunt,                   TAR_CHAR_DEFENSIVE,        POS_STANDING,
        &gsn_haunt,                      15,   8,           "haunting spirits",
        "You feel warmer as haunting spirits leave you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "headache", TRUE,
        { 53, 53, 53, 53, 53, 10, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_headache,                TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_headache,                   25,   8,                   "headache",
        "Your head stops throbbing.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "heal", TRUE,
        { 53, 30, 53, 53, 53, 53, 35, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_heal,                    TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        &gsn_heal,                       50,  12,                           "",
        "!Heal!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "high explosive", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53, 53,  1 },
        spell_high_explosive,          TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                             0,  12,        "high explosive ammo",
        "!High Explosive Ammo!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "holy word", TRUE,
        { 53, 40, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_holy_word,               TAR_IGNORE,                POS_FIGHTING,
        NULL,                            75,  24,               "divine wrath",
        "!Holy Word!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "identify", TRUE,
        { 16, 17, 22, 30, 30, 24, 17, 18, 19, 15, 16, 16, 16, 30 },
        {  1,  1,  2,  1,  1,  1,  1,  2,  2,  2,  1,  2,  1,  1 },
        spell_identify,                TAR_OBJ_INV,               POS_STANDING,
        NULL,                            12,   8,                           "",
        "!Identify!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "immolation", FALSE,
        { 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_immolation,              TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_immolation,                 10,  24,           "shroud of flames",
        "The flames surrounding you suddenly extinguish.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "infernal touch", FALSE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_guildspell,              TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            10,  24,                           "",
        "The glory of your guild leaves you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "infliction", TRUE,
        { 53, 53, 53, 53, 53,  1, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_infliction,              TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            15,   8,                "inflictions",
        "!Infliction!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "infravision", TRUE,
        {  6, 53, 53,  9, 53, 53,  7,  9, 12,  7,  6, 14,  1, 53 },
        {  1,  1,  1,  2,  1,  1,  1,  3,  2,  2,  1,  1,  1,  1 },
        spell_infravision,             TAR_CHAR_SELF,             POS_STANDING,
        NULL,                             5,   8,                           "",
        "You no longer see in the dark.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "infusion", FALSE,
        { 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_infusion,                TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            50,  12,                           "",
        "Your temporary infusion of life energy dissipates!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "insight", FALSE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_guildspell,              TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            10,  24,                           "",
        "The glory of your guild leaves you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "interred to earth", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
        {  1,  1,  2,  2,  3,  1,  2,  2,  2,  1,  1,  2,  1,  1 },
        spell_interred,                TAR_IGNORE,                POS_STANDING,
        NULL,                            60,   8,                           "",
        "!Portal!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "inquisition",  TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 20 },
        {  1,  1,  2,  2,  3,  1,  2,  3,  3,  2,  1,  2,  1,  2 },
        spell_inquisition,             TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            10,   6,                     "fervor",
        "",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "invisibility", TRUE,
        {  5, 53, 53, 53, 53, 53,  6, 53,  8,  8,  5,  9, 28, 53 },
        {  1,  1,  1,  1,  1,  1,  2,  1,  2,  2,  1,  2,  1,  1 },
        spell_invis,                   TAR_OBJ_CHAR_DEF,          POS_STANDING,
        &gsn_invis,                       5,  12,                           "",
        "You are no longer invisible.",
        "$p fades into view.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "jail key", FALSE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 20, 53,  1 },
        spell_key,                     TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            10,  24,                           "",
        "!Jail Key!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "judgment", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 30 },
        {  1,  1,  2,  3,  1,  2,  2,  1,  3,  2,  1,  2,  1,  2 },
        spell_judgment,                TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            20,   8,                           "",
        "",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "kit", FALSE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_kit,                     TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            10,   8,                           "",
        "The glory of your guild leaves you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "levitation", TRUE,
        { 53, 53, 53, 53, 53, 17, 53, 53, 53, 53, 53, 18, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_levitation,              TAR_CHAR_SELF,             POS_STANDING,
        NULL,                            10,  12,                           "",
        "You slowly float to the ground.",
        "$n lands near you.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "libraverim", FALSE,
        { 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1 },
        spell_reflect,                 TAR_CHAR_SELF,             POS_STANDING,
        &gsn_reflect,                    15,   8,                           "",
        "Your aura of equality fades.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "lifetap", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,  1, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_lifetap,                 TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_lifetap,                            20,   8,                    "lifetap",
        "!Lifetap!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },
	
	{
        "life transfer", FALSE,
        { 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_life_transfer,            TAR_CHAR_SELF,        POS_STANDING,
        NULL,                             0,   12,                 "",
        "!Lifetransfer!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },
	
    {
        "lightning bolt", TRUE,
        { 14, 53, 17, 24, 53, 53, 17, 53, 53, 53, 14, 53, 53, 53 },
        {  1,  1,  1,  3,  1,  1,  1,  2,  1,  1,  1,  1, 53,  1 },
        spell_lightning_bolt,          TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_lightning_bolt,             15,   8,             "lightning bolt",
        "!Lightning Bolt!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "lightning breath", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_lightning_breath,        TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            10,   4,         "blast of lightning",
        "!Lightning Breath!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "locate object", TRUE,
        { 15, 16, 30, 30, 30, 30, 16, 30, 22, 20, 15, 22, 26, 30 },
        {  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  1,  2,  1,  1 },
        spell_locate_object,           TAR_IGNORE,                POS_STANDING,
        NULL,                            20,   8,                           "",
        "!Locate Object!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "locate person", TRUE,
        { 33, 34, 40, 40, 40, 33, 34, 40, 36, 35, 33, 38, 53, 40 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  1,  2,  1,  1 },
        spell_locate_person,           TAR_IGNORE,                POS_STANDING,
        NULL,                            20,   8,                           "",
        "!Locate PERSON!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "magic missile", TRUE,
        {  1, 53, 53,  5, 53, 53, 53, 53, 53, 53,  1, 53, 53, 53 },
        {  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_magic_missile,           TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            15,  12,              "magic missile",
        "!Magic Missile!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "magic ward", TRUE,
        { 12, 14, 15, 16, 17, 12, 13, 16, 14, 13, 12, 53,  5, 16 },
        {  1,  2,  2,  3,  3,  1,  2,  3,  3,  2,  1,  1,  1,  3 },
        spell_magicward,               TAR_CHAR_SELF,             POS_STANDING,
        NULL,                             8,  12,                           "",
        "Your ward fades away.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "major heal", TRUE,
        { 53, 40, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_major_heal,              TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        &gsn_major_heal,                 75,  12,                           "",
        "!complete heal!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },
	
	{
        "mana sear", FALSE,
        { 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_mana_sear,              TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
            NULL,                         20,  12,                    "",
        "You begin to relax as your energy returns.",
        "$n begins to relax as $e energy returns.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "mass healing", TRUE,
        { 53, 42, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_mass_healing,            TAR_IGNORE,                POS_FIGHTING,
        NULL,                           100,  12,                           "",
        "!Mass Healing!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "mass invis", TRUE,
        { 41, 53, 53, 53, 53, 53, 53, 53, 53, 38, 41, 40, 40, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  2,  1,  1 },
        spell_mass_invis,              TAR_IGNORE,                POS_STANDING,
        &gsn_mass_invis,                 20,   8,                           "",
        "You are no longer invisible.",
        "$n steps out of the ether.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "mental barrier", TRUE,
        { 53, 53, 53, 53, 53, 16, 53, 53, 53, 53, 53, 17, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_mental_barrier,          TAR_CHAR_SELF,             POS_STANDING,
        NULL,                            12,   8,                           "",
        "Your mental barrier crumbles.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "mental fortress", TRUE,
        { 53, 53, 53, 53, 53, 12, 53, 53, 53, 53, 53, 13, 53, 53 },
        {  1,  2,  2,  3,  3,  1,  2,  3,  3,  2,  1,  2, 53,  1 },
        spell_mental_fortress,         TAR_CHAR_SELF,             POS_STANDING,
        NULL,                             8,  12,                           "",
        "Your mental fortress crumbles.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "meteor shower", TRUE,
        { 45, 53, 53, 53, 53, 53, 46, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  2,  1,  1,  1,  1,  1,  3,  1,  1,  1,  1, 53,  1 },
        spell_meteor,                  TAR_IGNORE,                POS_FIGHTING,
        NULL,                            25,  12,                     "meteor",
        "!Meteor Shower!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "mind blast", TRUE,
        { 53, 53, 53, 53, 53, 34, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_mind_blast,              TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_mind_blast,                 25,   8,                 "mind blast",
        "!Mind Blast!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "mind whip", TRUE,
        { 53, 53, 53, 53, 53, 25, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_mind_whip,               TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_mind_whip,                  20,  12,                           "",
        "Your mind feels less tortured.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "minor heal", TRUE,
        { 53,  5, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 21 },
        {  1,  1,  2,  3,  1,  1,  1,  2,  1,  2,  1,  1, 53,  3 },
        spell_cure_critical,           TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        NULL,                            20,   8,                           "",
        "!Cure Critical!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "mirror image", TRUE,
        { 24, 53, 53, 53, 53, 53, 53, 53, 53, 23, 27, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1 },
        spell_mirror,                  TAR_CHAR_SELF,             POS_STANDING,
        &gsn_mirror,                     25,  12,                           "",
        "Your images fade away.",
        "Suddenly, there is only one $n present in the room.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "natures pestilence", FALSE,
        { 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_natures_pestil,          TAR_CHAR_DEFENSIVE,        POS_STANDING,
        &gsn_natures_pestil,              1,  12,                           "",
        "The pestilent grip subsides, allowing you to move again.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "natures wrath", TRUE,
        { 53, 53, 53, 53, 53, 53, 36, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_natures_wrath,           TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_natures_wrath,              25,   8,            "force of nature",
        "!Natures Wrath",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "ordain", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 35 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3 },
        spell_ordain,                  TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            50,  12,                           "",
        "You are no longer ordained.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "pass door", TRUE,
        { 25, 27, 32, 34, 35, 36, 26, 32, 34, 33, 25, 35, 15, 34 },
        {  1,  1,  2,  2,  3,  1,  2,  3,  1,  2,  1,  2,  1,  2 },
        spell_pass_door,               TAR_CHAR_SELF,             POS_STANDING,
        NULL,                            20,  12,                           "",
        "You feel solid again.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "pestilence", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_pestilence,              TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_pestilence,                 20,  12,                  "pestilence",
        "The pustules covering your body dry up and heal over.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "pinch", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 25, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  1 },
        spell_pinch,                   TAR_CHAR_DEFENSIVE,        POS_STANDING,
        &gsn_pinch,                      15,   8,                      "pinch",
        "An ethereal hand waves to you and vanishes.",
        "An ethereal hand waves to $n and vanishes.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "plague", TRUE,
        { 53, 17, 53, 53, 53, 53, 53, 53, 53, 53, 53, 14,  8, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1 },
        spell_plague,                  TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_plague,                     20,  12,                   "sickness",
        "Your sores vanish.",
        "$n's boils vanish.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "plum pleasentness", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 53,  1 },
        spell_plum,                    TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            50,  12,                           "",
        "",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "poison", TRUE,
        { 53, 12, 53, 53, 53, 53, 53, 53, 53, 53, 53, 13,  2, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1 },
        spell_poison,                  TAR_OBJ_CHAR_OFF,          POS_FIGHTING,
        &gsn_poison,                     10,  12,                     "poison",
        "You feel less sick.",
        "The poison on $p dries up.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "portal", TRUE,
        { 14, 18, 20, 24, 34, 16, 15, 24, 27, 24, 14, 25, 19, 24 },
        {  1,  1,  2,  2,  3,  1,  2,  2,  2,  1,  1,  2,  1,  2 },
        spell_portal,                  TAR_IGNORE,                POS_STANDING,
        NULL,                            60,  12,                           "",
        "!Portal!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "power of the cheese", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 53,  1 },
        spell_cheese,                  TAR_CHAR_DEFENSIVE,        POS_STANDING,
        &gsn_cheese,                     50,  12,                           "",
        "You no longer feel so cheesy.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "profane word", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_profane_word,            TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_profane_word,               15,   8,                           "",
        "You are no longer afflicted by a profane word.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "protection of peaches", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 53,  1 },
        spell_peach,                   TAR_CHAR_DEFENSIVE,        POS_STANDING,
        &gsn_peaches,                    50,  12,                           "",
        "Eat a peach for peace!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "psi blade", TRUE,
        { 53, 53, 53, 53, 53, 17, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_psiblade,                TAR_IGNORE,                POS_STANDING,
        NULL,                            50,  12,                           "",
        "!Blade!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "psionic blast", TRUE,
        { 53, 53, 53, 53, 53, 27, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_psionic_blast,           TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            20,   8,              "psionic blast",
        "!Psionic Blast!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "psychic crush", TRUE,
        { 53, 53, 53, 53, 53, 18, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_psychic_crush,           TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            17,   8,              "psychic crush",
        "!Psychic Crush!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "psychic drain", TRUE,
        { 53, 53, 53, 53, 53, 15, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_psychic_drain,           TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_psy_drain,                  20,  12,                           "",
        "You feel less drained.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "psychic field", TRUE,
        { 53, 53, 53, 53, 53, 44, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  2,  1,  1,  1,  1,  2,  3,  1,  1,  1,  1,  1,  1 },
        spell_field,                   TAR_IGNORE,                POS_FIGHTING,
        NULL,                            25,  12,               "energy field",
        "!Psychic Field!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "psychic heal", TRUE,
        { 53, 53, 53, 53, 53, 33, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_psychic_heal,            TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            20,   8,                           "",
        "!psychic heal!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "ravage", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 34, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1 },
        spell_ravage,                  TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_ravage,                     20,   8,              "soul rendings",
        "!Battle Sorrow!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "recharge", TRUE,
        { 13, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_recharge,                TAR_OBJ_INV,               POS_STANDING,
        NULL,                            60,  24,                           "",
        "!Recharge!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "refresh", TRUE,
        {  7,  8,  9, 10, 53,  8,  8, 10, 14, 17,  7, 11,  4, 10 },
        {  1,  1,  2,  2,  1,  1,  1,  3,  2,  2,  1,  2,  1,  2 },
        spell_refresh,                 TAR_CHAR_DEFENSIVE,        POS_STANDING,
        &gsn_refresh,                    12,   8,                    "refresh",
        "!Refresh!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "remove curse", TRUE,
        { 53, 29, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_remove_curse,            TAR_OBJ_CHAR_DEF,          POS_STANDING,
        &gsn_remove_curse,                5,  12,                           "",
        "!Remove Curse!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "restore limb", TRUE,
        { 53, 23, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_restore_limb,            TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        &gsn_restore_limb,               30,  12,                           "",
        "!Restore Limb!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "resurrect", TRUE,
        { 53, 45, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_resurrect,               TAR_CHAR_DEFENSIVE,        POS_STANDING,
        &gsn_resurrect,                   8,   4,                           "",
        "!Resurrect!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "rune blade", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_blade2,                  TAR_IGNORE,                POS_FIGHTING,
        NULL,                            50,   8,                           "",
        "!RUNE!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "sanctuary", TRUE,
        { 38, 25, 27, 34, 44, 53, 36, 27, 40, 42, 38, 53, 20, 32 },
        {  1,  1,  2,  3,  3,  3,  2,  2,  3,  2,  1,  1,  1,  3 },
        spell_sanctuary,               TAR_CHAR_DEFENSIVE,        POS_STANDING,
        &gsn_sanctuary,                  75,   8,                           "",
        "The white aura around your body fades.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "sanguination", FALSE,
        { 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_sanguination,            TAR_CHAR_SELF,             POS_FIGHTING,
        &gsn_sanguination,               10,  24,           "profuse bleeding",
        "You sense your blood pressure fall.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "scry", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 22, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_scry,                    TAR_IGNORE,                POS_STANDING,
        NULL,                            20,  12,                    "lifetap",
        "!Lifetap!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "searing sunray", FALSE,
        { 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1 },
        spell_ray,                     TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            15,   8,                     "sunray",
        "Report this if you see it.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "shadow meld", FALSE,
        { 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 53, 53 },
        {  1,  1,  2,  3,  3,  3,  2,  2,  3,  2,  1,  1,  1,  1 },
        spell_shadow,                  TAR_CHAR_DEFENSIVE,        POS_STANDING,
        &gsn_shadow,                     75,   8,                           "",
        "You step out of the shadows.",
        "$n steps out of the shadows.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "shadows", FALSE,
        { 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1 },
        spell_shadows,                 TAR_CHAR_SELF,             POS_FIGHTING,
        &gsn_shadows,                    15,   8,                           "",
        "The shadows melt away and you are left exposed.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "shadowstep", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 18, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_shadow_step,             TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            15,   8,                 "shadowstep",
        "!SS!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "shield", TRUE,
        { 26, 25, 26, 25, 30, 53, 27, 26, 24, 20, 26, 53,  5, 25 },
        {  1,  1,  2,  3,  3,  1,  2,  3,  2,  2,  1,  1,  1,  2 },
        spell_shield,                  TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            12,   8,                           "",
        "Your force shield shimmers then fades away.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "shield of skulls", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 23, 53, 53, 40, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1 },
        spell_skulls,                  TAR_CHAR_SELF,             POS_STANDING,
        &gsn_skulls,                     25,  12,                           "",
        "Your spectral skulls crumble and fade away.",
        "...and the skulls around $n fade to nothing.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "shocking grasp", TRUE,
        { 14, 53, 53, 22, 53, 53, 15, 53, 53, 53, 14, 53, 53, 15 },
        {  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_shocking_grasp,          TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            15,  12,             "shocking grasp",
        "!Shocking Grasp!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "silence", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_silence,                 TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            20,   8,                           "",
        "You can cast spells again.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "sleep", TRUE,
        { 11, 53, 53, 53, 53, 53, 13, 53, 28, 19, 11, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1 },
        spell_sleep,                   TAR_CHAR_OFFENSIVE,        POS_STANDING,
        &gsn_sleep,                      15,  12,                           "",
        "You feel less tired.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "slow", TRUE,
        { 24, 15, 53, 30, 53, 53, 22, 53, 35, 30, 24, 39, 31, 53 },
        {  1,  1,  2,  2,  1,  1,  2,  1,  2,  2,  1,  2,  1,  1 },
        spell_slow,                    TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_slow,                            30,  12,                      "",
        "You feel yourself speed up.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "smite", TRUE,
        { 53, 38, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_smite,                   TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_smite,                      20,   8,               "Divine Wrath",
        "!Smite!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "somnambulance", FALSE,
        { 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_somnambulance,           TAR_CHAR_OFFENSIVE,        POS_STANDING,
        NULL,                                 30,  12,                      "",
        "You sense the lines between dreams and reality grow clearer.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "sorrow", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,  8, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2, 53,  1 },
        spell_sorrow,                  TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            10,   8,                    "sorrows",
        "!Battle Sorrow!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "sour shield", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 53,  1 },
        spell_sour,                    TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            50,  12,                           "",
        "The air around you regains its usual tang.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "spiritual hammer",  TRUE,
        { 53, 14, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_hammer,                  TAR_IGNORE,                POS_STANDING,
        NULL,                            50,  12,                           "",
        "!Hammer!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "steel skin", TRUE,
        { 53, 53, 53, 53, 53, 29, 53, 53, 53, 53, 53, 32, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2, 53,  1 },
        spell_steel_skin,              TAR_CHAR_SELF,             POS_STANDING,
        NULL,                            12,  12,                           "",
        "Your skin is no longer hard as steel.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "stone skin", TRUE,
        { 29, 28, 29, 30, 37, 53, 28, 29, 39, 38, 29, 53, 33, 12 },
        {  1,  1,  2,  2,  3,  1,  2,  3,  2,  2,  1,  1,  1,  2 },
        spell_stone_skin,              TAR_CHAR_SELF,             POS_STANDING,
        NULL,                            12,   8,                           "",
        "Your skin feels soft again.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "strawberry fulfillment", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 53,  1 },
        spell_strawberry,              TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                            50,  12,                           "",
        "Must have MORE STRAWBERRIES!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "summon", TRUE,
        { 23, 27, 28, 33, 35, 24, 25, 33, 35, 34, 53, 34, 26, 33 },
        {  1,  1,  2,  2,  3,  1,  1,  3,  2,  2,  1,  2,  1,  2 },
        spell_summon,                  TAR_IGNORE,                POS_STANDING,
        NULL,                            50,  12,                           "",
        "!Summon!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "summon animal", TRUE,
        { 53, 53, 53, 53, 53, 53, 20, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  2,  1,  2,  2,  1,  1, 53,  1 },
        spell_summon_animal,           TAR_IGNORE,                POS_FIGHTING,
        &gsn_sumanimal,                  50,  12,                           "",
        "$n runs off into the wild.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "summon meerkats", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_summonmeerkats,          TAR_CHAR_SELF,             POS_FIGHTING,
        &gsn_summon_meerkats,            40,   8,                           "",
        "$n scampers off.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 30 },
    },

    {
        "summon skeletal army", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
        {  1,  1,  1,  1,  1,  1,  2,  1,  2,  2,  1,  1,  1,  1 },
        spell_skeletons,               TAR_CHAR_SELF,             POS_FIGHTING,
        &gsn_summon_skeletal_army,      300,   8,                           "",
        "$n crumbles into dust.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "summoning sickness", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  2,  1,  2,  2,  1,  1, 53,  1 },
        spell_summon_sick,             TAR_CHAR_SELF,             POS_FIGHTING,
        &gsn_sumsick,                     0,   0,                           "",
        "Your summoning sickness fades.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "sunlight skeletals", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
        {  1,  1,  1,  1,  1,  1,  2,  1,  2,  2,  1,  1,  1,  1 },
        spell_sun_skeletals,           TAR_CHAR_SELF,             POS_FIGHTING,
        &gsn_sun_skeletals,             300,   8,                           "",
        "$n crumbles into dust.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "supermob", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  3,  1,  1,  1, 53,  1 },
        spell_supermob,                TAR_CHAR_DEFENSIVE,        POS_STANDING,
        &gsn_supermob,                   10,   8,                           "",
        "You feel less SUPER DUPER!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "teleport", TRUE,
        { 18, 21, 22, 24, 39, 19, 20, 24, 28,  9, 53, 10, 33, 19 },
        {  1,  1,  2,  2,  3,  1,  2,  3,  2,  2,  1,  2,  1,  2 },
        spell_teleport,                TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            35,  12,                           "",
        "!Teleport!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "teleport other", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  2,  2,  3,  1,  2,  3,  2,  2,  1,  2,  1,  1 },
        spell_teleport,                TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            35,  12,                           "",
        "!Teleport Other!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "terror", FALSE,
        { 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1 },
        spell_terror,                  TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_terror,                     15,   8,                           "",
        "You no longer feel the Terror of the Rain.",
        "$n seems to regain partial bladder control.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "thunderclap", TRUE,
        { 38, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_thunderclap,             TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            11,   4,            "clap of thunder",
        "!Thunderclap!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "torment", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 28, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2, 53,  1 },
        spell_torment,                 TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            10,   8,                   "torments",
        "!Battle Sorrow!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "touch of dawn", FALSE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_guildspell,              TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            10,  24,                           "",
        "The glory of your guild leaves you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "encomium", FALSE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_guildspell,              TAR_CHAR_SELF,             POS_FIGHTING,
        &gsn_encomium,                            10,  24,                           "",
        "You feel the glory of Syrin fade away from you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "tyrins wrath", FALSE,
        { 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1 },
        spell_twrath,                  TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_twrath,                     15,   8,                           "",
        "You no longer feel the wrath of Tyrin.",
        "$n looks more comfortable.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "unholy fire", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_unholyfire,              TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_unholyfire,                 17,   8,                "unholy fire",
        "The boils on your skin seem to have healed.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 30, 99 },
    },

    {
        "unholy touch", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_guildspell,              TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            10,  24,                           "",
        "The glory of your guild leaves you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "vanish", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 15, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1 },
        spell_vanish,                  TAR_IGNORE,                POS_FIGHTING,
        NULL,                            30,   1,                     "vanish",
        "You feel reoriented.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "veil", FALSE,
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_veil,                    TAR_CHAR_DEFENSIVE,        POS_STANDING,
        &gsn_veil,                       25,  12,                       "veil",
        "Your presence is unveiled.",
        "$n steps forth from a veil of darkness.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "ventriloquate", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53,  1, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_ventriloquate,           TAR_IGNORE,                POS_STANDING,
        NULL,                             5,  12,                           "",
        "!Ventriloquate!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },
	
	{
		"vorpalspell", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        spell_vorpalspell,               TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_vorpalspell,                  20,   8,                  "magic",
        "!VORPAL!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "warp time", TRUE,
        { 53, 53, 53, 53, 53, 25, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_warp_time,               TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            30,  12,                           "",
        "You no longer have a warped sense of time.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "water bolt", TRUE,
        { 40, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_water_bolt,              TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                             6,   4,             "blast of water",
        "!Water Bolt!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "weaken", TRUE,
        { 18, 10, 53, 25, 53, 53, 16, 53, 17, 15, 18, 16, 25, 53 },
        {  1,  1,  2,  2,  1,  1,  2,  1,  3,  2,  1,  2,  1,  1 },
        spell_weaken,                  TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_weaken,                     20,  12,                      "spell",
        "You feel stronger.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "web", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_web,                     TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            35,  12,                           "",
        "The webs surrounding you dissolve.",
        "",
        { 99, 99, 99, 30, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "white grimoire", FALSE,
        { 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1 },
        spell_white_grimoire,          TAR_CHAR_SELF,             POS_STANDING,
        &gsn_white_grimoire,              5,  12,              "burning flesh",
        "The burning pain subsides.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "wild aura", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 34, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  1 },
        spell_wildaura,                TAR_CHAR_SELF,             POS_STANDING,
        &gsn_wildaura,                   30,  12,                  "wild aura",
        "Your aura fades.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "wild enhance", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 28, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1, 53,  1 },
        spell_wildenhance,             TAR_CHAR_SELF,             POS_STANDING,
        &gsn_wildenhance,                20,  12,               "wild enhance",
        "You feel less enhanced.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "wild heal", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 26, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  1 },
        spell_wildheal,                TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        NULL,                            25,  12,                  "wild heal",
        "!WILD HEAL!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "wild shield", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 32, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  1 },
        spell_wildshield,              TAR_CHAR_SELF,             POS_STANDING,
        NULL,                             5,  12,                "wild shield",
        "Your wild shield dissipates.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "wild summon", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 23, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  1 },
        spell_wildsummon,              TAR_IGNORE,                POS_STANDING,
        NULL,                            45,  12,                "wild summon",
        "!Summon!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "wildfire", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 31, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  1 },
        spell_wildfire,                TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_wildfire,                   11,   6,                   "wildfire",
        "Your fire goes out.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "winter blast", TRUE,
        { 37, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_winter_blast,            TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        NULL,                            10,   4,               "blast of ice",
        "!Winter Blast!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "wither limb", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 44, 53 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_wither_limb,             TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_wither_limb,                10,  12,                "wither limb",
        "Your arm heals and is useful again.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "word of recall", TRUE,
        { 34, 32, 33, 35, 36, 32, 33, 35, 36, 35, 34, 33, 11, 37 },
        {  1,  1,  2,  2,  2,  1,  1,  2,  2,  1,  1,  1,  1,  2 },
        spell_word_of_recall,          TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                             5,  12,                           "",
        "!Word of Recall!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "wrath", TRUE,
        { 53, 41, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  1,  1,  1,  1,  1,  1,  2,  3,  1,  1,  1,  1,  1,  1 },
        spell_wrath,                   TAR_IGNORE,                POS_FIGHTING,
        NULL,                            25,  12,               "divine wrath",
        "!Wrath!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "wyrm venom", FALSE,
        { 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50 },
        {  1,  1,  2,  3,  1,  2,  2,  1,  3,  2,  1,  2,  1,  1 },
        spell_wyrm_venom,              TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_wyrm_venom,                 25,  12,                           "",
        "The effects of the venom wear off.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "armor pierce", FALSE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_armorpierce,                 0,  18,               "armor pierce",
        "Arrowheads fall out of your armor.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "armthrow", TRUE,
        { 53, 53, 13, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_armthrow,                    0,  24,                   "armthrow",
        "!armthrow!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "axe", TRUE,
        {  1,  1, 53,  1,  1, 53, 53,  1, 53, 53,  1, 53, 53,  1 },
        {  8,  6,  0,  4,  3,  0,  0,  5,  0,  0,  8,  0,  0,  5 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_axe,                         0,   0,                           "",
        "!Axe!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "backstab", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53,  1, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  3,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_backstab,                    0,  24,                   "backstab",
        "!Backstab!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "bash", TRUE,
        { 53, 53, 53,  1,  1, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  3,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_bash,                        0,  24,                       "bash",
        "!Bash!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "battle focus", TRUE,
        { 53, 53, 53, 13, 53, 53, 53, 13, 53, 53, 53, 15, 53, 53 },
        {  0,  0,  0,  4,  0,  0,  0,  5,  0,  0,  0,  5,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_battle,                      0,  12,                           "",
        "You lose your focus on the battle.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "bow", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_bow,                         0,   0,                           "",
        "!Bow!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "brew", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_brew,                        0,  24,                       "brew",
        "!Brew!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "burning palm", TRUE,
        { 53, 53, 37, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_burning_palm,                 0,  24,                "burning palm",
        "!burning palm!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "charge", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_charge,                      0,  24,                     "charge",
        "!Charge!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 30, 99, 99, 99 },
    },

    {
        "cleanse", TRUE,
        { 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_cleanse,                     0,  18,                    "cleanse",
        "You wake up with a squeaky clean soul.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "crane kick", TRUE,
        { 53, 53, 24, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_crane_kick,                  0,  18,                 "crane kick",
        "!crane kick!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "critical strike", TRUE,
        { 53, 53, 53, 20, 15, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  4,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_critical_strike,             0,   0,                           "",
        "!Critical Strike!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "dagger", TRUE,
        {  1,  1, 53,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53 },
        {  6,  5,  0,  3,  3,  6,  4,  4,  4,  4,  6,  4,  6,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_dagger,                      0,   0,                           "",
        "!Dagger!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "dirt kicking", TRUE,
        { 53, 53, 17,  8, 12, 53, 53, 17,  5,  3, 53, 53, 53, 53 },
        {  0,  0,  4,  4,  4,  0,  0,  4,  4,  3,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_dirt,                        0,  24,                "kicked dirt",
        "You rub the dirt out of your eyes.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "disarm", TRUE,
        { 53, 53, 53, 12, 15, 53, 53,  1, 11, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  4,  4,  0,  0,  4,  4,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_disarm,                      0,  24,                           "",
        "!Disarm!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "dislodge", TRUE,
        { 53, 53, 53, 12, 15, 53, 53,  1, 11, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  4,  4,  0,  0,  4,  3,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_dislodge,                    0,  12,                           "",
        "!Disarm!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "divine focus", TRUE,
        { 53, 53, 15, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 15 },
        {  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_divine_focus,                0,  12,                           "",
        "Your divine focus leaves you.", 
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "dodge", TRUE,
        { 20, 14,  8,  5,  6, 18, 19,  5,  4, 12, 20, 10, 15,  5 },
        {  9,  6,  4,  3,  3,  7,  6,  4,  4,  5,  8,  5,  9,  4 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_dodge,                       0,   0,                           "",
        "!Dodge!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "dragon punch", TRUE,
        { 53, 53, 39, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_dragon_punch,                0,   0,               "dragon punch",
        "!dragon punch!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "dual wield", TRUE,
        { 53, 53, 53, 53, 53, 53, 53,  9, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  3,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_dualwield,                   0,  24,                 "dual wield",
        "!Dual Wield",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "enhanced damage", TRUE,
        { 53, 27, 15,  2,  2, 53, 32,  2, 34, 26, 53, 24, 53,  2 },
        {  0,  7,  6,  4,  4,  0,  6,  4,  6,  6,  0,  6,  0,  6 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_enhanced_damage,             0,   0,                           "",
        "!Enhanced Damage!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "entangle", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 33, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  3,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_entangle,                    0,  24,                   "entangle",
        "!entangle!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "exhaustion", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_CHAR_SELF,             POS_FIGHTING,
        &gsn_exhaustion,                  0,   0,                           "",
        "Your hands warm as they become ready to heal again.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "fast healing", TRUE,
        { 17,  6,  4,  1,  1, 18, 12,  4, 25,  2, 17,  2, 19,  1 },
        { 10,  6,  6,  4,  4,  7,  6,  4,  6,  6,  8,  5, 10,  5 },
        spell_null,                    TAR_IGNORE,                POS_SLEEPING,
        &gsn_fast_healing,                0,   0,                           "",
        "!Fast Healing!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "feed", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_feed,                        0,   8,                           "",
        "",
        "",
        { 99, 99, 99, 30, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "fifth attack", TRUE,
        { 53, 53, 53, 53, 40, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_fifth_attack,                0,   0,                           "",
        "!Fifth Attack!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "fire", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_fire,                        0,  18,                      "arrow",
        "You stop bleeding.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "flail", TRUE,
        {  1,  1, 53,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        {  4,  4,  0,  3,  3,  4,  4,  4,  7,  4,  4,  2,  0,  4 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_flail,                       0,   0,                           "",
        "!Flail!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "fourth attack", TRUE,
        { 53, 53, 53, 33, 32, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  4,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_fourth_attack,               0,   0,                           "",
        "!Fourth Aattack!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "fury", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_fury,                        0,  24,                           "",
        "You no longer feel the beast within.",
        "$n looks less beastly.",
        { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
    },

    {
        "haggle", TRUE,
        { 22, 36, 27, 44, 46, 17, 29, 46,  1,  1, 22, 47, 22, 35 },
        {  5,  8,  6,  6,  7,  6,  7,  6,  1,  4,  5,  6,  7,  8 },
        spell_null,                    TAR_IGNORE,                 POS_RESTING,
        &gsn_haggle,                      0,   0,                           "",
        "!Haggle!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "hamstring", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_hamstring,                   0,  24,                  "hamstring",
        "!Hamstring!",
        "",
        { 99, 99, 99, 99, 30, 99, 30, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "hand to hand", TRUE,
        {  1,  1,  1,  1,  1, 53, 53,  1,  1,  1,  1,  1, 53,  1 },
        {  5,  6,  3,  4,  4,  0,  0,  4,  5,  5,  1,  5,  0,  8 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_hand_to_hand,                0,   0,                           "",
        "!Hand to Hand!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "harm touch", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 33, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  0 },
        spell_null,                    TAR_CHAR_OFFENSIVE,        POS_STANDING,
        &gsn_harmtouch,                   0,  18,                 "harm touch",
        "You feel prepared to fight in ethereal form once more.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "heightened senses", TRUE,
        { 53, 53, 14, 20, 20,  4, 53, 20, 53, 53, 53, 53, 53, 20 },
        {  0,  0,  4,  4,  4,  4,  0,  0,  0,  0,  0,  0,  0,  4 },
        spell_null,                    TAR_CHAR_DEFENSIVE,        POS_STANDING,
        &gsn_heightened_senses,          20,  16,                           "",
        "Your heightened senses diminish.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "hide", TRUE,
        { 53, 53, 53,  1,  1, 53,  1,  1,  1,  5, 53, 53, 18, 53 },
        {  0,  0,  0,  5,  5,  0,  2,  2,  5,  2,  0,  0, 10,  0 },
        spell_null,                    TAR_IGNORE,                 POS_RESTING,
        &gsn_hide,                        0,  12,                           "",
        "!Hide!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "inspire", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 35, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_inspire,                   100,  20,                           "",
        "You lose your inspiration.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "kick", TRUE,
        { 53,  9,  7,  8,  8, 53, 53,  8,  9, 15, 53, 53, 53,  8 },
        {  0,  6,  3,  4,  4,  0,  0,  4,  5,  5,  0,  0,  0,  5 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_kick,                        0,  12,                       "kick",
        "!Kick!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "lay on hands", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 40 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_lay_on_hands,                0,  12,                           "",
        "Your hands feel prepared to heal again.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "lore", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                 POS_RESTING,
        &gsn_lore,                        0,  36,                           "",
        "!Lore!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "love", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_love,                        0,  18,                           "",
        "You regain your composure as the warm, mushy feeling subsides.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "mace", TRUE,
        {  1,  1, 53,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        {  6,  2,  0,  3,  3,  6,  5,  4,  8,  6,  6,  4,  6,  4 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_mace,                        0,   0,                           "",
        "!Mace!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "maul", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_maul,                        0,  24,                       "maul",
        "!Maul!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 30, 99, 99, 99, 99 },
    },

    {
        "meditation", TRUE,
        {  4,  1, 14, 28, 36,  6,  7, 12, 34, 31,  4, 22,  1, 28 },
        {  6,  5,  4,  8,  8,  5,  5,  6,  7,  7,  4,  6,  5,  7 },
        spell_null,                    TAR_IGNORE,                POS_SLEEPING,
        &gsn_meditation,                  0,   0,                           "",
        "Meditation",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "palm block", TRUE,
        { 53, 53,  1, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_palm_block,                  0,   0,                           "",
        "!palm block!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "parry", TRUE,
        { 21, 17,  9,  1,  3, 16, 18,  1,  3,  4, 21,  5, 19,  1 },
        {  9,  5,  7,  4,  4,  7,  5,  4,  5,  6,  7,  6,  9,  5 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_parry,                       0,   0,                           "",
        "!Parry!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "peek", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53,  1, 11, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  3,  4,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_peek,                        0,   0,                           "",
        "!Peek!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "pick lock", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53,  1, 22, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  4,  3,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_pick_lock,                   0,  12,                           "",
        "!Pick!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "poison dagger", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 38, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_throw,                       0,  18,              "poison dagger",
        "The poison wears off.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "polearm", TRUE,
        {  1,  1, 53,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        {  7,  4,  0,  4,  3,  7,  7,  4,  6,  6,  7,  1,  4,  4 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_polearm,                     0,   0,                           "",
        "!Polearm!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "pounce", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_pounce,                      0,  24,                     "pounce",
        "!Pounce!",
        "",
        { 99, 99, 99, 99, 99, 30, 99, 30, 99, 99, 99, 99, 30, 99, 99 },
    },

    {
        "purify", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 45 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_purify,                      0,  12,            "purifying flame",
        "",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "rage", TRUE,
        { 53, 53, 53, 53, 24, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_rage,                        0,  24,                           "",
        "You feel less enraged.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "recall", TRUE,
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_recall,                      0,  12,                           "",
        "!Recall!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "rescue", TRUE,
        { 53, 53, 53,  6,  7, 53, 53,  6, 53, 53, 53, 53, 53,  6 },
        {  0,  0,  0,  4,  4,  0,  0,  4,  0,  0,  0,  0,  0,  4 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_rescue,                      0,  12,                           "",
        "!Rescue!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "sanguine bond", TRUE,
        { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_sanguine_bond,               0,  24,                           "",
        "Ancient knowledge leaves your mind as your sanguine bond fades.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "sap", TRUE,
        { 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_sap,                         0,  18,                        "sap",
        "You wake with a lump on your head.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "scrolls", TRUE,
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        {  2,  2,  4,  5,  6,  3,  2,  5,  6,  4,  2,  3,  1,  3 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_scrolls,                     0,  24,                           "",
        "!Scrolls!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "second attack", TRUE,
        { 30, 24,  7,  5,  5, 30, 25, 18, 16, 14, 30, 13, 15,  5 },
        { 10,  6,  2,  3,  3,  6,  6,  4,  5,  4,  8,  4,  8,  4 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_second_attack,               0,   0,                           "",
        "!Second Attack!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "shadow form", TRUE,
        { 53, 53, 53, 53, 53, 27, 53, 53, 53, 53, 53,  7, 53, 53 },
        {  0,  0,  0,  0,  0,  6,  0,  0,  0,  0,  0,  4,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_shadow_form,                10,  12,                           "",
        "You are no longer a shadow.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "shield block", TRUE,
        { 53,  1, 53,  1,  1,  5,  1,  1,  1,  1, 53,  1, 11,  1 },
        {  0,  6,  0,  4,  4,  5,  6,  4,  5,  4,  0,  4,  6,  6 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_shield_block,                0,   0,                           "",
        "!Shield!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "sneak", TRUE,
        { 53, 53, 53, 10, 23, 53, 16,  8,  5, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  5,  6,  0,  6,  4,  4,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_sneak,                       0,  12,                           "",
        "You no longer feel stealthy.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "solemn vow", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 30 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_solemn_vow,                  0,  24,                           "",
        "The conviction of your solemn vow wanes.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "soothe", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 39, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  0,  0,  0,  0 },
        spell_null,                    TAR_CHAR_DEFENSIVE,        POS_FIGHTING,
        &gsn_soothe,                     50,  25,                           "",
        "Your surroundings are suddenly less soothing.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "spear", TRUE,
        {  1,  1, 53,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53,  1 },
        {  7,  6,  0,  3,  3,  8,  6,  3,  7,  8,  7,  6,  0,  2 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_spear,                       0,   0,                           "",
        "!Spear!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "staff", TRUE,
        {  1,  1, 53,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53 },
        {  2,  5,  0,  3,  3,  5,  5,  4,  6,  4,  2,  2,  2,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_staff,                       0,   0,                           "",
        "!Staff!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "staves", TRUE,
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        {  2,  2,  4,  5,  6,  3,  2,  5,  6,  4,  2,  3,  2,  4 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_staves,                      0,  12,                           "",
        "!Staves!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "steal", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 13, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_steal,                       0,  24,                           "",
        "!Steal!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "sword", TRUE,
        {  1,  1, 53,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        {  8,  7,  0,  3,  3,  4,  6,  4,  5,  6,  8,  7,  6,  5 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_sword,                       0,   0,                           "",
        "!sword!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "third attack", TRUE,
        { 53, 53, 13, 11, 12, 53, 53, 23, 28, 30, 53, 29, 53, 11 },
        {  0,  0,  3,  5,  5,  0,  0,  6,  6,  6,  0,  7,  0,  5 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_third_attack,                0,   0,                           "",
        "!Third Attack!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "touch of death", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
        &gsn_touch,                       0,   8,                           "",
        "",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 30, 99, 99, 99, 99, 99, 99 },
    },

    {
        "trip", TRUE,
        { 53, 53, 53, 12, 53, 53, 53, 22,  1,  6, 53,  1, 53, 12 },
        {  0,  0,  0,  5,  0,  0,  0,  4,  4,  3,  0,  3,  0,  4 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_trip,                        0,  24,                       "trip",
        "!Trip!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },
	
	{
		"union strength", FALSE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_guildspell,              TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            10,  24,                           "",
        "The glory of your guild leaves you.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "vital strike", TRUE,
        { 53, 53, 29, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_vital_strike,                0,  12,               "vital strike",
        "The feeling comes back to your limbs.",
        "$n shivers a bit and flexes $s muscles.",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "wands", TRUE,
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        {  2,  2,  4,  5,  6,  3,  2,  5,  6,  4,  2,  3,  1,  3 },
        spell_null,                    TAR_IGNORE,                POS_STANDING,
        &gsn_wands,                       0,  12,                           "",
        "!Wands!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "whip", TRUE,
        {  1,  1, 53,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 53 },
        {  6,  5,  0,  3,  3,  6,  5,  4,  6,  4,  2,  2,  3,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_whip,                        0,   0,                           "",
        "!Whip!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "whirlwind", TRUE,
        { 53, 53, 53, 40, 35, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  5,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_CHAR_OFFENSIVE,                POS_FIGHTING,
        &gsn_whirlwind,                   0,   0,                  "whirlwind",
        "!Whirlwind!",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },

    {
        "wing buffet", TRUE,
        { 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_null,                    TAR_IGNORE,                POS_FIGHTING,
        &gsn_wing,                        0,  24,                "wing buffet",
        "You rub the debris out of your eyes.",
        "",
        { 99, 30, 30, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },
	
	// Seraph Spells
	    {
        "angelic lance", TRUE,
        { 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50 },
        {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        spell_angelic_lance,                    TAR_IGNORE,                POS_STANDING,
        NULL,                        50,  8,                "",
        "!angelicLance",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },
	
	    {
        "divinity", TRUE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_guildspell,              TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            10,  24,                           "",
        "You feel less divine.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },
	
	{
        "godspeed", TRUE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_godspeed,              TAR_CHAR_SELF,             POS_FIGHTING,
        NULL,                            80,  24,                           "",
        "You feel less agile.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },
	
	{
        "tabula rasa", TRUE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_tabula_rasa,              TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
        NULL,                            30,  10,                           "",
        "You feel your true powers return.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },
	
	{
        "hallowed discord", TRUE,
        { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 },
        {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
        spell_hallowed_discord,              TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
        NULL,                            30,  10,                           "",
        "You feel liberated.",
        "",
        { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    },
	
};

const   struct  group_type      group_table     [MAX_GROUP]     =
{

    {
	"rom basics",		{  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1 },
	{ "scrolls", "staves", "wands", "recall", "identify", "locate object" }
      },

    {
	"mage basics",		{  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "staff" }
      },

    {
	"cleric basics",        { -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "mace" }
      },
   
    {
	"monk basics",		{ -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "hand to hand", "second attack" }
      },

    {
	"warrior basics",	{ -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "sword", "second attack" }
      },
    
    { 
        "barbarian basics",     { -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { "axe",   "second attack", "third attack" }
      },
    
    { 
        "psionicist basics",    { -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { "staff", "meditation" }
      },

    {
        "druid basics",         { -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1 },
        { "staff" }
      },
    
    {
        "ranger basics",        { -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1 },
        { "sword", "sneak" }
      },
 
    {  
        "rogue basics",         { -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1 },
        { "dagger", "backstab" }
      },

    {
        "bard basics",          { -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1 },
        { "flail", "ventriloquate" }
      },

    {
       "wild basics",           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1 },
       { "whip" }
      },

    {
       "warlock basics",        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1 },
       { "polearm" }
      },

    {
       "necro basics",          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1 },
       { "staff" }
      },

    {
       "templar basics",        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1 },
       { "spear", "celestial spear" }
      },

    {
	"mage default",		{ 65, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "beguiling", "combat", "detection", "enhancement", "elemental force", "second attack",
	  "maledictions", "protective", "transportation", "dodge", "parry" }
      },

    {
	"cleric default",	{ -1, 65, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "attack", "maledictions", "curative",  "benedictions", "detection", "weather", "kick",
	  "enhanced damage", "second attack", 
          "healing", "transportation", "protective", "shield block", "dodge", "parry" }
      },
 
    {
	"monk default",	        { -1, -1, 70, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "fighter detects", "palm block", "enhancement", "kick", "armthrow", 
	  "crane kick", "enhanced damage", "parry", "dirt kicking", "vital strike", "dodge",
          "protective", "burning palm", "third attack", "dragon punch", "transportation",
          "divine focus" }
      },

    {
	"warrior default",	{ -1, -1, -1, 65, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "shield block", "bash", "disarm", "enhanced damage", "fourth attack", "fighter detects",  
	  "parry", "rescue", "third attack", "fast healing", "transportation", "maledictions",
          "dodge", "protective", "dirt kicking", "sneak" }
      },
   
    {   "barbarian default",    { -1, -1, -1, -1, 65, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { "protective", "shield block", "bash", "enhanced damage", "parry",
	  "dislodge", "critical strike", "kick", "sneak", "dirt kicking", "disarm",
          "fourth attack", "rage", "transportation", "dodge", "fifth attack", "hand to hand" }
      },

    {
	"wild default",       { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 65, -1, -1, -1 },
	{ "beguiling", "detection", "enhancement", "wild magic", "second attack",
	  "maledictions", "protective", "transportation", "combat", "dodge", "parry" }
      },

    {   "psionicist default",   { -1, -1, -1, -1, -1, 65, -1, -1, -1, -1, -1, -1, -1, -1 },
        { "telepathy","heightened senses", "psionic attack", "shield block",
          "psionic protection", "second attack", "mental healing", "shadow form",
          "psychic enhancement", "transportation", "dodge", "parry" }
      },

    {   
        "druid default",        { -1, -1, -1, -1, -1, -1, 65, -1, -1, -1, -1, -1, -1, -1 },
        { "healing", "transportation", "natural forces", "beguiling", "detection",
          "protective", "benedictions", "maledictions", "dodge", "parry", "enhanced damage",
	  "shield block", "second attack" }
      },

    {
        "ranger default",        { -1, -1, -1, -1, -1, -1, -1, 65, -1, -1, -1, -1, -1, -1 },
        { "dual wield", "parry", "weather", "entangle", "protective", "benedictions", 
          "dodge", "second attack", "transportation", "detection", "disarm", "enhanced damage",
	  "shield block", "dirt kicking", "third attack" }
      },
   
    {   "rogue default",         { -1, -1, -1, -1, -1, -1, -1, -1, 65, -1, -1, -1, -1, -1 },
        { "shield block", "dodge", "hide", "sneak", "detection", "beguiling", "dirt kicking",
	  "enhanced damage", "third attack", "poison dagger", "protective", "transportation",
          "second attack", "trip", "pick lock", "steal", "peek", "parry" }
      },
   
    {
        "bard default",         { -1 , -1, -1, -1, -1, -1, -1, -1, -1, 65, -1, -1, -1, -1 },
        { "battlesongs", "enhancement", "protective", "detection", "maledictions",
	  "shield block", "second attack", "enhanced damage", "third attack", "trip", 
          "dodge", "dirt kicking", "beguiling", "transportation", "parry" }
      },

    {   "warlock default",     { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 65, -1, -1 },
        { "inflictions", "second attack", "battle focus", "enhanced damage", "shield block",
          "psionic protection", "detection", "maledictions", "psychic enhancement", "weather",
          "transportation", "dodge", "parry", "third attack", "trip", "shadow form" }
      },

    {   "necro default",     { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 65, -1 },
        { "detection", "protective", "maledictions", "necromancy", "second attack",
          "transportation", "dodge", "parry" }
      },

    {   "templar default",
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 80 },
        { "transportation", "protective", "healing", "fighter detects", 
          "templar combat", "templar invocations", "dodge", "parry", 
          "shield block", "second attack", "third attack", "enhanced damage", 
          "lay on hands", "purify", "solemn vow", "divine focus" }
      },
     
    {
	"weaponsmaster",       { -1, -1, -1, 20, 20, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "axe", "dagger", "flail", "mace", "polearm", "spear", "staff", 
          "sword","whip" }
      },

    {
	"attack",	      { -1,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "smite", "flamestrike", "heat metal", "spiritual hammer", "wrath" }
      },

    {
	"beguiling",	     {  6, -1, -1, -1, -1, -1,  6, -1, 5, 4, 6, -1, -1, -1 },
	{ "calm", "charm person", "sleep" }
      },

    {
	"benedictions",	    { -1,  4,  5, -1, -1, -1,  5,  4, -1, -1, -1, -1, -1, -1 },
	{ "bless", "calm", "frenzy", "holy word", "remove curse" }
      },

    {
	"combat",           {  7, -1, -1, 8, -1, -1, -1, -1, -1, -1, 6, -1, -1, -1 },
	{ "acid blast", "burning hands", "chain lightning", "chill touch",
	  "colour spray", "fireball", "lightning bolt", "magic missile",
	  "shocking grasp", "meteor shower" }
      },

    {
	"creation",
        { -1,  4, -1, -1, -1, -1, 4, -1, -1, -1, -1, -1, -1,  5 },
	{ "continual light", "create food", "create spring", "create water",
	  "create rose" }
      },

    {
	"curative",	   { -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "cure blindness", "cure disease", "cure poison", "blood cool", "restore limb" }
      }, 

    {
	"detection",	   {  5,  4, -1, -1, -1, -1,  5,  5,  5,  4,  7,  5, 10, -1 },
 	{ "detect hidden", "detect invis", "detect magic", "detect poison",
        "identify", "locate object", "locate person" } 
      },
    {
        "fighter detects",
       { -1, -1,  4,  4,  4, -1, -1, -1, -1, -1, -1, -1, -1, 5 },
      { "heightened senses", "locate person" }
    },

    {
	"elemental force",       {  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "water bolt", "flashfire", "winter blast", "gas breath",
	  "thunderclap"  }
      },

    {
        "draconian",      { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { "acid breath", "gas breath", "fire breath", "lightning breath",
          "frost breath" }
      },

    {
	"enchantment",	  {  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "enchant armor", "enchant weapon", "fireproof", "recharge",
          "flame blade" }
      },

    { 
	"enhancement",    {  5, -1, 5,  6, -1, -1,  5,  6,  5,  5, 5, -1, 5, -1 },
	{ "giant strength", "haste", "infravision", "refresh" }
      },

    {
	"psionic attack", { -1, -1, -1, -1, -1,  7, -1, -1, -1, -1, -1, -1, -1 },
	{ "mind blast", "infliction", "psychic crush", "psionic blast",
          "psychic field" }
      },

    {
        "psychic enhancement", { -1, -1, -1, -1, -1, 6, -1, -1, -1, -1, -1, 5, -1, -1 },
        { "enhanced strength", "levitation", "adrenaline rush", "psi blade" }
      },

    {   
	"healing",       { -1,  3,  4,  5, -1,  -1,  4,  5, -1, 8, -1, -1, -1,  4 },
 	{ "cure critical", "cure light", "cure serious", "heal",
          "mass healing", "refresh", "resurrect", "major heal",
          "minor heal" }
      },

    {
	"illusion",	{  6, -1, -1, -1, -1, -1,  6, -1,  5, -1, 5, 3, 3, -1 },
	{ "invisibility", "mass invis", "ventriloquate" }
      },
  
    {
	"maledictions",	{  5,  5, -1, 8, -1, -1,  5, -1, 5, 5, 7, 5, 7, -1 },
	{ "blindness", "change sex", "curse", "plague", 
	  "poison", "slow", "weaken", "energy drain" }
      },

    { 
	"protective",	      {  5,  5,  5,  5,  5, -1,  5,  5,  5,  4, 7, -1, 11, 6 },
	{ "armor", "cancellation", "dispel magic", "fireproof", "magic ward",
	  "sanctuary", "shield", "stone skin", "mirror image" }
      },
    
    {   "psionic protection", { -1, -1, -1, -1, -1,  5, -1, -1, -1, -1, -1, 6, -1, -1 },
        { "steel skin", "biofeedback", "mental barrier", "cancellation",
          "astral projection", "mental fortress", "dispel magic", "displace" }
      },

    {
        "mental healing",   { -1, -1, -1, -1, -1,  8, -1, -1, -1, -1, -1, 4, -1, -1 },
         { "complete heal", "psychic heal","cellular purge", "refresh" }
      },

    {   "telepathy",        { -1, -1, -1, -1, -1,  9, -1, -1, -1, -1, -1, -1, -1, -1 },
        { "awe", "psychic drain", "warp time", "blindness", "energy drain",
          "mind whip", "domination", "locate person", "headache", "forget" }
      },

    {
	"templar combat",
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8 },
	{ "burning hands", "chill touch", "shocking grasp", "flamestrike", 
          "harrow" }
      },

    {
	"templar invocations",
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8 },
	{ "judgment", "ordain", "bless", "frenzy", "calm", "inquisition" }
      },

    {
	"celestial spear",   { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "celestial spear" }
      },

    {
	"transportation",   {  5,  5,  5,  5,  5,  5,  5,  5,  5,  4, 6, 5, 8, 6 },
	{ "fly", "gate", "pass door", "portal", "summon", "teleport", 
	  "word of recall" }
      },
   
    {
	"weather",         {  6,  5,  6, -1, -1, -1,  5, 8,  -1, -1, 5, 2, -1, -1 },
	{ "call lightning", "chain lightning", "control weather",
          "earthquake", "faerie fire", "faerie fog",
	  "lightning bolt" }
      },

    {  
       "natural forces",   { -1, -1, -1, -1, -1, -1,  6, -1, -1, -1, -1, -1, -1, -1 },
	 { "natures wrath", "burning hands", "shocking grasp", "fireball", 
          "chill touch", "summon animal", "meteor shower" }
      },

    {
       "battlesongs",     { -1, -1, -1, -1, -1, -1, -1, -1, -1, 4, -1. -1, -1, -1 },
       { "battle wail", "battlecry", "deathsong", "pinch", "battle sorrow",
          "echoes of power" }
      },

    {  "inflictions",     { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 6, -1, -1 },
       { "sorrow", "chaos", "torment", "ravage", "betray" }
      },

    {  "necromancy",     { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8, -1 },
       { "atrophy", "fester", "blister", "decay", "lifetap", "animate dead",
	"shadowstep", "wither limb", "scry", "shield of skulls" }
      },

    {  
       "wild magic",       { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 7, -1, -1, -1 },
       { "chaos demon", "wild shield", "wild aura", "wild heal", 
         "wild enhance", "wildfire", "vanish", "chaotic dispersal","wild summon" }
    }

};



/* WERE 's CORRESPOND TO THE RACES BY INDEX */
const	WERE_TYPE were_table	[]	=
{

/*
    {
	"short desc for half form", 	
	"long desc for half form ",
	"short desc for full form",
	"long desc for full form",
	stradd for half,
	dex add for half,
	str add for full,
	dex add for full,
	resistances half,
	vulnerabilities half,
	resistances full,
	vulnerabilities full,
	affects half,
	affects full,
	hps to add,
	mana to add
	
    },
*/
    { 
	NULL,
	NULL,
	NULL,
	NULL,
	1, 3, 1, 4,
	RES_MAGIC,
	VULN_BASH,
	RES_MAGIC,
	VULN_BASH,
	AFF_FLYING,
	AFF_FLYING,
	100,
	50	
    },

/* CONDOR */
    { 
	"a feathered humanoid",
	"A feathered humanoid stretches its wings here.",
	"a giant condor",
	"A giant condor circles overhead.",
	1, 3, 1, 4,
	RES_MAGIC,
	VULN_BASH | VULN_SILVER,
	RES_MAGIC,
	VULN_BASH | VULN_SILVER,
	AFF_FLYING,
	AFF_FLYING,
	100,
	50	
    },

/* DRAGON */
    { 
        "a large winged reptilian",
	"A large, slithering reptilian with small wings calmly glances at you.",
	"a large dragon",
	"A large dragon stares intently at you.",
	3, 2, 4, 2,
	RES_BASH,
	VULN_PIERCE | VULN_SILVER,
	RES_BASH,
	VULN_PIERCE | VULN_SILVER,
	AFF_FLYING,
	AFF_FLYING,
	100,
	50	
    },

/* GIANT SPIDER */
    { 
	"a horrid, black arachnos",
	"A black, spider-like humanoid lurks in the shadows.",
	"a giant black spider",
	"An enormous black spider views you with its many eyes.",
	1, 3, 2, 2,
	RES_MAGIC,
	VULN_BASH | VULN_SILVER,
	RES_MAGIC,
	VULN_BASH | VULN_SILVER,
	0,
	0,
	100,
	50	
    },

/* WOLVERINE */
    { 
	"a sharp-clawed humanoid",
	"A ferocious, coarse-haired humanoid with sharp claws is here.",
	"a tenacious wolverine",
	"An enormous, snarling wolverine prowls here.",
	2, 2, 2, 3,
	RES_SLASH | RES_BASH,
	VULN_PIERCE | VULN_LIGHTNING | VULN_SILVER,
	RES_SLASH | RES_BASH,
	VULN_PIERCE | VULN_LIGHTNING | VULN_SILVER,
	0,
	0,
	100,
	50	
    },



/* PANTHER */
    { 
	"a powerful feline humanoid",
	"A feline humanoid with sharp incisors stalks the room.",
	"a vicious panther",
	"A sleek, powerful panther looks hungrily around.",
	2, 3, 2, 4,
	RES_PIERCE,
	VULN_BASH | VULN_SILVER,
	RES_PIERCE,
	VULN_BASH | VULN_SILVER,
	AFF_SNEAK,
	AFF_SNEAK,
	100,
	50	
    },

/* WOLF  */
    { 
	"a wolf-like creature",
	"An eerie, wolf-like humanoid sniffs at the fear in the air.",
	"a large wolf",
	"A large wolf with wicked teeth considers its next prey.",
	2, 2, 2, 3,
	RES_MAGIC,
	VULN_SLASH | VULN_SILVER,
	RES_MAGIC,
	VULN_SLASH | VULN_SILVER,
	AFF_SNEAK,
	AFF_SNEAK,
	100,
	50	
    },

/* TIGER */
    { 
	"a striped feline humanoid",
	"A muscular, feline humanoid with dark stripes prowls nearby.",
	"a regal looking tiger",
	"A regal tiger confidently peers about the room.",
	3, 2, 3, 3,
	RES_SLASH,
	VULN_PIERCE | VULN_SILVER,
	RES_SLASH,
	VULN_PIERCE | VULN_SILVER,
	0,
	0,
	100,
	50	
    },


/* Dracolich */
    { 
	"a large, leathery winged skeleton",
	"A large, leathery winged skeleton looks menacingly at you!",
	"a dracolich",
	"A large Dracolich stares down at you with glowing red eyes.",
	3, 2, 3, 3,
	RES_PIERCE|RES_SLASH,
	VULN_HOLY | VULN_SILVER | VULN_BASH,
	RES_FIRE|RES_LIGHTNING|RES_POISON,
	VULN_HOLY | VULN_SILVER | VULN_BASH,
	AFF_FLYING,
        AFF_FLYING,
	100,
	50	
    },

/* Sea Devil */
    { 
	"a gilled, bipedal reptilian",
	"A bipedal reptile with thin gills flickers its tongue in the air.",
	"a sea devil",
	"A long, slender sea dragon with pale blue scales rests here.",
	3, 2, 3, 3,
	RES_MAGIC,
	VULN_FIRE | VULN_SILVER,
	RES_MAGIC,
	VULN_FIRE | VULN_SILVER,
	AFF_SWIM,
	AFF_SWIM,
	100,
	50	
    },

/* Bear */
    { 
	"a towering, bearlike humanoid",
	"A knuckle-dragging, bearlike humanoid lumbers towards you.",
	"a large, menacing bear",
	"A large, menacing bear rears up and growls in the air.",
	3, 1, 4, 1,
	RES_WEAPON,
	VULN_NEGATIVE | VULN_SILVER,
	RES_WEAPON,
	VULN_NEGATIVE | VULN_SILVER,
	0,
	0,
	100,
	50	
    },

/* BOAR */
    { 
	"a tusked, porcine humanoid",
	"A tusked, porcine humanoid with coarse hairs grunts at you.",
	"a sharp-tusked boar",
	"A grotesque boar with bristling coarse hairs bares its tusks at you.",
	3, 1, 3, 2,
	RES_BASH,
	VULN_SLASH | VULN_SILVER,
	RES_BASH,
	VULN_SLASH | VULN_SILVER,
	0,
	0,
	100,
	50	
    },

/* LYNX */
    { 
	"a bipedal feline",
	"A spotted bipedal with long whiskers snarls at you.",
	"a spotted lynx",
	"A large lynx with dark spots waves its tail nervously at you.",
	1, 3, 2, 3,
	RES_PIERCE,
	VULN_BASH | VULN_SILVER,
	RES_PIERCE,
	VULN_BASH | VULN_SILVER,
	0,
	0,
	100,
	50	
    },

/* FIEND */
    {
        "a demonic half-fiend",
        "A fiendish looking humanoid bears fangs and claws menacingly.",
        "a fiend",
        "A large fiendish creature seems lost to the darkness.",
        1, 3, 2, 2,
        RES_NEGATIVE | RES_MENTAL,
        VULN_LIGHT | VULN_COLD | VULN_SILVER,
        RES_NEGATIVE | RES_FIRE,
        VULN_LIGHT | VULN_COLD | VULN_SILVER, 
        0,
        0,
        100,
        50
    },

/* MEERKAT */
    {
        "a thin, hair-covered creature with a long nose",
        "A creature resembing a cross between a mongoose and a kine sniffs at you.",
        "a curious meerkat of unusual size",
        "A rather large meerkat sniffs at the air curiously.",
        3, 1, 4, 1,
        RES_POISON,
        VULN_ENERGY | VULN_DISEASE | VULN_SILVER,
        IMM_POISON,
        VULN_ENERGY | VULN_DISEASE | VULN_SILVER,
        0,
        0,
        100,
        50
    }
};

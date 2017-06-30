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

#ifndef __MERC_H
#define __MERC_H

#include <stdio.h>
#include <time.h>

#include "pueblo.h"


/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )	void fun( )
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#endif

/* system calls */
int unlink();
int system();

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int				sh_int;
typedef int				bool;
#define unix
#else
typedef short   int			sh_int;
typedef unsigned char			bool;
#endif

/* struct clan defined in clans code */
typedef struct clanNode Clan;

/*
 * Structure types.
 */
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct 	buf_type	 	BUFFER;
typedef struct	char_data		CHAR_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct  disabled_data		DISABLED_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	help_area_data		HELP_AREA;
typedef struct	kill_data		KILL_DATA;
typedef struct	mem_data		MEM_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct  badname_data		BADNAME_DATA;
typedef struct  multilogin_data		MULTILOGIN_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct  mprecog_data            MPRECOG_DATA;
typedef struct  mpdelay_data            MPDELAY_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef struct  gen_data		GEN_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	weather_data		WEATHER_DATA;
typedef struct  mprog_list		MPROG_LIST;
typedef struct  mprog_code		MPROG_CODE;
typedef struct  recipe                  RECIPE;
typedef struct  trade_data              TRADE_DATA;
typedef struct  bankacct_data           BANKACCT_DATA;
typedef struct  int_list                INT_LIST;
typedef struct  journal_data            JOURNAL_DATA;
typedef struct  quest_data              QUEST_DATA;
typedef struct  scoreboard_item         SCOREBOARD_ITEM;       /* Oct2011 gkl */
typedef struct  brawler_data            BRAWLER_DATA;          /* Oct2011 gkl */
typedef struct  fight_record            FIGHT_RECORD;          /* Oct2011 gkl */

/*
 * Function types.
 */
typedef	void DO_FUN	args( ( CHAR_DATA *ch, char *argument ) );
typedef bool SPEC_FUN	args( ( CHAR_DATA *ch ) );
typedef void SPELL_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo,\
				int target ) );

/*
 * Some interesting constants for PK
 * 8/26/06 values altered for experimental testing
 */
#define CONFIG_DIRTKICK_FMANA_CAP   100
#define CONFIG_TRIP_FMANA_CAP       100
#define CONFIG_REFLECT_CHANCE       20
#define CONFIG_CLEAR_CHANCE         25
#define CONFIG_CLEAR_LAG            12 /* 2.5 seconds */
#define CONFIG_HR_PARRY_SCALAR       7 /* = 1% loss in parry chance */
#define CONFIG_HR_DODGE_SCALAR      10 /* = 1% loss in dodge chance */
/* 
 * parameters to determine the max saves needed for fmana and non-fmana classes
 *   SCALEN is the numerator, SCALED is the denominator.  for the mathematically
 *   disadvantaged, cap/90 = SCALEN/SCALED.  So if you want the cap for fmana 
 *   classes to be -45 saves, -45/90 = 1/2 so SCALEN=1 and SCALED=2
 */
#define CONFIG_SAVES_FMANA_SCALEN    2
#define CONFIG_SAVES_FMANA_SCALED    1
#define CONFIG_SAVES_NOFMANA_SCALEN  9
#define CONFIG_SAVES_NOFMANA_SCALED  4
#define CONFIG_SOLEMN_VOWS_CHANCE   50
/*
 * AFFECTS DEFINES 
 */
#define AFF1 1
#define AFF2 2
#define AFF3 3

/*
 * String and memory management parameters.
 */
#define	MAX_KEY_HASH		 1024
#define MAX_STRING_LENGTH	 4608
#define MAX_INPUT_LENGTH	  512
#define PAGELEN			   33

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_NOTES_PER_LIST      100
#define MAX_STAT_LIMIT          27
#define MAX_SOCIALS             300
#define MAX_SKILL               500   /* increment for new skills/spells */
#define MAX_GROUP               100
#define MAX_IN_GROUP            18
#define MAX_CLASS               14
#define MAX_PC_RACE             16
#define MAX_GUILD               15
#define MAX_DAMAGE_MESSAGE      65
#define MAX_LEVEL               60
#define MAX_GOD                 5
#define MAX_LIQUID              89
#define LEVEL_ACOLYTE           (MAX_LEVEL - 30)
#define LEVEL_ADEPT             (MAX_LEVEL - 20)
#define LEVEL_HERO              (MAX_LEVEL - 10)
#define LEVEL_ADMIN             (MAX_LEVEL - 1)
#define LEVEL_IMMORTAL          (MAX_LEVEL - 9)

#define FIGHT_LAG               90
#define PUSH_LIMIT              2

#define NO_SKILL		   -5  /* What skill_lookup returns when
					  it can't find a skill */
#define PULSE_PER_SECOND	    4
#define PULSE_VIOLENCE		  ( 3 * PULSE_PER_SECOND)
#define PULSE_MOBILE		  ( 4 * PULSE_PER_SECOND)
#define PULSE_MUSIC		  ( 6 * PULSE_PER_SECOND)
#define PULSE_TICK		  (45 * PULSE_PER_SECOND)
#define PULSE_AREA		  (90 * PULSE_PER_SECOND)

#define IMPLEMENTOR		MAX_LEVEL
#define	ADMIN			(MAX_LEVEL - 1)
#define SUPREME			(MAX_LEVEL - 2)
#define DEITY			(MAX_LEVEL - 3)
#define GOD			(MAX_LEVEL - 4)
#define IMMORTAL		(MAX_LEVEL - 5)
#define DEMI			(MAX_LEVEL - 6)
#define ANGEL			(MAX_LEVEL - 7)
#define AVATAR			(MAX_LEVEL - 8)
#define HERO			LEVEL_HERO

/*
 * Gods
 */
#define GOD_MOB         0
#define GOD_NONE        0
#define GOD_RHIAN       1
#define GOD_TYRIN       2
#define GOD_SYRIN       3

/*
 * Colour stuff adapted from Lope
 */
#define CLEAR		"[0m"		/* Resets Colour	*/
#define C_RED		"[0;31m"	/* Normal Colours	*/
#define C_GREEN		"[0;32m"
#define C_YELLOW	"[0;33m"
#define C_BLUE		"[0;34m"
#define C_MAGENTA	"[0;35m"
#define C_CYAN		"[0;36m"
#define C_WHITE		"[0;37m"
#define C_D_GREY	"[1;30m"  	/* Light Colors		*/
#define C_B_RED		"[1;31m"
#define C_B_GREEN	"[1;32m"
#define C_B_YELLOW	"[1;33m"
#define C_B_BLUE	"[1;34m"
#define C_B_MAGENTA	"[1;35m"
#define C_B_CYAN	"[1;36m"
#define C_B_WHITE	"[1;37m"
#define BIG_BLUE_LINE   "{B================================================================================{x\n\r"
#define C_OOC       0
#define C_IC        1
#define C_GT        2
#define C_EMOTE     3
#define C_NB        3
#define C_TELL      4
#define C_REPLY     5
#define C_GDT       6
#define C_SAY       7
#define C_HP        8   /* old slot 8 was hp    */
#define C_SHOUT     8   /* new slot 8 is shout  */
#define C_MAXHP     9   /* old slot 9 was maxhp */
#define C_YELL      9   /* new slot 9 is yell   */
#define C_MANA      10
#define C_MAXMANA   11
#define C_MOVES     12
#define C_MAXMOVES  13
#define C_IMM       14
#define C_YBATTLE   15 
#define C_TBATTLE   16 
#define C_MINION    17 
#define C_AUCTION   18 
#define C_ADMIN     19 
#define C_WIZNET    20 
#define C_VSAY      21
#define C_BRAWLER   22
#define C_QSAY      23
#define C_QTSAY     24
#define C_CLANTALK  25
#define MAX_COLOR   26

struct color_settings
{
    const char *field;
    const char color;
};
extern const struct color_settings colordefs[MAX_COLOR];

#define QTYPE_XP        (A)
#define QTYPE_SKILL     (B)
#define QTYPE_MYSTERY   (C)

struct quest_data
{
    QUEST_DATA          *next;
    AREA_DATA           *area;          /* quest can be found in this area  */
    JOURNAL_DATA        *journal;       /* associated journal entries */
    char                *name;          /* name of quest to appear in journal */
    int                 flags;
    bool                valid;
    sh_int              completevnum;   /* flag to signify quest complete */
    int                 completeflag;   /* flag to signify quest complete */
};

/*
 * structs added by gkl
 */
struct journal_data
{
    JOURNAL_DATA        *next;
    QUEST_DATA          *quest;         /* associated quest */
    char                *text;          /* journal text */
    int                 flag;           /* mprecog flag */
    bool                valid;
    sh_int              step;           /* indicator of progress in quest */
    sh_int              vnum;           /* mprecog vnum */
};

struct int_list
{
    INT_LIST *          next;
    int                 value[2];
};

#define ACCT_PERSONAL   0
#define ACCT_GUILD      1
struct bankacct_data
{
    BANKACCT_DATA       *next;
    bool                valid;
    char                *owner;
    int                 type;
    long                gold;
    long                silver;
};

struct  mprecog_data
{
    MPRECOG_DATA *      next;
    bool                valid;
    sh_int              vnum;
    int                 flags;
    int                 temp;    /* Unused */
    INT_LIST *          timer;
};

struct  mpdelay_data
{
    MPDELAY_DATA *	next;
    bool		valid;
    bool		execute;
    sh_int		pvnum;
    char *		code;
    CHAR_DATA *	mob;
    CHAR_DATA *	ch;
    const void *	arg1;
    const void *	arg2;
};

#define BADNAME_EXACT     0
#define BADNAME_PREFIX    1
#define BADNAME_SUFFIX    2
#define BADNAME_INFIX     3

struct badname_data
{
    BADNAME_DATA *next;
    bool valid;
    short type;
    char * name;
};

#define MULTILOGIN_SUFFIX	A
#define MULTILOGIN_PREFIX	B
#define MULTILOGIN_SCHOOL	C
#define MULTILOGIN_COMPANY	D
#define MULTILOGIN_HOME_LAN	E

struct	multilogin_data
{
    MULTILOGIN_DATA *	next;
    bool	valid;
    sh_int	multilogin_flags;
    sh_int	level;
    char *	name;
    char *	comment;
};


#define BAN_SUFFIX		A
#define BAN_NEWBIES		B
#define BAN_ALL			C	
#define BAN_PERMIT		D
#define BAN_PERMANENT		E

struct	ban_data
{
    BAN_DATA *	next;
    bool	valid;
    sh_int	ban_flags;
    sh_int	level;
    char *	name;
    char *	comment;
  time_t stamp;
};


struct buf_type
{
    BUFFER *    next;
    bool        valid;
    sh_int      state;  /* error state of the buffer */
    sh_int      size;   /* size in k */
    char *      string; /* buffer's string */
};


/* Trade skill stuff */

struct   recipe
{
    int         tradeskill;		/* Bitvec saying which skill is used */
    int		wearloc;                /* For patterns */
    int         focusitem;              /* something that helps */
    int		component[10];		/* Components, up to 10 */
    int		goods[10];		/* Things created by the components */
    int   	improve_level;		/* Level base for the improvability range */
    int		trade_level;		/* Trade skill level needed! */
    sh_int	racial_bonus[MAX_PC_RACE]; /* Race bonus? */
    sh_int      class_bonus[MAX_CLASS];    /* Class bonus? */    
    
};

struct trade_data
{
  int trade;            /* Bit vector for which trade */
  char* trade_name;     /* String name for the trade (ie. "Fletching") */
  bool room_obj;        /* TRUE if trade requires an in-room object */
  int room_obj_vnum;    /* vnum for in-room object */
  char* no_room_obj;    /* message if a required room object isn't present */
  char* no_args;        /* message if no ingredients given */
  char* reuse_msg;      /* message to character for reusing ingredients */
  char* dam_msg_char;   /* message for failure (damage message) */
  char* dam_msg_room;   /* message for failure (to room) */
  int dam_type;         /* damage type on failure */
  int damage;           /* amount of damage rendered on failure */
  char* success_char;   /* success message to character */
  char* success_room;   /* success message to room */
  int skill_cap;        /* maximum skill value */
  int chance_low;       /* chance of improvement (1-100) for lower levels */
  int chance_high;      /* chance of improvement (1-100) for higher levels */
  int high_level;       /* level at which chance_high replaces chance_low */
  char* improve_msg;    /* message to char when skill improves */
  const RECIPE* recipe_set;
};

#define TAILORING 	  (A)
#define BAKING	 	  (B)
#define	FLETCHING	  (C)

/* End trade stuff */

/*
 * Time and weather stuff.
 */
#define SUN_DARK		    0
#define SUN_RISE		    1
#define SUN_LIGHT		    2
#define SUN_SET			    3

#define SKY_CLOUDLESS		    0
#define SKY_CLOUDY		    1
#define SKY_RAINING		    2
#define SKY_LIGHTNING		    3

struct	time_info_data
{
    int		hour;
    int		day;
    int		month;
    int		year;
    
};

struct	weather_data
{
    int		mmhg;
    int		change;
    int         cwmod;    /* For control weather spell */
    int		sky;
    int		sunlight;
};



/*
 * Connected state for a channel.
 */
#define CON_PLAYING			 0
#define CON_GET_NAME			 1
#define CON_GET_OLD_PASSWORD		 2
#define CON_CONFIRM_NEW_NAME		 3
#define CON_GET_NEW_PASSWORD		 4
#define CON_CONFIRM_NEW_PASSWORD	 5
#define CON_GET_NEW_RACE		 6
#define CON_GET_NEW_SEX			 7
#define CON_GET_NEW_CLASS		 8
#define CON_GET_GOD     		 9
#define CON_DEFAULT_CHOICE		10 
#define CON_GEN_GROUPS			11 
#define CON_PICK_WEAPON			12
#define CON_READ_PREGREETING		13
#define CON_READ_MOTD			14
#define CON_BREAK_CONNECT		15
#define CON_ANSI                        16
#define CON_CHOOSE_TERM			17
#define CON_READ_GREETING	        18


/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
  
  DESCRIPTOR_DATA*	next;
  DESCRIPTOR_DATA*	snoop_by;
  time_t		snoop_time;
  CHAR_DATA *		character;
  CHAR_DATA *		original;
  bool		valid; 
  char *		host;
  sh_int		descriptor;
  sh_int		connected;
  bool		fcommand;
  char		inbuf		[4 * MAX_INPUT_LENGTH];
  char		incomm		[MAX_INPUT_LENGTH];
  char		inlast		[MAX_INPUT_LENGTH];
  int			repeat;
  char *		outbuf;
  int			outsize;
  int			outtop;
  char *		showstr_head;
  char *		showstr_point;
  bool                ansi;
  bool		pueblo;
  void *              pEdit;		/* OLC */
  char **             pString;	/* OLC */
  int			editor;		/* OLC */
};



/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
    sh_int	tohit;
    sh_int	todam;
    sh_int	carry;
    sh_int	wield;
};

struct	int_app_type
{
    sh_int	learn;
};

struct	wis_app_type
{
    sh_int	practice;
};

struct	dex_app_type
{
    sh_int	defensive;
};

struct	con_app_type
{
    sh_int	hitp;
    sh_int	shock;
};



/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3
#define TO_ALL		    4



/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA *	next;
    HELP_DATA * next_area;
    sh_int	level;
    int         limits;
    char *	keyword;
    char *	text;
};

struct help_area_data
{
	HELP_AREA *	next;
	HELP_DATA *	first;
	HELP_DATA *	last;
	AREA_DATA *	area;
	char *		filename;
};

/*
 * Shop types.
 */
#define MAX_TRADE	 5
struct	shop_data
{
    SHOP_DATA *	next;			/* Next shop in list		*/
    sh_int	keeper;			/* Vnum of shop keeper mob	*/
    sh_int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    sh_int	profit_buy;		/* Cost multiplier for buying	*/
    sh_int	profit_sell;		/* Cost multiplier for selling	*/
    sh_int	open_hour;		/* First opening hour		*/
    sh_int	close_hour;		/* First closing hour		*/
};



/*
 * Per-class stuff.
 */

#define MAX_CLASSGUILD 	2
#define MAX_STATS 	5
#define STAT_STR 	0
#define STAT_INT	1
#define STAT_WIS	2
#define STAT_DEX	3
#define STAT_CON	4

struct	class_type
{
    char *	name;			/* the full name of the class   */
    char 	who_name	[4];	/* Three-letter name for 'who'	*/
	char cls_color[2];			 /* The color the class shows as in who */
    sh_int	attr_prime;		/* Prime attribute		*/
    sh_int	attr_sec;		/* Secondary Attribute *Kyuss*  */
    sh_int	weapon;			/* First weapon			*/
    sh_int	guild[MAX_GUILD];	/* Vnum of guild rooms		*/
    sh_int	skill_adept;		/* Maximum skill level		*/
    sh_int	thac0_00;		/* Thac0 for level  0		*/
    sh_int	thac0_32;		/* Thac0 for level 32		*/
    sh_int	hp_min;			/* Min hp gained on leveling	*/
    sh_int	hp_max;			/* Max hp gained on leveling	*/
    bool	fMana;			/* Class gains mana on level	*/
    char *	base_group;		/* base skills gained		*/
    char *	default_group;		/* default skills gained	*/
    sh_int      wstimer;                /* cooldown on special weap v4s */
    sh_int *    vorpalspell;            /* spell cast by vorpal weapons */
};

struct item_type
{
    int		type;
    char *	name;
};

struct weapon_type
{
    char *	name;
    sh_int	vnum;
    sh_int	type;
    sh_int	*gsn;
};

struct wiznet_type
{
    char *	name;
    long 	flag;
    int		level;
};

struct attack_type
{
    char *	name;			/* name */
    char *	noun;			/* message */
    int   	damage;			/* damage class */
};

struct race_type
{
  char *	name;			/* call name of the race */
  bool	pc_race;		/* can be chosen by pcs */
  bool     remort_race; /*is it a remort/special race*/
  bool visible; /* is the race visible during creation (only visibility, not availability ) */
    long	act;			/* act bits for the race */
    long	aff;			/* aff bits for the race */
    long	aff2;			/* aff2 bits for the race */
    long	aff3;			/* aff3 bits for the race */
    long	off;			/* off bits for the race */
    long	imm;			/* imm bits for the race */
    long        res;			/* res bits for the race */
    long	vuln;			/* vuln bits for the race */
    long	form;			/* default form flag for the race */
    long	parts;			/* default parts for the race */
};


struct pc_race_type  /* additional data for pc races */
{
    char *	name;			/* MUST be in race_type */
    char 	who_name[6];
    sh_int	points;			/* cost in points of the race */
    sh_int	class_mult[MAX_CLASS];	/* exp multiplier for class, * 100 */
    char *	skills[5];		/* bonus skills for the race */
    sh_int 	stats[MAX_STATS];	/* starting stats */
    sh_int	max_stats[MAX_STATS];	/* maximum stats */
    sh_int	size;			/* aff bits for the race */
};


struct spec_type
{
    char * 	name;			/* special function name */
    SPEC_FUN *	function;		/* the function */
};

struct god_type
{
     char *     name;                   /* call name of the god */
     int        rose;                   /* create rose's rose vnum */
     char *     descr;
     char *	aura;
};

/* Wand/Staff Table */
#define MAX_WANDSTAFF            2
struct	wandstaff_type
{
    char * name;
    char * wand_zap_ch_c;
    char * wand_zap_ch_v;
    char * wand_zap_ch_o;
    char * wand_zap_ob_c;
    char * wand_zap_ob_o;
    char * wand_fail_c;
    char * wand_fail_o;
    char * wand_die_c;
    char * wand_die_o;
    char * staff_brand_c;
    char * staff_brand_o;
    char * staff_fail_c;
    char * staff_fail_o;
    char * staff_die_c;
    char * staff_die_o;
};

/*
 * Liquids.
 */
#define LIQ_WATER        0
struct	liq_type
{
    char *	liq_name;
    char *	liq_color;
    sh_int	liq_affect[2];
    bool        disabled;
};

/*
 * Skills include spells as a particular case.
 */
struct	skill_type
{
  char *	name;			/* Name of skill		*/
  bool  selectable; /*whether or not you can select the skill on creation*/
  sh_int	skill_level[MAX_CLASS];	/* Level needed by class	*/
  sh_int	rating[MAX_CLASS];	/* How hard it is to learn	*/	
  SPELL_FUN *	spell_fun;		/* Spell pointer (for spells)	*/
  sh_int	target;			/* Legal targets		*/
  sh_int	minimum_position;	/* Position for caster / user	*/
  sh_int *	pgsn;			/* Pointer to associated gsn	*/
  /*  sh_int	slot;		*/	/* Slot for #OBJECT loading	*/
  sh_int	min_mana;		/* Minimum mana used		*/
  sh_int	beats;			/* Waiting time after use	*/
  char *	noun_damage;		/* Damage message		*/
  char *	msg_off;		/* Wear off message		*/
  char *	msg_obj;		/* Wear off message for obects	*/
  sh_int	were_level[MAX_PC_RACE];/* Level needed by were	*/
};

struct  group_type
{
    char *	name;
    sh_int	rating[MAX_CLASS];
    char *	spells[MAX_IN_GROUP];
};


typedef struct were_type /* were creature structure */
{
    char *	short_half;	/* short description for half shift */
    char *	long_half;	/* long description for half shift */
    char *	short_full;	/* short description for full shift */
    char *	long_full;	/* long description for full shift */
    sh_int	str_half;	/* strength addition for half shift */ 
    sh_int	dex_half; 	/* dex addition for half shift */ 
    sh_int	str_full;	/* strength addition for full shift */ 
    sh_int	dex_full; 	/* dex addition for full shift */ 
    long	res_half;	/* resistances for half shift */
    long	vuln_half;	/* vulnerabilities for half shift */
    long	res_full;	/* resistances for full shift */
    long	vuln_full;	/* vulnerabilities for full shift */
    long	aff_half;	/* affects in half shift */
    long	aff_full;	/* affects in full shift */
    sh_int	hpadd;		/* hps to add when shifted */
    sh_int	manaadd;	/* mana to add when shifted */
} WERE_TYPE;


extern	const	struct	class_type	class_table	[MAX_CLASS];
extern  const   struct  god_type        god_table       [MAX_GOD];
extern	const	struct	weapon_type	weapon_table	[];
extern  const   struct  item_type	item_table	[];
extern	const	struct	wiznet_type	wiznet_table	[];
extern	const	struct	attack_type	attack_table	[];
extern  const	struct  race_type	race_table	[];
extern  const	struct  were_type	were_table	[];
extern	const	struct	pc_race_type	pc_race_table	[];
extern  const	struct	spec_type	spec_table	[];
extern	const	struct	liq_type	liq_table	[];
extern	const	struct	wandstaff_type	wandstaff_table	[MAX_WANDSTAFF];
extern	const	struct	skill_type	skill_table	[MAX_SKILL];
extern  const   struct  group_type      group_table	[MAX_GROUP];
extern          struct social_type*      social_table;

/*
 * Data structure for notes.
 */

#define NOTE_NOTE	0
#define NOTE_IDEA	1
#define NOTE_HISTORY	2
#define NOTE_NEWS	3
#define NOTE_CHANGES	4
#define NOTE_TROUBLE    5
#define NOTE_GOSSIP    6
/*define NOTE_LORE       6*/

struct	note_data
{
    NOTE_DATA *	next;
    bool 	valid;
    bool        persist;
    sh_int	type;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    char *	text;
    time_t  	date_stamp;
};



/*
 * An affect.
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    bool		valid;
    sh_int		where;
    sh_int		type;
    sh_int		level;
    sh_int		duration;
    sh_int		location;
    sh_int		modifier;
    sh_int		whichaff;		/* affects bit one or 2 */
    int			bitvector;
};

/* where definitions */
#define TO_AFFECTS	0
#define TO_OBJECT	1
#define TO_IMMUNE	2
#define TO_RESIST	3
#define TO_VULN		4
#define TO_WEAPON	5
#define TO_SPELLS       6


/*
 * A kill structure (indexed by level).
 */
struct	kill_data
{
    sh_int		number;
    sh_int		killed;
};



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_FIDO		    3090
#define MOB_VNUM_CITYGUARD	    3060
#define MOB_VNUM_VAMPIRE	    3404
#define MOB_VNUM_SKELETAL       14
#define MOB_VNUM_FAMILIAR       15
#define MOB_VNUM_BATS           16
#define MOB_VNUM_SUNSKELLY      17

#define MOB_VNUM_PATROLMAN	    2106
#define GROUP_VNUM_TROLLS	    2100
#define GROUP_VNUM_OGRES	    2101

#define A		  	1
#define B			2
#define C			4
#define D			8
#define E			16
#define F			32
#define G			64
#define H			128

#define I			256
#define J			512
#define K		        1024
#define L		 	2048
#define M			4096
#define N		 	8192
#define O			16384
#define P			32768

#define Q			65536
#define R			131072
#define S			262144
#define T			524288
#define U			1048576
#define V			2097152
#define W			4194304
#define X			8388608

#define Y			16777216
#define Z			33554432
#define aa			67108864 	/* doubled due to conflicts */
#define bb			134217728
#define cc			268435456    
#define dd			536870912
#define ee			1073741824
#define MAX_FLAG                31

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		(A)		/* Auto set for mobs	*/
#define ACT_SENTINEL	    	(B)		/* Stays in one room	*/
#define ACT_SCAVENGER	      	(C)		/* Picks up objects	*/
#define ACT_NOKILL		(D)		/* Cannot be attacked	*/
#define ACT_NOTSEEN		(E)
#define ACT_AGGRESSIVE		(F)    		/* Attacks PC's		*/
#define ACT_STAY_AREA		(G)		/* Won't leave area	*/
#define ACT_WIMPY		(H)
#define ACT_PET			(I)		/* Auto set for pets	*/
#define ACT_TRAIN		(J)		/* Can train PC's	*/
#define ACT_PRACTICE		(K)		/* Can practice PC's	*/
#define ACT_WIZI		(L)
#define ACT_NOLOCATE            (M)

#define ACT_UNDEAD		(O)	
#define ACT_NOGIVE              (P) /* Used to be DRMODE.  Some players may
                                       have this enabled.  Only ref in code has
                                       IS_NPC check preceding though; should be
                                       ok */
#define ACT_CLERIC		(Q)
#define ACT_MAGE		(R)
#define ACT_THIEF		(S)
#define ACT_WARRIOR		(T)
#define ACT_WERETRAIN		(U)
#define ACT_NOPURGE		(V)
#define ACT_OUTDOORS		(W)

#define ACT_INDOORS		(Y)
#define ACT_IS_HEALER		(aa)
#define ACT_GAIN		(bb)
#define ACT_UPDATE_ALWAYS	(cc)
#define ACT_IS_CHANGER		(dd)

/* damage classes */
#define DAM_NONE                0
#define DAM_BASH                1
#define DAM_PIERCE              2
#define DAM_SLASH               3
#define DAM_FIRE                4
#define DAM_COLD                5
#define DAM_LIGHTNING           6
#define DAM_ACID                7
#define DAM_POISON              8
#define DAM_NEGATIVE            9
#define DAM_HOLY                10
#define DAM_ENERGY              11
#define DAM_MENTAL              12
#define DAM_DISEASE             13
#define DAM_DROWNING            14
#define DAM_LIGHT		15
#define DAM_OTHER               16
#define DAM_HARM		17
#define DAM_CHARM		18
#define DAM_SOUND		19
#define DAM_SILVER              20
#define DAM_WOOD                21
#define DAM_IRON                22
#define DAM_WIND                23

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK         (A)
#define OFF_BACKSTAB            (B)
#define OFF_BASH                (C)
#define OFF_BERSERK             (D)
#define OFF_DISARM              (E)
#define OFF_DODGE               (F)
#define OFF_FADE                (G)
#define OFF_FAST                (H)
#define OFF_KICK                (I)
#define OFF_KICK_DIRT           (J)
#define OFF_PARRY               (K)
#define OFF_RESCUE              (L)
#define OFF_TAIL                (M)
#define OFF_TRIP                (N)
#define OFF_CRUSH		(O)
#define ASSIST_ALL       	(P)
#define ASSIST_ALIGN	        (Q)
#define ASSIST_RACE    	     	(R)
#define ASSIST_PLAYERS      	(S)
#define ASSIST_GUARD        	(T)
#define ASSIST_VNUM		(U)

/* return values for check_imm */
#define IS_NORMAL		0
#define IS_IMMUNE		1
#define IS_RESISTANT		2
#define IS_VULNERABLE		3

/* IMM bits for mobs */
#define IMM_SUMMON              (A)
#define IMM_PUSH                (A)
#define IMM_CHARM               (B)
#define IMM_MAGIC               (C)
#define IMM_WEAPON              (D)
#define IMM_BASH                (E)
#define IMM_PIERCE              (F)
#define IMM_SLASH               (G)
#define IMM_FIRE                (H)
#define IMM_COLD                (I)
#define IMM_LIGHTNING           (J)
#define IMM_ACID                (K)
#define IMM_POISON              (L)
#define IMM_NEGATIVE            (M)
#define IMM_HOLY                (N)
#define IMM_ENERGY              (O)
#define IMM_MENTAL              (P)
#define IMM_DISEASE             (Q)
#define IMM_DROWNING            (R)
#define IMM_LAGSKILLS		(S)
#define IMM_SOUND		(T)
#define IMM_LIGHT		(U)
#define IMM_WOOD                (X)
#define IMM_SILVER              (Y)
#define IMM_IRON                (Z)
#define IMM_GATE                (aa)
#define IMM_PORTAL              (bb)
#define IMM_WIND                (cc)
 
/* RES bits for mobs */
#define RES_SUMMON		(A)
#define RES_PUSH                (A)
#define RES_CHARM		(B)
#define RES_MAGIC               (C)
#define RES_WEAPON              (D)
#define RES_BASH                (E)
#define RES_PIERCE              (F)
#define RES_SLASH               (G)
#define RES_FIRE                (H)
#define RES_COLD                (I)
#define RES_LIGHTNING           (J)
#define RES_ACID                (K)
#define RES_POISON              (L)
#define RES_NEGATIVE            (M)
#define RES_HOLY                (N)
#define RES_ENERGY              (O)
#define RES_MENTAL              (P)
#define RES_DISEASE             (Q)
#define RES_DROWNING            (R)
#define RES_LAGSKILLS		(S) /* HAS NO USE - PLACEHOLDER */
#define RES_SOUND		(T)
#define RES_LIGHT		(U)
#define RES_WOOD                (X)
#define RES_SILVER              (Y)
#define RES_IRON                (Z)
#define RES_GATE                (aa)
#define RES_PORTAL              (bb) 
#define RES_WIND                (cc)

/* VULN bits for mobs */
#define VULN_SUMMON		(A)
#define VULN_PUSH               (A)
#define VULN_CHARM		(B)
#define VULN_MAGIC              (C)
#define VULN_WEAPON             (D)
#define VULN_BASH               (E)
#define VULN_PIERCE             (F)
#define VULN_SLASH              (G)
#define VULN_FIRE               (H)
#define VULN_COLD               (I)
#define VULN_LIGHTNING          (J)
#define VULN_ACID               (K)
#define VULN_POISON             (L)
#define VULN_NEGATIVE           (M)
#define VULN_HOLY               (N)
#define VULN_ENERGY             (O)
#define VULN_MENTAL             (P)
#define VULN_DISEASE            (Q)
#define VULN_DROWNING           (R)
#define VULN_LAGSKILLS		(S) /* HAS NO USE - PLACEHOLDER */
#define VULN_SOUND		(T)
#define VULN_LIGHT		(U)
#define VULN_WOOD               (X)
#define VULN_SILVER             (Y)
#define VULN_IRON		(Z)
#define VULN_GATE               (aa)
#define VULN_PORTAL             (bb)
#define VULN_WIND               (cc)
 

/* body form */
#define FORM_EDIBLE             (A)
#define FORM_POISON             (B)
#define FORM_MAGICAL            (C)
#define FORM_INSTANT_DECAY      (D)
#define FORM_OTHER              (E)  /* defined by material bit */
 
/* actual form */
#define FORM_ANIMAL             (G)
#define FORM_SENTIENT           (H)
#define FORM_UNDEAD             (I)
#define FORM_CONSTRUCT          (J)
#define FORM_MIST               (K)
#define FORM_INTANGIBLE         (L)
 
#define FORM_BIPED              (M)
#define FORM_CENTAUR            (N)
#define FORM_INSECT             (O)
#define FORM_SPIDER             (P)
#define FORM_CRUSTACEAN         (Q)
#define FORM_WORM               (R)
#define FORM_BLOB		(S)
 
#define FORM_MAMMAL             (V)
#define FORM_BIRD               (W)
#define FORM_REPTILE            (X)
#define FORM_SNAKE              (Y)
#define FORM_DRAGON             (Z)
#define FORM_AMPHIBIAN          (aa)
#define FORM_FISH               (bb)
#define FORM_COLD_BLOOD		(cc)	
#define FORM_VAMPIRE            (dd)
 
/* body parts */
#define PART_HEAD               (A)
#define PART_ARMS               (B)
#define PART_LEGS               (C)
#define PART_HEART              (D)
#define PART_BRAINS             (E)
#define PART_GUTS               (F)
#define PART_HANDS              (G)
#define PART_FEET               (H)
#define PART_FINGERS            (I)
#define PART_EAR                (J)
#define PART_EYE		(K)
#define PART_LONG_TONGUE        (L)
#define PART_EYESTALKS          (M)
#define PART_TENTACLES          (N)
#define PART_FINS               (O)
#define PART_WINGS              (P)
#define PART_TAIL               (Q)
/* for combat */
#define PART_CLAWS              (U)
#define PART_FANGS              (V)
#define PART_HORNS              (W)
#define PART_SCALES             (X)
#define PART_TUSKS		(Y)


/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND               (A)
#define AFF_INVISIBLE           (B)
#define AFF_BLESS               (C)
#define AFF_DETECT_INVIS        (D)
#define AFF_DETECT_MAGIC        (E)
#define AFF_DETECT_HIDDEN       (F)
#define AFF_SHIFT_SICK          (G)
#define AFF_SANCTUARY           (H)
#define AFF_FAERIE_FIRE         (I)
#define AFF_INFRARED            (J)
#define AFF_CURSE               (K)
#define AFF_NIGHTHASTE          (L)
#define AFF_POISON              (M)
#define AFF_SHIFT_ON            (N)
#define AFF_MAGICWARD           (O)
#define AFF_SNEAK               (P)
#define AFF_HIDE                (Q)
#define AFF_SLEEP               (R)
#define AFF_CHARM               (S)
#define AFF_FLYING              (T)
#define AFF_PASS_DOOR           (U)
#define AFF_HASTE               (V)
#define AFF_CALM                (W)
#define AFF_PLAGUE              (X)
#define AFF_WEAKEN              (Y)
#define AFF_DARK_VISION         (Z)
#define AFF_BERSERK             (aa)
#define AFF_SWIM                (bb)
#define AFF_REGENERATION        (cc)
#define AFF_SLOW                (dd)
#define AFF_SHIFT_PERM          (ee)

/*** AFFECTS 2 VARIABLES ***/
#define AFF_GHOST               (A)
#define AFF_MIRROR              (B)
#define AFF_INSPIRE             (C)
#define AFF_HSENSES             (D)
#define AFF2_SUMMONEDPET        (E)
#define AFF_HEADACHE            (F)
#define AFF2_NOREGEN            (G)
#define AFF_WILDAURA            (H)
#define AFF_SUMANIMAL           (I)
#define AFF_FURY                (J)
#define AFF_FEAR                (K)
#define AFF_WEB                 (L)
#define AFF_SHADOWS             (M)
#define AFF_BBOIL               (N)
#define AFF_CONFUSE             (O)
#define AFF_ABSORB              (P) 
#define AFF2_FORGET             (Q)
#define AFF_WITHER              (R)
#define AFF_WANTED              (S)
#define AFF_FOCUS               (T)
#define AFF_SILENCE             (U)
#define AFF_GUILDSPELL          (V)
#define AFF_CHIMESTRY           (W)
#define AFF_SUMSICK             (X)
#define AFF_CELERITY            (Y)
#define AFF_TERROR              (Z)
#define AFF_DETECTWERE         (aa)
#define AFF_CHIMESTRY2         (bb)
#define AFF_REFLECT            (cc)
#define AFF2_BLINDSIGHT        (dd)
#define AFF2_BURNING           (ee)

/*** AFFECT 3 VARIABLES */
#define AFF3_SBOND              (A)
#define AFF_ORDAIN              (B)
#define AFF3_SOLEMN             (C)
#define AFF_SKULLS              (D)
#define AFF3_NARCOLEPSY         (F)
#define AFF_DIRT                (G)
#define AFF_VEIL                (H)
#define AFF_VEIL_WAIT           (I)
#define AFF_TORPOR              (J)
#define AFF_PINCH               (K)
#define AFF_WERESUMSICK         (L)
#define AFF_OFFERING            (M)
#define AFF_FROG                (N)
#define AFF3_ABJURE             (O) /* for specific white grimoire-related RP */
#define AFF3_SUPERCAST          (P) /* ignore saves check on all spells */
#define AFF3_FAMINE             (Q) /* book of famine */
#define AFF3_AURA_PAIN          (R)
#define AFF3_PROFANE_WORD       (S)
#define AFF3_GODSPEED       (T)
#define AFF3_FATIGUE       (U)

/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL		      0
#define SEX_MALE		      1
#define SEX_FEMALE		      2

/* AC types */
#define AC_PIERCE			0
#define AC_BASH				1
#define AC_SLASH			2
#define AC_EXOTIC			3

/* dice */
#define DICE_NUMBER			0
#define DICE_TYPE			1
#define DICE_BONUS			2

/* size */
#define SIZE_TINY			0
#define SIZE_SMALL			1
#define SIZE_MEDIUM			2
#define SIZE_LARGE			3
#define SIZE_HUGE			4
#define SIZE_GIANT			5



/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_SILVER_ONE	      1
#define OBJ_VNUM_GOLD_ONE	      2
#define OBJ_VNUM_GOLD_SOME	      3
#define OBJ_VNUM_SILVER_SOME	      4
#define OBJ_VNUM_COINS		      5

#define OBJ_VNUM_SOULGEM_BOT      21981
#define OBJ_VNUM_SOULGEM_TOP      22000

#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_CORPSE_PC	     11
#define OBJ_VNUM_SEVERED_HEAD	     12
#define OBJ_VNUM_TORN_HEART	     13
#define OBJ_VNUM_SLICED_ARM	     14
#define OBJ_VNUM_SLICED_LEG	     15
#define OBJ_VNUM_GUTS		     16
#define OBJ_VNUM_BRAINS		     17
#define OBJ_VNUM_SCALP               19

#define OBJ_VNUM_MUSHROOM	     20
#define OBJ_VNUM_LIGHT_BALL	     21
#define OBJ_VNUM_SPRING		     22
#define OBJ_VNUM_DISC		     23
#define OBJ_VNUM_PORTAL		     25

#define OBJ_VNUM_ROSE		   1001

#define OBJ_VNUM_PIT		   3010

#define OBJ_VNUM_SCHOOL_MACE	   3700
#define OBJ_VNUM_SCHOOL_DAGGER	   3701
#define OBJ_VNUM_SCHOOL_SWORD	   3702
#define OBJ_VNUM_SCHOOL_SPEAR	   3717
#define OBJ_VNUM_SCHOOL_STAFF	   3718
#define OBJ_VNUM_SCHOOL_AXE	   3719
#define OBJ_VNUM_SCHOOL_FLAIL	   3720
#define OBJ_VNUM_SCHOOL_WHIP	   3721
#define OBJ_VNUM_SCHOOL_POLEARM    3722
#define OBJ_VNUM_SCHOOL_KNUCKLES   3723

#define OBJ_VNUM_SCHOOL_VEST	   3703
#define OBJ_VNUM_SCHOOL_SHIELD	   3704
#define OBJ_VNUM_SCHOOL_BANNER     3716
#define OBJ_VNUM_MAP		   3162

#define OBJ_VNUM_WHISTLE	   2116

#define OBJ_VNUM_SLANCE 30000

 
/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT		      1
#define ITEM_SCROLL		      2
#define ITEM_WAND		      3
#define ITEM_STAFF		      4
#define ITEM_WEAPON		      5
#define ITEM_TREASURE		      8
#define ITEM_ARMOR		      9
#define ITEM_POTION		     10
#define ITEM_CLOTHING		     11
#define ITEM_FURNITURE		     12
#define ITEM_TRASH		     13
#define ITEM_CONTAINER		     15
#define ITEM_DRINK_CON		     17
#define ITEM_KEY		     18
#define ITEM_FOOD		     19
#define ITEM_MONEY		     20
#define ITEM_BOAT		     22
#define ITEM_CORPSE_NPC		     23
#define ITEM_CORPSE_PC		     24
#define ITEM_FOUNTAIN		     25
#define ITEM_PILL		     26
#define ITEM_PROTECT		     27
#define ITEM_MAP		     28
#define ITEM_PORTAL		     29
#define ITEM_WARP_STONE		     30
#define ITEM_ROOM_KEY		     31
#define ITEM_GEM		     32
#define ITEM_JEWELRY		     33
#define ITEM_JUKEBOX		     34
#define ITEM_ARROW		     35
#define ITEM_QUIVER                  36
#define ITEM_SEED                    37
#define ITEM_TREE                    38
#define ITEM_BOOK                    39
#define ITEM_PLANT                   40

/* Arrow flags, for ranger Archery */
#define	ARROW_BURNING 		(A)
#define ARROW_FREEZING		(B)
#define ARROW_KNOCKOUT		(C)
#define	ARROW_SLEEP		(D)
#define ARROW_POISON		(E)
#define ARROW_SHOCKING		(F)
#define ARROW_PIERCE		(G)
#define ARROW_LOVE              (H)
/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		(A)
#define ITEM_HUM		(B)
#define ITEM_DARK		(C)
#define ITEM_LOCK		(D)
#define ITEM_EVIL		(E)
#define ITEM_INVIS		(F)
#define ITEM_MAGIC		(G)
#define ITEM_NODROP		(H)
#define ITEM_BLESS		(I)
#define ITEM_CREATED     	(J) /* TRADE SKILLS */
#define ITEM_ANTI_EVIL		(K)
#define ITEM_ANTI_NEUTRAL	(L)
#define ITEM_NOREMOVE		(M)
#define ITEM_INVENTORY		(N)
#define ITEM_NOPURGE		(O)
#define ITEM_ROT_DEATH		(P)
#define ITEM_VIS_DEATH		(Q)
#define ITEM_NONMETAL		(S)
#define ITEM_NOLOCATE		(T)
#define ITEM_MELT_DROP		(U)
#define ITEM_HAD_TIMER		(V)
#define ITEM_SELL_EXTRACT	(W)
#define ITEM_BURN_PROOF		(Y)
#define ITEM_NOUNCURSE		(Z)
#define ITEM_NO_IDENTIFY        (aa)
#define ITEM_KO_ROT             (bb)
#define ITEM_NOTSEEN		(cc)
#define ITEM_NOLOOT             (dd)
#define ITEM_GUILD_ROT          (ee)

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		(A)
#define ITEM_WEAR_FINGER	(B)
#define ITEM_WEAR_NECK		(C)
#define ITEM_WEAR_BODY		(D)
#define ITEM_WEAR_HEAD		(E)
#define ITEM_WEAR_LEGS		(F)
#define ITEM_WEAR_FEET		(G)
#define ITEM_WEAR_HANDS		(H)
#define ITEM_WEAR_ARMS		(I)
#define ITEM_WEAR_SHIELD	(J)
#define ITEM_WEAR_ABOUT		(K)
#define ITEM_WEAR_WAIST		(L)
#define ITEM_WEAR_WRIST		(M)
#define ITEM_WIELD		(N)
#define ITEM_HOLD		(O)
#define ITEM_NO_SAC		(P)
#define ITEM_WEAR_FLOAT		(Q)
#define ITEM_WEAR_PRIDE         (R)

/* weapon class */
#define WEAPON_EXOTIC		0
#define WEAPON_SWORD		1
#define WEAPON_DAGGER		2
#define WEAPON_SPEAR		3
#define WEAPON_MACE		4
#define WEAPON_AXE		5
#define WEAPON_FLAIL		6
#define WEAPON_WHIP		7	
#define WEAPON_POLEARM		8
#define WEAPON_STAFF            9
#define WEAPON_HAND            10 
#define	WEAPON_BOW	       11 /* archery */

/* weapon types */
#define WEAPON_FLAMING		(A)
#define WEAPON_FROST		(B)
#define WEAPON_VAMPIRIC		(C)
#define WEAPON_SHARP		(D)
#define WEAPON_VORPAL		(E)
#define WEAPON_TWO_HANDS	(F)
#define WEAPON_SHOCKING		(G)
#define WEAPON_POISON		(H)
#define WEAPON_STICKY           (I)
#define WEAPON_ANGELIC          (J)

/* gate flags */
#define GATE_NORMAL_EXIT	(A)
#define GATE_NOCURSE		(B)
#define GATE_GOWITH		(C)
#define GATE_BUGGY		(D)
#define GATE_RANDOM		(E)
#define GATE_FATAL              (F)
#define GATE_DEAD_END           (G)
#define GATE_DRAINER            (H)

/* furniture flags */
#define STAND_AT		(A)
#define STAND_ON		(B)
#define STAND_IN		(C)
#define SIT_AT			(D)
#define SIT_ON			(E)
#define SIT_IN			(F)
#define REST_AT			(G)
#define REST_ON			(H)
#define REST_IN			(I)
#define SLEEP_AT		(J)
#define SLEEP_ON		(K)
#define SLEEP_IN		(L)
#define PUT_AT			(M)
#define PUT_ON			(N)
#define PUT_IN			(O)
#define PUT_INSIDE		(P)




/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE		      0
#define APPLY_STR		      1
#define APPLY_DEX		      2
#define APPLY_INT		      3
#define APPLY_WIS		      4
#define APPLY_CON		      5
#define APPLY_SEX		      6
#define APPLY_CLASS		      7
#define APPLY_LEVEL		      8
#define APPLY_AGE		      9
#define APPLY_HEIGHT		     10
#define APPLY_WEIGHT		     11
#define APPLY_MANA		     12
#define APPLY_HIT		     13
#define APPLY_MOVE		     14
#define APPLY_GOLD		     15
#define APPLY_EXP		     16
#define APPLY_AC		     17
#define APPLY_HITROLL		     18
#define APPLY_DAMROLL		     19
#define APPLY_SAVES		     20
#define APPLY_SAVING_PARA	     20
#define APPLY_SAVING_ROD	     21
#define APPLY_SAVING_PETRI	     22
#define APPLY_SAVING_BREATH	     23
#define APPLY_SAVING_SPELL	     24
#define APPLY_SPELL_AFFECT	     25
#define APPLY_SKILLS                 26
#define APPLY_BLOOD                  27

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		     (A)
#define CONT_PICKPROOF		     (B)
#define CONT_CLOSED		     (C)
#define CONT_LOCKED		     (D)
#define CONT_PUT_ON		     (E)
#define CONT_EASY                    (F)
#define CONT_HARD                    (G)
#define CONT_INFURIATING             (H)

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_CHAT		   1200
#define ROOM_VNUM_TEMPLE	   3001
#define ROOM_VNUM_ALTAR		   3054
#define ROOM_VNUM_SCHOOL	   3700
#define ROOM_VNUM_BALANCE	   4500
#define ROOM_VNUM_CIRCLE	   4400
#define ROOM_VNUM_DEMISE	   4201
#define ROOM_VNUM_HONOR		   4300



/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		(A)
#define ROOM_BANK               (B)
#define ROOM_NO_MOB		(C)
#define ROOM_INDOORS		(D)
#define ROOM_PRIVATE		(J)
#define ROOM_SAFE		(K)
#define ROOM_SOLITARY		(L)
#define ROOM_PET_SHOP		(M)
#define ROOM_NO_RECALL		(N)
#define ROOM_IMP_ONLY		(O)
#define ROOM_GODS_ONLY		(P)
#define ROOM_HEROES_ONLY	(Q)
#define ROOM_NEWBIES_ONLY	(R)
#define ROOM_LAW		(S)
#define ROOM_NOWHERE		(T)
#define ROOM_NO_GATE            (U)
#define ROOM_NO_SUMMON		(V)
#define ROOM_NO_PORTAL          (W)
#define ROOM_RED_SANDS          (X)
#define ROOM_SEMISAFE           (Y)

/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH		      0
#define DIR_EAST		      1
#define DIR_SOUTH		      2
#define DIR_WEST		      3
#define DIR_UP			      4
#define DIR_DOWN		      5
#define DIR_NONE                      6

/*
 * Room and Transportation Interaction
 * Used in #ROOMS
 */

#define TRANS_NO_PORTAL          (A)
#define TRANS_NO_PORTAL_IN       (B)
#define TRANS_NO_PORTAL_OUT      (C)
#define TRANS_NO_GATE            (D)
#define TRANS_NO_GATE_IN         (E)
#define TRANS_NO_GATE_OUT        (F)
#define TRANS_NO_TELEPORT        (G)
#define TRANS_NO_TELEPORT_IN     (H)
#define TRANS_NO_TELEPORT_OUT    (I)
#define TRANS_NO_SUMMON          (J)
#define TRANS_NO_SUMMON_IN       (K)
#define TRANS_NO_SUMMON_OUT      (L)
#define TRANS_NO_SPELLS          (M)
#define TRANS_NO_SPELLS_IN       (N)
#define TRANS_NO_SPELLS_OUT      (O)
#define TRANS_NO_RECALL          (P)
#define TRANS_NO_BAD_GATES       (Q)

/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		      (A)
#define EX_CLOSED		      (B)
#define EX_LOCKED		      (C)
#define EX_PICKPROOF		      (F)
#define EX_NOPASS		      (G)
#define EX_EASY			      (H)
#define EX_HARD			      (I)
#define EX_INFURIATING		      (J)
#define EX_NOCLOSE		      (K)
#define EX_NOLOCK		      (L)
#define EX_SECRET                     (M)



/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE		      0
#define SECT_CITY		      1
#define SECT_FIELD		      2
#define SECT_FOREST		      3
#define SECT_HILLS		      4
#define SECT_MOUNTAIN		      5
#define SECT_WATER_SWIM		      6
#define SECT_WATER_NOSWIM	      7
#define SECT_CAVE  		      8
#define SECT_AIR		      9
#define SECT_DESERT		     10
#define SECT_SNOW  		     11
#define SECT_LAVA  		     12
#define SECT_MAX		     13

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		     -1
#define WEAR_LIGHT		      0
#define WEAR_FINGER_L		      1
#define WEAR_FINGER_R		      2
#define WEAR_NECK_1		      3
#define WEAR_NECK_2		      4
#define WEAR_BODY		      5
#define WEAR_HEAD		      6
#define WEAR_LEGS		      7
#define WEAR_FEET		      8
#define WEAR_HANDS		      9
#define WEAR_ARMS		     10
#define WEAR_SHIELD		     11
#define WEAR_ABOUT		     12
#define WEAR_WAIST		     13
#define WEAR_WRIST_L		     14
#define WEAR_WRIST_R		     15
#define WEAR_WIELD		     16
#define WEAR_HOLD		     17
#define WEAR_FLOAT		     18
#define WEAR_PRIDE                   19
#define MAX_WEAR		     20



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK		      0
#define COND_FULL		      1
#define COND_THIRST		      2
#define COND_HUNGER		      3



/*
 * Positions.
 */
#define POS_DEAD		      0
#define POS_UNCONSCIOUS		      1
#define POS_SLEEPING		      3
#define POS_SPRAWLED		      4
#define POS_RESTING		      5
#define POS_SITTING		      6
#define POS_FLATFOOTED                6 /* if flat-footed, cannot be bashed */
#define POS_FIGHTING		      7 /* pos > flatfooted means bashable */
#define POS_STANDING		      8



/*
 * ACT bits for players.
 */
#define PLR_IS_NPC		(A)		/* Don't EVER set.	*/
#define PLR_PUEBLO		(B)		/* Pueblo Code */
#define PLR_AUTOASSIST		(C)
#define PLR_AUTOEXIT		(D)
#define PLR_AUTOLOOT		(E)
#define PLR_AUTOSAC             (F)
#define PLR_AUTOGOLD		(G)
#define PLR_AUTOSPLIT		(H)
#define PLR_BATTLEBRIEF         (I)
#define PLR_SUSPEND_LEVEL       (J)

#define PLR_BRAWLER             (L)
#define PLR_AUTO_AFK            (M)
#define PLR_HOLYLIGHT		(N)

#define PLR_NOTUSED		(P)  /* Used to be DRMODE; some players may
                                        have enabled. */
#define PLR_NOSUMMON		(Q)
#define PLR_NOFOLLOW		(R)

#define PLR_COLOUR		(T)    /* Colour Flag By Lope */
#define PLR_PERMIT		(U)

#define PLR_LOG			(W)
#define PLR_DENY		(X)
#define PLR_FREEZE		(Y)
#define PLR_NEWBIE		(Z)
#define PLR_KILLER		(aa)
#define PLR_QUEST               (bb)
#define PLR_LEADER              (cc)
#define PLR_LINKDEAD            (dd)
#define ACT_NOAPPROVE		(ee)
#define PLR_NOAPPROVE           (ee) /* Due to compatibility issues
                                        and bad past coding */

/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET              (A)
#define COMM_DEAF            	(B)
#define COMM_NOWIZ              (C)
#define COMM_NOAUCTION          (D)
#define COMM_NOGOSSIP           (E)
#define COMM_NOADMIN            (F)
#define COMM_NOMUSIC            (G)
#define COMM_NOGUILD		(H)
#define COMM_NOQUOTE		(I)
#define COMM_SHOUTSOFF		(J)
#define COMM_NOMINION		(K)


/* display flags */
#define COMM_COMPACT		(L)
#define COMM_BRIEF		(M)
#define COMM_PROMPT		(N)
#define COMM_COMBINE		(O)
#define COMM_TELNET_GA		(P)
#define COMM_SHOW_AFFECTS	(Q)
#define COMM_NOGRATS		(R)
#define COMM_NOSING		(S)
/* penalties */
#define COMM_NOEMOTE		(T)
#define COMM_NOSHOUT		(U)
#define COMM_NOTELL		(V)
#define COMM_NOCHANNELS		(W)

#define COMM_DRMODE             (X)
#define COMM_SNOOP_PROOF	(Y)
#define COMM_AFK		(Z)
#define COMM_NONEWBIE           (aa)
#define COMM_NOCLAN             (bb) 
#define COMM_VAMP               (cc)
#define COMM_NOBRAWLER          (dd)
#define COMM_VERBOSE            (ee)

/* WIZnet flags */
#define WIZ_ON			(A)
#define WIZ_TICKS		(B)
#define WIZ_LOGINS		(C)
#define WIZ_SITES		(D)
#define WIZ_LINKS		(E)
#define WIZ_DEATHS		(F)
#define WIZ_RESETS		(G)
#define WIZ_MOBDEATHS		(H)
#define WIZ_FLAGS		(I)
#define WIZ_PENALTIES		(J)
#define WIZ_SACCING		(K)
#define WIZ_LEVELS		(L)
#define WIZ_SECURE		(M)
#define WIZ_SWITCHES		(N)
#define WIZ_SNOOPS		(O)
#define WIZ_RESTORE		(P)
#define WIZ_LOAD		(Q)
#define WIZ_NEWBIE		(R)
#define WIZ_PREFIX		(S)
#define WIZ_MOBALERTS           (T)
#define WIZ_MULTI               (U)
#define WIZ_APPROVAL		(V)
#define WIZ_QUEST               (W)
#define WIZ_NOTES               (X)
#define WIZ_LOGS                (Y)
#define WIZ_BANS                (Z)
#define WIZ_COMMERCE		(aa)
#define WIZ_BUGS                (bb)

#define LIMIT_COVENANCE     (A)
#define LIMIT_CODER         (B)
#define LIMIT_IMP           (C)
#define LIMIT_BUILDER       (D)
#define LIMIT_GYPSY         (E)
#define LIMIT_INFERNO       (F)
#define LIMIT_VERMILLION    (G)
#define LIMIT_DAWNING       (H)
#define LIMIT_CRIMSON       (I)
#define LIMIT_ARCAENUM      (J)
#define LIMIT_LOSER         (K)
#define LIMIT_AEQUITAS      (L)
#define LIMIT_VAMPIRES      (M)
#define LIMIT_WERECREATURES (N)
#define LIMIT_BRAWLERS      (O)
#define LIMIT_IMMORTALS     (P)
#define LIMIT_PATRIARCHS    (Q)
#define LIMIT_RAVNOS        (R)
#define LIMIT_LASOMBRA      (S)
#define LIMIT_FLOCRIAN      (T)
#define LIMIT_SERAPHS      (U)
#define LIMIT_COALITION     (V)


/* For wiz_commerce, the maximum amount of GOLD that can be
   transferred before the wiznet alert goes off. */
#define MAX_MONEY_TRANSFER	4999

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
    MOB_INDEX_DATA *	next;
    SPEC_FUN *		spec_fun;
    SHOP_DATA *		pShop;
    MPROG_LIST *        mprogs;
    AREA_DATA *		area;
    RESET_DATA *        reset_first;
    RESET_DATA *        reset_last;
    sh_int		vnum;
    sh_int		group;
    bool		new_format;
    sh_int		count;
    sh_int		killed;
    char *		player_name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    char *		image;
    long		act;
    long		affected_by;
    long		affected_by2;
    long		affected_by3;
    sh_int		alignment;
    sh_int		level;
    sh_int		hitroll;
    sh_int		hit[3];
    sh_int		mana[3];
    sh_int		damage[3];
    sh_int		ac[4];
    sh_int 		dam_type;
    long		off_flags;
    long		imm_flags;
    long		res_flags;
    long		vuln_flags;
    sh_int		start_pos;
    sh_int		default_pos;
    sh_int		sex;
    sh_int		race;
    long		wealth;
    long		form;
    long		parts;
    sh_int		size;
    char *		material;
    long		mprog_flags;
};



/* memory settings */
#define MEM_CUSTOMER	A	
#define MEM_SELLER	B
#define MEM_HOSTILE	C
#define MEM_AFRAID	D

/* memory for mobs */
struct mem_data
{
    MEM_DATA 	*next;
    bool	valid;
    int		id; 	
    int 	reaction;
    time_t 	when;
};

/*
 * One character (PC or NPC).
 */
struct	char_data
{
    CHAR_DATA *		next;
    CHAR_DATA *		next_in_room;
    CHAR_DATA *		master;
    CHAR_DATA *		leader;
    CHAR_DATA *		fighting;
    CHAR_DATA *		reply;
	CHAR_DATA * 		immreply;
	CHAR_DATA * 		oreply;
    CHAR_DATA *		pet;
    CHAR_DATA *		mprog_target;
    MEM_DATA *		memory;
    SPEC_FUN *		spec_fun;
    MOB_INDEX_DATA *	pIndexData;
    DESCRIPTOR_DATA *	desc;
    AFFECT_DATA *	affected;
    NOTE_DATA *		pnote;
    OBJ_DATA *		carrying;
    OBJ_DATA *		on;
    ROOM_INDEX_DATA *	in_room;
    ROOM_INDEX_DATA *	was_in_room; /* This is used to keep track of voided 
                                        characters and in greet and greet-style
                                        mprogs */
    PC_DATA *		pcdata;
    GEN_DATA *		gen_data;
    MPRECOG_DATA *      mprecog;
    bool		valid;
    bool                purge_on_areset;
    char *		name;
    long		id;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    char *		prompt;
    char *		prefix;
    char *		image;
    char *		shiftimage1;
    char *		shiftimage2;
    long		shiftbits;
    char *		shiftdesc1;
    char *		shiftdesc2;
    char *		patriarch;             /* vamps */
	char *       cln_name; /*coalition*/
	char *       cln_leader;
	char *       cln_symbol;
	char *       cln_apply;
    char *              title_guild;
    char *		write_buffer;          /* for string exwrite */
    char *              charmed_by;            /* for anti-marker code */
    sh_int		blood;             
    sh_int              max_blood;         
    sh_int              blood_percentage;  
	sh_int        max_life_energy;
	sh_int        life_energy;
	sh_int        enochian;
	sh_int        availability;
	/*jousting shit*/
	sh_int      joust_ready;
	char *		joust_attack;
	sh_int  	jouster1;
	sh_int  	jouster2;
	sh_int 		joust_attack_num;
	sh_int		joust_win;
	sh_int		in_joust;
	/*end jousting shit*/
    sh_int		group;
    sh_int		guild;
    sh_int		sex;
    sh_int		class;
    sh_int		race;
    sh_int		level;
    sh_int		trust;
    int			played;
    int			lines;              /* for the pager */
    time_t		logon;
    sh_int		timer;
    sh_int		wait;               /* lag from skills/spells */
    sh_int		daze;               /* daze state...not really used */
    sh_int              wstimer;            /* lag for weapon special v4s */
    sh_int              kicktimer;          /* timer for mobs being kicked */
/*  sh_int		fightlag; */
    sh_int		hit;
    sh_int		max_hit;
    sh_int		mana;
    sh_int		max_mana;
    sh_int		move;
    sh_int		max_move;
    long		gold;
    long		silver;
    int			exp;
    long		act_bits;
    long		comm;
    long		wiznet_flags;
    long		imm_flags;
    long		res_flags;
    long		vuln_flags;
    sh_int		invis_level;
    sh_int		incog_level;
    sh_int              rinvis_level; 
    long		affected_by;
    long		affected_by2;
    long		affected_by3;
    sh_int		position;
    sh_int		practice;
    sh_int		train;
    sh_int		carry_weight;
    sh_int		carry_number;
    sh_int		saving_throw;
    sh_int		alignment;
    sh_int		hitroll;
    sh_int		damroll;
    sh_int		armor[4];
    sh_int		wimpy;
    /* stats */
    sh_int		perm_stat[MAX_STATS];
    sh_int		mod_stat[MAX_STATS];
    /* parts stuff */
    long		form;
    long		parts;
    sh_int		size;
    char*		material;
    /* mobile stuff */
    long		off_flags;
    sh_int		damagedice[3];
    sh_int		dam_type;
    sh_int		start_pos;
    sh_int		default_pos;
    sh_int		mprog_delay;
    int   		prand;	/* for OLC */

    char 		colors[MAX_COLOR];
    sh_int		charmies;


};

/*
 * Data which only PC's have.
 */
struct	pc_data
{
    PC_DATA *   next;
    BUFFER *    buffer;
    sh_int      version;
    bool        	aprilfools;
    bool        	valid;
    char *      pwd;
    char *      bamfin;
    char *      bamfout;
    char *      transin;
    char *      transout;
    char *      title;
    char *      pretitle;
    char *      notetags;
    char *      famshort;
    char *      famlong;
    char *      famname;
    char *      famdesc;
    sh_int      famsex;
    time_t      lastlogoff;
    time_t      lastpk;             /* fightlag for 'if inpk' mprog */
    time_t      lastfight;
    time_t      last_note;
    time_t      last_idea;
    time_t      last_history;
    time_t      last_news;
    time_t      last_changes;
	 time_t     last_gossip;
    time_t      last_trouble;
    sh_int      perm_hit;
    sh_int      perm_mana;
    sh_int      perm_move;
    sh_int      true_sex;
    int         	last_level;
    sh_int      condition [4];
    sh_int      learned  [MAX_SKILL];
    bool        	group_known [MAX_GROUP];
    sh_int      points;
    sh_int      god;
    bool        	confirm_delete;
    long        	gold_bank;
    long        	silver_bank;
    int         	security;
    int         	volume;
    int       	  	questscore;
    bool    	    latelog;
	/* Jousting Stuff */
	

    /* Immortal Key Performance Indicators 
       These probably belong in another structure altogether to
       preserve memory.  Oh well. */
    int         wizitime;
    int         vistime;
    int         approvals;
    int         unapprovals;
    int         guildnotes;
    int         gdts;
    int         immtalks;
    int         ics;
    int         oocs;
    int         questcount; 
  
    /* Silly stats...  Maybe one day I'll put these in. 
    int         portals;   <-- number of portals cast
    int         gates;     <-- number of gates cast
    int         mob_kills; <-- number of mobs killed
    int         mob_deaths;<-- number of times killed by mobs
    int         pc_kills;  <-- number of people you knocked out
    int         pc_deaths; <-- times knocked out by others
    int         ics;
    int         oocs;      <-- number of OOCs used
    int         says;      <-- number of says used
    int         blinded;   <-- times blinded
    int         cursed;    <-- times cursed
    int         refreshes; <-- cast on others
    */
    int                   proceeds;
  
    /* Trade skills */
    int         tailoring; /* Tailoring level */
    int         baking; /* Baking level */
    int         fletching; /* FLETCHING */
    char*       fake_ip;
    char*       last_ip;
    int         guild_time;
    int         guild_logs;
    time_t      guild_date;
    int         push_count;
    long        quest_team_score;
    char*       quest_team_captain;
    time_t      restore_channels;
    time_t      restore_emotes;
    time_t      restore_shouts;
    time_t      restore_tells;
    int         cleansings;
    CHAR_DATA * offering;
    int         cleanse_timer;
    time_t      last_cleanse;
    bool        verifytrain;
    time_t      skillbonus;

    BRAWLER_DATA *      brawler;
    BRAWLER_DATA *      brawling;

  Clan* pcClan;
  Clan* pcClanApply;
};

/* Data for generating characters -- only used during generation */
struct gen_data
{
    GEN_DATA	*next;
    bool	valid;
    bool	skill_chosen[MAX_SKILL];
    bool	group_chosen[MAX_GROUP];
    int		points_chosen;
};




/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	/* Next in list                     */
    bool valid;
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct	obj_index_data
{
    OBJ_INDEX_DATA *	next;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    AREA_DATA *		area;		/* OLC */
    bool		new_format;
    char *		name;
    char *		short_descr;
    char *		description;
    char *		owner;
    sh_int		vnum;
    sh_int		reset_num;
    char *		material;
    sh_int		item_type;
    int			extra_flags;
    int			wear_flags;
    sh_int		level;
    sh_int 		condition;
    sh_int		count;
    sh_int		weight;
    int			cost;
    int			value[5];
    int	                timedown;
};



/*
 * One object.
 */
struct	obj_data
{
    OBJ_DATA *		next;
    OBJ_DATA *		next_content;
    OBJ_DATA *		contains;
    OBJ_DATA *		in_obj;
    OBJ_DATA *		on;
    CHAR_DATA *		carried_by;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
    bool		valid;
    bool		enchanted;
    char *	        owner;
    char *		name;
    char *		short_descr;
    char *		description;
    char *              donor;
    sh_int		item_type;
    int			extra_flags;
    int			wear_flags;
    sh_int		wear_loc;
    sh_int		weight;
    int			cost;
    sh_int		level;
    sh_int 		condition;
    char *		material;
    sh_int		timer;
    int			value	[5];
};

/*
 * Disabled data.
 */
struct  disabled_data
{
   DISABLED_DATA *next;
   struct cmd_type const *command;
   char *disabled_by;
   sh_int level;
};

extern                  DISABLED_DATA    *           disabled_first;

/*
 * Exit data.
 */
struct	exit_data
{
    union
    {
	ROOM_INDEX_DATA *	to_room;
	sh_int			vnum;
    } u1;
    sh_int		exit_info;
    sh_int		key;
    char *		keyword;
    char *		description;
    EXIT_DATA *		next;		/* OLC */
    int			rs_flags;	/* OLC */
    int			orig_door;	/* OLC */
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
    RESET_DATA *	next;
    char		command;
    sh_int		arg1;
    sh_int		arg2;
    sh_int		arg3;
    sh_int		arg4;
};



/*
 * Area definition.
 */
struct	area_data
{
    AREA_DATA *		next;
    HELP_AREA *		helps;
    QUEST_DATA *        quests;
    char *		file_name;
    char *		name;
    char *		credits;
    char *              notes;
    sh_int		age;
    sh_int		nplayer;
    sh_int		low_range;
    sh_int		high_range;
    sh_int 		min_vnum;
    sh_int		max_vnum;
    bool		empty;
    char *		builders;	/* OLC */ /* Listing of */
    int			vnum;		/* OLC */ /* Area vnum  */
    int			area_flags;	/* OLC */
    int			security;	/* OLC */ /* Value 1-9  */
    bool		linked;
};



/*
 * Room type.
 */
struct	room_index_data
{
    ROOM_INDEX_DATA *	next;
    ROOM_INDEX_DATA *   master_next;
    ROOM_INDEX_DATA *   master_prev;
    CHAR_DATA *		people;
    OBJ_DATA *		contents;
    EXTRA_DESCR_DATA *	extra_descr;
    AREA_DATA *		area;
    EXIT_DATA *		exit	[6];
    RESET_DATA *	reset_first;
    RESET_DATA *	reset_last;
    char *		name;
    char *		description;
    char *		owner;
    char *		p_image;
    sh_int		vnum;
    int			room_flags;
    int                 trans_flags;
    sh_int		light;
    sh_int		sector_type;
    sh_int		heal_rate;
    sh_int 		mana_rate;
    sh_int		guild;
};

/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000

/*
 *  Target types.
 */
#define TAR_IGNORE		    0
#define TAR_CHAR_OFFENSIVE	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_SELF		    3
#define TAR_OBJ_INV		    4
#define TAR_OBJ_CHAR_DEF	    5
#define TAR_OBJ_CHAR_OFF	    6

#define TARGET_CHAR		    0
#define TARGET_OBJ		    1
#define TARGET_ROOM		    2
#define TARGET_NONE		    3



/*
 * MOBprog definitions
 */                   
#define TRIG_ACT	(A)
#define TRIG_BRIBE	(B)
#define TRIG_DEATH	(C)
#define TRIG_ENTRY	(D)
#define TRIG_FIGHT	(E)
#define TRIG_GIVE	(F)
#define TRIG_GREET	(G)
#define TRIG_GRALL	(H)
#define TRIG_KILL	(I)
#define TRIG_HPCNT	(J)
#define TRIG_RANDOM	(K)
#define TRIG_SPEECH	(L)
#define TRIG_EXIT	(M)
#define TRIG_EXALL	(N)
#define TRIG_DELAY	(O)
#define TRIG_SURR	(P)
#define TRIG_SPAWN	(Q)
#define TRIG_PPM        (R)
#define TRIG_SELSE      (S)
#define TRIG_GELSE      (T)
struct mprog_list
{
    int			trig_type;
    char *		trig_phrase;
    sh_int		vnum;
    char *  		code;
    MPROG_LIST * 	next;
    bool		valid;
};

struct mprog_code
{
    sh_int		vnum;
    char *		code;
    MPROG_CODE *	next;
    int                 assigned;
    int                 called;         /* only used/updated by do_mplistmob */
};

/*
 * These are skill_lookup return values for common skills and spells.
 */
extern sh_int   gsn_abjuration;
extern sh_int   gsn_absorb;
extern sh_int   gsn_acid_blast;
extern sh_int   gsn_animate_dead;
extern sh_int   gsn_armorpierce;
extern sh_int   gsn_armthrow;
extern sh_int   gsn_atrophy;
extern sh_int   gsn_aura_pain;
extern sh_int   gsn_axe;
extern sh_int   gsn_backstab;
extern sh_int   gsn_bash;
extern sh_int   gsn_battle;
extern sh_int   gsn_bboil;
extern sh_int   gsn_blindness;
extern sh_int   gsn_blister;
extern sh_int   gsn_bolas;
extern sh_int   gsn_bow;
extern sh_int   gsn_brew;
extern sh_int   gsn_brew;
extern sh_int   gsn_burning_palm;
extern sh_int   gsn_cancellation;
extern sh_int   gsn_call_spiders;
extern sh_int   gsn_call_water_spirit;
extern sh_int   gsn_chaosdemon;
extern sh_int   gsn_charge;
extern sh_int   gsn_charm_person;
extern sh_int   gsn_cheese;
extern sh_int   gsn_cleanse;
extern sh_int   gsn_conjure_bats;
extern sh_int   gsn_conjure_familiar;
extern sh_int   gsn_contrition;
extern sh_int   gsn_crane_kick;
extern sh_int   gsn_critical_strike;
extern sh_int   gsn_cure_blindness;
extern sh_int   gsn_cure_critical;
extern sh_int   gsn_cure_disease;
extern sh_int   gsn_cure_light;
extern sh_int   gsn_cure_poison;
extern sh_int   gsn_cure_serious;
extern sh_int   gsn_curse;
extern sh_int   gsn_dagger;
extern sh_int   gsn_dark_grimoire;
extern sh_int   gsn_deathsong;
extern sh_int   gsn_vorpalspell;
extern sh_int   gsn_decay;
extern sh_int   gsn_devotion;
extern sh_int   gsn_dirt;
extern sh_int   gsn_disarm;
extern sh_int   gsn_dislodge;
extern sh_int   gsn_dispel_magic;
extern sh_int   gsn_divine_focus;
extern sh_int   gsn_dodge;
extern sh_int   gsn_domination;
extern sh_int   gsn_dragon_punch;
extern sh_int   gsn_dualwield;
extern sh_int   gsn_encomium;
extern sh_int   gsn_energy_drain;
extern sh_int   gsn_enhanced_damage;
extern sh_int   gsn_entangle;
extern sh_int   gsn_entrance;
extern sh_int   gsn_exhaustion;
extern sh_int   gsn_familiar_link;
extern sh_int   gsn_fast_healing;
extern sh_int   gsn_fatigue;
extern sh_int   gsn_feed;
extern sh_int   gsn_fifth_attack;
extern sh_int   gsn_fire;
extern sh_int   gsn_fireball;
extern sh_int   gsn_flail;
extern sh_int   gsn_flamestrike;
extern sh_int   gsn_fourth_attack;
extern sh_int   gsn_forget;
extern sh_int   gsn_fury;
extern sh_int   gsn_haggle;
extern sh_int   gsn_hamstring;
extern sh_int   gsn_hand_to_hand;
extern sh_int   gsn_harmtouch;
extern sh_int   gsn_haunt;
extern sh_int   gsn_headache;
extern sh_int   gsn_heal;
extern sh_int   gsn_heightened_senses;
extern sh_int   gsn_hide;
extern sh_int   gsn_immolation;
extern sh_int   gsn_inspire;
extern sh_int   gsn_invis;
extern sh_int   gsn_kick;
extern sh_int   gsn_lay_on_hands;
extern sh_int   gsn_lightning_bolt;
extern sh_int   gsn_lifetap;
extern sh_int   gsn_lore;
extern sh_int   gsn_love;
extern sh_int   gsn_mace;
extern sh_int   gsn_major_heal;
extern sh_int   gsn_mass_invis;
extern sh_int   gsn_maul;
extern sh_int   gsn_meditation;
extern sh_int   gsn_mind_blast;
extern sh_int   gsn_mind_whip;
extern sh_int   gsn_mirror;
extern sh_int   gsn_natures_pestil;
extern sh_int   gsn_natures_wrath;
extern sh_int   gsn_palm_block;
extern sh_int   gsn_parry;
extern sh_int   gsn_peaches;
extern sh_int   gsn_peek;
extern sh_int   gsn_pick_lock;
extern sh_int   gsn_pinch;
extern sh_int   gsn_pestilence;
extern sh_int   gsn_plague;
extern sh_int   gsn_poison;
extern sh_int   gsn_polearm;
extern sh_int   gsn_pounce;
extern sh_int   gsn_profane_word;
extern sh_int   gsn_psy_drain;
extern sh_int   gsn_purify;
extern sh_int   gsn_rage;
extern sh_int   gsn_ravage;
extern sh_int   gsn_recall;
extern sh_int   gsn_reflect;
extern sh_int   gsn_refresh;
extern sh_int   gsn_remove_curse;
extern sh_int   gsn_rescue;
extern sh_int   gsn_restore_limb;
extern sh_int   gsn_resurrect;
extern sh_int   gsn_sanctuary;
extern sh_int   gsn_sanguination;
extern sh_int   gsn_sanguine_bond;
extern sh_int   gsn_sap;
extern sh_int   gsn_scrolls;
extern sh_int   gsn_second_attack;
extern sh_int   gsn_shadow;
extern sh_int   gsn_shadow_form;
extern sh_int   gsn_shadows;
extern sh_int   gsn_shield_block;
extern sh_int   gsn_skulls;
extern sh_int   gsn_sleep;
extern sh_int   gsn_slow;
extern sh_int   gsn_smite;
extern sh_int   gsn_sneak;
extern sh_int   gsn_solemn_vow;
extern sh_int   gsn_soothe;
extern sh_int   gsn_spear;
extern sh_int   gsn_staff;
extern sh_int   gsn_staves;
extern sh_int   gsn_steal;
extern sh_int   gsn_sumanimal;
extern sh_int   gsn_summon_meerkats;
extern sh_int   gsn_summon_skeletal_army;
extern sh_int   gsn_sumsick;
extern sh_int   gsn_sun_skeletals;
extern sh_int   gsn_supermob;
extern sh_int   gsn_sword;
extern sh_int   gsn_terror;
extern sh_int   gsn_third_attack;
extern sh_int   gsn_throw;
extern sh_int   gsn_touch;
extern sh_int   gsn_trip;
extern sh_int   gsn_twrath;
extern sh_int   gsn_unholyfire;
extern sh_int   gsn_veil;
extern sh_int   gsn_vital_strike;
extern sh_int   gsn_wands;
extern sh_int   gsn_weaken;
extern sh_int   gsn_whip;
extern sh_int   gsn_whirlwind;
extern sh_int   gsn_white_grimoire;
extern sh_int   gsn_wildaura;
extern sh_int   gsn_wildenhance;
extern sh_int   gsn_wildfire;
extern sh_int   gsn_wilt;
extern sh_int   gsn_wing;
extern sh_int   gsn_wither_limb;
extern sh_int   gsn_wyrm_venom;

/*
 * Utility macros.
 */
#define IS_VALID(data)		((data) != NULL && (data)->valid)
#define VALIDATE(data)		((data)->valid = TRUE)
#define INVALIDATE(data)	((data)->valid = FALSE)
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define IS_NULLSTR(str)		((str) == NULL || (str)[0] == '\0')
#define ENTRE(min,num,max)	( ((min) < (num)) && ((num) < (max)) )
#define CHECK_POS(a, b, c)	{							\
					(a) = (b);					\
					if ( (a) < 0 )					\
					bug( "CHECK_POS : " c " == %d < 0", a );	\
				}							\

/*
 * Character macros.
 */
#define IS_NPC(ch)		(IS_SET((ch)->act_bits, ACT_IS_NPC))
#define IS_ADMIN(ch)		(get_trust(ch) >= LEVEL_ADMIN)
#define IS_IMMORTAL(ch)		(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust(ch) >= LEVEL_HERO)
#define IS_TRUSTED(ch,level)	(get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define IS_AFFECTED2(ch, sn)	(IS_SET((ch)->affected_by2, (sn)))
#define IS_AFFECTED3(ch, sn)	(IS_SET((ch)->affected_by3, (sn)))
#define IS_DRMODE(ch)           (IS_SET((ch)->comm, COMM_DRMODE))
#define IS_BRAWLER(ch)          (!IS_SET((ch)->act_bits, ACT_IS_NPC) \
                                 && IS_SET((ch)->act_bits, PLR_BRAWLER))
#define IS_WERECREATURE(ch)     (!IS_NPC((ch)) &&  (ch)->level > 29 && \
                                 IS_SET((ch)->shiftbits,SHIFT_POTENTIAL) \
                                 && (ch)->pcdata->cleansings < 3 )
#define IS_VAMPIRE(ch)          (!IS_NPC((ch)) \
                                 && ( IS_SET((ch)->shiftbits, PERM_VAMP) \
                                 || IS_SET((ch)->shiftbits, TEMP_VAMP) ) )
#define IS_IN_FIGHTLAG(ch)       (!IS_NPC((ch)) && \
                         (current_time - (ch)->pcdata->lastfight < FIGHT_LAG ) )

#define GET_AGE(ch)		((int) (17 + ((ch)->played \
				    + current_time - (ch)->logon )/72000))

#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define GET_AC(ch,type)		((ch)->armor[type]			    \
		        + ( IS_AWAKE(ch)			    \
			? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0 ))  
#define GET_HITROLL(ch)	\
		((ch)->hitroll+str_app[get_curr_stat(ch,STAT_STR)].tohit)
#define GET_DAMROLL(ch) \
		((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam)

#define IS_OUTSIDE(ch)		(!IS_SET( (ch)->in_room->room_flags, ROOM_INDOORS))

#define WAIT_STATE(ch, npulse)	((ch)->wait = UMAX((ch)->wait, (npulse)))
	#define DAZE_STATE(ch, npulse)  ((ch)->daze = UMAX((ch)->daze, (npulse)))
	#define get_carry_weight(ch)	((ch)->carry_weight + (ch)->silver/10 +  \
							      (ch)->gold * 2 / 5)
	#define HAS_TRIGGER(ch,trig)	(IS_SET((ch)->pIndexData->mprog_flags,(trig)))
	#define IS_SWITCHED( ch )       ( ch->desc && ch->desc->original )
	#define IS_BUILDER(ch, Area)	( !IS_NPC(ch) && !IS_SWITCHED( ch ) &&	  \
					( ch->pcdata->security >= Area->security  \
					|| strstr( Area->builders, ch->name )	  \
					|| strstr( Area->builders, "All" ) ) )


/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))
#define WEIGHT_MULT(obj)	((obj)->item_type == ITEM_CONTAINER ? \
	(obj)->value[4] : 100)


/* Archery */
#define IS_ARROW_STAT(obj,stat)(IS_SET((obj)->value[2],(stat)))


/*
 * Description macros.
 */

extern char* PERS( CHAR_DATA*, CHAR_DATA* );

#define PERS2(ch, looker)	( can_see( looker, (ch) ) ?		\
					( IS_NPC(ch) ? (ch)->short_descr	\
					 : (ch)->name ) : "someone" )

/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
    char *     name;
    char *    char_no_arg;
    char *    others_no_arg;
    char *    char_found;
    char *    others_found;
    char *    vict_found;
    char *    char_not_found;
    char *      char_auto;
    char *      others_auto;
};



/*
 * Global constants.
 */
extern	const	struct	str_app_type	str_app		[28];
extern	const	struct	int_app_type	int_app		[28];
extern	const	struct	wis_app_type	wis_app		[28];
extern	const	struct	dex_app_type	dex_app		[28];
extern	const	struct	con_app_type	con_app		[28];


/*
 * Global variables.
 */
extern		HELP_DATA       *help_first;
extern		SHOP_DATA       *shop_first;

extern		CHAR_DATA       *char_list;
extern		DESCRIPTOR_DATA *descriptor_list;
extern		OBJ_DATA        *object_list;

extern		MPROG_CODE      *mprog_list;
extern		MPDELAY_DATA    *mpdelay_list;

extern          BANKACCT_DATA   *global_bankacct_list;
extern          bool             global_bankacct_changed;

extern		char			bug_buf		[];
extern          char                    global_prompt_t [];
extern          char                    global_prompt_a [];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		KILL_DATA		kill_table	[];
extern		char			log_buf		[];
extern		TIME_INFO_DATA		time_info;
extern          int                     bank_interest;
extern          int                     bank_interest_total_silver;
extern          int                     bank_interest_total_gold;

extern		WEATHER_DATA		weather_info;
extern		bool			MOBtrigger;
extern		bool			player_quitting;
extern          int                     global_config_aprilfools;
extern          int                     global_config_lookspam;
extern          int                     global_config_vorpalon;
extern          int                     global_config_tick_jitter;
extern          int                     global_config_newpoison;
extern          int                     global_config_strongwyrmpoison;
extern          int                     global_quest_count[MAX_FLAG+1];

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(apollo)
int	atoi		args( ( const char *string ) );
void *	calloc		args( ( unsigned nelem, size_t size ) );
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(hpux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(linux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(macintosh)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(MIPS_OS)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(MSDOS)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(NeXT)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(sequent)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if     defined(__SVR4) && defined(__sun)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
#if     defined(__SVR4) || defined(__sun)
size_t  fread           args( ( void *ptr, size_t size, size_t n, FILE *stream) );
#else
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
#endif
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(ultrix)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(macintosh)
#define PLAYER_DIR	""			/* Player files	*/
#define TEMP_FILE	"romtmp"
#define NULL_FILE	"proto.are"		/* To reserve one stream */
#endif

#if defined(MSDOS)
#define PLAYER_DIR	""			/* Player files */
#define TEMP_FILE	"romtmp"
#define NULL_FILE	"nul"			/* To reserve one stream */
#endif

#if defined(unix)
#define PLAYER_DIR      "../player/"        	/* Player files */
#define TEMP_FILE	"../player/romtmp"
#define NULL_FILE	"/dev/null"		/* To reserve one stream */
#endif

#define AREA_LIST       "area.lst"              /* List of areas*/
#define BUG_FILE        "bugs.txt"              /* For 'bug' and bug()*/
#define TYPO_FILE       "typos.txt"             /* For 'typo'*/
#define NOTE_FILE       "notes.not"             /* For 'notes'*/
#define IDEA_FILE	"ideas.not"
#define HISTORY_FILE	"history.not"
#define NEWS_FILE	"news.not"
#define CHANGES_FILE	"chang.not"
#define TROUBLE_FILE    "trouble.not"
#define SHUTDOWN_FILE   "shutdown.txt"          /* For 'shutdown'*/
#define BAN_FILE	"ban.txt"
#define MUSIC_FILE	"music.txt"
#define MULTILOGIN_FILE "multilogin.txt"
#define DISABLED_FILE	"disabled.txt"
#define BADNAMES_FILE   "badnames.txt"
#define BANKACCT_FILE   "bankaccts.txt"

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define AD	AFFECT_DATA
#define MPC	MPROG_CODE

/* act_comm.c */
void  	check_sex	args( ( CHAR_DATA *ch) );
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
bool	stop_follower	args( ( CHAR_DATA *ch ) );
void 	nuke_pets	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );

/* act_enter.c */
RID  *get_random_room   args ( (CHAR_DATA *ch) );

/* act_info.c */
void	set_title	args( ( CHAR_DATA *ch, char *title ) );
extern int get_object_points( OBJ_INDEX_DATA* obj );
void    mod_who         args( ( void ) );
extern const char LASOMBRA_PAT[];
extern const char RAVNOS_PAT[];
extern const char TRAMUIR_PAT[];
extern const char FEROCAI_PAT[];
extern const char default_prompt[];

/* act_move.c */
void	move_char	args( ( CHAR_DATA *ch, int door, bool follow ) );
bool    can_fly		args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *to_room ) );
bool    can_swim	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *to_room ) );
bool    can_swim_lava	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *to_room ) );
int	move_cost 	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *to_room ) );


/* act_obj.c */
void	wear_obj	args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace) );
void    get_obj         args( ( CHAR_DATA *ch, OBJ_DATA *obj,
                            OBJ_DATA *container ) );

/* act_wiz.c */
void wiznet		args( (char *string, CHAR_DATA *ch, OBJ_DATA *obj,
			       long flag, long flag_skip, int min_level ) );
void check_multiplay    args( (CHAR_DATA *ch) );

/* ban.c */
bool	check_ban	args( ( char *site, int type) );
double  ban_sortkey     args( ( const char* ban_name ) );


/* multilogin.c */
bool	check_multilogin args(( char *site, int type));

/* comm.c */
void	show_string	args( ( struct descriptor_data *d, char *input) );
void	close_socket	args( ( DESCRIPTOR_DATA *dclose ) );
void	write_to_buffer	args( ( DESCRIPTOR_DATA *d, const char *txt,
			    int length ) );
void    send_to_desc    args( ( const char *txt, DESCRIPTOR_DATA *d ) );
void	send_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	page_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	act		args( ( const char *format, CHAR_DATA *ch,
			    const void *arg1, const void *arg2, int type ) );
void	act_new		args( ( const char *format, CHAR_DATA *ch, 
			    const void *arg1, const void *arg2, int type,
			    int min_pos, int min_level ) );
void printf_to_char( CHAR_DATA* ch, char* fmt, ...);

/*
 * Colour stuff by Lope of Loping Through The MUD
 */
int	colour		args( ( char type, CHAR_DATA *ch, char *string ) );
void	colourconv	args( ( char *buffer, const char *txt, CHAR_DATA *ch ) );
void	send_to_char_bw	args( ( const char *txt, CHAR_DATA *ch ) );
void	page_to_char_bw	args( ( const char *txt, CHAR_DATA *ch ) );

/* db.c */
void    fix_obj_values  args( ( OBJ_DATA *obj ) );
void    sort_journals   args( ( AREA_DATA *pArea ) );
void    sort_quest_db   args( ( void ) );
void	reset_area      args( ( AREA_DATA * pArea, bool forceobj ) );		/* OLC */
void	reset_room	args( ( ROOM_INDEX_DATA *pRoom ) );	/* OLC */
void    reset_room_new  args( ( ROOM_INDEX_DATA *pRoom, bool forceobj ) );
char *	print_flags	args( ( int flag ));
void	boot_db		args( ( void ) );
void	area_update	args( ( void ) );
CD *	create_mobile	args( ( MOB_INDEX_DATA *pMobIndex ) );
void	clone_mobile	args( ( CHAR_DATA *parent, CHAR_DATA *clone) );
OD *	create_object	args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void	clone_object	args( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
void	clear_char	args( ( CHAR_DATA *ch ) );
char *	get_extra_descr	args( ( CHAR_DATA *ch, const char *name, 
                                EXTRA_DESCR_DATA *ed, OBJ_DATA *obj ) );
MID *	get_mob_index	args( ( int vnum ) );
OID *	get_obj_index	args( ( int vnum ) );
RID *	get_room_index	args( ( int vnum ) );
MPC *	get_mprog_index args( ( int vnum ) );
char	fread_letter	args( ( FILE *fp ) );
int	fread_number	args( ( FILE *fp ) );
long 	fread_flag	args( ( FILE *fp ) );
char *	fread_string	args( ( FILE *fp ) );
char *  fread_string_eol args(( FILE *fp ) );
void	fread_to_eol	args( ( FILE *fp ) );
char *	fread_word	args( ( FILE *fp ) );
long	flag_convert	args( ( char letter) );
void *	alloc_mem	args( ( int sMem ) );
void *	alloc_perm	args( ( int sMem ) );
void	free_mem	args( ( void *pMem, int sMem ) );
char *	str_dup		args( ( const char *str ) );
void	free_string	args( ( char *pstr ) );
int	number_fuzzy	args( ( int number ) );
int	number_range	args( ( int from, int to ) );
int	number_percent	args( ( void ) );
int	number_door	args( ( void ) );
int	number_bits	args( ( int width ) );
long     number_mm       args( ( void ) );
int	dice		args( ( int number, int size ) );
int	interpolate	args( ( int level, int value_00, int value_32 ) );
void	smash_tilde	args( ( char *str ) );
bool	str_cmp		args( ( const char *astr, const char *bstr ) );
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
char *	capitalize	args( ( const char *str ) );
void	append_file	args( ( CHAR_DATA *ch, char *file, char *str ) );
void	bug		args( ( const char *str, int param ) );
void	log_string	args( ( const char *str ) );
void	tail_chain	args( ( void ) );
void    load_disabled   args( ( void ) );
void    save_disabled  args( ( void ) );


/* effect.c */
void	acid_effect	args( (void *vo, int level, int dam, int target) );
void	cold_effect	args( (void *vo, int level, int dam, int target) );
void	fire_effect	args( (void *vo, int level, int dam, int target) );
void	poison_effect	args( (void *vo, int level, int dam, int target) );
void	shock_effect	args( (void *vo, int level, int dam, int target) );


/* fight.c */
bool 	is_safe		args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_safe_verb	args( (CHAR_DATA *ch, CHAR_DATA *victim, bool verb) );
bool 	is_safe_spell	args( (CHAR_DATA *ch, CHAR_DATA *victim, bool area ) );
void	violence_update	args( ( void ) );
void	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
bool	damage		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			        int dt, int class, bool show ) );
bool    damage_old      args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
                                int dt, int class, bool show ) );
void    raw_kill        args( ( CHAR_DATA *victim ) );
void    unconscious_kill args( ( CHAR_DATA *victim ) );
bool 	raw_damage	args( ( CHAR_DATA* ch, CHAR_DATA* victim, int dam, 
                                int dt, int dam_type, bool show, bool lethal ) );
void	update_pos	args( ( CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
int     xp_compute      args( ( CHAR_DATA *gch, int victim_level, int total_levels, int members ) );
bool    in_fightlag     args( ( CHAR_DATA *ch ) );

/* handler.c */
#if     defined(__SVR4) && defined(__sun)
char    *strsep         args( ( char** str, const char* delims) );
#endif
AD  	*affect_find    args( (AFFECT_DATA *paf, int sn));
char    *strip_color	args( ( const char *str ) );
char    *escape_color	args( ( const char *str ) );
char    *smash_crlf     args( ( const char *str ) );
void	affect_check	args( (CHAR_DATA *ch, int where, int vector) );
int	count_users	args( (OBJ_DATA *obj) );
void 	deduct_cost	args( (CHAR_DATA *ch, int cost) );
void	affect_enchant	args( (OBJ_DATA *obj) );
int 	check_immune	args( (CHAR_DATA *ch, int dam_type) );
int 	material_lookup args( ( const char *name) );
int     god_lookup      args( ( const char *name) );
int	weapon_lookup	args( ( const char *name) );
int	weapon_type	args( ( const char *name) );
char 	*weapon_name	args( ( int weapon_Type) );
char	*item_name	args( ( int item_type) ); 
int	attack_lookup	args( ( const char *name) );
long	wiznet_lookup	args( ( const char *name) );
int	class_lookup	args( ( const char *name) );
int     dam_to_imm      args( (int dam_type) );
bool	is_guild		args( (CHAR_DATA *ch) );
bool	is_same_guild	args( (CHAR_DATA *ch, CHAR_DATA *victim));
bool	is_old_mob	args ( (CHAR_DATA *ch) );
int	get_skill	args( ( CHAR_DATA *ch, int sn ) );
int	get_weapon_sn	args( ( CHAR_DATA *ch ) );
int	get_weapon_skill args(( CHAR_DATA *ch, int sn ) );
int     get_age         args( ( CHAR_DATA *ch ) );
void	reset_char	args( ( CHAR_DATA *ch )  );
int	get_trust	args( ( CHAR_DATA *ch ) );
int	get_curr_stat	args( ( CHAR_DATA *ch, int stat ) );
int 	get_max_train	args( ( CHAR_DATA *ch, int stat ) );
int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
bool    is_abbrev       args( ( const char* str, const char* namelist ) );
bool	is_name		args( ( char *str, char *namelist ) );
bool	is_exact_name	args( ( char *str, char *namelist ) );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_to_char2	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_to_char3	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_to_obj	args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove_obj args( (OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_strip	args( ( CHAR_DATA *ch, int sn ) );
bool	is_affected	args( ( CHAR_DATA *ch, int sn ) );
void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool merge ) );
void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_to_char_init args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
int	apply_ac	args( ( OBJ_DATA *obj, int iWear, int type ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
void	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
CD *	get_char_room	args( ( CHAR_DATA *ch, char *argument, bool abbrev ) );
CD *	get_char_room_seriously	args( ( CHAR_DATA *ch, char *argument, bool abbrev ) );
CD *	get_char_world	args( ( CHAR_DATA *ch, char *argument, bool abbrev ) );
OD *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData, ROOM_INDEX_DATA *pRoom ) );
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list, bool abbrev ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument, 
			    CHAR_DATA *viewer, bool abbrev ) );
OD *	get_obj_wear	args( ( CHAR_DATA *ch, char *argument, bool abbrev ) );
OD *	get_obj_here	args( ( CHAR_DATA *ch, char *argument, bool abbrev ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument, bool abbrev ) );
OD *	create_money	args( ( int gold, int silver ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
int	get_true_weight	args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	is_room_owner	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room) );
bool	room_is_private	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_see_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *	affect_loc_name	args( ( int location ) );
char *	extra_bit_name	args( ( int extra_flags ) );
char *	act_bit_name	args( ( int act_flags ) );
char *	off_bit_name	args( ( int off_flags ) );
char *  imm_bit_name	args( ( int imm_flags ) );
char * 	form_bit_name	args( ( int form_flags ) );
char *	part_bit_name	args( ( int part_flags ) );
char *	weapon_bit_name	args( ( int weapon_flags ) );
char *  shift_bit_name	args( ( int shift_flags ) );
char *	cont_bit_name	args( ( int cont_flags) );
char *  listize         args( ( char *argument, bool xand ) );

/* interp.c */
void	interpret	args( ( CHAR_DATA *ch, char *argument ) );
bool	is_number	args( ( char *arg ) );
int	number_argument	args( ( char *argument, char *arg ) );
int	mult_argument	args( ( char *argument, char *arg) );
char *	one_argument	args( ( char *argument, char *arg_first ) );
char *	one_sentence	args( ( char *argument, char *arg_first ) );
char *	one_word    	args( ( char *argument, char *arg_first ) );

/* magic.c */
int	find_spell	args( ( CHAR_DATA *ch, const char *name) );
int 	mana_cost 	(CHAR_DATA *ch, int min_mana, int level);
bool 	has_summonedpets	args( ( CHAR_DATA *ch ) );
int	skill_lookup	args( ( const char *name ) );
int	slot_lookup	args( ( int slot ) );
bool	saves_spell	args( ( int level, CHAR_DATA *ch, CHAR_DATA *victim, int dam_type ) );
void    spell_identify  args( ( int sn, int level, CHAR_DATA *ch,
                                   void *vo, int target )  );
void	obj_cast_spell	args( ( int sn, int level, CHAR_DATA *ch,
				    CHAR_DATA *victim, OBJ_DATA *obj ) );

/* mob_prog.c */
void	program_flow	args( ( sh_int vnum, char *source, CHAR_DATA *mob, CHAR_DATA *ch,
				const void *arg1, const void *arg2 ) );
void	mp_act_trigger	args( ( char *argument, CHAR_DATA *mob, CHAR_DATA *ch,
				const void *arg1, const void *arg2, int type ) );
bool	mp_percent_trigger args( ( CHAR_DATA *mob, CHAR_DATA *ch, 				
				const void *arg1, const void *arg2, int type ) );
bool    mp_ppm_trigger    args( ( CHAR_DATA *mob, CHAR_DATA *ch,
                                const void *arg1, const void *arg2, int type ) );
void	mp_bribe_trigger  args( ( CHAR_DATA *mob, CHAR_DATA *ch, int amount ) );
bool	mp_exit_trigger   args( ( CHAR_DATA *ch, int dir ) );
void	mp_give_trigger   args( ( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj ) );
void 	mp_greet_trigger  args( ( CHAR_DATA *ch ) );
void	mp_hprct_trigger  args( ( CHAR_DATA *mob, CHAR_DATA *ch ) );
void	mp_spawn_trigger  args( (CHAR_DATA *ch) );
bool    is_mob_room       args( ( CHAR_DATA *ch, char *argument, bool ch_pc ) );
bool    get_mob_vnum_room args( (CHAR_DATA *ch, sh_int vnum) );

/* mob_cmds.c */
void	mob_interpret	args( ( CHAR_DATA *ch, char *argument ) );

/* save.c */
void	save_char_obj	args( ( CHAR_DATA *ch ) );
bool	load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name ) );

/* skills.c */
bool 	parse_gen_groups args( ( CHAR_DATA *ch,char *argument ) );
void 	list_group_costs args( ( CHAR_DATA *ch ) );
void    list_group_known args( ( CHAR_DATA *ch ) );
int 	exp_per_level	args( ( CHAR_DATA *ch, int points ) );
void 	check_improve	args( ( CHAR_DATA *ch, int sn, bool success, 
				    int multiplier ) );
int 	group_lookup	args( (const char *name) );
void	gn_add		args( ( CHAR_DATA *ch, int gn) );
void 	gn_remove	args( ( CHAR_DATA *ch, int gn) );
void 	group_add	args( ( CHAR_DATA *ch, const char *name, bool deduct) );
void	group_remove	args( ( CHAR_DATA *ch, const char *name) );

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );
char *	spec_name	args( ( SPEC_FUN *function ) );

/* teleport.c */
RID *	room_by_name	args( ( char *target, int level, bool error) );

/* update.c */
void	advance_level	args( ( CHAR_DATA *ch, bool hide ) );
void	gain_exp	args( ( CHAR_DATA *ch, int gain ) );
void	gain_condition	args( ( CHAR_DATA *ch, int iCond, int value ) );
void	update_handler	args( ( void ) );

/* string.c */
void	string_edit	args( ( CHAR_DATA *ch, char **pString ) );
void    string_append   args( ( CHAR_DATA *ch, char **pString ) );
char *	string_replace	args( ( char * orig, char * old, char * new ) );
void    string_add      args( ( CHAR_DATA *ch, char *argument ) );
char *  format_string   args( ( char *oldstring /*, bool fSpace */ ) );
void    format_string_tmp args( ( char *oldstring, char *newstring ) );
char *  first_arg       args( ( char *argument, char *arg_first, bool fCase ) );
char *	string_unpad	args( ( char * argument ) );
char *	string_proper	args( ( char * argument ) );

/* olc.c */
bool	run_olc_editor	args( ( DESCRIPTOR_DATA *d ) );
char	*olc_ed_name	args( ( CHAR_DATA *ch ) );
char	*olc_ed_vnum	args( ( CHAR_DATA *ch ) );

/* lookup.c */
int	race_lookup      args( ( const char *name) );
int	item_lookup      args( ( const char *name) );
int	liq_lookup	 args( ( const char *name) );
int     wandstaff_lookup args( ( const char *name ) );

/* vampire.c */
void	vampire_enhance 	args( ( CHAR_DATA *ch ) );
void	vampire_unenhance	args( ( CHAR_DATA *ch ) );
void    vampire_unshift		args( ( CHAR_DATA *ch ) );

/*were.c*/
void   were_day                 args(  ( CHAR_DATA *ch) );
void   were_chance              args(  ( CHAR_DATA *ch) );
void   shift_strip              args(  ( CHAR_DATA *ch) );
void   shift_strip2             args(  ( CHAR_DATA *ch) );
void   were_night               args(  ( CHAR_DATA *ch) );

/* quest.c */

int get_quest_cmd_index args( ( const char* name ) );
void quest_bit_name( int flags, char* buffer );
void check_quest_score( CHAR_DATA* ch, CHAR_DATA* victim, int damage, int damtype );
void quest_join args( ( CHAR_DATA* ch, char* argument ) );
void quest_leave args( ( CHAR_DATA* ch, char* argument ) );
void quest_status args( ( CHAR_DATA* ch, char* argument ) );
void quest_score args( ( CHAR_DATA* ch, char* argument ) );
void quest_teamscore args( ( CHAR_DATA* ch, char* argument ) );
void quest_start args( ( CHAR_DATA* ch, char* argument ) );
void quest_open args( ( CHAR_DATA* ch, char* argument ) );
void quest_close args( ( CHAR_DATA* ch, char* argument ) );
void quest_end args( ( CHAR_DATA* ch, char* argument ) );
void quest_set args( ( CHAR_DATA* ch, char* argument ) );
void quest_zero args( ( CHAR_DATA* ch, char* argument ) );
void quest_award args( ( CHAR_DATA* ch, char* argument ) );
void quest_assign args( ( CHAR_DATA* ch, char* argument ) );
void quest_vnum args( ( CHAR_DATA* ch, char* argument ) );
void quest_levels args( ( CHAR_DATA* ch, char* argument ) );
void quest_boot args( ( CHAR_DATA* ch, char* argument ) );
void quest_name args( ( CHAR_DATA *ch, char *argument ) );

/* bank.c */
void acct_to_char               args( ( BANKACCT_DATA *acct, CHAR_DATA *ch ) );
BANKACCT_DATA *modify_acct      args( ( char *owner, long gold, long silver ) );
void load_bankaccts             args( ( void ) );
void save_bankaccts             args( ( void ) );

#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef AD

/*****************************************************************************
 *                                    OLC                                    *
 *****************************************************************************/

/*
 * Object defined in limbo.are
 * Used in save.c to load objects that don't exist.
 */
#define OBJ_VNUM_DUMMY	30

/*
 * Area flags.
 */
#define         AREA_NONE       0
#define         AREA_CHANGED    1	/* Area has been modified. */
#define         AREA_ADDED      2	/* Area has been added to. */
#define         AREA_LOADING    4	/* Used for counting in db.c */

#define MAX_DIR	6
#define NO_FLAG -99	/* Must not be used in flags or stats. */

/*
 * Global Constants
 */
extern	char *	const	dir_name        [];
extern	const	sh_int	rev_dir         [];          /* sh_int - ROM OLC */
extern	const	struct	spec_type	spec_table	[];

/*
 * Global variables
 */
extern		AREA_DATA *		area_first;
extern		AREA_DATA *		area_last;
extern		SHOP_DATA *		shop_last;

extern		int			top_affect;
extern		int			top_area;
extern		int			top_ed;
extern		int			top_exit;
extern		int			top_help;
extern		int			top_mob_index;
extern		int			top_obj_index;
extern		int			top_reset;
extern		int			top_room;
extern		int			top_shop;

extern		int			top_vnum_mob;
extern		int			top_vnum_obj;
extern		int			top_vnum_room;
extern          int                     area_version;

extern		char			str_empty       [1];

extern		MOB_INDEX_DATA *	mob_index_hash  [MAX_KEY_HASH];
extern		OBJ_INDEX_DATA *	obj_index_hash  [MAX_KEY_HASH];
extern		ROOM_INDEX_DATA *	room_index_hash [MAX_KEY_HASH];

/* 
 * Werecreature and vampire bits 
 */
#define SHIFT_POTENTIAL         (A)
#define SHIFT_HALF              (B)
#define SHIFT_FULL              (C)
#define PERM_VAMP               (D)
#define TEMP_VAMP               (E)
#define MIST_VAMP               (F)
#define BAT_VAMP                (G)
#define SHIFT_NOFORCE           (H)
#define SHIFT_FIXEDBP           (I)

/* 
 * for default dice structure 
 */
#define MAX_BUILD_LEVEL 60
typedef struct default_build_data {
    sh_int level;
    sh_int hpnum;
    sh_int hpdice;
    sh_int hpbonus;
    sh_int dmnum;
    sh_int dmdice;
    sh_int dmbonus;
    sh_int acpier; 
    sh_int acbash;
    sh_int acslash;
    sh_int acex;
    sh_int hitroll;
} DEFAULT_BUILD_DATA;   

extern DEFAULT_BUILD_DATA   olc_mob_default[MAX_BUILD_LEVEL];

typedef struct quest_info
{
  char name[MAX_INPUT_LENGTH];
  sh_int min_level;
  sh_int max_level;
  sh_int questmob_vnum;
} QUEST_INFO;

#define QUEST_INPROGRESS A
#define QUEST_OPEN B
#define QUEST_ALWAYS_KO C
#define QUEST_NO_LOOT D
#define QUEST_NO_MURDER E
#define QUEST_NO_DEATHROT F
#define QUEST_NO_SCORING G
#define QUEST_NO_MAGIC_SCORE H
#define QUEST_NO_MELEE_SCORE I
#define QUEST_NO_HEALING_SCORE J
#define QUEST_PC_DAMAGE_ONLY K
#define QUEST_MOB_DAMAGE_ONLY L
#define QUEST_NO_QUESTROT M

ROOM_INDEX_DATA* master_room_list;

const static int MIN_SKILL_PCT = 50;

#endif /* __MERC_H */

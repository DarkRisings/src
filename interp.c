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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

bool	check_social	args( ( CHAR_DATA *ch, char *command,
			    char *argument ) );

bool    check_disabled (const struct cmd_type *command);

void Check_Replay args( (CHAR_DATA *ch ) );


/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2


DISABLED_DATA *disabled_first;

#define END_MARKER      "END"

/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;



/*
 * Command table.
 */
const	struct	cmd_type	cmd_table	[] =
{
    /*
     * Common movement commands.
     */
    { "north",		do_north,	POS_STANDING,    0,  LOG_NEVER, 0 },
    { "east",		do_east,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "south",		do_south,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "west",		do_west,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "up",		do_up,		POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "down",		do_down,	POS_STANDING,	 0,  LOG_NEVER, 0 },

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
 /* { "backup",         do_backup,      POS_DEAD,       L1,  LOG_ALWAYS, 1 },*/
    { "at",             do_at,          POS_DEAD,       L6,  LOG_NORMAL, 1 },
	{ "available",           do_availability,        POS_DEAD,       L8,  LOG_NORMAL, 1 },
    { "cast",		do_cast,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "auction",        do_aucchannel,  POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "buy",		do_buy,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "channels",       do_channels,    POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "exits",		do_exits,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "get",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "goto",           do_goto,        POS_DEAD,       L8,  LOG_NORMAL, 1 },
    { "group",          do_group,       POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "guild",		do_guild,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "hit",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "inventory",	do_inventory,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "kill",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "look",		do_look,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "glance",		do_glance,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "loot",		do_loot,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "gdt",		do_guildtalk,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "minion",		do_minion,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
	{ "ct",		do_clantalk,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "qsay",           do_qsay,        POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "qtsay",          quest_teamsay,  POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "nb",		do_nb,	        POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
	 { "joust",		do_jousting,	        POS_SLEEPING,	 0,  LOG_NORMAL, 1 },

    { "chat",           do_nb,          POS_SLEEPING,    0,  LOG_NORMAL, 0 },
    { "music",          do_nb,          POS_SLEEPING,    0,  LOG_NORMAL, 0 },
    { "order",		do_order,	POS_SPRAWLED,	 0,  LOG_NORMAL, 1 },
    { "ooc",            do_music,   	POS_SLEEPING,    0,  LOG_NORMAL, 1 }, 
    { "oot",            do_otell,       POS_SLEEPING,    0,  LOG_NORMAL, 0 },
    { "otell",          do_otell,       POS_SLEEPING,    0,  LOG_NORMAL, 1 },
	{ "oreply",          do_oreply,       POS_SLEEPING,    0,  LOG_NORMAL, 1 },
	 { "imreply",          do_immreply,       POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "practice",       do_practice,	POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "learn",       	do_learn,	POS_SLEEPING,   30,  LOG_NORMAL, 1 },
    { "quest",		do_quest,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "rest",		do_rest,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "rank",           do_rank,        POS_DEAD,       L4,  LOG_ALWAYS, 1 },
    { "sit",		do_sit,		POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "sockets",        do_sockets,	POS_DEAD,       L3,  LOG_NORMAL, 1 },
    { "stand",		do_stand,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "tell",		do_tell,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "unlock",         do_unlock,      POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "wield",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "wizhelp",	do_wizhelp,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "wizstat",        do_wizstat,     POS_DEAD,       L2,  LOG_NORMAL, 1 },
    { "approve",	do_approve,	POS_DEAD,	IM,  LOG_ALWAYS, 1 },

    /*
     * Informational commands.
     */
    { "affects",	do_affects,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "areas",		do_areas,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "bloodpool",	do_bloodpool,	POS_RESTING,	30,  LOG_NORMAL, 1 },
    { "bug",		do_bug,		POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "changes",	do_changes,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "commands",	do_commands,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "compare",	do_compare,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "consider",	do_consider,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "count",		do_count,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "credits",	do_credits,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "date",           do_time,        POS_DEAD,        0,  LOG_NORMAL, 0 },
    { "equipment",	do_equipment,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "examine",	do_examine,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
/*  { "groups",		do_groups,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 }, */
    { "help",		do_help,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "idea",		do_idea,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "info",           do_info,        POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "motd",		do_motd,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "news",		do_news,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "read",		do_read,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "report",		do_report,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "rules",		do_rules,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "score",		do_score,	POS_DEAD,	 0,  LOG_NORMAL, 1 },

    { "sc",             do_score,       POS_DEAD,        0,  LOG_NORMAL, 0 },
    { "scan",           do_scan,        POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "skills",		do_skills,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "socials",	do_socials,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "show",		do_show,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "showhp",         do_showhp,      POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "spells",		do_spells,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "story",		do_story,	POS_DEAD,	 0,  LOG_NORMAL, 0 },
    { "time",		do_time,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "typo",		do_typo,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "weather",	do_weather,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "who",		do_who,		POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "whois",		do_whois,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "wizlist",	do_wizlist,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "worth",		do_worth,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "lore",           do_history,     POS_DEAD,        0,  LOG_NORMAL, 0 },

    /*
     * Configuration commands.
     */
    { "autolist",	do_autolist,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "autoassist",	do_autoassist,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autoexit",	do_autoexit,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autogold",	do_autogold,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autoloot",	do_autoloot,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autosac",	do_autosac,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autosplit",	do_autosplit,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "brief",		do_brief,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "battlebrief",    do_battlebrief, POS_DEAD,        0,  LOG_NORMAL, 1 },
/*  { "channels",	do_channels,	POS_DEAD,	 0,  LOG_NORMAL, 1 }, */
    { "color",          do_colour,      POS_DEAD,        0,  LOG_NORMAL, 0 },
    { "colour",         do_colour,      POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "combine",	do_combine,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "compact",	do_compact,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "description",	do_description,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "descfull",	do_descfull,	POS_DEAD,	 30, LOG_NORMAL, 1 },
    { "deschalf",	do_deschalf,	POS_DEAD,	 30, LOG_NORMAL, 1 },
    { "delet",		do_delet,	POS_DEAD,	 0,  LOG_NORMAL, 0 },
    { "delete",		do_delete,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "famdesc",        do_famdesc,     POS_DEAD,        40, LOG_NORMAL, 0 },
    { "descfam",        do_famdesc,     POS_DEAD,        40, LOG_NORMAL, 0 },
    { "famgender",      do_famsex,      POS_DEAD,        40, LOG_NORMAL, 0 },
    { "suicid",         do_delet,       POS_DEAD,        0,  LOG_NORMAL, 0 },
    { "suicide",        do_delete,      POS_STANDING,    0,  LOG_NORMAL, 0 },
    { "togglenewbie",   do_newbie,	POS_DEAD,        0,  LOG_NORMAL, 0 },
    { "nofollow",	do_nofollow,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "nosummon",	do_nosummon,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "outfit",		do_outfit,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "password",	do_password,	POS_DEAD,	 0,  LOG_NEVER,  1 },
    { "prompt",		do_prompt,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "scroll",		do_scroll,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "title",		do_title,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "wimpy",		do_wimpy,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "suspendlevellin",   do_suspendlevellin, POS_DEAD, 0,  LOG_NORMAL, 0 },
    { "suspendlevelling", do_suspendlevelling, POS_DEAD, 0,  LOG_NORMAL, 1 },

    /*
     * Communication commands.
     */
    { "afk",		do_afk,		POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "admin",		do_admin,	POS_SLEEPING,	 L1,  LOG_NORMAL, 1 },
/*  { "auction",	do_auction,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 }, */
    { "deaf",		do_deaf,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "emote",		do_emote,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "gmote",		do_gmote,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "pmote",		do_pmote,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "ic",		do_ic,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { ",",		do_emote,	POS_RESTING,	 0,  LOG_NORMAL, 0 },
    { "gtell",		do_gtell,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { ";",		do_gtell,	POS_DEAD,	 0,  LOG_NORMAL, 0 },
/*  { "ooc",		do_music,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 }, */
    { "leavebrawle",    do_leavebrawle, POS_RESTING,    20,  LOG_NORMAL, 0 },
    { "leavebrawler",   do_leavebrawler,POS_RESTING,    20,  LOG_NORMAL, 1 },
    { "note",		do_note,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
	 { "noteboard",		do_noteboard,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "pose",		do_pose,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "quiet",		do_quiet,	POS_SLEEPING, 	 0,  LOG_NORMAL, 1 },
    { "reply",		do_reply,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "replay",		do_replay,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "say",		do_say,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "local",          do_say,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "vsay",		do_vsay,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
	{ "enochian",		do_enochian,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
	{ "bestow",		do_bestow,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "'",		do_say,         POS_RESTING,	 0,  LOG_NORMAL, 0 },
    { "?",		do_vsay,	POS_RESTING,	 0,  LOG_NORMAL, 0 },
    { "shout",		do_shout,	POS_RESTING,	 3,  LOG_NORMAL, 1 },
    { "sing",		do_sing,	POS_RESTING,	 3,  LOG_NORMAL, 1 },
    { "unread",		do_unread,	POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "yell",		do_yell,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "brawl",          do_brawl,       POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "joinbrawle",     do_joinbrawle,  POS_RESTING,    20,  LOG_NORMAL, 0 },
    { "joinbrawler",    do_joinbrawler, POS_RESTING,    20,  LOG_ALWAYS, 1 },
    { "journal",        do_journal,     POS_DEAD,        0,  LOG_NORMAL, 1 },

    /*
     * Object manipulation commands.
     */
    { "brandish",	do_brandish,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "close",		do_close,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "drink",		do_drink,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "drag",	    	do_drag, 	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "drop",		do_drop,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "eat",		do_eat,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "envenom",	do_envenom,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "fill",		do_fill,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "fire",		do_fire,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "give",		do_give,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "harvest",        do_harvest,     POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "heal",		do_heal,	POS_RESTING,	 0,  LOG_NORMAL, 1 }, 
    { "hold",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "list",		do_list,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "lock",		do_lock,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "offhand",  	do_offhand,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "open",		do_open,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "pick",		do_pick,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "plant",          do_plant,       POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "pour",		do_pour,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "push",         	do_push,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "put",		do_put,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "quaff",		do_quaff,	POS_SPRAWLED,	 0,  LOG_NORMAL, 1 },
    { "recite",		do_recite,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "remove",		do_remove,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "sell",		do_sell,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "shift",		do_shift,	POS_RESTING,    30,  LOG_NORMAL, 1 },
    { "unshift",	do_unshift,	POS_RESTING,	30,  LOG_NORMAL, 1 },
    { "take",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "sacrifice",	do_sacrifice,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
/*  { "junk",           do_sacrifice,   POS_RESTING,     0,  LOG_NORMAL, 0 }, */
/*  { "tap",      	do_sacrifice,   POS_RESTING,     0,  LOG_NORMAL, 0 }, */
/*  { "unlock",		do_unlock,	POS_RESTING,	 0,  LOG_NORMAL, 1 }, */
    { "value",		do_value,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "wear",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "zap",		do_zap,		POS_RESTING,	 0,  LOG_NORMAL, 1 },

    /*
     * Pueblo Commands.
     */
    { "pueblo", do_pueblo, POS_DEAD, 0, LOG_NORMAL, 1 },
    { "pmusic", do_pmusic, POS_DEAD, 0, LOG_NORMAL, 1 },
    { "pstop", do_pstop, POS_DEAD, 0, LOG_NORMAL, 1 },
    { "volume", do_volume, POS_DEAD, 0, LOG_NORMAL, 1 },

    /*
     * Combat commands.
     */
    { "armthrow",	do_armthrow,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "entangle",       do_entangle,    POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "backstab",	do_backstab,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "bash",		do_bash,	POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "clear",		do_clear,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
/*  { "brew",		do_brew,	POS_FIGHTING,    0,  LOG_NORMAL, 1 }, */
    { "pounce",         do_pounce,      POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "maul",           do_maul,        POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "charge",         do_charge,      POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "bs",		do_backstab,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "bf",     	do_battle, 	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "buddha",		do_burning_palm,POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "burning",        do_burning_palm,POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "sap",            do_sap,         POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "throw",          do_throw,       POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "crane",		do_crane_kick,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "dirt",		do_dirt,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "unally",  	do_unally,	POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "alliance",	do_alliance,	POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "embrace",	do_embrace,	POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "bloodpercent",   do_bloodpercent, POS_RESTING,    0,  LOG_NORMAL, 1 },
    { "wing",           do_wing,        POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "disarm",		do_disarm,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "dislodge",	do_dislodge,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "flee",		do_flee,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "kick",		do_kick,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "murde",		do_murde,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "murder",		do_murder,	POS_FIGHTING,	 5,  LOG_ALWAYS, 1 },
    { "rage",           do_rage,        POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "divine",         do_divine_focus,POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "fury",           do_fury,        POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "rescue",		do_rescue,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "scal",		do_murde,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "scalp",		do_scalp,	POS_FIGHTING,	 0,  LOG_ALWAYS, 1 },
    { "throw",		do_armthrow,	POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "hamstring",      do_hamstring,   POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "trip",		do_trip,	POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "whirlwind",      do_whirlwind,   POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "touch",		do_touch,	POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "vital",          do_vital_strike,POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "feed",		do_feed,	POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "harm",		do_harm_touch,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "lay",		do_lay_on_hands,POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "feast",		do_feast,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "purify",         do_purify,      POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "solemn",         do_solemn,      POS_STANDING,    0,  LOG_NORMAL, 0 },
    { "vow",            do_solemn,      POS_STANDING,    0,  LOG_NORMAL, 1 },
    /*
     * Mob command interpreter (placed here for faster scan...)
     */
    { "mob",		do_mob,		POS_DEAD,	 0,  LOG_NEVER,  0 },

    /*
     * Miscellaneous commands.
     */
	 { "clan",		do_clan,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "enter", 		do_enter, 	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "follow",		do_follow,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "gain",		do_gain,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "go",		do_enter,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
/*  { "group",		do_group,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 }, */
    { "groups",		do_groups,	POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "hide",		do_hide,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "play",		do_play,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
/*  { "practice",	do_practice,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 }, */
    { "qui",		do_qui,		POS_DEAD,	 0,  LOG_NORMAL, 0 },
    { "quit",		do_quit,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "recall",		do_recall,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "/",		do_recall,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "rent",		do_rent,	POS_DEAD,	 0,  LOG_NORMAL, 0 },
    { "save",		do_save,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "sleep",		do_sleep,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "shadow form",    do_shadow_form, POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "inspire",	do_inspire,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "soothe",         do_soothe,      POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "heightened senses",do_heightened_senses,POS_STANDING,0,LOG_NORMAL, 1 },
    { "sneak",		do_sneak,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "split",		do_split,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "steal",		do_steal,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "train",		do_train,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "visible",	do_visible,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "wake",		do_wake,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "where",		do_where,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "togauction",     do_togauction,     POS_SLEEPING,    0,  LOG_NORMAL, 0 },
    { "grant",		do_grant,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },

    /*
     * Brawler commands
     */
    { "brawllist",      do_brawllist,   POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "brawlerboard",   do_scoreboard,  POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "brawlerlist",    do_brawllist,   POS_DEAD,        0,  LOG_NORMAL, 0 },
    { "brawlerscore",   do_scoreboard,  POS_DEAD,        0,  LOG_NORMAL, 0 },
    { "brawlerkick",    do_brawlerkick, POS_DEAD,       IM,  LOG_ALWAYS, 1 },
    { "brawlboard",     do_scoreboard,  POS_DEAD,        0,  LOG_NORMAL, 0 },
    { "brawlkick",      do_brawlerkick, POS_DEAD,       IM,  LOG_ALWAYS, 0 },
    { "brawlscore",     do_scoreboard,  POS_DEAD,        0,  LOG_NORMAL, 0 },
/*  { "clobber",        do_clobber,     POS_STANDING,    0,  LOG_NORMAL, 1 }, */
    { "victory",        do_victory,     POS_STANDING,    0,  LOG_NORMAL, 1 },

    /*
     * Immortal commands.
     */
	{ "ninjaget",	do_ninjaget,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
	{ "ninjadrop",	do_ninjadrop,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
	{ "ninjaput",	do_ninjaput,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
	{ "ninjagive",	do_ninjagive,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
	{ "ninjaload",	do_ninjaload,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "troublemakers",  do_trouble,     POS_DEAD,       IM,  LOG_NORMAL, 1 },
    { "mask",           do_mask,        POS_DEAD,       L1,  LOG_ALWAYS, 1 },
    { "advance",	do_advance,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "buffer",		do_buffer,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "confiscate",	do_confiscate,	POS_DEAD,	L3,  LOG_ALWAYS, 1 },
    { "gcast",		do_gcast,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },

    { "gaccount",       do_gaccount,    POS_SLEEPING,   L4,  LOG_NORMAL, 1 },
    { "gdeposit",       do_gdeposit,    POS_SLEEPING,   L4,  LOG_ALWAYS, 1 },
    { "gdaccount",      do_gaccount,    POS_SLEEPING,   L4,  LOG_NORMAL, 0 },
    { "gdwithdraw",     do_gwithdraw,   POS_SLEEPING,   L4,  LOG_ALWAYS, 0 },
    { "gwithdraw",      do_gwithdraw,   POS_SLEEPING,   L4,  LOG_ALWAYS, 1 },

    { "dump",		do_dump,	POS_DEAD,	L1,  LOG_ALWAYS, 0 },
    { "pload",	        do_pload,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "trust",		do_trust,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "violate",	do_violate,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "addapply",       do_addapply,    POS_DEAD,       ML,  LOG_ALWAYS, 1 },
    { "allow",		do_allow,	POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { "ban",		do_ban,		POS_DEAD,	L2,  LOG_ALWAYS, 1 },
	{ "damage",		do_damage,	POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { "deny",		do_deny,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "disable",        do_disable,     POS_DEAD,       L2,  LOG_ALWAYS, 1 },
    { "disconnect",	do_disconnect,	POS_DEAD,	L3,  LOG_ALWAYS, 1 },
    { "flag",		do_flag,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "freeze",		do_freeze,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "modacct",        do_modacct,     POS_DEAD,       L1,  LOG_ALWAYS, 1 },
    { "multilogin",     do_multilogin,  POS_DEAD,       L3,  LOG_ALWAYS, 1 },
    { "notetag",        do_notetag,     POS_DEAD,       ML,  LOG_ALWAYS, 1 },
    { "unallow",        do_unallow,     POS_DEAD,       L3,  LOG_ALWAYS, 1 },
    { "permban",	do_permban,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "protect",	do_protect,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "reboo",		do_reboo,	POS_DEAD,	L1,  LOG_NORMAL, 0 },
    { "reboot",		do_reboot,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "rename",		do_rename,      POS_DEAD,       L3,  LOG_ALWAYS, 1 },
    { "set",		do_set,		POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { "shutdow",	do_shutdow,	POS_DEAD,	L1,  LOG_NORMAL, 0 },
    { "shutdown",	do_shutdown,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "undeny",         do_undeny,      POS_DEAD,       ML,  LOG_ALWAYS, 1 },
    { "whomulti",	do_whomulti,	POS_DEAD,	L3,  LOG_NORMAL, 1 },
    { "wizlock",	do_wizlock,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },

    { "fgive",          do_fgive,       POS_RESTING,    L3,  LOG_NORMAL, 1 },
    { "force",		do_force,	POS_DEAD,	L7,  LOG_ALWAYS, 1 },
    { "wizlag",         do_lagger,      POS_DEAD,       L1,  LOG_ALWAYS, 1 },
    { "load",		do_load,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "ftrans",         do_ftrans,      POS_DEAD,       L5,  LOG_ALWAYS, 1 },
	{ "tardis",         do_tardis,      POS_DEAD,       L5,  LOG_NEVER, 1 },
    { "newlock",	do_newlock,	POS_DEAD,	L3,  LOG_ALWAYS, 1 },
    { "nochannels",	do_nochannels,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "noemote",	do_noemote,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "noshout",	do_noshout,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "notell",		do_notell,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "pecho",		do_pecho,	POS_DEAD,	L4,  LOG_ALWAYS, 1 }, 
    { "pardon",		do_pardon,	POS_DEAD,	L3,  LOG_ALWAYS, 1 },
    { "purge",		do_purge,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "restore",	do_restore,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "sla",		do_sla,		POS_DEAD,	L1,  LOG_NORMAL, 0 },
    { "slay",		do_slay,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "teleport",	do_transfer,    POS_DEAD,	L5,  LOG_ALWAYS, 0 },	
    { "transfer",	do_transfer,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "transin",        do_transin,     POS_DEAD,       L5,  LOG_NORMAL, 1 },
    { "transout",       do_transout,    POS_DEAD,       L5,  LOG_NORMAL, 1 },

    { "poofin",		do_bamfin,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "poofout",	do_bamfout,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "gecho",		do_echo,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "holylight",	do_holylight,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "incognito",	do_incognito,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "invis",		do_invis,	POS_DEAD,	IM,  LOG_NORMAL, 0 },
    { "log",		do_log,		POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "memory",		do_memory,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "mwhere",		do_mwhere,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "owhere",		do_owhere,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "peace",		do_peace,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "history",	do_history,	POS_DEAD,	0 ,  LOG_NORMAL, 1 },
    { "echo",		do_recho,	POS_DEAD,	L6,  LOG_ALWAYS, 1 },
    { "raceecho",       do_raceecho,    POS_DEAD,       L6,  LOG_ALWAYS, 1 },
    { "gdecho",         do_gdecho,      POS_DEAD,       L6,  LOG_ALWAYS, 1 },
    { "racewizi",	do_racewizi,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "return",         do_return,      POS_DEAD,       L6,  LOG_NORMAL, 1 },
    { "snoop",		do_snoop,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "spideysense",    do_spidey,      POS_RESTING,    IM,  LOG_NORMAL, 1 },
    { "stat",		do_stat,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "stats",          do_score,       POS_DEAD,        0,  LOG_NORMAL, 0 },
    { "string",		do_string,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "switch",		do_switch,	POS_DEAD,	L6,  LOG_ALWAYS, 1 },
    { "wizinvis",	do_invis,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "unstring",       do_unstring,    POS_DEAD,       IM,  LOG_NORMAL, 1 },
    { "vnum",		do_vnum,	POS_DEAD,	L4,  LOG_NORMAL, 1 },
    { "zecho",		do_zecho,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "quecho",         do_qecho,       POS_DEAD,       IM,  LOG_ALWAYS, 0 },
    { "qecho",          do_qecho,       POS_DEAD,       IM,  LOG_ALWAYS, 1 },

    { "bankaccts",      do_bankaccts,   POS_DEAD,       L1,  LOG_NORMAL, 1 },
    { "becho",          do_brawlerecho, POS_DEAD,       L4,  LOG_ALWAYS, 1 },
    { "clone",		do_clone,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "astrip",		do_astrip,	POS_DEAD,	IM,  LOG_ALWAYS, 1 },
    { "wiznet",		do_wiznet,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "immtalk",	do_immtalk,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "imotd",          do_imotd,       POS_DEAD,       IM,  LOG_NORMAL, 1 },
    { ":",		do_immtalk,	POS_DEAD,	IM,  LOG_NORMAL, 0 },
    { "smote",		do_smote,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "prefi",		do_prefi,	POS_DEAD,	IM,  LOG_NORMAL, 0 },
    { "prefix",		do_prefix,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "pretitle",	do_pretitle,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "mpdump",		do_mpdump,	POS_DEAD,	IM,  LOG_NORMAL, 0 },
    { "mpstat",		do_mpstat,	POS_DEAD,	IM,  LOG_NORMAL, 0 },
    { "mplistmobs",     do_mplistmobs,  POS_DEAD,       IM,  LOG_NORMAL, 0 },
    { "guildlist",      do_guildlist,   POS_DEAD,       L8,  LOG_NORMAL, 1 },
    { "pause",          do_pause,       POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "cleanse",        do_cleanse,     POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "subdue",         do_subdue,      POS_STANDING,   IM,  LOG_ALWAYS, 1 },
    { "quad",         do_quadruple,      POS_DEAD,   L1,  LOG_ALWAYS, 1 },

	

    /*
     * OLC
     */
    { "edit",		do_olc,		POS_DEAD,       L5,  LOG_NORMAL, 0 },
    { "asave",          do_asave,	POS_DEAD,       L5,  LOG_NORMAL, 0 },
    { "alist",		do_alist,	POS_DEAD,       IM,  LOG_NORMAL, 0 },
    { "resets",		do_resets,	POS_DEAD,       L5,  LOG_NORMAL, 0 },
    { "objresets",      do_objresets,   POS_DEAD,       L5,  LOG_NORMAL, 0 },
    { "mobresets",      do_mobresets,   POS_DEAD,       L5,  LOG_NORMAL, 0 },
    { "redit",		do_redit,	POS_DEAD,       L5,  LOG_NORMAL, 0 },
    { "medit",		do_medit,	POS_DEAD,       L5,  LOG_NORMAL, 0 },
    { "aedit",		do_aedit,	POS_DEAD,       L2,  LOG_NORMAL, 0 },
    { "oedit",		do_oedit,	POS_DEAD,       L5,  LOG_NORMAL, 0 },
    { "mpedit",		do_mpedit,	POS_DEAD,       L5,  LOG_NORMAL, 0 },
    { "sedit",          do_sedit,       POS_DEAD,       L1,  LOG_NORMAL, 0 },
    { "withdraw",	do_withdraw,	POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "laston",		do_laston,	POS_SLEEPING,   L5,  LOG_NORMAL, 1 },

    { "exchange",	do_change,      POS_STANDING,    0,  LOG_NORMAL, 1 }, 
    { "deposit",	do_deposit,	POS_STANDING,    0,  LOG_NORMAL, 1 }, 
    { "account",	do_account,	POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "interest",       do_interest,    POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "fees",           do_interest,    POS_SLEEPING,    0,  LOG_NORMAL, 1 },

    { "phase",	        do_phase,	POS_STANDING,    0,  LOG_NORMAL, 1 }, 
    { "openroom",	do_openroom,	POS_DEAD,	IM,  LOG_NORMAL, 0 },
    { "openobj",	do_openobj,	POS_DEAD,	IM,  LOG_NORMAL, 0 },
    { "openmob",	do_openmob,	POS_DEAD,	IM,  LOG_NORMAL, 0 },
/*  { "openspells",	do_openspells,  POS_DEAD,       IM,  LOG_NORMAL, 0 },*/
    { "objtoggle", 	do_objtoggle,   POS_DEAD,	IM,  LOG_ALWAYS, 1 },

  /* Trade skill commands */
    { "tailor",		do_tailor,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "trades",		do_trades,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "bake",		do_bake,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "fletch",		do_fletch,	POS_RESTING,	 0,  LOG_NORMAL, 1 },

    { "offer", 		do_offer, 	POS_RESTING, 	 0,  LOG_NORMAL, 1 },
    { "drain", 		do_drain, 	POS_RESTING, 	 0,  LOG_NORMAL, 1 },
    { "transmogrify",   do_transmogrify,POS_DEAD,       L1,  LOG_NORMAL, 1 },
    { "maxstat", 	do_maxstat, 	POS_DEAD, 	 0,  LOG_NORMAL, 1 },
    { "maxstats", 	do_maxstat, 	POS_DEAD, 	 0,  LOG_NORMAL, 0 },
    { "throwdice", 	do_throwdice, 	POS_RESTING, 	 0,  LOG_NORMAL, 1 },
    { "abilities", 	do_practice, 	POS_SLEEPING, 	 0,  LOG_NORMAL, 0 },
    { "power", 		do_practice, 	POS_SLEEPING, 	 0,  LOG_NORMAL, 0 },
    { "drmode",	        do_drmode,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "screwtime",	do_screwtime,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "screwtick",      do_screwtick,   POS_DEAD,       L1,  LOG_ALWAYS, 1 },
    { "findkey",        do_findkey,     POS_DEAD,       L1,  LOG_ALWAYS, 1 },
    { "mcast",          do_mcast,       POS_FIGHTING,   IM,  LOG_NORMAL, 1 },
    { "recogon",        do_mprecogon,   POS_DEAD,       L1,  LOG_NORMAL, 1 },
    { "recogoff",       do_mprecogoff,  POS_DEAD,       L1,  LOG_NORMAL, 1 },
    { "nuke",           do_nuke,        POS_DEAD,       IM,  LOG_ALWAYS, 1 },
    { "config",         do_config,      POS_DEAD,       ML,  LOG_ALWAYS, 1 },
    { "fakeslay",       do_fakeslay,    POS_FIGHTING,   IM,  LOG_ALWAYS, 0 }, 
    { "dumpwho",        do_dumpwho,     POS_DEAD,       ML,  LOG_ALWAYS, 1 },
    { "sanguine",       do_sanguine_bond,POS_STANDING,  15,  LOG_NORMAL, 0 },
    { "scast",          do_scast,       POS_FIGHTING,   IM,  LOG_NORMAL, 1 },
	/*seraph stuff*/
    { "energy",          do_lifeenergy,       POS_DEAD,  0,  LOG_NORMAL, 1 },
	{ "project",          do_project,       POS_DEAD,  0,  LOG_NORMAL, 1 },
	{ "makeseraph",         do_makeseraph,      POS_DEAD,       L1,  LOG_ALWAYS, 1 },
    /*
     * End of list.
     */
    { "",		0,		POS_DEAD,	 0,  LOG_NORMAL, 0 }
};




/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
  char command[MAX_INPUT_LENGTH];
  char logline[MAX_INPUT_LENGTH];
  int cmd;
  int trust;
  bool found;
  int wiz_flag = -1;

  /*
   * Strip leading spaces.
   */
  while ( isspace(*argument) || *argument == '/' )
    argument++;
  if ( argument[0] == '\0' )
    return;

  if (IS_SET(ch->comm, COMM_AFK))
    {
      send_to_char("AFK mode removed.",ch);
      Check_Replay(ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }
  
  /*
   * No hiding.
   */
  REMOVE_BIT( ch->affected_by, AFF_HIDE );

  /*
   * Implement freeze command.
   */
  if ( !IS_NPC(ch) && IS_SET(ch->act_bits, PLR_FREEZE) )
    {
      send_to_char( "You're totally frozen!\n\r", ch );
      return;
    }
  
  /*
   * Grab the command word.
   * Special parsing so ' can be a command,
   *   also no spaces needed after punctuation.
   */
  strcpy( logline, argument );

  if( argument[ 0 ] == '\'' ||
      ( !isalpha( argument[ 0 ] ) && !isdigit( argument[ 0 ] ) && isspace( argument[ 1 ] ) ) )
    {
      
      /* if ( !isalpha(argument[0]) && !isdigit(argument[0]) && isspace( argument[ 1 ] ) ) */
 
      command[0] = argument[0];
      command[1] = '\0';
      argument++;
      while ( isspace(*argument) )
	argument++;
    }
  else
    {
      argument = one_argument( argument, command );
    }

  /*
   * Look for command in command table.
   */
  found = FALSE;
  trust = get_trust( ch );
  for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
      if ( command[0] == cmd_table[cmd].name[0]
	   &&   !str_prefix( command, cmd_table[cmd].name )
	   &&   cmd_table[cmd].level <= trust )
	{
	  found = TRUE;
	  break;
	}
    }

  /*
   * Log and snoop.
   */
  if ( cmd_table[cmd].log == LOG_NEVER )
    strcpy( logline, "" );

  if( !IS_NPC( ch ) &&
      (IS_SET( ch->act_bits, PLR_LOG ) || fLogAll ) )
    {
      wiz_flag = WIZ_LOGS;
    }
  else if( cmd_table[ cmd ].log == LOG_ALWAYS )
    {
      wiz_flag = WIZ_SECURE;
    }

    if( wiz_flag != -1 )
    {
      char s[MAX_STRING_LENGTH];
/*    char s[2*MAX_INPUT_LENGTH], *ps;
      int i;

      ps = s;*/
      sprintf( log_buf, "Log %s: %s", ch->name, logline);
      /*
      for (i = 0; log_buf[i]; i++) {
	*ps++=log_buf[i];
	if (log_buf[i]=='$')
	  *ps++='$';
	if (log_buf[i]=='{')
	  *ps++='{';
      }
      *ps=0; */
      strcpy( s, escape_color(log_buf) );
      wiznet(s,ch,NULL,wiz_flag,0,get_trust(ch));
      log_string( log_buf );
    }


  if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
      write_to_buffer( ch->desc->snoop_by, "% ",    2 );
      write_to_buffer( ch->desc->snoop_by, logline, 0 );
      write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    if( !IS_NPC( ch ) &&
	IS_AFFECTED3( ch, AFF_FROG ) &&
	(*cmd_table[cmd].do_fun) != do_look &&
	(*cmd_table[cmd].do_fun) != do_glance &&
	(*cmd_table[cmd].do_fun) != do_where &&
	(*cmd_table[cmd].do_fun) != do_inventory &&
	(*cmd_table[cmd].do_fun) != do_nosummon &&
	(*cmd_table[cmd].do_fun) != do_quit &&
	(*cmd_table[cmd].do_fun) != do_affects &&
	(*cmd_table[cmd].do_fun) != do_heal &&
	(*cmd_table[cmd].do_fun) != do_help &&
	(*cmd_table[cmd].do_fun) != do_eat &&
	(*cmd_table[cmd].do_fun) != do_say )
      {
	send_to_char( "Frogs can't do that.\n\r", ch );
	act( "A frog hops around in frustration!", ch, NULL, NULL, TO_ROOM );
	return;
      }
	  
	  
	/** Skill checks for the seraph spell tabula rasa. If new skills
	*** are added that TR should prevent, add them to the list below. 
	*** spells are handled separately using a booleon in const.c
	**/
	if 	(is_affected(ch,skill_lookup("tabula rasa"))){
	
		if ((*cmd_table[cmd].do_fun) == do_charge || 
			(*cmd_table[cmd].do_fun) == do_feed ||
			(*cmd_table[cmd].do_fun) == do_fury ||
			(*cmd_table[cmd].do_fun) == do_hamstring ||
			(*cmd_table[cmd].do_fun) == do_maul ||
			(*cmd_table[cmd].do_fun) == do_pounce ||
			(*cmd_table[cmd].do_fun) == do_touch ||
			(*cmd_table[cmd].do_fun) == do_sap ||
			(*cmd_table[cmd].do_fun) == do_wing) {
		
			send_to_char("A divine force prevents you from using that skill.\n\r",ch);
			return;
		}
	}

  if ( !found )
    {
      /*
       * Look for command in socials table.
       */
      if ( !check_social( ch, command, argument ) )
	send_to_char( "Huh?\n\r", ch );
      return;
    }
  else
    if (check_disabled (&cmd_table[cmd]))
      {
	send_to_char("This command has been temporarily disabled.\n\r",ch);
	return;
      }

    /*
     * Character not in position for command?
     */
    if ( ch->position < cmd_table[cmd].position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "Lie still; you are DEAD.\n\r", ch );
	    break;

	case POS_SPRAWLED:
	    send_to_char( "You are sprawled out on the ground!\n\r", ch );
	    break;

	case POS_UNCONSCIOUS:
	    send_to_char( "You are unconscious you can't do anything!\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "Nah... You feel too relaxed...\n\r", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "Better stand up first.\n\r",ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;

	}
	return;
    }

    if (ch->pcdata)
      if (ch->pcdata->confirm_delete && !is_name("delete",command))
         ch->pcdata->confirm_delete = 0;


    /*
     * Dispatch the command.
     */
    (*cmd_table[cmd].do_fun) ( ch, argument );

    tail_chain( );
    return;
}



bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;

    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "You are anti-social!\n\r", ch );
	return TRUE;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "Lie still; you are DEAD.\n\r", ch );
	return TRUE;

	case POS_SPRAWLED:
	    send_to_char( "You are sprawled out on the ground!\n\r", ch );
		return(TRUE);

	case POS_UNCONSCIOUS:
	    send_to_char( "You are unconscious you can't do anything!\n\r", ch );
		return(TRUE);

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
	    break;
	send_to_char( "In your dreams, or what?\n\r", ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
	act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
	act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
    }
    else if ( ( victim = get_char_room( ch, arg, TRUE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
	act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
    }
    else
    {
	act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
	act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );
	act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim) 
	&&   victim->desc == NULL)
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act( social_table[cmd].others_found,
		    victim, NULL, ch, TO_NOTVICT );
		act( social_table[cmd].char_found,
		    victim, NULL, ch, TO_CHAR    );
		act( social_table[cmd].vict_found,
		    victim, NULL, ch, TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
		act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		act( "You slap $N.",  victim, NULL, ch, TO_CHAR    );
		act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
		break;
	    }
	}
    }

    return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number ( char *arg )
{
 
    if ( *arg == '\0' )
        return FALSE;
 
    if ( *arg == '+' || *arg == '-' )
        arg++;
 
    for ( ; *arg != '\0'; arg++ )
    {
        if ( !isdigit( *arg ) )
            return FALSE;
    }
 
    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(char *argument, char *arg)
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
        if ( *pdot == '*' )
        {
            *pdot = '\0';
            number = atoi( argument );
            *pdot = '*';
            strcpy( arg, pdot+1 );
            return number;
        }
    }
 
    strcpy( arg, argument );
    return 1;
}

/*
 * Same behavior as one_argument() except for case-preservation
 */

char* preserve_case_one_argument( char* argument, char* arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Pick off one sentence from a string and return the rest.
 */
char *one_sentence( char *argument, char *arg_first )
{
    bool eos = FALSE;
    bool lastlf = FALSE;

    /* Chop off leading spaces */
    while ( isspace(*argument) && *argument != '\0' )
	argument++;

    while ( *argument != '\0' )
    {
	if ( eos && isspace(*argument) )
	{
            if ( *argument == ' ' )
                argument++;
	    break;
	}
	else if ( *argument == '.' || *argument == '!' || *argument == '?' )
	{
            eos = TRUE;
	}

        if ( *argument == '\n' || *argument == '\r' )    /* Don't copy CR/LFs */
        {
            lastlf = TRUE;
        }
        else
        {
      /* if the last char was a CR/LF, we may need to substitute with a space */
            if ( lastlf && !isspace(*argument) && *(arg_first-1) != ' ' )
            {
                *arg_first = ' ';
                arg_first++;
            }
            *arg_first = *argument;
            arg_first++;
            lastlf = FALSE;
        }
        argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) && *argument != '\0' )
	argument++;

    return argument;
}

/*
 * Like one_argument, but disregards punctuation
 */
char *one_word( char *argument, char *arg_first )
{
    /* Chop off leading spaces */
    while ( !isalpha(*argument) && *argument != '\0' )
	argument++;

    while ( *argument != '\0' )
    {
        if ( !isalpha(*argument)
        &&   ( *argument != '\'' || !isalpha(*(argument+1)) ) )
        {
            break;
	}
        *arg_first = *argument;
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    return argument;
}

/*
 * Contributed by Alander.
 */
void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
 
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level <  LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust( ch ) 
	&&   cmd_table[cmd].show)
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    send_to_char( buf, ch );
	    if ( ++col % 6 == 0 )
		send_to_char( "\n\r", ch );
	}
    }
 
    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
    return;
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
 
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level >= LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust( ch ) 
        &&   cmd_table[cmd].show)
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    send_to_char( buf, ch );
	    if ( ++col % 6 == 0 )
		send_to_char( "\n\r", ch );
	}
    }
 
    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
    return;
}

void do_disable (CHAR_DATA *ch, char *argument)
{
        int i;
        DISABLED_DATA *p,*q;
        char buf[100];

        if (IS_NPC(ch))
        {
                send_to_char ("RETURN first.\n\r",ch);
                return;
        }

        if (!argument[0]) /* Nothing specified. Show disabled commands. */
        {
                if (!disabled_first) /* Any disabled at all ? */
                {
                        send_to_char ("There are no commands disabled.\n\r",ch);
                        return;
                }

                send_to_char ("Disabled commands:\n\r"
                              "Command      Level   Disabled by\n\r",ch);

                for (p = disabled_first; p; p = p->next)
                {
                        sprintf (buf, "%-12s %5d   %-12s\n\r",p->command->name,p->level, p->disabled_by);
                        send_to_char (buf,ch);
                }
                return;
        }

        /* command given */

        /* First check if it is one of the disabled commands */
        for (p = disabled_first; p ; p = p->next)
                if (!str_cmp(argument, p->command->name))
                        break;

        if (p) /* this command is disabled */
        {
                if (get_trust(ch) < p->level)
                {
                        send_to_char ("This command was disabled by a higher power.\n\r",ch);
                        return;
                }

                /* Remove */

                if (disabled_first == p) /* node to be removed == head ? */
                        disabled_first = p->next;
                else /* Find the node before this one */
                {
                        for (q = disabled_first; q->next != p; q = q->next);
                        q->next = p->next;
                }

                free_string (p->disabled_by); /* free name of disabler */
                free_mem (p,sizeof(DISABLED_DATA)); /* free node */
                save_disabled(); /* save to disk */
                send_to_char ("Command enabled.\n\r",ch);
        }
        else /* not a disabled command, check if that command exists */
        {
                /* IQ test */
                if (!str_cmp(argument,"disable"))
                {
                        send_to_char ("You cannot disable the disable command.\n\r",ch);
                        return;
                }

                /* Search for the command */
                for (i = 0; cmd_table[i].name[0] != '\0'; i++)
                        if (!str_cmp(cmd_table[i].name, argument))
                                break;

                /* Found? */
                if (cmd_table[i].name[0] == '\0')
                {
                        send_to_char ("No such command.\n\r",ch);
                        return;
                }

                /* Can the imm use this command at all ? */               
                if (cmd_table[i].level > get_trust(ch))
                {
                        send_to_char ("You don't have access to that command; you cannot disable it.\n\r",ch);
                        return;
                }

                /* Disable the command */

                p = alloc_mem (sizeof(DISABLED_DATA));

                p->command = &cmd_table[i];
                p->disabled_by = str_dup (ch->name); /* save name of disabler */
                p->level = get_trust(ch); /* save trust */
                p->next = disabled_first;
                disabled_first = p; /* add before the current first element */

                send_to_char ("Command disabled.\n\r",ch);
                save_disabled(); /* save to disk */
        }
}

/* Check if that command is disabled
   Note that we check for equivalence of the do_fun pointers; this means
   that disabling 'chat' will also disable the '.' command
*/
bool check_disabled (const struct cmd_type *command)
{
        DISABLED_DATA *p;

        for (p = disabled_first; p ; p = p->next)
                if (p->command->do_fun == command->do_fun)
                        return TRUE;

        return FALSE;
}

/* Load disabled commands */
void load_disabled()
{
        FILE *fp;
        DISABLED_DATA *p;
        char *name;
        int i;

        disabled_first = NULL;

        fp = fopen (DISABLED_FILE, "r");

        if (!fp) /* No disabled file.. no disabled commands : */
                return;

        name = fread_word (fp);

        while (str_cmp(name, END_MARKER)) /* as long as name is NOT END_MARKER */
        {
                /* Find the command in the table */
                for (i = 0; cmd_table[i].name[0] ; i++)
                        if (!str_cmp(cmd_table[i].name, name))
                                break;

                if (!cmd_table[i].name[0]) /* command does not exist? */
                {
                        bug ("Skipping unknown command in " DISABLED_FILE " file.",0);
                        fread_number(fp); /* level */
                        fread_word(fp); /* disabled_by */
                }
                else /* add new disabled command */
                {
                        p = alloc_mem(sizeof(DISABLED_DATA));
                        p->command = &cmd_table[i];
                        p->level = fread_number(fp);
                        p->disabled_by = str_dup(fread_word(fp));
                        p->next = disabled_first;

                        disabled_first = p;

                }

                name = fread_word(fp);
        }

        fclose (fp);
}

/* Save disabled commands */
void save_disabled()
{
        FILE *fp;
        DISABLED_DATA *p;

        if (!disabled_first) /* delete file if no commands are disabled */
        {
                unlink (DISABLED_FILE);
                return;
        }

        fp = fopen (DISABLED_FILE, "w");


        if (!fp)
        {
                bug ("Could not open " DISABLED_FILE " for writing",0);
                return;
        }

        for (p = disabled_first; p ; p = p->next)
                fprintf (fp, "%s %d %s\n", p->command->name, p->level, p->disabled_by);

        fprintf (fp, "%s\n",END_MARKER);

        fclose (fp);
}


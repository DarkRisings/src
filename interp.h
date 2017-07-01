/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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

/* for command types */
#define ML MAX_LEVEL	/* implementor */
#define L1 MAX_LEVEL - 1  	/* creator */
#define L2 MAX_LEVEL - 2	/* supreme being */
#define L3 MAX_LEVEL - 3	/* deity */
#define L4 MAX_LEVEL - 4	/* god */
#define L5 MAX_LEVEL - 5	/* immortal */
#define L6 MAX_LEVEL - 6	/* demigod */
#define L7 MAX_LEVEL - 7	/* angel */
#define L8 MAX_LEVEL - 8	/* avatar */
#define IM LEVEL_IMMORTAL 	/* avatar */
#define HE LEVEL_HERO	/* hero */

typedef void( *QUEST_FUN ) ( CHAR_DATA*, char* );

struct quest_cmd
{
  char* name;
  char* syntax;
  QUEST_FUN cmd;
  bool immort;
};

extern const struct quest_cmd quest_cmd_tbl[];


char* preserve_case_one_argument( char* argument, char* arg_first );


/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
    sh_int		position;
    sh_int		level;
    sh_int		log;
    sh_int              show;
};

/* the command table itself */
extern	const	struct	cmd_type	cmd_table	[];

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN( do_guildtalk );
DECLARE_DO_FUN( do_availability );
DECLARE_DO_FUN( do_transmogrify );
DECLARE_DO_FUN( do_drain );
DECLARE_DO_FUN( do_offer );
DECLARE_DO_FUN( do_cleanse );
DECLARE_DO_FUN( do_quest );
DECLARE_DO_FUN( do_qsay );
DECLARE_DO_FUN( quest_teamsay );
DECLARE_DO_FUN( do_suspendlevellin );
DECLARE_DO_FUN( do_suspendlevelling );
DECLARE_DO_FUN( do_whirlwind    );
DECLARE_DO_FUN( do_backup       );
DECLARE_DO_FUN( do_mplistmobs  );
DECLARE_DO_FUN( do_rename	);
DECLARE_DO_FUN( do_multilogin	);
DECLARE_DO_FUN( do_unallow	);
DECLARE_DO_FUN( do_addapply     );
DECLARE_DO_FUN( do_disable	);
DECLARE_DO_FUN(	do_shift	);
DECLARE_DO_FUN(	do_learn	);
DECLARE_DO_FUN(	do_offhand	);
DECLARE_DO_FUN(	do_pretitle	);
DECLARE_DO_FUN(	do_togauction	);
DECLARE_DO_FUN(	do_approve	);
DECLARE_DO_FUN(	do_advance	);
DECLARE_DO_FUN(	do_confiscate	);
DECLARE_DO_FUN(	do_gcast	);
DECLARE_DO_FUN( do_affects	);
DECLARE_DO_FUN( do_afk		);
DECLARE_DO_FUN(	do_allow	);
DECLARE_DO_FUN( do_admin	);
DECLARE_DO_FUN(	do_astrip	);
DECLARE_DO_FUN(	do_areas	);
DECLARE_DO_FUN( do_armthrow	);
DECLARE_DO_FUN( do_entangle     );
DECLARE_DO_FUN(	do_at		);
DECLARE_DO_FUN(	do_auction	);
DECLARE_DO_FUN(	do_aucchannel	);
DECLARE_DO_FUN( do_autoassist	);
DECLARE_DO_FUN( do_autoexit	);
DECLARE_DO_FUN( do_autogold	);
DECLARE_DO_FUN( do_autolist	);
DECLARE_DO_FUN( do_autoloot	);
DECLARE_DO_FUN( do_autosac	);
DECLARE_DO_FUN( do_autosplit	);
DECLARE_DO_FUN(	do_backstab	);
DECLARE_DO_FUN(	do_bamfin	);
DECLARE_DO_FUN(	do_bamfout	);
DECLARE_DO_FUN(	do_ban		);
DECLARE_DO_FUN( do_bash		);
DECLARE_DO_FUN( do_lagger       );
/*DECLARE_DO_FUN( do_brew		); */
DECLARE_DO_FUN( do_battlebrief  );
DECLARE_DO_FUN( do_bloodpool    );
DECLARE_DO_FUN( do_buffer	);
DECLARE_DO_FUN( do_pounce       );
DECLARE_DO_FUN( do_charge       );
DECLARE_DO_FUN( do_maul         );
DECLARE_DO_FUN( do_battle	);
DECLARE_DO_FUN(	do_brandish	);
DECLARE_DO_FUN( do_brief	);
DECLARE_DO_FUN( do_burning_palm );
DECLARE_DO_FUN( do_sap          );
DECLARE_DO_FUN( do_throw        );
DECLARE_DO_FUN(	do_bug		);
DECLARE_DO_FUN(	do_buy		);
DECLARE_DO_FUN(	do_cast		);
DECLARE_DO_FUN( do_changes	);
DECLARE_DO_FUN( do_channels	);
DECLARE_DO_FUN( do_clone	);
DECLARE_DO_FUN(	do_close	);
DECLARE_DO_FUN( do_colour       );	/* Colour Command By Lope */
DECLARE_DO_FUN(	do_commands	);
DECLARE_DO_FUN( do_combine	);
DECLARE_DO_FUN( do_compact	);
DECLARE_DO_FUN(	do_compare	);
DECLARE_DO_FUN(	do_consider	);
DECLARE_DO_FUN( do_count	);
DECLARE_DO_FUN( do_crane_kick	);
DECLARE_DO_FUN(	do_credits	);
DECLARE_DO_FUN(	do_clantalk	);
DECLARE_DO_FUN(	do_clan	);
DECLARE_DO_FUN(	do_damage	);
DECLARE_DO_FUN( do_deaf		);
DECLARE_DO_FUN( do_delet	);
DECLARE_DO_FUN( do_delete	);
DECLARE_DO_FUN(	do_deny		);
DECLARE_DO_FUN(	do_description	);
DECLARE_DO_FUN(	do_descfull	);
DECLARE_DO_FUN(	do_deschalf	);
DECLARE_DO_FUN( do_dirt		);
DECLARE_DO_FUN( do_drmode	);
DECLARE_DO_FUN( do_wing         );
DECLARE_DO_FUN(	do_disarm	);
DECLARE_DO_FUN(	do_dislodge	);
DECLARE_DO_FUN(	do_disconnect	);
DECLARE_DO_FUN(	do_down		);
DECLARE_DO_FUN(	do_drink	);
DECLARE_DO_FUN(	do_drag	);
DECLARE_DO_FUN(	do_drop		);
DECLARE_DO_FUN( do_dump		);
DECLARE_DO_FUN(	do_east		);
DECLARE_DO_FUN(	do_eat		);
DECLARE_DO_FUN(	do_echo		);
DECLARE_DO_FUN( do_unally	);
DECLARE_DO_FUN( do_alliance	);
DECLARE_DO_FUN( do_embrace	);
DECLARE_DO_FUN( do_bloodpercent );
DECLARE_DO_FUN(	do_emote	);
DECLARE_DO_FUN(	do_gmote	);
DECLARE_DO_FUN( do_enter	);
DECLARE_DO_FUN( do_envenom	);
DECLARE_DO_FUN(	do_equipment	);
DECLARE_DO_FUN(	do_examine	);
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN( do_feast	);
DECLARE_DO_FUN( do_feed		);
DECLARE_DO_FUN(	do_fill		);
DECLARE_DO_FUN( do_fire		);
DECLARE_DO_FUN( do_flag		);
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN(	do_follow	);
DECLARE_DO_FUN(	do_force	);
DECLARE_DO_FUN(	do_freeze	);
DECLARE_DO_FUN( do_gain		);
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN(	do_give		);
DECLARE_DO_FUN( do_joinbrawle   );
DECLARE_DO_FUN( do_joinbrawler  );
DECLARE_DO_FUN( do_brawl        );
DECLARE_DO_FUN(	do_goto		);
DECLARE_DO_FUN( do_grant	);
DECLARE_DO_FUN( do_grats	);
DECLARE_DO_FUN(	do_group	);
DECLARE_DO_FUN( do_groups	);
DECLARE_DO_FUN(	do_gtell	);
DECLARE_DO_FUN( do_guild    	);
DECLARE_DO_FUN( do_jousting    	);
DECLARE_DO_FUN( do_harm_touch	);
DECLARE_DO_FUN( do_heal		);
DECLARE_DO_FUN(	do_help		);
DECLARE_DO_FUN(	do_hide		);
DECLARE_DO_FUN(	do_holylight	);
DECLARE_DO_FUN(	do_idea		);
DECLARE_DO_FUN(	do_immtalk	);
DECLARE_DO_FUN( do_incognito	);
DECLARE_DO_FUN( do_clantalk	);
DECLARE_DO_FUN( do_imotd	);
DECLARE_DO_FUN(	do_inventory	);
DECLARE_DO_FUN(	do_invis	);
DECLARE_DO_FUN(	do_kick		);
DECLARE_DO_FUN(	do_kill		);
DECLARE_DO_FUN(	do_laston	);
DECLARE_DO_FUN(	do_lay_on_hands	);
DECLARE_DO_FUN(	do_list		);
/*DECLARE_DO_FUN( do_lore         );*/
DECLARE_DO_FUN( do_load		);
DECLARE_DO_FUN(	do_lock		);
DECLARE_DO_FUN(	do_log		);
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN(	do_glance		);
DECLARE_DO_FUN(	do_loot		);
DECLARE_DO_FUN( do_mask         );
DECLARE_DO_FUN(	do_memory	);
DECLARE_DO_FUN(	do_mfind	);
DECLARE_DO_FUN(	do_mload	);
DECLARE_DO_FUN( do_minion	);
DECLARE_DO_FUN(	do_mset		);
DECLARE_DO_FUN(	do_mstat	);
DECLARE_DO_FUN(	do_mwhere	);
DECLARE_DO_FUN( do_mob		);
DECLARE_DO_FUN( do_motd		);
DECLARE_DO_FUN( do_mpstat	);
DECLARE_DO_FUN( do_mpdump	);
DECLARE_DO_FUN(	do_murde	);
DECLARE_DO_FUN(	do_murder	);
DECLARE_DO_FUN( do_music	);
DECLARE_DO_FUN( do_ic				);
DECLARE_DO_FUN( do_nb   			);
DECLARE_DO_FUN( do_newbie		);
DECLARE_DO_FUN( do_newlock	);
DECLARE_DO_FUN( do_news		);
DECLARE_DO_FUN( do_ninjaget		);
DECLARE_DO_FUN( do_ninjadrop		);
DECLARE_DO_FUN( do_ninjaput		);
DECLARE_DO_FUN( do_ninjagive		);
DECLARE_DO_FUN( do_ninjaload		);
DECLARE_DO_FUN( do_nochannels	);
DECLARE_DO_FUN(	do_noemote	);
DECLARE_DO_FUN( do_nofollow	);
DECLARE_DO_FUN( do_noloot	);
DECLARE_DO_FUN(	do_north	);
DECLARE_DO_FUN(	do_noshout	);
DECLARE_DO_FUN( do_nosummon	);
DECLARE_DO_FUN(	do_note		);
DECLARE_DO_FUN(	do_noteboard		);
DECLARE_DO_FUN(	do_notell	);
DECLARE_DO_FUN( do_objtoggle	);
DECLARE_DO_FUN(	do_ofind	);
DECLARE_DO_FUN(	do_oload	);
DECLARE_DO_FUN(	do_open		);
DECLARE_DO_FUN( do_openroom	);
DECLARE_DO_FUN( do_openobj	);
DECLARE_DO_FUN( do_openmob	);
/*DECLARE_DO_FUN( do_openspells   );*/
DECLARE_DO_FUN(	do_order	);
DECLARE_DO_FUN(	do_oset		);
DECLARE_DO_FUN(	do_ostat	);
DECLARE_DO_FUN( do_outfit	);
DECLARE_DO_FUN( do_owhere	);
DECLARE_DO_FUN(	do_pardon	);
DECLARE_DO_FUN(	do_password	);
DECLARE_DO_FUN(	do_peace	);
DECLARE_DO_FUN( do_pecho	);
DECLARE_DO_FUN( do_history	);
DECLARE_DO_FUN( do_permban	);
DECLARE_DO_FUN(	do_phase	);
DECLARE_DO_FUN(	do_pick		);
DECLARE_DO_FUN( do_play		);
DECLARE_DO_FUN( do_pload	);
DECLARE_DO_FUN( do_pmote	);
DECLARE_DO_FUN(	do_pose		);
DECLARE_DO_FUN( do_pour		);
DECLARE_DO_FUN(	do_practice	);
DECLARE_DO_FUN( do_prefi	);
DECLARE_DO_FUN( do_prefix	);
DECLARE_DO_FUN( do_prompt	);
DECLARE_DO_FUN( do_promote      );
DECLARE_DO_FUN( do_protect	);
DECLARE_DO_FUN(	do_purge	);
DECLARE_DO_FUN(	do_push     );
DECLARE_DO_FUN(	do_put		);
DECLARE_DO_FUN(	do_quaff	);
DECLARE_DO_FUN( do_question	);
DECLARE_DO_FUN(	do_qui		);
DECLARE_DO_FUN( do_quiet	);
DECLARE_DO_FUN(	do_quit		);
DECLARE_DO_FUN( do_quote	);
DECLARE_DO_FUN( do_rage         );
DECLARE_DO_FUN( do_divine_focus );
DECLARE_DO_FUN( do_fury         );
DECLARE_DO_FUN( do_read		);
DECLARE_DO_FUN(	do_reboo	);
DECLARE_DO_FUN(	do_reboot	);
DECLARE_DO_FUN(	do_recall	);
DECLARE_DO_FUN(	do_recho	);
DECLARE_DO_FUN( do_raceecho     );
DECLARE_DO_FUN( do_racewizi	);
DECLARE_DO_FUN( do_gdecho       );
DECLARE_DO_FUN(	do_recite	);
DECLARE_DO_FUN(	do_remove	);
DECLARE_DO_FUN(	do_rent		);
DECLARE_DO_FUN( do_replay	);
DECLARE_DO_FUN(	do_reply	);
DECLARE_DO_FUN(	do_report	);
DECLARE_DO_FUN(	do_rescue	);
DECLARE_DO_FUN(	do_rest		);
DECLARE_DO_FUN(	do_rank		);
DECLARE_DO_FUN(	do_restore	);
DECLARE_DO_FUN(	do_return	);
DECLARE_DO_FUN(	do_rset		);
DECLARE_DO_FUN(	do_rstat	);
DECLARE_DO_FUN( do_rules	);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_save		);
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN(	do_vsay		);
DECLARE_DO_FUN(	do_enochian		);
DECLARE_DO_FUN(	do_bestow		);
DECLARE_DO_FUN( do_scalp	);
DECLARE_DO_FUN( do_scan         );
DECLARE_DO_FUN(	do_score	);
DECLARE_DO_FUN(	do_scribe	);
DECLARE_DO_FUN( do_scroll	);
DECLARE_DO_FUN(	do_sell		);
DECLARE_DO_FUN( do_set		);
DECLARE_DO_FUN(	do_shout 	);
DECLARE_DO_FUN(	do_sing 	);
DECLARE_DO_FUN( do_show		);
DECLARE_DO_FUN(	do_shutdow	);
DECLARE_DO_FUN(	do_shutdown	);
DECLARE_DO_FUN( do_sit		);
DECLARE_DO_FUN( do_skills	);
DECLARE_DO_FUN(	do_sla		);
DECLARE_DO_FUN(	do_slay		);
DECLARE_DO_FUN(	do_sleep	);
DECLARE_DO_FUN(	do_slookup	);
DECLARE_DO_FUN( do_smote	);
DECLARE_DO_FUN(	do_sneak	);
DECLARE_DO_FUN(	do_snoop	);
DECLARE_DO_FUN( do_socials	);
DECLARE_DO_FUN(	do_south	);
DECLARE_DO_FUN( do_sockets	);
DECLARE_DO_FUN( do_spells	);
DECLARE_DO_FUN(	do_split	);
DECLARE_DO_FUN(	do_sset		);
DECLARE_DO_FUN(	do_stand	);
DECLARE_DO_FUN( do_stat		);
DECLARE_DO_FUN(	do_steal	);
DECLARE_DO_FUN( do_story	);
DECLARE_DO_FUN( do_string	);
/*DECLARE_DO_FUN(	do_surrender	);*/
DECLARE_DO_FUN(	do_switch	);
DECLARE_DO_FUN(	do_tell		);
DECLARE_DO_FUN(	do_time		);
DECLARE_DO_FUN(	do_title	);
DECLARE_DO_FUN(	do_train	);
DECLARE_DO_FUN(	do_transfer	);
DECLARE_DO_FUN( do_trip		);
DECLARE_DO_FUN( do_trouble      );
DECLARE_DO_FUN( do_whirlwind    );
DECLARE_DO_FUN( do_touch	);
DECLARE_DO_FUN( do_hamstring    );
DECLARE_DO_FUN(	do_trust	);
DECLARE_DO_FUN(	do_typo		);
DECLARE_DO_FUN( do_undeny       );
DECLARE_DO_FUN(	do_unlock	);
DECLARE_DO_FUN( do_unread	);
DECLARE_DO_FUN(	do_up		);
DECLARE_DO_FUN( do_unshift	);
DECLARE_DO_FUN(	do_value	);
DECLARE_DO_FUN( do_vital_strike );
DECLARE_DO_FUN(	do_visible	);
DECLARE_DO_FUN( do_violate	);
DECLARE_DO_FUN( do_vnum		);
DECLARE_DO_FUN(	do_wake		);
DECLARE_DO_FUN(	do_wear		);
DECLARE_DO_FUN(	do_weather	);
DECLARE_DO_FUN(	do_west		);
DECLARE_DO_FUN(	do_where	);
DECLARE_DO_FUN(	do_who		);
DECLARE_DO_FUN( do_whois	);
DECLARE_DO_FUN(	do_wimpy	);
DECLARE_DO_FUN(	do_wizhelp	);
DECLARE_DO_FUN(	do_wizlock	);
DECLARE_DO_FUN( do_wizlist	);
DECLARE_DO_FUN( do_wiznet	);
DECLARE_DO_FUN( do_worth	);
DECLARE_DO_FUN( do_clear 	);
DECLARE_DO_FUN(	do_yell		);
DECLARE_DO_FUN(	do_zap		);
DECLARE_DO_FUN( do_zecho	);
DECLARE_DO_FUN( do_olc		);
DECLARE_DO_FUN( do_asave	);
DECLARE_DO_FUN( do_alist	);
DECLARE_DO_FUN( do_resets	);
DECLARE_DO_FUN( do_objresets    );
DECLARE_DO_FUN( do_mobresets    );
DECLARE_DO_FUN( do_redit	);
DECLARE_DO_FUN( do_aedit	);
DECLARE_DO_FUN( do_medit	);
DECLARE_DO_FUN( do_oedit	);
DECLARE_DO_FUN( do_mpedit	);
DECLARE_DO_FUN( do_deposit); /* GBS commands */
DECLARE_DO_FUN( do_withdraw);
DECLARE_DO_FUN( do_account);
DECLARE_DO_FUN( do_interest);
DECLARE_DO_FUN( do_change);
DECLARE_DO_FUN( do_shadow_form);
DECLARE_DO_FUN( do_inspire);
DECLARE_DO_FUN( do_soothe);
DECLARE_DO_FUN( do_heightened_senses);
DECLARE_DO_FUN( do_pstop	);
DECLARE_DO_FUN( do_pmusic	);
DECLARE_DO_FUN( do_volume	);
DECLARE_DO_FUN( do_pueblo	);
DECLARE_DO_FUN( do_guildlist    );
DECLARE_DO_FUN( do_maxstat );
DECLARE_DO_FUN( do_brawlerecho  );
/* TRADE SKILL COMMANDS */
DECLARE_DO_FUN( do_bake   	);
DECLARE_DO_FUN( do_tailor   	);
DECLARE_DO_FUN( do_trades       );
DECLARE_DO_FUN( do_fletch       );

DECLARE_DO_FUN( do_sedit        );
DECLARE_DO_FUN( do_pause        );
DECLARE_DO_FUN( do_throwdice    );

/* gkl commands */
DECLARE_DO_FUN( do_screwtime    );
DECLARE_DO_FUN( do_whomulti     );
DECLARE_DO_FUN( do_wizstat      );
DECLARE_DO_FUN( do_fgive        );
DECLARE_DO_FUN( do_info         );
DECLARE_DO_FUN( do_mcast        );
DECLARE_DO_FUN( do_showhp       );
DECLARE_DO_FUN( do_findkey      );
DECLARE_DO_FUN( do_notetag      );
DECLARE_DO_FUN( do_otell        );
DECLARE_DO_FUN( do_immreply       );
DECLARE_DO_FUN( do_oreply       );
DECLARE_DO_FUN( do_mprecogon    );
DECLARE_DO_FUN( do_mprecogoff   );
DECLARE_DO_FUN( do_spidey       );
DECLARE_DO_FUN( do_subdue       );
DECLARE_DO_FUN( do_qecho        );
DECLARE_DO_FUN( do_nuke         );
DECLARE_DO_FUN( do_config       );
DECLARE_DO_FUN( do_fakeslay     );
DECLARE_DO_FUN( do_dumpwho      );
DECLARE_DO_FUN( do_purify       );
DECLARE_DO_FUN( do_solemn       );
DECLARE_DO_FUN( do_lay_on_hands );
DECLARE_DO_FUN( do_journal      );
DECLARE_DO_FUN( do_plant        );
DECLARE_DO_FUN( do_bankaccts    );
DECLARE_DO_FUN( do_modacct      );
DECLARE_DO_FUN( do_gwithdraw    );
DECLARE_DO_FUN( do_gaccount     );
DECLARE_DO_FUN( do_gdeposit     );
DECLARE_DO_FUN( do_famdesc      );
DECLARE_DO_FUN( do_famsex       );
DECLARE_DO_FUN( do_unstring     );
DECLARE_DO_FUN(	do_scast        );
DECLARE_DO_FUN( do_harvest      );
DECLARE_DO_FUN( do_screwtick    );
DECLARE_DO_FUN( do_sanguine_bond);
DECLARE_DO_FUN( do_ftrans       );
DECLARE_DO_FUN( do_tardis       );
DECLARE_DO_FUN( do_transin      );
DECLARE_DO_FUN( do_transout     );
/*seraph stuff*/
DECLARE_DO_FUN( do_makeseraph     );
DECLARE_DO_FUN( do_lifeenergy     );
DECLARE_DO_FUN( do_project     );
/* Brawler commands */
DECLARE_DO_FUN( do_scoreboard   );
DECLARE_DO_FUN( do_victory      );
DECLARE_DO_FUN( do_brawllist    );
DECLARE_DO_FUN( do_brawlerkick  );
DECLARE_DO_FUN( do_leavebrawle  );
DECLARE_DO_FUN( do_leavebrawler );
DECLARE_DO_FUN( do_clobber      );
DECLARE_DO_FUN(do_quadruple);
DECLARE_DO_FUN(do_announcement);

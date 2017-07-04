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

#ifndef __TABLES_H
#define __TABLES_H

#include "merc.h"



struct flag_type
{
    char *name;
    int bit;
    bool settable;
};

struct idscale_type
{
    int max;
    char *string;
};

struct guild_type
{
  char 	*name;
  char 	*who_name;
};

struct position_type
{
    char *name;
    char *short_name;
};

struct sex_type
{
    char *name;
};

struct size_type
{
    char *name;
};

struct	bit_type
{
	const	struct	flag_type *	table;
	char *				help;
};

/* game tables */
extern  const   struct  flag_type       quest_flags[];
extern  const   struct  flag_type       quest_flags_verbose[];
extern  const   struct  flag_type       limit_flags[];
extern			struct	guild_type		guild_table[MAX_GUILD];
extern	const	struct	position_type	position_table[];
extern	const	struct	sex_type		sex_table[];
extern	const	struct	size_type		size_table[];

/* 
 * flag tables 
 *
 * When adding new flag_type tables, be sure to also modify flag_stat_table
 * in bit.c or else flag_string may not work very well on the new table
 *                                                              5/13/2011 gkl
 */
extern	const	struct	flag_type	qtype_flags[];
extern	const	struct	flag_type	mprecog_flags[];
extern	const	struct	flag_type	act_flags[];
extern	const	struct	flag_type	plr_flags[];
extern	const	struct	flag_type	affect_flags[];
extern	const	struct	flag_type	affect2_flags[];
extern	const	struct	flag_type	affect3_flags[];
extern	const	struct	flag_type	off_flags[];
extern	const	struct	flag_type	archery_flags[];
extern	const	struct	flag_type	imm_flags[];
extern	const	struct	flag_type	form_flags[];
extern	const	struct	flag_type	part_flags[];
extern	const	struct	flag_type	comm_flags[];
extern	const	struct	flag_type	extra_flags[];
extern	const	struct	flag_type	wear_flags[];
extern	const	struct	flag_type	weapon_flags[];
extern	const	struct	flag_type	container_flags[];
extern	const	struct	flag_type	portal_flags[];
extern	const	struct	flag_type	room_flags[];
extern  const   struct  flag_type       trans_flags[];
extern	const	struct	flag_type	exit_flags[];
extern 	const	struct  flag_type	mprog_flags[];
extern	const	struct	flag_type	area_flags[];
extern	const	struct	flag_type	sector_flags[];
extern	const	struct	flag_type	ic_sector_flags[];
extern	const	struct	flag_type	door_resets[];
extern	const	struct	flag_type	wear_loc_strings[];
extern	const	struct	flag_type	wear_loc_flags[];
extern	const	struct	flag_type	res_flags[];
extern	const	struct	flag_type	imm_flags[];
extern	const	struct	flag_type	vuln_flags[];
extern	const	struct	flag_type	type_flags[];
extern	const	struct	flag_type	apply_flags[];
extern	const	struct	flag_type	sex_flags[];
extern	const	struct	flag_type	furniture_flags[];
extern	const	struct	flag_type	weapon_class[];
extern	const	struct	flag_type	apply_types[];
extern	const	struct	flag_type	weapon_type2[];
extern	const	struct	flag_type	apply_types[];
extern	const	struct	flag_type	size_flags[];
extern	const	struct	flag_type	position_flags[];
extern	const	struct	flag_type	ac_type[];
extern	const	struct	bit_type	bitvector_type[];
/* identify tables */
extern  const   struct  idscale_type    weight_scale[];
extern  const   struct  idscale_type    value_scale[];
extern  const   struct  idscale_type    level_scale[];
extern  const   struct  idscale_type    splevel_scale[];
extern  const   struct  idscale_type    contnum_scale[];
extern  const   struct  idscale_type    contwgt_scale[];
extern  const   struct  idscale_type    contmlt_scale[];
extern  const   struct  idscale_type    weapdam_scale[];
extern  const   struct  idscale_type    armor_scale  [];
extern  const   struct  idscale_type    statapply_scale[];
extern  const   struct  idscale_type    hpmpapply_scale[];
extern  const   struct  flag_type       ic_extra_flags[];
extern	const	struct	flag_type	ic_container_flags[];
extern  const   struct  flag_type       ic_type_flags[];
extern	const	struct	flag_type	ic_apply_flags[];
extern	const	struct	flag_type	ic_weapon_type2[];
#endif

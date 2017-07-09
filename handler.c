/**************************************************************************r
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
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN(do_return	);

/* ISO string.h does not include strsep on Solaris */
#if defined(__SVR4) && defined(__sun)
char* strsep(char** str, const char* delims)
{
    char* token;

    if (*str==NULL) {
        return NULL;
    }

    token=*str;
    while (**str!='\0') {
        if (strchr(delims,**str)!=NULL) {
            **str='\0';
            (*str)++;
            return token;
        }
        (*str)++;
    }
    *str=NULL;
    return token;
}
#endif

/*
 * Local functions.
 */
void	affect_modify	args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
);

char   *flag_string     args ( ( const struct flag_type *flag_table,
                                         int bits ) );

/* 
 * Strips color codes out of a string really fast.  Also strips
 * {{, {-, {*, and anything else, for that matter. 
 */
char *strip_color( const char *str )
{
    static char strfix[MAX_STRING_LENGTH * 2];
	strcpy(strfix, str);
    return strfix;
}

/* 
 * Escapes color codes
 */
char *escape_color( const char *str )
{
    static char strfix[MAX_STRING_LENGTH * 2];
    int oldstr = 0, newstr = 0;

    while ( str[oldstr] != '\0' && oldstr < MAX_STRING_LENGTH*2 )
    {
        if ( str[oldstr] == '{' )
            strfix[newstr++] = '{';
        strfix[newstr++] = str[oldstr++];
    }
    strfix[newstr] = '\0';
    return strfix;
}

/* Removes \n and \r from strings.  Useful for printing time data */
char *smash_crlf( const char *str )
{
    static char strfix[MAX_STRING_LENGTH * 2];
    int oldstr = 0, newstr = 0;

    while ( str[oldstr] != '\0' )
    {
        if ( str[oldstr] == '\n' || str[oldstr] == '\r' )
            oldstr++;
        else
        {
            strfix[newstr] = str[oldstr];
            newstr++;
        }
        oldstr++;
    }
    strfix[newstr] = '\0';
    return strfix;
}

/* friend stuff -- for NPC's mostly */
bool is_friend(CHAR_DATA *ch,CHAR_DATA *victim)
{
    if (is_same_group(ch,victim))
	return TRUE;

    
    if (!IS_NPC(ch))
	return FALSE;

    if (!IS_NPC(victim))
    {
	if (IS_SET(ch->off_flags,ASSIST_PLAYERS))
	    return TRUE;
	else
	    return FALSE;
    }

    if (IS_AFFECTED(ch,AFF_CHARM))
	return FALSE;

    if (IS_SET(ch->off_flags,ASSIST_ALL))
	return TRUE;

    if (ch->group && ch->group == victim->group)
	return TRUE;

    if (IS_SET(ch->off_flags,ASSIST_VNUM) 
    &&  ch->pIndexData == victim->pIndexData)
	return TRUE;

    if (IS_SET(ch->off_flags,ASSIST_RACE) && ch->race == victim->race)
	return TRUE;
     
    return FALSE;
}

/* returns number of people on an object */
int count_users(OBJ_DATA *obj)
{
    CHAR_DATA *fch;
    int count = 0;

    if (obj->in_room == NULL)
	return 0;

    for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
	if (fch->on == obj)
	    count++;

    return count;
}
     
/* returns material number */
int material_lookup (const char *name)
{
    return 0;
}

/* returns god number */
int god_lookup (const char *name)
{
    int god;
 
    for ( god = 0; god_table[god].name != NULL; god++)
    {
         if (LOWER(name[0]) == LOWER(god_table[god].name[0])
         &&  !str_prefix( name,god_table[god].name))
             return god;
    }
 
    return -1;
}
 
int weapon_lookup (const char *name)
{
    int type;

    for (type = 0; weapon_table[type].name != NULL; type++)
    {
	if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
	&&  !str_prefix(name,weapon_table[type].name))
	    return type;
    }
 
    return -1;
}

int weapon_type (const char *name)
{
    int type;
 
    for (type = 0; weapon_table[type].name != NULL; type++)
    {
        if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
        &&  !str_prefix(name,weapon_table[type].name))
            return weapon_table[type].type;
    }
 
    return WEAPON_EXOTIC;
}

char *item_name(int item_type)
{
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
	if (item_type == item_table[type].type)
	    return item_table[type].name;
    return "none";
}

char *weapon_name( int weapon_type)
{
    int type;
 
    for (type = 0; weapon_table[type].name != NULL; type++)
        if (weapon_type == weapon_table[type].type)
            return weapon_table[type].name;
    return "exotic";
}

int attack_lookup  (const char *name)
{
  int att;

  for ( att = 0; attack_table[att].name != NULL; att++)
    {
      if (LOWER(name[0]) == LOWER(attack_table[att].name[0])
	  &&  !str_prefix(name,attack_table[att].name))
	return att;
    }

  return 0;

}

/* returns a flag for wiznet */
long wiznet_lookup (const char *name)
{
    int flag;

    for (flag = 0; wiznet_table[flag].name != NULL; flag++)
    {
	if (LOWER(name[0]) == LOWER(wiznet_table[flag].name[0])
	&& !str_prefix(name,wiznet_table[flag].name))
	    return flag;
    }

    return -1;
}

/* returns class number */
int class_lookup (const char *name)
{
   int class;
 
   for ( class = 0; class < MAX_CLASS; class++)
   {
        if (LOWER(name[0]) == LOWER(class_table[class].name[0])
        &&  !str_prefix( name,class_table[class].name))
            return class;
   }
 
   return -1;
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */

int dam_to_imm(int dam_type)
{
    switch (dam_type)
    {
        case(DAM_LIGHT):        return IMM_LIGHT;
        case(DAM_IRON):         return IMM_IRON; 
        case(DAM_BASH):	        return IMM_BASH; 
        case(DAM_PIERCE):       return IMM_PIERCE;
        case(DAM_SLASH):        return IMM_SLASH;
        case(DAM_FIRE):	        return IMM_FIRE;
        case(DAM_COLD):	        return IMM_COLD;
        case(DAM_LIGHTNING):    return IMM_LIGHTNING;
        case(DAM_ACID):	        return IMM_ACID;
        case(DAM_POISON):       return IMM_POISON;
        case(DAM_NEGATIVE):     return IMM_NEGATIVE;
        case(DAM_HOLY):	        return IMM_HOLY;
        case(DAM_ENERGY):       return IMM_ENERGY;
        case(DAM_MENTAL):       return IMM_MENTAL;
        case(DAM_DISEASE):      return IMM_DISEASE;
        case(DAM_DROWNING):     return IMM_DROWNING;
        case(DAM_CHARM):        return IMM_CHARM;
        case(DAM_SOUND):        return IMM_SOUND;
        case(DAM_SILVER):       return IMM_SILVER;
        case(DAM_WIND):         return IMM_WIND;
    }
    return 0;
}

int check_immune(CHAR_DATA *ch, int dam_type)
{
    int immune, def;
    int bit;

    immune = -1;
    def = IS_NORMAL;

    if (dam_type == DAM_NONE)
	return immune;

    if (dam_type <= 3)
    {
	if (IS_SET(ch->imm_flags,IMM_WEAPON))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_WEAPON))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_WEAPON))
	    def = IS_VULNERABLE;
    }
    else /* magical attack */
    {	
	if (IS_SET(ch->imm_flags,IMM_MAGIC))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_MAGIC))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_MAGIC))
	    def = IS_VULNERABLE;
    }

    /* set bits to check -- VULN etc. must ALL be the same or this will fail */
    bit = dam_to_imm( dam_type );
    if ( bit == 0 ) { return def; }

    if (IS_SET(ch->imm_flags,bit))
	immune = IS_IMMUNE;
    else if (IS_SET(ch->res_flags,bit) && immune != IS_IMMUNE)
	immune = IS_RESISTANT;
    else if (IS_SET(ch->vuln_flags,bit))
    {
	if (immune == IS_IMMUNE)
	    immune = IS_RESISTANT;
	else if (immune == IS_RESISTANT)
	    immune = IS_NORMAL;
	else
	    immune = IS_VULNERABLE;
    }

    if (immune == -1)
	return def;
    else
      	return immune;
}

bool is_guild(CHAR_DATA *ch)
{
    return ch->guild;
}

bool is_same_guild(CHAR_DATA *ch, CHAR_DATA *victim)
{
  return( ch->guild == victim->guild );
}

/* checks mob format */
bool is_old_mob(CHAR_DATA *ch)
{
    if (ch->pIndexData == NULL)
	return FALSE;
    else if (ch->pIndexData->new_format)
	return FALSE;
    return TRUE;
}
 
/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
{
    int skill;

    if (sn == -1) /* shorthand for level based skills */
    {
	skill = ch->level * 5 / 2;
    }

    else if (sn < -1 || sn > MAX_SKILL)
    {
	bug("Bad sn %d in get_skill.",sn);
	skill = 0;
    }

    else if (!IS_NPC(ch))
    {
	if (ch->level < skill_table[sn].skill_level[ch->class])
        {
            if (ch->level >=  skill_table[sn].were_level[ch->race])
                skill = ch->pcdata->learned[sn];
            else 
                skill = 0;
        }
        else
            skill = ch->pcdata->learned[sn];
    }

    else /* mobiles */
    {
        /* most commonly accessed skills first - unarmed combat */
	if (sn == gsn_hand_to_hand)
	    skill = 40 + 2 * ch->level;

        /* dodge/parry */
        else if ((sn == gsn_dodge && IS_SET(ch->off_flags,OFF_DODGE))
 	||       (sn == gsn_parry && IS_SET(ch->off_flags,OFF_PARRY)))
	    skill = ch->level * 2;

        /* armed combat */
	else if (sn == gsn_sword
	||  sn == gsn_dagger
	||  sn == gsn_spear
	||  sn == gsn_mace
	||  sn == gsn_axe
	||  sn == gsn_flail
	||  sn == gsn_whip
	||  sn == gsn_polearm
	||  sn == gsn_staff
        ||  sn == gsn_hand_to_hand )
	    skill = 40 + 5 * ch->level / 2;

	else if (sn == gsn_second_attack 
	&& (IS_SET(ch->act_bits,ACT_WARRIOR) || IS_SET(ch->act_bits,ACT_THIEF)))
	    skill = 10 + 3 * ch->level;

	else if (sn == gsn_third_attack && IS_SET(ch->act_bits,ACT_WARRIOR))
	    skill = 4 * ch->level - 40;

 	else if (sn == gsn_trip && IS_SET(ch->off_flags,OFF_TRIP))
	    skill = 10 + 3 * ch->level;

 	else if (sn == gsn_bash && IS_SET(ch->off_flags,OFF_BASH))
	    skill = 10 + 3 * ch->level;

	else if (sn == gsn_disarm 
	     &&  (IS_SET(ch->off_flags,OFF_DISARM) 
	     ||   IS_SET(ch->act_bits,ACT_WARRIOR)
	     ||	  IS_SET(ch->act_bits,ACT_THIEF)))
	    skill = 20 + 3 * ch->level;

	else if (sn == gsn_battle && IS_SET(ch->off_flags,OFF_BERSERK))
	    skill = 3 * ch->level;

	else if (sn == gsn_kick)
	    skill = 10 + 3 * ch->level;

 	else if (sn == gsn_shield_block)
	    skill = 10 + 2 * ch->level;

        /* all spells */
        else if (skill_table[sn].spell_fun != spell_null)
	    skill = 40 + 2 * ch->level;

        /* skills that are only evoked via the 'order' command */
	else if (sn == gsn_sneak || sn == gsn_hide)
	    skill = ch->level * 2 + 20;

	else if (sn == gsn_backstab && IS_SET(ch->act_bits,ACT_THIEF))
	    skill = 20 + 2 * ch->level;

  	else if (sn == gsn_rescue)
	    skill = 40 + ch->level; 

	else if (sn == gsn_recall)
	    skill = 40 + ch->level;

	else if (sn == gsn_pick_lock)
	    skill = 40 + ch->level;

	else 
            skill = 0;
    }

    if (ch->daze > 0)
    {
	if (skill_table[sn].spell_fun != spell_null)
	    skill /= 2;
	else
	    skill = 2 * skill / 3;
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	skill = 9 * skill / 10;

    if ( IS_AFFECTED2(ch,AFF2_FORGET) )
        skill = 9 * skill / 10;

    return URANGE(0,skill,100);
}

/* for returning weapon information */
int get_weapon_sn(CHAR_DATA *ch)
{
    OBJ_DATA *wield;
    int sn;

    wield = get_eq_char( ch, WEAR_WIELD );
    if (wield == NULL || wield->item_type != ITEM_WEAPON)
        sn = gsn_hand_to_hand;
    else switch (wield->value[0])
    {
        default :               sn = -1;                break;
        case(WEAPON_SWORD):     sn = gsn_sword;         break;
        case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
        case(WEAPON_SPEAR):     sn = gsn_spear;         break;
        case(WEAPON_MACE):      sn = gsn_mace;          break;
        case(WEAPON_AXE):       sn = gsn_axe;           break;
        case(WEAPON_FLAIL):     sn = gsn_flail;         break;
        case(WEAPON_WHIP):      sn = gsn_whip;          break;
        case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
        case(WEAPON_STAFF):     sn = gsn_staff;         break;
        case(WEAPON_HAND):      sn = gsn_hand_to_hand;  break;
	case(WEAPON_BOW):	sn = gsn_bow;		break;
   }
   return sn;
}

int get_weapon_skill(CHAR_DATA *ch, int sn)
{
     int skill;

     /* -1 is exotic */
    if (IS_NPC(ch))
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else if (sn == gsn_hand_to_hand)
	    skill = 40 + 2 * ch->level;
	else 
	    skill = 40 + 5 * ch->level / 2;
    }
    
    else
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else
	    skill = ch->pcdata->learned[sn];
    }

    return URANGE(0,skill,100);
} 


/* used to de-screw characters */
void reset_char(CHAR_DATA *ch)
{
     int loc,mod,stat;
     OBJ_DATA *obj;
     AFFECT_DATA *af;
     int i;

     if (IS_NPC(ch))
	return;

    if (ch->pcdata->perm_hit == 0 
    ||	ch->pcdata->perm_mana == 0
    ||  ch->pcdata->perm_move == 0
    ||	ch->pcdata->last_level == 0)
    {
    /* do a FULL reset */
	for (loc = 0; loc < MAX_WEAR; loc++)
	{
	    obj = get_eq_char(ch,loc);
	    if (obj == NULL)
		continue;
	    if (!obj->enchanted)
	    for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
	    {
		mod = af->modifier;
		switch(af->location)
		{
		    case APPLY_SEX:	ch->sex		-= mod;
					if (ch->sex < 0 || ch->sex >2)
					    ch->sex = IS_NPC(ch) ?
						0 :
						ch->pcdata->true_sex;
									break;
		    case APPLY_MANA:	ch->max_mana	-= mod;		break;
		    case APPLY_HIT:	ch->max_hit	-= mod;		break;
		    case APPLY_MOVE:	ch->max_move	-= mod;		break;
                    case APPLY_BLOOD:   ch->max_blood   -= mod;       break;
		}
	    }

            for ( af = obj->affected; af != NULL; af = af->next )
            {
                mod = af->modifier;
                switch(af->location)
                {
                    case APPLY_SEX:     ch->sex         -= mod;         break;
                    case APPLY_MANA:    ch->max_mana    -= mod;         break;
                    case APPLY_HIT:     ch->max_hit     -= mod;         break;
                    case APPLY_MOVE:    ch->max_move    -= mod;         break;
                    case APPLY_BLOOD:   ch->max_blood   -= mod;       break;
                }
            }
	}
	/* now reset the permanent stats */
        ch->pcdata->perm_hit    = ch->max_hit;
	ch->pcdata->perm_mana 	= ch->max_mana;
	ch->pcdata->perm_move	= ch->max_move;
	ch->pcdata->last_level	= ch->played/3600;
	if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
         {
		if (ch->sex > 0 && ch->sex < 3)
	    	    ch->pcdata->true_sex	= ch->sex;
		else
		    ch->pcdata->true_sex 	= 0;
         }

    }

    /* now restore the character to his/her true condition */
    for (stat = 0; stat < MAX_STATS; stat++)
	ch->mod_stat[stat] = 0;

    if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
	ch->pcdata->true_sex = 0; 
    ch->sex		= ch->pcdata->true_sex;
    ch->max_hit 	= ch->pcdata->perm_hit;
    ch->max_mana	= ch->pcdata->perm_mana;
    ch->max_move	= ch->pcdata->perm_move;
   
    for (i = 0; i < 4; i++)
    	ch->armor[i]	= 100;

    ch->hitroll		= 0;
    ch->damroll		= 0;
    ch->saving_throw	= 0;

    /* now start adding back the effects */
    for (loc = 0; loc < MAX_WEAR; loc++)
    {
        obj = get_eq_char(ch,loc);
        if (obj == NULL)
            continue;
	for (i = 0; i < 4; i++)
	    ch->armor[i] -= apply_ac( obj, loc, i );

        if (!obj->enchanted)
	for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
        {
            mod = af->modifier;
            switch(af->location)
            {
		case APPLY_STR:		ch->mod_stat[STAT_STR]	+= mod;	break;
		case APPLY_DEX:		ch->mod_stat[STAT_DEX]	+= mod; break;
		case APPLY_INT:		ch->mod_stat[STAT_INT]	+= mod; break;
		case APPLY_WIS:		ch->mod_stat[STAT_WIS]	+= mod; break;
		case APPLY_CON:		ch->mod_stat[STAT_CON]	+= mod; break;

		case APPLY_SEX:		ch->sex			+= mod; break;
		case APPLY_MANA:	ch->max_mana		+= mod; break;
		case APPLY_HIT:		ch->max_hit		+= mod; break;
		case APPLY_MOVE:	ch->max_move		+= mod; break;
		case APPLY_BLOOD:       ch->max_blood    	+= mod; break;

		case APPLY_AC:		
		    for (i = 0; i < 4; i ++)
			ch->armor[i] += mod; 
		    break;
		case APPLY_HITROLL:	ch->hitroll		+= mod; break;
		case APPLY_DAMROLL:	ch->damroll		+= mod; break;
	
		case APPLY_SAVES:		ch->saving_throw += mod; break;
		case APPLY_SAVING_ROD: 		ch->saving_throw += mod; break;
		case APPLY_SAVING_PETRI:	ch->saving_throw += mod; break;
		case APPLY_SAVING_BREATH: 	ch->saving_throw += mod; break;
		case APPLY_SAVING_SPELL:	ch->saving_throw += mod; break;
                case APPLY_SKILLS:              break;
	    }
        }
 
        for ( af = obj->affected; af != NULL; af = af->next )
        {
            mod = af->modifier;
            switch(af->location)
            {
                case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
                case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
                case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
                case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
 
                case APPLY_SEX:         ch->sex                 += mod; break;
                case APPLY_MANA:        ch->max_mana            += mod; break;
                case APPLY_HIT:         ch->max_hit             += mod; break;
                case APPLY_MOVE:        ch->max_move            += mod; break;
		case APPLY_BLOOD:       ch->max_blood    	+= mod; break;
 
                case APPLY_AC:
                    for (i = 0; i < 4; i ++)
                        ch->armor[i] += mod;
                    break;
		case APPLY_HITROLL:     ch->hitroll             += mod; break;
                case APPLY_DAMROLL:     ch->damroll             += mod; break;
 
                case APPLY_SAVES:               ch->saving_throw += mod; break;
                case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
                case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
                case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
                case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
                case APPLY_SKILLS:              break;
            }
	}
    }
  
    /* now add back spell effects */
    for (af = ch->affected; af != NULL; af = af->next)
    {
        mod = af->modifier;
        switch(af->location)
        {
                case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
                case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
                case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
                case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
 
                case APPLY_SEX:         ch->sex                 += mod; break;
                case APPLY_MANA:        ch->max_mana            += mod; break;
                case APPLY_HIT:         ch->max_hit             += mod; break;
                case APPLY_MOVE:        ch->max_move            += mod; break;
		case APPLY_BLOOD:       ch->max_blood    	+= mod; break;
 
                case APPLY_AC:
                    for (i = 0; i < 4; i ++)
                        ch->armor[i] += mod;
                    break;
                case APPLY_HITROLL:     ch->hitroll             += mod; break;
                case APPLY_DAMROLL:     ch->damroll             += mod; break;
 
                case APPLY_SAVES:               ch->saving_throw += mod; break;
                case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
                case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
                case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
                case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
                case APPLY_SKILLS:              break;
        } 
    }

    if (IS_SET(ch->shiftbits, SHIFT_FULL) ||
	IS_SET(ch->shiftbits, SHIFT_HALF)) {
	  ch->max_hit += were_table[ch->race].hpadd;
	  ch->max_mana += were_table[ch->race].manaadd;
    }


    if( IS_SET(ch->shiftbits, PERM_VAMP) ||
	IS_SET(ch->shiftbits, TEMP_VAMP) ) 
      {
	if( IS_SET( ch->shiftbits, BAT_VAMP ) )
	  {
	    ch->max_hit += 50;
	    ch->max_mana += 100;
	  }
	
	if( IS_SET( ch->shiftbits, MIST_VAMP ) )
	  {
	    ch->max_hit += 100;
	    ch->max_mana += 50;
	  }
	
	if (time_info.hour > 5 && time_info.hour < 20)
	   vampire_unenhance( ch );
        else
	   vampire_enhance( ch );
      }

	
    /* make sure sex is RIGHT!!!! */
    if (ch->sex < 0 || ch->sex > 2)
	ch->sex = ch->pcdata->true_sex;
}


/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch )
{
    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    if (ch->trust)
	return ch->trust;

    if ( IS_NPC(ch) && ch->level >= LEVEL_HERO )
	return LEVEL_HERO - 1;
    else
	return ch->level;
}


/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch )
{
    return 17 + ( ch->played + (int) (current_time - ch->logon) ) / 72000;
}

/* command for retrieving stats */
int get_curr_stat( CHAR_DATA *ch, int stat )
{
    int max;

    if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
      {
	max = MAX_STAT_LIMIT;
      }
    else if (ch->class == class_lookup("avatar"))
      {
        max = pc_race_table[ch->race].max_stats[stat] +6;

        max = UMIN(max,MAX_STAT_LIMIT);
      }

    else
      {
	max = pc_race_table[ch->race].max_stats[stat] + 4;
	
	if (class_table[ch->class].attr_prime == stat)
	    max += 2;

	if (class_table[ch->class].attr_sec == stat)
	    max += 1;

	if ( ch->race == race_lookup("human"))
	    max += 1;

 	max = UMIN(max,MAX_STAT_LIMIT);
      }
  
    return URANGE(3,ch->perm_stat[stat] + ch->mod_stat[stat], max);
}

/* command for returning max training score */
int get_max_train( CHAR_DATA *ch, int stat )
{
    int max;

    if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
	return MAX_STAT_LIMIT;

    max = pc_race_table[ch->race].max_stats[stat];
    if (class_table[ch->class].attr_prime == stat)
     {
	if (ch->race == race_lookup("human"))
	   max += 3;
	else
	   max += 2;
     }

    if (class_table[ch->class].attr_sec == stat)
	   max += 1;

    return UMIN(max,MAX_STAT_LIMIT);
}
   
	
/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 2000;

    if ( IS_NPC(ch) 
    && ( IS_SET(ch->act_bits, ACT_PET) || IS_SET(ch->act_bits, ACT_NOGIVE) ) )
	return 0;

    return MAX_WEAR +  2 * get_curr_stat(ch,STAT_DEX) + ch->level;
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 10000000;

    if ( IS_NPC(ch) 
    && ( IS_SET(ch->act_bits, ACT_PET) || IS_SET(ch->act_bits,ACT_NOGIVE) ) )
	return 0;

    return str_app[get_curr_stat(ch,STAT_STR)].carry * 10 + ch->level * 25;
}


bool is_abbrev( const char* str, const char* namelist )
{
  char* listcpysafe = 0;
  char* listcpy = 0;
  char* listptr = 0;
  int abblen;
  int i;
  bool succeed;

  if( str == NULL || namelist == NULL )
    return FALSE;

  listcpy = strdup( namelist );
  listcpysafe = listcpy;
  abblen = strlen( str );
  
  for( listptr = strsep( &listcpy, " " );
       listptr != NULL;
       listptr = strsep( &listcpy, " " ) )
    {
      succeed = TRUE;

      for( i = 0; i < abblen; i++ )
	{
	  if( tolower( str[ i ] ) != tolower( listptr[ i ] ) )
	    {
	      succeed = FALSE;
	      break;
	    }
	}

      if( succeed )
	{
	  free( listcpysafe );
	  return TRUE;
	}
    }

  free( listcpysafe );
  return FALSE;
}
    

/*
 * See if a string is one of the names of an object.
 */

bool is_name ( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;

    /* fix crash on NULL namelist */
    if (namelist == NULL || namelist[0] == '\0')
    	return FALSE;

    /* fixed to prevent is_name on "" returning TRUE */
    if ( str == NULL || str[0] == '\0')
	return FALSE;

    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
	str = one_argument(str,part);

	if (part[0] == '\0' )
	    return TRUE;

	/* check to see if this is part of namelist */
	list = namelist;
	for ( ; ; )  /* start parsing namelist */
	{
	    list = one_argument(list,name);
	    if (name[0] == '\0')  /* this name was not found */
		return FALSE;

	    if (!str_cmp(string,name))
		return TRUE; /* full pattern match */

	    if (!str_cmp(part,name))
		break;

	}
    }
}

bool is_exact_name(char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    if (namelist == NULL)
	return FALSE;

    for ( ; ; )
    {
	namelist = one_argument( namelist, name );
	if ( name[0] == '\0' )
	    return FALSE;
	if ( !str_cmp( str, name ) )
	    return TRUE;
    }
}

/* enchanted stuff for eq */
void affect_enchant(OBJ_DATA *obj)
{
    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
        AFFECT_DATA *paf, *af_new;
        obj->enchanted = TRUE;

        for (paf = obj->pIndexData->affected;
             paf != NULL; paf = paf->next)
        {
	    af_new = new_affect();

            af_new->next = obj->affected;
            obj->affected = af_new;
 
	    af_new->where	= paf->where;
            af_new->type        = UMAX(0,paf->type);
            af_new->level       = paf->level;
            af_new->duration    = paf->duration;
            af_new->location    = paf->location;
            af_new->modifier    = paf->modifier;
            af_new->bitvector   = paf->bitvector;
        }
    }
}
           

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    int mod,i;

    mod = paf->modifier;

    if ( fAdd )
    {
	switch (paf->where)
	{
	case TO_AFFECTS:
		if (paf->whichaff == AFF1)
	    	SET_BIT(ch->affected_by, paf->bitvector);
		if (paf->whichaff == AFF2)
	    	SET_BIT(ch->affected_by2, paf->bitvector);
		if (paf->whichaff == AFF3)
	    	SET_BIT(ch->affected_by3, paf->bitvector);
	    break;
	case TO_IMMUNE:
	    SET_BIT(ch->imm_flags,paf->bitvector);
	    break;
	case TO_RESIST:
	    SET_BIT(ch->res_flags,paf->bitvector);
	    break;
	case TO_VULN:
	    SET_BIT(ch->vuln_flags,paf->bitvector);
	    break;
	}
    }
    else
    {
        switch (paf->where)
        {
        case TO_AFFECTS:
			if (paf->whichaff == AFF1)
            	REMOVE_BIT(ch->affected_by, paf->bitvector);
			if (paf->whichaff == AFF2)
            	REMOVE_BIT(ch->affected_by2, paf->bitvector);
			if (paf->whichaff == AFF3)
            	REMOVE_BIT(ch->affected_by3, paf->bitvector);
            break;
        case TO_IMMUNE:
            REMOVE_BIT(ch->imm_flags,paf->bitvector);
            break;
        case TO_RESIST:
            REMOVE_BIT(ch->res_flags,paf->bitvector);
            break;
        case TO_VULN:
            REMOVE_BIT(ch->vuln_flags,paf->bitvector);
            break;
        }
	mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
	bug( "Affect_modify: unknown location %d.", paf->location );
	return;

    case APPLY_NONE:						break;
    case APPLY_STR:           ch->mod_stat[STAT_STR]	+= mod;	break;
    case APPLY_DEX:           ch->mod_stat[STAT_DEX]	+= mod;	break;
    case APPLY_INT:           ch->mod_stat[STAT_INT]	+= mod;	break;
    case APPLY_WIS:           ch->mod_stat[STAT_WIS]	+= mod;	break;
    case APPLY_CON:           ch->mod_stat[STAT_CON]	+= mod;	break;
    case APPLY_SEX:           ch->sex			+= mod;	break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:						break;
    case APPLY_AGE:						break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_MANA:          ch->max_mana		+= mod;	break;
    case APPLY_HIT:           ch->max_hit		+= mod;	break;
    case APPLY_MOVE:          ch->max_move		+= mod;	break;
    case APPLY_BLOOD:         ch->max_blood    	        += mod; break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;
    case APPLY_AC:
        for (i = 0; i < 4; i ++)
            ch->armor[i] += mod;
        break;
    case APPLY_HITROLL:       ch->hitroll		+= mod;	break;
    case APPLY_DAMROLL:       ch->damroll		+= mod;	break;
    case APPLY_SAVES:         ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_ROD:    ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_PETRI:  ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_BREATH: ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_SPELL:  ch->saving_throw		+= mod;	break;
    case APPLY_SPELL_AFFECT:  					break;
    case APPLY_SKILLS:                                          break;
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( !IS_NPC(ch) 
    &&   !player_quitting 
    &&   ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
    &&   get_obj_weight(wield) > (str_app[get_curr_stat(ch,STAT_STR)].wield*10))
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act( "You drop $p.", ch, wield, NULL, TO_CHAR );
	    act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
	    obj_from_char( wield );
	    obj_to_room( wield, ch->in_room );
	    depth--;
	}
    }

    if ( !IS_NPC(ch) 
    &&   !player_quitting 
    &&   ( wield = get_eq_char( ch, WEAR_HOLD ) ) != NULL
    &&   wield->item_type == ITEM_WEAPON
    &&   get_obj_weight(wield) > (str_app[get_curr_stat(ch,STAT_STR)].wield*10))
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act( "You drop $p.", ch, wield, NULL, TO_CHAR );
	    act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
	    obj_from_char( wield );
	    obj_to_room( wield, ch->in_room );
	    depth--;
	}
    }

    return;
}


/* find an effect in an affect list */
AFFECT_DATA  *affect_find(AFFECT_DATA *paf, int sn)
{
    AFFECT_DATA *paf_find;
    
    for ( paf_find = paf; paf_find != NULL; paf_find = paf_find->next )
    {
        if ( paf_find->type == sn )
	return paf_find;
    }

    return NULL;
}

/* fix object affects when removing one */
void affect_check(CHAR_DATA *ch,int where,int vector)
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    if (where == TO_OBJECT || where == TO_WEAPON || vector == 0)
	return;

    for (paf = ch->affected; paf != NULL; paf = paf->next)
	if (paf->where == where && paf->bitvector == vector)
	{
	    switch (where)
	    {
	        case TO_AFFECTS:
				if (paf->whichaff == AFF1)
		    		SET_BIT(ch->affected_by,vector);
				if (paf->whichaff == AFF2)
		    		SET_BIT(ch->affected_by2,vector);
				if (paf->whichaff == AFF3)
		    		SET_BIT(ch->affected_by3,vector);
		    break;
	        case TO_IMMUNE:
		    SET_BIT(ch->imm_flags,vector);   
		    break;
	        case TO_RESIST:
		    SET_BIT(ch->res_flags,vector);
		    break;
	        case TO_VULN:
		    SET_BIT(ch->vuln_flags,vector);
		    break;
	    }
	    return;
	}

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
	if (obj->wear_loc == -1)
	    continue;

            for (paf = obj->affected; paf != NULL; paf = paf->next)
            if (paf->where == where && paf->bitvector == vector)
            {
                switch (where)
                {
                    case TO_AFFECTS:
						if (paf->whichaff == AFF1)
                        	SET_BIT(ch->affected_by,vector);
						if (paf->whichaff == AFF2)
		    				SET_BIT(ch->affected_by2,vector);
						if (paf->whichaff == AFF3)
                        	SET_BIT(ch->affected_by3,vector);

                        break;
                    case TO_IMMUNE:
                        SET_BIT(ch->imm_flags,vector);
                        break;
                    case TO_RESIST:
                        SET_BIT(ch->res_flags,vector);
                        break;
                    case TO_VULN:
                        SET_BIT(ch->vuln_flags,vector);
                  
                }
                return;
            }

        if (obj->enchanted)
	    continue;

        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
            if (paf->where == where && paf->bitvector == vector)
            {
                switch (where)
                {
                    case TO_AFFECTS:
						if (paf->whichaff == AFF1)
                        			SET_BIT(ch->affected_by,vector);
						if (paf->whichaff == AFF2)
		    				SET_BIT(ch->affected_by2,vector);
						if (paf->whichaff == AFF3)
						SET_BIT(ch->affected_by3,vector);
                        break;
                    case TO_IMMUNE:
                        SET_BIT(ch->imm_flags,vector);
                        break;
                    case TO_RESIST:
                        SET_BIT(ch->res_flags,vector);
                        break;
                    case TO_VULN:
                        SET_BIT(ch->vuln_flags,vector);
                        break;
                }
                return;
            }
    }
}

/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;

    VALIDATE(paf_new);  /* the previous line hoses validation from new_affect */
    paf_new->next	= ch->affected;
    paf_new->whichaff	= AFF1;
    ch->affected	= paf_new;

    affect_modify( ch, paf_new, TRUE );

    return;
}

/** ADD THE SECOND AFFECT TO THE CHARACTER **/
void affect_to_char2( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;

    VALIDATE(paf_new);  /* the previous line hoses validation from new_affect */
    paf_new->next	= ch->affected;
    paf_new->whichaff	= AFF2;
    ch->affected	= paf_new;

    affect_modify( ch, paf_new, TRUE );
    return;
}

/** ADD THE THIRD AFFECT TO THE CHARACTER **/
void affect_to_char3( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;

    VALIDATE(paf_new);  /* the previous line hoses validation from new_affect */
    paf_new->next	= ch->affected;
    paf_new->whichaff	= AFF3;
    ch->affected	= paf_new;

    affect_modify( ch, paf_new, TRUE );
    return;
}

/* give an affect to an object */
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;
    paf_new->next	= obj->affected;
    obj->affected	= paf_new;

    /* apply any affect vectors to the object's extra_flags */
    if (paf->bitvector)
        switch (paf->where)
        {
        case TO_OBJECT:
    	    SET_BIT(obj->extra_flags,paf->bitvector);
	    break;
        case TO_WEAPON:
	    if (obj->item_type == ITEM_WEAPON)
	        SET_BIT(obj->value[4],paf->bitvector);
	    break;
        }
    

    return;
}



/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    extern void unshift();
    extern void frog_unshift();
    AFFECT_DATA veil_sick, fatigue;
    int where;
    int vector;

    if ( ( paf->bitvector == AFF_VEIL ) && paf->whichaff == AFF3 )
    {
        veil_sick.where = TO_AFFECTS;
        veil_sick.type = -4;
        veil_sick.level = ch->level;
        veil_sick.duration = 20;
        veil_sick.location = APPLY_NONE;
        veil_sick.modifier = 0;
        veil_sick.bitvector = AFF_VEIL_WAIT;
        affect_to_char3( ch, &veil_sick );
    }
	
 if ( ( paf->bitvector == AFF3_GODSPEED ) && paf->whichaff == AFF3 )
    {
        fatigue.where = TO_AFFECTS;
        fatigue.type = gsn_fatigue;
        fatigue.level = ch->level;
        fatigue.duration = 4;
        fatigue.location = APPLY_NONE;
        fatigue.modifier = 0;
        fatigue.bitvector = AFF3_FATIGUE;
        affect_to_char3( ch, &fatigue );
    }

    if ( ( !IS_NPC( ch ) )
    &&   ( paf->bitvector == AFF_OFFERING )
    &&   ( paf->whichaff == AFF3 )
    &&   ( paf->type == -5 ) )
    {
        if ( ch->pcdata->offering != NULL )
        ch->pcdata->offering = NULL;
    }
    
    if ( ( paf->bitvector == AFF_SHIFT_ON )
    &&   ( paf->whichaff == AFF1 ) )
    {
        switch( paf->type )
	{
            case( -1 ):
                unshift( ch );
                break;
            case( -3 ):
                vampire_unshift( ch );
                break;
            default:
                bug( "Affect_remove: bad aff->type for AFF_SHIFT_ON", 0 );
                break;
	}
    }

    if ( paf->whichaff == AFF3
    &&   paf->bitvector == AFF_FROG )
    {
        frog_unshift( ch, 0 );
    }

    if ( ch->affected == NULL )
    {
        bug( "Affect_remove: no affect.", 0 );
        return;
    }
 
    affect_modify( ch, paf, FALSE );
    where = paf->where;
    vector = paf->bitvector;

    if ( paf == ch->affected )
    {
        ch->affected = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;
	
        for ( prev = ch->affected; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }
	
        if ( prev == NULL )
        {
            bug( "Affect_remove: cannot find paf.", 0 );
            return;
        }
    }

    free_affect(paf);

    affect_check(ch,where,vector);
    return;
}

void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf)
{
    int where, vector;
    if ( obj->affected == NULL )
    {
        bug( "Affect_remove_object: no affect.", 0 );
        return;
    }

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_modify( obj->carried_by, paf, FALSE );

    where = paf->where;
    vector = paf->bitvector;

    /* remove flags from the object if needed */
    if (paf->bitvector)
	switch( paf->where)
        {
        case TO_OBJECT:
            REMOVE_BIT(obj->extra_flags,paf->bitvector);
            break;
        case TO_WEAPON:
            if (obj->item_type == ITEM_WEAPON)
                REMOVE_BIT(obj->value[4],paf->bitvector);
            break;
        }

    if ( paf == obj->affected )
    {
        obj->affected    = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;

        for ( prev = obj->affected; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Affect_remove_object: cannot find paf.", 0 );
            return;
        }
    }

    free_affect(paf);

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_check(obj->carried_by,where,vector);
    return;
}



/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA* paf = NULL;
    AFFECT_DATA* paf_next = NULL;
    bool success = FALSE;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;

	if ( paf->type == sn )
        {
	    affect_remove( ch, paf );
            success = TRUE;
        }
    }

    return;
}



/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Add or enhance an affect.
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf, bool merge )
{
    AFFECT_DATA *paf_old;
    bool found;

    found = FALSE;
    for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    {
	if ( paf_old->type == paf->type )
	{
/*          paf->level = (paf->level += paf_old->level) / 2; */
            paf->level = UMAX(paf->level, paf_old->level);
            if ( merge )
                paf->duration = UMAX(paf->duration, paf_old->duration);
            else
                paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    affect_to_char( ch, paf );
    return;
}

/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ch->in_room == NULL )
    {
	bug( "Char_from_room: NULL.", 0 );
	return;
    }

    if ( !IS_NPC(ch) )
	--ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    if ( ch == ch->in_room->people )
    {
	ch->in_room->people = ch->next_in_room;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
	{
	    if ( prev->next_in_room == ch )
	    {
		prev->next_in_room = ch->next_in_room;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Char_from_room: ch not found.", 0 );
    }

    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    ch->on 	     = NULL;  /* sanity check! */
    return;
}



/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];

    if ( pRoomIndex == NULL )
    {
	ROOM_INDEX_DATA *room;

        sprintf( buf, "char_to_room: NULL pRoomIndex for %s",
            ch->name );
        bug( buf, 0 );
	
	if ((room = get_room_index(ROOM_VNUM_TEMPLE)) != NULL)
	    char_to_room(ch,room);
	
	return;
    }

    ch->in_room		= pRoomIndex;
    ch->next_in_room	= pRoomIndex->people;
    pRoomIndex->people	= ch;

    if ( !IS_NPC(ch) )
    {
	if (ch->in_room->area->empty)
	{
	    ch->in_room->area->empty = FALSE;
	    ch->in_room->area->age = 0;
	}
	++ch->in_room->area->nplayer;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0 ) 
	++ch->in_room->light;

    if ( IS_AFFECTED(ch,AFF_PLAGUE) )
    {
        AFFECT_DATA *af, plague;
        CHAR_DATA *vch;
        
        for ( af = ch->affected; af != NULL; af = af->next )
            if (af->type == gsn_plague)
                break;
        
        if ( af == NULL
        ||   af->level == 1 )
            return;
        
	plague.where		= TO_AFFECTS;
        plague.type 		= gsn_plague;
        plague.level 		= af->level - 1; 
        plague.duration 	= number_range(1,2 * plague.level);
        plague.location		= APPLY_STR;
        plague.modifier 	= -5;
        plague.bitvector 	= AFF_PLAGUE;
        
        for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            if (!saves_spell(plague.level - 2,NULL,vch,DAM_DISEASE) 
	    &&  !IS_IMMORTAL(vch) &&
            	!IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(6) == 0)
            {
            	send_to_char("You feel hot and feverish.\n\r",vch);
            	act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
            	affect_join( vch, &plague, FALSE );
            }
        }
    }


    return;
}



/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    ch->carry_number	+= get_obj_number( obj );
    ch->carry_weight	+= get_obj_weight( obj );

    /* IF THE OBJECT SHOULD TIME DOWN AND ISN'T ALREADY, 
       SET THE TIMER - ARAMINA 8/23/02 */

    if (obj->timer <= 0 && obj->pIndexData->timedown > 0)
        obj->timer = obj->pIndexData->timedown;
/*  So players can't get around nodrop keys by using charmies */
    if ( IS_NPC( ch ) 
    &&  ( obj->item_type == ITEM_KEY || obj->item_type == ITEM_ROOM_KEY ) 
    &&   IS_OBJ_STAT(obj,ITEM_NODROP) )
        obj->timer = 10;

    return;
}

/*
 * This is used to give objects to characters (and mobs)
 * when a mob is first being reset.  It doesn't timer anything.
 */
void obj_to_char_init( OBJ_DATA *obj, CHAR_DATA *ch )
{
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    ch->carry_number	+= get_obj_number( obj );
    ch->carry_weight	+= get_obj_weight( obj );

    return;
}


/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    CHAR_DATA *ch;
    char buf[ MAX_STRING_LENGTH ];

    if ( ( ch = obj->carried_by ) == NULL )
    {
        sprintf(buf,"obj_from_char: null ch (obj=%s)", obj->short_descr );
	bug( buf, 0 );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
	unequip_char( ch, obj );

    if ( ch->carrying == obj )
    {
	ch->carrying = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = ch->carrying; prev != NULL; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Obj_from_char: obj not in list.", 0 );
    }

    obj->carried_by	 = NULL;
    obj->next_content	 = NULL;
    ch->carry_number	-= get_obj_number( obj );
    ch->carry_weight	-= get_obj_weight( obj );
    return;
}



/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear, int type )
{
    if ( obj->item_type != ITEM_ARMOR )
	return 0;

    switch ( iWear )
    {
    case WEAR_BODY:	return 3 * obj->value[type];
    case WEAR_HEAD:	return 2 * obj->value[type];
    case WEAR_LEGS:	return 2 * obj->value[type];
    case WEAR_FEET:	return     obj->value[type];
    case WEAR_HANDS:	return     obj->value[type];
    case WEAR_ARMS:	return     obj->value[type];
    case WEAR_SHIELD:	return     obj->value[type];
    case WEAR_NECK_1:	return     obj->value[type];
    case WEAR_NECK_2:	return     obj->value[type];
    case WEAR_ABOUT:	return 2 * obj->value[type];
    case WEAR_WAIST:	return     obj->value[type];
    case WEAR_WRIST_L:	return     obj->value[type];
    case WEAR_WRIST_R:	return     obj->value[type];
    case WEAR_HOLD:	return     obj->value[type];
    case WEAR_FLOAT:	return     obj->value[type];
    case WEAR_PRIDE:	return     obj->value[type];
    case WEAR_FINGER_L:	return	   obj->value[type];
    case WEAR_FINGER_R: return	   obj->value[type];
    }

    return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == iWear )
	    return obj;
    }

    return NULL;
}



/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    AFFECT_DATA *paf, *aff;
    char buf[MAX_STRING_LENGTH];
    int i;

    if ( get_eq_char( ch, iWear ) != NULL )
    {
        sprintf( buf, "equip_char: %s already equipped in slot %d", ch->name,iWear );
	bug( buf, 0 );
	return;
    }


    for (i = 0; i < 4; i++)
    	ch->armor[i]      	-= apply_ac( obj, iWear,i );
    obj->wear_loc	 = iWear;

    if (!obj->enchanted)
      for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	if ( paf->location != APPLY_SPELL_AFFECT )
	  affect_modify( ch, paf, TRUE );
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
      if ( paf->location == APPLY_SPELL_AFFECT ) {
	/* affect_to_char ( ch, paf ); */
	if (!is_affected( ch, paf->type )) {
	  obj_cast_spell(paf->type, paf->level, ch, ch, obj);
	  /* Find the affect, and make it infinite */
	  for (aff = affect_find(ch->affected, paf->type);
	       aff;
	       aff = affect_find(aff->next, paf->type)) {
	    aff->duration = -1;
	  }
	}
      }
      else
	affect_modify( ch, paf, TRUE );

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL )
	++ch->in_room->light;

    return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf = NULL;
    int i;

    if ( obj->wear_loc == WEAR_NONE )
    {
	bug( "Unequip_char: already unequipped.", 0 );
	return;
    }

    for (i = 0; i < 4; i++)
    	ch->armor[i]	+= apply_ac( obj, obj->wear_loc,i );
    obj->wear_loc	 = -1;

    if (!obj->enchanted)
     {
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	    if ( paf->location == APPLY_SPELL_AFFECT &&
		 is_affected(ch, paf->type))
	    {
	      affect_strip(ch,paf->type);
	      if ( skill_table[paf->type].msg_off )
	      {
		send_to_char( skill_table[paf->type].msg_off, ch );
		send_to_char( "\n\r", ch );
	      }
	    }
	    else
	    {
	        affect_modify( ch, paf, FALSE );
		affect_check(ch,paf->where,paf->bitvector);
	    }
      }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
	if ( paf->location == APPLY_SPELL_AFFECT &&
	     is_affected(ch, paf->type))
	{
	  affect_strip(ch,paf->type);
	  if ( skill_table[paf->type].msg_off )
	  {
	    send_to_char( skill_table[paf->type].msg_off, ch );
	    send_to_char( "\n\r", ch );
	  }

	}
	else
	{
	    affect_modify( ch, paf, FALSE );
	    affect_check(ch,paf->where,paf->bitvector);	
	}

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;
    }

    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
    ROOM_INDEX_DATA *in_room;
    CHAR_DATA *ch;
    char buf[ MAX_STRING_LENGTH ] ;

    if ( ( in_room = obj->in_room ) == NULL )
    {
        sprintf( buf, "obj_from_room: null in_room (obj=%s)", obj->short_descr );
	bug( buf, 0 );
	return;
    }

    for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
	if (ch->on == obj)
	    ch->on = NULL;

    if ( obj == in_room->contents )
    {
	in_room->contents = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = in_room->contents; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_room: obj not found.", 0 );
	    return;
	}
    }

    obj->in_room      = NULL;
    obj->next_content = NULL;
    return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{
    obj->next_content		= pRoomIndex->contents;
    pRoomIndex->contents	= obj;
    obj->in_room		= pRoomIndex;
    obj->carried_by		= NULL;
    obj->in_obj			= NULL;
    return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    obj->next_content		= obj_to->contains;
    obj_to->contains		= obj;
    obj->in_obj			= obj_to;
    obj->in_room		= NULL;
    obj->carried_by		= NULL;
    if (obj_to->pIndexData->vnum == OBJ_VNUM_PIT)
        obj->cost = 0; 

    for ( ; obj_to != NULL; obj_to = obj_to->in_obj )
    {
	if ( obj_to->carried_by != NULL )
	{
	    obj_to->carried_by->carry_number += get_obj_number( obj );
	    obj_to->carried_by->carry_weight += get_obj_weight( obj )
		* WEIGHT_MULT(obj_to) / 100;
	}
    }

    return;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
	bug( "Obj_from_obj: null obj_from.", 0 );
	return;
    }

    if ( obj == obj_from->contains )
    {
	obj_from->contains = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = obj_from->contains; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_obj: obj not found.", 0 );
	    return;
	}
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

    for ( ; obj_from != NULL; obj_from = obj_from->in_obj )
    {
	if ( obj_from->carried_by != NULL )
	{
	    obj_from->carried_by->carry_number -= get_obj_number( obj );
	    obj_from->carried_by->carry_weight -= get_obj_weight( obj ) 
		* WEIGHT_MULT(obj_from) / 100;
	}
    }

    return;
}



/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;

    if ( obj->in_room != NULL )
	obj_from_room( obj );
    else if ( obj->carried_by != NULL )
	obj_from_char( obj );
    else if ( obj->in_obj != NULL )
	obj_from_obj( obj );

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
	obj_next = obj_content->next_content;
	extract_obj( obj_content );
    }

    if ( object_list == obj )
    {
	object_list = obj->next;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = object_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == obj )
	    {
		prev->next = obj->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
	    return;
	}
    }

    --obj->pIndexData->count;
    free_obj(obj);
    return;
}



/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
    CHAR_DATA *wch;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf;

    /* doesn't seem to be necessary*/
    if ( ch->in_room == NULL )
    {
	bug( "Extract_char: NULL.", 0 );
	return;
    }
    if ( !IS_VALID( ch ) )
    {
        bug( "Extract_char: Passed invalid character", 0 );
        return;
    }
    
    nuke_pets(ch);
    ch->pet = NULL; /* just in case */

    if ( fPull )
	die_follower( ch );
    
    stop_fighting( ch, TRUE );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	extract_obj( obj );
    }
    
    if (ch->in_room != NULL)
        char_from_room( ch );

    /* Death room is set in the clan table now */
    if ( !fPull )
    {
        char_to_room(ch,get_room_index(ROOM_VNUM_ALTAR));
	return;
    }

    if ( IS_NPC(ch) )
	--ch->pIndexData->count;

    if ( ch->desc != NULL && ch->desc->original != NULL )
    {
	do_return( ch, "" );
	ch->desc = NULL;
    }

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch->reply == ch )
	    wch->reply = NULL;
        if ( !IS_NPC( wch ) )
            if ( wch->pcdata->offering == ch )
            {
                if ( ( paf = affect_find(wch->affected, -5) ) )
                    affect_remove( wch, paf );
	        wch->pcdata->offering = NULL;
            }
	if ( ch->mprog_target == wch )
	    wch->mprog_target = NULL;
    }

    if ( ch == char_list )
    {
       char_list = ch->next;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = char_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == ch )
	    {
		prev->next = ch->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_char: char not found.", 0 );
	    return;
	}
    }

    if ( ch->desc != NULL )
	ch->desc->character = NULL;
    free_char( ch );
    return;
}



/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument, bool abbrev )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;

    if( !str_cmp( arg, "self" ) )
      return ch;

    if (strlen(arg))
      {
	for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
	  {
	    if( rch->in_room == NULL )
	      continue;

	    if( can_see( ch, rch ) &&
		( is_name( arg, rch->name ) ||
		( IS_NPC( rch ) && abbrev && is_abbrev( arg, rch->name ) ) ) )
	      {
		if( ++count == number )
		  return rch;
	      }
	  }
      }
    else
      {
	for( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
	  {
	    if ( can_see( ch, rch ) 
            && (!IS_NPC(rch) || !IS_SET(rch->act_bits, ACT_NOTSEEN)) )
	      {
		if( ++count == number )
		  return rch;
	      }
	  }
      }

    return NULL;

}

/*
 * Find a char in the room NO MATTER WHAT
 */
CHAR_DATA *get_char_room_seriously( CHAR_DATA *ch, char *argument, bool abbrev )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;

    if ( !str_cmp(arg, "self") )
        return ch;

    if ( strlen(arg) )
    {
        for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
        {
            if( rch->in_room == NULL )
                continue;

            if ( is_name( arg, rch->name ) 
            || ( IS_NPC(rch) && abbrev && is_abbrev(arg, rch->name) ) )
            {
                if ( ++count == number )
                    return rch;
            }
        }
    }
    else
    {
        for( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
        {
            if ( !IS_NPC(rch) || !IS_SET(rch->act_bits, ACT_NOTSEEN) )
            {
                if ( ++count == number )
                    return rch;
            }
        }
    }
    return NULL;
}





/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument, bool abbrev )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *wch;
  int number;
  int count;

  if( strlen( argument ) == 0 )
    return NULL;

  if ( ( wch = get_char_room( ch, argument, abbrev ) ) != NULL )
    return wch;

  number = number_argument( argument, arg );
  count  = 0;

  for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
      if( wch->in_room == NULL )
	continue;

      if( can_see( ch, wch ) &&
	  ( is_name( arg, wch->name ) ||
	  ( IS_NPC( wch ) && abbrev && is_abbrev( arg, wch->name ) ) ) )
	{
	  if( ++count == number )
	    return wch;
	}
    }

  return NULL;
}



OBJ_DATA *get_top_container( OBJ_DATA *pObj )
{
    if ( pObj->in_obj == NULL ) return pObj;
    else return get_top_container ( pObj->in_obj );
}

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex, ROOM_INDEX_DATA *pRoom )
{
    OBJ_DATA *obj;
    OBJ_DATA *tobj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObjIndex )
        {
            tobj = get_top_container( obj );
            if ( tobj->in_room == pRoom )
                return obj;
            if ( tobj->carried_by != NULL
            &&   tobj->carried_by->in_room != NULL
            &&   tobj->carried_by->in_room == pRoom )
                return obj;
        }
    }
    return NULL;
}

/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list, bool abbrev )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;

  number = number_argument( argument, arg );
  count  = 0;
    
  if (strlen(arg)) 
    {
      for ( obj = list; obj != NULL; obj = obj->next_content )
	{
	  if ( can_see_obj( ch, obj ) && 
	       ( is_name( arg, obj->name ) || ( abbrev && is_abbrev( arg, obj->name ) ) ) )
	    {
	      if ( ++count == number )
		return obj;
	    }
	}
    }
  else 
    {
      for ( obj = list; obj != NULL; obj = obj->next_content )
	{
	  if ( can_see_obj( ch, obj ) 
          &&  (!IS_OBJ_STAT(obj, ITEM_NOTSEEN) || obj->in_room == NULL) )
	    {
	      if ( ++count == number )
		return obj;
	    }
	}
    }

  return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument, CHAR_DATA *viewer, bool abbrev )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;

  number = number_argument( argument, arg );
  count  = 0;

  if (strlen(arg))
    {
      for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	{
	  if ( obj->wear_loc == WEAR_NONE &&
	       can_see_obj( viewer, obj ) &&
	       ( is_name( arg, obj->name ) || ( abbrev && is_abbrev( arg, obj->name ) ) ) )
	    {
	      if ( ++count == number )
		return obj;
	    }
	}

      return NULL;
    }
  else {
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
      {
	if ( obj->wear_loc == WEAR_NONE
	     &&   (can_see_obj( viewer, obj ) ) )
	  {
	    if ( ++count == number )
	      return obj;
	  }
      }

    return NULL;
  }
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument, bool abbrev )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;

  number = number_argument( argument, arg );
  count  = 0;

  if (strlen(arg))
    {
      for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	{
	  if( obj->wear_loc != WEAR_NONE &&
	      can_see_obj( ch, obj ) &&
	      ( is_name( arg, obj->name ) || ( abbrev && is_abbrev( arg, obj->name ) ) ) )
	    {
	      if ( ++count == number )
		return obj;
	    }
	}

      return NULL;
    }
  else {
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
      {
	if ( obj->wear_loc != WEAR_NONE
	     &&   can_see_obj( ch, obj ))
	  {
	    if ( ++count == number )
	      return obj;
	  }
      }

    return NULL;
  }
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument, bool abbrev )
{
    OBJ_DATA *obj;

    obj = get_obj_list( ch, argument, ch->in_room->contents, abbrev );
    if ( obj != NULL )
	return obj;

    if ( ( obj = get_obj_carry( ch, argument, ch, abbrev ) ) != NULL )
	return obj;

    if ( ( obj = get_obj_wear( ch, argument, abbrev ) ) != NULL )
	return obj;

    return NULL;
}



/*
 * Find an obj in the world.
 */

OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument, bool abbrev )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;

  if ( ( obj = get_obj_here( ch, argument, abbrev ) ) != NULL )
    return obj;

  number = number_argument( argument, arg );
  count  = 0;
  for ( obj = object_list; obj != NULL; obj = obj->next )
    {
      if( can_see_obj( ch, obj ) && 
	  ( is_name( arg, obj->name ) || ( abbrev && is_abbrev( arg, obj->name ) ) ) )
	{
	  if ( ++count == number )
	    return obj;
	}
    }

  return NULL;
}

/* deduct cost from a character */

void deduct_cost(CHAR_DATA *ch, int cost)
{
    int silver = 0, gold = 0;

    silver = UMIN(ch->silver,cost); 

    if (silver < cost)
    {
	gold = ((cost - silver + 99) / 100);
	silver = cost - 100 * gold;
    }

    ch->gold -= gold;
    ch->silver -= silver;

    if (ch->gold < 0)
    {
	bug("deduct costs: gold %d < 0",ch->gold);
	ch->gold = 0;
    }
    if (ch->silver < 0)
    {
	bug("deduct costs: silver %d < 0",ch->silver);
	ch->silver = 0;
    }
}   
/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int gold, int silver )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if ( gold < 0 || silver < 0 || (gold == 0 && silver == 0) )
    {
	bug( "Create_money: zero or negative money.",UMIN(gold,silver));
	gold = UMAX(1,gold);
	silver = UMAX(1,silver);
    }

    if (gold == 0 && silver == 1)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_SILVER_ONE ), 0 );
    }
    else if (gold == 1 && silver == 0)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_GOLD_ONE), 0 );
    }
    else if (silver == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_GOLD_SOME ), 0 );
        sprintf( buf, obj->short_descr, gold );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[1]           = gold;
        obj->cost               = gold;
	obj->weight		= gold/5;
    }
    else if (gold == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SILVER_SOME ), 0 );
        sprintf( buf, obj->short_descr, silver );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[0]           = silver;
        obj->cost               = silver;
	obj->weight		= silver/20;
    }
 
    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_COINS ), 0 );
	sprintf( buf, obj->short_descr, silver, gold );
	free_string( obj->short_descr );
	obj->short_descr	= str_dup( buf );
	obj->value[0]		= silver;
	obj->value[1]		= gold;
	obj->cost		= 100 * gold + silver;
	obj->weight		= gold / 5 + silver / 20;
    }

    return obj;
}



/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
    int number;
 
    if ( obj->item_type == ITEM_CONTAINER 
    ||   obj->item_type == ITEM_MONEY 
    ||   obj->item_type == ITEM_GEM 
    ||   obj->item_type == ITEM_JEWELRY 
    ||   obj->item_type == ITEM_QUIVER 
    ||   obj->item_type == ITEM_TRASH 
    ||   obj->item_type == ITEM_BOOK )
        number = 0;
    else
        number = 1;
 
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        number += get_obj_number( obj );
 
    return number;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;
    OBJ_DATA *tobj;

    weight = obj->weight;

    for ( tobj = obj->contains; tobj != NULL; tobj = tobj->next_content )
	weight += get_obj_weight( tobj ) * WEIGHT_MULT(obj) / 100;

    return weight;
}

int get_true_weight(OBJ_DATA *obj)
{
    int weight;
 
    weight = obj->weight;
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        weight += get_true_weight( obj );
 
    return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *fch, *fch_next;
    OBJ_DATA *obj;

    if( pRoomIndex == NULL )
      {
	return FALSE;
      }

    if ( pRoomIndex->light > 0 )
	return FALSE;

    for ( fch = pRoomIndex->people; fch != NULL; fch = fch_next )
        {
            fch_next = fch->next_in_room;
          if (( obj = get_eq_char(fch, WEAR_LIGHT ) ) != NULL
          &&   obj->item_type == ITEM_LIGHT
          &&   obj->value[2] != 0)
         return FALSE;  
}
    if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
    ||   pRoomIndex->sector_type == SECT_CITY )
	return FALSE;

    if ( weather_info.sunlight == SUN_SET
    ||   weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}


bool is_room_owner(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
    if (room->owner == NULL || room->owner[0] == '\0')
	return FALSE;

    return is_name(ch->name,room->owner);
}

/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int count;


    if (pRoomIndex->owner != NULL && pRoomIndex->owner[0] != '\0')
	return TRUE;

    count = 0;
    for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room )
	if (!IS_NPC(rch))
	   count++;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
	return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
	return TRUE;
    
    if ( IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) )
	return TRUE;

    return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    if (IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) 
    &&  get_trust(ch) < MAX_LEVEL)
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
    &&  !IS_IMMORTAL(ch))
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_HEROES_ONLY)
    &&  !IS_IMMORTAL(ch))
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags,ROOM_NEWBIES_ONLY)
    &&  ch->level > 5 && !IS_IMMORTAL(ch))
	return FALSE;

    if (!IS_IMMORTAL(ch) && pRoomIndex->guild && ch->guild != pRoomIndex->guild)
	return FALSE;

    return TRUE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch == victim )
	return TRUE;

/*  ACT_WIZI mobs can see all other mobs for mprog purposes */
    if ( IS_NPC(ch) && IS_NPC(victim) && IS_SET(ch->act_bits,ACT_WIZI) )
        return TRUE;

/*  Wizi imms cannot be seen */
    if ( get_trust(ch) < victim->invis_level)
	return FALSE;

/*  Racewizi imms cannot be seen by PCs but can be seen by unswitched mobs */
    if ( get_trust(ch) < victim->rinvis_level 
    &&   !IS_NPC(ch) && ch->race != victim->race )
        return FALSE;

/*  Incog imms cannot be seen */
    if (get_trust(ch) < victim->incog_level && ch->in_room != victim->in_room)
	return FALSE;

/*  ACT_WIZI mobs can only be seen by admins or builders */
    if ( IS_NPC( victim ) && IS_SET(victim->act_bits, ACT_WIZI)
    &&   !IS_ADMIN( ch )
    &&   !IS_BUILDER( ch, victim->pIndexData->area ) )
	return FALSE;

/*  Holylight imms can see all */
    if ( (!IS_NPC(ch) && IS_SET(ch->act_bits, PLR_HOLYLIGHT) )
    ||   (IS_NPC(ch) && (IS_SET(ch->act_bits, ACT_WIZI) || IS_AFFECTED2(ch, AFF2_BLINDSIGHT) ) )
    ||   (IS_NPC(ch) && IS_IMMORTAL(ch)))
	return TRUE;

/*  if blinded and not blindsight */
    if ( IS_AFFECTED(ch, AFF_BLIND) && !IS_AFFECTED2(ch, AFF2_BLINDSIGHT) )
	return FALSE;

/*  same as above */
    if ( IS_AFFECTED3(ch, AFF_DIRT) && !IS_AFFECTED2(ch, AFF2_BLINDSIGHT ) )
	return FALSE;

/*  if the room has no light and ch isn't infrared */
    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) 
    &&   !IS_AFFECTED2(ch, AFF_HSENSES) )
	return FALSE;

    /* sneaking */
    if ( IS_AFFECTED(victim, AFF_SNEAK)
    &&   !IS_AFFECTED(ch,AFF_DETECT_HIDDEN) 
    &&   !IS_AFFECTED2(ch, AFF_HSENSES)
    &&   victim->fighting == NULL)
    {
	int chance;
	chance = get_skill(victim,gsn_sneak);
	/*chance += get_curr_stat(victim,STAT_DEX) * 3/2;*/
 	chance -= get_curr_stat(ch,STAT_INT) * 2;
	chance -= ch->level - victim->level * 3/2;

	if (number_percent() < chance)
	    return FALSE;
    }

    if ( IS_AFFECTED(victim, AFF_HIDE)
    &&   !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
    &&   !IS_AFFECTED2(ch, AFF_HSENSES)  
    &&   victim->fighting == NULL)
	return FALSE;

    return TRUE;
}



/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_NPC(ch) && IS_SET(ch->act_bits, PLR_HOLYLIGHT) )
	return TRUE;

    if ( IS_AFFECTED2( ch, AFF2_BLINDSIGHT ) )
        return TRUE;

    if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH))
	return FALSE;

    if ( IS_AFFECTED( ch, AFF_BLIND )
    &&   obj->item_type != ITEM_POTION && obj->item_type != ITEM_PILL )
	return FALSE;

    if ( IS_AFFECTED3( ch, AFF_DIRT )
    &&   obj->item_type != ITEM_POTION && obj->item_type != ITEM_PILL )
	return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	return TRUE;

    if ( IS_SET(obj->extra_flags, ITEM_INVIS)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS)
    &&   !IS_AFFECTED2(ch, AFF_HSENSES) )
        return FALSE;

    if ( IS_OBJ_STAT(obj,ITEM_GLOW))
	return TRUE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_SET(obj->extra_flags, ITEM_NODROP) )
	return TRUE;

    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return TRUE;

    return FALSE;
}


/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( int location )
{
    switch ( location )
    {
    case APPLY_NONE:		return "none";
    case APPLY_STR:		return "strength";
    case APPLY_DEX:		return "dexterity";
    case APPLY_INT:		return "intelligence";
    case APPLY_WIS:		return "wisdom";
    case APPLY_CON:		return "constitution";
    case APPLY_SEX:		return "sex";
    case APPLY_CLASS:		return "class";
    case APPLY_LEVEL:		return "level";
    case APPLY_AGE:		return "age";
    case APPLY_MANA:		return "mana";
    case APPLY_HIT:		return "hp";
    case APPLY_MOVE:		return "moves";
    case APPLY_BLOOD:           return "blood pressure";
    case APPLY_GOLD:		return "gold";
    case APPLY_EXP:		return "experience";
    case APPLY_AC:		return "armor class";
    case APPLY_HITROLL:		return "hit roll";
    case APPLY_DAMROLL:		return "damage roll";
    case APPLY_SAVES:		return "saves";
    case APPLY_SAVING_ROD:	return "save vs rod";
    case APPLY_SAVING_PETRI:	return "save vs petrification";
    case APPLY_SAVING_BREATH:	return "save vs breath";
    case APPLY_SAVING_SPELL:	return "save vs spell";
    case APPLY_SPELL_AFFECT:	return "none";
    case APPLY_SKILLS:          return "skills";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}

/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name( int extra_flags )
{
    static char buf[512];

    buf[0] = '\0';
    if ( extra_flags & ITEM_GLOW         ) strcat( buf, " glow"         );
    if ( extra_flags & ITEM_HUM          ) strcat( buf, " hum"          );
    if ( extra_flags & ITEM_DARK         ) strcat( buf, " dark"         );
    if ( extra_flags & ITEM_LOCK         ) strcat( buf, " lock"         );
    if ( extra_flags & ITEM_INVIS        ) strcat( buf, " invis"        );
    if ( extra_flags & ITEM_MAGIC        ) strcat( buf, " magic"        );
    if ( extra_flags & ITEM_NODROP       ) strcat( buf, " nodrop"       );
    if ( extra_flags & ITEM_BLESS        ) strcat( buf, " bless"        );
    if ( extra_flags & ITEM_NOREMOVE     ) strcat( buf, " noremove"     );
    if ( extra_flags & ITEM_INVENTORY    ) strcat( buf, " inventory"    );
    if ( extra_flags & ITEM_NOPURGE	 ) strcat( buf, " nopurge"	);
    if ( extra_flags & ITEM_VIS_DEATH	 ) strcat( buf, " visdeath"	);
    if ( extra_flags & ITEM_ROT_DEATH	 ) strcat( buf, " rotdeath"	);
    if ( extra_flags & ITEM_NOLOCATE	 ) strcat( buf, " nolocate"	);
    if ( extra_flags & ITEM_SELL_EXTRACT ) strcat( buf, " sellextract"  );
    if ( extra_flags & ITEM_BURN_PROOF	 ) strcat( buf, " burnproof"	);
    if ( extra_flags & ITEM_NOUNCURSE	 ) strcat( buf, " nouncurse"	);
    if ( extra_flags & ITEM_NO_IDENTIFY	 ) strcat( buf, " noidentify"	);
    if ( extra_flags & ITEM_KO_ROT	 ) strcat( buf, " quest"	);
    if ( extra_flags & ITEM_MELT_DROP    ) strcat( buf, " meltdrop"     );
    if ( extra_flags & ITEM_NOTSEEN      ) strcat( buf, " notseen"      );
    if ( extra_flags & ITEM_NOLOOT       ) strcat( buf, " noloot"       );
    if ( extra_flags & ITEM_GUILD_ROT    ) strcat( buf, " guildrot"     );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/* return ascii name of an act vector */
char *act_bit_name( int flags )
{
    static char buf[512];

    buf[0] = '\0';

    if (IS_SET(flags,ACT_IS_NPC))
    { 
/*      sprintf(buf, "%s", flag_string( act_flags, flags ) ); */
        strcat( buf, flag_string( act_flags, flags ) );
/*
 	strcat(buf," npc");
    	if (act_flags & ACT_WERETRAIN   ) strcat(buf, " were_train");
    	if (act_flags & ACT_SENTINEL 	) strcat(buf, " sentinel");
    	if (act_flags & ACT_SCAVENGER	) strcat(buf, " scavenger");
	if (act_flags & ACT_AGGRESSIVE	) strcat(buf, " aggressive");
        if (act_flags & ACT_NOKILL	) strcat(buf, " nokill");
        if (act_flags & ACT_NOTSEEN	) strcat(buf, " notseen");
	if (act_flags & ACT_STAY_AREA	) strcat(buf, " stay_area");
	if (act_flags & ACT_WIMPY	) strcat(buf, " wimpy");
	if (act_flags & ACT_PET		) strcat(buf, " pet");
	if (act_flags & ACT_TRAIN	) strcat(buf, " train");
	if (act_flags & ACT_PRACTICE	) strcat(buf, " practice");
	if (act_flags & ACT_WIZI	) strcat(buf, " wizi");
	if (act_flags & ACT_UNDEAD	) strcat(buf, " undead");
	if (act_flags & ACT_CLERIC	) strcat(buf, " cleric");
	if (act_flags & ACT_MAGE	) strcat(buf, " mage");
	if (act_flags & ACT_THIEF	) strcat(buf, " thief");
	if (act_flags & ACT_WARRIOR	) strcat(buf, " warrior");
	if (act_flags & ACT_NOPURGE	) strcat(buf, " no_purge");
	if (act_flags & ACT_IS_HEALER	) strcat(buf, " healer");
	if (act_flags & ACT_IS_CHANGER  ) strcat(buf, " changer");
	if (act_flags & ACT_GAIN	) strcat(buf, " skill_train");
	if (act_flags & ACT_UPDATE_ALWAYS) strcat(buf," update_always");
*/
    }
    else
    {
/*      sprintf(buf, "%s", flag_string( plr_flags, flags ) ); */
        strcat( buf, flag_string( plr_flags, flags ) );
/*
	strcat(buf," player");
	if (act_flags & PLR_AUTOASSIST	) strcat(buf, " autoassist");
	if (act_flags & PLR_AUTOEXIT	) strcat(buf, " autoexit");
	if (act_flags & PLR_AUTOLOOT	) strcat(buf, " autoloot");
	if (act_flags & PLR_AUTOSAC	) strcat(buf, " autosac");
	if (act_flags & PLR_AUTOGOLD	) strcat(buf, " autogold");
	if (act_flags & PLR_AUTOSPLIT	) strcat(buf, " autosplit");
	if (act_flags & PLR_HOLYLIGHT	) strcat(buf, " holy_light");
	if (act_flags & PLR_NOSUMMON	) strcat(buf, " no_summon");
	if (act_flags & PLR_NOFOLLOW	) strcat(buf, " no_follow");
	if (act_flags & PLR_FREEZE	) strcat(buf, " frozen");
	if (act_flags & PLR_COLOUR	) strcat(buf, " colour");
	if (act_flags & PLR_NEWBIE	) strcat(buf, " newbie");
	if (act_flags & PLR_KILLER	) strcat(buf, " killer");
	if (act_flags & PLR_PUEBLO	) strcat(buf, " pueblo");
	if (act_flags & PLR_LINKDEAD    ) strcat(buf, " link_dead");
	if (act_flags & PLR_SUSPEND_LEVEL ) strcat( buf, " suspend_level");
	if (act_flags & PLR_BRAWLER     ) strcat(buf, " brawler");
*/
    }
    return ( buf[0] != '\0' ) ? buf : "none";
}

char *shift_bit_name(int shift_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (shift_flags & SHIFT_POTENTIAL   ) strcat(buf, " were potential" );
    if (shift_flags & SHIFT_HALF        ) strcat(buf, " shifted half"   );
    if (shift_flags & SHIFT_FULL        ) strcat(buf, " shifted full"   );
    if (shift_flags & PERM_VAMP         ) strcat(buf, " patriarch"      );
    if (shift_flags & TEMP_VAMP         ) strcat(buf, " minion"         );
    if (shift_flags & MIST_VAMP         ) strcat(buf, " mist"           );
    if (shift_flags & BAT_VAMP          ) strcat(buf, " bat"            );
    if (shift_flags & SHIFT_NOFORCE     ) strcat(buf, " noforce"        );
    if (shift_flags & SHIFT_FIXEDBP     ) strcat(buf, " fixedbp"        );

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char* vuln_bit_name( int vuln_flags )
{
  static char buf[ 512 ];
  buf[ 0 ] = '\0';

  if( vuln_flags & VULN_SUMMON		   ) strcat( buf, " summon" );
  if( vuln_flags & VULN_CHARM		   ) strcat( buf, " charm" );
  if( vuln_flags & VULN_MAGIC              ) strcat( buf, " magic" );
  if( vuln_flags & VULN_WEAPON             ) strcat( buf, " weapon" );
  if( vuln_flags & VULN_BASH               ) strcat( buf, " bash" );
  if( vuln_flags & VULN_PIERCE             ) strcat( buf, " pierce" );
  if( vuln_flags & VULN_SLASH              ) strcat( buf, " slash" );
  if( vuln_flags & VULN_FIRE               ) strcat( buf, " fire" );
  if( vuln_flags & VULN_COLD               ) strcat( buf, " cold" );
  if( vuln_flags & VULN_LIGHT              ) strcat( buf, " light" );
  if( vuln_flags & VULN_LIGHTNING          ) strcat( buf, " lightning" );
  if( vuln_flags & VULN_ACID               ) strcat( buf, " acid" );
  if( vuln_flags & VULN_POISON             ) strcat( buf, " poison" );
  if( vuln_flags & VULN_NEGATIVE           ) strcat( buf, " negative" );
  if( vuln_flags & VULN_HOLY               ) strcat( buf, " holy" );
  if( vuln_flags & VULN_ENERGY             ) strcat( buf, " energy" );
  if( vuln_flags & VULN_MENTAL             ) strcat( buf, " mental" );
  if( vuln_flags & VULN_DISEASE            ) strcat( buf, " disease" );
  if( vuln_flags & VULN_DROWNING           ) strcat( buf, " drowning" );
  if( vuln_flags & VULN_SOUND		   ) strcat( buf, " sound" );
  if( vuln_flags & VULN_WIND               ) strcat( buf, " wind" );
  if( vuln_flags & VULN_WOOD               ) strcat( buf, " wood" );
  if( vuln_flags & VULN_SILVER             ) strcat( buf, " silver" );
  if( vuln_flags & VULN_IRON		   ) strcat( buf, " iron" );
  if( vuln_flags & VULN_GATE               ) strcat( buf, " gate" );
  if( vuln_flags & VULN_PORTAL             ) strcat( buf, " portal" );
  if( vuln_flags & VULN_LAGSKILLS          ) strcat( buf, " lagskills(!)" );
/* VULN_LAGSKILLS is defined but has no function - DO NOT ASSIGN */


  return( ( buf[ 0 ] == '\0' ) ? "none" : buf + 1 );
}

char* res_bit_name( int res_flags )
{
  static char buf[ 512 ];
  buf[ 0 ] = '\0';

  if( res_flags & RES_SUMMON    ) strcat( buf, " summon" );
  if( res_flags & RES_CHARM     ) strcat( buf, " charm" );
  if( res_flags & RES_MAGIC     ) strcat( buf, " magic" );
  if( res_flags & RES_WEAPON    ) strcat( buf, " weapon" );
  if( res_flags & RES_BASH      ) strcat( buf, " bash" );
  if( res_flags & RES_PIERCE    ) strcat( buf, " pierce" );
  if( res_flags & RES_SLASH     ) strcat( buf, " slash" );
  if( res_flags & RES_FIRE      ) strcat( buf, " fire" );
  if( res_flags & RES_COLD      ) strcat( buf, " cold" );
  if( res_flags & RES_LIGHT     ) strcat( buf, " light" );
  if( res_flags & RES_LIGHTNING ) strcat( buf, " lightning" );
  if( res_flags & RES_ACID      ) strcat( buf, " acid" );
  if( res_flags & RES_POISON    ) strcat( buf, " poison" );
  if( res_flags & RES_NEGATIVE  ) strcat( buf, " negative" );
  if( res_flags & RES_HOLY      ) strcat( buf, " holy" );
  if( res_flags & RES_ENERGY    ) strcat( buf, " energy" );
  if( res_flags & RES_MENTAL    ) strcat( buf, " mental" );
  if( res_flags & RES_DISEASE   ) strcat( buf, " disease" );
  if( res_flags & RES_DROWNING  ) strcat( buf, " drowning" );
  if( res_flags & RES_SOUND	) strcat( buf, " sound" );
  if( res_flags & RES_WIND      ) strcat( buf, " wind" );
  if( res_flags & RES_WOOD      ) strcat( buf, " wood" );
  if( res_flags & RES_SILVER    ) strcat( buf, " silver" );
  if( res_flags & RES_IRON      ) strcat( buf, " iron" );
  if( res_flags & RES_GATE      ) strcat( buf, " gate" );
  if( res_flags & RES_PORTAL    ) strcat( buf, " portal" );
  if ( res_flags & RES_LAGSKILLS) strcat( buf, " lagskills(!)" );
/* RES_LAGSKILLS is defined but has no function - DO NOT ASSIGN */

  return( ( buf[ 0 ] == '\0' ) ? "none" : buf + 1 );
}
/* Obsolete due to flag_string 
char *imm_bit_name(int imm_flags)
*/

char *form_bit_name(int form_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (form_flags & FORM_POISON	) strcat(buf, " poison");
    else if (form_flags & FORM_EDIBLE	) strcat(buf, " edible");
    if (form_flags & FORM_MAGICAL	) strcat(buf, " magical");
    if (form_flags & FORM_INSTANT_DECAY	) strcat(buf, " instant_rot");
    if (form_flags & FORM_OTHER		) strcat(buf, " other");
    if (form_flags & FORM_ANIMAL	) strcat(buf, " animal");
    if (form_flags & FORM_SENTIENT	) strcat(buf, " sentient");
    if (form_flags & FORM_UNDEAD	) strcat(buf, " undead");
    if (form_flags & FORM_CONSTRUCT	) strcat(buf, " construct");
    if (form_flags & FORM_MIST		) strcat(buf, " mist");
    if (form_flags & FORM_INTANGIBLE	) strcat(buf, " intangible");
    if (form_flags & FORM_BIPED		) strcat(buf, " biped");
    if (form_flags & FORM_CENTAUR	) strcat(buf, " centaur");
    if (form_flags & FORM_INSECT	) strcat(buf, " insect");
    if (form_flags & FORM_SPIDER	) strcat(buf, " spider");
    if (form_flags & FORM_CRUSTACEAN	) strcat(buf, " crustacean");
    if (form_flags & FORM_WORM		) strcat(buf, " worm");
    if (form_flags & FORM_BLOB		) strcat(buf, " blob");
    if (form_flags & FORM_MAMMAL	) strcat(buf, " mammal");
    if (form_flags & FORM_BIRD		) strcat(buf, " bird");
    if (form_flags & FORM_REPTILE	) strcat(buf, " reptile");
    if (form_flags & FORM_SNAKE		) strcat(buf, " snake");
    if (form_flags & FORM_DRAGON	) strcat(buf, " dragon");
    if (form_flags & FORM_AMPHIBIAN	) strcat(buf, " amphibian");
    if (form_flags & FORM_FISH		) strcat(buf, " fish");
    if (form_flags & FORM_COLD_BLOOD 	) strcat(buf, " cold_blooded");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *part_bit_name(int part_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (part_flags & PART_HEAD		) strcat(buf, " head");
    if (part_flags & PART_ARMS		) strcat(buf, " arms");
    if (part_flags & PART_LEGS		) strcat(buf, " legs");
    if (part_flags & PART_HEART		) strcat(buf, " heart");
    if (part_flags & PART_BRAINS	) strcat(buf, " brains");
    if (part_flags & PART_GUTS		) strcat(buf, " guts");
    if (part_flags & PART_HANDS		) strcat(buf, " hands");
    if (part_flags & PART_FEET		) strcat(buf, " feet");
    if (part_flags & PART_FINGERS	) strcat(buf, " fingers");
    if (part_flags & PART_EAR		) strcat(buf, " ears");
    if (part_flags & PART_EYE		) strcat(buf, " eyes");
    if (part_flags & PART_LONG_TONGUE	) strcat(buf, " long_tongue");
    if (part_flags & PART_EYESTALKS	) strcat(buf, " eyestalks");
    if (part_flags & PART_TENTACLES	) strcat(buf, " tentacles");
    if (part_flags & PART_FINS		) strcat(buf, " fins");
    if (part_flags & PART_WINGS		) strcat(buf, " wings");
    if (part_flags & PART_TAIL		) strcat(buf, " tail");
    if (part_flags & PART_CLAWS		) strcat(buf, " claws");
    if (part_flags & PART_FANGS		) strcat(buf, " fangs");
    if (part_flags & PART_HORNS		) strcat(buf, " horns");
    if (part_flags & PART_SCALES	) strcat(buf, " scales");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *weapon_bit_name(int weapon_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (weapon_flags & WEAPON_FLAMING	) strcat(buf, " flaming");
    if (weapon_flags & WEAPON_FROST	) strcat(buf, " frost");
    if (weapon_flags & WEAPON_VAMPIRIC	) strcat(buf, " vampiric");
	if (weapon_flags & WEAPON_ANGELIC	) strcat(buf, " angelic");
    if (weapon_flags & WEAPON_SHARP	) strcat(buf, " sharp");
    if (weapon_flags & WEAPON_VORPAL	) strcat(buf, " vorpal");
    if (weapon_flags & WEAPON_TWO_HANDS ) strcat(buf, " two-handed");
    if (weapon_flags & WEAPON_SHOCKING 	) strcat(buf, " shocking");
    if (weapon_flags & WEAPON_POISON	) strcat(buf, " poison");
    if (weapon_flags & WEAPON_STICKY    ) strcat(buf, " sticky");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *off_bit_name(int off_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (off_flags & OFF_AREA_ATTACK	) strcat(buf, " area attack");
    if (off_flags & OFF_BACKSTAB	) strcat(buf, " backstab");
    if (off_flags & OFF_BASH		) strcat(buf, " bash");
    if (off_flags & OFF_BERSERK		) strcat(buf, " battle");
    if (off_flags & OFF_DISARM		) strcat(buf, " disarm");
    if (off_flags & OFF_DODGE		) strcat(buf, " dodge");
    if (off_flags & OFF_FADE		) strcat(buf, " fade");
    if (off_flags & OFF_FAST		) strcat(buf, " fast");
    if (off_flags & OFF_KICK		) strcat(buf, " kick");
    if (off_flags & OFF_KICK_DIRT	) strcat(buf, " kick_dirt");
    if (off_flags & OFF_PARRY		) strcat(buf, " parry");
    if (off_flags & OFF_RESCUE		) strcat(buf, " rescue");
    if (off_flags & OFF_TAIL		) strcat(buf, " tail");
    if (off_flags & OFF_TRIP		) strcat(buf, " trip");
    if (off_flags & OFF_CRUSH		) strcat(buf, " crush");
    if (off_flags & ASSIST_ALL		) strcat(buf, " assist_all");
    if (off_flags & ASSIST_RACE		) strcat(buf, " assist_race");
    if (off_flags & ASSIST_PLAYERS	) strcat(buf, " assist_players");
    if (off_flags & ASSIST_GUARD	) strcat(buf, " assist_guard");
    if (off_flags & ASSIST_VNUM		) strcat(buf, " assist_vnum");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char* trans_bit_name( int trans_flags )
{
  static char buf[ 512 ];

  buf[ 0 ] = '\0';

  if( trans_flags & TRANS_NO_PORTAL       ) strcat( buf, " no_portal" );
  if( trans_flags & TRANS_NO_PORTAL_IN    ) strcat( buf, " no_portal_in" );
  if( trans_flags & TRANS_NO_PORTAL_OUT   ) strcat( buf, " no_portal_out" );
  if( trans_flags & TRANS_NO_GATE         ) strcat( buf, " no_gate" );
  if( trans_flags & TRANS_NO_GATE_IN      ) strcat( buf, " no_gate_in" );
  if( trans_flags & TRANS_NO_GATE_OUT     ) strcat( buf, " no_gate_out" );
  if( trans_flags & TRANS_NO_TELEPORT     ) strcat( buf, " no_teleport" );
  if( trans_flags & TRANS_NO_TELEPORT_IN  ) strcat( buf, " no_teleport_in" );
  if( trans_flags & TRANS_NO_TELEPORT_OUT ) strcat( buf, " no_teleport_out" );
  if( trans_flags & TRANS_NO_SUMMON       ) strcat( buf, " no_summon" );
  if( trans_flags & TRANS_NO_SUMMON_IN    ) strcat( buf, " no_summon_in" );
  if( trans_flags & TRANS_NO_SUMMON_OUT   ) strcat( buf, " no_summon_out" );
  if( trans_flags & TRANS_NO_SPELLS       ) strcat( buf, " no_spells" );
  if( trans_flags & TRANS_NO_SPELLS_IN    ) strcat( buf, " no_spells_in" );
  if( trans_flags & TRANS_NO_SPELLS_OUT   ) strcat( buf, " no_spells_out" );
  if( trans_flags & TRANS_NO_RECALL       ) strcat( buf, " no_recall" );
  if( trans_flags & TRANS_NO_BAD_GATES    ) strcat( buf, " no_bad_gates" );

  return( ( buf[ 0 ] == '\0' ) ? "none" : buf + 1 );
}



char* room_bit_name( int room_flags )
{
  static char buf[ 512 ];

  buf[ 0 ] = '\0';

  if( room_flags & ROOM_DARK         ) strcat( buf, " dark" );
  if( room_flags & ROOM_BANK         ) strcat( buf, " bank" );
  if( room_flags & ROOM_NO_MOB       ) strcat( buf, " no_mob" );
  if( room_flags & ROOM_INDOORS      ) strcat( buf, " indoors" );
  if( room_flags & ROOM_PRIVATE      ) strcat( buf, " private" );
  if( room_flags & ROOM_SAFE         ) strcat( buf, " safe" );
  if( room_flags & ROOM_SEMISAFE     ) strcat( buf, " semisafe" );
  if( room_flags & ROOM_SOLITARY     ) strcat( buf, " solitary" );
  if( room_flags & ROOM_PET_SHOP     ) strcat( buf, " pet_shop" );
  if( room_flags & ROOM_NO_RECALL    ) strcat( buf, " no_recall" );
  if( room_flags & ROOM_IMP_ONLY     ) strcat( buf, " imp_only" );
  if( room_flags & ROOM_GODS_ONLY    ) strcat( buf, " gods_only" );
  if( room_flags & ROOM_HEROES_ONLY  ) strcat( buf, " heroes_only" );
  if( room_flags & ROOM_NEWBIES_ONLY ) strcat( buf, " newbies_only" );
  if( room_flags & ROOM_LAW          ) strcat( buf, " law" );
  if( room_flags & ROOM_NOWHERE      ) strcat( buf, " nowhere" );
  if( room_flags & ROOM_NO_GATE      ) strcat( buf, " no_gate" );
  if( room_flags & ROOM_NO_PORTAL    ) strcat( buf, " no_portal" );
  if( room_flags & ROOM_NO_SUMMON    ) strcat( buf, " no_summon" );

  return( buf[ 0 ] != '\0' ) ? buf + 1 : "none";

}
/* end kes */

void substitute_prefix(DESCRIPTOR_DATA *d, char *argument)
{
    CHAR_DATA *ch;
    char buf[MAX_STRING_LENGTH],prefix[MAX_INPUT_LENGTH];
 
    ch = d->original ? d->original : d->character;
 
    /* check for prefix */
    if (ch->prefix[0] != '\0' && str_prefix("prefix",argument))
    {
        if (strlen(ch->prefix) + strlen(argument) > MAX_INPUT_LENGTH)
            send_to_char("Line to long, prefix not processed.\r\n",ch);
        else
        {
            sprintf(prefix,"%s %s",ch->prefix,argument);
            argument = prefix;
        }
    }

    if (IS_NPC(ch)
    ||  !str_prefix("prefix",argument))
    {
        interpret(d->character,argument);
        return;
    }

    strcpy(buf,argument);
    interpret(d->character,buf);
}

char *one_listize_arg( char *argument, char *arg_first )
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

char *listize( char *argument, bool xand )
{
    static char ostr[MAX_STRING_LENGTH];
    char thisarg[MAX_STRING_LENGTH] = "";
    char *pthisarg;
    int narg = 0, i;

    ostr[0] = '\0';
    pthisarg = argument;

/*  First loop through to count up number of arguments */
    while ( !IS_NULLSTR( argument ) )
    {
        argument = one_listize_arg(argument,thisarg);
        narg++;
    }
    argument = pthisarg;

    if ( narg == 1 )
        strcat( ostr, thisarg );
    else if ( narg == 2 )
    {
        strcat( ostr, thisarg );
        if ( xand )
            strcat( ostr, " and " );
        else
            strcat( ostr, " or " );
        one_listize_arg(argument,thisarg);
        strcat( ostr, thisarg );
    }
    else
    {
        for ( i = 1; i <= narg; i++ )
        {
            argument = one_listize_arg(argument,thisarg);
            strcat( ostr, thisarg );
            if ( i == narg-1 )
                if ( xand )
                    strcat( ostr, ", and " );
                else
                    strcat( ostr, ", or " );
            else if ( i != narg )
                strcat( ostr, ", " );
        }
    }

    return ostr;
}

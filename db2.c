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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include "merc.h"
#include "db.h"
#include "tables.h"
#include "lookup.h"
#include "recycle.h"
#include "olc.h"

extern int flag_lookup args((const char *name, const struct flag_type *flag_table));

/*
 * Snarf a mob section.  new style
 */
void load_mobiles( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
 
    if ( !area_last )   /* OLC */
    {
        bug( "Load_mobiles: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        sh_int vnum;
        char letter;
        int iHash;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_mobiles: # not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        fBootDb = FALSE;
        if ( get_mob_index( vnum ) != NULL )
        {
            bug( "Load_mobiles: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;
 
        pMobIndex                       = alloc_perm( sizeof(*pMobIndex) );
        pMobIndex->vnum                 = vnum;
        pMobIndex->area                 = area_last;               /* OLC */
	pMobIndex->new_format		= TRUE;
	newmobs++;
        pMobIndex->player_name          = fread_string( fp );
        pMobIndex->short_descr          = fread_string( fp );
        pMobIndex->long_descr           = fread_string( fp );
        pMobIndex->description          = fread_string( fp );
	pMobIndex->race		 	= race_lookup(fread_string( fp ));
 
        pMobIndex->long_descr[0]        = UPPER(pMobIndex->long_descr[0]);
        pMobIndex->description[0]       = UPPER(pMobIndex->description[0]);
 
        pMobIndex->act                  = fread_flag( fp ) | ACT_IS_NPC
					| race_table[pMobIndex->race].act;
/* This is a hacky check that points out any oldstyle mobs with
   defunct act flags set.  Could cause bad things to happen if these
   act flags' bit values are recycled, so re-enable to have it dump
   a list of any values into the log file.
        if ( IS_SET( pMobIndex->act, ACT_NOKILL   ) )
            bug( "Mob vnum %d is flagged ______.", vnum );
   */
        pMobIndex->affected_by          = fread_flag( fp )
					| race_table[pMobIndex->race].aff;
        pMobIndex->affected_by2          = fread_flag( fp )
					| race_table[pMobIndex->race].aff2;
        pMobIndex->affected_by3          = fread_flag( fp )
					| race_table[pMobIndex->race].aff3;
        pMobIndex->pShop                = NULL;
        pMobIndex->alignment            = fread_number( fp );
        pMobIndex->group                = fread_number( fp );

        pMobIndex->level                = fread_number( fp );
        pMobIndex->hitroll              = fread_number( fp );  

	/* read hit dice */
        pMobIndex->hit[DICE_NUMBER]     = fread_number( fp );  
        /* 'd'          */                fread_letter( fp ); 
        pMobIndex->hit[DICE_TYPE]   	= fread_number( fp );
        /* '+'          */                fread_letter( fp );   
        pMobIndex->hit[DICE_BONUS]      = fread_number( fp ); 

 	/* read mana dice */
	pMobIndex->mana[DICE_NUMBER]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->mana[DICE_TYPE]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->mana[DICE_BONUS]	= fread_number( fp );

	/* read damage dice */
	pMobIndex->damage[DICE_NUMBER]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_TYPE]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_BONUS]	= fread_number( fp );
	pMobIndex->dam_type		= attack_lookup(fread_word(fp));

	/* read armor class */
	pMobIndex->ac[AC_PIERCE]	= fread_number( fp ) * 10;
	pMobIndex->ac[AC_BASH]		= fread_number( fp ) * 10;
	pMobIndex->ac[AC_SLASH]		= fread_number( fp ) * 10;
	pMobIndex->ac[AC_EXOTIC]	= fread_number( fp ) * 10;

	/* read flags - add in data from the race table disabled */
	pMobIndex->off_flags		= fread_flag( fp )
					| race_table[pMobIndex->race].off;
	pMobIndex->imm_flags		= fread_flag( fp )
					 | race_table[pMobIndex->race].imm; 

	pMobIndex->res_flags		= fread_flag( fp )
					 | race_table[pMobIndex->race].res; 
	pMobIndex->vuln_flags		= fread_flag( fp )
					 | race_table[pMobIndex->race].vuln; 

/*
        if ( IS_SET(pMobIndex->res_flags, RES_LIGHT ) )
            bug( "Mob vnum %d was flagged res (U).", vnum );
        if ( IS_SET(pMobIndex->vuln_flags, VULN_LIGHT ) )
            bug( "Mob vnum %d was flagged vuln (U).", vnum );
        if ( IS_SET(pMobIndex->imm_flags, IMM_LIGHT ) )
            bug( "Mob vnum %d was flagged imm (U).", vnum );

*** REMOVE BELOW CODE AT SOME POINT AFTER CONVERTING AREA FILES ***/

        if ( IS_SET(pMobIndex->res_flags,RES_LAGSKILLS) )
        {
            bug( "Mob vnum %d was flagged res lagskills.  Changed to res light.", vnum );
            REMOVE_BIT( pMobIndex->res_flags, RES_LAGSKILLS );
            SET_BIT( pMobIndex->res_flags, RES_LIGHT );
        }
        if ( IS_SET(pMobIndex->vuln_flags,VULN_LAGSKILLS) )
        {
            bug( "Mob vnum %d was flagged vuln lagskills.  Changed to res light.", vnum );
            REMOVE_BIT( pMobIndex->vuln_flags, VULN_LAGSKILLS );
            SET_BIT( pMobIndex->vuln_flags, VULN_LIGHT );
        }

	/* vital statistics */
	pMobIndex->start_pos		= position_lookup(fread_word(fp));
	pMobIndex->default_pos		= position_lookup(fread_word(fp));
	pMobIndex->sex			= sex_lookup(fread_word(fp));

	pMobIndex->wealth		= fread_number( fp );

	pMobIndex->form			= fread_flag( fp )
					| race_table[pMobIndex->race].form;
	pMobIndex->parts		= fread_flag( fp )
					| race_table[pMobIndex->race].parts;
	/* size */
	CHECK_POS( pMobIndex->size, size_lookup(fread_word(fp)), "size" );
/*	pMobIndex->size			= size_lookup(fread_word(fp)); */
	pMobIndex->material		= str_dup(fread_word( fp ));
 
	for ( ; ; )
        {
            letter = fread_letter( fp );

            if (letter == 'F')
            {
		char *word;
		long vector;

                word                    = fread_word(fp);
		vector			= fread_flag(fp);

		if (!str_prefix(word,"act"))
		    REMOVE_BIT(pMobIndex->act,vector);
                else if (!str_prefix(word,"aff"))
		    REMOVE_BIT(pMobIndex->affected_by,vector);
		else if (!str_prefix(word,"aff2"))
		  REMOVE_BIT(pMobIndex->affected_by2,vector);
		else if (!str_prefix(word,"aff3"))
		  REMOVE_BIT(pMobIndex->affected_by3,vector);
		else if (!str_prefix(word,"off"))
		    REMOVE_BIT(pMobIndex->off_flags,vector);
		else if (!str_prefix(word,"imm"))
		    REMOVE_BIT(pMobIndex->imm_flags,vector);
		else if (!str_prefix(word,"res"))
		    REMOVE_BIT(pMobIndex->res_flags,vector);
		else if (!str_prefix(word,"vul"))
		    REMOVE_BIT(pMobIndex->vuln_flags,vector);
		else if (!str_prefix(word,"for"))
		    REMOVE_BIT(pMobIndex->form,vector);
		else if (!str_prefix(word,"par"))
		    REMOVE_BIT(pMobIndex->parts,vector);
		else
		{
		    bug("Flag remove: flag not found.",0);
		    exit(1);
		}
	     }
	     else if ( letter == 'M' )
	     {
		MPROG_LIST *pMprog;
		char *word;
		int trigger = 0;
		
		pMprog              = alloc_perm(sizeof(*pMprog));
		word   		    = fread_word( fp );
		if ( !(trigger = flag_lookup( word, mprog_flags )) )
		{
		    bug("MOBprogs: invalid trigger.",0);
		    exit(1);
		}
		SET_BIT( pMobIndex->mprog_flags, trigger );
		pMprog->trig_type   = trigger;
		pMprog->vnum        = fread_number( fp );
		pMprog->trig_phrase = fread_string( fp );
		pMprog->next        = pMobIndex->mprogs;
		pMobIndex->mprogs   = pMprog;
	     }

             else if ( letter == 'P' )
             {
                /* Pueblo support is being phased out 7/24/09 */
                fread_string( fp );
             }
             else if ( letter == 'Q' )
             {
                RESET_DATA *pReset;

                /* Read in the per-mob reset data */
                pReset          = new_reset_data();
                pReset->command = letter;
                /* arg1 is the item vnum */
                /* arg2 is the wear loc */
                pReset->arg1    = fread_number( fp );
                pReset->arg2    = fread_number( fp );
                pReset->arg3    = 0;
                pReset->arg4    = 0;

                fread_to_eol( fp );

                if ( !pMobIndex->reset_last )
                {
                    pMobIndex->reset_first = pReset;
                    pMobIndex->reset_last = pReset;
                }
                else
                {
                    pMobIndex->reset_last->next = pReset;
                    pMobIndex->reset_last       = pReset;
                    pMobIndex->reset_last->next = NULL;
                }
             }
	     else
	     {
		ungetc(letter,fp);
		break;
	     }
	}

        iHash                   = vnum % MAX_KEY_HASH;
        pMobIndex->next         = mob_index_hash[iHash];
        mob_index_hash[iHash]   = pMobIndex;
        top_mob_index++;
        top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;  /* OLC */
        assign_area_vnum( vnum );                                  /* OLC */
        kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }
 
    return;
}

void load_objects( FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;

    if ( !area_last )   /* OLC */
    {
        bug( "Load_objects: no #AREA seen yet.", 0 );
        exit( 1 );
    }
    for ( ; ; )
    {
        sh_int vnum;
        char letter;
        int iHash;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_objects: # not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        fBootDb = FALSE;
        if ( get_obj_index( vnum ) != NULL )
        {
            bug( "Load_objects: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;
 
        pObjIndex                       = alloc_perm( sizeof(*pObjIndex) );
        pObjIndex->vnum                 = vnum;
        pObjIndex->area                 = area_last;            /* OLC */
        pObjIndex->new_format           = TRUE;
	pObjIndex->reset_num		= 0;
	newobjs++;
        pObjIndex->name                 = fread_string( fp );
        pObjIndex->short_descr          = fread_string( fp );
        pObjIndex->description          = fread_string( fp );
        pObjIndex->material		= fread_string( fp );

	CHECK_POS(pObjIndex->item_type, item_lookup(fread_word( fp )), "item_type" );
        pObjIndex->extra_flags          = fread_flag( fp );
        pObjIndex->wear_flags           = fread_flag( fp );
	switch(pObjIndex->item_type)
	{
	case ITEM_WEAPON:
	    pObjIndex->value[0]		= weapon_type(fread_word(fp));
	    pObjIndex->value[1]		= fread_number(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= attack_lookup(fread_word(fp));
	    pObjIndex->value[4]		= fread_flag(fp);
	    break;
	case ITEM_CONTAINER:
	case ITEM_QUIVER:
        case ITEM_BOOK:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= fread_flag(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= fread_number(fp);
	    pObjIndex->value[4]		= fread_number(fp);
	    break;
        case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
            pObjIndex->value[0]         = fread_number(fp);
            pObjIndex->value[1]         = fread_number(fp);
	    CHECK_POS(pObjIndex->value[2], liq_lookup(fread_word(fp)), "liq_lookup" );
            pObjIndex->value[3]         = fread_number(fp);
            if ( area_version < 1 )
                pObjIndex->value[4]     = fread_number(fp);
            else
	        pObjIndex->value[4]     = -1*skill_lookup(fread_word(fp));
            break;
	case ITEM_WAND:
	case ITEM_STAFF:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= fread_number(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[4]		= fread_number(fp);
	    break;
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
 	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[2]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[3]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[4]		= skill_lookup(fread_word(fp));
	    break;
	default:
            pObjIndex->value[0]         = fread_flag( fp );
            pObjIndex->value[1]         = fread_flag( fp );
            pObjIndex->value[2]         = fread_flag( fp );
            pObjIndex->value[3]         = fread_flag( fp );
            pObjIndex->value[4]         = fread_flag( fp );

	    break;
	}
	pObjIndex->level		= fread_number( fp );
        pObjIndex->weight               = fread_number( fp );
        pObjIndex->cost                 = fread_number( fp ); 

        /* condition */
        letter 				= fread_letter( fp );
	switch (letter)
 	{
	    case ('P') :		pObjIndex->condition = 100; break;
	    case ('G') :		pObjIndex->condition =  90; break;
	    case ('A') :		pObjIndex->condition =  75; break;
	    case ('W') :		pObjIndex->condition =  50; break;
	    case ('D') :		pObjIndex->condition =  25; break;
	    case ('B') :		pObjIndex->condition =  10; break;
	    case ('R') :		pObjIndex->condition =   0; break;
	    default:			pObjIndex->condition = 100; break;
	}
 
        for ( ; ; )
        {
            char letter;
 
            letter = fread_letter( fp );
 
            if ( letter == 'A' )
            {
                AFFECT_DATA *paf;
 
                paf                     = alloc_perm( sizeof(*paf) );
		paf->where		= TO_OBJECT;
                paf->type               = -1;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->whichaff           = AFF1;
                paf->location           = fread_number( fp );
                paf->modifier           = fread_number( fp );
                paf->bitvector          = 0;
                paf->next               = pObjIndex->affected;
                pObjIndex->affected     = paf;
                top_affect++;
            }

	    else if (letter == 'F')
            {
                AFFECT_DATA *paf;
 
                paf                     = alloc_perm( sizeof(*paf) );
		letter 			= fread_letter(fp);
		switch (letter)
	 	{
		case 'A':
                    paf->where          = TO_AFFECTS;
		    break;
		case 'I':
		    paf->where		= TO_IMMUNE;
		    break;
		case 'R':
		    paf->where		= TO_RESIST;
		    break;
		case 'V':
		    paf->where		= TO_VULN;
		    break;
		default:
            	    bug( "Load_objects: Bad where on flag set.", 0 );
            	   exit( 1 );
		}
                paf->type               = -1;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->whichaff           = AFF1;
                paf->location           = fread_number(fp);
                paf->modifier           = fread_number(fp);
                paf->bitvector          = fread_flag(fp);
                paf->next               = pObjIndex->affected;
                pObjIndex->affected     = paf;
                top_affect++;
            }
 
            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA *ed;
 
                ed                      = alloc_perm( sizeof(*ed) );
                ed->keyword             = fread_string( fp );
                ed->description         = fread_string( fp );
                ed->next                = pObjIndex->extra_descr;
                pObjIndex->extra_descr  = ed;
                top_ed++;
            }

            /* object owner */
	    else if ( letter == 'O' )
	    {
		pObjIndex->owner   	= fread_string( fp );
	    }
            /* Legacy pueblo support */
            else if ( letter == 'P' )
            {
		fread_string( fp );
            }
            
            /* OBJECT DISINTIGRATION TIMER - Aramina 8/23/02 */
            else if (letter == 'T')
            {
                pObjIndex->timedown           = fread_number(fp);
            }
 
            else
            {
                ungetc( letter, fp );
                break;
            }
        }
 
        iHash                   = vnum % MAX_KEY_HASH;
        pObjIndex->next         = obj_index_hash[iHash];
        obj_index_hash[iHash]   = pObjIndex;
        top_obj_index++;
        top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;   /* OLC */
        assign_area_vnum( vnum );                                   /* OLC */

    }
 
    return;
}

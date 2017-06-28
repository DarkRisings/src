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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "olc.h"
#include "interp.h"

extern long queststatus;

/*
 * Local functions.
 */
void	say_spell	args( ( CHAR_DATA *ch, int sn ) );
void do_hallowed_gate args( (CHAR_DATA *ch) );

/* imported functions */
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void 	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
bool    check_charmlist args( ( CHAR_DATA *ch, CHAR_DATA *vch ) );


/* COUNT THE CHARMIES A PERSON HAS, HOPEFULLY MORE RELIABLE THAN 
   CONSTANTLY ADDING AND SUBTRACTING  */
int CountCharmies(CHAR_DATA *master)
{
    CHAR_DATA *ch = NULL;
    int count = 0;

    for ( ch = char_list; ch != NULL; ch = ch->next ) {
        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == master )
           count++;
	}

    return(count);
}

/* Updates the charmed_by list.  If mob has been chamred too many
   times, the mob freaks.  Returns TRUE if the mob is to be destroyed
   by this charm, FALSE if it is not and the list is updated 
   successfully. */
bool check_charmlist ( CHAR_DATA *ch, CHAR_DATA *vch )
{
    char buf[MAX_STRING_LENGTH];
    strcpy(buf, vch->charmed_by);
    if ( !IS_NPC( vch ) ) return FALSE;
    if (  IS_NPC( ch )     ) return FALSE;
    /* const int below refers to the max PC name length + 1 */
    if ( strlen( vch->charmed_by ) + 20 > MAX_STRING_LENGTH/2 )
    {
        act("You run away!",ch,NULL,vch,TO_VICT);
        act("$N runs away!",ch,NULL,vch,TO_NOTVICT);
        act("$N runs away!",ch,NULL,vch,TO_CHAR);
        extract_char( vch, TRUE );
        return TRUE;
    }
    else
    {
        strcat( buf, ch->name );
        strcat( buf, " " );
        free_string( vch->charmed_by );
        vch->charmed_by = str_dup( buf );
        vch->logon = current_time;
        
        return FALSE;
    }
}

bool has_summonedpets( CHAR_DATA *ch )
{
    CHAR_DATA *pet;

    for ( pet = char_list; pet != NULL; pet = pet->next )
    {
       if ( IS_AFFECTED2(pet,AFF2_SUMMONEDPET)
       &&   pet->master == ch )
           return TRUE;
    }
    return FALSE;
}

/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
  int sn;
    
    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {

      if( skill_table[ sn ].name == NULL )
	break;

      if( strcasecmp( name, skill_table[ sn ].name ) == 0 )
	return sn;

      /*
	if ( skill_table[sn].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !str_prefix( name, skill_table[sn].name ) )
	    return sn;
      */
    }

    if (!str_prefix(name,"wereon"))
      return -1;
    else if (!str_prefix(name,"wereoff"))
      return -2;
    else if (!str_prefix(name,"vampon"))
      return -3;
    else if( !str_prefix(name,"veilwait"))
      return -4;
    else if( !str_prefix( name, "offer" ) )
      return -5;
    else if( !str_prefix( name, "frog" ) )
      return -6;
    else if( !str_prefix(name,"torpor"))
      return -20;

    return NO_SKILL;
}

int find_spell( CHAR_DATA *ch, const char *name )
{
    /* finds a spell the character can cast if possible */
    int sn, found = -1;

    if ( IS_NPC(ch) )
    {
        for ( sn = 0; sn < MAX_SKILL; sn++ )
        {
           if (skill_table[sn].name == NULL)
	        break;
           if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
            &&   !str_prefix( name, skill_table[sn].name ) )
                return sn;
        }
    }
    else
    {
        for ( sn = 0; sn < MAX_SKILL; sn++ )
        {
            if (skill_table[sn].name == NULL)
	        break;
            if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
            &&  !str_prefix(name,skill_table[sn].name))
            {
                if ( found == -1)
                    found = sn;
/*** old code ***/
                if (ch->level >= skill_table[sn].skill_level[ch->class]
                &&  ch->pcdata->learned[sn] > 0)
                    return sn;
/****************/
/*** new code ***
                if ( ch->pcdata->learned[sn] > 0 )
                {
                    if ( ch->level >= skill_table[sn].skill_level[ch->class] )
                        return sn;
                    else if (IS_SET(ch->shiftbits,SHIFT_POTENTIAL)
                    &&       skill_table[sn].were_level[ch->race] < MAX_LEVEL
                    &&       ch->level >= skill_table[sn].were_level[ch->race])
                        return sn;
                }
 ****************/
            }
        }
    }
    return found;
}



/*
 * Lookup a skill by slot number.
 * Used for object loading.
 *
 * Obsolete as of 5/27/09 gkl
 *
int slot_lookup( int slot )
{
    extern bool fBootDb;
    int sn;

    if ( slot <= 0 )
	return -1;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( slot == skill_table[sn].slot )
	    return sn;
    }

    if ( fBootDb )
    {
	bug( "Slot_lookup: bad slot %d.", slot );
	abort( );
    }

    return -1;
}*/



/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
    char buf  [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *	old;
	char *	new;
    };

    if( skill_table[ sn ].spell_fun == spell_detect_magic ||
	skill_table[ sn ].spell_fun == spell_detect_invis ||
	skill_table[ sn ].spell_fun == spell_detect_hidden )
      {
	/* this should eliminate verbal casting of detects */
	return;
      }

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "		},
	{ "ar",		"abra"		},
	{ "au",		"kada"		},
	{ "bless",	"fido"		},
	{ "blind",	"nose"		},
	{ "bur",	"mosa"		},
	{ "cu",		"judi"		},
	{ "de",		"oculo"		},
	{ "en",		"unso"		},
	{ "light",	"dies"		},
	{ "lo",		"hi"		},
	{ "mor",	"zak"		},
	{ "move",	"sido"		},
	{ "ness",	"lacri"		},
	{ "ning",	"illa"		},
	{ "per",	"duda"		},
	{ "ra",		"gru"		},
	{ "fresh",	"ima"		},
	{ "re",		"candus"	},
	{ "son",	"sabru"		},
	{ "tect",	"infra"		},
	{ "tri",	"cula"		},
	{ "ven",	"nofo"		},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]	= '\0';
    for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
    {
	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
	{
	    if ( !str_prefix( syl_table[iSyl].old, pName ) )
	    {
		strcat( buf, syl_table[iSyl].new );
		break;
	    }
	}

	if ( length == 0 )
	    length = 1;
    }

    sprintf( buf2, "$n utters the words, '%s'.", buf );
    sprintf( buf,  "$n utters the words, '%s'.", skill_table[sn].name );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( rch != ch )
	    act((!IS_NPC(rch) && ch->class==rch->class) ? buf : buf2,
	        ch, NULL, rch, TO_VICT );
    }

    return;
}



/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA *ch, CHAR_DATA *victim, int dam_type )
{
    int save, rroll;
    char buf[MAX_STRING_LENGTH];

    if ( ch != NULL
    &&   IS_AFFECTED3( ch, AFF3_SUPERCAST ) )
        return FALSE;

    if ( IS_NPC( victim ) )
        save = victim->level + 30;
    else
    {
        if ( class_table[ victim->class ].fMana )
            save = -1 * CONFIG_SAVES_FMANA_SCALEN * victim->saving_throw 
                    / CONFIG_SAVES_FMANA_SCALED;
        else
            save = -1 * CONFIG_SAVES_NOFMANA_SCALEN * victim->saving_throw 
                    / CONFIG_SAVES_NOFMANA_SCALED;
/*      save = -2 * victim->saving_throw; */
    }

    /* each level of difference equals 2% modifier to percentile throw */
    save += 2 * ( victim->level - level );

    /* res/vuln is a 5% modifier */
    switch( check_immune( victim, dam_type ) )
    {
        case IS_IMMUNE:
        {
            return TRUE;
        }
        case IS_RESISTANT:
        {
            save += 5;
            break;
        }
        case IS_VULNERABLE:
        {
            save -= 5;
            break;
        }
    }

    /* floor 5%, ceiling 90% saves in all cases */
    save = URANGE( 5, save, 90 );
    rroll = number_percent();

    if ( !IS_NPC(victim) 
    &&   IS_SET( victim->comm, COMM_VERBOSE )
    &&   get_trust(victim) > LEVEL_ADMIN )
    {
        sprintf(buf, "Chance for saving is %d%%, rolled %d.\n\r", save, rroll );
        send_to_char( buf, victim );
    }

    if ( rroll < save )
        return TRUE;
    else
    {
        /* caster affected by profane word has a harder time landing spells */
        if ( ch 
        &&   IS_AFFECTED3( ch, AFF3_PROFANE_WORD ) 
        &&   number_percent() < 25 )
        {
            send_to_char("A profane word disrupts your concentration!\n\r", ch);
            return TRUE;
        }
        else
            return FALSE;
    }
}

/* RT save for dispels */
bool saves_dispel( int dis_level, int spell_level, int duration)
{
    int save;
    
    if (duration == -1)
      spell_level += 1;  
      /* very hard to dispel permanent effects */

    save = 50 + (spell_level - dis_level) * 5;
    save = URANGE( 5, save, 85 );
    return number_percent( ) < save;
}

/* co-routine for dispel magic and cancellation */
bool check_dispel( int dis_level, CHAR_DATA *victim, int sn)
{
    AFFECT_DATA *af;

    if (is_affected(victim, sn))
    {
        for ( af = victim->affected; af != NULL; af = af->next )
        {
            if ( af->type == sn )
            {
                if (!saves_dispel(dis_level,af->level,af->duration)
                && (sn != gsn_peaches || number_percent() > 25 ) )
                {
                    affect_strip(victim,sn);
        	    if ( skill_table[sn].msg_off )
        	    {
            		send_to_char( skill_table[sn].msg_off, victim );
            		send_to_char( "\n\r", victim );
        	    }

                 /* Autowake if sleep wears off. */
                    if ( IS_NPC( victim )
                    &&   ( sn == gsn_sleep || sn == gsn_sap )
                    &&   victim->position != victim->pIndexData->default_pos )
                    {
                        if ( victim->pIndexData->default_pos == POS_STANDING )
                            do_stand( victim, "" );
                        else if ( victim->pIndexData->default_pos == POS_RESTING )
                            do_rest ( victim, "" );
                        else if ( victim->pIndexData->default_pos == POS_SITTING )
                            do_sit  ( victim, "" );

                    }

		    return TRUE;
		}
		else
		    af->level--;
            }
        }
    }
    return FALSE;
}

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_cast( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  void *vo;
  int mana;
  int sn;
  int target;
  int chance;
  
  if (IS_AFFECTED2(ch,AFF_GHOST)){
    send_to_char("You're a ghost!\n\r",ch);
    return;
  }

  if (IS_AFFECTED2(ch,AFF_SILENCE)){
    send_to_char("You have been silenced!\n\r",ch);
    return;
  }
  /*
   * Switched NPC's can cast spells, but others can't.
   */
  if ( IS_NPC(ch) && ch->desc == NULL)
    return;

  target_name = one_argument( argument, arg1 );
  one_argument( target_name, arg2 );

  if ( arg1[0] == '\0' )
    {
      send_to_char( "Cast which what where?\n\r", ch );
      return;
    }

  if ((sn = find_spell(ch,arg1)) < 1
      ||  skill_table[sn].spell_fun == spell_null
      || (!IS_NPC(ch) && (ch->level < skill_table[sn].skill_level[ch->class]
			  && ch->level < skill_table[sn].were_level[ch->race]))
      || get_skill(ch,sn) < 1)
    {
      send_to_char( "You don't know any spells of that name.\n\r", ch );
      return;
    }

    if ( ch->position < skill_table[sn].minimum_position )
    {
      send_to_char( "You can't concentrate enough.\n\r", ch );
      return;
    }
	
	if (!skill_table[sn].selectable && is_affected(ch,skill_lookup("tabula rasa"))) {
		send_to_char("A divine force prevents you from casting that spell.\n\r",ch);
		return;
	}
    
    if ( sn == gsn_veil )
        mana = ch->level * 10;
/* what is this */
    else if (ch->level + 2 == skill_table[sn].skill_level[ch->class])
    {
        bug( "whoaaaaaaaaaaaa nelly! %d", sn );
        mana = 50;
    }
    else
        mana = skill_table[sn].min_mana;
/*      mana = UMAX( skill_table[sn].min_mana,
        100 / ( 2 + ch->level - skill_table[sn].skill_level[ch->class] ) ); */

    /*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;
    target	= TARGET_NONE;
      
  switch ( skill_table[sn].target )
    {
    default:
      bug( "Do_cast: bad target for sn %d.", sn );
      return;

    case TAR_IGNORE:
      break;

    case TAR_CHAR_OFFENSIVE:

      chance = number_range(1,100);

      if ( arg2[0] == '\0' )
	{
	  if ( ( victim = ch->fighting ) == NULL )
	    {
	      send_to_char( "Cast the spell on whom?\n\r", ch );
	      return;
	    }
	}
      else
	{
	  if ( ( victim = get_char_room( ch, target_name, TRUE ) ) == NULL )
	    {
	      send_to_char( "They aren't here.\n\r", ch );
	      return;
	    }

	  if( !can_see( ch, victim ) )
	    {
	      send_to_char( "You can't see your target!\n\r", ch );
	      return;
	    }
	}

      if ( !IS_NPC(ch) )
	{

	  if (is_safe(ch,victim) && victim != ch)
	    {
	      if (IS_AFFECTED2(victim,AFF_GHOST))
		send_to_char("Attacking a ghost?  That's a silly idea!\n\r",
			     ch);
	      else
		send_to_char("Not on that target.\n\r",ch);
	      return; 
	    }
	  check_killer(ch,victim);
	}

      /* attacking a pet causes the pet to fade */
      if ( ( ch->pet && ch->pet == victim ) )
        {
          nuke_pets( ch );
          return;
        }
      else if ( IS_NPC( victim ) 
           &&   IS_AFFECTED2( victim, AFF2_SUMMONEDPET ) 
           &&   victim->master == ch )
      {
      /* Attacking one multipet will only cause that specific pet to fade */
         stop_follower( victim );
         return;
      }

      if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	{
	  send_to_char( "You can't do that on your own follower.\n\r",
			ch );
	  return;
	}

      if (IS_AFFECTED2(victim,AFF_GHOST)){
	send_to_char("You can't do that to a ghost!\n\r",ch);
	return;
      }

    if ( IS_AFFECTED2(victim,AFF_REFLECT) )
    {
        if (chance <= CONFIG_REFLECT_CHANCE)
        {
            vo = (void *) ch;
            target = TARGET_CHAR;
/*          ch->fightlag = FIGHT_LAG; */
            if ( !IS_NPC(ch) )
            {
                ch->pcdata->lastfight = current_time;
                if ( !IS_NPC(victim) && ch != victim )
                    ch->pcdata->lastpk = current_time;
            }
            send_to_char("Your magic is reflected!\n\r",ch);
            act( "$n's magic is reflected!", ch, NULL, victim, TO_VICT );
            break;
        }
    }

    if ( IS_AFFECTED2(victim,AFF_ABSORB) && ch != victim)
    {
        if (chance < 21)
        {
            say_spell( ch, sn );
/*          ch->fightlag = FIGHT_LAG; */
            if ( !IS_NPC(ch) )
            {
                ch->pcdata->lastfight = current_time;
                if ( !IS_NPC(victim) && ch != victim )
                    ch->pcdata->lastpk = current_time;
            }
            victim->mana += skill_table[sn].min_mana;
            WAIT_STATE( ch, skill_table[sn].beats );
            send_to_char( 
                "Your magic falls before a circle of protection.\n\r", ch);
            act( "$N's magic falls before a circle of protection.\n\r",
                victim, NULL, ch, TO_NOTVICT );
            send_to_char("You feel energized.\n\r",victim);
            return;
        }
    }

      vo = (void *) victim;
      target = TARGET_CHAR;
/*    ch->fightlag = FIGHT_LAG; */
      if ( !IS_NPC(ch) )
      {
          ch->pcdata->lastfight = current_time;
          if ( !IS_NPC(victim) && ch != victim )
              ch->pcdata->lastpk = current_time;
      }

      break;

    case TAR_CHAR_DEFENSIVE:
      if ( arg2[0] == '\0' )
	{
	  victim = ch;
	}
      else
	{
	  if ( ( victim = get_char_room( ch, target_name, TRUE ) ) == NULL )
	    {
	      send_to_char( "They aren't here.\n\r", ch );
	      return;
	    }

	  if( !can_see( ch, victim ) )
	    {
	      send_to_char( "You can't see your target!\n\r", ch );
	      return;
	    }
	}

      if (IS_AFFECTED2(victim,AFF_GHOST) && (sn != gsn_resurrect)){ 
	send_to_char("You can't do that to a ghost!\n\r",ch);
	return;
      }

      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_CHAR_SELF:
      if ( arg2[0] != '\0' && !is_name( target_name, ch->name ) )
	{
	  send_to_char( "You cannot cast this spell on another.\n\r", ch );
	  return;
	}

      vo = (void *) ch;
      target = TARGET_CHAR;
      break;

    case TAR_OBJ_INV:
      if ( arg2[0] == '\0' )
	{
	  send_to_char( "What should the spell be cast upon?\n\r", ch );
	  return;
	}

      if ( ( obj = get_obj_carry( ch, target_name, ch, TRUE ) ) == NULL )
	{
	  send_to_char( "You are not carrying that.\n\r", ch );
	  return;
	}

      vo = (void *) obj;
      target = TARGET_OBJ;
      break;

    case TAR_OBJ_CHAR_OFF:
      if (arg2[0] == '\0')
	{
	  if ((victim = ch->fighting) == NULL)
	    {
	      send_to_char("Cast the spell on whom or what?\n\r",ch);
	      return;
	    }
	
	  target = TARGET_CHAR;
	}
      else if ((victim = get_char_room(ch,target_name, TRUE)) != NULL)
	{
	  target = TARGET_CHAR;
	}

      if (target == TARGET_CHAR) /* check the sanity of the attack */
	{
	  /*
	  if( !can_see( ch, victim ) )
	    {
	      send_to_char( "You can't see your target!\n\r", ch );
	      return;
	    }
	  */

	  if(is_safe_spell(ch,victim,FALSE) && victim != ch)
	    {
	      send_to_char("Not on that target.\n\r",ch);
	      return;
	    }

	  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
            {
	      send_to_char( "You can't do that on your own follower.\n\r",
			    ch );
	      return;
            }

	  if (!IS_NPC(ch))
	    check_killer(ch,victim);

	  if (IS_AFFECTED2(victim,AFF_GHOST)){
	    send_to_char("You can't do that to a ghost!\n\r",ch);
	    return;
	  }

	  vo = (void *) victim;
 	}
      else if ((obj = get_obj_here(ch,target_name, TRUE)) != NULL)
	{
	  vo = (void *) obj;
	  target = TARGET_OBJ;
	}
      else
	{
	  send_to_char("You don't see that here.\n\r",ch);
	  return;
	}
      break; 

    case TAR_OBJ_CHAR_DEF:
      if (arg2[0] == '\0')
        {
	  vo = (void *) ch;
	  target = TARGET_CHAR;                                                 
        }
      else if ((victim = get_char_room(ch,target_name, TRUE)) != NULL)
        {
	  if (IS_AFFECTED2(victim,AFF_GHOST)){
	    send_to_char("You can't do that to a ghost!\n\r",ch);
	    return;
	  }

	  if( !can_see( ch, victim ) )
	    {
	      send_to_char( "You can't see your victim!\n\r", ch );
	      return;
	    }

	  vo = (void *) victim;
	  target = TARGET_CHAR;
	}
      else if ((obj = get_obj_carry(ch,target_name,ch, TRUE)) != NULL)
	{
	  vo = (void *) obj;
	  target = TARGET_OBJ;
	}
      else
	{
	  send_to_char("You don't see that here.\n\r",ch);
	  return;
	}
      break;
    }
	    
  if ( !IS_NPC(ch) && ch->mana < mana )
    {
      send_to_char( "You don't have enough mana.\n\r", ch );
      return;
    }
      
  if ( str_cmp( skill_table[sn].name, "ventriloquate" ) &&
       str_cmp( skill_table[sn].name, "entrance" ) )
    say_spell( ch, sn );
     
	if(is_affected(ch,skill_lookup("godspeed"))) {
	WAIT_STATE(ch, 5);
	}
	else {
  WAIT_STATE( ch, skill_table[sn].beats );
  }
      
  if ( number_percent( ) > get_skill(ch,sn) )
    {
      switch (number_range(0, 4)) {
      case 0:
      default:
	send_to_char( "You mispronounce an arcane syllable.\n\r", ch );
	break;
      case 1:
	send_to_char( "Your attempts at spellcasting fail.  You should go practice it.\n\r", ch);
	break;
      }
	
      check_improve(ch,sn,FALSE,1);
      ch->mana -= mana / 2;
    }
  else
    {
      int casting_level = ch->level;
      ch->mana -= mana;
      if ( IS_AFFECTED2(ch,AFF_FOCUS) )
          casting_level = ch->level + 5;
      if ( ch != (CHAR_DATA *) vo
      &&  !IS_NPC(ch)
      &&  !class_table[ch->class].fMana
      &&  skill_table[sn].target != TAR_IGNORE )
          casting_level = casting_level -2;                

      (*skill_table[sn].spell_fun) ( sn, casting_level, ch,vo,target);
      check_improve(ch,sn,TRUE,1);
    }

  if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
       ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
      &&   victim != ch
      &&   victim->master != ch)
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	  vch_next = vch->next_in_room;
	  if ( victim == vch && victim->fighting == NULL )
	    {	check_killer(victim,ch);
	    multi_hit( victim, ch, TYPE_UNDEFINED );
	    break;
	    }
	}
    }

  return;
}
/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
    void *vo = (void *) victim;
    int target = TARGET_NONE;

    if ( sn <= 0 )
	return;

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
	bug( "Obj_cast_spell: bad sn %d.", sn );
	return;
    }

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( victim == NULL )
	    victim = ch->fighting;
	if ( victim == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	if (is_safe(ch,victim) && ch != victim)
	{
	    send_to_char("Something isn't right...\n\r",ch);
	    return;
	}
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

/* THIS LOOKS STUPID */
    case TAR_CHAR_DEFENSIVE:
	if ( victim == NULL )
	    victim = ch;
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;
    case TAR_CHAR_SELF:
    if(victim != ch) {
		send_to_char( "You cannot cast that spell on others.\n\r",ch);
		return;
	     }
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;
    case TAR_OBJ_INV:
	if ( obj == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:

      if( obj == NULL )
	{
	  if( victim == NULL )
	    {
	      if( ch->fighting != NULL )
		{
		  victim = ch->fighting;
		}
	      else
		{
		  send_to_char( "You can't do that.\n\r", ch );
		  return;
		}
	    }
	  else
	    {
	      if( ch != victim && is_safe_spell( ch, victim, FALSE ) )
		{
		  send_to_char( "Something isn't right...\n\r", ch );
		  return;
		}

	      vo = (void*) victim;
	      target = TARGET_CHAR;
	    }
	}
      else
	{
	  vo = (void*) obj;
	  target = TARGET_OBJ;
	}

      break;

      /*
        if ( victim == NULL && obj == NULL)
	  {
	    if (ch->fighting != NULL)
	      {
                victim = ch->fighting;
	      }
	    else
	      {
		send_to_char("You can't do that.\n\r",ch);
		return;
	      }

	    if (victim != NULL)
	      {
		if (is_safe_spell(ch,victim,FALSE) && ch != victim)
		  {
		    send_to_char("Somehting isn't right...\n\r",ch);
		    return;
		  }

		vo = (void *) victim;
		target = TARGET_CHAR;
	      }
	    else
	      {
	    	vo = (void *) obj;
	    	target = TARGET_OBJ;
	      }
	  }
        break;
      */


    case TAR_OBJ_CHAR_DEF:
	if (victim == NULL && obj == NULL)
	{
	    vo = (void *) ch;
	    target = TARGET_CHAR;
	}
	else if (victim != NULL)
	{
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	}
	else
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	
	break;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) ( sn, level, ch, vo,target);

    

    if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		check_killer(victim,ch);
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}



/*
 * Spell functions.
 */
void spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 12 );
    damage_old( ch, victim, dam, sn,DAM_ACID,TRUE);
    return;
}

void spell_cyclone( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, chance;
    bool knocked = FALSE;

/* need to sprawl BEFORE damage is done so that the the victim is already
   lagged when the damage is done.  this prevents wimpying out due to the
   damage of the knockdown */
    if ( victim != ch
    &&   victim->position > POS_FLATFOOTED
    &&  !IS_SET(victim->imm_flags,IMM_LAGSKILLS)
    &&  !saves_spell(level, ch, victim, DAM_WIND ) )
    {
        chance = level;
        chance += (get_curr_stat( ch, STAT_INT ) + 
                   get_curr_stat( ch, STAT_WIS ))/2;
        chance -= get_curr_stat(victim,STAT_DEX);
        chance += (ch->level - victim->level);
        if ( IS_AFFECTED( victim, AFF_FLYING ) )
          chance += 11 * chance / 10;
        if ( number_percent() < chance )
        {
            /* set the lag */
            WAIT_STATE(victim,PULSE_VIOLENCE*2);
            victim->position = POS_SPRAWLED;
            knocked = TRUE;
        }
    }

    /* do the damage */
    dam = dice( level, 10 );
    damage_old( ch, victim, dam, sn, DAM_WIND, TRUE);

    /* send the knockdown messages */
    if ( knocked )
    {
        act( "$n's blast of wind sends you flying off your feet!", 
            ch, NULL, victim, TO_VICT );
        act( "Your blast of wind sends $N flying!",
            ch, NULL, victim, TO_CHAR );
        act("$n's blast of wind knocks $N off $S feet.",
            ch, NULL, victim, TO_NOTVICT );
    }
    return;
}



void spell_resurrect( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED2(victim,AFF_GHOST)){
        REMOVE_BIT(victim->affected_by2, AFF_GHOST);
        REMOVE_BIT(victim->affected_by, AFF_PASS_DOOR);
        send_to_char("It's alive!\n\r",victim);
        send_to_char("You have restored them to life!\n\r",ch);
	victim->hit = victim->max_hit/2;
	victim->mana = victim->max_mana/2;
	victim->move = victim->max_move/2;
        }
    else {
        send_to_char("That person is already alive!\n\r",ch);
        send_to_char("You are already alive!\n\r",victim);
        }
    return;
}


void spell_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already armored.\n\r",ch);
	else
	  act("$N is already armored.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if(is_affected(victim,skill_lookup("displace")))
    {
        if (victim == ch)
          send_to_char("You already seem protected in this manner.\n\r",ch);
        else
          act("$N is already protected in this manner.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 24;
    af.modifier  = -20;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel someone protecting you.\n\r", victim );
    if ( ch != victim )
	act("$N is protected by your magic.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_wanted( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int chance;

  if( !victim )
    {
      send_to_char("Who do you want to cast the spell on?\n\r",ch);
      return;
    }

  if( IS_NPC( victim ) )
    {
      send_to_char( "A branding such as this would be an abuse of power, Officer.\n\r", ch );
      return;
    }

  if (IS_AFFECTED2(victim, AFF_WANTED))
    {
      if (victim == ch)
	send_to_char("You are already WANTED!.\n\r",ch);
      else
	act("$N is already WANTED.",ch,NULL,victim,TO_CHAR);
      return;
    }

  chance = number_range( 0, 100 );
  if( saves_spell( level, ch, victim, DAM_OTHER ) && chance > 20 )
    {
      send_to_char( "Your suspect evades you!\n\r", ch );
      return;
    }

  af.where	 = TO_AFFECTS;
  af.type      = sn;
  af.level	 = level;
  af.duration  = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_WANTED;
  affect_to_char2( victim, &af );

  send_to_char( "You are branded a WANTED criminal!\n\r", victim );

  if ( ch != victim )
    act("$N is branded a WANTED criminal!",ch,NULL,victim,TO_CHAR);

  return;
}

/* psi spells */
void spell_displace(int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected( ch, sn ))
    {
       if (victim == ch)
         send_to_char("You already feel out of place.\n\r",ch);
       else
        act("$N is already displaced.",ch,NULL,victim, TO_CHAR);
       return;
    }
    if(is_affected(victim,skill_lookup("armor")))
    {
        if (victim == ch)
          send_to_char("You already seem protected in this manner.\n\r",ch);
        else
          act("$N is already protected in this manner.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_AC;
    af.modifier  = -20;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n shimmers and displaces.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You feel slightly out of place.\n\r", victim );
    return;
}


void spell_steel_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected( ch, sn ))
    {
       if (victim == ch)
         send_to_char("Your skin is already tempered steel.\n\r", ch);
       else
        act("$N is already hard as can be.",ch,NULL,victim, TO_CHAR);
       return;
    }
    if(is_affected(victim,skill_lookup("stone skin")))
    {
        if (victim == ch)
          send_to_char("You already seem protected in this manner.\n\r",ch);
        else
          act("$N is already protected in this manner.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where    = TO_AFFECTS;
    af.type     = sn;
    af.level    = level;
    af.duration = level;
    af.location = APPLY_AC;
    af.modifier = -40;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's skin becomes tempered steel.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your skin becomes tempered steel.\n\r", victim );
    return;
}


void spell_mental_barrier( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected( ch, sn ))
    {
       if (victim == ch)
         send_to_char("You already have a barrier in place.\n\r", ch);
       else
        act("$N has already made a barrier.",ch,NULL,victim,TO_CHAR);
       return;
    }
    if(is_affected(victim,skill_lookup("shield")))
    {
        if (victim == ch)
          send_to_char("You already seem protected in this manner.\n\r",ch);
        else
          act("$N is already protected in this manner.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_AC;
    af.modifier  = -20;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act("$n constructs a mental barrier.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You construct a mental barrier.\n\r", victim );
    return;
}


void spell_biofeedback( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_SANCTUARY))
    {
       if (victim == ch)
         send_to_char("You are already in biofeedback.\n\r",ch);
       else
         act("$N is already in biofeedback.",ch, NULL,victim,TO_CHAR);
       return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( victim, &af );
    act("$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM);
    send_to_char("You are surrounded by a white aura.\n\r", victim);
    return;
}


void spell_adrenaline_rush(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim =(CHAR_DATA *) vo;
    AFFECT_DATA af;

    if(is_affected( victim, sn )|| IS_AFFECTED(victim,AFF_HASTE)
    || IS_SET(victim->off_flags,OFF_FAST))
    {
      if (victim ==ch )
        send_to_char("You can't handle anymore adrenaline!\n\r",ch);
      else
        act("$N is already moving as fast as $E can.",
           ch,NULL,victim,TO_CHAR);
      return;
    }
    if (IS_AFFECTED(victim,AFF_SLOW))
    {
       if (!check_dispel(level,victim,skill_lookup("slow"))
       || (!check_dispel(level,victim,skill_lookup("warp time"))))
       {
          if (victim != ch)
              send_to_char("The rush of adrenaline had no affect\n\r",ch);
          send_to_char("The rush of adrenaline had no affect.\n\r",victim);
          return;
       }
       act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
       return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = AFF_HASTE;

    af.location  = APPLY_DEX;
    affect_to_char(victim,&af);

    if ( !is_affected( victim, gsn_divine_focus ) )
    {
      af.modifier  = 2;
      af.location  = APPLY_CON;
      affect_to_char(victim,&af);
    }
    send_to_char("You feel adrenaline rushing through your veins!\n\r",victim);
    act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return;
}


void spell_enhanced_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim =(CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn )
    ||   is_affected( victim, skill_lookup("giant strength") )
    ||   is_affected( victim, gsn_divine_focus ) )
    {
       if (victim == ch)
         send_to_char("You are already as strong as you can get!\n\r",ch);
       else
         act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR);
       return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char("Your muscles surge with enhanced strength!\n\r", victim );
    act("$n's muscles surge with heightened strength.",victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_levitation(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FLYING))
    {
       if (victim == ch)
         send_to_char("You are already levitating.\n\r",ch);
       else
         act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
       return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char("You begin to levitate in the air.\n\r", victim );
    act("$n begins to levitate before your eyes!",victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_awe( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *vch;
  int mlevel = 0;
  int count = 0;
  int high_level = 0;
  int chance;
  AFFECT_DATA af;

  /*get sum of all mobile levels in the room*/
  for (vch = ch->in_room->people; vch != NULL; vch =vch->next_in_room)
    {
      if (vch->position == POS_FIGHTING)
	{
	  count++;
	  if (IS_NPC(vch))
            mlevel += vch->level;
	  else
            mlevel +=vch->level/2;
	  high_level = UMAX(high_level,vch->level);
	}
    }
  /*compute chance of stopping combat*/
  chance = 4 * level - high_level + 2 * count;

  if (IS_IMMORTAL(ch)) /*always works*/
    mlevel = 0;
  if (number_range(0, chance) >= mlevel) /*hard to stop large fights*/
    {
      for (vch =ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	  if( can_see( ch, vch ) )
	    {
	      if (IS_NPC(vch) && (IS_SET(vch->imm_flags,IMM_MENTAL)||
				  IS_SET(vch->act_bits,ACT_UNDEAD)))
		continue;

	      if (IS_AFFECTED(vch,AFF_CALM) 
              ||  IS_AFFECTED(vch,AFF_BERSERK)
              ||  is_affected(vch,skill_lookup("frenzy"))
              ||  IS_WIZI_AREA(ch, vch) )
		continue;

	      send_to_char("A sense of awe overcomes you.\n\r",vch);

	      if (vch->fighting || vch->position == POS_FIGHTING)
		stop_fighting(vch,FALSE);

	      af.where = TO_AFFECTS;
	      af.type = sn;
	      af.level = level;
	      af.duration = level/10;
	      af.location = APPLY_DAMROLL;
	      if (!IS_NPC(vch))
		af.modifier = -5;
	      else
		af.modifier = -2;
	      af.bitvector = AFF_CALM;
	      affect_to_char(vch,&af);

	      af.location = APPLY_HITROLL;
	      affect_to_char(vch,&af);
	    }
	}
    }
}


void spell_psychic_drain(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) 
    || saves_spell( level, ch, victim,DAM_MENTAL) )
    {
        send_to_char("Your spell had no effect.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED3(victim,AFF3_SOLEMN)
    &&   number_percent() > CONFIG_SOLEMN_VOWS_CHANCE )
    {
      send_to_char("The power of your vows shield you from taint.\n\r", victim);
      if ( ch != victim )
        act("$N's vows protect $M from your spell.", ch, NULL, victim, TO_CHAR);
      return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_STR;
    af.modifier  = -1*(level/5);
    af.bitvector = AFF_WEAKEN;
    affect_to_char( victim, &af );
    send_to_char("You feel drained of strength.\n\r", victim );
    act("$n looks drained of strength.",victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_warp_time( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    if (is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
    {
       if (victim == ch)
         send_to_char("You cant warp any slower!\n\r",ch);
       else
         act("$N cant warp any slower than that.",ch,NULL,victim,TO_CHAR);
       return;
    }

    if (saves_spell(level, ch, victim, DAM_MENTAL)
    || IS_SET(victim->imm_flags,IMM_MENTAL) )
    {
       if (victim != ch)
           send_to_char("Nothing seemed to happen.\n\r",ch);
       send_to_char("You feel your senses warp momentarily.\n\r",victim);
       return;
    }

    if ( IS_AFFECTED3(victim,AFF3_SOLEMN)
    &&   number_percent() > CONFIG_SOLEMN_VOWS_CHANCE )
    {
      send_to_char("The power of your vows shield you from taint.\n\r", victim);
      if ( ch != victim )
        act("$N's vows protect $M from your spell.", ch, NULL, victim, TO_CHAR);
      return;
    }

    if (IS_AFFECTED(victim,AFF_HASTE))
    {
      if (!check_dispel(level,victim,skill_lookup("haste"))
      || (!check_dispel(level,victim,skill_lookup("adrenaline rush"))))
      {
         if (victim != ch)
             send_to_char("You failed.\n\r",ch);
         send_to_char("You feel your senses momentarily warped.\n\r",victim);
         return;
      }
      act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
      return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_DEX;
    af.modifier  = -1-(level >= 18)-(level >= 25)-(level >= 32);
    af.bitvector = AFF_SLOW;
    affect_to_char3( victim, &af );
    send_to_char("You feel your sense of time being w a r p e d...\n\r",victim);
    act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_mind_whip( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED(victim,AFF_CURSE)
    ||  saves_spell(level, ch, victim, DAM_MENTAL) )
    {
        send_to_char("Your spell had no effect.\n\r",ch);
        return;
    }

    if ( IS_AFFECTED3(victim,AFF3_SOLEMN)
    &&   number_percent() > CONFIG_SOLEMN_VOWS_CHANCE )
    {
      send_to_char("The power of your vows shield you from taint.\n\r", victim);
      if ( ch != victim )
        act("$N's vows protect $M from your spell.", ch, NULL, victim, TO_CHAR);
      return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/3;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1*(level/8);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level/8;
    affect_to_char( victim, &af );

    send_to_char("Your mind feels tortured.\n\r",victim);
    if ( ch != victim )
        act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_smoke( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    short chance;

    chance = number_range(0,100);

    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED(victim,AFF_BLIND))
        return;


    if ( saves_spell( level, ch, victim, DAM_OTHER ) ) 
        {
          if (chance > 20)
             {
               act("Your smoke screen fizzles.",ch, 0, 0, TO_CHAR);
               return;
             }
        }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1;
    af.location  = APPLY_DEX;
    af.modifier  = -5;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );


    af.location  = APPLY_HITROLL;
    af.modifier  = -level/8;
    affect_to_char( victim, &af );
    send_to_char("You are surrounded by smoke!\n\r",victim);   
    if ( ch != victim )
        act("$N is surrounded by smoke!",ch,NULL,victim,TO_CHAR);
    return;

}

void spell_bolas( int sn, int level, CHAR_DATA* ch, void* vo, int target )
{
  CHAR_DATA* victim;
  AFFECT_DATA af;
  int chance;

  victim = ( CHAR_DATA* )vo;
  chance = number_percent();

  act( "You whirl you arm in a spinning motion and point at $N.", ch, NULL, victim, TO_CHAR );
  act( "$n whirls $s arm in a spinning motion and points at you.", ch, ch, victim, TO_VICT );
  act( "$n whirls $s arm in a spinning motion and points at $N.", ch, ch, victim, TO_NOTVICT );

  if( is_affected( victim, gsn_bolas ) )
    {
      send_to_char( "They're already tangled up.\n\r", ch );
      return;
    }

  if( saves_spell( level, ch, victim, DAM_OTHER ) )
    {
      if( chance > 20 )
	{
	  send_to_char( "Your bolas miss their target.\n\r", ch );
	  send_to_char( "The bolas miss you.\n\r", victim );
	  return;
	}
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 10;
  af.location = APPLY_MOVE;
  af.modifier = -( victim->max_move / 2 );
  af.bitvector = 0;
  affect_to_char3( victim, &af );

  act( "Bolas fly through the air and entwine themselves around $N.", ch, NULL, victim, TO_CHAR );
  act( "Bolas fly through the air and entwine themselves around $N.", ch, NULL, victim, TO_NOTVICT );
  send_to_char( "Bolas fly through the air and entwine themselves around you!\n\r", victim );

  victim->move = UMIN( victim->move, victim->max_move );
}

void spell_terror( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    short chance;

    chance = number_range(0,100);

    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED(victim,AFF_CURSE))
        return;
 
    if ( IS_AFFECTED2( victim, AFF_TERROR ) )
    {
        send_to_char("The victim is already terrorized.\n\r",ch);
        return;
    }
 
    if (IS_AFFECTED3(victim,AFF_ORDAIN))
	{
	send_to_char("Those who hold the power of the divine cannot have fear instilled in their heart.\n\r",ch);
	send_to_char("The courage given to you by your god may not be infringed upon.\n\r",victim);
		return;
	}
    if ( saves_spell( level, ch, victim, DAM_OTHER ) ) 
        {
          if (chance > 20)
             {
               act("You fail in terrorizing your victim.",ch, 0, 0, TO_CHAR);
               return;
             }
        }

    af.where     = TO_AFFECTS;
    af.type      = gsn_terror;
    af.level     = level;
    af.duration  = 1;
    af.location  = APPLY_DEX;
    af.modifier  = -5;
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );


    af.whichaff  = AFF2;
    af.location  = APPLY_HITROLL;
    af.modifier  = -level/8;
    af.bitvector = AFF_TERROR;
    affect_to_char2( victim, &af );
    send_to_char("You are struck with a surge of terror!\n\r",victim);   
    if ( ch != victim )
        act("$N eyes widen in panic-stricken terror!",ch,NULL,victim,TO_CHAR);
    return;

}

void spell_twrath( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;
  short chance;

  chance = number_range(0,100);

  victim = (CHAR_DATA *) vo;

  if ( saves_spell( level, ch, victim, DAM_OTHER ) ) 
    {
      if (chance > 20)
	{
	  act("You fail to call Tyrin's wrath.",ch, 0, 0, TO_CHAR);
	  return;
	}
    }

  if (!IS_AFFECTED(victim,AFF_CURSE))
    {
      af.where     = TO_AFFECTS;
      af.type      = sn;
      af.level     = level;
      af.duration  = 2;
      af.location  = APPLY_STR;
      af.modifier  = -5;
      af.bitvector = AFF_CURSE;
      affect_to_char( victim, &af );
      af.location  = APPLY_HITROLL;
      af.modifier  = -level/8;
      affect_to_char( victim, &af );
      send_to_char("You feel Tyrin's Wrath upon you!\n\r",victim);   
      if ( ch != victim )
	act("$N feels the curse of Tyrin's Wrath!",ch,NULL,victim,TO_CHAR);
    }
    
  if( !IS_SET( victim->affected_by, AFF_SHIFT_PERM ) )
    {
      do_unshift(victim, "auto");
    }
  else
    {
      send_to_char( "Even Tyrin's Wrath cannot overcome the effect of the moon.\n\r", ch );
    }
  
  return;

}

void spell_domination(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_safe(ch,victim)){
        if (IS_AFFECTED2(victim,AFF_GHOST))
            send_to_char("Dominate a ghost?\n\r",ch);
        return;
        }
    if ( victim == ch )
    {
       send_to_char("You like yourself even better!\n\r",ch);
       return;
    }

    if (IS_AFFECTED(victim,AFF_CHARM)
    ||  IS_AFFECTED(ch,AFF_CHARM)
    ||  IS_SET(victim->imm_flags,IMM_CHARM)
    ||  IS_SET(victim->imm_flags,IMM_MENTAL)
    ||  (victim->level > 50)
    ||  saves_spell( level,ch,victim,DAM_MENTAL))
        {
          send_to_char("Your domination had no effect.\n\r",ch);
          return;
        }

/*
    if (ch->charmies >= ch->level/5)
*/
    if (CountCharmies(ch) >= ch->level/5)
    {
        send_to_char("You can't dominate anything else yet.\n\r",ch);
        return;
    }

    if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
    {
        send_to_char(
          "The mayor doesnt allow that within the city limits.\n\r",ch);
        return;
    }

    if (victim->master)
        if ( stop_follower( victim ) )
/*          This won't report a message, but it should never really happen;
            a person would have to be able to charm someone else's pet for
            this to pop up. */
	    return;

    if ( !check_charmlist( ch, victim ) )
    {
        add_follower( victim, ch );
        victim->leader = ch;
        af.where     = TO_AFFECTS;
        af.type      = sn;
        af.level     = level;
        af.duration  = number_fuzzy( level/4);
        af.location  = 0;
        af.modifier  = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char (victim, &af );
        ch->charmies += 1;
        act("Isn't $n just so nice?",ch,NULL,victim, TO_VICT);
        if ( ch != victim )
            act("$N looks at you with a dull look in his eyes.",
                 ch,NULL,victim,TO_CHAR);
    }
    return;
}


void spell_psychic_heal(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(3,8)+level-6;
    victim->hit = UMIN(victim->hit + heal, victim->max_hit);
    send_to_char("You feel better.\n\r",victim);
    if ( ch != victim )
       send_to_char("Ok.\n\r",ch);
    return;
}


void spell_major_heal(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA* captain = NULL;
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  victim->hit = UMIN(victim->hit + 200,victim->max_hit);

  if( !IS_NPC( victim ) && !IS_NPC( ch ) &&
      IS_SET( victim->act_bits, PLR_QUEST ) && IS_SET( ch->act_bits, PLR_QUEST ) &&
      !IS_SET( queststatus, QUEST_NO_HEALING_SCORE ) &&
      !IS_SET( queststatus, QUEST_NO_SCORING ) )
		
    {

      ch->pcdata->questscore += 100;

      captain = get_char_world( ch, ch->pcdata->quest_team_captain, TRUE );

      if( captain )
	{
	  captain->pcdata->quest_team_score += 100;
	}
       
    }

  send_to_char("You feel a surge of energy!\n\r",victim);

  if (ch != victim)
    act("$N glows with a surge of energy!",ch,NULL,victim,TO_CHAR);

  return;
}


void spell_bless( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;
	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	{
	    act("$p is already blessed.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	
	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= level/10;
	af.location	= APPLY_SAVES;
	af.modifier	= -1;
	af.bitvector	= ITEM_BLESS;
	affect_to_obj(obj,&af);

	act("$p glows with a holy aura.",ch,obj,NULL,TO_ALL);

	if (obj->wear_loc != WEAR_NONE)
	    ch->saving_throw -= 1;
	return;
    }

    /* character target */
    victim = (CHAR_DATA *) vo;


    if ( IS_AFFECTED( victim, AFF_BLESS ) 
		|| IS_AFFECTED2(victim, AFF_INSPIRE))
    {
	if (victim == ch)
	  send_to_char("You are already blessed.\n\r",ch);
	else
	  act("$N has already been blessed.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 6+level;
    af.location  = APPLY_HITROLL;
    af.modifier  = level / 8;
    af.bitvector = AFF_BLESS;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 0 - level / 8;
    affect_to_char( victim, &af );
    send_to_char( "You feel righteous.\n\r", victim );
    if ( ch != victim )
	act("You grant $N the favor of your god.",ch,NULL,victim,TO_CHAR);
    return;
}


/* If you update the blindness spell, be aware that the judgment spell in
   templar.c should probably be updated as well */
void spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_BLIND)
    ||   IS_AFFECTED3(victim, AFF_DIRT)
    ||   saves_spell(level,ch,victim,DAM_OTHER) )
    {
        send_to_char("Your spell had no effect.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED3(victim,AFF3_SOLEMN)
    &&   number_percent() > CONFIG_SOLEMN_VOWS_CHANCE )
    {
      send_to_char("The power of your vows shield you from taint.\n\r", victim);
      if ( ch != victim )
        act("$N's vows protect $M from your spell.", ch, NULL, victim, TO_CHAR);
      return;
    }

    af.where     = TO_AFFECTS;
    af.whichaff  = AFF1;
    af.type      = sn;
    af.level     = level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    af.duration  = level/5;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );
    if ( IS_AFFECTED2(victim, AFF2_BLINDSIGHT) )
    {
      send_to_char( "Your vision becomes clouded!\n\r", victim );
      act("$n's vision appears to become clouded.",victim,NULL,NULL,TO_ROOM);
    }
    else
    {
      send_to_char( "You are blinded!\n\r", victim );
      act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
    }
    return;
}



void spell_battle_sorrow(int sn,int level, CHAR_DATA *ch, void *vo, int target)
{

    damage_old ( ch, ( CHAR_DATA *) vo, dice (2, 4) + level / 2,
                sn, DAM_SOUND,TRUE);
    return;
}



void spell_battlecry(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    damage_old( ch, ( CHAR_DATA *) vo, dice (2, 8) + level / 2,
               sn, DAM_SOUND,TRUE);
    return;
}



void spell_battle_wail( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    damage_old( ch, ( CHAR_DATA *) vo, dice (4, 8) + level / 2,
               sn, DAM_SOUND,TRUE);
    return;
}



void spell_deathsong( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    damage_old( ch, ( CHAR_DATA *) vo, dice (level, 8) + level / 2,
               sn, DAM_SOUND,TRUE);
    return;
}



void spell_sorrow(int sn,int level, CHAR_DATA *ch, void *vo, int target)
{
 
    damage_old ( ch, ( CHAR_DATA *) vo, dice (2, 4) + level / 2,
                sn, DAM_NEGATIVE,TRUE);
    return;
}
 
void spell_chaos(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    damage_old( ch, ( CHAR_DATA *) vo, dice (2, 8) + level / 2,
               sn, DAM_NEGATIVE,TRUE);
    return;
}
 
void spell_torment( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{ 
    damage_old( ch, ( CHAR_DATA *) vo, dice (4, 8) + level / 2,
               sn, DAM_NEGATIVE,TRUE);
    return;
}
 
void spell_ravage( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    damage_old( ch, ( CHAR_DATA *) vo, dice (level, 8) + level / 2,
               sn, DAM_NEGATIVE,TRUE);
    return;
}
 

void spell_burning_hands(int sn,int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 5,  5,  8, 11,	14,	17, 20, 23, 26, 29,
	29, 29, 30, 30,	31,	31, 32, 32, 33, 33,
	34, 34, 35, 35,	36,	36, 37, 37, 38, 38,
	39, 39, 40, 40,	41,	41, 42, 42, 43, 43,
	44, 44, 45, 45,	46,	46, 47, 47, 48, 48
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(1, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell(level, ch, victim,DAM_FIRE) )
	dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_FIRE,TRUE);
    return;
}



void spell_call_lightning( int sn, int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;

  if ( !IS_OUTSIDE(ch) )
    {
      send_to_char( "You must be out of doors.\n\r", ch );
      return;
    }

  if ( weather_info.sky < SKY_RAINING )
    {
      send_to_char( "You need bad weather.\n\r", ch );
      return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("Not in this room.\n\r",ch);
        return;
    }

  dam = dice(level/2, 8);

  send_to_char( "Thy God's lightning strikes your foes!\n\r", ch );
  act( "$n calls upon $s God's lightning to strike $s foes!",
       ch, NULL, NULL, TO_ROOM );

  for ( vch = char_list; vch != NULL; vch = vch_next )
    {
      vch_next	= vch->next;
      if ( vch->in_room == NULL )
	continue;
      if ( vch->in_room == ch->in_room )
	{
	  if ( vch != ch
	  &&  !is_same_group( vch, ch )
          &&  !is_safe_verb(ch, vch, FALSE) && !IS_IMMORTAL(vch) )
	    {
	      damage_old( ch, vch, saves_spell(level,ch,vch,DAM_LIGHTNING) 
			  ? dam / 2 : dam, sn,DAM_LIGHTNING,TRUE);
	    }
	  continue;
	}

      if ( vch->in_room->area == ch->in_room->area
	   &&   IS_OUTSIDE(vch)
	   &&   IS_AWAKE(vch) )
	send_to_char( "Lightning flashes in the sky.\n\r", vch );
    }

  return;
}

void spell_meteor( int sn, int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("Not in this room.\n\r",ch);
        return;
    }

    dam = dice(level/2, 10);

    send_to_char( "You call forth a shower of meteors!\n\r", ch );
    act( "$n calls forth a shower of Meteors!",
	ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_verb(ch, vch, FALSE) && !IS_IMMORTAL(vch) )
		damage_old( ch, vch, saves_spell(level,ch,vch,DAM_FIRE) 
		? dam / 2 : dam, sn,DAM_FIRE,TRUE);
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area
	&&   IS_OUTSIDE(vch)
	&&   IS_AWAKE(vch) )
	    send_to_char( "You see meteors streak through the sky.\n\r", vch );
    }

    return;
}

void spell_field( int sn, int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("Not in this room.\n\r",ch);
        return;
    }

    dam = dice(level/2, 10);

    send_to_char( "You send out a blinding field of energy!\n\r", ch );
    act( "$n sends out a blinding field of energy!",
	ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
        if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_verb(ch, vch, FALSE) && !IS_IMMORTAL(vch) )
		damage_old( ch, vch, saves_spell(level,ch,vch,DAM_MENTAL) 
		? dam / 2 : dam, sn,DAM_MENTAL,TRUE);
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area
	&&   IS_OUTSIDE(vch)
	&&   IS_AWAKE(vch) )
	    send_to_char( "You see a blinding white light in the distance.\n\r", vch
);
    }

    return;
}

void spell_wrath( int sn, int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("Not in this room.\n\r",ch);
        return;
    }

    dam = dice(level/2, 10);

    send_to_char( "You call forth the wrath of your God!\n\r", ch );
    act( "$n calls forth wrath of $s God!",
	ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_verb(ch, vch, FALSE) && !IS_IMMORTAL(vch) )
		damage_old( ch, vch, saves_spell(level,ch,vch,DAM_HOLY) 
		? dam / 2 : dam, sn,DAM_HOLY,TRUE);
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area
	&&   IS_OUTSIDE(vch)
	&&   IS_AWAKE(vch) )
	    send_to_char( "You hear an eerie thundering all around you.\n\r", vch
);
    }

    return;
}

void spell_brimstone( int sn, int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("Not in this room.\n\r",ch);
        return;
    }

    dam = dice(level/2, 11);

    send_to_char( "You call forth a shower of brimstone!\n\r", ch );
    act( "$n calls forth a shower of brimstone!",ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_verb(ch, vch, FALSE) && !IS_IMMORTAL(vch) )
		damage_old( ch, vch, saves_spell(level,ch,vch,DAM_NEGATIVE) 
		? dam / 2 : dam, sn,DAM_NEGATIVE,TRUE);
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area && 
	     IS_OUTSIDE(vch) &&
	     IS_AWAKE(vch) )
	  {
	    send_to_char( "You see the night sky light up with brimstone.\n\r", vch );
	  }
    }

    return;
}

/* RT calm spell stops all fighting in the room */

void spell_calm( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *vch;
  int mlevel = 0;
  int count = 0;
  int high_level = 0;    
  int chance;
  AFFECT_DATA af;

  /* get sum of all mobile levels in the room */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (vch->position == POS_FIGHTING)
	{
	  count++;
	  if (IS_NPC(vch))
	    mlevel += vch->level;
	  else
	    mlevel += vch->level/2;
	  high_level = UMAX(high_level,vch->level);
	}
    }

  /* compute chance of stopping combat */
  chance = 4 * level - high_level + 2 * count;

  if (IS_IMMORTAL(ch)) /* always works */
    mlevel = 0;

  if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
      for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
   	{
	  if( can_see( ch, vch ) )
	    {
	      if (IS_NPC(vch) && (IS_SET(vch->imm_flags,IMM_MAGIC) ||
				  IS_SET(vch->act_bits,ACT_UNDEAD)))
		return;

	      if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK)
		  ||  is_affected(vch,skill_lookup("frenzy")))
		return;
	    
	      send_to_char("A wave of calm passes over you.\n\r",vch);

	      if (vch->fighting || vch->position == POS_FIGHTING)
		stop_fighting(vch,FALSE);


	      af.where = TO_AFFECTS;
	      af.type = sn;
	      af.level = level;
	      af.duration = level/4;
	      af.location = APPLY_HITROLL;
	      if (!IS_NPC(vch))
		af.modifier = -5;
	      else
		af.modifier = -2;
	      af.bitvector = AFF_CALM;
	      affect_to_char(vch,&af);

	      af.location = APPLY_DAMROLL;
	      affect_to_char(vch,&af);
	    }
	}
    }
}

void spell_cancellation( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;
 
    level += 2;

    if ((!IS_NPC(ch) && IS_NPC(victim) && 
	 !(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) ) ||
        (IS_NPC(ch) && !IS_NPC(victim)) )
    {
	send_to_char("You failed.  Try dispel magic.\n\r",ch);
	return;
    }

    /* unlike dispel magic, the victim gets NO save */
 
    /* begin running through the spells */
 

    if( check_dispel( level, victim, gsn_haunt ) )
      found = TRUE;

    if( check_dispel( level, victim, gsn_pinch ) )
      found = TRUE;


    if (check_dispel(level,victim,skill_lookup("shadows")))
        found = TRUE;

    if( check_dispel(level,victim,gsn_veil ) )
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("ordain")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("libraverim")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("circle of protection")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("adrenaline rush")))
        found = TRUE;

    if (check_dispel(level,victim,gsn_divine_focus))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("chill touch")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("awe")))
        found = TRUE;

    if (check_dispel(level,victim,gsn_peaches))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("magic ward")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("armor")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("displace")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("silence")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("wild enhance")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("mental barrier")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("mental fortress")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("bless")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("calm")))
    {
	found = TRUE;
	act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("change sex")))
    {
        found = TRUE;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("betray")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("entrance")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("mirror image")))
    {
        found = TRUE;
        act("$n stops shimmering.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("shield of skulls")))
    {
        found = TRUE;
        act("$n's floating skulls crumble to dust.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("astral projection")))
    {
        found = TRUE;
        act("$n stops shimmering.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("domination")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
        found = TRUE;
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("curse")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("gypsy curse")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("mind whip")))
        found = TRUE;
        
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect were")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("levitation")))
    {
       act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
       found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("fly")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("dark risings")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
	act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("enhanced strength")))
    {
       act("$n no longer looks so powerful.",victim,NULL,NULL,TO_ROOM);
       found =TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("inspire")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("soothe")))
        found = TRUE; 

    if (check_dispel(level,victim,skill_lookup("heightened senses")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("haste")))
    {
	act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invisibility")))
    {
        act("$n fades into existence.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existence.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("biofeedback")))
    {
       act("The white aura around $n's body vanishes.",victim,NULL,NULL,TO_ROOM);
       found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,gsn_love))
    {
        act("$n looks sadder as love abandons $m.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("shield")))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("sleep")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("slow")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("warp time")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
   
    if (check_dispel(level,victim,skill_lookup("web")))
    {
	act("The webs surrounding $n dissolve.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("steel skin")))
    {
       act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
       found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("psychic drain")))
    {
       act("$n no longer looks so drained.",victim,NULL,NULL,TO_ROOM);
       found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("forget")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("festive spirit")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("ordain")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("solemn vow")))
        found = TRUE;

    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_infliction( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    damage_old( ch, (CHAR_DATA *) vo, dice(1, 8) + level / 3,
               sn,DAM_MENTAL,TRUE);
    return;
}



void spell_mind_blast(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    if(!saves_spell(level, ch, victim, DAM_MENTAL))
    {
	send_to_char("Your mental assault causes your enemy to bleed energy!\n\r",ch);
	send_to_char("Your mind reels, under attack!\n\r",victim);
	victim->mana = UMAX(0,victim->mana - number_range(20,40));
    }
    damage_old( ch, (CHAR_DATA *) vo,dice(level, 10),
                sn,DAM_MENTAL,TRUE);
    return;
}



void spell_natures_wrath(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
   damage_old( ch, (CHAR_DATA *) vo,dice(level, 8) +level / 2,
              sn,DAM_DROWNING,TRUE);
   return;
}


void spell_psychic_crush(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    damage_old( ch, (CHAR_DATA *) vo, dice(2, 8) + level / 2,
               sn,DAM_MENTAL,TRUE);
    return;
}

void spell_chain_lightning(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *tmp_vict,*last_vict,*next_vict;
  bool found;
  int dam;

  /* first strike */
  if(IS_NPC(ch)) return;

  act("A lightning bolt leaps from $n's hand and arcs to $N.",
      ch,NULL,victim,TO_ROOM);
  act("A lightning bolt leaps from your hand and arcs to $N.",
      ch,NULL,victim,TO_CHAR);
  act("A lightning bolt leaps from $n's hand and hits you!",
      ch,NULL,victim,TO_VICT);  

  dam = dice(level,6);
  if (saves_spell(level,ch,victim,DAM_LIGHTNING))
    dam /= 3;
  damage_old(ch,victim,dam,sn,DAM_LIGHTNING,TRUE);
  last_vict = victim;
  level -= 4;   /* decrement damage */

  /* new targets */
  while (level > 0)
    {
      found = FALSE;
      for (tmp_vict = ch->in_room->people; 
	   tmp_vict != NULL; 
	   tmp_vict = next_vict) 
	{
	  next_vict = tmp_vict->next_in_room;
	  if ( !is_safe_spell(ch,tmp_vict,TRUE) 
          &&    tmp_vict != last_vict
          &&   !IS_WIZI_AREA(ch,tmp_vict) )
	    {
	      found = TRUE;
	      last_vict = tmp_vict;
	      if((tmp_vict->in_room) != (ch->in_room)) 
		{
		  char debug[ MAX_STRING_LENGTH ];

		  sprintf( debug, "Chain lightning fucked up on %s", tmp_vict->name );
		 
		  bug("Chain lightning fucked up on %s", 0);
		  
		  return;
		}
	      act("The bolt arcs to $n!",tmp_vict,NULL,NULL,TO_ROOM);
	      act("The bolt hits you!",tmp_vict,NULL,NULL,TO_CHAR);
	      dam = dice(level,6);
	      if (saves_spell(level,ch,tmp_vict,DAM_LIGHTNING))
		dam /= 3;
	      damage_old(ch,tmp_vict,dam,sn,DAM_LIGHTNING,TRUE);
	      level -= 4;  /* decrement damage */
	    }
	}   /* end target searching loop */
	
      if (!found) /* no target found, hit the caster */
	{
	  if (ch == NULL)
     	    return;

	  if (last_vict == ch) /* no double hits */
	    {
	      act("The bolt seems to have fizzled out.",ch,NULL,NULL,TO_ROOM);
	      act("The bolt grounds out through your body.",
		  ch,NULL,NULL,TO_CHAR);
	      return;
	    }
	
	  last_vict = ch;
	  act("The bolt arcs to $n...whoops!",ch,NULL,NULL,TO_ROOM);
	  send_to_char("You are struck by your own lightning!\n\r",ch);
	  dam = dice(level,6);
	  if (saves_spell(level,ch,ch,DAM_LIGHTNING))
	    dam /= 3;
	  damage_old(ch,ch,dam,sn,DAM_LIGHTNING,TRUE);
	  level -= 4;  /* decrement damage */
	  if (ch == NULL) 
	    return;
	}
      /* now go back and find more targets */
    }
}
	  

void spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ))
    {
	if (victim == ch)
	  send_to_char("You've already been changed.\n\r",ch);
	else
	  act("$N has already had $s(?) sex changed.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if (saves_spell(level, ch, victim, DAM_OTHER))
	return;	
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2 * level;
    af.location  = APPLY_SEX;
    do
    {
	af.modifier  = number_range( 0, 2 ) - victim->sex;
    }
    while ( af.modifier == 0 );
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel different.\n\r", victim );
    act("$n doesn't look like $mself anymore...",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_safe(ch,victim)){
        if (IS_AFFECTED2(victim, AFF_GHOST))
	    send_to_char("Charm a ghost?\n\r",ch);
        return;
        }

    if ( victim == ch )
    {
	send_to_char( "You like yourself even better!\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   victim->fighting != NULL
    ||  (victim->level > 50)
    ||   saves_spell(level, ch, victim, DAM_CHARM) )
	return;

/*
    if (ch->charmies >= ch->level/5)
*/
    if (CountCharmies(ch) >= ch->level/5)
    {
	send_to_char("You can't charm anything else yet.\n\r",ch);
	return;
    }

    if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
    {
	send_to_char(
	    "The mayor does not allow charming in the city limits.\n\r",ch);
	return;
    }
  
    if ( victim->master )
	if ( stop_follower( victim ) )
/*          This won't report a message, but it should never really happen;
            a person would have to be able to charm someone else's pet for
            this to pop up. */
	    return;
    if ( !check_charmlist( ch, victim ) )
    {
        add_follower( victim, ch );
        victim->leader = ch;
        af.where     = TO_AFFECTS;
        af.type      = sn;
        af.level	 = level;
        af.duration  = number_fuzzy( level / 4 );
        af.location  = 0;
        af.modifier  = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char( victim, &af );
        act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );

        if ( ch != victim )
            act("$N looks at you with dull, adoring eyes.",ch,NULL,victim,TO_CHAR);
            ch->charmies += 1;
    }
    return;
}



void spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  6,  7,  8,	 9, 12, 13, 13, 13,
	14, 14, 14, 15, 15,	15, 16, 16, 16, 17,
	17, 17, 18, 18, 18,	19, 19, 19, 20, 20,
	20, 21, 21, 21, 22,	22, 22, 23, 23, 23,
	24, 24, 24, 25, 25,	25, 26, 26, 26, 27
    };
    AFFECT_DATA af;
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( !saves_spell( level, ch, victim, DAM_COLD ) )
    {
	act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM);
	af.where     = TO_AFFECTS;
	af.type      = sn;
        af.level     = level;
	af.duration  = 6;
	af.location  = APPLY_STR;
	af.modifier  = -1;
	af.bitvector = 0;
	affect_join( victim, &af, FALSE );
    }
    else
    {
	dam /= 2;
    }

    damage_old( ch, victim, dam, sn, DAM_COLD,TRUE );
    return;
}



void spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0,  0,  0,
	30, 35, 40, 45, 50,	55, 55, 55, 56, 57,
	58, 58, 59, 60, 61,	61, 62, 63, 64, 64,
	65, 66, 67, 67, 68,	69, 70, 70, 71, 72,
	73, 73, 74, 75, 76,	76, 77, 78, 79, 79
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2,  dam_each[level] * 2 );
    if ( saves_spell( level, ch, victim, DAM_LIGHT) )
	dam /= 2;
    else 
	spell_blindness(skill_lookup("blindness"),
	    level/2,ch,(void *) victim,TARGET_CHAR);

    damage_old( ch, victim, dam, sn, DAM_LIGHT,TRUE );
    return;
}



void spell_continual_light(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *light;

    if (target_name[0] != '\0')  /* do a glow on some object */
    {
	light = get_obj_carry(ch,target_name,ch, TRUE);
	
	if (light == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}

	if (IS_OBJ_STAT(light,ITEM_GLOW))
	{
	    act("$p is already glowing.",ch,light,NULL,TO_CHAR);
	    return;
	}

	SET_BIT(light->extra_flags,ITEM_GLOW);
	act("$p glows with a white light.",ch,light,NULL,TO_ALL);
	return;
    }

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    obj_to_room( light, ch->in_room );
    act( "$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM );
    act( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
    return;
}



void spell_control_weather(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{
    if ( !str_cmp( target_name, "better" ) )
/*	weather_info.change += dice( level / 3, 4 ); */
        weather_info.cwmod += dice( level / 3, 4 );
    else if ( !str_cmp( target_name, "worse" ) )
/*	weather_info.change -= dice( level / 3, 4 ); */
        weather_info.cwmod -= dice( level / 3, 4 );
    else
	send_to_char ("Do you want it to get better or worse?\n\r", ch );
    weather_info.cwmod = UMAX( weather_info.cwmod, -30 );
    weather_info.cwmod = UMIN( weather_info.cwmod, 30 );

    send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *mushroom;

    mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
/*    mushroom->value[0] = level / 2;
    mushroom->value[1] = level;                             */
    obj_to_room( mushroom, ch->in_room );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
    return;
}

/*** Moved to templar.c 5/24/09 gkl 
void spell_create_rose( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *rose;
    rose = create_object(get_obj_index(OBJ_VNUM_ROSE), 0);
    act("$n has created a beautiful red rose.",ch,rose,NULL,TO_ROOM);
    send_to_char("You create a beautiful red rose.\n\r",ch);
    obj_to_char(rose,ch);
    return;
} ***/

void spell_create_spring(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *spring;

    spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
    spring->timer = level;
    obj_to_room( spring, ch->in_room );
    act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
    act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );
    return;
}



void spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "It is unable to hold water.\n\r", ch );
	return;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
	send_to_char( "It contains some other liquid.\n\r", ch );
	return;
    }

    water = UMIN(
		level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		obj->value[0] - obj->value[1]
		);
  
    if ( water > 0 )
    {
	obj->value[2] = LIQ_WATER;
	obj->value[1] += water;
	if ( !is_name( "water", obj->name ) )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "%s water", obj->name );
	    free_string( obj->name );
	    obj->name = str_dup( buf );
	}
	act( "$p is filled.", ch, obj, NULL, TO_CHAR );
    }

    return;
}



void spell_cure_blindness(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;
    short chance;

    chance = number_range (0,100);

    victim = (CHAR_DATA *) vo;

    if (saves_spell(level,ch,victim,DAM_OTHER))
    {
        if (chance > 75)
        {
	   send_to_char( "You failed.\n\r", ch);
	   return;
        }
    }

    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("smoke screen")))
    {
        found = TRUE;
        act("$n is no longer blinded by smoke.",victim,NULL,NULL,TO_ROOM);
    }


    if (found)
        send_to_char("Ok.\n\r",ch);

    else
        send_to_char("Spell failed.\n\r",ch);
	return;
}


void spell_cure_critical( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(3, 8) + level - 6;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}


void spell_cure_disease( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA *paf;
    bool affected = FALSE;

    affected = is_affected(victim, gsn_plague) 
    ||         is_affected(victim, gsn_unholyfire)
    ||         is_affected(victim, gsn_pestilence);

    if ( !affected )
    {
        if (victim == ch)
            send_to_char("You aren't ill.\n\r",ch);
        else
            act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
    }
    else
    {
        if (check_dispel(level,victim,gsn_plague))
        {
            act("$n looks relieved as $s sores vanish.",
                victim,NULL,NULL,TO_ROOM);
        }
        else if (check_dispel( level, victim, gsn_unholyfire ))
        {
            act( "$n looks relieved as $s unholy fire is extinguished.", 
                victim, NULL, NULL, TO_ROOM );
        }
        else if ( (paf = affect_find(victim->affected, gsn_pestilence) ) != NULL
             &&   paf->duration > -1
             &&   check_dispel( level, victim, gsn_pestilence ))
        {
            act("The pustules covering $n's body quickly dry up and heal over.",
                victim, NULL, NULL, TO_ROOM );
        }
        else
            send_to_char("Spell failed.\n\r",ch);
    }
    return;
}



void spell_blood_cool( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_bboil ) )
    {
        if (victim == ch)
          send_to_char("Your blood isnt boiling.\n\r",ch);
        else
          act("$N blood isn't boiling.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    if (check_dispel(level,victim,gsn_bboil))
    {
	send_to_char("Your blood begins to cool.\n\r",victim);
	act("$n looks relieved as $s blood cools.",victim,NULL,NULL,TO_ROOM);
    }
    else
	send_to_char("Spell failed.\n\r",ch);
}



void spell_cure_light( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(1, 8) + level / 3;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
 
    if ( is_affected( victim, gsn_poison ) )
    {
        if ( check_dispel( level, victim, gsn_poison ) )
        {
            send_to_char("A warm feeling runs through your body.\n\r", victim);
            act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
        }
        else
            send_to_char("Spell failed.\n\r",ch);
    }
    /* don't want morts being able to heal wyrm poison off Bashalde */
    else if ( is_affected( victim, gsn_wyrm_venom ) )
    {
        if ( IS_IMMORTAL(victim) )
        {
            act("$N's poison is too powerful!",ch,NULL,victim,TO_CHAR);
        }
        else if ( check_dispel( level, victim, gsn_wyrm_venom) )
        {
            send_to_char("A warm feeling runs through your body.\n\r",victim);
            act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
        }
        else
            send_to_char("Spell failed.\n\r",ch);
    }
    else
    {
        if (victim == ch)
          send_to_char("You aren't poisoned.\n\r",ch);
        else
          act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
    }
    return;
}

void spell_cure_serious( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(2, 8) + level /2 ;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



/* If you change this spell, it may also be a good idea to change spell_judgment
   in templar.c, as it emulates spell_curse. */
void spell_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
        obj = (OBJ_DATA *) vo;

        if (IS_OBJ_STAT(obj,ITEM_DARK))
        {
            act("$p has already been tainted.",ch,obj,NULL,TO_CHAR);
            return;
        }
        if (IS_OBJ_STAT(obj,ITEM_BLESS))
        {
            AFFECT_DATA *paf;

            paf = affect_find(obj->affected,skill_lookup("bless"));
            if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
            {
                if (paf != NULL)
                    affect_remove_obj(obj,paf);
                act("$p glows with a red aura.",ch,obj,NULL,TO_ALL);
                REMOVE_BIT(obj->extra_flags,ITEM_BLESS);
                return;
            }
            else
            {
                act("The holy aura of $p is too powerful for you to overcome.",
                    ch,obj,NULL,TO_CHAR);
                return;
            }
        }

        af.where        = TO_OBJECT;
        af.type         = sn;
        af.level        = level;
        af.duration     = level/10;
        af.location     = APPLY_SAVES;
        af.modifier     = +1;
        af.bitvector    = ITEM_DARK;
        affect_to_obj(obj,&af);

        act("$p glows with a malevolent aura.",ch,obj,NULL,TO_ALL);

	if (obj->wear_loc != WEAR_NONE)
	    ch->saving_throw += 1;
        return;
    }

    /* character curses */
    victim = (CHAR_DATA *) vo;

    if(is_safe_spell(ch,victim,FALSE) && victim != ch)
    {
      send_to_char("Not on that target.\n\r",ch);
      return;
    }

    if (IS_AFFECTED(victim,AFF_CURSE) 
    ||  saves_spell(level,ch,victim,DAM_OTHER) )
    {
        send_to_char("Your spell had no effect.\n\r",ch);
        return;
    }

    if ( IS_AFFECTED3(victim,AFF3_SOLEMN) 
    &&   number_percent() > CONFIG_SOLEMN_VOWS_CHANCE ) 
    {
      send_to_char("The power of your vows shield you from taint.\n\r", victim);
      if ( ch != victim )
        act("$N's vows protect $M from your spell.", ch, NULL, victim, TO_CHAR);
      return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/3;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * (level / 8);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

    send_to_char( "You feel unclean.\n\r", victim );
    if ( ch != victim )
	act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_smite(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (victim != ch)
    {
	act("$n calls down the wrath of $s God upon $N!",
	    ch,NULL,victim,TO_ROOM);
        act("$n has stricken you down with the wrath of $s God!",
	    ch,NULL,victim,TO_VICT);
	send_to_char("You call upon the divine fury of your God!\n\r",ch);
    }
    dam = dice( level, 10 );
    damage_old( ch, victim, dam, sn, DAM_HOLY,TRUE);
}



void spell_detect_hidden(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_HIDDEN) )
    {
        if (victim == ch)
          send_to_char("You are already as alert as you can be. \n\r",ch);
        else
          act("$N can already sense hidden lifeforms.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( victim, &af );
    send_to_char( "Your awareness improves.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_detect_were(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

  if ( !IS_IMMORTAL( ch ) && !IS_NPC( ch ) && !IS_WERECREATURE( ch ) )
  {
    send_to_char( "You mispronounce an arcane syllable.\n\r", ch );
    return;
  }

    if ( IS_AFFECTED2(victim, AFF_DETECTWERE) )
    {
        if (victim == ch)
          send_to_char("You can already detect were creatures. \n\r",ch);
        else
          act("$N can already sense were creatures.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.whichaff  = AFF2;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECTWERE;
    affect_to_char2( victim, &af );
    send_to_char( "You can sense were creatures.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}


void spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
    {
        if (victim == ch)
          send_to_char("You can already see invisible.\n\r",ch);
        else
          act("$N can already see invisible things.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) )
    {
        if (victim == ch)
          send_to_char("You can already sense magical auras.\n\r",ch);
        else
          act("$N can already detect magic.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_to_char( victim, &af );
    send_to_char( "You can now sense magical auras.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
	if ( obj->value[3] != 0 )
	    send_to_char( "You smell poisonous fumes.\n\r", ch );
	else
	    send_to_char( "It looks delicious.\n\r", ch );
    }
    else
    {
	send_to_char( "It doesn't look poisoned.\n\r", ch );
    }

    return;
}

/* modified for enhanced use */

void spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;

    if (saves_spell(level,ch,victim,DAM_OTHER))
    {
	send_to_char( "You feel a brief tingling sensation.\n\r",victim);
	send_to_char( "You failed.\n\r", ch);
	return;
    }

    /* begin running through the spells */ 
    if( check_dispel( level, victim, gsn_pinch ) )
      found = TRUE;
    if( check_dispel( level, victim, gsn_haunt ) )
      found = TRUE;

    if( check_dispel(level,victim,skill_lookup("veil")))
      found = TRUE;
    if (check_dispel(level,victim,skill_lookup("betray")))
	found = TRUE;
    if (check_dispel(level,victim,skill_lookup("shadows")))
        found = TRUE;
    if (check_dispel(level,victim,skill_lookup("ordain")))
	found = TRUE;
    if (check_dispel(level,victim,skill_lookup("libraverim")))
        found = TRUE;
    if (check_dispel(level,victim,skill_lookup("circle of protection")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("adrenaline rush")))
        found = TRUE;

    if (check_dispel(level,victim,gsn_divine_focus))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("awe")))
        found = TRUE;

    if (check_dispel(level,victim,gsn_peaches))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("magic ward")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("armor")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("displace")))
       found = TRUE;

    if (check_dispel(level,victim,skill_lookup("mental barrier")))
       found = TRUE;

    if (check_dispel(level,victim,skill_lookup("mental fortress")))
       found = TRUE;

    if (check_dispel(level,victim,skill_lookup("wild enhance")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("silence")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("bless")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("calm")))
    {
        found = TRUE;
        act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("inspire")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("soothe")))
         found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("heightened senses")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("change sex")))
    {
        found = TRUE;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("betray")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("domination")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
        found = TRUE;
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("curse")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("gypsy curse")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("mind whip")))
        found = TRUE;
  
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("levitation")))
    {
       act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
       found= TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("fly")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("dark risings")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
 
    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
        act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("enhanced strength")))
    {
       act("$n no longer looks so powerful.",victim,NULL,NULL,TO_ROOM);
       found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("haste")))
    {
        act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invis")))
    {
        act("$n fades into existence.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existence.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
        found = TRUE;
 

    if (check_dispel(level,victim,skill_lookup("biofeedback")))
    {
       act("The white aura around $n's body vanishes.",victim,NULL,NULL,TO_ROOM);
       found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,gsn_love))
    {
        act("$n looks sadder as love abandons $m.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

 
    if (check_dispel(level,victim,skill_lookup("shield")))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("sleep")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("slow")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("psychic drain")))
    {
       act("$n no longer looks so drained.",victim,NULL,NULL,TO_ROOM);
       found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("steel skin")))
    {
       act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
       found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("warp time")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("forget")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("festive spirit")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("web")))
    {
	act("The webs surrounding $n dissolve.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("ordain")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("solemn vow")))
        found = TRUE;

    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
	return;
}

void spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("Not in this room.\n\r",ch);
        return;
    }

    send_to_char( "The earth trembles beneath your feet!\n\r", ch );
    act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_verb(ch,vch,FALSE) && !IS_IMMORTAL(vch))
	      {
		if (IS_AFFECTED(vch,AFF_FLYING))
		  {
		    damage_old(ch,vch,0,sn,DAM_BASH,TRUE);
		  }
		else
		  {
		    damage_old( ch,vch,level + dice(2, 8), sn, DAM_BASH,TRUE);
		  }
	      }

	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char( "The earth trembles and shivers.\n\r", vch );
    }

    return;
}

void spell_enchant_armor( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf; 
    int result, fail, savesfail;
    int ac_bonus, saves_bonus, other_bonus, added;
    bool ac_found = FALSE;
    bool saves_found = FALSE;

    if (obj->item_type != ITEM_ARMOR)
    {
        send_to_char("That isn't an armor.\n\r",ch);
        return;
    }
    else if (obj->wear_loc != -1)
    {
        send_to_char("The item must be carried to be enchanted.\n\r",ch);
        return;
    }

    /* find the bonuses--tally up AC boosts, saves boosts, and "other" affs */
    ac_bonus = 0;
    saves_bonus = 0;
    other_bonus = 0;
    if (!obj->enchanted)
    {
        for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
        {
            if ( paf->location == APPLY_AC )
            {
                ac_bonus += paf->modifier;
                ac_found = TRUE;
            }
            else if ( paf->location == APPLY_SAVES )
            {
                saves_bonus += paf->modifier;
                saves_found = TRUE;
            }
            else if ( paf->location == APPLY_SAVING_ROD
            ||        paf->location == APPLY_SAVING_PETRI
            ||        paf->location == APPLY_SAVING_BREATH
            ||        paf->location == APPLY_SAVING_SPELL )
            {
                saves_bonus += paf->modifier;
            }
            else
            {
                other_bonus++;
            }
        }
    }
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
        if ( paf->location == APPLY_AC )
        {
            ac_bonus += paf->modifier;
            ac_found = TRUE;
        }
        else if ( paf->location == APPLY_SAVES )
        {
            saves_bonus += paf->modifier;
            saves_found = TRUE;
        }
        else if ( paf->location == APPLY_SAVING_ROD
        ||        paf->location == APPLY_SAVING_PETRI
        ||        paf->location == APPLY_SAVING_BREATH
        ||        paf->location == APPLY_SAVING_SPELL )
        {
            saves_bonus += paf->modifier;
        }
        else
        {
            other_bonus++;
        }
    }

    fail = 25;
    fail = 25 + 5 * ac_bonus * ac_bonus + 20 * other_bonus - level;
    if ( IS_OBJ_STAT(obj,ITEM_BLESS) )  fail -= 15;
    if ( IS_OBJ_STAT(obj,ITEM_GLOW) )   fail -=  5;
    if ( saves_bonus < 0 )       /* negative saves make it harder to enchant */
        fail -= 5 * saves_bonus;

    fail = URANGE(5,fail,85);    /* cap failure at 85% */

    /* set rate of failing to enchant with saves--flat percent rates */
    if ( saves_bonus < -3 )      /* -4 or better is impossible */
        savesfail = 100;
    else if ( saves_bonus < -2 ) /* -3 saves is nearly impossible */
        savesfail = 95;
    else if ( saves_bonus < -1 ) /* -2 saves is not easy */
        savesfail = 85;
    else                         /* -1 saves or worse */
        savesfail = 50;

    result = number_percent();

    if ( IS_SET( ch->comm, COMM_VERBOSE )
    &&   get_trust(ch) > LEVEL_ADMIN )
    {
        char buf[1024];
        sprintf(buf,"Chance of failure is %d%%; failure to add saves %d%%.\n\r",
            fail, savesfail);
        send_to_char(buf, ch);
    }

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
        act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_CHAR);
        act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_ROOM);
        extract_obj(obj);
        return;
    }
    else if (result < (fail / 3)) /* item disenchanted */
    {
        AFFECT_DATA *paf_next;

        act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
        act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
        obj->enchanted = TRUE;

        for (paf = obj->affected; paf != NULL; paf = paf_next)
        {
            paf_next = paf->next; 
            free_affect(paf);
        }
        obj->affected = NULL;
        obj->extra_flags = 0;
        obj->cost = 0;
        return;
    }
    else if ( result <= fail )  /* failed, no bad result */
    {
        send_to_char("Nothing seemed to happen.\n\r",ch);
        return;
    }

    /* enchant and copy all affects from pIndex to obj */
    if (!obj->enchanted)
    {
        AFFECT_DATA *af_new;
        obj->enchanted = TRUE;

        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
        {
            af_new = new_affect();

            af_new->next = obj->affected;
            obj->affected = af_new;

            af_new->where	= paf->where;
            af_new->type 	= UMAX(0,paf->type);
            af_new->level	= paf->level;
            af_new->duration	= paf->duration;
            af_new->location	= paf->location;
            af_new->modifier	= paf->modifier;
            af_new->bitvector	= paf->bitvector;
        }
    }

    /* cursed/dark items get negative bonuses when enchanted */
    if ( IS_OBJ_STAT(obj, ITEM_DARK) )
    {
        bool found = FALSE;
        sh_int apply;
        apply = number_range(APPLY_STR, APPLY_CON);
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
            if ( !found && paf->location == apply)
	    {
                paf->type = sn;
                paf->modifier -= 1;
                paf->level = UMAX(paf->level,level);
                found = TRUE;
            }
        }
        if ( !found )
        {
            paf = new_affect();
            paf->where          = TO_OBJECT;
            paf->type           = sn;
            paf->level          = level;
            paf->duration       = -1;
            paf->location       = apply;
            paf->modifier       = -1;
            paf->bitvector      = 0;
            paf->next           = obj->affected;
            obj->affected       = paf;
        }
        act("$p crackles and darkens!",ch,obj,NULL,TO_CHAR);
        act("$p crackles and darkens.",ch,obj,NULL,TO_ROOM);
        return;
    }

    /* regular enchant vs exceptional (-2ac) enchant */
    if (result <= (90 - level/5))
    {
        SET_BIT(obj->extra_flags, ITEM_MAGIC);
        added = -1;
    }
    else
    {
        SET_BIT(obj->extra_flags,ITEM_MAGIC);
        SET_BIT(obj->extra_flags,ITEM_GLOW);
        added = -2;
    }

    if (obj->level < LEVEL_HERO)
        obj->level = UMIN(LEVEL_HERO - 1, obj->level + 1);

    /* if eq already has AC bonus, stack on top of existing aff */
    if (ac_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
            if ( paf->location == APPLY_AC)
	    {
                paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX(paf->level,level);
            }
        }
    }
    else
    {
        paf = new_affect();

        paf->where	= TO_OBJECT;
        paf->type	= sn;
        paf->level	= level;
        paf->duration	= -1;
        paf->location	= APPLY_AC;
        paf->modifier	=  added;
        paf->bitvector  = 0;
        paf->next	= obj->affected;
        obj->affected	= paf;
    }

    /* new addition of saves */
    if( number_percent() > savesfail )
    {
        if( saves_found )
        {
            for( paf = obj->affected; paf != NULL; paf = paf->next )
            {
                if( paf->location == APPLY_SAVES )
                {
                    paf->type = sn;
                    paf->modifier -= 1;
                    paf->level = UMAX( paf->level, level );
                }
            }
        }
        else
        {
            paf = new_affect();

            paf->where = TO_OBJECT;
            paf->type = sn;
            paf->level = level;
            paf->duration = -1;
            paf->location = APPLY_SAVES;
            paf->modifier = -1;
            paf->bitvector = 0;
            paf->next = obj->affected;
            obj->affected = paf;
        }
        act("$p glows a brilliant gold!",ch,obj,NULL,TO_CHAR);
        act("$p glows a brilliant gold!",ch,obj,NULL,TO_ROOM);
    }
    else
    {
        act("$p shimmers with a gold aura.",ch,obj,NULL,TO_CHAR);
        act("$p shimmers with a gold aura.",ch,obj,NULL,TO_ROOM);
    }

    return;
}

/*
 * Old version of the spell that went off of AC instead of saves
 *
void spell_enchant_armor( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA *paf; 
  int result, fail;
  int ac_bonus, added;
  bool ac_found = FALSE;
  bool saves_found = FALSE;

  if (obj->item_type != ITEM_ARMOR)
    {
      send_to_char("That isn't an armor.\n\r",ch);
      return;
    }

  if (obj->wear_loc != -1)
    {
      send_to_char("The item must be carried to be enchanted.\n\r",ch);
      return;
    }

  ac_bonus = 0;
  fail = 25;

  if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
      {
	if ( paf->location == APPLY_AC )
	  {
	    ac_bonus = paf->modifier;
	    ac_found = TRUE;
	    fail += 5 * (ac_bonus * ac_bonus);
	  }

	if( paf->location == APPLY_SAVES )
	  saves_found = TRUE;

	else
	  fail += 20;
      }
 
  for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
      if ( paf->location == APPLY_AC )
  	{
	  ac_bonus = paf->modifier;
	  ac_found = TRUE;
	  fail += 5 * (ac_bonus * ac_bonus);
	}

      if( paf->location == APPLY_SAVES )
        saves_found = TRUE;

      else
	fail += 20;
    }

  fail -= level;

  if (IS_OBJ_STAT(obj,ITEM_BLESS))
    fail -= 15;
  if (IS_OBJ_STAT(obj,ITEM_GLOW))
    fail -= 5;

  fail = URANGE(5,fail,85);

  result = number_percent();

  if (result < (fail / 5))
    {
      act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_CHAR);
      act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_ROOM);
      extract_obj(obj);
      return;
    }

  if (result < (fail / 3))
    {
      AFFECT_DATA *paf_next;

      act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
      act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
      obj->enchanted = TRUE;

      for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	  paf_next = paf->next; 
	  free_affect(paf);
	}
      obj->affected = NULL;

      obj->extra_flags = 0;
      obj->cost = 0;
      return;
    }

  if ( result <= fail )
    {
      send_to_char("Nothing seemed to happen.\n\r",ch);
      return;
    }

  if (!obj->enchanted)
    {
      AFFECT_DATA *af_new;
      obj->enchanted = TRUE;

      for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
	{
	  af_new = new_affect();
	
	  af_new->next = obj->affected;
	  obj->affected = af_new;

	  af_new->where	= paf->where;
	  af_new->type 	= UMAX(0,paf->type);
	  af_new->level	= paf->level;
	  af_new->duration	= paf->duration;
	  af_new->location	= paf->location;
	  af_new->modifier	= paf->modifier;
	  af_new->bitvector	= paf->bitvector;
	}
    }

  if (result <= (90 - level/5))
    {
      act("$p shimmers with a gold aura.",ch,obj,NULL,TO_CHAR);
      act("$p shimmers with a gold aura.",ch,obj,NULL,TO_ROOM);
      SET_BIT(obj->extra_flags, ITEM_MAGIC);
      added = -1;
    }
    
  else
    {
      act("$p glows a brilliant gold!",ch,obj,NULL,TO_CHAR);
      act("$p glows a brilliant gold!",ch,obj,NULL,TO_ROOM);
      SET_BIT(obj->extra_flags,ITEM_MAGIC);
      SET_BIT(obj->extra_flags,ITEM_GLOW);
      added = -2;
    }
		
  if (obj->level < LEVEL_HERO)
    obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

  if (ac_found)
    {
      for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	  if ( paf->location == APPLY_AC)
	    {
	      paf->type = sn;
	      paf->modifier += added;
	      paf->level = UMAX(paf->level,level);
	    }
	}
    }
  else
    {
      paf = new_affect();

      paf->where	= TO_OBJECT;
      paf->type	= sn;
      paf->level	= level;
      paf->duration	= -1;
      paf->location	= APPLY_AC;
      paf->modifier	=  added;
      paf->bitvector  = 0;
      paf->next	= obj->affected;
      obj->affected	= paf;
    }

  if( number_percent() < 50 )
    {
      if( saves_found )
	{
	  for( paf = obj->affected; paf != NULL; paf = paf->next )
	    {
	      if( paf->location == APPLY_SAVES )
		{
		  paf->type = sn;
		  paf->modifier -= 1;
		  paf->level = UMAX( paf->level, level );
		}
	    }
	}
      else
	{
	  paf = new_affect();

	  paf->where = TO_OBJECT;
	  paf->type = sn;
	  paf->level = level;
	  paf->duration = -1;
	  paf->location = APPLY_SAVES;
	  paf->modifier = -1;
	  paf->bitvector = 0;
	  paf->next = obj->affected;
	  obj->affected = paf;
	}
    }
       
}
*/

void spell_enchant_weapon(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf; 
    int result, fail;
    int hit_bonus, dam_bonus, added;
    bool hit_found = FALSE, dam_found = FALSE;

    if (obj->item_type != ITEM_WEAPON)
    {
	send_to_char("That isn't a weapon.\n\r",ch);
	return;
    }

    if (obj->wear_loc != -1)
    {
	send_to_char("The item must be carried to be enchanted.\n\r",ch);
	return;
    }

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_HITROLL )
            {
	    	hit_bonus = paf->modifier;
		hit_found = TRUE;
	    	fail += 2 * (hit_bonus * hit_bonus);
 	    }

	    else if (paf->location == APPLY_DAMROLL )
	    {
	    	dam_bonus = paf->modifier;
		dam_found = TRUE;
	    	fail += 2 * (dam_bonus * dam_bonus);
	    }

	    else  /* things get a little harder */
	    	fail += 25;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_HITROLL )
  	{
	    hit_bonus = paf->modifier;
	    hit_found = TRUE;
	    fail += 2 * (hit_bonus * hit_bonus);
	}

	else if (paf->location == APPLY_DAMROLL )
  	{
	    dam_bonus = paf->modifier;
	    dam_found = TRUE;
	    fail += 2 * (dam_bonus * dam_bonus);
	}

	else /* things get a little harder */
	    fail += 25;
    }

    /* apply other modifiers */
    fail -= 3 * level/2;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,95);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$p shivers violently and explodes!",ch,obj,NULL,TO_CHAR);
	act("$p shivers violently and explodes!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
	return;
    }

    if (result < (fail / 2)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
        obj->cost = 0;
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_char("Nothing seemed to happen.\n\r",ch);
	return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
	{
	    af_new = new_affect();
	
	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->where	= paf->where;
	    af_new->type 	= UMAX(0,paf->type);
	    af_new->level	= paf->level;
	    af_new->duration	= paf->duration;
	    af_new->location	= paf->location;
	    af_new->modifier	= paf->modifier;
	    af_new->bitvector	= paf->bitvector;
	}
    }

    if (result <= (100 - level/5))  /* success! */
    {
	act("$p glows blue.",ch,obj,NULL,TO_CHAR);
	act("$p glows blue.",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = 1;
    }
    
    else  /* exceptional enchant */
    {
	act("$p glows a brilliant blue!",ch,obj,NULL,TO_CHAR);
	act("$p glows a brilliant blue!",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = 2;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO - 1)
	obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

    if (dam_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_DAMROLL)
	    {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
		if (paf->modifier > 4)
		    SET_BIT(obj->extra_flags,ITEM_HUM);
	    }
	}
    }
    else /* add a new affect */
    {
	paf = new_affect();

	paf->where	= TO_OBJECT;
	paf->type	= sn;
	paf->level	= level;
	paf->duration	= -1;
	paf->location	= APPLY_DAMROLL;
	paf->modifier	=  added;
	paf->bitvector  = 0;
    	paf->next	= obj->affected;
    	obj->affected	= paf;
    }

    if (hit_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
            if ( paf->location == APPLY_HITROLL)
            {
		paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX(paf->level,level);
                if (paf->modifier > 4)
                    SET_BIT(obj->extra_flags,ITEM_HUM);
            }
	}
    }
    else /* add a new affect */
    {
        paf = new_affect();
 
        paf->type       = sn;
        paf->level      = level;
        paf->duration   = -1;
        paf->location   = APPLY_HITROLL;
        paf->modifier   =  added;
        paf->bitvector  = 0;
        paf->next       = obj->affected;
        obj->affected   = paf;
    }

}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( saves_spell(level, ch, victim, DAM_OTHER) )
    {
	send_to_char("You feel a momentary chill.\n\r",victim);  	
	return;
    }

    if ( IS_AFFECTED3(victim,AFF3_SOLEMN)
    &&   number_percent() > CONFIG_SOLEMN_VOWS_CHANCE )
    {
      send_to_char("The power of your vows shield you from taint.\n\r", victim);
      if ( ch != victim )
        act("$N's vows protect $M from your spell.", ch, NULL, victim, TO_CHAR);
      return;
    }

    if ( victim->level <= 2 )
    {
	dam		 = ch->hit + 1;
    }
    else
    {
	/*gain_exp( victim, 0 - number_range( level/2, 3 * level / 2 ) );*/
	victim->mana	/= 2;
	victim->move	/= 2;
	dam		 = dice(1, level);
	ch->hit		+= dam;
    }

    send_to_char("You feel your life slipping away!\n\r",victim);
    send_to_char("Wow....what a rush!\n\r",ch);
    damage_old( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);

    return;
}



void spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	  0,
	  0,   0,   0,   0,   0,	  0,   0,   0,   0,   0,
	  0,   0,   0,   0,  30,	 35,  40,  45,  50,  55,
	 60,  65,  70,  75,  80,	 82,  84,  86,  88,  90,
	 92,  94,  96,  98, 100,	102, 104, 106, 108, 110,
	112, 114, 116, 118, 120,	122, 124, 126, 128, 130
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    /*if ( saves_spell( level, victim, DAM_FIRE) )
	dam /= 2;*/
    damage_old( ch, victim, dam, sn, DAM_FIRE ,TRUE);
    return;
}


void spell_fireproof(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
 
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
    {
        act("$p is already protected from burning.",ch,obj,NULL,TO_CHAR);
        return;
    }
 
    af.where     = TO_OBJECT;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy(level / 4);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_BURN_PROOF;
 
    affect_to_obj(obj,&af);
 
    act("You protect $p from fire.",ch,obj,NULL,TO_CHAR);
    act("$p is surrounded by a protective aura.",ch,obj,NULL,TO_ROOM);
}



void spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(6 + level / 2, 8);
    /*if ( saves_spell( level, victim,DAM_HOLY) )
	dam /= 2;*/
    damage_old( ch, victim, dam, sn, DAM_HOLY ,TRUE);
    return;
}



void spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) )
	return;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char( victim, &af );
    send_to_char( "You are surrounded by a pink outline.\n\r", victim );
    act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
    return;
}



void spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *ich;

    act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You conjure a cloud of purple smoke.\n\r", ch );

    for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
	if (ich->invis_level > 0 || ich->rinvis_level > 0)
	    continue;

	if ( ich == ch || saves_spell( level,ch,ich,DAM_OTHER) )
	    continue;

	affect_strip ( ich, gsn_invis			);
	affect_strip ( ich, gsn_mass_invis		);
	affect_strip ( ich, gsn_sneak			);
	REMOVE_BIT   ( ich->affected_by, AFF_HIDE	);
	REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE	);
	REMOVE_BIT   ( ich->affected_by, AFF_SNEAK	);
	act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
	send_to_char( "You are revealed!\n\r", ich );
    }

    return;
}

void spell_floating_disc( int sn, int level,CHAR_DATA *ch,void *vo,int target )
{
    OBJ_DATA *disc, *floating;

    floating = get_eq_char(ch,WEAR_FLOAT);
    if (floating != NULL && IS_OBJ_STAT(floating,ITEM_NOREMOVE))
    {
	act("You can't remove $p.",ch,floating,NULL,TO_CHAR);
	return;
    }

    disc = create_object(get_obj_index(OBJ_VNUM_DISC), 0);
    disc->value[0]	= ch->level * 10; /* 10 pounds per level capacity */
    disc->value[3]	= ch->level * 5; /* 5 pounds per level max per item */
    disc->timer		= ch->level * 2 - number_range(0,level / 2); 

    act("$n has created a floating black disc.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You create a floating disc.\n\r",ch);
    obj_to_char(disc,ch);
    wear_obj(ch,disc,TRUE);
    return;
}


void spell_kit( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{

  OBJ_INDEX_DATA *pObjIndex;
  OBJ_INDEX_DATA *pObjIndex2;
  OBJ_INDEX_DATA *pObjIndex3;
  OBJ_INDEX_DATA *pObjIndex4;
  OBJ_DATA *obj;

  if (( pObjIndex = get_obj_index( 1261)) == NULL )
    {
     send_to_char("No object has that vnum.\n\r", ch);
     return;
    }

  if (( pObjIndex2 = get_obj_index( 3015)) == NULL )
    {
     send_to_char("No object has that vnum.\n\r", ch);
     return;
    }

  if (( pObjIndex3 = get_obj_index( 3016)) == NULL )
    {
     send_to_char("No object has that vnum.\n\r", ch);
     return;
    }

  if (( pObjIndex4 = get_obj_index( 3017)) == NULL )
    {
     send_to_char("No object has that vnum.\n\r", ch);
     return;
    }

   obj = create_object(pObjIndex,level);

   obj_to_char(obj,ch);

   obj = create_object(pObjIndex2,level);

   obj_to_char(obj,ch);

   obj = create_object(pObjIndex3,level);

   obj_to_char(obj,ch);

   obj = create_object(pObjIndex4,level);

   obj_to_char(obj,ch);
   send_to_char("You create a helpful survival kit!\n\r",ch);

}


void spell_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{

  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;

  if (( pObjIndex = get_obj_index( 1215)) == NULL )
    {
     send_to_char("No object has that vnum.\n\r", ch);
     return;
    }

   obj = create_object(pObjIndex,level);
   obj->value[1] = level/5;
   obj->value[2] = 7;
   obj->timer    = 10;

   obj_to_char(obj,ch);
   send_to_char("You summon forth a blade of flame!\n\r",ch);

}


void spell_echoes( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{

  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;

  if (( pObjIndex = get_obj_index( 1220)) == NULL )
    {
     send_to_char("No object has that vnum.\n\r", ch);
     return;
    }

   obj = create_object(pObjIndex,level);
   obj->value[1] = level/5;
   obj->value[2] = 7;
   obj->timer    = 10;

   obj_to_char(obj,ch);
   send_to_char("You summon forth an echo of ancient power!\n\r",ch);

}


void spell_psiblade( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;

  if (( pObjIndex = get_obj_index( 1217)) == NULL )
    {
     send_to_char("No object has that vnum.\n\r", ch);
     return;
    }

   obj = create_object(pObjIndex,level);
   obj->value[1] = level/5;
   obj->value[2] = 7;
   obj->timer    = 10;

   obj_to_char(obj,ch);
   send_to_char("You focus your energy and create a blade of pure thought!\n\r",ch);

}


void spell_hammer( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{

  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;

  if (( pObjIndex = get_obj_index( 1216)) == NULL )
    {
     send_to_char("No object has that vnum.\n\r", ch);
     return;
    }

   obj = create_object(pObjIndex,level);
   obj->value[1] = level/5;
   obj->value[2] = 7;
   obj->timer    = 10;

   obj_to_char(obj,ch);
   send_to_char("You summon forth a spiritual hammer!\n\r",ch);

}


void spell_key( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{

  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;

  if (( pObjIndex = get_obj_index( 1222)) == NULL )
    {
     send_to_char("No object has that vnum.\n\r", ch);
     return;
    }

   obj = create_object(pObjIndex,level);
   obj->timer    = 5;

   obj_to_char(obj,ch);
   send_to_char("You pull the key to the jail out of thin air!\n\r",ch);

}


void spell_fly( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FLYING) )
    {
	if (victim == ch)
	  send_to_char("You are already airborne.\n\r",ch);
	else
	  act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if (victim != ch
    && (IS_SET(victim->in_room->room_flags,ROOM_SAFE)
    || (IS_SET(victim->in_room->room_flags,ROOM_SEMISAFE) 
           && !in_fightlag(victim) ) ) ) 
    {
        send_to_char("Not in this room.\n\r",ch);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char( "Your feet rise off the ground.\n\r", victim );
    act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
    return;
}

/* RT clerical berserking spell */

void spell_frenzy(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim,sn) 
    || (    IS_AFFECTED(victim,AFF_BERSERK) 
        && !is_affected(victim,gsn_divine_focus) ) )
    {
	if (victim == ch)
	  send_to_char("You are already in a frenzy.\n\r",ch);
	else
	  act("$N is already in a frenzy.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (is_affected(victim,skill_lookup("calm")))
    {
	if (victim == ch)
	  send_to_char("Why don't you just relax for a while?\n\r",ch);
	else
	  act("$N doesn't look like $e wants to fight anymore.",
	      ch,NULL,victim,TO_CHAR);
	return;
    }


    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level	 = level;
    af.duration	 = level / 3;
    af.modifier  = level / 6;
    af.bitvector = 0;

    af.location  = APPLY_HITROLL;
    affect_to_char(victim,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_char(victim,&af);

    af.modifier  = 10 * (level / 12);
    af.location  = APPLY_AC;
    affect_to_char(victim,&af);

    send_to_char("You are filled with holy wrath!\n\r",victim);
    act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}

void spell_guildspell(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int unholy_touch_sn;
  int encomium_sn;

  if IS_AFFECTED2(victim,AFF_GUILDSPELL)
    {
      if (victim == ch)
	send_to_char("You are already affected by that.\n\r",ch);
      return;
    }

  unholy_touch_sn = skill_lookup( "unholy touch" );
  encomium_sn = skill_lookup( "encomium" );
  
  if( sn == unholy_touch_sn &&
      !IS_NPC( ch ) &&
      !IS_SET( ch->shiftbits, TEMP_VAMP ) &&
      !IS_SET( ch->shiftbits, PERM_VAMP ) )
    {
      ch->pcdata->learned[ sn ] = 0;
      send_to_char( "You don't know any spells of that name.\n\r", ch );
      return;
    }

  af.where     = TO_AFFECTS;
  af.type 	 = sn;
  af.level	 = level;
  af.duration	 = level / 2;
  af.modifier  = level / 6;
  af.bitvector = AFF_GUILDSPELL;

  af.location  = APPLY_HITROLL;
  affect_to_char2(victim,&af);

  af.location  = APPLY_DAMROLL;
  affect_to_char2(victim,&af);

  af.modifier  = - level;
  af.location  = APPLY_AC;
  affect_to_char2(victim,&af);

  if( sn == encomium_sn )
    {
      send_to_char( "Syrin has filled you with divine glory!\n\r", victim );
      act( "$n is filled with the glory of $s god!", victim, NULL, NULL, TO_ROOM );
    }
  else
    {
      send_to_char("You are filled with the glory of your guild!\n\r",victim);
      act("$n is filled with the glory of $s guild!",victim,NULL,NULL,TO_ROOM);
    }
}

/* RT ROM-style gate */
    
 void spell_portal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    char buffer[120];
    CHAR_DATA *victim;
    CHAR_DATA *pet/*, *next*/;
    ROOM_INDEX_DATA *was_room;
	ROOM_INDEX_DATA *pRoomIndex;
/*  bool gate_pet; */
    bool fail = FALSE;

    if ( ( victim = get_char_world( ch, target_name, FALSE ) ) == NULL
	 ||   victim == ch
	 ||   victim->in_room == NULL
	 ||   !can_see_room(ch,victim->in_room) 
	 ||   IS_SET(victim->in_room->room_flags, ROOM_IMP_ONLY)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_GODS_ONLY)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
	 ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	 ||   IS_SET(ch->in_room->room_flags, ROOM_NO_PORTAL )
	 ||   IS_SET(victim->in_room->room_flags, ROOM_NO_PORTAL )
	 ||   !victim->in_room->area->linked
	 ||   IS_AFFECTED(ch,AFF_CURSE)
	 ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_PORTAL))
	 ||   ((IS_SET(victim->in_room->room_flags,ROOM_SAFE) 
     || IS_SET(victim->in_room->room_flags,ROOM_SEMISAFE))
                  && in_fightlag(ch) 
                  && !in_fightlag(victim) ) )
		{
			fail = TRUE;
		}

    /* new trans bitset */
    if( !fail )
      {
			if( IS_SET( ch->in_room->trans_flags, TRANS_NO_PORTAL ) ||
			IS_SET( ch->in_room->trans_flags, TRANS_NO_PORTAL_OUT ) ||
			IS_SET( ch->in_room->trans_flags, TRANS_NO_SPELLS ) ||
			IS_SET( ch->in_room->trans_flags, TRANS_NO_SPELLS_OUT ) ||
			IS_SET( victim->in_room->trans_flags, TRANS_NO_PORTAL ) ||
			IS_SET( victim->in_room->trans_flags, TRANS_NO_PORTAL_IN ) ||
			IS_SET( victim->in_room->trans_flags, TRANS_NO_SPELLS ) ||
			IS_SET( victim->in_room->trans_flags, TRANS_NO_SPELLS_IN ) )
			{
				fail = TRUE;
			}
      }

    if( fail )
      {
        send_to_char( "You failed.\n\r", ch );
        if (victim)
          if (IS_AFFECTED(victim, AFF_DETECT_MAGIC || IS_SET(victim->act_bits, PLR_HOLYLIGHT))){
	    sprintf(buffer,"Your senses tingle!\n\r");
	    send_to_char(buffer,victim);
	  }
	
        return;
      }	
    
    if ( IS_AFFECTED(ch, AFF_CHARM)   
         &&   ch->master != NULL
         &&   ch->in_room == ch->master->in_room )
       {
         send_to_char( "What?  And leave your beloved master?\n\r", ch );
         return;
       }

	
	if(is_affected(ch,skill_lookup("hallowed discord"))) {
	pRoomIndex = get_random_room(ch);
	send_to_char("You step through a portal and vanish.\n\r",ch);
    ch->move -= 10;
    ch->was_in_room = ch->in_room;
    act( "$n steps through a portal and vanishes.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, pRoomIndex );
    do_look( ch, "auto" );
    act( "$n has arrived through a portal.", ch, NULL, NULL, TO_ROOM );
    ch->was_in_room = NULL;
    return;
	}

    if (IS_AFFECTED(victim, AFF_DETECT_MAGIC)){
       sprintf(buffer,"You feel the air stir around you!\n\r");
       send_to_char(buffer,victim);
       }

    was_room = ch->in_room;
    act("$n steps through a portal and vanishes.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You step through a portal and vanish.\n\r",ch);
    ch->was_in_room = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, victim->in_room);
    do_look(ch,"auto");
    act("$n has arrived through a portal.",ch,NULL,NULL,TO_ROOM);
    ch->was_in_room = NULL;


/*    if (gate_pet)
    {*/

    for ( pet = was_room->people; pet != NULL; )
    {
        if ( ch->pet == pet ||
           ( IS_AFFECTED2( pet, AFF2_SUMMONEDPET ) && pet->master == ch ) )
        {
/* Only one pet goes through portal.
            next = pet->next_in_room; */
            act("$n steps through a portal and vanishes.",pet,NULL,NULL,TO_ROOM);
            send_to_char("You step through a portal and vanish.\n\r",pet);
            char_from_room(pet);
            char_to_room(pet, victim->in_room);
            act("$n has arrived through a portal.",pet,NULL,NULL,TO_ROOM);
            do_look(pet,"auto");
/*          pet = next; */
            break;
        }
        else
            pet = pet->next_in_room;
    }
/*    }*/
    return;
}



void spell_interred( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim, *pet;
  ROOM_INDEX_DATA *was_room;

 if( ch->blood < 20 )
   {
     send_to_char( "You haven't enough blood to work this magic.\n\r", ch );
     return;
   }

 ch->blood -= 20;

  if ( ( victim = get_char_world( ch, target_name, FALSE ) ) == NULL
       ||   victim == ch
       ||   victim->in_room == NULL
       ||   !can_see_room(ch,victim->in_room) 
       ||   IS_SET(victim->in_room->room_flags, ROOM_IMP_ONLY)
       ||   IS_SET(victim->in_room->room_flags, ROOM_GODS_ONLY)
       ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
       ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
       ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
       ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
       ||   !victim->in_room->area->linked
       ||   ch->move < 21
       ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_PORTAL))
       ||   (IS_SET(victim->in_room->room_flags,ROOM_SAFE) 
             && in_fightlag(ch) && !in_fightlag(victim) ))
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }	

  if( IS_SET( ch->in_room->trans_flags, TRANS_NO_PORTAL ) ||
      IS_SET( ch->in_room->trans_flags, TRANS_NO_PORTAL_OUT ) ||
      IS_SET( ch->in_room->trans_flags, TRANS_NO_SPELLS ) ||
      IS_SET( ch->in_room->trans_flags, TRANS_NO_SPELLS_OUT ) ||
      IS_SET( victim->in_room->trans_flags, TRANS_NO_PORTAL ) ||
      IS_SET( victim->in_room->trans_flags, TRANS_NO_PORTAL_IN ) ||
      IS_SET( victim->in_room->trans_flags, TRANS_NO_SPELLS ) ||
      IS_SET( victim->in_room->trans_flags, TRANS_NO_SPELLS_IN ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }

  if ( IS_AFFECTED(ch, AFF_CHARM)   
       &&   ch->master != NULL
       &&   ch->in_room == ch->master->in_room )
    {
      send_to_char( "What?  And leave your beloved master?\n\r", ch );
      return;
    }

  was_room = ch->in_room;
  act("$n melds into the ground and vanishes.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You meld into the ground and vanish.\n\r",ch);
  ch->was_in_room = ch->in_room;
  char_from_room(ch);
  ch->move -= 20;
  char_to_room(ch,victim->in_room);

  do_look(ch,"auto");
  act("$n rises from the ground before you.",ch,NULL,NULL,TO_ROOM);
  ch->was_in_room = NULL;

    for ( pet = was_room->people; pet != NULL; )
    {
        if ( ch->pet == pet ||
           ( IS_AFFECTED2( pet, AFF2_SUMMONEDPET ) && pet->master == ch ) )
        {
            act("$n melds into the ground and vanishes.",pet,NULL,NULL,TO_ROOM);
            send_to_char("You meld into the ground and vanish.\n\r",pet);
            char_from_room(pet);
            char_to_room(pet,victim->in_room);
            act("$n rises from the ground before you.",pet,NULL,NULL,TO_ROOM);
            do_look(pet,"auto");
	    break;
        }
        else
            pet = pet->next_in_room;
    }
    return;
}



void spell_giant_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn )
    ||   is_affected( victim, skill_lookup("enhanced strength") )
    ||   is_affected( victim, gsn_divine_focus ) )
    {
	if (victim == ch)
	  send_to_char("You are already as strong as you can get!\n\r",ch);
	else
	  act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 1 + (ch->level >= 18) + (ch->level >= 25) + (ch->level >= 32);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Your muscles surge with heightened power!\n\r", victim );
    act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_psionic_blast( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = UMAX(  20, victim->hit - dice(1,4) );
    if ( saves_spell( level,ch,victim,DAM_MENTAL) )
	dam = UMIN( 50, dam / 2 );
    dam = UMIN( 100, dam );
    damage_old( ch, victim, dam, sn, DAM_MENTAL ,TRUE);
    return;
}

/* RT haste spell */

void spell_haste( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE)
    ||   IS_SET(victim->off_flags,OFF_FAST))
    {
	if (victim == ch)
	  send_to_char("You can't move any faster!\n\r",ch);
 	else
	  act("$N is already moving as fast as $E can.",
	      ch,NULL,victim,TO_CHAR);
        return;
    }

    if (IS_AFFECTED(victim,AFF_SLOW))
    {
	if (!check_dispel(level,victim,skill_lookup("slow"))
        || (!check_dispel(level,victim,skill_lookup("warp time"))))
	{
	    if (victim != ch)
	        send_to_char("Spell failed.\n\r",ch);
	    send_to_char("You feel momentarily faster.\n\r",victim);
	    return;
	}
        act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    if (victim == ch)
      af.duration  = level/2;
    else
      af.duration  = level/4;
    af.location  = APPLY_DEX;
    af.modifier  = 1 + (ch->level >= 18) + (ch->level >= 25) + (ch->level >= 32);
    af.bitvector = AFF_HASTE;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself moving more quickly.\n\r", victim );
    act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->hit = UMIN( victim->hit + 100, victim->max_hit );
    update_pos( victim );
    send_to_char( "A warm feeling fills your body.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_ray( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  OBJ_DATA *obj_lose, *obj_next;
  int dam = 0;
  bool fail = TRUE;
 
  /*  if (!saves_spell(level + 2,victim,DAM_FIRE) 
      &&  !IS_SET(victim->imm_flags,IMM_FIRE)) */

  if( !IS_NPC( victim ) 
  &&  !IS_SET( victim->imm_flags, IMM_LIGHT ) )
    {
      for ( obj_lose = victim->carrying;
	    obj_lose != NULL; 
	    obj_lose = obj_next)
        {
	  obj_next = obj_lose->next_content;
	  if ( number_range(1,2 * level) > obj_lose->level 
	       &&   !saves_spell(level,ch,victim,DAM_LIGHT)
	       &&   !IS_OBJ_STAT(obj_lose,ITEM_NONMETAL)
	       &&   !IS_OBJ_STAT(obj_lose,ITEM_BURN_PROOF))
            {
	      switch ( obj_lose->item_type )
                {
               	case ITEM_ARMOR:
		  if (obj_lose->wear_loc != -1) /* remove the item */
		    {
		      if (can_drop_obj(victim,obj_lose)
			  &&  (obj_lose->weight / 10) < 
			  number_range(1,2 * get_curr_stat(victim,STAT_DEX))
			  &&  remove_obj( victim, obj_lose->wear_loc, TRUE ))
			{
			  act("$n yelps and throws $p to the ground!",
			      victim,obj_lose,NULL,TO_ROOM);
			  act("You remove and drop $p before it burns you.",
			      victim,obj_lose,NULL,TO_CHAR);
                    /*    This prevents problems if obj_next is the weapon and removing
                          obj would make the weapon drop */
                          player_quitting = TRUE;
			  obj_from_char(obj_lose);
                          player_quitting = FALSE;
			  obj_to_room(obj_lose, victim->in_room);
			  fail = FALSE;
			}
		      else /* stuck on the body! ouch! */
			{
			  act("Your skin is seared by $p!",
			      victim,obj_lose,NULL,TO_CHAR);
			  dam += (number_range(1,obj_lose->level));
			  fail = FALSE;
			}

		    }
		  else /* drop it if we can */
		    {
		      if (can_drop_obj(victim,obj_lose))
			{
			  act("$n yelps and throws $p to the ground!",
			      victim,obj_lose,NULL,TO_ROOM);
			  act("You and drop $p before it burns you.",
			      victim,obj_lose,NULL,TO_CHAR);
		    /*    This drops items out of inventory, so do not need
                          to specify player quitting */
			  obj_from_char(obj_lose);
			  obj_to_room(obj_lose, victim->in_room);
			  fail = FALSE;
			}
		      else /* cannot drop */
			{
			  act("Your skin is seared by $p!",
			      victim,obj_lose,NULL,TO_CHAR);
			  dam += (number_range(1,obj_lose->level) / 2);
			  fail = FALSE;
			}
		    }
		  break;
                case ITEM_WEAPON:
		  if (obj_lose->wear_loc != -1) /* try to drop it */
		    {
		      if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
			continue;

		      if (can_drop_obj(victim,obj_lose) 
			  &&  remove_obj(victim,obj_lose->wear_loc,TRUE))
			{
			  act("$n is burned by $p, and throws it to the ground.",
			      victim,obj_lose,NULL,TO_ROOM);
			  send_to_char(
				       "You throw your red-hot weapon to the ground!\n\r",
				       victim);
                          player_quitting = TRUE;
			  obj_from_char(obj_lose);
                          player_quitting = FALSE;
			  obj_to_room(obj_lose,victim->in_room);
			  fail = FALSE;
			}
		      else /* YOWCH! */
			{
			  send_to_char("Your weapon sears your flesh!\n\r",
				       victim);
			  dam += number_range(1,obj_lose->level);
			  fail = FALSE;
			}
		    }
		  else /* drop it if we can */
		    {
		      if (can_drop_obj(victim,obj_lose))
			{
			  act("$n throws a burning hot $p to the ground!",
			      victim,obj_lose,NULL,TO_ROOM);
			  act("You and drop $p before it burns you.",
			      victim,obj_lose,NULL,TO_CHAR);
			  /*dam += (number_range(1,obj_lose->level) / 6);*/
			  obj_from_char(obj_lose);
			  obj_to_room(obj_lose, victim->in_room);
			  fail = FALSE;
			}
		      else /* cannot drop */
			{
			  act("Your skin is seared by $p!",
			      victim,obj_lose,NULL,TO_CHAR);
			  dam += (number_range(1,obj_lose->level) / 2);
			  fail = FALSE;
			}
		    }
		  break;
		}
	    }
	}
    }
  if (fail)
    {
      send_to_char("Your sunbeam had no affect.\n\r", ch);
      send_to_char("You are momentarily blinded by a ray of light.\n\r",victim);
    }
  else /* damage! */
    {
      if (saves_spell(level,ch,victim,DAM_LIGHT))
	dam = 2 * dam / 3;
      damage_old(ch,victim,UMAX(1,dam),sn,DAM_LIGHT,TRUE);
    }
}

/* The new version by Dracar - His first aren't you all proud! */
void spell_holy_word(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int bless_num, heal_num, frenzy_num,refresh_num;

    bless_num = skill_lookup("bless");
    heal_num = skill_lookup("heal");
    frenzy_num = skill_lookup("frenzy");
    refresh_num = skill_lookup("refresh");

    act("$n utters a word of divine power!",ch,NULL,NULL,TO_ROOM);
    send_to_char("You utter a word of divine power.\n\r",ch);

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;

        if ( !IS_WIZI_AREA(ch,vch) &&
             ( (IS_NPC(ch) && IS_NPC(vch))
        ||    (!IS_NPC(ch) && !IS_NPC(vch))) )
        {
          send_to_char("You feel more powerful.\n\r",vch);
          spell_bless(bless_num,level,ch,(void *) vch,TARGET_CHAR);
	  spell_heal(heal_num,level,ch,(void *) vch,TARGET_CHAR);
	  spell_refresh(refresh_num,level,ch,(void *) vch,TARGET_CHAR);
	  spell_frenzy(frenzy_num,level,ch,(void *) vch,TARGET_CHAR);
        }
   }
}

static const char *growth_time[] = 
{
      "It should grow very quickly.\n\r",
      "It will grow in a short amount of time.\n\r",
      "It will grow in due time.\n\r",
      "It will take a while to grow.\n\r",
      "It will take a very long time to grow.\n\r"
};
 
void spell_identify( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    char weap[128];
    int bit;
    AFFECT_DATA *paf;

    if (IS_SET(obj->extra_flags,ITEM_NO_IDENTIFY))
     {
       send_to_char("Your magic cannot identify this item.\n\r",ch);
       return;
     }

    sprintf( buf,
	"Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\r",

	obj->name,
	item_name(obj->item_type),
	extra_bit_name( obj->extra_flags ),
	obj->weight / 10,
	obj->cost,
	obj->level
	);
    send_to_char( buf, ch );

    switch ( obj->item_type )
    {
    case ITEM_SCROLL: 
    case ITEM_POTION:
    case ITEM_PILL:
	sprintf( buf, "Level %d spells of:", obj->value[0] );
	send_to_char( buf, ch );

	if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[1]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[2]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	{
	    send_to_char(" '",ch);
	    send_to_char(skill_table[obj->value[4]].name,ch);
	    send_to_char("'",ch);
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_WAND: 
    case ITEM_STAFF: 
	sprintf( buf, "Has %d charges of level %d",
	    obj->value[2], obj->value[0] );
	send_to_char( buf, ch );
      
	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_DRINK_CON:
        sprintf(buf,"It holds %s.\n\r",
            liq_table[obj->value[2]].liq_name);
        send_to_char(buf,ch);
        break;

/* Books are purposely left out here so that they look different from containers
   to players.  Adjust later if needed.  cf. act_wiz.c 9/2/2011 gkl */
/*  case ITEM_BOOK: */
    case ITEM_CONTAINER:
	sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
	    obj->value[0], obj->value[3], 
            flag_string( container_flags, obj->value[1] ) );
	send_to_char(buf,ch);
	if (obj->value[4] != 100 )
	{
	    sprintf(buf,"Weight multiplier: %d%%\n\r",
		obj->value[4]);
	    send_to_char(buf,ch);
	}
	break;
		
    case ITEM_WEAPON:
	switch (obj->value[0])
	{
	    case(WEAPON_EXOTIC) : strcpy( weap, "exotic");                break;
	    case(WEAPON_SWORD)  : strcpy( weap, "a sword");               break;
	    case(WEAPON_DAGGER) : strcpy( weap, "a dagger");	          break;
	    case(WEAPON_SPEAR)	: strcpy( weap, "a spear");	          break;
	    case(WEAPON_MACE) 	: strcpy( weap, "a mace");	          break;
	    case(WEAPON_AXE)	: strcpy( weap, "an axe") ;               break;
	    case(WEAPON_FLAIL)	: strcpy( weap, "a flail");	          break;
	    case(WEAPON_WHIP)	: strcpy( weap, "a whip");	          break;
	    case(WEAPON_POLEARM): strcpy( weap, "a polearm");	          break;
	    case(WEAPON_STAFF)  : strcpy( weap, "a staff");	          break;
            case(WEAPON_HAND)   : strcpy( weap, "hand to hand");          break;
	    case(WEAPON_BOW)	: strcpy( weap, "a bow");                 break;
            default		: strcpy( weap, "of an unknown type");    break;
 	}
        bit = dam_to_imm( attack_table[obj->value[3]].damage );
        if ( strcmp( flag_string( imm_flags, bit ), attack_table[obj->value[3]].noun ) )
        {
          sprintf(buf, "Weapon is %s and does %s (%s) damage.\n\r",
            weap, attack_table[obj->value[3]].noun, 
            flag_string( imm_flags, bit ) );
        }
        else
        {
          sprintf(buf, "Weapon is %s and does %s damage.\n\r",
            weap, attack_table[obj->value[3]].noun);
        }
        send_to_char( buf, ch );

	if (obj->pIndexData->new_format)
	    sprintf(buf,"Damage is %dd%d (average %d).\n\r",
		obj->value[1],obj->value[2],
		(1 + obj->value[2]) * obj->value[1] / 2);
	else
	    sprintf( buf, "Damage is %d to %d (average %d).\n\r",
	    	obj->value[1], obj->value[2],
	    	( obj->value[1] + obj->value[2] ) / 2 );
	send_to_char( buf, ch );
        if (obj->value[4])  /* weapon flags */
        {
            sprintf(buf,"Special weapons attributes: %s\n\r",weapon_bit_name(obj->value[4]));
            send_to_char(buf,ch);
        }
	break;

    case ITEM_ARMOR:
	sprintf( buf, 
	"Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r", 
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	send_to_char( buf, ch );
	break;
    case ITEM_FURNITURE:
        {
            char buf2[MAX_STRING_LENGTH];
            /* check hp/moves */
            if ( obj->value[3] > 100 )          /* boosts regen */
            {
                sprintf( buf, "Increases hp recovery rate by %d%%.\n\r", 
                  obj->value[3] - 100 );
                sprintf(buf2, "Increases moves recovery rate by %d%%.\n\r",
                  obj->value[3] - 100 );
            }
            else if ( obj->value[3] == 100 )    /* does nothing */
            {
                sprintf( buf, "Does not affect hp recovery rate.\n\r" );
                sprintf(buf2, "Does not affect moves recovery rate.\n\r" );
            }
            else if ( obj->value[3] == 0 )      /* halts regen  */
            {
                sprintf( buf, "Stops hp recovery.\n\r" );
                sprintf(buf2, "Stops moves recovery.\n\r" );
            }
            else if ( obj->value[3] < 0 )       /* damages you  */
            {
                sprintf( buf, "Drains hp at %d%% of normal recovery rate.\n\r",
                   obj->value[3]);
              sprintf(buf2, "Drains moves at %d%% of normal recovery rate.\n\r",
                   obj->value[3]);
            }
            send_to_char( buf, ch );
            /* check mana */
            if ( obj->value[4] > 100 )          /* boosts regen */
                sprintf( buf, "Increases mana recovery rate by %d%%.\n\r", 
                  obj->value[4] - 100 );
            else if ( obj->value[4] == 100 )    /* does nothing */
                sprintf( buf, "Does not affect mana recovery rate.\n\r" );
            else if ( obj->value[4] == 0 )      /* halts regen  */
                sprintf( buf, "Stops mana recovery.\n\r" );
            else if ( obj->value[4] < 0 )       /* damages you  */
               sprintf( buf, "Drains mana at %d%% of normal recovery rate.\n\r",
                   obj->value[4]);
            send_to_char( buf, ch );
            send_to_char( buf2, ch );
        }
	break;
    case ITEM_SEED:
        if      ( obj->value[3] >  399 )     send_to_char(growth_time[4], ch);
        else if ( obj->value[3] >  319 )     send_to_char(growth_time[3], ch);
        else if ( obj->value[3] >  239 )     send_to_char(growth_time[2], ch);
        else if ( obj->value[3] >  159 )     send_to_char(growth_time[1], ch);
        else                                 send_to_char(growth_time[0], ch);
        break;
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_char(buf,ch);
            if (paf->bitvector)
            {
                switch(paf->where)
                {
                    case TO_AFFECTS:
                        sprintf(buf,"Adds %s affect.\n",
                            flag_string( affect_flags, paf->bitvector));
                        break;
                    case TO_OBJECT:
                        sprintf(buf,"Adds %s object flag.\n",
                            extra_bit_name(paf->bitvector));
                        break;
                    case TO_IMMUNE:
                        sprintf(buf,"Adds immunity to %s.\n",
                            flag_string( imm_flags, paf->bitvector ) );
/*                          imm_bit_name(paf->bitvector)); */
                        break;
                    case TO_RESIST:
                        sprintf(buf,"Adds resistance to %s.\n\r",
                            flag_string( res_flags, paf->bitvector ) );
/*                          imm_bit_name(paf->bitvector)); */
                        break;
                    case TO_VULN:
                        sprintf(buf,"Adds vulnerability to %s.\n\r",
                            flag_string( vuln_flags, paf->bitvector ) );
/*                          imm_bit_name(paf->bitvector));*/
                        break;
                    default:
                        sprintf(buf,"Unknown bit %d: %d\n\r",
                            paf->where,paf->bitvector);
                        break;
                }
	        send_to_char( buf, ch );
	    }
	}
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d",
	    	affect_loc_name( paf->location ), paf->modifier );
	    send_to_char( buf, ch );
            if ( paf->duration > -1)
                sprintf(buf,", %d hours.\n\r",paf->duration);
            else
                sprintf(buf,".\n\r");
	    send_to_char(buf,ch);
            if (paf->bitvector)
            {
                switch(paf->where)
                {
                    case TO_AFFECTS:
                        sprintf(buf,"Adds %s affect.\n",
                            flag_string(affect_flags, paf->bitvector));
                        break;
                    case TO_OBJECT:
                        sprintf(buf,"Adds %s object flag.\n",
                            extra_bit_name(paf->bitvector));
                        break;
		    case TO_WEAPON:
			sprintf(buf,"Adds %s weapon flags.\n",
			    weapon_bit_name(paf->bitvector));
			break;
                    case TO_IMMUNE:
                        sprintf(buf,"Adds immunity to %s.\n",
                            flag_string( imm_flags, paf->bitvector ) );
/*                          imm_bit_name(paf->bitvector));*/
                        break;
                    case TO_RESIST:
                        sprintf(buf,"Adds resistance to %s.\n\r",
                            flag_string( res_flags, paf->bitvector ) );
/*                          imm_bit_name(paf->bitvector));*/
                        break;
                    case TO_VULN:
                        sprintf(buf,"Adds vulnerability to %s.\n\r",
                            flag_string( vuln_flags, paf->bitvector ) );
/*                          imm_bit_name(paf->bitvector));*/
                        break;
                    default:
                        sprintf(buf,"Unknown bit %d: %d\n\r",
                            paf->where,paf->bitvector);
                        break;
                }
                send_to_char(buf,ch);
            }
	}
	if (paf->location == APPLY_SPELL_AFFECT) {
	  sprintf(buf, "Adds spell affect '%s'.\n\r",
		  skill_table[paf->type].name);
	  send_to_char(buf, ch);
	}
    }

    if( strlen( obj->donor ) > 0 )
      {
	printf_to_char( ch, "Originally donated by %s\n\r", obj->donor );
      }

    return;
}



void spell_infravision( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_INFRARED) )
    {
	if (victim == ch)
	  send_to_char("You can already see in the dark.\n\r",ch);
	else
	  act("$N already has infravision.\n\r",ch,NULL,victim,TO_CHAR);
	return;
    }
    act( "$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM );

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 2 * level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes glow red.\n\r", victim );
    return;
}



void spell_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* object invisibility */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;	

	if (IS_OBJ_STAT(obj,ITEM_INVIS))
	{
	    act("$p is already invisible.",ch,obj,NULL,TO_CHAR);
	    return;
	}
	
	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= level + 12;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= ITEM_INVIS;
	affect_to_obj(obj,&af);

	act("$p fades out of sight.",ch,obj,NULL,TO_ALL);
	return;
    }

    /* character invisibility */
    victim = (CHAR_DATA *) vo;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE) )
	return;

    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level + 12;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char( victim, &af );
    send_to_char( "You fade out of existence.\n\r", victim );
    return;
}

void spell_lightning_bolt(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0, 25, 28,
	31, 34, 37, 40, 40,	41, 42, 42, 43, 44,
	44, 45, 46, 46, 47,	48, 48, 49, 50, 50,
	51, 52, 52, 53, 54,	54, 55, 56, 56, 57,
	58, 58, 59, 60, 60,	61, 62, 62, 63, 64
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell(level,ch,victim,DAM_LIGHTNING) )
	dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE);
    return;
}



void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  char buf[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  bool found;
  int number = 0, max_found;

  found = FALSE;
  number = 0;
  max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;

  buffer = new_buf();
 
  for ( obj = object_list; obj != NULL; obj = obj->next )
    {
      if ( !can_see_obj( ch, obj ) || !is_name( target_name, obj->name ) 
	   ||   IS_OBJ_STAT(obj,ITEM_NOLOCATE) || number_percent() > 2 * level
	   ||   ch->level < obj->level )
	continue;

      for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
        ;

      if  ( in_obj->carried_by 
      &&  ( IS_IMMORTAL( in_obj->carried_by )
      ||  ( !IS_IMMORTAL( ch ) && IS_AFFECTED3( in_obj->carried_by, AFF_VEIL ) ) ) )
	continue;

      found = TRUE;
      number++;

      if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by))
	{
/*	  if( !IS_IMMORTAL( ch ) && IS_AFFECTED3( in_obj->carried_by, AFF_VEIL ) )
	    {
	      sprintf( buf, "%s is in somewhere.\n\r",
		       obj->short_descr );
	    }
	  else
	    {*/
	      sprintf( buf, "%s is carried by %s\n\r",
		       obj->short_descr, PERS2(in_obj->carried_by, ch) );
/*	    }*/
	}
      else
	{
	  if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
	    sprintf( buf, "%s is in {c%s{x [Room %d]\n\r",
		     in_obj->short_descr,
		     in_obj->in_room->name, in_obj->in_room->vnum);
	  else 
	    sprintf( buf, "%s is in {c%s{x\n\r",
		     in_obj->short_descr,
		     in_obj->in_room == NULL
		     ? "somewhere" : in_obj->in_room->name );
	}

      buf[0] = UPPER(buf[0]);
      add_buf(buffer,buf);

      if (number >= max_found)
	break;
    }

  if ( !found )
    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
  else
    page_to_char(buf_string(buffer),ch);

  free_buf(buffer);

  return;
}



void spell_magic_missile( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 3,  3,  4,  4,  5,	 6,  6,  6,  6,  6,
	 7,  7,  7,  7,  7,	 8,  8,  8,  8,  8,
	 9,  9,  9,  9,  9,	10, 10, 10, 10, 10,
	11, 11, 11, 11, 11,	12, 12, 12, 12, 12,
	13, 13, 13, 13, 13,	14, 14, 14, 14, 14
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell(level,ch,victim,DAM_ENERGY) )
	dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_ENERGY ,TRUE);
    return;
}

void spell_mass_healing(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *gch;
    int heal_num, refresh_num;
    
    heal_num = skill_lookup("heal");
    refresh_num = skill_lookup("refresh"); 

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if (!IS_WIZI_AREA(ch,gch) && ((IS_NPC(ch) && IS_NPC(gch)) ||
	    (!IS_NPC(ch) && !IS_NPC(gch))))
	{
	    spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);
	    spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR);  
	}
    }
}
	    

void spell_mass_invis( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_AFFECTED(gch, AFF_INVISIBLE) 
        ||   IS_WIZI_AREA(ch,gch) )
	    continue;
	act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly fade out of existence.\n\r", gch );

	af.where     = TO_AFFECTS;
	af.type      = sn;
    	af.level     = level/2;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVISIBLE;
	affect_to_char( gch, &af );
    }
    send_to_char( "Ok.\n\r", ch );

    return;
}



void spell_null( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    send_to_char( "That's not a spell!\n\r", ch );
    return;
}



void spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
	if (victim == ch)
	  send_to_char("You are already out of phase.\n\r",ch);
	else
	  act("$N is already shifted out of phase.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );
    act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You turn translucent.\n\r", victim );
    return;
}

/* RT plague spell, very nasty */

void spell_plague( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( saves_spell(level,ch,victim,DAM_DISEASE)
    ||  (IS_NPC(victim) && IS_SET(victim->act_bits,ACT_UNDEAD)) )
    {
	if (ch == victim)
	  send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
	else
	  act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( IS_AFFECTED3(victim,AFF3_SOLEMN)
    &&   number_percent() > CONFIG_SOLEMN_VOWS_CHANCE )
    {
      send_to_char("The power of your vows shield you from taint.\n\r", victim);
      if ( ch != victim )
        act("$N's vows protect $M from your spell.", ch, NULL, victim, TO_CHAR);
      return;
    }

    af.where     = TO_AFFECTS;
    af.type 	  = sn;
    af.level	  = level * 3/4;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = -5; 
    af.bitvector = AFF_PLAGUE;
    affect_join( victim, &af, FALSE );
   
    send_to_char
      ("You scream in agony as plague sores erupt from your skin.\n\r",victim);
    act("$n screams in agony as plague sores erupt from $s skin.",
	victim,NULL,NULL,TO_ROOM);
}

/* New Psi Spell */
void spell_headache( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    if(IS_AFFECTED2(victim,AFF_HEADACHE))
    {
	send_to_char("Their head can't hurt more then it is already.\n\r",ch);
	return;
    }
    if (saves_spell(level,ch,victim,DAM_MENTAL) || 
        (IS_NPC(victim) && IS_SET(victim->act_bits,ACT_UNDEAD)))
    {
	if (ch == victim)
	  send_to_char("Your head throbs for a moment.\n\r",ch);
	else
	  act("$N is unaffected.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	  = sn;
    af.level	  = level;
    af.duration  = 0;
    af.location  = APPLY_INT;
    af.modifier  = -5; 
    af.bitvector = AFF_HEADACHE;
    affect_to_char2(victim, &af);   
    send_to_char
      ("You scream as your head begins to pound!\n\r",victim);
    act("$n screams in helplessness as $s head begins to pound.",
	victim,NULL,NULL,TO_ROOM);
}


void spell_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;


    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
	{
	    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	    {
		act("Your spell fails to corrupt $p.",ch,obj,NULL,TO_CHAR);
		return;
	    }
	    obj->value[3] = 1;
	    act("$p is infused with poisonous vapors.",ch,obj,NULL,TO_ALL);
	    return;
	}

	if (obj->item_type == ITEM_WEAPON)
	{
	    if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
	    ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
	    ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
		||  IS_WEAPON_STAT(obj,WEAPON_ANGELIC)
	    ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
	    ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
	    ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
	    ||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	    {
		act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
		return;
	    }

	    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	    {
		act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
		return;
	    }

	    af.where	 = TO_WEAPON;
	    af.type	 = sn;
	    af.level	 = level / 2;
	    af.duration	 = level/8;
 	    af.location	 = 0;
	    af.modifier	 = 0;
	    af.bitvector = WEAPON_POISON;
	    affect_to_obj(obj,&af);

	    act("$p is coated with deadly venom.",ch,obj,NULL,TO_ALL);
	    return;
	}

	act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
	return;
    }

    victim = (CHAR_DATA *) vo;

    if ( saves_spell(level,ch,victim,DAM_POISON) )
    {
	act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
	return;
    }

    if ( IS_AFFECTED3(victim,AFF3_SOLEMN)
    &&   number_percent() > CONFIG_SOLEMN_VOWS_CHANCE )
    {
      send_to_char("The power of your vows shield you from taint.\n\r", victim);
      if ( ch != victim )
        act("$N's vows protect $M from your spell.", ch, NULL, victim, TO_CHAR);
      return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = -2;
    af.bitvector = AFF_POISON;
    affect_join( victim, &af, TRUE );
    send_to_char( "You feel very sick.\n\r", victim );
    act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_recharge( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int chance, percent;
    bool success = FALSE;

    if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF)
    {
	send_to_char("That item does not carry charges.\n\r",ch);
	return;
    }

    if (obj->value[0] > LEVEL_HERO )
    {
	send_to_char("Your skills are not great enough for that.\n\r",ch);
	return;
    }

    if ( obj->value[2] >= obj->value[1] )
    {
	send_to_char("It cannot be charged further.\n\r",ch);
	return;
    }

    chance = 40 + 2 * level;

    chance -= obj->value[0];
    chance -= (obj->value[2] - obj->value[1]) *
	      (obj->value[2] - obj->value[1]);

    chance = UMAX( level/2, chance );
    chance = UMIN( 95, chance );

    if ( (percent = number_percent()) < chance / 2 )
    {
	obj->value[2] = UMAX(obj->value[1],obj->value[2]);
	obj->value[1] /= 2;
        success = TRUE;
    }
    else if (percent <= chance)
    {
	int chargeback, chargemax;

	chargemax = obj->value[1] - obj->value[2];
	
	if (chargemax > 0)
	    chargeback = UMAX(1,chargemax * percent / 100);
	else
	    chargeback = 0;

        if ( chargeback > 0 ) 
        {
            obj->value[2] += chargeback;
            obj->value[1] /= 2;
/*          obj->value[1] = 2 * obj->value[1] / 3; */
            success = TRUE;
        }
        else
            success = FALSE;
    }
    else 
    {
        success = FALSE;
    }

    if ( success )
    {
	act("$p crackles and glows.",ch,obj,NULL,TO_CHAR);
	act("$p crackles and glows.",ch,obj,NULL,TO_ROOM);
    }
    else if ( !success && obj->value[1] > 1 )
    {
        send_to_char("Nothing seems to happen.\n\r",ch);
        if (obj->value[1] > 1 && percent <= UMIN(75, 3 * chance / 2))
            obj->value[1]--;
    }    /* This is redundant because if v1==1, it must be at full charge   */
    else /* (v2>=v1==1) or an error has occurred and somehow the wand is at */
    {    /* zero current charges.  In case of error, wand will blow here.   */
        act("$p glows brightly and explodes!",ch,obj,NULL,TO_CHAR);
        act("$p glows brightly and explodes!",ch,obj,NULL,TO_ROOM);
        extract_obj(obj);
    }
}

void spell_reflect( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_REFLECT ) )
    {
      send_to_char( "You are already surrounded by an aura of reflection.\n\r",
          ch);
      return;
    }

    af.where     = TO_AFFECTS;
    af.type      = gsn_reflect;
    af.level     = level;
    af.duration  = level/5;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_REFLECT;

    affect_to_char2( victim, &af );

    send_to_char("You surround yourself in an aura of reflection.\n\r", victim);
    return;
}

void spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->move = UMIN( victim->move + level, victim->max_move );
    if (victim->max_move == victim->move)
        send_to_char("You feel fully refreshed!\n\r",victim);
    else
        send_to_char( "You feel less tired.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool found = FALSE;

    /* do object cases first */
    if (target == TARGET_OBJ)
    {
        obj = (OBJ_DATA *) vo;

        if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
        {
            if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE)
            &&  !saves_dispel(level + 2,obj->level,0))
            {
                REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
                REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
                act("$p glows blue.",ch,obj,NULL,TO_ALL);
                return;
            }

            act("The curse on $p is beyond your power.",ch,obj,NULL,TO_CHAR);
            return;
        }
        act("There doesn't seem to be a curse on $p.",ch,obj,NULL,TO_CHAR);
        return;
    }

    /* characters */
    
    victim = (CHAR_DATA *) vo;

    /* Only healers can uncurse in safe - Lars 7/22/99 */
    if ( ch != victim
    && ( IS_SET(victim->in_room->room_flags, ROOM_SAFE) 
         || (IS_SET(victim->in_room->room_flags,ROOM_SEMISAFE) 
             && !in_fightlag(victim) ) )
    && ( !( IS_NPC(ch) && IS_SET(ch->act_bits, ACT_IS_HEALER) ) ) )
    {
        send_to_char("You can't do that here.\n\r", ch);
        return;
    }
    /* End new code (Lars 7/22/99) */


    if( check_dispel( level, victim, gsn_bolas ) )
    {
        send_to_char( "The bolas fall away.\n\r", victim );
        act( "The bolas entangling $n fall away.\n\r", 
            victim, NULL, NULL, TO_ROOM );
    }
    if (check_dispel(level,victim,gsn_curse))
    {
        send_to_char("You feel better.\n\r",victim);
        act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
    }
    if (check_dispel(level,victim,gsn_mind_whip))
    {
        send_to_char("Your mind feels less tortured.\n\r",victim);
        act("The mental anguish leaves $n.",victim,NULL,NULL,TO_ROOM);
    }
    if (check_dispel(level,victim,gsn_twrath))
    {
        send_to_char("You no longer suffer Tyrin's Wrath.\n\r",victim);
        act(" $n no longer suffers the Wrath of Tyrin.",
            victim,NULL,NULL,TO_ROOM);
    }
    if (check_dispel(level,victim,gsn_terror))
    {
        send_to_char("You no longer feel the Terror of the Rain.\n\r",
            victim);
        act(" $n no longer feels the Terror of the Rain.",
            victim,NULL,NULL,TO_ROOM);
    }
    if (check_dispel(level,victim,gsn_profane_word))
    {
        send_to_char("You free yourself from the curse of a profane word.\n\r",
            victim);
        act(" $n frees $mself from the affliction of a profane word.",
            victim,NULL,NULL,TO_ROOM);
    }
	
	if (check_dispel(level,victim,gsn_contrition))
    {
        send_to_char("You no longer feel the anguish of your penance.\n\r",
            victim);
        act("$n looks less sorrowful.",
            victim,NULL,NULL,TO_ROOM);
    }
	

    for (obj= victim->carrying; (obj != NULL && !found); obj= obj->next_content)
    {
        if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
        &&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
        {   /* attempt to remove curse */
            if (!saves_dispel(level,obj->level,0))
            {
                found = TRUE;
                REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
                REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
                act("Your $p glows blue.",victim,obj,NULL,TO_CHAR);
                act("$n's $p glows blue.",victim,obj,NULL,TO_ROOM);
            }
        }
    }
}

void spell_pinch( int sn, int level, CHAR_DATA* ch, void* vo, int target )
{
  CHAR_DATA* victim = ( CHAR_DATA* ) vo;
  AFFECT_DATA af;

  if( IS_AFFECTED3( victim, AFF_PINCH ) )
    {
      if( victim == ch )
	{
	  send_to_char( "You already have an ethereal hand waiting to pinch you.\n\r", ch );
	}
      else
	{
	  act( "$N already has an ethereal hand waiting to pinch $S.\n\r", ch, NULL, victim, TO_CHAR );
	}

      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_PINCH;
  affect_to_char3( victim, &af );

  act( "An ethereal hand materializes near $n.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "An ethereal hand materializes and hovers near you.\n\r", victim );

}

/*
 * haunt - more or less the same as pinch                       10/25/2011 gkl
 */
void spell_haunt( int sn, int level, CHAR_DATA* ch, void* vo, int target )
{
    CHAR_DATA* victim = ( CHAR_DATA* ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED3( victim, AFF_PINCH ) )
    {
        if ( victim == ch )
	    {
	        send_to_char( 
                "You are already haunted by spirits of the dead.\n\r", 
                ch );
	    }
        else
	    {
	        act( "$N is already haunted by spirits of the dead.", 
                ch, NULL, victim, TO_CHAR );
	    }
        return;
    }

    af.where    = TO_AFFECTS;
    af.type     = sn;
    af.level    = level;
    af.duration = level;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_PINCH;
    affect_to_char3( victim, &af );

    act( "Cold, dead air begins to surround $n.", 
        victim, NULL, NULL, TO_ROOM );
    send_to_char( 
        "An uneasy chill runs up your spine as spirits begin haunting your thoughts.\n\r", 
        victim );

    return;
} 

void spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
	if (victim == ch)
	  send_to_char("You are already in sanctuary.\n\r",ch);
	else
	  act("$N is already in sanctuary.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a white aura.\n\r", victim );
    return;
}

void spell_shadow( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_SNEAK) )
    {
	if (victim == ch)
	  send_to_char("You are already melded with the shadows.\n\r",ch);
	else
	  act("$N is already melded with the shadows.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SNEAK;
    affect_to_char( victim, &af );
    act( "$n silently fades into the shadows.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You silently meld within the shadows.\n\r", victim );
    return;
}



void spell_shadows( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2(victim, AFF_SHADOWS) )
    {
	if (victim == ch)
	  send_to_char("You have already gathered the shadows about you.\n\r",ch);
	else
	  act("$N is already melded with the shadows.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.whichaff  = AFF2;
    af.duration  = level/2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SHADOWS;
    affect_to_char2( victim, &af );
    act( "$n gathers the shadows about them.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You gather the shadows about you.\n\r", victim );
    return;
}



void spell_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already shielded from harm.\n\r",ch);
	else
	  act("$N is already protected by a shield.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if(is_affected(victim,skill_lookup("mental barrier")))
    {
        if (victim == ch)
          send_to_char("You already seem protected in this manner.\n\r",ch);
        else
          act("$N is already protected by a shield.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 8 + level;
    af.location  = APPLY_AC;
    af.modifier  = -20;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a force shield.\n\r", victim );
    return;
}



void spell_shocking_grasp(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0, 20, 25, 29, 33,
	36, 39, 39, 39, 40,	40, 41, 41, 42, 42,
	43, 43, 44, 44, 45,	45, 46, 46, 47, 47,
	48, 48, 49, 49, 50,	50, 51, 51, 52, 52,
	53, 53, 54, 54, 55,	55, 56, 56, 57, 57
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level,ch,victim,DAM_LIGHTNING) )
	dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE);
    return;
}



void spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
  
    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||   (IS_NPC(victim) && IS_SET(victim->act_bits,ACT_UNDEAD))
    ||   (level + 2) < victim->level
    ||   saves_spell( level-4,ch,victim,DAM_CHARM) )
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af, FALSE );

    if ( IS_AWAKE(victim) )
    {
	send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
	act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
	victim->position = POS_SLEEPING;
    }
    return;
}

/* If you change spell_slow, it may also be a good idea to change spell_judgment
   in templar.c, as it emulates this spell */
void spell_slow( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
    {
        if (victim == ch)
          send_to_char("You can't move any slower!\n\r",ch);
        else
          act("$N can't get any slower than that.",
              ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (saves_spell(level,ch,victim,DAM_OTHER) 
    ||  IS_SET(victim->imm_flags,IMM_MAGIC) )
    {
	if (victim != ch)
            send_to_char("Nothing seemed to happen.\n\r",ch);
        send_to_char("You feel momentarily lethargic.\n\r",victim);
        return;
    }

    if ( IS_AFFECTED3(victim,AFF3_SOLEMN)
    &&   number_percent() > CONFIG_SOLEMN_VOWS_CHANCE )
    {
      send_to_char("The power of your vows shield you from taint.\n\r", victim);
      if ( ch != victim )
        act("$N's vows protect $M from your spell.", ch, NULL, victim, TO_CHAR);
      return;
    }
 
    if (IS_AFFECTED(victim,AFF_HASTE))
    {
        if (!check_dispel(level,victim,skill_lookup("haste"))
        || (!check_dispel(level,victim,skill_lookup("adrenaline rush"))))
        {
	    if (victim != ch)
            	send_to_char("Spell failed.\n\r",ch);
            send_to_char("You feel momentarily slower.\n\r",victim);
            return;
        }

        act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
        return;
    }
 

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_DEX;
    af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32);
    af.bitvector = AFF_SLOW;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
    act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
    return;
}




void spell_stone_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your skin is already as hard as a rock.\n\r",ch); 
	else
	  act("$N is already as hard as can be.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if(is_affected(victim,skill_lookup("steel skin")))
    {
        if (victim == ch)
          send_to_char("You already seem protected in this manner.\n\r",ch);
        else
          act("$N is already protected in this manner.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -40;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your skin turns to stone.\n\r", victim );
    return;
}



void spell_summon( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;

    if ( ( victim = get_char_world( ch, target_name, FALSE ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    /* if the character is safe due to saferoom or semisafe */
    ||   ( (IS_SET(victim->in_room->room_flags, ROOM_SAFE) 
           || (IS_SET(victim->in_room->room_flags,ROOM_SEMISAFE) 
               && !in_fightlag(victim) ) )
    /* and not veiled--can always summon a veiled person */
          && !IS_AFFECTED3( victim, AFF_VEIL ) )
    /* can't summon people out of certain rooms */
    ||   IS_SET(victim->in_room->room_flags, ROOM_GODS_ONLY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_GODS_ONLY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_IMP_ONLY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_SUMMON)
    /* can't summon mobs */
    ||   IS_NPC(victim)
    /* can't summon imms */
    ||   victim->level >= LEVEL_IMMORTAL
    /* cannot summon BRs who don't want to be summoned */
    ||   (IS_SET(victim->act_bits,PLR_NOSUMMON) 
         && ch->level > victim->level + 6) 
    /* cannot summon someone affected by natures pestilence - 9/3/12 gkl */
    ||   (IS_AFFECTED(victim, AFF_CURSE) 
         && is_affected(victim, gsn_natures_pestil)) )

    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if( IS_SET( ch->in_room->trans_flags, TRANS_NO_SUMMON ) ||
	IS_SET( ch->in_room->trans_flags, TRANS_NO_SUMMON_IN ) ||
	IS_SET( ch->in_room->trans_flags, TRANS_NO_SPELLS ) ||
	IS_SET( ch->in_room->trans_flags, TRANS_NO_SPELLS_IN ) ||
	IS_SET( victim->in_room->trans_flags, TRANS_NO_SUMMON ) ||
	IS_SET( victim->in_room->trans_flags, TRANS_NO_SUMMON_OUT ) ||
	IS_SET( victim->in_room->trans_flags, TRANS_NO_SPELLS ) ||
	IS_SET( victim->in_room->trans_flags, TRANS_NO_SPELLS_OUT ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }

/*  if( ch->saving_throw > -25 && !IS_SET( ch->act_bits, PLR_NOSUMMON ) )
      {
	send_to_char( "You failed.\n\r", ch );
	return;
      }
*/

    if (IS_AFFECTED2(victim,AFF_GHOST)){
	send_to_char( "Your spell will not work on ghosts.\n\r", ch );
	return;
       }
    
    if (IS_SET(victim->act_bits,PLR_NOSUMMON) 
    &&  saves_spell(level,ch,victim,DAM_NONE) 
    &&  !IS_NPC(ch))
    {
        send_to_char( "You failed.\n\r", ch);
          if (IS_AFFECTED(victim, AFF_DETECT_MAGIC)){
           send_to_char("A vortex appears in the air pulling you towards it.\n\r",victim);
           }
        return;

       }

    if (IS_SET(victim->act_bits,PLR_NOSUMMON))
    {
/*      ch->fightlag = FIGHT_LAG; */
        if ( !IS_NPC(ch) )
        {
            ch->pcdata->lastfight = current_time;
            ch->pcdata->lastpk = current_time;
        }
    }

    act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
    victim->was_in_room = victim->in_room;
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act( "$n has summoned you!", ch, NULL, victim,   TO_VICT );
    do_look( victim, "auto" );
    act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
    victim->was_in_room = NULL;

    return;
}



void spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  ROOM_INDEX_DATA *pRoomIndex;
  
  if( IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
      || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
      || ( victim != ch && IS_SET( victim->imm_flags,IMM_SUMMON ) )
      || ch->move < 11
      || IS_AFFECTED( victim,AFF_CURSE )
      || ( victim != ch && ( saves_spell( level - 5,ch, victim, DAM_OTHER ) ) ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }

  if( IS_SET( victim->in_room->trans_flags, TRANS_NO_TELEPORT ) ||
      IS_SET( victim->in_room->trans_flags, TRANS_NO_TELEPORT_OUT ) ||
      IS_SET( victim->in_room->trans_flags, TRANS_NO_SPELLS ) ||
      IS_SET( victim->in_room->trans_flags, TRANS_NO_SPELLS_OUT ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }
	
	
	if (is_affected(ch, skill_lookup("hallowed discord"))) {
		do_hallowed_gate( ch );
		return;
	}
	

  /* code to prevent certain target rooms */
    pRoomIndex = get_random_room(victim);

    if (victim != ch)
	send_to_char("You have been teleported!\n\r",victim);

    ch->move -= 10;

    victim->was_in_room = victim->in_room;
    act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    do_look( victim, "auto" );
    act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
    victim->was_in_room = NULL;
    return;
}



void spell_ventriloquate( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;

    target_name = one_argument( target_name, speaker );

    sprintf( buf1, "%s says {s'%s'{x.\n\r",              speaker, target_name );
    sprintf( buf2, "Someone makes %s say {s'%s'{x.\n\r", speaker, target_name );
    buf1[0] = UPPER(buf1[0]);

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if (!is_exact_name( speaker, vch->name) && IS_AWAKE(vch))
	    send_to_char( saves_spell(level,ch,vch,DAM_OTHER) ? buf2 : buf1, vch );
    }

    return;
}



/* If you change spell_weaken, you may also want to change spell_judgment
   in templar.c which emulates this spell */
void spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn )
    ||   saves_spell( level,ch, victim,DAM_OTHER) )
    {
        send_to_char("Your spell had no effect.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED3(victim,AFF3_SOLEMN)
    &&   number_percent() > CONFIG_SOLEMN_VOWS_CHANCE )
    {
      send_to_char("The power of your vows shield you from taint.\n\r", victim);
      if ( ch != victim )
        act("$N's vows protect $M from your spell.", ch, NULL, victim, TO_CHAR);
      return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 2;
    af.location  = APPLY_STR;
    af.modifier  = -1 * (level / 5);
    af.bitvector = AFF_WEAKEN;
    affect_to_char( victim, &af );
    send_to_char( "You feel your strength slip away.\n\r", victim );
    act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
    return;
}



/* RT recall spell is back */

void spell_word_of_recall( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *location, *pRoomIndex;
    
    if (IS_NPC(victim))
      return;

    if (victim != ch){
      send_to_char("You can only do this to yourself.\n\r",ch);
      return;
      }
   
    if ((location = get_room_index( ROOM_VNUM_TEMPLE)) == NULL)
    {
	send_to_char("You are completely lost.\n\r",victim);
	return;
    } 

    if (IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL) ||
	IS_AFFECTED(victim,AFF_CURSE))
    {
	send_to_char("Spell failed.\n\r",victim);
	return;
    }

    if( IS_SET( victim->in_room->trans_flags, TRANS_NO_RECALL ) ||
	IS_SET( victim->in_room->trans_flags, TRANS_NO_SPELLS ) ||
	IS_SET( victim->in_room->trans_flags, TRANS_NO_SPELLS_OUT ) )
      {
	send_to_char( "Spell failed.\n\r", victim );
	return;
      }
	  
	 if(is_affected(victim,skill_lookup("hallowed discord"))) {
	pRoomIndex = get_random_room(victim);
    victim->move /= 2;
    victim->was_in_room = victim->in_room;
    act( "$n disappears.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    do_look( victim, "auto" );
    act( "$n appears in the room.", victim, NULL, NULL, TO_ROOM );
    victim->was_in_room = NULL;
    return;
	}

    if (victim->fighting != NULL)
	stop_fighting(victim,TRUE);
    
    ch->move /= 2;
    act("$n disappears.",victim,NULL,NULL,TO_ROOM);
    victim->was_in_room = victim->in_room;
    char_from_room(victim);
    char_to_room(victim,location);
    do_look(victim,"auto");
    act("$n appears in the room.",victim,NULL,NULL,TO_ROOM);
    victim->was_in_room = NULL;
}

/*
 * NPC spells.
 */
void spell_water_bolt( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam;

    act("$n sends a blast of water at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n sends a blast of water at you.",ch,NULL,victim,TO_VICT);
    act("You send a blast of water at $N.",ch,NULL,victim,TO_CHAR);

    if (ch->max_hit > 10000)
     {
       hp_dam = 1200;
     }   
   else   
     {
        
      hp_dam = number_range((ch->max_hit/100)*9,(ch->max_hit/100)*10);
     }

     dam = hp_dam;
    
    if (saves_spell(level,ch,victim,DAM_DROWNING))
    {
	acid_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage_old(ch,victim,dam,sn,DAM_DROWNING,TRUE);
    }
    else
    {
	acid_effect(victim,level,dam,TARGET_CHAR);
	damage_old(ch,victim,dam,sn,DAM_DROWNING,TRUE);
    }
}



void spell_flashfire( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,hp_dam;

  act("$n engulfs $N in a blazing inferno!",ch,NULL,victim,TO_NOTVICT);
  act("$n engulfs you in a blazing inferno!",ch,NULL,victim,TO_VICT);
  act("You send forth a blazing inferno.",ch,NULL,NULL,TO_CHAR);

  if (ch->max_hit > 10000)
    {
      hp_dam = 1200;
    }   
  else   
    {
        
      hp_dam = number_range((ch->max_hit/100)*9,(ch->max_hit/100)*10);
    }

  dam = hp_dam; 
  
  fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);

  if (saves_spell(level,ch,victim,DAM_FIRE))
    {
      fire_effect(victim,level/2,dam/4,TARGET_CHAR);
      damage_old(ch,victim,dam,sn,DAM_FIRE,TRUE);
    }
  else
    {
      fire_effect(victim,level,dam,TARGET_CHAR);
      damage_old(ch,victim,dam,sn,DAM_FIRE,TRUE);
    }
}

void spell_winter_blast( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam;

    act("$n calls down shards of ice from the heavens!",ch,NULL,victim,TO_NOTVICT);
    act("$n calls down ice from the heavens over you!",
	ch,NULL,victim,TO_VICT);
    act("You call forth ice from the heavens.",ch,NULL,NULL,TO_CHAR);

    if (ch->max_hit > 10000)
     {
       hp_dam = 1200;
     }   
   else   
     {
        
      hp_dam = number_range((ch->max_hit/100)*9,(ch->max_hit/100)*10);
     }

     dam = hp_dam;

    cold_effect(victim->in_room,level,dam/2,TARGET_ROOM); 

/*
    if (is_safe_spell(ch,victim,TRUE))
	return;
*/
     if (saves_spell(level,ch,victim,DAM_COLD)) {
	cold_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage_old(ch,victim,dam,sn,DAM_COLD,TRUE);
        }
    else {
	cold_effect(victim,level,dam,TARGET_CHAR);
	damage_old(ch,victim,dam,sn,DAM_COLD,TRUE);
        }
}

void spell_thunderclap(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,hp_dam;

  act("$n sends a clap of thunder at $N.",ch,NULL,victim,TO_NOTVICT);
  act("$n sends a clap of thunder you!",ch,NULL,victim,TO_VICT);
  act("You send a clap of thunder at $N.",ch,NULL,victim,TO_CHAR);

  if (ch->max_hit > 10000)
    {
      hp_dam = 1200;
    }
  else
    {

      hp_dam = number_range((ch->max_hit/100)*10,(ch->max_hit/100)*11);
    }

  dam = hp_dam;

  if (saves_spell(level,ch,victim,DAM_LIGHTNING))
    {
      damage_old(ch,victim,dam,sn,DAM_LIGHTNING,TRUE);
    }
  else
    {
      damage_old(ch,victim,dam,sn,DAM_LIGHTNING,TRUE); 
    }
}


void spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam,hpch;

    act("$n spits acid at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n spits a stream of corrosive acid at you.",ch,NULL,victim,TO_VICT);
    act("You spit acid at $N.",ch,NULL,victim,TO_CHAR);

    hpch = UMAX(12,ch->max_hit);
    hp_dam = number_range(hpch/11 + 1, hpch/4);
    dice_dam = dice(level,16);

    dam = UMAX(hp_dam + dice_dam/9,dice_dam + hp_dam/9);
    
    if (saves_spell(level,ch,victim,DAM_ACID))
    {
	acid_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage_old(ch,victim,dam/12,sn,DAM_ACID,TRUE);
    }
    else
    {
	acid_effect(victim,level,dam,TARGET_CHAR);
	damage_old(ch,victim,dam/6,sn,DAM_ACID,TRUE);
    }
}



void spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam;
    int hpch;

    act("$n breathes forth a blast of fire.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a blast of hot fire over you!",ch,NULL,victim,TO_VICT);
    act("You breath forth a blast of fire.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX( 10, ch->max_hit );
    hp_dam  = number_range( hpch/9+1, hpch/4 );
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam /9, dice_dam + hp_dam / 9);
    fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);
/*
    if (is_safe_spell(ch,victim,TRUE) 
	||  (IS_NPC(victim) && IS_NPC(ch) 
	&&   (ch->fighting != victim || victim->fighting != ch)))
	    return;
*/
   if (saves_spell(level,ch,victim,DAM_FIRE))
    {
	fire_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage_old(ch,victim,dam/12,sn,DAM_FIRE,TRUE);
    }
    else
    {
	fire_effect(victim,level,dam,TARGET_CHAR);
	damage_old(ch,victim,dam/6,sn,DAM_FIRE,TRUE);
    }
}

void spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam, hpch;

    act("$n breathes out a freezing blast of frost!",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a freezing blast of frost over you!",
	ch,NULL,victim,TO_VICT);
    act("You breath out a cone of frost.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX(12,ch->max_hit);
    hp_dam = number_range(hpch/11 + 1, hpch/4);
    dice_dam = dice(level,16);

    dam = UMAX(hp_dam + dice_dam/9,dice_dam + hp_dam/9);
    cold_effect(victim->in_room,level,dam/2,TARGET_ROOM); 

/*
    if (is_safe_spell(ch,victim,TRUE))
	return;
*/
     if (saves_spell(level,ch,victim,DAM_COLD)) {
	cold_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage_old(ch,victim,dam/12,sn,DAM_COLD,TRUE);
        }
    else {
	cold_effect(victim,level,dam,TARGET_CHAR);
	damage_old(ch,victim,dam/6,sn,DAM_COLD,TRUE);
        }
}

void spell_lightning_breath(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam,hpch;

    act("$n breathes a bolt of lightning at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a bolt of lightning at you!",ch,NULL,victim,TO_VICT);
    act("You breathe a bolt of lightning at $N.",ch,NULL,victim,TO_CHAR);

    hpch = UMAX(10,ch->max_hit);
    hp_dam = number_range(hpch/9+1,hpch/4);
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam/9,dice_dam + hp_dam/9);

    if (saves_spell(level,ch,victim,DAM_LIGHTNING))
    {
	/* shock_effect(victim,level/2,dam/24,TARGET_CHAR); */
	damage_old(ch,victim,dam/12,sn,DAM_LIGHTNING,TRUE);
    }
    else
    {
	/* shock_effect(victim,level,dam/6,TARGET_CHAR); */
	damage_old(ch,victim,dam/6,sn,DAM_LIGHTNING,TRUE); 
    }
}


void spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam,hp_dam;
    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
	send_to_char("Not in this room.\n\r",ch);
	return;
    }

    act("$n breathes out a cloud of poisonous gas!",ch,NULL,NULL,TO_ROOM);
    act("You breath out a cloud of poisonous gas.",ch,NULL,NULL,TO_CHAR);

    if (ch->max_hit > 10000)
     {
       hp_dam = 1200;
     }   
   else   
     {
        
      hp_dam = number_range((ch->max_hit/100)*11,(ch->max_hit/100)*12);
     }

     dam = hp_dam;
     poison_effect(ch->in_room,level,dam,TARGET_ROOM);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if ( ch == vch || is_safe_verb( ch, vch, FALSE ) || IS_WIZI_AREA(ch,vch) )
	    continue;

	if (saves_spell(level,ch,vch,DAM_POISON))
	{
	    poison_effect(vch,level/2,dam/4,TARGET_CHAR);
	    damage_old(ch,vch,dam,sn,DAM_POISON,TRUE);
	}
	else
	{
	    poison_effect(vch,level,dam,TARGET_CHAR);
	    damage_old(ch,vch,dam,sn,DAM_POISON,TRUE);
	}
    }
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 25, 100 );
    if ( saves_spell( level,ch, victim, DAM_PIERCE) )
        dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_PIERCE ,TRUE);
    return;
}

void spell_high_explosive(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 30, 120 );
    if ( saves_spell( level,ch, victim, DAM_PIERCE) )
        dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_PIERCE ,TRUE);
    return;
}

void spell_cellular_purge( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
        CHAR_DATA *victim = (CHAR_DATA *) vo;
	int pcnt;
	int dcnt;	
	
	pcnt = 0; dcnt = 0;

        if  ( !is_affected(victim, gsn_poison) && !is_affected(victim, gsn_plague) )
        {
            send_to_char("Your body is in great condition already.\n\r",ch);
            return;
        }

	if (is_affected(victim, gsn_poison))
	{
          if (check_dispel(level,victim,gsn_poison))
          {
            pcnt = 1;
          }
	}
	
	if (is_affected(victim, gsn_plague))
	{
	  if (check_dispel(level,victim,gsn_plague))
	  {
	     dcnt = 1;
	  }
	}

        if (pcnt == 1 && dcnt == 1)
	 {
	   send_to_char("Your mind surges, cleansing your body of impurities!\n\r",ch);
	   act("$n's eyes widen, and sweat pours from $s body.\n\r",victim,NULL,NULL,TO_ROOM);
	 }
	else 
	  if (pcnt == 1 && dcnt == 0)
	  {
	   send_to_char("Your mind surges, cleansing your body of poison!\n\r",ch);
	   act("$n's eyes widen, and sweat pours from $s body.\n\r",victim,NULL,NULL,TO_ROOM);
	  }	
	else
	  if (pcnt == 0 && dcnt == 1)
	  {
	    send_to_char("Your mind surges, cleansing your body of disease!\n\r",ch);
	    act("$n's eyes widen, and sweat pours from $s body.\n\r",victim,NULL,NULL,TO_ROOM);
	  }	
	else
	  if (pcnt == 0 && dcnt == 0)
	  {
	    send_to_char("Spell failed.\n\r",ch);
	  }
}

void spell_mirror( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (( IS_AFFECTED2( victim, AFF_MIRROR ) )|| (IS_AFFECTED3( victim, AFF_SKULLS)))
    {
	send_to_char("You are already protected in this manner.\n\r",ch);
	return;
    }
    af.where	 = TO_AFFECTS;
    af.type      = gsn_mirror;
    af.level	 = level;
    af.whichaff	 = AFF2;
    af.duration  = number_range(1,5) + level/2;
    af.modifier  = number_range(1,3)+(level/10)+1;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_MIRROR;
    affect_to_char2( victim, &af );
    send_to_char( "You shimmer as multiple images of you form.\n\r", victim );
    return;
}

void spell_skulls( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (( IS_AFFECTED2( victim, AFF_MIRROR ) )|| (IS_AFFECTED3( victim, AFF_SKULLS)))
    {
	send_to_char("You are already protected in this manner.\n\r",ch);
	return;
    }
    af.where	 = TO_AFFECTS;
    af.type      = gsn_skulls;
    af.level	 = level;
    af.whichaff	 = AFF3;
    af.duration  = number_range(1,5) + level/2;
    af.modifier  = number_range(1,3)+(level/10)+1;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SKULLS;
    affect_to_char3( victim, &af );
    send_to_char( "Spectral skulls surround you for protection.\n\r", victim );
    return;
}

void spell_astral_projection(int sn, int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_MIRROR ) )
    {
	send_to_char("There are already more than one of you.\n\r",ch);
	return;
    }
    af.where	 = TO_AFFECTS;
    af.type      = gsn_mirror;
    af.level	 = level;
    af.whichaff	 = AFF2;
    af.duration  = number_range(1,5) + level/2;
    af.modifier  = number_range(1,3)+(level/10)+1;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_MIRROR;
    affect_to_char2( victim, &af );
    send_to_char( "You shimmer as multiple images of you form.\n\r", victim );
    return;
}

void do_phase( CHAR_DATA *ch, char * argument)
{
    ROOM_INDEX_DATA *pRoomIndex;

    if (!IS_AFFECTED2(ch,AFF_GHOST)){
       send_to_char("Only ghosts can phase.\n\r",ch);
       return;
       }

    do {
        pRoomIndex = get_random_room(ch);
	if(pRoomIndex->area->linked == FALSE) pRoomIndex = NULL;
    } while(pRoomIndex == NULL);

    act( "$n phases out of the room.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, pRoomIndex );
    act( "$n phases into the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return;
}

void spell_magicward(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
CHAR_DATA *victim = (CHAR_DATA *)vo;
AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_MAGICWARD)){
    	   if (victim == ch)
		send_to_char("You are already warded.\n\r",ch);
	   else
		act("$N is already warded.",ch,NULL,victim,TO_CHAR);
	   return;
	}

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = AFF_MAGICWARD;
    affect_to_char( victim, &af );
    send_to_char( "You feel warded against magic.\n\r", victim );
    if ( ch != victim )
        act("$N is warded against magic.",ch,NULL,victim,TO_CHAR);
    return;

}


void spell_mental_fortress(int sn, int level,CHAR_DATA *ch,void *vo,int target)
{
CHAR_DATA *victim = (CHAR_DATA *)vo;
AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_MAGICWARD)){
    	   if (victim == ch)
	      send_to_char("You have already constructed a mental fortress.\n\r",ch);
	   else
		act("$N is already warded.",ch,NULL,victim,TO_CHAR);
	   return;
	}

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = AFF_MAGICWARD;
    affect_to_char( victim, &af );
    send_to_char( "You construct a mental fortress.\n\r", victim );
    if ( ch != victim )
        act("$N is warded against magic.",ch,NULL,victim,TO_CHAR);
    return;

}

void spell_locate_person( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  if ( ( IS_AFFECTED(ch, AFF_BLIND) || IS_AFFECTED3(ch, AFF_DIRT) )
  ||   (victim = get_char_world(ch,target_name, FALSE)) == NULL 
  ||   ( !IS_NPC(victim) && IS_IMMORTAL(victim) )
  ||   (  IS_NPC(victim) && IS_SET(victim->act_bits,ACT_NOLOCATE )))
  {
    send_to_char("You cannot locate them.\n\r",ch);
    return;
  }

  if( !IS_IMMORTAL( ch ) && IS_AFFECTED3( victim, AFF_VEIL ) )
    {
      send_to_char( "You cannot locate them.\n\r", ch );
      return;
    }

/*sprintf(buf,"%s: %s\n\r",victim->name,victim->in_room->name);*/
  sprintf(buf,"%s: {c%s{x\n\r",
      (IS_NPC(victim) ? victim->short_descr : victim->name ),
       victim->in_room->name);
  buf[0] = UPPER(buf[0]);
  send_to_char(buf,ch);
  if (IS_AFFECTED(victim, AFF_DETECT_MAGIC)){
    sprintf(buf,"You feel watched!\n\r");
    send_to_char(buf,victim);
  }

  return;
}

void do_gcast( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    void *vo;
    int sn;
    int target;

    target_name = one_argument( argument, arg1 );
    if ( arg1[0] == '\0' )
    {
    send_to_char( "Cast which what where?\n\r", ch );
    return;
    }

    if ((sn = find_spell(ch,arg1)) < 1
    ||  skill_table[sn].spell_fun == spell_null)
    {
    send_to_char( "No spells of that name.\n\r", ch );
    return;
    }
 
    /*
     * Locate targets.
     */
    victim  = NULL;
    vo      = NULL;
    target  = TARGET_NONE;

    switch ( skill_table[sn].target )
    {
    default:
    bug( "Do_cast: bad target for sn %d.", sn );
    return;

    case TAR_IGNORE:
    case TAR_CHAR_OFFENSIVE:
    case TAR_OBJ_INV:
       send_to_char("Can't do that spell globally.\n\r",ch);
       return;
    break;

    case TAR_CHAR_SELF:
    case TAR_CHAR_DEFENSIVE:
    case TAR_OBJ_CHAR_DEF:
      target = TARGET_CHAR;
    break;
    }

    /* Cast it on everyone */
    for (d = descriptor_list; d !=NULL; d = d->next){
        if (d->connected != CON_PLAYING)
           continue;

        vo = (void *)((d->original != NULL) ? d->original : d->character);
        (*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
        }


    return;
}

void do_mcast( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *vch, *vch_next;
    void *vo;
    int sn, count = 0;
    int target;

    one_argument( argument, arg );
    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "Cast what spell?\n\r", ch );
        return;
    }
    if ((sn = find_spell(ch,arg)) < 1
    ||  skill_table[sn].spell_fun == spell_null)
    {
        send_to_char( "No spells of that name.\n\r", ch );
        return;
    }

    target  = TARGET_NONE;
    switch ( skill_table[sn].target )
    {
        default:
            bug( "do_mcast: bad target for sn %d.", sn );
        return;
        case TAR_IGNORE:
        case TAR_CHAR_DEFENSIVE:
        case TAR_CHAR_SELF:
        case TAR_OBJ_INV:
        case TAR_OBJ_CHAR_DEF:
           send_to_char("That spell cannot be mass cast.\n\r",ch);
           return;
        break;
        case TAR_CHAR_OFFENSIVE:
        case TAR_OBJ_CHAR_OFF:
            target = TARGET_CHAR;
        break;
    }
    say_spell( ch, sn );
    WAIT_STATE( ch, skill_table[sn].beats );
    for (vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        if ( vch->fighting == ch || ch->fighting == vch )
        {
            count++;
            vo = (void *)vch;
            (*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
        }
    }
    if ( count == 0 )
      send_to_char( "Your spell hit nothing!\n\r", ch );

    return;
}

/* silent, always-land casting */
void do_scast( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int sn;
    int target;
    int chance;

    if (IS_AFFECTED2(ch,AFF_GHOST))
    {
        send_to_char("You're a ghost!\n\r",ch);
        return;
    }

    /* can scast through silence */
    /* switched NPC's can cast spells, but others can't */
    if ( IS_NPC(ch) && ch->desc == NULL)
        return;

    target_name = one_argument( argument, arg1 );
    one_argument( target_name, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Cast which what where?\n\r", ch );
        return;
    }

    if ((sn = find_spell(ch,arg1)) < 1
    ||  skill_table[sn].spell_fun == spell_null
    || (!IS_NPC(ch) && (ch->level < skill_table[sn].skill_level[ch->class]
        && ch->level < skill_table[sn].were_level[ch->race]))
    || get_skill(ch,sn) < 1)
    {
        send_to_char( "You don't know any spells of that name.\n\r", ch );
        return;
    }

    if ( ch->position < skill_table[sn].minimum_position )
    {
        send_to_char( "You can't concentrate enough.\n\r", ch );
        return;
    }
    
    /*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;
    target	= TARGET_NONE;
      
    switch ( skill_table[sn].target )
    {
        default:
            bug( "Do_cast: bad target for sn %d.", sn );
            return;

        case TAR_IGNORE:
            break;

        case TAR_CHAR_OFFENSIVE:
            chance = number_range(1,100);

            if ( arg2[0] == '\0' )
            {
                if ( ( victim = ch->fighting ) == NULL )
                {
                    send_to_char( "Cast the spell on whom?\n\r", ch );
                    return;
                }
            }
            else
            {
                if ( (victim = get_char_room( ch, target_name, TRUE )) == NULL )
                {
                    send_to_char( "They aren't here.\n\r", ch );
                    return;
                }
    
                if ( !can_see( ch, victim ) )
                {
                    send_to_char( "You can't see your target!\n\r", ch );
                    return;
                }
            }

            /* can cast offensive spells in safe rooms */

            if ( IS_AFFECTED2(victim,AFF_GHOST) )
            {
                send_to_char( "You can't do that to a ghost!\n\r", ch );
                return; 
            }

            /* cannot be absorbed or reflected */

            vo = (void *) victim;
            target = TARGET_CHAR;
            break;

        case TAR_CHAR_DEFENSIVE:
            if ( arg2[0] == '\0' )
                victim = ch;
            else
            {
                if ( (victim = get_char_room( ch, target_name, TRUE) ) == NULL )
                {
                    send_to_char( "They aren't here.\n\r", ch );
                    return;
                }
                if ( !can_see( ch, victim ) )
                {
                    send_to_char( "You can't see your target!\n\r", ch );
                    return;
                }
            }

            if ( IS_AFFECTED2(victim,AFF_GHOST) 
            &&   sn != gsn_resurrect )
            {
                send_to_char("You can't do that to a ghost!\n\r",ch);
                return;
            }
    
            vo = (void *) victim;
            target = TARGET_CHAR;
            break;

        case TAR_CHAR_SELF:
            if ( arg2[0] != '\0' 
            &&   !is_name( target_name, ch->name ) )
            {
                send_to_char("You cannot cast this spell on another.\n\r", ch);
                return;
            }
            vo = (void *) ch;
            target = TARGET_CHAR;
            break;

        case TAR_OBJ_INV:
            if ( arg2[0] == '\0' )
            {
                send_to_char( "What should the spell be cast upon?\n\r", ch );
                return;
            }

            if ( ( obj = get_obj_carry( ch, target_name, ch, TRUE ) ) == NULL )
            {
                send_to_char( "You are not carrying that.\n\r", ch );
                return;
            }

            vo = (void *) obj;
            target = TARGET_OBJ;
            break;

        case TAR_OBJ_CHAR_OFF:
            if (arg2[0] == '\0')
            {
                if ((victim = ch->fighting) == NULL)
                {
                    send_to_char("Cast the spell on whom or what?\n\r",ch);
                    return;
                }
                target = TARGET_CHAR;
            }
            else if ((victim = get_char_room(ch,target_name, TRUE)) != NULL)
                target = TARGET_CHAR;
            if (target == TARGET_CHAR) /* check the sanity of the attack */
            {
                if (IS_AFFECTED2(victim,AFF_GHOST))
                {
                    send_to_char("You can't do that to a ghost!\n\r",ch);
                    return;
                }
                vo = (void *) victim;
            }
            else if ((obj = get_obj_here(ch,target_name, TRUE)) != NULL)
            {
                vo = (void *) obj;
                target = TARGET_OBJ;
            }
            else
            {
                send_to_char("You don't see that here.\n\r",ch);
                return;
            }
            break; 
        case TAR_OBJ_CHAR_DEF:
            if (arg2[0] == '\0')
            {
                vo = (void *) ch;
                target = TARGET_CHAR;
            }
            else if ((victim = get_char_room(ch,target_name, TRUE)) != NULL)
            {
                if ( IS_AFFECTED2(victim,AFF_GHOST) )
                {
                    send_to_char("You can't do that to a ghost!\n\r",ch);
                    return;
                }
                if ( !can_see( ch, victim ) )
                {
                    send_to_char( "You can't see your victim!\n\r", ch );
                    return;
                }
                vo = (void *) victim;
                target = TARGET_CHAR;
            }
            else if ((obj = get_obj_carry(ch,target_name,ch, TRUE)) != NULL)
            {
                vo = (void *) obj;
                target = TARGET_OBJ;
            }
            else
            {
                send_to_char("You don't see that here.\n\r",ch);
                return;
            }
            break;
    }
    
    WAIT_STATE( ch, skill_table[sn].beats );
      
    if ( number_percent( ) > get_skill(ch,sn) )
    {
        switch (number_range(0, 4)) 
        {
            case 0:
            default:
                send_to_char( "You mispronounce an arcane syllable.\n\r", ch );
                break;
            case 1:
                send_to_char( 
          "Your attempts at spellcasting fail.  You should go practice it.\n\r",
              ch);
                break;
        }
	
        check_improve(ch,sn,FALSE,1);
    }
    else
    {
        int casting_level = ch->level;
        if ( IS_AFFECTED2(ch,AFF_FOCUS) )
            casting_level = ch->level + 5;
        if ( ch != (CHAR_DATA *) vo
        &&   !IS_NPC(ch)
        &&   !class_table[ch->class].fMana
        &&   skill_table[sn].target != TAR_IGNORE )
             casting_level = casting_level -2;                

        (*skill_table[sn].spell_fun) ( sn, casting_level, ch,vo,target);
        check_improve(ch,sn,TRUE,1);
    }

    return;
}

void spell_white_grimoire( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

/*  The white grimoire only burns vampires */
    if ( IS_NPC( victim ) 
    || (!IS_SET(victim->shiftbits,TEMP_VAMP) && !IS_SET(victim->shiftbits,PERM_VAMP) ) )
        return;

    if( is_affected( victim, gsn_white_grimoire ) )
    {
       if (victim != ch)
         act("$N is already affected by the white grimoire's power.",ch, NULL,victim,TO_CHAR);
       return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 0;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char(victim,&af);

    send_to_char("Smoke begins to rise from your fingertips.\n\r",victim);
    act("Smoke begins to rise from $n's fingertips as $e grasps the book.",
        victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_dark_grimoire( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_NPC( victim ) 
    ||   IS_SET( victim->shiftbits,TEMP_VAMP ) 
    ||   IS_SET( victim->shiftbits,PERM_VAMP ) )
        return;

    if( is_affected( victim, gsn_dark_grimoire ) )
    {
       if (victim != ch)
         act("$N is already affected by the dark grimoire's power.",ch, NULL,victim,TO_CHAR);
       return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 0;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char(victim,&af);

    send_to_char("Smoke begins to rise from your fingertips.\n\r",victim);
    act("Smoke begins to rise from $n's fingertips as $e grasps the book.",
        victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_forget( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int chint, clvl;

    if ( IS_AFFECTED2( victim, AFF2_FORGET ) )
    {
        act( "$E already seems to be very forgetful.", ch, NULL, victim, TO_CHAR );
        return;
    }

/*  Having higher INT than your enemy means big bonus to landing this spell */
    chint = get_curr_stat(ch,STAT_INT);
    clvl = level + chint - get_curr_stat(victim,STAT_INT);

    if ( saves_spell( clvl,ch, victim,DAM_MENTAL) )
    {
        if ( ch != victim )
            send_to_char("You failed.\n\r", ch);
        send_to_char("You feel a tickle in the back of your mind.\n\r",victim);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.whichaff  = AFF2;
    af.level     = level;
    af.duration  = UMAX(chint/4,1);
    af.location  = APPLY_WIS;
    af.modifier  = UMIN(-clvl/10 + 1,-1);
    af.bitvector = AFF2_FORGET;
    affect_to_char2( victim, &af );
    af.location  = APPLY_SKILLS;
    af.modifier  = -10;
    affect_to_char2( victim, &af );

    if ( ch != victim )
        act( "You cause $N to become forgetful!",ch,NULL,victim,TO_CHAR);
    send_to_char( "Your mind suddenly goes blank!\n\r", victim );

    return;
}

void spell_immolation(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2(victim,AFF2_BURNING) )
        return;
  
    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level	 = level;
    af.duration	 = level / 10;
    af.modifier  = level / 5;
    af.bitvector = AFF2_BURNING;

    af.location  = APPLY_HITROLL;
    affect_to_char2(victim,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_char2(victim,&af);

    send_to_char("A burning fury consumes you as flames burst from your flesh!\n\r",victim);
    act( "An intense conflagration enshrouds $n as the air around $m bursts into flame!",victim,NULL,NULL,TO_ROOM);
}

void spell_sanguination(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your veins are already pounding with extra blood!\n\r",
              ch);
	else
	  act("$S veins are already pounding with extra blood.",
              ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level	 = level;
    af.duration	 = level / 10;
    af.modifier  = 50;
    af.bitvector = 0;
    af.location  = APPLY_BLOOD;
    affect_to_char2(victim,&af);

    if ( IS_NPC( victim ) 
    ||   IS_SET( victim->shiftbits,TEMP_VAMP ) 
    ||   IS_SET( victim->shiftbits,PERM_VAMP ) )
        send_to_char( "You feel your blood magic grow stronger!\n\r", victim );
    else
        send_to_char( "You find yourself uncomfortably aware of the blood pulsing through your veins.\n\r", victim );
    return;
}

void spell_infusion( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    victim = (CHAR_DATA *) vo;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your body cannot stand another infusion.\n\r",ch);
	else
	  act("$S body cannot stand another infusion.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level / 6;
    af.modifier  = level;
    af.bitvector = 0;

    af.location  = APPLY_HIT;
    affect_to_char( victim, &af );
    af.location  = APPLY_MANA;
    affect_to_char( victim, &af );
    af.location  = APPLY_MOVE;
    affect_to_char( victim, &af );

    act("You are infused with fresh life energy!",ch,NULL,victim,TO_VICT);
    act("$N is infused with fresh life energy!",ch,NULL,victim,TO_NOTVICT);
    act("You infuse $N with fresh life energy!",ch,NULL,victim,TO_CHAR);

    return;
}

void spell_dark_risings(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FLYING))
    {
       if (victim == ch)
         send_to_char("You have already risen off of the ground.\n\r",ch);
       else
         act("$N doesn't need your help to rise darkly.",ch,NULL,victim,TO_CHAR);
       return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char("The powers of darkness raise you into the air.\n\r", victim );
    act("Darkness surrounds $n and lifts $m into the air.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_gypsy_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if(is_safe_spell(ch,victim,FALSE) && victim != ch)
    {
      send_to_char("Not on that target.\n\r",ch);
      return;
    }

    if (IS_AFFECTED(victim,AFF_CURSE) )
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * (level / 8);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

    send_to_char( "A Gypsy curse overwhelms you!\n\r", victim );
    if ( ch != victim )
	act("A Gypsy curse overwhelms $N!",ch,NULL,victim,TO_CHAR);
    return;
}

/*
 * festive spirit - give wacky random effects.  used in a christmas present
 *   questeq back in 2008
 */
#define MAX_FESTIVITY 12
void spell_festive_spirit(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int iterations, i, apploc, appmag;
    const int rnd_apply[MAX_FESTIVITY] = 
                        { APPLY_STR, APPLY_INT, APPLY_WIS, APPLY_DEX, APPLY_CON,
                          APPLY_MANA, APPLY_HIT, APPLY_MOVE, 
                          APPLY_HITROLL, APPLY_DAMROLL,
                          APPLY_AC, APPLY_SAVES };
    const int max_apply[MAX_FESTIVITY] = 
                        { 4, 4, 4, 4, 4,
                         50, 50, 50,
                          4, 4,
                         10, 4 };
    bool lapplied[MAX_FESTIVITY] = { 0,0,0,0,0,0,0,0,0,0,0,0 };

    if ( victim == NULL ) victim = ch;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already full of festive spirit!\n\r",ch);
	else
	  act("$E is already full of festive spirit.",ch,NULL,victim,TO_CHAR);
	return;
    }

    iterations = number_range( 1, (level - 50)/2 ) + 1;
    for ( i = 1; i <= iterations; i++ )
    {
        apploc = number_range( 0, MAX_FESTIVITY-1 );
        lapplied[apploc] = TRUE;
        do 
            appmag = number_range( -1*max_apply[apploc], max_apply[apploc] );
        while ( appmag == 0 );
       
        af.where     = TO_AFFECTS;
        af.type      = sn;
        af.level     = level;
        af.duration  = level / 5;
        af.location  = rnd_apply[apploc];
        af.modifier  = appmag;
        af.bitvector = 0;
        affect_to_char( victim, &af );
    }
    send_to_char( "Festive spirit and merriment fills you!\n\r", victim );
    act("$n becomes visibly festive and merry.",victim,NULL,NULL,TO_ROOM);
    return;
}

/*
 * green thumb - tells caster what the room's sector is.  good for getting
 *   players to help find rooms with unset sectors
 */
void spell_green_thumb(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{
    char buf[MAX_STRING_LENGTH];
    if ( ch->in_room )
    {
        sprintf(buf, "Plants that can grow %s will do well here.\n\r", 
        flag_string( ic_sector_flags, ch->in_room->sector_type ) );
        send_to_char( buf, ch );
    }
    return;
}

/*
 * fortune - gives the caster some amount of gold.  useful for quest
 *   prizes, potions, etc.
 */
void spell_fortune( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    char buf[MAX_STRING_LENGTH];
    int chance, amount, i;


    for ( i = 0, amount = 0; i < level; i++ )
    {
        chance = number_percent();
        amount += chance;
    }
    amount /= 2;

    if ( chance < 25 )
    {
        sprintf(buf, "You find %d gold behind your ear!\n\r", amount );
        send_to_char( buf, ch );
        act( "$n discovers some gold behind $s ear!", ch, NULL, NULL, TO_ROOM );
    }
    else if ( chance < 50 )
    {
        OBJ_DATA *obj;
        bool found = FALSE;
        if ( (obj = get_eq_char( ch, WEAR_FEET )) != NULL )
        {
            if ( !str_infix( "boot", obj->name ) )
            {
                sprintf( buf, "You find %d gold in your boot!\n\r", amount );
                send_to_char( buf, ch );
                act( "$n discovers some gold in $s boot!", 
                    ch, NULL, NULL, TO_ROOM );
                found = TRUE;
            }
            else if ( !str_infix( "shoe", obj->name ) )
            {
                sprintf( buf, "You find %d gold in your shoe!\n\r", amount );
                send_to_char( buf, ch );
                act( "$n discovers some gold in $s shoe!", 
                    ch, NULL, NULL, TO_ROOM );
                found = TRUE;
            }
        }
        if ( !found )
        {
            sprintf( buf, "You find %d gold under your foot!\n\r", amount );
            send_to_char( buf, ch );
            act( "$n discovers some gold under $s foot!", 
                ch, NULL, NULL, TO_ROOM );
        }
    }
    else if ( chance < 75 )
    {
        sprintf( buf, 
            "You find %d gold in your pocket that you didn't know you had!\n\r",
            amount );
        send_to_char( buf, ch );
        act( "$n finds something jingly in $s pocket!", 
            ch, NULL, NULL, TO_ROOM );
    }
    else
    {
        sprintf( buf, "You spot %d gold on the ground and grab it up!\n\r", 
            amount );
        send_to_char( buf, ch );
        act( "$n spots some gold on the ground and grabs it up!", 
            ch, NULL, NULL, TO_ROOM );
    }

    ch->gold += amount;

    return;
}

/*
 * Pestilence--virulent damage-over-time spell that does damage on par with
 *   blister, but spreads faster than plague.  For the book of plagues.
 *   Only saves check is on spread in update.c
 */
void spell_pestilence( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    /* negative level simply means don't give the immunity message (used when
       pestilence passively spreads */
    if ( IS_SET(victim->imm_flags, IMM_DISEASE) )
    {
        if ( level > 0 )
            act( "$E is immune to the effects of pestilence.", 
                ch, NULL, victim, TO_CHAR);
        return;
    }

    if ( level < 0 ) level *= -1;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/5;
    af.location  = APPLY_STR;
    af.modifier  = -5; 
    af.bitvector = AFF_PLAGUE;
    affect_join( victim, &af, FALSE );

    send_to_char( "Painful pustules develop under your skin.\n\r", victim );
    act("Large pustules develop all over $n's body.", 
        victim, NULL, NULL, TO_ROOM );
/*  
    send_to_char( "Vile pustules rupture and ooze from your skin.\n\r", 
        victim );
    act("Vile pustules rupture and ooze from $n's skin.", 
        victim, NULL, NULL, TO_ROOM);
 */
    return;
}

/* 
 * somnambulance - effect spell that makes the target fall asleep randomly.
 *   for the Book of Dreams.  NO saves check.
 */
void spell_somnambulance(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_SET(victim->imm_flags, IMM_CHARM) )
        return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/5;
    af.location  = APPLY_NONE;
    af.modifier  = 0; 
    af.bitvector = AFF3_NARCOLEPSY;
    affect_to_char3( victim, &af );

    send_to_char( 
   "A sense of calm passes over you as your senses dull and muscles relax.\n\r",
        victim );
    act("$n blinks very slowly, as if fighting off sleep.", 
        victim, NULL, NULL, TO_ROOM );
    return;
}

/*
 * wyrm venom - for Dararn's vis quest (Murderous Wyrm pirate mutiny)
 */
void spell_wyrm_venom( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, gsn_wyrm_venom ) )
    {
	act("$E is already affected by wyrm venom.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( saves_spell(level, ch, victim, DAM_POISON) 
    &&   !global_config_strongwyrmpoison )
    {
	act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
	return;
    }

    if ( IS_AFFECTED3(victim,AFF3_SOLEMN)
    &&   number_percent() > CONFIG_SOLEMN_VOWS_CHANCE 
    &&   !global_config_strongwyrmpoison )
    {
        send_to_char("The power of your vows shield you from taint.\n\r", 
            victim);
        if ( ch != victim )
            act("$N's vows protect $M from the venom.", 
                ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = gsn_wyrm_venom;
    af.level     = level;
    af.duration  = level/8;
    af.location  = APPLY_DEX;
    af.modifier  = -4;
    af.bitvector = AFF2_NOREGEN;
    affect_to_char2( victim, &af );
    af.location  = APPLY_STR;
    af.modifier  = -4;
    affect_to_char2( victim, &af );
    send_to_char( "You feel extremely sick.\n\r", victim );
    act("$n looks extremely ill.",victim,NULL,NULL,TO_ROOM);
    return;
}

/*
 * pestilential force - book of pestilence spell
 */
void spell_natures_pestil(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *book;
    AFFECT_DATA af;

    if ( ( book = get_eq_char( ch, WEAR_HOLD ) ) == NULL 
    ||    (!IS_IMMORTAL(ch) && !is_name( "-PowerPlague- -imp-" , book->name)) )
    {
        send_to_char( "This spell is powerless unless you hold the Book.\n\r",
          ch );
        return;
    }
    else if ( victim == ch )
    {
        send_to_char("You must focus this power on someone else.\n\r", ch);
        return;
    }
    else if ( is_affected( ch, sn ) )
    {
        send_to_char( "You cannot invoke this power so quickly!\n\r", ch );
        return;
    }
    else if ( is_affected( victim, sn ) )
    {
        act( "$N is already crippled by nature's pestilence.", ch, NULL, 
            victim, TO_CHAR );
        return;
    }
    else if ( IS_NPC( victim ) )
    {
        send_to_char( "That would be a waste of the Book's power.\n\r", ch );
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 80;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char( ch, &af );

    af.location  = APPLY_MOVE;
    af.modifier  = -1000;
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );
    victim->move = 0;

    act( "$n's pestilential wrath of nature paralyzes you!", 
        ch, NULL, victim, TO_VICT );
    act( "$n's pestilential wrath of nature paralyzes $N!",
        ch, NULL, victim, TO_NOTVICT );
    act( "Your pestilential wrath of nature paralyzes $N!",
        ch, NULL, victim, TO_CHAR );

    if ( !IS_NPC( ch ) && !IS_IMMORTAL( ch ) )
        ch->pcdata->learned[sn] = 0;

    return;
}

/*
 * abjuration - new take on the "white grimoire" spell
 */
void spell_abjuration(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( !IS_VAMPIRE(victim) )
        return;

    if ( IS_AFFECTED3( victim, AFF3_ABJURE ) )
    {
       if (victim != ch) act( "$N is already affected by abjuration.", 
               ch, NULL, victim, TO_CHAR);
       return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 3*level/4;
    af.location  = APPLY_NONE;
    af.modifier  = 0; 
    af.bitvector = AFF3_ABJURE;
    affect_to_char3( victim, &af );

    send_to_char("Strange runes appear on your skin and begin to smoke!\n\r",
        victim);
    act( "Smoke rises from $n, and strange runes seem to writhe on the surface of $s skin!", victim, NULL, NULL, TO_ROOM);
    return;
}

/*
 * famine - another book of power spell
 */
void spell_famine( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/5;
    af.location  = APPLY_CON;
    af.modifier  = -5; 
    af.bitvector = AFF3_FAMINE;
    affect_to_char3( victim, &af );

    send_to_char( "You feel your body become gaunt and skeletal.\n\r", victim );
    act("$n grows visibly gaunt and skeletal.", victim, NULL, NULL, TO_ROOM );

    return;
}

void do_hallowed_gate(CHAR_DATA *ch) {

	OBJ_DATA *portal;
	ROOM_INDEX_DATA *to_room;
	ROOM_INDEX_DATA *from_room = ch->in_room;
	int rand;

	/*if (get_skill(ch, skill_lookup("gate")) < 1) 
	{
		do_hallowed_portal(ch);
	}*/
	
	rand = number_range(1,6);
	if (rand == 1) {to_room = get_room_index(3001);}
	else if (rand == 2) {to_room = get_room_index(933);}
	else if (rand == 3) {to_room = get_room_index(9604);}
	else if (rand == 4) {to_room = get_room_index(5487);}
	else if (rand == 5) {to_room = get_room_index(3880);}
	else if (rand == 6) {to_room = get_room_index(3850);}
	else {to_room = get_room_index(3001);}

	portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 1 + ch->level/10;
    portal->value[3] = from_room->vnum;

    obj_to_room(portal,to_room);
      
    if (to_room->people != NULL)
	{
		act("$p rises up from the ground.",to_room->people,portal,NULL,TO_ROOM);
		act("$p rises up from the ground.",to_room->people,portal,NULL,TO_CHAR);
	}
	
	send_to_char( "You feel your body become gaunt and skeletal.\n\r", ch );
}

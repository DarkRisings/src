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
#include <string.h>
#include <time.h>
#include "merc.h"
#include "music.h"
#include "interp.h"
#include "recycle.h"
#include "magic.h"

extern MPDELAY_DATA      *new_mpdelay  args( (void) );
extern void               free_mpdelay args( (MPDELAY_DATA *mpdelay) );
extern  int     max_on;
extern int quad;

/*
 * Local functions.
 */
int     hit_gain        args( ( CHAR_DATA *ch ) );
int     mana_gain       args( ( CHAR_DATA *ch ) );
int     move_gain       args( ( CHAR_DATA *ch ) );
void    mobile_update   args( ( void ) );
void    weather_update  args( ( void ) );
void    char_update     args( ( void ) );
void    time_update     args( ( void ) );
void    obj_update      args( ( void ) );
void    aggr_update     args( ( void ) );
void    mpdelay_update  args( ( void ) );
void    stats_update    args( ( void ) );
void    check_pinch     args( ( CHAR_DATA *ch ) );
void    damage_on_tick  args( ( CHAR_DATA *ch, int dot_bitvec[] ) );

bool check_social args((CHAR_DATA * ch, char *command, char *argument));

/* used for saving */
int     save_number = 0;

/* number of ticks for autosave */
const static int SAVE_INTERVAL = 7;

void quad_update()
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d, *d_next;


		if (quad > 0) {
			quad -= 1;
		} else if(quad == 0) {
			for (d = descriptor_list; d != NULL; d = d_next) {
				d_next = d->next;
				if (d->connected == CON_PLAYING) {
					sprintf(buf, "{YYour experience gain normalizes.{x\n\r");
					send_to_char(buf, d->character);
				}
			}
			quad = -1;
		}
	
	return;
}

/*
 * Advancement stuff.
 */
void advance_level( CHAR_DATA *ch, bool hide )
{
    char buf[MAX_STRING_LENGTH];
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;

    ch->pcdata->last_level = 
	( ch->played + (int) (current_time - ch->logon) ) / 3600;

    if (IS_SET(ch->act_bits,ACT_NOAPPROVE) && (ch->level == 20)) {
	send_to_char("You can advance no more until you gain approval.\n\r",ch);
	send_to_char("Please write a description and see your friendly neighborhood immortal.\n\r",ch);
	}

    add_hp	= con_app[get_curr_stat(ch,STAT_CON)].hitp + number_range(
		    class_table[ch->class].hp_min,
		    class_table[ch->class].hp_max );
    add_mana 	= number_range(10,(2*get_curr_stat(ch,STAT_INT)
				  + get_curr_stat(ch,STAT_WIS))/5);
    if (!class_table[ch->class].fMana)
	add_mana /= 2;
    add_move	= number_range( 1, (get_curr_stat(ch,STAT_CON)
				  + get_curr_stat(ch,STAT_DEX))/6 );
    add_prac	= wis_app[get_curr_stat(ch,STAT_WIS)].practice;

    add_hp = add_hp * 9/10;
    add_mana = add_mana * 9/10;
    add_move = add_move * 9/10;

    add_hp	= UMAX(  2, add_hp   );
    add_mana	= UMAX(  2, add_mana );
    add_move	= UMAX(  6, add_move );

    ch->max_hit 	+= add_hp;
    ch->max_mana	+= add_mana;
    ch->max_move	+= add_move;
    ch->practice	+= add_prac;
    ch->train		+= 1;

    ch->pcdata->perm_hit	+= add_hp;
    ch->pcdata->perm_mana	+= add_mana;
    ch->pcdata->perm_move	+= add_move;

    

   if (!hide)
    {
    	sprintf(buf,
	    "You gain %d hit point%s, %d mana, %d move, and %d practice%s.\n\r",
	    add_hp, add_hp == 1 ? "" : "s", add_mana, add_move,
	    add_prac, add_prac == 1 ? "" : "s");
	send_to_char( buf, ch );
    }

   if ((ch->level == 30) && IS_SET(ch->shiftbits, SHIFT_POTENTIAL)) {
      send_to_char("\n\rYou feel strangely different, as if an animal inside of you is trying to emerge.\n\r\n\r",ch);
      }
    return;
}   



void gain_exp( CHAR_DATA *ch, int gain )
{
  char buf[MAX_STRING_LENGTH];
  int xppl;

  if ( IS_NPC(ch) || ch->level >= LEVEL_HERO )
    return;

  if ( gain > 0 && IS_SET( ch->act_bits, PLR_SUSPEND_LEVEL ) )
    return;

  if (IS_SET(ch->act_bits,ACT_NOAPPROVE) && (ch->level == 20)) {
    send_to_char("You can advance no more until you gain approval.\n\r",ch);
    send_to_char("Please write a description and see your friendly neighborhood immortal.\n\r",ch);
    return;
  }

  xppl = exp_per_level( ch, ch->pcdata->points );

  ch->exp += gain;

  if( ch->exp < xppl * ch->level )
    ch->exp = xppl * ch->level;

  if ( gain > 300 )
    ch->pcdata->skillbonus = current_time;

  while ( ch->level < LEVEL_HERO && ch->exp >= 
	  xppl * (ch->level+1) )
    {
      send_to_char( "You raise a level!!  ", ch );
      ch->level += 1;
      sprintf(buf,"%s gained level %d",ch->name,ch->level);
      log_string(buf);
      sprintf(buf,"$N has attained level %d!",ch->level);
      wiznet(buf,ch,NULL,WIZ_LEVELS,0,0);
      advance_level(ch,FALSE);
      save_char_obj(ch);
    }

  return;
}



/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    int gain;
    int number;
    bool fheal, fbarb;

    if ( ch->in_room == NULL
    ||   IS_AFFECTED2(ch, AFF2_BURNING) 
    ||   IS_AFFECTED2(ch, AFF_BBOIL)
    ||   IS_AFFECTED2(ch, AFF2_NOREGEN) )
	return 0;

    fheal = FALSE;
    fbarb = FALSE;
    if ( IS_NPC(ch) )
    {
	gain =  5 + ch->level;
 	if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 2;

	switch(ch->position)
	{
	    default : 		gain /= 2;			break;
	    case POS_SLEEPING: 	gain = 3 * gain/2;		break;
	    case POS_RESTING:  					break;
	    case POS_FIGHTING:	gain /= 3;		 	break;
 	}
    }
    else
    {
	gain = UMAX(3,get_curr_stat(ch,STAT_CON) - 3 + ch->level); 
	gain += class_table[ch->class].hp_max - 10;
 	number = number_percent();

        if ( ch->class == class_lookup( "barbarian" ) )
            fbarb = TRUE;
	if (number < get_skill(ch,gsn_fast_healing))
	{
            fheal = TRUE;
	    gain += number * gain / 100;
	    if (ch->hit < ch->max_hit)
		check_improve(ch,gsn_fast_healing,TRUE,8);
	}
	switch ( ch->position )
	{
	    default:	   	gain /= 4;			break;
	    case POS_SLEEPING: 					break;
	    case POS_RESTING:  	         			break;
	    case POS_FIGHTING: 	gain /= 6;			break;
	}
    }

    gain = gain * ch->in_room->heal_rate / 100;
    
    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[3] / 100;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if ( ( !fbarb 
    &&      IS_AFFECTED(ch,AFF_HASTE) 
    &&     !IS_AFFECTED(ch, AFF_NIGHTHASTE) )
    ||   IS_AFFECTED(ch,AFF_SLOW) )
	gain /=2;

    if ( fbarb && fheal )
        gain *= 2;

    if ( IS_AFFECTED3( ch, AFF3_FAMINE ) )
        gain *= -1;

    return UMIN(gain, ch->max_hit - ch->hit);
}



int mana_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_AFFECTED2( ch, AFF2_NOREGEN ) && is_affected( ch, gsn_wyrm_venom ) )
        return 0;

    if ( IS_NPC(ch) )
    {
	gain = 5 + ch->level;
	switch (ch->position)
	{
	    default:		gain /= 2;		break;
	    case POS_SLEEPING:	gain = 3 * gain/2;	break;
   	    case POS_RESTING:				break;
	    case POS_FIGHTING:	gain /= 3;		break;
    	}
    }
    else
    {
	gain = (get_curr_stat(ch,STAT_WIS) 
	      + get_curr_stat(ch,STAT_INT) + ch->level);
	number = number_percent();
	if (number < get_skill(ch,gsn_meditation))
	{
	    gain += number * gain / 100;
	    if (ch->mana < ch->max_mana)
	        check_improve(ch,gsn_meditation,TRUE,8);
	}
	if (!class_table[ch->class].fMana)
	    gain /= 2;

	switch ( ch->position )
	{
	    default:		gain /= 4;			break;
	    case POS_SLEEPING: 					break;
	    case POS_RESTING:			 	        break;
	    case POS_FIGHTING:	gain /= 6;			break;
	}


    }

    gain = gain * ch->in_room->mana_rate / 100;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[4] / 100;

    if ( IS_AFFECTED( ch, AFF_POISON ) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED2(ch, AFF_BBOIL))
        gain = 0;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
        gain /=2 ;
		
		
	if (IS_AFFECTED(ch,AFF_SLOW))
        gain /=2 ;
	
    return UMIN(gain, ch->max_mana - ch->mana);
}



int move_gain( CHAR_DATA *ch )
{
    int gain;

    if (ch->in_room == NULL)
	return 0;
    if ( IS_AFFECTED2( ch, AFF2_NOREGEN ) && is_affected( ch, gsn_wyrm_venom ) )
        return 0;

    if ( IS_NPC(ch) )
    {
	gain = ch->level;
    }
    else
    {
	gain = UMAX( 15, ch->level );

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_stat(ch,STAT_DEX);		break;
	case POS_RESTING:  gain += get_curr_stat(ch,STAT_DEX) / 2;	break;
	}

    }

    gain = gain * ch->in_room->heal_rate/100;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[3] / 100;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED2(ch, AFF_BBOIL))
        gain = 0;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
        gain /=2 ;

    return UMIN(gain, ch->max_move - ch->move);
}



void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;

    if ( value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
	return;

    if (IS_AFFECTED2(ch,AFF_GHOST))
        return;

    condition				= ch->pcdata->condition[iCond];
    if (condition == -1)
        return;
    ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 48 );

    if ( ch->pcdata->condition[iCond] == 0 )
    {
	switch ( iCond )
	{
	case COND_DRUNK:
	    if ( condition != 0 )
		send_to_char( "You are sober.\n\r", ch );
	    break;
	}
    }

    return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;

    /* Examine all mobs. */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if ( !IS_NPC(ch) 
	||   ch->in_room == NULL 
	||   (ch->in_room->area->empty 
	      && !IS_SET(ch->act_bits,ACT_UPDATE_ALWAYS)) )
	    continue;

	/*
	 * Check triggers only if mobile still in default position
	 */
	if ( ch->pIndexData->mprog_flags
	&&   ch->position == ch->pIndexData->default_pos )
	{
	    /* Delay */
	    if ( HAS_TRIGGER( ch, TRIG_DELAY) 
	    &&   ch->mprog_delay > 0 )
	    {
		if ( --ch->mprog_delay <= 0 )
		{
		    mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_DELAY );
		    continue;
		}
	    } 
	    if ( HAS_TRIGGER( ch, TRIG_RANDOM) )
	    {
		if( mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_RANDOM ) )
		continue;
	    }
            if ( HAS_TRIGGER( ch, TRIG_PPM ) )
            {
                if ( mp_ppm_trigger( ch, NULL, NULL, NULL, TRIG_PPM ) )
                continue;
            }
	}

	if ( IS_AFFECTED(ch,AFF_CHARM) )
	    continue;

	/* Examine call for special procedure */
	if ( ch->spec_fun != 0 )
	{
	    if ( (*ch->spec_fun) ( ch ) )
		continue;
	}

	if (ch->pIndexData->pShop != NULL) /* give him some gold */
	    if ((ch->gold * 100 + ch->silver) < ch->pIndexData->wealth)
	    {
		ch->gold += ch->pIndexData->wealth * number_range(1,20)/5000000;
		ch->silver += ch->pIndexData->wealth * number_range(1,20)/50000;
	    }
	 


	/* That's all for sleeping / busy monster, and empty zones */
	if ( ch->position != POS_STANDING )
	    continue;

	/* Scavenge */
	if ( IS_SET(ch->act_bits, ACT_SCAVENGER)
	&&   ch->in_room->contents != NULL
	&&   number_bits( 6 ) == 0 )
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int max;

	    max         = 1;
	    obj_best    = 0;
	    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	    {
		if ( CAN_WEAR(obj, ITEM_TAKE)
		     && obj->cost > max && obj->cost > 0)
		{
                    RESET_DATA *pReset;
                    /* ensure scavengers don't keep picking up items reset onto
                       ground in their rooms */
                    for ( pReset = ch->in_room->reset_first; 
                          pReset; 
                          pReset = pReset->next )
                        if ( pReset->command == 'O' 
                        &&   pReset->arg1 == obj->pIndexData->vnum )
                            break;
                    if ( !pReset )
                    {
		        obj_best = obj;
		        max = obj->cost;
                    }
		}
	    }

	    if ( obj_best )
	    {
		obj_from_room( obj_best );
		obj_to_char( obj_best, ch );
		act( "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
	    }
	}

	/* Wander */
	if ( !IS_SET(ch->act_bits, ACT_SENTINEL) 
	&& number_bits(3) == 0
	&& ( door = number_bits( 5 ) ) <= 5
	&& ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
	&& ( !IS_SET(ch->act_bits, ACT_STAY_AREA)
	||   pexit->u1.to_room->area == ch->in_room->area ) 
	&& ( !IS_SET(ch->act_bits, ACT_OUTDOORS)
	||   !IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)) 
	&& ( !IS_SET(ch->act_bits, ACT_INDOORS)
	||   IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)))
	{
	    move_char( ch, door, FALSE );
	}
    }

    return;
}


/*
 * Update bank accounts and Otho's interest rate
 */
void bank_update( void )
{
    CHAR_DATA *ch;
    BANKACCT_DATA *acct, *acct_next;

    bank_interest = number_range( 0, 5 );

/*  go through every private account and see if the owner is online.  if so,
    credit the money to the person's account at Otho's */
    for ( acct = global_bankacct_list; acct != NULL; acct = acct_next )
    {
        acct_next = acct->next;
        if ( acct->type == ACCT_PERSONAL )
            for ( ch = char_list; ch != NULL ; ch = ch->next )
                if ( !IS_NPC(ch) && is_name( acct->owner, ch->name ) )
                {
                    acct_to_char( acct, ch );
                    break;
                }
    }

    if ( global_bankacct_changed )
    {
        save_bankaccts();
        global_bankacct_changed = FALSE;
    }
    return;
}

/*
 * Update the weather.
 */
void weather_update( void )
{
    char buf[MAX_STRING_LENGTH],
         buf2[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int diff;
    CHAR_DATA *sch;

    buf[0] = '\0';
    buf2[0] = '\0';

    switch ( ++time_info.hour )
    {
    case  5:
	weather_info.sunlight = SUN_LIGHT;
	strcat( buf2, "The day has begun.\n\r" );
	break;

    case  6:
	weather_info.sunlight = SUN_RISE;
	strcat( buf2, "The sun rises in the east.\n\r" );
	break;

    case 19:
	weather_info.sunlight = SUN_SET;
	strcat( buf2, "The sun slowly disappears in the west.\n\r" );
	break;

    case 20:
	weather_info.sunlight = SUN_DARK;
	strcat( buf2, "The night has begun.\n\r" );
	break;

    case 24:
	time_info.hour = 0;
	time_info.day++;
	break;
    }

    if ( time_info.day   >= 35 )
    {
	time_info.day = 0;
	time_info.month++;
    }

    if ( time_info.month >= 17 )
    {
	time_info.month = 0;
	time_info.year++;
    }

    /*
     * Weather change.
     */
    if ( time_info.month >= 9 && time_info.month <= 16 )
	diff = weather_info.mmhg >  985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change    = UMAX(weather_info.change, -12);
    weather_info.change    = UMIN(weather_info.change,  12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg += weather_info.cwmod;
    weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
    weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);
    weather_info.cwmod = 0;

    switch ( weather_info.sky )
    {
    default: 
	bug( "Weather_update: bad sky %d.", weather_info.sky );
	weather_info.sky = SKY_CLOUDLESS;
	break;

    case SKY_CLOUDLESS:
	if ( weather_info.mmhg <  990
	|| ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The sky is getting cloudy.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_CLOUDY:
	if ( weather_info.mmhg <  970
	|| ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "It starts to rain.\n\r" );
	    weather_info.sky = SKY_RAINING;
	}

	if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "The clouds disappear.\n\r" );
	    weather_info.sky = SKY_CLOUDLESS;
	}
	break;

    case SKY_RAINING:
	if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "Lightning flashes in the sky.\n\r" );
	    weather_info.sky = SKY_LIGHTNING;
	}

	if ( weather_info.mmhg > 1030
	|| ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The rain stopped.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_LIGHTNING:
	if ( weather_info.mmhg > 1010
	|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The lightning has stopped.\n\r" );
	    weather_info.sky = SKY_RAINING;
	    break;
	}
	break;
    }

    if ( buf2[0] != '\0' || buf[0] != '\0' )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
          if ( d->connected == CON_PLAYING )
          {
	    if  (IS_OUTSIDE(d->character)
	    &&   IS_AWAKE(d->character) )
            {
                if ( !IS_SET( d->character->in_room->room_flags, ROOM_RED_SANDS) ) 
                  send_to_char( buf2, d->character );
		send_to_char( buf, d->character );
            }

	    if ( IS_VAMPIRE( d->character ) ) 
            {
              if ( time_info.hour == 6 ) 
              {
                if ( IS_SET( d->character->in_room->room_flags, ROOM_RED_SANDS) ) 
                  send_to_char("The sun's heat begins to weaken your body.\n\r", 
                    d->character);
                else
                  send_to_char("The sun rises, and its heat begins to weaken your body.\n\r", 
                    d->character);
                vampire_unenhance( d->character );
              }
              else if ( time_info.hour == 20 ) 
              {
                if ( IS_SET( d->character->in_room->room_flags, ROOM_RED_SANDS) ) 
                  send_to_char("You feel yourself growing stronger.\n\r", 
                    d->character);
                else
                  send_to_char("Night falls and you feel yourself growing stronger.\n\r",
	            d->character);
                vampire_enhance( d->character );
              }
	    }
            if ( d->character->level > 29
            &&   IS_WERECREATURE( d->character ) )
            {
              if ( time_info.hour == 6 )
              {
                if ( IS_SET( d->character->in_room->room_flags, ROOM_RED_SANDS) ) 
                  send_to_char("The beast within begins to settle.\n\r", 
                    d->character);
                else
                  send_to_char("The sun rises, and the beast within begins to settle.\n\r",
                    d->character);
                if IS_SET(d->character->affected_by, AFF_SHIFT_PERM)
                {
                   shift_strip(d->character);
                   REMOVE_BIT(d->character->affected_by, AFF_SHIFT_PERM);
                }
              }
              if ( time_info.hour == 19 ) 
                  send_to_char("You feel the beast within you begin to stir...\n\r",  
                    d->character);
              if ( time_info.hour == 20 )
                  were_chance(d->character);
            }
          }
	}
    }

    if (time_info.hour == 6) 
    {
        CHAR_DATA *sch_next;
        for ( sch = char_list ; sch ; sch = sch_next ) 
        {
            sch_next = sch->next;
            if (IS_NPC(sch) && sch->pIndexData->vnum == MOB_VNUM_SKELETAL) 
            {
                stop_follower( sch );
            }
        }
    }
    else if ( time_info.hour == 20 )
    {
        CHAR_DATA *sch_next;
        for ( sch = char_list ; sch ; sch = sch_next ) 
        {
            sch_next = sch->next;
            if (IS_NPC(sch) && sch->pIndexData->vnum == MOB_VNUM_SUNSKELLY) 
            {
                stop_follower( sch );
            }
        }

    }
    return;
}

void check_penalties( CHAR_DATA* ch )
{
  char wizbuf[ MAX_STRING_LENGTH ] = "";
  time_t now = time( NULL );

  if( ch->pcdata->restore_channels > 0 && ch->pcdata->restore_channels < now )
    {
      /* restore channels */
      send_to_char( "Your channels have been restored.  Behave yourself.\n\r", ch );
      REMOVE_BIT( ch->comm, COMM_NOCHANNELS );
      sprintf( wizbuf, "Channels automatically restored to %s.", ch->name );
      wiznet( wizbuf, NULL, NULL, WIZ_PENALTIES, 0, 0 );
      ch->pcdata->restore_channels = 0;
    }

  if( ch->pcdata->restore_emotes > 0 && ch->pcdata->restore_emotes < now )
    {
      /* restore emotes */
      send_to_char( "Your emotes have been restored.  Behave yourself.\n\r", ch );
      REMOVE_BIT( ch->comm, COMM_NOEMOTE );
      sprintf( wizbuf, "Emotes automatically restored to %s.", ch->name );
      wiznet( wizbuf, NULL, NULL, WIZ_PENALTIES, 0, 0 );
      ch->pcdata->restore_emotes = 0;
    }

  if( ch->pcdata->restore_shouts > 0 && ch->pcdata->restore_shouts < now )
    {
      /* restore shouts */
      send_to_char( "Your shouts have been restored.  Behave yourself.\n\r", ch );
      REMOVE_BIT( ch->comm, COMM_NOSHOUT );
      sprintf( wizbuf, "Shouts automatically restored to %s.", ch->name );
      wiznet( wizbuf, NULL, NULL, WIZ_PENALTIES, 0, 0 );
      ch->pcdata->restore_shouts = 0;
    }

  if( ch->pcdata->restore_tells > 0 && ch->pcdata->restore_tells < now )
    {
      /* restore tells */
      send_to_char( "Your tells have been restored.  Behave yourself.\n\r", ch );
      REMOVE_BIT( ch->comm, COMM_NOTELL );
      sprintf( wizbuf, "Tells automatically restored to %s.", ch->name );
      wiznet( wizbuf, NULL, NULL, WIZ_PENALTIES, 0, 0 );
      ch->pcdata->restore_tells = 0;
    }
}



void time_update( void )
{
    strftime( global_prompt_t, 256, "%H:%M",   localtime( &current_time ) );
    strftime( global_prompt_a, 256, "%I:%M%p", localtime( &current_time ) );
    return;
}

/*
 * Update all chars, including mobs.
*/
#define DOT_ATROPHY     0
#define DOT_DECAY       1
#define DOT_FESTER      2
#define DOT_BLISTER     3
#define DOT_HEADACHE    4
void char_update( void )
{   
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  CHAR_DATA *ch_quit;
  AFFECT_DATA *paff;
  bool bloodthirsty = FALSE;
  bool wither = FALSE;
  bool purgech = FALSE;
  bool purgepet = FALSE;
  int wakeup = -1;
  int dot_bitvec[] = { -1, -1, -1, -1, -1 };

  ch_quit	= NULL;
  
  /* update save counter */
  save_number++;
  
  if (save_number > SAVE_INTERVAL)
    save_number = 0;
  
  for ( ch = char_list; ch != NULL; ch = ch_next )
    {
      AFFECT_DATA *paf;
      AFFECT_DATA *paf_next;
      
      purgech = FALSE;
      purgepet = FALSE;
      wakeup = -1;
      ch_next = ch->next;
      bloodthirsty = FALSE;
      
      if ( !IS_IMMORTAL( ch ) && ch->timer > 30 )
	ch_quit = ch;

      /* automated penalties */
      if( !IS_NPC( ch ) &&
	  ( ch->pcdata->restore_channels > 0 ||
	    ch->pcdata->restore_emotes > 0 ||
	    ch->pcdata->restore_shouts > 0 ||
	    ch->pcdata->restore_tells > 0 ) )

	{
	  check_penalties( ch );
	}

      if ( global_config_aprilfools && !IS_NPC( ch ) && !ch->pcdata->aprilfools )
      {
        if ( number_range(1,13) == 1 )
        {
            ch->pcdata->aprilfools = TRUE;
        }
      }
      
      /*
       * push timer code
       */
      
      if( !IS_NPC( ch ) )
	{
	  if( ch->pcdata->push_count > PUSH_LIMIT )
	    {
	      ch->pcdata->push_count -= ( PUSH_LIMIT / 2 );
	    }
	  else
	    {
	      ch->pcdata->push_count = 0;
	    }
	}

      if( ch->race == race_lookup( "githyanki" ) )
	{
	  ch->affected_by |= AFF_INVISIBLE;
	}

      /*
       * Immortal monitoring code
       */
      if ( !IS_NPC(ch) )
      {
          if ( IS_IMMORTAL( ch ) )
          {
	      if( !IS_SET( ch->act_bits, PLR_LINKDEAD ) )
              {
	          if( ( ch->invis_level >= LEVEL_HERO ) || ( ch->incog_level >= LEVEL_HERO )  )
		      ch->pcdata->wizitime += PULSE_TICK / PULSE_PER_SECOND; 
	          else 
		      ch->pcdata->vistime += PULSE_TICK / PULSE_PER_SECOND;
              }
          }
          if ( is_guild( ch ) )
              ch->pcdata->guild_time += PULSE_TICK / PULSE_PER_SECOND;
      }

      /*
       * Recog timers
       */
      if ( ch->mprecog != NULL )
      {
          MPRECOG_DATA *mpr;
          INT_LIST *mprt, *mprt_next, *mprt_prev = NULL;
          bool remove;

          /* Loop over all recogs' vnums... */
          for ( mpr = ch->mprecog; mpr != NULL; mpr = mpr->next )
          {   /* Loop over all timers within that vnum */
              mprt_prev = NULL;
              for ( mprt = mpr->timer; mprt != NULL; mprt = mprt_next )
              {
                  remove = FALSE;
                  mprt_next = mprt->next;
                  /* If the duration is longer than 20+ years, assume it refers
                     to an epoch time instead */
                  if ( mprt->value[1] > 1266200751 ) 
                  {
                      if ( current_time > mprt->value[1] )
                          remove = TRUE;
                  }
                  else
                  {   /* Decrement each timer */
                      mprt->value[1] -= PULSE_TICK / PULSE_PER_SECOND;
                      if ( mprt->value[1] <= 0 )
                          remove = TRUE;
                  }

                  if ( remove ) 
                  {   /* Expire the appropriate flag on that recog vnum */
                      REMOVE_BIT( mpr->flags, mprt->value[0] );
                      /* Unlink the timer from the list */
                      if ( mprt_prev == NULL ) 
                          mpr->timer = mprt_next;
                      else
                          mprt_prev->next = mprt_next;
                      free_int_list( mprt );
                  }
                  else
                      mprt_prev = mprt;
              }
          }
      }

      /* chimestry mob check for extraction */
      if (IS_NPC(ch)
	  && ch->pIndexData->vnum == 25
	  && ch->fighting == NULL
	  && !IS_AFFECTED2(ch,AFF_CHIMESTRY))
	{ 
	  act("The illusion of $n fades away.",ch,NULL,NULL,TO_ROOM);
/*	  extract_char(ch,TRUE); */
          purgech = TRUE;
	}

   /*
    * bloodthirsty/torpor code for vampires
    */
    if ( IS_VAMPIRE( ch )
    &&   ch->level < LEVEL_IMMORTAL )
    {
        if ( !IS_SET( ch->act_bits, PLR_LINKDEAD ) 
        &&   ch->desc 
        &&   ch->desc->connected == CON_PLAYING )
        {
            int consump = 1;

            if ( IS_AFFECTED3( ch, AFF3_ABJURE ) )
                consump = 10;
            else if ( IS_AFFECTED3( ch, AFF3_FAMINE ) ) 
                consump = 2;

            if (ch->blood > 6) 
            {
                if ( ch->blood > ch->max_blood )
                    ch->blood = ch->max_blood;
                else
                    ch->blood -= consump;
            }
            else if (ch->blood > 0) 
            {
                send_to_char("Your body is low on blood!\n\r",ch);
                ch->blood -= UMIN(consump, ch->blood);
            }
            else 
            {
                AFFECT_DATA* old_torpor = NULL;
                AFFECT_DATA new_torpor;

                /* check for torpor affect */
                old_torpor = affect_find( ch->affected, -20 );

                /* if present, extract modifier and double, then strip the 
                   affect.  add the torpor affect with 1 or new modifier */
                new_torpor.where = TO_AFFECTS;
                new_torpor.type = -20;
                new_torpor.level = ch->level;
                new_torpor.duration = -1;
              
                if ( old_torpor )
                    new_torpor.modifier = 2 * old_torpor->modifier;
                else
                    new_torpor.modifier = -1;

                new_torpor.bitvector = AFF_TORPOR;
                new_torpor.location = APPLY_HIT;

                if ( old_torpor )
                    affect_remove( ch, old_torpor );

                if ( ( -(new_torpor.modifier) ) > ch->max_hit )
                {
                    send_to_char( 
                   "The blood magic holding your being together fails you!\n\r",
                        ch );
                    if ( IS_AFFECTED3( ch, AFF3_ABJURE ) )
                    {
                        send_to_char( 
                            "You quickly slip into unconsciousness.\n\r", ch );
                        unconscious_kill( ch );
                    }
                    else
                    {
                        raw_kill( ch );
                    }
                }
                else
                {
                    affect_to_char3( ch, &new_torpor );
                    send_to_char("You thirst for blood!\n\r",ch);
                    bloodthirsty = TRUE;
                }
            }
        }
    }
	
/*Seraph Life Energy +/- */

 if (ch->race == race_lookup("seraph")) {
 
	CHAR_DATA *vch;
	int company;
	int subtractEnergy;
	int addEnergy;
	int energy;
	int maxenergy;
	company =  0;
	energy = ch->life_energy;
	maxenergy = ch->max_life_energy;
	
/**
** This is probably one of the most fun pieces of code I've done yet. This will
** cycle through all the players in the room with the character. It will skip over
** the character themselves, as well as other Seraphs, and then return a tally
** of how many players were found in the room under those conditions. It 
** then takes that tally, and applies life energy to the Seraph. If there were no
** players found that met the conditions (ie: Seraph is alone or in the company
** of only another Seraph), it will deduct life energy.
**/

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{		
		if (!IS_NPC(vch) && vch->race != race_lookup("seraph") && vch != ch)
		{
			company++;
		}
	}
	
	if (company > 0) 
	{		
		addEnergy = energy + company;
			
		if (energy < maxenergy) 
		{
			// Don't let the energy go over their max.
			if (addEnergy > maxenergy)
			{
				ch->life_energy = maxenergy;
			}
				else 
				{
					ch->life_energy = addEnergy;
				}
		}					
	}			
		else
		{		
			subtractEnergy = ch->life_energy - 3;
			
			// And don't let it go under 0 (but allow it to actually GET to 0)
			if (subtractEnergy < 0) 
			{
				ch->life_energy = 0;
			}
				else 
				{
					ch->life_energy = subtractEnergy;
				}
		}

/**
** Seraphs go batshit insane if they're alone for too long.
**/
AFFECT_DATA af;
int randint;

	//Between 80 and 100: Stage 1 insanity
	if (ch-> life_energy <= 100 && ch-> life_energy >= 80)
	{
		randint = number_range(1,20);
		if (randint < 12) 
		{
			switch(randint)
			{
				case 1: send_to_char( "The world shimmers in and out of view.\n\r", ch ); break;
				case 2: send_to_char( "You feel watched!\n\r", ch ); break;
				case 3: send_to_char( "Blood begins to seep from your pores.\n\r", ch ); break;
				case 4: check_social(ch,"hug",ch->name);break;
				case 5: check_social(ch,"cry", ch->name);break;
				case 6: check_social(ch,"giggle", ch->name);break;
				case 7: check_social(ch,"cling", ch->name);break;
				case 8: check_social(ch,"cower","");break;
				case 9: check_social(ch,"flinch", ch->name);break;
				case 10: check_social(ch,"wave", ch->name);break;
				case 11: send_to_char( "You feel insects crawling on your skin.\n\r", ch ); break;
			}
		}
	}
		
	//Between 79 and 60: Stage 2 insanity
	if (ch-> life_energy <= 79 && ch-> life_energy >= 60)
	{
		randint = number_range(1,30);
		if (randint < 25) 
		{
			switch(randint)
			{
				case 1: 		
				send_to_char( "You wonder what Stormy is up to.\n\r", ch );
				do_cast(ch,"portal stormy");
				break;
							
				case 2:					
				send_to_char( "You wonder what Esme is up to.\n\r", ch );
				do_cast(ch,"portal esme");
				break;
												
				case 3: 		
				send_to_char( "You wonder what Arianna is up to.\n\r", ch );
				do_cast(ch,"portal arianna");
				break;
							
				case 4: 
				send_to_char( "You decide to take a look around.\n\r", ch );
				do_scan(ch,"");
				break;
						
				case 5: 
				send_to_char( "You become worried that you're alone.\n\r", ch );
				do_look(ch,"");
				break;
							
				case 6: 
				send_to_char( "You become worried that you're alone.\n\r", ch );
				do_look(ch,"");
				break;
							
				case 7: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_north(ch,"");
				break;

				case 8: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_south(ch,"");
				break;

				case 9: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_east(ch,"");
				break;
							
				case 10: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_west(ch,"");
				break;
							
				case 11: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_up(ch,"");
				break;
						
				case 12: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_down(ch,"");
				break;
							
				case 13: 
				send_to_char( "You're bored.\n\r", ch );
				do_cast(ch,"teleport");
				break;
				
				case 14: send_to_char( "The world shimmers in and out of view.\n\r", ch ); break;
				case 15: send_to_char( "You feel watched!\n\r", ch ); break;
				case 16: send_to_char( "Blood begins to seep from your pores.\n\r", ch ); break;
				case 17: check_social(ch,"hug",ch->name);break;
				case 18: check_social(ch,"cry", ch->name);break;
				case 19: check_social(ch,"giggle", ch->name);break;
				case 20: check_social(ch,"cling", ch->name);break;
				case 21: check_social(ch,"cower","");break;
				case 22: check_social(ch,"flinch", ch->name);break;
				case 23: check_social(ch,"wave", ch->name);break;
				case 24: send_to_char( "You feel insects crawling on your skin.\n\r", ch ); break;
			}
		}
	}
	
	//Between 59 and 40: Stage 3 insanity
	if (ch-> life_energy <= 59 && ch-> life_energy >= 40)
	{
		randint = number_range(1,25);
		if (randint < 24) 
		{
			switch(randint)
			{
				case 1: 
				send_to_char( "You are overcome with lethargy.\n\r", ch ); 
				ch->move -=150;
				break;
				
				case 2: 
				do_remove(ch,"1.");
				do_drop(ch, "all");
				break;
				
				case 3: 
				do_remove(ch,"3.");
				do_drop(ch, "all");
				break;
				
				case 4: 
				do_remove(ch,"5.");
				do_drop(ch, "all");				
				break;
				
				case 5:
				
				af.where   	 = TO_AFFECTS;
				af.type     	 = gsn_sleep;
				af.level   		 = ch->level; 
				af.duration	 = 1;
				af.modifier 	= 0; 
				af.bitvector	 = AFF_SLEEP;
				affect_to_char( ch, &af );

				if ( IS_AWAKE(ch) )
				{
					send_to_char( "You decide to take a nap.\n\r", ch );
					act( "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
					ch->position = POS_SLEEPING;
				}				
				break;
				
				case 6: send_to_char( "The world shimmers in and out of view.\n\r", ch ); break;
				case 7: send_to_char( "You feel watched!\n\r", ch ); break;
				case 8: send_to_char( "Blood begins to seep from your pores.\n\r", ch ); break;
				case 9: check_social(ch,"hug",ch->name);break;
				case 10: check_social(ch,"cry", ch->name);break;
				case 11: check_social(ch,"giggle", ch->name);break;
				case 12: check_social(ch,"cling", ch->name);break;
				case 13: check_social(ch,"cower","");break;
				case 14: check_social(ch,"flinch", ch->name);break;
				case 15: check_social(ch,"wave", ch->name);break;
				case 16: send_to_char( "You feel insects crawling on your skin.\n\r", ch ); break;
				
				case 17: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_north(ch,"");
				break;

				case 18: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_south(ch,"");
				break;

				case 19: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_east(ch,"");
				break;
							
				case 20: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_west(ch,"");
				break;
							
				case 21: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_up(ch,"");
				break;
						
				case 22: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_down(ch,"");
				break;
							
				case 23: 
				send_to_char( "You're bored.\n\r", ch );
				do_cast(ch,"teleport");
				break;
				
				
			}
		}
	}
	
	//Between 39 and 20: Stage 4 insanity
	if (ch-> life_energy <= 39 && ch-> life_energy >= 20)
	{
		randint = number_range(1,20);
		if (randint < 18) 
		{
			switch(randint)
			{
				case 1: 
				send_to_char( "You lose all control and begin to scratch at your eyes!\n\r", ch );
				act( "$n begins to claw at $m own eyes!", ch, NULL, NULL, TO_ROOM );
				ch->pcdata->lastfight = current_time;
				ch->hit -= 50;
				 break;
				 
				case 2: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_north(ch,"");
				break;

				case 3: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_south(ch,"");
				break;

				case 4: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_east(ch,"");
				break;
							
				case 5: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_west(ch,"");
				break;
							
				case 6: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_up(ch,"");
				break;
						
				case 7: 
				send_to_char( "Time for a walk.\n\r", ch );
				do_down(ch,"");
				break;
							
				case 8: 
				send_to_char( "You're bored.\n\r", ch );
				do_cast(ch,"teleport");
				break;
				
				case 9: check_social(ch,"hug",ch->name);break;
				case 10: check_social(ch,"cry", ch->name);break;
				case 11: check_social(ch,"giggle", ch->name);break;
				case 12: check_social(ch,"cling", ch->name);break;
				case 13: check_social(ch,"cower","");break;
				case 14: check_social(ch,"flinch", ch->name);break;
				case 15: check_social(ch,"wave", ch->name);break;
				case 16: send_to_char( "You feel insects crawling on your skin.\n\r", ch ); break;
				
				case 17:
				
				af.where   		= TO_AFFECTS;
				af.type      		= gsn_forget;
				af.level      		= ch->level; 
				af.duration 		= 1;
				af.bitvector 	= AFF2_FORGET;
				af.location   	= APPLY_SKILLS;
				af.modifier  	= -10;
				affect_to_char( ch, &af );

				send_to_char( "Your mind goes blank.\n\r", ch );
				act( "$n gets a dazed look in $m eyes.", ch, NULL, NULL, TO_ROOM );
				break;
				
				
			}
		}
	}
	
	//Between 19 and 0: Stage 5 insanity
	if (ch-> life_energy <= 19 && ch-> life_energy >= 0)
	{
		randint = number_range(1,15);
		if (randint < 13) 
		{
			switch(randint)
			{				
				case 1: 
				send_to_char( "You are overcome with lethargy.\n\r", ch ); 
				ch->move -=150;
				break;
				
				case 2: 
				do_remove(ch,"1.");
				do_drop(ch, "all");
				break;
				
				case 3: 
				do_remove(ch,"3.");
				do_drop(ch, "all");
				break;
				
				case 4: send_to_char( "The world shimmers in and out of view.\n\r", ch ); break;
				
				case 5:
				
				af.where   		= TO_AFFECTS;
				af.type      		= gsn_forget;
				af.level      		= ch->level; 
				af.duration 		= 1;
				af.bitvector 	= AFF2_FORGET;
				af.location   	= APPLY_SKILLS;
				af.modifier  	= -10;
				affect_to_char( ch, &af );
				
				send_to_char( "Your mind goes blank.\n\r", ch );
				act( "$n gets a dazed look in $m eyes.", ch, NULL, NULL, TO_ROOM );
				break;
				
				case 6:
				
				af.where   		= TO_AFFECTS;
				af.type      		= gsn_decay;
				af.level      		= ch->level; 
				af.whichaff 		= 10;
				af.duration 		= 1;
				af.location  	= APPLY_DEX;
				af.modifier 	= -2; 
				af.bitvector 	= AFF2_NOREGEN;
				affect_to_char( ch, &af );
				
				send_to_char( "Your bones begin to decay.\n\r", ch );
				act( "$n's skeletal form staggers as $s bones begin to decay.", ch, NULL, NULL, TO_ROOM );
				break;
				
				case 7: 
				send_to_char( "You lose all control and begin to scratch at your eyes!\n\r", ch );
				act( "$n begins to claw at $m own eyes!", ch, NULL, NULL, TO_ROOM );
				ch->pcdata->lastfight = current_time;
				ch->hit -= 50;
				break;
				
				case 8: 		
				send_to_char( "{DChaotic whispers resound and drive you to madness.{x\n\r", ch );
				do_cast(ch,"portal clestus");
				break;
				
				case 9:
				
				af.where   	 = TO_AFFECTS;
				af.type     	 = gsn_sleep;
				af.level   		 = ch->level; 
				af.duration	 = 1;
				af.modifier 	= 0; 
				af.bitvector	 = AFF_SLEEP;
				affect_to_char( ch, &af );

				if ( IS_AWAKE(ch) )
				{
					send_to_char( "You decide to take a nap.\n\r", ch );
					act( "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
					ch->position = POS_SLEEPING;
				}				
				break;
				
				case 10: check_social(ch,"flinch", ch->name);break;
				case 11: check_social(ch,"wave", ch->name);break;
				case 12: send_to_char( "You feel insects crawling on your skin.\n\r", ch ); break;
				case 13: send_to_char( "You feel watched!\n\r", ch ); break;
			}
		}
	}
}

   /* 
    * recover hp/mana/moves on tick
    */
    if ( IS_AFFECTED3( ch, AFF3_FAMINE ) )
        ch->hit = UMAX(ch->hit-1, 1);
    if ( ch->hit < ch->max_hit )
    {
        ch->hit = UMAX(1,ch->hit + ((bloodthirsty) ? 0 : hit_gain(ch)));
        if ( IS_AFFECTED3( ch, AFF3_FAMINE ) )
        {
            OBJ_DATA *book;
            if ( ( book = get_eq_char( ch, WEAR_HOLD ) ) == NULL 
            ||    !is_name( "-PowerFamine- -imp-" , book->name) )
            {
                act("$n's body grows gaunt and skeletal as unnatural famine consumes $m.", ch, NULL, NULL, TO_ROOM );
                send_to_char( "Your body grows gaunt and skeletal as unnatural famine consumes you.\n\r", ch );
            }
            else
            {
                act( "$n's hands grow gaunt and skeletal, and the Book pulses with voracious hunger.", ch, NULL, NULL, TO_ROOM );
                send_to_char( "Your hands grow gaunt and skeletal as the Book pulses with voracious hunger.\n\r", ch );
            }
        }
    }
    else
        ch->hit = ch->max_hit;

    if ( ch->mana < ch->max_mana )
        ch->mana = UMAX(0,ch->mana+((bloodthirsty) ? 0 : mana_gain(ch)));
    else
        ch->mana = ch->max_mana;

    if ( ch->move < ch->max_move )
        ch->move = UMAX(0,ch->move + move_gain(ch));
    else
        ch->move = ch->max_move;

    /*
     * lights burning out and PC void-outs
     */
    if ( !IS_NPC(ch) )
    {
	  ch->timer++;
	    
	  if( !IS_IMMORTAL( ch ) )
	    {
	      OBJ_DATA *obj;

	      if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
		   &&   obj->item_type == ITEM_LIGHT
		   &&   obj->value[2] > 0 )
		{
		  if ( --obj->value[2] == 0 && ch->in_room != NULL )
		    {
		      --ch->in_room->light;
		      act( "$p goes out.", ch, obj, NULL, TO_ROOM );
		      act( "$p flickers and goes out.", ch, obj, NULL, TO_CHAR );
		      extract_obj( obj );
		    }
		  else if ( obj->value[2] <= 5 && ch->in_room != NULL)
		    act("$p flickers.",ch,obj,NULL,TO_CHAR);
		}
	    }

	  if ( ch->timer >= 12 )
	    {
	      if( IS_IMMORTAL( ch ) )
		{
		  if( !IS_SET( ch->comm, COMM_AFK ) && IS_SET( ch->act_bits, PLR_AUTO_AFK ) )
		    {
		      do_afk( ch, "" );
		    }
		}
	      else if ( ch->was_in_room == NULL 
                   &&   ch->in_room != NULL 
                   /* natures pestilence = no void out, 9/3/12 gkl */
                   &&   (   !IS_AFFECTED(ch,AFF_CURSE) 
                         || !is_affected(ch, gsn_natures_pestil) ) )
              {
		  ch->was_in_room = ch->in_room;
		  if ( ch->fighting != NULL )
		    stop_fighting( ch, TRUE );
		  act( "$n disappears into the void.",
		       ch, NULL, NULL, TO_ROOM );
		  send_to_char( "You disappear into the void.\n\r", ch );
		  if (ch->level > 1)
		    save_char_obj( ch );
		  char_from_room( ch );
		  char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
		  SET_BIT( ch->act_bits, PLR_LINKDEAD );
		}
	    }
	    
	  gain_condition( ch, COND_DRUNK,  -1 );
	}



     /* 
      * The checks for spells that have an effect on their last tick must go 
      * here (necro spells, headache).  That way their damage can be dealt
      * after the affect expires below
      *
      */
        if ((paff = affect_find(ch->affected,gsn_blister)) != NULL)
            dot_bitvec[DOT_BLISTER] = paff->level;
        else 
            dot_bitvec[DOT_BLISTER] = -1;
        if ((paff = affect_find(ch->affected,gsn_decay)) != NULL)
            dot_bitvec[DOT_DECAY] = paff->level;
        else 
            dot_bitvec[DOT_DECAY] = -1;
        if ((paff = affect_find(ch->affected,gsn_atrophy)) != NULL)
            dot_bitvec[DOT_ATROPHY] = paff->level;
        else
            dot_bitvec[DOT_ATROPHY] = -1;
        if ((paff = affect_find(ch->affected,gsn_wilt)) != NULL)
            dot_bitvec[DOT_FESTER] = paff->level;
        else
            dot_bitvec[DOT_FESTER] = -1;
        if (IS_AFFECTED2(ch, AFF_HEADACHE)
        &&  (paff = affect_find(ch->affected,gsn_headache)) != NULL)
            dot_bitvec[DOT_HEADACHE] = paff->level;
        else
            dot_bitvec[DOT_HEADACHE] = -1;

        /* remove the conjure familiar permboost if familiar is gone */
        if ( (paff = affect_find( ch->affected, gsn_familiar_link )) != NULL
        &&   ( ch->pet == NULL
            || !IS_NPC(ch->pet)
            || ch->pet->pIndexData->vnum != MOB_VNUM_FAMILIAR ) )
            paff->duration = 0;

        /* Spell duration function */
        for ( paf = ch->affected; paf != NULL; paf = paf_next )
        {
            paf_next	= paf->next;
            if ( paf->duration > 0 )
            {
                paf->duration--;
                if (number_range(0,4) == 0 && paf->level > 0)
                    paf->level--;  /* spell strength fades with time */
            }
            else if ( paf->duration < 0 )
                ;
            else /* paf->duration == 0 */
            {
                if ( paf_next == NULL   /* msg_off only fires for the last
                                           aff applied for a series of affs
                                           applied at once with the same gsn */
                ||   paf_next->type != paf->type
                ||   paf_next->duration > 0 )
                {
                    if ( paf->type > 0 && skill_table[paf->type].msg_off )
                    {
                    /* All new summonable pet spells need to be assigned a gsn 
                       and added here for them to fade properly.  This will 
                       ensure that they don't stick around after their spells 
                       wear off. */
                        if ( ch->leader != NULL
                        &&   (paf->type == gsn_sumanimal
                        ||    paf->type == gsn_chaosdemon
                        ||    paf->type == gsn_summon_skeletal_army
                        ||    paf->type == gsn_sun_skeletals
                        ||    paf->type == gsn_call_spiders
                        ||    paf->type == gsn_call_water_spirit
                        ||    paf->type == gsn_animate_dead
                        ||    paf->type == gsn_summon_meerkats 
                        ||    paf->type == gsn_conjure_familiar
                        ||    paf->type == gsn_conjure_bats) )
                        {
                            /* this being placed here means that pets whose
                            summon spell don't have a msg_off set will not fade?
                            is this right?  --gkl 4/23/2012 */
                            purgepet = TRUE;
                        }
                        else if ((paf->type == gsn_charm_person) 
                             && (ch->leader))
                        {
                            send_to_char( skill_table[paf->type].msg_off, ch);
                            send_to_char( "\n\r", ch);
                            ch->leader->charmies -= 1;
                        }
                        else if ((paf->type == gsn_domination) && (ch->leader))
                        {
                            send_to_char( skill_table[paf->type].msg_off, ch);
                            send_to_char( "\n\r", ch);
                            ch->leader->pet = NULL;
                            ch->leader->charmies -= 1;
                        }
                        else if (IS_NPC( ch ) 
                        &&  ch->position != ch->pIndexData->default_pos
                        && (paf->type == gsn_sleep
                        ||  paf->type == gsn_sap ) )
                        {
                            send_to_char( skill_table[paf->type].msg_off, ch );
                            send_to_char( "\n\r", ch );
                            wakeup = ch->pIndexData->default_pos;
                        }
                        else if ( paf->type == gsn_atrophy
                        ||   paf->type == gsn_wilt
                        ||   paf->type == gsn_decay
                        ||   paf->type == gsn_blister
                        ||   paf->type == gsn_headache )
                        /* Do nothing; these messages are handled separately 
                        because they require that damage be done after they wear
                        off. */;
                        else
                        {
                            send_to_char( skill_table[paf->type].msg_off, ch );
                            send_to_char( "\n\r", ch );
                        }
                    }
                }

                if ( IS_NPC(ch) && paf->type == gsn_wither_limb )
                    wither = TRUE;
                else
                    wither = FALSE;
		    
                affect_remove( ch, paf );

            if ( wither )
                do_wear( ch, "all" );
            if ( wakeup == POS_RESTING )
                do_rest( ch, "" );
            else if ( wakeup == POS_SITTING )
                do_sit( ch, "" );
            else if ( wakeup == POS_STANDING )
                do_stand( ch, "" );
	    }
	}

    if ( IS_AFFECTED3( ch, AFF_PINCH ) )
        check_pinch( ch );

    if ( purgech ) 
    {
        extract_char(ch, TRUE);
        continue;
    }

    if ( purgepet )
    {
        stop_follower( ch );
        continue;
    }

    /* 
     * sanguine bond -- detects vampires on tick 
     */
      if ( IS_AFFECTED3(ch, AFF3_SBOND) )
      {
          CHAR_DATA *vch;
          int vamps = 0;

          for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
              if ( IS_VAMPIRE(vch) && !IS_IMMORTAL(vch) 
              &&   (100*vch->blood/vch->max_blood) < 50 )
                  vamps++;

	      for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	      {
              if ( IS_VAMPIRE(vch) && !IS_IMMORTAL(vch)
              &&   (100*vch->blood/vch->max_blood) < 50
              /* 8% chance of seeing a single vamp */
              &&   number_range(1,1000) < 80/vamps )
              {
                  act( 
      "{rFor a brief moment, you see a flickering red aura appear around $N.{x",
                      ch, NULL, vch, TO_CHAR );
              }
          }
      }

      damage_on_tick( ch, dot_bitvec );
    }

  /*	
   * Autosave and autoquit.
   * Check that these chars still exist.
   */

  for ( ch = char_list; ch != NULL; ch = ch_next )
    {
      ch_next = ch->next;

      if (ch->desc != NULL && ch->desc->descriptor % SAVE_INTERVAL == save_number)
	save_char_obj(ch);

      if ( ch == ch_quit )
	do_quit( ch, "quit" );
    }

  return;
}


void damage_on_tick( CHAR_DATA *ch, int dot_bitvec[] )
{
    CHAR_DATA *vch;
    AFFECT_DATA *paff; 
    AFFECT_DATA af;
    bool diseased = FALSE;
    int dam;

    if ( !IS_VALID(ch) ||  ch->in_room == NULL )
        return;

   /*
    * Diseases (plague, unholy fire, pestilence) and poison
    */
    if ( IS_AFFECTED(ch, AFF_PLAGUE) )
    {
        diseased = TRUE;
       /*
        * unholy fire (githyanki were ability)
        */
        if ( (paff = affect_find(ch->affected, gsn_unholyfire)) != NULL )
        {
            act( "$n screams as unholy fire tortures $s flesh.",
                ch, NULL, NULL, TO_ROOM );
            send_to_char( 
              "You scream in agony as the unholy fire tortures your flesh.\n\r",
                ch );
    
            dam = UMIN( ch->level, paff->level/5+1);
            ch->mana -= dam;
            ch->move -= dam;
            damage( ch, ch, dam, gsn_unholyfire, DAM_DISEASE, FALSE );
            diseased = FALSE;
        }

        /* 
         * pestilence (book of plagues spell)
         */
        if ( (paff = affect_find(ch->affected, gsn_pestilence)) != NULL )
        {
            if (ch->in_room == NULL)
                return;
 
         act("Several boils covering $n burst, spraying vile pus into the air.",
                ch,NULL,NULL,TO_ROOM);
                send_to_char( 
      "Several boils covering your skin burst, spraying liquid everywhere.\n\r",
                ch );

            for ( vch=ch->in_room->people; vch != NULL; vch=vch->next_in_room )
            {
                if (!saves_spell(paff->level, NULL, vch, DAM_DISEASE) 
                &&  !IS_IMMORTAL(vch)
                &&  vch->position != POS_UNCONSCIOUS
                &&  !IS_AFFECTED2(vch, AFF_GHOST)
                &&  !IS_AFFECTED(vch, AFF_PLAGUE) 
                &&  ( !IS_NPC( vch ) 
                    || (!IS_SET(vch->act_bits, PLR_NEWBIE) && ch->level > 10) )
                /* can't infect special mobs (shopkeepers, etc) */
                &&  (!IS_NPC( vch )
                || ( vch->pIndexData->pShop == NULL
                &&   !IS_SET(vch->act_bits,ACT_TRAIN)
                &&   !IS_SET(vch->act_bits,ACT_PRACTICE)
                &&   !IS_SET(vch->act_bits,ACT_IS_HEALER)
                &&   !IS_SET(vch->act_bits,ACT_IS_CHANGER)
                &&   !IS_SET(vch->act_bits,ACT_NOKILL) ) ) )
                {
                    spell_pestilence( gsn_pestilence, - paff->level, ch, vch,
                        TARGET_CHAR );
                }
            }
            /* same amount of damage as blister */
            dam = dice(paff->level,10);
            damage_old( ch, ch, dam, gsn_plague, DAM_DISEASE, FALSE );
            diseased = TRUE;
        }
        else if ( is_affected(ch, gsn_plague) )
        {
            if (ch->in_room == NULL)
                return;
    
            act("$n writhes in agony as plague sores erupt from $s skin.",
                ch,NULL,NULL,TO_ROOM);
            send_to_char("You writhe in agony from the plague.\n\r",ch);
            for ( paff = ch->affected; paff != NULL; paff = paff->next )
            {
                if (paff->type == gsn_plague)
                    break;
            }
       
            if (paff == NULL)
            {
                REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
                return;
            }
        
            if (paff->level == 1)
                return;
        
            af.where    = TO_AFFECTS;
            af.type     = gsn_plague;
            af.level    = paff->level - 1; 
            af.duration = number_range(1,2 * af.level);
            af.location	= APPLY_STR;
            af.modifier = -5;
            af.bitvector = AFF_PLAGUE;
        
            for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
            {
                if (!saves_spell(af.level - 2,NULL,vch,DAM_DISEASE) 
                &&  !IS_IMMORTAL(vch)
                &&  vch->position != POS_UNCONSCIOUS
                &&  !IS_AFFECTED2(vch, AFF_GHOST)
                &&  !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(4) == 0)
                {
                    if ( !IS_NPC( vch )
                    || ( vch->pIndexData->pShop == NULL
                    &&   !IS_SET(vch->act_bits,ACT_TRAIN)
                    &&   !IS_SET(vch->act_bits,ACT_PRACTICE)
                    &&   !IS_SET(vch->act_bits,ACT_IS_HEALER)
                    &&   !IS_SET(vch->act_bits,ACT_IS_CHANGER)
                    &&   !IS_SET(vch->act_bits,ACT_NOKILL) ) )
                    {
                        send_to_char("You feel hot and feverish.\n\r",vch);
                        act("$n shivers and looks very ill.",
                            vch,NULL,NULL,TO_ROOM);
                        affect_join( vch, &af, FALSE );
                    }
                }
            }
            dam = UMIN(ch->level,paff->level/5+1);
            ch->mana -= dam;
            ch->move -= dam;
            damage_old( ch, ch, dam, gsn_plague, DAM_DISEASE, FALSE );
            diseased = TRUE;
        }
    }
  /*
   *  Poison
   */
    if ( !diseased
    &&   IS_AFFECTED(ch, AFF_POISON)
    &&  !IS_AFFECTED(ch, AFF_SLOW) 
    &&  ch->in_room )
    {
        sh_int pos_before;

        if ( (paff = affect_find(ch->affected,gsn_poison) ) != NULL)
        {
            act( "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You shiver and suffer.\n\r", ch );
  
            if ( IS_NPC( ch ) )
                dam = 50;
            else
                dam = ( ch->max_hit * 2 / 100 ) * 2;

            pos_before = ch->position;
            damage( ch, ch, dam, gsn_poison, DAM_POISON, TRUE );

            if ( pos_before == POS_SLEEPING
            && ( global_config_newpoison 
            ||   is_affected(ch, gsn_sap) ) )
                ch->position = POS_SLEEPING;
        }
    }

    /* if ch->in_room == NULL, that means ch has died at this point */
    if (ch->in_room == NULL) return;

   /*
    * blood boil
    */
    if ( IS_AFFECTED2(ch, AFF_BBOIL) 
    &&   (paff = affect_find(ch->affected,gsn_bboil)) != NULL )
    {
        act( "$n trembles with boiling blood.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You quiver as your blood boils.\n\r", ch );
        damage_old( ch, ch, dice( paff->level + 40, 10 ), gsn_bboil,
            DAM_DISEASE, TRUE );
        if (ch->in_room == NULL) return;
    }

    /*
     * immolation
     */
    if ( IS_AFFECTED2( ch, AFF2_BURNING )
    &&   ( paff = affect_find(ch->affected,gsn_immolation) ) != NULL )
    {
        raw_damage( ch, ch, dice( paff->level, 6 ), gsn_immolation,
            DAM_FIRE, TRUE, TRUE );
        if (ch->in_room == NULL) return;
    }

    /* 
     * book of power spells
     */
    if ( IS_VAMPIRE( ch ) )
    {
        if ( (paff = affect_find(ch->affected,gsn_white_grimoire)) != NULL )
        {
            act( "$n's flesh smolders and smokes.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "Your flesh burns and smokes.\n\r", ch );
            damage_old( ch, ch, dice( paff->level, 7 ), gsn_white_grimoire,
                DAM_HOLY, TRUE );
            if (ch->in_room == NULL) return;
        }
        if ( IS_AFFECTED3( ch, AFF3_ABJURE ) 
        &&   (paff = affect_find(ch->affected,gsn_abjuration)) != NULL )
        {
            act( "Smoke rises from $n, and strange runes seem to writhe on the surface of $s skin!", ch, NULL, NULL, TO_ROOM );
            send_to_char( "Your flesh burns and smokes.\n\r", ch );
            raw_damage( ch, ch, number_range(100,200), paff->type,
                DAM_HOLY, TRUE, FALSE );
            if (ch->in_room == NULL) return;
        }
    }
    else if ((paff = affect_find(ch->affected,gsn_dark_grimoire) ) != NULL)
    {
        act("$n's flesh smolders and smokes.", ch, NULL, NULL, TO_ROOM);
        send_to_char( "Your flesh burns and smokes.\n\r", ch );
        damage_old( ch, ch, dice( paff->level, 7 ), gsn_dark_grimoire,
            DAM_NEGATIVE, TRUE );
        if (ch->in_room == NULL) return;
    }
    if ( !IS_VAMPIRE(ch)
    &&   ( paff = affect_find(ch->affected,gsn_sanguination) ) != NULL )
    {
        act( "Blood oozes from $n's flesh like sweat.", 
            ch, NULL, NULL, TO_ROOM );
        send_to_char( "Blood oozes from your flesh like sweat!\n\r", ch );
        raw_damage( ch, ch, dice( paff->level, 3 ), gsn_sanguination,
            DAM_NEGATIVE, TRUE, FALSE );
        if (ch->in_room == NULL) return;
    }

    /* Headache check done this way because it deals 1 damage to the victim */
    if ( dot_bitvec[DOT_HEADACHE] > 0 )
    {
        act( "$n's head throbs.", ch, NULL, NULL, TO_ROOM );
        send_to_char("Your head throbs, then your headache subsides.\n\r", ch);
        damage_old( ch, ch, 1, gsn_headache, DAM_MENTAL, FALSE );
        ch->mana = UMAX(ch->mana - dot_bitvec[DOT_HEADACHE]*4, 0);
        if (ch->in_room == NULL) return;
    }

    if ( dot_bitvec[DOT_BLISTER] > 0)
    {
        act( "$n's skin blisters and burns.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You blister and burn.\n\r", ch );
        damage_old(ch,ch,dice(dot_bitvec[DOT_BLISTER],10),gsn_blister,
            DAM_DISEASE,TRUE);
        if (ch->in_room == NULL) return;
        if ( !is_affected(ch, gsn_blister) )
        {
            send_to_char( skill_table[gsn_blister].msg_off, ch);
            send_to_char( "\n\r", ch);
        }
    }
    if ( dot_bitvec[DOT_DECAY] > 0 )
    {
        act( "$n's body lapses into decay.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "Your body decays.\n\r", ch );
        damage_old(ch,ch,dice(dot_bitvec[DOT_DECAY]-15,10),gsn_decay,
            DAM_DISEASE,TRUE);
        if (ch->in_room == NULL) return;
        if ( !is_affected(ch, gsn_decay) )
        {
            send_to_char( skill_table[gsn_decay].msg_off, ch);
            send_to_char( "\n\r", ch);
        }
    }
    if ( dot_bitvec[DOT_ATROPHY] > 0 )
    {
        act( "$n's body wastes away.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "Your body wastes away.\n\r", ch );
        damage_old(ch,ch,dot_bitvec[DOT_ATROPHY]*2+number_range(1,25),
            gsn_atrophy, DAM_DISEASE,TRUE);
        if (ch->in_room == NULL) return;
        if ( !is_affected(ch, gsn_atrophy) )
        {
            send_to_char( skill_table[gsn_atrophy].msg_off, ch);
            send_to_char( "\n\r", ch);
        }
    }
    if ( dot_bitvec[DOT_FESTER] > 0 )
    {
        act( "$n screams in pain!", ch, NULL, NULL, TO_ROOM );
        send_to_char( "Your body is stricken with pain.\n\r", ch );
        damage_old(ch,ch,dice(dot_bitvec[DOT_FESTER]+35,10),gsn_wilt,
            DAM_DISEASE,TRUE);
        if (ch->in_room == NULL) return;
        if ( !is_affected(ch, gsn_wilt) )
        {
            send_to_char( skill_table[gsn_wilt].msg_off, ch);
            send_to_char( "\n\r", ch);
        }
    }
    return;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf, *paf_next;

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	CHAR_DATA *rch;
	char *message;

	obj_next = obj->next;

	/* go through affects and decrement */
        for ( paf = obj->affected; paf != NULL; paf = paf_next )
        {
            paf_next    = paf->next;
            if ( paf->duration > 0 )
            {
                paf->duration--;
                if (number_range(0,4) == 0 && paf->level > 0)
                  paf->level--;  /* spell strength fades with time */
            }
            else if ( paf->duration < 0 )
                ;
            else
            {
                if ( paf_next == NULL
                ||   paf_next->type != paf->type
                ||   paf_next->duration > 0 )
                {
                    if ( paf->type > 0 && skill_table[paf->type].msg_obj )
                    {
			if (obj->carried_by != NULL)
			{
			    rch = obj->carried_by;
			    act(skill_table[paf->type].msg_obj,
				rch,obj,NULL,TO_CHAR);
			}
			if (obj->in_room != NULL 
			&& obj->in_room->people != NULL)
			{
			    rch = obj->in_room->people;
			    act(skill_table[paf->type].msg_obj,
				rch,obj,NULL,TO_ALL);
			}
                    }
                }

                affect_remove_obj( obj, paf );
            }
        }

        /* seeds growing--seeds on ground that aren't takeable are "planted" */
        if ( obj->item_type == ITEM_SEED 
        &&   !CAN_WEAR(obj, ITEM_TAKE) 
        &&   obj->in_room != NULL ) /* if char has item, in_room == NULL */
        {
            OBJ_INDEX_DATA *fruitObjIndex;
            OBJ_DATA *fruitObj;
            MOB_INDEX_DATA *fruitMobIndex;
            CHAR_DATA *fruitMob;
            int i;
           
            if ( --obj->value[3] < 0 )
            {
                if ( obj->value[4] == 0
                &&   (fruitObjIndex = get_obj_index(obj->value[0])) != NULL )
                {
                    for ( i = 0; i < obj->value[2]; i++ )
                    {
                        fruitObj = create_object( fruitObjIndex, 0 );
                        obj_to_room( fruitObj, obj->in_room );
                    }
                    if ( obj->in_room->people ) 
                    {
                        act( "$p sprouts into $P.", obj->in_room->people, obj, 
                            fruitObj, TO_ROOM );
                        act( "$p sprouts into $P.", obj->in_room->people, obj, 
                            fruitObj, TO_CHAR );
                    }
                }
                else if ( obj->value[4] != 0
                &&   (fruitMobIndex = get_mob_index(obj->value[0])) != NULL )
                {
                    for ( i = 0; i < obj->value[2]; i++ )
                    {
                        fruitMob = create_mobile(fruitMobIndex);
                        char_to_room( fruitMob, obj->in_room );
                    }
                    if ( obj->in_room->people ) 
                    {
                        act( "$p sprouts into $n.", fruitMob, obj, 
                            NULL, TO_ROOM );
                    }
                }
	        extract_obj( obj );
                continue;
            }
        }

	if ( obj->timer <= 0 || --obj->timer > 0 )
	    continue;

	switch ( obj->item_type )
	{
	default:              message = "$p crumbles into dust.";  break;
	case ITEM_FOUNTAIN:   message = "$p dries up.";         break;
	case ITEM_CORPSE_NPC: message = "$p decays into dust."; break;
	case ITEM_CORPSE_PC:  message = "$p decays into dust."; break;
        case ITEM_PLANT:      message = "$p withers and dies."; break;
        case ITEM_SEED:
	case ITEM_FOOD:       message = "$p decomposes.";	break;
	case ITEM_POTION:     message = "$p has evaporated from disuse.";	
								break;
	case ITEM_PORTAL:     message = "$p fades out of existence."; break;
	case ITEM_CONTAINER: 
	    if (CAN_WEAR(obj,ITEM_WEAR_FLOAT))
		if (obj->contains)
		    message = 
		"$p flickers and vanishes, spilling its contents on the floor.";
		else
		    message = "$p flickers and vanishes.";
	    else
		message = "$p crumbles into dust.";
	    break;
	}

	if ( obj->carried_by != NULL )
	{
	    if (IS_NPC(obj->carried_by) 
	    &&  obj->carried_by->pIndexData->pShop != NULL)
		obj->carried_by->silver += obj->cost/5;
	    else
	    {
	    	act( message, obj->carried_by, obj, NULL, TO_ROOM );
	    	act( message, obj->carried_by, obj, NULL, TO_CHAR );
		if ( obj->wear_loc == WEAR_FLOAT)
		    act(message,obj->carried_by,obj,NULL,TO_ROOM);
	    }
	}
	else if ( obj->in_room != NULL
	&&      ( rch = obj->in_room->people ) != NULL )
	{
	    if (! (obj->in_obj && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
	           && !CAN_WEAR(obj->in_obj,ITEM_TAKE)))
	    {
	    	act( message, rch, obj, NULL, TO_ROOM );
	    	act( message, rch, obj, NULL, TO_CHAR );
	    }
	}

        /* save the contents of PC corpses, floating containers, and books */
        if ( (obj->item_type == ITEM_CORPSE_PC 
           || obj->wear_loc == WEAR_FLOAT
           || obj->item_type == ITEM_BOOK)
	&&  obj->contains)
	{
     	    OBJ_DATA *t_obj, *next_obj;

	    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
	    {
		next_obj = t_obj->next_content;
		obj_from_obj(t_obj);

		if (obj->in_obj) /* in another object */
		    obj_to_obj(t_obj,obj->in_obj);

		else if (obj->carried_by)  /* carried */
		    if (obj->wear_loc == WEAR_FLOAT)
			if (obj->carried_by->in_room == NULL)
			    extract_obj(t_obj);
			else
			    obj_to_room(t_obj,obj->carried_by->in_room);
		    else
		    	obj_to_char(t_obj,obj->carried_by);

		else if (obj->in_room == NULL)  /* destroy it */
		    extract_obj(t_obj);

		else /* to a room */
		    obj_to_room(t_obj,obj->in_room);
	    }
	}

	extract_obj( obj );
    }

    return;
}



/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update( void )
{
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;

    for ( wch = char_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;
	if ( IS_NPC(wch)
	||   wch->level >= LEVEL_IMMORTAL
	||   wch->in_room == NULL 
	||   wch->in_room->area->empty)
	    continue;

	for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
	{
	    int count;

	    ch_next	= ch->next_in_room;

	    if ( !IS_NPC(ch)
	    ||   !IS_SET(ch->act_bits, ACT_AGGRESSIVE)
	    ||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
	    ||   IS_AFFECTED(ch,AFF_CALM)
	    ||   ch->fighting != NULL
	    ||   IS_AFFECTED(ch, AFF_CHARM)
	    ||   !IS_AWAKE(ch)
	    ||   ( IS_SET(ch->act_bits, ACT_WIMPY) && IS_AWAKE(wch) )
	    ||   !can_see( ch, wch ) 
	    ||   number_bits(1) == 0)
		continue;

	    /*
	     * Ok we have a 'wch' player character and a 'ch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the room,
	     *   giving each 'vch' an equal chance of selection.
	     */
	    count	= 0;
	    victim	= NULL;
	    for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	    {
		vch_next = vch->next_in_room;

		if ( !IS_NPC(vch)
		&&   vch->level < LEVEL_IMMORTAL
		&&   ch->level >= vch->level - 5 
		&&   ( !IS_SET(ch->act_bits, ACT_WIMPY) || !IS_AWAKE(vch) )
		&&   ( !IS_AFFECTED2(vch, AFF_GHOST))
		&&   can_see( ch, vch ) 
                &&   ( !IS_SET(vch->in_room->room_flags,ROOM_SEMISAFE) 
                     || in_fightlag(vch) ) )
		{
		    if ( number_range( 0, count ) == 0 )
			victim = vch;
		    count++;
		}
	    }

	    if ( victim == NULL )
		continue;

	    multi_hit( ch, victim, TYPE_UNDEFINED );
	}
    }

    return;
}



/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler( void )
{
    static  int     pulse_area;
    static  int     pulse_mobile;
    static  int     pulse_violence;
    static  int     pulse_point;
    static  int	    pulse_music;
    static  int     pulse_weather;
    static  int     pulse_bank;
    static  int     pulse_second;
    CHAR_DATA* ch;


    if ( --pulse_second <= 0 )
    {
        pulse_second = PULSE_PER_SECOND;
        
        if (( current_time % 3600 ) == 0 )
            stats_update( );

        for ( ch = char_list; ch != NULL; ch = ch->next )
        {
            /* blood pool updates every hour for minions */
            if ( IS_SET( ch->shiftbits, TEMP_VAMP ) 
            &&   ch->max_blood < 500 
            &&   !IS_SET( ch->shiftbits, SHIFT_FIXEDBP )
            &&   !IS_SET( ch->comm, COMM_AFK ) 
            &&   !IS_SET( ch->act_bits, PLR_LINKDEAD ) 
            &&   ((ch->played + (int)(current_time - ch->logon)) % 3600 == 0 ) )
            {
                ch->max_blood += 1;
                send_to_char( 
             "The passage of time has made your blood magic grow stronger.\n\r",
                    ch );
            }
            if ( !IS_NPC( ch ) )
            { 
                /* tick down cleanse timers for Covenance */
                if ( ch->pcdata->cleanse_timer > 0 )
                    ch->pcdata->cleanse_timer--;
                /* call fightlag check to update fightlag timer, %f in prompt */
                in_fightlag(ch);
            }
        }
    }

    if ( --pulse_area     <= 0 )
    {
	pulse_area	= PULSE_AREA;
	/* number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 ); */
	area_update	( );
    }

    if ( --pulse_music	  <= 0 )
    {
	pulse_music	= PULSE_MUSIC;
	song_update();
    }

    if ( --pulse_mobile   <= 0 )
    {
	pulse_mobile	= PULSE_MOBILE;
	mobile_update	( );
    }

    if ( --pulse_violence <= 0 )
    {
	pulse_violence	= PULSE_VIOLENCE;
	violence_update	( );
    }

    if ( --pulse_point    <= 0 )
    {
        wiznet("TICK.",NULL,NULL,WIZ_TICKS,0,0);
/*      pulse_point     = PULSE_TICK; */
        pulse_point     = number_range( PULSE_TICK - global_config_tick_jitter, 
                                       PULSE_TICK + global_config_tick_jitter );

        time_update     ( );
        char_update	( );
        obj_update	( );
		quad_update( );
    }

    if ( --pulse_weather  <= 0 )
    {
	pulse_weather   = PULSE_TICK * 4;
        weather_update  ( );
        mod_who         ( );
    }

    if( --pulse_bank <= 0 )
    {
	pulse_bank = PULSE_TICK * 4;
	bank_update();
    }

    aggr_update( );
    mpdelay_update( );
    tail_chain( );
    return;
}

void mpdelay_update( void )
{
    MPDELAY_DATA *mpdnow_list, *mpd, *prev;

    if ( mpdelay_list == NULL )
	return;

    mpdnow_list = mpdelay_list;
    mpdelay_list = NULL;

    prev = NULL;

    mpd = mpdnow_list;
    while ( mpd != NULL )
    {
        if ( mpd->execute == FALSE )
	{
	    if ( prev != NULL )
		prev->next = mpd->next;
	    else
		mpdnow_list = mpd->next;

	    mpd->next = mpdelay_list;
	    mpdelay_list = mpd;
	    mpd->execute = TRUE;

            mpd = mpdnow_list;
	    continue;
        }
	if ( mpd->mob != NULL && mpd->mob->valid 
	&& ( ( mpd->ch != NULL && mpd->ch->valid )
           ||  mpd->ch == NULL ) )
	    program_flow( mpd->pvnum, mpd->code, mpd->mob, mpd->ch, 
                          mpd->arg1, mpd->arg2 );
        prev = mpd;
        mpd = mpd->next;
    }

    for ( mpd = mpdnow_list; mpd != NULL; mpd = mpd->next )
    {
        free_mpdelay(mpd);
    }
}

void stats_update( void )
{
    int visi_imms = 0;
    int wizi_imms = 0;
    int newbies = 0;
    int unapproved = 0;
    int flagged_nb = 0;
    int guilded = 0;
    int unguilded = 0;
    int mortals = 0;
    DESCRIPTOR_DATA* d;
    char* strtime;
    FILE* stat_log;
    extern long queststatus;

    d = descriptor_list;
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *ch;
        if ( (ch = d->character) != NULL )
        {
            if ( IS_IMMORTAL( ch ) )
            {
                if ( ch->invis_level >= LEVEL_HERO 
                ||   ch->incog_level >= LEVEL_HERO )
                    wizi_imms++;
                else
                    visi_imms++;
            }
            else
            {
                if ( ch->level < 11 )
                    newbies++;
                if ( IS_SET( ch->act_bits, PLR_NEWBIE ) )
                    flagged_nb++;
                if ( IS_SET( ch->act_bits, ACT_NOAPPROVE ) )
                    unapproved++;
                if ( is_guild( ch ) )
                    guilded++;
                else
                    unguilded++;
                mortals++;
            }
        }
    }
    max_on = UMAX(max_on, visi_imms + mortals);

    if ( (stat_log = fopen("stats.log", "a")) != NULL )
    {
        strtime = ctime( &current_time );
        strtime[ strlen( strtime ) - 1 ] = '\0';

        fprintf( stat_log, "%s %d %d %d %d %d %d %d %d %s \n",
            strtime,
            wizi_imms, visi_imms,
            newbies, flagged_nb, unapproved,
            guilded, unguilded, mortals,
            ( IS_SET( queststatus, QUEST_INPROGRESS ) ? "T" : "F" ) );
        fclose( stat_log );
    }
    return;
}

/*
 * Check to see if pinch should wake someone up
 */
#define FAIL_MIRRORS    1
#define FAIL_SKULLS     2
void check_pinch( CHAR_DATA *ch )
{
    AFFECT_DATA *pinch, *sleep, *mirrors;
    int dam, imagehit, fail = 0;

/*
 * can't pinch someone who's not sleeping; can't pinch someone who's not 
 * affected by pinch, and can't pinch someone who's been sapped 
 */
    if ( ch->position != POS_SLEEPING
    || (  (pinch = affect_find( ch->affected, gsn_pinch )) == NULL
       && (pinch = affect_find( ch->affected, gsn_haunt )) == NULL )
    ||   affect_find( ch->affected, gsn_sap ) != NULL )
        return;
/*** removed 7/24/2012 ***
 *  if ((sleep = affect_find(ch->affected, gsn_sleep)) == NULL )
 *      sleep = affect_find( ch->affected, gsn_sap );
 */

    /*
     *  Mirrors should prevent pinch from working sometimes
     */
    if ( IS_AFFECTED2(ch, AFF_MIRROR)
    &&   (mirrors = affect_find(ch->affected, gsn_mirror)) 
    &&   (imagehit = number_range(0, mirrors->modifier)) )
    {
        fail = FAIL_MIRRORS;
        if ( --mirrors->modifier == 0) affect_strip( ch, gsn_mirror );
    }
    else if ( IS_AFFECTED3( ch , AFF_SKULLS )
    &&        (mirrors = affect_find(ch->affected, gsn_skulls))
    &&        (imagehit = number_range(0,mirrors->modifier)) )
    {
        fail = FAIL_SKULLS;
        if ( --mirrors->modifier == 0 ) affect_strip( ch, gsn_skulls );
    }

    if ( pinch->type == gsn_haunt )
    {
        dam = DAM_NEGATIVE;
        if ( fail == FAIL_MIRRORS )
        {
            send_to_char( "Spirits haunt your dreams!\n\r",
                ch );
            act( "The spirits haunting $n consume one of $s images.", 
                ch, NULL, NULL, TO_ROOM);
        }
        else if ( fail == FAIL_SKULLS )
        {
            send_to_char( "Spirits haunt your dreams!\n\r",
                ch );
            act( "The spirits haunting $n consume one of $s spectral skulls.", 
                ch, NULL, NULL, TO_ROOM);
        }
        else
        {
            send_to_char( "Spirits haunt your dreams and jostle you awake!\n\r",
                ch );
            act( "The spirits haunting $n jostle $m awake.", 
                ch, NULL, NULL, TO_ROOM);
        }
    }
    else
    {
        dam = DAM_ENERGY;
        send_to_char( 
            "You feel an ethereal thumb and finger pinch you HARD!\n\r", ch );

        if ( fail == FAIL_MIRRORS )
        {
            send_to_char("The pinch wasn't strong enough to wake you.\n\r", ch);
            act( "A ghostly hand appears and pinches an image of $n.", 
                ch, NULL, NULL, TO_ROOM );
        }
        else if ( fail == FAIL_SKULLS )
        {
            send_to_char("The pinch wasn't strong enough to wake you.\n\r", ch);
            act( 
              "A ghostly hand appears and pinches one of $n's spectral skulls.",
                ch, NULL, NULL, TO_ROOM );
        }
        else
        {
            send_to_char( "That pinch woke you up!\n\r", ch );
            act( "A ghostly hand appears and pinches $n to wake $m.", 
                ch, NULL, NULL, TO_ROOM );
        }
    }

    if( !fail )
      {
	if ( (sleep = affect_find(ch->affected, gsn_sleep)) != NULL ) 
	  affect_strip( ch, sleep->type );

        raw_damage( ch, ch, pinch->level/10+1, pinch->type, dam, FALSE, FALSE );
      }

    return;
}
#undef FAIL_MIRRORS
#undef FAIL_SKULLS

/*
 * forces a tick but does NOT delay the next tick.  use with extreme caution!
 */
void do_screwtick( CHAR_DATA *ch, char *argument )
{
    wiznet("TICK.",NULL,NULL,WIZ_TICKS,0,0);
    time_update ( );
    char_update	( );
    obj_update	( );
    return;
}

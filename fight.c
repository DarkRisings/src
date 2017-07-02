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
#include "tables.h"
#include "lookup.h"
#include "recycle.h"

extern int quad;
extern long queststatus;
extern void spell_wyrm_venom args( ( int sn, int level, CHAR_DATA *ch, 
        void *vo, int target) );

/* command procedures needed */
DECLARE_DO_FUN(do_armthrow      );
DECLARE_DO_FUN(do_harm_touch      );
DECLARE_DO_FUN(do_entangle      );
DECLARE_DO_FUN(do_backstab	);
DECLARE_DO_FUN(do_burning_palm   );
DECLARE_DO_FUN(do_crane_kick	);
DECLARE_DO_FUN(do_emote		);
DECLARE_DO_FUN(do_battle	);
DECLARE_DO_FUN(do_bash		);
DECLARE_DO_FUN(do_pounce        );
DECLARE_DO_FUN(do_trip		);
DECLARE_DO_FUN(do_hamstring     );
DECLARE_DO_FUN(do_dirt		);
DECLARE_DO_FUN(do_flee		);
DECLARE_DO_FUN(do_kick		);
DECLARE_DO_FUN(do_disarm	);
DECLARE_DO_FUN(do_dislodge	);
DECLARE_DO_FUN(do_get		);
DECLARE_DO_FUN(do_rage          );
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_vital_strike  );
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_sacrifice	);
DECLARE_DO_FUN(do_wing          );
DECLARE_DO_FUN(do_fury          );
DECLARE_DO_FUN(do_throw         );
/*guild skill*/
DECLARE_DO_FUN(do_sap           );

/*
 * Local functions.
 */
void	check_assist	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge	args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool vuln ) );
bool    check_dragon_punch   args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt ) );
bool    check_aura_pain     args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam )  );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_palm_block args(( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_parry	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_shadows	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_shield_block     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    dam_message 	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
                            int dt, bool immune ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	make_corpse	args( ( CHAR_DATA *ch ) );
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void    mob_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	unconscious_kill	args( ( CHAR_DATA *victim ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	dislodge	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int     target_damage   args( ( int low, int high ) );
bool    break_web	args( ( CHAR_DATA *ch ) );

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next	= ch->next;
/*      if ( !IS_NPC(ch)
        &&   ch->position != POS_FIGHTING 
        &&   ch->fightlag > 0 )
        {
             ch->fightlag -= 3;
             if ( (current_time - ch->pcdata->lastpk) < FIGHT_LAG )
                 ch->pcdata->lastpk = current_time - FIGHT_LAG + ch->fightlag;
        } */

        /* Latelog checking. */
	if (!IS_NPC(ch) 
        &&  ch->pcdata->latelog /* If !latelog, no point in proceeding */
        &&  (current_time - ch->logon) > 179 )
            ch->pcdata->latelog = FALSE;

/* Throw somnambulance here so that it fires off-tick and on average once every
   ten minutes. */
#define NARCOLEPSY_CHANCE       ( 600 / PULSE_VIOLENCE * PULSE_PER_SECOND )
        if ( IS_AFFECTED3( ch, AFF3_NARCOLEPSY )
        &&   ch->fighting == NULL
        &&   IS_AWAKE( ch )
        &&   !IS_AFFECTED( ch, AFF_SLEEP )
        &&   number_range(1,NARCOLEPSY_CHANCE) == 1 )
        {
            AFFECT_DATA af;
            int somnambulance;

            af.where     = TO_AFFECTS;
            af.type      = gsn_sleep;
            af.level     = ch->level;
            af.duration  = 3;
            af.location  = APPLY_NONE;
            af.modifier  = 0;
            af.bitvector = AFF_SLEEP;
            affect_join( ch, &af, FALSE );

            send_to_char(
              "The dream world creeps into your thoughts and vision.\n\r", ch );
	    act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
	    ch->position = POS_SLEEPING;
            
            if ( (somnambulance = skill_lookup("somnambulance")) != NO_SKILL
            &&   is_affected(ch, somnambulance) )
            {
                char buf[MAX_STRING_LENGTH];
                sprintf(buf, "%s fell asleep due to somnambulance in room %d.",
                    IS_NPC( ch ) ? ch->short_descr : ch->name, 
                    ( ch->in_room ) ? ch->in_room->vnum : -1 );
                wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
            }
            continue;
        }
#undef NARCOLEPSY_CHANCE

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
	    multi_hit( ch, victim, TYPE_UNDEFINED );
	else
	    stop_fighting( ch, FALSE );


	if ( ( victim = ch->fighting ) == NULL )
	    continue;

	/*
	 * Fun for the whole family!
	 */
	check_assist(ch,victim);

	if ( IS_NPC( ch ) )
	{
	    if ( HAS_TRIGGER( ch, TRIG_FIGHT ) )
		mp_percent_trigger( ch, victim, NULL, NULL, TRIG_FIGHT );
	    if ( HAS_TRIGGER( ch, TRIG_HPCNT ) )
		mp_hprct_trigger( ch, victim );
            if ( ch->in_room == NULL ) return;
	}
    }

    /* The list has to be looped through a second time to disable all
       effects that last one round of combat */
    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
        /* this works a lot faster with a good branch predictor */
        if ( ch->kicktimer > 0 ) 
            ch->kicktimer--;
        if ( ch->wstimer > 0 )
            ch->wstimer--;
    }

    return;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;
	
	if (IS_AWAKE(rch) && rch->fighting == NULL)
	{

	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(rch) 
	    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
	    &&  rch->level + 6 > victim->level)
	    {
		do_emote(rch,"screams and attacks!");
		multi_hit(rch,victim,TYPE_UNDEFINED);
		continue;
	    }

	    /* PCs next */
	    if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM))
	    {
		if ( ( (!IS_NPC(rch) && IS_SET(rch->act_bits,PLR_AUTOASSIST))
		||     IS_AFFECTED(rch,AFF_CHARM)) 
		&&   is_same_group(ch,rch) 
		&&   !(IS_AFFECTED2(rch,AFF_GHOST))
		&&   !is_safe(rch, victim))
		    multi_hit (rch,victim,TYPE_UNDEFINED);
		
		continue;
	    }
  	
	    /* now check the NPC cases */
	    
 	    if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
	
	    {
		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

		||   (IS_NPC(rch) && rch->group && rch->group == ch->group)

		||   (IS_NPC(rch) && rch->race == ch->race 
		   && IS_SET(rch->off_flags,ASSIST_RACE))

		||   (rch->pIndexData == ch->pIndexData 
		   && IS_SET(rch->off_flags,ASSIST_VNUM)))

	   	{
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;
		
		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next)
		    {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
			&&  number_range(0,number) == 0)
			{
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL)
		    {
			do_emote(rch,"screams and attacks!");
			multi_hit(rch,target,TYPE_UNDEFINED);
		    }
		}	
	    }
	}
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
  OBJ_DATA *wield, *wield2;
  int chance, dw;
  extern void do_stand(CHAR_DATA *ch, char *argument);

  /* decrement the wait */
  if (ch->desc == NULL){
    ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);
    if ((ch->wait == 0) && (ch->position == POS_SPRAWLED))
    {
      do_stand(ch,"");
      ch->position = POS_FIGHTING;
    }
  }

  if (ch->desc == NULL)
    ch->daze = UMAX(0,ch->daze - PULSE_VIOLENCE); 


  /* no attacks for stunnies -- just a check */
  if (ch->position < POS_SPRAWLED)
    return;

  if (IS_NPC(ch))
    {
      mob_hit(ch,victim,dt);
      return;
    }

  one_hit( ch, victim, dt );

  if (ch->fighting != victim)
    return;

/* Haste and vampire night haste don't stack, but also do not conflict
   with each other if both on at the same time. */
  if ( IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch, AFF_NIGHTHASTE) )
    one_hit(ch,victim,dt);

  if (IS_AFFECTED2(ch,AFF_CELERITY))
    one_hit(ch,victim,dt);

  if ( ch->fighting != victim || dt == gsn_backstab )
    return;

  chance = get_skill(ch,gsn_second_attack)/2;

  if (IS_AFFECTED2(ch,AFF_FEAR))
    chance /= 3;

  if (IS_AFFECTED(ch,AFF_BERSERK))
    chance *= 11/6;

  if (IS_AFFECTED(ch,AFF_SLOW))
    chance /= 3;

  if ( number_percent( ) < chance )
    {
      one_hit( ch, victim, dt );
      check_improve(ch,gsn_second_attack,TRUE,5);
      if ( ch->fighting != victim )
	return;
    }

  chance = get_skill(ch,gsn_third_attack)/3;

  if (IS_AFFECTED2(ch,AFF_FEAR))
    chance *= 0;

  if (IS_AFFECTED(ch,AFF_BERSERK))
    chance *=15/6;

  if (IS_AFFECTED(ch,AFF_SLOW))
    chance = 0;

  if ( number_percent( ) < chance )
    {
      one_hit( ch, victim, dt );
      check_improve(ch,gsn_third_attack,TRUE,6);
      if ( ch->fighting != victim )
	return;
    }

  chance = get_skill(ch,gsn_fourth_attack)/5;
  if (IS_AFFECTED2(ch,AFF_FEAR))
    chance *= 0;

  if (IS_AFFECTED(ch,AFF_BERSERK))
    chance *= 17/6;

  if (IS_AFFECTED(ch,AFF_SLOW))
    chance = 0;

  if ( number_percent( ) < chance )
    {
      one_hit( ch, victim, dt );
      check_improve(ch,gsn_fourth_attack,TRUE,6);
      if ( ch->fighting != victim )
	return;
    } 

  chance = get_skill(ch,gsn_fifth_attack)/10;
  if (IS_AFFECTED2(ch,AFF_FEAR))
    chance *= 0;

  if (IS_AFFECTED(ch,AFF_BERSERK))
    chance *= 17/6;

  if (IS_AFFECTED(ch,AFF_SLOW))
    chance = 0;

  if ( number_percent( ) < chance )
    {
      one_hit( ch, victim, dt );
      check_improve(ch,gsn_fifth_attack,TRUE,6);
      if ( ch->fighting != victim )
	return;
    }

  if ( (wield = get_eq_char(ch,WEAR_HOLD)) != NULL
  &&   wield->item_type == ITEM_WEAPON
  &&   wield->value[0]  != WEAPON_BOW )
  {
      dw = get_skill(ch,gsn_dualwield);
      chance = dw/2;
      if (IS_AFFECTED2(ch,AFF_FEAR))
        chance /= 3;
      if (IS_AFFECTED(ch,AFF_BERSERK))
        chance *= 11/6;
      if (IS_AFFECTED(ch,AFF_SLOW))
        chance /= 3;

      if ( number_percent( ) < chance )
      {
	  wield2 = get_eq_char(ch,WEAR_WIELD);
	  wield->wear_loc = WEAR_WIELD;
	  
	  if (wield2) 
	    wield2->wear_loc = WEAR_HOLD;
	  
	  one_hit( ch, victim, dt );

	  if (wield2) 
	    wield2->wear_loc = WEAR_WIELD;
	  
	  wield->wear_loc = WEAR_HOLD;
	  check_improve(ch,gsn_dualwield,TRUE,6);
	  
	}

        if ( IS_AFFECTED(ch, AFF_HASTE)
        &&  !IS_AFFECTED(ch, AFF_SLOW)
        &&  !IS_AFFECTED2(ch, AFF_FEAR) )
        {
          chance = dw/3;
          if (IS_AFFECTED(ch,AFF_BERSERK))
            chance *= 15/6;

          if ( number_percent( ) < chance )
          {
              wield2 = get_eq_char(ch,WEAR_WIELD);
              wield->wear_loc = WEAR_WIELD;
	  
              if (wield2) 
              wield2->wear_loc = WEAR_HOLD;
  
              one_hit( ch, victim, dt );

              if (wield2) 
              wield2->wear_loc = WEAR_WIELD;
  
              wield->wear_loc = WEAR_HOLD;
              check_improve(ch,gsn_dualwield,TRUE,6);
	  }
        }

    }
    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int chance,number;
    CHAR_DATA *vch, *vch_next;

    one_hit(ch,victim,dt);

    if (ch->fighting != victim)
	return;

    /* Area attack -- BALLS nasty! */
 
    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next;
	    if ((vch != victim && vch->fighting == ch))
		one_hit(ch,vch,dt);
	}
    }

    if (IS_AFFECTED(ch,AFF_HASTE) 
    ||  (IS_SET(ch->off_flags,OFF_FAST) && !IS_AFFECTED(ch,AFF_SLOW)))
	one_hit(ch,victim,dt);

    if (ch->fighting != victim || dt == gsn_backstab)
	return;

    chance = get_skill(ch,gsn_second_attack)/2;
    if (IS_AFFECTED2(ch,AFF_FEAR))
	chance *= .75;


    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
	chance /= 2;

    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt);
	if (ch->fighting != victim)
	    return;
    }

    chance = get_skill(ch,gsn_third_attack)/4;
    if (IS_AFFECTED2(ch,AFF_FEAR))
	chance *= .5;


    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
	chance = 0;

    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt);
	if (ch->fighting != victim)
	    return;
    } 

    chance = get_skill(ch,gsn_fourth_attack)/4;
    if (IS_AFFECTED2(ch,AFF_FEAR))
	chance *= .75;


    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
        chance = 0;
    if (number_percent() < chance)
    {
        one_hit(ch,victim,dt);
        if (ch->fighting != victim)
            return;
    }
    /* oh boy!  Fun stuff! */

    if (ch->wait > 0)
	return;

    number = number_range(0,2);

    if (number == 1 && IS_SET(ch->act_bits,ACT_MAGE))
    {
	/*  { mob_cast_mage(ch,victim); return; } */ ;
    }

    if (number == 2 && IS_SET(ch->act_bits,ACT_CLERIC))
    {	
	/* { mob_cast_cleric(ch,victim); return; } */ ;
    }

    /* now for the skills */

    number = number_range(0,8);

    switch(number) 
    {
    case (0) :
	if (IS_SET(ch->off_flags,OFF_BASH))
	    do_bash(ch,"");
            WAIT_STATE(ch,PULSE_VIOLENCE*2+9);
	break;

    case (1) :
	if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
	    do_battle(ch,"");
	break;


    case (2) :
	if (IS_SET(ch->off_flags,OFF_DISARM) 
	|| (get_weapon_sn(ch) != gsn_hand_to_hand 
	&& (IS_SET(ch->act_bits,ACT_WARRIOR)
   	||  IS_SET(ch->act_bits,ACT_THIEF))))
	    do_disarm(ch,"");
	break;

    case (3) :
	if (IS_SET(ch->off_flags,OFF_KICK))
	    do_kick(ch,"");
	break;

    case (4) :
	if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
	    do_dirt(ch,"");
	break;

    case (5) :
	if (IS_SET(ch->off_flags,OFF_TAIL))
	{
	    /* do_tail(ch,"") */ ;
	}
	break; 

    case (6) :
	if (IS_SET(ch->off_flags,OFF_TRIP))
	    do_trip(ch,"");
	break;

    case (7) :
	if (IS_SET(ch->off_flags,OFF_CRUSH))
	{
	    /* do_crush(ch,"") */ ;
	}
	break;
    case (8) :
	if (IS_SET(ch->off_flags,OFF_BACKSTAB))
	{
	    do_backstab(ch,"");
	}
    }
}

/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
  OBJ_DATA *wield;
  int victim_ac;
  int thac0;
  int thac0_00;
  int thac0_32;
  int dam;
  int diceroll;
  int sn, skill, bonus;
  int dam_type;
  int result;

  sn = -1;


  /* just in case */
  if (victim == ch || ch == NULL || victim == NULL)
    return;

  /*
   * Can't beat a dead char!
   * Guard against weird room-leavings.
   */
  if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
    return;

  /*
   * Figure out the type of damage message.
   */
  wield = get_eq_char( ch, WEAR_WIELD );

  if ( dt == TYPE_UNDEFINED )
    {
      dt = TYPE_HIT;

      if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	{
	  dt += wield->value[3];
	}
      else
	{
	  dt += ch->dam_type;
	}
    }

  if (dt < TYPE_HIT)
    {
      if (wield != NULL)
	{
	  dam_type = attack_table[wield->value[3]].damage;
	}
      else
	{
	  dam_type = attack_table[ch->dam_type].damage;
	}
    }
  else
    {
      dam_type = attack_table[dt - TYPE_HIT].damage;
    }

  if (dam_type == -1)
    dam_type = DAM_BASH;

  /* get the weapon skill */
  sn = get_weapon_sn(ch);
  skill = 20 + get_weapon_skill(ch,sn);

  /*
   * Calculate to-hit-armor-class-0 versus armor.
   */
  if ( IS_NPC(ch) )
    {
      thac0_00 = 20;
      thac0_32 = -4;   /* as good as a thief */ 
      if (IS_SET(ch->act_bits,ACT_WARRIOR))
	thac0_32 = -10;
      else if (IS_SET(ch->act_bits,ACT_THIEF))
	thac0_32 = -4;
      else if (IS_SET(ch->act_bits,ACT_CLERIC))
	thac0_32 = 2;
      else if (IS_SET(ch->act_bits,ACT_MAGE))
	thac0_32 = 6;
    }
  else
    {
      thac0_00 = class_table[ch->class].thac0_00;
      thac0_32 = class_table[ch->class].thac0_32;
    }
  thac0  = interpolate( ch->level, thac0_00, thac0_32 );

  if (thac0 < 0)
    thac0 = thac0/2;

  if (thac0 < -5)
    thac0 = -5 + (thac0 + 5) / 2;

  thac0 -= GET_HITROLL(ch) * skill/100;
  thac0 += 5 * (100 - skill) / 100;

  if (dt == gsn_backstab)
    {
      /* this look like a bonus for having backstab under 100% -- proposed fix below
       * thac0 -= 10 * (100 - get_skill(ch,gsn_backstab));
       */

      thac0 += ( 100 - get_skill( ch, gsn_backstab ) ) / 10;

    }

  switch(dam_type)
    {
    case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/8;	break;
    case(DAM_BASH):	 victim_ac = GET_AC(victim,AC_BASH)/8;		break;
    case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/8;	break;
    default:	 victim_ac = GET_AC(victim,AC_EXOTIC)/8;	break;
    }; 
	
  if (victim_ac < -15)
    victim_ac = (victim_ac + 15) / 2 - 15;
     
  if ( !can_see( ch, victim ) )
    victim_ac -= 4;

  if ( victim->position < POS_FIGHTING)
    victim_ac += 4;
 
 /* if (victim->position < POS_RESTING)
    victim_ac += 6;                      */

  /*
   * The moment of excitement!  See if there's a HIT!
   */
  while ( ( diceroll = number_bits( 5 ) ) >= 20 )
    ;

  if ( diceroll == 0
       || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
      /* Miss. */
      damage( ch, victim, 0, dt, dam_type, TRUE );
      tail_chain( );
      return;
    }

  /*
   * Hit.
   * Calc damage.
   */
  if ( IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
    if (!ch->pIndexData->new_format)
      {
	dam = number_range( ch->level / 2, ch->level * 3 / 2 );
	if ( wield != NULL )
	  dam += dam / 2;
      }
    else
      dam = dice(ch->damagedice[DICE_NUMBER],ch->damagedice[DICE_TYPE]);
	
  else
    {
      if (sn != -1)
	check_improve(ch,sn,TRUE,5);
      if ( wield != NULL )
	{
	  if (wield->pIndexData->new_format)
	    dam = dice(wield->value[1],wield->value[2]) * skill/100;
	  else
	    dam = number_range( wield->value[1] * skill/100, 
				wield->value[2] * skill/100);

	  if (get_eq_char(ch,WEAR_SHIELD) == NULL)  /* no shield = more */
	    dam = dam * 11/10;

	  /* sharpness! */
	  if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
	    {
	      int percent;

	      if ((percent = number_percent()) <= (skill / 8))
		dam = 2 * dam + (dam * 2 * percent / 100);
	    }
	}
      else
	dam = number_range( 1 + 4 * skill/100, 2 * ch->level/3 * skill/100);
    }

    /*
     * Bonuses.
     */
    if ( (bonus = get_skill(ch,gsn_enhanced_damage)) > 0 )
    {
        diceroll = number_percent();
        if (diceroll <= bonus)
        {
            check_improve(ch,gsn_enhanced_damage,TRUE,6);
            dam += 2 * ( dam * diceroll/300);
        }
    }

    /*
     * Critical strike
     */
    if ( (bonus = get_skill( ch, gsn_critical_strike )) > 0 )
    {
        int chance;
        if ( number_percent() <= bonus )
        {
            if ( ch->class == 3 )         /* warriors */
                chance = 15;
            else
                chance = 5;
	    if ( number_percent() <= chance )
            {
                check_improve( ch, gsn_critical_strike, TRUE, 2 );
                dam = ( 3 * dam ) / 2;
            }
	}
    }

    if ( !IS_AWAKE(victim) )
        dam *= 2;
    else if (victim->position < POS_FIGHTING)
        dam = dam * 3 / 2;

    if ( dt == gsn_backstab && wield != NULL) 
    {
      if ( wield->value[0] != 2 )
	dam *= 2 + (ch->level / 10); 
      else 
	dam *= 2 + (ch->level / 8);
    }

    dam += GET_DAMROLL(ch) * UMIN(100,skill) /100;

    if ( dam <= 0 )
        dam = 1;

    if ( IS_AFFECTED3( victim, AFF3_AURA_PAIN )
    &&   check_aura_pain( ch, victim, dam ) )
        result = 0;
    else if ( check_dragon_punch( ch, victim, dam, dt ) )
        result = 0;
    else
        result = damage( ch, victim, dam, dt, dam_type, TRUE );

    /* but do we have a funky weapon? */
    if ( result && wield != NULL )
    {
        int dam;

        if ( ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_POISON) )
        {
            int level;
            AFFECT_DATA *poison, af;

            if ((poison = affect_find(wield->affected,gsn_poison)) == NULL)
                level = wield->level;
            else
                level = poison->level;
	
            if (!saves_spell(level / 2, NULL, victim, DAM_POISON)) 
            {
	        send_to_char("You feel poison coursing through your veins.\n\r",
                    victim);
                act("$n is poisoned by the venom on $p.", victim, wield, NULL,
                    TO_ROOM);

                af.where     = TO_AFFECTS;
                af.type      = gsn_poison;
                af.level     = level * 3/4;
                af.duration  = level / 2;
                af.location  = APPLY_STR;
                af.modifier  = -1;
                af.bitvector = AFF_POISON;
                affect_join( victim, &af, TRUE );
            }

            /* weaken the poison if it's temporary */
            if (poison != NULL)
            {
                poison->level = UMAX(0,poison->level - 2);
                poison->duration = UMAX(0,poison->duration - 1);
                if (poison->level == 0 || poison->duration == 0)
                    act("The poison on $p has worn off.",ch,wield,NULL,TO_CHAR);
            }
        }


        if ( ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC) )
        {
            dam = number_range(1, wield->level / 5 + 1);
            act("$p draws life from $n.",victim,wield,NULL,TO_ROOM);
            act("You feel $p drawing your life away.",
                victim,wield,NULL,TO_CHAR);
            damage_old(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
            ch->hit += dam/2;
        }

	    if ( ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_ANGELIC) )
        {
            dam = number_range(1, wield->level / 5 + 1);
            act("$p draws energy from $n.",victim,wield,NULL,TO_ROOM);
            act("You feel $p drawing your energy away.",
                victim,wield,NULL,TO_CHAR);
            damage_old(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
            ch->mana += dam/2;
        }
		
        if ( ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FLAMING) )
        {
            dam = number_range(1,wield->level / 4 + 1);
            act("$n is burned by $p.",victim,wield,NULL,TO_ROOM);
            act("$p sears your flesh.",victim,wield,NULL,TO_CHAR);
            fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
            damage(ch,victim,dam,0,DAM_FIRE,FALSE);
        }

        if ( ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FROST) )
        {
            dam = number_range(1,wield->level / 6 + 2);
            act("$p freezes $n.",victim,wield,NULL,TO_ROOM);
            act("The cold touch of $p surrounds you with ice.",
            victim,wield,NULL,TO_CHAR);
            cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
            damage(ch,victim,dam,0,DAM_COLD,FALSE);
        }

        if ( ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING) )
        {
            dam = number_range(1,wield->level/5 + 2);
            act("$n is struck by lightning from $p.",victim,wield,NULL,TO_ROOM);
            act("You are shocked by $p.",victim,wield,NULL,TO_CHAR);
            shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
            damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE);
        }

        if ( ch->fighting == victim
        &&   global_config_vorpalon
        &&   !IS_NPC(ch)
        &&   class_table[ch->class].vorpalspell != NULL
        &&   IS_WEAPON_STAT(wield,WEAPON_VORPAL)
        &&   ch->wstimer == 0 )
        {
            obj_cast_spell( *class_table[ch->class].vorpalspell, 
                wield->level, ch, victim, wield );
            if ( IS_NPC(ch) )
                ch->wstimer = 2;
            else
                ch->wstimer = class_table[ch->class].wstimer; 
        }
    }
    tail_chain( );
    return;
}

/*
 * raw_damage() applies damage to characters without benefit of most modifiers.
 *  if lethal = FALSE, fatal damage will only KO regardless of if ch is mob
 *  if lethal = TRUE, fatal damage will kill regardless of if ch is a mob or pc
 */
bool raw_damage( CHAR_DATA* ch, CHAR_DATA* victim, int dam, int dt, int dam_type, bool show, bool lethal )
{
  char buf[ MAX_STRING_LENGTH ] = "";
  OBJ_DATA* corpse = NULL;
  bool immune = FALSE;

  if ( victim->position == POS_DEAD || victim->in_room == NULL )
    return FALSE;

  if (!IS_NPC(ch) && !IS_NPC(victim))
    {
      if( ch != victim )
	{
	  if(ch->pcdata->latelog)
	    {
	      sprintf(buf,"Latelog attack by %s on %s\n",ch->name,victim->name);
	      ch->pcdata->latelog = FALSE;
	      wiznet(buf,NULL,NULL,WIZ_DEATHS,0,0);
	      log_string(buf);
	    }

	  if(victim->pcdata->latelog) victim->pcdata->latelog = FALSE;	
	}

    }
  if (IS_AFFECTED2(victim,AFF_GHOST))
    return FALSE;

  if ( dam > 1200 && dt >= TYPE_HIT)
    {
      bug( "Damage: %d: more than 1200 points!", dam );
      dam = 1200;

      if (!IS_IMMORTAL(ch))
	{
	  OBJ_DATA *obj;
	  obj = get_eq_char( ch, WEAR_WIELD );
	  send_to_char("You really shouldn't cheat.\n\r",ch);
	  if (obj != NULL)
	    extract_obj(obj);
	}

    }

  if ( victim != ch )
    {
      if ( is_safe( ch, victim ) )
	return FALSE;

      check_killer( ch, victim );

      /*
       * everything past here is considered fightlag on aggressor!
       */
      if ( !IS_NPC(ch) )
      {
          ch->pcdata->lastfight = current_time;
          if ( !IS_NPC(victim) )
          {
              ch->pcdata->lastpk = current_time;
              victim->pcdata->lastpk = current_time;
          }
      }
      if ( !IS_NPC(victim) )
          victim->pcdata->lastfight = current_time;

      /* attacking a pet causes the pet to fade */
      if ( ( ch->pet && ch->pet == victim ) )
        {
/*          stop_fighting( ch, FALSE );*/
          nuke_pets( ch );
          return FALSE;
        }
      else if ( IS_NPC(victim) 
           &&   IS_AFFECTED2( victim, AFF2_SUMMONEDPET ) 
           &&   victim->master == ch )
      {
      /* Attacking one multipet will only cause that specific pet to fade */
/*         stop_fighting( ch, FALSE );  <-- Not sure that this is necessary. */
         stop_follower( victim );
         return FALSE;
      }

      if ( victim->position >= POS_SLEEPING )
      {
          if ( victim->fighting == NULL )
              set_fighting( victim, ch );
          if ( ch->fighting == NULL )
          {
	      set_fighting( ch, victim );
              if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_KILL ) )
                  mp_percent_trigger( victim, ch, NULL, NULL, TRIG_KILL );
              if ( victim->in_room == NULL ) return FALSE;
          }
      }

      if ( victim->master == ch )
          if ( stop_follower( victim ) )
              return FALSE;
    }

  if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
      affect_strip( ch, gsn_invis );
      affect_strip( ch, gsn_mass_invis );
      REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
      act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    }

  if ( IS_AFFECTED(ch, AFF_SNEAK) )
    {
      affect_strip( ch, gsn_sneak );
      affect_strip( ch, gsn_shadow_form);
      affect_strip(ch, gsn_shadow);
      REMOVE_BIT( ch->affected_by, AFF_SNEAK );
      act( "$n steps out of the shadows.", ch, NULL, NULL, TO_ROOM );
    }

    /*
     * everything past here puts the victim in fightlag
     */
    if ( !IS_NPC(victim) )
    {
        victim->pcdata->lastfight = current_time;
        if ( !IS_NPC(ch) )
            victim->pcdata->lastpk = current_time;
    }


  switch(check_immune(victim,dam_type))
    {
    case(IS_IMMUNE):
      immune = TRUE;
      dam = 0;
      break;
    case(IS_RESISTANT):	
      dam -= dam/4;
      break;
    case(IS_VULNERABLE):
      dam += dam/2;
      break;
    }

  if (show)
    dam_message( ch, victim, dam, dt, immune );

  if (dam == 0)
    return FALSE;

  check_quest_score( ch, victim, dam, dam_type );
  victim->hit -= dam;

  if( !IS_NPC( victim ) &&
      victim->level >= LEVEL_IMMORTAL &&
      victim->hit < 1 )
    {
      victim->hit = 1;
    }

  update_pos( victim );

/*
    if ( (!IS_NPC(ch) 
       || (IS_AFFECTED2(ch,AFF2_SUMMONEDPET) || IS_SET(ch->act_bits,ACT_PET)))
    &&  victim->position == POS_DEAD
    &&  !IS_NPC( victim )
    &&  !lethal )
    {
        victim->position = POS_UNCONSCIOUS;
    }
 */
    if ( victim->position == POS_DEAD 
    &&   !IS_NPC(victim) 
    &&   !lethal )
        victim->position = POS_UNCONSCIOUS;

    switch( victim->position )
    {
        case POS_UNCONSCIOUS:
            act( "$n is unconscious.", victim, NULL, NULL, TO_ROOM );
            send_to_char( "You are unconscious.\n\r", victim );
            break;

      case POS_DEAD:
          act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
          send_to_char( "You have been KILLED!!\n\r\n\r", victim );
          break;

      default:
          if ( dam > victim->max_hit / 4 )
              send_to_char( "That really did HURT!\n\r", victim );
          if ( victim->hit < victim->max_hit / 4 )
              send_to_char( "You sure are BLEEDING!\n\r", victim );
          break;
    }

  /*
   * Sleep spells and extremely wounded folks.
   */
  if ( !IS_AWAKE(victim) )
    stop_fighting( victim, FALSE );

  /*
   * Payoff for killing things.
   */
  if ( victim->position == POS_DEAD || victim->position == POS_UNCONSCIOUS )
  {
      group_gain( ch, victim );

      if ( !IS_NPC(victim) )
      {
	  sprintf( log_buf, "%s killed by %s at %d",
		   victim->name,
		   (IS_NPC(ch) ? ch->short_descr : ch->name),
		   ch->in_room->vnum );
	  log_string( log_buf );

	  /*
	   * Dying penalty:
	   * 2/3 way back to previous level.
	   */
	  if ( victim->exp > exp_per_level(victim,victim->pcdata->points) 
	       * victim->level )
	    gain_exp( victim, (2 * (exp_per_level(victim,victim->pcdata->points)
				    * victim->level - victim->exp)/3) + 50 );
      }

      sprintf( log_buf, "%s got toasted by %s at %s [room %d]",
	       (IS_NPC(victim) ? victim->short_descr : victim->name),
	       (IS_NPC(ch) ? ch->short_descr : ch->name),
	       ch->in_room->name, ch->in_room->vnum);
 
      if (IS_NPC(victim))
	wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
      else
	wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

      /*
       * Death trigger
       */
      if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_DEATH) )
	{
	  victim->position = POS_STANDING;
	  mp_percent_trigger( victim, ch, NULL, NULL, TRIG_DEATH );
          if ( victim->in_room == NULL ) return FALSE;
	}

/*
 * removed 4/2/2011 gkl
 *
      if (IS_NPC(victim) 
      || (IS_NPC(ch) && !IS_SET(ch->act_bits,ACT_PET) && !IS_AFFECTED2(ch,AFF2_SUMMONEDPET)))
	raw_kill( victim );
      else {
	unconscious_kill( victim );
      }
 *
 *  and replaced with
 */
      if ( lethal || IS_NPC(victim) )
          raw_kill( victim);
      else
          unconscious_kill( victim );
/*
 *  end of new unconscious code
 */


      /* RT new auto commands */

      if (!IS_NPC(ch)
	  &&  (corpse = get_obj_list(ch,"corpse",ch->in_room->contents, TRUE)) != NULL
	  &&  corpse->item_type == ITEM_CORPSE_NPC && can_see_obj(ch,corpse))
	{
	  OBJ_DATA *coins;

	  corpse = get_obj_list( ch, "corpse", ch->in_room->contents, TRUE ); 

	  if ( IS_SET(ch->act_bits, PLR_AUTOLOOT) &&
	       corpse && corpse->contains) /* exists and not empty */
	    do_get( ch, "all corpse" );

	  if (IS_SET(ch->act_bits,PLR_AUTOGOLD) &&
	      corpse && corpse->contains  && /* exists and not empty */
	      !IS_SET(ch->act_bits,PLR_AUTOLOOT))
	    if ((coins = get_obj_list(ch,"gcash",corpse->contains, TRUE))
		!= NULL)
	      do_get(ch, "all.gcash corpse");
            
	  if ( IS_SET(ch->act_bits, PLR_AUTOSAC) )
	    {
	      if ( IS_SET(ch->act_bits,PLR_AUTOLOOT) && corpse && corpse->contains)
	 	return TRUE;  /* leave if corpse has treasure */
	      else
	 	do_sacrifice( ch, "corpse" );
	    }
	}

      return TRUE;
    }

  if ( victim == ch )
    return TRUE;

  /*
   * Take care of link dead people.
   */
  if ( !IS_NPC(victim) && victim->desc == NULL )
    {
      if ( number_range( 0, victim->wait ) == 0 )
	{
	  do_recall( victim, "" );
	  return TRUE;
	}
    }

  /*
   * Wimp out?
   */
  if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
      if ( ( IS_SET(victim->act_bits, ACT_WIMPY) && number_bits( 2 ) == 0
	     &&   victim->hit < victim->max_hit / 5) 
	   ||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
		  &&     victim->master->in_room != victim->in_room ) )
	do_flee( victim, "" );
    }

  if ( !IS_NPC(victim)
       &&   victim->hit > 0
       &&   victim->hit <= victim->wimpy
       &&   victim->wait < PULSE_VIOLENCE / 2 )
    do_flee( victim, "" );

  tail_chain( );
  return TRUE;
}


/*
 * Inflict damage from a hit.
 */
bool damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type, bool show)
{
  AFFECT_DATA *paf;
  short imagehit;
  short aura;
  OBJ_DATA *corpse;
  bool immune, vuln;
  char buf[MAX_STRING_LENGTH];

  if ( victim->in_room == NULL             /* 7/26/12 - trap recursive damage */
  ||   victim->position == POS_DEAD 
  ||   (!IS_NPC(victim) && victim->position == POS_UNCONSCIOUS) )
    return FALSE;

  /* Catch all lateloggers */
  if (!IS_NPC(ch) && !IS_NPC(victim))
    {
      if( ch != victim )
	{
	  if(ch->pcdata->latelog)
	    {
	      sprintf(buf,"Latelog attack by %s on %s\n",ch->name,victim->name);
	      ch->pcdata->latelog = FALSE;
	      wiznet(buf,NULL,NULL,WIZ_DEATHS,0,0);
	      log_string(buf);
	    }
	  if(victim->pcdata->latelog) victim->pcdata->latelog = FALSE;	
	}
    }
  if (IS_AFFECTED2(victim,AFF_GHOST))
    return FALSE;

  if( IS_AFFECTED3( victim, AFF_FROG ) )
    {
      send_to_char( "Your transmogrified state protects you from harm!", victim );
      send_to_char( "Your attack cannot seem to harm the frog!", ch );
      return FALSE;
    }

  /*
   * Stop up any residual loopholes.
   */
  if ( dam > 1200 && dt >= TYPE_HIT)
    {
//    bug( "Damage: %d: more than 1200 points!", dam );
      dam = 1200;
      if (!IS_IMMORTAL(ch))
	{
	  OBJ_DATA *obj;
	  obj = get_eq_char( ch, WEAR_WIELD );
	  send_to_char("You really shouldn't cheat.\n\r",ch);
	  if (obj != NULL)
	    extract_obj(obj);
	}

    }

    
  /* damage reduction */
  if ( dam > 35)
      dam = (dam - 35)/2 + 35;

  if ( dam > 80)
      dam = (dam - 80)/2 + 80;
   
  if ( victim != ch )
    {
      /*
       * Certain attacks are forbidden.
       * Most other attacks are returned.
       */
      if ( is_safe( ch, victim ) )
	return FALSE;
      check_killer( ch, victim );

      /*
       * everything past here is considered fightlag on aggressor!
       */
      if ( !IS_NPC(ch) )
      {
          ch->pcdata->lastfight = current_time;
          if ( !IS_NPC(victim) )
          {
              ch->pcdata->lastpk = current_time;
              victim->pcdata->lastpk = current_time;
          }
      }
      if ( !IS_NPC(victim) )
          victim->pcdata->lastfight = current_time;

      /* attacking a pet causes the pet to fade */
      if ( ( ch->pet && ch->pet == victim ) )
	{
/*	  stop_fighting( ch, FALSE );*/
          nuke_pets( ch );
	  return FALSE;
	}
      else if ( IS_NPC(victim) 
           &&   IS_AFFECTED2( victim, AFF2_SUMMONEDPET ) 
           &&   victim->master == ch )
      {
      /* Attacking one multipet will only cause that specific pet to fade */
/*         stop_fighting( ch, FALSE );*/
         stop_follower( victim );
         return FALSE;
      }

      if ( victim->position >= POS_SLEEPING )
      {
          if ( victim->fighting == NULL )
              set_fighting( victim, ch );
          if ( ch->fighting == NULL )
          {
	      set_fighting( ch, victim );
              if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_KILL ) )
                  mp_percent_trigger( victim, ch, NULL, NULL, TRIG_KILL );
              if ( victim->in_room == NULL ) return FALSE;
          }
      }

/*
 *   When a person attacks his own follower, make the victim stop following.
 *   If the victim is in fact a pet, stop_follower will extract the pet and
 *   the rest of this damage function should not execute (because the victim
 *   no longer exists!) 6/17/06 gkl 
 */
      if ( victim->master == ch )
	if ( stop_follower( victim ) )
            return FALSE;
    }

  /*
   * Inviso attacks ... not.
   */
  if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
      affect_strip( ch, gsn_invis );
      affect_strip( ch, gsn_mass_invis );
      REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
      act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    }

  if ( IS_AFFECTED(ch, AFF_SNEAK) )
    {
      affect_strip( ch, gsn_sneak );
      affect_strip( ch, gsn_shadow_form);
      affect_strip(ch, gsn_shadow);
      REMOVE_BIT( ch->affected_by, AFF_SNEAK );
      act( "$n steps out of the shadows.", ch, NULL, NULL, TO_ROOM );
    }

  /*
   * Damage modifiers.
   */

  if ( dam > 1 && !IS_NPC(victim) 
       &&   victim->pcdata->condition[COND_DRUNK]  > 10 )
    {
      dam = 9 * dam / 10;
    }


  if ( dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
      dam /= 2;
    }

  if ( dam > 1 && IS_AFFECTED(victim, AFF_MAGICWARD) )
    {
      dam = dam * 3/4;
    }


  immune = FALSE;

  /*
   * Check for parry, and dodge.
   */
    if ( dt >= TYPE_HIT && ch != victim)
    {
        vuln = FALSE;
/*      In pvp, vulnerability to the damnoun translates to dodge penalty */
        if ( !IS_NPC( victim ) && !IS_NPC( ch ) 
        &&   check_immune(victim, dam_type) == IS_VULNERABLE )
            vuln = TRUE;
       
        if ( check_parry( ch, victim ) )
            return FALSE;
        if ( check_palm_block( ch, victim ) )
            return FALSE;
        if ( check_dodge( ch, victim, vuln ) )
            return FALSE;
        if ( check_shield_block(ch,victim))
            return FALSE;
        if ( check_shadows(ch,victim))
            return FALSE; 
    }


/* 4/6/08 gkl
   It was decided that the combat systems in the game have diverged to the
   point where the dynamics of mobkilling (higher-level, immobile, high-hp)
   is so different from player killing that the two should really be split
   into separate damage modes.  As of this change, the handling of resists
   and vulns in and out of pk are now different.  Take note of this! */
    if ( !IS_NPC( victim ) 
    &&   !IS_NPC( ch ) 
    &&   dt >= TYPE_HIT )
    { /* PK melee thread */
        int immunity;
        immunity = check_immune(victim,dam_type);
        switch( immunity )
        {
            case(IS_IMMUNE):
              immune = TRUE;
              dam = 0;
              break;
            case(IS_RESISTANT):	
              dam -= dam/4;
              break;
            case(IS_VULNERABLE):
              if ( !IS_NPC(ch) && IS_SET(ch->comm,COMM_VERBOSE) )
                  send_to_char( "New vuln melee hit!\n\r", ch );
              dam += 17 * dam / 100;
              break;
        }
    }
    else
    { /* Non-pk thread (mob/mob or pc/mob) - same as before */
        int immunity;
        immunity = check_immune(victim,dam_type);
        switch( immunity )
        {
            case(IS_IMMUNE):
                immune = TRUE;
                dam = 0;
                break;
            case(IS_RESISTANT):	
                dam -= dam/4;
                break;
            case(IS_VULNERABLE):
                dam += dam/4;
                break;
        }
    }


    if (show)
        dam_message( ch, victim, dam, dt, immune );

    if (dam == 0)
        return FALSE;

    /*
     * new mirror image routines - 4/4/2012 gkl 
     */
    if ( IS_AFFECTED2(victim, AFF_MIRROR)
    &&  (paf = affect_find(victim->affected, gsn_mirror))
    &&  (imagehit = number_range(0,paf->modifier)) 
    &&  dt != gsn_backstab
    &&  dt != gsn_blister
    &&  dt != gsn_decay
    &&  dt != gsn_atrophy
    &&  dt != gsn_wilt )
    {
        act("$n hits an image of $N",ch,NULL,victim,TO_NOTVICT);
        act("$n DESTROYS an image of you!",ch,NULL,victim,TO_VICT);
        act("You DESTROY an image of $n!",victim,NULL,ch,TO_VICT);
        if ( --paf->modifier == 0 )
            affect_strip(victim,gsn_mirror);
        return FALSE;
    }
    if ( IS_AFFECTED3(victim, AFF_SKULLS)
    &&  (paf = affect_find(victim->affected, gsn_skulls))
    &&  (imagehit = number_range(0,paf->modifier))
    &&  dt != gsn_backstab
    &&  dt != gsn_blister
    &&  dt != gsn_decay
    &&  dt != gsn_atrophy
    &&  dt != gsn_wilt )
    {
        act("$n hits a spectral skull of $N",ch,NULL,victim,TO_NOTVICT);
        act("$n DESTROYS one of your spectral skulls!",ch,NULL,victim,TO_VICT);
        act("You DESTROY one of $n's spectral skulls!",victim,NULL,ch,TO_VICT);
        if ( --paf->modifier == 0 ) 
            affect_strip(victim,gsn_skulls);
        return FALSE;
    }

  if (dam > 1 && (IS_AFFECTED2(victim, AFF_WILDAURA)))
  {
    aura =  number_range(1,5);
    switch(aura){
    case 1:
      send_to_char("Your aura shimmers black.\n\r",victim);
      dam *= 1.5;
      break;
    case 2:
      send_to_char("Your aura shimmers grey.\n\r",victim);
      dam *= 1.25;
      break;
    case 3:
      send_to_char("Your aura does nothing.\n\r",victim);
      dam *= 1;
      break;
    case 4:
      send_to_char("Your aura shimmers yellow.\n\r",victim);
      dam = 3 * dam / 4;
      break;
    case 5:
    default:
      send_to_char("Your aura shimmers a brilliant white.\n\r",victim);
      dam /= 4;
      break;
    }
  }
  /*
   * Hurt the victim.
   * Inform the victim of his new state.
   */


  check_quest_score( ch, victim, dam, dam_type );

  victim->hit -= dam;

  if ( !IS_NPC(victim)
       &&   victim->level >= LEVEL_IMMORTAL
       &&   victim->hit < 1 )
    victim->hit = 1;
  update_pos( victim );

   if( (!IS_NPC( ch ) || ( IS_AFFECTED2(ch,AFF2_SUMMONEDPET) || IS_SET(ch->act_bits,ACT_PET) ) )
   &&  victim->position == POS_DEAD
   &&  !IS_NPC( victim ) )
      victim->position = POS_UNCONSCIOUS;

  switch( victim->position )
    {
    case POS_UNCONSCIOUS:
      act( "$n is unconscious.",
	   victim, NULL, NULL, TO_ROOM );
      send_to_char( 
		   "You are unconscious.\n\r",
		   victim );
      break;

    case POS_DEAD:
      act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
      send_to_char( "You have been KILLED!!\n\r\n\r", victim );
      break;

    default:
      if ( dam > victim->max_hit / 4 )
	send_to_char( "That really did HURT!\n\r", victim );
      if ( victim->hit < victim->max_hit / 4 )
	send_to_char( "You sure are BLEEDING!\n\r", victim );
      break;
    }

  /*
   * Sleep spells and extremely wounded folks.
   */
  if ( !IS_AWAKE(victim) )
    stop_fighting( victim, FALSE );

  /*
   * Payoff for killing things.
   */
  if ( victim->position == POS_DEAD )
    {
      group_gain( ch, victim );

      if ( !IS_NPC(victim) )
	{
	  sprintf( log_buf, "%s killed by %s at %d",
		   victim->name,
		   (IS_NPC(ch) ? ch->short_descr : ch->name),
		   ch->in_room->vnum );
	  log_string( log_buf );

	  /*
	   * Dying penalty:
	   * 2/3 way back to previous level.
	   */
	  if ( victim->exp > exp_per_level(victim,victim->pcdata->points) 
	       * victim->level )
	    gain_exp( victim, (2 * (exp_per_level(victim,victim->pcdata->points)
				    * victim->level - victim->exp)/3) + 50 );
	}

      sprintf( log_buf, "%s got toasted by %s at %s [room %d]",
	       (IS_NPC(victim) ? victim->short_descr : victim->name),
	       (IS_NPC(ch) ? ch->short_descr : ch->name),
	       ch->in_room->name, ch->in_room->vnum);
 
      if (IS_NPC(victim))
	wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
      else
	wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

      /*
       * Death trigger
       */
      if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_DEATH) )
      {
          victim->position = POS_STANDING;
          mp_percent_trigger( victim, ch, NULL, NULL, TRIG_DEATH );
          if ( victim->in_room == NULL ) return FALSE;
      }

      if (IS_NPC(victim)
      || (IS_NPC(ch) && !IS_SET(ch->act_bits,ACT_PET) && !IS_AFFECTED2(ch,AFF2_SUMMONEDPET)))
	raw_kill( victim );
      else {
	unconscious_kill( victim );
      }


      /* RT new auto commands */

      if (!IS_NPC(ch)
	  &&  (corpse = get_obj_list(ch,"corpse",ch->in_room->contents, TRUE)) != NULL
	  &&  corpse->item_type == ITEM_CORPSE_NPC && can_see_obj(ch,corpse))
	{
	  OBJ_DATA *coins;

	  corpse = get_obj_list( ch, "corpse", ch->in_room->contents, TRUE ); 

	  if ( IS_SET(ch->act_bits, PLR_AUTOLOOT) &&
	       corpse && corpse->contains) /* exists and not empty */
	    do_get( ch, "all corpse" );

	  if (IS_SET(ch->act_bits,PLR_AUTOGOLD) &&
	      corpse && corpse->contains  && /* exists and not empty */
	      !IS_SET(ch->act_bits,PLR_AUTOLOOT))
	    if ((coins = get_obj_list(ch,"gcash",corpse->contains, TRUE))
		!= NULL)
	      do_get(ch, "all.gcash corpse");
            
	  if ( IS_SET(ch->act_bits, PLR_AUTOSAC) )
	    {
	      if ( IS_SET(ch->act_bits,PLR_AUTOLOOT) && corpse && corpse->contains)
	 	return TRUE;  /* leave if corpse has treasure */
	      else
	 	do_sacrifice( ch, "corpse" );
	    }
	}

      return TRUE;
    }

  if ( victim == ch )
    return TRUE;

  /*
   * Take care of link dead people.
   */
  if ( !IS_NPC(victim) && victim->desc == NULL )
    {
      if ( number_range( 0, victim->wait ) == 0 )
	{
	  do_recall( victim, "" );
	  return TRUE;
	}
    }

  /*
   * Wimp out?
   */
  if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
      if ( ( IS_SET(victim->act_bits, ACT_WIMPY) && number_bits( 2 ) == 0
	     &&   victim->hit < victim->max_hit / 5) 
	   ||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
		  &&     victim->master->in_room != victim->in_room ) )
	do_flee( victim, "" );
    }

  if ( !IS_NPC(victim)
       &&   victim->hit > 0
       &&   victim->hit <= victim->wimpy
       &&   victim->wait < PULSE_VIOLENCE / 2 )
    do_flee( victim, "" );

  tail_chain( );
  return TRUE;
}






/*
 * Inflict damage from a hit.
 */
bool damage_old( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int 
dam_type, bool show ) 
{
  return damage( ch, victim, dam, dt, dam_type, show );
}

bool is_safe_verb(CHAR_DATA *ch, CHAR_DATA *victim, bool verb)
{
  char buf[ MAX_INPUT_LENGTH ] = "";

  if (victim->in_room == NULL || ch->in_room == NULL)
	return TRUE;

    if (victim->position == POS_UNCONSCIOUS)
    {
	if ( verb && victim != ch ) 
            send_to_char("You can murder them but beware the consequences!\n\r",
                ch);
	return TRUE;
    }

    if (IS_AFFECTED2(victim, AFF_GHOST))
        return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_AFFECTED2(victim, AFF_WANTED))
        return FALSE;

    if (IS_AFFECTED3( victim, AFF_VEIL ))
	return FALSE;

    /* safe room? */
    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
    {
        if ( verb ) send_to_char("Not in this room.\n\r",ch);
        return TRUE;
    }

    if ( IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL )
	return FALSE;

    /* killing mobiles */
    if (IS_NPC(victim))
    {
	if (victim->pIndexData->pShop != NULL)
	{
	    if ( verb ) send_to_char("The shopkeeper wouldn't like that.\n\r",ch);
	    return TRUE;
	}

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act_bits,ACT_TRAIN)
	||  IS_SET(victim->act_bits,ACT_PRACTICE)
	||  IS_SET(victim->act_bits,ACT_IS_HEALER)
	||  IS_SET(victim->act_bits,ACT_IS_CHANGER)
        ||  IS_SET(victim->act_bits,ACT_NOKILL))
	{
	  if( verb )
	    {
	      if( !IS_NPC( ch ) )
		{
		  if( ch->pcdata->god == 0 )
		    {
		      send_to_char( "I don't think you should approve of that.\n\r", ch );
		    }
		  else
		    {
		      sprintf( buf, "I don't think %s would approve.\n\r", god_table[ ch->pcdata->god ].descr );
		      send_to_char( buf, ch );
		    }
		}
	    }

	    return TRUE;
	}
    }
    /* killing players */
    else
    {
        /* semi-safe room? */
        if ( IS_SET(victim->in_room->room_flags,ROOM_SEMISAFE) 
        &&  (current_time - victim->pcdata->lastfight) >= FIGHT_LAG )
        {
            if ( verb ) send_to_char("Not in this room.\n\r",ch);
            return TRUE;
        }

	/* player doing the killing */
        if (!IS_NPC(ch) && victim->level < 11)
        {
            if ( verb ) 
                send_to_char("Pick on someone other than a newling.\n\r",ch);
            return TRUE;
        }
    }
    return FALSE;
}
 
bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
    return is_safe_verb( ch, victim, TRUE );
}

bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    if (victim->in_room == NULL || ch->in_room == NULL)
        return TRUE;

    if (victim->position == POS_UNCONSCIOUS){
	send_to_char("You can murder them but beware the consequences!\n\r",ch);
	return TRUE;
	}

    if (victim == ch && area)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL && !area)
	return FALSE;

    /* safe room? */
    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
        return TRUE;

    /* killing mobiles */
    if (IS_NPC(victim))
    {
	if (victim->pIndexData->pShop != NULL)
	    return TRUE;

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act_bits,ACT_TRAIN)
	||  IS_SET(victim->act_bits,ACT_PRACTICE)
	||  IS_SET(victim->act_bits,ACT_IS_HEALER)
	||  IS_SET(victim->act_bits,ACT_IS_CHANGER)
        ||  IS_SET(victim->act_bits,ACT_NOKILL) )
	    return TRUE;

	if (!IS_NPC(ch))
	{
	    /* no pets */
	    if (IS_SET(victim->act_bits,ACT_PET))
	   	return TRUE;
	}
    }
    /* killing players */
    else
    {
	if (area && IS_IMMORTAL(victim))
	    return TRUE;

        /* only PCs can ever be safe in semi-safe */
        if (IS_SET(victim->in_room->room_flags,ROOM_SEMISAFE) 
        && (current_time - victim->pcdata->lastfight) >= FIGHT_LAG )
            return TRUE;

        if ( !IS_NPC(ch) && victim->level < 11 && area)
            return TRUE;

    }
    return FALSE;
}
/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    AFFECT_DATA *paf;

    if ( IS_AFFECTED3( ch, AFF_VEIL ) 
    &&  (paf = affect_find( ch->affected, gsn_veil )) != NULL )
    {
	affect_remove( ch, paf );
    }

    return;
}



/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance, rroll;
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *wield;

    if ( !IS_AWAKE(victim)
    ||   victim->kicktimer > 0 )
        return FALSE;

    chance = get_skill(victim,gsn_parry) / 2;

    if (chance < 4) return FALSE;
  
    if ( (wield = get_eq_char( victim, WEAR_WIELD )) == NULL )
    {
        if (IS_NPC(victim))
            chance /= 2;
        else
            return FALSE;
    }
  
    if (wield != NULL) 
    {
        if( ( IS_WEAPON_STAT( wield, WEAPON_TWO_HANDS ) ) &&
            ( wield->value[ 0 ] == WEAPON_STAFF ||
	      wield->value[ 0 ] == WEAPON_POLEARM ||
	      wield->value[ 0 ] == WEAPON_SPEAR ) )
             chance += 10;
    }

    wield = NULL;
    if ( get_skill(ch,gsn_dualwield) > 1 )
    {
        wield = get_eq_char(ch,WEAR_HOLD);
        if (wield != NULL && wield->item_type == ITEM_WEAPON)  
            chance += 20;
    }
  
    if ( !can_see(victim,ch) 
    &&  ( IS_NPC(victim) || victim->race != race_lookup("drow") ) )
        chance /= 2;
  
    chance += victim->level - ch->level;

/* Hitroll modifier to parry - added 3/30/08 gkl */
    rroll = number_percent(); 
    if ( !IS_NPC(ch) && !IS_NPC(victim) 
    &&   !class_table[ch->class].fMana )
    {
        if ( IS_SET( ch->comm, COMM_VERBOSE )
        &&   get_trust(ch) > LEVEL_ADMIN )
        {
            sprintf(buf, "%s's parry chance before hitroll bonus: %d.  After bonus: %d.  Roll: %d.\n\r",
                PERS( victim, ch ), chance, chance - GET_HITROLL(ch) / CONFIG_HR_PARRY_SCALAR, rroll );
            send_to_char( buf, ch );
        }

        chance -= GET_HITROLL(ch) / CONFIG_HR_PARRY_SCALAR;
    }
/* ** */
  
/*  if ( number_percent( ) >= chance ) */
    if ( rroll >= chance )
        return FALSE;

    if (!IS_SET(victim->act_bits, PLR_BATTLEBRIEF))
      act( "You parry $n's attack.",  ch, NULL, victim, TO_VICT    );
  
    if (!IS_SET(ch->act_bits, PLR_BATTLEBRIEF))
      act( "$N parries your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_parry,TRUE,6);
    return TRUE;
}

bool check_palm_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;
    OBJ_DATA *wield;
    
    if ( !IS_AWAKE(victim) 
    ||   victim->kicktimer > 0 )
      return FALSE;

    if ( (chance = get_skill(victim,gsn_palm_block)) < 5 )
      return FALSE;

    if ((wield = get_eq_char(victim, WEAR_WIELD)) != NULL) 
      if (wield->value[0] != WEAPON_HAND)
	return FALSE;

    chance /= 2;

    if (!can_see(victim, ch))
      chance /= 2;

    chance += get_curr_stat(victim, STAT_DEX) - get_curr_stat(ch, STAT_DEX);

    if ( number_percent( ) >= chance + victim->level - ch->level )
      return FALSE;

    if (!IS_SET(victim->act_bits,PLR_BATTLEBRIEF))
      act( "You block $n's attack with your palm.", ch, NULL, victim, TO_VICT );

    if (!IS_SET(ch->act_bits,PLR_BATTLEBRIEF))
      act( "$N blocks your attack with $S palm.", ch, NULL, victim, TO_CHAR );
    check_improve(victim,gsn_palm_block,TRUE,6);
    return TRUE;
}


/*
 * Check for shield block.
 */
bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) 
    ||   victim->kicktimer > 0 )
        return FALSE;

    chance = get_skill(victim,gsn_shield_block) / 5 + 3;

    if (chance < 5)
       return FALSE;


    if ( get_eq_char( victim, WEAR_SHIELD ) == NULL )
        return FALSE;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    if (!IS_SET(victim->act_bits,PLR_BATTLEBRIEF))
      act( "You block $n's attack with your shield.",  ch, NULL, victim, 
TO_VICT    );

    if (!IS_SET(ch->act_bits,PLR_BATTLEBRIEF))
      act( "$N blocks your attack with a shield.", ch, NULL, victim, 
TO_CHAR    );
    check_improve(victim,gsn_shield_block,TRUE,6);
    return TRUE;
}


/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim, bool vuln )
{
    int chance, rroll;
    char buf[MAX_STRING_LENGTH]; 

    if ( !IS_AWAKE(victim) 
    ||   victim->kicktimer > 0 )
	return FALSE;

    chance = get_skill(victim,gsn_dodge) / 2;

    if( is_affected( victim, gsn_bolas ) )
      return FALSE;

/*  This was removed to offset the dramatic penalty to parry that
    was introduced when a bug was fixed.  4/6/08 gkl
    if (!can_see(victim,ch))
	chance /= 2;
*/

    chance += victim->level - ch->level;

/* Hitroll modifier to dodge - added 3/30/08 gkl */
    rroll = number_percent();
    if ( !IS_NPC(ch) && !IS_NPC(victim) 
    &&   !class_table[ch->class].fMana )
    {
        if ( IS_SET( ch->comm, COMM_VERBOSE )
        &&   get_trust(ch) > LEVEL_ADMIN )
        {
            sprintf(buf, "%s's dodge chance before hitroll bonus: %d.  After bonus: %d.  Roll: %d.\n\r",
                PERS( victim, ch ), chance, chance - GET_HITROLL(ch) / CONFIG_HR_DODGE_SCALAR, rroll );
            send_to_char( buf, ch );
        }

        chance -= GET_HITROLL(ch) / CONFIG_HR_DODGE_SCALAR;
    }

/*  For PK only */
    if ( vuln )
    {
        if ( IS_SET( ch->comm, COMM_VERBOSE )
        &&   get_trust(ch) > LEVEL_ADMIN )
        {
            sprintf(buf, "%s's dodge chance penalized due to vuln: %d%%-15%%=%d%%.  Roll: %d.\n\r",
                PERS( victim, ch ), chance, chance - 15, rroll );
            send_to_char( buf, ch );
        }
        chance -= 15;
    }
 
/*  if ( number_percent( ) >= chance ) */
    if ( rroll >= chance )
        return FALSE;

    if (!IS_SET(victim->act_bits,PLR_BATTLEBRIEF))
      act( "You dodge $n's attack.", ch, NULL, victim, TO_VICT    );

    if (!IS_SET(ch->act_bits,PLR_BATTLEBRIEF))
      act( "$N dodges your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_dodge,TRUE,6);
    return TRUE;
}



bool check_shadows( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    chance = number_range (1,100);

    if (IS_AFFECTED2(victim,AFF_SHADOWS) && (chance < 31))
     {
       if ( !IS_AWAKE(victim) )
        return FALSE;

       if (!can_see(victim,ch))
	chance /= 2;

       act( "You step into a shadow and evade $n's attack.", ch, NULL, victim, TO_VICT );
       act( "$N steps into a shadow and evades your attack.", ch, NULL, victim, TO_CHAR );
       
       return TRUE;

     }
   else
     return FALSE;
}

bool check_dragon_punch( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt)
{
        int chance;
        int dam_type;
        OBJ_DATA *wield;
 
        if (   /* ( get_eq_char(victim, WEAR_WIELD) == NULL ) ||*/
                ( !IS_AWAKE(victim) ) ||
                ( !can_see(victim,ch) ) ||
                ( get_skill(victim,gsn_dragon_punch) < 1 )
           )
           return FALSE;
 
        wield = get_eq_char(victim,WEAR_WIELD);
 
        chance = get_skill(ch,gsn_dragon_punch) / 6;
        chance += ( victim->level - ch->level ) / 2;
        chance += (get_curr_stat(ch,STAT_DEX));   
        chance += (get_curr_stat(ch,STAT_STR)) / 2;   
        chance -= get_curr_stat(victim,STAT_DEX);
    if ( number_percent( ) >= chance )
        return FALSE;
 
        dt = gsn_dragon_punch;
 
    if ( dt == TYPE_UNDEFINED )
    {
        dt = TYPE_HIT;
        if ( wield != NULL && wield->item_type == ITEM_WEAPON )
            dt += wield->value[3];
        else
            dt += ch->dam_type;
    }
 
    if (dt < TYPE_HIT)
        if (wield != NULL)
            dam_type = attack_table[wield->value[3]].damage;
        else
            dam_type = attack_table[ch->dam_type].damage;
    else
        dam_type = attack_table[dt - TYPE_HIT].damage;
 
    if (dam_type == -1)
        dam_type = DAM_BASH;
 
    act( "You block $n's attack and counter with a dragon punch!", ch, NULL, victim, TO_VICT    );
    act( "$N blocks your attack and dragon punches you!", ch, NULL, victim, TO_CHAR );
 
    damage(victim,ch,dam, gsn_dragon_punch , dam_type ,TRUE ); 
    check_improve(victim,gsn_dragon_punch,TRUE,6);
 
    return TRUE;
}

bool check_aura_pain( CHAR_DATA *ch, CHAR_DATA *victim, int dam )
{
    int chance = 16;
    int dam_type;
    OBJ_DATA *wield;
 
    /* assume that check_aura_pain is not called unless ch is affected */
    if ( !IS_AWAKE(victim) || !can_see(victim,ch) )
        return FALSE;
 
    chance += ( victim->level - ch->level ) / 2;
    chance += get_curr_stat(ch,STAT_INT);   
    chance += get_curr_stat(ch,STAT_WIS) / 2;   
    chance -= get_curr_stat(victim,STAT_INT);

    if ( number_percent( ) >= chance )
        return FALSE;
 
    if ( (wield = get_eq_char(victim,WEAR_WIELD)) )
        dam_type = attack_table[wield->value[3]].damage;
    else
        dam_type = attack_table[ch->dam_type].damage;
 
    if ( dam_type == -1 )
        dam_type = DAM_NEGATIVE;
 
    act( "Your aura of pain turns $n's attack back upon $mself!", 
        ch, NULL, victim, TO_VICT    );
    act( "$N's aura of pain turns your attack back upon you!", 
        ch, NULL, victim, TO_CHAR );
 
    damage( victim, ch, dam, gsn_aura_pain, dam_type ,TRUE ); 
 
    return TRUE;
}


void update_pos( CHAR_DATA *victim )
{

/*
    if ( victim->hit > 0 )
    {
    	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_STANDING;
	return;
    }
*/

    if ( IS_NPC(victim) && victim->hit < 1 )
    {
	victim->position = POS_DEAD;
	return;
    }

    if ( victim->hit <= 0 )
    {
	victim->position = POS_DEAD;
	return;
    }

    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->fighting != NULL )
    {
        sprintf(buf, "set_fighting: already fighting ch=%s victim=%s ch->fighting=%s",
           ch->name, victim->name,ch->fighting->name );
	bug( buf, 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
    {
	affect_strip( ch, gsn_sleep );
/*      affect_strip( ch, gsn_sap ); */
    }

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    /*
     * For NPCs
     */
    if ( IS_NPC(ch) )
    {
        for ( fch = char_list; fch != NULL; fch = fch->next )
        {
            if ( fch == ch || ( fBoth && fch->fighting == ch ) )
            {
                fch->fighting	= NULL;
                fch->position	= IS_NPC(fch) ? fch->default_pos : POS_STANDING;
                update_pos( fch );
                if ( !IS_NPC(fch) ) 
                    /* e.g., wimpy mob(ch) fleeing from PC(fch) */
                    fch->pcdata->lastfight = current_time;
            }
        }
    }
    /*
     * For PCs
     */
    else
    {
        bool pk = FALSE;
        for ( fch = char_list; fch != NULL; fch = fch->next )
        {
            if ( fch == ch || ( fBoth && fch->fighting == ch ) )
            {
                fch->fighting	= NULL;
                fch->position	= IS_NPC(fch) ? fch->default_pos : POS_STANDING;
                update_pos( fch );
                if ( !IS_NPC(fch) && fch != ch ) 
                {   /* e.g., player(ch) fleeing from another player(fch) */
                    fch->pcdata->lastfight = current_time;
                    fch->pcdata->lastpk = current_time;
                    pk = TRUE;
                }
            }
        }
        ch->pcdata->lastfight = current_time;
        if ( pk ) ch->pcdata->lastpk = current_time;
    }
    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;
    const static int ROT_DEATH_PERCENT = 25;
    const static int KO_ROT_PERCENT = 40;
    const static int GUILD_ROT_PERCENT = 50;

    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 3, 6 );
	if ( ch->gold > 0 )
	{
	    obj_to_obj( create_money( ch->gold, ch->silver ), corpse );
	    ch->gold = 0;
	    ch->silver = 0;
	}
	corpse->cost = 0;
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= number_range( 25, 40 );
	if (!is_guild(ch))
	    corpse->owner = str_dup(ch->name);
	else
	{
	    corpse->owner = NULL;
	    if (ch->gold > 1 || ch->silver > 1)
	    {
		obj_to_obj(create_money(ch->gold / 2, ch->silver/2), corpse);
		ch->gold -= ch->gold/2;
		ch->silver -= ch->silver/2;
	    }
	}
		
	corpse->cost = 0;
    }

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
      {
	bool floating = FALSE;
	
	obj_next = obj->next_content;
	
	if (obj->wear_loc == WEAR_FLOAT)
	  floating = TRUE;
	obj_from_char( obj );
	
	if (obj->item_type == ITEM_POTION)
	  obj->timer = number_range(500,1000);
	
	if (obj->item_type == ITEM_SCROLL)
	  obj->timer = number_range(1000,2500);
	
	if( !floating && !IS_NPC( ch ) )
	  {
	    if( ( IS_SET( obj->extra_flags, ITEM_ROT_DEATH ) && number_percent() < ROT_DEATH_PERCENT ) ||
		( IS_SET( obj->extra_flags, ITEM_KO_ROT ) && number_percent() < KO_ROT_PERCENT ) ||
		( IS_SET( obj->extra_flags, ITEM_GUILD_ROT ) && number_percent() < GUILD_ROT_PERCENT ) )
	      {
		
		if (!(IS_SET(queststatus, QUEST_INPROGRESS)
		      &&    IS_SET(queststatus, QUEST_NO_DEATHROT)
		      &&    IS_SET(ch->act_bits,PLR_QUEST)))
		  {
		    obj->timer = number_range(5,10);
		    REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);
		  }
	      }
	  }
	    
	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);
	
	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	  extract_obj( obj );
	else if( floating && !IS_NPC( ch ) )
	  {
	    if( ( IS_OBJ_STAT( obj, ITEM_ROT_DEATH ) && number_percent() < ROT_DEATH_PERCENT ) ||
		( IS_OBJ_STAT( obj, ITEM_KO_ROT ) && number_percent() < KO_ROT_PERCENT ) ||
		( IS_OBJ_STAT( obj, ITEM_GUILD_ROT ) && number_percent() < GUILD_ROT_PERCENT ) )
	      {
		if (obj->contains != NULL)
		  {
		    OBJ_DATA *in, *in_next;
		      
		    act("$p evaporates, scattering its contents.",
			ch,obj,NULL,TO_ROOM);
		    for (in = obj->contains; in != NULL; in = in_next)
		      {
			in_next = in->next_content;
			obj_from_obj(in);
			obj_to_room(in,ch->in_room);
		      }
		  }
		else
		  act("$p evaporates.",
		      ch,obj,NULL,TO_ROOM);
		extract_obj(obj);
	      }
	    else
	      {
		act("$p falls to the floor.",ch,obj,NULL,TO_ROOM);
		obj_to_room(obj,ch->in_room);
	      }
	  }
	else
	  obj_to_obj( obj, corpse );
      }

    obj_to_room( corpse, ch->in_room );
    return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    bool soulgem = FALSE;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear $n's death cry.";

    switch ( number_bits(4))
    {
    case  0: msg  = "$n hits the ground ... DEAD.";			break;
    case  1: 
	if (ch->material == 0)
	{
	    msg  = "$n splatters blood on your armor.";		
	    break;
	}
    case  2: 							
	if (IS_SET(ch->parts,PART_GUTS))
	{
	    msg = "$n spills $s guts all over the floor.";
	    vnum = OBJ_VNUM_GUTS;
	}
	break;
    case  3: 
	if (IS_SET(ch->parts,PART_HEAD))
	{
	    msg  = "$n's severed head plops on the ground.";
	    vnum = OBJ_VNUM_SEVERED_HEAD;				
	}
	break;
    case  4: 
	if (IS_SET(ch->parts,PART_HEART))
	{
	    msg  = "$n's heart is torn from $s chest.";
	    vnum = OBJ_VNUM_TORN_HEART;				
	}
	break;
    case  5: 
	if (IS_SET(ch->parts,PART_ARMS))
	{
	    msg  = "$n's arm is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_ARM;				
	}
	break;
    case  6: 
	if (IS_SET(ch->parts,PART_LEGS))
	{
	    msg  = "$n's leg is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_LEG;				
	}
	break;
    case 7:
	if (IS_SET(ch->parts,PART_BRAINS))
	{
	    msg = "$n's head is shattered, and $s brains splash all over you.";
	    vnum = OBJ_VNUM_BRAINS;
	}
    }
	
	if (IS_NPC( ch )) {

		if ( ch->max_hit > 25000 && number_percent() <= 20)  {
		
			msg = "An intense magical backlash imprisons $n's soul into a gem!";
            vnum = number_range(OBJ_VNUM_SOULGEM_BOT,OBJ_VNUM_SOULGEM_TOP);
            soulgem = TRUE;
			
		}
		
		else if ( ch->max_hit > 24000 && number_percent() <= 19) {
		
			msg = "An intense magical backlash imprisons $n's soul into a gem!";
            vnum = number_range(OBJ_VNUM_SOULGEM_BOT,OBJ_VNUM_SOULGEM_TOP);
            soulgem = TRUE;
			
		}
		
		else if ( ch->max_hit > 22000 && number_percent() <= 18) {
		
			msg = "An intense magical backlash imprisons $n's soul into a gem!";
            vnum = number_range(OBJ_VNUM_SOULGEM_BOT,OBJ_VNUM_SOULGEM_TOP);
            soulgem = TRUE;
			
		}
		
		else if ( ch->max_hit > 20000 && number_percent() <= 17) {
		
			msg = "An intense magical backlash imprisons $n's soul into a gem!";
            vnum = number_range(OBJ_VNUM_SOULGEM_BOT,OBJ_VNUM_SOULGEM_TOP);
            soulgem = TRUE;
			
		}
		
		else if ( ch->max_hit > 18000 && number_percent() <= 16) {
		
			msg = "An intense magical backlash imprisons $n's soul into a gem!";
            vnum = number_range(OBJ_VNUM_SOULGEM_BOT,OBJ_VNUM_SOULGEM_TOP);
            soulgem = TRUE;
			
		}
		
		else if ( ch->max_hit > 16000 && number_percent() <= 15) {
		
			msg = "An intense magical backlash imprisons $n's soul into a gem!";
            vnum = number_range(OBJ_VNUM_SOULGEM_BOT,OBJ_VNUM_SOULGEM_TOP);
            soulgem = TRUE;
			
		}
		
		else if ( ch->max_hit > 14000 && number_percent() <= 14) {
		
			msg = "An intense magical backlash imprisons $n's soul into a gem!";
            vnum = number_range(OBJ_VNUM_SOULGEM_BOT,OBJ_VNUM_SOULGEM_TOP);
            soulgem = TRUE;
			
		}
		
		else if ( ch->max_hit > 12000 && number_percent() <= 13) {
		
			msg = "An intense magical backlash imprisons $n's soul into a gem!";
            vnum = number_range(OBJ_VNUM_SOULGEM_BOT,OBJ_VNUM_SOULGEM_TOP);
            soulgem = TRUE;
			
		}
		
		else if ( ch->max_hit > 11000 && number_percent() <= 12) {
		
			msg = "An intense magical backlash imprisons $n's soul into a gem!";
            vnum = number_range(OBJ_VNUM_SOULGEM_BOT,OBJ_VNUM_SOULGEM_TOP);
            soulgem = TRUE;
			
		}
		
		else if ( ch->max_hit > 9999 && number_percent() <= 11) {
		
			msg = "An intense magical backlash imprisons $n's soul into a gem!";
            vnum = number_range(OBJ_VNUM_SOULGEM_BOT,OBJ_VNUM_SOULGEM_TOP);
            soulgem = TRUE;
			
		}
	
	}
	
    if (!IS_NPC(ch))
    {
        act( msg, ch, NULL, NULL, TO_ROOM );
        msg = "$n's ghost rises up from $s body.";
        act( msg, ch, NULL, NULL, TO_ROOM );
        send_to_char("You rise up from your body as a ghost.\n\r",ch);
    }

    if ( vnum != 0 )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	
        if ( !soulgem )
            obj->timer	= number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );
	
		if (soulgem) {
			sprintf( buf, "caged soul gem %s", name );
			free_string(obj->name);
			obj->name = str_dup(buf);
		}
		
        if ( soulgem && obj->pIndexData->extra_descr )
        {
            EXTRA_DESCR_DATA *ed;

            ed = new_extra_descr();
            sprintf(buf, obj->pIndexData->extra_descr->description, name );
            ed->keyword = str_dup( obj->pIndexData->extra_descr->keyword );
            ed->next = obj->extra_descr;
            ed->description = str_dup( buf );
            obj->extra_descr = ed; 
        }

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	if(IS_NPC(ch)) act( msg, ch, NULL, NULL, TO_ROOM );
	obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}

void unconscious_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg  = "$n falls to the ground unconscious.";			

    act( msg, ch, NULL, NULL, TO_ROOM );

	msg = "You hear someone cry out as they're knocked unconscious.";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}


void raw_kill( CHAR_DATA *victim )
{
    char pueblo_buf[MAX_STRING_LENGTH];
    int i;
    if(IS_SET(victim->act_bits,PLR_QUEST) &&
	!IS_NPC(victim) &&
	IS_SET(queststatus,QUEST_ALWAYS_KO))
	{
	unconscious_kill(victim);
	return;
	}

    stop_fighting( victim, TRUE );
    death_cry( victim );
    player_quitting = TRUE;
    make_corpse( victim );
    player_quitting = FALSE;

            if (IS_SET(victim->act_bits, PLR_PUEBLO)
		&& !IS_NPC(victim)) 
	{
        sprintf(pueblo_buf,"<img xch_sound=play xch_volume=%dsrc=\"%s%s\">"
                "<img src=\"%s%s\">",
victim->pcdata->volume,PUEBLO_DIR,PUEBLO_END, PUEBLO_DIR, PUEBLO_D_IMG);
        send_to_char("</xch_mudtext><img xch_mode=html>",victim);
        send_to_char(pueblo_buf,victim);
        send_to_char("<br><img xch_mode=text>",victim);
        send_to_char("</xch_mudtext><img xch_mode=html>",victim);
                sprintf(pueblo_buf,"<img xch_sound=stop>");
        send_to_char(pueblo_buf,victim);
        send_to_char("<br><img xch_mode=text>",victim);
        }

    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
//      bug( "extract_char called from raw_kill", 0 );
	extract_char( victim, TRUE );
	return;
    }

/*
    extract_char( victim, FALSE );
*/
    nuke_pets( victim );
    while ( victim->affected )
	affect_remove( victim, victim->affected );
    victim->affected_by	= race_table[victim->race].aff;
    for (i = 0; i < 4; i++)
    	victim->armor[i]= 100;
    victim->hit		= UMAX( 1, victim->hit  );
    victim->mana	= UMAX( 1, victim->mana );
    victim->move	= UMAX( 1, victim->move );
	victim->daze 	= PULSE_TICK;
    SET_BIT(victim->affected_by2,AFF_GHOST);
    SET_BIT(victim->affected_by,AFF_PASS_DOOR);
    if(IS_SET(victim->shiftbits,TEMP_VAMP)) 
      {
	REMOVE_BIT(victim->shiftbits,TEMP_VAMP);
	free_string(victim->patriarch);
	victim->patriarch = &str_empty[ 0 ];
      }

    victim->wait = 0;
    victim->daze = 0;
    victim->position = POS_STANDING;

    return;
}


void dispersal_kill( CHAR_DATA* victim )
{
  OBJ_DATA* corpse;
  OBJ_DATA* obj;
  OBJ_DATA* obj_next;
  ROOM_INDEX_DATA* target;
  char* name;
  char buf[ MAX_STRING_LENGTH ];
  char pueblo_buf[ MAX_STRING_LENGTH ];
  int i;

  if( IS_SET( victim->act_bits, PLR_QUEST ) &&
      !IS_NPC( victim ) &&
      IS_SET( queststatus, QUEST_ALWAYS_KO ) )
    {
      unconscious_kill( victim );
      return;
    }

  stop_fighting( victim, TRUE );
  death_cry( victim );

  if( IS_NPC( victim ) )
    {
      name = victim->short_descr;
      corpse = create_object( get_obj_index( OBJ_VNUM_CORPSE_NPC ), 0 );
      corpse->timer = number_range( 3, 6 );
    }
  else
    {
      name = victim->name;
      corpse = create_object( get_obj_index( OBJ_VNUM_CORPSE_PC ), 0 );
      corpse->timer = number_range( 25, 40 );

      if( !is_guild( victim ) )
	{
	  corpse->owner = str_dup( victim->name );
	}
      else
	{
	  corpse->owner = NULL;
	}
    }

  corpse->cost = 0;
  corpse->level = victim->level;

  sprintf( buf, corpse->short_descr, name );
  free_string( corpse->short_descr );
  corpse->short_descr = str_dup( buf );

  sprintf( buf, corpse->description, name );
  free_string( corpse->description );
  corpse->description = str_dup( buf );

  obj_to_room( corpse, victim->in_room );

  for( obj = victim->carrying; obj != NULL; obj = obj_next )
    {
      obj_next = obj->next_content;

/*    If obj_next is the weapon and obj gives the str requierd to wield it,
      terrible things can happen without this. */
      player_quitting = TRUE;
      obj_from_char( obj );
      player_quitting = FALSE;

      switch( obj->item_type )
	{
	case ITEM_POTION:
	  obj->timer = number_range( 500, 1000 );
	  break;

	case ITEM_SCROLL:
	  obj->timer = number_range( 1000, 2500 );
	  break;

	default:
	  break;
	}

      if( IS_SET( obj->extra_flags, ITEM_ROT_DEATH ) && !IS_NPC( victim ) )
	{
	  if( !( IS_SET( queststatus, QUEST_INPROGRESS ) &&
		 IS_SET( queststatus, QUEST_NO_DEATHROT ) ) )
	    {
	      obj->timer = number_range( 5, 10 );
	      REMOVE_BIT( obj->extra_flags, ITEM_ROT_DEATH );
	    }
	}

      REMOVE_BIT( obj->extra_flags, ITEM_VIS_DEATH );

      if( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	extract_obj( obj );

      /* TSM begin section for puking containers */
      if( obj->contains != NULL )
	{
	  OBJ_DATA* in;
	  OBJ_DATA* in_next;

	  for( in = obj->contains; in != NULL; in = in_next )
	    {
	      in_next = in->next_content;
	      obj_from_obj( in );
	      target = get_random_room( victim );
	      obj_to_room( in, target );
	    }
	}
      /* TSM end section for puking containers */

      target = get_random_room( victim );
      obj_to_room( obj, target );
    }

  if( IS_SET( victim->act_bits, PLR_PUEBLO ) && !IS_NPC( victim ) ) 
    {
      sprintf(pueblo_buf,"<img xch_sound=play xch_volume=%d src=\"%s%s\">" "<img src=\"%s%s\">",
	      victim->pcdata->volume,PUEBLO_DIR,PUEBLO_END, PUEBLO_DIR, PUEBLO_D_IMG);

      send_to_char("</xch_mudtext><img xch_mode=html>",victim);
      send_to_char(pueblo_buf,victim);
      send_to_char("<br><img xch_mode=text>",victim);
      send_to_char("</xch_mudtext><img xch_mode=html>",victim);
      sprintf(pueblo_buf,"<img xch_sound=stop>");
      send_to_char(pueblo_buf,victim);
      send_to_char("<br><img xch_mode=text>",victim);
    }

  if ( IS_NPC(victim) )
    {
      victim->pIndexData->killed++;
      kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
      extract_char( victim, TRUE );
      return;
    }

  nuke_pets( victim );
  while ( victim->affected )
    affect_remove( victim, victim->affected );
  victim->affected_by	= race_table[victim->race].aff;
  
  for (i = 0; i < 4; i++)
    victim->armor[i]= 100;
  
  victim->hit		= UMAX( 1, victim->hit  );
  victim->mana	= UMAX( 1, victim->mana );
  victim->move	= UMAX( 1, victim->move );
  victim->daze 	= PULSE_TICK;
  
  SET_BIT(victim->affected_by2,AFF_GHOST);
  SET_BIT(victim->affected_by,AFF_PASS_DOOR);
  
  if(IS_SET(victim->shiftbits,TEMP_VAMP)) 
    {
      REMOVE_BIT(victim->shiftbits,TEMP_VAMP);
      REMOVE_BIT(victim->shiftbits,SHIFT_FIXEDBP);
      free_string(victim->patriarch);
      victim->patriarch = &str_empty[ 0 ];
    }
  
  victim->wait = 0;
  victim->daze = 0;
  victim->position = POS_STANDING;

}


void unconscious_kill( CHAR_DATA *victim )
{
  const static int GUILD_ROT_KO_PERCENT = 20;

  OBJ_DATA* obj;

   if (IS_NPC(victim)) 
    {
      raw_kill(victim);
      return;
    }
    
  stop_fighting( victim, TRUE );
  unconscious_cry( victim );

  if( !IS_SET( victim->act_bits, PLR_NOSUMMON ) )
    SET_BIT( victim->act_bits, PLR_NOSUMMON );

  nuke_pets( victim );
  while ( victim->affected )
    affect_remove( victim, victim->affected );
  
  victim->affected_by = race_table[victim->race].aff;
  victim->position    = POS_UNCONSCIOUS;
  victim->daze 	      = PULSE_TICK;
  victim->wait	      = 0; 
  victim->hit	      = UMAX( 1, victim->hit  );
  victim->mana	      = UMAX( 1, victim->mana );
  victim->move	      = UMAX( 1, victim->move );

  if( !IS_NPC( victim ) && 
      victim->level >= 30 && 
      IS_SET( victim->shiftbits, SHIFT_POTENTIAL ) )
    {
      victim->pcdata->cleanse_timer = PULSE_TICK;
    }
  
  /* guild rot flag */
  for( obj = victim->carrying; obj != NULL; obj = obj->next_content )
    {
      if( IS_SET( obj->extra_flags, ITEM_GUILD_ROT ) && number_percent() < GUILD_ROT_KO_PERCENT )
	{
	  if( !( IS_SET( queststatus, QUEST_INPROGRESS ) &&
		 IS_SET( queststatus, QUEST_NO_DEATHROT ) &&
		 IS_SET( victim->act_bits, PLR_QUEST ) ) )
	    {
	      obj->timer = number_range( 5, 10 );
	      REMOVE_BIT( obj->extra_flags, ITEM_GUILD_ROT );
	    }
	}
    }
}


void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    int group_levels;

    /*
     * Monsters don't get kill xp's 
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( victim == ch )
	return;

    if (!IS_NPC(victim))
	return;

    members = 0;
    group_levels = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
        {
	    members++;
	    group_levels += IS_NPC(gch) ? gch->level / 2 : gch->level;
	}
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
	group_levels = ch->level ;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room ) {
		if ( !is_same_group( gch, ch ) || IS_NPC(gch)) continue;

		xp = xp_compute( gch, victim->level, group_levels, members );  

		if( !IS_NPC( gch ) && IS_SET( gch->act_bits, PLR_SUSPEND_LEVEL ) ) {
			send_to_char( "You have suspended your levelling.\n\r", gch );
		} else {
			if (quad >= 0) {
				sprintf(buf, "{YYou receive %d experience points.{x\n\r", xp);
				xp = xp * 4;
			} else {
				sprintf(buf, "You receive %d experience points.\n\r", xp);
			}

			send_to_char(buf, gch);
			gain_exp(gch, xp);
			
		}

    }

    return;
}



/*
 * Compute xp for a kill.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, int victim_level, int total_levels, int members )
{
    int xp1, xp2, xp3, base_exp;
    int level_range;
    const float level_mult[] = { 1, 1, .95, .893, .830,
				 .764, .695, .626, .52 }; 
    level_range = victim_level - gch->level;

    if (level_range >= 20)
       return 0;

    if (IS_AFFECTED2(gch,AFF_GHOST))
       return 0;

    if( IS_SET( gch->act_bits, PLR_SUSPEND_LEVEL ) )
      return 0;

 
    /* compute the base exp */
    switch (level_range)
    {
 	default : 	base_exp =   0;		break;
	case -9 :	base_exp =   1;		break;
	case -8 :	base_exp =   3;		break;
	case -7 :	base_exp =   7;		break;
	case -6 : 	base_exp =  11;		break;
	case -5 :	base_exp =  22;		break;
	case -4 :	base_exp =  33;		break;
	case -3 :	base_exp =  50;		break;
	case -2 :	base_exp =  68;		break;
	case -1 :	base_exp =  85;		break;
	case  0 :	base_exp = 100;		break;
	case  1 :	base_exp = 180;		break;
	case  2 :	base_exp = 270;		break;
	case  3 :	base_exp = 370;		break;
	case  4 :	base_exp = 480;		break;
    } 
    
    if (level_range > 4)
	base_exp = 490 + 100 * (level_range - 4);

    switch (level_range) { /* xp drops off near 20 */
	case 17:   base_exp = 3 * base_exp / 4;  break;
	case 18:   base_exp /= 2;		 break;
        case 19:   base_exp /= 4;		 break;
	default:				 break;
    }
    /* calculate exp multiplier */
    xp1 = base_exp;

    /* more exp at the low levels */
    if (gch->level < 6)
    	xp1 = 10 * xp1 / (gch->level + 4);

    /* less at high */
    if (gch->level > 35 )
	xp1 =  15 * xp1 / (gch->level - 25 );

    /* reduce for playing time */
    
#if 0
    {
	/* compute quarter-hours per level */
	time_per_level = 4 *
			 (gch->played + (int) (current_time - gch->logon))/3600
			 / gch->level;

	time_per_level = URANGE(2,time_per_level,12);
	if (gch->level < 15)  /* make it a curve */
	    time_per_level = UMAX(time_per_level,(15 - gch->level));
	xp = xp * time_per_level / 12;
    }
#endif
   
    /* randomize the rewards */
    xp1 = number_range (xp1 * 3/4, xp1 * 5/4);

    xp1 = UMIN( 1000, xp1 );
    if (members == 1) {
	return xp1;
    }

    total_levels = 6 * total_levels / 5;
    /* adjust for grouping */
    xp2 = xp1 * gch->level/total_levels;

    members = URANGE(1, members, 8);
    xp3 = xp1 * level_mult[members];

    xp1 = (xp1 + xp2 + xp3) / 3;

    if( !IS_NPC( gch ) && IS_SET( gch->act_bits, PLR_SUSPEND_LEVEL ) )
      {
	xp1 = 0;
      }

    return xp1;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune )
{
    char buf1[256], buf2[256], buf3[256];
    char verb[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;

    if (ch == NULL || victim == NULL)
	return;

	 if ( dam ==   0 ) { vs = "miss{x";	vp = "misses{x";	}
    else if ( dam <=   4 ) { vs = "scratch{x"; vp = "scratches{x";	}
    else if ( dam <=   8 ) { vs = "graze{x";	vp = "grazes{x";	}
    else if ( dam <=  12 ) { vs = "hit{x";	vp = "hits{x";	}
    else if ( dam <=  16 ) { vs = "injure{x";	vp = "injures{x";	}
    else if ( dam <=  20 ) { vs = "wound{x";	vp = "wounds{x";	}
    else if ( dam <=  24 ) { vs = "maul{x";       vp = "mauls{x";	}
    else if ( dam <=  28 ) { vs = "decimate{x"; vp = "decimates{x";	}
    else if ( dam <=  32 ) { vs = "devastate{x"; vp = "devastates{x";}
    else if ( dam <=  36 ) { vs = "maim{x";	vp = "maims{x";	}
    else if ( dam <=  40 ) { vs = "MUTILATE{x"; vp = "MUTILATES{x";	}
    else if ( dam <=  44 ) { vs = "DISEMBOWEL{x";vp = "DISEMBOWELS{x";}
    else if ( dam <=  48 ) { vs = "DISMEMBER{x";vp = "DISMEMBERS{x";}
    else if ( dam <=  52 ) { vs = "MASSACRE{x"; vp = "MASSACRES{x";	}
    else if ( dam <=  56 ) { vs = "MANGLE{x";	vp = "MANGLES{x";	}
    else if ( dam <=  60 ) { vs = "*** DEMOLISH ***{x";
			     vp = "*** DEMOLISHES ***{x";		}
    else if ( dam <=  75 ) { vs = "*** DEVASTATE ***{x";
			     vp = "*** DEVASTATES ***{x";		}
    else if ( dam <= 100)  { vs = "=== OBLITERATE ==={x";
			     vp = "=== OBLITERATES ==={x";		}
    else if ( dam <= 125)  { vs = ">>> ANNIHILATE <<<{x";
			     vp = ">>> ANNIHILATES <<<{x";		}
    else if ( dam <= 150)  { vs = "<<< ERADICATE >>>{x";
			     vp = "<<< ERADICATES >>>{x";		}
    else                   { vs = "<*>_MORTALLY WOUND_<*>{x";
    			     vp = "<*>_MORTALLY WOUNDS_<*>{x";}

    if ( !IS_NPC(ch) && IS_SET(ch->comm,COMM_VERBOSE) )
        sprintf( verb, " (%d)", dam );
    else
        verb[0] = '\0';

    punct   = (dam <= 24) ? '.' : '!';

    if ( dt == TYPE_HIT )
    {
	if (ch  == victim)
	{
	    sprintf( buf1, "$n %s $mself%c",vp,punct);
	    sprintf( buf2, "You {%c%s yourself%s%c",ch->colors[C_YBATTLE],vs,verb,punct);
	}
	else
	{
	    sprintf( buf1, "$n %s $N%c",  vp, punct );
	    sprintf( buf2, "You {%c%s $N%s%c", ch->colors[C_YBATTLE],vs, verb, punct );
	    sprintf( buf3, "$n {%c%s you%c", victim->colors[C_TBATTLE],vp, punct );
	}
    }
    else
    {
	if ( dt >= 0 && dt < MAX_SKILL )
	    attack	= skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT
	&& dt < TYPE_HIT + MAX_DAMAGE_MESSAGE) 
	    attack	= attack_table[dt - TYPE_HIT].noun;
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0].name;
	}

	if (immune)
	{
	    if (ch == victim)
	    {
		sprintf(buf1,"$n is unaffected by $s own %s.",attack);
		sprintf(buf2,"Luckily, you are immune to that.");
	    } 
	    else
	    {
	    	sprintf(buf1,"$N is unaffected by $n's %s!",attack);
	    	sprintf(buf2,"$N is unaffected by your %s!",attack);
	    	sprintf(buf3,"$n's %s is powerless against you.",attack);
	    }
	}
	else
	{
	    if (ch == victim)
	    {
		sprintf( buf1, "$n's %s %s $m%c",attack,vp, punct);
		sprintf( buf2, "Your %s {%c%s you%s%c",attack,
		    ch->colors[C_YBATTLE],vp,verb,punct);
	    }
	    else
	    {
	    	sprintf( buf1, "$n's %s %s $N%c",  attack, vp, punct );
	    	sprintf( buf2, "Your %s {%c%s $N%s%c",  attack, 
		  ch->colors[C_YBATTLE],vp, verb, punct );
	    	sprintf( buf3, "$n's %s {%c%s you%c", attack, 
		  victim->colors[C_TBATTLE],vp, punct );
	    }
	}
    }

    if (ch == victim)
    {
	act(buf1,ch,NULL,NULL,TO_ROOM);
	act(buf2,ch,NULL,NULL,TO_CHAR);
    }
    else
    {
    	act( buf1, ch, NULL, victim, TO_NOTVICT );
    	act( buf2, ch, NULL, victim, TO_CHAR );
    	act( buf3, ch, NULL, victim, TO_VICT );
    }

    return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE) || IS_WEAPON_STAT( obj, WEAPON_STICKY ) )
    {
	act("$S weapon won't budge!",ch,NULL,victim,TO_CHAR);
	act("$n tries to disarm you, but your weapon won't budge!",
	    ch,NULL,victim,TO_VICT);
	act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    act( "$n DISARMS you and sends your weapon flying!", 
	 ch, NULL, victim, TO_VICT    );
    act( "You disarm $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n disarms $N!",  ch, NULL, victim, TO_NOTVICT );

    obj_from_char( obj );

    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
	obj_to_char_init( obj, victim ); /* Init because disarming a mob shouldn't cause a weapon to start rotting */
    else
    {
	obj_to_room( obj, victim->in_room );
	if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
	    get_obj(victim,obj,NULL);
    }

    return;
}

void dislodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;
    int chance;

    chance = number_range(1,100);

    if ( ( obj = get_eq_char( victim, WEAR_SHIELD ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
	act("$S shield won't budge!",ch,NULL,victim,TO_CHAR);
	act("$n tries to dislodge your shield, but your shield won't budge!",
	    ch,NULL,victim,TO_VICT);
	act("$n tries to dislodge $N's shield, but fails.",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    if (chance < 21)
     {
       act( "$n SHATTERS your shield!",  ch, NULL, victim, TO_VICT    );
       act( "You SHATTER $N's shield!",  ch, NULL, victim, TO_CHAR    );
       act( "$n SHATTERS $N's shield!",  ch, NULL, victim, TO_NOTVICT );

       extract_obj(obj);
       return;
     }

   else
     {
       act( "$n DISLODGES your shield and sends it flying!", 
           ch, NULL, victim, TO_VICT    );
       act( "You dislodge $N's shield!",  ch, NULL, victim, TO_CHAR    );
       act( "$n dislodge $N's shield!",  ch, NULL, victim, TO_NOTVICT );

       obj_from_char( obj );
       if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY))
	obj_to_char_init( obj, victim );
       else
       {
	obj_to_room( obj, victim->in_room );
	if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
	    get_obj(victim,obj,NULL);
       }

       return;
     }
}

void do_cleanse( CHAR_DATA* ch, char* argument )
{
  char log_buf[ MAX_STRING_LENGTH ] = "";
  char victim_name[ MAX_INPUT_LENGTH ] = "";
  CHAR_DATA* victim = NULL;

  if( !IS_IMMORTAL( ch ) &&
      ch->guild != guild_lookup( "covenance" ) )
    {
      send_to_char( "Huh?\n\r", ch );
      return;
    }

  if( get_skill( ch, gsn_cleanse ) < 1 )
    {
      send_to_char( "Your faith is too weak to cleanse another.\n\r", ch );
      return;
    }

  argument = one_argument( argument, victim_name );
  if( *victim_name == '\0' )
    {
      send_to_char( "Who should be cleansed?\n\r", ch );
      return;
    }

  victim = get_char_room( ch, victim_name, FALSE );
  if( victim == NULL )
    {
      send_to_char( "That abomination is not here.\n\r", ch );
      return;
    }

  if( IS_NPC( victim ) )
    {
      send_to_char( "Purifying that abomination is beyond your power.\n\r", ch );
      return;
    }

  if( victim->position > POS_UNCONSCIOUS )
    {
      send_to_char( "You must defeat your opponent before it can be cleansed.\n\r", ch );
      return;
    }

  if( victim->pcdata->cleanse_timer > 0 )
    {

      victim->pcdata->cleansings++;
      victim->pcdata->cleanse_timer = 0;
      victim->pcdata->last_cleanse = current_time;

      send_to_char( "You invoke the name of your god and cleanse the abomination.\n\r", ch );
      send_to_char( "You feel a burning pain in the depths of your soul!\n\r", victim );
      act_new( "$n invokes the name of $s god and cleanses the soul of $N!", ch, NULL, victim, TO_NOTVICT, POS_RESTING, 0 );

      sprintf( log_buf, "%s cleansed by %s at %d",
	       victim->name,
	       ( IS_NPC( ch ) ? ch->short_descr : ch->name ),
	       ch->in_room->vnum );
      log_string( log_buf );
      wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,LEVEL_ADMIN);

      check_improve( ch, gsn_cleanse, TRUE, 2 );
    }
  else
    {
      send_to_char( "Your window of opportunity has passed.\n\r", ch );
    }

}
  

  
void do_battle( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}

    if ((chance = get_skill(ch,gsn_battle)) == 0
    ||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    ||  (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_battle].skill_level[ch->class]))
    {
	send_to_char("You can't seem to focus on the battle.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,gsn_battle)
    ||  is_affected(ch,skill_lookup("frenzy")))
    {
	send_to_char("You are already focused on the battle.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	send_to_char("You're feeling to calm to battle.\n\r",ch);
	return;
    }

    if (ch->mana < 50)
    {
	send_to_char("You can't get up enough energy.\n\r",ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
	AFFECT_DATA af;

	WAIT_STATE(ch,PULSE_VIOLENCE/3*2);
	ch->mana -= 25;
	ch->move -= 50;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit,ch->max_hit);

	send_to_char("You focus on the battle before you!\n\r",ch);
	act("$n gets a calculating look in $s eyes.",ch,NULL,NULL,TO_ROOM);
	check_improve(ch,gsn_battle,TRUE,2);

	af.where	= TO_AFFECTS;
	af.type		= gsn_battle;
	af.level	= ch->level;
	af.duration	= number_fuzzy(ch->level / 5);
	af.modifier	= UMAX(1,ch->level/5);
	af.bitvector 	= AFF_BERSERK;

	af.location	= APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	= APPLY_DAMROLL;
	affect_to_char(ch,&af);

	af.modifier	= - ch->level/5*4;
	af.location	= APPLY_AC;
	affect_to_char(ch,&af);
    }

    else
    {
	WAIT_STATE(ch,PULSE_VIOLENCE);
	ch->mana -= 25;
	ch->move -= 50;

	send_to_char("You clear your mind but aren't ready for battle.\n\r",ch);
	check_improve(ch,gsn_battle,FALSE,2);
    }
}

void do_bash( CHAR_DATA *ch, char *argument )
{
  char arg[ MAX_INPUT_LENGTH ] = "";
  CHAR_DATA *victim = NULL;
  int chance;

  if( ch->wait > 0 )
    {
      return;
    }

  if (IS_AFFECTED2(ch,AFF_GHOST))
    {
      send_to_char("You're a ghost!\n\r",ch);
      return;
    }
  
  one_argument(argument,arg);

  if( ( chance = get_skill( ch, gsn_bash ) ) == 0 ||
      ( IS_NPC( ch ) && !IS_SET( ch->off_flags, OFF_BASH ) ) ||
      ( !IS_NPC( ch ) && ch->level < skill_table[ gsn_bash ].skill_level[ ch->class ] ) )
    {
      send_to_char("Bashing?  What's that?\n\r",ch);
      return;
    }


  if (arg[0] == '\0')
    {
      victim = ch->fighting;
      if (victim == NULL)
	{
	  send_to_char("But you aren't fighting anyone!\n\r",ch);
	  return;
 	}
    }

  else if ((victim = get_char_room(ch,arg, TRUE)) == NULL)
    {
      send_to_char("They aren't here.\n\r",ch);
      return;
    }

  if (is_safe(ch,victim))
    return;

  if (victim->position < POS_FIGHTING)
    {
      act("$N is already down.",ch,NULL,victim,TO_CHAR);
      return;
    }

  if (victim == ch)
    {
      send_to_char("You try to bash yourself and fail!\n\r",ch);
      WAIT_STATE(ch,PULSE_VIOLENCE);
      act("$n tries to bash $mself!",ch,NULL,NULL,TO_ROOM);
      return;
    }

  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
      act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
      return;
    }

  if(IS_SET(victim->imm_flags,IMM_LAGSKILLS)) {
    act("You can't seem to knock $M over!",ch,NULL,victim,TO_CHAR);
    return;
  }

  /* modifiers */

  /* size */ 
  if (ch->size < victim->size)
      chance += (ch->size - victim->size) * 5;
        
/*chance += get_curr_stat( ch,STAT_STR ); */

  /*flying*/
  if( IS_AFFECTED( ch, AFF_FLYING ) )
    chance -= 40;

  /* speed */
  if( IS_SET( ch->off_flags, OFF_FAST) || IS_AFFECTED( ch, AFF_HASTE ) )
    chance += 10;

  /* level */
  chance += (ch->level - victim->level);

    /* now the attack */
    if (number_percent() < chance)
    {
      act( "$n sends you sprawling with a powerful bash!", ch, NULL, victim, TO_VICT );
      act( "You slam into $N, and send $M flying!", ch, NULL, victim, TO_CHAR );
      act( "$n sends $N sprawling with a powerful bash.", ch, NULL, victim, TO_NOTVICT );
      check_improve( ch, gsn_bash, TRUE, 1 );

      WAIT_STATE(victim,PULSE_VIOLENCE*2);
      WAIT_STATE(ch,PULSE_VIOLENCE*2+4);
      damage( ch, victim, number_range( 2, 2 + 2 * ch->size ), gsn_bash, 
          DAM_BASH, TRUE );

/* damage() can cause wimpy which changes victim->in_room */
      if ( victim->position > POS_FLATFOOTED && victim->in_room == ch->in_room )
          victim->position = POS_SPRAWLED;
    }
    else
    {
      damage(ch,victim,0,gsn_bash,DAM_BASH,FALSE);
      act("You fall flat on your face!",
	  ch,NULL,victim,TO_CHAR);
      act("$n falls flat on $s face.",
	  ch,NULL,victim,TO_NOTVICT);
      act("You evade $n's bash, causing $m to fall flat on $s face.",
	  ch,NULL,victim,TO_VICT);
      check_improve(ch,gsn_bash,FALSE,1);
      WAIT_STATE(ch,PULSE_VIOLENCE);
      ch->position = POS_SPRAWLED;
    } 
  check_killer(ch,victim);
}


void do_dirt( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_dirt)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT))
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_dirt].skill_level[ch->class]))
    {
	send_to_char("You get your feet dirty.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg, TRUE)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("$E's already been blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (IS_AFFECTED3(victim,AFF_DIRT))
    {
	act("$E's already been blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;


    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

   if(IS_SET(victim->imm_flags,IMM_LAGSKILLS)) {
       act("You can't seem to affect $S sight that way!",ch,NULL,victim,TO_CHAR);
       return;
   }
    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):		chance -= 20;	break;
	case(SECT_CITY):		chance -= 10;	break;
	case(SECT_SNOW):		chance -= 10;	break;
	case(SECT_FIELD):		chance +=  5;	break;
	case(SECT_FOREST):				break;
	case(SECT_HILLS):				break;
	case(SECT_MOUNTAIN):		chance -= 10;	break;
	case(SECT_WATER_SWIM):		chance  =  0;	break;
	case(SECT_WATER_NOSWIM):	chance  =  0;	break;
	case(SECT_AIR):			chance  =  0;  	break;
	case(SECT_LAVA):		chance  =  0;  	break;
	case(SECT_DESERT):		chance += 10;   break;
    }

    if (chance == 0)
    {
	send_to_char("There isn't any dirt to kick.\n\r",ch);
	return;
    }

    if ( class_table[ch->class].fMana )
        chance = UMIN(chance,CONFIG_DIRTKICK_FMANA_CAP);

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("$n is blinded by the dirt in $s eyes!",victim,NULL,NULL,TO_ROOM);
	act("$n kicks dirt in your eyes!",ch,NULL,victim,TO_VICT);
        damage(ch,victim,number_range(2,5),gsn_dirt,DAM_NONE,FALSE);
	send_to_char("You can't see a thing!\n\r",victim);
	check_improve(ch,gsn_dirt,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);

	af.where	= TO_AFFECTS;
        af.whichaff     = AFF3;
	af.type 	= gsn_dirt;
	af.level 	= ch->level;
	af.duration	= 0;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_DIRT;

	affect_to_char3(victim,&af);
    }
    else
    {
	damage(ch,victim,0,gsn_dirt,DAM_NONE,TRUE);
	check_improve(ch,gsn_dirt,FALSE,2);
	WAIT_STATE(ch,PULSE_VIOLENCE);
    }
	check_killer(ch,victim);
}


void do_trip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    if( ch->wait > 0 )
      {
	return;
      }

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_trip)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch) 
	  && ch->level < skill_table[gsn_trip].skill_level[ch->class]))
    {
	send_to_char("Tripping?  What's that?\n\r",ch);
	return;
    }


    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,arg, TRUE)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

   /* if (IS_AFFECTED(victim,AFF_FLYING))
    {
	act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR);
	return;
    }*/

    if (victim->position < POS_FIGHTING)
    {
	act("$N is already down.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You fall flat on your face!\n\r",ch);
	WAIT_STATE(ch,PULSE_VIOLENCE);
	act("$n trips over $s own feet!",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if(IS_SET(victim->imm_flags,IMM_LAGSKILLS)) {
       act("You can't seem to knock $M over!",ch,NULL,victim,TO_CHAR);
       return;
}

    /* modifiers */

    /* size doesn't matter right now
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10; 
	*/

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
/*    chance -= get_curr_stat(victim,STAT_DEX); */

    /*flying*/
    if( IS_AFFECTED(victim,AFF_FLYING) )
       chance -= 40;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
/*    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 20;
*/

    /* level */
    chance += (ch->level - victim->level);

    if ( class_table[ch->class].fMana )
        chance = UMIN(chance,CONFIG_TRIP_FMANA_CAP);

    /* now the attack */
    if (number_percent() < chance)
    {
	act("$n trips you and you go down!",ch,NULL,victim,TO_VICT);
	act("You trip $N and $N goes down!",ch,NULL,victim,TO_CHAR);
	act("$n trips $N, sending $M to the ground.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_trip,TRUE,1);

        WAIT_STATE(victim,PULSE_VIOLENCE*2);
        WAIT_STATE(ch,PULSE_VIOLENCE*2+4);
	damage(ch,victim,number_range(2, 2 +  2 * victim->size),gsn_trip,
	    DAM_BASH,TRUE);

        /* damage() can cause wimpy which changes victim->in_room */
        if (victim->position > POS_FLATFOOTED && victim->in_room == ch->in_room)
            victim->position = POS_SPRAWLED;
    }
    else
    {
	damage(ch,victim,0,gsn_trip,DAM_BASH,TRUE);
	WAIT_STATE(ch,PULSE_VIOLENCE);
	check_improve(ch,gsn_trip,FALSE,1);
    } 
	check_killer(ch,victim);
}


void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}

    if (IS_AFFECTED2(ch,AFF_FEAR)){
	send_to_char("You are too scared to fight!\n\r",ch);
	return;
	}

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg, TRUE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    if ( victim == ch )
    {
	send_to_char( "You hit yourself.  Ouch!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if (IS_AFFECTED2(victim,AFF_GHOST)){
	send_to_char("Your weapon passes right through them!\n\r",ch);
	return;
	}

    if ( is_safe( ch, victim ) )
	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    send_to_char( "Murder whom?\n\r", ch );
    return;
    }

    if ( ( victim = get_char_room( ch, arg, TRUE ) ) == NULL )
    {
    send_to_char( "They aren't here.\n\r", ch );
    return;
    }

    if ( victim == ch )
    {
    send_to_char( "Suicide is a mortal sin.\n\r", ch );
    return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "You can't murder them!\n\r", ch );
        return;
    }

    if (IS_NPC(ch) && IS_AFFECTED(ch,AFF_CHARM)) {
       act( "$n says {s'Do your own dirty work!'{x", ch, NULL, victim, TO_ROOM);
       return;	
      }

    if ( IS_SET(victim->in_room->room_flags,ROOM_SAFE)
    ||   (IS_SET(victim->in_room->room_flags,ROOM_SEMISAFE) 
           && !in_fightlag(victim) ) )
     {
        send_to_char("You cannot murder anyone in this sanctuary!\n\r",ch);
        return;
     }

    if (victim->position != POS_UNCONSCIOUS){
        send_to_char( "You can't murder a conscious person!\n\r", ch );
        return;
        }
    if((IS_SET(victim->act_bits,PLR_QUEST)) && IS_SET(queststatus,QUEST_NO_MURDER))
	{
	send_to_char("You cannot murder a questor!\n\r",ch);
        return;
        }
    if((IS_SET(victim->act_bits,PLR_QUEST)) && IS_SET(queststatus,QUEST_ALWAYS_KO))
	{
	send_to_char("You cannot murder a questor!\n\r",ch);
        return;
        }
    

    act( "You murder $M in cold blood!",  ch, NULL, victim, TO_CHAR    );
    act_new( "$n murders you in cold blood!",ch,NULL,victim,TO_VICT,POS_DEAD, 0 );
    act( "$n murders $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    raw_kill( victim );

    sprintf( log_buf, "%s killed by %s at %d",
        victim->name,
        (IS_NPC(ch) ? ch->short_descr : ch->name),
        ch->in_room->vnum );

    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

    return;

}

void do_scalp( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Scalp whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg, TRUE ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "That sounds painful.\n\r", ch );
        return;
    }

    if ( IS_NPC(ch) ) 
    {
       send_to_char( "Silly mobile, scalping is for players!\n\r", ch );
       return;
    }

    if (victim->position != POS_UNCONSCIOUS)
    {
        send_to_char( "You can't scalp a conscious person!\n\r", ch );
        return;
    }

    /* I don't know if NPCs will ever be unconscious, but just in case */
    if ( IS_NPC(victim) )
    {
        act( "You can't scalp $M!", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( IS_SET(victim->in_room->room_flags,ROOM_SAFE)
    ||   (IS_SET(victim->in_room->room_flags,ROOM_SEMISAFE) 
           && !in_fightlag(victim) ) )
    {
        act( "You can't scalp $M in this sanctuary!", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( ( IS_SET(victim->act_bits,PLR_QUEST) && IS_SET(queststatus,QUEST_NO_MURDER) )
    ||   ( IS_SET(victim->act_bits,PLR_QUEST) && IS_SET(queststatus,QUEST_ALWAYS_KO) ) )
    {
        send_to_char("You cannot scalp a Questor!\n\r",ch);
        return;
    }

    act( "You tear the scalp from $S head, killing $M!",  ch, NULL, victim, TO_CHAR    );
    act_new( "$n rips your scalp from your head, killing you!",ch,NULL,victim,TO_VICT,POS_DEAD,0 );
    act( "$n tears the scalp from $N's head, killing $M!",  ch, NULL, victim, TO_NOTVICT );
    raw_kill( victim );

    obj = create_object( get_obj_index( OBJ_VNUM_SCALP ), 0 );

    sprintf( buf, obj->short_descr, victim->name );
    free_string( obj->short_descr );
    obj->short_descr = str_dup( buf );

    sprintf( buf, obj->description, victim->name );
    free_string( obj->description );
    obj->description = str_dup( buf );

    sprintf( buf, obj->name, victim->name );
    free_string( obj->name );
    obj->name = str_dup( buf );

    obj->level = victim->level;

    obj_to_char( obj, ch );

    sprintf( log_buf, "%s scalped by %s at %d",
        victim->name,
        (IS_NPC(ch) ? ch->short_descr : ch->name),
        ch->in_room->vnum );

    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

    return;

}

void do_backstab( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    one_argument( argument, arg );

    if (arg[0] == '\0')
    {
        send_to_char("Backstab whom?\n\r",ch);
        return;
    }

    if (ch->fighting != NULL)
    {
	send_to_char("You're facing the wrong end.\n\r",ch);
	return;
    }
 
    else if ((victim = get_char_room(ch,arg, TRUE)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
      {
	send_to_char( "And stab your beloved master in the back?\n\r", ch );
	return;
      }

    if ( is_safe( ch, victim ) )
      return;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
	return;
    }

    if ( victim->hit < (victim->max_hit*4/5))
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[gsn_backstab].beats );
    if ( number_percent( ) < get_skill(ch,gsn_backstab)
    || ( !IS_AWAKE(victim) ) )
    {
       if (get_skill(ch,gsn_backstab >=2))
	check_improve(ch,gsn_backstab,TRUE,1);

	multi_hit( ch, victim, gsn_backstab );
    }
    else
    {
	check_improve(ch,gsn_backstab,FALSE,1);
	damage( ch, victim, 0, gsn_backstab,DAM_NONE,TRUE);
    }

    return;
}



void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    if ( ( victim = ch->fighting ) == NULL )
    {
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	||   (IS_SET(pexit->exit_info, EX_CLOSED) &&
             !(IS_AFFECTED(ch,AFF_PASS_DOOR)))
	||   number_range(0,ch->daze) != 0
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) 
	|| (IS_AFFECTED2(ch, AFF_WEB) && !break_web(ch)) )
	    continue;

	move_char( ch, door, FALSE );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n has fled!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC(ch) )
	{
	    send_to_char( "You flee from combat!\n\r", ch );
	if( (ch->class == 2) 
	    && (number_percent() < 3*(ch->level/2) ) )
		send_to_char( "You snuck away safely.\n\r", ch);
	else
	    {
	    send_to_char( "You lost 10 exp.\n\r", ch); 
	    gain_exp( ch, -10 );
	    }
	}

	stop_fighting( ch, TRUE );
	return;
    }

    if (IS_AFFECTED2(ch, AFF_WEB)) {
	WAIT_STATE(ch, 12);
	send_to_char("You struggle but you can't break free!\n\r",ch);
	act("$n struggles in $s webs.",ch,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char( "PANIC! You couldn't escape!\n\r", ch );
    return;
}



void do_rage( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
       send_to_char("You can't rage as a ghost, your DEAD!\n\r",ch);
       return;
       }

    if ((chance = get_skill(ch,gsn_rage)) == 0
    || (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    || (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_rage].skill_level [ch->class]))
    {
        send_to_char("You turn red in the face, but you just cant feel the rage.\n\r",ch);
       return;
    }

    if (IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,gsn_battle)
    ||  is_affected(ch,skill_lookup("frenzy")))
    {
        send_to_char("You rage a little harder.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
        send_to_char("You're feeling too calm to rage.\n\r",ch);
        return;
    }

    if (ch->mana < 50)
    {
        send_to_char("You can't get up enough energy to rage.\n\r",ch);
        return;
    }

    /*modifiers*/

    /*fighting*/
    if (ch->position == POS_FIGHTING)
        chance += 10;

    /*damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
        AFFECT_DATA af;

        WAIT_STATE(ch,PULSE_VIOLENCE);
        ch->mana -= 50;

        /*heal a little damage*/
        ch->hit += ch->level * 2;
        ch->hit = UMIN(ch->hit,ch->max_hit);

        send_to_char("Fury consumes you and rage permeates your being!\n\r",ch);
        act("$n is consumed by rage!",ch,NULL,NULL,TO_ROOM);
        check_improve(ch,gsn_rage,TRUE,2);

        af.where     = TO_AFFECTS;
        af.type      = gsn_rage;
        af.level     = ch->level;
        af.duration  = number_fuzzy(ch->level / 5);
        af.modifier  = UMAX(1, ch->level/5);
        af.bitvector = AFF_BERSERK;

        af.location  = APPLY_HITROLL;
        affect_to_char(ch,&af);

        af.location  = APPLY_DAMROLL;
        affect_to_char(ch,&af);

        af.modifier  = number_fuzzy(ch->level / 12);
        af.bitvector = AFF_HASTE;
        af.location  = APPLY_DEX;
        affect_to_char(ch,&af);
    }

    else
    {
        WAIT_STATE(ch,PULSE_VIOLENCE);
        ch->mana -= 25;

        send_to_char("You can't seem to find your rage.\n\r",ch);
        check_improve(ch,gsn_rage,FALSE,2);
    }
}
 
void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg, TRUE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }


    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( number_percent( ) > get_skill(ch,gsn_rescue))
    {
	send_to_char( "You fail the rescue.\n\r", ch );
	check_improve(ch,gsn_rescue,FALSE,1);
	return;
    }

    act( "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n rescues you!", ch, NULL, victim, TO_VICT    );
    act( "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );
    check_improve(ch,gsn_rescue,TRUE,1);

    stop_fighting( victim, FALSE ); /* Remove the rescued person from combat with
                                       the fch */
    stop_fighting( fch, FALSE );    /* Clean break the fight so fch attacks ch, */
    stop_fighting( ch, FALSE );     /* ch fights fch */
    set_fighting( ch, fch );
    set_fighting( fch, ch );

/*  This was all rewritten for this mud and it doesn't make any sense.
    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );

    check_killer( ch, fch );
    if ( ch->fighting != NULL )
        set_fighting( ch, fch );
    if ( fch->fighting != NULL )
        set_fighting( fch, ch );
*/
    return;
}



void do_kick( CHAR_DATA *ch, char *argument )
{
  char victim_name[ MAX_INPUT_LENGTH ] = "";
  CHAR_DATA *victim;
  int kick_damage = 0;

  if( IS_AFFECTED2(ch,AFF_GHOST) )
    {
      send_to_char("You're a ghost!\n\r",ch);
      return;
    }

  if( !IS_NPC(ch) &&
      ch->level < skill_table[gsn_kick].skill_level[ch->class] )
    {
      send_to_char( "You better leave the martial arts to fighters.\n\r", ch );
      return;
    }

  if( IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK) )
    return;
  
      argument = one_argument( argument, victim_name );

  if( IS_NULLSTR(victim_name) )
  {
    if( ch->fighting == NULL )
    {
      send_to_char( "You must specify a target to start combat with a kick.\n\r", ch );
      return;
    }
    else
      victim = ch->fighting;
  }
  else
  {
    if ((victim = get_char_room(ch, victim_name, TRUE)) == NULL )
    {
      send_to_char( "That person isn't here.\n\r", ch );
      return;
    }
  }

  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
  {
      act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
      return;
  }

    if (is_safe(ch,victim))
        return;

  if( get_skill(ch,gsn_kick) > number_percent() )
      kick_damage = number_range( 3, ( 2 * ch->level ) );
  else
      kick_damage = 0;

  damage( ch, victim, kick_damage, gsn_kick, DAM_BASH, TRUE );

  if ( kick_damage > 0 
  &&   !IS_NPC( ch )
  &&   victim->hit > 1 
  &&   victim->position > POS_UNCONSCIOUS 
  &&   victim->hit < victim->max_hit / 2 )
  {
      if ( !IS_NPC( victim ) || victim->desc != NULL )
          victim->move = UMAX(victim->move-30,0);
      else
          victim->kicktimer = 1;
      send_to_char( "You see your victim's muscles bruise from the blow!\n\r", 
          ch );
      send_to_char( "Your muscles begin to bruise and stiffen!\n\r", victim );
  }

  check_improve( ch, gsn_kick, (kick_damage > 0) ? TRUE : FALSE, 1 );

  WAIT_STATE( ch, skill_table[ gsn_kick ].beats );
  check_killer(ch,victim);
  return;
}

void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    hth = 0;

    if ((chance = get_skill(ch,gsn_disarm)) == 0)
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL 
    &&   ((hth = get_skill(ch,gsn_hand_to_hand)) == 0
    ||    (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_DISARM))))
    {
	send_to_char( "You must wield a weapon to disarm.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
	send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
	return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim));

    /* modifiers */

    /* skill */
    if ( get_eq_char(ch,WEAR_WIELD) == NULL)
	chance = chance * hth/150;
    else
	chance = chance * ch_weapon/100;

    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

    /* dex vs. strength */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;
 
    /* and now the attack */
    if (number_percent() < chance)
    {
    	WAIT_STATE(ch, PULSE_VIOLENCE );
	disarm( ch, victim );
	check_improve(ch,gsn_disarm,TRUE,1);
    }
    else
    {
	WAIT_STATE(ch,PULSE_VIOLENCE);
	act("You fail to disarm $N.",ch,NULL,victim,TO_CHAR);
	act("$n tries to disarm you, but fails.",ch,NULL,victim,TO_VICT);
	act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_disarm,FALSE,1);
    }
    check_killer(ch,victim);
    return;
}

void do_dislodge( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance,hth,ch_shield,vict_shield,ch_vict_shield;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    hth = 0;

    if ((chance = get_skill(ch,gsn_dislodge)) == 0)
    {
	send_to_char( "You don't know how to dislodge a shield.\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL 
    &&   ((hth = get_skill(ch,gsn_hand_to_hand)) == 0))
    {
	send_to_char( "You must wield a weapon to dislodge.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_SHIELD ) ) == NULL )
    {
	send_to_char( "Your opponent is not wearing a shield.\n\r", ch );
	return;
    }

    /* find weapon skills */
    ch_shield = get_weapon_skill(ch,get_weapon_sn(ch));
    vict_shield = get_weapon_skill(victim,get_weapon_sn(victim));
    ch_vict_shield = get_weapon_skill(ch,get_weapon_sn(victim));

    /* modifiers */

    /* skill */
    if ( get_eq_char(ch,WEAR_SHIELD) == NULL)
	chance = chance * hth/150;
    else
	chance = chance * ch_shield/100;

    chance += (ch_vict_shield/2 - vict_shield) / 2; 

    /* dex vs. strength */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;
 
    /* and now the attack */
    if (number_percent() < chance)
    {
    	WAIT_STATE( ch, PULSE_VIOLENCE );
	dislodge( ch, victim );
	check_improve(ch,gsn_dislodge,TRUE,1);
    }
    else
    {
	WAIT_STATE(ch,PULSE_VIOLENCE);
	act("You fail to dislodge $N's shield.",ch,NULL,victim,TO_CHAR);
	act("$n tries to dislodge your shield, but fails.",ch,NULL,victim,TO_VICT);
	act("$n tries to dislodge $N's shield, but fails.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_dislodge,FALSE,1);
    }
    check_killer(ch,victim);
    return;
}

void do_surrender( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    if ( (mob = ch->fighting) == NULL )
    {
	send_to_char( "But you're not fighting!\n\r", ch );
	return;
    }
    act( "You surrender to $N!", ch, NULL, mob, TO_CHAR );
    act( "$n surrenders to you!", ch, NULL, mob, TO_VICT );
    act( "$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT );
    stop_fighting( ch, TRUE );

    if ( !IS_NPC( ch ) && IS_NPC( mob ) 
    &&   ( !HAS_TRIGGER( mob, TRIG_SURR ) 
        || !mp_percent_trigger( mob, ch, NULL, NULL, TRIG_SURR ) ) )
    {
	act( "$N seems to ignore your cowardly act!", ch, NULL, mob, TO_CHAR );
	multi_hit( mob, ch, TYPE_UNDEFINED );
    }
}

void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->level >= get_trust(ch) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR    );
    act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
    act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    raw_kill( victim );
/*  victim->fightlag = 0;
    if ( !IS_NPC(victim) )
        victim->pcdata->lastpk = 0; */
    if ( !IS_NPC(victim) )
    {
      victim->pcdata->lastfight = current_time - FIGHT_LAG;
      victim->pcdata->lastpk = current_time - FIGHT_LAG;
    }
    return;
}




void do_crane_kick( CHAR_DATA *ch, char *argument ) {
  CHAR_DATA *victim;
  int chance;
  int mana_tap = 0;
  int i;

  if ( !IS_NPC(ch) &&
       ch->level < skill_table[gsn_crane_kick].skill_level[ch->class] )
    {
      send_to_char("You better leave the martial arts to monks.\n\r", ch);
      return;
    }

  if ( (chance = get_skill(ch, gsn_crane_kick)) < 1 ) {
    send_to_char("You don't know how to do that.\n\r", ch);
    return;
  }

  if (IS_AFFECTED2(ch,AFF_GHOST)) {
    send_to_char("You're a ghost!\n\r", ch);
    return;
  }
  if ( ( victim = ch->fighting ) == NULL )
    {
      if (argument[0] == '\0') {
	send_to_char("Who do you want to kick?\n\r",ch);
	return;
      }
      else if ( (victim = get_char_room(ch, argument, TRUE)) == NULL ) 
	{
	  send_to_char("They aren't here.\n\r", ch );
	  return;
	}
    }

  if (IS_AFFECTED2(victim, AFF_GHOST)) {
    send_to_char("Your foot passes right through them!\n\r", ch);
    return;
  }

  if ( is_safe( ch, victim ) )
    return;

  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
      act("$N is your beloved master.", ch, NULL, victim, TO_CHAR );
      return;
    }
  chance /= 2;
  chance += get_curr_stat(ch, STAT_DEX) * 2;
  chance -= 10;

  WAIT_STATE( ch, skill_table[gsn_crane_kick].beats );
  if ( chance > number_percent()) {
    damage(ch, victim, target_damage( 35, ch->level + get_curr_stat(ch, STAT_STR)*3 ),
	   gsn_crane_kick, DAM_BASH, TRUE);
    check_improve(ch, gsn_crane_kick, TRUE, 2);

    for( i = 5; i > 0; i-- )
      {
	if( victim->hit < ( victim->max_hit * i / 10 ) )
	  {
	    mana_tap = 40 - 5 * ( i - 1 );
	  }
      }

    if( mana_tap > 0 )
      {
	victim->mana -= mana_tap;
	
	if( victim->mana < 0 )
	  victim->mana = 0;

	send_to_char( "Your kick drained some of your victim's magical energy!\n\r", ch );
	send_to_char( "You feel your magical energy ebbing away!\n\r", victim );
      }
    
      
  }
  else {
    damage( ch, victim, 0, gsn_crane_kick, DAM_BASH, TRUE);
    check_improve(ch, gsn_crane_kick, FALSE, 1);
  }
  return;
}

/* Function to reverse the damage reduction
   that the damage function does.  Useful
   for knowing what damage you are coding into
   skills.
   Kyuss 12/18/98
   */
int target_damage(int low, int high) {

  int result;
  result = number_range(low, high);

  if (result > 35)
    result = 2 * (result - 35) + 35;
  else if (result > 80)
    result = 2 * (result - 80) + 80;

  return result;
}


void do_burning_palm( CHAR_DATA *ch, char *argument ) 
{
   CHAR_DATA *victim;
    int chance, dam;

    if ( (!IS_NPC(ch) 
    &&   ch->level < skill_table[gsn_burning_palm].skill_level[ch->class])
    ||   (chance = get_skill(ch, gsn_burning_palm)) < 1 )
    {
        send_to_char("You don't know how to do that.\n\r", ch);
        return;
    }

    if ( IS_AFFECTED2(ch,AFF_GHOST) ) 
    {
        send_to_char("You're a ghost!\n\r", ch);
        return;
    }
    
    if ( ch->mana < 15) 
    {
        send_to_char("You don't have enough mana.\n\r",ch);
        return;
    }

    if (*argument == '\0') 
    {
        if ( ( victim = ch->fighting ) == NULL )
        {
            send_to_char("Who do you want to punch?\n\r",ch);
            return;
        }
    }
    else if ( (victim = get_char_room(ch, argument, TRUE)) == NULL ) 
    {
        send_to_char("They aren't here.\n\r", ch );
        return;
    }
 
    if ( IS_AFFECTED2(victim, AFF_GHOST) ) 
    {
        act( "Your hand passes right through $M!", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( is_safe( ch, victim ) )
        return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
        act("$N is your beloved master.", ch, NULL, victim, TO_CHAR );
        return;
    }

    chance += (get_curr_stat(ch, STAT_DEX) - get_curr_stat(victim, STAT_DEX)) * 2;
    chance += ch->hitroll;

    ch->mana -= 15;
    WAIT_STATE( ch, 8 );
    if ( chance > number_percent() )
    {
        dam = target_damage( (ch->level/2 + 7)*3, 2 );
        damage( ch, victim, dam, gsn_burning_palm, DAM_BASH, TRUE );
        check_improve(ch, gsn_burning_palm, TRUE, 2);

        if (number_percent() < 90 
        && !saves_spell(ch->level, NULL, victim, DAM_FIRE)) 
        {
            char godname[24];
            int chgod;
            if ( IS_NPC( ch ) )
                chgod = GOD_MOB;
            else
                chgod = ch->pcdata->god;
            strcpy(godname,god_table[chgod].descr);

            act("You invoke the power of $t to burn $N with your palm!",
                ch, godname, victim, TO_CHAR );
            act("$n invokes the power of $t to burn $N with $s palm!", 
                ch, godname, victim, TO_NOTVICT );
            act("$n invokes the power of $t to burn you with $s palm!",
                ch, godname, victim, TO_VICT );
            fire_effect( victim, ch->level/2, dam/4, TARGET_CHAR );
            damage( ch, victim, dam/2, gsn_burning_palm, DAM_FIRE, TRUE );
            check_improve( ch, gsn_burning_palm, TRUE, 1 );
        }
    }
    else 
    {
        damage( ch, victim, 0, gsn_burning_palm, DAM_BASH, TRUE);
        check_improve(ch, gsn_burning_palm, FALSE, 1);
    }
    return;
}

void do_throw( CHAR_DATA *ch, char *argument ) 
{
    CHAR_DATA *victim;
    int chance, dam;
    bool wyrm = FALSE; /* questmobs can use wyrm venom as a skill */
 
    if ( !IS_NPC(ch) 
    &&   ch->level < skill_table[gsn_throw].skill_level[ch->class] )
    {
        send_to_char("You dont know how to do that.\n\r", ch);
        return;
    }

    if ( IS_NPC(ch) && IS_IMMORTAL(ch) )
        wyrm = TRUE;

/* need to have taken poison dagger OR be a mob+be an imm*/
    if ( (chance = get_skill(ch, gsn_throw)) < 1 && !wyrm )
    {
        send_to_char("You don't know how to do that.\n\r", ch);
        return;
    }

    if (IS_AFFECTED2(ch,AFF_GHOST)) 
    {
        send_to_char("You're a ghost!\n\r", ch);
        return;
    }
    
    if (ch->mana < 15) 
    {
        send_to_char("You don't have enough mana.\n\r",ch);
        return;
    }

    if ( IS_NULLSTR(argument) ) 
    {
        if ( ( victim = ch->fighting ) == NULL )
        {
            send_to_char( "Who do you want to throw your dagger at?\n\r", ch );
            return;
        }
    }
    else if ( (victim = get_char_room(ch, argument, TRUE)) == NULL ) 
    {
        send_to_char("They aren't here.\n\r", ch );
        return;
    }
 
    if (IS_AFFECTED2(victim, AFF_GHOST)) 
    {
        act("Your dagger would pass right through $M!",ch,NULL,victim,TO_CHAR);
        return;
    }

    if ( is_safe( ch, victim ) )
        return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
        act("$N is your beloved master.", ch, NULL, victim, TO_CHAR );
        return;
    }

 /* chance /= 3;*/
    chance += 2*(get_curr_stat(ch, STAT_DEX) - get_curr_stat(victim, STAT_DEX));
    chance += ch->hitroll;

    ch->mana -= 15;
    WAIT_STATE( ch, 8 );

/* special code for wyrm venom */
    if ( wyrm )
    {
        dam = target_damage((ch->level/2)*3, 2);
        damage_old(ch,victim,dam*2, gsn_throw, DAM_POISON, TRUE);
        spell_wyrm_venom(gsn_wyrm_venom,ch->level,ch,(void*)victim,TARGET_CHAR);
        if ( global_config_strongwyrmpoison )
            victim->hit = 1;
        return;
    }

    if ( chance > number_percent() )
    {
        dam = target_damage((ch->level/2)*3, 2);
        damage(ch, victim, dam, gsn_throw, DAM_POISON, TRUE);
        check_improve(ch, gsn_throw, TRUE, 2);
        if (number_percent() < 90 
        && !saves_spell(ch->level, NULL, victim, DAM_POISON)) 
        {
            AFFECT_DATA af;

            af.where     = TO_AFFECTS;      
            af.type      = gsn_poison;
            af.level     = ch->level;
            af.duration  = 10;
            af.location  = APPLY_STR;
            af.modifier  = -2;
            af.bitvector = AFF_POISON;
            affect_join( victim, &af, TRUE );
 
	        damage_old(ch,victim,dam/2, gsn_throw, DAM_POISON, TRUE);
     	    act("You are poisoned by $n's tiny dagger!",ch,NULL,victim,TO_VICT);
	        act("You poison $N with your tiny dagger!",ch,NULL,victim,TO_CHAR);
            check_improve(ch,gsn_throw, TRUE,1);
        }
    }
    else 
    {
        damage( ch, victim, 0, gsn_throw, DAM_PIERCE, TRUE);
        check_improve(ch, gsn_throw, FALSE, 1);
    }
    return;
}

void do_sap( CHAR_DATA *ch, char *argument ) 
{
  CHAR_DATA *victim;
  int chance, dam;
 
  if ( !IS_NPC(ch) &&
       ch->level < skill_table[gsn_sap].skill_level[ch->class] )
    {
      send_to_char("You dont know how to do that.\n\r", ch);
      return;
    }

  if ( (chance = get_skill(ch, gsn_sap)) < 1 ) 
    {
      send_to_char("You don't know how to do that.\n\r", ch);
      return;
    }

  if (IS_AFFECTED2(ch,AFF_GHOST)) 
    {
      send_to_char("You're a ghost!\n\r", ch);
      return;
    }
    
  if (ch->mana < 15) 
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }

  if (*argument == '\0') 
    {
      if ( ( victim = ch->fighting ) == NULL )
	{
	  send_to_char("Who do you want to sap?\n\r",ch);
	  return;
	}
    }
  else if ( (victim = get_char_room(ch, argument, TRUE)) == NULL ) 
    {
      send_to_char("They aren't here.\n\r", ch );
      return;
    }
 
  if (IS_AFFECTED2(victim, AFF_GHOST)) 
    {
      send_to_char("Your hand passes right through them!\n\r", ch);
      return;
    }

  if ( is_safe( ch, victim ) )
    return;

  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
      act("$N is your beloved master.", ch, NULL, victim, TO_CHAR );
      return;
    }

  ch->mana -= 15;
  WAIT_STATE( ch, 16 );

  chance = number_range( 0, 100 );

  if( saves_spell( ch->level, NULL, victim, DAM_BASH ) && chance > 20 )
    {
      act( "You fail to sap your victim.", ch, 0, 0, TO_CHAR );
      return;
    }

  if( IS_AFFECTED( victim, AFF_SLEEP ) )
    {
      if( IS_AWAKE( victim ) )
	{
	  /* deflection */
	  act("Your victim is aware of your attack and deflects the blow to $S head!\n\r", ch, NULL, victim, TO_CHAR );
	  dam = target_damage((ch->level/2)*3, 2);   
	  damage(ch, victim, dam, gsn_sap, DAM_BASH, TRUE);
	  check_improve(ch, gsn_sap, TRUE, 2);
/*        victim->fightlag = FIGHT_LAG; */
	  if ( ch != victim && !IS_NPC(ch) && !IS_NPC(victim) ) 
              victim->pcdata->lastpk = current_time;
	}
      else
	{
	  /* victim already unconscious */
	  send_to_char( "Your victim is already unconscious.\n\r", ch );
	}

      return;
    }
  else
    {
      AFFECT_DATA af;

      af.where     = TO_AFFECTS;   
      af.type      = gsn_sap;
      af.level     = ch->level;
      af.duration  = 1;
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = AFF_SLEEP;
      affect_to_char( victim, &af );

      af.modifier  = -5;
      af.location  = APPLY_DEX;
      affect_to_char( victim, &af );
 
      af.modifier  = 2;
      af.location  = APPLY_SAVES;
      affect_to_char(victim, &af); 

      send_to_char( "You feel a knock to your head and black out.\n\r",victim );
      act( "$n dizzies and passes out.", victim, NULL, NULL, TO_ROOM );
      victim->position = POS_SLEEPING;	
/*    victim->fightlag = FIGHT_LAG; */
      if ( ch != victim )
      { 
          if ( !IS_NPC(victim) )
          {
              victim->pcdata->lastfight = current_time;
              if ( !IS_NPC(ch) ) 
                  victim->pcdata->lastpk = current_time;
          }
          if ( !IS_NPC( ch ) )
              ch->pcdata->lastfight = current_time;
      }
      check_improve(ch,gsn_sap, TRUE,2);
    }
}

void  do_vital_strike( CHAR_DATA *ch, char *argument ) {
  CHAR_DATA *victim;
  int chance;

  if ( !IS_NPC(ch) &&
       ch->level < skill_table[gsn_vital_strike].skill_level[ch->class] )
  {
    send_to_char("You better leave the martial arts to monks.\n\r", ch);
    return;
  }

  if ( (chance = get_skill(ch, gsn_vital_strike)) < 1 ) {
    send_to_char("You don't know how to do that.\n\r", ch);
    return;
  }

  if (IS_AFFECTED2(ch,AFF_GHOST)) {
      send_to_char("You're a ghost!\n\r", ch);
      return;
  }
    
  if (*argument == '\0') {
     if ( ( victim = ch->fighting ) == NULL )
     {
        send_to_char("Who do you want to strike?\n\r",ch);
        return;
     }
  }
  else if ( (victim = get_char_room(ch, argument, TRUE)) == NULL ) 
  {
     send_to_char("They aren't here.\n\r", ch );
     return;
  }

  if (IS_AFFECTED2(victim, AFF_GHOST)) {
      send_to_char("Your hand passes right through them!\n\r", ch);
      return;
  }

  if ( is_safe( ch, victim ) )
    return;

  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
  {
      act("$N is your beloved master.", ch, NULL, victim, TO_CHAR );
      return;
  }
  
  if ( is_affected(victim, gsn_vital_strike) )
  {
      send_to_char("They are already numb.\n\r",ch);
      return;
  }

  chance +=(get_curr_stat(ch, STAT_DEX) - get_curr_stat(victim, STAT_DEX)) * 2;
  if (IS_AFFECTED(ch, AFF_HASTE) || IS_SET(ch->off_flags, OFF_FAST))
    chance += 10;
  if (IS_AFFECTED(victim, AFF_HASTE) || IS_SET(victim->off_flags, OFF_FAST))
    chance -= 20;

  chance -= GET_AC(victim, AC_BASH)/15;
  chance += ch->level - victim->level;
  if (victim->fighting == NULL)
    chance += 10;
  else
    chance -= 20;

  WAIT_STATE( ch, skill_table[gsn_vital_strike].beats );
  WAIT_STATE( victim, 12 );
  if ( chance > number_percent()) {
      AFFECT_DATA af;

      af.where     = TO_AFFECTS;
      af.type      = gsn_vital_strike;
      af.level     = ch->level;
      af.duration  = (ch->level > 25) ? 2 : 1;
      af.location  = APPLY_AC;
      af.modifier  = 2 * ch->level;

      af.bitvector = 0;
      affect_to_char(victim, &af);

      af.location  = APPLY_HITROLL;
      af.modifier  = ch->level / -8;
      affect_to_char(victim, &af);
      act("$n strikes you and you begin to lose feeling in your limbs.",
	  ch, NULL, victim, TO_VICT);
      act("You strike $N, hitting vital nerves.", ch, NULL, victim, TO_CHAR);
      damage( ch, victim, number_range(ch->level/2, 3 * ch->level / 2),
	      gsn_vital_strike, DAM_BASH, TRUE);
      check_improve(ch, gsn_vital_strike, TRUE, 2);
  }
  else {
      damage( ch, victim, 0, gsn_vital_strike, DAM_BASH, TRUE);
      check_improve(ch, gsn_vital_strike, FALSE, 1);
  }
  return;
}


void do_armthrow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance,victim_skill;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_armthrow)) == 0
    ||   (!IS_NPC(ch) 
	  && ch->level <
    skill_table[gsn_armthrow].skill_level[ch->class]))
    {
	send_to_char("Throwing?  What's that?\n\r",ch);
	return;
    }


    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,arg, TRUE)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (victim->position < POS_FIGHTING)
    {
	act("$N is already down.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Where would you like to throw yourself?\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if(IS_SET(victim->imm_flags,IMM_LAGSKILLS)) {
       act("You can't seem to knock $M over!",ch,NULL,victim,TO_CHAR);
       return;
}

    /* modifiers */
    victim_skill = get_skill(victim,gsn_armthrow);
    chance -= victim_skill / 4;

    /* size vs. strength */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10 +
		  get_curr_stat(ch, STAT_STR) / 2;

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX);

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 15;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* flying */
    if (IS_AFFECTED(ch,AFF_FLYING))
	chance -= 15;

    /* now the attack */
    if (number_percent() < chance)
    {
	act("With lightning quick speed, $n throws you to the ground!",
	ch,NULL,victim,TO_VICT);
	act("With lightning quick speed, you throw $N to the ground!",
	ch,NULL,victim,TO_CHAR);
	act("$n throws $N to the ground.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_armthrow,TRUE,1);

        WAIT_STATE(victim, (PULSE_VIOLENCE*2));
        WAIT_STATE(ch, ((PULSE_VIOLENCE*2)+9));
	damage(ch,victim,number_range(2, 2 +  2 * victim->size),
		gsn_armthrow, DAM_BASH,TRUE);

        /* damage() can cause wimpy which changes victim->in_room */
        if (victim->position > POS_FLATFOOTED && victim->in_room == ch->in_room)
            victim->position = POS_SPRAWLED;
    }
    else
    {
	if (number_percent() < victim_skill) {
	  act("$N counters your throw, and you get thrown yourself!",
	      ch, NULL, victim, TO_CHAR);
	  act("You catch $n's armthrow and throw $m to the ground instead.",
	      ch, NULL, victim, TO_VICT);
	  act("$N throws $n to the ground.", ch,NULL,victim,TO_ROOM);
  	  damage(victim,ch,number_range(2, 2 * ch->size),
	         gsn_armthrow,DAM_BASH,TRUE);
	  WAIT_STATE(ch,24);
	  check_improve(ch,gsn_armthrow,FALSE,1);
        }
	else {
	   damage(ch,victim,0,gsn_armthrow,DAM_BASH,TRUE);
	   WAIT_STATE(ch,12);
	   check_improve(ch,gsn_armthrow,FALSE,1);
	}
    } 
	check_killer(ch,victim);
}

void do_entangle( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance,victim_skill;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_entangle)) == 0
    ||   (!IS_NPC(ch) 
	  && ch->level < skill_table[gsn_entangle].skill_level[ch->class]))
    {
	send_to_char("The flora refuses to listen to you.\n\r",ch);
	return;
    }


    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,arg, TRUE)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (victim->position < POS_FIGHTING)
    {
	act("$N is already down.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("The plants refuse to entangle you!\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if(IS_SET(victim->imm_flags,IMM_LAGSKILLS)) {
       act("You can't seem to knock $M over!",ch,NULL,victim,TO_CHAR);
       return;
   } 
   /* modifiers */
    victim_skill = get_skill(victim,gsn_entangle);
    chance -= victim_skill / 4;

    /* size vs. strength */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10 +
		  get_curr_stat(ch, STAT_STR) / 2;

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX);

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 15;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* flying */
    if (IS_AFFECTED(ch,AFF_FLYING))
	chance -= 10;

    /* now the attack */
    if (number_percent() < chance)
    {
	act("The plants rise up and entangle your feet!",
	ch,NULL,victim,TO_VICT);
	act("The plants rise up and entangle the feet of your foe!",
	ch,NULL,victim,TO_CHAR);
	act("$N falls to the ground entangled in plantlife.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_entangle,TRUE,1);

        WAIT_STATE(victim, PULSE_VIOLENCE*2);   
        WAIT_STATE(ch,PULSE_VIOLENCE*2+4);
        damage(ch,victim,number_range(4, 2 +  2 *victim->size + chance),
            gsn_entangle, DAM_ENERGY,TRUE);

        /* damage() can cause wimpy which changes victim->in_room */
        if (victim->position > POS_FLATFOOTED && victim->in_room == ch->in_room)
            victim->position = POS_SPRAWLED;
    
    }
    else
    {
	if (number_percent() < victim_skill) {
	  act("You trip on one of your own vines!",
	      ch, NULL, victim, TO_CHAR);
	  act("$n trips over an overgrown vine!",
	      ch, NULL, victim, TO_VICT);
	  act("$n is entangled in an overgrown vine.",ch,NULL,victim,TO_ROOM);
  	  damage(victim,ch,number_range(2, 2 * ch->size),
	         gsn_entangle,DAM_BASH,TRUE);
	  WAIT_STATE(ch, PULSE_VIOLENCE );
	  check_improve(ch,gsn_entangle,FALSE,1);
        }
	else {
	   damage(ch,victim,0,gsn_entangle,DAM_ENERGY,TRUE);
	   WAIT_STATE(ch, PULSE_VIOLENCE );
	   check_improve(ch,gsn_entangle,FALSE,1);
	}
    } 
	check_killer(ch,victim);
}

bool break_web(CHAR_DATA *ch) {

    AFFECT_DATA *paf;
    int chance;
    if ((paf = affect_find(ch->affected, skill_lookup("web"))) == NULL) 
	return FALSE;

    chance = get_curr_stat(ch, STAT_STR) + get_curr_stat(ch, STAT_DEX) +
	ch->level / 2;

    chance = URANGE(paf->level / 2, chance, 95);
    if (number_range(5, paf->level * 2) < chance) {
	send_to_char("You break free of the webs!\n\r",ch);
	act("$n breaks free of the webs!",ch,NULL,NULL, TO_ROOM);
	affect_remove(ch, paf);
	return TRUE;
    }
    else 
	return FALSE;
}

/* Archery -- Bavor*/
void do_fire(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj, *bow, *arrow;
  CHAR_DATA *victim;
  CHAR_DATA *vofi;
  int dam, dam_type;
  int vopos;
  bool found = FALSE;
  char arg0[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  argument = one_argument(argument,arg0);
  argument = one_argument(argument,arg1);
  bow = get_eq_char(ch,WEAR_HOLD);
  /* Non Rangers can't use Archery */
  if(get_skill(ch,gsn_dualwield) < 1)
    {
      send_to_char("Only an experienced ranger can use a bow and arrow\n\r",ch);
      return;
    }
  if (bow == NULL)
    {
      send_to_char("You hold nothing!\n\r",ch);
      return;
    }
  if ( bow->value[0] != WEAPON_BOW )
    {
      send_to_char( "You hold no bow to shoot arrows with.\n\r", ch );
      return;
    }

  if(arg0[0] == '\0')
    {
      /* Scan inventory for arrows */
      for ( obj =ch->carrying; obj != NULL; obj = obj->next_content)
	{
	  if(obj->item_type == ITEM_ARROW)
	    {
	      found = TRUE;
	      arrow = obj;			
	      break;
	    }
	}
      if(!found)
	{
	  send_to_char("You need arrows to fire.\n\r",ch);
	  return;
	}
    } /* Valid arguments, right? Check if they have their arrow */
  if ( ( obj = get_obj_carry( ch, arg0, ch, TRUE ) ) == NULL )
    {
      send_to_char( "You do not have that arrow.\n\r", ch );
      return;
    }
  arrow = obj;
  /* Legitimate ARrow? */
  if(arrow->item_type != ITEM_ARROW)
    {
      send_to_char("You can only fire arrows.\n\r",ch);
      return;
    }
  if(arg1[0] == '\0')
    {
      victim = ch->fighting;
      if (victim == NULL)
	{
	  send_to_char("You aren't fighting anyone!\n\r",ch);
	  return;
	}
    } else if ((victim = get_char_room(ch,arg1, TRUE)) == NULL)
      {
	send_to_char("They aren't here.\n\r",ch);
	return;
      }
  /* Check safety/ghosts/other weirdness */
  if (IS_AFFECTED2(ch,AFF_GHOST)) {
    send_to_char("You're a ghost!\n\r", ch);
    return;
  }

  if (IS_AFFECTED2(victim, AFF_GHOST)) {
    send_to_char("Your hand passes right through them!\n\r", ch);
    return;
  }

  if ( is_safe( ch, victim ) )
    return;

  vofi = victim->fighting;
  vopos = victim->position;

  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
      act("$N is your beloved master.", ch, NULL, victim, TO_CHAR );
      return;
    }
  dam = bow->value[1] * bow->value[2];
  dam += arrow->value[0] * arrow->value[1];
  dam_type = attack_table[arrow->value[3]].damage;
  act( "Pain shoots through your body as an arrow lodges itself in your flesh.", ch, NULL, victim, TO_VICT);
  send_to_char("You release your bowstring, arching the arrow towards its target.\n\r",ch);
/*send_to_char("Pain shoots through your body as an arrow lodges itself in your flesh.\n\r",victim);*/
  act("$n shoots an arrow from $s bow, striking $N.", ch, NULL,
      victim, TO_NOTVICT);
  damage(ch, victim, dam, gsn_fire, DAM_PIERCE, TRUE );
  /* Arrow flags */
  /* Burning */
  if(IS_ARROW_STAT(arrow,ARROW_BURNING))
    {
      act("$n is burned by the arrow!",victim,arrow,NULL,TO_ROOM);
      act("$p sets your skin aflame!",victim,arrow,NULL,TO_CHAR);
      fire_effect( (void *) victim,arrow->level,dam,TARGET_CHAR);
      damage(ch,victim,5,0,DAM_FIRE,FALSE);
    }		
  /* Freezing */
  if(IS_ARROW_STAT(arrow,ARROW_FREEZING))
    {
      act("$n is frozen by the arrow!",victim,arrow,NULL,TO_ROOM);
      act("Your skin begins to freeze.",victim,arrow,NULL,TO_CHAR);
      cold_effect( (void *) victim,arrow->level,dam,TARGET_CHAR);
      damage(ch,victim,5,0,DAM_COLD,FALSE);
    }		
  /* Lightning */
  if(IS_ARROW_STAT(arrow,ARROW_SHOCKING))
    {
      act("$n is shocked by the arrow!",victim,arrow,NULL,TO_ROOM);
      act("You twitch uncontrollably as the arrow shocks you!",victim,arrow,NULL,TO_CHAR);
      shock_effect( (void *) victim,arrow->level,dam,TARGET_CHAR);
      damage(ch,victim,5,0,DAM_LIGHTNING,FALSE);
    }
  /* Armor pierce */
  if(IS_ARROW_STAT(arrow,ARROW_PIERCE))
    {
      AFFECT_DATA af;
      act("$n's armor is pierced by the arrow.",victim,arrow,NULL,TO_ROOM);
      act("Your armor is pierced by the arrow.",victim,arrow,NULL,TO_CHAR);
      af.where     = TO_AFFECTS;   
      af.type      = gsn_armorpierce;
      af.level     = ch->level;
      af.duration  = 20;
      af.location  = APPLY_AC;
      af.modifier  = 10;
      af.bitvector = APPLY_NONE;
      affect_to_char( victim, &af );

    }				
  if(IS_ARROW_STAT(arrow,ARROW_LOVE))
  {
      if ( !affect_find(victim->affected,gsn_love) )
      {
        AFFECT_DATA af;
        act("$n suddenly looks very happy.",victim,arrow,NULL,TO_ROOM);
        act("You feel overwhelmed by good feelings as your heart is pierced by the arrow.",
          victim, arrow,NULL,TO_CHAR);
        af.where     = TO_AFFECTS;   
        af.type      = gsn_love;
        af.level     = ch->level;
        af.duration  = ch->level / 6;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF_SANCTUARY;
        affect_to_char( victim, &af );
        stop_fighting( victim, FALSE );
        stop_fighting( ch, FALSE );
      }
  }
    /* Blunt */
    if ( IS_ARROW_STAT(arrow,ARROW_KNOCKOUT)
    &&   number_range(1,100) < 20 
    &&   victim->position > POS_FLATFOOTED
    &&   victim->in_room == ch->in_room )
    {
        act("$n is knocked over by the arrow!",victim,arrow,NULL,TO_ROOM);
        act("A blow to the chest from the arrow knocks you down!",
            victim,arrow,NULL,TO_CHAR);
        WAIT_STATE(victim, (PULSE_VIOLENCE*2));
        victim->position = POS_SPRAWLED;
    }

  /* Heavy */
  if( IS_ARROW_STAT(arrow,ARROW_SLEEP)
  /* Prevents a KO by sleep arrow from instantly un-KOing victim */
  &&  victim->position > POS_SLEEPING  
  /* Prevents damaging a slept person and re-sleeping them in the same turn */
  &&  vopos > POS_SLEEPING  
  /* Prevents sleeping mid-combat ) */
  &&  vofi == NULL
  /* People awoken by pinch/poison cannot be re-slept with arrows */
  &&  !IS_AFFECTED( victim, AFF_SLEEP ) )
    {
      if(number_range(1,100) < 30) 
	{
	  AFFECT_DATA af;
	  act("$n is pierced with an arrow infused with curare, rendering $m unconscious!",victim,arrow,NULL,TO_ROOM);
	  act("An arrow infused with curare pierces your skin, rendering you unconscious!",victim,arrow,NULL,TO_CHAR);
	  af.where     = TO_AFFECTS;   
	  af.type      = gsn_sleep;
	  af.level     = ch->level;
	  af.duration  = 1;
	  af.location  = APPLY_NONE;
	  af.modifier  = 0;
	  af.bitvector = AFF_SLEEP;
	  affect_to_char( victim, &af );
	  stop_fighting( victim, FALSE );
	  stop_fighting( ch, FALSE );
	  victim->position = POS_SLEEPING;
	}
    }
  /* Poison */
  if(IS_ARROW_STAT(arrow,ARROW_POISON))
    {	
      if (!saves_spell(ch->level / 2, NULL, victim, DAM_POISON)) 
	{
	  AFFECT_DATA af;
	  send_to_char("Your skin is pierced by a poison-tipped arrowhead.\n\r",
		       victim);
	  act("$n is poisoned by the venom on $p.",
	      victim,arrow,NULL,TO_ROOM);

	  af.where     = TO_AFFECTS;
	  af.type      = gsn_poison;
	  af.level     = ch->level;
	  af.duration  = ch->level / 2;
	  af.location  = APPLY_STR;
	  af.modifier  = -1;
	  af.bitvector = AFF_POISON;
	  affect_join( victim, &af, TRUE );
	}
    } /* poison missed */

  /* Arrow go byeby */
  extract_obj(arrow);

  /* Bow breaking? 1% chance */ 
  if(number_range(1,10000) < 2)
    {
      send_to_char("Your bow snaps in two!\n\r",ch);
      extract_obj(bow);
    } 
  WAIT_STATE( ch, 8 );
}

/* Harm touch */

void do_harm_touch( CHAR_DATA *ch, char *argument ) 
{
    CHAR_DATA *victim;
    int chance, dam;
    AFFECT_DATA af;

    if ( !IS_NPC(ch) 
    &&   ch->level < skill_table[gsn_harmtouch].skill_level[ch->class] )
    {
        send_to_char("You don't know how to do that.\n\r", ch);
        return;
    }

    if ( (chance = get_skill(ch, gsn_harmtouch)) < 1 ) 
    {
        send_to_char("You don't know how to do that.\n\r", ch);
        return;
    }

    if ( IS_AFFECTED2(ch,AFF_GHOST) )
    {
        send_to_char("You're a ghost!\n\r", ch);
        return;
    }
    
    if ( ch->mana < 150 )
    {
        send_to_char("You don't have enough mana.\n\r",ch);
        return;
    }

    if ( IS_NULLSTR(argument) ) 
    {
        if ( ( victim = ch->fighting ) == NULL )
        {
            send_to_char("Who do you want to hurt?\n\r",ch);
            return;
        }
    }
    else if ( (victim = get_char_room(ch, argument, TRUE)) == NULL ) 
    {
        send_to_char("They aren't here.\n\r", ch );
        return;
    }
 
    if ( is_safe( ch, victim ) )
        return;

    if ( IS_AFFECTED2(victim, AFF_GHOST) )
    {
        act( "Your hand passes right through $M!", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
        act("$N is your beloved master.", ch, NULL, victim, TO_CHAR );
        return;
    }


    if ( is_affected(victim,gsn_harmtouch) 
    ||   victim->hit < (3*victim->max_hit/4) )
    {
	act( 
       "$N is hurting too badly for you to strike at $M on the ethereal plane.",
        ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( number_percent() > chance
    ||   saves_spell(ch->level, ch, victim, DAM_NEGATIVE) )
    {
        act( "$E rolls away from your touch.\n\rYou failed.", 
            ch, NULL, victim, TO_CHAR );
	act( "$n attempts to touch your soul, but fails!",
            ch, NULL, victim, TO_VICT );
        ch->mana = UMAX( 0, ch->mana-50 );
        check_improve( ch, gsn_harmtouch, FALSE, 1 );
        WAIT_STATE(ch, 10);
        return;
    }

    dam = victim->max_hit/4;

    af.where    = TO_AFFECTS;
    af.type     = gsn_harmtouch;
    af.level    = ch->level;
    af.duration = 10;
    af.location = APPLY_HIT;
    af.modifier = -1 * dam ;
    af.bitvector = APPLY_NONE;
    affect_to_char( victim, &af );

    act( "$n reaches towards you, grabbing ahold of your soul!", 
        ch, NULL, victim, TO_VICT);
    act( "$n grabs ahold of $N, mounting an assault on the soul!",
        ch,NULL, victim, TO_ROOM);
    act( "You grab ahold of $S soul!", ch, NULL, victim, TO_CHAR );

    ch->mana = UMAX(0,ch->mana-150);

    WAIT_STATE(ch, PULSE_VIOLENCE);

    damage( ch, victim, dam, gsn_harmtouch, DAM_NEGATIVE, TRUE );
    victim->hit = UMIN( victim->hit, victim->max_hit );

    check_improve( ch, gsn_harmtouch, TRUE, 1 );

    return;
}

void do_barbarian_whirlwind( CHAR_DATA *ch, int chance )
{
    CHAR_DATA *victim = ch->fighting;
    CHAR_DATA *next_victim;
    int fighting = 0, perchance;
    bool knocked = FALSE;
    for ( victim = ch->in_room->people; 
          victim != NULL; 
          victim = victim->next_in_room )
    {
        if ( victim->fighting == ch || ch->fighting == victim )
            fighting++;
    }

    for ( victim = ch->in_room->people; victim != NULL; victim = next_victim )
    {
        next_victim = victim->next_in_room;
        if ( (victim->fighting == ch  || ch->fighting == victim)
        &&   victim->position > POS_FLATFOOTED )
        {
            perchance = chance;
            if ( ch->size < victim->size )
                perchance += (ch->size - victim->size) * 5;
/*          perchance += get_curr_stat( ch, STAT_STR ); */
            if ( !IS_AFFECTED( ch, AFF_FLYING ) )
                perchance -= 70;
            if (  IS_SET( ch->off_flags, OFF_FAST) 
            ||    IS_AFFECTED( ch, AFF_HASTE ) )
                perchance += 10;
            perchance += (ch->level - victim->level);
            perchance /= fighting;
            perchance = UMAX( perchance, 1 );

            if ( IS_SET( ch->comm, COMM_VERBOSE )
            &&   get_trust(ch) > LEVEL_ADMIN )
            {
                char buf[1024];
                sprintf(buf, "chance to whirl %s=%d from base %d\n\r",
                PERS( victim, ch ), perchance, chance );
                send_to_char( buf, ch );
            }

            if ( !IS_SET(victim->imm_flags,IMM_LAGSKILLS) 
            &&   number_percent() < perchance )
            {
                act( "Your whirlwind fury sends $N flying!", 
                  ch, NULL, victim, TO_CHAR );
                act( "$n's whirlwind fury sends $N flying.", 
                  ch, NULL, victim, TO_NOTVICT );
                act( "$n's furious whirlwind bowls you over!", 
                  ch, NULL, victim, TO_VICT );
                WAIT_STATE(victim,PULSE_VIOLENCE*2);
                damage( ch, victim, 10, gsn_whirlwind, DAM_BASH, TRUE );
                if ( victim->position > POS_FLATFOOTED 
                &&   victim->in_room == ch->in_room )
                    victim->position = POS_SPRAWLED;
                knocked = TRUE;
            }
            else
            {
/*              act( "Your whirlwind attack clips $N!", 
                  ch, NULL, victim, TO_CHAR );
                act( "$n's whirlwind fury clips you!", 
                  ch, NULL, victim, TO_VICT ); */
                damage( ch, victim, 0, gsn_whirlwind, DAM_BASH, TRUE );
            }
        }
    }
    if ( knocked )
    {
        check_improve( ch, gsn_whirlwind, TRUE, 1 );
        WAIT_STATE( ch, PULSE_VIOLENCE*2+4 );
    }
    else
    {
        check_improve( ch, gsn_whirlwind, TRUE, 2 );
        WAIT_STATE( ch, PULSE_VIOLENCE );
    }
        
    return;
}

void do_generic_whirlwind( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *next_victim;
    bool knocked = FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = next_victim )
    {
        next_victim = victim->next_in_room;
        if ( (victim->fighting == ch || ch->fighting == victim)
        &&   victim->position > POS_FLATFOOTED )
        {
            if ( !IS_SET(victim->imm_flags,IMM_LAGSKILLS) 
            &&   number_percent() < 20 )
            {
                act( "Your whirlwind fury knocks $N over!", 
                  ch, NULL, victim, TO_CHAR );
                act( "$n's whirlwind knocks $N to the ground.", 
                  ch, NULL, victim, TO_NOTVICT );
                act( "$n's furious whirlwind bowls you over!", 
                  ch, NULL, victim, TO_VICT );
                WAIT_STATE(victim,PULSE_VIOLENCE*2);
                damage( ch, victim, 10, gsn_whirlwind, DAM_BASH, TRUE );
                if ( victim->in_room == ch->in_room 
                &&   victim->position > POS_FLATFOOTED )
                    victim->position = POS_SPRAWLED;
                knocked = TRUE;
            }
            else
            {
/*              act( "Your whirlwind attack clips $N!", 
                  ch, NULL, victim, TO_CHAR );
                act( "$n's whirlwind fury clips you!", 
                  ch, NULL, victim, TO_VICT ); */
                damage( ch, victim, 0, gsn_whirlwind, DAM_BASH, TRUE );
            }
        }
    }
    if ( knocked )
        check_improve( ch, gsn_whirlwind, TRUE, 1 );
    else
        check_improve( ch, gsn_whirlwind, TRUE, 2 );

    WAIT_STATE( ch, PULSE_VIOLENCE );
    return;
}

void do_whirlwind( CHAR_DATA* ch, char* argument )
{
    int chance;
    CHAR_DATA* victim;

    if ( IS_AFFECTED2( ch, AFF_GHOST ) )
    {
      send_to_char( "You're a ghost!", ch );
      return;
    }

    if ( (chance = get_skill( ch, gsn_whirlwind )) == 0
    || ( !IS_NPC( ch ) 
      && ch->level < skill_table[ gsn_whirlwind ].skill_level[ ch->class ] ) )
    {
        send_to_char( "You spin around the room with your arms held out.\n\r", 
          ch );
        act( "$n spins around the room with $s arms held out.", 
          ch, NULL, NULL, TO_NOTVICT );
        return;
    }

    if ( (victim = ch->fighting) == NULL )
    {
         send_to_char( "But you aren't fighting anyone!\n\r", ch );
         return;
    }

    /* if ch is fighting victim, how could victim be safe? */
    if ( is_safe( ch, victim ) )
        return;

    if ( number_percent() < chance )
    {
        if ( ch->class == 4 )   /* barbarians get special whirlwind */
            do_barbarian_whirlwind( ch, chance );
        else                    /* generic whirlwind skill for most classes */
            do_generic_whirlwind( ch );

    }
    else
    {
        send_to_char( "You fail to get up enough momentum.\n\r", ch );
        check_improve( ch, gsn_whirlwind, FALSE, 2 );
        WAIT_STATE( ch, PULSE_VIOLENCE );
    }
    return;
}

/* FAKE APRIL FOOLS COMMANDS */
void fake_corpse( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 3, 6 );
	corpse->cost = 0;
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= number_range( 25, 40 );
        corpse->owner = NULL;
		
	corpse->cost = 0;
    }

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	bool floating = FALSE;

	obj_next = obj->next_content;
	if (obj->wear_loc == WEAR_FLOAT)
	    floating = TRUE;

	if (floating)
	{
	    if (IS_OBJ_STAT(obj,ITEM_ROT_DEATH)  &&
		!IS_NPC(ch)) /* get rid of it! */
	    { 
		if (obj->contains != NULL)
		    act("$p evaporates, scattering its contents.",
			ch,obj,NULL,TO_ROOM);
                else
		    act("$p evaporates.", ch,obj,NULL,TO_ROOM);
	    }
	    else
		act("$p falls to the floor.",ch,obj,NULL,TO_ROOM);
	}
    }

    obj_to_room( corpse, ch->in_room );
    return;
}

void fake_kill( CHAR_DATA *victim )
{
    char pueblo_buf[MAX_STRING_LENGTH];

    stop_fighting( victim, TRUE );
    death_cry( victim );
    fake_corpse( victim );

    if (IS_SET(victim->act_bits, PLR_PUEBLO)
    && !IS_NPC(victim)) 
    {
        sprintf(pueblo_buf,"<img xch_sound=play xch_volume=%dsrc=\"%s%s\">"
                "<img src=\"%s%s\">",
victim->pcdata->volume,PUEBLO_DIR,PUEBLO_END, PUEBLO_DIR, PUEBLO_D_IMG);
        send_to_char("</xch_mudtext><img xch_mode=html>",victim);
        send_to_char(pueblo_buf,victim);
        send_to_char("<br><img xch_mode=text>",victim);
        send_to_char("</xch_mudtext><img xch_mode=html>",victim);
                sprintf(pueblo_buf,"<img xch_sound=stop>");
        send_to_char(pueblo_buf,victim);
        send_to_char("<br><img xch_mode=text>",victim);
    }

    return;
}

void do_fakeslay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Fake slay who?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_ADMIN( ch ) && !IS_NPC(victim) && victim->level >= get_trust(ch) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR    );
    act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
    act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    fake_kill( victim );
    if ( !IS_NPC(victim) )
    {
        victim->pcdata->lastfight = current_time - FIGHT_LAG;
        victim->pcdata->lastpk = current_time - FIGHT_LAG;;
    }
    if ( global_config_aprilfools && !IS_NPC(victim) )
        victim->pcdata->aprilfools = TRUE;
    return;
}

/*
 * check fightlag status; also update PK fightlag timer if in PK
 *   note that player prompt %f does NOT use this; as such, it can be out of 
 *   sync unless in_fightlag is called at regular intervals (e.g., every 
 *   violence update)
 */
bool in_fightlag( CHAR_DATA *ch )
{
    /* NPCs are never in fightlag */
    if ( IS_NPC(ch) ) return FALSE;

    /* Check if ch is in fightlag */
    if ( (current_time - ch->pcdata->lastfight) < FIGHT_LAG )
    {
        /* if they are in fightlag, check to see if it's PK fightlag */
        if ( ch->pcdata->lastpk 
        &&   (current_time - ch->pcdata->lastpk) < FIGHT_LAG )
            ch->pcdata->lastpk = ch->pcdata->lastfight;
        else
            ch->pcdata->lastpk = 0;
        return TRUE;
    }
    else
    {
        ch->pcdata->lastfight = 0;
        return FALSE;
    }
}

void do_clear(CHAR_DATA *ch, char *argument)
{
    bool water = FALSE;

    if ( ch->in_room 
    &&   (ch->in_room->sector_type == SECT_WATER_NOSWIM
    ||    ch->in_room->sector_type == SECT_WATER_SWIM) )
        water = TRUE;

    if ( !IS_AFFECTED3(ch,AFF_DIRT) )
    {
        send_to_char(
            "You try to clear your eyes, not believing what you're seeing!\n\r",
            ch);
        return;
    }


    if (number_range(1,100) > (CONFIG_CLEAR_CHANCE * ( water ? 2 : 1 )) )
    {
        send_to_char(
            "Try as you might you cannot clear the debris from your eyes!\n\r",
            ch);
        WAIT_STATE(ch,CONFIG_CLEAR_LAG);
    }
    else
    {
        if ( is_affected( ch, gsn_dirt ) )
            affect_strip(ch, gsn_dirt );
        else
            affect_strip( ch, gsn_wing );

        if ( water )
            send_to_char(
                "You splash water into your eyes and rinse out the debris!\n\r",
                ch );
        else
            send_to_char("You clear the debris from your eyes!\n\r",ch);

        WAIT_STATE(ch,PULSE_VIOLENCE);
    }
    return;
}

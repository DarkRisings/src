/***************************************************************************
 * Wild Magic Spells by Sharon P. Goza 1/8/99
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
#include "interp.h"

extern char *target_name;

void spell_wildshield( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  short chance;
  short acmod;
  
  if(is_affected(ch,skill_lookup("wild shield")))
    {
      send_to_char("You're already surrounded by a wild shield.\n\r",ch);
      return;
    }

  chance = number_range(1,100);

  if (chance < 20)
    {
    acmod = 20;
    	act("$n is surrounded by a black force field.",
			ch,NULL,victim,TO_NOTVICT);
    	if (ch != victim) {
       		act("$N is surrounded by a black force field.",
           		 ch, 0, victim, TO_CHAR);
       		act("You are surrounded by a black force field.",
           		 ch, 0, victim, TO_VICT);
    		} else {
       		act("You are surrounded by a black force field.",
           		ch, 0, 0, TO_CHAR);
     			}
		}
	else if (chance < 85){
		acmod = -35;
    	act("$n is surrounded by a strong force field.",
			ch,NULL,victim,TO_NOTVICT);
    	if (ch != victim) {
       		act("$N is surrounded by a strong force field.",
           		ch, 0, victim, TO_CHAR);
       		act("You are surrounded by a strong force field.",
           		ch, 0, victim, TO_VICT);
    		} else {
       		act("You are surrounded by a strong force field.",
           		ch, 0, 0, TO_CHAR);
     			}
		}
	else {
		acmod = -50;
    	act("$n is surrounded by a brilliant white force field.",
			ch,NULL,victim,TO_NOTVICT);
    	if (ch != victim) {
       		act("$N is surrounded by a brilliant white force field.",
           		ch, 0, victim, TO_CHAR);
       		act("You are surrounded by a brilliant white force field.",
           		ch, 0, victim, TO_VICT);
    		} else {
       		act("You are surrounded by a brilliant white force field.",
           		ch, 0, 0, TO_CHAR);
     			}
		}

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 24;
    af.modifier  = acmod;
    af.location  = APPLY_AC;
    af.bitvector = APPLY_NONE;
    affect_to_char2( victim, &af );
    return;
 
}

void spell_wildheal( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  short chance;
  CHAR_DATA *victim = (CHAR_DATA*)vo;


  
  if (!ch || !victim)
    return;

  chance = number_range(1,100);

  if (chance < 10)
    {
      /* black */
      victim->hit -= number_range(100,250);
      
      if (victim->hit < 1)
	victim->hit = 1;

      act( "A black haze descends on $N.", ch, 0, victim, TO_NOTVICT );
      send_to_char( "A black haze descends on you.\n\r", victim );

      if( ch != victim )
	{
	  act( "A black haze descends on $M.", ch, 0, victim, TO_CHAR );
	}
    }
  else if (chance < 12)
    {
      /* grey */
      victim->hit -= 50;
      
      if (victim->hit < 1)
	victim->hit = 1;
    
      act( "A grey haze descends on $N.", ch, 0, victim, TO_NOTVICT );
      send_to_char( "A grey haze descends on you.\n\r", victim );

      if( ch != victim )
	{
	  act( "A grey haze descends on $M.", ch, 0, victim, TO_CHAR );
	}
    }
  else if (chance < 80)
    {
      victim->hit += 80;
      
      if (victim->hit >= victim->max_hit)
	victim->hit = victim->max_hit;
    
      act( "A blue haze descends on $N.", ch, 0, victim, TO_NOTVICT );
      send_to_char( "A blue haze descends on you.\n\r", victim );

      if( ch != victim )
	{
	  act( "A blue haze descends on $M.", ch, 0, victim, TO_CHAR );
	}
    }
   else if (chance < 90)
     {
       /* yellow */
       victim->hit += 120;
       
       if (victim->hit >= victim->max_hit)
	 victim->hit = victim->max_hit;

      act( "A yellow haze descends on $N.", ch, 0, victim, TO_NOTVICT );
      send_to_char( "A yellow haze descends on you.\n\r", victim );

      if( ch != victim )
	{
	  act( "A yellow haze descends on $M.", ch, 0, victim, TO_CHAR );
	}
     }
  else 
    {
      /* white */
      victim->hit += number_range(121,150);
      
      if (victim->hit >= victim->max_hit)
	victim->hit = victim->max_hit;

      act( "A white haze descends on $N.", ch, 0, victim, TO_NOTVICT );
      send_to_char( "A white haze descends on you.\n\r", victim );

      if( ch != victim )
	{
	  act( "A white haze descends on $M.", ch, 0, victim, TO_CHAR );
	}
    }
}

void spell_wildenhance( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
CHAR_DATA *victim = (CHAR_DATA *) vo;
AFFECT_DATA af;
short chance;
short hitdammod;
        if(is_affected(ch,skill_lookup("wild enhance")))
	{
        return;
    	}

	chance = number_range(1,100);

	if (chance < 15){
		hitdammod = -5;
    	act("$n looks weaker and a bit sickly.",
			ch,NULL,victim,TO_NOTVICT);
    	if (ch != victim)
        {
            act("$N looks weaker and a bit sickly.", ch, 0, victim, TO_CHAR);
            act("You feel weaker and a bit sickly.", ch, 0, victim, TO_VICT);
    	}
        else
        {
            act("You feel weaker and a bit sickly.", ch, 0, 0, TO_CHAR);
  	}
	}
	else if (chance < 80){
		hitdammod = 2;
    	act("$n looks a bit stronger and ready for action.",
			ch,NULL,victim,TO_NOTVICT);
    	if (ch != victim) {
       		act("$N looks a bit stronger and ready for action.",
           		ch, 0, victim, TO_CHAR);
       		act("You feel a bit stronger and ready for action.",
           		ch, 0, victim, TO_VICT);
    		} else {
       		act("You feel a bit stronger and ready for action.",
           		ch, 0, 0, TO_CHAR);
     			}
		}
	else {
		hitdammod = 5;
    	act("$n's muscle's bulge before your eyes!",
			ch,NULL,victim,TO_NOTVICT);
    	if (ch != victim) {
       		act("$N's muscle's bulge before your eyes!",
           		ch, 0, victim, TO_CHAR);
       		act("Your muscles bulge and surge with power!",
           		ch, 0, victim, TO_VICT);
    		} else {
       		act("Your muscles bulge and surge with power!",
           		ch, 0, 0, TO_CHAR);
     			}
		}

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 12;
    af.modifier  = hitdammod;
    af.location  = APPLY_HITROLL;
    af.bitvector = APPLY_NONE;
    affect_to_char2( victim, &af );

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 12;
    af.modifier  = hitdammod;
    af.location  = APPLY_DAMROLL;
    af.bitvector = APPLY_NONE;
    affect_to_char2( victim, &af );
    return;
 
}

void spell_wildaura( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
CHAR_DATA *victim = (CHAR_DATA *) vo;
AFFECT_DATA af;

    if (IS_AFFECTED2(ch,AFF_WILDAURA)){
        send_to_char("You are already surrounded by a chaotic aura.\n\r",
           ch);
        return;
    	}

    act("$n is surrounded by a multi-colored sphere.",
			ch,NULL,victim,TO_NOTVICT);
    act("You are surrounded by a multi-colored sphere.",
           		ch, 0, victim, TO_CHAR);

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level/6;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_WILDAURA;
    affect_to_char2( victim, &af );
    return;
}

void spell_wildfire( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
CHAR_DATA *victim = (CHAR_DATA *)vo;
int damage;
int chance;

	chance = number_range(1,100);

        if (chance < 15){
	   damage = ch->level;
           if (saves_spell(level,ch,ch,DAM_NONE))
              damage /=1;
           damage_old(ch, ch, damage, sn, DAM_NONE, TRUE);
           return;
           }
         else if (chance < 30){
	   damage = ch->level * 1.5;
           if (saves_spell(level,ch,ch,DAM_NONE))
              damage /=1;
           damage_old(ch, victim, damage, sn, DAM_NONE, TRUE);
           return;
           }
         else if (chance < 70){
	   damage = ch->level * 3;
           if (saves_spell(level,ch,victim,DAM_NONE))
              damage /=1;
           damage_old(ch, victim, damage, sn, DAM_NONE, TRUE);
           return;
           }
         else if (chance < 85){
	   damage = ch->level * 3.5;
           if (saves_spell(level,ch,victim,DAM_NONE))
              damage /=1;
           damage_old(ch, victim, damage, sn, DAM_NONE, TRUE);
           return;
           }
         else {
	   damage = ch->level * 4.5;
           if (saves_spell(level,ch,victim,DAM_NONE))
              damage /=1;
           damage_old(ch, victim, damage, sn, DAM_NONE, TRUE);
           }
}

void 
spell_chaosdemon(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
CHAR_DATA *mob;
CHAR_DATA *victim;
AFFECT_DATA af;
MOB_INDEX_DATA *pmobindex;
short indexnum;
short chance;

    victim = (CHAR_DATA *) vo;

 
    if (ch->pet != NULL || has_summonedpets(ch) ){
       send_to_char("You already have a demon following you.\n\r",ch);
	return;
       }

    if (IS_SET(ch->in_room->room_flags,ROOM_SAFE)
    || (IS_SET(ch->in_room->room_flags,ROOM_SEMISAFE) 
        && !in_fightlag(ch) ) )
    {
      send_to_char("Not in this room.\n\r",ch);
      return;
    }
    
    if ( IS_AFFECTED2( victim, AFF_SUMSICK ) )
     {
       send_to_char("You cannot summon another demon so quickly!\n\r",ch);
       return;
     }
 
    spell_summon_sick(gsn_sumsick,level,ch,(void *) victim,TARGET_CHAR);

    chance = number_range(1,100);

    if (chance < 10)
       indexnum = 7;		/* Hoarde */
    else if (chance < 20)
       indexnum = 5;		/* wimp */
    else if (chance < 40)
       indexnum = 6;		/* blue frost */
    else if (chance < 80)	
       indexnum = 9;		/* red deamon */
    else 
       indexnum = 8;		/* angel */

    pmobindex = get_mob_index(indexnum);

    if (!pmobindex){
        return;
        }

    mob = create_mobile(pmobindex);
    char_to_room(mob,ch->in_room);

    switch( indexnum){
      case 5:
    	mob->max_hit = .25 * ch->max_hit;
        act( "$n creeps out of nowhere and clings to your leg!",
	    mob, NULL, victim, TO_VICT );
        act( "$n creeps out of nowhere and clings to $N's leg.",
	    mob, NULL, victim, TO_NOTVICT);
        break;
      case 6:
        act( "Ice crystallizes on your clothes as $n answers your summons!",
	    mob, NULL, victim, TO_VICT );
        act( "Ice crystallizes on your clothes as $n answers $N's summons.",
            mob, NULL, victim, TO_NOTVICT );
    	mob->max_hit = .75 * ch->max_hit;
        break;
      case 7:
        act( "$n rips its way through space and time and attacks you!",
	    mob, NULL, victim, TO_VICT );
        act( "$n rips its way through space and time and attacks $N!",
	    mob, NULL, victim, TO_NOTVICT );
    	mob->max_hit = 2.5 * ch->max_hit;
        break;
      case 8:
        act( "$n descends from the heavens to give you aid!",
	    mob, NULL, victim, TO_VICT );
        act( "$n descends from the heavens to give $N aid.",
	    mob, NULL, victim, TO_NOTVICT );
    	mob->max_hit = 2.5 * ch->max_hit;
        break;
      default:
        act( "In a burst of heat and with the stench of sulfur, $n appears!",
	    mob, NULL, victim, TO_VICT );
        act( "In a burst of heat and with the stench of sulfur, $n appears.",
            mob, NULL, victim, TO_NOTVICT );
    	mob->max_hit = ch->max_hit;
        break;
      }
     
    mob->hit = mob->max_hit;
    mob->level = ch->level;


    if ( indexnum != 7) {
	    add_follower(mob, ch);
	    mob->leader = ch;
	    ch->pet = mob;
             SET_BIT(mob->act_bits, ACT_PET);

	    af.where     = TO_AFFECTS;
	    af.type      = sn;
	    af.level     = level;
	    af.duration  = number_fuzzy( level / 4 );
	    af.location  = 0;
	    af.modifier  = 0;
	    af.bitvector = AFF_CHARM;
	    affect_to_char( mob, &af );

/*          Boring old message replaced with newer, cooler ones thanks to 
              Sidonie. */
/*	    act ("$n summons $N to do $s bidding.",ch,NULL,mob, TO_ROOM);*/
	}
    else {
    	mob->level = 60;
	    af.where     = TO_AFFECTS;
	    af.type      = gsn_sumanimal;
	    af.level     = level;
	    af.duration  = 3;
	    af.location  = 0;
	    af.modifier  = 0;
	    af.bitvector = 0;
	    affect_to_char( mob, &af );
/*	   act ("$n summons a $N and is attacked!",ch,NULL,mob, TO_ROOM);*/
	   multi_hit( mob, ch, TYPE_UNDEFINED );
	}
}

void
spell_wildsummon(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    if ( ( victim = get_char_world( ch, target_name, FALSE ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   (IS_SET(victim->in_room->room_flags,ROOM_SEMISAFE) 
           && !in_fightlag(victim) )
    ||   IS_SET(victim->in_room->room_flags, ROOM_GODS_ONLY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_GODS_ONLY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_IMP_ONLY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_SUMMON)
    ||   IS_NPC(victim)
    /*||   victim->level >= level + 3*/
    ||   victim->level >= LEVEL_IMMORTAL
    /*||   victim->fighting != NULL*/
    ||   (IS_SET(victim->act_bits,PLR_NOSUMMON) &&
          ch->level > victim->level + 6) )

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

    if (IS_AFFECTED2(victim,AFF_GHOST)){
        send_to_char( "Your spell will not work on ghosts.\n\r", ch );
        return;
       }
    if (IS_SET(victim->act_bits,PLR_NOSUMMON) &&
        (number_range(1,100) < 85) &&
        !IS_NPC(ch)) {
	if(number_range(1,100) > 80) /* Backfire */
	{
		send_to_char("Your chaotic magic backfires!\n\r",ch);
		send_to_char("The world spins around you!\n\r",ch);
                act( "$n disappears in a multicolored cloud.", ch, NULL, NULL, TO_ROOM );
	        char_from_room( ch );
	        char_to_room( ch, victim->in_room );
                act( "$n materializes from a multicolored cloud.", ch, NULL, NULL, TO_ROOM );
                do_look( ch, "auto" );
		return;
	}
	send_to_char("Your chaotic magic sizzles with no result.\n\r",ch);
        return;
       }
    if (IS_SET(victim->act_bits,PLR_NOSUMMON))
    {
/*    ch->fightlag = FIGHT_LAG; */
      if ( !IS_NPC(ch) )
      {
          ch->pcdata->lastfight = current_time;
          ch->pcdata->lastpk = current_time;
      }
    }
    act( "$n disappears in a multicolored cloud.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act( "$n materializes from a multicolored cloud.", victim, NULL, NULL, TO_ROOM );
    act( "$n has called you into a chaotic vortex!", ch, NULL, victim,TO_VICT );
    do_look( victim, "auto" );
    if (number_range(1,100) < 6)
    {
   	send_to_char("The vortex burns your eye!\n\rYou are blinded!\n\r",victim);
            af.where     = TO_AFFECTS;
            af.type      = gsn_blindness;
            af.level     = level-20;
            af.duration  = 1;
            af.location  = 0;
            af.modifier  = 0;
            af.bitvector = AFF_BLIND;
            affect_to_char( victim, &af );
	}
    return;
}
	

void 
spell_wild_dispel(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	int chance;
	chance = number_range(1,100);
	switch(chance) {
	case 10:
		spell_cancellation(gsn_cancellation,level,ch,(void *)ch,target);
		break;
	case 15:
		spell_dispel_magic(gsn_dispel_magic,level+1,ch,vo,target);
		break;
	case 30:
		spell_dispel_magic(gsn_dispel_magic,level+2,ch,(void *)ch,target);
		break;
	case 33:
		spell_dispel_magic(gsn_dispel_magic,level+1,ch,vo,target);
		break;
	case 40:
		spell_dispel_magic(gsn_dispel_magic,level+2,ch,vo,target);
		break;
	case 45:
		spell_dispel_magic(gsn_dispel_magic,level+5,ch,(void *)ch,target);
		break;
	case 60:
		spell_dispel_magic(gsn_dispel_magic,level-5,ch,(void *)ch,target);
		break;
	case 65:
		spell_dispel_magic(gsn_dispel_magic,level+3,ch,vo,target);
		break;
	case 75:
		spell_dispel_magic(gsn_dispel_magic,level+4,ch,vo,target);
		break;
	case 80:
		spell_dispel_magic(gsn_dispel_magic,level+15,ch,(void *)ch,target);
		break;
	case 82:
		spell_dispel_magic(gsn_dispel_magic,level+5,ch,vo,target);
		break;

	case 87:
		spell_dispel_magic(gsn_dispel_magic,level+6,ch,vo,target);
		break;

	case 92:
		spell_dispel_magic(gsn_dispel_magic,level+8,ch,vo,target);
		break;

	case 96:
		spell_dispel_magic(gsn_dispel_magic,level+10,ch,vo,target);
		break;
	case 99:
		spell_dispel_magic(gsn_dispel_magic,level+15,ch,vo,target);
		break;
	default:
		spell_dispel_magic(gsn_dispel_magic,level,ch,vo,target);
		break;
	}
	return;
}

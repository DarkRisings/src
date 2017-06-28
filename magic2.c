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


extern char     *target_name;
extern bool     add_blood       args( ( CHAR_DATA* ch, sh_int blood ) );
extern bool     check_charmlist args( ( CHAR_DATA *ch, CHAR_DATA *vch ) );
extern bool     check_dispel    args(( int dis_level, CHAR_DATA* ch, int sn ));
extern void 	do_cast  args(( CHAR_DATA *ch, char *argument ));

void spell_epiclesis( int sn, int level, CHAR_DATA* ch, void* vo, int target )
{
  CHAR_DATA* victim = NULL;
  CHAR_DATA* next_victim = NULL;
  int victim_ct;
  
  for( victim = ch->in_room->people, victim_ct = 0; victim != NULL; victim = next_victim )
    {
      next_victim = victim->next_in_room;

      if( !IS_NPC( victim ) && victim->fighting == ch )
	victim_ct++;
    }

  if( victim_ct < 2 )
    {
      send_to_char( "You are not in enough danger to invoke the power of Tyrin in this way.\n\r", ch );
      return;
    }

  for( victim = ch->in_room->people; victim != NULL; victim = next_victim )
    {
      next_victim = victim->next_in_room;

      if( !IS_NPC( victim ) && victim->fighting == ch )
	{
	  act( "You call down the power of Tyrin against $N!", ch, NULL, victim, TO_CHAR );
	  act( "$n has called down the power of Tyrin against you!", ch, NULL, victim, TO_VICT );
	  act( "$n has called down the power of Tyrin against $N!", ch, NULL, victim, TO_NOTVICT );
	  spell_dispel_magic( sn, level + 5, ch, victim, target );
	}
    }
}


void spell_betray( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    char buf[MAX_STRING_LENGTH], \
         buf2[MAX_STRING_LENGTH];

    if ( victim == ch )
    {
        send_to_char( "Betray yourself?  You're weird.\n\r", ch );
        return;
    }

    if ( !IS_AFFECTED(victim, AFF_CHARM)
    ||   saves_spell( level, ch,victim,DAM_CHARM) )
     {
        send_to_char( "Your betrayal failed.\n\r", ch );
        return;
     }

    if ( victim->fighting == ch )
        stop_fighting( victim, TRUE );

    if ( victim->master )
    {
        strcpy( buf, victim->short_descr );
        if ( stop_follower( victim ) )
        {
/*          Since the above if-check will only be true if victim is an NPC,
            it is safe to use the short description without verification. */
            sprintf(buf2, "You send %s fleeing in confusion!\n\r", buf );
            send_to_char( buf2, ch );
	    return;
        }
    }

    if ( !check_charmlist( ch, victim ) )
    {
        add_follower( victim, ch );
        victim->leader = ch; 
        af.where     = TO_AFFECTS;
        af.type      = sn;
        af.level     = level;
        af.duration  = number_fuzzy( level / 4 );
        af.location  = 0;
        af.modifier  = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char( victim, &af );
        act( "$N has betrayed!", ch, NULL, victim, TO_CHAR );
        act( "You now follow $n!", ch, NULL, victim, TO_VICT );
    }
    return;
}


void spell_nexus( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal;

        if ( ( victim = get_char_world( ch, target_name, TRUE ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room)
    ||   IS_SET(victim->in_room->room_flags, ROOM_GODS_ONLY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_IMP_ONLY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NOWHERE)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NOWHERE)
    ||	(is_guild(victim) && !is_same_guild(ch,victim)))
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }   

    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 2 + level / 25; 
    portal->value[3] = victim->in_room->vnum;

    obj_to_room(portal,ch->in_room);

    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
}

void spell_gate( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  char buffer[120];
  CHAR_DATA *victim = NULL;
  OBJ_DATA *portal;
  ROOM_INDEX_DATA *to_room = NULL; 
  ROOM_INDEX_DATA *from_room = NULL;
  bool fail = FALSE;
  bool one_way = FALSE;
  bool fatal = FALSE;
  bool random = FALSE;
  bool nowhere = FALSE;
  
  from_room = ch->in_room;
  victim = get_char_world( ch, target_name, FALSE );
  
  if( victim )
    to_room = victim->in_room;

  if( IS_SET( from_room->room_flags, ROOM_IMP_ONLY ) ||
      IS_SET( from_room->room_flags, ROOM_GODS_ONLY ) ||
      IS_SET( from_room->trans_flags, TRANS_NO_BAD_GATES ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }
	
		if (is_affected(ch, skill_lookup("hallowed discord"))) {
			return;
		}

  if( victim == NULL ||
      victim == ch ||
      to_room == NULL ) 
    {
      fail = TRUE;
      one_way = TRUE;
    }

  if( !fail )
    {
      if ( !can_see_room(ch,to_room) 
      || !can_see_room(ch,from_room) 
      || IS_SET(from_room->room_flags,ROOM_NO_GATE) 
      || IS_SET(to_room->room_flags, ROOM_NO_GATE) 
      || IS_SET(to_room->room_flags, ROOM_SOLITARY) 
      || !to_room->area->linked 
      || !from_room->area->linked 
      || ( IS_NPC( victim ) && IS_SET( victim->imm_flags, IMM_GATE ) ) 
      || ( (IS_SET( victim->in_room->room_flags, ROOM_SAFE )
           || IS_SET( victim->in_room->room_flags, ROOM_SEMISAFE))
         && in_fightlag(ch) && !in_fightlag(victim) ) )
	{
	  fail = TRUE;
	}
    }
  if( !fail )
    {
      if( IS_SET( ch->in_room->trans_flags, TRANS_NO_GATE ) ||
	  IS_SET( ch->in_room->trans_flags, TRANS_NO_GATE_OUT ) ||
	  IS_SET( ch->in_room->trans_flags, TRANS_NO_SPELLS ) ||
	  IS_SET( ch->in_room->trans_flags, TRANS_NO_SPELLS_OUT ) ||
	  IS_SET( victim->in_room->trans_flags, TRANS_NO_GATE ) ||
	  IS_SET( victim->in_room->trans_flags, TRANS_NO_GATE_IN ) ||
	  IS_SET( victim->in_room->trans_flags, TRANS_NO_SPELLS ) ||
	  IS_SET( victim->in_room->trans_flags, TRANS_NO_SPELLS_IN ) )
	{
	  fail = TRUE;
	}
   }

  if( fail )
    {
      if ( victim &&
           IS_SET( victim->in_room->trans_flags, TRANS_NO_BAD_GATES ) )
          one_way = TRUE;
      if( victim &&
	  IS_AFFECTED( victim, AFF_DETECT_MAGIC ) )
	{
	  sprintf(buffer,"%s is gating to you!\n\r",ch->name);
	  send_to_char(buffer,victim);
	}

      int chance = number_percent();

      if( chance < 50 )
	{
	  nowhere = TRUE;
	}
      else if( chance < 75 )
	{
	  random = TRUE;
	}
      else
	{
	  fatal = TRUE;
	}
    }   
  else if( victim && IS_AFFECTED(victim, AFF_DETECT_MAGIC))
    {
      send_to_char("You feel as if a door opened behind you!\n\r",victim);
    }

  /* portal one */ 
  portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
  portal->timer = 1 + level / 10;
  portal->value[3] = ( to_room ? to_room->vnum : ROOM_VNUM_LIMBO );
 
  if( nowhere )
    SET_BIT( portal->value[ 2 ], GATE_DEAD_END );
 
  if( random )
    SET_BIT( portal->value[ 2 ], GATE_RANDOM );

  if( fatal )
  {
/*  SET_BIT( portal->value[ 2 ], GATE_FATAL ); */
    SET_BIT( portal->value[2], GATE_RANDOM );
    SET_BIT( portal->value[2], GATE_DRAINER );
  }

  obj_to_room(portal,from_room);
 
  act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
  act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
  
  if( fail )
    act("$p wavers and shimmers.  It may be unstable.", ch, portal, NULL, TO_CHAR );

  /* no second portal if rooms are the same */
  if (to_room == from_room)
    return;

  /* portal two */
  if( !one_way )
    {
      portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
      portal->timer = 1 + level/10;
      portal->value[3] = from_room->vnum;

      if( nowhere )
	SET_BIT( portal->value[ 2 ], GATE_DEAD_END );
      
      if( random )
	SET_BIT( portal->value[ 2 ], GATE_RANDOM );

      if( fatal )
      {
/*        SET_BIT( portal->value[ 2 ], GATE_FATAL ); */
          SET_BIT( portal->value[2], GATE_RANDOM );
          SET_BIT( portal->value[2], GATE_DRAINER );
      }

      obj_to_room(portal,to_room);
      
      if (to_room->people != NULL)
	{
	  act("$p rises up from the ground.",to_room->people,portal,NULL,TO_ROOM);
	  act("$p rises up from the ground.",to_room->people,portal,NULL,TO_CHAR);
	}
    }
}

void spell_summon_animal(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *mob;
    AFFECT_DATA af;
    MOB_INDEX_DATA *pMobIndex;
    short indexnum;
 
    if ( ch->pet != NULL || has_summonedpets( ch ) ) 
    {
        send_to_char("You already have a creature following you.\n\r",ch);
        return;
    }
 
    if ( IS_AFFECTED2( ch, AFF_SUMSICK ) )
     {
       send_to_char("You cannot summon another animal so quickly!\n\r",ch);
       return;
     }
 
    spell_summon_sick(gsn_sumsick,level,ch,(void *) ch,TARGET_CHAR);
 
    if (ch->level > 40)
        indexnum = 4;
    else if (ch->level > 30)
        indexnum = 3;
    else
        indexnum = 2;

    if ( !str_cmp( target_name, "wolf" ) && ch->level > 40 )
        indexnum = 4;
    else if ( !str_cmp( target_name, "bear" ) && ch->level > 30 )
        indexnum = 3;
    else if ( !str_cmp( target_name, "stag" ) )
        indexnum = 2;
 
    pMobIndex = get_mob_index(indexnum);
 
    if (!pMobIndex){
        return;
        }
 
    mob = create_mobile(pMobIndex);
    char_to_room(mob,ch->in_room);
 
    mob->max_hit = .75 * ch->max_hit;
    mob->hit = mob->max_hit;
    mob->level = ch->level;
  
    add_follower(mob, ch);
    mob->leader = ch;
    ch->pet = mob;
    SET_BIT(mob->act_bits, ACT_PET);
    ch->charmies += 1;
 
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( mob, &af );
 
    act ("$n summons $N to do $s bidding.",ch,NULL,mob, TO_ROOM);
    act ("You summon $N to do your bidding.",ch,NULL,mob, TO_CHAR);
 }
 
void spell_summon_sick( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
 
    victim = (CHAR_DATA *) vo;
 
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.whichaff  = AFF2;
    af.duration  = 7;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SUMSICK;
    affect_to_char2( victim, &af );

    return;
}


/* New spells */
/* This one is for Jackel -- Bavor */
void spell_devotion( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (victim->level > 50)
    {
      send_to_char("Gods need not pray\n\r",ch);
      return;
    }
  send_to_char("You bow your head in silent devotion to the Gods.\n\r",victim);
  if(ch != victim) 
    {
      act ("$n bows $s head in silent devotion to the Gods.",victim,NULL,NULL, TO_ROOM);
    }
  af.where     = TO_AFFECTS;
  af.type      = gsn_devotion;
  af.level     = level;
  af.whichaff  = AFF2;
  af.duration  = 5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = 0;
  affect_to_char2( victim, &af );

  victim->reply = NULL;

  return;

}


/* Food spells, grin */
/* Apple Goodness*/
void spell_apple( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
	int hp_to_heal;
	hp_to_heal = 5 * level;
	ch->hit = UMIN(ch->hit + hp_to_heal,ch->max_hit);
	send_to_char("You are filled with delicious apples!\n\r",ch);
}
/* Plum Pleasenetness */
void spell_plum( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
	int mv_to_heal;
	mv_to_heal = 3 * level;
	ch->move = UMIN(ch->move + mv_to_heal,ch->max_move);
	send_to_char("The soothing taste of plums refreshes your weary bones.\n\r",ch);
	return;
}


void spell_strawberry( int sn, int level, CHAR_DATA* ch, void* vo, int target )
{
    /* old strawberry spell did a plague removal */
    AFFECT_DATA af;

    if( is_affected( ch, skill_lookup( "strawberry fulfillment" ) ) )
      send_to_char( 
          "You are as fulfilled as strawberry goodness can make you!\n\r", ch );
    else
    {
        af.where = TO_AFFECTS;
        af.type = sn;
        af.level = level;
        af.duration = 5 * ( level / 10 + 1 );
        af.location = APPLY_SAVES;
        af.modifier = -( level / 10 + 1 );
        af.bitvector = APPLY_NONE;
        affect_to_char( ch, &af );
        send_to_char( "Strawberry goodness fulfills you!\n\r", ch );
    }
    return;
}

void spell_chocalate( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
	AFFECT_DATA af;
	int dam_to_add = level / 5;
	if(is_affected(ch,skill_lookup("chocolate fortification")))
	{
	send_to_char("Chocalate has made you as strong as it can!\n\r",ch);
	return;
	}
         af.where = TO_AFFECTS;
         af.type = sn;
         af.level = level;
         af.duration = level/3;
         af.location = APPLY_DAMROLL;
	 af.modifier = dam_to_add;
	 af.bitvector = APPLY_NONE;
         affect_to_char(ch,&af);
	send_to_char("You lick your lips and grow momentarily stronger, on a sugar rush.\n\r",ch);
}

/* POWER OF THE CARNIVORE !*/
void spell_carnivore( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
	AFFECT_DATA af;
	int dam_to_add = level / 5;
	if(is_affected(ch,skill_lookup("carnivorous tendancies")))
	{
	send_to_char("You can't eat any more meat!\n\r",ch);
	return;
	}
         af.where = TO_AFFECTS;
         af.type = sn;
         af.level = level;
         af.duration = level/3;
         af.location = APPLY_HITROLL;
	 af.modifier = dam_to_add;
	 af.bitvector = APPLY_NONE;
         affect_to_char(ch,&af);
	send_to_char("You are filled with the taste of blood!\n\r",ch);
}	
/* Banana Ward */
void spell_banana( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
	AFFECT_DATA af;
        if(is_affected(ch,skill_lookup("banana ward")))
        {
        send_to_char("Bananas can protect you no further.\n\r",ch);
        return;
        }
         af.where = TO_AFFECTS;
         af.type = sn;
         af.level = level;
         af.duration = level;
         af.location = APPLY_AC;
         af.modifier = -level*2;
         af.bitvector = APPLY_NONE;
         affect_to_char(ch,&af);
        send_to_char("You are surrounded by a strange banana ward.\n\r",ch);
	return;
}
/* Sour Shield */
void spell_sour( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    AFFECT_DATA af;

    if ( IS_AFFECTED2( ch, AFF_MIRROR ) )
    {
	send_to_char("The sour shield did not take hold.\n\r",ch);
	return;
    }
    if (is_affected(ch,skill_lookup("sour shield")))
    {
	send_to_char("You are already as sour as possible.\n\r",ch);
	return;
    }
    af.where	 = TO_AFFECTS;
    af.type      = gsn_mirror;
    af.level	 = level;
    af.whichaff	 = AFF2;
    af.duration  = number_range(10,30);
    af.modifier  = level/5;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_MIRROR;
    affect_to_char2( ch, &af );

    /* removing mirror lag
    af.bitvector = APPLY_NONE;
    af.duration = 3;
    af.modifier = 0;
    af.type = sn;		
    affect_to_char2(ch, &af);
    */

    send_to_char( "You are protected by a sour shield.\n\r",ch );
    return;
}

/* Blood spell */
void spell_bloodrush( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  if (!IS_SET(ch->shiftbits,PERM_VAMP) &&
      !IS_SET(ch->shiftbits,TEMP_VAMP)) 
    {
      send_to_char("You choke on the blood.\n\r",ch);
      return;
    }

  if( add_blood( ch, level * 2 ) )
    {
      send_to_char("You feel a slight ectasy as you ingest delicious blood.\n\r",ch);
    }
  else
    {
      send_to_char( "You're too full to enjoy this blood.\n\r", ch );
    }
}

/* Protection of Peaches */
void spell_peach( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    AFFECT_DATA af;
    if ( IS_AFFECTED(ch, AFF_SANCTUARY))
    {
       send_to_char("You are already protected by the essence of peaches.\n\r",ch);
       return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = 0;
    af.duration  = level/3;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( ch, &af );
    act("$n eats a peach and begins to glow.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You feel the power of peaches!\n\r", ch);
}

/* Power of Cheese */
void spell_cheese(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  int old_sn;
  AFFECT_DATA af;
  old_sn = sn;
	
  if( is_affected(ch,skill_lookup("power of the cheese")))
    {
      send_to_char("Too much cheese too soon gives you constipation.\n\r",ch);
      return;
    }
  
  send_to_char("Your hunger is filled by delicious cheese.\n\r",ch);
	
  old_sn = skill_lookup("frenzy");
  if(is_affected(ch,old_sn))
    {
      affect_strip(ch,old_sn);
      if ( skill_table[old_sn].msg_off )
	{
	  send_to_char( skill_table[old_sn].msg_off, ch );
	  send_to_char( "\n\r", ch );
	}
    }
	
  old_sn = skill_lookup("battle focus");
  if(is_affected(ch,old_sn))
    {
      affect_strip(ch,old_sn);
      if ( skill_table[old_sn].msg_off )
	{
	  send_to_char( skill_table[old_sn].msg_off, ch );
	  send_to_char( "\n\r", ch );
	}
    }
  
  old_sn = skill_lookup("rage");
  if(is_affected(ch,old_sn))
    {
      affect_strip(ch,old_sn);
      if ( skill_table[old_sn].msg_off )
	{
	  send_to_char( skill_table[old_sn].msg_off, ch );
	  send_to_char( "\n\r", ch );
	}
    }
    
  af.where = TO_AFFECTS;
  af.type = gsn_cheese;
  af.level = level;
  af.duration = level;
  af.whichaff = AFF2;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = APPLY_NONE;
  affect_to_char2(ch,&af);
  return;
}

void spell_fish(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        send_to_char("You have had enough fish for now.\n\r",ch);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 0 + (level > 5) + (level > 15) + (level > 25) + 2*(level > 35);
/*  af.duration  = level; */
    af.location  = APPLY_INT;
    af.modifier  = level/5;
/*  af.modifier  = 1 + (ch->level >= 18) + (ch->level >= 25) + (ch->level >= 32); */
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel enlightened upon tasting the fish.\n\r", victim );
    return;
}

void spell_supermob( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  AFFECT_DATA af;
  af.where = TO_AFFECTS;
  af.type = gsn_supermob;
  af.level = level;
  af.duration = -1;
  af.whichaff = AFF2;
  af.location = APPLY_SAVES;
  af.modifier = -50;
  af.bitvector = APPLY_NONE;
  affect_to_char2(ch,&af);
  af.location = APPLY_STR;
  af.modifier = 25;
  affect_to_char2(ch,&af);
  af.location = APPLY_DEX;
  af.modifier = 25;
  affect_to_char2(ch,&af);
}

/* New Wildmage spell */

extern void do_look( CHAR_DATA* ch, char* argument );

void spell_vanish( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  char buffer[120];
  CHAR_DATA *victim, *pet/*, *next*/;
  ROOM_INDEX_DATA *destination;
  ROOM_INDEX_DATA *was_room;
  int vnum;
/*bool gate_pet;*/
  bool done;
  AFFECT_DATA af;

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
       ||   IS_AFFECTED(ch,AFF_CURSE)
       /*||   victim->level >= level + 3*/
       /*||   (is_clan(victim) && !is_same_clan(ch,victim))*/
       /*||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)*/
       ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_PORTAL)
    /* ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER) )*/ )
       ||   ((IS_SET(victim->in_room->room_flags,ROOM_SAFE) 
              || IS_SET(victim->in_room->room_flags,ROOM_SEMISAFE)) 
              && in_fightlag(ch) 
              && !in_fightlag(victim) ) )
    {
      send_to_char( "You failed.\n\r", ch );
      if (victim)
	if (IS_AFFECTED(victim, AFF_DETECT_MAGIC) || IS_SET( victim->act_bits, PLR_HOLYLIGHT ) ){
	  sprintf(buffer,"Your senses tingle!\n\r");
	  send_to_char(buffer,victim);
	}
    
      return;
    }

  if( IS_SET( ch->in_room->trans_flags, TRANS_NO_PORTAL ) ||
      IS_SET( ch->in_room->trans_flags, TRANS_NO_PORTAL_OUT ) ||
      IS_SET( ch->in_room->trans_flags, TRANS_NO_SPELLS ) ||
      IS_SET( ch->in_room->trans_flags, TRANS_NO_SPELLS_OUT ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }
    
  if ( IS_AFFECTED(ch, AFF_CHARM)
       && ch->master != NULL
       && ch->in_room == ch->master->in_room )
    {
      send_to_char( "What?  And leave your beloved master?\n\r", ch );
      return;
    }
/*  if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
    gate_pet = TRUE;
  else
    gate_pet = FALSE;*/
  if (IS_AFFECTED(victim, AFF_DETECT_MAGIC)){
    sprintf(buffer,"You feel a slight vibration.\n\r");
    send_to_char(buffer,victim);
  }
  
  if (is_affected(victim, skill_lookup("hallowed discord"))) {
		return;
	}

  done = FALSE;
  while ( ! done )
    {
      vnum =  number_range(victim->in_room->area->min_vnum,victim->in_room->area->max_vnum);
      if (((destination = get_room_index( vnum ) ) )&&(destination!=victim->in_room))
	{

	  done = TRUE;

	  if( IS_SET( destination->room_flags, ROOM_IMP_ONLY ) ||
	      IS_SET( destination->room_flags, ROOM_GODS_ONLY ) ||
	      room_is_private( destination ) )
	    {
	      done = FALSE;
	    }


	  if( IS_SET( destination->trans_flags, TRANS_NO_TELEPORT ) ||
	      IS_SET( destination->trans_flags, TRANS_NO_TELEPORT_IN ) ||
	      IS_SET( destination->trans_flags, TRANS_NO_SPELLS ) ||
	      IS_SET( destination->trans_flags, TRANS_NO_SPELLS_IN ) )
	    {
	      done = FALSE;
	    }


	  if((vnum >= 1200) && (vnum <= 1299))
	    {
	      send_to_char("Yeah, right.",ch);
	      return;
	    }
	  if(vnum == 3143) done = FALSE;
	}

    }
  was_room = ch->in_room;
  act("$n shatters, turning into tiny shards which quickly vanish.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You shatter and move at high speed!\n\r",ch);
  ch->was_in_room = ch->in_room;
  char_from_room(ch);
  char_to_room(ch,destination);
/*ch->fightlag = FIGHT_LAG; */
  if ( !IS_NPC(ch) ) ch->pcdata->lastfight = current_time;
  do_look(ch,"auto");
  act("$n forms out of shards.",ch,NULL,NULL,TO_ROOM);
  ch->was_in_room = NULL;
  if(number_range(1,100) < 3)
    {
      af.where     = TO_AFFECTS;
      af.type      = sn;
      af.level     = level;
      af.duration  = 0;
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = AFF_CONFUSE;
      affect_to_char2( ch, &af );
      send_to_char("You feel disoriented.\n\r",ch);
    }
/*  if (gate_pet)
    {*/
    for ( pet = was_room->people; pet != NULL; )
    {
        if ( ch->pet == pet ||
           ( IS_AFFECTED2( pet, AFF2_SUMMONEDPET ) && pet->master == ch ) )
        {
/*        next = pet->next_in_room;		*/
          act("$n shatters suddenly!",pet,NULL,NULL,TO_ROOM);
          send_to_char("You shatter and reform yourself elsewhere.\n\r",pet);
          char_from_room(pet);
          char_to_room(pet,destination);
          act("$n forms out of shards.",pet,NULL,NULL,TO_ROOM);
          do_look(pet,"auto");
/*        pet = next;				*/
	  break;
        }
        else
          pet = pet->next_in_room;
    }
/*    }*/
    return;
}

void spell_lifetap( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  int victim_hp, dam;
  if (victim != ch) 
    {
      act("$n drains $N's life energy, feeding upon it!",
	  ch,NULL,victim,TO_NOTVICT);
      act("$n drains your life, for $s own benefit!",
	  ch,NULL,victim,TO_VICT);
      act("You feed on $N's life energy!",
	  ch,NULL,victim,TO_CHAR);
    }
        dam = dice(level,8);
	victim_hp = victim->hit;
	damage_old( ch, victim, dam, sn, DAM_NEGATIVE, TRUE); 
	if(victim != ch)
                ch->hit = UMIN(ch->hit + ((victim_hp - victim->hit)/2),ch->max_hit);
}

// [Zalyriel: 11/3/2013]
// Filian's Khoury spell for Arcaenum. Exchange health for mana. 
void spell_life_transfer (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  victim = (CHAR_DATA *) vo;

  if (victim->hit <= 100)
    {
      send_to_char
	("You aren't currently healthy enough to transfer life essence into energy.\n\r", ch);
      return;
    }

  send_to_char
    ("Your god answers your prayer by increasing your spirit at the cost of your physical health.\n\r",
     victim);
  act
    ("$n's burst of confidence seems to contradict $s sudden decrease in health.",
     victim, NULL, NULL, TO_ROOM);

  victim->mana = UMIN (victim->mana + 50, victim->max_mana);
  victim->hit -= 100;

  return;
}

// [Zalyriel: 11/3/2013]
// Filian's Khoury spell for Arcaenum. Mana DoT. 
// As of 11/4/2013, this spell still needs to be added to update.c.
// - 20 mana a tick, no regen.
void spell_mana_sear( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {

   CHAR_DATA *victim;
   victim = (CHAR_DATA *) vo;
   AFFECT_DATA af;
	
	if (victim == ch) {
      send_to_char ("You cannot drain your own life essence.\n\r", ch);
      return;
		}
		
	if (is_affected(victim, skill_lookup("mana sear"))) {
        act("Your magic didn't work. Perhaps $e is already afflicted?", ch, NULL, victim, TO_CHAR);
      return;
	}
	
	if (victim->mana <= 50){
      send_to_char ("$e does not have enough energy for you to sap.\n\r", ch);
      return;
    }
   
    if ( saves_spell ( level, ch, victim, DAM_HOLY ) )
    {
	act("$n shudders momentarily, but regains composure.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You shudder momentarily, but regain composure.\n\r",victim);
	return;
    }
	
	victim->mana -= 50;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 5;
    af.location  = APPLY_INT;
    af.modifier  = -5;
    af.bitvector = 0;
    affect_join( victim, &af, TRUE );
    send_to_char( "You feel a horrible tingling sensation in the base of your spine.\n\r", victim );
    act("$n looks weak as $s energy begins to drain.",victim,NULL,NULL,TO_ROOM);
    return;
}

/* Atrophy */
void spell_atrophy( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if(is_affected(victim,skill_lookup("atrophy")))
	{
		send_to_char("Atrophy has already set in.\n\r",ch);
		return;
	}
    if (saves_spell(level+20,ch,victim,DAM_DISEASE) || 
        (IS_NPC(victim) && IS_SET(victim->act_bits,ACT_UNDEAD)))
    {
	if (ch == victim)
	  send_to_char("Your tissues momentarily stop growing.\n\r",ch);
	else
	  act("$N is unchanged.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	  = sn;
    af.level	  = level;
    af.whichaff 	  = 10;
    af.duration  = 10;
    af.location  = APPLY_DEX;
    af.modifier  = -2; 
    af.bitvector = AFF2_NOREGEN;
    affect_to_char2(victim,&af);
   
    send_to_char
      ("Your body begins to waste away.\n\r",victim);
    act("$n's body begins to waste away.",
	victim,NULL,NULL,TO_ROOM);
}



void spell_blister( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if(is_affected(victim,skill_lookup("blister")))
	{
		send_to_char("A person's skin can only support so many blisters.\n\r",ch);
		return;
	}
    if (saves_spell(level,ch,victim,DAM_DISEASE) 
    || (IS_NPC(victim) && IS_SET(victim->act_bits,ACT_UNDEAD)))
    {
	if (ch == victim)
	  send_to_char("Your skin begins to blister, but you cool down.\n\r",ch);
	else
	  act("$N is unchanged.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	  = sn;
    af.level	  = level;
    af.whichaff 	  = 10;
    af.duration  = 2;
    af.location  = APPLY_DEX;
    af.modifier  = -2; 
    af.bitvector = AFF2_NOREGEN;
    affect_to_char2(victim,&af);
   
    send_to_char
      ("Your skin begins to blister and burn.\n\r",victim);
    act("$n's complexion turns bright red as $s skin blisters.",
	victim,NULL,NULL,TO_ROOM);
}




void spell_decay( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if(is_affected(victim,skill_lookup("decay")))
	{
		send_to_char("Their mortal form is already in a state of decay.\n\r",ch);
		return;
	}
    if (saves_spell(level,ch,victim,DAM_DISEASE) || 
        (IS_NPC(victim) && IS_SET(victim->act_bits,ACT_UNDEAD)))
    {
	if (ch == victim)
	  send_to_char("Your bones are momentarily chilled.\n\r",ch);
	else
	  act("$N is unchanged.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	  = sn;
    af.level	  = level;
    af.whichaff 	  = 10;
    af.duration  = 5;
    af.location  = APPLY_DEX;
    af.modifier  = -2; 
    af.bitvector = AFF2_NOREGEN;
    affect_to_char2(victim,&af);
   
    send_to_char
      ("Your bones begin to decay.\n\r",victim);
    act("$n's skeletal form staggers as $s bones begin to decay.",
	victim,NULL,NULL,TO_ROOM);
}



void spell_wilt( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if(is_affected(victim,skill_lookup("fester")))
	{
		send_to_char("They are already hurting.\n\r",ch);
		return;
	}
    if (saves_spell(level*2,ch,victim,DAM_DISEASE) || 
        (IS_NPC(victim) && IS_SET(victim->act_bits,ACT_UNDEAD)))
    {
	if (ch == victim)
	  send_to_char("You feel cold, but it passes.\n\r",ch);
	else
	  act("$N is unchanged.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	  = sn;
    af.level	  = level;
    af.whichaff 	  = 10;
    af.duration  = 0;
    af.location  = APPLY_STR;
    af.modifier  = -2; 
    af.bitvector = AFF2_NOREGEN;
    affect_to_char2(victim,&af);
   
    send_to_char
      ("You begin to hurt.\n\r",victim);
    act("$n shakes as $s body is stricken with pain.",
	victim,NULL,NULL,TO_ROOM);
}






void spell_wither_limb( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    int arm;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj;
    AFFECT_DATA af;
    arm = number_range(1,3); /* 1 = weapon, 2 = shield, 3 = held */
    if (saves_spell( level, ch,victim,DAM_DISEASE) || (victim->level > 50))
    {
	send_to_char("Your magic had no effect.\n\r",ch);
	return;
   }
   if(arm == 1) {
	   if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
		return;	
	    act( "Your arm decays, causing you to lose your grip on your weapon!", 
		 ch, NULL, victim, TO_VICT    );
	    act( "$N's arm decays, and $N loses $S grip on $S weapon. ", ch, NULL, victim, TO_CHAR    );
	    act( "$N's arm decays, and $S weapon falls out of $S hands.", ch, NULL, victim, TO_NOTVICT);
 	   obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
	obj_to_char_init( obj, victim );
    else
           obj_to_room( obj, victim->in_room );

	    af.where     = TO_AFFECTS;
	    af.type       = sn;
	    af.level      = level;
	    af.duration  = level/10;
	    af.location  = APPLY_NONE;	
	    af.modifier  = 1;
    	    af.bitvector = AFF_WITHER;
	    affect_to_char2(victim, &af);
    
	return;
}
   if(arm == 2) {
	   if ( ( obj = get_eq_char( victim, WEAR_SHIELD ) ) == NULL )
		return;	
	    act( "Your arm decays, causing you to lose your grip on your shield!", 
		 ch, NULL, victim, TO_VICT    );
	    act( "$N's arm decays, and $N loses $S grip on $S shield. ", ch, NULL, victim, TO_CHAR    );
	    act( "$N's arm decays, and $S shield falls out of $S hands.", ch, NULL, victim, TO_NOTVICT);
 	   obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
	obj_to_char_init( obj, victim );
    else
           obj_to_room( obj, victim->in_room );
	    af.where     = TO_AFFECTS;
	    af.type       = sn;
	    af.level      = level;
	    af.duration  = level/10;
	    af.location  = APPLY_NONE;	
	    af.modifier  = 2;
    	    af.bitvector = AFF_WITHER;
	    affect_to_char2(victim, &af);
	   return;
}

   if(arm == 3) {
	   if ( ( obj = get_eq_char( victim, WEAR_HOLD) ) == NULL )
		return;	
	    act( "Your arm decays, causing you to lose your grip!", 
		 ch, NULL, victim, TO_VICT    );
	    act( "$N's arm decays, and $N's hand becomes weak. ",  ch, NULL, victim, TO_CHAR );
	    act( "$N's arm decays, and something falls out of $S hands.", ch, NULL, victim, TO_NOTVICT);
 	   obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
	obj_to_char_init( obj, victim );
    else
           obj_to_room( obj, victim->in_room );
	    af.where     = TO_AFFECTS;
	    af.type       = sn;
	    af.level      = level;
	    af.duration  = level/10;
	    af.location  = APPLY_NONE;	
	    af.modifier  = 3;
    	    af.bitvector = AFF_WITHER;
	    affect_to_char2(victim, &af);
	return;
	}


}

/* New Cleric Spell to Balance Wither */
void spell_restore_limb( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

  if(!IS_AFFECTED2(victim,AFF_WITHER))
    {
        if (victim == ch)
          send_to_char("Your arm is fine.\n\r",ch);
        else
          act("$N's arms are perfectly healthy.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    if (check_dispel(level,victim,skill_lookup("wither limb")))
    {
	send_to_char("Your arm feels better.\n\r",victim);
	act("$n's arm becomes visibly stronger.",victim,NULL,NULL,TO_ROOM);
    }
    else
	send_to_char("Spell failed.\n\r",ch);
}


void spell_animate_dead( int sn, int level, CHAR_DATA* ch, void* vo, int target )
{
  CHAR_DATA *mob;
  AFFECT_DATA af;
  ROOM_INDEX_DATA *room;
  MOB_INDEX_DATA *pMobIndex;
  bool match = FALSE;
  OBJ_DATA *obj, *obj_next;
  
  if (ch->pet != NULL || has_summonedpets( ch ) )
    {
      send_to_char("You already have command of a creature.\n\r",ch);
      return;
    }

  room = ch->in_room;

  for (obj = room->contents; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      if(obj->item_type == ITEM_CORPSE_NPC) { match = TRUE; break; }
    }

  if(!match) 
    {
      send_to_char("There are no useful corpses to animate.\n\r",ch);
      return;
    }
 
  pMobIndex = get_mob_index(12);
 
  if (!pMobIndex)
    {
      return;
    }
 
  mob = create_mobile(pMobIndex);
  char_to_room(mob,ch->in_room);
  
  mob->max_hit = 50* obj->level + 1;
  mob->hit = mob->max_hit;
  mob->level = ch->level;
  
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
 
  extract_obj( obj );
  act ("$n utters an arcane word, and $N rises from the dead.",ch,NULL,mob, TO_ROOM);
  act ("You successfully raise $N from the dead. ",ch,NULL,mob, TO_CHAR);
}




void spell_shadow_step( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  ROOM_INDEX_DATA *destination, *was_room;
  CHAR_DATA *pet/*, *next*/;
  int vnum;
  int vnum_count;
  int attempts;
  int rand;
/*bool gate_pet;*/
  bool done;

  
  if( IS_AFFECTED( ch, AFF_CURSE ) ||
      IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) ||
      IS_SET( ch->in_room->room_flags, ROOM_IMP_ONLY ) ||
      IS_SET( ch->in_room->room_flags, ROOM_GODS_ONLY ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }

  if( IS_SET( ch->in_room->trans_flags, TRANS_NO_TELEPORT ) ||
      IS_SET( ch->in_room->trans_flags, TRANS_NO_TELEPORT_OUT ) ||
      IS_SET( ch->in_room->trans_flags, TRANS_NO_SPELLS ) ||
      IS_SET( ch->in_room->trans_flags, TRANS_NO_SPELLS_OUT ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }
	
	if (is_affected(ch, skill_lookup("hallowed discord"))) {

		 if (get_skill(ch, skill_lookup("gate")) < 1) {
			rand = number_range(1,6);
			if (rand == 1) {do_cast(ch, "portal meshra");}
			else if (rand == 2) {do_cast(ch, "portal high priest");}
			else if (rand == 3) {do_cast(ch, "portal clestus");}
			else if (rand == 4) {do_cast(ch, "portal pet dragon");}
			else if (rand == 5) {do_cast(ch, "portal grank");}
			else if (rand == 6) {do_cast(ch, "portal trolgar");}
			else {send_to_char("BUG.\n\r", ch);}
			return;
		}
	
		rand = number_range(1,6);
		if (rand == 1) {do_cast(ch, "gate meshra");}
		else if (rand == 2) {do_cast(ch, "gate high priest");}
		else if (rand == 3) {do_cast(ch, "gate clestus");}
		else if (rand == 4) {do_cast(ch, "gate pet dragon");}
		else if (rand == 5) {do_cast(ch, "gate grank");}
		else if (rand == 6) {do_cast(ch, "gate trolgar");}
		else {send_to_char("BUG.\n\r", ch);}
		return;
	}
      /*
  if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
    gate_pet = TRUE;
  else
    gate_pet = FALSE;
*/

  done = FALSE;
  vnum_count = ch->in_room->area->max_vnum - ch->in_room->area->min_vnum;
  attempts = 0;

  while( !done )
    {
      destination = NULL;
      vnum = number_range( ch->in_room->area->min_vnum,
			   ch->in_room->area->max_vnum );
      
      destination = get_room_index( vnum );
      
      if( destination != NULL && 
	  destination != ch->in_room &&
	  !IS_SET( destination->room_flags, ROOM_IMP_ONLY ) &&
	  !IS_SET( destination->room_flags, ROOM_GODS_ONLY ) &&
	  !IS_SET( destination->room_flags, ROOM_NEWBIES_ONLY ) &&
	  !room_is_private( destination ) &&
	  vnum != 3143 )
	{
	  done = TRUE;

	  if( IS_SET( destination->trans_flags, TRANS_NO_TELEPORT ) ||
	      IS_SET( destination->trans_flags, TRANS_NO_TELEPORT_IN ) ||
	      IS_SET( destination->trans_flags, TRANS_NO_SPELLS ) ||
	      IS_SET( destination->trans_flags, TRANS_NO_SPELLS_IN ) )
	    {
	      done = FALSE;
	    }
	}
      else
	{
	  attempts++;
	  
	  if( attempts == vnum_count )
	    {
	      done = TRUE;
	    }
	}
    }

  if( destination == NULL )
    {
      send_to_char( "You cannot find a shadowy spot.\n\r", ch );
      return;
    }
  was_room = ch->in_room;
  act("$n steps into the shadows, disappearing!",ch,NULL,NULL,TO_ROOM);
  send_to_char("You step into the shadows.\n\r",ch);

  ch->was_in_room = ch->in_room;
  char_from_room(ch);
  char_to_room(ch,destination);

/*ch->fightlag = FIGHT_LAG; */
  if ( !IS_NPC(ch) ) ch->pcdata->lastfight = current_time;
  do_look(ch,"auto");
  act("$n appears in the shadows!",ch,NULL,NULL,TO_ROOM);
  ch->was_in_room = NULL;

  for ( pet = was_room->people; pet != NULL; )
  {
      if ( ch->pet == pet ||
         ( IS_AFFECTED2( pet, AFF2_SUMMONEDPET ) && pet->master == ch ) )
      {
        act("$n steps into the shadows, disappearing!",pet,NULL,NULL,TO_ROOM);
        char_from_room(pet);
        char_to_room(pet,destination);
        act("$n appears in the shadows!",pet,NULL,NULL,TO_ROOM);
        do_look(pet,"auto");
        break;
      }
      else
        pet = pet->next_in_room;
  }
  return;
}


void spell_circle_of_protection( int sn, int level, CHAR_DATA* ch, void* vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED2( victim, AFF_ABSORB ) )
    {
      send_to_char("You are already surrounded by a circle of protection.\n\r",ch);
      return;
    }

  af.where	 = TO_AFFECTS;
  af.type      =  gsn_absorb;
  af.level	 = level;
  af.whichaff	 = AFF2;
  af.duration  = level/5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_ABSORB;
  affect_to_char2( victim, &af );
  send_to_char( "You are surrounded by a circle of protection.\n\r", victim );
  act("$n is surrounded by a circle of protection.", ch, NULL, NULL, TO_ROOM);
  return;
}

void spell_scry( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *old_room = ch->in_room;
  if ((victim = get_char_world(ch,target_name, FALSE)) == NULL) {
    send_to_char("You cannot locate them.\n\r",ch);
    return;
  }

  if (!IS_NPC(victim) && IS_IMMORTAL(victim)){
    send_to_char("You cannot locate them.\n\r",ch);
    return;
  }

  if( !IS_IMMORTAL( ch ) && IS_AFFECTED3( victim, AFF_VEIL ) )
    {
      send_to_char( "You cannot locate them.\n\r", ch );
      return;
    }

  if( IS_SET( victim->in_room->room_flags, ROOM_IMP_ONLY ) ||
      IS_SET( victim->in_room->room_flags, ROOM_GODS_ONLY ) )
    {
      send_to_char( "A powerful barrier blocks your vision.\n\r", ch );
      return;
    }

  if ( ch != victim )
  {
    char_from_room( ch );
    char_to_room( ch, victim->in_room );
  }
  do_look(ch, "auto");
  if ( ch != victim )
  {
    char_from_room(ch);
    char_to_room(ch, old_room);
  }

  if (IS_AFFECTED(victim, AFF_DETECT_MAGIC)){
    sprintf(buf,"You feel someone's eyes upon you!\n\r");
    send_to_char(buf,victim);
  }

  return;
}

void spell_veil( int sn, int level, CHAR_DATA* ch, void* vo, int target )
{
  AFFECT_DATA veil_af;

  if( is_affected( ch, gsn_veil ) )
    {
      send_to_char( "You have already veiled your presence.\n\r", ch );
      return;
    }

/*if( ch->fightlag > 0 )*/
    if ( in_fightlag(ch) )
    {
      send_to_char( "You have been in battle too recently to hide yourself.\n\r", ch );
      return;
    }

  if( IS_AFFECTED3( ch, AFF_VEIL_WAIT ) )
    {
      send_to_char( "Your presence is too obvious to veil.\n\r", ch );
      return;
    }

  veil_af.where = TO_AFFECTS;
  veil_af.type = gsn_veil;
  veil_af.level = level;
  veil_af.duration = ( ch->level * 2 );
  veil_af.location = APPLY_NONE;
  veil_af.modifier = 0;
  veil_af.bitvector = AFF_VEIL;
  affect_to_char3( ch, &veil_af );

  send_to_char( "You have veiled your presence.\n\r", ch );

}

void spell_conjure_familiar(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *mob;
    MOB_INDEX_DATA *pMobIndex;
    AFFECT_DATA af;

    if ( IS_NPC(ch) )
    {
        send_to_char( "Only PCs can use this spell.\n\r", ch );
        return;
    }

    if ( ch->move < 10 )
    {
        send_to_char( "You lack the energy to summon your familiar.\n\r", ch );
        return;
    }
 
    if ( ch->pet != NULL || has_summonedpets( ch ) ) 
    {
        send_to_char( "You already have a creature following you.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CURSE) )
    {
        send_to_char( "You are not comfortable doing this.\n\r", ch );
        return;
    }
    if ( IS_AFFECTED(ch, AFF_BLIND)
    ||   IS_AFFECTED3(ch, AFF_DIRT) )
    {
        send_to_char( "You cannot see your familiar to call it.\n\r", ch );
        return;
    }
    
    if ( (pMobIndex = get_mob_index(MOB_VNUM_FAMILIAR)) == NULL )
    {
        bug( "spell_conjure_familiar: Familiar mob index does not exist.", 0 );
        return;
    }

    ch->move -= 10;
 
    mob = create_mobile(pMobIndex);
    mob->sex = ch->pcdata->famsex;
    char_to_room(mob,ch->in_room);

/* restring the mob if the conjurer has all famdescs set */
    if ( !IS_NULLSTR( ch->pcdata->famname )
    &&   !IS_NULLSTR( ch->pcdata->famshort )
    &&   !IS_NULLSTR( ch->pcdata->famlong ) )
    {
        free_string( mob->name );
        free_string( mob->short_descr );
        free_string( mob->long_descr );
        free_string( mob->description );
        mob->name        = str_dup( ch->pcdata->famname );
        mob->short_descr = str_dup( ch->pcdata->famshort );
        mob->long_descr  = str_dup( ch->pcdata->famlong );
        mob->description = str_dup( ch->pcdata->famdesc );
    }

/* 
 * the placement of the acts is atypical; they are located here to ensure that
 * the player sees the familiar summoned, THEN follow.  If any mprogs depend
 * on these acts, bear in mind that, at the time they are shown, the mob is
 * not yet charmed or anything. 
 */
    act ("With a loud poof, a dark cloud of smoke appears next to $n.", 
        ch, NULL, mob, TO_ROOM);
    act ("$N steps out from the dark cloud of smoke.", ch, NULL, mob, TO_ROOM);

    send_to_char(
        "With a loud poof, a dark cloud of smoke appears next to you.\n\r",
        ch );
    act ( "$N steps out from the dark cloud of smoke.",ch,NULL, mob, TO_CHAR );
 
    add_follower(mob, ch);
    mob->leader = ch;
    ch->pet = mob;
    SET_BIT(mob->act_bits, ACT_PET);
    ch->charmies += 1;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = -1;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( mob, &af );

    spell_familiar_link(gsn_familiar_link, level, ch, (void *) ch, TARGET_CHAR);
 
    return;
}

void spell_conjure_bats(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *mob;
    MOB_INDEX_DATA *pMobIndex;
    AFFECT_DATA af;

    if ( IS_NPC(ch) )
    {
        send_to_char( "Only PCs can use this spell.\n\r", ch );
        return;
    }

    if ( ch->move < 10 )
    {
        send_to_char( "You lack the energy to summon bats.\n\r", ch );
        return;
    }
 
    if ( ch->pet != NULL || has_summonedpets( ch ) ) 
    {
        send_to_char( "You already have a creature following you.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CURSE) )
    {
        send_to_char( "You are not comfortable doing this.\n\r", ch );
        return;
    }
    if ( IS_AFFECTED(ch, AFF_BLIND)
    ||   IS_AFFECTED3(ch, AFF_DIRT) )
    {
        send_to_char( "You must be able to see what you want to conjure!\n\r", 
            ch );
        return;
    }

    if ( (pMobIndex = get_mob_index(MOB_VNUM_BATS)) == NULL )
    {
        bug( "spell_conjure_bats: bats mob index does not exist.", 0 );
        return;
    }

    ch->move -= 10;
 
    mob = create_mobile(pMobIndex);
    char_to_room(mob,ch->in_room);

    add_follower(mob, ch);
    mob->leader = ch;
    ch->pet = mob;
    SET_BIT(mob->act_bits, ACT_PET);
    ch->charmies += 1;

    act ("With a loud poof, a black cloud of smoke appears next to $n.", 
        ch, NULL, mob, TO_ROOM);
    act ("$N angrily flies out from the black cloud of smoke.", 
        ch, NULL, mob, TO_ROOM);
    act ("You successfully dominate $N.", ch, NULL, mob, TO_CHAR);

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 8 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( mob, &af );

    spell_summon_sick(gsn_sumsick,level,ch,(void *) ch,TARGET_CHAR);
 
    return;
}



void spell_familiar_link(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
 
    victim = (CHAR_DATA *) vo;
 
    if ( !is_affected(victim, sn) )
    {
        af.where     = TO_AFFECTS;
        af.type      = sn;
        af.level     = level;
        af.duration  = -1;
        af.location  = APPLY_MANA;
        af.modifier  = 25;
        af.bitvector = 0;
        affect_to_char( victim, &af );
        send_to_char( 
            "You sense the tie to your familiar increasing your power.\n\r", 
            victim );
    }
    return;
}

void spell_contrition( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
	short chance;
	int saves;
	
	chance = number_range(0,100);
    victim = (CHAR_DATA *) vo;
	saves = victim->saving_throw;

    if(is_safe_spell(ch,victim,FALSE) && victim != ch)
    {
      send_to_char("Not on that target.\n\r",ch);
      return;
    }
	
    if (is_affected(victim,skill_lookup("contrition")))
    {
		if (victim == ch) {
		send_to_char("You are already affected by the scales of justice.\n\r",ch);
		} else {
		act("$N is already affected by the scales of justice.",ch,NULL,victim,TO_CHAR);
		}
        return;
    }

	  if ( saves_spell( level, ch, victim, DAM_OTHER ) ) 
    {
      if (chance > 20)
	{
	  act("You fail to penetrate $N's soul with hallowed sorrow.",ch,NULL,victim,TO_CHAR);
	  return;
	}
    }
	
	
    af.where     = TO_AFFECTS;
    af.type      = gsn_contrition;
    af.level     = level;
    af.duration  = 2;
	af.bitvector = 0;
		
		/*set saves to 5 below minimum class requirement*/
	if ( class_table[ victim->class ].fMana ) {
	
	    af.location  = APPLY_SAVING_SPELL;
		af.modifier  = ((saves + 50) * -1) + 5;
		affect_to_char( victim, &af );
	
		af.location  = APPLY_DEX;
		af.modifier  = -5;
		affect_to_char( victim, &af );
		
	} else {
	
		af.location  = APPLY_SAVING_SPELL;
		af.modifier  = ((saves + 40) * -1) + 5;
		affect_to_char( victim, &af );
	
		af.location  = APPLY_STR;
		af.modifier  = -5;
		affect_to_char( victim, &af );
		
	}
		
    send_to_char( "You are overcome with hallowed sorrow.\n\r", victim );
    if ( ch != victim )
	act("You penetrate $N's soul with hallowed sorrow!",ch,NULL,victim,TO_CHAR);
    return;
}


/* VORPAL */
void spell_vorpalspell( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    damage_old( ch, ( CHAR_DATA *) vo, dice (level, 7) + level / 2, sn, DAM_NONE,TRUE);
    return;
}

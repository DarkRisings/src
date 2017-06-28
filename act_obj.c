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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"

extern long queststatus;

/* command procedures needed */
DECLARE_DO_FUN(do_split		);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_wake		);
DECLARE_DO_FUN(do_look		);

extern  bool has_incontainer_name( OBJ_DATA *obj_list, char *argument, sh_int cont_vnum, char *cont_name );

/*
 * Local functions.
 */
#define CD CHAR_DATA
#define OD OBJ_DATA
bool	remove_obj	args( (CHAR_DATA *ch, int iWear, bool fReplace ) );
void	wear_obj	args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
CD *	find_keeper	args( (CHAR_DATA *ch ) );
int	get_cost	args( (CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
void 	obj_to_keeper	args( (OBJ_DATA *obj, CHAR_DATA *ch ) );
OD *	get_obj_keeper	args( (CHAR_DATA *ch,CHAR_DATA *keeper,char *argument));

#undef OD
#undef	CD

char *rev_name[] = {"south", "west", "north", "east", "down", "up"};

/* for commerce code, books of power */
bool has_incontainer_abbrev( OBJ_DATA *obj_list, char *argument )
{
    OBJ_DATA *obj;
    bool found = FALSE;
    for ( obj = obj_list; obj; obj = obj->next_content )
    {
        if ( is_abbrev( argument, obj->name ) && obj->in_obj )
            return TRUE;
        if ( obj->contains )
            found = has_incontainer_abbrev( obj->contains, argument );
        if ( found )
            return TRUE;
    }
    return FALSE;
}

void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];
    char buf[MAX_STRING_LENGTH];

    if ( !CAN_WEAR(obj, ITEM_TAKE)  && (ch->level < LEVEL_IMMORTAL))
    {
	send_to_char( "You can't take that.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        act( "$d: you can't carry that many items.",
            ch, NULL, obj->short_descr, TO_CHAR );
        return;
    }

    if ( obj->item_type != ITEM_MONEY
    ||   get_carry_weight(ch) > can_carry_w(ch) )
    {

        if ((!obj->in_obj || obj->in_obj->carried_by != ch)
        &&  (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)))
        {
            act( "$d: you can't carry that much weight.",
                ch, NULL, obj->short_descr, TO_CHAR );
            return;
        }
    }

    if (obj->in_room != NULL)
    {
	for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	    if (gch->on == obj)
	    {
		act("$N appears to be using $p.",
		    ch,obj,gch,TO_CHAR);
		return;
	    }
    }
		

    if ( container != NULL )
    {
/*  	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  get_trust(ch) < obj->level-65)
	{
	    send_to_char("You are not powerful enough to use it.\n\r",ch);
	    return;
	} */

    	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  !CAN_WEAR(container, ITEM_TAKE)
	&&  !IS_OBJ_STAT(obj,ITEM_HAD_TIMER))
	    obj->timer = 0;	
/*      The acts precede the obj_from_char ONLY if the item type is mone-
        this is because money objects are destroyed when picked up, which
        would make referring to "obj" in the act refer to an invalid obj */
        if ( obj->item_type == ITEM_MONEY )
        {
            act( "You get $p from $P.", ch, obj, container, TO_CHAR );
            act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
        }
	REMOVE_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	obj_from_obj( obj );
    }
    else
    {
        if ( obj->item_type == ITEM_MONEY )
        {
            act( "You get $p.", ch, obj, container, TO_CHAR );
            act( "$n gets $p.", ch, obj, container, TO_ROOM );
        }
	obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_MONEY)
    {
	ch->silver += obj->value[0];
	ch->gold += obj->value[1];

/*** Wiznet commerce code ***/
        if ( (obj->value[1] + obj->value[0]/100) > MAX_MONEY_TRANSFER )
        {
            sprintf(buf, "%s picked up %d gold and %d silver in room %d.",
                IS_NPC( ch ) ? ch->short_descr : ch->name, obj->value[1],
                obj->value[0], ( ch->in_room ) ? ch->in_room->vnum : -1 );
            wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
            log_string( buf );
        }
/****************************/

        if (IS_SET(ch->act_bits,PLR_AUTOSPLIT))
        { /* AUTOSPLIT code */
    	  members = 0;
    	  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	  {
            if (!IS_AFFECTED(gch,AFF_CHARM) && is_same_group( gch, ch ) )
              members++;
    	  }

	  if ( members > 1 && (obj->value[0] > 1 || obj->value[1]))
	  {
	    sprintf(buffer,"%d %d",obj->value[0],obj->value[1]);
	    do_split(ch,buffer);	
	  }
        }
 
	extract_obj( obj );
    }
    else
    {
	obj_to_char( obj, ch );

/* The act has to be AFTER obj_to_char so that mobprogs that are triggered
   by the act can manipulate the object that is grabbed */
        if ( container != NULL )
        {
            act( "You get $p from $P.", ch, obj, container, TO_CHAR );
            act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
        }
        else
        {
            act( "You get $p.", ch, obj, container, TO_CHAR );
            act( "$n gets $p.", ch, obj, container, TO_ROOM );
        }

/*** Wiznet commerce code ***/
        if ( !container
        || ( container && !container->carried_by ) )
        {
            if ( is_name( "restring", obj->name )
            ||   is_name( "token", obj->name )
            ||   is_name( "questeq", obj->name ) 
            ||   is_abbrev( "-power", obj->name ) )
            {
                sprintf(buf, "%s picked up %s in room %d.",
                    IS_NPC( ch ) ? ch->short_descr : ch->name,
                    strip_color( obj->short_descr ),
                    ( ch->in_room ) ? ch->in_room->vnum : -1 );
                wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                log_string( buf );
            }
            else if ( obj->item_type == ITEM_CONTAINER 
                 ||   obj->item_type == ITEM_BOOK )
            {
                if ( has_incontainer_name( obj->contains, "restring", -5, NULL )
                ||   has_incontainer_name( obj->contains, "token", -5, NULL )
                ||   has_incontainer_name( obj->contains, "questeq", -5, NULL ) 
                ||   has_incontainer_abbrev( obj->contains, "-power" ) )
                {
                    sprintf(buf, "%s picked up a container with valuables in room %d.",
                        IS_NPC( ch ) ? ch->short_descr : ch->name,
                        ( ch->in_room ) ? ch->in_room->vnum : -1 );
                    wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                    log_string( buf );
                }
            }
        }
/****************************/
    }

    return;
}

void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("Your hand passes right through it!\n\r",ch);
	return;
	}

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"from"))
	argument = one_argument(argument,arg2);

    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Get what?\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents, TRUE );
	    if ( obj == NULL )
	    {
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

	    get_obj( ch, obj, NULL );
	}
	else
	{
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    get_obj( ch, obj, NULL );
		}
	    }

	    if ( !found ) 
	    {
		if ( arg1[3] == '\0' )
		    send_to_char( "I see nothing here.\n\r", ch );
		else
		    act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
	    }
	}
    }
    else
    {
	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, arg2, TRUE ) ) == NULL )
	{
	    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	switch ( container->item_type )
	{
	default:
	    send_to_char( "That's not a container.\n\r", ch );
	    return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
        case ITEM_BOOK:
	    break;
	}

	if ( IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( "It is closed.", ch, NULL, container->name, TO_CHAR );
	    return;
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->contains, TRUE );
	    if ( obj == NULL )
	    {
		act( "I see nothing like that in $P.",
		    ch, NULL, container, TO_CHAR );
		return;
	    }
	    get_obj( ch, obj, container );
	}
	else
	{
	    /* 'get all container' or 'get all.obj container' */
	    found = FALSE;
	    for ( obj = container->contains; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    if (container->pIndexData->vnum == OBJ_VNUM_PIT
		    &&  !IS_IMMORTAL(ch))
		    {
			send_to_char("Don't be so greedy!\n\r",ch);
			return;
		    }
		    get_obj( ch, obj, container );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    act( "I see nothing in $P.",
			ch, NULL, container, TO_CHAR );
		else
		    act( "I see nothing like that in $P.",
			ch, NULL, container, TO_CHAR );
	    }
	}
    }

    return;
}

void do_push_drag( CHAR_DATA *ch, char *argument, char *verb )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *in_room;
  ROOM_INDEX_DATA *to_room;
  CHAR_DATA *victim;
  CHAR_DATA* counter;
  EXIT_DATA *pexit;
  int door;
  int chance;
  int count = 0;
  int iGuild, iClass;
  bool failure = FALSE;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        sprintf( buf, "%s whom where?\n\r", capitalize(verb));
        send_to_char( buf, ch );
        return;
    }

    victim = get_char_room(ch,arg1, TRUE);

    if (!victim || !can_see(ch,victim))
    {
        sprintf(buf,"%s whom where?\n\r", capitalize(verb));
        send_to_char( buf, ch );
        return;
    }

    if ( IS_AFFECTED2(ch,AFF_GHOST) )
    {
        act( "Your hand passes right through $M!", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( !str_prefix( arg2, "north" ) ) door = 0;
    else if ( !str_prefix( arg2, "east"  ) ) door = 1;
    else if ( !str_prefix( arg2, "south" ) ) door = 2;
    else if ( !str_prefix( arg2, "west"  ) ) door = 3;
    else if ( !str_prefix( arg2, "up"    ) ) door = 4;
    else if ( !str_prefix( arg2, "down"  ) ) door = 5;
    else
    {
      sprintf( buf, "Alas, you cannot %s in that direction.\n\r", verb );
      send_to_char( buf, ch );
      return;
    }

    if ( ch == victim )
    {
      act( "You $t yourself about the room and look very silly.", 
        ch, verb, NULL, TO_CHAR );
      act( "$n decides to be silly and $t $mself about the room.", 
        ch, verb, NULL, TO_ROOM );
      return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) 
    &&  ch->master != NULL 
    &&  ch->master == victim )
    {
      if (!str_cmp( verb, "push" )) 
      {
	send_to_char("You don't want your beloved master to leave!\n\r", ch);
	return;
      }
      else 
      {
	send_to_char("They take you where they want to go, not the other way around.\n\r", ch);
	return;
      }
    }
  
    in_room = victim->in_room;
    if ( (pexit = in_room->exit[door]) == NULL
    ||   (to_room = pexit->u1.to_room) == NULL 
    ||   !can_see_room(victim,pexit->u1.to_room) )
    {
        sprintf( buf, "Alas, you cannot %s them that way.\n\r", verb );
        send_to_char( buf, ch );
        return;
    }

    if (IS_SET(pexit->exit_info, EX_CLOSED)
    && (!IS_AFFECTED(victim, AFF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS)))
    {
      if ( IS_SET( pexit->exit_info, EX_SECRET ) )
      {
        sprintf( buf, "Alas, you cannot %s them that way.\n\r", verb );
        send_to_char( buf, ch );
        return;
      }
      else
      {
        act( "You try to $t them through the $d.", ch, verb, pexit->keyword, TO_CHAR );
        act( "$n decides to $t you around!", ch, verb, victim, TO_VICT );
        act( "$n decides to $t $N around!", ch, verb, victim, TO_NOTVICT );
      }
      return;
    }

    if ( !str_cmp( verb, "drag" ) )
    {
        if (  IS_SET(pexit->exit_info, EX_CLOSED)
        &&  (!IS_AFFECTED(ch, AFF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS)))
        {
            if ( IS_SET( pexit->exit_info, EX_SECRET ) )
            {
                sprintf( buf, "Alas, you cannot %s them that way.\n\r", verb );
                send_to_char( buf, ch );
                return;
            }
            sprintf( buf, "The %s is closed.\n\r", pexit->keyword );
            failure = TRUE;
        }
        else if ( !can_fly ( ch, to_room ) )
        {
            strcpy(buf, "You can't fly.\n\r");
            failure = TRUE;
        }
        else if ( !can_swim ( ch, to_room ) )
        {
            strcpy(buf, "You need a boat to go there.\n\r" );
            failure = TRUE;
        }
        else if ( !can_swim_lava( ch, to_room ) )
        {
            strcpy(buf, "It's far too hot to go there.\n\r" );
            failure = TRUE;
        }
        if ( failure )
        {
            send_to_char( buf, ch );
            act( "$n decides to $t you around!", ch, verb, victim, TO_VICT );
            act( "$n decides to $t $N around!", ch, verb, victim, TO_NOTVICT );
            return;
        }
    }

  /* prevent third char being dragged pushed into private */
  if( IS_SET( to_room->room_flags, ROOM_PRIVATE ) )
    {
      for( counter = to_room->people; counter != NULL; counter = counter->next_in_room )
	{
	  if( !IS_NPC( counter ) )
	    count++;
	}
      
      if( ( !str_cmp( verb, "push" ) && count > 1 ) ||
	  ( !str_cmp( verb, "drag" ) && count > 0 ) )
	{
	  act( "$n attempts to $t $N out of the room.", ch, verb, victim, TO_NOTVICT );
	  act( "$n attempts to $t you out of the room.", ch, verb, victim, TO_VICT );
	  send_to_char( "They won't fit in that room.\n\r", ch );
	  return;
	}
    }

  if  (IS_NPC(victim)
  &&	 (IS_SET(victim->act_bits,ACT_TRAIN)
  ||	 IS_SET(victim->act_bits,ACT_PRACTICE)
  ||	 IS_SET(victim->act_bits,ACT_IS_HEALER)
  ||	 IS_SET(victim->act_bits,ACT_IS_CHANGER)
  ||	 IS_SET(victim->imm_flags,IMM_SUMMON)
  ||	 victim->pIndexData->pShop )) 
  {
    act("$n attempts to $t $N out of the room.",ch,verb,victim,TO_NOTVICT);
    send_to_char( "Try as you might, this being will not budge.\n\r",ch );
    return;
  }

    if ( !IS_NPC(victim) 
    && ( (IS_SET(victim->in_room->room_flags,ROOM_SAFE) 
       || IS_SET(victim->in_room->room_flags,ROOM_SEMISAFE))
    && ( !in_fightlag(victim) && !IS_AFFECTED3( victim, AFF_VEIL ) ) ) )
      {
	act("$n attempts to $t $N out of the room.",ch,verb,victim,TO_NOTVICT);
	act("$n attempts to $t you out of the room.",ch,verb,victim,TO_VICT);
	send_to_char( "They are safe from your malintentions.\n\r", ch );
	return;
      }
    
  if (!str_cmp( verb, "push" ) && (victim->position != POS_STANDING))
    {
      act("$n attempts to $t $N out of the room.",ch,verb,victim,TO_NOTVICT);
      act("$n attempts to $t you out of the room.",ch,verb,victim,TO_VICT);
      send_to_char( "You might want to try dragging your victim.\n\r", ch );
      return;
    }

  if (!str_cmp( verb, "drag" ) && (victim->position == POS_STANDING))
    {
      act("$n attempts to $t $N out of the room.",ch,verb,victim,TO_NOTVICT);
      act("$n attempts to $t you out of the room.",ch,verb,victim,TO_VICT);
      send_to_char( "You might want to try pushing your victim.\n\r", ch );
      return;
    }


  if (!is_room_owner(victim,pexit->u1.to_room) && 
      room_is_private( pexit->u1.to_room ) ){
    act("$n attempts to $t $N out of the room.",ch,verb,victim,TO_NOTVICT);
    act("$n attempts to $t you out of the room.",ch,verb,victim,TO_VICT);
    send_to_char( "They can't go in there.\n\r", ch );
    return;
  }

  for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
    {
      for ( iGuild = 0; iGuild < MAX_CLASSGUILD; iGuild ++)
	{
	  if ( iClass != victim->class
	       &&   pexit->u1.to_room->vnum == class_table[iClass].guild[iGuild] )
	    {
	      act("$n attempts to $t $N out of the room.",ch,verb,victim,TO_NOTVICT);
	      act("$n attempts to $t you out of the room.",ch,verb,victim,TO_VICT);
	      send_to_char( "They can't go in there.\n\r", ch );
	      return;
	    }
	}
    }


  chance = 70;
  chance += (ch->level - victim->level);
  chance += (ch->size - victim->size);
  chance += (get_curr_stat(ch,STAT_STR) - get_curr_stat(victim,STAT_STR));

    if (number_percent() > chance)
    {
        act("$n attempts to $t $N out of the room.",ch,verb,victim,TO_NOTVICT);
        act("$n attempts to $t you out of the room.",ch,verb,victim,TO_VICT);
        send_to_char( "They won't budge.\n\r", ch );
/*      ch->fightlag = FIGHT_LAG; */
        if ( !IS_NPC(ch) )
        {
            ch->pcdata->lastfight = current_time;
            if ( !IS_NPC(victim) ) victim->pcdata->lastpk = current_time;
        }
        return;
    }

    if ( ch->move >= ( 2 + move_cost(ch,to_room) ) )
    {
        ch->move -= 2;
        if (!str_cmp( verb, "drag" ))
	{
            move_char( ch, door, FALSE );
            act( "$N drags $n $t!", victim, dir_name[door], ch, TO_ROOM );
            act( "$N drags you $t!\n\r", victim, dir_name[door], ch, TO_CHAR );
            act( "You drag $n into the room.", victim, NULL, ch, TO_VICT );
            char_from_room( victim );
            char_to_room( victim, pexit->u1.to_room );
            do_look( victim, "auto" );
            act( "$N drags $n into the room.", victim, NULL, ch, TO_NOTVICT );
	}
        else if (!str_cmp( verb, "push" ))
	{
            if (victim->fighting)
            {
                act( "You cannot push $N while he is in combat", ch, NULL, 
                    victim, TO_CHAR );
                return;
            }
            else
            {
                act( "$N {Wslams{x into $n, pushing $m $t!", victim, 
                    dir_name[door], ch, TO_NOTVICT );
                act( "You slam into $N, pushing $M $t!", ch, dir_name[door],
                    victim, TO_CHAR );
                act( "$N {Wslams{x into you, pushing you $t!\n\r", victim, 
                    dir_name[door], ch, TO_CHAR );
                char_from_room( victim );
                char_to_room( victim, pexit->u1.to_room );
                do_look( victim, "auto" );

                act( "You notice movement from nearby to the $T.",
                    victim, NULL, rev_name[door], TO_ROOM );
                act( "$n {Wflies{x into the room!", victim, NULL, NULL, TO_ROOM);
            }
        }

        if ( !IS_NPC(ch) )
        {
            ch->pcdata->lastfight = current_time;
            if ( !IS_NPC(victim) )
            {
                ch->pcdata->lastpk = current_time;
                victim->pcdata->lastpk = current_time;
            }
        }

        if ( !IS_NPC( victim ) )
        {
            victim->pcdata->lastfight = current_time;
            if( victim->pcdata->push_count < PUSH_LIMIT )
                WAIT_STATE( victim, PULSE_VIOLENCE / 2 );
            victim->pcdata->push_count++;
        }
    }
    else
    {
        sprintf( buf, "You are too tired to %s anybody around!\n\r", verb );
        send_to_char( buf, ch );
    }
    return;
}
               
void do_push( CHAR_DATA *ch, char *argument )
{
    do_push_drag( ch, argument, "push" );
    return;
}

void do_drag( CHAR_DATA *ch, char *argument )
{
    do_push_drag( ch, argument, "drag" );
    return;
}

void do_put( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *container;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;

  if (IS_AFFECTED2(ch,AFF_GHOST)){
    send_to_char("Your hand passes right through it!\n\r",ch);
    return;
  }
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (!str_cmp(arg2,"in") || !str_cmp(arg2,"on"))
    argument = one_argument(argument,arg2);

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
      send_to_char( "Put what in what?\n\r", ch );
      return;
    }

  if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
      send_to_char( "You can't do that.\n\r", ch );
      return;
    }

  if ( ( container = get_obj_here( ch, arg2, TRUE ) ) == NULL )
    {
      act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
      return;
    }

  if ( container->item_type != ITEM_CONTAINER 
  &&   container->item_type != ITEM_QUIVER
  &&   container->item_type != ITEM_BOOK )
    {
      send_to_char( "That's not a container.\n\r", ch );
      return;
    }

  if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
      act( "$d is closed.", ch, NULL, container->short_descr, TO_CHAR );
      return;
    }

  if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
      /* 'put obj container' */
      if ( ( obj = get_obj_carry( ch, arg1, ch, TRUE ) ) == NULL )
	{
	  send_to_char( "You do not have that item.\n\r", ch );
	  return;
	}

      if ( obj == container )
	{
	  send_to_char( "You can't fold it into itself.\n\r", ch );
	  return;
	}

      if ( !can_drop_obj( ch, obj ) )
	{
	  send_to_char( "You can't let go of it.\n\r", ch );
	  return;
	}

      if( container->item_type == ITEM_QUIVER &&
	  obj->item_type != ITEM_ARROW )
	{
	  send_to_char( "You can only put arrows in a quiver.\n\r", ch );
	  return;
	}

      if (WEIGHT_MULT(obj) != 100 && container->pIndexData->vnum != OBJ_VNUM_PIT )
    	{
	  send_to_char("You have a feeling that would be a bad idea.\n\r",ch);
	  return;
        }

      if (get_obj_weight( obj ) + get_true_weight( container ) - container->weight
	  > (container->value[0] * 10) 
	  ||  get_obj_weight(obj) > (container->value[3] * 10))
	{
	  send_to_char( "It won't fit.\n\r", ch );
	  return;
	}
	
      if( container->pIndexData->vnum == OBJ_VNUM_PIT )
	{
	  if( strlen( obj->donor ) == 0 )
	    {
	      free_string( obj->donor );
	      obj->donor = str_dup( ch->name );
	    }

	  if( !CAN_WEAR(container,ITEM_TAKE) )
	    {
	      if (obj->timer)
		SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	      else
		obj->timer = number_range(100,200);
	    }
	}

        if ( !container->carried_by )
        {
            if ( is_name( "restring", obj->name )
            ||   is_name( "token", obj->name )
            ||   is_name( "questeq", obj->name )
            ||   is_abbrev( "-power", obj->name ) )
            {
                sprintf(buf, "%s dropped %s in room %d.",
                    IS_NPC( ch ) ? ch->short_descr : ch->name,
                    strip_color( obj->short_descr ),
                    ( ch->in_room ) ? ch->in_room->vnum : -1 );
                wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                log_string( buf );
            }
            else if ( obj->item_type == ITEM_CONTAINER
                 ||   obj->item_type == ITEM_BOOK )
            {
                if ( has_incontainer_name(obj->contains, "restring", -5, NULL)
                ||   has_incontainer_name(obj->contains, "token", -5, NULL)
                ||   has_incontainer_name(obj->contains, "questeq", -5, NULL)
                ||   has_incontainer_abbrev(obj->contains, "-power") )
                {
                    sprintf(buf, "%s dropped a container with valuables in room %d.",
                        IS_NPC( ch ) ? ch->short_descr : ch->name,
                        ( ch->in_room ) ? ch->in_room->vnum : -1 );
                    wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                    log_string( buf );
                }
            }
      }

      obj_from_char( obj );
      obj_to_obj( obj, container );
	
      if (IS_SET(container->value[1],CONT_PUT_ON))
	{
	  act("$n puts $p on $P.",ch,obj,container, TO_ROOM);
	  act("You put $p on $P.",ch,obj,container, TO_CHAR);
	}
      else
	{
	  act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	  act( "You put $p in $P.", ch, obj, container, TO_CHAR );
	}
    }
  else
    {
      /* 'put all container' or 'put all.obj container' */
      for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	  obj_next = obj->next_content;

	  if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	       &&   can_see_obj( ch, obj )
	       &&   WEIGHT_MULT(obj) == 100
	       &&   obj->wear_loc == WEAR_NONE
	       &&   obj != container
	       &&   can_drop_obj( ch, obj )
	       &&   get_obj_weight( obj ) + get_true_weight( container ) - container->weight
	       <= (container->value[0] * 10) 
	       &&   get_obj_weight(obj) <= (container->value[3] * 10))
	    {
	      if( container->item_type == ITEM_QUIVER 
              &&  obj->item_type != ITEM_ARROW )
		continue;

	      if( container->pIndexData->vnum == OBJ_VNUM_PIT )
		{
		  if( strlen( obj->donor ) == 0 )
		    {
		      free_string( obj->donor );
		      obj->donor = str_dup( ch->name );
		    }

		  if( !CAN_WEAR(obj, ITEM_TAKE) )
		    {
		      if (obj->timer)
			SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
		      else
			obj->timer = number_range(100,200);
		    }
		}

                if ( !container->carried_by )
                {
                    if ( is_name( "restring", obj->name )
                    ||   is_name( "token", obj->name )
                    ||   is_name( "questeq", obj->name )
                    ||   is_abbrev( "-power", obj->name ) )
                    {
                        sprintf(buf, "%s dropped %s in room %d.",
                            IS_NPC( ch ) ? ch->short_descr : ch->name,
                            strip_color( obj->short_descr ),
                            ( ch->in_room ) ? ch->in_room->vnum : -1 );
                        wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                        log_string( buf );
                    }
                    else if ( obj->item_type == ITEM_CONTAINER 
                         ||   obj->item_type == ITEM_BOOK )
                    {
                        if ( has_incontainer_name(obj->contains, "restring", -5, NULL )
                        ||   has_incontainer_name(obj->contains, "token", -5, NULL )
                        ||   has_incontainer_name(obj->contains, "questeq", -5, NULL ) 
                        ||   has_incontainer_abbrev(obj->contains, "-power" ) )
                        {
                            sprintf(buf, "%s dropped a container with valuables in room %d.",
                               IS_NPC( ch ) ? ch->short_descr : ch->name,
                                ( ch->in_room ) ? ch->in_room->vnum : -1 );
                            wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                            log_string( buf );
                        }
                    }
              }

	      obj_from_char( obj );
	      obj_to_obj( obj, container );

	      if (IS_SET(container->value[1],CONT_PUT_ON))
        	{
		  act("$n puts $p on $P.",ch,obj,container, TO_ROOM);
		  act("You put $p on $P.",ch,obj,container, TO_CHAR);
        	}
	      else
		{
		  act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
		  act( "You put $p in $P.", ch, obj, container, TO_CHAR );
		}
	    }
	}
    }
}

void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("Your hand passes right through it!\n\r",ch);
	return;
	}
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drop what?\n\r", ch );
	return;
    }

    if ( is_number( arg ) )
    {
	/* 'drop NNNN coins' */
	int amount, gold = 0, silver = 0;

	amount   = atoi(arg);
	argument = one_argument( argument, arg );
	if ( amount <= 0
	|| ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) && 
	     str_cmp( arg, "gold"  ) && str_cmp( arg, "silver") ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	if ( !str_cmp( arg, "coins") || !str_cmp(arg,"coin") 
	||   !str_cmp( arg, "silver"))
	{
	    if (ch->silver < amount)
	    {
		send_to_char("You don't have that much silver.\n\r",ch);
		return;
	    }

	    ch->silver -= amount;
	    silver = amount;
            if ( amount/100 > MAX_MONEY_TRANSFER )
            {
                sprintf(buf, "%s dropped %d silver in room %d.",
                    IS_NPC( ch ) ? ch->short_descr : ch->name, amount,
                    ( ch->in_room ) ? ch->in_room->vnum : -1 );
                wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                log_string( buf );
            }
	}

	else
	{
	    if (ch->gold < amount)
	    {
		send_to_char("You don't have that much gold.\n\r",ch);
		return;
	    }

	    ch->gold -= amount;
  	    gold = amount;
            if ( amount > MAX_MONEY_TRANSFER )
            {
                sprintf(buf, "%s dropped %d gold in room %d.",
                    IS_NPC( ch ) ? ch->short_descr : ch->name, amount, 
                    ( ch->in_room ) ? ch->in_room->vnum : -1 );
                wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                log_string( buf );
            }
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    switch ( obj->pIndexData->vnum )
	    {
	    case OBJ_VNUM_SILVER_ONE:
		silver += 1;
		extract_obj(obj);
		break;

	    case OBJ_VNUM_GOLD_ONE:
		gold += 1;
		extract_obj( obj );
		break;

	    case OBJ_VNUM_SILVER_SOME:
		silver += obj->value[0];
		extract_obj(obj);
		break;

	    case OBJ_VNUM_GOLD_SOME:
		gold += obj->value[1];
		extract_obj( obj );
		break;

	    case OBJ_VNUM_COINS:
		silver += obj->value[0];
		gold += obj->value[1];
		extract_obj(obj);
		break;
	    }
	}

	obj_to_room( create_money( gold, silver ), ch->in_room );
	act( "$n drops some coins.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "OK.\n\r", ch );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg, ch, TRUE ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
	act( "You drop $p.", ch, obj, NULL, TO_CHAR );

        if ( is_name( "restring", obj->name )
        ||   is_name( "token", obj->name )
        ||   is_name( "questeq", obj->name ) 
        ||   is_abbrev( "-power", obj->name ) )
        {
            sprintf(buf, "%s dropped %s in room %d.",
                IS_NPC( ch ) ? ch->short_descr : ch->name,
                strip_color( obj->short_descr ),
                ( ch->in_room ) ? ch->in_room->vnum : -1 );
            wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
            log_string( buf );
        }
        else if ( obj->item_type == ITEM_CONTAINER
             ||   obj->item_type == ITEM_BOOK )
        {
            if ( has_incontainer_name( obj->contains, "restring", -5, NULL )
            ||   has_incontainer_name( obj->contains, "token", -5, NULL )
            ||   has_incontainer_name( obj->contains, "questeq", -5, NULL ) 
            ||   has_incontainer_abbrev( obj->contains, "-power" ) )
            {
                sprintf(buf, "%s dropped a container with valuables in room %d.",
                    IS_NPC( ch ) ? ch->short_descr : ch->name,
                    ( ch->in_room ) ? ch->in_room->vnum : -1 );
                wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                log_string( buf );
            }
        }

	if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
	{
	    act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
	    act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
	    extract_obj(obj);
	}
    }
    else
    {
	/* 'drop all' or 'drop all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj ) )
	    {
		found = TRUE;
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
		act( "You drop $p.", ch, obj, NULL, TO_CHAR );
                if ( is_name( "restring", obj->name )
                ||   is_name( "token", obj->name )
                ||   is_name( "questeq", obj->name ) 
                ||   is_abbrev( "-power", obj->name ) )
                {
                    sprintf(buf, "%s dropped %s in room %d.",
                        IS_NPC( ch ) ? ch->short_descr : ch->name,
                        strip_color( obj->short_descr ),
                        ( ch->in_room ) ? ch->in_room->vnum : -1 );
                    wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                    log_string( buf );
                }
                else if ( obj->item_type == ITEM_CONTAINER 
                     ||   obj->item_type == ITEM_BOOK )
                {
                    if ( has_incontainer_name( obj->contains, "restring", -5, NULL )
                    ||   has_incontainer_name( obj->contains, "token", -5, NULL )
                    ||   has_incontainer_name( obj->contains, "questeq", -5, NULL ) 
                    ||   has_incontainer_abbrev( obj->contains, "-power" ) )
                    {
                        sprintf(buf, "%s dropped a container with valuables in room %d.",
                            IS_NPC( ch ) ? ch->short_descr : ch->name,
                            ( ch->in_room ) ? ch->in_room->vnum : -1 );
                        wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                        log_string( buf );
                    }
                }

        	if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
        	{
             	    act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
            	    act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
            	    extract_obj(obj);
        	}
	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		act( "You are not carrying anything.",
		    ch, NULL, arg, TO_CHAR );
	    else
		act( "You are not carrying any $T.",
		    ch, NULL, &arg[4], TO_CHAR );
	}
    }

    return;
}


void do_give_core( CHAR_DATA *ch, char *argument, bool force )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("Your hand passes right through it!\n\r",ch);
	return;
	}
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Give what to whom?\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) )
    {
	/* 'give NNNN coins victim' */
	int amount;
	bool silver;

	amount   = atoi(arg1);
	if ( amount <= 0
	|| ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) && 
	     str_cmp( arg2, "gold"  ) && str_cmp( arg2, "silver")) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	silver = str_cmp(arg2,"gold");

	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Give what to whom?\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, arg2, TRUE ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}


    if (IS_AFFECTED2(victim,AFF_GHOST)){
	send_to_char("Your hand passes right through them!\n\r",ch);
	return;
	}

	if ( (!silver && ch->gold < amount) || (silver && ch->silver < amount) )
	{
	    send_to_char( "You haven't got that much.\n\r", ch );
	    return;
	}

	if (silver)
	{
	    ch->silver		-= amount;
	    victim->silver 	+= amount;
            if ( amount/100 > MAX_MONEY_TRANSFER && victim != ch )
            {
                sprintf(buf, "%s gave %d silver to %s.", 
                    IS_NPC( ch ) ? ch->short_descr : ch->name, amount,
		    IS_NPC( victim ) ? victim->short_descr : victim->name );
		wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                log_string( buf );
            }
	}
	else
	{
	    ch->gold		-= amount;
	    victim->gold	+= amount;
            if ( amount > MAX_MONEY_TRANSFER && victim != ch )
            {
                sprintf(buf, "%s gave %d gold to %s.",
                    IS_NPC( ch ) ? ch->short_descr : ch->name, amount,
                    IS_NPC( victim ) ? victim->short_descr : victim->name );
                wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                log_string( buf );
            }
	}

	sprintf(buf,"$n gives you %d %s.",amount, silver ? "silver" : "gold");
	act( buf, ch, NULL, victim, TO_VICT    );
	act( "$n gives $N some coins.",  ch, NULL, victim, TO_NOTVICT );
	sprintf(buf,"You give $N %d %s.",amount, silver ? "silver" : "gold");
	act( buf, ch, NULL, victim, TO_CHAR    );

	/*
	 * Bribe trigger
	 */
	if ( IS_NPC(victim) && !IS_NPC( ch ) && HAS_TRIGGER( victim, TRIG_BRIBE ) )
	    mp_bribe_trigger( victim, ch, silver ? amount : amount * 100 );

	if (IS_NPC(victim) && IS_SET(victim->act_bits,ACT_IS_CHANGER))
	{
	    int change;

	    change = (silver ? 95 * amount / 100 / 100 
		 	     : 95 * amount);


	    if (!silver && change > victim->silver)
	    	victim->silver += change;

	    if (silver && change > victim->gold)
		victim->gold += change;

	    if (change < 1 && can_see(victim,ch))
	    {
		act(
	"$n tells you {t'I'm sorry, you did not give me enough to change.'{x"
		    ,victim,NULL,ch,TO_VICT);
		ch->reply = victim;
		sprintf(buf,"%d %s %s", 
			amount, silver ? "silver" : "gold",ch->name);
		do_give_core(victim,buf,force);
	    }
	    else if (can_see(victim,ch))
	    {
		sprintf(buf,"%d %s %s", 
			change, silver ? "gold" : "silver",ch->name);
		do_give_core(victim,buf,force);
		if (silver)
		{
		    sprintf(buf,"%d silver %s", 
			(95 * amount / 100 - change * 100),ch->name);
		    do_give_core(victim,buf,force);
		}
		act("$n tells you {t'Thank you.  Come again!'{x",
		    victim,NULL,ch,TO_VICT);
		ch->reply = victim;
	    }
	}
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1, ch, TRUE ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2, TRUE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_AFFECTED2(victim,AFF_GHOST)){
	send_to_char("Your hand passes right through them!\n\r",ch);
	return;
	}

    if (!force && IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    {
	act("$N tells you {t'Sorry, you'll have to sell that.'{x",
	    ch,NULL,victim,TO_CHAR);
	ch->reply = victim;
	return;
    }

    if (!force && !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if (!force && victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
	act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (!force && get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w( victim ) )
    {
	act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (!force && !can_see_obj( victim, obj ) )
    {
	act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );

    if ( victim != ch && 
       ( is_name( "restring", obj->name ) 
    ||   is_name( "token", obj->name )
    ||   is_name( "questeq", obj->name )
    ||   is_abbrev( "-power", obj->name ) ) )
    {
        sprintf(buf, "%s gave %s to %s.",
            IS_NPC( ch ) ? ch->short_descr : ch->name, 
            strip_color( obj->short_descr ),
            IS_NPC( victim ) ? victim->short_descr : victim->name );
        wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
        log_string( buf );
    }
    else if ( obj->item_type == ITEM_CONTAINER
         ||   obj->item_type == ITEM_BOOK )
    {
        if ( has_incontainer_name( obj->contains, "restring", -5, NULL )
        ||   has_incontainer_name( obj->contains, "token", -5, NULL )
        ||   has_incontainer_name( obj->contains, "questeq", -5, NULL ) 
        ||   has_incontainer_abbrev(obj->contains, "-power" ) )
        {
            sprintf(buf, "%s gave a container with valuables to %s.",
                IS_NPC( ch ) ? ch->short_descr : ch->name,
                IS_NPC( victim ) ? victim->short_descr : victim->name );
            wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
            log_string( buf );
        }
    }

    MOBtrigger = FALSE;
    act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
    act( "$n gives you $p.",   ch, obj, victim, TO_VICT    );
    act( "You give $p to $N.", ch, obj, victim, TO_CHAR    );
    MOBtrigger = TRUE;

    /*
     * Give trigger
     */
    if ( IS_NPC(victim) && !IS_NPC(ch) && HAS_TRIGGER( victim, TRIG_GIVE ) )
	mp_give_trigger( victim, ch, obj );

    return;
}

void do_give( CHAR_DATA *ch, char *argument )
{
    do_give_core( ch, argument, FALSE );
    return;
}

void do_fgive( CHAR_DATA *ch, char *argument )
{
    do_give_core( ch, argument, TRUE );
    return;
}

/* for poisoning weapons and food/drink */
void do_envenom(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
/*  AFFECT_DATA af;
    int percent,skill; */

    /* find out what */
    if (argument == '\0')
    {
	send_to_char("Envenom what item?\n\r",ch);
	return;
    }

    obj =  get_obj_list(ch,argument,ch->carrying, TRUE);

    if (obj== NULL)
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }
/*
    if ((skill = get_skill(ch,gsn_envenom)) < 1)
    { */
	send_to_char("Are you crazy? You'd poison yourself!\n\r",ch);
	return;
/*  }

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {
	if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	{
	    act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	if (number_percent() < skill)
	{
	    act("$n treats $p with deadly poison.",ch,obj,NULL,TO_ROOM);
	    act("You treat $p with deadly poison.",ch,obj,NULL,TO_CHAR);
	    if (!obj->value[3])
	    {
		obj->value[3] = 1;
		check_improve(ch,gsn_envenom,TRUE,4);
	    }
	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	    return;
	}

	act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
	if (!obj->value[3])
	    check_improve(ch,gsn_envenom,FALSE,4);
	WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	return;
     }

    if (obj->item_type == ITEM_WEAPON)
    {
        if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
        ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
        ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
        ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
        ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
        ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
        ||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
        {
            act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
            return;
        }

	if (obj->value[3] < 0 
	||  attack_table[obj->value[3]].damage == DAM_BASH)
	{
	    send_to_char("You can only envenom edged weapons.\n\r",ch);
	    return;
	}

        if (IS_WEAPON_STAT(obj,WEAPON_POISON))
        {
            act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
            return;
        }

	percent = number_percent();
	if (percent < skill)
	{
 
            af.where     = TO_WEAPON;
            af.type      = gsn_poison;
            af.level     = ch->level * percent / 100;
            af.duration  = ch->level/2 * percent / 100;
            af.location  = 0;
            af.modifier  = 0;
            af.bitvector = WEAPON_POISON;
            affect_to_obj(obj,&af);
 
            act("$n coats $p with deadly venom.",ch,obj,NULL,TO_ROOM);
	    act("You coat $p with venom.",ch,obj,NULL,TO_CHAR);
	    check_improve(ch,gsn_envenom,TRUE,3);
	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
            return;
        }
	else
	{
	    act("You fail to envenom $p.",ch,obj,NULL,TO_CHAR);
	    check_improve(ch,gsn_envenom,FALSE,3);
	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	    return;
	}
    }
 
    act("You can't poison $p.",ch,obj,NULL,TO_CHAR); */
    return;
}

void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *fountain;

    if (IS_AFFECTED2(ch,AFF_GHOST))
    {
	send_to_char("Your hand passes right through it!\n\r",ch);
	return;
    }

    argument = one_argument( argument, arg );
    if ( IS_NULLSTR( arg ) )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch, TRUE ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    one_argument( argument, arg );
    if ( IS_NULLSTR( arg ) )
    {
	for ( fountain = ch->in_room->contents; fountain != NULL;
              fountain = fountain->next_content )
	{
	    if ( fountain->item_type == ITEM_FOUNTAIN )
		break;
	}
        if ( fountain == NULL )
        {
            send_to_char( "There is no fountain here.\n\r", ch );
            return;
        }
    }
    else
    {
        fountain = get_obj_list( ch, arg, ch->in_room->contents, TRUE );
        if ( fountain == NULL )
        {
            send_to_char( "You can't find that fountain.\n\r", ch );
            return;
        }
        else if ( fountain->item_type != ITEM_FOUNTAIN )
        {
            send_to_char("You cannot fill from that.\n\r", ch );
            return;
        }
    }

/*  New parameter parsing above allows players to specify which fountain
    01/18/2009 gkl
    for ( fountain = ch->in_room->contents; fountain != NULL;
	fountain = fountain->next_content )
    {
	if ( fountain->item_type == ITEM_FOUNTAIN )
	{
	    found = TRUE;
	    break;
	}
    }
*/


    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "You can't fill that.\n\r", ch );
	return;
    }


    if ( obj->value[1] != 0 && obj->value[2] != fountain->value[2] )
    {
	send_to_char( "There is already another liquid in it.\n\r", ch );
	return;
    }

    if ( obj->value[1] >= obj->value[0] )
    {
	send_to_char( "Your container is full.\n\r", ch );
	return;
    }

    sprintf(buf,"You fill $p with %s from $P.",
	liq_table[fountain->value[2]].liq_name);
    act( buf, ch, obj,fountain, TO_CHAR );
    sprintf(buf,"$n fills $p with %s from $P.",
	liq_table[fountain->value[2]].liq_name);
    act(buf,ch,obj,fountain,TO_ROOM);
    obj->value[2] = fountain->value[2];
    obj->value[1] = obj->value[0];
    return;
}

void do_pour (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
    OBJ_DATA *out, *in;
    CHAR_DATA *vch = NULL;
    int amount;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("Your hand passes right through it!\n\r",ch);
	return;
	}
    argument = one_argument(argument,arg);
    
    if (arg[0] == '\0' || argument[0] == '\0')
    {
	send_to_char("Pour what into what?\n\r",ch);
	return;
    }
    

    if ((out = get_obj_carry(ch,arg, ch, TRUE)) == NULL)
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if (out->item_type != ITEM_DRINK_CON)
    {
	send_to_char("That's not a drink container.\n\r",ch);
	return;
    }

    if (!str_cmp(argument,"out"))
    {
	if (out->value[1] == 0)
	{
	    send_to_char("It's already empty.\n\r",ch);
	    return;
	}

	out->value[1] = 0;
	out->value[3] = 0;
        if ( out->value[4] < 0 )
            out->value[4] = 0;
	sprintf(buf,"You invert $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,out,NULL,TO_CHAR);
	
	sprintf(buf,"$n inverts $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,out,NULL,TO_ROOM);
	return;
    }

    if ((in = get_obj_here(ch,argument, TRUE)) == NULL)
    {
	vch = get_char_room(ch,argument, TRUE);

	if (vch == NULL)
	{
	    send_to_char("Pour into what?\n\r",ch);
	    return;
	}

	in = get_eq_char(vch,WEAR_HOLD);

	if (in == NULL)
	{
	    send_to_char("They aren't holding anything.",ch);
 	    return;
	}
    }

    if (in->item_type != ITEM_DRINK_CON)
    {
	send_to_char("You can only pour into other drink containers.\n\r",ch);
	return;
    }
    
    if (in == out)
    {
	send_to_char("You cannot change the laws of physics!\n\r",ch);
	return;
    }

    if (in->value[1] != 0 && in->value[2] != out->value[2])
    {
	send_to_char("They don't hold the same liquid.\n\r",ch);
	return;
    }

    if (out->value[1] == 0)
    {
	act("There's nothing in $p to pour.",ch,out,NULL,TO_CHAR);
	return;
    }

    if (in->value[1] >= in->value[0])
    {
	act("$p is already filled to the top.",ch,in,NULL,TO_CHAR);
	return;
    }

    amount = UMIN(out->value[1],in->value[0] - in->value[1]);

    in->value[1] += amount;
    out->value[1] -= amount;
    in->value[2] = out->value[2];
    
    if (vch == NULL)
    {
    	sprintf(buf,"You pour %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
    	act(buf,ch,out,in,TO_CHAR);
    	sprintf(buf,"$n pours %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
    	act(buf,ch,out,in,TO_ROOM);
    }
    else
    {
        sprintf(buf,"You pour some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_CHAR);
	sprintf(buf,"$n pours you some %s.",
	    liq_table[out->value[2]].liq_name);
	act(buf,ch,NULL,vch,TO_VICT);
        sprintf(buf,"$n pours some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_NOTVICT);
	
    }
}

void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("Ghosts don't need to drink.\n\r",ch);
	return;
	}
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
	    if ( obj->item_type == ITEM_FOUNTAIN )
		break;
	}

	if ( obj == NULL )
	{
	    send_to_char( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, arg, TRUE ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
    {
	send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
	return;
    }

    switch ( obj->item_type )
    {
    default:
	send_to_char( "You can't drink from that.\n\r", ch );
	return;

    case ITEM_FOUNTAIN:
        if ( ( liquid = obj->value[2] )  < 0 )
        {
            bug( "Do_drink: bad liquid number %d.", liquid );
            liquid = obj->value[2] = 0;
        }
	amount = liq_table[liquid].liq_affect[1] * 3;
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "It is already empty.\n\r", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] )  < 0 )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

        amount = liq_table[liquid].liq_affect[1];
        amount = UMIN(amount, obj->value[1]);
	break;
     }
    act( "$n drinks $T from $p.",
	ch, obj, liq_table[liquid].liq_name, TO_ROOM );
    act( "You drink $T from $p.",
	ch, obj, liq_table[liquid].liq_name, TO_CHAR );

    gain_condition( ch, COND_DRUNK,
	amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36 );

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	send_to_char( "You feel drunk.\n\r", ch );
	
    if ( obj->value[3] != 0 )
    {
	/* The drink was poisoned ! */
	AFFECT_DATA af;

	act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You choke and gag.\n\r", ch );
	af.where     = TO_AFFECTS;
	af.type      = gsn_poison;
	af.level	 = number_fuzzy(amount); 
	af.duration  = 3 * amount;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_POISON;
	affect_join( ch, &af, TRUE );
    }

    if (obj->value[0] > 0)
        obj->value[1] -= amount;


    if ( obj->value[4] > 0 )
    {
        obj_cast_spell( obj->value[4], obj->level, ch, ch, obj );
        WAIT_STATE(ch,PULSE_VIOLENCE);
    }
    else if ( obj->value[4] < 0 )
    {
        obj_cast_spell( obj->value[4]*-1, obj->level, ch, ch, obj );
        if ( obj->value[1] == 0 && obj->item_type != ITEM_FOUNTAIN )
            obj->value[4] = 0;
        WAIT_STATE(ch,PULSE_VIOLENCE);
    }
	
    return;
}



void do_eat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;


    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("Ghosts don't need to eat!\n\r",ch);
	return;
	}

    if( IS_AFFECTED3( ch, AFF_FROG ) )
      {
	send_to_char( "You dart your tongue at a buzzing fly...and miss!", ch );
	act( "A frog darts $s tongue at a buzzing fly...and misses!", ch, NULL, NULL, TO_ROOM );
	return;
      }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch, TRUE ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(ch) )
    {
	if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
	{
	    send_to_char( "That's not edible.\n\r", ch );
	    return;
	}

    }

    act( "$n eats $p.",  ch, obj, NULL, TO_ROOM );
    act( "You eat $p.", ch, obj, NULL, TO_CHAR );

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
	if ( obj->value[3] != 0 )
	{
	    /* The food was poisoned! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );

	    af.where	 = TO_AFFECTS;
	    af.type      = gsn_poison;
	    af.level 	 = number_fuzzy(obj->value[0]);
	    af.duration  = 2 * obj->value[0];
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af, TRUE );
	}
	break;

    case ITEM_PILL:
	obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
        obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL );
	WAIT_STATE(ch,PULSE_VIOLENCE);
	break;
    }

    extract_obj( obj );
    return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace )
	return FALSE;

    if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE)
    && (ch->level <= 51))
    {
	act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }

    unequip_char( ch, obj );
    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
    return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace )
{
    AFFECT_DATA *af;
    if ( obj->item_type == ITEM_LIGHT )
    {
	if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
	    return;
	act( "$n holds $p and a light shines forth from it.", 
		ch, obj, NULL, TO_ROOM );
	act( "You hold $p and a light shines forth from it.",  
		ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LIGHT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
	{
	    act( "$n slides $p onto $s left finger.",    ch, obj, NULL, TO_ROOM );
	    act( "You slide $p onto your left finger.",  ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
	{
	    act( "$n slides $p onto $s right finger.",   ch, obj, NULL, TO_ROOM );
	    act( "You slide $p onto your right finger.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}

	bug( "Wear_obj: no free finger.", 0 );
	send_to_char( "You already wear two rings.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
	    act( "$n places $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You place $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
	    act( "$n places $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You place $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}

	bug( "Wear_obj: no free neck.", 0 );
	send_to_char( "You already wear two neck items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
	if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
	    return;
	act( "$n puts $p on $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You put $p on your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
	if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
	    return;
	act( "$n places $p on $s head.",   ch, obj, NULL, TO_ROOM );
	act( "You place $p on your head.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
	if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
	    return;
	act( "$n steps into $p.",   ch, obj, NULL, TO_ROOM );
	act( "You step into $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
	if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
	    return;
	act( "$n pulls on $p.",   ch, obj, NULL, TO_ROOM );
	act( "You pull on $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
	if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
	    return;
	act( "$n slips $p on $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You slip $p on your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
	if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
	    return;
	act( "$n straps $p on $s arms.",   ch, obj, NULL, TO_ROOM );
	act( "You strap $p on your arms.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
	    return;
	act( "$n pulls $p about $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You pull $p about your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
	if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	    return;
	act( "$n fastens $p about $s waist.",   ch, obj, NULL, TO_ROOM );
	act( "You fasten $p about your waist.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
	{
	    act( "$n snaps $p around $s left wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You snap $p around your left wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
	{
	    act( "$n snaps $p around $s right wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You snap $p around your right wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}

	bug( "Wear_obj: no free wrist.", 0 );
	send_to_char( "You already wear two wrist items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
	OBJ_DATA *weapon;
        bool templar_bonus = FALSE;

	if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
	    return;

	weapon = get_eq_char(ch,WEAR_WIELD);
        if ( !IS_NPC(ch)
	&&   weapon != NULL
	&&   weapon->item_type == ITEM_WEAPON
	&&   weapon->value[0] == WEAPON_SPEAR
        &&   ch->class == class_lookup("templar") 
        &&   get_skill(ch,gsn_spear) == 100 )
	    templar_bonus = TRUE;

	if ( weapon != NULL 
        &&   ch->size < SIZE_HUGE 
        &&  !templar_bonus
	&&   IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS) )
	{
	    send_to_char("Your hands are tied up with your weapon!\n\r",ch);
	    return;
	}
	if(IS_AFFECTED2(ch,AFF_WITHER))
	{
	     for(af = ch->affected; af != NULL; af = af->next)
	     {
		if((af->type == skill_lookup("wither limb")) &&
		  (af->modifier == 2))
		{
			send_to_char("Your arm is too weak.\n\r",ch);
			return;
		}
	    }   
	}

	act( "$n straps $p on $s shield arm.", ch, obj, NULL, TO_ROOM );
	act( "You strap $p on your shield arm.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_SHIELD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	int sn,skill;
	bool templar_bonus = FALSE;

	if ( !remove_obj( ch, WEAR_WIELD, fReplace ) )
	    return;

	if ( !IS_NPC(ch) 
	&& get_obj_weight(obj) > (str_app[get_curr_stat(ch,STAT_STR)].wield  
		* 10))
	{
	    send_to_char( "It is too heavy for you to wield.\n\r", ch );
	    return;
	}

        if ( !IS_NPC(ch)
	&&   obj->item_type == ITEM_WEAPON
	&&   obj->value[0] == WEAPON_SPEAR
        &&   ch->class == class_lookup("templar") 
        &&   get_skill(ch,gsn_spear) == 100 )
	    templar_bonus = TRUE;


	if (IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
	&&  !IS_NPC(ch) && ch->size < SIZE_HUGE 
 	&&  get_eq_char(ch,WEAR_SHIELD) != NULL
        &&  !templar_bonus)
	{
	    send_to_char("You need two hands free for that weapon.\n\r",ch);
	    return;
	}
	if(IS_AFFECTED2(ch,AFF_WITHER))
	{
	     for(af = ch->affected; af != NULL; af = af->next)
	     {
		if((af->type == skill_lookup("wither limb")) &&
		  (af->modifier == 1))
		{
			send_to_char("Your arm is too weak.\n\r",ch);
			return;
		}
	    }   
	}
	act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
	act( "You wield $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WIELD );

        sn = get_weapon_sn(ch);

	if (sn == gsn_hand_to_hand)
	   return;

        skill = get_weapon_skill(ch,sn);
 
        if (skill >= 100)
            act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
        else if (skill > 85)
            act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 70)
            act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 50)
            act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
        else if (skill > 25)
            act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
        else if (skill > 1)
            act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
        else
            act("You don't even know which end is up on $p.",
                ch,obj,NULL,TO_CHAR);

	return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
	if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
	    return;
	if(IS_AFFECTED2(ch,AFF_WITHER))
	{
	     for(af = ch->affected; af != NULL; af = af->next)
	     {
		if((af->type == skill_lookup("wither limb")) &&
		  (af->modifier == 3))
		{
			send_to_char("Your arm is too weak.\n\r",ch);
			return;
		}
	    }   
	}
	act( "$n grasps $p in $s hand.",   ch, obj, NULL, TO_ROOM );
	act( "You grasp $p in your hand.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HOLD );
	return;
    }

    if ( CAN_WEAR(obj,ITEM_WEAR_FLOAT) )
    {
	if (!remove_obj(ch,WEAR_FLOAT, fReplace) )
	    return;
	act("$n releases $p to float next to $m.",ch,obj,NULL,TO_ROOM);
	act("You release $p and it floats next to you.",ch,obj,NULL,TO_CHAR);
	equip_char(ch,obj,WEAR_FLOAT);
	return;
    }

    if ( CAN_WEAR(obj,ITEM_WEAR_PRIDE) )
    {
	if (!remove_obj(ch,WEAR_PRIDE, fReplace) )
	    return;
	act("$n wears $p with pride.",ch,obj,NULL,TO_ROOM);
	act("You wear $p with pride.",ch,obj,NULL,TO_CHAR);
	equip_char(ch,obj,WEAR_PRIDE);
	return;
    }

    if ( fReplace )
	send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

    return;
}



void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("Your hand passes right through it!\n\r",ch);
	return;
	}
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wear, wield, or hold what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
		wear_obj( ch, obj, FALSE );
	}
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg, ch, TRUE ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	wear_obj( ch, obj, TRUE );
    }

    return;
}



void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("Your hand passes right through it!\n\r",ch);
	return;
	}
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_wear( ch, arg, TRUE ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    remove_obj( ch, obj->wear_loc, TRUE );
    return;
}



void do_sacrifice( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char godname[24];
  OBJ_DATA *obj;
  int min_silver;
  int max_silver;
  int silver;
  int chgod;
    
  /* variables for AUTOSPLIT */
  CHAR_DATA *gch;
  int members;
  char buffer[100];

  if (IS_AFFECTED2(ch,AFF_GHOST))
    {
      send_to_char("Your hand passes right through it!\n\r",ch);
      return;
    }

  one_argument( argument, arg );

  chgod = ( IS_NPC(ch) ? GOD_MOB : ch->pcdata->god );
  strcpy(godname,god_table[chgod].descr);

  if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) ) 
  {
    if( !IS_NPC( ch ) )
    {
      act("$n offers $mself to $t!",ch,godname,NULL,TO_ROOM);
      act("You offer yourself to $t!",ch,godname,NULL,TO_CHAR);
    }
    return;
  }
  
  obj = get_obj_list( ch, arg, ch->in_room->contents, TRUE );
  if ( obj == NULL )
    {
      send_to_char( "You can't find it.\n\r", ch );
      return;
    }
  
  if ( obj->item_type == ITEM_CORPSE_PC )
    {
      if (obj->contains)
        {
	  send_to_char(
		       "Your mom wouldn't like that.\n\r",ch);
	  return;
        }
    }


  if ( !CAN_WEAR(obj, ITEM_TAKE) || CAN_WEAR(obj, ITEM_NO_SAC))
    {
      act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
      return;
    }
  
  if (obj->in_room != NULL)
    {
      for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	if (gch->on == obj)
	  {
	    act("$N appears to be using $p.",
		ch,obj,gch,TO_CHAR);
	    return;
	  }
    }

  min_silver = 5 * ( obj->level > 3 ? ( obj->level - 3 ) : 1 );
  max_silver = 5 * ( obj->level + 3 );
  silver = number_range( min_silver, max_silver );

  
  if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    silver = UMIN(silver,obj->cost);

  if (silver == 1)
    {
      if (IS_NPC(ch))
        {
          send_to_char("Tyrin gives you one silver coin for your sacrifice./n/r",ch);
        }
      else
        {
	  if( ch->pcdata->god != 0 )
	    sprintf(buf, "%s gives you one silver coin for your sacrifice.\n\r",god_table[ch->pcdata->god].name );
	  else
	    sprintf( buf, "You receive one silver coin for your sacrifice.\n\r" );

	  send_to_char(buf,ch);
        }
    }
  
    else
      {
	if(IS_NPC(ch))
	  {
	    sprintf(buf,"Tyrin gives you %d silver coins for your sacrifice.\n\r",silver);
	    send_to_char(buf,ch);
	  }
	else
	  {
	    if( ch->pcdata->god != 0 )
	      sprintf(buf,"%s gives you %d silver coins for your sacrifice.\n\r",god_table[ch->pcdata->god].name,silver);
	    else
	      sprintf( buf, "You receive %d silver coins for your sacrifice.\n\r", silver );

	    send_to_char(buf,ch);
	  }
      }
  
    ch->silver += silver;
    
    if (IS_SET(ch->act_bits,PLR_AUTOSPLIT) )
      { /* AUTOSPLIT code */
    	members = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	  {
    	    if ( is_same_group( gch, ch ) )
	      members++;
	  }
	
	if ( members > 1 && silver > 1)
	  {
	    sprintf(buffer,"%d",silver);
	    do_split(ch,buffer);	
	  }
      }
    
    
    if( IS_NPC( ch ) )
      {
	act( "$n sacrifices $p to $T.", ch, obj, GOD_MOB, TO_ROOM );
      }
    else if( ch->pcdata->god != 0 )
      {
	act( "$n sacrifices $p to $T.", ch, obj, god_table[ ch->pcdata->god ].name, TO_ROOM );
      }
    else
      {
	act( "$n sacrifices $p.", ch, obj, NULL, TO_ROOM );
      }
    
    wiznet("$N sends up $p as a burnt offering.",ch,obj,WIZ_SACCING,0,0);
    extract_obj( obj );
    return;
    
}



void do_quaff( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("Your hand passes right through it!\n\r",ch);
	return;
	}
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Quaff what?\n\r", ch );
	return;
    }


    if ( ( obj = get_obj_carry( ch, arg, ch, TRUE ) ) == NULL )
    {
	send_to_char( "You do not have that potion.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION )
    {
	send_to_char( "You can quaff only potions.\n\r", ch );
	return;
    }


    act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
    act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );
    WAIT_STATE( ch, PULSE_VIOLENCE );
    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL );

    extract_obj( obj );
    return;
}



void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("Your hand passes right through it!\n\r",ch);
	return;
	}
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( scroll = get_obj_carry( ch, arg1, ch, TRUE ) ) == NULL )
    {
	send_to_char( "You do not have that scroll.\n\r", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	send_to_char( "You can recite only scrolls.\n\r", ch );
	return;
    }


    obj = NULL;
    if ( arg2[0] == '\0' )
    {
	victim = ch;
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg2, TRUE ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg2, TRUE ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
    act( "You recite $p.", ch, scroll, NULL, TO_CHAR );

    WAIT_STATE( ch, PULSE_VIOLENCE );
   
    if (number_percent() >= 20 + get_skill(ch,gsn_scrolls) * 4/5)
    {
	send_to_char("You mispronounce a syllable.\n\r",ch);
	check_improve(ch,gsn_scrolls,FALSE,1);
    }

    else
    {
    	obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
    	obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
    	obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
        obj_cast_spell( scroll->value[4], scroll->value[0], ch, victim, obj );
	check_improve(ch,gsn_scrolls,TRUE,1);
    }

    extract_obj( scroll );
    return;
}



void do_brandish( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    int sn, v4;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("Your hand passes right through it!\n\r",ch);
	return;
	}
    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char( "You can brandish only with a staff.\n\r", ch );
	return;
    }

    if ( ( sn = staff->value[3] ) < 0
    ||   sn >= MAX_SKILL
    ||   skill_table[sn].spell_fun == 0 )
    {
	bug( "Do_brandish: bad sn %d.", sn );
	return;
    }

    if ( ( v4 = staff->value[4] ) > MAX_WANDSTAFF-1
    ||   v4 < 0 )
    {
        bug( "do_brandish: Bad staff v4 (#%d)",staff->pIndexData->vnum);
        v4 = 0;
    }

    WAIT_STATE( ch, PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
	act( wandstaff_table[v4].staff_brand_o, ch, staff, NULL, TO_ROOM );
	act( wandstaff_table[v4].staff_brand_c, ch, staff, NULL, TO_CHAR );
	if ( number_percent() >= 20 + get_skill(ch,gsn_staves) * 4/5)
 	{
	    act (wandstaff_table[v4].staff_fail_c,ch,staff,NULL,TO_CHAR);
	    act (wandstaff_table[v4].staff_fail_o,ch,NULL,NULL,TO_ROOM);
	    check_improve(ch,gsn_staves,FALSE,2);
	}
	
	else for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next	= vch->next_in_room;

	    switch ( skill_table[sn].target )
	    {
	    default:
		bug( "Do_brandish: bad target for sn %d.", sn );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
		    continue;
		break;
		
	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;
	    }

	    obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL );
	    check_improve(ch,gsn_staves,TRUE,2);
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( wandstaff_table[v4].staff_die_o, ch, staff, NULL, TO_ROOM );
	act( wandstaff_table[v4].staff_die_c, ch, staff, NULL, TO_CHAR );
	extract_obj( staff );
    }

    return;
}



void do_zap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int v4;
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("Your hand passes right through it!\n\r",ch);
	return;
	}
    one_argument( argument, arg );
    if ( arg[0] == '\0' && ch->fighting == NULL )
    {
	send_to_char( "Zap whom or what?\n\r", ch );
	return;
    }

    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
	send_to_char( "You can zap only with a wand.\n\r", ch );
	return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting != NULL )
	{
	    victim = ch->fighting;
	}
	else
	{
	    send_to_char( "Zap whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg, TRUE ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg, TRUE ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    WAIT_STATE( ch, PULSE_VIOLENCE );
    if ( ( v4 = wand->value[4] ) > MAX_WANDSTAFF-1
    ||   v4 < 0 )
    {
        bug( "do_zap: Bad wand v4 (#%d)",wand->pIndexData->vnum);
        v4 = 0;
    }

    if ( wand->value[2] > 0 )
    {
	if ( victim != NULL )
	{
	    act( wandstaff_table[v4].wand_zap_ch_o, ch, wand, victim, TO_NOTVICT );
	    act( wandstaff_table[v4].wand_zap_ch_c, ch, wand, victim, TO_CHAR );
	    act( wandstaff_table[v4].wand_zap_ch_v,ch, wand, victim, TO_VICT );
	}
	else
	{
	    act( wandstaff_table[v4].wand_zap_ob_o, ch, wand, obj, TO_ROOM );
	    act( wandstaff_table[v4].wand_zap_ob_o, ch, wand, obj, TO_CHAR );
	}

 	if (number_percent() >= 20 + get_skill(ch,gsn_wands) * 4/5) 
	{
	    act( wandstaff_table[v4].wand_fail_c,
		 ch,wand,NULL,TO_CHAR);
	    act( wandstaff_table[v4].wand_fail_o,
		 ch,wand,NULL,TO_ROOM);
	    check_improve(ch,gsn_wands,FALSE,2);
	}
	else
	{
	    obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
	    check_improve(ch,gsn_wands,TRUE,2);
	}
    }

    if ( --wand->value[2] <= 0 )
    {
	act( wandstaff_table[v4].wand_die_o, ch, wand, NULL, TO_ROOM );
	act( wandstaff_table[v4].wand_die_c, ch, wand, NULL, TO_CHAR );
	extract_obj( wand );
    }

    return;
}



void do_steal( CHAR_DATA *ch, char *argument )
{
  char buf  [MAX_STRING_LENGTH];
  char bufbb[MAX_STRING_LENGTH];
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int percent, skill;

  if (IS_AFFECTED2(ch,AFF_GHOST)){
    send_to_char("Your hand passes right through it!\n\r",ch);
    return;
  }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
      send_to_char( "Steal what from whom?\n\r", ch );
      return;
    }

  if ( ( victim = get_char_room( ch, arg2, TRUE ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  if (is_safe(ch,victim))
    return;

/*if( ch->saving_throw > -25 && !IS_NPC( victim ) )
    {
      send_to_char( "You are ill-equiped for thieving.\n\r", ch );
      return;
    }*/

  if( !IS_NPC( victim ) && IS_SET( victim->act_bits, PLR_NEWBIE ) )
    {
      send_to_char( "Stealing from newbies is hazardous to your health.\n\r", ch );
      return;
    }


  if ( !IS_NPC(ch)
  &&    IS_SET(ch->act_bits,PLR_NEWBIE)
  &&   !IS_NPC(victim) )
  {
      send_to_char( "Newbies shouldn't steal from other players.\n\r", ch );
      return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM) &&
      ch->master != NULL &&
      ch->master == victim)
    {
      send_to_char("Why would you steal from your beloved master?\n\r",ch);
      return;
    }

  if ( victim == ch )
    {
      send_to_char( "That's pointless.\n\r", ch );
      return;
    }

  if( IS_IMMORTAL( victim ) &&
      !IS_IMMORTAL( ch ) )
      
    {
      send_to_char( "Stealing from immortals is a sure path to an early death.\n\r", ch );
      printf_to_char( victim, "%s just tried to steal from you.\n\r", ch->name );
      return;
    }

  if( !IS_NPC( ch ) &&
      !IS_NPC( victim ) &&
      ch->pcdata->latelog )
    {
      sprintf( buf, "Latelog steal by %s on %s.\n\r", ch->name, victim->name );
      ch->pcdata->latelog = FALSE;
      wiznet( buf, NULL, NULL, WIZ_DEATHS, 0, 0 );
      log_string( buf );
    }
  if ( IS_NPC(victim) 
       && victim->position == POS_FIGHTING)
    {
      send_to_char(  "Kill stealing is not permitted.\n\r"
		     "You'd better not -- you might get hit.\n\r",ch);
      return;
    }

  WAIT_STATE( ch, PULSE_VIOLENCE );
  percent  = number_percent();

  if (!IS_AWAKE(victim))
    percent -= 15;
  else if (!can_see(victim,ch))
    percent += 10;
  else 
    percent += 20;


  if ((!IS_NPC(ch) && (percent > (skill = get_skill(ch,gsn_steal) )
  ||   (IS_AWAKE(victim) && skill < 1)))
  ||   (IS_NPC(victim) && IS_AFFECTED2(victim,AFF_WITHER)) 
  ||   (!IS_NPC(victim) 
        && (victim->level - ch->level) > 6
        &&  number_percent() <= 50))
    {
      /*
       * Failure.
       */
      send_to_char( "Oops.\n\r", ch );
      affect_strip(ch,gsn_sneak);
      REMOVE_BIT(ch->affected_by,AFF_SNEAK);

      act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT    );
      act( "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT );
      switch(number_range(0,3))
	{
	case 0 :
	  sprintf( buf, "%s is a lousy thief!", ch->name );
	  break;
        case 1 :
	  sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
		   ch->name,(ch->sex == 2) ? "her" : "his");
	  break;
	case 2 :
	  sprintf( buf,"%s tried to rob me!",ch->name );
	  break;
	case 3 :
	  sprintf(buf,"Keep your hands out of there, %s!",ch->name);
	  break;
        }
      if (!IS_AWAKE(victim))
	do_wake(victim,"");
      if (IS_AWAKE(victim))
	do_yell( victim, buf );
      if ( !IS_NPC(ch) )
	{
	  if ( IS_NPC(victim) )
	    {
	      check_improve(ch,gsn_steal,FALSE,1);
	      multi_hit( victim, ch, TYPE_UNDEFINED );
	    }
	  else
	    {
	      sprintf(buf,"$N tried to steal from %s.",victim->name);
	      check_improve(ch,gsn_steal,FALSE,1);
	      multi_hit( victim, ch, TYPE_UNDEFINED );
	      wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
	    }
	}

      return;
    }

  if ( !str_cmp( arg1, "coin"  )
       ||   !str_cmp( arg1, "coins" )
       ||   !str_cmp( arg1, "gold"  ) 
       ||	 !str_cmp( arg1, "silver"))
    {
      int gold, silver;

      gold = victim->gold * number_range(1, ch->level) / 60;
      silver = victim->silver * number_range(1,ch->level) / 60;
      if ( gold <= 0 && silver <= 0 )
	{
	  send_to_char( "You couldn't get any coins.\n\r", ch );
	  return;
	}

      ch->gold     	+= gold;
      ch->silver   	+= silver;
      victim->silver 	-= silver;
      victim->gold 	-= gold;
      if (silver <= 0)
	sprintf( buf, "Bingo!  You got %d gold coins.\n\r", gold );
      else if (gold <= 0)
	sprintf( buf, "Bingo!  You got %d silver coins.\n\r",silver);
      else
	sprintf(buf, "Bingo!  You got %d silver and %d gold coins.\n\r",
		silver,gold);

      if ( silver/100 + gold > MAX_MONEY_TRANSFER )
      {
          sprintf(bufbb, "%s stole %d gold and %d silver from %s.",
              IS_NPC( ch ) ? ch->short_descr : ch->name, gold, silver,
              IS_NPC( victim ) ? victim->short_descr : victim->name );
          wiznet( bufbb, NULL, NULL, WIZ_COMMERCE, 0, 0 );
          log_string( bufbb );
      }

      if ( !IS_NPC(ch) )        ch->pcdata->lastfight = current_time;
      if ( !IS_NPC(victim) )    victim->pcdata->lastpk = current_time;

      send_to_char( buf, ch );
      check_improve(ch,gsn_steal,TRUE,1);
      return;
    }

  if ( ( obj = get_obj_carry( victim, arg1, ch, TRUE ) ) == NULL )
    {
      send_to_char( "You can't find it.\n\r", ch );
      return;
    }
	
  if ( !can_drop_obj( ch, obj )
       ||   IS_SET(obj->extra_flags, ITEM_INVENTORY))
    {
      send_to_char( "You can't pry it away.\n\r", ch );
      return;
    }

  if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
      send_to_char( "You have your hands full.\n\r", ch );
      return;
    }

  if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
      send_to_char( "You can't carry that much weight.\n\r", ch );
      return;
    }

  obj_from_char( obj );
  obj_to_char( obj, ch );
  act("You pocket $p.",ch,obj,NULL,TO_CHAR);
  check_improve(ch,gsn_steal,TRUE,1);
  send_to_char( "Got it!\n\r", ch );

    if ( !IS_NPC( ch ) )     ch->pcdata->lastfight= current_time;
    if ( !IS_NPC( victim ) ) victim->pcdata->lastpk = current_time;


    if ( is_name( "restring", obj->name )
    ||   is_name( "token", obj->name )
    ||   is_name( "questeq", obj->name ) 
    ||   is_abbrev( "-power", obj->name ) )
    {
        sprintf(buf, "%s stole %s from %s.",
            IS_NPC( ch ) ? ch->short_descr : ch->name,
            strip_color( obj->short_descr ),
            IS_NPC( victim ) ? victim->short_descr : victim->name );
        wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
        log_string( buf );
    }
    else if ( obj->item_type == ITEM_CONTAINER
         ||   obj->item_type == ITEM_BOOK )
    {
        if ( has_incontainer_name( obj->contains, "restring", -5, NULL )
        ||   has_incontainer_name( obj->contains, "token", -5, NULL )
        ||   has_incontainer_name( obj->contains, "questeq", -5, NULL ) 
        ||   has_incontainer_abbrev( obj->contains, "-power" ) )
        {
            sprintf(buf, "%s stole a container with valuables from %s.",
                IS_NPC( ch ) ? ch->short_descr : ch->name,
                IS_NPC( victim ) ? victim->short_descr : victim->name );
            wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
            log_string( buf );
        }
    }

  return;
}



/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
    /*char buf[MAX_STRING_LENGTH];*/
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

    pShop = NULL;
    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
	if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
	    break;
    }

    if ( pShop == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return NULL;
    }

    /*
     * Undesirables.
     *
    if ( !IS_NPC(ch) && IS_SET(ch->act_bits, PLR_KILLER) )
    {
	do_say( keeper, "Killers are not welcome!" );
	sprintf( buf, "%s the KILLER is over here!\n\r", ch->name );
	do_yell( keeper, buf );
	return NULL;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act_bits, PLR_THIEF) )
    {
	do_say( keeper, "Thieves are not welcome!" );
	sprintf( buf, "%s the THIEF is over here!\n\r", ch->name );
	do_yell( keeper, buf );
	return NULL;
    }
	*/
    /*
     * Shop hours.
     */
    if ( time_info.hour < pShop->open_hour )
    {
	do_say( keeper, "Sorry, I am closed. Come back later." );
	return NULL;
    }
    
    if ( time_info.hour > pShop->close_hour )
    {
	do_say( keeper, "Sorry, I am closed. Come back tomorrow." );
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
	do_say( keeper, "I don't trade with folks I can't see." );
	return NULL;
    }

    return keeper;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper( OBJ_DATA *obj, CHAR_DATA *ch )
{
    OBJ_DATA *t_obj, *t_obj_next;

    /* see if any duplicates are found */
    for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next)
    {
	t_obj_next = t_obj->next_content;

	if (obj->pIndexData == t_obj->pIndexData 
	&&  !str_cmp(obj->short_descr,t_obj->short_descr))
	{
	    /* if this is an unlimited item, destroy the new one */
	    if (IS_OBJ_STAT(t_obj,ITEM_INVENTORY))
	    {
		extract_obj(obj);
		return;
	    }
	    obj->cost = t_obj->cost; /* keep it standard */
	    break;
	}
    }

    if (t_obj == NULL)
    {
	obj->next_content = ch->carrying;
	ch->carrying = obj;
    }
    else
    {
	obj->next_content = t_obj->next_content;
	t_obj->next_content = obj;
    }

    obj->carried_by      = ch;
    obj->in_room         = NULL;
    obj->in_obj          = NULL;
    ch->carry_number    += get_obj_number( obj );
    ch->carry_weight    += get_obj_weight( obj );
}

/* get an object from a shopkeeper's list */
OBJ_DATA *get_obj_keeper( CHAR_DATA *ch, CHAR_DATA *keeper, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;
 
    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = keeper->carrying; obj != NULL; obj = obj->next_content )
    {
        if (obj->wear_loc == WEAR_NONE
        &&  can_see_obj( keeper, obj )
	&&  can_see_obj(ch,obj)
        &&  is_name( arg, obj->name ) )
        {
            if ( ++count == number )
                return obj;
	
	    /* skip other objects of the same name */
	    while (obj->next_content != NULL
	    && obj->pIndexData == obj->next_content->pIndexData
	    && !str_cmp(obj->short_descr,obj->next_content->short_descr))
		obj = obj->next_content;
        }
    }
 
    return NULL;
}

int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
    SHOP_DATA *pShop;
    int cost;

    if ( obj == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
	return 0;

    if ( fBuy )
    {
	cost = obj->cost * pShop->profit_buy  / 100;
    }
    else
    {
	OBJ_DATA *obj2;
	int itype;

	cost = 0;
	for ( itype = 0; itype < MAX_TRADE; itype++ )
	{
	    if ( obj->item_type == pShop->buy_type[itype] )
	    {
		cost = obj->cost * pShop->profit_sell / 100;
		break;
	    }
	}

	if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
	    for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
	    {
	    	if ( obj->pIndexData == obj2->pIndexData
		&&   !str_cmp(obj->short_descr,obj2->short_descr) )
                 {
	 	    if (IS_OBJ_STAT(obj2,ITEM_INVENTORY))
			cost /= 2;
		    else
                    	cost = cost * 3 / 4;
                 }
	    }
    }

/* Removed at request of Sidonie 5/25/06 gkl
    if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
    {
	if (obj->value[1] == 0)
	    cost /= 4;
	else
	    cost = cost * obj->value[2] / obj->value[1];
    }
*/
    return cost;
}



void do_buy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cost,roll;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("Your hand passes right through it!\n\r",ch);
	return;
	}
    if ( argument[0] == '\0' )
    {
	send_to_char( "Buy what?\n\r", ch );
	return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;

	smash_tilde(argument);

	if ( IS_NPC(ch) )
	    return;

	argument = one_argument(argument,arg);

	/* hack to make new thalos pets work */
	if (ch->in_room->vnum == 9621)
	    pRoomIndexNext = get_room_index(9706);
	else
	    pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet         = get_char_room( ch, arg, TRUE );
	ch->in_room = in_room;

	if ( pet == NULL || !IS_SET(pet->act_bits, ACT_PET) )
	{
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	if ( ch->pet != NULL || has_summonedpets( ch ) )
	{
	    send_to_char("You already own a pet.\n\r",ch);
	    return;
	}

/*      cost = 10 * pet->level * pet->level; */
        cost = pet->pIndexData->wealth;
   

	if ( (ch->silver + 100 * ch->gold) < cost )
	{
	    send_to_char( "You can't afford it.\n\r", ch );
	    return;
	}

/*      if ( ch->level < pet->level )
	{
	    send_to_char(
		"You're not powerful enough to master this pet.\n\r", ch );
	    return;
	}*/

	/* haggle */
	roll = number_percent();
	if (roll < get_skill(ch,gsn_haggle))
	{
	    cost -= cost * roll / 200;
	    sprintf(buf,"You haggle the price down to %d coins.\n\r",cost);
	    send_to_char(buf,ch);
	    check_improve(ch,gsn_haggle,TRUE,4);
	
	}

	deduct_cost(ch,cost);
	pet			= create_mobile( pet->pIndexData );
	SET_BIT(pet->act_bits, ACT_PET);
	SET_BIT(pet->affected_by, AFF_CHARM);
	pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;

	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' )
	{
	    sprintf( buf, "%s %s", pet->name, arg );
	    free_string( pet->name );
	    pet->name = str_dup( buf );
	}

	sprintf( buf, "%sA neck tag says 'I belong to %s'.\n\r",
	    pet->description, ch->name );
	free_string( pet->description );
	pet->description = str_dup( buf );

	char_to_room( pet, ch->in_room );
	add_follower( pet, ch );
	pet->leader = ch;
	ch->pet = pet;
        pet->gold = 0;
        pet->silver = 0;
	send_to_char( "Enjoy your pet.\n\r", ch );
	act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );

        /* so we can control pets with mprogs */
	if ( HAS_TRIGGER( pet, TRIG_SPAWN ) )
	    mp_spawn_trigger( pet );

	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj,*t_obj;
	char arg[MAX_INPUT_LENGTH];
	int number, count = 1;

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;

	number = mult_argument(argument,arg);
	obj  = get_obj_keeper( ch,keeper, arg );
	cost = get_cost( keeper, obj, TRUE );

	if (number < 1)
	{
	    act("$n tells you {t'Get real!{x",keeper,NULL,ch,TO_VICT);
	    return;
	}
	if (number > 50)
	{
	    act("$n tells you {t'Get real!{x",keeper,NULL,ch,TO_VICT);
	    return;
	}

	if ( cost <= 0 || !can_see_obj( ch, obj ) )
	{
	    act( "$n tells you {t'I don't sell that -- try 'list''{x",
		keeper, NULL, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}

	if (!IS_OBJ_STAT(obj,ITEM_INVENTORY))
	{
	    for (t_obj = obj->next_content;
	     	 count < number && t_obj != NULL; 
	     	 t_obj = t_obj->next_content) 
	    {
	    	if (t_obj->pIndexData == obj->pIndexData
	    	&&  !str_cmp(t_obj->short_descr,obj->short_descr))
		    count++;
	    	else
		    break;
	    }

	    if (count < number)
	    {
	    	act("$n tells you {t'I don't have that many in stock.'{x",
		    keeper,NULL,ch,TO_VICT);
	    	ch->reply = keeper;
	    	return;
	    }
	}

	if ( (ch->silver + ch->gold * 100) < cost * number )
	{
	    if (number > 1)
		act("$n tells you {t'You can't afford to buy that many.'{x",
		    keeper,obj,ch,TO_VICT);
	    else
	    	act( "$n tells you {t'You can't afford to buy $p.'{x",
		    keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}
	
	if (ch->carry_number +  number * get_obj_number(obj) > can_carry_n(ch))
	{
	    send_to_char( "You can't carry that many items.\n\r", ch );
	    return;
	}

	if ( ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch))
	{
	    send_to_char( "You can't carry that much weight.\n\r", ch );
	    return;
	}

	/* Haggle first but do not invoke act() */
	roll = number_percent();
	if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) && roll < get_skill(ch,gsn_haggle))
	{
	    cost -= cost * roll / 200;
	    act("You haggle with $N.",ch,NULL,keeper,TO_CHAR);
	    check_improve(ch,gsn_haggle,TRUE,4);
	}
	deduct_cost(ch,cost * number);
	keeper->gold += cost * number/100;
	keeper->silver += cost * number - (cost * number/100) * 100;

        /* Give the objects to the player BEFORE the act()s - this is so
           mprogs can manipulate objects bought via the act trigger */
	for (count = 0; count < number; count++)
	{
	    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    	t_obj = create_object( obj->pIndexData, obj->level );
	    else
	    {
		t_obj = obj;
		obj = obj->next_content;
	    	obj_from_char( t_obj );
	    }

	    if (t_obj->timer > 0 && !IS_OBJ_STAT(t_obj,ITEM_HAD_TIMER))
	    	t_obj->timer = 0;
	    REMOVE_BIT(t_obj->extra_flags,ITEM_HAD_TIMER);
            REMOVE_BIT(t_obj->extra_flags,ITEM_INVENTORY);
	    obj_to_char( t_obj, ch );
	    if (cost < t_obj->cost)
	    	t_obj->cost = cost;
/* book proceeds go to author; eliminate royalties on resale */
            if ( t_obj->item_type == ITEM_BOOK
            &&   !IS_NULLSTR(t_obj->owner) )
            {
                roll = t_obj->value[4] * cost / 100;
                modify_acct( t_obj->owner, roll / 100, roll % 100 );
                t_obj->value[4] = 0;
                sprintf( buf,
        "%s just bought %s, and %s will get %d gold and %d silver in royalties",
                    ch->name, t_obj->short_descr, t_obj->owner, 
                    roll / 100, roll % 100 );
                bug( buf, 0 );
            }
	}
        /* Echoes are very last */
	if (number > 1)
	{
	    sprintf(buf,"$n buys $p[%d].",number);
	    act(buf,ch,t_obj,NULL,TO_ROOM);
	    sprintf(buf,"You buy $p[%d] for %d silver.",number,cost * number);
	    act(buf,ch,t_obj,NULL,TO_CHAR);
	}
	else
	{
	    act( "$n buys $p.", ch, t_obj, NULL, TO_ROOM );
	    sprintf(buf,"You buy $p for %d silver.",cost);
	    act( buf, ch, t_obj, NULL, TO_CHAR );
	}

    }
}



void do_list( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	ROOM_INDEX_DATA *pRoomIndexNext;
	CHAR_DATA *pet;
	bool found;

        /* hack to make new thalos pets work */
        if (ch->in_room->vnum == 9621)
            pRoomIndexNext = get_room_index(9706);
        else
            pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );

	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	found = FALSE;
	for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
	{
	    if ( IS_NPC( pet ) && IS_SET(pet->act_bits, ACT_PET) )
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "Pets for sale:\n\r", ch );
		}
		sprintf( buf, "[%2d] %8d - %s\n\r",
		    pet->level,
/*                  10 * pet->level * pet->level, */
                    (int) pet->pIndexData->wealth,
		    pet->short_descr );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "Sorry, we're out of pets right now.\n\r", ch );
	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost,count;
	bool found;
	char arg[MAX_INPUT_LENGTH];

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;
        one_argument(argument,arg);

	found = FALSE;
	for ( obj = keeper->carrying; obj; obj = obj->next_content )
	{
	    if ( obj->wear_loc == WEAR_NONE
	    &&   can_see_obj( ch, obj )
	    &&   ( cost = get_cost( keeper, obj, TRUE ) ) > 0 
	    &&   ( arg[0] == '\0'  
 	       ||  is_name(arg,obj->name) ))
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "[Lv Price Qty] Item\n\r", ch );
		}

		if (IS_OBJ_STAT(obj,ITEM_INVENTORY))
		    sprintf(buf,"[%2d %5d -- ] %s\n\r",
			obj->level,cost,obj->short_descr);
		else
		{
		    count = 1;

		    while (obj->next_content != NULL 
		    && obj->pIndexData == obj->next_content->pIndexData
		    && !str_cmp(obj->short_descr,
			        obj->next_content->short_descr))
		    {
			obj = obj->next_content;
			count++;
		    }
		    sprintf(buf,"[%2d %5d %2d ] %s\n\r",
			obj->level,cost,count,obj->short_descr);
		}
		send_to_char( buf, ch );
	    }
	}

	if ( !found )
	    send_to_char( "You can't buy anything here.\n\r", ch );
	return;
    }
}



void do_sell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Sell what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg, ch, TRUE ) ) == NULL )
    {
	act( "$n tells you {t'You don't have that item.'{x",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( obj->item_type != ITEM_WEAPON && !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
	act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }
    if ( cost > (keeper-> silver + 100 * keeper->gold) )
    {
	act("$n tells you {t'I'm afraid I don't have enough wealth to buy $p.'{x",
	    keeper,obj,ch,TO_VICT);
	return;
    }

    act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
    
    /* NO haggle */
    /*
    roll = number_percent();
    if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) && roll < get_skill(ch,gsn_haggle))
    {
        send_to_char("You haggle with the shopkeeper.\n\r",ch);
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN(cost,95 * get_cost(keeper,obj,TRUE) / 100);
	cost = UMIN(cost,(keeper->silver + 100 * keeper->gold));
        check_improve(ch,gsn_haggle,TRUE,4);
    }
    */

    sprintf( buf, "You sell $p for %d silver and %d gold piece%s.",
	cost - (cost/100) * 100, cost/100, cost == 1 ? "" : "s" );
    act( buf, ch, obj, NULL, TO_CHAR );
    ch->gold     += cost/100;
    ch->silver 	 += cost - (cost/100) * 100;
    deduct_cost(keeper,cost);
    if ( keeper->gold < 0 )
	keeper->gold = 0;
    if ( keeper->silver< 0)
	keeper->silver = 0;

    if ( IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
    {
	extract_obj( obj );
    }
    else
    {
	obj_from_char( obj );
	if (obj->timer)
	    SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	else
	    obj->timer = number_range( 960, 1920 );
	obj_to_keeper( obj, keeper );
    }

    return;
}


void do_value( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Value what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg, ch, TRUE ) ) == NULL )
    {
	act( "$n tells you {t'You don't have that item.'{x",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
        act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
        return;
    }

    if ( obj->item_type != ITEM_WEAPON && !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    sprintf( buf, 
	"$n tells you {t'I'll give you %d silver and %d gold coins for $p.'{x", 
	cost - (cost/100) * 100, cost/100 );
    act( buf, keeper, obj, ch, TO_VICT );
    ch->reply = keeper;

    return;
}

/**** LOOT CODE FOR UNCONSCIOUS PEOPLE *****/
void do_loot( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *target;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
       send_to_char("Your hand passes right through it.\n\r",ch);
       return;
       }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (( arg1[0] == '\0' ) || (arg2[0] == '\0'))
    {
    send_to_char( "Loot what from whom?\n\r", ch );
    return;
    }

    if (!(target = get_char_room(ch,arg2, TRUE))){
        send_to_char( "Loot what from whom?\n\r", ch );
        return;
        }

    if (target->position != POS_UNCONSCIOUS){
        send_to_char( "They're not unconscious!\n\r", ch );
        return;
        }

    /* special case for gold */
    if( strcasecmp( arg1, "gold" ) == 0 )
      {
	if( target->gold > 0 || target->silver > 0 )
	  {
	    printf_to_char( ch, "You take %d gold pieces and %d silver pieces from your victim.\n\r",
			    target->gold,
			    target->silver );

            if ( (target->gold + target->silver/100) > MAX_MONEY_TRANSFER )
            {
                sprintf(buf, "%s looted %ld gold and %ld silver from %s.",
                    IS_NPC( ch ) ? ch->short_descr : ch->name, 
                    target->gold, target->silver,
                    target->name );
                wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                log_string( buf );
            }

	    ch->gold += target->gold;
	    target->gold = 0;

	    ch->silver += target->silver;
	    target->silver = 0;

            if ( !IS_NPC( ch ) )        ch->pcdata->lastfight= current_time;
            if ( !IS_NPC( target ) )    target->pcdata->lastpk = current_time;

	    return;
	  }
	else
	  {
	    send_to_char( "Your victim carried no coin.\n\r", ch );
	  }
      }


    for (obj = target->carrying; obj!= NULL; obj = obj->next_content){
        if (is_name(arg1, obj->name))
            break;
        }

    if (obj == NULL){
        send_to_char( "They don't have that.\n\r", ch );
        return;
        }

    if( IS_SET( queststatus, QUEST_INPROGRESS ) &&
	IS_SET( queststatus, QUEST_NO_LOOT ) &&
	IS_SET( target->act_bits, PLR_QUEST ) )
      {
	send_to_char( "This quest prohibits looting.\n\r", ch );
	return;
      }

    if (  IS_OBJ_STAT(obj, ITEM_NOLOOT)
    &&   ( IS_NPC(ch) || (!IS_NPC(ch) && !IS_IMMORTAL(ch)) ) )
    {
        send_to_char( "You can't loot that.\n\r", ch );
        return;
    }

    obj_from_char(obj);
    obj_to_char(obj,ch);

    if ( is_name( "restring", obj->name )
    ||   is_name( "token", obj->name )
    ||   is_name( "questeq", obj->name ) 
    ||   is_abbrev( "-power", obj->name ) )
    {
        sprintf(buf, "%s looted %s from %s.",
            IS_NPC( ch ) ? ch->short_descr : ch->name,
            strip_color( obj->short_descr ),
            IS_NPC( target ) ? target->short_descr : target->name );
        wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
        log_string( buf );
    }
    else if ( obj->item_type == ITEM_CONTAINER
         ||   obj->item_type == ITEM_BOOK )
    {
        if ( has_incontainer_name( obj->contains, "restring", -5, NULL )
        ||   has_incontainer_name( obj->contains, "token", -5, NULL )
        ||   has_incontainer_name( obj->contains, "questeq", -5, NULL ) 
        ||   has_incontainer_abbrev( obj->contains, "-power" ) )
        {
            sprintf(buf, "%s looted a container with valuables from %s.",
                IS_NPC( ch ) ? ch->short_descr : ch->name,
                IS_NPC( target ) ? target->short_descr : target->name );
            wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
            log_string( buf );
        }
    }

    act("You get $p from $N's unconscious body.",ch,obj,target,TO_CHAR);
    act("$n gets $p from $N's unconscious body.",ch,obj,target,TO_NOTVICT);

    if ( !IS_NPC( ch ) )        ch->pcdata->lastfight = current_time;;
    if ( !IS_NPC( target ) )    target->pcdata->lastpk = current_time;

    return;
}

void do_offhand (CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    if (get_skill(ch,gsn_dualwield) < 2){
	send_to_char("You have no idea how to wield two weapons!\n\r",ch);
	return;
	}

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("Your hand passes right through it!\n\r",ch);
	return;
	}
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wield what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch, TRUE ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	int sn,skill;

	if ( !remove_obj( ch, WEAR_HOLD, TRUE ) )
	    return;

	if ( !IS_NPC(ch) 
	&& get_obj_weight(obj) > (str_app[get_curr_stat(ch,STAT_STR)].wield  
		* 10))
	{
	    send_to_char( "It is too heavy for you to wield.\n\r", ch );
	    return;
	}

	if ( IS_AFFECTED2(ch,AFF_WITHER) )
	{
             AFFECT_DATA *af;
             int sn = skill_lookup("wither limb");
	     for (af = ch->affected; af != NULL; af = af->next)
	     {
		if ( af->type == sn && af->modifier == 2 )
		{
                    send_to_char("Your arm is too weak.\n\r",ch);
                    return;
		}
	    }
	}

	if (!IS_NPC(ch) && ch->size < SIZE_HUGE 
	&&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
 	&&  get_eq_char(ch,WEAR_SHIELD) != NULL)
/*      Templar spear check SHOULD go here, but templars do not get offhand */
	{
	    send_to_char("You need two hands free for that weapon.\n\r",ch);
	    return;
	}

	act( "$n wields $p in $s off hand.", ch, obj, NULL, TO_ROOM );
	act( "You wield $p in your off hand.", ch, obj, NULL, TO_CHAR );

	equip_char( ch, obj, WEAR_HOLD );

        sn = get_weapon_sn(ch);

	if (sn == gsn_hand_to_hand)
	   return;

        skill = get_weapon_skill(ch,sn);
 
        if (skill >= 100)
            act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
        else if (skill > 85)
            act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 70)
            act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 50)
            act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
        else if (skill > 25)
            act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
        else if (skill > 1)
            act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
        else
            act("You don't even know which end is up on $p.",
                ch,obj,NULL,TO_CHAR);
    }

    return;
}

/* do_plant */
void do_plant( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    OBJ_DATA *seed, *obj;

    if (IS_AFFECTED2(ch,AFF_GHOST))
    {
        send_to_char("Alas, you cannot garden while dead.\n\r",ch);
        return;
    }

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "Plant what?\n\r", ch );
        return;
    }

    if ( ( seed = get_obj_carry( ch, arg, ch, TRUE ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if ( seed->item_type != ITEM_SEED )
    {
        do_drop( ch, argument );
        return;
    }

    if ( !can_drop_obj( ch, seed ) )
    {
        send_to_char( "You can't let go of it.\n\r", ch );
        return;
    }

    if ( ch->in_room == NULL
    ||   ch->in_room->sector_type != seed->value[1] )
    {
        act( "This place is not suitable for planting $p.", ch, seed, NULL,
          TO_CHAR );
        return;
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
        if ( obj->item_type == ITEM_SEED 
        &&   !CAN_WEAR(obj, ITEM_TAKE) ) /* if char has item, in_room == NULL */
        {
            send_to_char( "Something has already been planted here.\n\r", ch );
            return;
        }
    }

    switch ( ch->in_room->sector_type )
    {
        case SECT_CITY:
        case SECT_FIELD:
        case SECT_FOREST:
        case SECT_HILLS:
        case SECT_MOUNTAIN:
            strcpy(buf, "You carefully plant $p in the soil.");
            strcpy(buf2,"$n carefully plants $p in the soil.");
            break;
        case SECT_WATER_SWIM:
        case SECT_WATER_NOSWIM:
            strcpy(buf, "You release $p into the water.");
            strcpy(buf2,"$n releases $p into the water.");
            break;
        case SECT_AIR:
            strcpy(buf,  "You release $p into the air.");
            strcpy(buf2,"$n releases $p into the air.");
            break;
        case SECT_DESERT:
            strcpy(buf, "You carefully bury $p in the sand.");
            strcpy(buf2,"$n carefully buries $p in the sand.");
            break;
        default:
            strcpy(buf, "You carefully plant $p.");
            strcpy(buf2,"$n carefully plants $p.");
            break;
    }

    obj_from_char( seed );
    obj_to_room( seed, ch->in_room );
    REMOVE_BIT(seed->wear_flags, ITEM_TAKE);
    SET_BIT(seed->extra_flags, ITEM_NOTSEEN);
    SET_BIT(seed->extra_flags, ITEM_NOLOCATE);
    free_string( seed->description );
    seed->description = str_dup( "You see something growing here." );
    act( buf, ch, seed, NULL, TO_CHAR );
    act( buf2, ch, seed, NULL, TO_ROOM );
    WAIT_STATE( ch, PULSE_VIOLENCE );

    return;
}

void do_harvest( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj, *seed;
    OBJ_INDEX_DATA *pSeedIndex;
    char arg[MAX_INPUT_LENGTH];

    if ( IS_AFFECTED2( ch, AFF_GHOST ) )
    {
        send_to_char( "Your hand passes right through it!\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) ) 
    {
        send_to_char( "Harvest what?\n\r", ch );
        return;
    }
  
    if ( (obj = get_obj_list( ch, arg, ch->in_room->contents, TRUE )) == NULL )
    {
        send_to_char( "You can't find it.\n\r", ch );
        return;
    }
  
    if ( obj->item_type != ITEM_PLANT 
    ||   IS_SET(obj->wear_flags, ITEM_TAKE) )
    {
        do_get( ch, argument );
        return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        send_to_char( "You seem to have your hands full.\n\r", ch );
        return;
    }

    if ( get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch) )
    {
        send_to_char( "You can't carry that much weight.\n\r", ch );
        return;
    }

    SET_BIT(obj->wear_flags, ITEM_TAKE);

    obj_from_room( obj );
    obj_to_char( obj, ch );
    act( "You pick $p.", ch, obj, NULL, TO_CHAR );
    act( "$n picks $p.", ch, obj, NULL, TO_ROOM );

    if ( obj->value[2] > 0
    &&   (pSeedIndex = get_obj_index(obj->value[0])) != NULL 
    &&   number_percent() <= obj->value[2] )
    {
        seed = create_object( pSeedIndex, 0 );
        obj_to_char( seed, ch );
        act( "Additionally, you are able to harvest $p from $P!", 
            ch, seed, obj, TO_CHAR );
    }

    return;
}

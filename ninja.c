/*********************************************************************************************
 *
 * Here there be ninjas. Ninja code written by Zalyriel. July 2014.
 * 
**********************************************************************************************/

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


void do_ninjadrop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
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
	send_to_char( "ninjadrop what?\n\r", ch );
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
	send_to_char( "ninjadropped!\n\r", ch );
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
	act( "You ninjadrop $p.", ch, obj, NULL, TO_CHAR );

	if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
	{
	    act("$p ninja dissolves into smoke.",ch,obj,NULL,TO_CHAR);
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
		act( "You ninjadrop $p.", ch, obj, NULL, TO_CHAR );

        	if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
        	{
            	    act("$p ninja dissolves into smoke.",ch,obj,NULL,TO_CHAR);
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

void ninjaget_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    CHAR_DATA *gch;

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

    	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  !CAN_WEAR(container, ITEM_TAKE)
	&&  !IS_OBJ_STAT(obj,ITEM_HAD_TIMER))
	    obj->timer = 0;	
/*      The acts precede the obj_from_char ONLY if the item type is mone-
        this is because money objects are destroyed when picked up, which
        would make referring to "obj" in the act refer to an invalid obj */
        if ( obj->item_type == ITEM_MONEY )
        {
            act( "You ninjaget $p from $P.", ch, obj, container, TO_CHAR );
        }
	REMOVE_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	obj_from_obj( obj );
    }
    else
    {
        if ( obj->item_type == ITEM_MONEY )
        {
            act( "You ninjaget $p.", ch, obj, container, TO_CHAR );
        }
	obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_MONEY)
    {
	ch->silver += obj->value[0];
	ch->gold += obj->value[1];
	extract_obj( obj );
    }
    else
    {
	obj_to_char( obj, ch );

	/* The act has to be AFTER obj_to_char so that mobprogs that are triggered
   by the act can manipulate the object that is grabbed */
        if ( container != NULL )
        {
            act( "You ninjaget $p from $P.", ch, obj, container, TO_CHAR );
        }
        else
        {
            act( "You ninjaget $p.", ch, obj, container, TO_CHAR );
        }
    }

    return;
}


void do_ninjaget( CHAR_DATA *ch, char *argument )
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
	send_to_char( "ninjaget what?\n\r", ch );
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

	    ninjaget_obj( ch, obj, NULL );
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
		    ninjaget_obj( ch, obj, NULL );
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
	    ninjaget_obj( ch, obj, container );
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
		    ninjaget_obj( ch, obj, container );
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

void do_ninjaput( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
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

      obj_from_char( obj );
      obj_to_obj( obj, container );
	
      if (IS_SET(container->value[1],CONT_PUT_ON))
	{
	  act("You ninjaput $p on $P.",ch,obj,container, TO_CHAR);
	}
      else
	{
	  act( "You ninjaput $p in $P.", ch, obj, container, TO_CHAR );
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


	      obj_from_char( obj );
	      obj_to_obj( obj, container );

	      if (IS_SET(container->value[1],CONT_PUT_ON))
        	{
		  act("You put $p on $P.",ch,obj,container, TO_CHAR);
        	}
	      else
		{
		  act( "You ninjaput $p in $P.", ch, obj, container, TO_CHAR );
		}
	    }
	}
    }
}

void do_ninjagive( CHAR_DATA *ch, char *argument)
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
	send_to_char( "ninjagive what to whom?\n\r", ch );
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
	    send_to_char( "ninjaive what to whom?\n\r", ch );
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
	}
	else
	{
	    ch->gold		-= amount;
	    victim->gold	+= amount;
	}

	sprintf(buf,"You ninjagive $N %d %s.",amount, silver ? "silver" : "gold");
	act( buf, ch, NULL, victim, TO_CHAR    );
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

    obj_from_char( obj );
    obj_to_char( obj, victim );

    act( "You ninjagive $p to $N.", ch, obj, victim, TO_CHAR    );

    return;
}

void do_ninjaoload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "Syntax: ninjaload obj <vnum> <level>.\n\r", ch );
	return;
    }
    
    level = get_trust(ch); /* default */
  
    if ( arg2[0] != '\0')  /* load with a level */
    {
	if (!is_number(arg2))
        {
	  send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
	  return;
	}
        level = atoi(arg2);
        if (level < 0 || level > get_trust(ch))
	{
	  send_to_char( "Level must be be between 0 and your level.\n\r",ch);
  	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );
    wiznet("$N ninja loads $p.",ch,obj,WIZ_LOAD,0,get_trust(ch));
    send_to_char( "Ninja loaded!\n\r", ch );
    return;
}

void do_ninjamload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: ninjaload mob <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    sprintf(buf,"$N ninja loads %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,0,get_trust(ch));
    send_to_char( "Ninja loaded!\n\r", ch );
    return;
}

void do_ninjaload(CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  ninjaload mob <vnum>\n\r",ch);
	send_to_char("  ninjaload obj <vnum> <level>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_ninjamload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_ninjaoload(ch,argument);
	return;
    }
    /* echo syntax */
    do_ninjaload(ch,"");
}

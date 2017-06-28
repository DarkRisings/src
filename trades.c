#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "trades.h"

/* action functions */
void do_tailor( CHAR_DATA* ch, char* argument );
void do_bake( CHAR_DATA* ch, char* argument );
void do_fletch( CHAR_DATA* ch, char* argument );
void do_trades( CHAR_DATA* ch, char* argument );

/* auxilliary functions */
TRADE_DATA* trade_lookup( int tradeskill );
void do_trade( CHAR_DATA* ch, char* argument, TRADE_DATA* a_trade );
bool char_can_trade( CHAR_DATA* ch, char* argument, TRADE_DATA* a_trade );
bool char_has( CHAR_DATA* ch, char* argument );
bool check_duplicates( int objects[ 11 ] );
bool recipe_needs( RECIPE* card, int vnum );
void create_from_recipe( RECIPE* card, CHAR_DATA* ch, char* argument, TRADE_DATA* a_trade );
void check_improvement( CHAR_DATA* ch, TRADE_DATA* a_trade, RECIPE* a_recipe );
void destroy_components( CHAR_DATA* ch, char* argument );
int* get_character_skill( CHAR_DATA* ch, TRADE_DATA* a_trade );
RECIPE* get_recipe_set( TRADE_DATA* a_trade );

/*
 * These are wrapper functions that the command parser
 * uses to invoke trade code.
 */

void do_tailor( CHAR_DATA* ch, char* argument )
{
  do_trade( ch, argument, trade_lookup( TAILORING ) );
}


void do_bake( CHAR_DATA* ch, char* argument )
{
  do_trade( ch, argument, trade_lookup( BAKING ) );
}


void do_fletch( CHAR_DATA* ch, char* argument )
{
  do_trade( ch, argument, trade_lookup( FLETCHING ) );
}



/*
 * This function allows characters to check their skill
 */

void do_trades( CHAR_DATA* ch, char* argument )
{
  char buffer[ MAX_STRING_LENGTH ];
  TRADE_DATA* current_trade = &( trade_matrix[ 0 ] );
  int* current_trade_skill;

  if( !IS_NPC( ch ) )
    {
      sprintf( buffer, "%20s : %20s\n\r", "Trade", "Skill Level" );
      send_to_char( buffer, ch );
      send_to_char( "---------------------------------------------------------------------\n\r", ch );

      while( current_trade->trade != 0 )
	{
	  if( char_can_trade( ch, argument, current_trade ) )
	    {
	      current_trade_skill = get_character_skill( ch, current_trade );

	      sprintf( buffer, "%20s : %20d\n\r", current_trade->trade_name, *current_trade_skill );
	      send_to_char( buffer, ch );
	    }

	  current_trade++;
	}
    }
}



/*
 * Returns the structure that matches the value "tradeskill"
 */

TRADE_DATA* trade_lookup( int tradeskill )
{
  TRADE_DATA* match = NULL;
  int i = 0;

  for( i = 0; trade_matrix[ i ].trade != 0; i++ )
    {
      if( trade_matrix[ i ].trade == tradeskill )
	{
	  match = &( trade_matrix[ i ] );
	}
    }

  return match;

}



/*
 * Performs integrity-checking
 */

void do_trade( CHAR_DATA* ch, char* argument, TRADE_DATA* a_trade )
{
  int counting;
  int objcheck;
  int components[ 11 ];
  int components_needed;
  int components_total;
  char arg[ 10 ][ MAX_STRING_LENGTH ];
  char buf[ MAX_STRING_LENGTH ];
  char* buf2;
  bool match = FALSE;
  OBJ_DATA* obj;
  OBJ_DATA* obj_next;
  ROOM_INDEX_DATA* room;
  RECIPE* recipe_set;
  bool room_obj_found = FALSE;



  if( !IS_NPC( ch ) )
    {
      if( !char_can_trade( ch, argument, a_trade ) )
	{
	  send_to_char( "You are not of the right class.\n\r", ch );
	  return;
	}

      if( !strcmp( argument, "" ) )
	{
	  send_to_char( "Proper usage:\n\r", ch );
	  send_to_char( a_trade->no_args, ch );
	  return;
	}
      
      if( a_trade->room_obj )
	{
	  room = ch->in_room;

	  for( obj = room->contents; obj != NULL; obj = obj_next )
	    {
	      obj_next = obj->next_content;

	      if( obj->pIndexData->vnum == a_trade->room_obj_vnum )
		{
		  room_obj_found = TRUE;
		}
	    }

	  if( !room_obj_found )
	    {
	      send_to_char( a_trade->no_room_obj, ch );
	      return;
	    }
	}

      buf2 = one_argument( argument, arg[ 0 ] );
      for( counting = 1; counting < 11; counting++ )
	{
	  buf2 = one_argument( buf2, arg[ counting ] );
	}

      for( counting = 0; counting < 11; counting++ )
	{
	  if( arg[ counting ] != NULL )
	    {
                obj = NULL;
/*              if( !( char_has( ch, arg[ counting ] ) ) ) */
                if ( (obj = get_obj_carry( ch, arg[counting], ch, TRUE )) == NULL )
		{
		  sprintf( buf, "You don't have %s.\n\r", arg[ counting ] );
		  send_to_char( buf, ch );
		  return;
		}
                else if ( !can_drop_obj( ch, obj ) )
                {
                    act( "You can't let go of $p.", ch, obj, NULL, TO_CHAR );
                    return;
                }
	    }
	}

      for( counting = 0; counting < 11; counting++ )
	{
	  components[ counting ] = 0;

	  obj = get_obj_carry( ch, arg[ counting ], ch, FALSE );

	  if( ( obj != NULL ) && ( strcmp( arg[ counting ], "" ) ) )
	    {
	      components[ counting ] = ( int )( obj->pIndexData->vnum );

	      if( IS_SET( obj->extra_flags, ITEM_CREATED ) )
		{
		  send_to_char( a_trade->reuse_msg, ch );
		  return;
		}
	    }
	}

      if( check_duplicates( components ) )
	{
	  send_to_char( "You cannot combine two of the same thing.\n\r", ch );
	  return;
	}

      recipe_set = get_recipe_set( a_trade );

      if( recipe_set == NULL )
	{
	  bug( "do_trade: NULL recipe_set", 0);
	  return;
	}

      for( counting = 0; recipe_set[ counting ].trade_level != 0; counting++ )
	{
	  components_total = 0;

	  for( components_needed = 0;
	       recipe_set[ counting ].component[ components_needed ] != 0;
	       components_needed++ );

	  match = TRUE;
	  
	  for( objcheck = 0; components[ objcheck ] != 0; objcheck++ )
	    {

	      if( !recipe_needs( &recipe_set[ counting ], components[ objcheck ] ) )
		{
		  match = FALSE;
		}
	      else
		{
		  components_total++;
		}
	    }

	  if( ( match == TRUE ) && ( components_total == components_needed ) )
	    {
	      create_from_recipe( &( recipe_set[ counting ] ), ch, argument, a_trade );
	      return;
	    }
	}
    
      send_to_char( "That didn't make anything.\n\r", ch );
      destroy_components( ch, argument );
    }
        
}



/*
 * Returns TRUE if the character is eligible to practice the trade
 */

bool char_can_trade( CHAR_DATA* ch, char* argument, TRADE_DATA* a_trade )
{
  bool eligible = TRUE;

  if( a_trade->trade == FLETCHING )
    {
      if( ch->level < LEVEL_IMMORTAL )
	{
	  if( ch->class != class_lookup( "ranger" ) &&
	      ch->class != class_lookup( "avatar" ) )
	    {
	      eligible = FALSE;
	    }
	}
    }

  return eligible;
}



/*
 * Returns TRUE if the character has the object "argument" in his inventory
 */

bool char_has( CHAR_DATA* ch, char* argument )
{
  bool obj_present = TRUE;

  if( get_obj_carry( ch, argument, ch, TRUE ) == NULL )
    {
      obj_present = FALSE;
    }
  
  return obj_present;
}



/*
 * Returns TRUE if any two elements of objects are the same
 */

bool check_duplicates( int objects[ 11 ] )
{
  int outer;
  int inner;

  for( outer = 0; outer < 10; outer++ )
    {
      for( inner = ( outer + 1 ); inner < 11; inner++ )
	{
	  if( ( objects[ inner ] == objects[ outer ] ) &&
	      ( objects[ inner ] != 0 ) )
	    {
	      return TRUE;
	    }
	}
    }

  return FALSE;
}



/*
 * Returns TRUE if "vnum" is an ingredient in the recipe "card"
 */

bool recipe_needs( RECIPE* card, int vnum )
{
  int i;
  bool found = FALSE;

  for( i = 0; i < 9; i++ )
    {
      if( card->component[ i ] == vnum )
	{
	  found = TRUE;
	}
    }

  return found;
}



/*
 * Determines failure or sucess, and does the action
 */
void create_from_recipe( RECIPE* card, CHAR_DATA* ch, char* argument, TRADE_DATA* a_trade )
{
  int counting;
  int chance;
  int* character_trade;
  OBJ_DATA* obj;
  OBJ_INDEX_DATA* pObjIndex;
  char buf[ MAX_STRING_LENGTH ];
  char arg[ 10 ][ MAX_STRING_LENGTH ];
  bool success = TRUE;
  bool improve_possible = TRUE;

  if( card->focusitem )
    {
      obj = get_eq_char( ch, WEAR_HOLD );

      if( ( obj == NULL ) || ( obj->pIndexData->vnum != card->focusitem ) )
	{
	  send_to_char( "You lack the tools you need to combine those items\n\r", ch);
	  success = FALSE;
	  improve_possible = FALSE;
	}
    }

  if( success )
    {
      destroy_components( ch, argument );

      for( counting = 0; counting < 11; counting++ )
	{
	  argument = one_argument( argument, arg[ counting ] );
	}
      
      character_trade = get_character_skill( ch, a_trade );
    
      if( *character_trade < card->trade_level - 6 )
	{
	  send_to_char( "You lack the skill to fashion these items together.\n\r", ch );
	  success = FALSE;
	  improve_possible = FALSE;
	}
    }

/*if( success )
    {
      if( ch->level < card->level - 6 )
	{
	  send_to_char( "You aren't large enough to properly fashion these items.\n\r", ch );
	  success = FALSE;
	  improve_possible = FALSE;
	}
    }
 */
      
  if( success )
    {
      chance = 90;
      chance += *character_trade;
/*    chance += ch->level; */
      chance -= card->trade_level;
/*    chance -= card->level; */

      if( ( number_range( 65, 100 ) > chance ) || ( number_range( 1, 100 ) < 5 ) )
	{
	  send_to_char( a_trade->dam_msg_char, ch );
	  act( a_trade->dam_msg_room, ch, NULL, NULL, TO_ROOM );
	  damage_old( ch, ch, a_trade->damage, TYPE_HIT, a_trade->dam_type, TRUE );
	  success = FALSE;
	}
    }

  if( success )
    {
      for( counting = 0; card->goods[ counting ] != 0; counting++ )
	{
	  pObjIndex = get_obj_index( card->goods[ counting ] );

	  if( pObjIndex == NULL )
	    {
	      send_to_char( "No object has that vnum.\n\r", ch );
	      success = FALSE;
	    }
	  else
	    {
	      obj = create_object( pObjIndex, 0 );

	      if( card->wearloc )
		{
		  switch( card->wearloc )
		    {
		    case ITEM_WEAR_FINGER:
		      sprintf(buf,"a %s ring",obj->short_descr);
		      break;
		    case ITEM_WEAR_NECK:
		      sprintf(buf,"a %s necklace",obj->short_descr);
		      break;
		    case ITEM_WEAR_BODY:
		      sprintf(buf,"a %s vest",obj->short_descr);
		      break;
		    case ITEM_WEAR_HEAD:
		      sprintf(buf,"a %s cap",obj->short_descr);
		      break;
		    case ITEM_WEAR_LEGS:
		      sprintf(buf,"a %s kilt",obj->short_descr);
		      break;
		    case ITEM_WEAR_FEET:
		      sprintf(buf,"%s socks",obj->short_descr);
		      break;
		    case ITEM_WEAR_HANDS:
		      sprintf(buf,"a pair of %s gloves",obj->short_descr);
		      break;
		    case ITEM_WEAR_ARMS:
		      sprintf(buf,"%s sleeves",obj->short_descr);
		      break;
		    case ITEM_WEAR_SHIELD:
		      sprintf(buf,"a %s shield",obj->short_descr);
		      break;
		    case ITEM_WEAR_ABOUT:
		      sprintf(buf,"a cloak made of %s",obj->short_descr);
		      break;
		    case ITEM_WEAR_WAIST:
		      sprintf(buf,"a %s sash",obj->short_descr);
		      break;
		    case ITEM_WEAR_WRIST:
		      sprintf(buf,"a %s bracer",obj->short_descr);
		      break;
		    case ITEM_WEAR_FLOAT:
		      sprintf(buf,"a floating piece of %s",obj->short_descr);
		      break;
		    default:
		      sprintf(buf,"BAD EQUIPMENT - PLEASE RETURN TO AN IMM");
		      break;
		    }

		  obj->wear_flags = card->wearloc + 1;
		  free_string( obj->short_descr );
		  obj->short_descr = str_dup( buf );
		  free_string( obj->name );
		  obj->name = str_dup( buf );
	
		}
	      
	      SET_BIT( obj->extra_flags, ITEM_CREATED );
	      obj_to_char( obj, ch );
	    }

	  send_to_char( a_trade->success_char, ch );

	  act( a_trade->success_room, ch, NULL, NULL, TO_ROOM );

	}
    }

  if( improve_possible )
    {
      check_improvement( ch, a_trade, card );
    }

}



/*
 * Checks the appropriate skill for improvement
 */
void check_improvement( CHAR_DATA* ch, TRADE_DATA* a_trade, RECIPE* a_recipe )
{
  int* character_trade;

  if( !IS_NPC( ch ) )
    {
      character_trade = get_character_skill( ch, a_trade );

      if( *character_trade < a_trade->skill_cap )
	{
	  if( *character_trade > a_recipe->improve_level + 5 )
	    {
	      send_to_char( "This item no longer presents a challenge to make.\n\r", ch );
	    }
	  else if( *character_trade > a_trade->high_level )
	    {
	      if( number_range( 1, 100 ) < a_trade->chance_high )
		{
		  send_to_char( a_trade->improve_msg, ch );
		  (*character_trade)++;
		}
	    }
	  else
	    {
	      if( number_range( 1, 100 ) < a_trade->chance_low )
		{
		  send_to_char( a_trade->improve_msg, ch );
		  (*character_trade)++;
		}
	    }
	}
    }
  else
    {
      send_to_char( "Only PCs may exercise a trade\n\r", ch );
    }
}



/*
 * Disposes of the ingredients
 */
void destroy_components( CHAR_DATA* ch, char* argument )
{
  int i;
  OBJ_DATA* obj;
  char arg[ 11 ][ MAX_STRING_LENGTH ];

  for( i = 0; i < 11; i++ )
    {
      argument = one_argument( argument, arg[ i ] );
    }

  for( i = 0; i < 11; i++ )
    {
      obj = get_obj_carry( ch, arg[ i ], ch, FALSE );

      if( obj != NULL && strcmp( arg[ i ], "" ) )
	{
	  extract_obj( obj );
	}
    }
}



/*
 * Returns a pointer to the character's skill specified by a_trade.
 * This function allows the actual trade functions to be generic, and
 * will need to be updated if any new trades are added.
 */
int* get_character_skill( CHAR_DATA* ch, TRADE_DATA* a_trade )
{
  int* the_skill;
  int a_trade_number;

  a_trade_number = a_trade->trade;

  switch( a_trade_number )
    {
    
    case TAILORING:
      the_skill = &( ch->pcdata->tailoring );
      break;

    case BAKING:
      the_skill = &( ch->pcdata->baking );
      break;

    case FLETCHING:
      the_skill = &( ch->pcdata->fletching );
      break;

    default:
      the_skill = NULL;
    }

  return the_skill;
}



RECIPE* get_recipe_set( TRADE_DATA* a_trade )
{
/*  return a_trade->recipe_set; */
return (RECIPE*)a_trade->recipe_set;
}

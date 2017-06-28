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
#include <sys/time.h>
#endif
#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"
#include "clans.h"

/* command procedures needed */
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_help		);
DECLARE_DO_FUN( do_affects	);
DECLARE_DO_FUN( do_play		);
DECLARE_DO_FUN( check_social	);
DECLARE_DO_FUN( do_ostat        );
DECLARE_DO_FUN( do_mpstat       );

void look_sky args(( CHAR_DATA *ch ));

extern void do_bloodpool( CHAR_DATA* ch, char* argument );

const char default_prompt[] = 
  "({B%h{x/{B%H{xhp {m%m{x/{m%M{xmn {g%v{xmv) (%X TNL) (%e)%c";

char* PERS( CHAR_DATA* ch, CHAR_DATA* looker )
{
  if( can_see( looker, ch ) )
    {
      if( IS_NPC( ch ) )
	return ch->short_descr;
    
      if( IS_SET( ch->shiftbits, SHIFT_FULL ) )
	return were_table[ ch->race ].short_full;

      if( IS_SET( ch->shiftbits, SHIFT_HALF ) )
	return were_table[ ch->race ].short_half;

      if( IS_SET( ch->shiftbits, BAT_VAMP ) )
	return "a bat";

      if( IS_SET( ch->shiftbits, MIST_VAMP ) )
	return "some mist";

      return ch->name;
    }

  return "someone";
}

bool is_extra_descr_abbrev( const char *name, EXTRA_DESCR_DATA *ed )
{
  EXTRA_DESCR_DATA *ped;
  for ( ped = ed; ped != NULL; ped = ped->next )
    {
      if ( is_abbrev( (char *) name, ped->keyword )
	   &&   str_cmp(ped->keyword,"sky") )
	return TRUE;
    }
  return FALSE;
}

char *	const	where_name	[] =
  {
    "<used as light>     ",
    "<worn on finger>    ",
    "<worn on finger>    ",
    "<worn around neck>  ",
    "<worn around neck>  ",
    "<worn on torso>     ",
    "<worn on head>      ",
    "<worn on legs>      ",
    "<worn on feet>      ",
    "<worn on hands>     ",
    "<worn on arms>      ",
    "<worn as shield>    ",
    "<worn about body>   ",
    "<worn about waist>  ",
    "<worn around wrist> ",
    "<worn around wrist> ",
    "<wielded>           ",
    "<held>              ",
    "<floating nearby>   ",
    "<worn with pride>   ",
  };


/* for do_count */
int max_on = 0;



/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
					bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
					bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );


int armor_points( OBJ_INDEX_DATA* obj )
{
  int total = 0;
  int scratch = 0;
  AFFECT_DATA* paff = 0;
  int i;

  if( obj == NULL )
    return -1;

  /* base AC sum/2 */
  if( obj->item_type == ITEM_ARMOR )
    {
      for( i = 0; i < 4; i++ )
	scratch += obj->value[ i ];
      
      total += scratch / 2;
      scratch = 0;
    }

  for( paff = obj->affected; paff; paff = paff->next )
    {
      switch( paff->location )
	{
	  /* affect AC */
	case APPLY_AC:
	  total -= paff->modifier;
	  break;

	  /* level stats *4 */
	case APPLY_INT:
	case APPLY_WIS:
	case APPLY_CON:
	  total += 4 * paff->modifier;
	  break;

	  /* battle stats *5 */
	case APPLY_STR:
	case APPLY_DEX:
	  total += 5 * paff->modifier;
	  break;

	  /* life stats / 5 */
	case APPLY_HIT:
	case APPLY_MANA:
	case APPLY_MOVE:
	case APPLY_BLOOD:
	  total += paff->modifier / 5;
	  break;

	  /* hitroll ^2 */
	  /* damroll ^2 */
	case APPLY_HITROLL:
	case APPLY_DAMROLL:
	  {
	    if( paff->modifier > 0 )
	      total += paff->modifier * paff->modifier;
	    else
	      total -= paff->modifier * paff->modifier;

	    break;
	  }

	  /* saves ^2+X */
	case APPLY_SAVES:
	  {
	    if( paff->modifier < 0 )
	      {
		total += ( paff->modifier * paff->modifier );
		total -= paff->modifier;
	      }
	    else
	      {
		total -= ( paff->modifier * paff->modifier ); 
		total += paff->modifier;
	      }
	    break;
	  }

	default:
	  break;
	      
	}
    }

  return UMIN(total, 1000);
}

int weapon_points( OBJ_INDEX_DATA* obj )
{
  int total = 0;
  AFFECT_DATA* paff = 0;

  if( obj == NULL )
    return -1;

  /* avg damage /2 */
  /*  total += ( ( obj->value[ 1 ] * obj->value[ 2 ] + obj->value[ 1 ] ) / 2 ) / 2; */
  total = (1 + obj->value[2]) * obj->value[1] / 2;
  total /= 2;

  for( paff = obj->affected; paff; paff = paff->next )
    {
      switch( paff->location )
	{
	  /* AC /2 */
	case APPLY_AC:
	  total -= (paff->modifier / 2);
	  break;

	  /* level stats *4 */
	case APPLY_INT:
	case APPLY_WIS:
	case APPLY_CON:
	  total += 4 * paff->modifier;
	  break;

	  /* battle stats *5 */
	case APPLY_DEX:
	case APPLY_STR:
	  total += 5 * paff->modifier;
	  break;

	  /* life stats /5 */
	case APPLY_HIT:
	case APPLY_MANA:
	case APPLY_MOVE:
	case APPLY_BLOOD:
	  total += (paff->modifier / 5);
	  break;

	  /* hitdam (hit+dam)*2 */
	case APPLY_HITROLL:
	case APPLY_DAMROLL:
	  total += paff->modifier * 2;
	  break;

	  /* saves ^2 */
	case APPLY_SAVES:
	  {
	    if( paff->modifier < 0 )
	      total += paff->modifier * paff->modifier;
	    else
	      total -= paff->modifier * paff->modifier;
	    
	    break;
	  }

	default:
	  break;

	}
    }

  /* magic +5 */
  if( attack_table[ obj->value[ 3 ] ].damage > DAM_SLASH )
    total += 5;
  
  /* two-handed -10 */
  if( IS_WEAPON_STAT( obj, WEAPON_TWO_HANDS ) )
    total -= 10;

  if ( IS_WEAPON_STAT( obj, WEAPON_SHARP ) )
    total += 10;

  if ( IS_WEAPON_STAT( obj, WEAPON_VORPAL ) )
    total += 10;

  return UMIN(total,1000);
}

  
int get_object_points( OBJ_INDEX_DATA* obj )
{
  int total = 0;

  if( obj == NULL )
    return -1;

  if ( obj->item_type == ITEM_WEAPON )
    total = weapon_points( obj );
  else
    total = armor_points ( obj );

  return UMIN(total,1000);
}

/*void
  equip_aff(CHAR_DATA *ch)
  {
  int i;
  AFFECT_DATA *paf;
  OBJ_DATA *pobj;
  int found = 0;
  char buf[MAX_STRING_LENGTH];

  send_to_char("Equipment affects: \n\r",ch);

  for (i = 0; i < MAX_WEAR; i++){
  if ((pobj = get_eq_char(ch,i)) == NULL)
  continue;

  for (paf = pobj->pIndexData->affected; paf !=NULL; paf= paf->next){
  if (paf->location != APPLY_SPELL_AFFECT) {
  found = 1;
  switch(paf->where){
  case TO_AFFECTS:
  sprintf(buf,"Affected by: %s\n\r",
  flag_string( affect_flags, paf->bitvector ) );
  send_to_char(buf,ch);
  break;
  case TO_IMMUNE:
  sprintf(buf,"Immune: %s\n\r",
  flag_string( imm_flags, paf->bitvector ) );
*//*                          imm_bit_name(paf->bitvector));*//*
								send_to_char(buf,ch);
								break;
								case TO_RESIST:
								sprintf(buf,"Resist: %s\n\r",
								flag_string( res_flags, paf->bitvector ) );
							      *//*                        imm_bit_name(paf->bitvector));*//*
															    send_to_char(buf,ch);
															    break;
															    case TO_VULN:
															    sprintf(buf,"Vulnerable: %s\n\r",
															    flag_string( vuln_flags, paf->bitvector ) );
															  *//*                        imm_bit_name(paf->bitvector));*//*
																							send_to_char(buf,ch);
																							break;
																							}
																							}
																							}
																							}

																							if (!found)
																							send_to_char("None\n\r",ch);
																							}
																						      */

char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
  static char buf[MAX_STRING_LENGTH];

  buf[0] = '\0';

  if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
      ||  (obj->description == NULL || obj->description[0] == '\0'))
    return buf;

  if ( !IS_NPC(ch) /* && ch->pcdata->security > 0 )*/
       &&   ( IS_ADMIN(ch) || IS_BUILDER(ch, obj->pIndexData->area) ) )
    {
      if ( IS_OBJ_STAT(obj, ITEM_NOTSEEN) && !fShort ) strcat( buf, "({MNotseen{x) " );
      if ( IS_OBJ_STAT(obj, ITEM_VIS_DEATH) )          strcat( buf, "({MVisdeath{x) " );
      if ( IS_OBJ_STAT(obj, ITEM_INVENTORY ) )         strcat( buf, "({MInventory{x) " );
    }
  if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )   strcat( buf, "(Invis) "     );
  if ( ( IS_AFFECTED(ch, AFF_DETECT_MAGIC) || IS_SET( ch->act_bits, PLR_HOLYLIGHT ) )
       && IS_OBJ_STAT(obj, ITEM_MAGIC)  )   strcat( buf, "(Magical) "   );
  if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "(Glowing) "   );
  if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "(Humming) "   );

  if ( fShort )
    {
      if ( obj->short_descr != NULL )
	strcat( buf, obj->short_descr );
    }
  else
    {
      if ( obj->description != NULL ) 
	strcat( buf, obj->description );
    }

  if ( IS_OBJ_STAT(obj, ITEM_NOTSEEN) 
       &&  !fShort  
       &&  (IS_NPC(ch) || (!IS_ADMIN(ch) && !IS_BUILDER(ch, obj->pIndexData->area)) ) )
    buf[0] = '\0';

  return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
  char buf[MAX_STRING_LENGTH];
  BUFFER *output;
  char **prgpstrShow;
  int *prgnShow;
  char *pstrShow;
  OBJ_DATA *obj;
  int nShow;
  int iShow;
  int count;
  bool fCombine;

  if ( ch->desc == NULL || list == NULL)
    return;

  /*
   * Alloc space for output lines.
   */
  output = new_buf();

  count = 0;
  for ( obj = list; obj != NULL; obj = obj->next_content )
    count++;

  prgpstrShow	= alloc_mem( count * sizeof(char *) );
  prgnShow    = alloc_mem( count * sizeof(int)    );
  nShow	= 0;

  /*
   * Format the list of objects.
   */
  for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
      if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj )) 
	{
	  pstrShow = format_obj_to_char( obj, ch, fShort );

	  fCombine = FALSE;

	  if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    {
	      /*
	       * Look for duplicates, case sensitive.
	       * Matches tend to be near end so run loop backwords.
	       */
	      for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		  if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
		      prgnShow[iShow]++;
		      fCombine = TRUE;
		      break;
		    }
		}
	    }

	  /*
	   * Couldn't combine, or didn't want to.
	   */
	  if ( !fCombine )
	    {
	      prgpstrShow [nShow] = str_dup( pstrShow );
	      prgnShow    [nShow] = 1;
	      nShow++;
	    }
	}
    }

  /*
   * Output the formatted list.
   */
  for ( iShow = 0; iShow < nShow; iShow++ )
    {
      if (prgpstrShow[iShow][0] == '\0')
	{
	  free_string(prgpstrShow[iShow]);
	  continue;
	}

      if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	{
	  if ( prgnShow[iShow] != 1 )
	    {
	      sprintf( buf, "(%2d) ", prgnShow[iShow] );
	      add_buf(output,buf);
	    }
	  else
	    {
	      add_buf(output,"     ");
	    }
	}
      add_buf(output,prgpstrShow[iShow]);
      add_buf(output,"\n\r");
      free_string( prgpstrShow[iShow] );
    }

  if ( fShowNothing && nShow == 0 )
    {
      if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	send_to_char( "     ", ch );
      send_to_char( "Nothing.\n\r", ch );
    }
  page_to_char(buf_string(output),ch);

  /*
   * Clean up.
   */
  free_buf(output);
  free_mem( prgpstrShow, count * sizeof(char *) );
  free_mem( prgnShow,    count * sizeof(int)    );

  return;
}



void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH],message[MAX_STRING_LENGTH];
  char temp[ MAX_STRING_LENGTH ];

  buf[0] = '\0';
  temp[0] = '\0';
    

  if ( !IS_IMMORTAL( ch ) && IS_AFFECTED3( victim, AFF_VEIL ) )
    {
      send_to_char( "A shady figure lurks in the shadows.\n\r", ch );
      return;
    }

  if ( !IS_IMMORTAL( ch ) && IS_AFFECTED3( victim, AFF_FROG ) )
    {
      send_to_char( "A helpless green frog is here.\n\r", ch );
      return;
    }

  if ( IS_SET(victim->comm,COMM_AFK	  )   ) strcat( buf, "[AFK] "	     );

  if ( victim->invis_level >= LEVEL_HERO    ) 
    {
      sprintf( temp, "(Wizi %d) ", victim->invis_level );
      strcat( buf, temp );
    }
	
  if ( IS_NPC(victim) )	/* For Builders */
    {
      if ( IS_SET(victim->act_bits, ACT_NOTSEEN) )
	strcat( buf, "({MNotseen{x) ");
      if ( IS_SET(victim->act_bits, ACT_WIZI) )
	strcat( buf, "({MMobwizi{x) ");
    }

  if ( IS_AFFECTED (victim, AFF_INVISIBLE)   ) strcat( buf, "({BInvis{x) "            );
  if ( IS_AFFECTED (victim, AFF_HIDE)        ) strcat( buf, "(Hide) "                 );
  if ( IS_AFFECTED (victim, AFF_CHARM)       ) strcat( buf, "({YCharmed{x)"           );
  if ( IS_AFFECTED2(victim, AFF_WEB)	       ) strcat( buf, "({rWebbed{x) "           );
  if ( IS_AFFECTED2(victim, AFF_GHOST)       ) strcat( buf, "({MGhost{x) "            );
  if ( IS_AFFECTED2(victim, AFF_WILDAURA)    ) strcat( buf, "({cMu{Blti{Mco{Blor{ced{x) "    );
  if ( IS_AFFECTED2(victim, AFF_MIRROR)      ) strcat( buf, "({CShimmering{x) "       );
  if ( IS_AFFECTED3(victim, AFF_SKULLS)      ) strcat( buf, "({DSkulls{x) "           );
  if ( IS_AFFECTED2(victim, AFF_SHADOWS)     ) strcat( buf, "({wShadows{x) "          );
  if ( IS_AFFECTED2(victim, AFF_REFLECT)     ) strcat( buf, "({BRighteous{x) "        );
  if ( IS_AFFECTED (victim, AFF_PASS_DOOR)   ) strcat( buf, "({RTranslucent{x) "      );
  if ( IS_AFFECTED (victim, AFF_FAERIE_FIRE) ) strcat( buf, "({MPink Aura{x) "        );
  //if ( IS_AFFECTED (victim, AFF_SPECTRAL_SHROUD) ) strcat( buf, "{r({DSpectral Shroud{r){x "        );
  if ( IS_AFFECTED (victim, AFF_SANCTUARY)   )
    {
      if ( affect_find(victim->affected,gsn_love) )
	strcat( buf, "({MLove{x) " );
      else
	strcat( buf, "({WWhite Aura{x) " );
    }
  if ( IS_AFFECTED2(ch, AFF_DETECTWERE) && 
       (IS_SET(victim->shiftbits,SHIFT_POTENTIAL) ) &&
       (ch->level >= 30) && (victim->level >=30)) strcat( buf, "({bWere{x) " );
  if ( IS_AFFECTED2(victim, AFF_WANTED)    ) strcat( buf, "{G(WANTED){x "     );
  if ( !IS_NPC(victim) && IS_SET(victim->act_bits, PLR_KILLER ) )
    strcat( buf, "(KILLER) "     );
  if ( !IS_NPC(victim) && IS_SET(victim->act_bits, PLR_NEWBIE  ) )
    strcat( buf, "[{G+{x] "     );
  if( IS_AFFECTED3( victim, AFF_VEIL ) ) strcat( buf, "({rVeiled{x) " );
  if( IS_AFFECTED2( victim, AFF2_BURNING ) ) strcat( buf, "({rBurning{x) " );
   
  if ( victim->position == victim->start_pos && victim->long_descr[0] != '\0' )
    {
      if (IS_SET(victim->shiftbits,SHIFT_FULL)){
	strcat(buf,were_table[victim->race].long_full);
	strcat(buf,"\n\r");
	send_to_char( buf, ch );
	return;
      }
      if (IS_SET(victim->shiftbits,SHIFT_HALF)){
	strcat(buf,were_table[victim->race].long_half);
	strcat(buf,"\n\r");
	send_to_char( buf, ch );
	return;
      }
      if (IS_SET(victim->shiftbits,BAT_VAMP)) {
	send_to_char("A large bat is here.\n\r",ch);
	return;
      }

      if (IS_SET(victim->shiftbits,MIST_VAMP)) {
	send_to_char("Some mist is here.\n\r",ch);
	return;
      }

      strcat( buf, victim->long_descr );
      send_to_char( buf, ch );
      return;
    }

  if (victim->position == POS_STANDING){
    if (IS_SET(victim->shiftbits,SHIFT_FULL)){
      strcat(buf,were_table[victim->race].long_full);
      strcat(buf,"\n\r");
      send_to_char( buf, ch );
      return;
    }
    if (IS_SET(victim->shiftbits,SHIFT_HALF)){
      strcat(buf,were_table[victim->race].long_half);
      strcat(buf,"\n\r");
      send_to_char( buf, ch );
      return;
    }
    if (IS_SET(victim->shiftbits,BAT_VAMP)) {
      send_to_char("A large bat is here.\n\r",ch);
      return;
    }

    if (IS_SET(victim->shiftbits,MIST_VAMP)) {
      send_to_char("Some mist is here.\n\r",ch);
      return;
    }

    if( !IS_IMMORTAL( ch ) && IS_AFFECTED3( victim, AFF_VEIL ) )
      {
	send_to_char( "A shady figure lurks in the shadows.\n\r", ch );
	return;
      }

  }

  strcat( buf, PERS( victim, ch ) );
  if ( !IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF) 
       &&   victim->position == POS_STANDING && ch->on == NULL )
    strcat( buf, victim->pcdata->title );

  switch ( victim->position )
    {
    case POS_DEAD:     strcat( buf, " is DEAD!!" );              break;
    case POS_UNCONSCIOUS:   strcat( buf, " is unconscious." );   break;
    case POS_SPRAWLED:    strcat( buf, " is sprawled out on the ground." );      break;
    case POS_SLEEPING: 
      if (victim->on != NULL)
	{
	  if (IS_SET(victim->on->value[2],SLEEP_AT))
  	    {
	      sprintf(message," is sleeping at %s.",
		      victim->on->short_descr);
	      strcat(buf,message);
	    }
	  else if (IS_SET(victim->on->value[2],SLEEP_ON))
	    {
	      sprintf(message," is sleeping on %s.",
		      victim->on->short_descr); 
	      strcat(buf,message);
	    }
	  else
	    {
	      sprintf(message, " is sleeping in %s.",
		      victim->on->short_descr);
	      strcat(buf,message);
	    }
	}
      else 
	strcat(buf," is sleeping here.");
      break;
    case POS_RESTING:  
      if (victim->on != NULL)
	{
	  if (IS_SET(victim->on->value[2],REST_AT))
            {
	      sprintf(message," is resting at %s.",
		      victim->on->short_descr);
	      strcat(buf,message);
            }
	  else if (IS_SET(victim->on->value[2],REST_ON))
            {
	      sprintf(message," is resting on %s.",
		      victim->on->short_descr);
	      strcat(buf,message);
            }
	  else 
            {
	      sprintf(message, " is resting in %s.",
		      victim->on->short_descr);
	      strcat(buf,message);
            }
	}
      else
	strcat( buf, " is resting here." );       
      break;
    case POS_SITTING:  
      if (victim->on != NULL)
        {
	  if (IS_SET(victim->on->value[2],SIT_AT))
            {
	      sprintf(message," is sitting at %s.",
		      victim->on->short_descr);
	      strcat(buf,message);
            }
	  else if (IS_SET(victim->on->value[2],SIT_ON))
            {
	      sprintf(message," is sitting on %s.",
		      victim->on->short_descr);
	      strcat(buf,message);
            }
	  else
            {
	      sprintf(message, " is sitting in %s.",
		      victim->on->short_descr);
	      strcat(buf,message);
            }
        }
      else
	strcat(buf, " is sitting here.");
      break;
    case POS_STANDING: 
      if (victim->on != NULL)
	{
	  if (IS_SET(victim->on->value[2],STAND_AT))
	    {
	      sprintf(message," is standing at %s.",
		      victim->on->short_descr);
	      strcat(buf,message);
	    }
	  else if (IS_SET(victim->on->value[2],STAND_ON))
	    {
	      sprintf(message," is standing on %s.",
		      victim->on->short_descr);
	      strcat(buf,message);
	    }
	  else
	    {
	      sprintf(message," is standing in %s.",
		      victim->on->short_descr);
	      strcat(buf,message);
	    }
	}
      else
	strcat( buf, " is here." );               
      break;
    case POS_FIGHTING:
      strcat( buf, " is here, fighting " );
      if ( victim->fighting == NULL )
	strcat( buf, "thin air??" );
      else if ( victim->fighting == ch )
	strcat( buf, "YOU!" );
      else if ( victim->in_room == victim->fighting->in_room )
	{
	  strcat( buf, PERS( victim->fighting, ch ) );
	  strcat( buf, "." );
	}
      else
	strcat( buf, "someone who left??" );
      break;
    }

  strcat( buf, "\n\r" );
  buf[0] = UPPER(buf[0]);
  send_to_char( buf, ch );
  return;
}

void show_frog_to_char_1( CHAR_DATA* victim, CHAR_DATA* ch )
{

  if( ch == victim )
    {
      act( "A frog looks at $mself.", ch, NULL, victim, TO_NOTVICT );
    }
  else if( ch->invis_level == 0 )
    {
      act( "$n looks at $N.", ch, NULL, victim, TO_NOTVICT );
    }

  act( "You see a frog looking around with what might be a frantic look on $s face.", ch, NULL, victim, TO_CHAR );

  return;
}
  


void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  char pueblo_buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  int iWear;
  int percent;
  int peekpct;
  bool found;

  /*
    if ( can_see( victim, ch ) )
    {
    if (ch == victim)
    act( "$n looks at $mself.",ch,NULL,NULL,TO_ROOM);
    else
    {
    act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
    act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
    }
    }
  */

  if( IS_AFFECTED3( victim, AFF_FROG ) )
    {
      show_frog_to_char_1( victim, ch );
      return;
    }

  if( ch == victim )
    {
      act( "$n looks at $mself.", ch, NULL, victim, TO_NOTVICT );
    }
  else if( ch->invis_level == 0 )
    {
      act( "$n looks at $N.", ch, NULL, victim, TO_NOTVICT );
    }

  if( can_see( victim, ch ) )
    {
      peekpct = get_skill( ch, gsn_peek );

      if ( IS_IMMORTAL(victim) 
	   ||   IS_NPC(victim) 
	   ||   peekpct < 1 
	   /*         20% chance of automatically being caught */
	   ||   number_percent() < 21
	   /*           a percent peek check after the 20% autofail */
	   ||   ( number_percent() > peekpct ) )
	{
	  act( "$n looks at you.", ch, NULL, victim, TO_VICT );
	}
    }


  if ( (IS_SET(victim->shiftbits,SHIFT_FULL) && 
	(IS_SET(ch->act_bits,PLR_PUEBLO))) ||
       (IS_SET(victim->shiftbits,MIST_VAMP) &&
	(!IS_NULLSTR(victim->shiftimage2)) &&
	(IS_SET(ch->act_bits,PLR_PUEBLO))))
    {
      sprintf(pueblo_buf,"<img src=\"%s%s\">",
	      PUEBLO_DIR,victim->shiftimage2);
      send_to_char("</xch_mudtext><img xch_mode=html>",ch);
      send_to_char(pueblo_buf,ch);
      send_to_char("<br><img xch_mode=text>",ch);
    }

  else if ((IS_SET(victim->shiftbits,SHIFT_HALF) &&
	    (IS_SET(ch->act_bits,PLR_PUEBLO))) ||
	   (IS_SET(victim->shiftbits,BAT_VAMP) &&
	    (!IS_NULLSTR(victim->shiftimage1)) &&
	    (IS_SET(ch->act_bits,PLR_PUEBLO))))
    {
      sprintf(pueblo_buf,"<img src=\"%s%s\">",
	      PUEBLO_DIR,victim->shiftimage1);
      send_to_char("</xch_mudtext><img xch_mode=html>",ch);
      send_to_char(pueblo_buf,ch);
      send_to_char("<br><img xch_mode=text>",ch);
    }

  else if (IS_NPC(victim)
	   && (!IS_NULLSTR(victim->image))
	   && (IS_SET(ch->act_bits,PLR_PUEBLO)))
    {
      sprintf(pueblo_buf,"<img src=\"%s%s\">",
	      PUEBLO_DIR,victim->image);
      send_to_char("</xch_mudtext><img xch_mode=html>",ch);
      send_to_char(pueblo_buf,ch);
      send_to_char("<br><img xch_mode=text>",ch);
    }

  else if (IS_NPC(victim)
	   && (IS_NULLSTR(victim->image))
	   && (!IS_NULLSTR(victim->pIndexData->image))
	   && (IS_SET(ch->act_bits,PLR_PUEBLO)))
    {
      sprintf(pueblo_buf,"<img src=\"%s%s\">",
	      PUEBLO_DIR,victim->pIndexData->image);
      send_to_char("</xch_mudtext><img xch_mode=html>",ch);
      send_to_char(pueblo_buf,ch);
      send_to_char("<br><img xch_mode=text>",ch);
    }

  else if (!IS_NPC(victim)
	   && (!IS_NULLSTR(victim->image)) 
	   && (IS_SET(ch->act_bits,PLR_PUEBLO))) 
    {
      sprintf(pueblo_buf,"<img src=\"%s%s\">",
	      PUEBLO_DIR,victim->image);
      send_to_char("</xch_mudtext><img xch_mode=html>",ch);
      send_to_char(pueblo_buf,ch);
      send_to_char("<br><img xch_mode=text>",ch);
    }

  if ((IS_SET(victim->shiftbits,SHIFT_FULL) ||
       IS_SET(victim->shiftbits,MIST_VAMP)) 
      && (victim->shiftdesc2[0] != '\0'))
    send_to_char( victim->shiftdesc2, ch );
  else if ((IS_SET(victim->shiftbits,SHIFT_HALF) ||
	    IS_SET(victim->shiftbits,BAT_VAMP))
	   && (victim->shiftdesc1[0] != '\0'))
    send_to_char( victim->shiftdesc1, ch );
  else if ( victim->description[0] != '\0' )
    {
      send_to_char( victim->description, ch );
    }
  else
    {
      act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

  if ( victim->max_hit > 0 )
    percent = ( 100 * victim->hit ) / victim->max_hit;
  else
    percent = -1;

  strcpy( buf, PERS(victim, ch) );

  if (percent >= 100) 
    strcat( buf, "{g is in excellent condition.{x\n\r");
  else if (percent >= 90) 
    strcat( buf, "{g has a few scratches.{x\n\r");
  else if (percent >= 75) 
    strcat( buf,"{g has some small wounds and bruises.{x\n\r");
  else if (percent >=  50) 
    strcat( buf, "{c has quite a few wounds.{x\n\r");
  else if (percent >= 30)
    strcat( buf, "{c has some big nasty wounds and scratches.{x\n\r");
  else if (percent >= 15)
    strcat ( buf, "{r looks pretty hurt.{x\n\r");
  else if (percent >= 0 )
    strcat (buf, "{r is in awful condition.{x\n\r");
  else
    strcat(buf, "{r is bleeding to death.{x\n\r");

  buf[0] = UPPER(buf[0]);
  send_to_char( buf, ch );

  if (IS_SET(victim->shiftbits,SHIFT_HALF) ||
      IS_SET(victim->shiftbits,MIST_VAMP) ||
      IS_SET(victim->shiftbits,BAT_VAMP) ||
      IS_SET(victim->shiftbits,SHIFT_FULL)) {
    return;
  }

  found = FALSE;
  for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
      if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	   &&   can_see_obj( ch, obj ) )
	{
	  if ( !found )
	    {
	      send_to_char( "\n\r", ch );
	      act( "$N is using:", ch, NULL, victim, TO_CHAR );
	      found = TRUE;
	    }
	  send_to_char( where_name[iWear], ch );
	  send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	  send_to_char( "\n\r", ch );
	}
    }

  if ( victim != ch
       &&   !IS_NPC(ch)
       && (victim->position != POS_UNCONSCIOUS)
       &&   number_percent( ) < get_skill(ch,gsn_peek))
    {
      send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
      check_improve(ch,gsn_peek,TRUE,4);
      show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }

  if ( victim != ch
       &&   !IS_NPC(ch)
       &&   (victim->position == POS_UNCONSCIOUS))
    {
      send_to_char( "\n\rIn their inventory:\n\r", ch );
      show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }

  return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
  CHAR_DATA *rch;

  for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
      if ( rch == ch )
	continue;

      if ( get_trust(ch) < rch->invis_level)
	continue;

      if (  IS_NPC( rch ) 
	    &&    IS_SET( rch->act_bits, ACT_NOTSEEN )
	    &&   !IS_ADMIN( ch )
	    &&   !IS_BUILDER( ch, rch->pIndexData->area ) )
	continue;

      if ( can_see( ch, rch ) )
	{
	  show_char_to_char_0( rch, ch );
	}
      else if ( room_is_dark( ch->in_room )
		&&        IS_AFFECTED(rch, AFF_INFRARED ) 
		&& !(IS_NPC( rch ) && IS_SET(rch->act_bits, ACT_WIZI)))
	{
	  send_to_char( "You see glowing red eyes watching YOU!\n\r", ch );
	}
    }

  return;
} 



bool check_blind( CHAR_DATA *ch )
{

  if (!IS_NPC(ch) && IS_SET(ch->act_bits,PLR_HOLYLIGHT))
    return TRUE;

  if ( IS_AFFECTED(ch, AFF_BLIND) )
    { 
      send_to_char( "You can't see a thing!\n\r", ch ); 
      return FALSE; 
    }

  if ( IS_AFFECTED3(ch, AFF_DIRT) )
    { 
      send_to_char( "You can't see a thing!\n\r", ch ); 
      return FALSE; 
    }

  return TRUE;
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[100];
  int lines;

  one_argument(argument,arg);
    
  if (arg[0] == '\0')
    {
      if (ch->lines == 0)
	send_to_char("You do not page long messages.\n\r",ch);
      else
	{
	  sprintf(buf,"You currently display %d lines per page.\n\r",
		  ch->lines + 2);
	  send_to_char(buf,ch);
	}
      return;
    }

  if (!is_number(arg))
    {
      send_to_char("You must provide a number.\n\r",ch);
      return;
    }

  lines = atoi(arg);

  if (lines == 0)
    {
      send_to_char("Paging disabled.\n\r",ch);
      ch->lines = 0;
      return;
    }

  if (lines < 10 || lines > 100)
    {
      send_to_char("You must provide a reasonable number.\n\r",ch);
      return;
    }

  sprintf(buf,"Scroll set to %d lines.\n\r",lines);
  send_to_char(buf,ch);
  ch->lines = lines - 2;
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char hiddensocials[] = "castline playline sethook waveflag thirku yutsu";
  int iSocial;
  int col;
     
  col = 0;
   
  for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
      if ( is_name( social_table[iSocial].name, hiddensocials ) )
	continue;
      sprintf(buf,"%-12s",social_table[iSocial].name);
      send_to_char(buf,ch);
      if (++col % 6 == 0)
	send_to_char("\n\r",ch);
    }

  if ( col % 6 != 0)
    send_to_char("\n\r",ch);
  return;
}

void do_maxstat( CHAR_DATA* ch, char* argument )
{
  int ceiling[ MAX_STATS ];
  int i;
  int deficit[ MAX_STATS ];
  char* outputpt = 0;
  char output[ MAX_STRING_LENGTH ];
  char names[ MAX_STATS ][ MAX_STRING_LENGTH ] = { "strength",
						   "intelligence",
						   "wisdom",
						   "dexterity",
						   "constitution" };
  bool printstat;

  outputpt = output;

  if( !IS_NPC( ch ) )
    {
      for( i = 0; i < MAX_STATS; i++ )
	{
	  ceiling[ i ] = ( pc_race_table[ ch->race ].max_stats[ i ] + 4 );
	  
	  if( class_table[ ch->class ].attr_prime == i )
	    ceiling[ i ] += 2;
	  
	  if( class_table[ ch->class ].attr_sec == i )
	    ceiling[ i ] += 1;

	  if( ceiling[ i ] > MAX_STAT_LIMIT )
	    ceiling[ i ] = MAX_STAT_LIMIT;
	  
	  deficit[ i ] = ceiling[ i ] - get_curr_stat( ch, i );

	  printstat = TRUE;

	  if( ch->level < 50 && ( i == STAT_STR || i == STAT_DEX ) )
	    printstat = FALSE;

	  if( printstat )
	    {
	      if( deficit[ i ] > 0 )
		{
		  outputpt += sprintf( outputpt, "You need %d points to maximize %s.\n\r",
				       deficit[ i ],
				       names[ i ] );
		}
	      else
		{
		  outputpt += sprintf( outputpt, "Your %s is maximized.\n\r", names[ i ] );
		}
	    }
	}

      if( deficit[ STAT_INT ] == 0 &&
	  deficit[ STAT_WIS ] == 0 &&
	  deficit[ STAT_CON ] == 0 )
	{
	  outputpt += sprintf( outputpt, "You are ready to level.\n\r" );
	}
      else
	{
	  outputpt += sprintf( outputpt, "YOU ARE NOT READY TO LEVEL.\n\r" );
	}

      send_to_char( output, ch );

    }
}
			  




void do_motd(CHAR_DATA *ch, char *argument)
{
  do_help(ch,"motd");
}

void do_imotd(CHAR_DATA *ch, char *argument)
{  
  do_help(ch,"imotd");
}

void do_rules(CHAR_DATA *ch, char *argument)
{
  do_help(ch,"rules");
}

void do_story(CHAR_DATA *ch, char *argument)
{
  do_help(ch,"story");
}

void do_wizlist(CHAR_DATA *ch, char *argument)
{
  do_help(ch,"wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist(CHAR_DATA *ch, char *argument)
{
  /* lists most player flags */
  if (IS_NPC(ch))
    return;

  send_to_char("   action     status\n\r",ch);
  send_to_char("---------------------\n\r",ch);
 
  send_to_char("autoassist     ",ch);
  if (IS_SET(ch->act_bits,PLR_AUTOASSIST))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch); 

  send_to_char("autoexit       ",ch);
  if (IS_SET(ch->act_bits,PLR_AUTOEXIT))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("autogold       ",ch);
  if (IS_SET(ch->act_bits,PLR_AUTOGOLD))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("autoloot       ",ch);
  if (IS_SET(ch->act_bits,PLR_AUTOLOOT))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("autosac        ",ch);
  if (IS_SET(ch->act_bits,PLR_AUTOSAC))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("autosplit      ",ch);
  if (IS_SET(ch->act_bits,PLR_AUTOSPLIT))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("battlebrief    ",ch);
  if (IS_SET(ch->act_bits,PLR_BATTLEBRIEF))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("compact mode   ",ch);
  if (IS_SET(ch->comm,COMM_COMPACT))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("prompt         ",ch);
  if (IS_SET(ch->comm,COMM_PROMPT))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("combine items  ",ch);
  if (IS_SET(ch->comm,COMM_COMBINE))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  if (IS_SET(ch->act_bits,PLR_NOSUMMON))
    send_to_char("You cannot be summoned.\n\r",ch);
  else
    send_to_char("You can be summoned.\n\r",ch);
   
  if (IS_SET(ch->act_bits,PLR_NOFOLLOW))
    send_to_char("You do not welcome followers.\n\r",ch);
  else
    send_to_char("You accept followers.\n\r",ch);

  if (IS_SET(ch->act_bits,PLR_NEWBIE))
    send_to_char("You have newbie status.\n\r",ch);
  else
    send_to_char("You don't have newbie status.\n\r",ch);

  if( IS_SET( ch->act_bits, PLR_SUSPEND_LEVEL ) )
    {
      send_to_char( "YOUR LEVELLING IS SUSPENDED!\n\r", ch );
    }

}

void do_autoassist(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;
    
  if (IS_SET(ch->act_bits,PLR_AUTOASSIST))
    {
      send_to_char("Autoassist removed.\n\r",ch);
      REMOVE_BIT(ch->act_bits,PLR_AUTOASSIST);
    }
  else
    {
      send_to_char("You will now assist when needed.\n\r",ch);
      SET_BIT(ch->act_bits,PLR_AUTOASSIST);
    }
}

void do_autoexit(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act_bits,PLR_AUTOEXIT))
    {
      send_to_char("Exits will no longer be displayed.\n\r",ch);
      REMOVE_BIT(ch->act_bits,PLR_AUTOEXIT);
    }
  else
    {
      send_to_char("Exits will now be displayed.\n\r",ch);
      SET_BIT(ch->act_bits,PLR_AUTOEXIT);
    }
}

void do_autogold(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act_bits,PLR_AUTOGOLD))
    {
      send_to_char("Autogold removed.\n\r",ch);
      REMOVE_BIT(ch->act_bits,PLR_AUTOGOLD);
    }
  else
    {
      send_to_char("Automatic gold looting set.\n\r",ch);
      SET_BIT(ch->act_bits,PLR_AUTOGOLD);
    }
}
void do_pueblo(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act_bits,PLR_PUEBLO))
    {
      send_to_char("Pueblo enhancements disabled.\n\r",ch);
      REMOVE_BIT(ch->act_bits,PLR_PUEBLO);
    }
  else
    {
      send_to_char("Pueblo enhancements enabled.\n\r",ch);
      SET_BIT(ch->act_bits,PLR_PUEBLO);
    }
}

void do_autoloot(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act_bits,PLR_AUTOLOOT))
    {
      send_to_char("Autolooting removed.\n\r",ch);
      REMOVE_BIT(ch->act_bits,PLR_AUTOLOOT);
    }
  else
    {
      send_to_char("Automatic corpse looting set.\n\r",ch);
      SET_BIT(ch->act_bits,PLR_AUTOLOOT);
    }
}

void do_autosac(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act_bits,PLR_AUTOSAC))
    {
      send_to_char("Autosacrificing removed.\n\r",ch);
      REMOVE_BIT(ch->act_bits,PLR_AUTOSAC);
    }
  else
    {
      send_to_char("Automatic corpse sacrificing set.\n\r",ch);
      SET_BIT(ch->act_bits,PLR_AUTOSAC);
    }
}

void do_autosplit(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act_bits,PLR_AUTOSPLIT))
    {
      send_to_char("Autosplitting removed.\n\r",ch);
      REMOVE_BIT(ch->act_bits,PLR_AUTOSPLIT);
    }
  else
    {
      send_to_char("Automatic gold splitting set.\n\r",ch);
      SET_BIT(ch->act_bits,PLR_AUTOSPLIT);
    }
}

void do_bat(CHAR_DATA *ch, char *argument)
{
  (check_social(ch,"bat"));
  return;
}

void do_battlebrief(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act_bits,PLR_BATTLEBRIEF))
    {
      send_to_char("Brief combat removed.\n\r",ch);
      REMOVE_BIT(ch->act_bits,PLR_BATTLEBRIEF);
    }
  else
    {
      send_to_char("Brief combat set.\n\r",ch);
      SET_BIT(ch->act_bits,PLR_BATTLEBRIEF);
    }
}

void do_brief(CHAR_DATA *ch, char *argument)
{
  if (IS_SET(ch->comm,COMM_BRIEF))
    {
      send_to_char("Full descriptions activated.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_BRIEF);
    }
  else
    {
      send_to_char("Short descriptions activated.\n\r",ch);
      SET_BIT(ch->comm,COMM_BRIEF);
    }
}

void do_compact(CHAR_DATA *ch, char *argument)
{
  if (IS_SET(ch->comm,COMM_COMPACT))
    {
      send_to_char("Compact mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMPACT);
    }
  else
    {
      send_to_char("Compact mode set.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMPACT);
    }
}

void do_show(CHAR_DATA *ch, char *argument)
{
  if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
    {
      send_to_char("Affects will no longer be shown in score.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_SHOW_AFFECTS);
    }
  else
    {
      send_to_char("Affects will now be shown in score.\n\r",ch);
      SET_BIT(ch->comm,COMM_SHOW_AFFECTS);
    }
}


void do_suspendlevellin( CHAR_DATA* ch, char* argument )
{
  send_to_char( "If you want to suspend your levelling, you'll have to type it out.\n\r", ch );
}


void do_suspendlevelling( CHAR_DATA* ch, char* argument )
{
  if( IS_NPC( ch ) )
    {
      send_to_char( "You can't advance.  What's the point?\n\r", ch );
      return;
    }

  if( IS_SET( ch->act_bits, PLR_NEWBIE ) )
    {
      send_to_char( "Are you sure that's a good idea?\n\r", ch );
      return;
    }

  if( IS_SET( ch->act_bits, PLR_SUSPEND_LEVEL ) )
    {
      REMOVE_BIT( ch->act_bits, PLR_SUSPEND_LEVEL );
      send_to_char( "You are ready to advance again.\n\r", ch );
    }
  else
    {
      SET_BIT( ch->act_bits, PLR_SUSPEND_LEVEL );
      send_to_char( "You have postponed your advancement.\n\r", ch );
    }
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH] = "";
 
  if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_PROMPT))
   	{
	  send_to_char("You will no longer see prompts.\n\r",ch);
	  REMOVE_BIT(ch->comm,COMM_PROMPT);
    	}
      else
    	{
	  send_to_char("You will now see prompts.\n\r",ch);
	  SET_BIT(ch->comm,COMM_PROMPT);
    	}
      return;
    }
 
  if( !str_cmp( argument, "old" ) )
    strcpy( buf, "<{y%h{xhp {c%m{xm {r%v{xmv>%c " );
  else if ( !str_cmp( argument, "default" ) || !str_cmp( argument, "all" ) )
    strcpy( buf, default_prompt );
  else
    {
      if( strlen( argument ) >= ( MAX_STRING_LENGTH - 1 ) )
	{
	  send_to_char( "Prompt too long.\n\r", ch );
	  return;
	}

      strcpy( buf, argument );
      smash_tilde( buf );

      if( str_suffix( "%c", buf ) )
	strcat( buf, " " );
    }
 
  free_string( ch->prompt );
  ch->prompt = str_dup( buf );
  sprintf(buf,"Prompt set to %s\n\r",ch->prompt );
  send_to_char(buf,ch);
  return;
}

void do_combine(CHAR_DATA *ch, char *argument)
{
  if (IS_SET(ch->comm,COMM_COMBINE))
    {
      send_to_char("Long inventory selected.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMBINE);
    }
  else
    {
      send_to_char("Combined inventory selected.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMBINE);
    }
}


void do_nofollow(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (IS_AFFECTED(ch,AFF_CHARM)){
    send_to_char("You couldn't bear to leave your leader.\n\r",ch);
    return;
  }
 
  if (IS_SET(ch->act_bits,PLR_NOFOLLOW))
    {
      send_to_char("You now accept followers.\n\r",ch);
      REMOVE_BIT(ch->act_bits,PLR_NOFOLLOW);
    }
  else
    {
      send_to_char("You no longer accept followers.\n\r",ch);
      SET_BIT(ch->act_bits,PLR_NOFOLLOW);
      nuke_pets(ch);
      die_follower( ch );
    }
}

void do_nosummon(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    {
      if (IS_SET(ch->imm_flags,IMM_SUMMON))
	{
	  send_to_char("You are no longer immune to summon.\n\r",ch);
	  REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
	}
      else
	{
	  send_to_char("You are now immune to summoning.\n\r",ch);
	  SET_BIT(ch->imm_flags,IMM_SUMMON);
	}
    }
  else
    {
      if (IS_SET(ch->act_bits,PLR_NOSUMMON))
	{
	  send_to_char("You are no longer immune to summon.\n\r",ch);
	  REMOVE_BIT(ch->act_bits,PLR_NOSUMMON);
	}
      else
	{
	  send_to_char("You are now immune to summoning.\n\r",ch);
	  SET_BIT(ch->act_bits,PLR_NOSUMMON);
	}
    }
}

void do_newbie( CHAR_DATA* ch, char* argument )
{
  if( IS_NPC( ch ) )
    return;

  if( IS_SET( ch->act_bits, PLR_NEWBIE ) )
    {
      REMOVE_BIT( ch->act_bits, PLR_NEWBIE );

      if( ch->level >= 15 )
	{
	  send_to_char( "Newbie status removed permanently.\n\r", ch );
	}
      else
	{
	  send_to_char( "Newbie status removed.\n\r", ch );
	}
    }
  else
    {
      if( IS_SET( ch->act_bits, PLR_SUSPEND_LEVEL ) )
	{
	  send_to_char( "You're not a newbie if you've suspended your levelling.\n\r", ch );
	}
      else if( ch->level >= 15 )
	{
	  send_to_char( "Newbie status cannot be regained after level 15 without an immortal's approval.\n\r", ch );
	}
      else
	{
	  SET_BIT( ch->act_bits, PLR_NEWBIE );
	  send_to_char( "Newbie status set.\n\r", ch );
	}
    }
}

/*
  void do_newbie(CHAR_DATA *ch, char *argument)
  {
  if (IS_NPC(ch))
  return;
 
  if (IS_SET(ch->act_bits,PLR_NEWBIE) && (ch->level >= 15))
  {
  send_to_char("Newbie status removed permanently.\n\r",ch);
  REMOVE_BIT(ch->act_bits,PLR_NEWBIE);
  }  
  else if (IS_SET(ch->act_bits,PLR_NEWBIE) && (ch->level <= 15))
  {
  send_to_char("Newbie status removed.\n\r",ch);
  REMOVE_BIT(ch->act_bits,PLR_NEWBIE);
  }
  else if (ch->level >= 15)
  {
  send_to_char("Newbie status cannot be regained after level 15 without an immortals approval\n\r",ch);
  return;
  }
  else if (ch->level <= 15)
  {
  send_to_char("Newbie status set.\n\r",ch);
  SET_BIT(ch->act_bits,PLR_NEWBIE);
  }
  }
*/

void do_drmode(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;
 
  if (IS_DRMODE(ch))
    {
      send_to_char("Going back to stock mode.  How sad.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_DRMODE);
    }  

  else
    {
      send_to_char("Dark Risings mode enabled!\n\r",ch);
      SET_BIT(ch->comm,COMM_DRMODE);
    }
}
int getlf( char *string )
{
  int count = 0;
  char* ptr;

  for ( ptr = string; *ptr != '\0'; ptr++ )
    {
      if ( *ptr == '\n' )
	count++;
    }
  return count;
}
int getcr( char *string )
{
  int count = 0;
  char* ptr;

  for ( ptr = string; *ptr != '\0'; ptr++ )
    {
      if ( *ptr == '\r' )
	count++;
    }
  return count;
}

bool check_room_desc( CHAR_DATA *ch, char *argument )
{
  char sentence[MAX_STRING_LENGTH], 
    word[MAX_STRING_LENGTH],
    buf[MAX_STRING_LENGTH];
  char *pRestSentence, *pRestWord;

  if ( ! ch->in_room )
    return FALSE;

  pRestSentence = ch->in_room->description;
  do
    {
      pRestSentence = one_sentence( pRestSentence, sentence );
      pRestWord = sentence;
      do
        {
	  pRestWord = one_word( pRestWord, word );
	  if ( !str_cmp( argument, word ) )
            {
	      if ( global_config_lookspam )
                {
		  sprintf(buf, "%s looked at %s in room %d",
			  ch->name, argument, ch->in_room->vnum );
		  bug(buf,0);
                }
	      strcat(sentence, "\n\r");
	      send_to_char( sentence, ch );
	      return TRUE;
            }
        } while ( !IS_NULLSTR(pRestWord) ); 
    } while ( !IS_NULLSTR(pRestSentence) );

  return FALSE;
}
 
void do_look( CHAR_DATA *ch, char *argument )
{
  char buf  [MAX_STRING_LENGTH];
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  EXIT_DATA *pexit;
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *to_room, *from_room;
  OBJ_DATA *obj;
  char *pdesc;
  int door;
  int number,count;

  if ( ch->desc == NULL )
    return;

  if ( ch->position < POS_SLEEPING )
    {
      send_to_char( "You can't see anything but stars!\n\r", ch );
      return;
    }

  if ( ch->position == POS_SLEEPING )
    {
      send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
      return;
    }

  if ( !check_blind( ch ) )
    return;

  if ( !IS_NPC(ch)
       &&   !IS_SET(ch->act_bits, PLR_HOLYLIGHT)
       &&   room_is_dark( ch->in_room ) )
    {
      send_to_char( "It is pitch black ... \n\r", ch );
      show_char_to_char( ch->in_room->people, ch );
      return;
    }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  number = number_argument(arg1,arg3);
  count = 0;

  if ( !str_cmp( arg1, "sky" ) )
    {
      if ( (pdesc = get_extra_descr(ch, arg1, ch->in_room->extra_descr, NULL)) != NULL)
	{
	  send_to_char(pdesc,ch);
	}
      else if ( !IS_OUTSIDE(ch) )
	{
	  send_to_char( "You can't see the sky indoors.\n\r", ch );
	}
      else
	{
	  look_sky(ch);
	}
      return;
    }

  if (arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
      /* 'look' or 'look auto' 
	 if ((!IS_NULLSTR(ch->in_room->p_image)) 
	 && (IS_SET(ch->act_bits,PLR_PUEBLO))) 
	 {
	 sprintf(pueblo_buf,"<img src=\"%s%s\">",
	 PUEBLO_DIR,ch->in_room->p_image);
	 send_to_char("</xch_mudtext><img xch_mode=html>",ch);
	 send_to_char(pueblo_buf,ch);
	 send_to_char("<br><img xch_mode=text>",ch);
	 }
      */

      sprintf(buf,"{c%s{x",ch->in_room->name);
      send_to_char( buf, ch );

      if ( (IS_IMMORTAL(ch) && (IS_NPC(ch) || IS_SET(ch->act_bits,PLR_HOLYLIGHT)))
	   ||   IS_BUILDER(ch, ch->in_room->area) )
	{
	  sprintf(buf," [Room %d]",ch->in_room->vnum);
	  send_to_char(buf,ch);
	}

      send_to_char( "\n\r", ch );

      if ( arg1[0] == '\0'
	   || ( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) ) )
	{
	  send_to_char( "  ",ch);
	  send_to_char( ch->in_room->description, ch );
	}

      if ( !IS_NPC(ch) && IS_SET(ch->act_bits, PLR_AUTOEXIT) )
	{
	  send_to_char("\n\r",ch);
	  if (IS_SET(ch->comm, COMM_COMPACT))
	    do_exits( ch, "auto" );
	  else
	    do_exits( ch, "noauto" );
	}

      if ( IS_NPC(ch) && ch->desc != NULL) /* Switched mobs get autoexit */
	do_exits ( ch, "noauto");
      show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
      show_char_to_char( ch->in_room->people,   ch );
      return;
    }

  if ( !str_cmp( arg1, "i" ) || !str_cmp(arg1, "in")  || !str_cmp(arg1,"on"))
    {
      /* 'look in' */
      if ( arg2[0] == '\0' )
	{
	  send_to_char( "Look in what?\n\r", ch );
	  return;
	}

      if ( ( obj = get_obj_here( ch, arg2, TRUE ) ) == NULL )
        {
	  send_to_char( "You do not see that here.\n\r", ch );
	  return;
	}

      switch ( obj->item_type )
	{
	default:
	  send_to_char( "That is not a container.\n\r", ch );
	  break;

	case ITEM_DRINK_CON:
	  if ( obj->value[1] <= 0 )
	    {
	      send_to_char( "It is empty.\n\r", ch );
	      break;
	    }
          if (obj->value[2] < MAX_LIQUID )
	    {
	      sprintf( buf, "It's %sfilled with a %s liquid.\n\r",
		       obj->value[1] <     obj->value[0] / 4
		       ? "less than half-" :
		       obj->value[1] < 3 * obj->value[0] / 4
		       ? "about half-"     : "more than half-",
		       liq_table[obj->value[2]].liq_color);
	    }

	  send_to_char( buf, ch );
	  break;
	case ITEM_FOUNTAIN:
          if (obj->value[2] < MAX_LIQUID )
	    {
              sprintf( buf, "A %s liquid gushes forth from it.\n\r",
		       liq_table[obj->value[2]].liq_color);
              send_to_char( buf, ch );
	    }
	  break;


	case ITEM_CONTAINER:
        case ITEM_BOOK:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	case ITEM_QUIVER:
	  if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
	      send_to_char( "It is closed.\n\r", ch );
	      break;
	    }

	  act( "$p holds:", ch, obj, NULL, TO_CHAR );
	  show_list_to_char( obj->contains, ch, TRUE, TRUE );
	  break;

	  /* Kyuss -- added look for gates,portals */
	case ITEM_PORTAL:
          if ( IS_SET(obj->value[1],EX_CLOSED) )  
	    {
	      send_to_char( "It is closed.\n\r", ch );
	      break;
	    }
	  else if( IS_SET( obj->value[2], GATE_FATAL ) 
		   ||  IS_SET( obj->value[2],   GATE_DRAINER) )
	    {
	      send_to_char( "You see yourself staring back at you.  This gate seems very unstable.\n\r", ch );
	    }
	  else if( IS_SET( obj->value[ 2 ], GATE_DEAD_END ) )
	    {
	      send_to_char( "This gate seems to go nowhere.\n\r", ch );
	    }
	  else if( IS_SET( obj->value[ 2 ], GATE_RANDOM ) )
	    {
	      send_to_char( "A panorama of rooms flashes before you.  This gate seems slightly unstable.\n\r", ch );
	    }
	  else if( ( to_room = get_room_index(obj->value[3])) == NULL ) 
	    {
	      send_to_char("The magical opening is very unstable.\n\r",ch);
	      send_to_char("You are unable to see what lies behind.\n\r",ch);
	    }
	  else
	    {
	      from_room = ch->in_room;
	      ch->in_room = to_room;
	      do_look(ch,"auto");
	      ch->in_room = from_room;
	    }

	  break;
	    
	}
      return;
    }

  if ( ( victim = get_char_room( ch, arg1, TRUE ) ) != NULL )
    {
      show_char_to_char_1( victim, ch );
      return;
    }

  for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
      if ( can_see_obj( ch, obj ) )
	{
	  pdesc = get_extra_descr( ch, arg3, obj->extra_descr, obj );
	  if ( pdesc != NULL )
	    if (++count == number)
	      {
		send_to_char( pdesc, ch );
		return;
	      }

	  pdesc = get_extra_descr(ch, arg3, obj->pIndexData->extra_descr, obj);
	  if ( pdesc != NULL )
	    if (++count == number)
	      {
		send_to_char( pdesc, ch );
		return;
	      }

	  if ( is_abbrev( arg3, obj->name ) )
	    if (++count == number)
	      {
		/*		if ((!IS_NULLSTR(obj->pIndexData->o_image)) 
				&& (IS_SET(ch->act_bits,PLR_PUEBLO))) 
				{
				send_to_char("</xch_mudtext><img xch_mode=html>",ch);
				sprintf(pueblo_buf,"<img src=\"%s%s\">",
				PUEBLO_DIR,obj->pIndexData->o_image);
				send_to_char(pueblo_buf,ch);
				send_to_char("<br><img xch_mode=text>",ch);
				} */
		send_to_char( obj->description, ch );
		send_to_char("\n\r",ch);

		return;
	      }
	}
    }


  for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
      if ( can_see_obj( ch, obj ) )
	{  /* player can see object */
	  pdesc = get_extra_descr( ch, arg3, obj->extra_descr, obj );
	  if ( pdesc != NULL )
	    {
	      if (++count == number)
	    	{
		  send_to_char( pdesc, ch );
		  return;
	    	}
	      else continue;
	    }

	  pdesc = get_extra_descr(ch, arg3, obj->pIndexData->extra_descr, obj);
	  if ( pdesc != NULL )
	    {
	      if (++count == number)
 	    	{	
		  send_to_char( pdesc, ch );
		  return;
	     	}
	      else continue;
	    }

	  if ( is_abbrev( arg3, obj->name ) )
	    if (++count == number)
	      {
		/*if ((!IS_NULLSTR(obj->pIndexData->o_image))
		  && (IS_SET(ch->act_bits,PLR_PUEBLO))) 
		  {
		  send_to_char("</xch_mudtext><img xch_mode=html>",ch);
		  sprintf(pueblo_buf,"<img src=\"%s%s\">",
		  PUEBLO_DIR,obj->pIndexData->o_image);
		  send_to_char(pueblo_buf,ch);
		  send_to_char("<br><img xch_mode=text>",ch);
		  } */
		send_to_char( obj->description, ch );
		send_to_char( "\n\r",ch);

		if( strlen( obj->donor ) != 0 )
		  {
		    printf_to_char( ch, "A little tag says 'Originally donated by %s'\n\r", obj->donor );
		  }

		return;
	      }
	}
    }


  pdesc = get_extra_descr( ch, arg3, ch->in_room->extra_descr, NULL );
  if (pdesc != NULL)
    {
      if (++count == number)
	{
	  send_to_char(pdesc,ch);
	  return;
	}
    }
    
  if (count > 0 && count != number)
    {
      if (count == 1)
	sprintf(buf,"You only see one %s here.\n\r",arg3);
      else
	sprintf(buf,"You only see %d of those here.\n\r",count);
    	
      send_to_char(buf,ch);
      return;
    }

  if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door = 0;
  else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door = 1;
  else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door = 2;
  else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door = 3;
  else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door = 4;
  else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door = 5;
  else
    {
      if ( is_extra_descr_abbrev( arg3,ch->in_room->extra_descr ) )
	send_to_char( "Look more closely at what you want to see.\n\r",ch);
      else
	if ( !check_room_desc( ch, arg3 ) )
	  send_to_char( "You do not see that here.\n\r", ch );
      return;
    }

  /* 'look direction' */
  if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
      send_to_char( "Nothing special there.\n\r", ch );
      return;
    }

  if ( pexit->description != NULL && pexit->description[0] != '\0' )
    send_to_char( pexit->description, ch );
  else
    send_to_char( "Nothing special there.\n\r", ch );

  if ( pexit->keyword    != NULL
       &&   pexit->keyword[0] != '\0'
       &&   pexit->keyword[0] != ' ' )
    {
      if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	  act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	}
      else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
	{
	  act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
	}
    }

  return;
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA *ch, char *argument )
{
  do_look(ch,argument);
}

void do_examine( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
      send_to_char( "Examine what?\n\r", ch );
      return;
    }

  do_look( ch, arg );

  if ( ( obj = get_obj_here( ch, arg, TRUE ) ) != NULL )
    {
      switch ( obj->item_type )
	{
	default:
	  break;
	
	case ITEM_JUKEBOX:
	  do_play(ch,"list");
	  break;

	case ITEM_MONEY:
	  if (obj->value[0] == 0)
	    {
	      if (obj->value[1] == 0)
		sprintf(buf,"Odd...there's no coins in the pile.\n\r");
	      else if (obj->value[1] == 1)
		sprintf(buf,"Wow. One gold coin.\n\r");
	      else
		sprintf(buf,"There are %d gold coins in the pile.\n\r",
			obj->value[1]);
	    }
	  else if (obj->value[1] == 0)
	    {
	      if (obj->value[0] == 1)
		sprintf(buf,"Wow. One silver coin.\n\r");
	      else
		sprintf(buf,"There are %d silver coins in the pile.\n\r",
			obj->value[0]);
	    }
	  else
	    sprintf(buf,
		    "There are %d gold and %d silver coins in the pile.\n\r",
		    obj->value[1],obj->value[0]);
	  send_to_char(buf,ch);
	  break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	case ITEM_QUIVER:
	case ITEM_PORTAL:
        case ITEM_BOOK:
	  sprintf(buf,"in %s",argument);
	  do_look( ch, buf );
	}
    }

  return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
  extern char * const dir_name[];
  char buf[MAX_STRING_LENGTH];
  EXIT_DATA *pexit;
  bool found;
  bool fAuto;
  int door;

  fAuto  = !str_cmp( argument, "auto" );

  if ( !check_blind( ch ) )
    return;

  if (fAuto)
    sprintf(buf,"{c[Exits:");
  else if (IS_IMMORTAL(ch))
    sprintf(buf,"Obvious exits from room %d:\n\r",ch->in_room->vnum);
  else
    sprintf(buf,"Obvious exits:\n\r");

  found = FALSE;
  for ( door = 0; door <= 5; door++ )
    {
      if ( ( pexit = ch->in_room->exit[door] ) != NULL
	   &&   pexit->u1.to_room != NULL
	   &&   can_see_room(ch,pexit->u1.to_room) 
	   &&   !IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	  found = TRUE;
	  if ( fAuto )
	    {
	      strcat( buf, " " );
	      strcat( buf, dir_name[door] );
	    }
	  else
	    {
	      sprintf( buf + strlen(buf), "%-5s - {c%s{x",
		       capitalize( dir_name[door] ),
		       room_is_dark( pexit->u1.to_room )
		       ?  "Too dark to tell"
		       : pexit->u1.to_room->name
		       );
	      if (IS_IMMORTAL(ch))
		sprintf(buf + strlen(buf), 
			" (room %d)\n\r",pexit->u1.to_room->vnum);
	      else
		sprintf(buf + strlen(buf), "\n\r");
	    }
	}
    }

  if ( !found )
    strcat( buf, fAuto ? " none" : "None.\n\r" );

  if ( fAuto )
    strcat( buf, "]{x\n\r" );

  send_to_char( buf, ch );
  return;
}

void do_worth( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    {
      sprintf(buf,"You have %ld gold and %ld silver.\n\r",
	      ch->gold,ch->silver);
      send_to_char(buf,ch);
      return;
    }

  sprintf(buf, 
	  "You have %ld gold, %ld silver, and %d experience (%d exp to level).\n\r",
	  ch->gold, ch->silver,ch->exp,
	  (ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);

  send_to_char(buf,ch);
  sprintf(buf, 
	  "You have %ld gold and  %ld silver in the bank.\n\r",
	  ch->pcdata->gold_bank, ch->pcdata->silver_bank);

  send_to_char(buf,ch);

  if ( ch->pcdata->proceeds > 0 )
    {
      sprintf(buf, 
	      "You have made %d gold and %d silver in royalties to date.\n\r", 
	      ch->pcdata->proceeds / 100, ch->pcdata->proceeds % 100 );
      send_to_char(buf, ch );
    }

  return;
}

void do_showhp( CHAR_DATA *ch, char *argument )
{
  char buf[32];
  sprintf( buf, "%d\n\r", ch->hit );
  send_to_char( buf, ch );
  return;
}


void do_score( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char color1[3] = "{Y",
    color2[3] = "{G", /* For cleanse meter */
    color3[3] = "{C",
    color4[3] = "{W";
  int i;
  bool showaff = FALSE;

  if( IS_NPC( ch ) || IS_SET( ch->comm, COMM_SHOW_AFFECTS ) )
    showaff = TRUE;


  send_to_char("{Y+{B--------------------------------------------------------------------{Y+{x\n\r",
	       ch );

  sprintf( buf,
	   "{B {GName:{x %13s  {GTitle:{x %41s\n\r",
	   ch->name,
	   IS_NPC(ch) ? "" : ch->pcdata->title);
  send_to_char( buf, ch );


  send_to_char("{Y+{B--------------------------------------------------------------------{Y+{x\n\r",
	       ch );

  sprintf(buf, "{B|{Ghp:{Y%6d/%-5d          {Gmana:{B%6d/%-5d         {Gmove:{g%6d/%-5d{B|{x\n\r",
	  ch->hit, ( showaff ? ch->max_hit : ch->pcdata->perm_hit ),
	  ch->mana,( showaff ? ch->max_mana : ch->pcdata->perm_mana ),
	  ch->move,( showaff ? ch->max_move : ch->pcdata->perm_move ) );
  send_to_char( buf, ch );


  send_to_char("{Y+{B--------------------------------------------------------------------{Y+{x\n\r",
	       ch );
   
  sprintf(buf, "{B|{GStr:%9d{x({M%2d{x)        {GRace:%13s        exp:%13d{B|{x\n\r",
	  ch->perm_stat[STAT_STR],
	  get_curr_stat(ch,STAT_STR),
	  race_table[ch->race].name,
	  ch->exp);
  send_to_char(buf,ch);
	
  /* RT shows exp to level */
  if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
    {
      sprintf( buf,
	       "{B|{GInt:%9d{x({M%2d{x){G        sex:%14s        tnl:%13d{B|{x\n\r",
	       ch->perm_stat[STAT_INT],
	       get_curr_stat(ch,STAT_INT),
	       ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
	       ((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp));
      send_to_char( buf, ch );
    }
  else
    {
      sprintf( buf,
	       "{B|{GInt:%9d{x({M%2d{x){G        sex:%14s        tnl:%13d{B|{x\n\r",
	       ch->perm_stat[STAT_INT],
	       get_curr_stat(ch,STAT_INT),
	       ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female", 0 );
      send_to_char( buf, ch ); 
    }

  sprintf( buf,
	   "{B|{GWis:%9d{x({M%2d{x){G        Class:%12s        items:%5d/%-5d{B|{x\n\r",
	   ch->perm_stat[STAT_WIS],
	   get_curr_stat(ch,STAT_WIS),
	   IS_NPC(ch) ? "mobile" : class_table[ch->class].name,
	   ch->carry_number, can_carry_n(ch) );
  send_to_char( buf, ch );

  sprintf( buf,
	   "{B|{GDex:%9d{x({M%2d{x){G        Level:%12d        Trains:%10d{B|{x\n\r",
	   ch->perm_stat[STAT_DEX],
	   get_curr_stat(ch,STAT_DEX),
	   ch->level, ch->train );
  send_to_char( buf, ch );

  sprintf( buf,
	   "{B|{GCon:%9d{x({M%2d{x){G        Wimpy:%12d        Practices:%7d{B|{x\n\r",
	   ch->perm_stat[STAT_CON],
	   get_curr_stat(ch,STAT_CON), 
	   ch->wimpy, ch->practice );
  send_to_char( buf, ch );


  send_to_char("{Y+{B--------------------------------------------------------------------{Y+{x\n\r",
	       ch );

  sprintf( buf,
	   "{B|{Y%11ld {Ggold.{c%13ld {Gsilver.   weight {W%7ld/%-7d {Glbs.{B|{x\n\r",
	   ch->gold, 
	   ch->silver,
	   get_carry_weight(ch) / 10, can_carry_w(ch) /10 );
  send_to_char( buf, ch );


  send_to_char("{Y+{B--------------------------------------------------------------------{Y+{x\n\r",
	       ch );

  sprintf( buf, "{B|{GHitroll:{W%5d                {GDamroll:{W%5d               {GSaves:{W%5d{B|{x\n\r",
	   GET_HITROLL(ch), GET_DAMROLL(ch), ch->saving_throw );
  send_to_char( buf, ch );


  send_to_char("{Y+{B--------------------------------------------------------------------{Y+{x\n\r",
	       ch );

  sprintf( buf,"{B|{GArmor:   {rpierce:{W%5d     {rbash:{W%5d     {rslash:{W%5d     {rmagic:{W%5d{B|{c\n\r",
	   GET_AC(ch,AC_PIERCE), GET_AC(ch,AC_BASH), 
	   GET_AC(ch,AC_SLASH), GET_AC(ch,AC_EXOTIC));
  send_to_char(buf,ch);

  if (!IS_NPC(ch))
    {

      send_to_char("{Y+{B--------------------------------------------------------------------{Y+{x\n\r",
		   ch );

      if (ch->race == race_lookup("seraph")) 
	{
	  strcpy(buf, "{B|                               {cSeraph{B                               |{x\n\r");
	}
	  
      else if (!strcmp(god_table[ch->pcdata->god].name,"none"))
	{
	  strcpy(buf, "{B|                        {WYou worship no deity{B                        |{x\n\r");
	}
	  
      else 
	{
	  sprintf(buf, "{B|                    {WYou worship the deity {M%s                     {B|{x\n\r",
		  god_table[ch->pcdata->god].name);
	}
      send_to_char(buf,ch);

      send_to_char("{Y+{B--------------------------------------------------------------------{Y+{x\n\r",
		   ch );
    }

  for (i = 0; i < 4; i++)
    {
      char * temp;

      switch(i)
	{
	case(AC_PIERCE):	temp = "piercing";	break;
	case(AC_BASH):	temp = "bashing";	break;
	case(AC_SLASH):	temp = "slashing";	break;
	case(AC_EXOTIC):	temp = "magic";		break;
	default:		temp = "error";		break;
	}
	
      send_to_char("{B|{xYou are ", ch);

      if      (GET_AC(ch,i) >=  101 ) 
	sprintf(buf,"hopelessly vulnerable to %-8s                           {B|{x\n\r",temp);
      else if (GET_AC(ch,i) >= 80) 
	sprintf(buf,"defenseless against %-8s                                {B|{x\n\r",temp);
      else if (GET_AC(ch,i) >= 60)
	sprintf(buf,"barely protected from %-8s                              {B|{x\n\r",temp);
      else if (GET_AC(ch,i) >= 40)
	sprintf(buf,"slightly armored against %-8s                           {B|{x\n\r",temp);
      else if (GET_AC(ch,i) >= 20)
	sprintf(buf,"somewhat armored against %-8s                           {B|{x\n\r",temp);
      else if (GET_AC(ch,i) >= 0)
	sprintf(buf,"armored against %-8s                                    {B|{x\n\r",temp);
      else if (GET_AC(ch,i) >= -20)
	sprintf(buf,"well-armored against %-8s                               {B|{x\n\r",temp);
      else if (GET_AC(ch,i) >= -40)
	sprintf(buf,"very well-armored against %-8s                          {B|{x\n\r",temp);
      else if (GET_AC(ch,i) >= -60)
	sprintf(buf,"heavily armored against %-8s                            {B|{x\n\r",temp);
      else if (GET_AC(ch,i) >= -80)
	sprintf(buf,"superbly armored against %-8s                           {B|{x\n\r",temp);
      else if (GET_AC(ch,i) >= -100)
	sprintf(buf,"almost invulnerable to %-8s                             {B|{x\n\r",temp);
      else
	sprintf(buf,"divinely armored against %-8s                           {B|{x\n\r",temp);

      send_to_char(buf,ch);
    }


  /* RT wizinvis and holy light */
  if ( IS_IMMORTAL(ch))
    {
      send_to_char("|Holy Light: ",ch);
      if (IS_SET(ch->act_bits,PLR_HOLYLIGHT))
        send_to_char("on                                                      |\n\r",ch);
      else
        send_to_char("off                                                     |\n\r",ch);
 
      if (ch->invis_level)
	{
	  sprintf(buf, "|Invisible: level %2d                                                 |\n\r",
		  ch->invis_level);
	  send_to_char(buf,ch);
	}

      if (ch->incog_level)
	{
	  sprintf(buf, "|Incognito: level %2d                                                 |\n\r",
		  ch->incog_level);
	  send_to_char(buf,ch);
	}
      if (ch->rinvis_level)
	{
	  sprintf(buf, "|Race Wizi: level %2d                                                 |\n\r",
		  ch->rinvis_level);
	  send_to_char(buf, ch );
	}
    }

  if ( get_trust( ch ) != ch->level )
    {
      sprintf( buf, "|You are trusted at level %2d.                                       |\n\r",
	       get_trust( ch ) );
      send_to_char( buf, ch );
    }


  send_to_char("{Y+{B--------------------------------------------------------------------{Y+{x\n\r",
	       ch );

  if (IS_SET(ch->shiftbits,PERM_VAMP) ||
      IS_SET(ch->shiftbits,TEMP_VAMP))
    {
      sprintf(buf,"{B|{GPatriarch:{r%14s                              {Gbloodpool:{r%4d{B|{x\n\r",
	      ch->patriarch,
	      ch->blood);
      send_to_char(buf,ch);

      send_to_char("{Y+{B--------------------------------------------------------------------{Y+{x\n\r",
		   ch );
      send_to_char("\n\r",ch);
      do_bloodpool(ch,"");
    }

  if (IS_SET(ch->shiftbits,SHIFT_POTENTIAL)&&
      (ch->level > 29))
    {
      if ( !IS_NPC( ch ) )
	{
	  if ( ch->pcdata->cleansings > 2 )
	    {
	      strcpy(color1,"{D");
	      strcpy(color2,"{D");
	      strcpy(color3,"{D");
	      strcpy(color4,"{D");
	    }
	  else
	    { 
	      if ( ch->pcdata->cleansings > 0 )
		{
		  strcpy(color1,"{R"); 
		  strcpy(color2,"{R");
		}
	      if ( ch->pcdata->cleansings > 1 )
                strcpy(color3,"{R");
	    }
	}

      sprintf(buf, "{B|                       %s<%s--%s==%sWERECREATURE%s==%s--%s>                       {B|\n\r", 
	      color1,color2,color3,color4,color3,color2,color1);
      send_to_char(buf,ch);
      send_to_char("{Y+{B--------------------------------------------------------------------{Y+{x\n\r",
		   ch );
    }
}

void do_affects(CHAR_DATA *ch, char *argument )
{
  AFFECT_DATA *paf, *paf_last = NULL;
  char buf[MAX_STRING_LENGTH];
    
  if ( ch->affected != NULL )
    {
      send_to_char( "You are affected by the following spells:\n\r", ch );
      for ( paf = ch->affected; paf != NULL; paf = paf->next )
	{
	  if (paf_last != NULL && paf->type == paf_last->type)
	    if (ch->level >= 0)
	      sprintf( buf, "                      ");
	    else
	      continue;
				
	  else if (paf->type == -4)
	    sprintf( buf, "Veil:  %-15s", "Veil wait" );
	  else if (paf->type == -1)
	    sprintf( buf, "Were:  %-15s", "Shifted");
	  else if (paf->type == -2)
	    sprintf( buf, "Were:  %-15s", "Shift wait");
	  else if (paf->type == -3)
	    sprintf( buf, "Vamp:  %-15s", IS_SET(ch->shiftbits,BAT_VAMP) ? "bat" : "mist" );
	  else if (paf->type == -20)
	    sprintf( buf, "Vamp:  %-15s","Torpor" );
	  else if (paf->type == -5 )
	    sprintf( buf, "Vamp:  %-15s", "Offering" );
	  else if (paf->type == -6 )
	    sprintf( buf, "Wrath: %-15s", "Transmogrified" );
	  else if (paf->type ==  gsn_fatigue)
	    sprintf( buf, "Godspeed: %-15s", "Fatigue" );
	  else
	    sprintf( buf, "Spell: %-15s", skill_table[paf->type].name );

	  send_to_char( buf, ch );

	  if ( ch->level >= 0)
	    {
	      sprintf( buf,": modifies %s by %d ", affect_loc_name( paf->location ), paf->modifier);
	      send_to_char( buf, ch );
		
	      if ( paf->duration == -1 ) 
		sprintf( buf, "permanently" );
					
	      else if ( paf->duration < 3 )
		sprintf( buf, "for {r%d hours{x", paf->duration );
					
	      else sprintf( buf, "for %d hours", paf->duration );
		
	      send_to_char( buf, ch );
		
	    }

	  send_to_char( "\n\r", ch );
	  paf_last = paf;
	}
    }
	
  else 
    send_to_char("You are not affected by any spells.\n\r",ch);
  return;
}



char *	const	day_name	[] =
  {
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
  };

char *	const	month_name	[] =
  {
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Furies", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
  };

void do_time( CHAR_DATA *ch, char *argument )
{
  extern char str_boot_time[];
  struct tm *ltime;
  char buf[MAX_STRING_LENGTH];
  char buf2[256];
  char *suf;
  int day;
  /*  struct tm* timezone_calc = NULL;
      time_t time_calc;
  */
  day     = time_info.day + 1;
  ltime = localtime( &current_time );

  if ( day > 4 && day <  20 ) suf = "th";
  else if ( day % 10 ==  1       ) suf = "st";
  else if ( day % 10 ==  2       ) suf = "nd";
  else if ( day % 10 ==  3       ) suf = "rd";
  else                             suf = "th";

  if (IS_DRMODE(ch))
    send_to_char(BIG_BLUE_LINE,ch);

  strftime( buf2, 256, "It is %A, %B %d, and the game time is %I:%M %p.", ltime );

  sprintf( buf,
	   "It is %d o'clock %s, Day of %s, %d%s day of the Month of %s.\n\r",
	   (time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
	   time_info.hour >= 12 ? "pm" : "am",
	   day_name[day % 7],
	   day, suf,
	   month_name[time_info.month]);
  send_to_char(buf,ch);
  sprintf(buf,"\n\r%s\n\r\n\r"
	  "Dark Risings started up at %s\n\r",
	  buf2,
	  str_boot_time
	  );

  send_to_char( buf, ch );

  if (IS_DRMODE(ch))
    send_to_char(BIG_BLUE_LINE,ch);
  return;
}



void do_weather( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  static char * const sky_look[4] =
    {
      "cloudless",
      "cloudy",
      "rainy",
      "lit by flashes of lightning"
    };

  if ( !IS_OUTSIDE(ch) )
    {
      send_to_char( "You can't see the weather indoors.\n\r", ch );
      return;
    }

  sprintf( buf, "The sky is %s and %s.\n\r",
	   sky_look[weather_info.sky],
	   weather_info.change >= 0
	   ? "a warm southerly breeze blows"
	   : "a cold northern gust blows"
	   );
  send_to_char( buf, ch );
  return;
}

void do_help( CHAR_DATA *ch, char *argument )
{
  HELP_DATA *pHelp;
  BUFFER *output;
  bool found = FALSE, limited = FALSE;
  char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
  int level;

  output = new_buf();

  if ( argument[0] == '\0' )
    argument = "summary";

  /* this parts handles help a b so that it returns help 'a b' */
  argall[0] = '\0';
  while (argument[0] != '\0' )
    {
      argument = one_argument(argument,argone);
      if (argall[0] != '\0')
	strcat(argall," ");
      strcat(argall,argone);
    }

  for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
      limited = FALSE;
      level = pHelp->level;
      if ( level < 0 )
        {
	  if ( level == -1 )
	    level = 0;
	  else
	    level *= -1;
        }
      /*  	level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level; */

      if (level > get_trust( ch ) )
	continue;
      /*      Mobs can never see limited helpfiles - this is to prevent unruly
	      imms from viewing imp-only helpfiles by switching into mobs */
      if ( pHelp->limits > 0 && IS_NPC( ch ) )
	continue;

      /*      New helpfile limit flags
       *      ------------------------
       *      For every new LIMIT flag, you MUST add its functionality
       *      here.  It's a bit sloppy, but I can't think of a better way
       *      to do this without being over-complicated.
       */
      if ( pHelp->limits > 0 && get_trust( ch ) < LEVEL_ADMIN )
        {
	  limited = TRUE;
	  if ( ( IS_SET( pHelp->limits, LIMIT_COVENANCE ) 
		 && ch->guild == guild_lookup("covenance") )
	       || ( IS_SET( pHelp->limits, LIMIT_CODER )     
		    && ch->guild == guild_lookup("coder") )
	       || ( IS_SET( pHelp->limits, LIMIT_IMP )       
		    && ch->guild == guild_lookup("imp") )
	       || ( IS_SET( pHelp->limits, LIMIT_BUILDER )
		    && ch->guild == guild_lookup("builder") )
	       || ( IS_SET( pHelp->limits, LIMIT_GYPSY )     
		    && ch->guild == guild_lookup("gypsy")     )
	       || ( IS_SET( pHelp->limits, LIMIT_INFERNO )   
		    && ch->guild == guild_lookup("inferno")   )
	       || ( IS_SET( pHelp->limits, LIMIT_VERMILLION )
		    && ch->guild == guild_lookup("vermillion"))
	       || ( IS_SET( pHelp->limits, LIMIT_DAWNING )   
		    && ch->guild == guild_lookup("dawning")   )
	       || ( IS_SET( pHelp->limits, LIMIT_CRIMSON )   
		    && ch->guild == guild_lookup("crimson rain"))
	       || ( IS_SET( pHelp->limits, LIMIT_ARCAENUM )  
		    && ch->guild == guild_lookup("arcaenum")  )
	       || ( IS_SET( pHelp->limits, LIMIT_LOSER )     
		    && ch->guild == guild_lookup("loser")     )
	       || ( IS_SET( pHelp->limits, LIMIT_AEQUITAS )  
		    && ch->guild == guild_lookup("aequitas")  )
	       || ( IS_SET( pHelp->limits, LIMIT_COALITION )  
		    && ch->guild == guild_lookup("coalition")  )
	       || ( IS_SET( pHelp->limits, LIMIT_VAMPIRES )
		    && IS_VAMPIRE(ch) )
	       || ( IS_SET( pHelp->limits, LIMIT_PATRIARCHS ) 
		    && IS_SET(ch->shiftbits,PERM_VAMP)
		    && !str_cmp(ch->name, ch->patriarch) )
	       || ( IS_SET( pHelp->limits, LIMIT_WERECREATURES ) 
		    && IS_WERECREATURE( ch ) )
	       || ( IS_SET( pHelp->limits, LIMIT_BRAWLERS )   
		    && IS_BRAWLER( ch ) )
	       || ( IS_SET( pHelp->limits, LIMIT_IMMORTALS )  
		    && IS_IMMORTAL( ch ) )
	       || ( IS_SET( pHelp->limits, LIMIT_RAVNOS )
		    && IS_VAMPIRE(ch)
		    && !str_cmp(ch->patriarch, RAVNOS_PAT) )
	       || ( IS_SET( pHelp->limits, LIMIT_LASOMBRA )
		    && IS_VAMPIRE(ch)
		    && !str_cmp(ch->patriarch, LASOMBRA_PAT) )
	       || ( IS_SET( pHelp->limits, LIMIT_FLOCRIAN ) 
		    && !str_cmp(ch->name, "Flocrian") )
	       || ( IS_SET( pHelp->limits, LIMIT_SERAPHS ) 
		    && ch->race == race_lookup("seraph"))
               )
	    {
	      limited = FALSE;
	    }
        }

      if ( is_name( argall, pHelp->keyword ) && !limited )
	{
	  if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
	    {
	      add_buf(output,pHelp->keyword);
	      add_buf(output,"\n\r");
	    }

	  /*
	   * Strip leading '.' to allow initial blanks.
	   */
	  if ( pHelp->text[0] == '.' )
	    add_buf(output,pHelp->text+1);
	  else
	    add_buf(output,pHelp->text);
	  found = TRUE;
	  /* small hack :) */
	  if (ch->desc != NULL && ch->desc->connected != CON_PLAYING 
	      &&  		    ch->desc->connected != CON_GEN_GROUPS)
	    break;
	}
    }

  if (!found)
    send_to_char( "No help on that word.\n\r", ch );
  else
    page_to_char(buf_string(output),ch);
  free_buf(output);
}

/*
  consolidated identical functions from who/whois for deduplication
  and ease of extenstion/modification later
*/

char* whoLine( CHAR_DATA* ch, CHAR_DATA* looker )
{
  char IMMCLASSES[9][4] = { "IMP", "CRE", "SUP", "DEI", "GOD", "IMM", "DEM", "ANG", "AVA" };
  char BRAWLER[] = "{D[B]{x ";
  char QUEST[] = "(QUEST) ";
  char AFK[] = "[AFK] ";
  char VEILED[] = "({rVeiled{x) ";
  char WANTED[] = "(WANTED) ";
  char GHOST[] = "(GHOST) ";
  char UNAPPROVED[] = "(UA) ";
  char NEWBIE[] = "[{G+{x] ";
  char LEVEL_AR[] = "AR";
  char LEVEL_BR[] = "BR";
  char LEVEL_NB[] = "NB";
  char LEVEL_IR[] = "IR";
  bool showBrawler = FALSE;
 
  char* relativeLevel = NULL;
  const char* chClass = NULL;
  char* chLineItem;

  char chPretitle[ MAX_INPUT_LENGTH ] = "";
  char chIncog[ MAX_INPUT_LENGTH ] = "";
  char chWizi[ MAX_INPUT_LENGTH ] = "";
  char chClan[ MAX_INPUT_LENGTH ] = "";

  chLineItem = ( char* )malloc( sizeof( char ) * MAX_INPUT_LENGTH );
  if( chLineItem == NULL )
    return NULL;

  /* determine AR/IR/BR/NB/level */
  if( ch != looker )
    {
      if( IS_IMMORTAL( looker ) )
	{
	  relativeLevel = NULL;
	}
      else if( ch->level < 11 )
	{
	  relativeLevel = LEVEL_NB;
	}
      else if( abs( ch->level - looker->level ) < 7 || ( looker->level == 43 && ch->level == 50 ) )
	{
	  relativeLevel = LEVEL_IR;
	}
      else if( ch->level > looker->level )
	{
	  relativeLevel = LEVEL_AR;
	}
      else
	{
	  relativeLevel = LEVEL_BR;
	}
    }

  if( IS_IMMORTAL( ch ) )
    {
      chClass = IMMCLASSES[ 60 - ch->level ];
    }
  else
    {
      chClass = class_table[ ch->class ].who_name;
    }

  if( ch->incog_level >= LEVEL_HERO )
    sprintf( chIncog, "(Incog %d) ", ch->incog_level );

  if( ch->invis_level >= LEVEL_HERO )
    sprintf( chWizi, "(Wizi %d) ", ch->invis_level );

  if( !IS_NULLSTR( ch->pcdata->pretitle ) )
    {
      sprintf( chPretitle, "[%14s] ", ch->pcdata->pretitle );
    }
  else if( ch->race == race_lookup( "seraph" ) )
    {
      sprintf( chPretitle, "[{cSeraph{x    %s]", chClass );
    }
  else
    {
      if( relativeLevel == NULL )
	{
	  sprintf( chPretitle, "[%2d %6s %2s] ", ch->level, pc_race_table[ ch->race ].who_name, chClass );
	}
      else
	{
	  sprintf( chPretitle, "[%2s %6s %2s] ", relativeLevel, pc_race_table[ ch->race ].who_name, chClass );
	}
    }
  
  if( !IS_NPC( ch ) && ch->pcdata->pcClan != NULL )
    {
      sprintf( chClan, "[%s] ", ch->pcdata->pcClan->symbol );
    }

  if( ( IS_SET( looker->act_bits, PLR_BRAWLER ) || IS_IMMORTAL( looker ) ) &&
      IS_SET( ch->act_bits, PLR_BRAWLER ) )
    {
      showBrawler = TRUE;
    }

  snprintf( chLineItem, MAX_INPUT_LENGTH, "%s %s%s%s%s%s%s%s%s%s%s%s%s%s%s\n\r",
	    chPretitle,
	    chIncog,
	    chWizi,
	    IS_AFFECTED3( ch, AFF_VEIL ) ? VEILED : "",
	    guild_table[ ch->guild ].who_name,
	    IS_SET( ch->act_bits, PLR_QUEST ) ? QUEST : "",
	    IS_SET( ch->comm, COMM_AFK ) ? AFK : "",
	    IS_AFFECTED2( ch, AFF_WANTED ) ? WANTED : "",
	    IS_AFFECTED2( ch, AFF_GHOST ) ? GHOST : "",
	    IS_SET( ch->act_bits, ACT_NOAPPROVE ) ? UNAPPROVED : "",
	    IS_SET( ch->act_bits, PLR_NEWBIE ) ? NEWBIE : "",
	    showBrawler ? BRAWLER : "",
	    chClan,
	    ch->name,
	    IS_NPC( ch ) ? "" : ch->pcdata->title );

  return chLineItem;
}
  

/* whois command */
void do_whois (CHAR_DATA *ch, char *argument)
{
  char wchName[ MAX_INPUT_LENGTH ] = "";
  CHAR_DATA* wch = NULL;
  char* wchLine = NULL;

  argument = one_argument( argument, wchName );

  if( IS_NULLSTR( wchName ) )
    {
      send_to_char("You must provide a name.\n\r",ch);
      return;
    }

  wch = get_char_world( ch, wchName, FALSE );

  if( wch == NULL || IS_NPC( wch ) )
    {
      send_to_char( "Nobody by that name is playing.\n\r", ch );
      return;
    }
  else
    {
      wchLine = whoLine( wch, ch );
      if( wchLine != NULL )
	{
	  send_to_char( wchLine, ch );
	  free( wchLine );
	  wchLine = NULL;
	}
    }
}

/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void do_who( CHAR_DATA *ch, char *argument )
{
  int iClass;
  int iRace;
  int iGuild;
  int iLevelLower = 0;
  int iLevelUpper = MAX_LEVEL;
  int nMatch = 0;
  bool rgfClass[ MAX_CLASS ];
  bool rgfRace[ MAX_PC_RACE ];
  bool rgfGuild[ MAX_GUILD ];
  bool fUnapproved = FALSE;
  bool fBrawler = FALSE;
  bool fQuestOnly = FALSE;
  bool fNewbieOnly = FALSE;
  bool fImmortalOnly = FALSE;
  bool fOwnGuildRestrict = FALSE;
  bool fGuildRestrict = FALSE;
  bool fRaceRestrict = FALSE;
  bool fClassRestrict = FALSE;
  bool range_caught = FALSE;
  char arg[ MAX_INPUT_LENGTH ] = "";
  char arg2[ MAX_INPUT_LENGTH ] = "";
  char buf[ MAX_INPUT_LENGTH ] = "";
  char* wchOneLine = NULL;
  DESCRIPTOR_DATA* d = NULL;
  CHAR_DATA* wch = NULL;
  BUFFER* output = NULL;

  for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
    rgfClass[iClass] = FALSE;

  for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
    rgfRace[iRace] = FALSE;

  for (iGuild = 0; iGuild < MAX_GUILD; iGuild++)
    rgfGuild[iGuild] = FALSE;
 
  /*
   * Parse arguments.
   */

  while( !IS_NULLSTR( argument ) )
    {
      argument = one_argument( argument, arg );
      argument = one_argument( argument, arg2 );

      if( !IS_NULLSTR( arg ) )
	{
	  if( !strcasecmp( arg, "AR" ) )
	    {
	      if( ch->level > 43 )
		{
		  send_to_char( "Players found: 0\n\r", ch );
		  return;
		}
	      
	      iLevelLower = ( ch->level + 7 );
	      iLevelUpper = 50;
	      
	      if( iLevelLower >= iLevelUpper )
		{
		  send_to_char( "Players found: 0\n\r", ch );
		  return;
		}
	    }
	  else if( !strcasecmp( arg, "IR" ) )
	    {
	      if( ch->level < 11 )
		{
		  send_to_char( "Players found: 0\n\r", ch );
		  return;
		}
	      
	      iLevelLower = ch->level - 6;
	      
	      /* This is strange.  If you are level 43, level 50s appear as IR
		 to you, but to a level 50, level 43s are BR.  Silly, but
		 whatever.  Ask Mark. */
	      iLevelUpper = ( ch->level == 43 ) ? 50 : ( ch->level + 6 );

	      if (iLevelUpper > 50)
		iLevelUpper = 50;		 
	    }
	  else if( !strcasecmp( arg,"BR" ) )
	    {
	      if (ch->level < 11)
		{
		  send_to_char( "Players found: 0\n\r", ch );
		  return;
		}
	      
	      iLevelLower = 11;
	      iLevelUpper = ch->level - 7;
	      
	      if( iLevelLower >= iLevelUpper )
		{
		  send_to_char( "Players found: 0\n\r", ch );
		  return;
		}
	    }
	  else if( !strcasecmp( arg, "NB" ) )
	    {
	      iLevelLower = 1;
	      iLevelUpper = 10;
	    }
	  else if( !strcasecmp( arg, "UA" ) )
	    {
	      fUnapproved = TRUE;
	    }
	  else if( !strcasecmp(arg, "brawler" ) )
	    {
	      if( IS_IMMORTAL( ch ) || IS_SET( ch->act_bits,PLR_BRAWLER ) )
		{
		  fBrawler = TRUE;
		}
	      else
		{
		  send_to_char( "Players found: 0\n\r", ch );
		  return;
		}
	    }
	  else if( is_number( arg ) )
	    {
	      if( IS_IMMORTAL( ch ) )
		{
		  iLevelLower = atoi( arg );

		  if( is_number( arg2 ) )
		    {
		      range_caught = TRUE;
		      iLevelUpper = atoi( arg2 );
		    }
		  else
		    {
		      /* support for who X */
		      iLevelUpper = iLevelLower;
		    }
		}
	      else
		{
		  send_to_char( "Try {Gwho AR, who BR, or who IR.{x See \"help AR.\"", ch );
		  return;
		}
	    }
	  else if( !strcasecmp( arg, "quest" ) )
	    {
	      fQuestOnly = TRUE;
	    }
	  else if( !strcasecmp( arg, "newbie" ) )
	    {
	      fNewbieOnly = TRUE;
	    }
	  else if( !range_caught )
	    {	      
	      /*
	       * Look for classes to turn on.
	       */
	      if( !str_prefix( arg, "immortals" ) )
		{
		  fImmortalOnly = TRUE;
		} 
	      else
		{
		  iClass = class_lookup( arg );
		  
		  if( iClass == -1 )
		    {
		      iRace = race_lookup(arg);
		      
		      if( iRace == 0 || iRace >= MAX_PC_RACE )
			{
			  if( !str_prefix( arg, "guild" ) )
			    {
			      /* your own guild */
			      fOwnGuildRestrict = TRUE;
			    }
			  else
			    {
			      /* guild by name */
			      iGuild = guild_lookup(arg);
			      if( iGuild )
				{
				  fGuildRestrict = TRUE;
				  rgfGuild[ iGuild ] = TRUE;
				}
			      else
				{
				  send_to_char( "That's not a valid race, class, or guild.\n\r",ch);
				  return;
				}
			    }
			}
		      else
			{
			  fRaceRestrict = TRUE;
			  rgfRace[ iRace ] = TRUE;
			}
		    }
		  else
		    {
		      fClassRestrict = TRUE;
		      rgfClass[ iClass ] = TRUE;
		    }
		}
	    }
	}
    }

  /*
   * Now show matching chars.
   */

  output = new_buf();
  for ( d = descriptor_list; d != NULL; d = d->next )
    {
      /*
       * Check for match against restrictions.
       * Don't use trust as that exposes trusted mortals.
       */
      if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
	continue;
 
      wch   = ( d->original != NULL ) ? d->original : d->character;

      if (!can_see(ch,wch))
	continue;

      if ( wch->level < iLevelLower
	   ||   wch->level > iLevelUpper
	   || ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
	   || ( fClassRestrict && !rgfClass[wch->class] )
	   || ( fRaceRestrict && !rgfRace[wch->race])
	   || ( fOwnGuildRestrict && !is_guild(wch))
	   || ( fGuildRestrict && !rgfGuild[wch->guild] )
	   || ( fNewbieOnly && !IS_SET( wch->act_bits, PLR_NEWBIE ) ) 
	   || ( fUnapproved && !IS_SET( wch->act_bits, ACT_NOAPPROVE ) ) 
	   || ( fQuestOnly && !IS_SET( wch->act_bits, PLR_QUEST ) )
	   || ( fBrawler && !IS_SET( wch->act_bits, PLR_BRAWLER ) )
	   || ( !IS_IMMORTAL( ch ) && IS_AFFECTED3( wch, AFF_VEIL ) ) )
	continue;
 
      nMatch++;
      
      wchOneLine = whoLine( wch, ch );
      
      if( wchOneLine != NULL )
	{
	  add_buf( output, wchOneLine );
	  free( wchOneLine );
	  wchOneLine = NULL;
	}
    }

  sprintf( buf, "\n\rPlayers found: %d\n\r", nMatch );
  add_buf( output, buf );
  page_to_char( buf_string( output ), ch );
  
  free_buf( output );
  return;
}

void do_count ( CHAR_DATA *ch, char *argument )
{
  int count;
  DESCRIPTOR_DATA *d;
  char buf[MAX_STRING_LENGTH];

  count = 0;

  for ( d = descriptor_list; d != NULL; d = d->next )
    {
      if( d->connected == CON_PLAYING && 
	  can_see( ch, d->character ) &&
	  ( !( !IS_IMMORTAL( ch ) && IS_AFFECTED3( d->character, AFF_VEIL ) ) ) )
	count++;
    }

  if (count > max_on)
    max_on = count;
    
  sprintf(buf,"There are %d characters connected.  The most on since last reboot was %d.\n\r",
	  count,max_on);
    
  send_to_char(buf,ch);

}

void do_inventory( CHAR_DATA *ch, char *argument )
{
  send_to_char( "You are carrying:\n\r", ch );
  show_list_to_char( ch->carrying, ch, TRUE, TRUE );
  return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  int iWear;
  bool found;

  send_to_char( "You are using:\n\r", ch );
  found = FALSE;

  for( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
      send_to_char( where_name[ iWear ], ch );
      
      obj = get_eq_char( ch, iWear );

      if( obj == NULL || (global_config_aprilfools && !IS_NPC(ch) && ch->pcdata->aprilfools ) )
	{
	  send_to_char( "<nothing>\n\r", ch );
	}
      else
	{
	  found = TRUE;

	  if( can_see_obj( ch, obj ) )
	    {
	      printf_to_char( ch, "%s\n\r", format_obj_to_char( obj, ch, TRUE ) );
	    }
	  else
	    {
              send_to_char( "something.\n\r", ch );
	    }
	}
    }
  if ( global_config_aprilfools && !IS_NPC( ch ) && ch->pcdata->aprilfools )
    ch->pcdata->aprilfools = FALSE;
  /*
    if( !found )
    send_to_char( "Nothing.\n\r", ch );
  */   

  /*
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
    continue;

    send_to_char( where_name[iWear], ch );
    if ( can_see_obj( ch, obj ) )
    {
    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
    send_to_char( "\n\r", ch );
    }
    else
    {
    send_to_char( "something.\n\r", ch );
    }
    found = TRUE;
    }

    if ( !found )
    send_to_char( "Nothing.\n\r", ch );

    return;
  */
}



void do_compare( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj1;
  OBJ_DATA *obj2;
  int value1;
  int value2;
  char *msg;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  if ( arg1[0] == '\0' )
    {
      send_to_char( "Compare what to what?\n\r", ch );
      return;
    }

  if ( ( obj1 = get_obj_carry( ch, arg1, ch, TRUE ) ) == NULL )
    {
      sprintf(buf, "You do not have '%s'.\n\r", arg1 );
      send_to_char( buf, ch );
      //      send_to_char( "You do not have that item.\n\r", ch );
      return;
    }

  if (arg2[0] == '\0')
    {
      for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
	  if (obj2->wear_loc != WEAR_NONE
	      &&  can_see_obj(ch,obj2)
	      //          &&  obj1->item_type == obj2->item_type
	      &&  ( (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 
		    || (  obj1->item_type == ITEM_LIGHT 
			  && obj2->wear_loc == WEAR_LIGHT ) 
		    )
	      )
	    break;
	}

      if (obj2 == NULL)
	{
	  send_to_char("You aren't wearing anything comparable.\n\r",ch);
	  return;
	}
    } 

  else if ( (obj2 = get_obj_carry(ch,arg2,ch, TRUE) ) == NULL )
    {
      sprintf(buf, "You do not have '%s'.\n\r", arg2 );
      send_to_char( buf, ch );
      //      send_to_char("You do not have that item.\n\r",ch);
      return;
    }

  msg		= NULL;
  value1	= 0;
  value2	= 0;

  if ( obj1 == obj2 )
    {
      msg = "You compare $p to itself.  It looks about the same.";
    }
  /*  else if ( obj1->item_type != obj2->item_type )
      {
      msg = "You can't compare $p and $P.";
      }*/
  else
    {/* Old subroutine - rather useless.  Removed by gkl Jun10 2007
	switch ( obj1->item_type )
	{
	default:
	msg = "You can't compare $p and $P.";
	break;

	case ITEM_ARMOR:
	value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	break;

	case ITEM_WEAPON:
	if (obj1->pIndexData->new_format)
	value1 = (1 + obj1->value[2]) * obj1->value[1];
	else
	value1 = obj1->value[1] + obj1->value[2];

	if (obj2->pIndexData->new_format)
	value2 = (1 + obj2->value[2]) * obj2->value[1];
	else
	value2 = obj2->value[1] + obj2->value[2];
	break;
	}*/
      if ( obj1->pIndexData && obj2->pIndexData )
        {
	  value1 = get_object_points( obj1->pIndexData );
	  value2 = get_object_points( obj2->pIndexData );
        }
      else
        {
	  value1 = 0;
	  value2 = 0;
        }
    }

  if ( msg == NULL )
    {
      if ( value1 == value2 ) msg = "$p and $P look about the same.";
      else if ( value1  > value2 ) msg = "$p looks better than $P.";
      else                         msg = "$p looks worse than $P.";
    }

  act( msg, ch, obj1, obj2, TO_CHAR );
  return;
}



void do_credits( CHAR_DATA *ch, char *argument )
{
  do_help( ch, "diku" );
  return;
}



void do_where( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *d;
  bool found;


  if (IS_AFFECTED(ch,AFF_BLIND)){
    send_to_char("You're blinded!\n\r",ch);
    return;
  }

  if (IS_AFFECTED3(ch,AFF_DIRT)){
    send_to_char("You're blinded!\n\r",ch);
    return;
  }

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
      send_to_char( "Players near you:\n\r", ch );
      found = FALSE;

      for( d = descriptor_list; d; d = d->next )
	{
	  if( d->connected == CON_PLAYING 
	      && ( victim = d->character ) != NULL
	      && !IS_NPC( victim )
	      && victim->in_room != NULL
	      && !IS_SET( victim->in_room->room_flags, ROOM_NOWHERE )
	      && ( is_room_owner( ch, victim->in_room ) ||
		   !room_is_private( victim->in_room ) )
	      && victim->in_room->area == ch->in_room->area )
	    {

	      /* 
		 this should take hidden victims out of where, regardless
		 of detects status
	      */

	      if( !IS_AFFECTED( victim, AFF_HIDE ))
		{
		  if( can_see( ch, victim ) )
		    {
		      found = TRUE;
		      sprintf( buf, "%-28s {c%s{x\n\r",
			       victim->name, victim->in_room->name );
		      send_to_char( buf, ch );
		    }
		}
	    }
	}

      if ( !found )
	send_to_char( "None\n\r", ch );
    }
  else
    {
      found = FALSE;
      for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	  if ( victim->in_room != NULL
	       &&   victim->in_room->area == ch->in_room->area
	       &&   !IS_AFFECTED(victim, AFF_HIDE)
	       &&   !IS_AFFECTED(victim, AFF_SNEAK)
	       &&   can_see( ch, victim )
	       &&   is_name( arg, victim->name ) )
	    {
	      found = TRUE;
	      sprintf( buf, "%-28s %s\n\r",
		       PERS2(victim, ch), victim->in_room->name );
	      send_to_char( buf, ch );
	      break;
	    }
	}
      if ( !found )
	act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

  return;
}




void do_consider( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  char *msg;
  int diff;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
      send_to_char( "Consider killing whom?\n\r", ch );
      return;
    }

  if ( ( victim = get_char_room( ch, arg, TRUE ) ) == NULL )
    {
      send_to_char( "They're not here.\n\r", ch );
      return;
    }

  if (is_safe(ch,victim))
    {
      send_to_char("Don't even think about it.\n\r",ch);
      return;
    }

  diff = victim->level - ch->level;

  if ( diff <= -10 ) msg = "You can kill $N naked and weaponless.";
  else if ( diff <=  -5 ) msg = "$N is no match for you.";
  else if ( diff <=  -2 ) msg = "$N looks like an easy kill.";
  else if ( diff <=   1 ) msg = "The perfect match!";
  else if ( diff <=   4 ) msg = "$N says 'Do you feel lucky, punk?'.";
  else if ( diff <=   9 ) msg = "$N smirks and says 'You think so?  I don't.";
  else if ( diff <=   14 ) msg = "$N laughs at you mercilessly.";
  else if ( diff <=   19 ) msg = "$N asks 'You and what army?";
  else                    msg = "Your death will be pointless, but quick.";

  act( msg, ch, NULL, victim, TO_CHAR );
  return;
}



void set_title( CHAR_DATA *ch, char *title )
{
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    {
      bug( "Set_title: NPC.", 0 );
      return;
    }

  if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' )
    {
      buf[0] = ' ';
      strcpy( buf+1, title );
    }
  else
    {
      strcpy( buf, title );
    }

  sprintf(buf,"%s{x",buf);

  free_string( ch->pcdata->title );
  ch->pcdata->title = str_dup( buf );
  return;
}

void do_title( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) )
    return;

  if ( argument[0] == '\0' )
    {
      send_to_char( "Change your title to what?\n\r", ch );
      return;
    }

  if ( strlen(argument) > 45 )
    argument[45] = '\0';

  smash_tilde( argument );
  set_title( ch, argument );
  send_to_char( "Ok.\n\r", ch );
}



void do_description( CHAR_DATA *ch, char *argument )
{
  char buf[ MAX_STRING_LENGTH + 2*MAX_INPUT_LENGTH ];

  if( !str_cmp( argument, "write" ) ) 
    {
      string_append(ch, &ch->description);
      return;
    }

  if ( !str_cmp( argument, "format" ) )
    {
      ch->description = format_string( ch->description );
      send_to_char( "Description formatted.\n\r", ch );
      return;
    }

  if ( argument[0] != '\0' )
    {
      buf[0] = '\0';
      smash_tilde( argument );

      /* subtract bottom line */
      if (argument[0] == '-')
    	{
	  int len;
	  bool found = FALSE;
 
	  if (ch->description == NULL || ch->description[0] == '\0')
            {
	      send_to_char("No lines left to remove.\n\r",ch);
	      return;
            }

	  strcpy(buf,ch->description);
 
	  for (len = strlen(buf); len > 0; len--)
            {
	      if (buf[len] == '\r')
                {
		  if (!found)
                    {
		      if (len > 0)
			len--;
		      found = TRUE;
                    }
		  else
                    {
		      buf[len + 1] = '\0';
		      free_string(ch->description);
		      ch->description = str_dup(buf);
		      send_to_char( "Your description is:\n\r", ch );
		      send_to_char( ch->description ? ch->description : 
				    "(None).\n\r", ch );
		      return;
                    }
                }
            }

	  /* in case of only one line */

	  buf[0] = '\0';
	  free_string(ch->description);
	  ch->description = str_dup(buf);
	  send_to_char("Description cleared.\n\r",ch);
	  return;
        }

      if ( argument[0] == '+' )
        {
	  buf[0] = '\0';
	  if ( ch->description != NULL )
	    strcat( buf, ch->description );
	  argument++;
	  while ( isspace(*argument) )
	    argument++;

          strcat( buf, argument );
          strcat( buf, "\n\r" );

	  if( strlen(buf) >= MAX_STRING_LENGTH )
	    send_to_char( "Description too long.\n\r", ch );
          else
	    {
	      free_string( ch->description );
	      ch->description = str_dup( buf );
	    }

	  return;

        }
    }

  send_to_char( "Your description is:\n\r", ch );
  send_to_char( ch->description ? ch->description : "(None).\n\r", ch );

}



void do_report( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_INPUT_LENGTH];

  sprintf( buf,
	   "You say 'I have %d/%d hp %d/%d mana %d/%d mv %d xp.'\n\r",
	   ch->hit,  ch->max_hit,
	   ch->mana, ch->max_mana,
	   ch->move, ch->max_move,
	   ch->exp   );

  send_to_char( buf, ch );

  sprintf( buf, "$n says 'I have %d/%d hp %d/%d mana %d/%d mv %d xp.'",
	   ch->hit,  ch->max_hit,
	   ch->mana, ch->max_mana,
	   ch->move, ch->max_move,
	   ch->exp   );

  act( buf, ch, NULL, NULL, TO_ROOM );

  return;
}



void do_practice( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  /*  char buf2[MAX_STRING_LENGTH]; */
  int sn;
  /*  int dr = 0; */

  if ( IS_NPC(ch) )
    return;

  if (IS_AFFECTED2(ch,AFF_GHOST)){
    send_to_char("You're a ghost, you don't need to practice!.\n\r",ch);
    return;
  }

  if ( argument[0] == '\0' )
    {                
      int col;
      col    = 0;
      /*        if (IS_DRMODE(ch))
		{
		send_to_char("{G======================================================================{x\n\r",ch);
		} */
      for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	  if ( skill_table[sn].name == NULL )
	    break;
	  if ( ch->level < skill_table[sn].skill_level[ch->class] 
	       || ch->pcdata->learned[sn] < 1 /* skill is not known */)
	    continue;
 
	  /*          if (ch->pcdata->learned[sn] == 100)
		      dr = 5;
		      if (ch->pcdata->learned[sn] < 100)
		      dr = 4;
		      if (ch->pcdata->learned[sn] < 86)
		      dr = 3;
		      if (ch->pcdata->learned[sn] < 75)
		      dr = 2;
		      if (ch->pcdata->learned[sn] < 51)
		      dr = 1;
		      if (ch->pcdata->learned[sn] < 11)
		      dr = 0;
	  */
	  if  ( global_config_aprilfools && ch->pcdata->aprilfools )
	    sprintf( buf, "%-18s %3d%%  ",
		     skill_table[sn].name, 1 );
	  else
	    sprintf( buf, "%-18s %3d%%  ",
		     skill_table[sn].name, ch->pcdata->learned[sn] );
	  //          sprintf( buf2, "{G|{M%-17s {c%2d{G|{x  ",
	  //          skill_table[sn].name, dr );
	  //          if (!IS_DRMODE(ch))
	  send_to_char( buf, ch );
	  //          else
	  //              send_to_char( buf2, ch);

	  if ( ++col % 3 == 0 )
	    send_to_char( "\n\r", ch );

	}

      if ( col % 3 != 0 )
	send_to_char( "\n\r", ch );

      //      if (!IS_DRMODE(ch))
      //       {
      sprintf( buf, "You have %d practice sessions left.\n\r",ch->practice );
      send_to_char( buf, ch );
      /*       }
	       else
	       {
	       send_to_char("{G======================================================================{x\n\r",ch);
	       sprintf(buf,"{G| {BYou have {c%3d {Bpractice sessions left.                               {G|{x\n\r",ch->practice);
	       send_to_char( buf, ch);
	       send_to_char("{G======================================================================{x\n\r",ch);
	       } */
      if ( global_config_aprilfools && ch->pcdata->aprilfools )
	ch->pcdata->aprilfools = FALSE;
    }
  else
    {
      CHAR_DATA *mob;
      int adept;

      if ( !IS_AWAKE(ch) )
	{
	  send_to_char( "In your dreams, or what?\n\r", ch );
	  return;
	}

      for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{
	  if ( IS_NPC(mob) && IS_SET(mob->act_bits, ACT_PRACTICE) )
	    break;
	}

      if ( mob == NULL )
	{
	  send_to_char( "You can't do that here.\n\r", ch );
	  return;
	}

      if ( ch->practice <= 0 )
	{
	  send_to_char( "You have no practice sessions left.\n\r", ch );
	  return;
	}

      if ( ( sn = find_spell( ch,argument ) ) < 0
	   || ( !IS_NPC(ch)
		&&   (ch->level < skill_table[sn].skill_level[ch->class] 
		      ||    ch->pcdata->learned[sn] < 1 /* skill is not known */
		      ||    skill_table[sn].rating[ch->class] == 0)))
	{
	  send_to_char( "You can't practice that.\n\r", ch );
	  return;
	}


      adept = IS_NPC(ch) ? 100 : class_table[ch->class].skill_adept;

      if ( ch->pcdata->learned[sn] >= adept )
	{
	  sprintf( buf, "You are already learned at %s.\n\r",
		   skill_table[sn].name );
	  send_to_char( buf, ch );
	}
      else
	{
	  ch->practice--;
	  ch->pcdata->learned[sn] += 
	    int_app[get_curr_stat(ch,STAT_INT)].learn / 
	    skill_table[sn].rating[ch->class];
	  if ( ch->pcdata->learned[sn] < adept )
	    {
	      act( "You practice $T.",
		   ch, NULL, skill_table[sn].name, TO_CHAR );
	      act( "$n practices $T.",
		   ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	  else
	    {
	      ch->pcdata->learned[sn] = adept;
	      act( "You are now learned at $T.",
		   ch, NULL, skill_table[sn].name, TO_CHAR );
	      act( "$n is now learned at $T.",
		   ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	}
    }
  return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int wimpy;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
    wimpy = ch->max_hit / 5;
  else
    wimpy = atoi( arg );

  if ( wimpy < 0 )
    {
      send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
      return;
    }

  if ( wimpy > ch->max_hit/2 )
    {
      send_to_char( "Such cowardice ill becomes you.\n\r", ch );
      return;
    }

  ch->wimpy	= wimpy;
  sprintf( buf, "Wimpy set to %d hit points.\n\r", wimpy );
  send_to_char( buf, ch );
  return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char *pArg;
  char *pwdnew;
  char *p;
  char cEnd;

  if ( IS_NPC(ch) )
    return;

  /*
   * Can't use one_argument here because it smashes case.
   * So we just steal all its code.  Bleagh.
   */
  pArg = arg1;
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
      *pArg++ = *argument++;
    }
  *pArg = '\0';

  pArg = arg2;
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
      *pArg++ = *argument++;
    }
  *pArg = '\0';

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
      send_to_char( "Syntax: password <old> <new>.\n\r", ch );
      return;
    }

  if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
      WAIT_STATE( ch, 40 );
      send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
      return;
    }

  if ( strlen(arg2) < 5 )
    {
      send_to_char(
		   "New password must be at least five characters long.\n\r", ch );
      return;
    }

  /*
   * No tilde allowed because of player file format.
   */
  pwdnew = crypt( arg2, ch->name );
  for ( p = pwdnew; *p != '\0'; p++ )
    {
      if ( *p == '~' )
	{
	  send_to_char(
		       "New password not acceptable, try again.\n\r", ch );
	  return;
	}
    }

  free_string( ch->pcdata->pwd );
  ch->pcdata->pwd = str_dup( pwdnew );
  save_char_obj( ch );
  send_to_char( "Ok.\n\r", ch );
  return;
}

void do_deschalf( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  /* Kes addition 04 Jan 2001 */
  if( !str_cmp( argument, "write" ) )
    {
      string_append( ch, &ch->shiftdesc1 );
      return;
    }
  /* End addition */

  if ( argument[0] != '\0' )
    {
      buf[0] = '\0';
      smash_tilde( argument );

      if (argument[0] == '-')
    	{
	  int len;
	  bool found = FALSE;
 
	  if (ch->shiftdesc1 == NULL || ch->shiftdesc1[0] == '\0')
            {
	      send_to_char("No lines left to remove.\n\r",ch);
	      return;
            }
	
	  strcpy(buf,ch->shiftdesc1);
 
	  for (len = strlen(buf); len > 0; len--)
            {
	      if (buf[len] == '\r')
                {
		  if (!found)  /* back it up */
                    {
		      if (len > 0)
			len--;
		      found = TRUE;
                    }
		  else /* found the second one */
                    {
		      buf[len + 1] = '\0';
		      free_string(ch->shiftdesc1);
		      ch->shiftdesc1 = str_dup(buf);
		      send_to_char( "Your half shifted description is:\n\r", ch );
		      send_to_char( ch->shiftdesc1 ? ch->shiftdesc1 : 
				    "(None).\n\r", ch );
		      return;
                    }
                }
            }
	  buf[0] = '\0';
	  free_string(ch->shiftdesc1);
	  ch->shiftdesc1 = str_dup(buf);
	  send_to_char("Half shift description cleared.\n\r",ch);
	  return;
        }
      if ( argument[0] == '+' )
	{
	  if ( ch->shiftdesc1 != NULL )
	    strcat( buf, ch->shiftdesc1 );
	  argument++;
	  while ( isspace(*argument) )
	    argument++;
	}

      if ( strlen(buf) >= 1024)
	{
	  send_to_char( "Description too long.\n\r", ch );
	  return;
	}

      strcat( buf, argument );
      strcat( buf, "\n\r" );
      free_string( ch->shiftdesc1 );
      ch->shiftdesc1 = str_dup( buf );
    }

  send_to_char( "Your half shifted description is:\n\r", ch );
  send_to_char( ch->shiftdesc1 ? ch->shiftdesc1 : "(None).\n\r", ch );
  return;
}

void do_descfull( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  /* Kes Addition 04 Jan 2002 */
  if( !str_cmp( argument, "write" ) )
    {
      string_append( ch, &ch->shiftdesc2 );
      return;
    }
  /* End addition */

  if ( argument[0] != '\0' )
    {
      buf[0] = '\0';
      smash_tilde( argument );

      if (argument[0] == '-')
    	{
	  int len;
	  bool found = FALSE;
 
	  if (ch->shiftdesc2 == NULL || ch->shiftdesc2[0] == '\0')
            {
	      send_to_char("No lines left to remove.\n\r",ch);
	      return;
            }
	
	  strcpy(buf,ch->shiftdesc2);
 
	  for (len = strlen(buf); len > 0; len--)
            {
	      if (buf[len] == '\r')
                {
		  if (!found)  /* back it up */
                    {
		      if (len > 0)
			len--;
		      found = TRUE;
                    }
		  else /* found the second one */
                    {
		      buf[len + 1] = '\0';
		      free_string(ch->shiftdesc2);
		      ch->shiftdesc2 = str_dup(buf);
		      send_to_char( "Your full shifted description is:\n\r", ch );
		      send_to_char( ch->shiftdesc2 ? ch->shiftdesc2 : 
				    "(None).\n\r", ch );
		      return;
                    }
                }
            }
	  buf[0] = '\0';
	  free_string(ch->shiftdesc2);
	  ch->shiftdesc2 = str_dup(buf);
	  send_to_char("Half shift description cleared.\n\r",ch);
	  return;
        }
      if ( argument[0] == '+' )
	{
	  if ( ch->shiftdesc2 != NULL )
	    strcat( buf, ch->shiftdesc2 );
	  argument++;
	  while ( isspace(*argument) )
	    argument++;
	}

      if ( strlen(buf) >= 1024)
	{
	  send_to_char( "Description too long.\n\r", ch );
	  return;
	}

      strcat( buf, argument );
      strcat( buf, "\n\r" );
      free_string( ch->shiftdesc2 );
      ch->shiftdesc2 = str_dup( buf );
    }

  send_to_char( "Your full shifted description is:\n\r", ch );
  send_to_char( ch->shiftdesc2 ? ch->shiftdesc2 : "(None).\n\r", ch );
  return;
}

void do_famdesc( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    {
      send_to_char( "Mobs can't do that.\n\r", ch );
      return;
    }

  if ( !IS_ADMIN( ch ) && ch->guild != guild_lookup("arcaenum") )
    {
      send_to_char( "Huh?\n\r", ch );
      return;
    }
    
  if( !str_cmp( argument, "write" ) )
    {
      string_append( ch, &ch->pcdata->famdesc );
      return;
    }

  if ( argument[0] != '\0' )
    {
      buf[0] = '\0';
      smash_tilde( argument );

      if (argument[0] == '-')
    	{
	  int len;
	  bool found = FALSE;
 
	  if (ch->pcdata->famdesc == NULL || ch->pcdata->famdesc[0] == '\0')
            {
	      send_to_char("No lines left to remove.\n\r",ch);
	      return;
            }
	
	  strcpy(buf,ch->pcdata->famdesc);
 
	  for (len = strlen(buf); len > 0; len--)
            {
	      if (buf[len] == '\r')
                {
		  if (!found)
                    {
		      if (len > 0)
			len--;
		      found = TRUE;
                    }
		  else
                    {
		      buf[len + 1] = '\0';
		      free_string(ch->pcdata->famdesc);
		      ch->pcdata->famdesc = str_dup(buf);
		      send_to_char("Your familiar's description is:\n\r", ch);
		      send_to_char(ch->pcdata->famdesc ? ch->pcdata->famdesc :
				   "(None).\n\r", ch );
		      return;
                    }
                }
            }
	  buf[0] = '\0';
	  free_string(ch->pcdata->famdesc);
	  ch->pcdata->famdesc = str_dup(buf);
	  send_to_char("Your familiar's description has been cleared.\n\r",ch);
	  return;
        }
      if ( argument[0] == '+' )
	{
	  if ( ch->pcdata->famdesc != NULL )
	    strcat( buf, ch->pcdata->famdesc );
	  argument++;
	  while ( isspace(*argument) )
	    argument++;
	}

      if ( strlen(buf) >= 1024)
	{
	  send_to_char( "Description too long.\n\r", ch );
	  return;
	}

      strcat( buf, argument );
      strcat( buf, "\n\r" );
      free_string( ch->pcdata->famdesc );
      ch->pcdata->famdesc = str_dup( buf );
    }

  send_to_char( "Your familiar's description is:\n\r", ch );
  send_to_char(ch->pcdata->famdesc ? ch->pcdata->famdesc : "(None).\n\r", ch);
  return;
}

void do_famsex( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  const char *sexlabel[] = { "of neutral gender", "male", "female" };

  if ( IS_NPC(ch) )
    {
      send_to_char( "Mobs can't do that.\n\r", ch );
    }
  else if ( !IS_ADMIN( ch ) && ch->guild != guild_lookup("arcaenum") )
    {
      send_to_char( "Huh?\n\r", ch );
    }
  else if ( IS_NULLSTR(argument) )
    {
      if ( ch->pcdata->famsex <= SEX_FEMALE
	   &&   ch->pcdata->famsex >= SEX_NEUTRAL )
        {
	  sprintf(buf, "Your familiar is currently %s.\n\r", 
		  sexlabel[ch->pcdata->famsex] );
	  send_to_char( buf, ch );
        }
      else
        {
	  bug( "do_famgender: fam gender is out of bounds: %d", 
	       ch->pcdata->famsex );
	  send_to_char( "Something is horribly wrong with your familiar!\n\r",
			ch );
        }
    }
  else if ( LOWER(argument[0]) == 'm' )
    {
      ch->pcdata->famsex = SEX_MALE;
      send_to_char( "Your familiar will now be male.\n\r", ch );
    }
  else if ( LOWER(argument[0]) == 'f' )
    {
      ch->pcdata->famsex = SEX_FEMALE;
      send_to_char( "Your familiar will now be female.\n\r", ch );
    }
  else if ( LOWER(argument[0]) == 'n' )
    {
      ch->pcdata->famsex = SEX_FEMALE;
      send_to_char( "Your familiar will now be gender neutral.\n\r", ch );
    }
  else
    {
      send_to_char( 
		   "You must specify male, female, or none as your familiar's gender.\n\r",
		   ch );
    }

  return;
}

void do_learn( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  int sn;
  CHAR_DATA *mob;
  int adept;

  if ( IS_NPC(ch) )
    return;

  if (IS_AFFECTED2(ch,AFF_GHOST)){
    send_to_char("You're a ghost, you don't need to learn!.\n\r",ch);
    return;
  }

  if (!IS_SET(ch->shiftbits,SHIFT_POTENTIAL)){
    send_to_char("There's nothing for you to learn.\n\r",ch);
    return;
  }

  if ( argument[0] == '\0' )
    {
      int col;
      bool found = FALSE;

      col    = 0;
      for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	  if ( skill_table[sn].name == NULL )
	    break;
	  if ( ch->level < skill_table[sn].were_level[ch->race] 
	       || ch->pcdata->learned[sn] > 0) /* skill is known */
	    continue;

	  if (!found) {
	    send_to_char("You can learn the following:\n\r",ch);
	    found = TRUE;
	  }
	  sprintf( buf, "%-18s ",
		   skill_table[sn].name );
	  send_to_char( buf, ch );
	  if ( ++col % 3 == 0 )
	    send_to_char( "\n\r", ch );
	}

      if ( col % 3 != 0 )
	send_to_char( "\n\r", ch );

      if (!found) 
	send_to_char("There is nothing for you to learn right now.\n\r",ch);

      sprintf( buf, "You have %d training sessions left.\n\r",
	       ch->train );
      send_to_char( buf, ch );

      return;
    }

  if ( !IS_AWAKE(ch) ) {
    send_to_char( "In your dreams, or what?\n\r", ch );
    return;
  }

  for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room ) {
    if ( IS_NPC(mob) && IS_SET(mob->act_bits, ACT_WERETRAIN) )
      break;
  }

  if ( mob == NULL )
    {
      send_to_char( "You can't do that here.\n\r", ch );
      return;
    }

  if ( ch->train <= 1 )
    {
      send_to_char( "You don't have enough training sessions left.\n\r", ch );
      return;
    }

  if (ch->gold < 500) {
    send_to_char( "You don't have enough gold left.\n\r", ch );
    return;
  }

  if ( ( sn = find_spell( ch,argument ) ) < 0
       || ( !IS_NPC(ch)
	    &&   (ch->level < skill_table[sn].were_level[ch->race] )))
    {
      send_to_char( "You can't learn that.\n\r", ch );
      return;
    }

  adept = IS_NPC(ch) ? 100 : 100;

  if ( ch->pcdata->learned[sn] >= adept )
    {
      sprintf( buf, "You are already learned at %s.\n\r",
	       skill_table[sn].name );
      send_to_char( buf, ch );
    }
  else
    {
      ch->practice--;
      ch->pcdata->learned[sn] = 100;
      ch->pcdata->learned[sn] = adept;
      act( "You are now learned at $T.",
	   ch, NULL, skill_table[sn].name, TO_CHAR );
      act( "$n is now learned at $T.",
	   ch, NULL, skill_table[sn].name, TO_ROOM );

      ch->train -= 2;
      ch->gold -= 500;	

    }
  return;
}

void do_throwdice( CHAR_DATA* ch, char* argument )
{
  char toroom[ MAX_STRING_LENGTH ] = "";
  char output[ MAX_STRING_LENGTH ] = "";
  char* outputpt = output;
  char arg[ MAX_INPUT_LENGTH ] = "";
  int numdice = 0;
  int diceside = 0;
  int total = 0;
  int onethrow;
  int i;

  argument = one_argument( argument, arg );
  if( *arg == '\0' )
    {
      send_to_char( "How many dice would you like to throw?\n\r", ch );
      return;
    }
  numdice = atoi( arg );

  argument = one_argument( argument, arg );
  if( *arg == '\0' )
    {
      send_to_char( "What kind of dice would you like to throw?\n\r", ch );
      return;
    }
  diceside = atoi( arg );

  if( numdice < 1 )
    {
      send_to_char( "You must throw at least one die.\n\r", ch );
      return;
    }

  if( diceside < 2 )
    {
      send_to_char( "A die must have at least two sides.\n\r", ch );
      return;
    }

  if( numdice > 100 ||
      diceside > 100 )
    {
      send_to_char( "There is a limit of 100 dice and 100 sides to a die.\n\r", ch );
      return;
    }

  if( numdice == 1 )
    {
      sprintf( output, "You throw a %d-sided die.  The result is...\n\r", diceside );
      sprintf( toroom, "$n throws a %d-sided die.  The result is...", diceside );
    }
  else
    {
      sprintf( output, "You throw %d %d-sided dice.  The results are...\n\r", numdice, diceside );
      sprintf( toroom, "$n throws %d %d-sided dice.  The results are...", numdice, diceside );
    }

  send_to_char( output, ch );
  act( toroom, ch, NULL, NULL, TO_ROOM );


  for( i = 1; i <= numdice; i++ )
    {
      onethrow = dice( 1, diceside );
      outputpt += sprintf( outputpt, "%d ", onethrow );

      if( i % 15 == 0 )
	outputpt += sprintf( outputpt, "\n\r" );

      total += onethrow;
    }

  outputpt += sprintf( outputpt, "(Total: %d)", total );

  printf_to_char( ch, "%s\n\r", output );
  act( output, ch, NULL, NULL, TO_ROOM );

}

  

void do_glance( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int percent;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
      send_to_char( "Glance at whom?\n\r", ch );
      return;
    }

  if ( ( victim = get_char_room( ch, arg, TRUE ) ) == NULL )
    {
      send_to_char( "They're not here.\n\r", ch );
      return;
    }


  if ((IS_SET(victim->shiftbits,SHIFT_FULL) ||
       IS_SET(victim->shiftbits,MIST_VAMP)) 
      && (victim->shiftdesc2[0] != '\0'))
    send_to_char( victim->shiftdesc2, ch );
  else if ((IS_SET(victim->shiftbits,SHIFT_HALF) ||
	    IS_SET(victim->shiftbits,BAT_VAMP))
	   && (victim->shiftdesc1[0] != '\0'))
    send_to_char( victim->shiftdesc1, ch );
  else if ( victim->description[0] != '\0' )
    {
      send_to_char( victim->description, ch );
    }
  else
    {
      act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

  if ( victim->max_hit > 0 )
    percent = ( 100 * victim->hit ) / victim->max_hit;
  else
    percent = -1;

  strcpy( buf, PERS(victim, ch) );

  if (percent >= 100) 
    strcat( buf, "{g is in excellent condition.{x\n\r");
  else if (percent >= 90) 
    strcat( buf, "{g has a few scratches.{x\n\r");
  else if (percent >= 75) 
    strcat( buf,"{g has some small wounds and bruises.{x\n\r");
  else if (percent >=  50) 
    strcat( buf, "{c has quite a few wounds.{x\n\r");
  else if (percent >= 30)
    strcat( buf, "{c has some big nasty wounds and scratches.{x\n\r");
  else if (percent >= 15)
    strcat ( buf, "{r looks pretty hurt.{x\n\r");
  else if (percent >= 0 )
    strcat (buf, "{r is in awful condition.{x\n\r");
  else
    strcat(buf, "{r is bleeding to death.{x\n\r");

  buf[0] = UPPER(buf[0]);
  send_to_char( buf, ch );

  return;
}
/* The IC-identify support code */

bool is_blankstring( const char *argument )
{
  if ( IS_NULLSTR( argument ) )
    return TRUE;

  for ( ; *argument != '\0'; argument++ )
    if ( !isspace( *argument ) )
      return FALSE;

  return TRUE;
}

bool is_vowel( char banana )
{
  if ( LOWER(banana) == 'a' || LOWER(banana) == 'e' ||
       LOWER(banana) == 'i' || LOWER(banana) == 'o' ||
       LOWER(banana) == 'u' )
    return TRUE;
  else
    return FALSE;
}

char *exact_flag_string( const struct flag_type *flag_table, int bit )
{
  static char buf[1024];
  int i;

  buf[0] = '\0';
  for ( i = 0; flag_table[i].name != NULL; i++ )
    if ( flag_table[i].bit == bit )
      {
	strcpy( buf, flag_table[i].name );
	break;
      }
  if ( IS_NULLSTR(buf) )
    sprintf( buf, "unknown" );

  return buf;
}



char *idscale_compare( const struct idscale_type *idscale_table, int val )
{
  int i;

  /* Cutoff - Table values only go up to 32767 */
  if ( val > 32767 )
    val = 32767;

  for ( i = 0; !IS_NULLSTR(idscale_table[i].string); i++ )
    if ( val <= idscale_table[i].max )
      return idscale_table[i].string;
  return NULL;
}


void show_obj_stats( CHAR_DATA *ch, OBJ_DATA *obj )
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char objtype[1024];
  long counter;
  bool noident = FALSE;
  int i;
  AFFECT_DATA *paf, *afsrc;

  if ( IS_NPC(ch) )
    return;

  strcpy( objtype, exact_flag_string( ic_type_flags, obj->item_type ) );
  if ( !str_cmp( objtype, "unknown" ) )
    sprintf( objtype, "unknown type of object" );

  strcpy( buf2, flag_string( ic_extra_flags, obj->extra_flags ) );
  if ( !str_cmp( buf2, "none" ) )
    buf2[0] = '\0';

  /*  if ( obj->item_type == ITEM_WEAPON && obj->value[4] )
      {
      strcat( buf2, " " );
      strcat( buf2, flag_string( ic_weapon_type2, obj->value[4] ) );
      }
      else*/ if ( (obj->item_type == ITEM_CONTAINER 
		   || obj->item_type == ITEM_BOOK ) 
		  &&    obj->value[1] )
    {
      strcat( buf2, " " );
      strcat( buf2, flag_string( ic_container_flags, obj->value[1] ) );
    }

  if ( IS_SET(obj->extra_flags,ITEM_NO_IDENTIFY) )
    {
      noident = TRUE;
      strcpy( buf2, "unidentifiable" );
    }
  sprintf( buf, "%s is a%s %s that is %s.\n\r",
	   obj->short_descr,
	   is_vowel( objtype[0] ) ? "n" : "",
	   objtype,
	   ( is_blankstring(buf2) ) ?
	   "without any notable features" :
	   listize(buf2, TRUE) );
  buf[0] = UPPER(buf[0]);
  send_to_char( buf, ch );

  if ( noident ) return;

  /*
    sprintf( buf, "It is referred to as %s.\n\r",
    listize( obj->name, TRUE ) );
    send_to_char( buf, ch );
  */
    
  sprintf( buf, "The %s is also %s.\n\r",
	   ( obj->item_type == ITEM_WEAPON ) ? 
	   ( obj->value[0] == WEAPON_EXOTIC ? "exotic weapon" : 
	     exact_flag_string( weapon_class, obj->value[0] ) ) : 
	   objtype,
	   idscale_compare(weight_scale,obj->weight * 100 / can_carry_w(ch)));

  send_to_char( buf, ch );

  switch ( obj->item_type )
    {
    case ITEM_SCROLL: 
    case ITEM_POTION:
    case ITEM_PILL:
      buf2[0] = '\0';
      counter = 0;
      for ( i = 1; i <= 4; i++ )
	if ( obj->value[i] > 0 && obj->value[i] < MAX_SKILL )
	  {
	    counter++;
	    strcat( buf2, " '" );
	    strcat( buf2, skill_table[obj->value[i]].name );
	    strcat( buf2, "'" );
	  }
      if ( is_blankstring(buf2) )
	sprintf(buf, "It isn't imbued with any power.\n\r" );
      else
	sprintf(buf, "It is imbued with the power of%s %s %s spell%s.\n\r",
	        ( counter == 1 ) ? " a" : "",
                idscale_compare( splevel_scale, obj->value[0] - ch->level ),
		listize( buf2, TRUE ),
		( counter > 1 ) ? "s" : "" );
      send_to_char( buf, ch );
      break;
    case ITEM_WAND: 
    case ITEM_STAFF:
      if ( obj->value[3] > 0 && obj->value[3] < MAX_SKILL )
        {
	  sprintf( buf, "It has %d charge%s of a %s %s.\n\r",
		   obj->value[2],
		   obj->value[2] == 1 ? "" : "s",
		   idscale_compare( splevel_scale, obj->value[0] - ch->level ),
		   skill_table[obj->value[3]].name );
        }
      else
        {
	  sprintf( buf, "This is a mythical broken %s!  "
		   "You should notify the immortals of this finding at once!\n\r",
		   objtype );
        }
      send_to_char( buf, ch );
      break;
    case ITEM_DRINK_CON:
      sprintf(buf,"It holds %s-colored %s.\n\r",
	      liq_table[obj->value[2]].liq_color,
	      liq_table[obj->value[2]].liq_name);
      send_to_char( buf, ch );
      break;
    case ITEM_CONTAINER:
      sprintf(buf, "It can hold %s item%s, %s weight, and %s.\n\r",
	      idscale_compare( contnum_scale, obj->value[0] * 100 / can_carry_n(ch) ),
	      ( (obj->value[0] == 1) ? "" : "s" ),
	      idscale_compare( contwgt_scale, obj->value[3] * 100 / can_carry_w(ch) ),
	      idscale_compare( contmlt_scale, obj->value[4] ) );
      send_to_char( buf, ch );            
      break;
    case ITEM_BOOK:
      sprintf(buf, "It can hold %s item%s.\n\r",
	      idscale_compare( contnum_scale, obj->value[0] * 100 / can_carry_n(ch) ),
	      ( (obj->value[0] == 1) ? "" : "s" ) );
      send_to_char( buf, ch );            
      break;

    case ITEM_WEAPON:
      strcpy( buf2, 
	      idscale_compare( weapdam_scale, (1 + obj->value[2]) * obj->value[1] / 2 ) );
      sprintf(buf, "It is a%s %s %s.\n\r",
	      ( buf2[0] == 'a' || buf2[0] == 'e' || buf2[0] == 'i' || buf2[0] == 'o' ||
		buf2[0] == 'u' ) ? "n" : "", buf2,
	      ( obj->value[0] == WEAPON_EXOTIC ? "exotic weapon" :
		exact_flag_string( weapon_class, obj->value[0] ) ) );
      send_to_char( buf, ch );
      break;

    case ITEM_ARMOR:
      break;
    }

  if (!obj->enchanted)
    afsrc = obj->pIndexData->affected;
  else
    afsrc = obj->affected;

  buf2[0] = '\0';
  for ( paf = afsrc; paf != NULL; paf = paf->next )
    {
      if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	  /*** For str/int/wis/dex/con/hitroll/damroll/ac boosts, use stat table */
	  if ( ( paf->location >= APPLY_STR && paf->location <= APPLY_CON )
	       ||     paf->location == APPLY_HITROLL || paf->location == APPLY_DAMROLL )
	    sprintf( buf, "'%s %s' ",
		     idscale_compare( statapply_scale, paf->modifier ),
		     exact_flag_string( ic_apply_flags, paf->location ) );
	  else if ( paf->location == APPLY_AC )
	    sprintf( buf, "'%s %s' ",
		     idscale_compare( statapply_scale, paf->modifier / -5 ),
		     exact_flag_string( ic_apply_flags, paf->location ) );
	  /*** For hp/mana/move boosts, use hp table */
	  else if ( paf->location == APPLY_HIT
		    ||   paf->location == APPLY_MANA
		    ||   paf->location == APPLY_MOVE
		    ||   paf->location == APPLY_BLOOD )
	    sprintf( buf, "'%s %s' ",
		     idscale_compare( hpmpapply_scale, paf->modifier ),
		     exact_flag_string( ic_apply_flags, paf->location ) );
	  /*** For saves, use inverse stat table */
	  else if ( ( paf->location >= APPLY_SAVES && paf->location <= APPLY_SAVING_SPELL ) )
	    sprintf( buf, "'%s %s' ",
		     idscale_compare( statapply_scale, paf->modifier * -1 ),
		     exact_flag_string( ic_apply_flags, paf->location ) );
	  /*** For sex, class, lvl, age, xp, height, weight, gold (unused ones) */
	  else
	    sprintf( buf, "'modifies %s by %d points'",
		     exact_flag_string( ic_apply_flags, paf->location ),
		     paf->modifier );
	  /*          buf[0] = UPPER(buf[0]);
		      send_to_char(buf,ch);*/
	  strcat(buf2,buf);
	  /*
            if ( IS_IMMORTAL( ch ) )
            {
            if (paf->bitvector)
            {
	    send_to_char( "Your spidey senses also detect the following:\n\r", ch );
				
	    switch(paf->where)
	    {
	    case TO_AFFECTS:
	    sprintf(buf,"Adds the %s affect.\n",
	    flag_string( affect_flags, paf->bitvector));
	    break;
	    case TO_OBJECT:
	    sprintf(buf,"Adds %s object flag.\n",
	    flag_string( extra_flags, paf->bitvector));
	    break;
	    case TO_WEAPON:
	    sprintf(buf,"Adds %s weapon flags.\n",
	    flag_string( weapon_type2, paf->bitvector));
	    break;
	    case TO_IMMUNE:
	    sprintf(buf,"Adds immunity to %s.\n",
	    flag_string( imm_flags, paf->bitvector));
	    break;
	    case TO_RESIST:
	    sprintf(buf,"Adds resistance to %s.\n\r",
	    flag_string( res_flags, paf->bitvector));
	    break;
	    case TO_VULN:
	    sprintf(buf,"Adds vulnerability to %s.\n\r",
	    flag_string( vuln_flags, paf->bitvector));
	    break;
	    default:
	    sprintf(buf,"Unknown bit %d: %d\n\r",
	    paf->where,paf->bitvector);
	    break;
	    }
	    send_to_char(buf,ch);
            }
            } */
	}
    }
  if (!IS_NULLSTR(buf2) && !is_blankstring(buf2) )
    {
      sprintf(buf,"It %s.\n\r", listize(buf2, TRUE) );
      send_to_char(buf,ch);
    }
  return;
}

void do_spidey( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  char arg[MAX_STRING_LENGTH];

  argument = one_argument( argument, arg );

  if ( IS_NULLSTR( arg ) )
    {
      send_to_char( "What do you wish to examine?\n\r", ch );
      return;
    }
  if ( ( obj = get_obj_carry( ch, arg, ch, TRUE ) ) == NULL )
    {
      send_to_char( "You don't have that item.\n\r", ch );
      return;
    }
  if ( IS_NPC( ch ) )
    {
      send_to_char( "You use your immense knowledge as a mobile to probe the object!\n\r", ch );
      do_ostat( ch, arg );
      return;
    }

  show_obj_stats( ch, obj );
  return;
}

void mod_who ( void ) 
{
  char buf[MAX_STRING_LENGTH];
  char level[10];
  char pretitle[256];
  char chguild[64];
  char chtitle[256];
  struct tm *ltime;
  DESCRIPTOR_DATA *d;
  int nMatch;
  FILE *fp;

  if ( (fp = fopen( "wholist.info", "w" ) ) == NULL )
    {
      bug( "mod_who: Couldn't open who list for write.", 0 );
      return;
    }

  nMatch = 0;
  buf[0] = '\0';
  for ( d = descriptor_list; d != NULL; d = d->next )
    {
      CHAR_DATA *wch;
      char const *class, *race;
 
      if ( d->connected != CON_PLAYING )
	continue;

      wch   = ( d->original != NULL ) ? d->original : d->character;

      if ( wch->invis_level  > 50
	   ||   wch->rinvis_level > 50
	   ||   wch->incog_level  > 50)
	continue;

      if ( IS_AFFECTED3( wch, AFF_VEIL ) ) 
	continue;
 
      nMatch++;
 
      /*
       * Figure out what to print for class.
       */
      class = class_table[wch->class].who_name;
      switch ( wch->level )
	{
	default: break;
	  {
	  case MAX_LEVEL - 0 : class = "IMP";     break;
	  case MAX_LEVEL - 1 : class = "CRE";     break;
	  case MAX_LEVEL - 2 : class = "SUP";     break;
	  case MAX_LEVEL - 3 : class = "DEI";     break;
	  case MAX_LEVEL - 4 : class = "GOD";     break;
	  case MAX_LEVEL - 5 : class = "IMM";     break;
	  case MAX_LEVEL - 6 : class = "DEM";     break;
	  case MAX_LEVEL - 7 : class = "ANG";     break;
	  case MAX_LEVEL - 8 : class = "AVA";     break;
	  }
	}

      if (wch->level < 11)
	sprintf(level,"NB");
      else if ( abs(50 - wch->level) <= 6 )
	sprintf(level,"IR");
      else if (wch->level > 50) 
	sprintf(level,"AR");
      else 
	sprintf(level,"BR");

      /* scramble race/class */
      if ( global_config_aprilfools && wch->level < LEVEL_IMMORTAL )
        {
	  int offset, new;
	  offset = strlen(wch->name);
	  new = ( wch->race + offset ) % ( MAX_PC_RACE - 1 ) + 1;
	  race = pc_race_table[new].who_name;
	  new = ( wch->class + offset ) % MAX_CLASS;
	  class = class_table[new].who_name;
        }
      else
	race = pc_race_table[wch->race].who_name;

      if (wch->pcdata->pretitle[0] == '\0') 
	sprintf(pretitle, "%2s %6s %s", level, race, class );
      else
	sprintf(pretitle, strip_color(wch->pcdata->pretitle));

      strcpy( chguild, strip_color(guild_table[wch->guild].who_name) );
      strcpy( chtitle, strip_color(wch->pcdata->title ));
      fprintf( fp, "[%13s] %s%s%s%s%s%s%s%s\n",
	       pretitle,
	       chguild,
	       IS_SET(wch->act_bits,PLR_QUEST) ? "(QUEST) " : "",
	       IS_SET(wch->comm, COMM_AFK) ? "[AFK] " : "",
	       IS_AFFECTED2(wch,AFF_WANTED) ? "(WANTED) " : "",
	       IS_SET(wch->act_bits,ACT_NOAPPROVE) ? "(UA) " : "",
	       IS_SET(wch->act_bits,PLR_NEWBIE) ? "[+] " : "",
	       wch->name,
	       chtitle );

    }

  fprintf( fp, "\n\rPlayers found: %d\n", nMatch );

  ltime = localtime( &current_time );
  strftime( pretitle, 256, "Last updated on %A, %B %d at %I:%M %p.", ltime );
  fprintf( fp, pretitle );
  fclose(fp);

  return;
}

void do_dumpwho( CHAR_DATA *ch, char *argument)
{
  mod_who();
  send_to_char("Ok.\n\r", ch );
  return;
}

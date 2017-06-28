/* Vampire stuff */
/***************************************************************************
 * Vampires by Ted Elliott 4/18/99
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

extern void unshift(CHAR_DATA *ch);
extern bool check_charmlist( CHAR_DATA *ch, CHAR_DATA *vch );

void    raw_kill        args( ( CHAR_DATA *victim ) );
bool    check_social    args( ( CHAR_DATA *victim, char *command, char *argument ) );
bool    add_blood       args( ( CHAR_DATA* ch, sh_int blood ) );

bool add_blood( CHAR_DATA* ch, sh_int blood )
{
  AFFECT_DATA* torpor = NULL;
  sh_int old_blood;
  sh_int current_max;

  old_blood = ch->blood;
  current_max = ch->max_blood * ch->blood_percentage / 100;

  ch->blood += blood;
  ch->blood = UMIN( ch->blood, current_max );

  torpor = affect_find( ch->affected, -20 );
  if( torpor )
    {
      send_to_char( "The blood magic holding your being together strengthens.\n\r", ch );
      affect_remove( ch, torpor );
    }

  return( ch->blood > old_blood );
}

void do_offer( CHAR_DATA* ch, char* argument )
{
  char arg[ MAX_INPUT_LENGTH ] = "";
  AFFECT_DATA af;
  AFFECT_DATA *paf;
  CHAR_DATA* target = NULL;

  if( IS_NPC( ch ) )
    {
      send_to_char( "You have nothing to offer.\n\r", ch );
      return;
    }

  one_argument( argument, arg );
  if( *arg == '\0' )
    {
      send_to_char( "To whom do you wish to offer yourself?\n\r", ch );
      return;
    }

  target = get_char_world( ch, arg, FALSE );
  if( target == NULL || IS_NPC( target ) )
    {
      send_to_char( "That person is not here to feed from you.\n\r", ch );
      return;
    }

  if ( target == ch )
  {
      send_to_char( "That won't do you any good.\n\r", ch );
      return;
  }

  act( "You offer yourself to $M.", ch, NULL, target, TO_CHAR );
  act( "$n offers $mself to $N!", ch, NULL, target, TO_NOTVICT );

  if( !IS_SET( target->shiftbits, PERM_VAMP ) &&
      !IS_SET( target->shiftbits, TEMP_VAMP ) )
      act( "$n boldly offers $mself to you.  Strange.", ch, NULL, target, TO_VICT );
  else
      act( "$n offers $mself as a feast for you!", ch, NULL, target, TO_VICT );

  /* If player wants to offer to another person, this revokes any outstandinf offerings */
  if ( ( paf = affect_find(ch->affected, -5) ) )
        affect_remove( ch, paf );

  ch->pcdata->offering = target;

  af.where     = TO_AFFECTS;
  af.whichaff  = AFF3;
  af.type      = -5;
  af.level     = ch->level;     
  af.duration  = 2;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_OFFERING;
  affect_to_char3(ch,&af);
}

void do_drain( CHAR_DATA* ch, char* argument )
{
  char arg[ MAX_INPUT_LENGTH ] = "";
  CHAR_DATA* victim = NULL;

  if( IS_NPC( ch ) )
    {
      send_to_char( "You haven't any fangs.\n\r", ch );
      return;
    }

  if( in_fightlag(ch) )
    {
      send_to_char( "Your blood runs too fast to accept the offering.\n\r", ch );
      return;
    }

  if( !IS_SET( ch->shiftbits, PERM_VAMP ) &&
      !IS_SET( ch->shiftbits, TEMP_VAMP ) )
    {
      if( !check_social( ch, "drain", argument ) )
	send_to_char( "Huh?\n\r", ch );

      return;
    }

  one_argument( argument, arg );
  if( *arg == '\0' )
    {
      send_to_char( "Who do you want to drain?\n\r", ch );
      return;
    }

  victim = get_char_room( ch, arg, FALSE );
  if( victim == NULL )
    {
      send_to_char( "That cattle specimen is not here.\n\r", ch );
      return;
    }

  if( IS_NPC( victim ) )
    {
      send_to_char( "That is not a suitable food source.\n\r", ch );
      return;
    }

  if( victim->pcdata->offering == NULL ||
      victim->pcdata->offering != ch )
    {
      send_to_char( "That food source has not offered you a meal.\n\r", ch );
      return;
    }

  victim->hit /= 2;
  ch->blood += ( victim->level / 2 );
  ch->blood = UMIN( ch->blood, ch->max_blood );

  send_to_char( "You feel drained as a vampire accepts your offering.\n\r", victim );
  send_to_char( "You feel invigorated by the blood offered to you.\n\r", ch );
  act( "$n sips blood from the neck of $N!", ch, 0, victim, TO_NOTVICT );

  ch->pcdata->lastfight = current_time;
  return;
}




void do_embrace(CHAR_DATA *ch, char *argument) 
{
  char arg[MAX_INPUT_LENGTH] = "";
  CHAR_DATA *victim = NULL;
/*int chance = 80; */

  if( !IS_SET( ch->shiftbits, PERM_VAMP ) )
    {
      if( IS_SET( ch->shiftbits, TEMP_VAMP ) )
	{
	  send_to_char( "You are but a minion, you don't know how to embrace.\n\r", ch );
	}

      if( !check_social( ch, "embrace", argument ) )
	{
	  send_to_char( "Huh?\n\r", ch );
	}

      return;
    }

  one_argument( argument, arg );
  if (arg[0] == '\0') {
    send_to_char("Who do you want to embrace?\n\r",ch);
    return;
  }

  if ( ( victim = get_char_room(ch, arg, FALSE)) == NULL) {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }

  if ( victim->position != POS_UNCONSCIOUS ) {
    send_to_char("They're not unconscious.\n\r",ch);
    return;
  }

  if ( victim->level < 30 ) {
    send_to_char("They couldn't survive at such a young age!\n\r",ch);
    return;
  }

  if ( !IS_IMMORTAL(ch)
  &&  (IS_SET(victim->shiftbits, SHIFT_POTENTIAL)
  ||   IS_SET(victim->shiftbits, PERM_VAMP)
  ||   victim->race == race_lookup( "githyanki" ) 
  ||   victim->race == race_lookup( "lich" )
  ||   victim->class == class_lookup("cleric")) )
  {

    send_to_char("You fail, killing your victim!\n\r",ch);
    act_new( "$n murders you in cold blood!",ch,NULL,victim,
	     TO_VICT,POS_DEAD,0 );
    act( "$n murders $N in cold blood!",  ch, NULL, victim, 
	 TO_NOTVICT);
    raw_kill( victim );

    sprintf( log_buf, "%s killed by an attempted embrace by %s at %d",
	     victim->name,
	     (IS_NPC(ch) ? ch->short_descr : ch->name),
	     ch->in_room->vnum );

    log_string( log_buf );

    /*
    wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);
    */

    return;

  }
  else {
    ch->mana /= 2;
    ch->move /= 2;
    ch->blood /= 2;
    act("You bestow your embrace upon $N, leaving yourself feeling drained.",
	ch,NULL,victim,TO_CHAR);
    act_new("You feel a sharp pain in your neck, as something strange flows into you.",
	    ch,NULL,victim,TO_VICT,POS_DEAD,0);
    act("$n stoops over $N's lifeless body.", ch,NULL,victim,TO_ROOM);
        /* Lilith can embrace anyone, including patriarchs/minions */
        if ( !IS_NULLSTR(victim->patriarch) )
           free_string( victim->patriarch ); 
    victim->patriarch = str_dup(ch->name);
    if ( victim->race == race_lookup( "werekin" ) )
    {
        victim->blood = 1;
        victim->max_blood = 1;
        victim->blood_percentage = 100;
    }
    else
    {
        victim->blood = victim->level;
        victim->max_blood = 250;
        victim->blood_percentage = 100;
    }
    SET_BIT(victim->shiftbits, TEMP_VAMP);
    victim->vuln_flags |= VULN_WOOD|VULN_LIGHT;
    return;
  }
}    

void do_alliance(CHAR_DATA *ch, char *argument) 
{

    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;


    if (!IS_SET(ch->shiftbits, PERM_VAMP) &&
         IS_SET(ch->shiftbits, TEMP_VAMP)) {
        send_to_char("You are but a minion, you cannot make alliances.\n\r",
        ch);
        return;
    }

    if (!IS_SET(ch->shiftbits, PERM_VAMP)) {
           send_to_char("Huh?\n\r",ch);
        return;
    }

    one_argument( argument, arg );
    if (arg[0] == '\0') {
        send_to_char("Who do you want to become your ally?\n\r",ch);
        return;
    }

    if ( ( victim = get_char_room(ch, arg, FALSE)) == NULL) {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }
    else {
    act("$N has now become your ally.",
         ch,NULL,victim,TO_CHAR);
    act_new("You have now allied with $n.",
        ch,NULL,victim,TO_VICT,POS_DEAD,0);
    victim->patriarch = str_dup(ch->name);
    return;

    }
}


void do_unally(CHAR_DATA *ch, char *argument)
{

  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  
  if( !IS_SET(ch->shiftbits, PERM_VAMP) &&
      IS_SET(ch->shiftbits, TEMP_VAMP)) 
    {
      send_to_char("You are but a minion, you cannot break alliances.\n\r",
		   ch);
      return;
    }

  if (!IS_SET(ch->shiftbits, PERM_VAMP)) 
    {
      send_to_char("Huh?\n\r",ch);
      return;
    }

  one_argument( argument, arg );
  if (arg[0] == '\0') 
    {
      send_to_char("Who do you want to break alliance with?\n\r",ch);
      return;
    }

  if ( ( victim = get_char_room(ch, arg, FALSE)) == NULL) 
    {
      send_to_char("They aren't here.\n\r",ch);
      return;
    }

  act("You have broken alliance with $N.",
      ch,NULL,victim,TO_CHAR);
  
  act_new("You are no longer allied with $n.",
	  ch,NULL,victim,TO_VICT,POS_DEAD,0);
  
  free_string(victim->patriarch);
  victim->patriarch = &str_empty[ 0 ];
  return;


}


void do_minion(CHAR_DATA *ch, char *argument) {

  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;


  if ( IS_NULLSTR(ch->patriarch) )
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
 
  if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOMINION))
	{
	  send_to_char("Minion channel is now ON\n\r",ch);
	  REMOVE_BIT(ch->comm,COMM_NOMINION);
	}
      else
	{
	  send_to_char("Minion channel is now OFF\n\r",ch);
	  SET_BIT(ch->comm,COMM_NOMINION);
	}
      return;
    }
  REMOVE_BIT(ch->comm,COMM_NOMINION);

  sprintf(buf,"You minion '{%c%s{x'\n\r",ch->colors[C_MINION],argument);
  send_to_char(buf,ch);

  for ( d = descriptor_list; d != NULL; d = d->next )
    {
      victim = d->original ? d->original : d->character;

      if ( d->connected == CON_PLAYING
      &&  !IS_SET(victim->comm,COMM_NOMINION)
      &&   victim != ch
      &&  !str_cmp(ch->patriarch,victim->patriarch))
	{
	  if (IS_SET(ch->shiftbits, PERM_VAMP))
	    sprintf( buf, "[Minion] {%c%s %s{x: '%s'{x\n\r",
		     victim->colors[C_MINION],
		     ( ( ch->sex == 0 || ch->sex == 1 ) ? "Master" : "Mistress" ),
		     capitalize( PERS2(ch, d->character)),
		     argument);
	  else
	    sprintf( buf, "[Minion] {%c%s{x: '%s'{x\n\r",victim->colors[C_MINION],
		     capitalize( PERS2(ch, d->character)),
		     argument);
	  send_to_char(buf,victim);
        }        
    }
  return;
}


void vampire_enhance( CHAR_DATA *ch ) {

     ch->affected_by |= AFF_NIGHTHASTE;
     ch->res_flags |= RES_POISON|RES_DISEASE;
     send_to_char("You feel yourself moving more quickly.\n\r",ch);
     return;
}

void vampire_unenhance( CHAR_DATA *ch ) {

     ch->affected_by &= ~(AFF_NIGHTHASTE);
     ch->res_flags &= ~(RES_POISON|RES_DISEASE);
     ch->res_flags |= race_table[ch->race].res;
     send_to_char("You feel yourself slow down.\n\r",ch);
     return;
}

void do_bloodpool( CHAR_DATA *ch, char *argument )
{
  char blood_buffer[ MAX_STRING_LENGTH ] = "";
  char blood_line[ MAX_STRING_LENGTH ] = "";
  char* buff_ins_point = blood_buffer;
  char* line_ins_point = blood_line;
  int blood_per;
  int available;
  int available_per;
  int total_blood_per;
  int i;
  
  if( !IS_SET( ch->shiftbits, PERM_VAMP ) &&
      !IS_SET( ch->shiftbits, TEMP_VAMP ) )
    {
      send_to_char( "Huh?\n\r", ch );
      return;
    }

  if( IS_AFFECTED2( ch, AFF_GHOST ) )
    {
      send_to_char( "How can you have blood without a body?\n\r", ch );
      return;
    }
  
  available = ch->max_blood * ch->blood_percentage / 100;
  available_per = available * 100 / ch->max_blood;
  blood_per = ch->blood * 100 / available;

  total_blood_per = ch->blood * 100 / ch->max_blood;

  buff_ins_point += sprintf( buff_ins_point, "Current blood  : {G%d {w(%d%%){x\n\r", ch->blood, blood_per );
  buff_ins_point += sprintf( buff_ins_point, "Maximum blood  : {G%d{x\n\r", ch->max_blood );
  buff_ins_point += sprintf( buff_ins_point, "Available blood: {G%d {w(%d%%){x\n\r\n\r", 
			     available, 
			     available_per );
  
  buff_ins_point += sprintf( buff_ins_point, "{G<========={rBlood Pool{G==========>{x\n\r" );
  line_ins_point += sprintf( line_ins_point, "{Y<=(" );

  for( i = 0; i < 100; i += 4 )
    {
      if( i < total_blood_per )
	{
	  line_ins_point += sprintf( line_ins_point, "{r*" );
	}
      else if( i < ch->blood_percentage )
	{
	  line_ins_point += sprintf( line_ins_point, "{D*" );
	}
      else
	{
	  line_ins_point += sprintf( line_ins_point, "{b-" );
	}

      /*
      if( i > ch->blood_percentage )
	{
	  line_ins_point += sprintf( line_ins_point, "{D-" );
	}
      else if( i > ( blood_per + ch->blood_percentage ) )
	{
	  line_ins_point += sprintf( line_ins_point, "{r*" );
	}
      else
	{
	  line_ins_point += sprintf( line_ins_point, "{b-" );
	}
      */
    }

  line_ins_point += sprintf( line_ins_point, "{Y)=>{x" );

  buff_ins_point += sprintf( buff_ins_point, "%s\n\r", blood_line );
  buff_ins_point += sprintf( buff_ins_point, "%s\n\r", blood_line );

  send_to_char( blood_buffer, ch );

}

void do_bloodpercent( CHAR_DATA* ch, char* argument )
{
  char victim_name[ MAX_INPUT_LENGTH ] = "";
  char percent_name[ MAX_INPUT_LENGTH ] = "";
  CHAR_DATA* victim = NULL;
  int percent = 0;
  int max_blood = 0;

  argument = one_argument( argument, victim_name );
  argument = one_argument( argument, percent_name );

  if( !IS_SET( ch->shiftbits, PERM_VAMP ) )
    {
      if( IS_SET( ch->shiftbits, TEMP_VAMP ) )
	{
	  send_to_char( "Only your patriarch can control the level of your blood magic.\n\r", ch );
	}
      else
	{
	  send_to_char( "Huh?\n\r", ch );
	}

      return;
    }

  if( victim_name[ 0 ] == '\0' )
    {
      send_to_char( "Change who?\n\r", ch );
      return;
    }

  victim = get_char_room( ch, victim_name, TRUE );

  if( victim == NULL || IS_NPC( victim ) )
    {
      send_to_char( "They're not here.\n\r", ch );
      return;
    }

  if( percent_name[ 0 ] == '\0' )
    {
      percent = 100;
    }
  else
    {
      percent = atoi( percent_name );
    }

  if( percent < 1 || percent > 100 )
    {
      send_to_char( "Valid percentage between 1 and 100.\n\r", ch );
      return;
    }

  if( !IS_SET( victim->shiftbits, TEMP_VAMP ) &&
      !IS_SET( victim->shiftbits, PERM_VAMP ) )
    {
      send_to_char( "That person has no blood magic.\n\r", ch );
      return;
    }

  if( strcasecmp( ch->name, victim->patriarch ) != 0 )
    {
      send_to_char( "That vampire's blood magic is not under your control.\n\r", ch );
      return;
    }

  victim->blood_percentage = percent;
  max_blood = victim->max_blood * percent / 100;
  victim->blood = UMIN( victim->blood, max_blood );

  printf_to_char( ch, "You have restricted %s to %d%% of normal blood magic.\n\r",
		  victim->name,
		  percent );

  printf_to_char( victim, "Your patriarch has restricted you to %d%% of your normal blood magic!\n\r",
		  percent );

}


void do_feast( CHAR_DATA *ch, char *argument ) 
{
  int hp = 0;
  int mana = 0;
  int blood = 0;
  char arg[MAX_INPUT_LENGTH] = "";
  OBJ_DATA* obj = NULL;
  CHAR_DATA* victim = NULL;

  if( !IS_SET(ch->shiftbits,PERM_VAMP) &&
      !IS_SET(ch->shiftbits,TEMP_VAMP))
    {
      send_to_char("Huh?\n\r",ch);
      return;
    }

  if (IS_AFFECTED2(ch,AFF_GHOST))
    {
      send_to_char("Your a ghost!.\n\r",ch);
      return;
    }

  one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
      send_to_char("What do you want to feast on?\n\r",ch);
      return;
    }

  if ( ( victim = get_char_room(ch,arg, TRUE) ) != NULL ) 
    {
	
      if ( victim->position != POS_UNCONSCIOUS ) 
	{
	  send_to_char("You try but they resist.\n\r",ch);
	  act("$n tries to bite you!",ch,NULL,victim,TO_VICT);
	  return;
	}

      act( "You feast on $N, killing $M!", ch, NULL, victim, TO_CHAR );
      act( "$n feasts on $N, killing $M!", ch, NULL, victim, TO_ROOM );

      act_new( "You feel $n suck the life right out of you!",ch,NULL,victim,
	       TO_VICT,POS_DEAD,0 );

      raw_kill( victim );

      sprintf( log_buf, "%s killed by %s at %d",
	       victim->name,
	       (IS_NPC(ch) ? ch->short_descr : ch->name),
	       ch->in_room->vnum );

      log_string( log_buf );
      wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

      hp = 4 * victim->level;
      mana = 2 * victim->level;
      blood = 2 * victim->level;
    }
  else 
    {
      obj = get_obj_list( ch, arg, ch->in_room->contents, TRUE );
      if ( obj == NULL )
	{
	  send_to_char( "You can't find it.\n\r", ch );
	  return;
	}

      if ( obj->item_type == ITEM_CORPSE_PC ) {
	send_to_char( "You prefer live victims, over dead.\n\r",ch);
	return;
      }

      if ( obj->item_type != ITEM_CORPSE_NPC ) 
	{
	  send_to_char("You can't feast on that.\n\r",ch);
	  return;
	}

      act( "$n feasts on $p!", ch, obj, NULL, TO_ROOM );
      extract_obj( obj );

      hp = 2 * obj->level;
      mana = obj->level;
      blood = obj->level;

      
    }

  if( add_blood( ch, blood ) )
    {
      ch->hit += hp;
      ch->hit = UMIN( ch->hit, ch->max_hit );

      ch->mana += mana;
      ch->mana = UMIN( ch->mana, ch->max_mana );

      send_to_char( "You satisfy your hunger for now...\n\r", ch );
    }
  else
    {
      send_to_char( "You are too full to drink any more.\n\r", ch );
    }
}

void vampire_shift(CHAR_DATA *ch, char *argument) {

    AFFECT_DATA af;
    char arg[MAX_STRING_LENGTH];

    if (IS_SET(ch->shiftbits, BAT_VAMP) ||
	IS_SET(ch->shiftbits, MIST_VAMP)) 
    {
	send_to_char("You are already in an alternate form.\n\r",ch);
	return;
    }

    if (ch->fighting != NULL) {
	send_to_char("There is too much going on to shift right now!\n\r",
	ch);
	return;
    }

    one_argument( argument, arg );
 
    if (arg[0] == '\0') {
	send_to_char("Do you want to shift into a bat, or mist?\n\r", ch);
	return;
    }

    if (!str_prefix(arg,"mist")) {

	act("$n dissolves into mist.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You dissolve into mist.\n\r",ch);

	ch->max_hit += 100;
	ch->hit	    += 100;
	ch->max_mana += 50;
	ch->mana     += 50;
	SET_BIT(ch->shiftbits,MIST_VAMP);
	ch->res_flags |= RES_PIERCE|RES_BASH|RES_SLASH;
        ch->vuln_flags |= VULN_WOOD|VULN_HOLY;
	ch->affected_by |= AFF_PASS_DOOR;

	af.where     = TO_AFFECTS;
	af.type      = -3;
        af.level     = ch->level;     
        af.duration  = 15;
        af.modifier  = 0;
        af.location  = APPLY_NONE;
        af.bitvector = AFF_SHIFT_ON;
        affect_to_char(ch,&af);
    	return;
    }
    else if (!str_prefix(arg,"bat")) {

        act("$n turns $mself into a bat.",ch,NULL,NULL,TO_ROOM);
        send_to_char("You turn yourself into a bat.\n\r",ch);
        
        ch->max_hit += 50;
        ch->hit     += 50;
        ch->max_mana += 100;
        ch->mana     += 100;
        SET_BIT(ch->shiftbits,BAT_VAMP);
        ch->res_flags |= RES_MAGIC;
        ch->vuln_flags |= VULN_WOOD|VULN_HOLY;
        ch->affected_by |= AFF_FLYING|AFF_SNEAK|AFF_INFRARED;
            
        af.where     = TO_AFFECTS;
        af.type      = -3;
        af.level     = ch->level;
        af.duration  = 15;
        af.modifier  = 0;
        af.location  = APPLY_NONE;
        af.bitvector = AFF_SHIFT_ON;
        affect_to_char(ch,&af);
        
        return;
   }
   else {
	send_to_char("That is not an available form.\n\r",ch);
	return;
   }

}

void shift_strip(CHAR_DATA *ch)
{
AFFECT_DATA *paf;

          paf = affect_find(ch->affected, -1); 
          if ( paf )
          affect_remove( ch, paf );
         return;
}

void shift_strip2(CHAR_DATA *ch)
{
AFFECT_DATA *paf;

          paf = affect_find(ch->affected, -2); 
          if ( paf )
          affect_remove( ch, paf );
         return;
}

void do_unshift( CHAR_DATA *ch, char *argument ) 
{

  AFFECT_DATA *paf;

  if (IS_SET(ch->shiftbits,SHIFT_HALF) || IS_SET(ch->shiftbits,SHIFT_FULL))
    {
      if (IS_AFFECTED(ch,AFF_SHIFT_PERM))
        {
          send_to_char("The beast rages out of control and you cannot return to your normal form!\n\r",ch);
          return;
        }
      else
        {         
          paf = affect_find(ch->affected, -1); 
          if ( paf )
	    affect_remove( ch, paf );
	  return;
	}
    }
  
  if (!IS_SET(ch->shiftbits,TEMP_VAMP) && 
      !IS_SET(ch->shiftbits,PERM_VAMP) ) 
    {
      if(strcmp(argument,"auto")) 
	send_to_char("Huh!?\n\r",ch);
      return;
    }

  if (!IS_SET(ch->shiftbits,BAT_VAMP) &&
      !IS_SET(ch->shiftbits,MIST_VAMP) ) 
    {
      send_to_char("You aren't shifted.\n\r",ch);
      return;
    }
  
  paf = affect_find( ch->affected, -3 );
  if( paf )
    {
      affect_remove( ch, paf );
    }
  else
    bug( "do_unshift() bad vamp affect.", 0 );
  
  return;
}

void vampire_unshift( CHAR_DATA *ch ) 
{
  if (IS_SET(ch->shiftbits,BAT_VAMP)) 
    {
      ch->max_hit -= 50;
      if (ch->hit > 50)
	ch->hit -= 50;
      if (ch->hit > ch->max_hit)
	ch->hit = ch->max_hit;
      
      ch->max_mana -= 100;
      if (ch->mana > 100)
	ch->mana -= 100;
      
      if (ch->mana > ch->max_mana)
	ch->mana = ch->max_mana;
      
      ch->affected_by &= ~(AFF_FLYING|AFF_SNEAK|AFF_INFRARED);
      ch->res_flags -= RES_MAGIC; 
      ch->vuln_flags -= VULN_WOOD|VULN_HOLY;
      act("$n shimmers and returns to $s normal form.",ch,NULL,NULL,
	  TO_ROOM);
      send_to_char("You shimmer and return to your normal form.\n\r",ch);
      
    }
  else if (IS_SET(ch->shiftbits, MIST_VAMP)) 
    {
      ch->max_hit -= 100;
      if (ch->hit > ch->max_hit)
	ch->hit = ch->max_hit;
      if (ch->hit > 100)
	ch->hit -= 100;
        
      ch->max_mana -= 50;
      if (ch->mana > 50)
	ch->mana -= 50;
      
      if (ch->mana > ch->max_mana)
	ch->mana = ch->max_mana;
      
      ch->affected_by &= ~AFF_PASS_DOOR;
      ch->res_flags -= RES_PIERCE|RES_BASH|RES_SLASH;
      ch->vuln_flags -= VULN_WOOD|VULN_HOLY;
      act("$n solidifies into $s natural form.",ch,NULL,NULL,
	  TO_ROOM);
      send_to_char("You solidify into your natural form.\n\r",ch);
      
    }
    else 
      {
	bug("vampire_unshift: Shiftbits not set.", 0 );
      }

  /* RESTORE JUST IN CASE THEY WERE TAKEN AWAY */
  ch->res_flags |= race_table[ch->race].res;
  ch->vuln_flags |= race_table[ch->race].vuln;
  ch->affected_by |= race_table[ch->race].aff;

  REMOVE_BIT( ch->shiftbits, BAT_VAMP );
  REMOVE_BIT( ch->shiftbits, MIST_VAMP );
  WAIT_STATE( ch, 24 );

}

void spell_skeletons(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *mob;
    AFFECT_DATA af;
    MOB_INDEX_DATA *pmobindex;
    short indexnum;
    int loop;

    if (!IS_SET(ch->shiftbits,TEMP_VAMP) 
    &&  !IS_SET(ch->shiftbits,PERM_VAMP) 
    &&  !IS_NPC(ch)) 
    {
        ch->pcdata->learned[sn] = 0;
        send_to_char("You don't know any spells of that name.\n\r",ch);
        return;
    }

    if ( IS_AFFECTED2( ch, AFF_SUMSICK ) )  
    {
        send_to_char("You cannot summon another army so quickly!\n\r",ch);
        return;
    }

    if (ch->pet != NULL || has_summonedpets( ch ) )
    {
        send_to_char("You already have creatures following you!\n\r",ch);
        return;
    }

    if( ch->blood < 100 )
    {
        send_to_char( "You haven't enough blood to work this magic.\n\r", ch );
        return;
    }


    if (time_info.hour >= 6 && time_info.hour < 20) 
    {
        send_to_char("They won't survive in the sunlight.\n\r", ch);
        return;
    }

    ch->blood -= 100;
    if (ch->level > 50)
        indexnum = MOB_VNUM_SKELETAL;
    else if (ch->level > 45)
        indexnum = MOB_VNUM_SKELETAL;
    else
        indexnum = MOB_VNUM_SKELETAL;

    pmobindex = get_mob_index(indexnum);

    if (!pmobindex)
        return;

    act( "Ancient bones wrench themselves from the ground to form skeletons.",
        ch, NULL, NULL, TO_ROOM );
    act( "Ancient bones wrench themselves from the ground to form skeletons!",
        ch, NULL, NULL, TO_CHAR );

    for (loop = 0; loop < 5; loop++)
    {
        mob = create_mobile(pmobindex);
        char_to_room(mob,ch->in_room);

        mob->max_hit = ch->max_hit / 2;
        mob->hit = mob->max_hit;
        mob->level = ch->level;

        add_follower(mob, ch);
        mob->leader = ch;
/*      ch->pet  = mob;*/
        SET_BIT(mob->act_bits, ACT_PET);  
        SET_BIT(mob->affected_by2, AFF2_SUMMONEDPET );

        af.where     = TO_AFFECTS;
        af.type      = sn;
        af.level     = level;
        af.duration  = 20;
        af.location  = 0;
        af.modifier  = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char( mob, &af );
    }
    spell_summon_sick(gsn_sumsick,level,ch,(void *) ch,TARGET_CHAR);
    return;
}

void spell_sun_skeletals(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    AFFECT_DATA af;
    MOB_INDEX_DATA *pMobIndex;
    int i, n;

    if (!IS_SET(ch->shiftbits,TEMP_VAMP) 
    &&  !IS_SET(ch->shiftbits,PERM_VAMP) 
    &&  !IS_NPC(ch)) 
    {
        ch->pcdata->learned[sn] = 0;
        send_to_char("You don't know any spells of that name.\n\r",ch);
        return;
    }

    if ( IS_AFFECTED2( ch, AFF_SUMSICK ) )  
    {
        send_to_char("You cannot summon another army so quickly!\n\r",ch);
        return;
    }

    if (ch->pet != NULL || has_summonedpets( ch ) )
    {
        send_to_char("You already have creatures following you!\n\r",ch);
        return;
    }

    if( ch->blood < 100 )
    {
        send_to_char( "You haven't enough blood to work this magic.\n\r", ch );
        return;
    }


    if (time_info.hour < 6 || time_info.hour > 19) 
    {
        send_to_char( "The skeletons can only be raised during daytime.\n\r", 
            ch );
        return;
    }


    if ((pMobIndex = get_mob_index(MOB_VNUM_SUNSKELLY)) == NULL )
    {
        bug( "spell_sun_skeletals: cannot find sun skelly mob vnum %d",
            MOB_VNUM_SUNSKELLY );
        return;
    }

    act( "Ancient bones wrench themselves from the ground to form skeletons.",
        ch, NULL, NULL, TO_ROOM );
    act( "Ancient bones wrench themselves from the ground to form skeletons!",
        ch, NULL, NULL, TO_CHAR );

    n = number_range(2,5);
    /* restore some mana if less than five skeletons are summoned */
    if ( n < 5 ) ch->mana = UMIN( ch->max_mana, ch->mana + (5-n)*60 );
    for (i = 0; i < n; i++)
    {
        CHAR_DATA *mob;
        ch->blood -= 20;

        mob = create_mobile(pMobIndex);
        char_to_room(mob,ch->in_room);

        mob->max_hit = ch->max_hit / 2;
        mob->hit = mob->max_hit;
        mob->level = ch->level;

        add_follower(mob, ch);
        mob->leader = ch;
/*      ch->pet  = mob;*/
        SET_BIT(mob->act_bits, ACT_PET);  
        SET_BIT(mob->affected_by2, AFF2_SUMMONEDPET );

        af.where     = TO_AFFECTS;
        af.type      = sn;
        af.level     = level;
        af.duration  = 20;
        af.location  = 0;
        af.modifier  = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char( mob, &af );
    }
    spell_summon_sick(gsn_sumsick,level,ch,(void *) ch,TARGET_CHAR);
    return;
}


void spell_entrance( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{       
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    short chance;

   chance = number_range(1, 100);

 if (!IS_SET(ch->shiftbits,TEMP_VAMP) &&
        !IS_SET(ch->shiftbits,PERM_VAMP) &&
	!IS_NPC(ch)) {
        ch->pcdata->learned[sn] = 0;
      send_to_char("You don't know any spells of that name.\n\r",ch);
      return;
    }
    
    if (is_safe(ch,victim)){
        if (IS_AFFECTED2(victim, AFF_GHOST))
            send_to_char("You aren't sure how to entrance a ghost?\n\r",ch);
        return;
        }
        
    if ( victim == ch )
    {
        send_to_char( "You like yourself even better!\n\r", ch );
        return;
    }

 if( ch->blood < 10 )
   {
     send_to_char( "You haven't enough blood to work this magic.\n\r", ch );
     return;
   }

 (ch->blood) -= 10;

	
    if (IS_SET(ch->shiftbits,PERM_VAMP)) 
	level = ch->level;
    
    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   victim->fighting != NULL
    ||  (victim->level > 50))
    {
        act( "You stare into $N's eyes.", ch, NULL, victim, TO_CHAR );
        act( "$n stares into your eyes.", ch, NULL, victim, TO_VICT );
        return;
    }
    
    if (saves_spell(level,ch,victim,DAM_OTHER) )
     {
       if (chance < 75)
       {
          act( "You stare into $N's eyes.", ch, NULL, victim, TO_CHAR );
          act( "$n stares into your eyes.", ch, NULL, victim, TO_VICT );
          return;
       }
     }

    if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
    {
        send_to_char(
            "The mayor does not allow entrancing in the city limits.\n\r",ch);
        return;
    }

    if (!IS_AWAKE(victim) || !can_see(ch,victim)) {
/* send_to_char("They can't see your eyes.\n\r",ch);*/
        act( "$E can't see your eyes.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( victim->master )
        if ( stop_follower( victim ) )
	    return;

    if ( !check_charmlist( ch, victim ) )
    {
        act( "You stare deep into $N's eyes.",ch,NULL,victim,TO_CHAR);
        act( "$n stares deep into your eyes, and you fall into a trance.", ch, NULL, victim, TO_VICT );
        act( "$n stares into $N's eyes.", ch, NULL, victim, TO_NOTVICT );
        add_follower( victim, ch );
        victim->leader = ch;
        af.where     = TO_AFFECTS;
        af.type      = sn;
        af.level     = level;
        af.duration  = number_fuzzy( level / 5 );
        af.location  = 0;
        af.modifier  = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char( victim, &af );
        if ( ch != victim )
            act("$N is ready to do your bidding.",ch,NULL,victim,TO_CHAR);
    }
    return;
}

void spell_bboil( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  short chance;

  chance = number_range(1,100);

  if (!IS_SET(ch->shiftbits,TEMP_VAMP) &&
      !IS_SET(ch->shiftbits,PERM_VAMP) &&
      !IS_NPC(ch)) {
    ch->pcdata->learned[sn] = 0;
    send_to_char("You don't know any spells of that name.\n\r",ch);
    return;
  }

  if( ch->blood < 5 )
    {
      send_to_char( "You haven't enough blood to work this magic.\n\r", ch );
      return;
    }

 (ch->blood) -= 5;

  if (IS_AFFECTED2(victim,AFF_BBOIL)){
    send_to_char("Their blood is already boiling!.\n\r",ch);
    return;
  }

  if (saves_spell(level,ch,victim,DAM_DISEASE) ||
      (IS_NPC(victim) && IS_SET(victim->act_bits,ACT_UNDEAD)))
    {
      if (ch == victim)
	send_to_char("You feel your blood momentarily heating.\n\r",ch);
      else
	{
	  if (chance < 75)
	    {
	      act("$N seems to be unaffected by your blood magic.",ch,NULL,victim,TO_CHAR);   
	      return;
	    }
	}
    }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 5;
  af.modifier  = -5;
  af.bitvector = AFF_BBOIL;
  
  af.location  = APPLY_DEX;
  affect_to_char2(victim,&af);

  af.location  = APPLY_STR;
  affect_to_char2(victim,&af);

  send_to_char
    ("You scream in agony as your blood begins to boil!\n\r",victim);
  act("$n screams in agony as $s blood begins to boil!",victim,NULL,NULL,TO_ROOM);
} 

void spell_confusion( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    AFFECT_DATA af;
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   short chance;

   chance = number_range(1,100);

    if (!IS_SET(ch->shiftbits,TEMP_VAMP) &&
        !IS_SET(ch->shiftbits,PERM_VAMP) &&
	!IS_NPC(ch)) {
      ch->pcdata->learned[sn] = 0;
      send_to_char("You don't know any spells of that name.\n\r",ch);
      return;
     }

    if (IS_AFFECTED2(victim,AFF_CONFUSE))        
       {
         act( "$E already seems confused enough.", ch, NULL, victim, TO_CHAR );
/*       send_to_char("They already seem confused enough.\n\r",ch); */
         return;
       }
 
 if( ch->blood < 15 )
   {
     send_to_char( "You haven't enough blood to work this magic.\n\r", ch );
     return;
   }

 (ch->blood) -= 15;

    if (saves_spell(level,ch,victim,DAM_OTHER))
    {
      if (chance < 75)
       {
        send_to_char("Your spell had no effect.\n\r",ch);
        return;
       }
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_CONFUSE;
    affect_to_char2( victim, &af );
 
    send_to_char( "You feel very confused.\n\r", victim );
    act( "$n gets a puzzled look on $s face.", victim, NULL, NULL, TO_ROOM );
    
    return;
}


void spell_focus( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    AFFECT_DATA af;
   CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (!IS_SET(ch->shiftbits,TEMP_VAMP) &&
        !IS_SET(ch->shiftbits,PERM_VAMP) &&
	!IS_NPC(ch)) {
      ch->pcdata->learned[sn] = 0;
      send_to_char("You don't know any spells of that name.\n\r",ch);
      return;
     }
 
    if (IS_AFFECTED2(victim,AFF_FOCUS))
    {
      send_to_char("You cannot focus anymore blood energy.\n\r",ch);
      return;
    }

 if( ch->blood < 30 )
   {
     send_to_char( "You haven't enough blood to work this magic.\n\r", ch );
     return;
   }

 (ch->blood) -= 30;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2 * ( level/5 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FOCUS;
    affect_to_char2( ch, &af );
 
    send_to_char( "You focus your blood magic.\n\r", ch );
    
    return;
}

void spell_chimestry( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *mob;
  MOB_INDEX_DATA *pmobindex;
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  short indexnum;
  short chance;
  int loop;

  chance = number_range(1,100);

  if (!IS_SET(ch->shiftbits,TEMP_VAMP) &&
      !IS_SET(ch->shiftbits,PERM_VAMP) &&
      !IS_NPC(ch)) {
    ch->pcdata->learned[sn] = 0;
    send_to_char("You don't know any spells of that name.\n\r",ch);
    return;
  }
 
  if (victim == ch)
    {
      if (IS_AFFECTED2(victim,AFF_CHIMESTRY))
	{
	  send_to_char("You cannot summon another illusion so quickly.\n\r",ch);
	  return;
	}

      if( ch->blood < 10 )
	{
	  send_to_char( "You haven't enough blood to work this magic.\n\r", ch );
	  return;
	}

      (ch->blood) -= 10;

      indexnum = 25;

      pmobindex = get_mob_index(indexnum);

      if(!pmobindex){
	return;
      }

      for (loop = 0; loop < 5; loop++)
        {
	  mob = create_mobile(pmobindex);
	  char_to_room(mob,ch->in_room);

	  mob->max_hit = 10;
	  mob->hit = mob->max_hit;
	  mob->level = ch->level;

	  sprintf(buf,"%s %s", mob->name, ch->name);
	  free_string(mob->name);
	  mob->name  = str_dup(buf);

	  if IS_SET(victim->shiftbits, BAT_VAMP)
	    {
	      sprintf(buf,"A bat");
	      free_string(mob->short_descr);
	      mob->short_descr =str_dup(buf);

	      sprintf(buf,"A large bat is here.\n\r");
	      free_string(mob->long_descr);
	      mob->long_descr = str_dup(buf);

	      sprintf(buf, "%s",ch->shiftdesc1);
	      free_string(mob->description);
	      mob->description = str_dup(buf);
	    }

	  else if IS_SET(victim->shiftbits, MIST_VAMP)
	    {
	      sprintf(buf,"Some mist");
	      free_string(mob->short_descr);
	      mob->short_descr =str_dup(buf);

	      sprintf(buf,"Some mist is here.\n\r");
	      free_string(mob->long_descr);
	      mob->long_descr = str_dup(buf);

	      sprintf(buf, "%s",ch->shiftdesc2);
	      free_string(mob->description);
	      mob->description = str_dup(buf);
	    }

	  else
	    {
	      sprintf(buf,"%s",ch->name);
	      free_string(mob->short_descr);
	      mob->short_descr = str_dup(buf);

	      sprintf(buf,"%s%s is here.\n\r", ch->name, ch->pcdata->title);
	      free_string(mob->long_descr);
	      mob->long_descr = str_dup(buf);

	      sprintf(buf,"%s", ch->description);
	      free_string(mob->description);
	      mob->description = str_dup(buf);
	    }

	  af.where     = TO_AFFECTS;
	  af.type      = sn;
	  af.level     = level;
	  af.duration  = 3;
	  af.location  = APPLY_NONE;
	  af.modifier  = 0;
	  af.bitvector = AFF_CHIMESTRY;
	  affect_to_char2( mob, &af );

	  af.where     = TO_AFFECTS;
	  af.type      = sn;
	  af.level     = level;
	  af.duration  = 3;
	  af.location  = APPLY_NONE;
	  af.modifier  = 0;
	  af.bitvector = AFF_CHIMESTRY;
	  affect_to_char2(ch, &af);
	  send_to_char( "You summon forth an illusion.\n\r", ch );
	  return;
        }
    }  
  if (victim != ch)
    {
      if (IS_AFFECTED2(victim, AFF_CHIMESTRY2))
	{
	  send_to_char("They seem to already be plagued by nightmares.\n\r",ch);
	  return;
	}

      if ( IS_AWAKE(victim) 
      &&  ( IS_SET(victim->in_room->room_flags,ROOM_SAFE)
            || (IS_SET(victim->in_room->room_flags,ROOM_SEMISAFE) 
                && !in_fightlag(victim) ) ) )
	{
	  send_to_char("Your target is too aware for such trickery!\n\r",ch);
	  return;
	}

      if (!IS_AWAKE(victim))
	{
	  send_to_char("You wake screaming from the nightmares!\n\r",victim);
	  send_to_char("You plague your victim with nightmares!\n\r",ch);
	  act( "$n wakes screaming from a nightmare!",victim, NULL, NULL, TO_ROOM );
	  victim->position = POS_STANDING;

	  af.where     = TO_AFFECTS;
	  af.type      = sn;
	  af.level     = level;
	  af.duration  = 5;
	  af.location  = APPLY_NONE;
	  af.modifier  = 0;
	  af.bitvector = AFF_CHIMESTRY2;
	  affect_to_char2(victim, &af);
	  return;
	}

      if (saves_spell(level,ch,victim,DAM_MENTAL)) 
	{
	  if (chance < 75)
	    {
	      send_to_char("Your nightmares have no effect.\n\r",ch);
	      return;
	    }
	}
      af.where     = TO_AFFECTS;
      af.type      = sn;
      af.level     = level;
      af.duration  = 5;
      af.bitvector = AFF_CHIMESTRY2;

      af.location  = APPLY_STR;
      af.modifier  = -2;
      affect_to_char2(victim, &af);

      af.location  = APPLY_DEX;
      af.modifier  = -2;
      affect_to_char2(victim, &af);
      send_to_char("You plague your victim with waking nightmares!\n\r",ch);
      send_to_char("Visions of monstrosities plague your psyche!\n\r", victim);
      return;
    }
}

void spell_blade2( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
 
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;

    if (!IS_SET(ch->shiftbits,TEMP_VAMP) &&
        !IS_SET(ch->shiftbits,PERM_VAMP) &&
        !IS_NPC(ch)) {
      ch->pcdata->learned[sn] = 0;
      send_to_char("You don't know any spells of that name.\n\r",ch);
      return;
     }
 
 if( ch->blood < 25 )
   {
     send_to_char( "You haven't enough blood to work this magic.\n\r", ch );
     return;
   }
 (ch->blood) -= 25;
  if (( pObjIndex = get_obj_index( 24)) == NULL )
    {
     send_to_char("No object has that vnum.\n\r", ch);
     return;
    }
 
   obj = create_object(pObjIndex,level);
   obj->timer    = 25;

   obj_to_char(obj,ch);
   act("$n focuses $s energy and summons forth an ancient rune blade!", ch, NULL, ch, TO_ROOM);
   send_to_char("You summon forth an ancient rune blade of {rblood{x!\n\r",ch);

}


void spell_celerity( int sn, 
int level, CHAR_DATA 
*ch, void *vo,int target)
{
    AFFECT_DATA af;
   CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (!IS_SET(ch->shiftbits,TEMP_VAMP) &&
        !IS_SET(ch->shiftbits,PERM_VAMP) &&
	!IS_NPC(ch)) {
      ch->pcdata->learned[sn] = 0;
      send_to_char("You don't know any spells of that name.\n\r",ch);
      return;
     }
 
 if( ch->blood < 10 )
   {
     send_to_char( "You haven't enough blood to work this magic.\n\r", ch );
     return;
   }
 (ch->blood) -= 10;
    if (IS_AFFECTED2(victim,AFF_CELERITY))
    {
      send_to_char("Your blood cannot pump any faster.\n\r",ch);
      return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/3;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_CELERITY;
    affect_to_char2( ch, &af );
 
    send_to_char( "Your blood runs faster!\n\r", ch );
    
    return;
}

void spell_silence( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    AFFECT_DATA af;
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   short chance;

    chance = number_range(1,100);

    if (!IS_SET(ch->shiftbits,TEMP_VAMP) &&
        !IS_SET(ch->shiftbits,PERM_VAMP) &&
	!IS_NPC(ch)) {
      ch->pcdata->learned[sn] = 0;
      send_to_char("You don't know any spells of that name.\n\r",ch);
      return;
    }
 
 if( ch->blood < 20 )
   {
     send_to_char( "You haven't enough blood to work this magic.\n\r", ch );
     return;
   }
 (ch->blood) -= 20;
    if (IS_AFFECTED2(victim,AFF_SILENCE))
    {
      send_to_char("They are already silenced.\n\r",ch);
      return;
    }

    if (saves_spell(level,ch,victim,DAM_MENTAL))
    {
      if (chance < 75)
       {
        send_to_char("Your spell had no effect.\n\r",ch);
        return;
       }
    }


    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/10;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SILENCE;
    affect_to_char2( victim, &af );
 
    send_to_char( "You silence your victim.\n\r", ch );
    send_to_char( "You have been silenced!\n\r", victim);    
    return;
}

void do_grant(CHAR_DATA *ch, char *argument) 
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    int sn;

    if (!IS_SET(ch->shiftbits,PERM_VAMP)) 
    {
        send_to_char("Huh?\n\r",ch);
        return;
    }

    argument = one_argument(argument, arg);

    if (*arg == '\0') 
    {
        send_to_char("Who do you want to grant something to?\n\r",ch);
        return;
    }

    if ( (victim = get_char_world(ch,arg, FALSE)) == NULL ) 
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if (IS_NPC(victim)) return;
    if (victim != ch 
    && (!IS_SET(victim->shiftbits,TEMP_VAMP) 
        || str_cmp(ch->name,victim->patriarch))) 
    {
        send_to_char("They aren't your minion.\n\r",ch);
        return;
    }

    if (*argument == '\0') 
    {
        send_to_char("Available skills/spells:\n\r",ch);
        send_to_char("  unholy touch         rune blade      confusion\n\r",ch);
        send_to_char("  entrance             unholy touch\n\r",ch);
        send_to_char("  summon skeletal army\n\r",ch);
        if ( !str_cmp( ch->name, LASOMBRA_PAT ) )
        {
            send_to_char("  interred to earth    blood boil\n\r",ch);
            send_to_char("  sunlight skeletals\n\r",ch);
        }
        else if ( !str_cmp( ch->name, RAVNOS_PAT ) )
        {
            send_to_char("  chimestry            celerity\n\r",ch);
        }
        else if ( !str_cmp( ch->name, FEROCAI_PAT ) )
        {
            send_to_char("  aura of pain        profane word\n\r", ch );
        }
        return;
    }
    else if (!str_prefix(argument,"confusion"))
        sn = skill_lookup("confusion");
    else if (!str_prefix(argument,"entrance"))
        sn = gsn_entrance;
    else if (!str_prefix(argument,"summon skeletal army"))
        sn = skill_lookup("summon skeletal army");
    else if (!str_prefix(argument,"focus"))
        sn = skill_lookup("focus");
    else if (!str_prefix(argument,"unholy touch"))
        sn = skill_lookup("unholy touch");
    else if (!str_prefix(argument,"rune blade"))
        sn = skill_lookup("rune blade");

    /* the brood formerly known as ravnos */
    else if (!str_prefix(argument,"celerity"))
        sn = skill_lookup("celerity");
    else if (!str_prefix(argument,"chimestry"))
        sn = skill_lookup("chimestry");

    /* lambrosa */
    else if (!str_prefix(argument,"interred to earth"))
        sn = skill_lookup("interred to earth");
    else if (!str_prefix(argument,"blood boil"))
        sn = gsn_bboil;
    else if (!str_prefix(argument,"sunlight skeletals"))
        sn = gsn_sun_skeletals;

    /* new brood */
    else if (!str_prefix(argument,"aura of pain"))
        sn = gsn_aura_pain;
    else if (!str_prefix(argument,"profane word"))
        sn = gsn_profane_word;

    else 
    {
        send_to_char("That is not a valid vampire skill or spell.\n\r",ch);
        return;
    }

    if (sn == NO_SKILL) 
    {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf,"Error do_grant: skill %s not found.", argument);
        bug(buf,0);
        send_to_char(
            "There is a problem with that spell.  Please notify IMP.\n\r", ch );
        return;
    }

    if (victim->pcdata->learned[sn] != 0) 
    {
        victim->pcdata->learned[sn] = 0;

        if( ch == victim )
            act( "You disallow yourself the use of $t.", ch, 
                skill_table[sn].name, NULL, TO_CHAR );
        else
        {
            act( "$n disallows you the use of $t.", ch, skill_table[sn].name,
                victim, TO_VICT );
            act( "You disallow $N the use of $t.", ch, skill_table[sn].name,
                victim, TO_CHAR );
        }
        return;
    }

    victim->pcdata->learned[sn] = 80;
    if (ch == victim) 
    {
        act("You grant yourself the use of $t.",ch,skill_table[sn].name,
            NULL,TO_CHAR);
    }
    else 
    {
        act("$n grants you the use of $t.",ch,skill_table[sn].name,
            victim,TO_VICT);
        act("You grant $N the use of $t.",ch,skill_table[sn].name,
            victim,TO_CHAR);
    }
    return;
}
   
void spell_divine_essence(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    
    if ( IS_NPC(ch) )
    {
        send_to_char( "Mobiles shouldn't be casting this spell.\n\r", ch );
        return;
    }

    if ( !IS_SET(ch->shiftbits,TEMP_VAMP)
    &&   !IS_SET(ch->shiftbits,PERM_VAMP) ) 
    {
        ch->pcdata->learned[sn] = 0;
        send_to_char("You don't know any spells of that name.\n\r",ch);
        return;
    }

    if ( ch == victim )
    {
        send_to_char( "You can sense the blood pumping vigorously through your veins.\n\r", ch );
        return;
    }

    if ( ch->blood < 50 )
    {
        send_to_char( "You haven't enough blood to work this magic on another.\n\r", ch );
        return;
    }
    else
        ch->blood -= 50;

/* NPCs and morts casting on imms doesn't work ever */
    if ( IS_NPC(victim)
    ||   (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) )
    {
       act( "You lack the ability to sense anything about $S blood.", 
            ch, NULL, victim, TO_CHAR );
       return;
    }

/* victim sees a tingle message regardless, knowing something is happening */
    send_to_char( "You feel the blood tingle in your veins.\n\r", victim );

/* victim must fail a saves throw at this point, or */
    if (saves_spell(level,ch,victim,DAM_OTHER))
    {
        if (number_percent() < 75)
        {
           act( "You fail to sense the nature of $S blood.", 
                ch, NULL, victim, TO_CHAR );
            return;
        }
    }

/* cast on another non-imm player character */
    if ( IS_VAMPIRE( victim ) )
    {
        act( "You sense the curse of Kiyanne present in $S blood!",
          ch, NULL, victim, TO_CHAR );
    }
    else
    {
        act( "$N's blood flows normally through $S veins.",
          ch, NULL, victim, TO_CHAR );
    }
 
    return;
}

void do_sanguine_bond( CHAR_DATA * ch, char *argument )
{
    AFFECT_DATA af;
    int sk;

    if ( (sk = get_skill(ch, gsn_sanguine_bond) ) < 1 )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( IS_AFFECTED3( ch, AFF3_SBOND ) )
      send_to_char("You have already developed a strong sanguine bond.\n\r",ch);
    else
    {
        if ( number_percent() < get_skill(ch, gsn_sanguine_bond) )
        {
            af.where    = TO_AFFECTS;
            af.type     = gsn_sanguine_bond;
            af.level    = ch->level;
            af.duration = 80;                   /* one real-life hour */
            af.location = APPLY_NONE;
            af.modifier = 0;
            af.bitvector = AFF3_SBOND;
            affect_to_char3( ch, &af );

            send_to_char(
             "You tap into the ancient knowledge contained in your blood.\n\r",
             ch );
            check_improve ( ch, gsn_sanguine_bond, TRUE, 3 );
        }
        else
        {
            send_to_char("You fail to establish a deep sanguine bond.\n\r",ch);
            check_improve ( ch, gsn_sanguine_bond, FALSE, 3 );
        }
    }
    WAIT_STATE( ch, PULSE_VIOLENCE );
    return;
}

void spell_aura_pain( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (!IS_SET(ch->shiftbits,TEMP_VAMP) 
    &&  !IS_SET(ch->shiftbits,PERM_VAMP) 
    &&  !IS_NPC(ch)) 
    {
        ch->pcdata->learned[sn] = 0;
        send_to_char("You don't know any spells of that name.\n\r",ch);
        return;
    }
 
    if (IS_AFFECTED3(victim,AFF3_AURA_PAIN))
    {
        send_to_char("You are already surrounded by an aura of pain.\n\r",ch);
        return;
    }

    if( ch->blood < 20 )
    {
        send_to_char( "You haven't enough blood to work this magic.\n\r", ch );
        return;
    }

    (ch->blood) -= 20;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 10;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF3_AURA_PAIN;
    affect_to_char3( ch, &af );
 
    send_to_char( "You surround yourself in an aura of pain.\n\r", ch );
    
    return;
}

void spell_profane_word(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    sh_int chance;

    chance = number_range(1,100);

    if ( !IS_SET(ch->shiftbits,TEMP_VAMP) 
    &&   !IS_SET(ch->shiftbits,PERM_VAMP) 
    &&   !IS_NPC(ch) )
    {
        ch->pcdata->learned[sn] = 0;
        send_to_char("You don't know any spells of that name.\n\r",ch);
        return;
    }

    if ( IS_AFFECTED3(victim,AFF3_PROFANE_WORD) )
    {
        act( "$E has already been cursed by a profane word.", 
            ch, NULL, victim, TO_CHAR );
        return;
    }
 
    if ( ch->blood < 15 )
    {
        send_to_char( "You haven't enough blood to work this magic.\n\r", ch );
        return;
    }

    (ch->blood) -= 15;

    if (saves_spell(level,ch,victim,DAM_OTHER))
    {
        if (chance < 75)
        {
            send_to_char("Your spell had no effect.\n\r",ch);
            return;
        }
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF3_PROFANE_WORD;
    affect_to_char3( victim, &af );
 
    send_to_char( "A profane word disrupts your concentration!\n\r", victim );
    act( "You curse $N with a profane word!", ch, NULL, victim, TO_CHAR );
    
    return;
}

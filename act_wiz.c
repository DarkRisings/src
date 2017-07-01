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
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glob.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"
#include "brawler.h"
#include "clans.h"
#include "interp.h"

/* command procedures needed */
DECLARE_DO_FUN(do_rstat		);
DECLARE_DO_FUN(do_mstat		);
DECLARE_DO_FUN(do_ostat		);
DECLARE_DO_FUN(do_rset		);
DECLARE_DO_FUN(do_mset		);
DECLARE_DO_FUN(do_oset		);
DECLARE_DO_FUN(do_sset		);
DECLARE_DO_FUN(do_tset		);
DECLARE_DO_FUN(do_mfind		);
DECLARE_DO_FUN(do_ofind		);
DECLARE_DO_FUN(do_mload		);
DECLARE_DO_FUN(do_oload		);
DECLARE_DO_FUN(do_quit		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_stand		);

extern bool 	check_multilogin (char *site,int type);
extern char* 	res_bit_name	 ( int res_flags );
extern char* 	vuln_bit_name	 ( int res_flags );
extern char* 	room_bit_name	 ( int room_flags );
extern char*	trans_bit_name	 ( int trans_flags );
extern bool     check_parse_name ( char *name );

//int 		make_tars 	args( (CHAR_DATA* ch ) );
//void 		purge_pfiles 	args( (CHAR_DATA *ch) );

/*
 *
 * Descriptor Table Converter
 * added by Davian
 *
 */
#define GET_STATUS( a )         ((a)==0 ) ? "PLAYING             " :\
                                ((a)==1 ) ? "GET NAME            " : \
                                ((a)==2 ) ? "GET OLD PASSWORD    " : \
                                ((a)==3 ) ? "CONFIRM NEW NAME   " : \
                                ((a)==4 ) ? "GET NEW PASSWORD    " : \
                                ((a)==5 ) ? "CONFIRM NEW PASSWORD" : \
                                ((a)==6 ) ? "GET NEW RACE        " : \
                                ((a)==7 ) ? "GET NEW SEX         " : \
                                ((a)==8 ) ? "GET NEW CLASS       " : \
                                ((a)==9) ? "GET ALIGNMENT        " : \
                                ((a)==10) ? "DEFAULT CHOICE      " : \
                                ((a)==11) ? "GEN GROUPS          " : \
                                ((a)==12) ? "PICK WEAPON         " : \
                                ((a)==13) ? "READ PREGREETING    " : \
                                ((a)==14) ? "READ MOTD           " : \
                                ((a)==15) ? "BREAK CONNECT       " : \
                                ((a)==16) ? "GET ANSI            " : \
				((a)==17) ? "CHOOSE TERM	 " : \
                                            "UNKNOWN             "




extern  AFFECT_DATA        *affect_free;
/*
 * Local functions.
 */
ROOM_INDEX_DATA *	find_location	args( ( CHAR_DATA *ch, char *arg ) );
extern void    weather_update  args( ( void ) );

int toggle_notetags( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH+MAX_INPUT_LENGTH+1], name[MAX_STRING_LENGTH];

    one_argument( argument, name );

    if ( IS_NULLSTR( name ) )
    {
      bug( "toggle_notetags: Passed NULL name", 0 );
      return 0;
    }
    if ( IS_NPC ( ch ) )
    {
      bug( "toggle_notetags: Passed NPC ch", 0 );
      return 0;
    }

/*  Note tags must all be lowercase! */
    strcpy( name, capitalize( name ) );
    name[0] = LOWER( name[0] );

    if ( strstr( ch->pcdata->notetags, name ) != '\0' )
    {
        ch->pcdata->notetags = string_replace( ch->pcdata->notetags, name, "" );
        ch->pcdata->notetags = string_unpad( ch->pcdata->notetags );
        return 1;
    }
    else
    {
        buf[0] = '\0';

        if ( !IS_NULLSTR( ch->pcdata->notetags ) )
        {
            strcat( buf, ch->pcdata->notetags );
            if ( buf[strlen(buf)] == ' ' )
              buf[strlen(buf)] = '\0';
            strcat( buf, " " );
        }
        strcat( buf, name );
        if ( strlen(buf) >= MAX_STRING_LENGTH )
        {
            send_to_char( "ERROR: Target is tagged to receive too many notes.\n\r", ch );
            return 0;
        }
        free_string( ch->pcdata->notetags );
        ch->pcdata->notetags = str_dup( buf );
        return 2;
    }
}

void do_notetag ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    FILE *fp;
    char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int rstat = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( IS_NPC( ch ) )
    {
        send_to_char( "NPCs cannot issue this command.\n\r", ch );
        return;
    }
    if ( IS_NULLSTR( arg1 ) || IS_NULLSTR( arg2 ) )
    {
        send_to_char( "Syntax: notetag <char> <tag>\n\r",ch);
        return;
    }
    if ( ( victim = get_char_world( ch, arg1, TRUE ) ) == NULL )
    {
        send_to_char( "They aren't playing.\n\r", ch );
        return;
    }

    if( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPCs.\n\r", ch );
        return;
    }
    sprintf(buf, "%s%s", PLAYER_DIR, capitalize( arg2 ) );

    fclose( fpReserve );
    if ( ( fp = fopen(buf, "r" ) ) != NULL )
    {
      fclose( fp );
      fpReserve = fopen( NULL_FILE, "r" );
      send_to_char( "You cannot tag someone to receive another character's notes.\n\r", ch );
      return;
    }
    fpReserve = fopen( NULL_FILE, "r" );

    rstat = toggle_notetags( victim, arg2 );
    if ( rstat == 0 )
    {
       bug( "do_notetag: toggle_notetags failed.", 0 );
       send_to_char( "You cannot tag that.\n\r", ch );
       return;
    }

    if ( victim == ch )
    {
      if ( IS_NULLSTR( victim->pcdata->notetags ) )
          send_to_char( "You will no longer see any special notes.\n\r", ch );
      else
      {
          sprintf(buf, "You will now see notes also addressed to: %s\n\r", 
              victim->pcdata->notetags);
          send_to_char( buf, ch );
      }
    }
    else
    {
      if ( IS_NULLSTR( victim->pcdata->notetags ) )
      {
          act( "$E will no longer see any special notes.", ch, NULL, victim, TO_CHAR );
          send_to_char( "You will no longer see any special notes.\n\r", victim );
      }
      else
      {
          sprintf(buf, "$E will now see notes also addressed to: %s", victim->pcdata->notetags );
          act( buf, ch, NULL, victim, TO_CHAR );
          sprintf(buf, "You will now see notes also addressed to: %s", victim->pcdata->notetags );
          send_to_char( buf, victim );
      }
    }
    return;
}

void do_screwtime( CHAR_DATA *ch, char *argument )
{
    int i,j;
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NULLSTR( argument ) )
        i = UMAX(1, atoi( argument ) );
    else
        i = 1;
    for ( j = 1; j <= i; j++ )
        weather_update  ( );

    sprintf(buf, "The time is now %d:00.\n\r",
        time_info.hour );
    send_to_char( buf, ch );
    return;
}

void do_wiznet( CHAR_DATA *ch, char *argument )
{
    int flag;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
      	if (IS_SET(ch->wiznet_flags,WIZ_ON))
      	{
            send_to_char("Signing off of Wiznet.\n\r",ch);
            REMOVE_BIT(ch->wiznet_flags,WIZ_ON);
      	}
      	else
      	{
            send_to_char("Welcome to Wiznet!\n\r",ch);
            SET_BIT(ch->wiznet_flags,WIZ_ON);
      	}
      	return;
    }

    if (!str_prefix(argument,"on"))
    {
	send_to_char("Welcome to Wiznet!\n\r",ch);
	SET_BIT(ch->wiznet_flags,WIZ_ON);
	return;
    }

    if (!str_prefix(argument,"off"))
    {
	send_to_char("Signing off of Wiznet.\n\r",ch);
	REMOVE_BIT(ch->wiznet_flags,WIZ_ON);
	return;
    }

    /* show wiznet status */
    if (!str_prefix(argument,"status")) 
    {
	buf[0] = '\0';

	if (!IS_SET(ch->wiznet_flags,WIZ_ON))
	    strcat(buf,"off ");

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	    if (IS_SET(ch->wiznet_flags,wiznet_table[flag].flag))
	    {
		strcat(buf,wiznet_table[flag].name);
		strcat(buf," ");
	    }

	strcat(buf,"\n\r");

	send_to_char("Wiznet status:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }

    if (!str_prefix(argument,"show"))
    /* list of all wiznet options */
    {
	buf[0] = '\0';

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	{
	    if (wiznet_table[flag].level <= get_trust(ch))
	    {
	    	strcat(buf,wiznet_table[flag].name);
	    	strcat(buf," ");
	    }
	}

	strcat(buf,"\n\r");

	send_to_char("Wiznet options available to you are:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }
   
    flag = wiznet_lookup(argument);

    if (flag == -1 || get_trust(ch) < wiznet_table[flag].level)
    {
	send_to_char("No such option.\n\r",ch);
	return;
    }
   
    if (IS_SET(ch->wiznet_flags,wiznet_table[flag].flag))
    {
	sprintf(buf,"You will no longer see %s on wiznet.\n\r",
	        wiznet_table[flag].name);
	send_to_char(buf,ch);
	REMOVE_BIT(ch->wiznet_flags,wiznet_table[flag].flag);
    	return;
    }
    else
    {
    	sprintf(buf,"You will now see %s on wiznet.\n\r",
		wiznet_table[flag].name);
	send_to_char(buf,ch);
    	SET_BIT(ch->wiznet_flags,wiznet_table[flag].flag);
	return;
    }

}

void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
	    long flag, long flag_skip, int min_level) 
{
    char buf [MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if (d->connected == CON_PLAYING
	&&  IS_IMMORTAL(d->character) 
	&&  IS_SET(d->character->wiznet_flags,WIZ_ON) 
	&&  (!flag || IS_SET(d->character->wiznet_flags,flag))
	&&  (!flag_skip || !IS_SET(d->character->wiznet_flags,flag_skip))
	&&  get_trust(d->character) >= min_level
	&&  d->character != ch)
        {
	    if (IS_SET(d->character->wiznet_flags,WIZ_PREFIX))
	  	send_to_char("--> ",d->character);
if (strlen(string) <= MAX_STRING_LENGTH) {
 	   sprintf(buf,"{%c%s{x",d->character->colors[C_WIZNET],string); 
           act_new(buf,d->character,obj,ch,TO_CHAR,POS_DEAD,0);
	} else act_new(string,d->character,obj,ch,TO_CHAR,POS_DEAD,0);
        }
    }
 
    return;
}

void do_guild( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int guild;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
      send_to_char( "Syntax: guild <char> <guild name>\n\r",ch);
      return;
    }
  if ( ( victim = get_char_world( ch, arg1, TRUE ) ) == NULL )
    {
      send_to_char( "They aren't playing.\n\r", ch );
      return;
    }

  if( IS_NPC( victim ) )
    {
      send_to_char( "Not on NPCs.\n\r", ch );
      return;
    }
    
  if (!str_prefix(arg2,"none"))
    {
      send_to_char("They are now guildless.\n\r",ch);
      send_to_char("You are now a member of no guild!\n\r",victim);

      if( victim->guild == guild_lookup( "coalition" ) )
	{
	  free_string( victim->cln_apply );
	  free_string( victim->cln_name );
	  free_string( victim->cln_leader );
	  free_string( victim->cln_symbol );
	    
	  victim->cln_apply = str_empty;
	  victim->cln_name = str_empty;
	  victim->cln_leader = str_empty;
	  victim->cln_symbol = str_empty;
	}
	
      victim->guild = 0;
      return;
    }

    
  if ((guild = guild_lookup(arg2)) == 0)
    {
      send_to_char("No such guild exists.\n\r",ch);
      return;
    }

  if( strcmp( guild_table[ guild ].name, "imp" ) == 0 )
    {
      if( ch->level < MAX_LEVEL )
	{
	  send_to_char( "You are not of high enough level.\n\r", ch );
	  return;
	}
    }


  sprintf(buf,"They are now a member of guild %s.\n\r",
	  capitalize(guild_table[guild].name));
  send_to_char(buf,ch);
  sprintf(buf,"You are now a member of guild %s.\n\r",
	  capitalize(guild_table[guild].name));

  if( victim->guild == guild_lookup( "coalition" ) )
    {
      free_string( victim->cln_apply );
      free_string( victim->cln_name );
      free_string( victim->cln_leader );
      free_string( victim->cln_symbol );
	    
      victim->cln_apply = str_empty;
      victim->cln_name = str_empty;
      victim->cln_leader = str_empty;
      victim->cln_symbol = str_empty;
    }

  victim->guild = guild;
  victim->pcdata->guild_time = 0;
  victim->pcdata->guild_logs = 1;
  victim->pcdata->guild_date = current_time;
}

void do_rank( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: rank <char> <rank>\n\r",ch);
        return;
    }

    if ( ( victim = get_char_world( ch, arg1, TRUE) ) == NULL )
    {
        send_to_char( "They aren't playing.\n\r", ch );
        return;
    }

    free_string( victim->title_guild );
    smash_tilde( arg2 );
    sprintf(buf,"%s",arg2);
    victim->title_guild = str_dup( buf );
    
    sprintf(buf,"They have achieved %s status within the guild.\n\r", arg2);
    send_to_char(buf,ch);
    sprintf(buf,"You have achieved %s status within the guild.\n\r", arg2);
    send_to_char(buf, victim);

 
}

/* equips a character */
void do_outfit ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int i,sn,vnum;

    if (ch->level > 5 || IS_NPC(ch))
    {
	send_to_char("Find it yourself!\n\r",ch);
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL )
    {
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_LIGHT );
    }
 
    if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL )
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_BODY );
    }

    /* do the weapon thing */
    if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL)
    {
    	sn = 0; 
    	vnum = OBJ_VNUM_SCHOOL_SWORD; /* just in case! */

    	for (i = 0; weapon_table[i].name != NULL; i++)
    	{
	    if (ch->pcdata->learned[sn] < 
		ch->pcdata->learned[*weapon_table[i].gsn])
	    {
	    	sn = *weapon_table[i].gsn;
	    	vnum = weapon_table[i].vnum;
	    }
    	}

    	obj = create_object(get_obj_index(vnum),0);
     	obj_to_char(obj,ch);
    	equip_char(ch,obj,WEAR_WIELD);
    }

    if (((obj = get_eq_char(ch,WEAR_WIELD)) == NULL 
    ||   !IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)) 
    &&  (obj = get_eq_char( ch, WEAR_SHIELD ) ) == NULL )
    {
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_SHIELD );
    }

    if (ch->race == race_lookup("lich"))
        send_to_char("Thou hast been equipped by Syrin.\n\r",ch);
    else
        send_to_char("Thou hast been equipped by Tyrin.\n\r",ch);

}

     
/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument )
{
  char arg[ MAX_INPUT_LENGTH ] = "";
  char times[ MAX_INPUT_LENGTH ] = "";
  char buf[ MAX_STRING_LENGTH ] = "";
  CHAR_DATA* victim = NULL;
  static const time_t MINUTE = 60;
  static const time_t HOUR = 3600;
  static const time_t DAY = 86400;
  time_t duration = time( NULL );
  bool timed = FALSE;

  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
      send_to_char( "Nochannel whom?\n\r", ch );
      return;
    }

  if ( ( victim = get_char_world( ch, arg, TRUE ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  argument = one_argument( argument, times );
  if( *times != '\0' )
    {
      duration += ( atoi( times ) * DAY );
      timed = TRUE;
    }

  argument = one_argument( argument, times );
  if( *times != '\0' )
    {
      duration += ( atoi( times ) * HOUR );
      timed = TRUE;
    }

  argument = one_argument( argument, times );
  if( *times != '\0' )
    {
      duration += ( atoi( times ) * MINUTE );
      timed = TRUE;
    }
  
  if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }

  if ( IS_SET(victim->comm, COMM_NOCHANNELS ) )
    {
      REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
      send_to_char( "You can use channels again.\n\r", victim );
      send_to_char( "NOCHANNEL removed.\n\r", ch );
      sprintf(buf,"$N restores channels to %s.",victim->name);
      wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);

      if( !IS_NPC( victim ) )
	{
	  victim->pcdata->restore_channels = 0;
	}
    }
  else
    {
      SET_BIT(victim->comm, COMM_NOCHANNELS);
      send_to_char( "You can't use channels!\n\r", victim );
      send_to_char( "NOCHANNEL set.\n\r", ch );
      sprintf(buf,"$N revokes %s's channels.",victim->name);
      wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);

      if( !IS_NPC( victim ) && timed )
	{
	  victim->pcdata->restore_channels = duration;
	}
    }

  return;

}


void do_smote(CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char *letter,*name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;
 
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        send_to_char( "You can't show your emotions.\n\r", ch );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }
    
    if (strstr(argument,ch->name) == NULL)
    {
	send_to_char("You must include your name in an smote.\n\r",ch);
	return;
    }
   
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
 
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->desc == NULL || vch == ch)
            continue;
 
        if ((letter = strstr(argument,vch->name)) == NULL)
        {
	    send_to_char(argument,vch);
	    send_to_char("\n\r",vch);
            continue;
        }
 
        strcpy(temp,argument);
        temp[strlen(argument) - strlen(letter)] = '\0';
        last[0] = '\0';
        name = vch->name;
 
        for (; *letter != '\0'; letter++)
        {
            if (*letter == '\'' && matches == strlen(vch->name))
            {
                strcat(temp,"r");
                continue;
            }
 
            if (*letter == 's' && matches == strlen(vch->name))
            {
                matches = 0;
                continue;
            }
 
            if (matches == strlen(vch->name))
            {
                matches = 0;
            }
 
            if (*letter == *name)
            {
                matches++;
                name++;
                if (matches == strlen(vch->name))
                {
                    strcat(temp,"you");
                    last[0] = '\0';
                    name = vch->name;
                    continue;
                }
                strncat(last,letter,1);
                continue;
            }
 
            matches = 0;
            strcat(temp,last);
            strncat(temp,letter,1);
            last[0] = '\0';
            name = vch->name;
        }
 
	send_to_char(temp,vch);
	send_to_char("\n\r",vch);
    }
 
    return;
}

void do_pretitle( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your pretitle is %s\n\r",ch->pcdata->pretitle);
	    send_to_char(buf,ch);
	    return;
	}

	free_string( ch->pcdata->pretitle );
	ch->pcdata->pretitle = str_dup( argument );

        sprintf(buf,"Your pretitle is now %s\n\r",ch->pcdata->pretitle);
        send_to_char(buf,ch);
    }
    return;
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your poofin is %s\n\r",ch->pcdata->bamfin);
	    send_to_char(buf,ch);
	    return;
	}

	if ( strstr(argument,ch->name) == NULL)
	{
	    send_to_char("You must include your name.\n\r",ch);
	    return;
	}
	     
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );

        sprintf(buf,"Your poofin is now %s\n\r",ch->pcdata->bamfin);
        send_to_char(buf,ch);
    }
    return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
 
        if (argument[0] == '\0')
        {
            sprintf(buf,"Your poofout is %s\n\r",ch->pcdata->bamfout);
            send_to_char(buf,ch);
            return;
        }
 
        if ( strstr(argument,ch->name) == NULL)
        {
            send_to_char("You must include your name.\n\r",ch);
            return;
        }
 
        free_string( ch->pcdata->bamfout );
        ch->pcdata->bamfout = str_dup( argument );
 
        sprintf(buf,"Your poofout is now %s\n\r",ch->pcdata->bamfout);
        send_to_char(buf,ch);
    }
    return;
}

bool check_trans_string( const char *format )
{
    const char *str;

    str = format;
    while ( *str != '\0' )
    {
        if ( *str == '$' )
        {
            str++;
            if ( *str == '\0' )
                return TRUE;
            else if ( *str != 'n'
                 &&   *str != 'N'
                 &&   *str != 'm'
                 &&   *str != 'M'
                 &&   *str != 's'
                 &&   *str != 'S'
                 &&   *str != 'e'
                 &&   *str != 'E' )
            {
                return FALSE;
            }
        }
        str++;
    }
 
    return TRUE;
}

void do_transin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
        if (argument[0] == '\0')
        {
            sprintf(buf,"Your transin is %s\n\r",ch->pcdata->transin);
            send_to_char(buf,ch);
            return;
        }

        if ( strstr(argument,"$n")  == NULL )
        {
            send_to_char("You must include $n.\n\r",ch);
            return;
        }

        if ( !check_trans_string(argument) )
        {
            send_to_char(
        "You can only use the variables available to socials (help social)\n\r",
            ch );
            return;
        }
             
        free_string( ch->pcdata->transin );
        ch->pcdata->transin = str_dup( argument );

        sprintf(buf,"Your transin is now %s\n\r",ch->pcdata->transin);
        send_to_char(buf,ch);
    }
    return;
}

void do_transout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
 
        if ( argument[0] == '\0' )
        {
            sprintf(buf,"Your transout is %s\n\r",ch->pcdata->transout);
            send_to_char(buf,ch);
            return;
        }
 
        if ( strstr(argument, "$n") == NULL )
        {
            send_to_char( "You must include your name and $n.\n\r", ch );
            return;
        }

        if ( !check_trans_string(argument) )
        {
            send_to_char(
        "You can only use the variables available to socials (help social)\n\r",
                ch );
            return;
        }

        free_string( ch->pcdata->transout );
        ch->pcdata->transout = str_dup( argument );
 
        sprintf(buf,"Your transout is now %s\n\r",ch->pcdata->transout);
        send_to_char(buf,ch);
    }
    return;
}

void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->act_bits, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
    sprintf(buf,"$N denies access to %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    send_to_char( "OK.\n\r", ch );
    save_char_obj(victim);
    stop_fighting(victim,TRUE);
/*  victim->fightlag = 0; */
    victim->pcdata->lastfight = 0;
    do_quit( victim, "" );

    return;
}

void do_undeny( CHAR_DATA* ch, char* argument )
{
  DESCRIPTOR_DATA d;
  bool pfileExists = FALSE;
  char name[ MAX_INPUT_LENGTH ];
  char wiznet_buffer[ MAX_INPUT_LENGTH ];

  /* check for valid arguments */
  if( argument[ 0 ] == '\0' )
    {
      send_to_char( "Undeny who?\n\r", ch );
      return;
    }

  argument[ 0 ] = UPPER( argument[ 0 ] );
  argument = one_argument( argument, name );
  
  /* no sense in undenying characters who are playing */
  if( get_char_world( ch, name, FALSE ) != NULL )
    {
      send_to_char( "That person is already playing.\n\r", ch );
      return;
    }

  /* load pfile */
  pfileExists = load_char_obj( &d, name );

  if( !pfileExists )
    {
      send_to_char( "No such player.\n\r", ch );
      return;
    }

  d.character->desc = NULL;
  d.character->next = char_list;
  char_list = d.character;
  d.connected = CON_PLAYING;

  /* remove deny bit */
  REMOVE_BIT( d.character->act_bits, PLR_DENY );
  sprintf( wiznet_buffer, "$N denies access to %s", name );
  wiznet( wiznet_buffer, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0 );
  send_to_char( "OK.\n\r", ch );

  /* purge character */
  save_char_obj( d.character );
  extract_char( d.character, TRUE );


}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

    if (is_number(arg))
    {
	int desc;

	desc = atoi(arg);
    	for ( d = descriptor_list; d != NULL; d = d->next )
    	{
            if ( d->descriptor == desc )
            {
            	close_socket( d );
            	send_to_char( "Ok.\n\r", ch );
            	return;
            }
	}
    }

    if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( victim->level >= ch->level )
    {
	send_to_char("Yeah, right.\n\r", ch );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}



void do_pardon( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: pardon <character> <killer|newbie|brawler>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "killer" ) )
    {
	if ( IS_SET(victim->act_bits, PLR_KILLER) )
	{
	    REMOVE_BIT( victim->act_bits, PLR_KILLER );
	    send_to_char( "Killer flag removed.\n\r", ch );
	    send_to_char( "You are no longer a KILLER.\n\r", victim );
	}
	return;
    }

    if ( !str_cmp( arg2, "newbie" ) )
    {
	if ( IS_SET(victim->act_bits, PLR_NEWBIE) )
	{
	    REMOVE_BIT( victim->act_bits, PLR_NEWBIE );
	    send_to_char( "Newbie status removed.\n\r", ch );
	    send_to_char( "You are no longer a newbie.\n\r", victim );
	}
	return;
    }

    if( !str_cmp( arg2, "brawler" ) )
    {
        BRAWLER_DATA *pBrawler;
	if( IS_SET( victim->act_bits, PLR_BRAWLER ) )
        {
	    REMOVE_BIT( victim->act_bits, PLR_BRAWLER );
            SET_BIT( victim->comm, COMM_NOBRAWLER );
	    send_to_char( "Brawler status removed without prejudice.\n\r", ch );
	    send_to_char( "You are no longer a Brawler.\n\r", victim );
        }
        else if ( ((pBrawler = victim->pcdata->brawler) != NULL
               || (pBrawler = get_brawler( victim->name )) == NULL)
             && pBrawler->banuntil > 0 )
        {
            pBrawler->banuntil = 0;
            send_to_char( 
                "You are no longer prohibited from joining Brawler.\n\r",
                victim );
            act( "$S ban from re-joining Brawler has been pardoned.", ch, NULL,
                victim, TO_CHAR );
        }
	return;
    }

    send_to_char( "Syntax: pardon <character> <killer|newbie|brawler>.\n\r", ch );
    return;
}



void do_echo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Global echo what?\n\r", ch );
	return;
    }
    
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char( "global> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}


void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Local echo what?\n\r", ch );

	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )
	{
            if (get_trust(d->character) >= get_trust(ch))
                send_to_char( "local> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_zecho(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("Zone echo what?\n\r",ch);
	return;
    }

    for (d = descriptor_list; d; d = d->next)
      {
	if (d->connected == CON_PLAYING
	&&  d->character->in_room != NULL && ch->in_room != NULL
	&&  d->character->in_room->area == ch->in_room->area)
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char("zone> ",d->character);
	    send_to_char(argument,d->character);
	    send_to_char("\n\r",d->character);
	}
    }
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);
 
    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
	send_to_char("Personal echo what?\n\r", ch); 
	return;
    }
   
    if  ( (victim = get_char_world(ch, arg, FALSE) ) == NULL )
    {
	send_to_char("Target not found.\n\r",ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
        send_to_char( "personal> ",victim);

    send_to_char(argument,victim);
    send_to_char("\n\r",victim);
    send_to_char( "personal> ",ch);
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
}



void do_raceecho( CHAR_DATA* ch, char* argument )
{
  char arg[ MAX_INPUT_LENGTH ];
  CHAR_DATA* victim;
  int race;
  DESCRIPTOR_DATA* d;

  argument = one_argument( argument, arg );

  if( argument[ 0 ] == '\0' || arg[ 0 ] == '\0' )
    {
      send_to_char( "Race echo what?\n\r", ch );
      return;
    }

  if( ( race = race_lookup( arg ) ) == 0 )
    {
      send_to_char( "Echo to which race?\n\r", ch );
      return;
    }

  for( d = descriptor_list; d; d = d->next )
    {
      if( d->connected == CON_PLAYING )
	{
	  victim = d->character;
	  
	  if( victim->race == race )
	    {
	      if( get_trust( victim ) >= get_trust( ch ) )
		send_to_char( "racial> ", victim );
	      
	      send_to_char( argument, victim );
	      send_to_char( "\n\r", victim );
	    }
	}
    }
}


void do_gdecho( CHAR_DATA* ch, char* argument )
{
  char arg[ MAX_INPUT_LENGTH ];
  CHAR_DATA* victim;
  int guild;
  DESCRIPTOR_DATA* d;

  argument = one_argument( argument, arg );

  if( argument[ 0 ] == '\0' )
    {
      send_to_char( "Guild echo what?\n\r", ch );
      return;
    }

  if( ( guild = guild_lookup( arg ) ) == 0 )
    {
      send_to_char( "Echo to which guild?\n\r", ch );
      return;
    }

  for( d = descriptor_list; d; d = d->next )
    {
      if( d->connected == CON_PLAYING )
	{
	  victim = d->character;
	  
	  if( victim->guild == guild )
	    {
	      if( get_trust( victim ) >= get_trust( ch ) )
		send_to_char( "guild> ", victim );
	      
	      send_to_char( argument, victim );
	      send_to_char( "\n\r", victim );
	    }
	}
    }
}

void do_brawlerecho( CHAR_DATA* ch, char* argument )
{
  CHAR_DATA* victim;
  DESCRIPTOR_DATA* d;

  if( IS_NULLSTR( argument ) )
    {
      send_to_char( "Brawler echo what?\n\r", ch );
      return;
    }

  for( d = descriptor_list; d; d = d->next )
    {
      if( d->connected == CON_PLAYING )
        {
          victim = d->character;

          if( !IS_NPC(victim) && IS_SET(victim->act_bits,PLR_BRAWLER) )
            {
              if( get_trust( victim ) >= get_trust( ch ) )
                send_to_char( "brawler> ", victim );
              send_to_char( argument, victim );
              send_to_char( "\n\r", victim );
            }
        }
    }
}

void do_qecho( CHAR_DATA* ch, char* argument )
{
    DESCRIPTOR_DATA* d;
  
    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Quest echo what?\n\r", ch );
        return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING
        && (( IS_NPC( d->character ) || IS_SET(d->character->act_bits,PLR_QUEST) )
        ||  d->character == ch ) )
        {
            if( get_trust( d->character ) >= get_trust( ch ) )
                send_to_char( "quest> ", d->character );
            send_to_char( argument, d->character );
            send_to_char( "\n\r", d->character );
        }
    }
}
  



ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg, FALSE ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg, FALSE ) ) != NULL )
	return obj->in_room;

    return NULL;
}


void transfer_char( CHAR_DATA *ch, char *argument, bool flavor )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( IS_NPC(ch) 
    ||   IS_NULLSTR( ch->pcdata->transin )
    ||   IS_NULLSTR( ch->pcdata->transout ) )
        flavor = FALSE;

    if ( !str_cmp( arg1, "quest" ) )
    {
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->connected == CON_PLAYING
            &&   d->character != ch
            &&   d->character->in_room != NULL
            &&   can_see( ch, d->character ) 
            &&  IS_SET(d->character->act_bits,PLR_QUEST))
            {
                char buf[MAX_STRING_LENGTH];
                sprintf( buf, "%s %s", d->character->name, arg2 );
                transfer_char( ch, buf, flavor );
            }
        }
        return;
    }

    if ( IS_ADMIN( ch ) && !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		transfer_char( ch, buf, flavor );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( !is_room_owner(ch,location) && room_is_private( location ) 
	&&  get_trust(ch) < MAX_LEVEL)
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );

    if ( flavor )
        act_new( ch->pcdata->transout, victim, NULL, ch, 
            TO_ROOM, POS_SPRAWLED, victim->invis_level );
    else
        act_new( "$n disappears in a mushroom cloud.", victim, NULL, NULL, 
            TO_ROOM, POS_SPRAWLED, victim->invis_level );

    char_from_room( victim );
    char_to_room( victim, location );

    if ( flavor )
        act_new( ch->pcdata->transin, victim, NULL, ch, 
            TO_ROOM, POS_SPRAWLED, victim->invis_level );
    else
        act_new( "$n arrives from a puff of smoke.", victim, NULL, NULL, 
            TO_ROOM, POS_SPRAWLED, victim->invis_level );

    if ( ch != victim )
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT );

    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_transfer( CHAR_DATA *ch, char *argument )
{
    transfer_char( ch, argument, FALSE );
    return;
}

void do_ftrans( CHAR_DATA *ch, char *argument )
{
    transfer_char( ch, argument, TRUE );
    return;
}

void do_tardis( CHAR_DATA *ch, char *argument )
{
/*
A secret timey-wimey command that transfers players 
or yourself on the sly. It is essentially just like the
regular transfer command, but doesn't make the player
perform an 'auto look' and has no poofin/poofout 
messages. One can effectively sneak around and move
other players around without them knowing. This command
doesn't log, and should be kept under wraps for the
sake of intrique. I thought it easier / cleaner for the
future if I just copied over the transfer code and altered
it, rather than adjust the current transfer code to
accommodate.
*/
   
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( !is_room_owner(ch,location) && room_is_private( location ) 
	&&  get_trust(ch) < MAX_LEVEL)
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );


    char_from_room( victim );
    char_to_room( victim, location );

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch;
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && room_is_private( location ) 
    &&  get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    ch->on = on;
	    break;
	}
    }

    return;
}



void do_goto( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    count = 0;
    for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
        count++;

    if (!is_room_owner(ch,location) && room_is_private(location) 
    &&  (count > 1 || get_trust(ch) < MAX_LEVEL))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (get_trust(rch) >= ch->invis_level
        && (get_trust(rch) >= ch->rinvis_level || ch->race == rch->race ))
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	    else
		act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
    }

    char_from_room( ch );
    char_to_room( ch, location );


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level
        && (get_trust(rch) >= ch->rinvis_level || ch->race == rch->race ))
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }

    do_look( ch, "auto" );
    return;
}

void do_violate( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Goto where?\n\r", ch );
        return;
    }
 
    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    if (!room_is_private( location ))
    {
        send_to_char( "That room isn't private, use goto.\n\r", ch );
        return;
    }
 
    if ( ch->fighting != NULL )
        stop_fighting( ch, TRUE );
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
                act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
            else
                act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    char_from_room( ch );
    char_to_room( ch, location );
 
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    do_look( ch, "auto" );
    return;
}

/* RT to replace the 3 stat commands */

void do_stat ( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  stat <name>\n\r",ch);
	send_to_char("  stat obj <name>\n\r",ch);
	send_to_char("  stat mob <name>\n\r",ch);
 	send_to_char("  stat room <number>\n\r",ch);
	return;
   }

   if (!str_cmp(arg,"room"))
   {
	do_rstat(ch,string);
	return;
   }
  
   if (!str_cmp(arg,"obj"))
   {
	do_ostat(ch,string);
	return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
	do_mstat(ch,string);
	return;
   }
   
   /* do it the old way */

   obj = get_obj_world(ch,argument, FALSE);
   if (obj != NULL)
   {
     do_ostat(ch,argument);
     return;
   }

  victim = get_char_world(ch,argument, FALSE);
  if (victim != NULL)
  {
    do_mstat(ch,argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_rstat(ch,argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}

   



void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private( location ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: '%s'\n\rArea: '%s'\n\r",
	location->name,
	location->area->name );
    send_to_char( buf, ch );

    sprintf( buf,
	"Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
	location->vnum,
	location->sector_type,
	location->light,
	location->heal_rate,
	location->mana_rate );
    send_to_char( buf, ch );

    sprintf( buf,
	     "Room flags: %s\n\rTrans flags: %s\n\r",
	     room_bit_name( location->room_flags ),
	     trans_bit_name( location->trans_flags ) );

    send_to_char( buf, ch );

    sprintf( buf,
	     "Description:\n\r%s",
	     location->description );
    send_to_char( buf, ch );

    if ( location->p_image != NULL )
    {
	sprintf( buf,
	"Pueblo Image: %s.\n\r", location->p_image );
	send_to_char( buf, ch );
    }

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (can_see(ch,rch))
        {
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    sprintf( buf,
		"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

		door,
		(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
	    	pexit->key,
	    	pexit->exit_info,
	    	pexit->keyword,
	    	pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r" );
	    send_to_char( buf, ch );
	}
    }

    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *obj_inx;
    int vnum;
    int i;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, argument, FALSE ) ) == NULL )
    {
      /* Look for vnum */
      if (is_number(arg)) {
	vnum = atoi(arg);
	obj_inx = get_obj_index( vnum );
	if (obj_inx == NULL || 
	    ( obj = get_obj_world( ch, obj_inx->name, FALSE ) ) == NULL ) {
	  send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	  return;
	}
      }
      else {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
      }
    }

    sprintf( buf, "Name(s): %s\n\r",
	obj->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
	obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
	item_name(obj->item_type), obj->pIndexData->reset_num );
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
	obj->short_descr, obj->description );
    send_to_char( buf, ch );

/*  if ( obj->pIndexData->o_image != NULL )
    {
	sprintf( buf,
	"Pueblo Image: %s.\n\r", obj->pIndexData->o_image );
	send_to_char( buf, ch );
    } */

    sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\r",
	flag_string(wear_flags, obj->wear_flags), extra_bit_name( obj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ),get_true_weight(obj) );
    send_to_char( buf, ch );

    sprintf( buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
	obj->level, obj->cost, obj->condition, obj->timer );
    send_to_char( buf, ch );

    sprintf( buf,
	"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" : 
	    can_see(ch,obj->carried_by) ? obj->carried_by->name
				 	: "someone",
	obj->wear_loc );
    send_to_char( buf, ch );
    
    sprintf( buf, "Values: %d %d %d %d %d\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	obj->value[4] );
    send_to_char( buf, ch );
    
    /* now give out vital statistics as per identify */
    
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
	    sprintf( buf, "Has %d(%d) charges of level %d",
	    	obj->value[1], obj->value[2], obj->value[0] );
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

	  i = 0;

	  while( liq_table[ i ].liq_color != NULL )
	    {
	      i++;
	    }

	  if( obj->value[ 2 ] >= i )
	    {
	      sprintf( buf, "Liquid has invalid v2.\n\r" );
	    }
	  else
	    {
	      sprintf(buf,"It holds %s %s.\n\r",
		      liq_table[obj->value[2]].liq_color,
		      liq_table[obj->value[2]].liq_name);
	    }

	    send_to_char(buf,ch);
	    break;
		
      
    	case ITEM_WEAPON:
 	    send_to_char("Weapon type is ",ch);
	    switch (obj->value[0])
	    {
	    	case(WEAPON_EXOTIC): 
		    send_to_char("exotic\n\r",ch);
		    break;
	    	case(WEAPON_SWORD): 
		    send_to_char("sword\n\r",ch);
		    break;	
	    	case(WEAPON_DAGGER): 
		    send_to_char("dagger\n\r",ch);
		    break;
	    	case(WEAPON_SPEAR):
		    send_to_char("spear\n\r",ch);
		    break;
	    	case(WEAPON_MACE): 
		    send_to_char("mace\n\r",ch);	
		    break;
	   	case(WEAPON_AXE): 
		    send_to_char("axe\n\r",ch);	
		    break;
	    	case(WEAPON_FLAIL): 
		    send_to_char("flail\n\r",ch);
		    break;
	    	case(WEAPON_WHIP): 
		    send_to_char("whip\n\r",ch);
		    break;
	    	case(WEAPON_POLEARM): 
		    send_to_char("polearm\n\r",ch);
		    break;
	    	case(WEAPON_STAFF): 
		    send_to_char("staff\n\r",ch);
		    break;
	        case(WEAPON_HAND):
		    send_to_char("hand\n\r",ch);
		    break;
		case(WEAPON_BOW):
		    send_to_char("bow\n\r",ch);
		    break;
	    	default: 
		    send_to_char("unknown\n\r",ch);
		    break;
 	    }
	    if (obj->pIndexData->new_format)
	    	sprintf(buf,"Damage is %dd%d (average %d)\n\r",
		    obj->value[1],obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
	    else
	    	sprintf( buf, "Damage is %d to %d (average %d)\n\r",
	    	    obj->value[1], obj->value[2],
	    	    ( obj->value[1] + obj->value[2] ) / 2 );
	    send_to_char( buf, ch );

	    sprintf(buf,"Damage noun is %s.\n\r",
		(obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ?
		    attack_table[obj->value[3]].noun : "undefined");
	    send_to_char(buf,ch);
	    
	    if (obj->value[4])  /* weapon flags */
	    {
	        sprintf(buf,"Weapons flags: %s\n\r",
		    weapon_bit_name(obj->value[4]));
	        send_to_char(buf,ch);
            }
	break;

    	case ITEM_ARMOR:
	    sprintf( buf, 
	    "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
	        obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    send_to_char( buf, ch );
	break;

        case ITEM_CONTAINER:
        case ITEM_BOOK:
            sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
                obj->value[0], obj->value[3], 
                flag_string( container_flags, obj->value[1]));
            send_to_char(buf,ch);
            if (obj->value[4] != 100 && obj->item_type == ITEM_CONTAINER)
            {
                sprintf(buf,"Weight multiplier: %d%%\n\r",
		    obj->value[4]);
                send_to_char(buf,ch);
            }
            else if ( obj->item_type == ITEM_BOOK )
            {
                sprintf(buf,"Sale royalties: %d%%\n\r",
		    obj->value[4]);
                send_to_char(buf,ch);
            }
        break;

    case ITEM_QUIVER:
      sprintf( buf, "Arrow capacity: %d, flags: %s\n\r",
	       obj->value[ 0 ],
               flag_string( container_flags, obj->value[1]));
      send_to_char( buf, ch );
      break;
    }


    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
	    	send_to_char( " ", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
      if (paf->location == APPLY_SPELL_AFFECT) {
	  sprintf(buf, "Adds spell affect '%s', level %d.\n\r",
		  skill_table[paf->type].name, paf->level);
	  send_to_char(buf, ch);
      }
      else {
	sprintf( buf, "Affects %s by %d, level %d",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char(buf,ch);
	if ( paf->duration > -1)
	    sprintf(buf,", %d hours.\n\r",paf->duration);
	else
	    sprintf(buf,".\n\r");
	send_to_char( buf, ch );
	if (paf->bitvector)
	{
	    switch(paf->where)
	    {
		case TO_AFFECTS:
		    sprintf(buf,"Adds %s affect.\n",
			flag_string( affect_flags, paf->bitvector));
		    break;
                case TO_WEAPON:
                    sprintf(buf,"Adds %s weapon flags.\n",
                        weapon_bit_name(paf->bitvector));
		    break;
		case TO_OBJECT:
		    sprintf(buf,"Adds %s object flag.\n",
			extra_bit_name(paf->bitvector));
		    break;
		case TO_IMMUNE:
		    sprintf(buf,"Adds immunity to %s.\n",
                        flag_string( imm_flags, paf->bitvector ) );
/*                      imm_bit_name(paf->bitvector));*/
		    break;
		case TO_RESIST:
		    sprintf(buf,"Adds resistance to %s.\n\r",
                        flag_string( res_flags, paf->bitvector ) );
/*                      imm_bit_name(paf->bitvector)); */
		    break;
		case TO_VULN:
		    sprintf(buf,"Adds vulnerability to %s.\n\r",
                        flag_string( vuln_flags, paf->bitvector ) );
/*                      imm_bit_name(paf->bitvector));*/
		    break;
		default:
		    sprintf(buf,"Unknown bit %d: %d\n\r",
			paf->where,paf->bitvector);
		    break;
	    }
	    send_to_char(buf,ch);
	}
      }
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char( buf, ch );
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
                case TO_IMMUNE:
                    sprintf(buf,"Adds immunity to %s.\n",
                        flag_string( imm_flags, paf->bitvector ) );
/*                      imm_bit_name(paf->bitvector));*/
                    break;
                case TO_RESIST:
                    sprintf(buf,"Adds resistance to %s.\n\r",
                        flag_string( res_flags, paf->bitvector ) );
/*                      imm_bit_name(paf->bitvector)); */
                    break;
                case TO_VULN:
                    sprintf(buf,"Adds vulnerability to %s.\n\r",
                        flag_string( vuln_flags, paf->bitvector ) );
/*                      imm_bit_name(paf->bitvector)); */
                    break;
                default:
                    sprintf(buf,"Unknown bit %d: %d\n\r",
                        paf->where,paf->bitvector);
                    break;
            }
            send_to_char(buf,ch);
        }
    }

    return;
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;
    MOB_INDEX_DATA *mob_inx;
    int vnum, wizt, vist;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument, FALSE ) ) == NULL )
    {
      /* Look for vnum */
      if (is_number(arg)) {
	vnum = atoi(arg);
	mob_inx = get_mob_index( vnum );
	if ( mob_inx == NULL ||
	     ( victim = get_char_world( ch, mob_inx->player_name, FALSE ) ) == NULL ) {
	  send_to_char( "They aren't here.\n\r", ch );
	  return;

	}
      }
      else {
	  send_to_char( "They aren't here.\n\r", ch );
	  return;
      }
    }
	

    sprintf( buf, "Name: %s\n\r", victim->name);
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d  Format: %s  Race: %s  Group: %d  Sex: %s  Room: %d\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
	race_table[victim->race].name,
	IS_NPC(victim) ? victim->group : 0, sex_table[victim->sex].name,
	victim->in_room == NULL    ?        0 : victim->in_room->vnum
	);
    send_to_char( buf, ch );

    if (IS_NPC(victim))
    {
	sprintf(buf,"Count: %d  Killed: %d\n\r",
	    victim->pIndexData->count,victim->pIndexData->killed);
	send_to_char(buf,ch);
    }

    sprintf( buf, 
   	"Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR),
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX),
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON) );
    send_to_char( buf, ch );

    sprintf( buf, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move );
    send_to_char( buf, ch );
	
    if ( !IS_NPC(victim) && IS_ADMIN(ch) )
    {
        if ( IS_VAMPIRE(victim) )
        {
            sprintf( buf, "  Blood: %d/%d\n\r", 
                victim->blood, victim->max_blood );
            send_to_char( buf, ch );
        }
        else
            send_to_char( "\n\r", ch );
        sprintf(buf2, "Gold: %ld/%ld  Silver: %ld/%ld",
            victim->gold, victim->pcdata->gold_bank,
            victim->silver, victim->pcdata->silver_bank );
    }
    else
    {
        send_to_char( "\n\r", ch );
	sprintf(buf2, "Gold: %ld  Silver: %ld",
	    victim->gold, victim->silver );
    }

    sprintf( buf,
	"Lv: %d  Class: %s  %s  Exp: %d\n\r",
	victim->level,       
	IS_NPC(victim) ? "mobile" : class_table[victim->class].name,            
	buf2, victim->exp );
    send_to_char( buf, ch );

    sprintf(buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
	    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
	    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));
    send_to_char(buf,ch);

    sprintf( buf, 
	"Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\n\r",
	GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
	size_table[victim->size].name, position_table[victim->position].name,
	victim->wimpy );
    send_to_char( buf, ch );

    if (IS_NPC(victim) && victim->pIndexData->new_format)
    {
	sprintf(buf, "Damage: %dd%d  Message:  %s\n\r",
	    victim->damagedice[DICE_NUMBER],victim->damagedice[DICE_TYPE],
	    attack_table[victim->dam_type].noun);
	send_to_char(buf,ch);
    }
 
    if ( !IS_NPC(victim) )
    {
        int fightlag = 0, pk = 0;
        if ( in_fightlag(victim) )
        {
            fightlag = FIGHT_LAG - (int) 
                (current_time - victim->pcdata->lastfight);
            if ( (current_time - victim->pcdata->lastpk) < FIGHT_LAG ) pk = 1;
        }
        sprintf(buf2, "  Fightlag: %ds%s", fightlag, ( pk ? " (in PK)" : "") );
    }
    else
        buf2[0] = '\0';

    sprintf( buf, "Fighting: %s%s\n\r",
	victim->fighting ? victim->fighting->name : "(none)",
        buf2 );
    send_to_char( buf, ch );

    sprintf( buf, "Items:   %4d/%4d   Weight: %ld/%d\n\r",
	victim->carry_number, can_carry_n(victim),
        get_carry_weight(victim) / 10, can_carry_w(victim) / 10 );
    send_to_char( buf, ch );


    if (!IS_NPC(victim))
      {
    	sprintf( buf, 
	    "Played: %10d   Last Level: %5d   Timer:    %d\n\r",
	    (int) (victim->played + current_time - victim->logon) / 3600, 
	    victim->pcdata->last_level, 
	    victim->timer );
    	send_to_char( buf, ch );
    }

    if ( !IS_NPC(victim) && IS_IMMORTAL( victim )
    && (  victim == ch   || IS_ADMIN( ch ) ) )
    {
        wizt = victim->pcdata->wizitime / 3600;
        vist = victim->pcdata->vistime / 3600;
	sprintf( buf, "Visible: %5d/%2d%c   Wizi:   %5d/%2d%c   Total:    %d\n\r",
	    vist, 100 * vist / UMAX(1,(vist + wizt)), '%',
            wizt, 100 * wizt / UMAX(1,(vist + wizt)), '%',
            vist + wizt );
        send_to_char(buf, ch );
    }

   if ( !IS_NPC( victim ) &&  is_guild( victim ) )
    {
	int guilded_time;
	guilded_time = (int)(victim->pcdata->guild_time + current_time - victim->logon );

	sprintf( buf,
                 "Guild Hours: %5d   Guild Logs: %5d   Average:  %d mins\n\r"
		 "Guild Date:  %s\n\r",
		 guilded_time / 3600,
		 victim->pcdata->guild_logs,
		 victim->pcdata->guild_logs > 0 ? ( guilded_time / 60 / victim->pcdata->guild_logs ) : 0 ,
		 smash_crlf( ctime( &( victim->pcdata->guild_date ) ) ) );
	send_to_char( buf, ch );

   } 
    if ( ( get_trust( ch ) == MAX_LEVEL || ch == victim )
    &&  !IS_NPC( victim ) 
    &&  !IS_NULLSTR( victim->pcdata->notetags ) )
    {
        sprintf(buf, "Note Tags:   %s\n\r", victim->pcdata->notetags );
        send_to_char( buf, ch );
    }

    sprintf(buf, "Act: %s\n\r",act_bit_name(victim->act_bits));
    send_to_char(buf,ch);
    
    if (victim->comm)
    {
    	sprintf(buf,"Comm: %s\n\r",flag_string(comm_flags, victim->comm));
    	send_to_char(buf,ch);
    }

    if( IS_ADMIN( ch ) && victim->shiftbits )
    {
    	sprintf(buf,"Shift Bits: %s\n\r",shift_bit_name(victim->shiftbits));
    	send_to_char(buf,ch);
	if (IS_SET(victim->shiftbits,TEMP_VAMP)) {
	  sprintf(buf,"Patriarch: %s\n\r",victim->patriarch);
	  send_to_char(buf,ch);
	}
    }

    if (IS_NPC(victim) && victim->off_flags)
    {
    	sprintf(buf, "Offense: %s\n\r",off_bit_name(victim->off_flags));
	send_to_char(buf,ch);
    }

    if (victim->imm_flags)
    {
	sprintf(buf, "Immune: %s\n\r",/*imm_bit_name(victim->imm_flags)*/
            flag_string(imm_flags,victim->imm_flags));
	send_to_char(buf,ch);
    }
 
    if (victim->res_flags)
    {
	sprintf(buf, "Resist: %s\n\r", /*res_bit_name(victim->res_flags)*/
            flag_string(res_flags,victim->res_flags));
	send_to_char(buf,ch);
    }

    if (victim->vuln_flags)
    {
	sprintf(buf, "Vulnerable: %s\n\r", /*vuln_bit_name(victim->vuln_flags)*/
            flag_string(vuln_flags,victim->vuln_flags));
	send_to_char(buf,ch);
    }

    sprintf(buf, "Form: %s\n\rParts: %s\n\r", 
	form_bit_name(victim->form), part_bit_name(victim->parts));
    send_to_char(buf,ch);

    if (victim->affected_by)
    {
	sprintf(buf, "Affected by %s\n\r", 
	    flag_string(affect_flags, victim->affected_by));
	send_to_char(buf,ch);
    }

    if (victim->affected_by2)
    {
	sprintf(buf, "Affect2: %s\n\r",
		flag_string(affect2_flags, victim->affected_by2));
	send_to_char(buf,ch);
    }

    if (victim->affected_by3)
    {
	sprintf(buf, "Affect3: %s\n\r",
		flag_string(affect3_flags, victim->affected_by3));
	send_to_char(buf,ch);
    }
    sprintf( buf, "Master: %s  Leader: %s  Pet: %s\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	victim->pet 	    ? victim->pet->name	     : "(none)");
    send_to_char( buf, ch );
    if ( IS_ADMIN( ch ) && IS_NPC( victim ) && !IS_NULLSTR( victim->charmed_by ) )
    {
        sprintf( buf,    "Charmed by: %s\n\r", victim->charmed_by );
        send_to_char( buf, ch );
        sprintf( buf,    "Last charmed %d minutes ago.\n\r", (int)((current_time - victim->logon)/60) );
        send_to_char( buf, ch );
    }

    if (!IS_NPC(victim))
    { 	
        sprintf( buf, "Security: %3d   Practices: %3d  Trains: %3d\n\r", 
            victim->pcdata->security, 
	    victim->practice,
            victim->train );
	send_to_char( buf, ch );

        sprintf( buf, "Deity: %6.6s   Cleansings: %2d",
            god_table[victim->pcdata->god].name, victim->pcdata->cleansings );
/*      sprintf( buf,    "Cleansings: %d   Deity:  %s\n\r", victim->pcdata->cleansings,
            god_table[victim->pcdata->god].name ); */
        send_to_char( buf, ch );

        if ( ( IS_ADMIN( ch ) || ch->guild == guild_lookup("covenance") )
        &&   victim->pcdata->last_cleanse != 0 )
        {
            sprintf( buf, " (%d days ago)\n\r", 
                (int) (current_time - victim->pcdata->last_cleanse) / 86400 );
            send_to_char( buf, ch );
        }
        else
            send_to_char( "\n\r", ch );
        if ( IS_ADMIN( ch ) && victim->pcdata->offering != NULL )
        {
            sprintf(buf, "Offering to: %s\n\r", victim->pcdata->offering->name );
            send_to_char( buf, ch );
        }
    }

    if ( !IS_NPC(victim)
    &&  get_trust( ch ) == MAX_LEVEL
    && ( victim->pcdata->fake_ip[ 0 ] != '\0' ) )
      {
	sprintf( buf, "Fake IP: %s\n\r", victim->pcdata->fake_ip );
	send_to_char( buf, ch );
      }

    if (IS_NPC(victim) && !IS_NULLSTR(victim->pIndexData->image) )
    {
        sprintf( buf,
        "Default Pueblo Image: %s.\n\r", victim->pIndexData->image );
        send_to_char( buf, ch );
    }

    if ( !IS_NULLSTR(victim->image) )
    {
	sprintf( buf,
	"Current Pueblo Image: %s.\n\r", victim->image );
	send_to_char( buf, ch );
    }

    if ( !IS_NULLSTR(victim->shiftimage1) )
    {
	sprintf( buf,
	"Halfshift Image: %s.\n\r", victim->shiftimage1 );
	send_to_char( buf, ch );
    }

    if ( !IS_NULLSTR(victim->shiftimage2) )
    {
	sprintf( buf,
	"Fullshift Image: %s.\n\r", victim->shiftimage2 );
	send_to_char( buf, ch );
    }

    sprintf( buf, "Short description: %s\n\rLong description:  %s",
	!IS_NULLSTR(victim->short_descr) ? victim->short_descr : "(none)",
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
    send_to_char( buf, ch );
	
    if ( !IS_NPC(victim) 
    && ( IS_ADMIN( ch ) || ch->guild == guild_lookup("arcaenum") ) )
    {
        if ( !IS_NULLSTR(victim->pcdata->famname) )
        {
          sprintf( buf, "Familiar name:     %s\n\r", victim->pcdata->famname );
          send_to_char( buf, ch );
        }
        if ( !IS_NULLSTR(victim->pcdata->famshort) )
        {
          sprintf( buf, "Familiar short:    %s\n\r", victim->pcdata->famshort );
          send_to_char( buf, ch );
        }
        if ( !IS_NULLSTR(victim->pcdata->famlong) )
        {
          sprintf( buf, "Familiar long:     %s", victim->pcdata->famlong );
          send_to_char( buf, ch );
        }
    }

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
    {
	sprintf(buf,"Mobile has special procedure %s.\n\r",
		spec_name(victim->spec_fun));
	send_to_char(buf,ch);
    }

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
     if (paf->type >= 0){
         if(paf->whichaff == AFF1)
        sprintf( buf,
            "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
            skill_table[(int) paf->type].name,
            affect_loc_name( paf->location ),
            paf->modifier,
            paf->duration,
            flag_string( affect_flags, paf->bitvector ),
            paf->level
            );
         if(paf->whichaff == AFF2)
        sprintf( buf,
            "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
            skill_table[(int) paf->type].name,
            affect_loc_name( paf->location ),
            paf->modifier,
            paf->duration,
            flag_string( affect2_flags, paf->bitvector ),
            paf->level
            );

         if(paf->whichaff == AFF3)
        sprintf( buf,
            "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
            skill_table[(int) paf->type].name,
            affect_loc_name( paf->location ),
            paf->modifier,
            paf->duration,
            flag_string( affect3_flags, paf->bitvector ),
            paf->level
            );
        send_to_char( buf, ch );
        }

    }
    if ( global_config_aprilfools && !IS_NPC( victim ) )
    {
        sprintf( buf, "April fools: %s\n\r", ( victim->pcdata->aprilfools ? 
            "Waiting to be fooled" : "Already fooled" ) );
        send_to_char( buf, ch );
    }

    return;

}

/* ofind and mfind replaced with vnum, vnum skill also added */
void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument,arg);
 
    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  vnum obj name <name>\n\r",ch);
	send_to_char("  vnum obj type <type>\n\r", ch );
	send_to_char("  vnum mob <name>\n\r",ch);
	send_to_char("  vnum skill <skill or spell>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_ofind(ch,string);
 	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    { 
	do_mfind(ch,string);
	return;
    }

    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
        int sn;
        char buf[MAX_STRING_LENGTH];
        if ( *string == '\'' || *string == '"' )
        {
            one_argument(string,arg);
            string = arg;
        }
        if ( ( sn = skill_lookup( string ) ) < 0 )
	{
	    send_to_char( "No such skill or spell.\n\r", ch );
	}
        else
        {
            sprintf( buf, "The '%s' ability is number %d.\n\rRemember that this number may change in the future, but your items will be okay.\n\r",
	        skill_table[sn].name, sn );
            send_to_char( buf, ch );
        }
	return;
    }
    /* do both */
    do_mfind(ch,argument);
    do_ofind(ch,argument);
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH] = "";
  char arg[MAX_INPUT_LENGTH] = "";
  MOB_INDEX_DATA *pMobIndex = 0;
  int vnum = 0;
  int nMatch = 0;
  bool fAll = FALSE;
  bool found = FALSE;

  one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
      send_to_char( "Find whom?\n\r", ch );
      return;
    }


  /*
   * Yeah, so iterating over all vnum's takes 10,000 loops.
   * Get_mob_index is fast, and I don't feel like threading another link.
   * Do you?
   * -- Furey
   */

  /*    for ( vnum = 0; nMatch < top_mob_index; vnum++ ) */

  printf_to_char( ch, "Top Mob Vnum: %d\n\r", top_vnum_mob );

  for( vnum = 0; vnum <= top_vnum_mob; vnum++ )
    {
      pMobIndex = get_mob_index( vnum );

      if( pMobIndex != NULL )
	{
	  nMatch++;
	  if ( fAll || is_name( argument, pMobIndex->player_name ) )
	    {
	      found = TRUE;
	      sprintf( buf, "[%5d] %s\n\r",
		       pMobIndex->vnum, pMobIndex->short_descr );
	      send_to_char( buf, ch );
	    }
	}
    }

  if ( !found )
    send_to_char( "No mobiles by that name.\n\r", ch );

  return;
}

/* top_obj_vnum doesn't do what people think it does...it's a reference */
/* counter, not a vnum tracker. We need a helper function to calculate  */
/* the highest theoretical vnum */

void do_ofind( CHAR_DATA* ch, char* argument )
{
  char buf[ MAX_STRING_LENGTH ];
  char query[ MAX_INPUT_LENGTH ];
  char target[ MAX_INPUT_LENGTH ];
  int vnum = 0;
  int matches = 0;
  int targetType;
  int highestVnum = max_vnum_lookup();
  bool byType = FALSE;
  bool itemMatch = FALSE;
  OBJ_INDEX_DATA* pObjIndex = NULL;
  
  if( argument == NULL || *argument == '\0' )
    {
      send_to_char( "Syntax:\n\r", ch );
      send_to_char( "vnum obj name <name>\n\r", ch );
      send_to_char( "vnum obj type <type>\n\r", ch );
      return;
    }

  argument = one_argument( argument, query );
  argument = one_argument( argument, target );
  if( *query == '\0' || *target == '\0' )
    {
      send_to_char( "Syntax:\n\r", ch );
      send_to_char( "vnum obj name <name>\n\r", ch );
      send_to_char( "vnum obj type <type>\n\r", ch );
      return;
    }
  
  if( !str_cmp( query, "type" ) )
    {
      byType = TRUE;

      targetType = item_lookup( target );
      if( targetType == -1 )
	{
	  sprintf( buf, "%s isn't a valid obj type.\n\r", query );
	  send_to_char( buf, ch );
	  return;
	}
    }
  else if( str_cmp( query, "name" ) )
    {
      send_to_char( "You have to search 'vnum obj' by name or by type.\n\r", ch );
      return;
    }

  for( vnum = 0; vnum < highestVnum; vnum++ )
    {
      pObjIndex = get_obj_index( vnum );
      if( pObjIndex )
	{
	  itemMatch = FALSE;
	  
	  if( byType )
	    {  
	      if( pObjIndex->item_type == targetType )
		{
		  itemMatch = TRUE;
		}
	    }
	  else if( is_name( target, pObjIndex->name ) )
	    {
	      itemMatch = TRUE;
	    }
	  
	  if( itemMatch )
	    {
	      matches++;
	      sprintf( buf, "[%5d] %s\n\r", pObjIndex->vnum, pObjIndex->short_descr );
	      send_to_char( buf, ch );
	    }
	}
    }
  
  sprintf( buf, "\n\rSummary: %d match%s found\n\r", matches, (matches == 1 ? "" : "es" ) );
  send_to_char( buf, ch );

}

void do_owhere(CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char *oarg;
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found, noimm;
    int number = 0, max_found;
    int trust;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf();

    if (argument[0] == '\0')
    {
	send_to_char("Find what?\n\r",ch);
	return;
    }

    oarg = argument;
    argument = one_argument( argument, arg );

    if ( !str_cmp(arg,"noimm") || !str_cmp(arg,"noimms") )
        noimm = TRUE;
    else
    {
        argument = oarg;
        noimm = FALSE;
    }
 
    trust = get_trust(ch);
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || !is_name( argument, obj->name ) )
            continue;

        /* regular imms can only see objects as high as level 60 */
        if ( obj->level > MAX_LEVEL && trust < ADMIN)
            continue;

        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
            ;

        if ( in_obj->carried_by != NULL )
        {
            if ( noimm && IS_IMMORTAL(in_obj->carried_by) )
                continue;
/*          Protection against crooked imms.  Sad that it has to come to this. */
            if ( in_obj->carried_by->invis_level > get_trust( ch )
            ||   in_obj->carried_by->incog_level > get_trust( ch ) )
                continue;
        }

        found = TRUE;
        number++;
 
        if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
	&&   in_obj->carried_by->in_room != NULL)
            sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
                number, obj->short_descr,PERS(in_obj->carried_by, ch),
		in_obj->carried_by->in_room->vnum );
        else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
            sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
                number, obj->short_descr,in_obj->in_room->name, 
	   	in_obj->in_room->vnum);
	else
            sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr);
 
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
}


void do_mwhere( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  BUFFER *buffer;
  CHAR_DATA *victim;
  bool found;
  int count = 0;
  
  if ( argument[0] == '\0' )
    {
      DESCRIPTOR_DATA *d;
      
      /* show characters logged */
      
      buffer = new_buf();
      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  if (d->character != NULL && d->connected == CON_PLAYING
	      &&  d->character->in_room != NULL && can_see(ch,d->character)
	      &&  can_see_room(ch,d->character->in_room))
	    {
	      victim = d->character;
	      count++;
	      if (d->original != NULL)
		sprintf(buf,"%3d) %s (in the body of %s) is in {c%s{x [%d]\n\r",
			count, d->original->name,victim->short_descr,
			victim->in_room->name,victim->in_room->vnum);
	      else
		sprintf(buf,"%3d) %s is in {c%s{x [%d]\n\r",
			count, victim->name,victim->in_room->name,
			victim->in_room->vnum);
	
	      add_buf(buffer,buf);
	    }
	}
      
      page_to_char(buf_string(buffer),ch);
      free_buf(buffer);
      return;
    }
  
  found = FALSE;
  buffer = new_buf();
  for ( victim = char_list; victim != NULL; victim = victim->next )
    {
      if ( victim->in_room != NULL
	   &&   is_name( argument, victim->name )
	   && can_see( ch, victim ) )
	{
	  found = TRUE;
	  count++;
	  sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
		   IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		   IS_NPC(victim) ? victim->short_descr : victim->name,
		   victim->in_room->vnum,
		   victim->in_room->name );
	  add_buf(buffer,buf);
	}
    }
  
  if ( !found )
    act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
  else
    page_to_char(buf_string(buffer),ch);
  
  free_buf(buffer);
  
  return;
}



void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;

    if (ch->invis_level < LEVEL_HERO)
    {
    	sprintf( buf, "Reboot by %s.", ch->name );
    	do_echo( ch, buf );
    }

    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;
	if (vch != NULL)
	    save_char_obj(vch);
    	close_socket(d);
    }
    save_bankaccts();
    save_brawlers();
    saveClans();
    
    return;
}



void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;

    sprintf( buf, "Shutdown by %s.\n", 
      ( ch->invis_level < LEVEL_HERO ? ch->name : "someone" ) );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\r" );

    do_echo( ch, buf );
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next)
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;
	if (vch != NULL)
	    save_char_obj(vch);
	close_socket(d);
    }
    save_bankaccts();
    save_brawlers();
    saveClans();

    return;
}

void do_protect( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	send_to_char("Protect whom from snooping?\n\r",ch);
	return;
    }

    if ((victim = get_char_world(ch,argument, FALSE)) == NULL)
    {
	send_to_char("You can't find them.\n\r",ch);
	return;
    }

    if (IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
	act_new("$N is no longer snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD, 0);
	send_to_char("Your snoop-proofing was just removed.\n\r",victim);
	REMOVE_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
    else
    {
	act_new("$N is now snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD,0);
	send_to_char("You are now immune to snooping.\n\r",victim);
	SET_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
}
  
void do_transmogrify( CHAR_DATA* ch, char* argument )
{
  char vict_name[ MAX_INPUT_LENGTH ] = "";
  char duration[ MAX_INPUT_LENGTH ] = "";
  int ticks = -1;
  CHAR_DATA* victim = 0;
  AFFECT_DATA af;

  argument = one_argument( argument, vict_name );
  one_argument( argument, duration );

  if ( victim == ch ) {
	send_to_char("You're dumb. Don't do that. Seriously. No.\n\r", ch);
	return;
	}
	
  if( vict_name[ 0 ] == '\0' )
    {
      send_to_char( "Transmogrify who?\n\r", ch );
      return;
    }

  if( duration[ 0 ] != '\0' )
    ticks = atoi( duration ) - 1;

  victim = get_char_world( ch, vict_name, FALSE );

  if( IS_NPC( victim ) )
    {
      send_to_char( "Not on NPCs\n\r", ch );
      return;
    }
  if ( get_trust( victim ) > get_trust ( ch ) || victim->name == "Zalyriel" ) // Because fuck that shit.
  {
      send_to_char( "You failed.  Miserably.\n\r", ch );
      return;
  } 

  act( "You summon immortal magic to transmogrify $N.", ch, NULL, victim, TO_CHAR );
  act( "$n summons immortal power and transforms you into a frog!", ch, NULL, victim, TO_VICT );
  act( "$n summons immortal magic and transmogrifies $N!", ch, NULL, victim, TO_ROOM );

  af.where = TO_AFFECTS;
  af.type = -6;
  af.level = ch->level;
  af.duration = ticks;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_FROG;
  affect_to_char3( victim, &af );
 

}

void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    CHAR_DATA *snooper;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
		/* print out a list of who's snooping whom if high enough level */
		if (ch->level < LEVEL_ADMIN ){
		send_to_char( "Snoop whom?\n\r", ch );
		return;
	}
		else 
		{
			send_to_char("Immortal:   Victim:      Snoop Started At:\n\r",ch);
			send_to_char("==========================================\n\r",ch);
			
			for ( d = descriptor_list; d != NULL; d = d->next ) 
			{
				if ( d->snoop_by != NULL) 
				{
					/* make sure both pointers to char data are valid */
					if (d->snoop_by->character)
					{
					
						if (d->character)
						{
							snooper = d->snoop_by->character;
						}
						
						if (ch->level >= snooper->level)
						{
							victim = d->character;
							sprintf(buf,"%-12s%-14s%s\r",
							(IS_NPC(ch) ? snooper->short_descr : snooper->name),
							(IS_NPC(ch) ? victim->short_descr : victim->name),
							ctime(&(d->snoop_time)));
							send_to_char(buf,ch);
						}
					}	
				}
			}
			
			return;
         }
    }

    if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
    }

    if ( victim->desc == NULL )
    {
		send_to_char( "No descriptor to snoop.\n\r", ch );
		return;
    }

    if ( victim == ch )
    {
		send_to_char( "Cancelling all snoops.\n\r", ch );
		wiznet("$N stops being such a snoop.",
		ch,NULL,WIZ_SNOOPS,WIZ_SECURE,59);
		
		for ( d = descriptor_list; d != NULL; d = d->next )
		{
			if ( d->snoop_by == ch->desc )
			d->snoop_by = NULL;
		}
		
		return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
		send_to_char( "Busy already.\n\r", ch );
		return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        send_to_char("That character is in a private room.\n\r",ch);
        return;
    }

    if ( get_trust( victim ) > get_trust( ch ) 
    ||   IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
		send_to_char( "You failed.\n\r", ch );
		return;
    }

    if ( ch->desc != NULL )
    {
		for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
		{
			if ( d->character == victim || d->original == victim )
			{
				send_to_char( "No snoop loops.\n\r", ch );
				return;
			}
		}
    }

    victim->desc->snoop_by = ch->desc;
    victim->desc->snoop_time = time( NULL );
    sprintf(buf,"$N starts snooping on %s",
	(IS_NPC(ch) ? victim->short_descr : victim->name));
	
    wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,59);
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int counting_colors;
    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char("That character is in a private room.\n\r",ch);
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    sprintf(buf,"$N switches into %s",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,0,get_trust(ch));

    /* kill OLC modes so Sidonie doesn't crash the mud */
    ch->desc->pEdit = NULL;
    ch->desc->editor = 0;

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    /* change communications to match */
    if (ch->prompt != NULL)
        victim->prompt = str_dup(ch->prompt);
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    /* By Varien's request --- lets see if we can bring color to switching */
    for(counting_colors = 0;counting_colors!=20;counting_colors++)
    {
      victim->colors[counting_colors] = ch->colors[counting_colors];
    }

    send_to_char( "Ok.\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    send_to_char( 
"You return to your original body. Type replay to see any missed tells.\n\r", 
	ch );
    if (ch->prompt != NULL)
    {
	free_string(ch->prompt);
	ch->prompt = NULL;
    }

    sprintf(buf,"$N returns from %s.",ch->short_descr);
    wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,0,get_trust(ch));
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc; 
    ch->desc                  = NULL;
    return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (IS_TRUSTED(ch,GOD)
	|| (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
	|| (IS_TRUSTED(ch,DEMI)	    && obj->level <= 10 && obj->cost <= 500)
	|| (IS_TRUSTED(ch,ANGEL)    && obj->level <=  5 && obj->cost <= 250)
	|| (IS_TRUSTED(ch,AVATAR)   && obj->level ==  0 && obj->cost <= 100))
	return TRUE;
    else
	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData,0);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char rest[ MAX_INPUT_LENGTH];
    CHAR_DATA *mob;
    OBJ_DATA  *obj;
    int number=0;
    number = mult_argument(argument,arg);
    one_argument(argument,rest);
    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch,argument, FALSE);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch,rest, FALSE);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch,argument, FALSE);
	obj = get_obj_here(ch,arg, FALSE);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone = NULL;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	for ( number=UMAX(1,number) ; number != 0; number--)
	{
            clone = create_object(obj->pIndexData,0); 
	    clone_object(obj,clone);
	    if (obj->carried_by != NULL)
	        obj_to_char(clone,ch);
	    else
	        obj_to_room(clone,ch->in_room);
 	    recursive_clone(ch,obj,clone);
	}
        if ( clone != NULL )
        {
	    act("$n has created $p.",ch,clone,NULL,TO_ROOM);
	    act("You clone $p.",ch,clone,NULL,TO_CHAR);
	    wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
        }
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;
	char buf[MAX_STRING_LENGTH];

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	if ((mob->level > 20 && !IS_TRUSTED(ch,GOD))
	||  (mob->level > 10 && !IS_TRUSTED(ch,IMMORTAL))
	||  (mob->level >  5 && !IS_TRUSTED(ch,DEMI))
	||  (mob->level >  0 && !IS_TRUSTED(ch,ANGEL))
	||  !IS_TRUSTED(ch,AVATAR))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);
	clone_mobile(mob,clone); 
	
	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_obj = create_object(obj->pIndexData,0);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room);
        act("$n has created $N.",ch,NULL,clone,TO_ROOM);
        act("You clone $N.",ch,NULL,clone,TO_CHAR);
	sprintf(buf,"$N clones %s.",clone->short_descr);
	wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
        return;
    }
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  load mob <vnum>\n\r",ch);
	send_to_char("  load obj <vnum> <level>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_mload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_oload(ch,argument);
	return;
    }
    /* echo syntax */
    do_load(ch,"");
}


void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    sprintf(buf,"$N loads %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,0,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "Syntax: load obj <vnum> <level>.\n\r", ch );
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
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,0,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && !IS_SET(victim->act_bits,ACT_NOPURGE) 
	    &&   victim != ch /* safety precaution */ )
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
	      extract_obj( obj );
	}

	act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {

	if (ch == victim)
	{
	  send_to_char("Ho ho ho.\n\r",ch);
	  return;
	}

	if (get_trust(ch) < get_trust(victim))
	{
	  send_to_char("Maybe that wasn't a good idea...\n\r",ch);
	  sprintf(buf,"%s tried to purge you!\n\r",ch->name);
	  send_to_char(buf,victim);
	  return;
	}
        else if ((get_trust(ch) == get_trust(victim)) &&
                 (victim->desc)){
	  send_to_char("They're not linkdead yet!\n\r",ch);
	  sprintf(buf,"%s tried to purge you!\n\r",ch->name);
	  send_to_char(buf,victim);
	  return;
	}

	act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);

    	if (victim->level > 1)
	    save_char_obj( victim );
    	d = victim->desc;
    	extract_char( victim, TRUE );
    	if ( d != NULL )
          close_socket( d );

	return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}



void do_advance( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1, FALSE ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > 60 )
    {
	send_to_char( "Level must be 1 to 60.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= victim->level )
    {
        int temp_prac;

	send_to_char( "Lowering a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim );
	temp_prac = victim->practice;
	victim->level    = 1;
	victim->exp      = exp_per_level(victim,victim->pcdata->points);
	victim->max_hit  = 10;
	victim->max_mana = 100;
	victim->max_move = 100;
	victim->practice = 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;
	advance_level( victim, FALSE );
	victim->practice = temp_prac;
    }
    else
    {
	send_to_char( "Raising a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
    }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
	victim->level += 1;
	advance_level( victim, FALSE );
    }
    sprintf(buf,"You are now level %d.\n\r",victim->level);
    send_to_char(buf,victim);
    victim->exp   = exp_per_level(victim,victim->pcdata->points) 
		  * UMAX( 1, victim->level );
    victim->trust = 0;
    save_char_obj(victim);
    return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1, FALSE ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > 60 )
    {
	send_to_char( "Level must be 0 (reset) or 1 to 60.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust.\n\r", ch );
	return;
    }

    victim->trust = level;
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */
    	
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            affect_strip(vch,gsn_plague);
            affect_strip(vch,gsn_poison);
            affect_strip(vch,gsn_blindness);
            affect_strip(vch,gsn_sleep);
            affect_strip(vch,gsn_curse);
            affect_strip(vch,gsn_wilt);
            affect_strip(vch,gsn_decay);
            affect_strip(vch,gsn_atrophy);
            affect_strip(vch,gsn_blister);
            
            vch->hit 	= vch->max_hit;
            vch->mana	= vch->max_mana;
            vch->move	= vch->max_move;
	    if (vch->pcdata){
	       vch->pcdata->condition[COND_HUNGER] = 42;
	       vch->pcdata->condition[COND_THIRST] = 42;
	       vch->pcdata->condition[COND_DRUNK] = 0;
	       vch->pcdata->condition[COND_FULL] = 36;
	       }
            update_pos( vch);
            act("$n has restored you.",ch,NULL,vch,TO_VICT);
        }

        sprintf(buf,"$N restored room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
        
        send_to_char("Room restored.\n\r",ch);
        return;

    }
    
    /* restore the quest? */
  if ( !str_cmp( arg, "quest" ) )
    {
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->connected == CON_PLAYING
            &&   d->character != ch
            &&   d->character->in_room != NULL
            &&   can_see( ch, d->character ) 
            &&  IS_SET(d->character->act_bits,PLR_QUEST))
            {
                vch = d->character;              
            affect_strip(vch,gsn_plague);
            affect_strip(vch,gsn_poison);
            affect_strip(vch,gsn_blindness);
            affect_strip(vch,gsn_sleep);
            affect_strip(vch,gsn_curse);
            affect_strip(vch,gsn_wilt);
            affect_strip(vch,gsn_decay);
            affect_strip(vch,gsn_atrophy);
            affect_strip(vch,gsn_blister);

                vch->hit    = vch->max_hit;
                vch->mana   = vch->max_mana;
                vch->move   = vch->max_move;
            act("$n has restored you.",ch,NULL,vch,TO_VICT);
            if (vch->pcdata){
               vch->pcdata->condition[COND_HUNGER] = 42;
               vch->pcdata->condition[COND_THIRST] = 42;
               vch->pcdata->condition[COND_DRUNK] = 0;
               vch->pcdata->condition[COND_FULL] = 36;
               }

            }
        }
        return;
    }

    if ( get_trust(ch) >=  MAX_LEVEL - 1 && !str_cmp(arg,"all"))
    {
    /* cure all */
    	
        for (d = descriptor_list; d != NULL; d = d->next)
        {
	    victim = d->character;

	    if (victim == NULL || IS_NPC(victim))
		continue;
                
            affect_strip(victim,gsn_plague);
            affect_strip(victim,gsn_poison);
            affect_strip(victim,gsn_blindness);
            affect_strip(victim,gsn_sleep);
            affect_strip(victim,gsn_curse);
            affect_strip(victim,gsn_wilt);
            affect_strip(victim,gsn_decay);
            affect_strip(victim,gsn_atrophy);
            affect_strip(victim,gsn_blister);
            
            victim->hit 	= victim->max_hit;
            victim->mana	= victim->max_mana;
            victim->move	= victim->max_move;
	    if (victim->pcdata){
	      victim->pcdata->condition[COND_HUNGER] = 42;
	      victim->pcdata->condition[COND_THIRST] = 42;
	      victim->pcdata->condition[COND_DRUNK] = 0;
	      victim->pcdata->condition[COND_FULL] = 36;
	      }
            update_pos( victim);
	    if (victim->in_room != NULL)
                act("$n has restored you.",ch,NULL,victim,TO_VICT);
        }
	send_to_char("All active players restored.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg, TRUE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    affect_strip(victim,gsn_plague);
    affect_strip(victim,gsn_poison);
    affect_strip(victim,gsn_blindness);
    affect_strip(victim,gsn_sleep);
    affect_strip(victim,gsn_curse);
    affect_strip(victim,gsn_wilt);
    affect_strip(victim,gsn_decay);
    affect_strip(victim,gsn_atrophy);
    affect_strip(victim,gsn_blister);

    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    if (victim->pcdata){
      victim->pcdata->condition[COND_HUNGER] = 42;
      victim->pcdata->condition[COND_THIRST] = 42;
      victim->pcdata->condition[COND_DRUNK] = 0;
      victim->pcdata->condition[COND_FULL] = 36;
      }
    update_pos( victim );
    act( "$n has restored you.", ch, NULL, victim, TO_VICT );
    sprintf(buf,"$N restored %s",
	IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}

 	
void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act_bits, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act_bits, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
	sprintf(buf,"$N thaws %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->act_bits, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
	sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    save_char_obj( victim );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act_bits, PLR_LOG) )
    {
	REMOVE_BIT(victim->act_bits, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act_bits, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
  char arg[ MAX_INPUT_LENGTH ] = "";
  char times[ MAX_INPUT_LENGTH ] = "";
  char buf[ MAX_STRING_LENGTH ] = "";
  CHAR_DATA* victim = NULL;
  static const time_t MINUTE = 60;
  static const time_t HOUR = 3600;
  static const time_t DAY = 86400;
  time_t duration = time( NULL );
  bool timed = FALSE;

  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
      send_to_char( "Noemote whom?\n\r", ch );
      return;
    }

  if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  argument = one_argument( argument, times );
  if( *times != '\0' )
    {
      duration += ( atoi( times ) * DAY );
      timed = TRUE;
    }

  argument = one_argument( argument, times );
  if( *times != '\0' )
    {
      duration += ( atoi( times ) * HOUR );
      timed = TRUE;
    }

  argument = one_argument( argument, times );
  if( *times != '\0' )
    {
      duration += ( atoi( times ) * MINUTE );
      timed = TRUE;
    }
  
  if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }

  if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
      REMOVE_BIT(victim->comm, COMM_NOEMOTE);
      send_to_char( "You can emote again.\n\r", victim );
      send_to_char( "NOEMOTE removed.\n\r", ch );
      sprintf(buf,"$N restores emotes to %s.",victim->name);
      wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);

      if( !IS_NPC( victim ) )
	{
	  victim->pcdata->restore_emotes = 0;
	}
    }
  else
    {
      SET_BIT(victim->comm, COMM_NOEMOTE);
      send_to_char( "You can't emote!\n\r", victim );
      send_to_char( "NOEMOTE set.\n\r", ch );
      sprintf(buf,"$N revokes %s's emotes.",victim->name);
      wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);

      if( !IS_NPC( victim ) && timed )
	{
	  victim->pcdata->restore_emotes = duration;
	}
    }

  return;
}



void do_noshout( CHAR_DATA *ch, char *argument )
{
  char arg[ MAX_INPUT_LENGTH ] = "";
  char times[ MAX_INPUT_LENGTH ] = "";
  char buf[ MAX_STRING_LENGTH ] = "";
  CHAR_DATA* victim = NULL;
  static const time_t MINUTE = 60;
  static const time_t HOUR = 3600;
  static const time_t DAY = 86400;
  time_t duration = time( NULL );
  bool timed = FALSE;

  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
      send_to_char( "Noshout whom?\n\r", ch );
      return;
    }

  if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  argument = one_argument( argument, times );
  if( *times != '\0' )
    {
      duration += ( atoi( times ) * DAY );
      timed = TRUE;
    }

  argument = one_argument( argument, times );
  if( *times != '\0' )
    {
      duration += ( atoi( times ) * HOUR );
      timed = TRUE;
    }

  argument = one_argument( argument, times );
  if( *times != '\0' )
    {
      duration += ( atoi( times ) * MINUTE );
      timed = TRUE;
    }
  
  if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }

  if ( IS_SET(victim->comm, COMM_NOSHOUT) )
    {
      REMOVE_BIT(victim->comm, COMM_NOSHOUT);
      send_to_char( "You can shout again.\n\r", victim );
      send_to_char( "NOSHOUT removed.\n\r", ch );
      sprintf(buf,"$N restores shout to %s.",victim->name);
      wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);

      if( !IS_NPC( victim ) )
	{
	  victim->pcdata->restore_shouts = 0;
	}
    }
  else
    {
      SET_BIT(victim->comm, COMM_NOSHOUT);
      send_to_char( "You can't shout!\n\r", victim );
      send_to_char( "NOSHOUT set.\n\r", ch );
      sprintf(buf,"$N revokes %s's shouts.",victim->name);
      wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);

      if( !IS_NPC( victim ) && timed )
	{
	  victim->pcdata->restore_shouts = duration;
	}
    }

  return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
  char arg[ MAX_INPUT_LENGTH ] = "";
  char times[ MAX_INPUT_LENGTH ] = "";
  char buf[ MAX_STRING_LENGTH ] = "";
  CHAR_DATA* victim = NULL;
  static const time_t MINUTE = 60;
  static const time_t HOUR = 3600;
  static const time_t DAY = 86400;
  time_t duration = time( NULL );
  bool timed = FALSE;

  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
      send_to_char( "Notell whom?\n\r", ch );
      return;
    }

  if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  argument = one_argument( argument, times );
  if( *times != '\0' )
    {
      duration += ( atoi( times ) * DAY );
      timed = TRUE;
    }

  argument = one_argument( argument, times );
  if( *times != '\0' )
    {
      duration += ( atoi( times ) * HOUR );
      timed = TRUE;
    }

  argument = one_argument( argument, times );
  if( *times != '\0' )
    {
      duration += ( atoi( times ) * MINUTE );
      timed = TRUE;
    }
  
  if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }

  if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
      REMOVE_BIT(victim->comm, COMM_NOTELL);
      send_to_char( "You can send tells again.\n\r", victim );
      send_to_char( "NOTELL removed.\n\r", ch );
      sprintf(buf,"$N restores tells to %s.",victim->name);
      wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);

      if( !IS_NPC( victim ) )
	{
	  victim->pcdata->restore_tells = 0;
	}
    }
  else
    {
      SET_BIT(victim->comm, COMM_NOTELL);
      send_to_char( "You can't send tells!\n\r", victim );
      send_to_char( "NOTELL set.\n\r", ch );
      sprintf(buf,"$N revokes %s's tells.",victim->name);
      wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);

      if( !IS_NPC( victim ) && timed )
	{
	  victim->pcdata->restore_tells = duration;
	}
    }

  return;
}



void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL )
	    stop_fighting( rch, TRUE );
	if (IS_NPC(rch) && IS_SET(rch->act_bits,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act_bits,ACT_AGGRESSIVE);
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
    {
	wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
	send_to_char( "Game wizlocked.\n\r", ch );
    }
    else
    {
	wiznet("$N removes wizlock.",ch,NULL,0,0,0);
	send_to_char( "Game un-wizlocked.\n\r", ch );
    }

    return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
    extern bool newlock;
    newlock = !newlock;
 
    if ( newlock )
    {
	wiznet("$N locks out new characters.",ch,NULL,0,0,0);
        send_to_char( "New characters have been locked out.\n\r", ch );
    }
    else
    {
	wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
        send_to_char( "Newlock removed.\n\r", ch );
    }
 
    return;
}



/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set mob        <name> <field> <value>\n\r",ch);
	send_to_char("  set obj        <name> <field> <value>\n\r",ch);
	send_to_char("  set room       <room> <field> <value>\n\r",ch);
        send_to_char("  set skill      <name> <spell or skill> <value>\n\r",ch);
	send_to_char("  set tradeskill <name> <skill> <value>\n\r", ch );
	return;
    }

    if( !str_prefix( arg, "tradeskill" ) )
      {
	do_tset( ch, argument );
	return;
      }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	do_mset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
	do_sset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	do_oset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"room"))
    {
	do_rset(ch,argument);
	return;
    }
    /* echo syntax */
    do_set(ch,"");
}

extern int* get_character_skill( CHAR_DATA* ch, TRADE_DATA* trade );

void do_tset( CHAR_DATA* ch, char* argument )
{
  char name[ MAX_INPUT_LENGTH ];
  char skill[ MAX_INPUT_LENGTH ];
  char value[ MAX_INPUT_LENGTH ];
  char output[ MAX_STRING_LENGTH ];
  int int_value;
  TRADE_DATA* current_trade = NULL;
  extern TRADE_DATA trade_matrix[];
  CHAR_DATA* victim = NULL;
  int* current_skill;
  bool found = FALSE;
  bool all = FALSE;

  argument = one_argument( argument, name );
  argument = one_argument( argument, skill );
  argument = one_argument( argument, value );

  if( name[ 0 ] == '\0' || skill[ 0 ] == '\0' || value[ 0 ] == '\0' )
    {
      send_to_char( "Syntax:\n\r", ch );
      send_to_char( "  set tradeskill <name> <skill> <value>\n\r", ch );
      return;
    }

  if( ( victim = get_char_world( ch, name, TRUE ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  if( IS_NPC( victim ) )
    {
      send_to_char( "Not on NPCs.\n\r", ch );
      return;
    }

  if( strcasecmp( skill, "all" ) == 0 )
    all = TRUE;

  int_value = atoi( value );
  current_trade = &( trade_matrix[ 0 ] );

  while( !found && current_trade->trade != 0 )
    {
      if( !all && ( strcasecmp( current_trade->trade_name, skill ) == 0 ) )
	{
	  found = TRUE;
	}

      current_skill = get_character_skill( victim, current_trade );

      if( found || all )
	{
	  if( int_value > current_trade->skill_cap )
	    {
	      sprintf( output, "Limits for %s are between 0 and %d.\n\r",
		       current_trade->trade_name, current_trade->skill_cap );

	      send_to_char( output, ch );
	    }
	  else
	    {
	      sprintf( output, "%s set to %d for %s.\n\r",
		       current_trade->trade_name,
		       int_value,
		       victim->name );

	      send_to_char( output, ch );

	      *current_skill = int_value;
	    }
	}

      current_trade++;
    }

  if( !found && !all )
    send_to_char( "No such tradeskill.\n\r", ch );

  return;

}


void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char( "  set skill <name> all <value>\n\r",ch);  
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
		victim->pcdata->learned[sn]	= value;
	}
        sprintf(buf, "All of %s skills have been set to %d%c.",
            ( victim == ch ) ? "your" : "$S", 
            value, '%' );
        act( buf, ch, NULL, victim, TO_CHAR );
    }
    else
    {
	victim->pcdata->learned[sn] = value;
        sprintf(buf, "%s \'%s\' has been set to %d%c.",
            ( victim == ch ) ? "Your" : "$S",
            ( skill_table[sn].name != NULL ) ? skill_table[sn].name : "reserved",
            victim->pcdata->learned[sn],
            '%' );
        act( buf, ch, NULL, victim, TO_CHAR );
    }

    return;
}



void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char("Syntax:\n\r",ch);
        send_to_char("  set char <name> <field> <value>\n\r", ch); 
        send_to_char( "  Field being one of:\n\r", ch );
        send_to_char( "    str int wis dex con sex class level god\n\r", ch);
        send_to_char( "    race group gold silver hp mana move prac\n\r", ch);
	send_to_char( "    clanleader clanname\n\r", ch);
        if ( IS_ADMIN( ch ) )
        {
            send_to_char( "    train drunk security were cleansings\n\r", ch );
            send_to_char( "    vampire blood maxblood energy maxenergy enochian\n\r", ch );
        }
        else if ( !IS_NPC(ch) 
        &&   ch->guild == guild_lookup("covenance") )
            send_to_char( "    train drunk security were cleansings\n\r", ch );
        else
            send_to_char( "    train drunk security\n\r", ch );


        return;
    }

    if ( ( victim = get_char_world( ch, arg1, FALSE ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

  /*
   * Set something.
   */
  if ( !str_cmp( arg2, "str" ) )
    {
      if ( value < 3 || value > get_max_train(victim,STAT_STR) )
	{
	  sprintf(buf,
		  "Strength range is 3 to %d.\n\r",
		  get_max_train(victim,STAT_STR));
	  send_to_char(buf,ch);
	  return;
	}

      victim->perm_stat[STAT_STR] = value;
      return;
    }

  if ( !str_cmp( arg2, "security" ) )	/* OLC */
    {
      if ( IS_NPC(ch) )
	{
	  send_to_char( "Si, claro.\n\r", ch );
	  return;
	}

      if ( IS_NPC( victim ) )
        {
	  send_to_char( "Not on NPC's.\n\r", ch );
	  return;
        }

      if ( value > ch->pcdata->security || value < 0 )
	{
	  if ( ch->pcdata->security != 0 )
	    {
	      sprintf( buf, "Valid security is 0-%d.\n\r",
		       ch->pcdata->security );
	      send_to_char( buf, ch );
	    }
	  else
	    {
	      send_to_char( "Valid security is 0 only.\n\r", ch );
	    }
	  return;
	}
      victim->pcdata->security = value;
      return;
    }

  if ( !str_cmp( arg2, "int" ) )
    {
      if ( value < 3 || value > get_max_train(victim,STAT_INT) )
        {
	  sprintf(buf,
		  "Intelligence range is 3 to %d.\n\r",
		  get_max_train(victim,STAT_INT));
	  send_to_char(buf,ch);
	  return;
        }
 
      victim->perm_stat[STAT_INT] = value;
      return;
    }

  if ( !str_cmp( arg2, "wis" ) )
    {
      if ( value < 3 || value > get_max_train(victim,STAT_WIS) )
	{
	  sprintf(buf,
		  "Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
	  send_to_char( buf, ch );
	  return;
	}

      victim->perm_stat[STAT_WIS] = value;
      return;
    }

  if ( !str_cmp( arg2, "dex" ) )
    {
      if ( value < 3 || value > get_max_train(victim,STAT_DEX) )
	{
	  sprintf(buf,
		  "Dexterity ranges is 3 to %d.\n\r",
		  get_max_train(victim,STAT_DEX));
	  send_to_char( buf, ch );
	  return;
	}

      victim->perm_stat[STAT_DEX] = value;
      return;
    }

  if ( !str_cmp( arg2, "con" ) )
    {
      if ( value < 3 || value > get_max_train(victim,STAT_CON) )
	{
	  sprintf(buf,
		  "Constitution range is 3 to %d.\n\r",
		  get_max_train(victim,STAT_CON));
	  send_to_char( buf, ch );
	  return;
	}

      victim->perm_stat[STAT_CON] = value;
      return;
    }

  if ( !str_prefix( arg2, "sex" ) )
    {
      if ( value < 0 || value > 2 )
	{
	  send_to_char( "Sex range is 0 to 2.\n\r", ch );
	  return;
	}
      victim->sex = value;
      if (!IS_NPC(victim))
	victim->pcdata->true_sex = value;
      return;
    }

  if ( !str_prefix( arg2, "class" ) )
    {
      int class;

      if (IS_NPC(victim))
	{
	  send_to_char("Mobiles have no class.\n\r",ch);
	  return;
	}

      class = class_lookup(arg3);
      if ( class == -1 )
	{
	  char buf[MAX_STRING_LENGTH];

	  strcpy( buf, "Possible classes are: " );
	  for ( class = 0; class < MAX_CLASS; class++ )
	    {
	      if ( class > 0 )
		strcat( buf, " " );
	      strcat( buf, class_table[class].name );
	    }
	  strcat( buf, ".\n\r" );

	  send_to_char(buf,ch);
	  return;
	}

      victim->class = class;
      return;
    }

  if ( !str_prefix( arg2, "level" ) )
    {
      if ( !IS_NPC(victim) )
	{
	  send_to_char( "Not on PC's.\n\r", ch );
	  return;
	}

      if ( value < 0 || value > 60 )
	{
	  send_to_char( "Level range is 0 to 60.\n\r", ch );
	  return;
	}
      victim->level = value;
      return;
    }

  if (!str_prefix(arg2,"god"))
    {
      int god;

      if ( IS_NPC(victim) )
      {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
      }
 
      god = god_lookup(arg3);

      if( god < 0 )
        {
	  send_to_char("That is not a valid god for that character.\n\r",ch);
	  return;
        }

      victim->pcdata->god = god;
      return;
    }

  if ( !str_prefix( arg2, "gold" ) )
    {
      victim->gold = value;
      return;
    }

  if ( !str_prefix(arg2, "silver" ) )
    {
      victim->silver = value;
      return;
    }

  if ( !str_prefix( arg2, "hp" ) )
    {
      if ( value < -10 || value > 30000 )
	{
	  send_to_char( "Hp range is -10 to 30,000 hit points.\n\r", ch );
	  return;
	}
      victim->max_hit = value;
      if (!IS_NPC(victim))
	victim->pcdata->perm_hit = value;
      return;
    }

  if ( !str_prefix( arg2, "were" )
  &&  ( IS_ADMIN( ch ) || (!IS_NPC(ch) && ch->guild == guild_lookup("covenance") ) ) )
    {
      if ( value == 1)
	{
	  SET_BIT(victim->shiftbits,SHIFT_POTENTIAL);
	  send_to_char("Were set.\n\r",ch);
	}
      else {
	REMOVE_BIT(victim->shiftbits,SHIFT_POTENTIAL);
	send_to_char("Were cleared.\n\r",ch);
      }
      return;
    }
	

/* vampire stuff is for admins only */
    if ( IS_ADMIN( ch ) )
    {
        if ( !str_prefix( arg2, "vampire" ) )
        {
            if ( IS_NPC(victim) )
            {
                send_to_char( "Not on NPCs.\n\r", ch );
                return;
            }
    
            if ( !str_cmp(arg3, "pat")
            ||   !str_cmp(arg3, "patriarch")
            ||   value == 1 )
            {
                REMOVE_BIT(victim->shiftbits, TEMP_VAMP);
                SET_BIT(victim->shiftbits,PERM_VAMP);
                free_string(victim->patriarch);
                victim->patriarch = str_dup(victim->name);
    
                victim->blood = victim->level;
                victim->max_blood = 750;
                victim->blood_percentage = 100;
    
                act( "$E is now a patriarch.", ch, NULL, victim, TO_CHAR );
            }
            else if ( !str_cmp(arg3, "off" )
            ||        !str_cmp(arg3, "none" )
            ||        value == 0 )
            {
                REMOVE_BIT(victim->shiftbits,PERM_VAMP|TEMP_VAMP);
                send_to_char("Vampire cleared.\n\r",ch);
                free_string(victim->patriarch);
                victim->patriarch = &str_empty[0];
            }
            else if ( !str_cmp(arg3, "minion" ) )
            {
                REMOVE_BIT(victim->shiftbits, PERM_VAMP);
                SET_BIT(victim->shiftbits,TEMP_VAMP);
                free_string(victim->patriarch);
                victim->patriarch = str_dup("Kiyanne");
    
                victim->blood = victim->level;
                victim->max_blood = 250;
                victim->blood_percentage = 100;
    
                act( "$E is now a rogue minion.", ch, NULL, victim, TO_CHAR );
    
            }
            else if ( !str_cmp(arg3, "fixedbp" ) )
            {
                if ( !IS_SET( victim->shiftbits, TEMP_VAMP ) )
                    send_to_char("Only minions gain max bp over time.\n\r", ch);
                else
                {
                    SET_BIT(victim->shiftbits,SHIFT_FIXEDBP);
                    act( "$E will no longer gain max bp over time.",
                        ch, NULL, victim, TO_CHAR );
                }
            }
            else if ( !str_cmp(arg3, "unfixedbp" ) )
            {
                REMOVE_BIT(victim->shiftbits,SHIFT_FIXEDBP);
                send_to_char( "Ok.\n\r", ch );
            }
            else
            {
                send_to_char( 
"Valid options are pat, minion, fixedbp, unfixedbp, and none.\n\r", ch );
            }
            return;
        }

        if ( !str_cmp( arg2, "maxbp" ) || !str_prefix( arg2, "maxbloodpool" ) )
        {
            if ( !IS_VAMPIRE(victim) )
            {
                send_to_char( "This can only be done on vampires.\n\r", ch );
                return;
            }
            if ( value < 1 || value > 1000 )
            {
                send_to_char("Blood pool must be between 1 and 1000.\n\r", ch);
                return;
            }
            victim->max_blood = value;
            return;
        }
        if ( !str_cmp( arg2, "bp" ) || !str_prefix( arg2, "bloodpool" ) )
        {
            if ( !IS_VAMPIRE(victim) )
            {
                send_to_char( "This can only be done on vampires.\n\r", ch );
                return;
            }
            if ( value < 1 || value > victim->max_blood )
            {
                char buf[1024];
                sprintf(buf, "Blood pool must be between 1 and %d.\n\r", 
                victim->max_blood );
                send_to_char( buf, ch );
                return;
            }
            victim->blood = value;
            return;
        }
		
		/*Begin admin-only seraph sets*/
		
		// Set current energy level
		if ( !str_prefix( arg2, "energy" ) )
		{
		
			if (victim->race != race_lookup("seraph"))
				{
					send_to_char( "They're not a Seraph.\n\r", ch );
					return;
				}
				
			if ( value < 1 || value > victim->max_life_energy )
				{
					char buf[1024];
					sprintf(buf, "Value must be between 1 and %d.\n\r", 
					victim->max_life_energy );
					send_to_char( buf, ch );
					return;
				}
				
			victim->life_energy = value;
			send_to_char("Okay.\n\r",ch);
			return;
		}
		
		// Set enochian language
		if ( !str_prefix( arg2, "enochian" ) )
		{
				
			if ( value > 1 )
				{
					char buf[1024];
					sprintf(buf, "0 = No enochian. 1 = Enochian.");
					send_to_char( buf, ch );
					return;
				}
				
			victim->enochian = value;
			send_to_char("Okay.\n\r",ch);
			return;
		}
		
		// set max energy level
		if ( !str_prefix( arg2, "maxenergy" ) )
		{
		
			if (victim->race != race_lookup("seraph"))
				{
					send_to_char( "They're not a Seraph.\n\r", ch );
					return;
				}
				
			victim->max_life_energy = value;
			send_to_char("Okay.\n\r",ch);
			return;
		}
		
    } //end admin-only vamp & seraph sets
		
    if ( !str_prefix( arg2, "mana" ) )
    {
      if ( value < 0 || value > 30000 )
	{
	  send_to_char( "Mana range is 0 to 30,000 mana points.\n\r", ch );
	  return;
	}
      victim->max_mana = value;
      if (!IS_NPC(victim))
	victim->pcdata->perm_mana = value;
      return;
    }

  if ( !str_prefix( arg2, "move" ) )
    {
      if ( value < 0 || value > 30000 )
	{
	  send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
	  return;
	}
      victim->max_move = value;
      if (!IS_NPC(victim))
	victim->pcdata->perm_move = value;
      return;
    }

  if ( !str_prefix( arg2, "practice" ) )
    {
      if ( value < 0 || value > 250 )
	{
	  send_to_char( "Practice range is 0 to 250 sessions.\n\r", ch );
	  return;
	}
      victim->practice = value;
      return;
    }

  if ( !str_prefix( arg2, "train" ))
    {
      if (value < 0 || value > 50 )
	{
	  send_to_char("Training session range is 0 to 50 sessions.\n\r",ch);
	  return;
	}
      victim->train = value;
      return;
    }



  if ( !str_prefix( arg2, "drunk" ) )
    {
      if ( IS_NPC(victim) )
	{
	  send_to_char( "Not on NPC's.\n\r", ch );
	  return;
	}

      if ( value < -1 || value > 100 )
	{
	  send_to_char( "Drunk range is -1 to 100.\n\r", ch );
	  return;
	}

      victim->pcdata->condition[COND_DRUNK] = value;
      return;
    }

  if (!str_prefix( arg2, "race" ) )
    {
      int race;
      int old_race;
	
      old_race = victim->race;
      race = race_lookup(arg3);

      if ( race == 0)
	{
	  send_to_char("That is not a valid race.\n\r",ch);
	  return;
	}

      if (!IS_NPC(victim) && !race_table[race].pc_race)
	{
	  send_to_char("That is not a valid player race.\n\r",ch);
	  return;
	}

      victim->race = race;
	
      if( !IS_NPC( victim ) )
	{
	  int old_level;
	  int temp_trains;
	  int temp_practices;
	  int i;

	  old_level = victim->level;

	  /* reset stats */
	  for( i = 0; i < MAX_STATS; i++ )
	    {
	      victim->perm_stat[ i ] = pc_race_table[ race ].stats[ i ];
	    }
	  victim->affected_by = 0 | race_table[ race ].aff;
	  victim->affected_by2 = 0 | race_table[ race ].aff2;
	  victim->affected_by3 = 0 | race_table[ race ].aff3;
	  victim->imm_flags = 0 | race_table[ race ].imm;
	  victim->res_flags = 0 | race_table[ race ].res;
	  victim->vuln_flags = 0 | race_table[ race ].vuln;
	  victim->form = race_table[ race ].form;
	  victim->parts = race_table[ race ].parts;
	  victim->size = pc_race_table[ race ].size;

	  /* reset skills */
	  for( i = 0; i < 5; i++ )
	    {
	      if( pc_race_table[ old_race ].skills[ i ] != NULL )
		{
		  group_remove( victim, pc_race_table[ old_race ].skills[ i ] );
		}
	      if( pc_race_table[ race ].skills[ i ] != NULL )
		{
		  group_add( victim, pc_race_table[ race ].skills[ i ], FALSE );
		}
	    }

	  /* advance cycle to change hp/mana/move */
	  temp_trains = victim->train;
	  temp_practices = victim->practice;

	  victim->level = 1;
	  victim->exp = exp_per_level( victim, victim->pcdata->points );

	  int loc;
	  int mod;
	  OBJ_DATA* obj;
	  AFFECT_DATA* af;

	  victim->hit = 20;
	  victim->max_hit = 20;
	  victim->pcdata->perm_hit = 20;

	  victim->mana = 100;
	  victim->max_mana = 100;
	  victim->pcdata->perm_mana = 100;

	  victim->move = 100;
	  victim->max_move = 100;
	  victim->pcdata->perm_move = 100;

	  victim->practice = 0;
	  victim->train = 0;			    
	    
	  for( i = victim->level; i < old_level; i++ )
	    {
	      victim->level += 1;
	      advance_level( victim, TRUE );
	    }

	  victim->practice = temp_practices;
	  victim->train = temp_trains;

	  for( loc = 0; loc < MAX_WEAR; loc++ )
	    {
	      obj = get_eq_char( victim, loc );

	      if( obj == NULL )
		continue;

	      if( !obj->enchanted )
		{
		  for( af = obj->pIndexData->affected; af != NULL; af = af->next )
		    {
		      mod = af->modifier;

		      switch( af->location )
			{
			case APPLY_MANA:  victim->max_mana += mod; break;
			case APPLY_HIT:   victim->max_hit  += mod; break;
			case APPLY_MOVE:  victim->max_move += mod; break;
			case APPLY_BLOOD: victim->max_blood += mod; break;
			default: break;
			}
		    }
		}
	    }

	}

      return;
    }
   
  if (!str_prefix(arg2,"group"))
    {
      if (!IS_NPC(victim))
	{
	  send_to_char("Only on NPCs.\n\r",ch);
	  return;
	}
      victim->group = value;
      return;
    }

  if ( ( !str_prefix(arg2,"cleansings") || !str_prefix(arg2,"cleanse") ) 
  &&  ( IS_ADMIN( ch ) 
  ||  ( !IS_NPC(ch) && ch->guild == guild_lookup("covenance") ) ) )
  {
      if ( IS_NPC(victim) )
      {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
      }
      victim->pcdata->cleansings = UMAX(value,0);
      if ( victim->pcdata->cleansings != 0 )
          victim->pcdata->last_cleanse = current_time;
      else
          victim->pcdata->last_cleanse = 0;
      send_to_char( "Done.\n\r", ch );
      return;
  }

  /*
   * Generate usage message.
   */
  do_mset( ch, "" );
  return;
}

/* Kind of an obscure command.  I hope it can find some use in
   the future as far as using the string editor for altering
   non-permanent structures in the game (like objects) 

   This function ripped verbatim from do_description.          */

void do_buffer( CHAR_DATA *ch, char *argument )
{
  char buf[ MAX_STRING_LENGTH + 2*MAX_INPUT_LENGTH ];

  if( !str_cmp( argument, "write" ) )
    {
      string_append(ch, &ch->write_buffer);
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

          if (ch->write_buffer == NULL || ch->write_buffer[0] == '\0')
            {
              send_to_char("No lines left to remove.\n\r",ch);
              return;
            }

          strcpy(buf,ch->write_buffer);
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
                      free_string(ch->write_buffer);
                      ch->write_buffer = str_dup(buf);
                      send_to_char( "Your buffer contains:\n\r", ch );
                      send_to_char( !IS_NULLSTR(ch->write_buffer) ? ch->write_buffer :
                                    "  (Nothing)\n\r", ch );
                      return;
                    }
                }
            }

          buf[0] = '\0';
          free_string(ch->write_buffer);
          ch->write_buffer = str_dup(buf);
          send_to_char("Buffer cleared.\n\r",ch);
          return;
        }

      if ( argument[0] == '+' )
        {
          if ( ch->write_buffer != NULL )
            strcat( buf, ch->write_buffer );
          argument++;
          while ( isspace(*argument) )
            argument++;

          strcat( buf, argument );
          strcat( buf, "\n\r" );

          if( strlen(buf) >= MAX_STRING_LENGTH )
              send_to_char( "Buffer string too long.\n\r", ch );
          else
          {
              free_string( ch->write_buffer );
              ch->write_buffer = str_dup( buf );
          }

          return;
        }
    }

  send_to_char( "Your buffer contains:\n\r", ch );
  send_to_char( !IS_NULLSTR(ch->write_buffer) ? ch->write_buffer : "  (Nothing)\n\r", ch );
}


void do_string( CHAR_DATA *ch, char *argument )
{
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
	char buf [MAX_INPUT_LENGTH]; 
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool isarc;

    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );
    
    if ( !IS_NPC(ch)
    &&   ( IS_ADMIN(ch) || ch->guild == guild_lookup("arcaenum") ) )
        isarc = TRUE;
    else
        isarc = FALSE;

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  string char <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long desc title spec\n\r",ch);
        if ( isarc )
            send_to_char("            famshort famlong famname famdesc\n\r",ch);
        if ( IS_ADMIN( ch ) )
            send_to_char("            patriarch\n\r", ch );
	send_to_char("  string obj  <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long extended exwrite\n\r",ch);
	return;
    }
    
    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
    	if ( ( victim = get_char_world( ch, arg1, FALSE ) ) == NULL )
    	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
    	}

	/* string something */

     	if ( !str_prefix( arg2, "name" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }
	    free_string( victim->name );
	    victim->name = str_dup( arg3 );
	    return;
    	}
    	
    	if ( !str_prefix( arg2, "description" ) )
    	{
    	    free_string(victim->description);
            strcat(arg3,"\n\r");
    	    victim->description = str_dup(arg3);
    	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( victim->short_descr );
	    victim->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( victim->long_descr );
	    strcat(arg3,"\n\r");
	    victim->long_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "title" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }

	    set_title( victim, arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "spec" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }

	    if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	    {
	    	send_to_char( "No such spec fun.\n\r", ch );
	    	return;
	    }

	    return;
    	}

        if ( isarc )
        {
            if ( IS_NPC(victim) )
            {
                send_to_char( "Not on NPCs.\n\r", ch );
                return;
            }
            if ( !str_prefix( arg2, "famshort" ) )
            {
                free_string( victim->pcdata->famshort );
                victim->pcdata->famshort = str_dup( arg3 );
                return;
            }
            if ( !str_prefix( arg2, "famlong" ) )
            {
                free_string( victim->pcdata->famlong );
	        strcat(arg3,"\n\r");
                victim->pcdata->famlong = str_dup( arg3 );
                return;
            }
            if ( !str_prefix( arg2, "famname" ) )
            {
                free_string( victim->pcdata->famname );
                victim->pcdata->famname = str_dup( arg3 );
                return;
            }
            if ( !str_prefix( arg2, "famdesc" ) )
            {
                free_string( victim->pcdata->famdesc );
	        strcat(arg3,"\n\r");
                victim->pcdata->famdesc = str_dup( arg3 );
                return;
            }
        }
		
		/*** COALITION ***/
		
		  if ( !str_prefix( arg2, "clanname" ) )
            {
                if ( IS_NPC(victim) )
                {
                    send_to_char( "Not on NPCs.\n\r", ch );
                }
                else
                {
                    if ( !str_cmp( arg3, "none" ) )
                    {
                            free_string( victim->cln_leader );
                            victim->cln_leader = &str_empty[0];
							free_string( victim->cln_name );
                            victim->cln_name = &str_empty[0];
							send_to_char( "They now have no leader and no clan.\n\r", ch );
                    }
                    else
                        victim->cln_name = str_dup( arg3 );
						sprintf(buf, "%s is now a member of clan %s.\n\r", victim->name, victim->cln_name);
						send_to_char( buf, ch );
                }
            }
		
			if ( !str_prefix( arg2, "clanleader" ) )
            {
                if ( IS_NPC(victim) )
                {
                    send_to_char( "Not on NPCs.\n\r", ch );
					return;
                }
				
				if (IS_NULLSTR(ch->cln_name) ) 
				{
					send_to_char( "They aren't part of a clan.\n\r", ch );
					return;
				}
			
                if ( !str_cmp( arg3, "none" ) )
                    {
                            free_string( victim->cln_leader );
                            victim->cln_leader = &str_empty[0];
							free_string( victim->cln_name );
                            victim->cln_name = &str_empty[0];
							send_to_char( "They now have no leader and no clan.\n\r", ch );
							return;
                    }
          
                    victim->cln_leader = str_dup( arg3 );
					sprintf(buf, "%s is now led by %s.\n\r", victim->name, victim->cln_leader);
					send_to_char( buf, ch );
            }
			
			if ( !str_prefix( arg2, "clansymbol" ) )
            {
                if ( IS_NPC(victim) )
                {
                    send_to_char( "Not on NPCs.\n\r", ch );
					return;
                }
				
				if (IS_NULLSTR(ch->cln_name) ) 
				{
					send_to_char( "They aren't part of a clan.\n\r", ch );
					return;
				}
				
				if (str_cmp( victim->name, victim->cln_leader )) 
				{
					send_to_char( "They aren't the clan leader.\n\r", ch );
					return;
				}
			
                if ( !str_cmp( arg3, "none" ) )
                    {
                            free_string( victim->cln_symbol );
                            victim->cln_symbol = &str_empty[0];
							send_to_char( "Clan symbol cleared.\n\r", ch );
							return;
                    }
          
                    victim->cln_symbol = str_dup( arg3 );
					sprintf(buf, "Clan %s's symbol is now: %s \n\r", victim->cln_name, victim->cln_symbol);
					send_to_char( buf, ch );
            }
			
        if ( IS_ADMIN( ch ) )
        {
            if ( !str_prefix( arg2, "patriarch" ) )
            {
                if ( IS_NPC(victim) )
                {
                    send_to_char( "Not on NPCs.\n\r", ch );
                }
                else
                {
                    if ( !str_cmp( arg3, "none" ) )
                    {
                        if ( !IS_VAMPIRE( victim ) )
                        {
                            free_string( victim->patriarch );
                            victim->patriarch = &str_empty[0];
                        }
                        else
                        {
  act( "You should set $S vamp to 0 instead.", ch, NULL, victim, TO_CHAR );
                        }
                    }
                    else
                        victim->patriarch = str_dup( arg3 );
                }
            }
            return;
        }
    }
    
    if (!str_prefix(type,"object"))
    {
    	/* string an obj */
    	
   	if ( ( obj = get_obj_here( ch, arg1, FALSE ) ) == NULL )
    	{
	    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	    return;
    	}

        if ( !str_prefix( arg2, "name" ) )
    	{
	    free_string( obj->name );
	    obj->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( obj->description );
	    obj->description = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
	    	send_to_char( "Syntax: string <object> extended <keyword> <string>\n\r",
		    ch );
	    	return;
	    }

 	    strcat(argument,"\n\r");

	    ed = new_extra_descr();

	    ed->keyword		= str_dup( arg3     );
	    ed->description	= str_dup( argument );
	    ed->next		= obj->extra_descr;
	    obj->extra_descr	= ed;
	    return;
    	}

        if ( !str_prefix( arg2, "exwrite" ) )
        {
	    EXTRA_DESCR_DATA *ed;

	    one_argument( argument, arg3 );
	    if ( IS_NULLSTR( ch->write_buffer ) )
	    {
	    	send_to_char( "Syntax: string <object> exwrite <keyword>\n\r"
                              "Be sure to write the description in your buffer before doing this!\n\r",
		    ch );
	    	return;
	    }

	    ed = new_extra_descr();

	    ed->keyword		= str_dup( arg3     );
	    ed->description	= str_dup( ch->write_buffer );
	    ed->next		= obj->extra_descr;
	    obj->extra_descr	= ed;
 
            free_string(ch->write_buffer);
            ch->write_buffer = &str_empty[0];

            return;
        }
    }
    
    	
    /* echo bad use message */
    do_string(ch,"");
}

/* for unstringing stuff */
void do_unstring( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument,arg);

    if ( IS_NULLSTR(arg) || IS_NULLSTR(argument) )
    {
        send_to_char( "Syntax:\n\r  unstring <name> <field>\n\r    fields: short long both pretitle famshort famlong famname famdesc transin transout\n\r",ch);
        return;
    }

    if ( (victim = get_char_world( ch, arg, FALSE )) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPCs.\n\r", ch );
        return;
    }

    if ( !str_prefix( argument, "short" ) )
    {
        free_string( victim->short_descr );
        victim->short_descr = &str_empty[0];
        send_to_char( "Short description unstrung.\n\r", ch );
    }
    else if ( !str_prefix( argument, "long" ) )
    {
        free_string( victim->long_descr );
        victim->long_descr = &str_empty[0];
        send_to_char( "Long description unstrung.\n\r", ch );
    }
    else if ( !str_prefix( argument, "pretitle" ) )
    {
        free_string( victim->pcdata->pretitle );
        victim->pcdata->pretitle = &str_empty[0];
        send_to_char( "Pretitle unstrung.\n\r", ch );
    }
    else if ( !str_prefix( argument, "famshort" ) )
    {
        free_string( victim->pcdata->famshort );
        victim->pcdata->famshort = &str_empty[0];
        send_to_char( "Familiar short description unstrung.\n\r", ch );
    }
    else if ( !str_prefix( argument, "famlong" ) )
    {
        free_string( victim->pcdata->famlong );
        victim->pcdata->famlong = &str_empty[0];
        send_to_char( "Familiar long description unstrung.\n\r", ch );
    }
    else if ( !str_prefix( argument, "famname" ) )
    {
        free_string( victim->pcdata->famname );
        victim->pcdata->famname = &str_empty[0];
        send_to_char( "Familiar name unstrung.\n\r", ch );
    }
    else if ( !str_prefix( argument, "famdesc" ) )
    {
        free_string( victim->pcdata->famdesc );
        victim->pcdata->famdesc = &str_empty[0];
        send_to_char( "Familiar description unstrung.\n\r", ch );
    }
    else if ( !str_prefix( argument, "transin" ) )
    {
        free_string( victim->pcdata->transin );
        victim->pcdata->transin = &str_empty[0];
        send_to_char( "Transin unstring.\n\r", ch );
    }
    else if ( !str_prefix( argument, "transout" ) )
    {
        free_string( victim->pcdata->transout );
        victim->pcdata->transout = &str_empty[0];
        send_to_char( "Transout unstring.\n\r", ch );
    }
    else
        do_unstring( ch, "" );

    return;
}


void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",				ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",	ch );
	send_to_char("    extra wear level weight cost timer\n\r",		ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1, FALSE ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );
    one_argument( argument, arg3 );
    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = value;
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
        if ( obj->item_type == ITEM_POTION
        ||   obj->item_type == ITEM_PILL 
        ||   obj->item_type == ITEM_SCROLL )
        {
            if ( !is_number( arg3 ) )
                value = skill_lookup( arg3 );
            value = UMAX(0, value);
            obj->value[1] = value;
            sprintf(buf, "Value1 is now set to cast %s.\n\r", skill_table[value].name );
            send_to_char( buf, ch );
            return;
        }
        else
        {
            obj->value[1] = value;
	    return;
        }
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
        if ( obj->item_type == ITEM_POTION
        ||   obj->item_type == ITEM_PILL
        ||   obj->item_type == ITEM_SCROLL )
        {
            if ( !is_number( arg3 ) )
                value = skill_lookup( arg3 );
            value = UMAX(0, value);
            obj->value[2] = value;
            sprintf(buf, "Value2 is now set to cast %s.\n\r", skill_table[value].name );
            send_to_char( buf, ch );
            return;
        }
        else
        {
            obj->value[2] = value;
            return;
        }
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
        if ( obj->item_type == ITEM_WAND 
        ||   obj->item_type == ITEM_STAFF
        ||   obj->item_type == ITEM_POTION
        ||   obj->item_type == ITEM_PILL 
        ||   obj->item_type == ITEM_SCROLL )
        {
            if ( !is_number( arg3 ) )
                value = skill_lookup( arg3 );
            value = UMAX(0, value);
            obj->value[3] = value;
            sprintf(buf, "Value3 is now set to cast %s.\n\r", skill_table[value].name );
            send_to_char( buf, ch );
            return;
        }
        else
        {
            obj->value[3] = value;
            return;
        }
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
        if ( obj->item_type == ITEM_POTION
        ||   obj->item_type == ITEM_PILL 
        ||   obj->item_type == ITEM_SCROLL 
        ||   obj->item_type == ITEM_DRINK_CON
        ||   obj->item_type == ITEM_FOUNTAIN )
        {
            if ( !is_number( arg3 ) )
                value = skill_lookup( arg3 );
            value = UMAX(0, value);
            obj->value[4] = value;
            sprintf(buf, "Value4 is now set to cast %s.\n\r", skill_table[value].name );
            send_to_char( buf, ch );
            return;
        }
        else
        {
            obj->value[4] = value;
            return;
        }
    }

    if ( !str_prefix( arg2, "extra" ) )
    {
	obj->extra_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
	obj->wear_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
      if (value > MAX_LEVEL && get_trust(ch) < MAX_LEVEL )
      {
        send_to_char("You cannot set an object above the max pc level.\n\r",ch);
        return;
      }
      else
      {
	 obj->level = value;
	 return;
      }
    }
	
    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }
	
    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    heal_rate mana_rate light flags trans sector\n\r", ch);
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        send_to_char("That room is private right now.\n\r",ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "flags" ) )
    {
	location->room_flags	= value;
	return;
    }

    if ( !str_prefix( arg2, "trans" ) )
      {
	location->trans_flags = value;
	return;
      }

   if ( !str_prefix( arg2, "light" ) )
    {
        location->light         = value;
        return;
    }

    if ( !str_prefix( arg2, "heal_rate" ) )
    {
        location->heal_rate     = value;
        return;
    }

    if ( !str_prefix (arg2, "mana_rate" ) )
    {
        location->mana_rate     = value;
        return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_rset( ch, "" );
    return;
}



void do_sockets( CHAR_DATA *ch, char *argument )
{
  char buf[2 * MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  int count;
  
  count	= 0;
  buf[0]	= '\0';

  if( IS_NPC( ch ) )
    {
      send_to_char( "You can't do that while switched.\n\r", ch );
      return;
    }
  
  one_argument(argument,arg);
  for ( d = descriptor_list; d != NULL; d = d->next )
    {
      if ( d->character != NULL && can_see( ch, d->character ) 
	   && (arg[0] == '\0' || is_name(arg,d->character->name)
	       || (d->original && is_name(arg,d->original->name))))
	{
	  if(   ( get_trust( ch ) < 59 ) 
		&& ( d->character->level > 50 )
		&& ( d->character != ch ) )
	    continue;
	  count++;

	  sprintf( buf3, "%s (masked)", d->host );

	  sprintf( buf + strlen(buf), "[%3d %s] %s@%s\n\r",
		   d->descriptor,
		   GET_STATUS(d->connected),
		   d->original  ? d->original->name  :
		   d->character ? d->character->name : "(none)",
		   d->original  ? (d->original->level > ch->level ? "***.***.***.***" : d->host)
		   : (d->character->level > ch->level ? "***.***.***.***" : 
		      ( d->character->pcdata->fake_ip[ 0 ] != '\0' ?
			( ch->level < MAX_LEVEL ? d->character->pcdata->fake_ip : buf3 ) :
			d->host ) )
		);


	}
    }
    if (count == 0)
    {
	send_to_char("No one by that name is connected.\n\r",ch);
	return;
    }

    sprintf( buf2, "%d user%s\n\r", count, count == 1 ? "" : "s" );
    strcat(buf,buf2);
    page_to_char( buf, ch );
    return;
}

void do_whomulti( CHAR_DATA *ch, char *argument )
{
  char buf[2 * MAX_STRING_LENGTH];
  char nameslist[MAX_STRING_LENGTH * 4]; /* This will need to be upped in the event that
                                            the mud suddenly gets a HUGE amount of
                                            multiplayers. */
  char currentip[1024];
  char compip[1024];
  DESCRIPTOR_DATA *d;
  DESCRIPTOR_DATA *pd;
  int count = 0;
  int totcount = 0;
  bool ignoremask = FALSE;

  buf[0] = '\0';
  nameslist[0] = '\0';

  if ( !IS_NULLSTR( argument ) && !str_prefix(argument,"all") && get_trust(ch) == MAX_LEVEL )
    ignoremask = TRUE;

  for ( d = descriptor_list; d != NULL; d = d->next )
    {
      if ( d->character != NULL && can_see( ch, d->character ) )
        {
          if ( ( !ignoremask && IS_IMMORTAL( d->character ) ) || IS_NPC( d->character ) )
              continue;

          count = 0;
          if ( !ignoremask )
              strcpy( currentip, ( !IS_NULLSTR( d->character->pcdata->fake_ip ) ?
                  d->character->pcdata->fake_ip : d->host ) );
          else
              strcpy( currentip, d->host );

          for ( pd = descriptor_list; pd != NULL; pd = pd->next )
          {
              if ( pd->character == NULL )
                  continue;
              if ( ( !ignoremask && IS_IMMORTAL( pd->character ) ) || IS_NPC( pd->character ) )
                  continue;

              if ( !ignoremask )
                  strcpy( compip, ( !IS_NULLSTR( pd->character->pcdata->fake_ip ) ?
                                pd->character->pcdata->fake_ip : pd->host ) );
              else
                  strcpy( compip, pd->host );

              if ( d != pd
                   &&  !strcmp( currentip, compip )
                   &&  !is_name ( pd->character->name, nameslist ) )
              {
                  if ( count == 0 )
                  {
                      count++;
                      sprintf( buf + strlen(buf), "Characters from %s:\n\r", currentip );
                      sprintf( buf + strlen(buf), "  %s\n\r", d->character->name );
                      sprintf( nameslist + strlen(nameslist), " %s", d->character->name );
                  }
                  sprintf( buf + strlen(buf), "  %s\n\r", pd->character->name );
                  sprintf( nameslist + strlen(nameslist), " %s ", pd->character->name );
                  count++;
              }
          } 
          if ( count > 0 )
          {
              sprintf( buf + strlen(buf), "  Total: %d\n\r", count );
              totcount += count;
          }
        }
    }

    if (totcount == 0)
    {
        send_to_char("No multiple connections found.\n\r",ch);
        return;
    }

    page_to_char( buf, ch );
    return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    one_argument(argument,arg2);
  
    if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    sprintf( buf, "$n forces you to '%s'.", argument );

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (get_trust(ch) < MAX_LEVEL - 3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
	    {
		act( buf, ch, NULL, vch, TO_VICT );
		interpret( vch, argument );
	    }
	}
    }
    else if (!str_cmp(arg,"players"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) 
	    &&	 vch->level < LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else if (!str_cmp(arg,"gods"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
            &&   vch->level >= LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

    	if (!is_room_owner(ch,victim->in_room) 
	&&  ch->in_room != victim->in_room 
        &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    	{
            send_to_char("That character is in a private room.\n\r",ch);
            return;
        }

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	act( buf, ch, NULL, victim, TO_VICT );
	interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char arg[MAX_STRING_LENGTH];
    int level;
    bool deflevel;

    vch = ch;
    deflevel = FALSE;
    level = 0;
    argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
        deflevel = TRUE;
    else
    {
        if ( is_number( arg ) )
        {
            level = atoi( arg );
            if ( level == 0 )
                deflevel = TRUE;
        }
        else
        {
            if ( (vch = get_char_world( ch, arg, FALSE ) ) == NULL || IS_NPC(vch) )
            {
                send_to_char( "That player is not here.\n\r", ch );
                return;
            }
            if ( !IS_ADMIN( ch )  || !IS_IMMORTAL( vch ) )
                vch = ch;
            argument = one_argument( argument, arg );
            if ( IS_NULLSTR( arg ) || (level = atoi( arg )) == 0 )
                deflevel = TRUE;
        }
    }
    if ( deflevel )
    {
        if ( vch->invis_level > 0 )
            level = 0;
        else
            level = 51;
    }

    if (level != 0 && ( level < 2 || level > get_trust(vch)))
    {
        if ( vch == ch )
            send_to_char("Invis level must be between 2 and your level.\n\r",ch);
        else
            act( "Invis level must be between 2 and $S level.", ch, NULL, vch, TO_CHAR );
        return;
    }

    if ( level == 0 )
    {
        if ( vch != ch )
        {
            act( "$n is compelling you to become visible!", ch, NULL, vch, TO_VICT );
            act( "You compel $M to become visible!", ch, NULL, vch, TO_CHAR );
        }
        vch->invis_level = 0;
        act( "$n slowly fades into existence.", vch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly fade back into existence.\n\r", vch );
    }
    else
    {
        if ( vch != ch )
        {
            act( "$n is compelling you to disappear!", ch, NULL, vch, TO_VICT );
            act( "You compel $M to disappear!", ch, NULL, vch, TO_CHAR );
        }
        act( "$n slowly fades into thin air.", vch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly vanish into thin air.\n\r", vch );
        vch->invis_level = level;
    }

    return;
}

void do_racewizi( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];


    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mobiles can't do that.\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
      if ( ch->rinvis_level)
      {
          ch->rinvis_level = 0;
          send_to_char( "The shroud of darkness that surrounds you dissipates.\n\r", ch );
          act( "The shroud of darkness surrounding $n dissipates.", ch, NULL, NULL, TO_ROOM );
      }
      else
      {
          act( "A shroud of darkness descends upon $n.", ch, NULL, NULL, TO_ROOM );
          sprintf(buf, "A shroud of darkness surrounds you, rendering you visible only to %s.\n\r",
              race_table[ch->race].name );
          send_to_char( buf, ch );
          ch->rinvis_level = 30;
      }
    else
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
        send_to_char("Race invis level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
          ch->reply = NULL;
          act( "A shroud of darkness descends upon $n.", ch, NULL, NULL, TO_ROOM );
          sprintf(buf, "A shroud of darkness surrounds you, rendering you visible only to %s.\n\r",
              race_table[ch->race].name );
          send_to_char( buf, ch );
          ch->rinvis_level = level;
      }
    }

    return;
}


void do_incognito( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];
 
    /* RT code for taking a level argument */
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    /* take the default path */
 
      if ( ch->incog_level)
      {
          ch->incog_level = 0;
          act( "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You are no longer cloaked.\n\r", ch );
      }
      else
      {
          ch->incog_level = 51;
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
        send_to_char("Incog level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
          ch->reply = NULL;
          ch->incog_level = level;
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    }
 
    return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act_bits, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act_bits, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act_bits, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi (CHAR_DATA *ch, char *argument)
{
    send_to_char("You cannot abbreviate the prefix command.\r\n",ch);
    return;
}

void do_prefix (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if (argument[0] == '\0')
    {
	if (ch->prefix[0] == '\0')
	{
	    send_to_char("You have no prefix to clear.\r\n",ch);
	    return;
	}

	send_to_char("Prefix removed.\r\n",ch);
	free_string(ch->prefix);
	ch->prefix = &str_empty[0];
	return;
    }

    if (ch->prefix[0] != '\0')
    {
	sprintf(buf,"Prefix changed to %s.\r\n",argument);
	free_string(ch->prefix);
    }
    else
    {
	sprintf(buf,"Prefix set to %s.\r\n",argument);
    }

    ch->prefix = str_dup(argument);
}

void do_laston (CHAR_DATA *ch, char *argument)
{
  char charfile[120];
  char name[120];
  FILE *fp;
  time_t last_time;
  time_t current_time;
  int days;
  char *strtime;

  argument = preserve_case_one_argument(argument,name);
  
  if (name[0] == '\0')
    {
      send_to_char("Who do you want to check?\n\r",ch);
      return;
    }

  sprintf(charfile,"%s%s", PLAYER_DIR, capitalize( name ) );
  
  if ( (fp = fopen(charfile,"r")) == NULL)
    {
      send_to_char("Player not found.\n\r",ch);
      return;
    }

  /* GET BY PLAYER, name, ID and to LogO */
  fgets(charfile,120,fp);
  fgets(charfile,120,fp);
  fgets(charfile,120,fp);
  fgets(charfile,120,fp);

  fclose(fp);
    
  sscanf(charfile,"%*s %ld",&last_time);
  
  strtime                    = ctime( &last_time );
  current_time = time( NULL );

  days = ((int) current_time) - ((int) last_time);
  days /= 86400;

  sprintf( charfile,
	   "%s: (%d %s ago) %s",
	   capitalize( name ),
	   days,
	   ( days == 1 ? "day" : "days" ),
	   strtime );
  
  send_to_char( charfile, ch );

}

/*void do_skdump(CHAR_DATA *ch, char *argument) {
    
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int sn,i,cnt;
      
   one_argument(argument, arg);
    
   if(*arg == '\0')
   {
      send_to_char("Usage: skdump <skill>\n\r",ch);
      return;
   }
   
   if( (sn = skill_lookup(arg)) == -1 )
   {
      send_to_char("Skill not found.\n\r",ch);
      return;
   }
   
   sprintf(buf,"Skill: %s\n\r",skill_table[sn].name);
   send_to_char(buf,ch);
      
   cnt = 0;
   sprintf(buf,"Levels: \n\r");
   send_to_char(buf,ch);
   for( i = 0; i < MAX_CLASS; i++ )
   {
      if( skill_table[sn].skill_level[i] < 101 )
      {
        sprintf(buf,"%15s %3d      ",
        class_table[i].name,
        skill_table[sn].skill_level[i]);
        send_to_char(buf,ch);
   
        if ( !(++cnt % 3) )
           send_to_char("\n\r",ch);
      }
    
   }
   send_to_char("\n\r",ch);
        
   switch(skill_table[sn].target)
   {
        
      case TAR_IGNORE:
        sprintf(buf,"Target type: TAR_IGNORE\n\r");
        break;
       
      case TAR_CHAR_OFFENSIVE:
        sprintf(buf,"Target type: TAR_CHAR_OFFENSIVE\n\r");
        break;
        
      case TAR_CHAR_DEFENSIVE:   
        sprintf(buf,"Target type: TAR_CHAR_DEFENSIVE\n\r");
        break;
      
      case TAR_CHAR_SELF:
        sprintf(buf,"Target type: TAR_CHAR_SELF\n\r");
        break;
      
      case TAR_OBJ_INV:
        sprintf(buf,"Target type: TAR_OBJ_INV\n\r");
        break;
      
      case TAR_OBJ_CHAR_DEF:
        sprintf(buf,"Target type: TAR_OBJ_CHAR_DEF\n\r");
        break;
      
      case TAR_OBJ_CHAR_OFF:
        sprintf(buf,"Target type: TAR_OBJ_CHAR_OFF\n\r");
        break;
      
      default:
        sprintf(buf,"Target type: Unknown\n\r");
        break;
      
   }
   send_to_char(buf,ch);
      
   switch(skill_table[sn].minimum_position)
   {
      case POS_STANDING:
        sprintf(buf,"Min Pos: Standing\n\r");
        break;
      case POS_FIGHTING:
        sprintf(buf,"Min Pos: Fighting\n\r");
        break;
      case POS_SITTING:
        sprintf(buf,"Min Pos: Sitting\n\r");
        break;
      case POS_RESTING:
        sprintf(buf,"Min Pos: Resting\n\r");
        break;
      case POS_SLEEPING:
        sprintf(buf,"Min Pos: {rSleeping{x\n\r");
        break;
      default:
        sprintf(buf,"Warning: min pos is less than sleeping!\n\r");
        break;
     }
   send_to_char(buf,ch);
        
   sprintf(buf,"Min mana: %d     Lag: %d  (1 Round = 12)\n\r",
        skill_table[sn].min_mana,
        skill_table[sn].beats);
   send_to_char(buf,ch);
          
   sprintf(buf,"Damage message: %s\n\rWear off message: %s\n\r",
        skill_table[sn].noun_damage,
        skill_table[sn].msg_off);
   send_to_char(buf,ch);
      
   sprintf(buf,"Obj wear off message: %s\n\r",
        skill_table[sn].msg_obj);
   send_to_char(buf,ch);
        
   return;

}*/

void do_approve (CHAR_DATA *ch, char *argument)
{
CHAR_DATA *victim;
char name[120], buf[MAX_STRING_LENGTH];
   
    argument = one_argument(argument,name);
    if (name[0] == '\0'){
        send_to_char("Who do you want to approve?\n\r",ch);
        return;
        }

    if ( ( victim = get_char_world( ch, name, FALSE ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

   if (IS_SET(victim->act_bits,ACT_NOAPPROVE)){
      REMOVE_BIT(victim->act_bits,ACT_NOAPPROVE);
      send_to_char("Character approved.\n\r",ch);
      send_to_char("Character approved.\n\r",victim);
      sprintf(buf,"%s has approved %s",ch->name, victim->name);
      wiznet(buf, ch, NULL, WIZ_APPROVAL, 0, 0);
      if ( !IS_NPC( ch ) ) ch->pcdata->approvals++;
      }
   else
      {
      SET_BIT(victim->act_bits,ACT_NOAPPROVE);
      send_to_char("Approval removed.\n\r",ch);
      send_to_char("Approval removed.\n\r",victim);
      sprintf(buf,"$N has revoked %s's approval",victim->name);
      wiznet(buf, ch, NULL, WIZ_APPROVAL, 0, 0);
      if ( !IS_NPC( ch ) ) ch->pcdata->unapprovals++;
      }
}

void do_confiscate(CHAR_DATA *ch,char *argument)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj; 
    char arg1[MAX_INPUT_LENGTH];
    bool found = FALSE;
        
    argument = one_argument(argument,arg1);
    
    if (IS_NPC(ch))
      return;
      
    if ((argument[0] == '0') || (arg1[0] == '\0'))
    { 
      send_to_char("Syntax:\n\r",ch);
      send_to_char("confiscate <item> <char>\n\r",ch);
      return;
    }
    
    if ((victim = get_char_world(ch,argument, FALSE)) == NULL)
    { 
      send_to_char("They aren't here.\n\r",ch);
      return; 
    }
    
    /*
     * Confiscate is the only way to get visdeath items from mobs
     *
    if (IS_NPC(victim))
    { 
      send_to_char("They aren't here.\n\r",ch);
      return; 
    }
     */
    
    if ( victim->level >= ch->level  && !IS_NPC(victim) )
    { 
      send_to_char("They are too high level for you to do that.\n\r",ch );
      return; }
    
    for ( obj = victim->carrying; obj != NULL; obj = obj->next_content )
    {
      if ( is_name( arg1, obj->name )
      &&   can_see_obj( ch, obj ))
      {
        found = TRUE;
        break;
      }
    }
    
    if (!found)
    { 
      send_to_char("They don't have that item.\n\r",ch);
      return; 
    }

    obj_from_char( obj );  
    obj_to_char( obj, ch );
    send_to_char("Got it.\n\r",ch);
    send_to_char("A piece of your eq has been confiscated.\n\r",victim);
    return;
}

void do_addapply(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    AFFECT_DATA *paf,*af_new;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    int affect_modify = 0, bit = 0, enchant_type;
    int level = 0, sn = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char("Syntax for applies: addapply <object> <apply type> <value>\n\r",ch);
        send_to_char("Apply Types: hp str dex int wis con mana\n\r", ch);
        send_to_char("             ac move hitroll damroll saves\n\r\n\r", ch);
        send_to_char("Syntax for affects: addapply <object> spell <spell name> <level>\n\r",ch);
        return;
    }

  
    if ( (obj = get_obj_carry(ch,arg1,ch, FALSE)) == NULL )
    {
        send_to_char("You don't have that object!\n\r",ch);
        return;
    }


    if      ( !str_prefix(arg2,"hp") )
        enchant_type = APPLY_HIT;
    else if ( !str_prefix(arg2,"str") )
        enchant_type = APPLY_STR;
    else if ( !str_prefix(arg2,"dex") )
        enchant_type = APPLY_DEX;
    else if ( !str_prefix(arg2,"int") )
        enchant_type = APPLY_INT;
    else if ( !str_prefix(arg2,"wis") )
        enchant_type = APPLY_WIS;
    else if ( !str_prefix(arg2,"con") )
        enchant_type = APPLY_CON;
    else if ( !str_prefix(arg2,"mana") )
        enchant_type = APPLY_MANA;
    else if ( !str_prefix(arg2,"move") )
        enchant_type = APPLY_MOVE;
    else if ( !str_prefix(arg2,"blood") )
        enchant_type = APPLY_BLOOD;
    else if ( !str_prefix(arg2,"ac") )
        enchant_type = APPLY_AC;
    else if ( !str_prefix(arg2,"hitroll") )
        enchant_type = APPLY_HITROLL;
    else if ( !str_prefix(arg2,"damroll") )
        enchant_type = APPLY_DAMROLL;
    else if ( !str_prefix(arg2,"saves") )
        enchant_type = APPLY_SAVING_SPELL;
    else if ( !str_prefix(arg2,"spell") )
        enchant_type = APPLY_SPELL_AFFECT;
    else
    {
        send_to_char("That apply is not possible!\n\r",ch);
        return;
    }


    if ( enchant_type == APPLY_SPELL_AFFECT )
    {
        sn = skill_lookup(arg3);
        if (sn == NO_SKILL) 
        {
            send_to_char("That spell does not exist.\n\r", ch);
            return;
        }
        if (skill_table[sn].spell_fun == NULL
  	||  skill_table[sn].target == TAR_IGNORE 
  	||  skill_table[sn].target == TAR_OBJ_INV) 
        {
            send_to_char("You can't add that spell to the object.\n\r", ch);
            return;
        }
        if ( is_number(arg4) )
            level = atoi(arg4);
        else 
            level = ch->level;
    }
    else
    {
        if ( is_number(arg3) )
            affect_modify=atoi(arg3);
        else
        {
            send_to_char("Applies require a value.\n\r", ch);
            return;
        }
    }

    if (!obj->enchanted)
    {
        obj->enchanted = TRUE;
        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
        {
            af_new = new_affect();
            af_new->type        = UMAX(0,paf->type);
            af_new->level       = paf->level;
            af_new->duration    = paf->duration;
            af_new->location    = paf->location;
            af_new->modifier    = paf->modifier;
            af_new->bitvector   = paf->bitvector;
            af_new->next        = obj->affected;
            obj->affected       = af_new;
        }
    }
  
    paf             = new_affect();
    paf->type       = 0;
    paf->level      = ch->level;
    paf->duration   = -1;
    paf->location   = enchant_type;
    paf->modifier   = affect_modify;
    paf->bitvector  = bit;  
    if ( enchant_type == APPLY_SPELL_AFFECT )
    {
        paf->level     = level;
        paf->bitvector = 0;
        paf->type      = sn;
        paf->where     = TO_SPELLS;
        paf->modifier  = 0;
    }
    paf->next       = obj->affected;
    obj->affected   = paf;
  
    send_to_char("Ok.\n\r", ch);
    return;
}

void check_multiplay(CHAR_DATA *ch) {

  DESCRIPTOR_DATA *d;
  char buf[MAX_STRING_LENGTH];

  if (get_trust(ch) > 50)
    return;

 for (d = descriptor_list; d != NULL; d = d->next) {
    if (ch->desc == d || (d->character == NULL))
      continue;

    if (!str_cmp(d->host,ch->desc->host)) {
      if(d->character->level > 50) {
        return;
    }
  }
 }
  /* CHECK THE HOST VS THE HOSTS ON THE MULTI LIST - MINA 1/4/2002 */
  if (check_multilogin(ch->desc->host,MULTILOGIN_PREFIX|MULTILOGIN_SUFFIX)) 
      return;

  for (d = descriptor_list; d != NULL; d = d->next) {
    if (ch->desc == d || (d->character == NULL))
      continue;

    if (!str_cmp(d->host,ch->desc->host) &&
	d->character->level <= 50) {
      sprintf( buf, "$N has logged on with the same IP address as %s.  IP address: %s.\n\r", 
	       d->character->name,
	       d->host );
      wiznet(buf, ch, NULL, WIZ_MULTI, 0, 0);
    }
  }
}

void do_mask( CHAR_DATA* ch, char* argument )
{
  char name[ MAX_INPUT_LENGTH ];
  char fake_ip[ MAX_INPUT_LENGTH ];
  char buf[ MAX_INPUT_LENGTH ];
  CHAR_DATA* victim;

  if( argument[ 0 ] == '\0' )
    {
      send_to_char( "Mask who?\n\r", ch );
      return;
    }

  argument = one_argument( argument, name );
  argument = one_argument( argument, fake_ip );

  if( ( victim = get_char_world( ch, name, FALSE ) ) == NULL )
    {
      send_to_char( "That character isn't connected\n\r", ch );
      return;
    }

  if( IS_NPC( victim ) )
    {
      send_to_char( "Not on NPCs\n\r", ch );
      return;
    }


  if( fake_ip[ 0 ] != '\0' )
  {
      smash_tilde( fake_ip );
      sprintf( buf, fake_ip );
      free_string( victim->pcdata->fake_ip );
      victim->pcdata->fake_ip = str_dup( buf );
      send_to_char( "Character masked!\n\r", ch );
  }
  else
  {
      free_string( victim->pcdata->fake_ip );
      victim->pcdata->fake_ip = &str_empty[0];
      send_to_char( "Character unmasked!\n\r", ch );
  }


}

/*
 * do_pload, loads player files.
 *  - Davian
 */
void do_pload( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA d;
  bool isChar = FALSE;
  char name[MAX_INPUT_LENGTH];

  if (argument[0] == '\0')
  {
    send_to_char("Load who?\n\r", ch);
    return;
  }

  argument[0] = UPPER(argument[0]);
  argument = one_argument(argument, name);

  /* Dont want to load a second copy of a player who's already online! */
  if ( get_char_world( ch, name, FALSE ) != NULL )
  {
    send_to_char( "That person is already connected!\n\r", ch );
    return;
  }

  isChar = load_char_obj(&d, name); /* char pfile exists? */

  if (!isChar)
  {
    send_to_char("Load Who? Are you sure? I cant seem to find them.\n\r",ch);
    return;
  }

  d.character->desc     = NULL;
  d.character->next     = char_list;
  char_list             = d.character;
  d.connected           = CON_PLAYING;
  reset_char(d.character);

  /* bring player to imm */
  if ( d.character->in_room != NULL )
  {
    char_to_room( d.character, ch->in_room); /* put in room imm is in */
  }

  act( "$n has pulled $N from the shell to the game!",
        ch, NULL, d.character, TO_ROOM );

  if (d.character->pet != NULL)
   {
     char_to_room(d.character->pet,d.character->in_room);
     act("$n has entered the game.",d.character->pet,NULL,NULL,TO_ROOM);
   }

}

void do_openroom( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    int count = 0, high = 0, low = 0, found = 0;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],
buf[MAX_STRING_LENGTH];
    argument = one_argument(argument,arg);
    argument = one_argument(argument,arg2);
    if(!is_number(arg) || !is_number(arg2)) {
	send_to_char("Syntax: openroom <lowvnum> <highvnum>\n\r",ch);
	return;
	}
    high = atoi(arg2); low = atoi(arg);
    if((!high) || (!low)) {
	send_to_char("Syntax: openroom <lowvnum> <highvnum>\n\r",ch);
	return;
	}
    if(high < low) {
	send_to_char("Syntax: openroom <lowvnum> <highvnum>\n\r",ch);
	return;
	}
    for (count = low; count != high;count++)
	{
        sprintf(buf,"%d",count);
           if ( ( location = find_location( ch, buf ) ) == NULL )
              {
		found++;
                sprintf(buf,"%d ",count);
                send_to_char(buf, ch );
		if(!(found % 5)) send_to_char("\n\r",ch);
              }
         }
}

void do_openobj( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObjIndex;
    int count = 0, high = 0, low = 0, found = 0;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],
buf[MAX_STRING_LENGTH];
    argument = one_argument(argument,arg);
    argument = one_argument(argument,arg2);
    if(!is_number(arg) || !is_number(arg2)) {
	send_to_char("Syntax: openobj <lowvnum> <highvnum>\n\r",ch);
	return;
	}
    high = atoi(arg2); low = atoi(arg);
    if((!high) || (!low)) {
	send_to_char("Syntax: openobj <lowvnum> <highvnum>\n\r",ch);
	return;
	}
    if(high < low) {
	send_to_char("Syntax: openobj <lowvnum> <highvnum>\n\r",ch);
	return;
	}
    for (count = low; count != high;count++)
	{
        sprintf(buf,"%d",count);
    if ( ( pObjIndex = get_obj_index( count ) ) == NULL )
                {
		found++;
                sprintf(buf,"%d ",count);
                send_to_char(buf, ch );
		if(!(found % 5)) send_to_char("\n\r",ch);
              }
         }
}

void do_openmob( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMobIndex;
    int count = 0, high = 0, low = 0, found = 0;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],
buf[MAX_STRING_LENGTH];
    argument = one_argument(argument,arg);
    argument = one_argument(argument,arg2);
    if(!is_number(arg) || !is_number(arg2)) {
	send_to_char("Syntax: openmob <lowvnum> <highvnum>\n\r",ch);
	return;
	}
    high = atoi(arg2); low = atoi(arg);
    if((!high) || (!low)) {
	send_to_char("Syntax: openmob <lowvnum> <highvnum>\n\r",ch);
	return;
	}
    if(high < low) {
	send_to_char("Syntax: openmob <lowvnum> <highvnum>\n\r",ch);
	return;
	}
    for (count = low; count != high;count++)
	{
        sprintf(buf,"%d",count);
    if ( ( pMobIndex = get_mob_index( count  ) ) == NULL )
                {
		found++;
                sprintf(buf,"%d ",count);
                send_to_char(buf, ch );
		if(!(found % 5)) send_to_char("\n\r",ch);
              }
         }
}

/* Removed 5/27/09 gkl
void do_openspells (CHAR_DATA *ch, char *argument)
{
	int sn, counting;
	bool open;
	char buf[MAX_STRING_LENGTH];
	send_to_char("Open slots:\n\r",ch);
	for (counting = 0; counting < MAX_SKILL; counting ++)
	{
	   open = TRUE;
	   for ( sn = 0; sn < MAX_SKILL; sn++ )
	    {
	        if ( counting == skill_table[sn].slot )
        	    open = FALSE;
   	    }
	if (open) 
		{
		sprintf(buf," %d ",counting);
		send_to_char(buf,ch);
		}
	}
} */
/* Astrip has a use. Really. */
void do_astrip( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *victim;
   if ( ( victim = get_char_world( ch, argument, FALSE ) ) == NULL )
    {
	send_to_char("I can't find them anywhere.\n\r",ch);
	return;
    }
    while ( victim->affected )
        affect_remove( victim, victim->affected );
    victim->affected_by = race_table[victim->race].aff;
    victim->affected_by2 = race_table[victim->race].aff2;
    victim->affected_by3 = race_table[victim->race].aff3;

}

void do_objtoggle( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int changes;
    char arg[MAX_STRING_LENGTH];
    argument = one_argument(argument,arg);
    if (argument[0] == '\0')
    {
	send_to_char("Syntax: objtoggle <obj> <wear|extra> <flags...>\n\r",ch);
    }
    if ( ( obj = get_obj_world( ch, arg, FALSE ) ) == NULL )
    {
        send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
        return;
    }
    argument = one_argument(argument,arg); /* Cycle argument */
    if(strcmp(arg,"extra") && strcmp(arg,"wear"))
    {
	send_to_char("You can only toggle extra and wear flags.\n\r",ch);
	return;
    }
    if(!strcmp(arg,"extra"))
    {
        if ( ( changes = flag_value( extra_flags, argument ) ) != NO_FLAG )
        {
            TOGGLE_BIT(obj->extra_flags, changes);

            send_to_char( "Flags toggled.\n\r", ch);
            return;
	}
	send_to_char("Invalid flags\n\r",ch);
	return;
    } /* end extra */
    if(!strcmp(arg,"wear"))
    {
        if ( ( changes = flag_value( wear_flags, argument ) ) != NO_FLAG )
        {
            TOGGLE_BIT(obj->wear_flags, changes);

            send_to_char( "Flags toggled.\n\r", ch);
            return;
	}
	send_to_char("Invalid flags\n\r",ch);
	return;
    } /* end wear */
    send_to_char("Something went horribly wrong.\n\r",ch);
}


/* BACKUP/PURGE CODE
   This should not be in the game's code.  It should be a
   standalone program or shell script so that it does not
   halt the entire game when issued.
void do_backup( CHAR_DATA* ch, char* argument )
{
  char arg1[ MAX_INPUT_LENGTH ];
  bool run_tar = FALSE;
  bool run_purge = FALSE;

  argument = one_argument( argument, arg1 );

  if( arg1[ 0 ] == '\0' )
    {
      run_tar = TRUE;
      run_purge = TRUE;
    }

  if( !str_cmp( arg1, "purge" ) )
    run_purge = TRUE;

  if( !str_cmp( arg1, "tar" ) )
    run_tar = TRUE;

  if( !run_tar && !run_purge )
    send_to_char( "Optional arguments: purge, tar\n\r", ch );

  if( run_tar )
    make_tars( ch );

  if( run_purge )
    purge_pfiles( ch );

  return;
}
*/
/*
int make_tars( CHAR_DATA* ch )
{
  const char HOME_DIR[] = "/home/a-e/drmud/";
  const char TAR_FILE[] = "/bin/tar";
  const char TAR_FLAGS[] = "--exclude '*.log' --exclude '*.o' -czf";
  const char GAME_PATH[] = "game-port";
  const char TEST_PATH[] = "test-port";
  const char BUILD_PATH[] = "build-port";
  const char BACKUP_PATH[] = "backups";

  const char GAME_FILES[][30] = { "gamebak1.tar.gz",
				  "gamebak2.tar.gz",
				  "gamebak3.tar.gz" };

  const char BUILD_FILES[][30] = { "buildbak1.tar.gz",
				   "buildbak2.tar.gz",
				   "buildbak3.tar.gz" };

  const char TEST_FILES[][30] = { "testbak1.tar.gz",
				  "testbak2.tar.gz",
				  "testbak3.tar.gz" };
  char* old_cwd;
  char msg_to_char[ MAX_STRING_LENGTH ];
  char cmd_line[ MAX_STRING_LENGTH ];
  FILE* current_file;

  old_cwd = getcwd( NULL, 0 );

  chdir( HOME_DIR );
  chdir( BACKUP_PATH );

  if( ( current_file = fopen( GAME_FILES[2], "r" ) ) != NULL )
    {
      sprintf( msg_to_char, "Deleting %s\n\r", GAME_FILES[2] );
      send_to_char( msg_to_char, ch );
      fclose( current_file );
      unlink( GAME_FILES[2] );
    }

  if( ( current_file = fopen( GAME_FILES[1], "r" ) ) != NULL )
    {
      sprintf( msg_to_char, "%s --> %s\n\r", GAME_FILES[1], GAME_FILES[2] );
      send_to_char( msg_to_char, ch );
      fclose( current_file );
      rename( GAME_FILES[1], GAME_FILES[2] );
    }

  if( ( current_file = fopen( GAME_FILES[0], "r" ) ) != NULL )
    {
      sprintf( msg_to_char, "%s --> %s\n\r", GAME_FILES[0], GAME_FILES[1] );
      send_to_char( msg_to_char, ch );
      fclose( current_file );
      rename( GAME_FILES[0], GAME_FILES[1] );
    }

  if( ( current_file = fopen( BUILD_FILES[2], "r" ) ) != NULL )
    {
      sprintf( msg_to_char, "Deleting %s\n\r", BUILD_FILES[2] );
      send_to_char( msg_to_char, ch );
      fclose( current_file );
      unlink( BUILD_FILES[2] );
    }

  if( ( current_file = fopen( BUILD_FILES[1], "r" ) ) != NULL )
    {
      sprintf( msg_to_char, "%s --> %s\n\r", BUILD_FILES[1], BUILD_FILES[2] );
      send_to_char( msg_to_char, ch );
      fclose( current_file );
      rename( BUILD_FILES[1], BUILD_FILES[2] );
    }

  if( ( current_file = fopen( BUILD_FILES[0], "r" ) ) != NULL )
    {
      sprintf( msg_to_char, "%s --> %s\n\r", BUILD_FILES[0], BUILD_FILES[1] );
      send_to_char( msg_to_char, ch );
      fclose( current_file );
      rename( BUILD_FILES[0], BUILD_FILES[1] );
    }

  if( ( current_file = fopen( TEST_FILES[2], "r" ) ) != NULL )
    {
      sprintf( msg_to_char, "Deleting %s\n\r", TEST_FILES[2] );
      send_to_char( msg_to_char, ch );
      fclose( current_file );
      unlink( TEST_FILES[2] );
    }

  if( ( current_file = fopen( TEST_FILES[1], "r" ) ) != NULL )
    {
      sprintf( msg_to_char, "%s --> %s\n\r", TEST_FILES[1], TEST_FILES[2] );
      send_to_char( msg_to_char, ch );
      fclose( current_file );
      rename( TEST_FILES[1], TEST_FILES[2] );
    }

  if( ( current_file = fopen( TEST_FILES[0], "r" ) ) != NULL )
    {
      sprintf( msg_to_char, "%s --> %s\n\r", TEST_FILES[0], TEST_FILES[1] );
      send_to_char( msg_to_char, ch );
      fclose( current_file );
      rename( TEST_FILES[0], TEST_FILES[1] );
    }

  chdir( HOME_DIR );

  sprintf( cmd_line, "%s %s %s/%s %s",
	   TAR_FILE,
	   TAR_FLAGS,
	   BACKUP_PATH,
	   GAME_FILES[ 0 ],
	   GAME_PATH );
  system( cmd_line );

  sprintf( cmd_line, "%s %s %s/%s %s",
	   TAR_FILE,
	   TAR_FLAGS,
	   BACKUP_PATH,
	   BUILD_FILES[ 0 ],
	   BUILD_PATH );
  system( cmd_line );

  sprintf( cmd_line, "%s %s %s/%s %s",
	   TAR_FILE,
	   TAR_FLAGS,
	   BACKUP_PATH,
	   TEST_FILES[ 0 ],
	   TEST_PATH );
  system( cmd_line );

  chdir( old_cwd );

  return 0;

}
*/

/*
void purge_pfiles( CHAR_DATA* ch )
{
  DIR* player_dir;
  FILE* current_pfile;
  const time_t DAY = 86400;
  time_t grace_period;
  time_t laston;
  time_t now;
  time_t difference;
  char filename[ MAX_STRING_LENGTH ];
  char charfile[ MAX_STRING_LENGTH ];
  char delmsg[ MAX_STRING_LENGTH ];
  struct dirent* dir_entry;
  int level;

  if( ( player_dir = opendir( PLAYER_DIR ) ) != NULL )
    {
      while( ( dir_entry = readdir( player_dir ) ) )
	{
	  if( strcmp( "." , dir_entry->d_name ) != 0 &&
	      strcmp( "..", dir_entry->d_name ) != 0 )
	    {
	      if( !get_char_world( ch, dir_entry->d_name, FALSE ) )
		{
		  sprintf( filename, "%s%s", PLAYER_DIR, dir_entry->d_name );

		  if( ( current_pfile = fopen( filename, "r" ) ) != NULL )
		    {
		      fgets( charfile, 120, current_pfile );
		      fgets( charfile, 120, current_pfile );
		      fgets( charfile, 120, current_pfile );
		      fgets( charfile, 120, current_pfile );

		      sscanf( charfile, "%*s %ld", &laston );

		      while( !strstr( charfile, "Levl" ) )
			{
			  fgets( charfile, 120, current_pfile );
			}

		      sscanf( charfile, "%*s %d", &level );

		      fclose( current_pfile );

		      if( level < 50 )
			{

			  if( level < 12 )
			    {
			      grace_period = 20 * DAY;
			    }
			  else if( level < 30 )
			    {
			      grace_period = 30 * DAY;
			    }
			  else 
			    {
			      grace_period = 45 * DAY;
			    }

			  now = time( NULL );
			  difference = now - laston;

			  if( difference - grace_period > 0 )
			    {
			      sprintf( delmsg, "Deleting %s (level %d) (%d days)\n\r",
				       dir_entry->d_name,
				       level,
				       (int) ( difference / DAY ) );

			      send_to_char( delmsg, ch );
			      unlink( filename );
			    }
			}
		    }
		}
	    }
	}
    }
}
*/
 

void do_guildlist( CHAR_DATA* ch, char* argument )
{
  int guild = 0;
  int count = 0;
  char cwd[ MAX_STRING_LENGTH ] = "";
  char temp[ MAX_STRING_LENGTH ] = "";
  FILE* output = NULL;
  char current_name[ MAX_INPUT_LENGTH ] = "";
  BUFFER* results = NULL;

  if( ch->level < 59 || ( argument[ 0 ] == '\0' ) )
    {
      guild = ch->guild;
    }
  else
    {
      guild = guild_lookup( argument );
    }
  
  if( guild == 0 )
    {
      send_to_char( "No such guild exists.\n\r", ch );
      return;
    }

  /* doubled up to handle clan/guild transition */
  sprintf( temp, "grep -l 'Clan %s' *", guild_table[ guild ].name );

  getcwd( cwd, MAX_STRING_LENGTH );
  chdir( PLAYER_DIR );

  output = popen( temp, "r" );
  if( output == NULL )
    {
      bug( "guildlist, failure to get FILE* handle to output", 0 );
      return;
    }

  results = new_buf();
  if( results == NULL )
    {
      bug( "guildlist, failure to allocate new BUFFER", 0 );
      pclose( output );
      return;
    }

  add_buf( results, "Results:\n\r-----------------------\n\r" );

  while( !feof( output ) )
    {
      fscanf( output, "%s\n", current_name );
      add_buf( results, current_name );
      add_buf( results, "\n\r" );
      count++;
    }

  pclose( output );

  sprintf( temp, "grep -l 'Guild %s' *", guild_table[ guild ].name );

  output = popen( temp, "r" );
  if( output == NULL )
    {
      bug( "guildlist, failure to get FILE* handle to output", 1 );
      free_buf( results );
      return;
    }

  while( !feof( output ) )
    {
      fscanf( output, "%s\n", current_name );
      add_buf( results, current_name );
      add_buf( results, "\n\r" );
      count++;
    }

  pclose( output );

  sprintf( temp, "\n\rTotal: %d\n\r", count );
  add_buf( results, temp );

  page_to_char( buf_string( results ), ch );
  free_buf( results );

  chdir( cwd );
  
}

void do_wizstat ( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    int vist, wizt;

    if ( IS_NULLSTR( argument )
    ||  !IS_ADMIN(ch) )
        victim = ch;
    else if ( ( victim = get_char_world( ch, argument, FALSE ) ) == NULL )
    {
        send_to_char( "That character is not logged in.\n\r", ch );
        return;
    }
    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPCs.\n\r", ch );
        return;
    }

    if (IS_DRMODE(ch))
    {
        send_to_char("{B==================================="
              "========================================{x\n\r",ch);
        sprintf(buf, "%-12s                        "
              "                               Level %2d\n\r",
            victim->name, victim->level );
        send_to_char( buf, ch );
        send_to_char("{B==================================="
              "========================================{x\n\r",ch);
    }
    else
    {
        sprintf( buf, "%s (Level %2d)\n\r\n\r", victim->name, victim->level );
        send_to_char( buf, ch );
    }
    sprintf( buf,
        "Played: %10d   Last Level: %5d   Timer:    %d\n\r",
        (int) (victim->played + current_time - victim->logon) / 3600,
        victim->pcdata->last_level,
        victim->timer );
    send_to_char( buf, ch );

    if ( IS_IMMORTAL( victim ) )
    {
        wizt = victim->pcdata->wizitime / 3600;
        vist = victim->pcdata->vistime / 3600;
        sprintf( buf, "Visible: %5d/%2d%c   Wizi:   %5d/%2d%c   Total:    %d\n\r",
            vist, 100 * vist / UMAX(1,(vist + wizt)), '%',
            wizt, 100 * wizt / UMAX(1,(vist + wizt)), '%',
            vist + wizt );
        send_to_char(buf, ch );

        sprintf( buf, "Approvals:    %4d   Unapprovals: %4d   Immtalks: %d\n\r",
            victim->pcdata->approvals,
            victim->pcdata->unapprovals,
            victim->pcdata->immtalks );
        send_to_char( buf, ch );
    }

    sprintf( buf, "ICs:    %10d   OOCs:  %10d   Quests:   %d\n\r",
        victim->pcdata->ics,
        victim->pcdata->oocs,
        victim->pcdata->questcount );
    send_to_char( buf, ch );

    if ( is_guild( victim ) && get_trust( victim ) <= get_trust( ch ) )
    {
        int guilded_time;
        guilded_time = (int)(victim->pcdata->guild_time + current_time - victim->logon );

        if ( ch == victim ||  IS_ADMIN( ch ) )
        {
            sprintf( buf, "Guild Notes: %5d   GDTs:  %10d\n\r",
                victim->pcdata->guildnotes,
                victim->pcdata->gdts );
            send_to_char( buf, ch );
        }

        sprintf( buf,
                 "Guild Hours: %5d   Guild Logs: %5d   Average:  %d mins\n\r",
                 guilded_time / 3600,
                 victim->pcdata->guild_logs,
                 victim->pcdata->guild_logs > 0 ? ( guilded_time / 60 / victim->pcdata->guild_logs ) : 0
                 );
        send_to_char( buf, ch );

    }
    if ( ( get_trust( ch ) == MAX_LEVEL || ch == victim )
    &&     !IS_NULLSTR( victim->pcdata->notetags ) )
    {
        sprintf(buf, "Note Tags:   %s\n\r", victim->pcdata->notetags );
        send_to_char( buf, ch );
    }


    if (IS_DRMODE(ch))
        send_to_char("{B==================================="
              "========================================{x\n\r",ch);

    return;
}

void do_findkey( CHAR_DATA *ch, char *argument )
{
    extern char * const dir_name[];
    EXIT_DATA *pExit;
    OBJ_INDEX_DATA *pObj, *pKey;
    ROOM_INDEX_DATA *pRoom;
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];
    int vkey, v, door;
    bool found = FALSE;

    if ( !is_number(argument) )
    {
        send_to_char( "Syntax: findkey <vnum>\n\r", ch );
        return;
    }

    vkey = atoi( argument );
    pKey = get_obj_index( vkey );
    if ( pKey == NULL )
    {
        send_to_char( "No object has that vnum.\n\r", ch );
        return;
    }
    pArea = pKey->area;
    sprintf(buf, "Key %d (%s) unlocks:\n\r",
        vkey, pKey->short_descr );
    send_to_char( buf, ch );
    for ( v = pArea->min_vnum; v <= pArea->max_vnum; v++ )
    {
/*      Check for object with matching vnum */
        if ( ( pObj = get_obj_index( v ) ) != NULL )
        {
            if ( ( pObj->item_type == ITEM_CONTAINER
            ||     pObj->item_type == ITEM_QUIVER 
            ||     pObj->item_type == ITEM_BOOK )
            &&   pObj->value[2] == vkey )
            {
                found = TRUE;
                sprintf(buf, "Container: %5d (%s)\n\r",
                    v, pObj->short_descr );
                send_to_char( buf, ch );
            }
            else if ( pObj->item_type == ITEM_PORTAL 
            &&   pObj->value[4] == vkey )
            {
                found = TRUE;
                sprintf(buf, "Portal:    %5d (%s)\n\r",
                    v, pObj->short_descr );
                send_to_char( buf, ch );
            }
        }
        if ( ( pRoom = get_room_index( v ) ) != NULL )
        {
            for ( door = 0; door <= 5; door++ )
            {
                if ( ( pExit = pRoom->exit[door] ) != NULL
                &&   pExit->u1.to_room != NULL )
                {
                    if ( pExit->key > 0 && pExit->key == vkey )
                    {
                        found = TRUE;
                        sprintf(buf, "Room:      %5d (%s)\n\r",
                            v, dir_name[door] );
                        send_to_char( buf, ch );
                    }
                }
            }
        }

/*      Check for room with matching vnum */
/*      Check ALL exits from that room if found */
    }
    if ( !found )
        send_to_char( "Nothing.\n\r", ch );

    return;
}

void do_subdue( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( IS_NULLSTR( argument ) )
    {
	send_to_char( "Whose beast would you like to subdue?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, argument, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPCs.\n\r", ch );
        return;
    }


    if ( IS_ADMIN( ch ) || ch->guild == guild_lookup("covenance") )
    {
        if ( !IS_SET( victim->shiftbits, SHIFT_NOFORCE ) )
        {
            act("You subdue the beast residing within $M.", 
                 ch, NULL, victim, TO_CHAR );
            act("You feel more at peace as the beast within you settles.",
                 victim, NULL, NULL, TO_CHAR);
            SET_BIT( victim->shiftbits, SHIFT_NOFORCE );
        }
        else
        {
            act("You release the chains binding the beast residing within $M.", 
                 ch, NULL, victim, TO_CHAR );
            act("The beast within you begins to stir wildly once again!",
                 victim, NULL, NULL, TO_CHAR);
            REMOVE_BIT( victim->shiftbits, SHIFT_NOFORCE );
        }
    }
    else
    {
        send_to_char( "This command is not for your use.\n\r", ch );
        return;
    }

    return;
}

void do_nuke ( CHAR_DATA *ch, char *argument )
{
    int vnum;
    CHAR_DATA *victim, *vnext;
    CHAR_DATA *fpc;
    OBJ_DATA *obj, *obj_next;
    DESCRIPTOR_DATA *d;
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;

    if ( (pArea = ch->in_room->area) == NULL )
        return;

    if ( IS_NPC(ch) )
    {
        send_to_char("NPCs should not call this command.", ch );
        return;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
        send_to_char( "You don't have the authority to nuke this area.\n\r", ch );
        return;
    }

    for (d = descriptor_list; d; d = d->next)
	if (d->connected == CON_PLAYING
	&&  d->character->in_room != NULL
	&&  d->character->in_room->area == pArea)
	    send_to_char("An unnatural, fiery-red light fills the room!\n\r",d->character);

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
        if ( ( pRoom = get_room_index(vnum) ) )
        {
            fpc = NULL;
            for ( victim = pRoom->people; victim != NULL; victim = vnext )
            {
                vnext = victim->next_in_room;
                if ( IS_NPC(victim) 
                &&  !IS_SET(victim->act_bits,ACT_NOPURGE)
                &&   victim != ch )
                {
                    act("$n instantly disintegrates!", victim, NULL, NULL, TO_ROOM );
                    extract_char( victim, TRUE );
                }
                else if ( !IS_NPC( victim ) )
                  fpc = victim;
            }

            for ( obj = pRoom->contents; obj != NULL; obj = obj_next )
            {
                obj_next = obj->next_content;
                if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
                {
                    if ( fpc != NULL )
                    {
                        act( "$p instantly disintegrates!", fpc, obj, NULL, TO_ROOM );
                        act( "$p instantly disintegrates!", fpc, obj, NULL, TO_CHAR );
                    }
                    extract_obj( obj );
                }
            }
        }
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



/* Dynamic configuration functionality */
struct config_toggle_type
{
    char *fieldname;
    int *setting;
    char *descr;
};
struct config_toggle_type config_toggle_table[] =
{
    { "aprilfools",  &global_config_aprilfools,  "April fools mode"           },
    { "lookspam",    &global_config_lookspam,    "Room desc look spam"        },
    { "vorpalon",    &global_config_vorpalon,    "New vorpal capabilities"    },
    { "tickjitter",  &global_config_tick_jitter, "Jitter in tick length"      },
    { "newpoison",   &global_config_newpoison,   "New poison/pinch behavior"  },
    { "strongwyrmpoison",  &global_config_strongwyrmpoison,   "Beefy version of wyrm poison"  },
    { NULL,             NULL,                           NULL                  }
};

void do_config ( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    bool boolopt;
    int i, icfg = -1;

    argument = one_argument(argument, arg);
    for ( i = 0; config_toggle_table[i].fieldname != NULL; i++ )
      if ( !str_cmp(arg,config_toggle_table[i].fieldname) )
        icfg = i;

    argument = one_argument(argument, arg);
    if ( icfg == -1 )
    {
      send_to_char( "The following configurations are valid:\n\r", ch );
      for ( i = 0; config_toggle_table[i].fieldname != NULL; i++ )
      {
/*      sprintf(buf, "  %-20s: [%3s]\n\r", config_toggle_table[i].fieldname,
 *        (*config_toggle_table[i].setting == 0 ? "OFF" : "ON") );
 */
        sprintf(buf, "  [%3s] %-20s (%s)\n\r", 
          (*config_toggle_table[i].setting == 0 ? "OFF" : "ON"),
          config_toggle_table[i].fieldname,
          config_toggle_table[i].descr );

        send_to_char( buf, ch );
      }
    }
    else
    {
      if ( !str_cmp( arg, "on" ) )
        boolopt = TRUE;
      else if ( !str_cmp( arg, "off" ) )
        boolopt = FALSE;
      else if ( !str_cmp(config_toggle_table[icfg].fieldname, "tickjitter" ) )
      {
          if ( IS_NULLSTR(arg) )
          {
              sprintf( buf, 
                  "Ticks are currently between %d and %d seconds long.\n\r",
                  45 - *config_toggle_table[icfg].setting, 
                  45 + *config_toggle_table[icfg].setting );
              send_to_char( buf, ch );
          }
          else
          {
              int dur;
              dur = atoi(arg);
              *config_toggle_table[icfg].setting = dur;
              sprintf( buf, 
                  "Ticks will now be between %d and %d seconds long.\n\r",
                  45 - *config_toggle_table[icfg].setting, 
                  45 + *config_toggle_table[icfg].setting );
              send_to_char( buf, ch );
          }
          return;
      }
      else
      {
        sprintf(buf, "%s is currently %s.\n\r", 
          config_toggle_table[icfg].descr,
          (*config_toggle_table[icfg].setting == 0 ? "OFF" : "ON") );
        send_to_char( buf, ch );
        return;
      }
      *config_toggle_table[icfg].setting = boolopt;
      sprintf(buf, "%s has been set to %s.\n\r", 
          config_toggle_table[icfg].descr,
          (*config_toggle_table[icfg].setting == 0 ? "OFF" : "ON") );
      send_to_char( buf, ch );
    }

    return;
}


void do_quadruple(CHAR_DATA *ch, char *argument)
{
	extern int quad;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
    int dur;

    argument = one_argument(argument, arg);

	if (IS_IMMORTAL(ch)) {
		if (!IS_NULLSTR(arg)) {
			dur = atoi(arg);
			quad = dur;

			for (d = descriptor_list; d != NULL; d = d->next) {
				if (d->connected == CON_PLAYING) {
					send_to_char("{YThe blessings of the gods shower down upon you. You gain more experience in your adventures.{x\n\r", 
						d->character);
				}
			}
		}
		else {
			sprintf(buf, "There are %d ticks of quad remaining.\n\r", quad);
			send_to_char(buf, ch);
		}
	}
	else {
		send_to_char("Huh?\n\r", ch);
	}
    

    return;
}

/* =0, name is fine; =1, is invalid name; =2, name already exists */
int bad_newname( char *name )
{
    int bad = FALSE;
    char pfile_path[MAX_STRING_LENGTH];
    FILE *renamefp;

    fclose(fpReserve);
    sprintf(pfile_path, "%s%s", PLAYER_DIR, capitalize( name ) );

    if ( (renamefp = fopen(pfile_path, "r")) == NULL)
        bad = ( check_parse_name(name) ? 0 : 1 ) ;
    else
    {
        bad = 2;
        fclose(renamefp);
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return bad;
}

void do_rename(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA  *victim;
    char buf[MAX_STRING_LENGTH], arg1[MAX_STRING_LENGTH], 
         newname[MAX_INPUT_LENGTH], pfile[MAX_STRING_LENGTH];
    int errcode;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, newname);

    if ( IS_NPC(ch) )
    {
        send_to_char( "Only PCs can issue this command.\n\r", ch );
        return;
    }

    if ( IS_NULLSTR(arg1) || IS_NULLSTR(newname) )
    {
        send_to_char("Syntax: rename <char> <newname>\n\r", ch);
        return;
    }

    if ( (victim = get_char_world(ch, arg1, FALSE)) == NULL )
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char("Not on NPC's\n\r", ch);
        return;
    }


    if (get_trust(ch) < victim->level)
    {
        send_to_char("You failed.\n\r",ch);
        return;
    }

    if ( (errcode = bad_newname(newname)) == 0 )
    {
        strcpy(newname, capitalize(newname));

        sprintf(buf, "%s has been renamed %s.\n\r", victim->name, newname);
        send_to_char(buf,ch);
        sprintf(buf, "%s renamed %s to %s", ch->name, victim->name, newname);
        log_string( buf );

        sprintf( pfile, "%s%s", PLAYER_DIR, capitalize( victim->name ) );

        free_string(victim->name);
        victim->name = str_dup( newname );
        save_char_obj(victim);

        sprintf(buf, "Your name has been changed to %s.\n\r", victim->name );
        send_to_char( buf, victim );

        unlink( pfile );

        if ( victim->pcdata->brawler != NULL )
        {
            free_string( victim->pcdata->brawler->name );
            victim->pcdata->brawler->name = str_dup( newname );
        }
    }
    else
    {
        if ( errcode == 1 )
            send_to_char("Illegal name.\n\r", ch);
        else if ( errcode == 2 )
            send_to_char("That name is already taken.\n\r", ch );
    }
    return;
}


void do_lagger( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int dur;

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR( arg ) )
    {
	send_to_char( "Whose lag do you want to examine?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( (dur = atoi(argument)) < 1 )
    {
        sprintf( buf, "$E has a lag of %.2f second%s.", 
            (float)(victim->wait)/(PULSE_PER_SECOND),
            (victim->wait == PULSE_PER_SECOND) ? "" : "s" );
        act(buf, ch, NULL, victim, TO_CHAR);
    }
    else
    {
        victim->wait = UMAX(victim->wait, dur);
        sprintf( buf, "$E now has a lag of %.2f second%s.", 
            (float)(victim->wait)/PULSE_PER_SECOND,
            (victim->wait == PULSE_PER_SECOND) ? "" : "s" );
        act(buf, ch, NULL, victim, TO_CHAR);
    }

    return;
}

void do_damage(CHAR_DATA* ch, char* argument)
{
	char arg1[MAX_INPUT_LENGTH],  arg2[MAX_INPUT_LENGTH];
	int dam = 0;
	CHAR_DATA *victim;
		
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	
	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Syntax: damage target amount message\n\r",ch);
		send_to_char("IE: damage zalyriel 1000 FUCK YOU ZAL.\n\r",ch);
		return;
	}
		
	if ( (victim = get_char_world(ch, arg1, FALSE) ) == NULL )
    {
		send_to_char("Target not found.\n\r",ch);
		return;
    }
	
	if (!is_number(arg2))
	{
		send_to_char("Syntax: damage target amount message\n\r",ch);
		send_to_char("IE: damage zalyriel 1000 FUCK YOU ZAL.\n\r",ch);
		return;
	}
	
	dam = atoi(arg2);

		send_to_char(argument,victim);
		send_to_char("\n\r",victim);
		raw_damage( victim, victim, dam, TYPE_UNDEFINED, DAM_NONE, FALSE, TRUE );

}

void do_availability( CHAR_DATA* ch, char* argument ) 
{
  if( IS_IMMORTAL( ch ) ) 
    {	
      if( ch->availability == 1 ) 
	{
	  ch->availability = 0;
	  send_to_char( "You are now set as unavailable.\n\r", ch );
	}
      else 
	{
	  ch->availability = 1;
	  send_to_char( "You are now set as available.\n\r", ch );
	}
    }  
  else 
    {
      send_to_char("Huh?\n\r",ch);
    }

}


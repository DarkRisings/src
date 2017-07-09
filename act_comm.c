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
 **************************************************************************/

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
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "interp.h"
#include "lookup.h"
#include "guilds.h"

/* Minimum length of a message for it to "count" 
   as a gdt, ic, ooc, etc */
#define MIN_CHARACTER_LOGGING 1

extern long queststatus;
extern QUEST_INFO global_quest_info;
extern struct pc_guild_type pc_guild_table[MAX_PC_GUILD];

const struct quest_cmd quest_cmd_tbl[] =
  {
    { "join",       "join",               &quest_join, FALSE },
    { "leave",      "leave",              &quest_leave, FALSE },
    { "start",      "start",              &quest_start, TRUE },
    { "open",       "open",               &quest_open, TRUE },
    { "close",      "close",              &quest_close, TRUE },
    { "end",        "end",                &quest_end, TRUE },
    { "status",     "status",             &quest_status, FALSE },
    { "score",      "score",              &quest_score, FALSE },
    { "name",       "name <quest name>",  &quest_name, TRUE },
    { "set",        "set <options...>",   &quest_set, TRUE },
    { "zero",       "zero <player>|all",  &quest_zero, TRUE },
    { "award",      "award <player> <points>", &quest_award, TRUE },
    { "assign",     "assign <player> <captain>", &quest_assign, TRUE },
    { "mob_vnum",   "mob_vnum <vnum>|0",  &quest_vnum, TRUE },
    { "levels",     "levels <min> <max>", &quest_levels, TRUE },
    { "teamscore",  "teamscore",          &quest_teamscore, FALSE },
    { "teamsay",    "teamsay <message>",  &quest_teamsay, FALSE },
    { "boot",       "boot <player>",      &quest_boot, TRUE },
    { NULL,         NULL,                 NULL, FALSE }
  };



//void Check_Replay (CHAR_DATA *ch);

/* command procedures needed */
DECLARE_DO_FUN(do_quit	);
/* RT code to delete yourself */

void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\n\r",ch);
}

void do_delete( CHAR_DATA *ch, char *argument)
{
  char strsave[MAX_INPUT_LENGTH];
  char log_buf[ MAX_STRING_LENGTH ] = "";

  if (IS_NPC(ch))
    return;
  
/*  if( ch->fightlag ) */
    if ( in_fightlag(ch) )
    {
      send_to_char("You are too excited to kill yourself!\n\r",ch);
      return;
    }

  
/*if( !( ch->level < 30 && IS_SET( ch->act_bits, PLR_NEWBIE ) ) )
    {
      if( !IS_SET( ch->act_bits, ACT_NOAPPROVE ) ) */
      if ( ch->level > 30 && !IS_SET( ch->act_bits, ACT_NOAPPROVE ) )
	{
	  send_to_char( "You cannot delete once you have been approved without the aid of an immortal.\n\r", ch );
	  return;
	}
//  }

  if (ch->pcdata->confirm_delete)
    {
      if (argument[0] != '\0')
	{
	  send_to_char("Delete status removed.\n\r",ch);
	  ch->pcdata->confirm_delete = FALSE;
	  return;
	}
      else
	{
	  sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	  wiznet("$N turns $Mself into line noise.",ch,NULL,0,0,0);
	  sprintf( log_buf, "%s deleted.", capitalize( ch->name ) );
	  log_string( log_buf );
	  stop_fighting(ch,TRUE);
          ch->pcdata->lastfight = 0;
/*        ch->fightlag = 0; */
	  do_quit(ch,"quit");
	  unlink(strsave);
	  return;
 	}
    }

  if (argument[0] != '\0')
    {
      send_to_char("Just type delete. No argument.\n\r",ch);
      return;
    }

  send_to_char("Type delete again to confirm this command.\n\r",ch);
  send_to_char("WARNING: this command is irreversible.\n\r",ch);
  send_to_char("Typing delete with an argument will undo delete status.\n\r",
	       ch);
  WAIT_STATE(ch,5 * PULSE_VIOLENCE);
  ch->pcdata->confirm_delete = TRUE;
  wiznet("$N is contemplating deletion.",ch,NULL,0,0,get_trust(ch));

  sprintf( log_buf, "Log %s: delete", ch->name );
  log_string( log_buf );
  
}
	    

/* RT code to display channel status */

void do_channels( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    /* lists all channels and their status */
    send_to_char("   channel     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);
 
    send_to_char("[NB]            ",ch);
    if (!IS_SET(ch->comm,COMM_NONEWBIE))
      send_to_char("ON\n\r", ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("[IC]            ",ch);
    if (!IS_SET(ch->comm,COMM_NOGOSSIP))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("auction         ",ch);
    if (!IS_SET(ch->comm,COMM_NOAUCTION))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("[OOC]           ",ch);
    if (!IS_SET(ch->comm,COMM_NOMUSIC))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char( "[Brawl]         ", ch );
    if( IS_SET( ch->comm, COMM_NOBRAWLER ) )
      send_to_char( "OFF\n\r", ch );
    else
      send_to_char( "ON\n\r", ch );


    if (IS_IMMORTAL(ch))
    {
      send_to_char("[IMMORTAL]      ",ch);
      if(!IS_SET(ch->comm,COMM_NOWIZ))
        send_to_char("ON\n\r",ch);
      else
        send_to_char("OFF\n\r",ch);
    }

    send_to_char("shouts          ",ch);
    if (!IS_SET(ch->comm,COMM_SHOUTSOFF))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("tells           ",ch);
    if (!IS_SET(ch->comm,COMM_DEAF))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("quiet mode      ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    if (IS_SET(ch->comm,COMM_SNOOP_PROOF))
	send_to_char("You are immune to snooping.\n\r",ch);
   
    if (ch->lines != PAGELEN)
    {
	if (ch->lines)
	{
	    sprintf(buf,"You display %d lines of scroll.\n\r",ch->lines+2);
	    send_to_char(buf,ch);
 	}
	else
	    send_to_char("Scroll buffering is off.\n\r",ch);
    }

    if (ch->prompt != NULL)
    {
	sprintf(buf,"Your current prompt is: %s\n\r",ch->prompt);
	send_to_char(buf,ch);
        sprintf(buf,"     Prompt with codes: %s\n\r", escape_color(ch->prompt));
	send_to_char(buf,ch);
    }

    if (IS_SET(ch->comm,COMM_NOSHOUT))
      send_to_char("You cannot shout.\n\r",ch);
  
    if (IS_SET(ch->comm,COMM_NOTELL))
      send_to_char("You cannot use tell.\n\r",ch);
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
     send_to_char("You cannot use channels.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOEMOTE))
      send_to_char("You cannot show emotions.\n\r",ch);

}

/* RT deaf blocks out all shouts */

void do_deaf( CHAR_DATA *ch, char *argument)
{
    
   if (IS_SET(ch->comm,COMM_DEAF))
   {
     send_to_char("You can now hear tells again.\n\r",ch);
     REMOVE_BIT(ch->comm,COMM_DEAF);
   }
   else 
   {
     send_to_char("From now on, you won't hear tells.\n\r",ch);
     SET_BIT(ch->comm,COMM_DEAF);
   }
}

/* RT quiet blocks out all communication */

void do_quiet ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_QUIET))
    {
      send_to_char("Quiet mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
    }
   else
   {
     send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
     SET_BIT(ch->comm,COMM_QUIET);
   }
}

void Check_Replay (CHAR_DATA *ch)
{
  if (buf_string(ch->pcdata->buffer)[0] != '\0')
    {
      send_to_char("  You have new tells, type 'replay' to view them.\n\r",ch);
      return;
    }

  send_to_char("\n\r", ch);
  return;
}


/* afk command */

void do_afk ( CHAR_DATA *ch, char * argument)
{

    if (IS_NPC(ch)) return;

    if (IS_SET(ch->comm,COMM_AFK))
    {
      send_to_char("AFK mode removed.",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }
   else
   {
     send_to_char("You are now in AFK mode.\n\r",ch);
     SET_BIT(ch->comm,COMM_AFK);
   }
}


void do_replay (CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
	send_to_char("You can't replay.\n\r",ch);
	return;
    }

    if (buf_string(ch->pcdata->buffer)[0] == '\0')
    {
	send_to_char("You have no tells to replay.\n\r",ch);
	return;
    }

    page_to_char(buf_string(ch->pcdata->buffer),ch);
    clear_buf(ch->pcdata->buffer);
}

/* RT chat replaced with ROM gossip */
void do_ic( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOGOSSIP))
      {
        send_to_char("[IC] channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOGOSSIP);
      }
      else
      {
        send_to_char("[IC] channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOGOSSIP);
      }
    }
    else   /*gossip message sent, turn gossip on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
        }

	if( IS_SET( ch->act_bits, ACT_NOAPPROVE ) )
	  {
	    send_to_char( "You must be approved to use this channel.  See 'help approval'\n\r", ch );
	    return;
	  }
        
        if (IS_SET(ch->shiftbits,SHIFT_HALF) || IS_SET(ch->shiftbits,SHIFT_FULL))
        {
          send_to_char("The world does not understand you in this beastial form\n\r",ch);
          return;
        }

      REMOVE_BIT(ch->comm,COMM_NOGOSSIP);
 
      sprintf( buf, "You [IC]: {%c'%s'{x\n\r", ch->colors[C_IC],argument );
      send_to_char( buf, ch );

      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOGOSSIP) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
          sprintf( buf, "[IC] %s: {%c'%s'{x\n\r", 
	    capitalize( PERS2(ch, d->character)),
            d->character->colors[C_IC],
            argument);
          send_to_char(buf,d->character);
        }
      }

    if ( !IS_NPC( ch ) && strlen( argument ) > MIN_CHARACTER_LOGGING )
      ch->pcdata->ics++;

    }
}

void do_quote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOQUOTE))
      {
        send_to_char("Quote channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOQUOTE);
      }
      else
      {
        send_to_char("Quote channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOQUOTE);
      }
    }
    else  /* quote message sent, turn quote on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
 
        }
 
      REMOVE_BIT(ch->comm,COMM_NOQUOTE);
 
      sprintf( buf, "You quote '%s'\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOQUOTE) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
          act_new( "$n quotes '$t'",
                   ch,argument, d->character, TO_VICT,POS_SLEEPING, 0 );
        }
      }
    }
}


void do_admin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOADMIN))
      {
        send_to_char("Admin channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOADMIN);
      }
      else
      {
        send_to_char("Admin channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOADMIN);
      }
    }
    else  /* answer sent, turn Q/A on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
	}
 
        REMOVE_BIT(ch->comm,COMM_NOADMIN);
 
      sprintf( buf, "{%c[ADMIN] {x%s: {%c'%s'{x\n\r", 
	ch->colors[C_ADMIN],ch->name,
	ch->colors[C_ADMIN],argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOADMIN) &&
/*           (victim->level >= 59) && */
              IS_ADMIN( victim ) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
          sprintf( buf, "{%c[ADMIN] {x%s: {%c'%s'{x\n\r", 
	    d->character->colors[C_ADMIN],capitalize( PERS2(ch, d->character)),
            d->character->colors[C_ADMIN],argument);
          send_to_char(buf,d->character);
        }
      }
    }
}

void do_aucchannel( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOAUCTION))
      {
        send_to_char("AUCTION channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOAUCTION);
      }
      else
      {
        send_to_char("AUCTION channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOAUCTION);
      }
    }
    else  
    {

      if( IS_SET( ch->act_bits, ACT_NOAPPROVE ) )
	{
	  send_to_char( "You must be approved to use this channel.  See 'help approval'.\n\r", ch );
	  return;
	}

        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 

        if (IS_SET(ch->shiftbits,SHIFT_HALF) || IS_SET(ch->shiftbits,SHIFT_FULL))
        {
            send_to_char("You cannot sell or bid and be understood in your current form.\n\r",ch);
            return;
        }

        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
	}
 
        REMOVE_BIT(ch->comm,COMM_NOAUCTION);
 
      sprintf( buf, "You auction: {%c'%s'{x\n\r", ch->colors[C_AUCTION],
 	argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOAUCTION) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
          sprintf( buf, "auction %s: {%c'%s'{x\n\r", 
	    capitalize( PERS2(ch, d->character)),
	    d->character->colors[C_AUCTION],
            argument);
          send_to_char(buf,d->character);
        }
      }
    }
}

/* RT music channel */
void do_music( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOMUSIC))
      {
        send_to_char("[OOC] channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOMUSIC);
      }
      else
      {
        send_to_char("[OOC] channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOMUSIC);
      }
    }
    else  /* music sent, turn music on if it isn't already */
    {
      if( IS_SET( ch->act_bits, ACT_NOAPPROVE ) )
	{
	  send_to_char( "You must be approved to use this channel.  See 'help approval'.\n\r", ch );
	  return;
	}
	  
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
	}
 
        REMOVE_BIT(ch->comm,COMM_NOMUSIC);
 
      sprintf( buf, "You [OOC]: {%c'%s'{x\n\r", ch->colors[C_OOC],argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOMUSIC) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
			if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
			{
					if (ch->availability == 1) {
						sprintf( buf, "[OOC] {G%s{x: {%c'%s'{x\n\r", 
						ch->name,
						d->character->colors[C_OOC],
						argument);
						send_to_char(buf,d->character);
					}
					else {
						sprintf( buf, "[OOC] {r%s{x: {%c'%s'{x\n\r", 
						ch->name,
						d->character->colors[C_OOC],
						argument);
						send_to_char(buf,d->character);
					}
			}
			
			else {
						sprintf( buf, "[OOC] %s: {%c'%s'{x\n\r", 
						capitalize( PERS2(ch, d->character)),
						d->character->colors[C_OOC],
						argument);
						send_to_char(buf,d->character);
			}
		  
        }
      }
      if ( !IS_NPC( ch ) && strlen( argument ) > MIN_CHARACTER_LOGGING )
          ch->pcdata->oocs++;

    }
}
void do_nb( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NONEWBIE))
      {
        send_to_char("[NB] channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NONEWBIE);
      }
      else
      {
        send_to_char("[NB] channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NONEWBIE);
      }
    }
    else  /* music sent, turn music on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
	}
 
        REMOVE_BIT(ch->comm,COMM_NONEWBIE);
 
      sprintf( buf, "You [NB]: {%c'%s'{x\n\r", ch->colors[C_NB], argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NONEWBIE) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
          sprintf( buf, "{%c[NB]{x %s: {%c'%s'{x\n\r", 
	    d->character->colors[C_NB],
            capitalize( PERS2(ch, d->character)),
	    d->character->colors[C_NB],
            argument);
          send_to_char(buf,d->character);
        }
      }
    }
}

/* guild channels*/
void do_guildtalk( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	MEMBER *m;

	if (!is_guild(ch))  {
		send_to_char("You aren't in a guild.\n\r",ch);
		return;
	}
	
	if ( argument[0] == '\0' ) {
		if (IS_SET(ch->comm,COMM_NOGUILD)) {
			send_to_char("Guild channel is now ON\n\r",ch);
			REMOVE_BIT(ch->comm,COMM_NOGUILD);
		} else {
			send_to_char("Guild channel is now OFF\n\r",ch);
			SET_BIT(ch->comm,COMM_NOGUILD);
		}

		return;
	}

	if( IS_SET( ch->act_bits, ACT_NOAPPROVE ) ) {
		send_to_char( "You must be approved to use this channel.  See 'help approval'.\n\r", ch );
		return;
	}


	REMOVE_BIT(ch->comm,COMM_NOGUILD);
	m = get_member(ch->guild, ch->name);

	sprintf( buf, "%s %s %s: {%c'%s'{x\n\r", 
		pc_guild_table[ch->guild].symbol,
		m->gtitle,
		ch->name,
		ch->colors[C_GDT],
		argument 
	);

	send_to_char( buf, ch );
	for ( d = descriptor_list; d != NULL; d = d->next ) {
		if ( d->connected == CON_PLAYING 
			&& d->character != ch 
			&& is_same_guild(ch,d->character) 
			&& !IS_SET(d->character->comm,COMM_NOGUILD) 
			&& !IS_SET(d->character->comm,COMM_QUIET) )  {

			sprintf( buf, "%s %s %s: {%c'%s'{x\n\r",
				pc_guild_table[ ch->guild ].symbol, m->gtitle,
				(IS_NPC( ch ) ? capitalize(PERS2( ch, d->character)) :  ch->name ),
				d->character->colors[ C_GDT ],
				argument 
			);

			send_to_char( buf, d->character );

		}
	}

	if ( !IS_NPC( ch ) && strlen( argument ) > MIN_CHARACTER_LOGGING ) {
		ch->pcdata->gdts++;
	}
	
	return;
}

void do_immtalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
		if (IS_SET(ch->comm,COMM_NOWIZ))
		{
			send_to_char("[IMMORTAL] channel is now ON\n\r",ch);
			REMOVE_BIT(ch->comm,COMM_NOWIZ);
		}
		else
		{
			send_to_char("[IMMORTAL] channel is now OFF\n\r",ch);
			SET_BIT(ch->comm,COMM_NOWIZ);
		} 
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOWIZ);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
		if ( d->connected == CON_PLAYING && 
	    IS_IMMORTAL(d->character) && 
        !IS_SET(d->character->comm,COMM_NOWIZ) )
		{
			sprintf( buf, "{%c[IMM] {x%s: {%c'%s'{x\n\r", 
			d->character->colors[C_IMM],capitalize( PERS2(ch, d->character)),
			d->character->colors[C_IMM],argument);
			send_to_char(buf,d->character);
		}
    }

    if ( !IS_NPC( ch ) && strlen( argument ) > MIN_CHARACTER_LOGGING )
	{
		ch->pcdata->immtalks++;
	}
	
    return;
}

char *translate_to_vsay( char *argument )
{

    static char outstr[MAX_STRING_LENGTH+100];
    char *pstr;
    int length, newlength;
    int iSyl;

    struct syl_type
    {
		char *	old;
		char *	new;
    };

    static const struct syl_type syl_table[] =
    {
		{ " ",		" "						},
        { "am",         "une"				},
        { "at",         "quo"        		},
        { "kine",       "kine"      		},
        { "human",      "kine"      	},
        { "tion",       "piin"          		},
        { "ing" ,       "pheng"         	},
        { "kiyanne",    "Cain"         },
        { "lilith",     "Lianit",       		},
        { "you",        "tir",          		},
        { "me",         "mme"           },
        { "the",        "fieens"        	},
        { "ess",        "exs"           	},
        { "myself",     "semas"      	},
        { "yourself",   "tirnis"        	},
        { "mine",       "menis"       	},
        { "yours",      "treies"        	},
        { "ings",       "engs"          	},
        { "stupid",     "fenis"         	},
        { "kill",       "vasupate"      	},
        { "blood",      "blod"          	},
        { "with",       "quis"          	},
        { "bastien",    "nivil"         	},
        { "death",      "sinem"        },
        { "ed",         "es"            		},
        { "life",       "nemiis"        	},
        { "without",    "quisem"      },
        { "no",         "ba"            		},
        { "yes",        "bas"           	},
        { "patriarch",  "grisom"     },
        { "dead",       "ex"            	},
        { "drake",      "skarlon"		},
        { "gypsy",      "romanie"	},
        { "savannah",   "me"			},
        { "eat",        "overtan"		},
        { "drink",      "taast"         	},
        { "desire",     "seetcha" 	},
        { "and",        "et"            		},
        { "he",         "m"             		},
        { "they",       "entus"         	},
        { "to",         "obsent"       	},
        { "madelaine",  "Thorn" 	},
        { "leech",      "lehitor"       	},
        { "cursed",     "tomas"       },
        { "curse",      "toma"         	},
        { "revenge",    "taran"       	},
        { "my",         "tra"           		},
        { "pride",      "muir"          	},
        { "plague",     "ferocai"     	},
		{ "a", "a" 	},		{ "b", "p"   	}, 	{ "c", "qi" 	}, 	{ "d", "z" 	},
		{ "e", "i"  	},		{ "f", "snt" 	},		{ "g", "or"  	},		{ "h", "pg" 	},
		{ "i", "u"  	},		{ "j", "y"    	}, 	{ "k", "t"   	}, 	{ "l", "r" 		},
		{ "m", "w"  	}, 	{ "n", "u"    	}, 	{ "o", "i"   	}, 	{ "p", "s" 	},
		{ "q", "dp" 	}, 	{ "r", "f"    	}, 	{ "s", "g"   	}, 	{ "t", "h" 		},
		{ "u", "o"  	}, 	{ "v", "zute" }, 	{ "w", "wwy" }, 	{ "x", "ex" 	},
		{ "y", "wi" 	}, 	{ "?", "?"    	}, 	{ "!", "!"  		},		{ "", "" 		}
    };

    outstr[0] = '\0';
    newlength = 0;

    for ( pstr = argument; *pstr != '\0' && 
	(newlength = strlen(outstr)) < MAX_STRING_LENGTH; pstr += length )
    {
	
		for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) > 0; iSyl++)
		{		
			if ( !str_prefix( syl_table[iSyl].old, pstr ) )
			{
				strcat( outstr, syl_table[iSyl].new );
				break;
			}	
		}
		
		if ( length == 0 )
        {
            /* need to copy color codes over unmodified */
            if ( pstr[0] == '{' && pstr[1] != '\0' )
			{
                length = 2;
			}			
            else
			{
                length = 1;
			}

            strncat( outstr, pstr, length );
        }
    }
    
    if ( newlength > MAX_STRING_LENGTH )
	{
		outstr[MAX_STRING_LENGTH-1] = '\0';
	}
	
    return outstr;
}

void do_vsay( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char vbuf [MAX_STRING_LENGTH];
    CHAR_DATA *rch;

    if ( !IS_VAMPIRE(ch)
    &&   !IS_SET(ch->comm, COMM_VAMP) )
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( IS_NULLSTR(argument) )
    {
      send_to_char( "Say what?\n\r", ch );
      return;
    }

    sprintf(buf,"You say {%c'%s'{x\n\r", ch->colors[C_VSAY], argument);
    send_to_char(buf,ch);

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
        sprintf( vbuf, "%s says {%c'%s'{x\n\r",
		capitalize( PERS(ch, rch)), ch->colors[C_VSAY], translate_to_vsay(argument) );

        /* so bug-ugly, but it's an institution now */
        sprintf( buf,  "%s says {%c'%s{x'.\n\r",
		capitalize( PERS(ch, rch)), ch->colors[C_VSAY], argument );

		if ( rch != ch )
		{
			if ( rch->position > POS_SLEEPING )
			{ 
                if ( IS_SET(rch->comm,COMM_VAMP)
                ||   IS_SET(rch->shiftbits, PERM_VAMP)
                ||   IS_SET(rch->shiftbits, TEMP_VAMP) )
				{
                   send_to_char(buf,rch);
				}
                else
				{
                   send_to_char(vbuf,rch);
				}
			}
		}
    }
    return;
}

char *translate_to_enochian( char *argument )
{

    static char outstr[MAX_STRING_LENGTH+100];
    char *pstr;
    int length, newlength;
    int iSyl;

    struct syl_type
    {
		char *	old;
		char *	new;
    };

    static const struct syl_type syl_table[] =
    {
{ " ",		" "							},
{"abiding", "cafafam"			}, {"admiration", "rsam"			}, {"age", "homin"				}, {"ages", "cocasb"			},
{"all", "i"								}, {"all powerful", "ia-idon"		}, {"also", "t"						}, {"amoment", "oanio"		},
{"among", "aai"					}, {"among us", "a-ai-om"		}, {"and", "od"						}, {"anger", "vnph"				},
{"any", "droln"						}, {"apply", "im-va-mar"			}, {"are", "chis"					}, {"are not", "chis-ge"			},
{"arise", "torzu"					}, {"art", "ge"							}, {"artists", "geh"				}, {"as", "ta"							},
{"as many", "plosi"				}, {"balance", "piap"				}, {"be", "lu-la"						}, {"beauty", "turbs"				},
{"become", "noan"				}, {"beginning", "iaod"				}, {"behold", "micma"			}, {"bind", "allar"					},
{"blind", "uran"						}, {"blood", "cnila"					}, {"branches", "ilonon"		}, {"breath", "gigpah"			},
{"brightness", "piripsol"		}, {"bring", "drix"						}, {"brothers", "saisch"		}, {"building", "trof"				},
{"burn", "ialpon"					}, {"burning flames", "ial-prg"	}, {"but", "crp"						}, {"called", "vmd"				},
{"can", "adgt"						}, {"children", "pashs"				}, {"circle", "coselha"			}, {"come", "niis"					},
{"come away", "niiso"			}, {"comfort", "bliora"				}, {"comforter", "bilgiad"		}, {"comforters", "blior"		},
{"conclude", "via-ial"			}, {"conquest", "zilodarp"		}, {"content", "q"					}, {"continually","pild"			},
{"continuance", "miam"		}, {"creator", "qaal"					}, {"creatures", "hami"		}, {"cried", "bahal"				},
{"crown", "momao"				}, {"crowned", "momar"			}, {"cups", "talho"				}, {"cursed", "amma"			},
{"day", "basigm"					}, {"death", "teloch"					}, {"decend", "vniglag"			}, {"deliver", "obelisong"		},
{"delivered", "zonrensg"		}, {"divided", "poilp"					}, {"do", "gnay"					}, {"dragon", "vovin"			},
{"drunken", "orsba"				}, {"dryness", "orscor"			}, {"dwelling", "faonts"			}, {"eagle", "vabzir"				},
{"earth", "caosgo"				}, {"earthquakes", "gizyax"		}, {"east", "raas"					}, {"eastern", "rassy"		 	},
{"echoing", "matorb"			}, {"elders", "vran"					}, {"empty", "affa"				}, {"end", "ipamis"				},
{"eternal", "io-iad"				}, {"even", "nomig"					}, {"everyone", "lonsa"		}, {"execution", "mian"		}, 
{"eyes", "ooaona"				}, {"faith", "gono"						}, {"fall", "loncho"					}, {"feet", "lusda"					},
{"fire", "prge"}, {"fires", "malpirg"}, {"firey", "malpg"}, {"firmament", "pil"},
{"first", "elo"}, {"flame", "ial-prt"}, {"flames", "ialpir"}, {"flaming", "ialpor"},
{"flourish", "ca-cacom"}, {"flowers", "lorslq"}, {"for", "lap"}, {"friendly", "zorge"},
{"from", "fafen"}, {"frown not", "vcim"},{"fury", "baghie"}, {"garland", "obloc"},
{"garments", "nothoa"}, {"garnished", "gnonp"}, {"gathering", "aldi"}, {"girdles", "atraah"},
{"given", "dlugam"}, {"giving", "dluga"}, {"gladness", "nazarth"}, {"glory", "busdir"},
{"gods", "iad"}, {"going before", "tastax"}, {"government", "netaab"}, {"governments", "gnetaab"},
{"governor", "tabaan"}, {"great", "drilpa"}, {"greater than", "drilpi"}, {"groaned", "holdo"},
{"guard", "bransg"}, {"half", "obza"}, {"hand", "ozien"}, {"hands", "zol"},
{"happy", "vlcinin"}, {"harlot", "ababalond"}, {"harvest", "aziagiar"}, {"have", "brin"},
{"he", "tia"},
{"his", "tox"},
{"head", "dazis"},
{"heads", "daziz"},
{"heart", "monons"},
{"heaven", "madriax"},
{"her", "tilb"},
{"here", "kures"},
{"highest", "iaida"},
{"him", "tox"},
{"honor", "iaiadix"},
{"I", "ol"},
{"I am", "zirdo"},
{"in", "a"},
{"include", "iaial"},
{"increase", "coazior"},
{"inhavit", "zodireda"},
{"iniquity", "madrid"},
{"intent", "fafen"},
{"into", "raasi"},
{"invoke", "vinu"},
{"is", "i"},
{"it", "t"},
{"it is", "ti"},
{"itself", "zylna"},
{"jaw", "piadph"},
{"joy", "qzmoz"},
{"judgment", "alca"},
{"just", "balit"},
{"justice", "baltim"},
{"kingdom", "adohi"},
{"kingdoms", "londoh"},
{"know", "om"},
{"knowledge", "iadnah"},
{"known", "ixomaxip"},
{"laid", "maafi"},
{"lamentation", "eophan"},
{"lamp", "hubai"},
{"law", "ohorela"},
{"let", "tabaord"},
{"life", "malpirgi"},
{"lift", "goholor"},
{"light", "micaloz"},
{"like", "aziagor"},
{"likeness", "aziazor"},
{"listen", "solpeth"},
{"live", "salbrox"},
{"long", "solamian"},
{"look", "dorpha"},
{"lord", "enay"},
{"loud", "bahal"},
{"lower", "oadriax"},
{"made", "oln"},
{"magnify", "ovof"},
{"make", "eol"},
{"man", "cordziz"},
{"many", "plosi"},
{"master", "iad"},
{"may", "restil"},
{"maybe", "noaln"},
{"measure", "holq"},
{"mercy", "rit"},
{"mighty", "micalzq"},
{"mind", "manin"},
{"mine", "ozien"},
{"mix", "cyrnix"},
{"moment", "ol"},
{"moon", "graa"},
{"mother", "exentaser"},
{"motion", "zna"},
{"mount", "adroch"},
{"mouth", "butmon"},
{"mourning", "ser"},
{"move", "zacar"},
{"my", "lasdi"},
{"name", "dooain"},
{"neither", "larag"},
{"night", "dosig"},
{"no", "ag"},
{"no one", "agt"},
{"noise", "nidali"},
{"none", "agz"},
{"noon", "bazm"},
{"north ", "lucal"},
{"not", "ip"},
{"number", "cormf"},
{"oh", "c"},
{"oak", "paeb"},
{"obedience", "gono"},
{"obey", "darbs"},
{"of", "de"},
{"olive", "adroch"},
{"on", "mirc"},
{"one", "saga"},
{"only", "crip"},
{"open", "odo"},
{"or", "q"},
{"ornaments", "luciftian"},
{"our", "ge"},
{"out", "vors"},
{"over", "vorsg"},
{"own", "ralao"},
{"pair", "pala"},
{"palace", "poamal"},
{"part", "ol"},
{"peace", "fetharsi"},
{"perform", "fifis"},
{"period", "capimao"},
{"pillar", "naz"},
{"place", "ripir"},
{"plant", "harg"},
{"pleasant", "obelisong"},
{"pleasure", "norqrasahi"},
{"poison", "faboan"},
{"possibility", "bab"},
{"pour down", "panpir"},
{"power", "lonsa"},
{"praise", "ercin"},
{"pray", "lava"},
{"prepare", "abramg"},
{"presence", "gmicalzo"},
{"promise", "isro"},
{"protect", "blans"},
{"providence", "tooat"},
{"put", "oali"},
{"quality", "aspian"},
{"raise", "farzm"},
{"range", "dsonf"},
{"reasonable", "cordziz"},
{"receive", "ednas"},
{"regret", "moooah"},
{"reign", "bogpa"},
{"rejoice", "chirlan"},
{"remain", "paaox"},
{"remember", "papnor"},
{"repent", "mootzo"},
{"require", "vnig"},
{"rest", "page"},
{"rich", "las"},
{"righteous", "balit"},
{"righteousness", "baltoh"},
{"rise", "torsvl"},
{"roar", "oanio"},
{"rock", "patraxl"},
{"rod", "cab"},
{"rose", "torzvla"},
{"rotten", "qting"},
{"rule", "bogpa"},
{"run", "parmgi"},
{"said", "gohvlim"},
{"saint", "soyga"},
{"salt", "balie"},
{"same", "lel"},
{"say", "gohia"},
{"scorpion", "siatris"},
{"seal", "emetgis"},
{"seas", "zvmvi"},
{"season", "nibm"},
{"seat", "othil"},
{"second", "viu"},
{"secret", "laiad"},
{"see", "odipvran"},
{"separate", "tliob"},
{"servant", "noco"},
{"serve", "aboapri"},
{"set", "othil"},
{"settle", "alar"},
{"shall", "trian"},
{"sharp", "tapvin"},
{"shetler", "blans"},
{"shine", "loholo"},
{"show", "odzamran"},
{"shrine", "arba"},
{"sin", "doalim"},
{"sing", "zamran"},
{"sink", "carbaf"},
{"sit", "trint"},
{"skirt", "unalab"},
{"sleep", "brgda"},
{"smile", "ucim"},
{"song", "lviahe"},
{"son", "noromi"},
{"sorrow", "tibibp"},
{"sound", "sapah"},
{"south", "babage"},
{"speak", "camlix"},
{"spirit", "gah"},
{"stand", "biah"},
{"sting", "grosb"},
{"stir", "zixzia"},
{"stone", "orri"},
{"strange", "vgeg"},
{"strength", "ugear"},
{"such", "cors"},
{"sulphur", "salbrox"},
{"sun", "ror"},
{"surge", "molvi"},
{"swear", "surzas"},
{"sword", "nazpsad"},
{"talk", "brita"},
{"tell", "mapsama"},
{"temple", "siaion"},
{"terror", "ciaofi"},
{"that", "na"},
{"the", "a"},
{"their", "cafafam"}, 
{"them", "dlugar"},
{"there", "da"},
{"these", "unal"},
{"they", "par"},
{"thought", "angelard"},
{"throne", "oxiayal"},
{"through", "malprg"},
{"thunder", "avavago"},
{"time", "cocasb"},
{"to", "de"},
{"together", "commah"},
{"torment", "mir"},
{"triad", "na"},
{"triumph", "homtoh"},
{"truth", "vaoan"},
{"under", "oroch"},
{"underneath", "orocha"},
{"understand", "faaip"},
{"unspeakable", "adphaht"},
{"until", "cacrg"},
{"unto", "pugo"},
{"up", "allar"},
{"upon", "mir"},
{"us", "ef"},
{"variety", "damploz"},
{"veil", "zodimipe"},
{"vessel", "zizop"},
{"vial", "ofafafe"},
{"virgin", "paradiz"},
{"visit", "f"},
{"voice", "bialo"},
{"walk", "insi"},
{"warden", "lixipsp"},
{"was", "zirop"},
{"water", "zodinu"},
{"waters", "sobam"},
{"we", "gohia"},
{"weave", "oado"},
{"wedding", "paraceda"},
{"weeping", "raclir"},
{"which", "dsi"},
{"who", "ds"},
{"whom", "casarma"},
{"whose", "soba"},
{"why", "bagle"},
{"wicked", "babalon"},
{"will", "gemeganza"},
{"wind", "zong"},
{"wine", "roxtan"},
{"wing", "upaah"},
{"wisdom", "ananael"},
{"with", "c"},
{"woe", "ohio"},
{"work", "vaul"},
{"works", "sobhaath"},
{"worship", "boaluahe"},
{"wrath", "unph"},
{"ye", "niss"},
{"yes", "noib"},
{"you", "gi"},
{"to you", "noncf"},
{"your", "g"},
{"yourselves", "amiran"},
{"a", "un"								}, {"b", "p"								}, {"c","h"							}, {"d","g"							},
{"e","ah"							}, {"f","r"									}, {"g","e"							}, {"h","z"							},
{"i","on"								}, {"j","h"							}, {"k","v"						}, {"l","r"								},
{"m","t"								}, {"n","r"							}, {"o","eh"						}, {"p","m"						},
{"q","r"								}, {"r","d"								}, {"s","f"							}, {"t","i"							},
{"u","v"								}, {"v","z"								}, {"w","n"							}, {"x","p"							},
{"y","n"							}, {"z","r"								}, {"?","?"								}, {"!","!"								}, 
{ "", "" 		},
      { NULL, NULL }
    };

    outstr[0] = '\0';
    newlength = 0;

    for ( pstr = argument; *pstr != '\0' && 
	(newlength = strlen(outstr)) < MAX_STRING_LENGTH; pstr += length )
    {
	
		for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) > 0; iSyl++)
		{		
			if ( !str_prefix( syl_table[iSyl].old, pstr ) )
			{
				strcat( outstr, syl_table[iSyl].new );
				break;
			}	
		}
		
		if ( length == 0 )
        {
            /* need to copy color codes over unmodified */
            if ( pstr[0] == '{' && pstr[1] != '\0' )
			{
                length = 2;
			}			
            else
			{
                length = 1;
			}

            strncat( outstr, pstr, length );
        }
    }
    
    if ( newlength > MAX_STRING_LENGTH )
	{
		outstr[MAX_STRING_LENGTH-1] = '\0';
	}
	
    return outstr;
}

void do_enochian( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char ebuf [MAX_STRING_LENGTH];
    CHAR_DATA *rch;

    if (ch->enochian == 0)
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( IS_NULLSTR(argument) )
    {
      send_to_char( "Say what?\n\r", ch );
      return;
    }

    sprintf(buf,"You say {c'%s'{x\n\r", argument);
    send_to_char(buf,ch);

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
        sprintf(ebuf, "%s says {c'%s'{x\n\r",
		capitalize( PERS(ch, rch)), translate_to_enochian(argument) );

        /* so bug-ugly, but it's an institution now */
        sprintf(buf,  "%s says {c'%s'{x\n\r",
		capitalize( PERS(ch, rch)), argument );

		if ( rch != ch )
		{
			if ( rch->position > POS_SLEEPING )
			{ 
                if (rch->enochian == 1)
				{
                   send_to_char(buf,rch);
				}
                else
				{
                   send_to_char(ebuf,rch);
				}
			}
		}
    }
    return;
}

void do_sing( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *rch;

    if (argument[0] == '\0' )
    {
      	if (IS_SET(ch->comm,COMM_NOSING))
      	{
            send_to_char("You can sing again.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_NOSING);
      	}
      	else
      	{
            send_to_char("You will no longer hear singing.\n\r",ch);
            SET_BIT(ch->comm,COMM_NOSING);
      	}
      	return;
    }

    if (IS_SET(ch->shiftbits,SHIFT_HALF) || IS_SET(ch->shiftbits,SHIFT_FULL))
    {
       send_to_char("Your cannot sing in this beastial form\n\r",ch);
       return;
    }

    if ( IS_SET(ch->comm, COMM_NOSING) )
    {
        send_to_char( "You can't sing.\n\r", ch );
        return;
    }
 
   sprintf(buf,"You sing: {%c'%s'{x\n\r",ch->colors[C_SAY],argument);
   send_to_char(buf,ch);

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room)
    {
       if (rch != ch) 
       {
        if (rch->position > POS_SLEEPING)
        {
          { 
            sprintf( buf, "%s sings: {%c'%s'{x\n\r", capitalize( PERS(ch, rch)),
               rch->colors[C_SAY], argument);
             send_to_char(buf,rch);
          }
        }
       }
    }
}



char* were_parse( const char* original )
{
  const char* original_ptr = NULL;
  char* new_string = NULL;
  int orig_length = 0;
  int new_length = 0;
  int i = 0;
  bool found = FALSE;

  struct syl_type
    {
      char* old;
      char* new;
    };

  static const struct syl_type syl_table[] =
    {
      { " ",		" "					},
      { "sh",         "sshh"		},
      { "ck",         "ckhh"		},
      { "gr",         "ggrr"			},
      { "cr",         "ccrr"			},
      { "ing",        "inngrr"		},
      { "tion",       "ssshuun"	},
      { "th",         "dr"				},
      { "qu",         "cchh"		},
      { "a", "a" 	}, 	{ "b", "b" 	}, 	{ "c", "c" 	}, 	{ "d", "d" 		},
      { "e", "e" 	}, 	{ "f", "ph" 	}, 	{ "g", "gr" 	}, 	{ "h", "h"			},
      { "i", "i" 	}, 	{ "j", "j" 		}, 	{ "k", "k" 	}, 	{ "l", "ll" 			},
      { "m", "m" }, 	{ "n", "n" 	}, 	{ "o", "o" 	}, 	{ "p", "p" 		},
      { "q", "q" 	}, 	{ "r", "rr" 		}, 	{ "s", "s" 	}, 	{ "t", "t" 		 	},
      { "u", "u" 	}, 	{ "v", "f" 		}, 	{ "w", "wr" 	}, 	{ "x", "echs"	},
      { "y", "y" 	}, 	{ "?", "?" 	}, 	{ "!", "!" 		},
      { NULL, NULL }
    };

  for( original_ptr = original; *original_ptr != '\0'; original_ptr += orig_length )
    {
		found = FALSE;
		for( i = 0; syl_table[ i ].old != NULL; i++ )
		{
			if( !str_prefix( syl_table[ i ].old, original_ptr ) )
			{
				orig_length = strlen( syl_table[ i ].old );
				new_length += strlen( syl_table[ i ].new );
				found = TRUE;
				break;
			}
		}

		if( !found )
		{
			orig_length = 1;
			new_length += 1;
		}
    }

	new_string = ( char* )malloc( ( sizeof( char ) * new_length ) + 1 );
	new_string[ 0 ] = '\0';

	new_length = 0;

	for( original_ptr = original; *original_ptr != '\0'; original_ptr += orig_length )
    {
		found = FALSE;

		for( i = 0; syl_table[ i ].old != NULL; i++ )
		{
			if( !str_prefix( syl_table[ i ].old, original_ptr ) )
			{
				orig_length = strlen( syl_table[ i ].old );
				strcat( new_string, syl_table[ i ].new );
				found = TRUE;
				break;
			}
		}

		if( !found )
		{
			orig_length = 1;
			new_string[ new_length ] = *original_ptr;
			new_string[ new_length + 1 ] = '\0';
		}

		new_length = strlen( new_string );
    }

  return new_string;
}

void do_say( CHAR_DATA* ch, char* argument )
{
  static const int LANGUAGE_ASK = 1;
  static const int LANGUAGE_EXC = 2;
  char msg_to_self[ MAX_STRING_LENGTH ] = "";
  char msg_to_room[ MAX_STRING_LENGTH ] = "";
  char* were_speak = NULL;
  int language_choice = 0;
  CHAR_DATA* rch = NULL;

  struct language_says
  {
    char* to_self;
    char* to_others;
  };

  static const struct language_says real_table[] =
  {
      { "say", "says" },
      { "ask", "asks" },
      { "exclaim", "exclaims" }
  };
  static const struct language_says sings_table[] =
  {
      { "sing", "sings" },
      { "sing", "sings" },
      { "sing", "sings" }
  };

  struct language_says says_table[3];


  if ( global_config_aprilfools )
  {
    says_table[0].to_self = sings_table[0].to_self;
    says_table[1].to_self = sings_table[1].to_self;
    says_table[2].to_self = sings_table[2].to_self;
    says_table[0].to_others = sings_table[0].to_others;
    says_table[1].to_others = sings_table[1].to_others;
    says_table[2].to_others = sings_table[2].to_others;
  }
  else
  {
    says_table[0].to_self = real_table[0].to_self;
    says_table[1].to_self = real_table[1].to_self;
    says_table[2].to_self = real_table[2].to_self;
    says_table[0].to_others = real_table[0].to_others;
    says_table[1].to_others = real_table[1].to_others;
    says_table[2].to_others = real_table[2].to_others;
  }


  if( IS_SET( ch->shiftbits, SHIFT_FULL ) )
    {
      send_to_char( "Animals cannot speak the language of people!\n\r", ch );
      return;
    }

	if (ch->race == race_lookup("seraph") && ch->life_energy < 60) 
	{
		int chance = number_range(1,100);
		
		if (chance < 50) 
		{
			send_to_char( "You begin to speak, but decide you don't have the energy.\n\r", ch );
			act("$n starts to speak but only stammers.",ch,NULL,NULL,TO_ROOM);
			return;
		}
	
	}
	

  if( *argument == '\0' )
    {
      send_to_char( "Say what?\n\r", ch );
      return;
    }

  if( IS_SET( ch->shiftbits, SHIFT_HALF ) )
    {
      were_speak = were_parse( argument );
    }

  if( strchr( argument, '?' ) )
    {
      language_choice = LANGUAGE_ASK;
    }
  else if( strchr( argument, '!' ) )
    {
      language_choice = LANGUAGE_EXC;
    }

  if( IS_AFFECTED3( ch, AFF_FROG ) )
    {
      sprintf( msg_to_self, "You croak noisily!\n\r" );
    }
  else
    {
      sprintf( msg_to_self, "You %s {%c'%s'{x\n\r",
	       says_table[ language_choice ].to_self,
	       ch->colors[ C_SAY ],
	       were_speak == NULL ? argument : were_speak );
    }

  send_to_char( msg_to_self, ch );

  for( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
      if( rch != ch )
	{
	  if( rch->position > POS_SLEEPING )
	    {
	      if( IS_AFFECTED3( ch, AFF_FROG ) )
		{
		  sprintf( msg_to_room, "%s %s {%c'%s'{x\n\r",
			   capitalize( PERS( ch, rch ) ),
			   says_table[ language_choice ].to_others,
			   rch->colors[ C_SAY ],
			   "ribbit, ribbit." );
		}
	      else if( IS_AFFECTED2( rch, AFF_DETECTWERE ) || IS_SET( rch->act_bits, PLR_HOLYLIGHT ) )
		{
		  sprintf( msg_to_room, "%s %s {%c'%s'{x\n\r",
			   capitalize( PERS( ch, rch ) ),
			   says_table[ language_choice ].to_others,
			   rch->colors[ C_SAY ],
			   argument );

		}
	      else
		{
		  sprintf( msg_to_room, "%s %s {%c'%s'{x\n\r",
			   capitalize( PERS( ch, rch ) ),
			   says_table[ language_choice ].to_others,
			   rch->colors[ C_SAY ],
			   were_speak == NULL ? argument : were_speak );
		}

	      send_to_char( msg_to_room, rch );
	    }
	}
    }

  if( were_speak )
    free( were_speak );
}


void do_shout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0' )
    {
      	if (IS_SET(ch->comm,COMM_SHOUTSOFF))
      	{
            send_to_char("You can hear shouts again.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);
      	}
      	else
      	{
            send_to_char("You will no longer hear shouts.\n\r",ch);
            SET_BIT(ch->comm,COMM_SHOUTSOFF);
      	}
      	return;
    }

    if (IS_SET(ch->shiftbits,SHIFT_HALF) || IS_SET(ch->shiftbits,SHIFT_FULL))
    {
       send_to_char(
           "Your shouts cannot be understood in this beastial form.\n\r" ,ch);
       return;
    }

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "You can't shout.\n\r", ch );
        return;
    }
 
    REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);

    WAIT_STATE( ch, 12 );

    sprintf(buf,"You shout {%c'%s'{x\n\r", ch->colors[C_SHOUT], argument);
    send_to_char(buf,ch);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;
	victim = d->original ? d->original : d->character;
	if ( d->connected == CON_PLAYING 
        &&   d->character != ch 
        &&   !IS_SET(victim->comm, COMM_SHOUTSOFF) 
        &&   !IS_SET(victim->comm, COMM_QUIET) ) 
	{
          sprintf( buf, "%s shouts {%c'%s'{x\n\r", 
	    capitalize( PERS(ch, d->character)),
            d->character->colors[C_SHOUT],
            argument);
          send_to_char(buf,d->character);
	}
    }

    return;
}



void do_tell( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

    if (IS_SET(ch->shiftbits,SHIFT_HALF) || IS_SET(ch->shiftbits,SHIFT_FULL))
    {
        send_to_char("They cannot understand you in this beastial form.\n\r",ch);
        return;
    }

    if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_DEAF))
    {
		send_to_char( "Your message didn't get through.\n\r", ch );
		return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
		send_to_char( "You must turn off quiet mode first.\n\r", ch);
		return;
    }

    if (IS_SET(ch->comm,COMM_DEAF))
    {
		send_to_char("You must turn off deaf mode first.\n\r",ch);
		return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
		send_to_char( "Tell whom what?\n\r", ch );
		return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
        if ( !str_cmp( arg, "someone" ) )
		{
            send_to_char( "You should use the 'reply' command instead.\n\r",ch);
		}
		else
		{
			send_to_char( "They aren't here.\n\r", ch );
		}
	return;
    }

    if( !IS_NPC( victim ) && !IS_IMMORTAL( ch ) && IS_AFFECTED3( victim, AFF_VEIL ) )
    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
    }

    if((is_affected(ch,skill_lookup("devotion"))) && (victim->level > 50))
    {
		act("You are too busy pondering life to speak to the Gods.",
		ch,NULL,NULL,TO_CHAR);
		return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
		act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
		
			if (victim->race == race_lookup("seraph") && victim->life_energy < 60)
			{
				sprintf(buf,"A distance voice tells you {g'%s'{x\n\r",argument);
			} 
				else if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
				{
					if (ch->availability == 1) {
					sprintf(buf,"{G%s{x tells you {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
					}
						else {
							sprintf(buf,"{r%s{x tells you {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
						}
				}
					else 
					{
						sprintf(buf,"%s tells you {g'%s'{x\n\r",PERS2(ch,victim),argument);
					}
			
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
		return;
    }

    if ( !(IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL) && !IS_AWAKE(victim) )
    {
		act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
		return;
    }
  
    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF)) && !IS_IMMORTAL(ch))
    {
		act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
		return;
    }

    if (IS_SET(victim->comm,COMM_AFK))
    {
		if (IS_NPC(victim))
		{
			act("$E is AFK, and not receiving tells.",ch,NULL,victim,TO_CHAR);
			return;
		}

		act("$E is AFK, but your tell will go through when $E returns.", ch,NULL,victim,TO_CHAR);
		
		if (victim->race == race_lookup("seraph") && victim->life_energy < 60)
			{
				sprintf(buf,"A distance voice tells you {g'%s'{x\n\r",argument);
			} 
				else if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
				{
					if (ch->availability == 1) {
					sprintf(buf,"{G%s{x tells you {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
					}
						else {
							sprintf(buf,"{r%s{x tells you {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
						}
				}
					else 
					{
						sprintf(buf,"%s tells you {g'%s'{x\n\r",PERS2(ch,victim),argument);
					}
		
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
		return;
    }

    sprintf( buf, "You tell %s {%c'%s'{x\n\r", PERS2(victim, ch),ch->colors[C_TELL],argument);
    send_to_char(buf,ch);
	
    if (victim->race == race_lookup("seraph") && victim->life_energy < 60)
	{
		sprintf(buf,"A distant voice tells you {%c'%s'{x\n\r",victim->colors[C_TELL],argument);
	}
	
		else if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
		{
				if (ch->availability == 1) {
				sprintf(buf,"{G%s{x tells you {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
				}
					else {
						sprintf(buf,"{r%s{x tells you {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
					}
		}
	
			else 
			{
				sprintf(buf,"%s tells you {%c'%s'{x\n\r",PERS2(ch,victim), victim->colors[C_TELL],argument);
			}
	
    buf[0] = UPPER(buf[0]);
    send_to_char(buf,victim);
    victim->reply	= ch;
	
	if (!IS_NPC(ch) && IS_IMMORTAL(ch)) {
	 victim->immreply = ch;
	 }

    if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
	{
		mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );
	}

    return;
}


void do_otell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    if (IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char("You must turn off deaf mode first.\n\r",ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if( !IS_NPC( victim ) &&
	!IS_IMMORTAL( ch ) &&
	IS_AFFECTED3( victim, AFF_VEIL ) )
      {
	send_to_char( "They aren't here.\n\r", ch );
	return;
      }


    if((is_affected(ch,skill_lookup("devotion"))) 
	&& (victim->level > 50))
    {
	act("You are too busy pondering life to speak to the gods.",
	  ch,NULL,NULL,TO_CHAR);
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
		act("$N seems to have misplaced $S link...try again later.", ch,NULL,victim,TO_CHAR);
		
		 if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
				{
					if (ch->availability == 1) {
					sprintf(buf,"{G%s{x tells you OOCly {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
					}
						else {
							sprintf(buf,"{r%s{x tells you OOCly {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
						}
				}
					else 
					{
						sprintf(buf,"%s tells you OOCly {%c'%s'{x\n\r", PERS2(ch,victim),victim->colors[C_TELL],argument);
					}
					
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
	return;
    }

    if ( !(IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }
  
    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
    && !IS_IMMORTAL(ch))
    {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
  	return;
    }

    if (IS_SET(victim->comm,COMM_AFK))
    {
	if (IS_NPC(victim))
	{
	    act("$E is AFK and not receiving tells.",ch,NULL,victim,TO_CHAR);
	    return;
	}

	act("$E is AFK but your tell will go through when $E returns.", ch,NULL,victim,TO_CHAR);
	
	if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
				{
					if (ch->availability == 1) {
					sprintf(buf,"{G%s{x tells you OOCly {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
					}
						else {
							sprintf(buf,"{r%s{x tells you OOCly {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
						}
				}
					else 
					{
						sprintf(buf,"%s tells you OOCly {%c'%s'{x\n\r", PERS2(ch,victim),victim->colors[C_TELL],argument);
					}
					
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer,buf);
	return;
    }

	if (IS_IMMORTAL(victim) && (victim->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
	{
		if (victim->availability == 1) {
			sprintf(buf,"You tell {G%s{x OOCly {%c'%s'{x\n\r", victim->name, ch->colors[C_TELL],argument);
		}
			else {
				sprintf(buf,"You tell {r%s{x OOCly {%c'%s'{x\n\r", victim->name, ch->colors[C_TELL],argument);
			}
	}
		
	else {
		sprintf( buf, "You tell %s OOCly {%c'%s'{x\n\r", PERS2(victim, ch), ch->colors[C_TELL],argument);
	}
    send_to_char(buf,ch);
	
	if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
	{
		if (ch->availability == 1) {
			sprintf(buf,"{G%s{x tells you OOCly {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
		}
			else {
				sprintf(buf,"{r%s{x tells you OOCly {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
			}
	}
	
	else {
    sprintf( buf, "%s tells you OOCly {%c'%s'{x\n\r", PERS2(ch, victim),victim->colors[C_TELL],argument);
	}
	
    buf[0] = UPPER(buf[0]);
    send_to_char(buf,victim);
    victim->reply	= ch;
	victim->oreply	= ch;
	
	if (!IS_NPC(ch) && IS_IMMORTAL(ch)) {
	 victim->immreply = ch;
	 }

    return;
}

void do_oreply( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim = NULL;
  char buf[MAX_STRING_LENGTH] = "";
  char* were_speak = NULL;
  char* msg_to_char = NULL;
  char* msg_to_vict = NULL;

  if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
      send_to_char( "Your message didn't get through.\n\r", ch );
      return;
    }

  if ( ( victim = ch->oreply ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  if ( victim->desc == NULL && !IS_NPC(victim))
    {
      act("$N seems to have misplaced $S link...try again later.",ch,NULL,victim,TO_CHAR);
	  
	  if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
	{
		if (ch->availability == 1) {
			sprintf(buf,"{G%s{x tells you OOCly {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
		}
			else {
				sprintf(buf,"{r%s{x tells you OOCly {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
			}
	}
	
	else {
      sprintf(buf,"%s tells you OOCly {%c'%s'\n\r",PERS2(ch,victim),victim->colors[C_REPLY],argument);
	}
	
      buf[0] = UPPER(buf[0]);
      add_buf(victim->pcdata->buffer,buf);
      return;
    }

  if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
      act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
      return;
    }

  if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF)) 
  &&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim))
    {
      act_new( "$E is not receiving tells.", ch, 0, victim, TO_CHAR,POS_DEAD, 0);
      return;
    }

  if (!IS_IMMORTAL(victim) && !IS_AWAKE(ch))
    {
      send_to_char( "In your dreams, or what?\n\r", ch );
      return;
    }

  if((is_affected(ch,skill_lookup("devotion"))) &&
     (IS_IMMORTAL(victim)))
    {
		act("You are too busy pondering life to speak to the Gods.",
        ch,NULL,NULL,TO_CHAR);
		return;
    }
	
  if (IS_SET(victim->comm,COMM_AFK))
    {
	
		if (IS_NPC(victim))
		{
			act_new("$E is AFK, and not receiving tells.",
			ch,NULL,victim,TO_CHAR,POS_DEAD, 0);
			return;
		}
 
		act_new("$E is AFK, but your tell will go through when $E returns.",ch,NULL,victim,TO_CHAR,POS_DEAD, 0);
		
		if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
		{
			if (ch->availability == 1) {
			sprintf(buf,"{G%s{x tells you OOCly {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
			}
				else {
					sprintf(buf,"{r%s{x tells you OOCly {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
				}
		}
	
	else {
		sprintf(buf,"%s tells you OOCly {%c'%s'\n\r",PERS2(ch,victim),victim->colors[C_REPLY],argument);
	}
	
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
		return;
    }

  if( IS_SET( ch->shiftbits, SHIFT_HALF ) || IS_SET( ch->shiftbits, SHIFT_FULL ) )
    {
      were_speak = were_parse( argument );
    }

  if( were_speak )
    {
		if( IS_AFFECTED2( ch, AFF_DETECTWERE ) || IS_SET( ch->act_bits, PLR_HOLYLIGHT ) )
		{
			msg_to_char = argument;
		}
		
		else
		{
			msg_to_char = were_speak;
		}

		if( IS_AFFECTED2( victim, AFF_DETECTWERE ) || IS_SET( victim->act_bits, PLR_HOLYLIGHT ) )
		{
			msg_to_vict = argument;
		}
		
		else
		{
			msg_to_vict = were_speak;
		}

		sprintf( buf, "You reply to %s OOCly {%c'%s'{x\n\r",PERS2( victim, ch ),ch->colors[C_REPLY],msg_to_char );

		send_to_char(buf,ch);

		sprintf( buf, "%s replies OOCly {%c'%s'{x\n\r", PERS2(ch, victim),victim->colors[C_REPLY],msg_to_vict );

		buf[0] = UPPER(buf[0]);
		send_to_char(buf,victim);

		msg_to_char = NULL;
		msg_to_vict = NULL;
		free( were_speak );

      /* Were speech goes through unadulterated for reply */
		if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
		{
			mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );
		}

    }
	
  else
    {
	
		if (ch->race == race_lookup("seraph") && ch->life_energy < 60)
		{
			sprintf( buf, "You reply to a distant voice {%c'%s'{x\n\r",ch->colors[C_REPLY],argument );
		}
		
		else if (IS_IMMORTAL(victim) && (victim->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
		{
			if (victim->availability == 1) {
				sprintf(buf,"You reply to {G%s{x OOCly {%c'%s'{x\n\r", victim->name, ch->colors[C_REPLY],argument);
			}
				else {
				sprintf(buf,"You reply to {r%s{x OOCly {%c'%s'{x\n\r", victim->name, ch->colors[C_REPLY],argument);
				}
		}
		
		else 
		{
			sprintf( buf, "You reply to %s OOCly {%c'%s'{x\n\r",PERS2( victim, ch ),ch->colors[C_REPLY],argument ); 
		}
		
		send_to_char(buf,ch);
		
		if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
		{
			if (ch->availability == 1) {
				sprintf(buf,"{G%s{x replies OOCly {%c'%s'{x\n\r", ch->name, victim->colors[C_REPLY],argument);
			}
				else {
				sprintf(buf,"{r%s{x replies OOCly {%c'%s'{x\n\r", ch->name, victim->colors[C_REPLY],argument);
				}
		}
		
		else {
		sprintf( buf, "%s replies OOCly {%c'%s'{x\n\r", PERS2(ch, victim),victim->colors[C_REPLY],argument );
		}

		buf[0] = UPPER(buf[0]);
		send_to_char(buf,victim);
       
		if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
		{
			mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );
		}
		
    }
	
	victim->reply	= ch;
	victim->oreply	= ch;
	
	if (!IS_NPC(ch) && IS_IMMORTAL(ch)) {
	 victim->immreply = ch;
	 }
	 
	return;
}


void do_reply( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim = NULL;
  char buf[MAX_STRING_LENGTH] = "";
  char* were_speak = NULL;
  char* msg_to_char = NULL;
  char* msg_to_vict = NULL;

  if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
      send_to_char( "Your message didn't get through.\n\r", ch );
      return;
    }

  if ( ( victim = ch->reply ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  if ( victim->desc == NULL && !IS_NPC(victim))
    {
      act("$N seems to have misplaced $S link...try again later.",ch,NULL,victim,TO_CHAR);
	  
	  if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
	{
		if (ch->availability == 1) {
			sprintf(buf,"{G%s{x tells you {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
		}
			else {
				sprintf(buf,"{r%s{x tells you {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
			}
	}
	
	else {
      sprintf(buf,"%s tells you {%c'%s'\n\r",PERS2(ch,victim),victim->colors[C_REPLY],argument);
	}
	
      buf[0] = UPPER(buf[0]);
      add_buf(victim->pcdata->buffer,buf);
      return;
    }

  if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
      act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
      return;
    }

  if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF)) 
  &&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim))
    {
      act_new( "$E is not receiving tells.", ch, 0, victim, TO_CHAR,POS_DEAD, 0);
      return;
    }

  if (!IS_IMMORTAL(victim) && !IS_AWAKE(ch))
    {
      send_to_char( "In your dreams, or what?\n\r", ch );
      return;
    }

  if((is_affected(ch,skill_lookup("devotion"))) &&
     (IS_IMMORTAL(victim)))
    {
		act("You are too busy pondering life to speak to the Gods.",
        ch,NULL,NULL,TO_CHAR);
		return;
    }
	
  if (IS_SET(victim->comm,COMM_AFK))
    {
	
		if (IS_NPC(victim))
		{
			act_new("$E is AFK, and not receiving tells.",
			ch,NULL,victim,TO_CHAR,POS_DEAD, 0);
			return;
		}
 
		act_new("$E is AFK, but your tell will go through when $E returns.",ch,NULL,victim,TO_CHAR,POS_DEAD, 0);
		
		if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
		{
			if (ch->availability == 1) {
			sprintf(buf,"{G%s{x tells you {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
			}
				else {
					sprintf(buf,"{r%s{x tells you {%c'%s'{x\n\r", ch->name, victim->colors[C_TELL],argument);
				}
		}
	
	else {
		sprintf(buf,"%s tells you {%c'%s'\n\r",PERS2(ch,victim),victim->colors[C_REPLY],argument);
	}
	
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
		return;
    }

  if( IS_SET( ch->shiftbits, SHIFT_HALF ) || IS_SET( ch->shiftbits, SHIFT_FULL ) )
    {
      were_speak = were_parse( argument );
    }

  if( were_speak )
    {
		if( IS_AFFECTED2( ch, AFF_DETECTWERE ) || IS_SET( ch->act_bits, PLR_HOLYLIGHT ) )
		{
			msg_to_char = argument;
		}
		
		else
		{
			msg_to_char = were_speak;
		}

		if( IS_AFFECTED2( victim, AFF_DETECTWERE ) || IS_SET( victim->act_bits, PLR_HOLYLIGHT ) )
		{
			msg_to_vict = argument;
		}
		
		else
		{
			msg_to_vict = were_speak;
		}

		sprintf( buf, "You reply to %s {%c'%s'{x\n\r",PERS2( victim, ch ),ch->colors[C_REPLY],msg_to_char );

		send_to_char(buf,ch);

		sprintf( buf, "%s replies {%c'%s'{x\n\r", PERS2(ch, victim),victim->colors[C_REPLY],msg_to_vict );

		buf[0] = UPPER(buf[0]);
		send_to_char(buf,victim);

		msg_to_char = NULL;
		msg_to_vict = NULL;
		free( were_speak );

      /* Were speech goes through unadulterated for reply */
		if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
		{
			mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );
		}

    }
	
  else
    {
	
		if (ch->race == race_lookup("seraph") && ch->life_energy < 60)
		{
			sprintf( buf, "You reply to a distant voice {%c'%s'{x\n\r",ch->colors[C_REPLY],argument );
		}
		
		else if (IS_IMMORTAL(victim) && (victim->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
		{
			if (victim->availability == 1) {
				sprintf(buf,"You reply to {G%s{x {%c'%s'{x\n\r", victim->name, ch->colors[C_REPLY],argument);
			}
				else {
				sprintf(buf,"You reply to {r%s{x {%c'%s'{x\n\r", victim->name, ch->colors[C_REPLY],argument);
				}
		}
		
		else 
		{
			sprintf( buf, "You reply to %s {%c'%s'{x\n\r",PERS2( victim, ch ),ch->colors[C_REPLY],argument ); 
		}
		
		send_to_char(buf,ch);
		
		if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
		{
			if (ch->availability == 1) {
				sprintf(buf,"{G%s{x replies {%c'%s'{x\n\r", ch->name, victim->colors[C_REPLY],argument);
			}
				else {
				sprintf(buf,"{r%s{x replies {%c'%s'{x\n\r", ch->name, victim->colors[C_REPLY],argument);
				}
		}
		
		else {
		sprintf( buf, "%s replies {%c'%s'{x\n\r", PERS2(ch, victim),victim->colors[C_REPLY],argument );
		}

		buf[0] = UPPER(buf[0]);
		send_to_char(buf,victim);
       
		if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
		{
			mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );
		}
		
    }
	
	victim->reply	= ch;
	
	if (!IS_NPC(ch) && IS_IMMORTAL(ch)) {
	 victim->immreply = ch;
	 }
	 
	return;
}

void do_immreply( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim = NULL;
  char buf[MAX_STRING_LENGTH] = "";

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
      send_to_char( "Your message didn't get through.\n\r", ch );
      return;
    }

  if ( ( victim = ch->immreply ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  if ( victim->desc == NULL && !IS_NPC(victim))
    {
      act("$N seems to have misplaced $S link...try again later.",ch,NULL,victim,TO_CHAR);
	  
	  if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
	{
		if (ch->availability == 1) {
			sprintf(buf,"{G%s{x tells you {%c'%s'{x\n\r", ch->name, victim->colors[C_REPLY],argument);
		}
			else {
				sprintf(buf,"{r%s{x tells you {%c'%s'{x\n\r", ch->name, victim->colors[C_REPLY],argument);
			}
	}
	
	else {
      sprintf(buf,"%s tells you {%c'%s'\n\r",PERS2(ch,victim),victim->colors[C_REPLY],argument);
	}
	
      buf[0] = UPPER(buf[0]);
      add_buf(victim->pcdata->buffer,buf);
      return;
    }

  if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
      act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
      return;
    }

  if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF)) 
  &&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim))
    {
      act_new( "$E is not receiving tells.", ch, 0, victim, TO_CHAR,POS_DEAD, 0);
      return;
    }

  if (!IS_IMMORTAL(victim) && !IS_AWAKE(ch))
    {
      send_to_char( "In your dreams, or what?\n\r", ch );
      return;
    }

  if((is_affected(ch,skill_lookup("devotion"))) &&
     (IS_IMMORTAL(victim)))
    {
		act("You are too busy pondering life to speak to the Gods.",
        ch,NULL,NULL,TO_CHAR);
		return;
    }
	
  if (IS_SET(victim->comm,COMM_AFK))
    {
	
		if (IS_NPC(victim))
		{
			act_new("$E is AFK, and not receiving tells.",
			ch,NULL,victim,TO_CHAR,POS_DEAD, 0);
			return;
		}
 
		act_new("$E is AFK, but your tell will go through when $E returns.",ch,NULL,victim,TO_CHAR,POS_DEAD, 0);
		
		if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || ch->incog_level >= LEVEL_HERO)) 
		{
			if (ch->availability == 1) {
			sprintf(buf,"{G%s{x tells you {%c'%s'{x\n\r", ch->name, victim->colors[C_REPLY],argument);
			}
				else {
					sprintf(buf,"{r%s{x tells you {%c'%s'{x\n\r", ch->name, victim->colors[C_REPLY],argument);
				}
		}
	
	else {
		sprintf(buf,"%s tells you {%c'%s'\n\r",PERS2(ch,victim),victim->colors[C_REPLY],argument);
	}
	
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
		return;
    }
	
  else
    {
	
		if (ch->race == race_lookup("seraph") && ch->life_energy < 60)
		{
			sprintf( buf, "You reply to a distant voice {%c'%s'{x\n\r",ch->colors[C_REPLY],argument );
		}
		
		else if (IS_IMMORTAL(victim) && (victim->invis_level >= LEVEL_HERO || victim->incog_level >= LEVEL_HERO)) 
		{
			if (victim->availability == 1) {
				sprintf(buf,"You reply to {G%s{x {%c'%s'{x\n\r", victim->name, ch->colors[C_REPLY],argument);
			}
				else {
				sprintf(buf,"You reply to {r%s{x {%c'%s'{x\n\r", victim->name, ch->colors[C_REPLY],argument);
				}
		}
		
		else 
		{
			sprintf( buf, "You reply to %s {%c'%s'{x\n\r",PERS2( victim, ch ),ch->colors[C_REPLY],argument ); 
		}
		send_to_char(buf,ch);
		
		if (IS_IMMORTAL(ch) && (ch->invis_level >= LEVEL_HERO || victim->incog_level >= LEVEL_HERO)) 
		{
			if (ch->availability == 1) {
				sprintf(buf,"{G%s{x replies {%c'%s'{x\n\r", ch->name, victim->colors[C_REPLY],argument);
			}
				else {
				sprintf(buf,"{r%s{x replies {%c'%s'{x\n\r", ch->name, victim->colors[C_REPLY],argument);
				}
		}
		
		else {
		sprintf( buf, "%s replies {%c'%s'{x\n\r", PERS2(ch, victim),victim->colors[C_REPLY],argument );
		}

		buf[0] = UPPER(buf[0]);
		send_to_char(buf,victim);
       
		if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
		{
			mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );
		}
		
    }
	
	victim->reply	= ch;
	
	if (!IS_NPC(ch) && IS_IMMORTAL(ch)) {
	 victim->immreply = ch;
	 }
	 
	return;
}

void do_yell( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    if (IS_SET(ch->shiftbits,SHIFT_HALF) || IS_SET(ch->shiftbits,SHIFT_FULL))
    {
       send_to_char("Your shouts cannot be understood in this beastial form\n\r",ch);
       return;
    }

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "You can't yell.\n\r", ch );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
	send_to_char( "Yell what?\n\r", ch );
	return;
    }

    sprintf(buf,"You yell {%c'%s'{x\n\r", ch->colors[C_YELL], argument);
    send_to_char(buf,ch);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   d->character->in_room != NULL
	&&   d->character->in_room->area == ch->in_room->area 
        &&   !IS_SET(d->character->comm,COMM_QUIET) )
	{
/*    act("$n yells '$t'",ch,argument,d->character,TO_VICT); */
            sprintf( buf, "%s yells {%c'%s'{x\n\r", 
                capitalize( PERS(ch, d->character)),
                d->character->colors[C_YELL],
                argument );
            send_to_char(buf,d->character);

	}
    }

    return;
}


void do_emote( CHAR_DATA *ch, char *argument )
{
    char *pname;

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

    pname = NULL;
    pname = strstr(argument,"+n");

    if (pname == NULL){
       MOBtrigger = FALSE;
       act( "$n $T", ch, NULL, argument, TO_ROOM );
       act( "$n $T", ch, NULL, argument, TO_CHAR );
       MOBtrigger = TRUE;
       }
    else {
       pname[0] = '$';
       MOBtrigger = FALSE;
       act( argument, ch, NULL, argument, TO_ROOM );
       act( argument, ch, NULL, argument, TO_CHAR );
       MOBtrigger = TRUE;
       }
    return;
}


void do_pmote( CHAR_DATA *ch, char *argument )
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
 
    act( "$n $t", ch, argument, NULL, TO_CHAR );

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->desc == NULL || vch == ch)
	    continue;

	if ((letter = strstr(argument,vch->name)) == NULL)
	{
	    MOBtrigger = FALSE;
	    act("$N $t",vch,argument,ch,TO_CHAR);
	    MOBtrigger = TRUE;
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

	MOBtrigger = FALSE;
	act("$N $t",vch,temp,ch,TO_CHAR);
	MOBtrigger = TRUE;
    }
	
    return;
}


/*
 * All the posing stuff.
 */
struct	pose_table_type
{
    char *	message[2*MAX_CLASS];
};

const	struct	pose_table_type	pose_table	[]	=
{
    {
	{
	    "You sizzle with energy.",
	    "$n sizzles with energy.",
	    "You feel very holy.",
	    "$n looks very holy.",
            "You focus your chi energy and are surrounded by a blue aura.",
            "$n is surrounded by a blue aura.",
	    "You show your bulging muscles.",
	    "$n shows $s bulging muscles."
             "You show your bulging muscles.",
	    "$n shows $s bulging muscles.",
            "You perform a small card trick.",
	    "$n performs a small card trick.",
	}
    },

    {
	{
	    "You turn into a butterfly, then return to your normal shape.",
	    "$n turns into a butterfly, then returns to $s normal shape.",
	    "You nonchalantly turn wine into water.",
	    "$n nonchalantly turns wine into water.",
            "You break a board with your forehead, OUCH!",
            "$n breaks a board with his forehead!",
	    "You crack nuts between your fingers.",
	    "$n cracks nuts between $s fingers.",
            "You crack nuts between your fingers.",
	    "$n cracks nuts between $s fingers.",
            "You wiggle your ears alternately.",
	    "$n wiggles $s ears alternately.",
	}
    },

    {
	{
	    "Blue sparks fly from your fingers.",
	    "Blue sparks fly from $n's fingers.",
	    "A halo appears over your head.",
	    "A halo appears over $n's head.",
            "You tell everyone the importance of meditation.",
            "$n begins to bore you again with $s meditation prattle.",
	    "You grizzle your teeth and look mean.",
	    "$n grizzles $s teeth and looks mean.",
	    "You grizzle your teeth and look mean.",
	    "$n grizzles $s teeth and looks mean.",
            "You nimbly tie yourself into a knot.",
	    "$n nimbly ties $mself into a knot.",
	}
    },

    {
	{
	    "Little red lights dance in your eyes.",
	    "Little red lights dance in $n's eyes.",
	    "You recite words of wisdom.",
	    "$n recites words of wisdom.",
            "You show everyone the deadly art of chopsticks.",
            "$n shows $s prowess with chopsticks on an innocent bowl of rice.",
	    "You hit your head, and your eyes roll.",
	    "$n hits $s head, and $s eyes roll.",
	    "You hit your head, and your eyes roll.",
	    "$n hits $s head, and $s eyes roll.",
	    "You juggle with daggers, apples, and eyeballs.",
	    "$n juggles with daggers, apples, and eyeballs.",
	}
    },

    {
	{
	    "A slimy green monster appears before you and bows.",
	    "A slimy green monster appears before $n and bows.",
	    "Deep in prayer, you levitate.",
	    "Deep in prayer, $n levitates.",
            "You fall asleep as you chant a hymn.",
            "$n falls asleep chanting a hymn.",
	    "Crunch, crunch -- you munch a bottle.",
	    "Crunch, crunch -- $n munches a bottle.",
	    "Crunch, crunch -- you munch a bottle.",
	    "Crunch, crunch -- $n munches a bottle.",
	    "You steal the underwear off every person in the room.",
	    "Your underwear is gone!  $n stole it!",
	}
    },

    {
	{
	    "You turn everybody into a little pink elephant.",
	    "You are turned into a little pink elephant by $n.",
	    "An angel consults you.",
	    "An angel consults $n.",
            "You rub your belly for luck",
            "$n rubs $s belly and chants 'Buddah Buddah Buddah...'",
	    "... 98, 99, 100 ... you do pushups.",
	    "... 98, 99, 100 ... $n does pushups.",
	    "... 98, 99, 100 ... you do pushups.",
	    "... 98, 99, 100 ... $n does pushups.",
	    "The dice roll ... and you win again.",
	    "The dice roll ... and $n wins again.",
	}
    },

    {
	{
	    "A small ball of light dances on your fingertips.",
	    "A small ball of light dances on $n's fingertips.",
	    "Your body glows with an unearthly light.",
	    "$n's body glows with an unearthly light.",
            "You bow your head in prayer.",
            "$n bows $s head in prayer.",
            "You beat Hercules in an armwrestling contest.",
            "$n beats Hercules in an armwrestling contest.",
            "You beat Hercules in an armwrestling contest.",
            "$n beats Hercules in an armwrestling contest.",
	    "You count the money in everyone's pockets.",
	    "Check your money, $n is counting it.",
	}
    },

    {
	{
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "A spot light hits you.",
	    "A spot light hits $n.",
            "Your roundhouse knocks over a tree....Oops.",
            "$n's roundhouse knocks over a tree.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders.",
	    "You balance a pocket knife on your tongue.",
	    "$n balances a pocket knife on your tongue.",
	}
    },

    {
	{
	    "The light flickers as you rap in magical languages.",
	    "The light flickers as $n raps in magical languages.",
	    "Everyone levitates as you pray.",
	    "You levitate as $n prays.",
            "Kaine himself teaches you one of his moves.",
            "Kaine appears before $n and teaches $m a maneuver.",
	    "Oomph!  You squeeze water out of a granite boulder.",
	    "Oomph!  $n squeezes water out of a granite boulder.",
	    "Oomph!  You squeeze water out of a granite boulder.",
	    "Oomph!  $n squeezes water out of a granite boulder.",
	    "You produce a coin from everyone's ear.",
	    "$n produces a coin from your ear.",
	}
    },

    {
	{
	    "Your head disappears.",
	    "$n's head disappears.",
	    "A cool breeze refreshes you.",
	    "A cool breeze refreshes $n.",
            "You teach everyone the Zen of Kung Fu.",
            "$n tries to teach you all about Kung Fu.",
	    "You pick your teeth with a spear.",
	    "$n picks $s teeth with a spear.",
	    "You pick your teeth with a spear.",
	    "$n picks $s teeth with a spear.",
	    "You step behind your shadow.",
	    "$n steps behind $s shadow.",
	}
    },

    {
	{
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "The sun pierces through the clouds to illuminate you.",
	    "The sun pierces through the clouds to illuminate $n.",
            "You teach everyone the Zen of Kung Fu.",
            "$n tries to teach you all about Kung Fu.",
	    "Everyone is swept off their foot by your hug.",
	    "You are swept off your feet by $n's hug.",
	    "Everyone is swept off their foot by your hug.",
	    "You are swept off your feet by $n's hug.",
	    "Your eyes dance with greed.",
	    "$n's eyes dance with greed.",
	}
    },

    {
	{
	    "The sky changes color to match your eyes.",
	    "The sky changes color to match $n's eyes.",
	    "The ocean parts before you.",
	    "The ocean parts before $n.",
            "You knock yourself out headbutting a brick.",
            "$n knocks $mself out headbutting a brick.",
	    "Your karate chop splits a tree.",
	    "$n's karate chop splits a tree.",
	    "Your karate chop splits a tree.",
	    "$n's karate chop splits a tree.",
	    "You deftly steal everyone's weapon.",
	    "$n deftly steals your weapon.",
	}
    },

    {
	{
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
            "You knock yourself out headbutting a brick.",
            "$n knocks $mself out headbutting a brick.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s mighty thews.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s mighty thews.",
	    "The Grey Mouser buys you a beer.",
	    "The Grey Mouser buys $n a beer.",
	}
    },

    {
	{
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The Burning Man speaks to you.",
	    "The Burning Man speaks to $n.",
            "Kaine himself teaches you one of his moves.",
            "Kaine appears before $n and teaches $m a maneuver.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown.",
	    "Everyone's pocket explodes with your fireworks.",
	    "Your pocket explodes with $n's fireworks.",
	}
    },

    {
	{
	    "Everyone's clothes are transparent, and you are laughing.",
	    "Your clothes are transparent, and $n is laughing.",
	    "An eye in a pyramid winks at you.",
	    "An eye in a pyramid winks at $n.",
            "Your roundhouse knocks over a tree....Oops.",
            "$n's roundhouse knocks over a tree.",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding.",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding.",
	    "Everyone discovers your dagger a centimeter from their eye.",
	    "You discover $n's dagger a centimeter from your eye.",
	}
    },

    {
	{
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
	    "Valentine Michael Smith offers you a glass of water.",
	    "Valentine Michael Smith offers $n a glass of water.",
            "You rub your belly for luck",
            "$n rubs $s belly and chants 'Buddah Buddah Buddah...'",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot.",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot.",
	    "Where did you go?",
	    "Where did $n go?",
	}
    },

    {
	{
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "The great god Micheal gives you a staff.",
	    "The great god Micheal gives $n a staff.",
            "You break a board with your forehead, OUCH!",
            "$n breaks a board with his forehead!",
	    "Atlas asks you to relieve him.",
	    "Atlas asks $n to relieve him.",
	    "Atlas asks you to relieve him.",
	    "Atlas asks $n to relieve him.",
	    "Click.",
	    "Click.",
	}
    }
};



void do_pose( CHAR_DATA *ch, char *argument )
{
    int level;
    int pose;

    if ( IS_NPC(ch) )
	return;

    level = UMIN( ch->level, sizeof(pose_table) / sizeof(pose_table[0]) - 1 );
    pose  = number_range(0, level);

    act( pose_table[pose].message[2*ch->class+0], ch, NULL, NULL, TO_CHAR );
    act( pose_table[pose].message[2*ch->class+1], ch, NULL, NULL, TO_ROOM );

    return;
}



void do_bug( CHAR_DATA *ch, char *argument )
{
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Bug logged.\n\r", ch );
    return;
}

void do_typo( CHAR_DATA *ch, char *argument )
{
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}

void do_rent( CHAR_DATA *ch, char *argument )
{
    send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
    return;
}


void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}



void do_quit( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA *d,*d_next;
/*char pueblo_buf[MAX_STRING_LENGTH]; */
  int id;
  bool force;

  if ( IS_NPC(ch) )
    return;

  if ( !str_cmp( argument, "quit" )
  ||   !str_cmp( argument, "all" )
  ||   !str_cmp( argument, "force" ) )
    force = TRUE;
  else
    force = FALSE;

  if ( ch->position == POS_FIGHTING )
    {
      send_to_char( "No way! You are fighting.\n\r", ch );
      return;
    }

  if ( ch->position == POS_SPRAWLED )
    {
      send_to_char( "No way! You are fighting.\n\r", ch );
      return;
    }

  if( ch->pnote != NULL && !force )
    {
      send_to_char( "You have a note in progress.\n\r", ch );
      return;
    }

    if ( IS_AFFECTED(ch, AFF_CURSE) && is_affected( ch, gsn_natures_pestil ) )
    {
      send_to_char( "You cannot escape nature's pestilence that easily!\n\r", 
          ch );
      return;
    }

/*
 *
    if ( ch->pcdata->brawling != NULL )
    {
        send_to_char( "You need to forfeit your brawl first.\n\r", ch );
        return;
    }
 *
 */

  if( (ch->position < POS_SLEEPING) && !IS_SET( ch->act_bits, PLR_LINKDEAD ) )
    {
      send_to_char( "You're not in a position to do that yet.\n\r", ch );
      return;
    }

  if( !IS_IMMORTAL( ch ) && in_fightlag(ch) )
    {
      send_to_char("You're still thinking about your last fight.  See help fightlag.\n\r",ch);
      return;
    }
/*
  if (IS_SET(ch->act_bits, PLR_PUEBLO)) {
    sprintf(pueblo_buf,"<img xch_sound=play xch_volume=%d src=\"%s%s\">"
	    "<img src=\"%s%s\">",ch->pcdata->volume,PUEBLO_DIR,PUEBLO_END,
	    PUEBLO_DIR, PUEBLO_E_IMG);
    send_to_char("</xch_mudtext><img xch_mode=html>",ch);
    send_to_char(pueblo_buf,ch);
    send_to_char("<br><img xch_mode=text>",ch);
  }
  else */
    /*
      send_to_char("    |                                                              |\n\r",ch);
      send_to_char("    |          {gThine adventure awaits thy return.{x                  |\n\r",ch);
      send_to_char("  __|                                                              |\n\r",ch);
      send_to_char(" |@@/--------------------------------------------------------------/\n\r",ch);
      send_to_char("  @/--------------------------------------------------------------/\n\r",ch);
    */

    do_help( ch, "logout" );

  act_new( "{g$n has left the realm of mortal men.{x", ch, NULL, NULL, 
      TO_ROOM, POS_SPRAWLED, ch->invis_level );
  sprintf( log_buf, "%s has quit.", ch->name );
  log_string( log_buf );
  REMOVE_BIT(ch->act_bits,PLR_QUEST);
  d = ch->desc;
  if (d != NULL) 
    {
      if( ch->pcdata->fake_ip[ 0 ] != '\0' )
	{

	  sprintf(log_buf,"%s@%s rejoins the real world.",ch->name,ch->pcdata->fake_ip);
	  wiznet(log_buf,ch,NULL,WIZ_LOGINS,0,ch->level);

	  sprintf(log_buf,"%s@%s (REAL IP) rejoins the real world.",ch->name,d->host);
	  wiznet(log_buf,ch,NULL,WIZ_LOGINS,0,MAX_LEVEL);
	}
      else
	{
	  sprintf(log_buf,"%s@%s rejoins the real world.",ch->name,d->host);
	  wiznet(log_buf,ch,NULL,WIZ_LOGINS,0,ch->level);
	}
    }
  else wiznet("$N rejoins the real world.",ch,NULL,WIZ_LOGINS,0,ch->level);

  /*
   * After extract_char the ch is no longer valid!
   */

  if( is_guild( ch ) )
    {
      ch->pcdata->guild_logs += 1;
    }

  save_char_obj( ch );
  id = ch->id;
  player_quitting = TRUE;
  extract_char( ch, TRUE );
  if ( d != NULL )
    close_socket( d );

  /* toast evil cheating bastards */
  for (d = descriptor_list; d != NULL; d = d_next)
    {
      CHAR_DATA *tch;

      d_next = d->next;
      tch = d->original ? d->original : d->character;
      if (tch && tch->id == id)
	{
	  extract_char(tch,TRUE);
	  close_socket(d);
	} 
    }

  player_quitting = FALSE;
  return;
}



void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    save_char_obj( ch );
    send_to_char("Saving. Remember that Dark Risings has automatic saving.\n\r", ch);
    WAIT_STATE(ch,4 * PULSE_VIOLENCE);
    return;
}



void do_follow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    bool extracted = FALSE;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->act_bits,PLR_NOFOLLOW) && !IS_IMMORTAL(ch))
    {
	act("$N doesn't seem to want any followers.\n\r",
             ch,NULL,victim, TO_CHAR);
        return;
    }

    REMOVE_BIT(ch->act_bits,PLR_NOFOLLOW);
    
    if ( ch->master != NULL )
	extracted = stop_follower( ch );

    if ( !extracted )
        add_follower( ch, victim );

    return;
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}

/*
 * stop_follower:  This makes ch stop following somone; ch should be
     pre-checked for having a master before this is called.  Any code 
     that follows a stop_follower call MUST make sure that ch still
     exists, because this function will extract any pets or multipets.

     To facilitate this check, this function will return TRUE if it
     extracted the character, FALSE if not.

 *   6/17/06 gkl
 */
bool stop_follower( CHAR_DATA *ch )
{
    AFFECT_DATA *paf;
    char buf[MAX_STRING_LENGTH];
    bool killpet = FALSE;
    bool altmsg = FALSE;
    bool death = FALSE;

    if ( ch->master == NULL )
    {
        sprintf(buf, "stop_follower: null master ch=%s",
            ch->name );
	bug( buf, 0 );
	return FALSE;
    }

    if ( IS_NPC( ch ) && IS_AFFECTED2( ch, AFF2_SUMMONEDPET ) )
	killpet = TRUE;

    if ( ch->hit < 1 )
        death = TRUE;

    if ( can_see( ch->master, ch ) && ch->in_room != NULL)
    {
/* *********************************** */
/* Bonus- summoned pets have customized departure messages if they are in
          the same room as their master.  Otherwise, get generic msg. */ 
        if ( IS_NPC( ch ) 
        &&   !death 
        &&   ( ch->master->pet == ch || IS_AFFECTED2(ch, AFF2_SUMMONEDPET) )
        &&   ch->master->in_room
        &&   ch->in_room == ch->master->in_room )
        {
/* Find an affect with charm (caused by the spell that summoned the critter)
   and look up its fade msg */
	    for ( paf = ch->affected; paf != NULL; paf = paf->next )
            {
                if ( paf->where == TO_AFFECTS
                &&   paf->whichaff == AFF1
                &&   paf->bitvector == AFF_CHARM
                &&   paf->type > 0
                &&   paf->type < MAX_SKILL
                &&   skill_table[paf->type].msg_off )
                {
                    altmsg = TRUE;
                    act( skill_table[paf->type].msg_off, ch, NULL, NULL, TO_ROOM );
                    break;
                }
                
            }
        }
/* *********************************** */
        if ( !altmsg )
            act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
    	act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
        REMOVE_BIT( ch->affected_by, AFF_CHARM );
        affect_strip( ch, gsn_charm_person );
        affect_strip( ch, gsn_domination );
        affect_strip( ch, gsn_entrance   );
        ch->master->charmies -=1;
    }

    if (ch->master->pet == ch)
    {
//      This happens when a pet fades naturally
//      sprintf(buf, "stop_follower: ch=%s, ch->master=%s", ch->name, ch->master->name );
//      bug( "stop_follower: a pet persists; please note this to admin!", 0 );
//      bug( buf, 0 );
        killpet = TRUE;
	ch->master->pet = NULL;
    }

    ch->master = NULL;
    ch->leader = NULL;

/*  Pet extraction is handled exclusively by this function! */ 
    if ( killpet )
    {
        if ( !altmsg && !death )
            act( "$n slowly fades away.", ch, NULL, NULL, TO_ROOM );
        extract_char( ch, TRUE );
        return TRUE;
    }
    else
        return FALSE;
}

/* nuke_pets:  Extracts ch's pet and makes any multipets stop
 *   following ch.  Primarily used when a character dies or
 *   logs out.
 */
void nuke_pets( CHAR_DATA *ch )
{    
  CHAR_DATA *pet, *next;

  if ((pet = ch->pet) != NULL)
    {
/*      if (pet->in_room != NULL)
	act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);*/
      stop_follower(pet);
    }
  ch->pet = NULL;

/* Handles multipets */
    for ( pet = char_list; pet != NULL; )
    {
       next = pet->next;
       if ( IS_AFFECTED2(pet,AFF2_SUMMONEDPET)
       &&   pet->master == ch )
           stop_follower(pet);
       pet = next;
    }

  return;
}

/*
 * die_follower:  Essentially removes all of ch's followers and ensures
     that ch isn't following anyone.  Called by do_nofollow and by
 *   extract_char, so this function should NEVER refer to extract_char
 */
void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    ch->leader = NULL;

    if ( ch->master != NULL )
    {
    	if (ch->master->pet == ch)
    	    ch->master->pet = NULL;
	stop_follower( ch );
    }

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );
    one_argument(argument,arg2);

    if (!str_cmp(arg2,"offe" ) || !str_cmp( arg2, "offer" ) ||
	!str_cmp(arg2,"joinbrawler") ||
	!str_cmp(arg2,"brawl") || !str_cmp(arg2, "brawle") || !str_cmp(arg2, "brawler") ||
	!str_cmp(arg2,"idea") /*|| !str_cmp(arg2,"lore")*/ ||
	!str_cmp(arg2,"delete") || !str_cmp(arg2,"mob") ||
        !str_prefix(arg2,"oo") || !str_prefix(arg2,"tit") ||
	!str_prefix(arg2,"sav") || !str_cmp(arg2,"de") ||
	!str_cmp(arg2,"des") || !str_prefix(arg2, "desc") ||
        !str_cmp(arg2,"with") || !str_prefix(arg2, "with") ||
        !str_cmp(arg2,"c port") || !str_prefix(arg2,"c port") ||
        !str_cmp(arg2,"c portal") || !str_prefix(arg2,"c portal") ||
        !str_cmp(arg2,"cast port") || !str_prefix(arg2,"cast port") ||
        !str_cmp(arg2,"cast portal") || !str_prefix(arg2,"cast portal") ||
        !str_cmp(arg2,"c gate") || !str_prefix(arg2,"c gate") ||
        !str_cmp(arg2,"cast gate") || !str_prefix(arg2,"cast gate") ||
        !str_cmp(arg2,"deposit") || !str_prefix(arg2, "deposit") ||
        !str_cmp(arg2,"account") || !str_prefix(arg2, "account") ||
        !str_cmp(arg2,"note") || !str_prefix(arg2, "note") ||
        !str_cmp(arg2,"desc") || !str_prefix(arg2, "description") ||
        !str_cmp(arg2,"pro") || !str_prefix(arg2, "prompt") ||
        !str_cmp(arg2,"his") || !str_prefix(arg2, "history") ||
        !str_cmp(arg2,"id") || !str_prefix(arg2, "idea") ||
        !str_cmp(arg2,"chang") || !str_prefix(arg2, "change") ||
        !str_cmp(arg2,"shi") || !str_prefix(arg2, "shift") ||
        !str_cmp(arg2,"sav") || !str_prefix(arg2, "save") ||
        !str_cmp(arg2,"withdraw") || !str_prefix(arg2, "withdraw") ||
	!str_cmp(arg2,"fea") || !str_prefix(arg2, "feast" ) ||
	!str_cmp(arg2,"pau") || !str_prefix(arg2, "pause" ) ||
	!str_cmp(arg2,"nos") || !str_prefix( arg2, "nosummon" ) )
        
    {
        send_to_char("There are some things even charm cannot make someone do.\n\r",ch);
        return;
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, arg, FALSE ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch 
	||  (IS_IMMORTAL(victim) && victim->trust >= ch->trust))
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;

	    if( !IS_NPC( och ) )
	      {
		if( IS_AFFECTED( och, AFF_SLEEP ) &&
		( ( strcasecmp( argument, "sleep" ) == 0 ) ||
		  ( strcasecmp( argument, "slee" ) == 0 ) ||
		  ( strcasecmp( argument, "sle" ) == 0 ) ||
		  ( strcasecmp( argument, "sl" ) == 0 ) ) )
		  {
		    send_to_char( "But they're not tired!\n\r", ch );
		    continue;
		  }

		if( strcasecmp( argument, "go gate" ) == 0 ||
		    strcasecmp( argument, "ente gate" ) == 0 ||
		    strcasecmp( argument, "enter gate" ) == 0 )
		  {
		    send_to_char( "Why would you send your follower away?\n\r", ch );
		    continue;
		  }
	      }

	    sprintf( buf, "$n orders you to '%s'.", argument );
	    act( buf, ch, NULL, och, TO_VICT );
	    interpret( och, argument );

	}
    }


    if ( found )
    {
	WAIT_STATE(ch,PULSE_VIOLENCE);
	send_to_char( "Ok.\n\r", ch );
    }
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}



void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf( buf, "%s's group:\n\r", PERS2(leader, ch) );
	send_to_char( buf, ch );

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		sprintf( buf,
		"[%2d %s] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d mv %5d xp\n\r",
		    gch->level,
		    IS_NPC(gch) ? "Mob" : class_table[gch->class].who_name,
		    capitalize( PERS2(gch, ch) ),
		    gch->hit,   gch->max_hit,
		    gch->mana,  gch->max_mana,
		    gch->move,  gch->max_move,
		    gch->exp    );
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg, FALSE ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act_new("$N isn't following you.",ch,NULL,victim,TO_CHAR,POS_SLEEPING, 0 );
	return;
    }
    
    if ( is_same_group( victim, ch ) && ch != victim )
    {
      if (IS_AFFECTED(victim,AFF_CHARM))
      {
        send_to_char(
          "You can't remove charmed members from your group.\n\r",ch);
      } 
      else if (IS_AFFECTED(ch,AFF_CHARM))
      {
        act_new("You like your master too much to leave $m!",
          ch,NULL,victim,TO_VICT,POS_SLEEPING, 0 );
      }
      else
      {
        victim->leader = NULL;
        act_new("$n removes $N from $s group.",
          ch,NULL,victim,TO_NOTVICT,POS_RESTING, 0 );
        act_new("$n removes you from $s group.",
          ch,NULL,victim,TO_VICT,POS_SLEEPING, 0 );
        act_new("You remove $N from your group.",
          ch,NULL,victim,TO_CHAR,POS_SLEEPING, 0 );
      }
    }
    else
    {
      victim->leader = ch;
      act_new("$N joins $n's group.",ch,NULL,victim,TO_NOTVICT,POS_RESTING, 0 );
      act_new("You join $n's group.",ch,NULL,victim,TO_VICT,POS_SLEEPING, 0 );
      act_new("$N joins your group.",ch,NULL,victim,TO_CHAR,POS_SLEEPING, 0 );
    }
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount_gold = 0, amount_silver = 0;
    int share_gold, share_silver;
    int extra_gold, extra_silver;

    argument = one_argument( argument, arg1 );
	       one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount_silver = atoi( arg1 );

    if (arg2[0] != '\0')
	amount_gold = atoi(arg2);

    if ( amount_gold < 0 || amount_silver < 0)
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount_gold == 0 && amount_silver == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->gold <  amount_gold || ch->silver < amount_silver)
    {
	send_to_char( "You don't have that much to split.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }
	    
    share_silver = amount_silver / members;
    extra_silver = amount_silver % members;

    share_gold   = amount_gold / members;
    extra_gold   = amount_gold % members;

    if ( share_gold == 0 && share_silver == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->silver	-= amount_silver;
    ch->silver	+= share_silver + extra_silver;
    ch->gold 	-= amount_gold;
    ch->gold 	+= share_gold + extra_gold;

    if (share_silver > 0)
    {
	sprintf(buf,
	    "You split %d silver coins. Your share is %d silver.\n\r",
 	    amount_silver,share_silver + extra_silver);
	send_to_char(buf,ch);
    }

    if (share_gold > 0)
    {
	sprintf(buf,
	    "You split %d gold coins. Your share is %d gold.\n\r",
	     amount_gold,share_gold + extra_gold);
	send_to_char(buf,ch);
    }

    if (share_gold == 0)
    {
	sprintf(buf,"$n splits %d silver coins. Your share is %d silver.",
		amount_silver,share_silver);
    }
    else if (share_silver == 0)
    {
	sprintf(buf,"$n splits %d gold coins. Your share is %d gold.",
		amount_gold,share_gold);
    }
    else
    {
	sprintf(buf,
"$n splits %d silver and %d gold coins, giving you %d silver and %d gold.\n\r",
	 amount_silver,amount_gold,share_silver,share_gold);
    }

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
	{
	    act( buf, ch, NULL, gch, TO_VICT );
	    gch->gold += share_gold;
	    gch->silver += share_silver;

            if ( (share_gold + share_silver/100) > MAX_MONEY_TRANSFER )
            {
                sprintf(buf, "%s split %d gold and %d silver to %s.",
                    IS_NPC( ch ) ? ch->short_descr : ch->name, 
                    share_gold, share_silver,
                    IS_NPC( gch ) ? gch->short_descr : gch->name );
                wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
                log_string( buf );
            }
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if (IS_SET(ch->shiftbits,SHIFT_HALF) || IS_SET(ch->shiftbits,SHIFT_FULL))
    {
       send_to_char("The members of your group cannot understand your gutteral speech!\n\r",ch);
       return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

    sprintf(buf,"You tell the group {%c'%s'{x\n\r",
	ch->colors[C_GT],argument);
    send_to_char(buf,ch);

    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
      if (gch != ch)
	if ( is_same_group( gch, ch ) ){
           sprintf( buf, "%s tells the group {%c'%s'{x\n\r", 
            capitalize( PERS2(ch, gch)),
            gch->colors[C_GT],
            argument);
           send_to_char(buf,gch);
           }
    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach == NULL || bch == NULL)
	return FALSE;

    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}

/*
 * Colour code - adapted from Lope, April 2012 gkl
 *
 * OOC     IC      GT      EMOTE   TELL
 * REPLY   GDT     SAY     HP      MAXHP
 * MANA    MAXMANA MOVES   MAXMOVES IMM
 * YBATTLE TBATTLE MINION  AUCTION ADMIN
 * WIZNET  VSAY    BRAWLER QSAY    QTSAY
 */
const struct color_settings colordefs[MAX_COLOR] = 
{
    { "ooc",      'B'   },   /*  0 */
    { "ic",       'm'   },   /*  1 */
    { "gtell",    'R'   },   /*  2 */
    { "nb",       'G'   },   /*  3 */
    { "tell",     'g'   },   /*  4 */
    { "reply",    'c'   },   /*  5 */
    { "gdt",      'C'   },   /*  6 */
    { "say",      'y'   },   /*  7 */
/*  { "hp",       'y',  },   old 8 */
    { "shout",    'M'   },   /*  8 */
/*  { "maxhp",    'c',  },   old 9 */
    { "yell",     'Y'   },   /*  9 */
    { "mana",     'c'   },   /* 10 */
    { "maxmana",  'r'   },   /* 11 */
    { "moves",    'r'   },   /* 12 */
    { "maxmoves", 'C'   },   /* 13 */
    { "imm",      'y'   },   /* 14 */
    { "battle",   'g'   },   /* 15 */
    { "opponent", 'g'   },   /* 16 */
    { "minion",   'r'   },   /* 17 */
    { "auction",  'm'   },   /* 18 */
    { "admin",    'm'   },   /* 19 */
    { "wiznet",   'y'   },   /* 20 */
    { "vsay",     'y'   },   /* 21 */
    { "brawler",  'D'   },   /* 22 */
    { "qsay",     'c'   },   /* 23 */
    { "qtsay",    'c'   },   /* 24 */
    { "ct",       'D'   }    /* 25 */
};

void show_color_options( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char const fmtline[] = { "   {%c%-12s{%c%-12s{%c%-12s{%c%-10s{x%s\n\r" };
    bool found = FALSE;

    if (IS_DRMODE(ch)) 
    {
        sprintf( buf, "%s%44s\n\r%s", 
            BIG_BLUE_LINE, "Colour Show",BIG_BLUE_LINE);
        send_to_char( buf, ch );
    }

    send_to_char("The following fields may have their colours changed:\n\r\n\r",
        ch);

/* standard options that everyone gets */
    sprintf( buf, fmtline,
        ch->colors[C_OOC],      colordefs[C_OOC].field, 
        ch->colors[C_IC],       colordefs[C_IC].field,
        ch->colors[C_AUCTION],  colordefs[C_AUCTION].field,
        ch->colors[C_NB],       colordefs[C_NB].field,
        "<--- Channel communication" );
    send_to_char( buf, ch );
    sprintf( buf, fmtline,
        ch->colors[C_SAY],      colordefs[C_SAY].field,
        ch->colors[C_TELL],     colordefs[C_TELL].field,
        ch->colors[C_REPLY],    colordefs[C_REPLY].field,
        ch->colors[C_GT],       colordefs[C_GT].field ,
        "<--- Private communication" );
    send_to_char( buf, ch );
    sprintf( buf, fmtline,
        ch->colors[C_SHOUT],    colordefs[C_SHOUT].field,
        ch->colors[C_YELL],     colordefs[C_YELL].field,
        'x',                    "",
        'x',                    "",
        "<--- Mass communication" );
    send_to_char( buf, ch );
    sprintf( buf, fmtline,
        ch->colors[C_YBATTLE],  colordefs[C_YBATTLE].field,
        ch->colors[C_TBATTLE],  colordefs[C_TBATTLE].field,
        ch->colors[C_QSAY],     colordefs[C_QSAY].field,
        ch->colors[C_QTSAY],    colordefs[C_QTSAY].field,
        "<--- Combat and quests" );
    send_to_char( buf, ch );

/* build a special line for vampires, brawlers, guildies */
    if ( is_guild( ch ) )
    {
        sprintf( buf, "%s{%c%-12s", ( !found ? "   " : "" ),
            ch->colors[C_GDT], colordefs[C_GDT].field );
        send_to_char( buf, ch );
        found = TRUE;
    }
    if ( IS_BRAWLER( ch ) )
    {
        sprintf( buf, "%s{%c%-12s", ( !found ? "   " : "" ),
            ch->colors[C_BRAWLER], colordefs[C_BRAWLER].field );
        send_to_char( buf, ch );
        found = TRUE;
    }
    if ( IS_VAMPIRE( ch ) )
    {
        sprintf( buf, "%s{%c%-12s{%c%-12s", ( !found ? "   " : "" ),
            ch->colors[C_MINION], colordefs[C_MINION].field,
            ch->colors[C_VSAY], colordefs[C_VSAY].field );
        send_to_char( buf, ch );
        found = TRUE;
    }
    if ( !IS_NPC( ch ) && ch->pcdata->pcClan != NULL )
    {
        sprintf( buf, "%s{%c%-12s", ( !found ? "   " : "" ),
            ch->colors[C_CLANTALK], colordefs[C_CLANTALK].field );
        send_to_char( buf, ch );
        found = TRUE;
    }


    if ( found ) send_to_char( "\n\r", ch );

/* settings for imms only */
    if ( IS_IMMORTAL(ch) )
    {
        sprintf( buf, fmtline,
            ch->colors[C_IMM],      colordefs[C_IMM].field,
            ch->colors[C_ADMIN],    colordefs[C_ADMIN].field,
            ch->colors[C_WIZNET],   colordefs[C_WIZNET].field,
            'x',                    "",
            "" );
        send_to_char( buf, ch );
    }

    send_to_char( "\n\rAvailable colours:\n\r{c(c)yan {y(y)ellow {m(m)agenta {g(g)reen {b(b)lue {r(r)ed {w(w)hite{x\n\rAvailable bold colours:\n\r{C(C)yan {Y(Y)ellow {M(M)agenta {G(G)reen {B(B)lue {R(R)ed {W(W)hite {D(D)ark{x\n\r", ch );
    if (IS_DRMODE(ch)) 
    {
        send_to_char( BIG_BLUE_LINE, ch );
        send_to_char( "See Also: {BColour{x, {BColour Show{x\n\r", ch );
    }
    return;
}

void do_colour( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *arg2;
    int i;

    if ( IS_NPC(ch) )
    {
        send_to_char( "NPCs can't do that.\n\r", ch );
        return;
    }
    arg2 = one_argument( argument, arg );

/* toggle color on/off */
    if ( IS_NULLSTR(arg) )
    {
        if ( !IS_SET( ch->act_bits, PLR_COLOUR ) )
        {
            SET_BIT( ch->act_bits, PLR_COLOUR );
            send_to_char( "Colour is now {Ren{Yabl{Ged{x.\n\r", ch );
        }
        else
        {
            REMOVE_BIT( ch->act_bits, PLR_COLOUR );
            send_to_char_bw( "Colour is now disabled.\n\r", ch );
        }
        return;
    }

/* show valid color fields */
    if ( !str_cmp( arg, "show" ) )
    {
        show_color_options( ch );
        return;
    }

/* show valid colors */
    if ( IS_NULLSTR( arg2 )
    ||  !strchr("cymgbrwxCYMGBRWXD", arg2[0]) )
    {
        send_to_char( "Available colours:\n\r{c(c)yan {y(y)ellow {m(m)agenta {g(g)reen {b(b)lue {r(r)ed {w(w)hite{x\n\rAvailable bold colours:\n\r{C(C)yan {Y(Y)ellow {M(M)agenta {G(G)reen {B(B)lue {R(R)ed {W(W)hite {D(D)ark{x\n\r", ch );
	return;
    }

/* set color */
    for ( i = 0; i < MAX_COLOR; i++ )
    {
        if ( !str_cmp( arg, colordefs[i].field ) )
        {
            char buf[MAX_STRING_LENGTH];
            ch->colors[i] = arg2[0];
            sprintf( buf, "Colour for {%c%s{x set.\n\r", 
                ch->colors[i], colordefs[i].field );
            send_to_char( buf, ch );
            break;
        }
    }
    if ( i == MAX_COLOR )
    {
       send_to_char(
   "No fields by that name.  Use 'colour show' to see valid colour fields.\n\r",
           ch);
    }
    return;
}

/* 
 * legacy command -- in for posterity
 *
void do_colour( CHAR_DATA *ch, char *argument )
{
  char 	arg[ MAX_STRING_LENGTH ];
  char 	*arg2;
  char	colorline[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    return;

    arg2 = one_argument( argument, arg );

    if( !*arg )
      {
	if( !IS_SET( ch->act_bits, PLR_COLOUR ) )
	  {
	    SET_BIT( ch->act_bits, PLR_COLOUR );
	    send_to_char( "Color is now {Ren{Yabl{Ged{x.\n\r", ch );
	  }
	else
	  {
	    send_to_char_bw( "Color is now disabled.\n\r", ch );
	    REMOVE_BIT( ch->act_bits, PLR_COLOUR );
	  }

	return;
      }
    else if (!strcasecmp(arg,"show"))
      {
	sprintf(colorline,"{%c%-10s  {%c%-10s  {%c%-10s  {%c%-10s  {%c%-10s  {%c%-10s{x\n\r",
		ch->colors[C_OOC], "OOC", 
		ch->colors[C_IC], "IC", 
		ch->colors[C_GT], "GT", 
		ch->colors[C_EMOTE],"nothing0",
		ch->colors[C_TELL], "tell",
		ch->colors[C_REPLY], "reply");

	send_to_char(colorline,ch);

	sprintf(colorline,"{%c%-10s  {%c%-10s  {%c%-10s  {%c%-10s  {%c%-10s  {%c%-10s{x\n\r",
		ch->colors[C_GDT], "gdt", 
		ch->colors[C_SAY],  "say",
		ch->colors[C_HP],  "nothing1",
		ch->colors[C_MAXHP], "nothing2",
		ch->colors[C_MANA], "nothing3",
		ch->colors[C_MAXMANA], "nothing4");
	
	send_to_char(colorline,ch);
    
	sprintf(colorline,"{%c%-10s  {%c%-10s  {%c%-10s  {%c%-10s  {%c%-10s  {%c%-10s{x\n\r",
		ch->colors[C_MOVES], "nothing5",
		ch->colors[C_MAXMOVES], "nothing6",
		ch->colors[C_IMM], "imm", 
		ch->colors[C_YBATTLE],"battle",
		ch->colors[C_MINION], "minion",
		ch->colors[C_TBATTLE],"opponent");
	
	send_to_char(colorline,ch);
    
	sprintf(colorline,"{%c%-10s  {%c%-10s  {%c%-10s  {%c%-10s  {%c%-10s  {%c%-10s{x\n\r",
		ch->colors[C_AUCTION], "auction",
		ch->colors[C_ADMIN], "admin",
		ch->colors[C_WIZNET],"wiznet",
		ch->colors[C_BRAWLER],"Brawl",
		ch->colors[C_VSAY],"vsay",
                ch->colors[C_QSAY],"qsay");
	
	send_to_char(colorline,ch);

	sprintf(colorline,"{%c%-10s{x\n\r",
                ch->colors[C_QTSAY],"qtsay");
	
	send_to_char(colorline,ch);
    
    
	return;
      }
    else if( !*arg2 ) 
      {
        sprintf(colorline,
		"Available Regular Colors \n\r{c(c)yan {y(y)ellow {m(m)agenta {g(g)reen {b(b)lue {r(r)ed {w(w)hite{x\n\r");
	send_to_char(colorline,ch);
    
	sprintf(colorline,
		"Available Bold Colors \n\r{C(C)yan {Y(Y)ellow {M(M)agenta {G(G)reen {B(B)lue {R(R)ed {W(W)hite {D(D)ark{x\n\r");
	send_to_char(colorline,ch);
	return;
          
      }
    else if (!strchr("cymgbrwxCYMGBRWXD",arg2[0]))
      {
	sprintf(colorline,
		"Available Colors \n\r{c(c)yan {y(y)ellow {m(m)agenta {g(g)reen {b(b)lue {r(r)ed {w(w)hite{x\n\r");
	send_to_char(colorline,ch);
	
	sprintf(colorline,
		"Available Bold Colors \n\r{C(C)yan {Y(Y)ellow {M(M)agenta {G(G)reen {B(B)lue {R(R)ed {W(W)hite {D(D)ark{x\n\r");
	send_to_char(colorline,ch);
	return;
      }
    else if (!strcasecmp(arg,"admin")){
          ch->colors[C_ADMIN] = arg2[0];
          }
    else if (!strcasecmp(arg,"ooc")){
          ch->colors[C_OOC] = arg2[0];
          }
    else if (!strcasecmp(arg,"ic")){
          ch->colors[C_IC] = arg2[0];
          }
    else if (!strcasecmp(arg,"brawl")){
      ch->colors[C_BRAWLER] = arg2[0];
    }
    else if (!strcasecmp(arg,"gt")){
          ch->colors[C_GT] = arg2[0];
          }
    else if (!strcasecmp(arg,"tell")){
          ch->colors[C_TELL] = arg2[0];
          }
    else if (!strcasecmp(arg,"reply")){
          ch->colors[C_REPLY] = arg2[0];
          }
    else if (!strcasecmp(arg,"gdt")){
          ch->colors[C_GDT] = arg2[0];
          }
    else if (!strcasecmp(arg,"say")){
          ch->colors[C_SAY] = arg2[0];
          }
    else if (!strcasecmp(arg,"hp")){
          ch->colors[C_HP] = arg2[0];
          } 
    else if (!strcasecmp(arg,"emote")){
          ch->colors[C_EMOTE] = arg2[0];
          }
    else if (!strcasecmp(arg,"maxhp")){
          ch->colors[C_MAXHP] = arg2[0];
          }
    else if (!strcasecmp(arg,"mana")){
          ch->colors[C_MANA] = arg2[0];
          }
    else if (!strcasecmp(arg,"maxmana")){
          ch->colors[C_MAXMANA] = arg2[0];
          }
    else if (!strcasecmp(arg,"move")){
          ch->colors[C_MOVES] = arg2[0];
          }
    else if (!strcasecmp(arg,"maxmove")){
          ch->colors[C_MAXMOVES] = arg2[0];
          }
    else if (!strcasecmp(arg,"imm")){
          ch->colors[C_IMM] = arg2[0];
          }
    else if (!strcasecmp(arg,"battle")){
          ch->colors[C_YBATTLE] = arg2[0];
          }
    else if (!strcasecmp(arg,"opponent")){
          ch->colors[C_TBATTLE] = arg2[0];
          }
    else if (!strcasecmp(arg,"minion")){
          ch->colors[C_MINION] = arg2[0];
          }
    else if (!strcasecmp(arg,"auction")){
          ch->colors[C_AUCTION] = arg2[0];
          }
    else if (!strcasecmp(arg,"admin")){
          ch->colors[C_ADMIN] = arg2[0];
          }
    else if (!strcasecmp(arg,"wiznet")){
          ch->colors[C_WIZNET] = arg2[0];
          }
    else if (!strcasecmp(arg,"vsay")){
          ch->colors[C_VSAY] = arg2[0];
          }
    else if (!strcasecmp(arg,"qsay"))
        ch->colors[C_QSAY] = arg2[0];
    else if (!strcasecmp(arg, "qtsay"))
        ch->colors[C_QTSAY] = arg2[0];
    else {
       send_to_char("No argument by that name.\n\r",ch);
       return;
       }

     send_to_char("Color set.\n\r",ch);
    return;
}
*/

void do_togauction( CHAR_DATA *ch, char *argument )
{
 
    if (IS_SET(ch->comm,COMM_NOAUCTION))
      {
        send_to_char("auction channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOAUCTION);
      }
    else
      {
        send_to_char("auction channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOAUCTION);
      }
}

void do_gmote( CHAR_DATA *ch, char *argument )
{
  char *pname;
  CHAR_DATA *gch;

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

  pname = NULL;
  pname = strstr(argument,"+n");

  if (pname == NULL){
    act_new( "$n $t", ch, argument, NULL,  TO_CHAR, POS_SLEEPING, 0 );
  }
  else {
    pname[0] = '$';
    act_new( argument, ch, NULL, NULL, TO_CHAR, POS_SLEEPING, 0 );
  }

  for (gch = char_list; gch != NULL; gch = gch->next){
    if (is_same_group(gch, ch)){
      if (pname == NULL){
	act_new( "$n $t", ch, argument, gch,  TO_VICT, POS_SLEEPING, 0 );
      }
      else {
	pname[0] = '$';
	act_new( argument, ch, NULL, gch, TO_VICT, POS_SLEEPING, 0 );
      }
    }
  }
  return;
}


int get_quest_cmd_index( const char* name )
{
  int i = 0;
  int match = -1;
  bool done = FALSE;

  while( !done )
    {
      if( quest_cmd_tbl[ i ].name == NULL )
	{
	  done = TRUE;
	}
      else if( strcasecmp( name, quest_cmd_tbl[ i ].name ) == 0 )
	{
	  done = TRUE;
	  match = i;
	}
      else
	{
	  i++;
	}
    }

  return match;
}



void quest_bit_name( int flags, char* buffer )
{
  int i;

  for( i = 0; quest_flags[ i ].name != NULL; i++ )
    {
      if( flags & quest_flags[ i ].bit )
	{
	  strcat( buffer, quest_flags[ i ].name );
	  strcat( buffer, " " );
	}
    }

  i = strlen( buffer );
  buffer[ i ] = '\0';

}



void check_quest_score( CHAR_DATA* ch, CHAR_DATA* victim, int damage, int damtype )
{
  CHAR_DATA* captain = NULL;

  if( !IS_SET( queststatus, QUEST_INPROGRESS ) )
    return;
    
  if( IS_NPC( ch ) )
    return;

  if( !IS_SET( ch->act_bits, PLR_QUEST ) )
    return;

  if( IS_SET( queststatus, QUEST_NO_SCORING ) )
    return;

  if( IS_SET( queststatus, QUEST_PC_DAMAGE_ONLY ) && IS_NPC( victim ) )
    return;

  if( IS_SET( queststatus, QUEST_MOB_DAMAGE_ONLY ) && !IS_NPC( victim ) )
    return;

  if( IS_NPC( victim ) && global_quest_info.questmob_vnum != 0 )
    {
      if( global_quest_info.questmob_vnum != victim->pIndexData->vnum )
	return;
    }

  if( ( damtype == DAM_NONE ) ||
      ( damtype == DAM_BASH ) ||
      ( damtype == DAM_PIERCE ) ||
      ( damtype == DAM_SLASH ) ||
      ( damtype == DAM_OTHER ) ||
      ( damtype == DAM_SILVER ) ||
      ( damtype == DAM_WOOD ) ||
      ( damtype == DAM_IRON ) )
    {
      if( IS_SET( queststatus, QUEST_NO_MELEE_SCORE ) )
	{
	  return;
	}
    }
  else 
    {
      if( IS_SET( queststatus, QUEST_NO_MAGIC_SCORE ) )
	{
	  return;
	}
    }
    

  ch->pcdata->questscore += damage;

  captain = get_char_world( ch, ch->pcdata->quest_team_captain, FALSE );

  if( captain != NULL )
    {
      if( !IS_NPC( captain ) )
	captain->pcdata->quest_team_score += damage;
    }
  else
    {
      if( !IS_NPC( ch ) )
	ch->pcdata->quest_team_score += damage;
    }
  
}





void quest_join( CHAR_DATA* ch, char* argument )
{
  char wiznet_msg[ MAX_STRING_LENGTH ] = "";

  if( IS_NPC( ch ) )
    {
      send_to_char( "NPCs can't join quests.\n\r", ch );
      return;
    }

  if( !IS_SET( queststatus, QUEST_INPROGRESS ) )
    {
      send_to_char( "There is no active quest at the moment.\n\r", ch );
      return;
    }

  if( !IS_SET( queststatus, QUEST_OPEN ) )
    {
      send_to_char( "The quest is closed.\n\r", ch );
      return;
    }

  if( !IS_IMMORTAL( ch ) &&
      ( ch->level > global_quest_info.max_level ||
	ch->level < global_quest_info.min_level ) )
    {
      printf_to_char( ch, 
		      "This quest is restricted to levels %d and %d.\n\r",
		      global_quest_info.min_level,
		      global_quest_info.max_level );
      return;
    }

  if( IS_SET( ch->act_bits, PLR_QUEST ) )
    {
      send_to_char( "You're already in the quest.\n\r", ch );
      return;
    }

  send_to_char( "Welcome to the QUEST!\n\r", ch );
  SET_BIT( ch->act_bits, PLR_QUEST );
  ch->pcdata->questscore = 0;

  sprintf( wiznet_msg, "%s has joined the quest.", ch->name );
  wiznet( wiznet_msg, ch, NULL, WIZ_QUEST, 0, 0 );

}



void quest_leave( CHAR_DATA* ch, char* argument )
{
  char wiznet_msg[ MAX_STRING_LENGTH ] = "";

  if( IS_NPC( ch ) )
    {
      send_to_char( "NPCs can't be in quests.\n\r", ch );
      return;
    }

  if( IS_SET( ch->act_bits, PLR_QUEST ) )
    {
      REMOVE_BIT( ch->act_bits, PLR_QUEST );
      printf_to_char( ch, "You have left the QUEST with a score of %d.\n\r", ch->pcdata->questscore );
      if ( ch->pcdata->questscore != 0 )
        sprintf( wiznet_msg, "%s left the QUEST with a score of %d.", ch->name, ch->pcdata->questscore );
      else
        sprintf( wiznet_msg, "%s left the QUEST.", ch->name );
      wiznet( wiznet_msg, ch, NULL, WIZ_QUEST, 0, 0 );
    }
  else
    {
      send_to_char( "You're not in a quest right now.\n\r", ch );
    }
}



void quest_status( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH] = "";
    char qbits[MAX_STRING_LENGTH];

    if ( !IS_NULLSTR( global_quest_info.name ) )
    {
      if ( !IS_NPC( ch ) )
      {
        sprintf(buf, "-- {%c%s{x --\n\r",
          ch->colors[C_QSAY], global_quest_info.name );
        send_to_char( buf, ch );
      }
      else
      {
        send_to_char( "-- ", ch );
        send_to_char( global_quest_info.name, ch );
        send_to_char( " --\n\r", ch );
      }
    }

/* New "quest status" command */
    qbits[0] = '\0';
    if ( IS_SET( queststatus, QUEST_INPROGRESS ) ) strcat(qbits, "'in progress' " );
    if ( IS_SET( queststatus, QUEST_OPEN )       ) strcat(qbits, "open " );
    if ( !IS_NULLSTR( qbits ) )
    {
        sprintf( buf, "The quest is currently %s.\n\r", listize(qbits,TRUE) );
        send_to_char( buf, ch );
    }

    qbits[0] = '\0';
    if ( IS_SET( queststatus, QUEST_ALWAYS_KO  ) ) strcat(qbits, "mobs " );
    if ( IS_SET( queststatus, QUEST_NO_MURDER  ) ) strcat(qbits, "players " );
    if ( !IS_NULLSTR( qbits ) )
    {
        sprintf( buf, "No risk of death by %s.\n\r", listize(qbits,FALSE) );
        send_to_char( buf, ch );
    }
    else
        send_to_char( "Risk of death by mobs and players.\n\r", ch );

    qbits[0] = '\0';
    if ( IS_SET( queststatus, QUEST_NO_QUESTROT) ) strcat(qbits, "'knocked out' " );
    if ( IS_SET( queststatus, QUEST_NO_LOOT    ) ) strcat(qbits, "looted " );
    if ( IS_SET( queststatus, QUEST_NO_DEATHROT) ) strcat(qbits, "killed " );
    if ( !IS_NULLSTR( qbits ) )
    {
        sprintf( buf, "No risk of item loss due to being %s.\n\r", listize(qbits,FALSE) );
        send_to_char( buf, ch );
    }
    else
        send_to_char( "Risk of item loss due to being knocked out, looted, and killed.\n\r", ch );

    if ( IS_SET( queststatus, QUEST_NO_SCORING ) )
    {
        send_to_char( "All quest points awarded by moderator.\n\r", ch );
    }
    else
    {
        qbits[0] = '\0';
        if ( IS_SET( queststatus, QUEST_NO_MAGIC_SCORE) ) strcat(qbits, "magic " );
        if ( IS_SET( queststatus, QUEST_NO_MELEE_SCORE) ) strcat(qbits, "melee " );
        if ( IS_SET( queststatus, QUEST_NO_HEALING_SCORE) ) strcat(qbits, "healing " );
        if ( IS_SET( queststatus, QUEST_PC_DAMAGE_ONLY) ) strcat(qbits, "'fighting mobs' " );
        if ( IS_SET( queststatus, QUEST_MOB_DAMAGE_ONLY) ) strcat(qbits, "'fighting players' " );
        if ( !IS_NULLSTR( qbits ) )
        {
            sprintf( buf, "No quest points for %s.\n\r", listize(qbits,FALSE) );
            send_to_char( buf, ch );
        }
    }

    if ( global_quest_info.min_level != 1 || global_quest_info.max_level != 50 )
        printf_to_char( ch, "Quest is restricted to players between levels %d and %d.\n\r", 
            global_quest_info.min_level, global_quest_info.max_level );

    if( IS_IMMORTAL( ch ) )
    {
        buf[0]='\0';
        quest_bit_name( queststatus, buf );
        printf_to_char( ch, "Quest settings: %s\n\r", buf );
        printf_to_char( ch, "Quest mob vnum: %d\n\r", global_quest_info.questmob_vnum );
    }
}



void quest_score( CHAR_DATA* ch, char* argument )
{
  char score_entry[ MAX_STRING_LENGTH ] = "";
  DESCRIPTOR_DATA* d;
  char leaders[MAX_STRING_LENGTH] = "";
  int lead_score = 0, nlead = 0;

  if ( !IS_NULLSTR( global_quest_info.name ) )
  {
    if ( !IS_NPC( ch ) )
    {
      sprintf(score_entry, "Score for {%c%s{x:\n\r",
        ch->colors[C_QSAY], global_quest_info.name );
      send_to_char( score_entry, ch );
    }
    else
    {
      send_to_char( "Score for ", ch );
      send_to_char( global_quest_info.name, ch );
      send_to_char( ":\n\r", ch );
    }
  }
  else
    send_to_char( "Quest score:\n\r", ch );

  for( d = descriptor_list; d != NULL; d = d->next )
    {
      if( d->connected == CON_PLAYING
      && !IS_NPC( d->character )
      &&  IS_SET( d->character->act_bits, PLR_QUEST ) )
	{
	  sprintf( score_entry,
		   "%20s:{w%d{x\n\r",
		   d->character->name, d->character->pcdata->questscore );

	  send_to_char( score_entry, ch );

	  if( d->character->pcdata->questscore > lead_score )
          {
              lead_score = d->character->pcdata->questscore;
          }
	}
    }
/*  Loop through to get all the leaders */
    for( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING
        &&  !IS_NPC( d->character )
        &&   IS_SET( d->character->act_bits, PLR_QUEST ) )
	{
	    if ( d->character->pcdata->questscore == lead_score )
            {
                nlead++;
                strcat( leaders, d->character->name );
                strcat( leaders, " " );
            }
	}
    }

    if ( !IS_NULLSTR(leaders) )
    {
      sprintf( score_entry, "\n\r{YLeader%s: %s (%d){x\n\r", 
          (nlead > 1 ) ? "s" : "", listize(leaders,TRUE), lead_score );
      send_to_char( score_entry, ch );
    }
}


void quest_teamscore( CHAR_DATA* ch, char* argument )
{
  char score_entry[ MAX_STRING_LENGTH ] = "";
  DESCRIPTOR_DATA* d;
  char* lead_name = NULL;
  int lead_score = 0;

  send_to_char( "QUEST TEAM SCORE:\n\r", ch );

  for( d = descriptor_list; d != NULL; d = d->next )
    {
      if( d->connected == CON_PLAYING
      && !IS_NPC( d->character )
      &&  IS_SET( d->character->act_bits, PLR_QUEST )
      &&  strlen( d->character->pcdata->quest_team_captain ) == 0 )
	{
	  sprintf( score_entry,
		   "%20s:{w%ld{x\n\r",
		   d->character->name, d->character->pcdata->quest_team_score );

	  send_to_char( score_entry, ch );

	  if( d->character->pcdata->quest_team_score > lead_score )
	    {
	      lead_name = d->character->name;
	      lead_score = d->character->pcdata->quest_team_score;
	    }
	}
    }

  if( lead_name != NULL )
    printf_to_char( ch, "\n\r{YLeader: %s (%d){x\n\r", lead_name, lead_score );
}


void quest_teamsay( CHAR_DATA* ch, char* argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    char *captain = NULL;
  
    if ( IS_NPC( ch ) )
    {
        send_to_char( "NPCs can't do that.\n\r", ch );
        return;
    }

    if ( IS_NULLSTR( ch->pcdata->quest_team_captain ) )
        captain = ch->name;
    else
        captain = ch->pcdata->quest_team_captain;

    if ( argument[ 0 ] == '\0' )
    {
        send_to_char( "Tell your team what?\n\r", ch );
        return;
    }

    if ( !IS_SET( ch->act_bits, PLR_QUEST ) )
    {
        send_to_char( "Non-questors cannot be on a team.\n\r", ch );
        return;
    }

    sprintf( buf, "You tell your team {%c'%s'{x\n\r", 
        ch->colors[C_QTSAY], argument );
    send_to_char( buf, ch );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING 
        &&   d->character != ch 
        &&   !IS_NPC(d->character)
        &&   IS_SET(d->character->act_bits, PLR_QUEST) 
        &&  (!str_cmp(captain, d->character->pcdata->quest_team_captain)
        ||   (IS_NULLSTR(d->character->pcdata->quest_team_captain)
        &&   !str_cmp(captain, d->character->name)) ) )
        {
          sprintf( buf, "%s tells the team {%c'%s'{x\n\r", 
              ch->name, d->character->colors[C_QTSAY], argument );
	  send_to_char( buf, d->character );
	}
    }

    return;
}

void quest_start( CHAR_DATA* ch, char* argument )
{
  CHAR_DATA *och = NULL;
  char wiznet_msg[MAX_STRING_LENGTH];

  if( IS_SET( queststatus, QUEST_INPROGRESS ) )
    {
      send_to_char( "A quest is already in progress!\n\r", ch );
    }
  else
    {
      SET_BIT( queststatus, QUEST_INPROGRESS );
      SET_BIT( queststatus, QUEST_OPEN );

      if ( !IS_NULLSTR( argument ) )
      {
          strcpy( global_quest_info.name, argument );
          sprintf( wiznet_msg, "You have begun a new quest called %s.\n\r",
              global_quest_info.name );
          send_to_char( wiznet_msg, ch );
          sprintf( wiznet_msg, "%s has started a new quest called %s.", ch->name,
              global_quest_info.name );
          wiznet( wiznet_msg, ch, NULL, WIZ_QUEST, 0, 0 );
      }
      else
      {
          send_to_char( "You have begun a new quest.\n\r", ch );
          sprintf( wiznet_msg, "%s has started a new quest.", ch->name );
          wiznet( wiznet_msg, ch, NULL, WIZ_QUEST, 0, 0 );
      }

      global_quest_info.min_level = 1;
      global_quest_info.max_level = 50;
      global_quest_info.questmob_vnum = 0;

      if ( IS_NPC( ch ) )
      {
          if ( ch->desc )
              och = ch->desc->original;
      }
      else
          och = ch;

      if ( och && !IS_NPC( och ) )
          och->pcdata->questcount++;
    }
}



void quest_open( CHAR_DATA* ch, char* argument )
{
  char wiznet_msg[ MAX_STRING_LENGTH ] = "";

  if( !IS_SET( queststatus, QUEST_INPROGRESS ) )
    {
      quest_start( ch, argument );
      return;
    }

  if( IS_SET( queststatus, QUEST_OPEN ) )
    {
      send_to_char( "The quest is already open.\n\r", ch );
    }
  else
    {
      SET_BIT( queststatus, QUEST_OPEN );
      send_to_char( "You have reopened the quest.\n\r", ch );

      sprintf( wiznet_msg, "%s has reopened the quest.", ch->name );
      wiznet( wiznet_msg, ch, NULL, WIZ_QUEST, 0, 0 );
    }
}



void quest_close( CHAR_DATA* ch, char* argument )
{
  char wiznet_msg[ MAX_STRING_LENGTH ] = "";

  if( !IS_SET( queststatus, QUEST_INPROGRESS ) )
    {
      send_to_char( "There is no quest currently in progress.\n\r", ch );
      return;
    }

  if( !IS_SET( queststatus, QUEST_OPEN ) )
    {
      send_to_char( "The quest isn't open.\n\r", ch );
    }
  else
    {
      REMOVE_BIT( queststatus, QUEST_OPEN );
      send_to_char( "You have closed the quest.\n\r", ch );

      sprintf( wiznet_msg, "%s has closed the quest.", ch->name );
      wiznet( wiznet_msg, ch, NULL, WIZ_QUEST, 0, 0 );
    }
}



void quest_boot( CHAR_DATA* ch, char* argument )
{
  CHAR_DATA* victim = NULL;
  char wiznet_msg[ MAX_STRING_LENGTH ] = "";

  if( !IS_SET( queststatus, QUEST_INPROGRESS ) )
    {
      send_to_char( "You need to start a quest before you can boot people from it.\n\r", ch );
      return;
    }

  if( argument[ 0 ] == '\0' )
    {
      send_to_char( "Boot who?\n\r", ch );
      return;
    }

  victim = get_char_world( ch, argument, FALSE );

  if( victim == NULL )
    {
      send_to_char( "They're not here.\n\r", ch );
      return;
    }

  if( IS_NPC( victim ) )
    {
      send_to_char( "NPCs can't be in quests, so don't bother.\n\r", ch );
      return;
    }

  if( !IS_SET( victim->act_bits, PLR_QUEST ) )
    {
      send_to_char( "They're not in the quest.\n\r", ch );
      return;
    }

  REMOVE_BIT( victim->act_bits, PLR_QUEST );

  send_to_char( "You've been booted from the quest!\n\r", victim );
  printf_to_char( ch, "You've booted %s from the quest!\n\r", victim->name );

  sprintf( wiznet_msg, "%s has booted %s from the quest.", ch->name, victim->name );
  wiznet( wiznet_msg, ch, NULL, WIZ_QUEST, 0, 0 );

}
  
 
void quest_end( CHAR_DATA* ch, char* argument )
{
  DESCRIPTOR_DATA* d = NULL;
  char wiznet_msg[ MAX_STRING_LENGTH ] = "";

  if( !IS_SET( queststatus, QUEST_INPROGRESS ) )
    {
      send_to_char( "You need to start a quest before you can end one.\n\r", ch );
      return;
    }

  for( d = descriptor_list; d != NULL; d = d->next )
    {
      if( d->connected == CON_PLAYING
      &&  d->character
      && !IS_NPC(d->character)
      &&  IS_SET( d->character->act_bits, PLR_QUEST ) )
      {
	  REMOVE_BIT( d->character->act_bits, PLR_QUEST );
	  d->character->pcdata->questscore = 0;
      }
    }


  queststatus = 0;
  global_quest_info.name[0] = '\0';
  send_to_char( "You have ended the quest.\n\r", ch );

  sprintf( wiznet_msg, "%s has ended the quest.", ch->name );
  wiznet( wiznet_msg, ch, NULL, WIZ_QUEST, 0, 0 );
}



void quest_set( CHAR_DATA* ch, char* argument )
{
  char flag_name[ MAX_INPUT_LENGTH ] = "";
  int table_index;
  int i;

  if( !IS_SET( queststatus, QUEST_INPROGRESS ) )
    {
      send_to_char( "There is no active quest at the moment.\n\r", ch );
      return;
    }

  do
    {
      if( argument == NULL )
	{
	  flag_name[ 0 ] = '\0';
	}
      else
	{
	  argument = one_argument( argument, flag_name );
	}

      if( flag_name[ 0 ] == '\0' )
	{
	  send_to_char( "Available quest options:\n\r", ch );

	  for( i = 0; quest_flags[ i ].name != NULL; i++ )
	    {
	      if( quest_flags[ i ].settable )
		{
		  printf_to_char( ch, "  %s\n\r", quest_flags[ i ].name );
		}
	    }
	}
      else
	{
	  i = flag_lookup( flag_name, quest_flags );
	  
	  if( i ==  -99 )
	    {
	      printf_to_char( ch, "%s is not a valid quest option.\n\r", flag_name );
	    }
	  else
	    {
	      table_index = 0;
	      while( quest_flags[ table_index ].name != NULL &&
		     quest_flags[ table_index ].bit != i )
		table_index++;
	      
	      if( quest_flags[ table_index ].settable )
		{
		  if( IS_SET( queststatus, quest_flags[ table_index ].bit ) )
		    {
		      printf_to_char( ch, "Turning off %s.\n\r", quest_flags[ table_index ].name );
		      REMOVE_BIT( queststatus, quest_flags[ table_index ].bit );
		    }
		  else
		    {
		      printf_to_char( ch, "Turning on %s.\n\r", quest_flags[ table_index ].name );
		      SET_BIT( queststatus, quest_flags[ table_index ].bit );
		    }
		}
	      else
		{
		  printf_to_char( ch, "You cannot set %s with this command.\n\r", quest_flags[ table_index ].name );
		}
	    }
	}

    } while( *argument != '\0' );
}



void quest_zero( CHAR_DATA* ch, char* argument )
{
  char target_name[ MAX_INPUT_LENGTH ] = "";

  argument = one_argument( argument, target_name );
  
  if( target_name[ 0 ] == '\0' )
    {
      send_to_char( "Syntax: quest zero <player>|all\n\r", ch );
      return;
    }

  if( strcasecmp( target_name, "all" ) == 0 )
    {
      DESCRIPTOR_DATA* d;

      for( d = descriptor_list; d != NULL; d = d->next )
	{
	  if( d->connected == CON_PLAYING 
          &&  d->character
          && !IS_NPC( d->character )
	  &&  IS_SET( d->character->act_bits, PLR_QUEST ) )
	    {
	      d->character->pcdata->questscore = 0;
	      d->character->pcdata->quest_team_score = 0;
	    }
	}

      send_to_char( "All quest scores set to 0.\n\r", ch );
    }
  else
    {
      CHAR_DATA* target;

      target = get_char_world( ch, target_name, FALSE );
      
      if( target )
	{
	  if( IS_NPC( target ) )
	    {
	      send_to_char( "NPCs can't score points.\n\r", ch );
	      return;
	    }

	  if( IS_SET( target->act_bits, PLR_QUEST ) )
	    {
	      target->pcdata->questscore = 0;
	      target->pcdata->quest_team_score = 0;

	      printf_to_char( ch, "Quest score for %s set to 0.\n\r", target->name );
	    }
	  else
	    {
	      send_to_char( "That character isn't in the quest.\n\r", ch );
	    }
	}
      else
	{
	  send_to_char( "That character isn't here.\n\r", ch );
	}
    }

}

void quest_award( CHAR_DATA* ch, char* argument )
{
  char target_name[ MAX_INPUT_LENGTH ] = "";
  char award[ MAX_INPUT_LENGTH ] = "";
  char announcement[ MAX_STRING_LENGTH ] = "";
  char qsay_buffer[ MAX_STRING_LENGTH ] = "";
  CHAR_DATA* target = NULL;
  CHAR_DATA* captain = NULL;
  DESCRIPTOR_DATA* d = NULL;
  int award_points = 0;

  argument = one_argument( argument, target_name );
  argument = one_argument( argument, award );
  
  if( target_name[ 0 ] == '\0' || award[ 0 ] == '\0' )
    {
      send_to_char( "Syntax: quest award <character> <points> \n\r", ch );
    }
  else
    {
      target = get_char_world( ch, target_name, FALSE );
      
      if( target )
	{
	  if( IS_NPC( target ) )
	    {
	      send_to_char( "What's the point of awarding points to a mob?\n\r", ch );
	      return;
	    }

	  if( IS_SET( target->act_bits, PLR_QUEST ) )
          {
              char verb[10] = "";
              char preposition[10] = "";
	      award_points = atoi( award );
              if ( award_points == 0 )
              {
                  send_to_char( "You can't award zero points.\n\r", ch );
                  return;
              }

	      target->pcdata->questscore += award_points;
	      if ( (captain = get_char_world( ch, target->pcdata->quest_team_captain, FALSE )) != NULL )
                  captain->pcdata->quest_team_score += award_points;

              if ( award_points < 0 ) 
              {
                  strcat(verb, "strip");
                  strcat(preposition, "from" );
                  award_points = award_points * -1;
              }
              else
              {
                  strcat(verb, "award");
                  strcat(preposition, "to" );
              }
	      sprintf( announcement, "You %s %d points %s %s.\n\r", 
                  verb, award_points, preposition, target->name );
	      send_to_char( announcement, ch );

	      for( d = descriptor_list; d != NULL; d = d->next )
              {
		  if ( d->connected == CON_PLAYING
		  &&   d->character != ch
		  &&  ( IS_NPC( d->character ) 
                  ||    IS_SET(d->character->act_bits,PLR_QUEST) ) )
                  {
                       sprintf( qsay_buffer, "{%c[QUEST]{x %s %ss %d points %s %s.\n\r", 
                         d->character->colors[C_QSAY],
                         ch->name, 
                         verb, award_points, preposition, target->name );
                       send_to_char( qsay_buffer ,d->character );
		  }
              }
	  }
	  else
	    {
	      send_to_char( "That character isn't in the quest.\n\r", ch );
	    }
	}
      else
	{
	  send_to_char( "That character isn't here.\n\r", ch );
	}
    }
}



void quest_assign( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* captain;
    CHAR_DATA* player;
    DESCRIPTOR_DATA *d;
    char captain_name[MAX_INPUT_LENGTH];
    char player_name[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool reassigned = FALSE;

    argument = one_argument( argument, player_name );
    argument = one_argument( argument, captain_name );

    if ( IS_NULLSTR(player_name) )
    {
        send_to_char( "Syntax: quest assign <player> [captain]\n\r", ch );
        return;
    }
    if ( (player = get_char_world( ch, player_name, FALSE )) == NULL )
    {
        send_to_char( "They're not here.\n\r", ch );
        return;
    }
    if ( IS_NPC(player) )
    {
        send_to_char( "Not on NPCs.\n\r", ch );
        return;
    }
    if ( !IS_SET( player->act_bits, PLR_QUEST ) )
    {
        act( "$E isn't in the quest.", ch, NULL, player, TO_CHAR );
        return;
    }

    if ( IS_NULLSTR(captain_name) )
    {
        if ( IS_NULLSTR(player->pcdata->quest_team_captain) )
        {
            act( "$E isn't on a team yet.", ch, NULL, player, TO_CHAR );
            return;
        }
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->connected == CON_PLAYING
            &&  ( IS_NPC( d->character ) 
            ||    IS_SET(d->character->act_bits,PLR_QUEST) ) )
            {
                sprintf( buf,
                    "{%c[QUEST]{x %s has been removed from team %s.\n\r",
                    d->character->colors[C_QSAY],
                    player->name,
                    player->pcdata->quest_team_captain );
                send_to_char( buf, d->character );
            }
        }
        free_string( player->pcdata->quest_team_captain );
        player->pcdata->quest_team_captain = str_dup( "" );
        player->pcdata->quest_team_score = 0;
        return;
    }

    if ((captain = get_char_world( ch, captain_name, FALSE )) == NULL)
    {
        send_to_char( "Invalid team captain.  ", ch );
        act( "To remove $N from $S team, just type 'quest assign $N'", 
            ch, NULL, player, TO_CHAR );
        return;
    }
    if ( IS_NPC(captain) )
    {
        send_to_char( "Mobs can't be team captains.\n\r", ch );
        return;
    }
    if ( !IS_SET( captain->act_bits, PLR_QUEST ) )
    {
        send_to_char( "The assigned captain is not in the quest.\n\r", ch );
        return;
    }

    if ( !IS_NULLSTR( player->pcdata->quest_team_captain ) )
        reassigned = TRUE;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING
        &&  ( IS_NPC( d->character ) 
        ||    IS_SET(d->character->act_bits,PLR_QUEST) ) )
        {
            sprintf( buf,
                "{%c[QUEST]{x %s has been %sassigned to team %s.\n\r",
                d->character->colors[C_QSAY],
                player->name,
                ( reassigned ? "re" : "" ),
                captain->name );
            send_to_char( buf, d->character );
        }
    }
    free_string( player->pcdata->quest_team_captain );
    player->pcdata->quest_team_captain = str_dup( captain->name );
    captain->pcdata->quest_team_score += player->pcdata->questscore;

    return;
}



void quest_vnum( CHAR_DATA* ch, char* argument )
{
  int mob_vnum;

  if( argument[ 0 ] == '\0' )
    {
      send_to_char( "Syntax: quest mob_vnum <vnum>|0\n\r", ch );
      return;
    }

  mob_vnum = atoi( argument );

  global_quest_info.questmob_vnum = mob_vnum;

  printf_to_char( ch, "Questmob vnum set to %d\n\r", mob_vnum );
}



void quest_levels( CHAR_DATA* ch, char* argument )
{
  char min_level[ MAX_INPUT_LENGTH ] = "";
  char max_level[ MAX_INPUT_LENGTH ] = "";
  int min;
  int max;

  argument = one_argument( argument, min_level );
  argument = one_argument( argument, max_level );

  if( min_level[ 0 ] == '\0' || max_level[ 0 ] == '\0' )
    {
      send_to_char( "Syntax: quest levels <min> <max>\n\r", ch );
      return;
    }
  
  min = atoi( min_level );
  max = atoi( max_level );

  global_quest_info.min_level = min;
  global_quest_info.max_level = max;

  printf_to_char( ch, "Quest restricted between levels %d and %d\n\r", min, max );  
}



void do_quest( CHAR_DATA* ch, char* argument )
{
  char cmd[ MAX_INPUT_LENGTH ] = "";
  int cmd_index;
  int i;
  bool immortal = FALSE;

  immortal = IS_IMMORTAL( ch );

  if( argument == NULL )
    {
      quest_status( ch, argument );
      return;
    }

  argument = one_argument( argument, cmd );

  cmd_index = get_quest_cmd_index( cmd );

  if( cmd_index == -1 || ( !immortal && quest_cmd_tbl[ cmd_index ].immort == TRUE ) )
    {
      send_to_char( "Available commands:\n\r", ch );
      
      for( i = 0; quest_cmd_tbl[ i ].name != NULL; i++ )
	{
	  if( immortal || quest_cmd_tbl[ i ].immort == FALSE )
	    {
	      printf_to_char( ch, "quest %s\n\r", quest_cmd_tbl[ i ].syntax );
	    }
	}
    }
  else
    {
      ( *quest_cmd_tbl[ cmd_index ].cmd )( ch, argument );
    }
}



void do_qsay( CHAR_DATA* ch, char* argument )
{
  char buf[MAX_STRING_LENGTH] = "";
  DESCRIPTOR_DATA *d = NULL;

  if( argument[0] == '\0' )
    {
      send_to_char( "Tell the questors what?\n\r", ch );
      return;
    }
  
  if( !IS_NPC( ch ) && !IS_SET( ch->act_bits, PLR_QUEST ) )
    {
      send_to_char( "Non questors cannot use the quest channel!\n\r", ch );
      return;
    }

  sprintf( buf, "You {%c[QSAY]:{x '%s'{x\n\r", ch->colors[C_QSAY], argument );
  send_to_char( buf, ch );

  for( d = descriptor_list; d != NULL; d = d->next )
    {
      if( d->connected == CON_PLAYING
      &&  d->character != ch
      &&  ( IS_NPC( d->character ) || IS_SET(d->character->act_bits,PLR_QUEST) ) )
        {
	  sprintf( buf, "{%c[QSAY]{x %s: '%s'{x\n\r", d->character->colors[C_QSAY],
              IS_NPC(ch) ? ch->short_descr : ch->name, argument );
          send_to_char(buf,d->character);
        }
    }
}

void quest_name ( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_SET( queststatus, QUEST_INPROGRESS ) )
    {
        send_to_char( "There is no quest currently in progress.\n\r", ch );
        return;
    }

    if ( !IS_NULLSTR( argument ) )
    {
        strcpy( global_quest_info.name, argument );
        sprintf( buf, "The quest is now called %s.\n\r", global_quest_info.name );
        send_to_char( buf, ch );
    }
    else
    {
        send_to_char( "What would you like to call this quest?\n\r", ch );
    }
    return;
}


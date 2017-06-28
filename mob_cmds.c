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
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  Based on MERC 2.2 MOBprograms by N'Atas-ha.                            *
 *  Written and adapted to ROM 2.4 by                                      *
 *          Markku Nylander (markku.nylander@uta.fi)                       *
 *                                                                         *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "mob_cmds.h"
#include "tables.h"
#include "lookup.h"
#include "recycle.h"

DECLARE_DO_FUN( do_look 	);
DECLARE_SPELL_FUN( spell_null   );
extern ROOM_INDEX_DATA *find_location( CHAR_DATA *, char * );
extern MPRECOG_DATA *new_mprecog(void);
extern char *flag_string( const struct flag_type *flag_table, int bits );


/*
 * Command table.
 */
const   struct  mob_cmd_type    mob_cmd_table   [] =
{
    {   "asound",           do_mpasound,            FALSE   },
    {   "gecho",            do_mpgecho,             FALSE   },
    {   "zecho",            do_mpzecho,             FALSE   },
    {   "kill",             do_mpkill,              FALSE   },
    {   "assist",           do_mpassist,            FALSE   },
    {   "junk",             do_mpjunk,              FALSE   },
    {   "echo",             do_mpecho,              FALSE   },
    {   "echoaround",       do_mpechoaround,        FALSE   },
    {   "echoat",           do_mpechoat,            FALSE   },
    {   "mload",            do_mpmload,             FALSE   },
    {   "oload",            do_mpoload,             FALSE   },
    {   "purge",            do_mppurge,             TRUE    },
    {   "goto",             do_mpgoto,              FALSE   },
    {   "at",               do_mpat,                FALSE   },
    {   "transfer",         do_mptransfer,          FALSE   },
    {   "gtransfer",        do_mpgtransfer,         FALSE   },
    {   "otransfer",        do_mpotransfer,         FALSE   },
    {   "force",            do_mpforce,             FALSE   },
    {   "gforce",           do_mpgforce,            FALSE   },
    {   "vforce",           do_mpvforce,            FALSE   },
    {   "cast",             do_mpcast,              FALSE   },
    {   "mcast",            do_mpmcast,             FALSE   },
    {   "damage",           do_mpdamage,            FALSE   },
    {   "remember",         do_mpremember,          FALSE   },
    {   "forget",           do_mpforget,            FALSE   },
    {   "delay",            do_mpdelay,             FALSE   },
    {   "cancel",           do_mpcancel,            FALSE   },
    {   "call",             do_mpcall,              TRUE    },
    {   "invoke",           do_mpinvoke,            TRUE    },
    {   "flee",             do_mpflee,              FALSE   },
    {   "remove",           do_mpremove,            FALSE   },
    {   "pause",            do_mppause,             FALSE   },
    {   "grab",             do_mpgrab,              FALSE   },
    {   "resetprand",       do_mpresetprand,        FALSE   },
    {   "peace",            do_mppeace,             FALSE   },
    {   "plant",            do_mpput,               FALSE   },
    {   "rawdamage",        do_mprawdamage,         FALSE   },
    {   "recogon",          do_mprecogon,           TRUE    },
    {   "recogoff",         do_mprecogoff,          TRUE    },
    {   "giveexp",          do_mpgiveexp,           TRUE    },
    {   "teleport",         do_mpteleport,          FALSE   },
    {   "tell",             do_mptell,              FALSE   },
    {   "blindsighton",     do_mpbson,              FALSE   },
    {   "blindsightoff",    do_mpbsoff,             FALSE   },
    {   "zechoaround",      do_mpzechoaround,       FALSE   },
    {   "sreset",           do_mpsreset,            FALSE   },
    {   "alert",            do_mpalert,             FALSE   },
    {   "bankroll",         do_mpbankroll,          FALSE   },
    {   "secho",            do_mpsecho,             FALSE   },
    {   "boostskill",       do_mpboostskill,        TRUE    },
    {   "setskill",         do_mpsetskill,          TRUE    },
    {   "",                 0,                      0       }
};

void do_mob( CHAR_DATA *ch, char *argument )
{
    /*
     * Security check!
     */
    if ( !IS_NPC( ch ) )
/*  if ( ch->desc != NULL && get_trust(ch) < MAX_LEVEL ) */
	return;
    mob_interpret( ch, argument );
}
/*
 * Mob command interpreter. Implemented separately for security and speed
 * reasons. A trivial hack of interpret()
 */
void mob_interpret( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH], command[MAX_INPUT_LENGTH];
    int cmd;

    argument = one_argument( argument, command );

    /*
     * Look for command in command table.
     */
    for ( cmd = 0; mob_cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == mob_cmd_table[cmd].name[0]
	&&   !str_prefix( command, mob_cmd_table[cmd].name ) )
	{
	    (*mob_cmd_table[cmd].do_fun) ( ch, argument );
	    tail_chain( );
	    return;
	}
    }
    sprintf( buf, "Mob_interpret: invalid cmd from mob %d: '%s'",
	IS_NPC(ch) ? ch->pIndexData->vnum : 0, command );
    bug( buf, 0 );
}

char *mprog_type_to_name( int type )
{
    switch ( type )
    {
    case TRIG_ACT:             	return "ACT";
    case TRIG_SPEECH:          	return "SPEECH";
    case TRIG_RANDOM:          	return "RANDOM";
    case TRIG_FIGHT:           	return "FIGHT";
    case TRIG_HPCNT:           	return "HPCNT";
    case TRIG_DEATH:           	return "DEATH";
    case TRIG_ENTRY:           	return "ENTRY";
    case TRIG_GREET:           	return "GREET";
    case TRIG_GRALL:        	return "GRALL";
    case TRIG_GIVE:            	return "GIVE";
    case TRIG_BRIBE:           	return "BRIBE";
    case TRIG_KILL:	      	return "KILL";
    case TRIG_DELAY:           	return "DELAY";
    case TRIG_SURR:	      	return "SURRENDER";
    case TRIG_EXIT:	      	return "EXIT";
    case TRIG_EXALL:	      	return "EXALL";
    case TRIG_SPAWN:		return "SPAWN";
    case TRIG_PPM:              return "PPM";
    case TRIG_SELSE:            return "SELSE";
    case TRIG_GELSE:            return "GELSE";
    default:                  	return "ERROR";
    }
}

/* 
 * Displays MOBprogram triggers of a mobile
 *
 * Syntax: mpstat [name]
 */
void do_mpstatpc( CHAR_DATA *ch, CHAR_DATA *victim )
{
    MPRECOG_DATA *mpr;
    INT_LIST *mprt;
    char buf[MAX_STRING_LENGTH], expire[512];
    int counter = 0, timers = 0;
    if ( victim->mprecog == NULL )
    {
        act("No recogs currently set on $N.",ch,NULL,victim,TO_CHAR);
    }
    else
    {
        sprintf(buf, "  [%6s] %-26s     [%6s] %-26s", 
            "Recog#", "Flags", "Recog#", "Flags" );
        send_to_char( buf , ch );
        for ( mpr = victim->mprecog; mpr != NULL; mpr = mpr->next )
        {
            if ( mpr->flags == 0 ) continue;
            if ( counter % 2 == 0 ) send_to_char( "\n\r", ch );
            sprintf(buf, "  [%6d] %-26s   ", 
                mpr->vnum, flag_string( mprecog_flags, mpr->flags ) );
            send_to_char( buf, ch );
            counter++;
            if ( mpr->timer != NULL )
                timers++;
        }
        send_to_char( "\n\r", ch );

        if ( timers > 0 )
        {
            counter = 0;
            sprintf(buf, "\n\r  [%6s] %-3s %-24s   [%6s] %-3s %-24s", 
                "Recog#", "Flg", "Expiration", "Recog#", "Flg", "Expiration" );
            send_to_char( buf , ch );
            for ( mpr = victim->mprecog; mpr != NULL; mpr = mpr->next )
            {
                if ( mpr->flags == 0 ) continue;
                if ( mpr->timer == NULL ) continue;
                for ( mprt = mpr->timer; mprt != NULL; mprt = mprt->next )
                { /*       12345678901234567890123456 */ 
                    if ( mprt->value[1] > 1266200751 )
                    {
                        strftime( expire, 512, "on %d/%m/%y %I:%M%p",
                            localtime( (time_t *) &mprt->value[1] ) );
                    }
                    else
                    {
                        char scratch[512];
                        int found = 0;
                        int running, days, hrs, mins;
                        sprintf(expire, "in ");

                        running = mprt->value[1];
                        days = running / 86400;
                        running = running % 86400;

                        hrs = running / 3600;
                        running = running % 3600;

                        mins = running / 60;
                        running = running % 60;
                        if ( days > 0 )
                        {
                            sprintf(scratch,"%dd", days);
                            strcat(expire, scratch);
                            found++;
                        }
                        if ( hrs > 0 )
                        {
                            if ( found > 0 ) 
                                strcat(expire, ", ");
                            sprintf(scratch,"%dh", hrs );
                            strcat(expire, scratch);
                            found++;
                        }
                        if ( mins > 0 )
                        {
                            if ( found > 0 )
                                strcat(expire, ", ");
                            sprintf(scratch,"%dm", mins);
                            strcat(expire,scratch);
                            found++;
                        }
                        if ( found > 0 )
                            strcat(expire, ", ");
                        sprintf(scratch,"%ds", running);
                        strcat(expire,scratch);
                    }
    
                    sprintf(buf, "  [%6d] %-3s %-24s ", 
                        mpr->vnum, 
                        flag_string( mprecog_flags, mprt->value[0] ),
                        expire );
                    if ( counter % 2 == 0 ) send_to_char( "\n\r", ch );
                    send_to_char( buf, ch );
                    counter++;
                }
            }
            send_to_char( "\n\r", ch );
        }
    }
    return;
}

void do_mpstat( CHAR_DATA *ch, char *argument )
{
    char        arg[ MAX_STRING_LENGTH  ];
    MPROG_LIST  *mprg;
    CHAR_DATA   *victim;
    int i;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Mpstat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
    {
	send_to_char( "No such creature.\n\r", ch );
	return;
    }

    if ( !IS_NPC( victim ) )
    {
        do_mpstatpc( ch, victim );
	return;
    }

    if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
    {
	send_to_char( "No such creature visible.\n\r", ch );
	return;
    }

    sprintf( arg, "Mobile #%-6d [%s]\n\r",
	victim->pIndexData->vnum, victim->short_descr );
    send_to_char( arg, ch );

    sprintf( arg, "Delay   %-6d [%s]\n\r",
	victim->mprog_delay,
	victim->mprog_target == NULL 
		? "No target" : victim->mprog_target->name );
    send_to_char( arg, ch );

    if ( !victim->pIndexData->mprog_flags )
    {
	send_to_char( "(No mobprogs set)\n\r\n\r", ch);
        do_mpstatpc( ch, victim );
	return;
    }

    for ( i = 0, mprg = victim->pIndexData->mprogs; mprg != NULL;
	 mprg = mprg->next )

    {
	sprintf( arg, "[%2d] Trigger [%-8s] Program [%4d] Phrase [%s]\n\r",
	      ++i,
	      mprog_type_to_name( mprg->trig_type ),
	      mprg->vnum,
	      mprg->trig_phrase );
	send_to_char( arg, ch );
    }

    send_to_char( "\n\r", ch );
    do_mpstatpc( ch, victim );

    return;

}

/*
 * Displays the source code of a given MOBprogram
 *
 * Syntax: mpdump [vnum]
 */
void do_mpdump( CHAR_DATA *ch, char *argument )
{
   char buf[ MAX_INPUT_LENGTH ];
   MPROG_CODE *mprg;

   one_argument( argument, buf );
   if ( ( mprg = get_mprog_index( atoi(buf) ) ) == NULL )
   {
	send_to_char( "No such MOBprogram.\n\r", ch );
	return;
   }
   page_to_char( mprg->code, ch );
}

/*
 * Prints the argument to all active players in the game
 *
 * Syntax: mob gecho [string]
 */
void do_mpgecho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
	bug( "MpGEcho: missing argument from vnum %d",
	    IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
 	{
	    if ( IS_IMMORTAL(d->character) )
		send_to_char( "Mob echo> ", d->character );
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r", d->character );
	}
    }
}

void do_mppause( CHAR_DATA* ch, char* argument )
{
  int arg;

  if( argument[ 0 ] == '\0' )
    {
      bug( "mppause: missing argument from vnum %d",
	   IS_NPC( ch ) ? ch->pIndexData->vnum : 0 );
      return;
    }

  arg = atoi( argument );

  if( arg < 1 )
    {
      bug( "mppause: negative or non-numeric argument from vnum %d",
	   IS_NPC( ch ) ? ch->pIndexData->vnum : 0 );
      return;
    }

  WAIT_STATE( ch, arg );

}

/*
 * Prints the argument to all players in the same area as the mob
 *
 * Syntax: mob zecho [string]
 */
void do_mpzecho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
	bug( "MpZEcho: missing argument from vnum %d",
	    IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    if ( ch->in_room == NULL )
	return;

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING 
	&&   d->character->in_room != NULL 
	&&   d->character->in_room->area == ch->in_room->area )
 	{
	    if ( IS_IMMORTAL(d->character) )
		send_to_char( "Mob echo> ", d->character );
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r", d->character );
	}
    }
}

/*
 * Prints the argument to all the rooms aroud the mobile
 *
 * Syntax: mob asound [string]
 */
void do_mpasound( CHAR_DATA *ch, char *argument )
{

    ROOM_INDEX_DATA *was_in_room;
    int              door;

    if ( argument[0] == '\0' )
	return;

    was_in_room = ch->in_room;
    for ( door = 0; door < 6; door++ )
    {
    	EXIT_DATA       *pexit;
      
      	if ( ( pexit = was_in_room->exit[door] ) != NULL
	  &&   pexit->u1.to_room != NULL
	  &&   pexit->u1.to_room != was_in_room )
      	{
	    ch->in_room = pexit->u1.to_room;
	    MOBtrigger  = FALSE;
	    act( argument, ch, NULL, NULL, TO_ROOM );
	    MOBtrigger  = TRUE;
	}
    }
    ch->in_room = was_in_room;
    return;

}

/*
 * Lets the mobile kill any player or mobile without murder
 *
 * Syntax: mob kill [victim]
 */
void do_mpkill( CHAR_DATA *ch, char *argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	return;

    if ( ( victim = get_char_room( ch, arg, FALSE ) ) == NULL )
	return;

    if ( victim == ch || IS_NPC(victim) || ch->position == POS_FIGHTING )
	return;

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
	bug( "MpKill - Charmed mob attacking master from vnum %d.",
	    IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}

/*
 * Lets the mobile assist another mob or player
 *
 * Syntax: mob assist [character]
 */
void do_mpassist( CHAR_DATA *ch, char *argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	return;

    if ( ( victim = get_char_room( ch, arg, FALSE ) ) == NULL )
	return;

    if ( victim == ch || ch->fighting != NULL || victim->fighting == NULL )
	return;

    multi_hit( ch, victim->fighting, TYPE_UNDEFINED );
    return;
}


/*
 * Lets the mobile destroy an object in its inventory
 * it can also destroy a worn object and it can destroy 
 * items using all.xxxxx or just plain all of them 
 *
 * Syntax: mob junk [item]
 */

void do_mpjunk( CHAR_DATA *ch, char *argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    one_argument( argument, arg );

    if ( arg[0] == '\0')
	return;

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
    	if ( ( obj = get_obj_wear( ch, arg, FALSE ) ) != NULL )
      	{
      	    unequip_char( ch, obj );
	    extract_obj( obj );
    	    return;
      	}
      	if ( ( obj = get_obj_carry( ch, arg, ch, FALSE ) ) == NULL )
	    return; 
	extract_obj( obj );
    }
    else
      	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
      	{
            obj_next = obj->next_content;
	    if ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
            {
          	if ( obj->wear_loc != WEAR_NONE)
	    	unequip_char( ch, obj );
          	extract_obj( obj );
            } 
      	}

    return;

}

/*
 * Prints the message to everyone in the room other than the mob and victim
 *
 * Syntax: mob echoaround [victim] [string]
 */

void do_mpechoaround( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
	return;

    if ( ( victim=get_char_room( ch, arg, FALSE ) ) == NULL )
	return;

    act( argument, ch, NULL, victim, TO_NOTVICT );
}

/*
 * Prints the message to only the victim
 *
 * Syntax: mob echoat [victim] [string]
 */
void do_mpechoat( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
	return;

    if ( ( victim = get_char_room( ch, arg, FALSE ) ) == NULL )
	return;

    act( argument, ch, NULL, victim, TO_VICT );
}

/*
 * Prints the message to the room at large
 *
 * Syntax: mpecho [string]
 */
void do_mpecho( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
	return;
    act( argument, ch, NULL, NULL, TO_ROOM );
}

/*
 * Lets the mobile load another mobile.
 *
 * Syntax: mob mload [vnum]
 */
void do_mpmload( CHAR_DATA *ch, char *argument )
{
    char            arg[ MAX_INPUT_LENGTH ];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA      *victim;
    int vnum;

    one_argument( argument, arg );

    if ( ch->in_room == NULL || arg[0] == '\0' || !is_number(arg) )
	return;

    vnum = atoi(arg);
    if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
    {
	sprintf( arg, "Mpmload: bad mob index (%d) from mob %d",
	    vnum, IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	bug( arg, 0 );
	return;
    }
    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    if ( HAS_TRIGGER( victim, TRIG_SPAWN ) )
        mp_spawn_trigger( victim );

    return;
}

/*
 * Lets the mobile load an object
 *
 * Syntax: mob oload [vnum] [level] {R}
 */
void do_mpoload( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA       *obj;
    int             level;
    bool            fToroom = FALSE, fWear = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
               one_argument( argument, arg3 );
 
    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        bug( "Mpoload - Bad syntax from vnum %d.",
	    IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }
 
    if ( arg2[0] == '\0' )
    {
	level = get_trust( ch );
    }
    else
    {
	/*
	 * New feature from Alander.
	 */
        if ( !is_number( arg2 ) )
        {
	    bug( "Mpoload - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	    return;
        }
	level = atoi( arg2 );
	if ( level < 0 || level > get_trust( ch ) )
	{
	    bug( "Mpoload - Bad level from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	    return;
	}
    }

    /*
     * Added 3rd argument
     * omitted - load to mobile's inventory
     * 'R'     - load to room
     * 'W'     - load to mobile and force wear
     */
    if ( arg3[0] == 'R' || arg3[0] == 'r' )
	fToroom = TRUE;
    else if ( arg3[0] == 'W' || arg3[0] == 'w' )
	fWear = TRUE;

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	bug( "Mpoload - Bad vnum arg from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    obj = create_object( pObjIndex, level );
    if ( (fWear || !fToroom) && CAN_WEAR(obj, ITEM_TAKE) )
    {
	obj_to_char_init( obj, ch );
	if ( fWear )
	    wear_obj( ch, obj, TRUE );
    }
    else
    {
	obj_to_room( obj, ch->in_room );
    }

    return;
}

/*
 * Lets the mobile purge all objects and other npcs in the room,
 * or purge a specified object or mob in the room. The mobile cannot
 * purge itself for safety reasons.
 *
 * syntax mob purge {target}
 */
#define MPPURGE_MOBS    1
#define MPPURGE_OBJS    2
void do_mppurge( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;
    CHAR_DATA *vnext;
    OBJ_DATA  *obj_next;
    int flags = 0;

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
        flags = MPPURGE_MOBS|MPPURGE_OBJS;
    else if ( !str_cmp(arg, "mobs") )
        flags = MPPURGE_MOBS;
    else if ( !str_cmp(arg, "objs") )
        flags = MPPURGE_OBJS;

    if ( flags == 0 ) /* purge something specific */
    {
        if ( ( victim = get_char_room_seriously( ch, arg, FALSE ) ) == NULL )
        {
            if ( ( obj = get_obj_here( ch, arg, FALSE ) ) )
                extract_obj( obj );
            else
                bug( "Mppurge - Bad argument from vnum %d.",
                    IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
            return;
        }
        if ( !IS_NPC( victim ) )
            bug( "Mppurge - Purging a PC from vnum %d.", 
                IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        else
            extract_char( victim, TRUE );
        return;
    }

/* purge all of something */
    if ( IS_SET(flags, MPPURGE_MOBS) )
    {
        for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
        {
            vnext = victim->next_in_room;
            if ( IS_NPC( victim ) && victim != ch 
            &&   !IS_SET(victim->act_bits, ACT_NOPURGE) )
                extract_char( victim, TRUE );
        }
    }

    if ( IS_SET(flags, MPPURGE_OBJS) )
    {
        for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;
            if ( !IS_SET(obj->extra_flags, ITEM_NOPURGE) )
                extract_obj( obj );
        }
    }

    return;
}
#undef MPPURGE_MOBS
#undef MPPURGE_OBJS

/*
 * Lets the mobile goto any location it wishes that is not private.
 *
 * Syntax: mob goto [location]
 */
void do_mpgoto( CHAR_DATA *ch, char *argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "Mpgoto - No argument from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	bug( "Mpgoto - No such location from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    char_from_room( ch );
    char_to_room( ch, location );

    return;
}

/* 
 * Lets the mobile do a command at another location.
 *
 * Syntax: mob at [location] [commands]
 */
void do_mpat( CHAR_DATA *ch, char *argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA       *wch;
    OBJ_DATA 	    *on;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "Mpat - Bad argument from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	bug( "Mpat - No such location from vnum %d.",
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
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
 
/*
 * Lets the mobile transfer people.  The 'all' argument transfers
 *  everyone in the current room to the specified location
 *
 * Syntax: mob transfer [target|'all'] [location]
 */
void do_mptransfer( CHAR_DATA *ch, char *argument )
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    char	     buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA       *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	bug( "Mptransfer - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	CHAR_DATA *victim_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = victim_next )
	{
	    victim_next = victim->next_in_room;
	    if ( !IS_NPC(victim) )
	    {
		sprintf( buf, "%s %s", victim->name, arg2 );
		do_mptransfer( ch, buf );
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
	    bug( "Mptransfer - No such location from vnum %d.",
	        IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	    return;
	}

	if ( room_is_private( location ) )
	    return;
    }

    if ( ( victim = get_char_world( ch, arg1, FALSE ) ) == NULL )
	return;

    if ( victim->in_room == NULL )
	return;

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    char_from_room( victim );
    char_to_room( victim, location );
    do_look( victim, "auto" );

    return;
}

/*
 * Lets the mobile transfer all chars in same group as the victim.
 *
 * Syntax: mob gtransfer [victim] [location]
 */
void do_mpgtransfer( CHAR_DATA *ch, char *argument )
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    char	     buf[MAX_STRING_LENGTH];
    CHAR_DATA       *who, *victim, *victim_next;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	bug( "Mpgtransfer - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    if ( (who = get_char_room( ch, arg1, FALSE )) == NULL )
	return;

    for ( victim = ch->in_room->people; victim; victim = victim_next )
    {
    	victim_next = victim->next_in_room;
    	if( is_same_group( who,victim ) )
    	{
	    sprintf( buf, "%s %s", victim->name, arg2 );
	    do_mptransfer( ch, buf );
    	}
    }
    return;
}

/*
 * Lets the mobile force someone to do something. Must be mortal level
 * and the all argument only affects those in the room with the mobile.
 *
 * Syntax: mob force [victim] [commands]
 */
void do_mpforce( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "Mpforce - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( vch->in_room == ch->in_room
		&& get_trust( vch ) < get_trust( ch ) 
		&& can_see( ch, vch ) )
	    {
		interpret( vch, argument );
	    }
	}
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_room( ch, arg, FALSE ) ) == NULL )
	    return;

	if ( victim == ch )
	    return;

	interpret( victim, argument );
    }

    return;
}

/*
 * Lets the mobile force a group something. Must be mortal level.
 *
 * Syntax: mob gforce [victim] [commands]
 */
void do_mpgforce( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim, *vch, *vch_next;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "MpGforce - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    if ( ( victim = get_char_room( ch, arg, FALSE ) ) == NULL )
	return;

    if ( victim == ch )
	return;

    for ( vch = victim->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	if ( is_same_group(victim,vch) )
        {
	    interpret( vch, argument );
	}
    }
    return;
}

/*
 * Forces all mobiles of certain vnum to do something (except ch)
 *
 * Syntax: mob vforce [vnum] [commands]
 */
void do_mpvforce( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim, *victim_next;
    char arg[ MAX_INPUT_LENGTH ];
    int vnum;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "MpVforce - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    if ( !is_number( arg ) )
    {
	bug( "MpVforce - Non-number argument vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    vnum = atoi( arg );

    for ( victim = char_list; victim; victim = victim_next )
    {
	victim_next = victim->next;
	if ( IS_NPC(victim) && victim->pIndexData->vnum == vnum
	&&   ch != victim && victim->fighting == NULL )
	    interpret( victim, argument );
    }
    return;
}


/*
 * Lets the mobile cast spells --
 * Beware: this does only crude checking on the target validity
 * and does not account for mana etc., so you should do all the
 * necessary checking in your mob program before issuing this cmd!
 *
 * Syntax: mob cast [spell] {target}
 */

void do_mpcast( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    OBJ_DATA *obj;
    void *victim = NULL;
    char spell[ MAX_INPUT_LENGTH ],
	 target[ MAX_INPUT_LENGTH ];
    int sn;

    argument = one_argument( argument, spell );
               one_argument( argument, target );

    if ( spell[0] == '\0' )
    {
	bug( "MpCast - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
    {
	bug( "MpCast - No such spell from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    if ( !IS_NULLSTR( target ) )
    {
        vch = get_char_room( ch, target, FALSE );
        obj = get_obj_here( ch, target, FALSE );
    }
    else
    {
        vch = ch->fighting;
        obj = NULL;
    }
    switch ( skill_table[sn].target )
    {
	default: return;
	case TAR_IGNORE: 
	    break;
	case TAR_CHAR_OFFENSIVE: 
	    if ( vch == NULL || vch == ch )
		return;
	    victim = ( void * ) vch;
	    break;
	case TAR_CHAR_DEFENSIVE:
	    victim = vch == NULL ? ( void *) ch : (void *) vch; break;
	case TAR_CHAR_SELF:
	    victim = ( void *) ch; break;
	case TAR_OBJ_CHAR_DEF:
	case TAR_OBJ_CHAR_OFF:
	  if( vch == NULL || vch == ch )
	    return;
	  victim = ( void* ) vch;
	  break;
	case TAR_OBJ_INV:
	    if ( obj == NULL )
		return;
	    victim = ( void * ) obj;
    }
    (*skill_table[sn].spell_fun)( sn, ch->level, ch, victim,
	skill_table[sn].target );
    return;
}

/* mob mcast - casts a single offensive spell on all PCs in the
   room, all NPCs fighting the caster, and whoever the caster is
   fighting 5/4/07 gkl */
void do_mpmcast( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *vch, *vch_next;
    void *vo;
    int sn;
    int target;

    if ( ch->in_room == NULL )
    {
        bug( "mpmcast called from mob %d in NULL room.",
            IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }

    one_argument( argument, arg );
    if ( IS_NULLSTR(arg) )
    {
        bug( "mpmcast - bad syntax from mob %d.", 
            IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }
    if ((sn = skill_lookup( arg ) ) < 0 )
    {
        bug( "mpmcast - bad spell from mob %d.", 
            IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }

    target  = TARGET_NONE;
    switch ( skill_table[sn].target )
    {
        default:
            bug( "mpmcast - bad target for sn %d.", sn );
        return;
        case TAR_IGNORE:
        case TAR_CHAR_DEFENSIVE:
        case TAR_CHAR_SELF:
        case TAR_OBJ_INV:
        case TAR_OBJ_CHAR_DEF:
            bug( "mpmcast - spell from mob %d cannot be mcast.", 
                IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
           return;
        break;
        case TAR_CHAR_OFFENSIVE:
        case TAR_OBJ_CHAR_OFF:
            target = TARGET_CHAR;
        break;
    }

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;

        if ( vch->fighting == ch 
        ||   ch->fighting == vch 
        || ( !IS_NPC(vch) && !is_safe_verb(ch, vch, FALSE ) ) )
        {
            vo = (void *)vch;
            (*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
        }
    }

    return;
}


/*
 * Lets mob cause unconditional damage to someone. Nasty, use with caution.
 * Also, this is silent, you must show your own damage message...
 *
 * Syntax: mob damage [victim] [min] [max] {kill}
 */
void do_mpdamage( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim = NULL, *victim_next;
    char target[ MAX_INPUT_LENGTH ],
	 min[ MAX_INPUT_LENGTH ],
	 max[ MAX_INPUT_LENGTH ];
    int low, high;
    bool fAll = FALSE, fKill = FALSE;

    argument = one_argument( argument, target );
    argument = one_argument( argument, min );
    argument = one_argument( argument, max );

    if ( target[0] == '\0' )
    {
	bug( "MpDamage - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    if( !str_cmp( target, "all" ) )
	fAll = TRUE;
    else if( ( victim = get_char_room( ch, target, FALSE ) ) == NULL )
	return;

    if ( is_number( min ) )
	low = atoi( min );
    else
    {
	bug( "MpDamage - Bad damage min vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    if ( is_number( max ) )
	high = atoi( max );
    else
    {
	bug( "MpDamage - Bad damage max vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    one_argument( argument, target );

    /*
     * If kill parameter is omitted, this command is "safe" and will not
     * kill the victim.
     */

    if ( target[0] != '\0' )
	fKill = TRUE;
    if ( fAll )
    {
	for( victim = ch->in_room->people; victim; victim = victim_next )
	{
	    victim_next = victim->next_in_room;
	    if ( victim != ch )
    		damage( (fKill ? ch : victim), victim, 
		    number_range(low,high),
	        TYPE_UNDEFINED, DAM_NONE, FALSE );
	}
    }
    else
    	damage( (fKill ? ch : victim ), victim, 
	    number_range(low,high),
        TYPE_UNDEFINED, DAM_NONE, FALSE );
    return;
}

/* Damage without modifiers */

void do_mprawdamage( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim = NULL, *victim_next;
    char target[ MAX_INPUT_LENGTH ],
         min[ MAX_INPUT_LENGTH ],
         max[ MAX_INPUT_LENGTH ];
    int low, high;
    bool fAll = FALSE, fKill = FALSE;

    argument = one_argument( argument, target );
    argument = one_argument( argument, min );
    argument = one_argument( argument, max );

    if ( target[0] == '\0' )
    {
        bug( "mprawdamage - Bad syntax from vnum %d.",
                IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }
    if( !str_cmp( target, "all" ) )
        fAll = TRUE;
    else if( ( victim = get_char_room( ch, target, FALSE ) ) == NULL )
        return;

    if ( is_number( min ) )
        low = atoi( min );
    else
    {
        bug( "mprawdamage - Bad damage min vnum %d.",
                IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }
    if ( is_number( max ) )
        high = atoi( max );
    else
    {
        bug( "mprawdamage - Bad damage max vnum %d.",
                IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }
    one_argument( argument, target );

    if ( target[0] != '\0' )
        fKill = TRUE;
    if ( fAll )
    {
        for( victim = ch->in_room->people; victim; victim = victim_next )
        {
            victim_next = victim->next_in_room;
            if ( victim != ch )
                raw_damage( victim, victim, number_range(low,high),
                    TYPE_UNDEFINED, DAM_NONE, FALSE, fKill );
        }
    }
    else
        raw_damage( victim, victim, number_range(low,high),
            TYPE_UNDEFINED, DAM_NONE, FALSE, fKill );
    return;
}


/*
 * Lets the mobile to remember a target. The target can be referred to
 * with $q and $Q codes in MOBprograms. See also "mob forget".
 *
 * Syntax: mob remember [victim]
 */
void do_mpremember( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
    one_argument( argument, arg );
    if ( arg[0] != '\0' )
	ch->mprog_target = get_char_world( ch, arg, FALSE );
    else
	bug( "MpRemember: missing argument from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
}

/*
 * Reverse of "mob remember".
 *
 * Syntax: mob forget
 */
void do_mpforget( CHAR_DATA *ch, char *argument )
{
    ch->mprog_target = NULL;
}

/*
 * Sets a delay for MOBprogram execution. When the delay time expires,
 * the mobile is checked for a MObprogram with DELAY trigger, and if
 * one is found, it is executed. Delay is counted in PULSE_MOBILE
 *
 * Syntax: mob delay [pulses]
 */
void do_mpdelay( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];

    one_argument( argument, arg );
    if ( !is_number( arg ) )
    {
	bug( "MpDelay: invalid arg from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    ch->mprog_delay = atoi( arg );
}

/*
 * Reverse of "mob delay", deactivates the timer.
 *
 * Syntax: mob cancel
 */
void do_mpcancel( CHAR_DATA *ch, char *argument )
{
   ch->mprog_delay = -1;
}
/*
 * Lets the mobile to call another MOBprogram withing a MOBprogram.
 * This is a crude way to implement subroutines/functions. Beware of
 * nested loops and unwanted triggerings... Stack usage might be a problem.
 * Characters and objects referred to must be in the same room with the
 * mobile.
 *
 * Syntax: mob call [vnum] [victim|'null'] [object1|'null'] [object2|'null']
 *
 */
void do_mpcall( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *vch;
    OBJ_DATA *obj1, *obj2;
    MPROG_CODE *prg;
    extern void program_flow( sh_int, char *, CHAR_DATA *, CHAR_DATA *, const void *, const void * );

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "MpCall: missing arguments from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    if ( ( prg = get_mprog_index( atoi(arg) ) ) == NULL )
    {
	bug( "MpCall: invalid prog from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    vch = NULL;
    obj1 = obj2 = NULL;
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
        vch = get_char_room_seriously( ch, arg, FALSE );
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
    	obj1 = get_obj_here( ch, arg, FALSE );
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
    	obj2 = get_obj_here( ch, arg, FALSE );
    program_flow( prg->vnum, prg->code, ch, vch, (void *)obj1, (void *)obj2 );
}


/*
 * Syntax: mob invoke <name or mvnum> <mpvnum> <'all'|'one'> [victim|'null'] [object1|'null'] [object2|'null']
 *
 */
void do_mpinvoke( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch, *rch;
    OBJ_DATA *obj1, *obj2;
    MPROG_CODE *prg;
    char targstr[MAX_INPUT_LENGTH],arg[MAX_INPUT_LENGTH];
    bool fall, usevnum;
    sh_int vn = 0;
    extern void program_flow( sh_int, char *, CHAR_DATA *, CHAR_DATA *, const void *, const void * );

    argument = one_argument( argument, targstr );

    if ( IS_NULLSTR(targstr) )
    {
        bug( "mpinvoke: missing target argument from vnum %d.", 
            IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }
    if ( !is_number( targstr ) )
    {
        usevnum = FALSE;
        if ( !is_mob_room( ch, targstr, FALSE ) )
        {
            bug( "mpinvoke: target mob not in room from vnum %d.", 
                IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
            return;
        }
    }
    else
    {
        usevnum = TRUE;
        vn = atoi( targstr );
        if ( !get_mob_vnum_room( ch, vn ) )
        {
            bug( "mpinvoke: target mob not in room from vnum %d.", 
                IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
            return;
        }
    }
    argument = one_argument( argument, arg );
    if ( ( prg = get_mprog_index( atoi(arg) ) ) == NULL )
    {
        bug( "mpinvoke: invalid prog from vnum %d.",
                IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }
    argument = one_argument( argument, arg );
    if ( !str_prefix( arg, "all" ) )
      fall = TRUE;
    else
      fall = FALSE;

    vch = NULL;
    obj1 = obj2 = NULL;
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
        vch = get_char_room_seriously( ch, arg, FALSE );
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
        obj1 = get_obj_here( ch, arg, FALSE );
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
        obj2 = get_obj_here( ch, arg, FALSE );

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if ( rch->in_room == NULL )
        {
            bug( "mpinvoke: found rch in room that is not in room!  This is bad!", 0 );
            continue;
        }

        if ( IS_NPC( rch ) && rch != ch )
        {
            if ( ( usevnum && rch->pIndexData->vnum == vn )
            || ( !usevnum && is_name( targstr, rch->name ) ) )
            {
                program_flow( prg->vnum, prg->code, rch, vch, (void *)obj1, (void *)obj2 );
                if ( !fall ) return;
            }
        }
    }
}

/*
 * Forces the mobile to flee.
 *
 * Syntax: mob flee
 *
 */
void do_mpflee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    EXIT_DATA *pexit;
    int door, attempt;

    if ( ch->fighting != NULL )
	return;

    if ( (was_in = ch->in_room) == NULL )
	return;

    for ( attempt = 0; attempt < 6; attempt++ )
    {
        door = number_door( );
        if ( ( pexit = was_in->exit[door] ) == 0
        ||   pexit->u1.to_room == NULL
        ||   IS_SET(pexit->exit_info, EX_CLOSED)
        || ( IS_NPC(ch)
        &&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
            continue;

        move_char( ch, door, FALSE );
        if ( ch->in_room != was_in )
	    return;
    }
}

/*
 * Lets the mobile to transfer an object. The object must be in the same
 * room with the mobile.
 *
 * Syntax: mob otransfer [item name] [location]
 */
void do_mpotransfer( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *location;
    char arg[ MAX_INPUT_LENGTH ];
    char buf[ MAX_INPUT_LENGTH ];

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "MpOTransfer - Missing argument from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    one_argument( argument, buf );
    if ( ( location = find_location( ch, buf ) ) == NULL )
    {
	bug( "MpOTransfer - No such location from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    if ( (obj = get_obj_here( ch, arg, FALSE )) == NULL )
	return;
    if ( obj->carried_by == NULL )
	obj_from_room( obj );
    else
    {
	if ( obj->wear_loc != WEAR_NONE )
	    unequip_char( ch, obj );
	obj_from_char( obj );
    }
    obj_to_room( obj, location );
}

/*
 * Lets the mobile to strip an object or all objects from the victim.
 * Useful for removing e.g. quest objects from a character.
 *
 * Syntax: mob remove [victim] [object vnum|'all']
 */
void do_mpremove( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj, *obj_next;
    sh_int vnum = 0;
    bool fAll = FALSE;
    char arg[ MAX_INPUT_LENGTH ];

    argument = one_argument( argument, arg );
    if ( ( victim = get_char_room( ch, arg, FALSE ) ) == NULL )
	return;

    one_argument( argument, arg );
    if ( !str_cmp( arg, "all" ) )
	fAll = TRUE;
    else if ( !is_number( arg ) )
    {
	bug ( "MpRemove: Invalid object from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    else
	vnum = atoi( arg );

    for ( obj = victim->carrying; obj; obj = obj_next )
    {
	obj_next = obj->next_content;
	if ( fAll || obj->pIndexData->vnum == vnum )
	{
             if ( fAll ) player_quitting = TRUE;
	     unequip_char( ch, obj );
	     obj_from_char( obj );
	     extract_obj( obj );
             if ( fAll ) player_quitting = FALSE;
	}
    }
}

void do_mpgrab( CHAR_DATA* ch, char* argument )
{
  CHAR_DATA* victim;
  OBJ_DATA* obj;
  sh_int vnum = 0;
  char arg[ MAX_INPUT_LENGTH ];
/* Kesavaram's old code
  bool obj_found = FALSE;

  argument = one_argument( argument, arg );
  victim = get_char_room( ch, arg, FALSE );
  if( victim == NULL )
    return;

  argument = one_argument( argument, arg );
  if( !is_number( arg ) )
    {
      bug( "MpTake: Invalid object from vnum %d.",
	   IS_NPC( ch ) ? ch->pIndexData->vnum : 0 );
      return;
    }

  vnum = atoi( arg );

  obj = victim->carrying;

  while( obj && !obj_found )
    {
      if( obj->pIndexData->vnum == vnum )
	{
	  unequip_char( ch, obj );
	  obj_from_char( obj );
	  extract_obj( obj );
	  obj_found = TRUE;
	}
      else
	{
	  obj = obj->next_content;
	}

    } */

    argument = one_argument( argument, arg );
    
    if( (victim = get_char_room( ch, arg, FALSE )) == NULL )
        return;

    if ( !is_number( argument ) )
    {
        if ( ( obj = get_obj_carry( victim, argument, ch, FALSE ) ) == NULL )
        {
            bug( "do_mpgrab: invalid named object from mob %d",
                IS_NPC( ch ) ? ch->pIndexData->vnum : 0 );
            return;
        }
    }
    else
    {
        vnum = atoi( argument );
        for ( obj = victim->carrying; obj; obj = obj->next_content )
            if ( obj->wear_loc == WEAR_NONE && obj->pIndexData->vnum == vnum )
                break;
        if ( !obj )
        {
            bug( "do_mpgrab: invalid object from mob %d",
                IS_NPC( ch ) ? ch->pIndexData->vnum : 0 );
            return;

        }
    }

    obj_from_char( obj );
    extract_obj( obj );

}

void do_mpput( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* victim;
    OBJ_DATA* obj;
    sh_int vnum = 0;
    char arg[ MAX_INPUT_LENGTH ];

    argument = one_argument( argument, arg );
    
    if( (victim = get_char_room( ch, arg, FALSE )) == NULL )
        return;

    if ( !is_number( argument ) )
    {
        if ( ( obj = get_obj_carry( ch, argument, ch, FALSE ) ) == NULL )
        {
            bug( "do_mpput: invalid named object from mob %d",
                IS_NPC( ch ) ? ch->pIndexData->vnum : 0 );
            return;
        }
    }
    else
    {
        vnum = atoi( argument );
        for ( obj = ch->carrying; obj; obj = obj->next_content )
            if ( obj->wear_loc == WEAR_NONE && obj->pIndexData->vnum == vnum )
                break;
        if ( !obj )
        {
            bug( "do_mpput: invalid object from mob %d",
                IS_NPC( ch ) ? ch->pIndexData->vnum : 0 );
            return;
        }
    }

    obj_from_char( obj );
/*  Kind of a hacky way to handle objects with on-load timers */
    if ( IS_NPC(victim) )
      obj_to_char_init( obj, victim );
    else
      obj_to_char( obj, victim );

    return;
}

void do_mpresetprand( CHAR_DATA *ch, char *argument )
{
    ch->prand = -1;
}

void do_mppeace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
        if ( rch->fighting == ch || ch->fighting == rch )
            stop_fighting( rch, TRUE );

    return;
}

/* mob recogon/recogoff for mp recog controls
 * Syntax: mob recogon <$n/target> <set vnum> <flag> [timer [units]]
 * For future, maybe "recogon $n <flag alias>
 */
void do_mprecogon( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    MPRECOG_DATA *mpr;
    INT_LIST *mprt;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    sh_int vnum;
    int bit, timer;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2);

    if( (victim = get_char_room_seriously( ch, arg, FALSE )) == NULL )
        return;

    argument = one_argument( argument, arg );

/*  For MPRECOG flag aliases */
    if ( !is_number( arg2 ) )
    {
    }
/*  Specifying both set number and flag */
    else
    {
        vnum = atoi(arg2); /* "vnum" = set vnum */
                           /* "arg"  = flag */
        if ((bit = flag_lookup( arg, mprecog_flags )) == -99)
        {
            bug( "do_mprecogon: bad flag from mob %d",
              IS_NPC( ch ) ? ch->pIndexData->vnum : 0 );
            return;
        } 

        for ( mpr = victim->mprecog; mpr; mpr = mpr->next )
        {
            if ( mpr->vnum == vnum ) break;
        } 
        if ( mpr != NULL ) /* if mpr rolodex already exists... */
        {
            SET_BIT(mpr->flags, bit);
        }
        else               /* otherwise create new rolodex, set bit */
        {
            mpr = victim->mprecog;
            victim->mprecog = new_mprecog();
            victim->mprecog->next = mpr;
            victim->mprecog->vnum = vnum;
            victim->mprecog->flags = bit;
            victim->mprecog->temp = FALSE;
            mpr = victim->mprecog;
        }
        if ( !IS_NPC(ch) ) send_to_char("Recog set.\n\r", ch );

        /* Check to see if a timer should be added */
        argument = one_argument( argument, arg );
        if ( !IS_NULLSTR(arg) ) 
        {
            timer = atoi(arg);
            if ( timer <= 0 )
            {
                bug( "do_mprecogon: given bad timer duration from mob %d",
                  IS_NPC( ch ) ? ch->pIndexData->vnum : 0 );
                return;
            }
            argument = one_argument( argument, arg );
            if ( !IS_NULLSTR( arg ) ) 
            {
                if ( arg[0] == 'd' )
                    timer *= 86400;
                else if ( arg[0] == 'h' )
                    timer *= 3600;
                else if ( arg[0] == 'm' )
                    timer *= 60;
                else if ( !str_prefix( arg, "realdays" ) )
                    timer = current_time + timer * 86400;
                else if ( !str_prefix( arg, "realhours" ) )
                    timer = current_time + timer * 3600;
                else if ( !str_prefix( arg, "realminutes" ) )
                    timer = current_time + timer * 60;
                else if ( arg[0] != 's' )
                    bug( "do_mprecogon: got a bad time unit from mob %d",
                  IS_NPC( ch ) ? ch->pIndexData->vnum : 0 );
            }
            /* Find an existing timer to reset if exists */
            for ( mprt = mpr->timer; mprt != NULL; mprt = mprt->next )
                if ( mprt->value[0] == bit ) break;
            /* Otherwise, just make a new timer */
            if ( mprt == NULL )
            {
                mprt = new_int_list();
                mprt->next = mpr->timer;
                mpr->timer = mprt;
                mprt->value[0] = bit;
            }
            mprt->value[1] = timer;
        }
    }

    return;
}
void do_mprecogoff( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    MPRECOG_DATA *mpr;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    sh_int vnum;
    int bit;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if( (victim = get_char_room_seriously( ch, arg, FALSE )) == NULL )
        return;

    one_argument( argument, arg );
/*  For MPRECOG flag aliases */
/*  mob recog $n hasballfromdragon       */
/*            arg      arg2          arg */
    if ( !is_number( arg2 ) )
    {
    }
/*  Specifying both set number and flag */
/*  mob recog $n  1200  A */
/*            arg arg2 arg */
    else
    {
        vnum = atoi(arg2); /* "vnum" = set vnum */
                          /* "arg"  = flag */
        if ((bit = flag_lookup( arg, mprecog_flags )) == -99)
        {
            bug( "do_mprecogoff: bad flag from mob %d",
              IS_NPC( ch ) ? ch->pIndexData->vnum : 0 );
            return;
        } 

        for ( mpr = victim->mprecog; mpr; mpr = mpr->next )
        {
            if ( mpr->vnum == vnum ) break;
        } 
        if ( mpr != NULL ) /* mpr rolodex already exists */
        {
            INT_LIST *mprt, *mprt_next, *mprt_prev = NULL;
            /* remove the recog itself */
            REMOVE_BIT(mpr->flags, bit);
            if ( !IS_NPC(ch) ) send_to_char("Removed bit.\n\r", ch );

            /* also strip off any existing timers */
            for ( mprt = mpr->timer; mprt != NULL; mprt = mprt_next )
            {
                mprt_next = mprt->next;
                if ( mprt->value[0] == bit )
                {
                    /* Unlink the timer from the list */
                    if ( mprt_prev == NULL ) 
                        mpr->timer = mprt_next;
                    else
                        mprt_prev->next = mprt_next;
                    free_int_list( mprt );
                    continue;
                }
                mprt_prev = mprt;
            }
        }
        else /* Wasn't set in the first place, so why waste memory? */
        {
            /* Do nothing */
            if ( !IS_NPC(ch) ) send_to_char("Couldn't find bit.\n\r", ch );
        }
    }

    return;
}

void do_mpgiveexp( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char arg[ MAX_INPUT_LENGTH ];
    char buf[ MAX_STRING_LENGTH ];
    int numa, expg;

    argument = one_argument( argument, arg );
    if ( IS_NULLSTR(arg)
    || ( vch = get_char_room_seriously(ch, arg, FALSE) ) == NULL 
    || IS_NPC(vch) )
    {
	bug( "MpGiveExp: bad ch from mob %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }
    argument = one_argument( argument, arg );
    if ( IS_NULLSTR( arg ) )
    {
	bug( "MpGiveExp: NULL arg2 from mob %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }

    if ( !is_number(arg) ) /* specifying a mob-level based kill */
    {
        numa = atoi(argument);
        if ( numa < 0 || numa > MAX_LEVEL )
        {
            bug( "MpGiveExp: bad level from mob %d.",
                IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
            return;
        }
        expg = xp_compute( vch, numa, numa, 1 );
    }
    else
    {
        numa = atoi(arg);
        if ( numa < 0 )
        {
            bug( "MpGiveExp: bad amount from mob %d.",
                IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
            return;
        }
        expg = numa; 
    }

    sprintf( buf, "You receive %d experience points.\n\r", expg );
    send_to_char( buf, vch );

    gain_exp( vch, expg );

    return;
}

void do_mpboostskill( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char arg[ MAX_INPUT_LENGTH ];
    char buf[ MAX_STRING_LENGTH ];
    int value, sn, current;

    argument = one_argument( argument, arg );
    if ( IS_NULLSTR(arg)
    || ( vch = get_char_room_seriously(ch, arg, FALSE) ) == NULL 
    || IS_NPC(vch) )
    {
	bug( "MpBoostSkill: bad ch from mob %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }

    argument = one_argument( argument, arg );
    if ( IS_NULLSTR( arg ) || ( sn = skill_lookup( arg ) ) < 0 )
    {
	bug( "MpBoostSkill: unknown skill from mob %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }

    argument = one_argument( argument, arg );
    if ( IS_NULLSTR( arg ) 
    || !is_number( arg )
    || ( value = atoi( arg ) ) < 1
    || value > 100 )
    {
	bug( "MpBoostSkill: invalid skill increase from mob %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }
    
    if ( (current = vch->pcdata->learned[sn]) > 0
    &&   vch->level >= skill_table[sn].skill_level[vch->class]
    &&   current < 100 )
    {
        vch->pcdata->learned[sn] = UMIN(current+value, 100 );
        sprintf(buf,"You have become better at %s!\n\r", skill_table[sn].name);
        send_to_char(buf,vch);
    }

    return;
}

void do_mpsetskill( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char arg[ MAX_INPUT_LENGTH ];
    char buf[ MAX_STRING_LENGTH ];
    int value, sn, current;
    bool isspell;

    argument = one_argument( argument, arg );
    if ( IS_NULLSTR(arg)
    || ( vch = get_char_room_seriously(ch, arg, FALSE) ) == NULL 
    || IS_NPC(vch) )
    {
	bug( "MpSetSkill: bad ch from mob %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }

    argument = one_argument( argument, arg );
    if ( IS_NULLSTR( arg ) || ( sn = skill_lookup( arg ) ) < 0 )
    {
        sprintf(buf, "MpSetSkill: unknown skill (%s) from mob %d.",
            arg, IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	bug( buf, 0 );
        return;
    }

    argument = one_argument( argument, arg );
    if ( IS_NULLSTR( arg ) 
    || !is_number( arg )
    || ( value = atoi( arg ) ) < 0
    || value > 100 )
    {
        sprintf(buf, "MpSetSkill: invalid skill setting (%s) from mob %d.",
            arg, IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
          
	bug( buf, 0 );
        return;
    }

/*  if vch's class does not get the skill */
    if ( skill_table[sn].skill_level[vch->class] > LEVEL_HERO )
    {
        sprintf(buf, 
            "MpSetSkill: valid skill (%s) on invalid class (%s) from mob %d.",
             skill_table[sn].name, 
             class_table[vch->class].name,
             IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	bug( buf, 0 );
        return;
    }
    
    current = vch->pcdata->learned[sn];
    vch->pcdata->learned[sn] = value;
    if ( skill_table[sn].spell_fun == spell_null )
        isspell = FALSE;
    else
        isspell = TRUE;


    buf[0] = '\0';
    if ( current == 0 && value > current )
    {
        if ( skill_table[sn].skill_level[vch->class] > vch->level )
            sprintf(buf,"You have gained the potential to %s %s!\n\r",
              ( isspell ? "cast" : "use" ),
              skill_table[sn].name);
        else
            sprintf(buf,"You have learned how to %s %s!\n\r",
              ( isspell ? "cast" : "use" ),
              skill_table[sn].name);
    }
    else if ( current < value )
    {
        if ( skill_table[sn].skill_level[vch->class] > vch->level )
            sprintf(buf,"Your potential to %s %s has improved!\n\r", 
                ( isspell ? "cast" : "use" ),
                skill_table[sn].name);
        else
            sprintf(buf,"You have become better at %s!\n\r", 
                skill_table[sn].name);
    }
    else if ( value == 0 ) 
    {
        if ( skill_table[sn].skill_level[vch->class] > vch->level )
            sprintf(buf,"You have lost the potential to learn %s!\n\r", 
                skill_table[sn].name);
        else
            sprintf(buf,"You have lost all knowledge of how to %s %s!\n\r", 
                ( isspell ? "cast" : "use" ),
                skill_table[sn].name);
    }
    else if ( current > value )
    {
        /* this should only happen in odd cases of de-advancement */
        if ( skill_table[sn].skill_level[vch->class] > vch->level )
            sprintf(buf,"Your potential to %s %s has diminished!\n\r", 
                ( isspell ? "cast" : "use" ),
                skill_table[sn].name);
        else
            sprintf(buf,"You have become worse at %s!\n\r", 
                skill_table[sn].name);
    }
/*  else, value == current and no change */

    if ( !IS_NULLSTR(buf) )
        send_to_char(buf,vch);

    return;
}

void do_mpteleport( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *pRoomIndex;

    if ( IS_NULLSTR(argument)
    || ( victim = get_char_room(ch, argument, FALSE) ) == NULL )
    {
	bug( "MpTeleport: bad vch from mob %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }

  /* code to prevent certain target rooms */
    pRoomIndex = get_random_room(victim);

    if (victim != ch)
	send_to_char("You have been teleported!\n\r",victim);

    act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
}

/* Works just like the tell command, but for mobs.  Less safety checking; the
   command is still mostly verbose on errors to help with switched mprog
   debugging. */
void do_mptell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        bug( "mptell: no target/message specified on mob vnum %d",
            IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }

    if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
        bug( "mptell: cannot find tell target on mob vnum %d",
            IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
        act("$N seems to have misplaced $S link...try again later.",
            ch,NULL,victim,TO_CHAR);
        sprintf(buf,"%s tells you {g'%s'{x\n\r",PERS2(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
        return;
    }

    if ( !(IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL) && !IS_AWAKE(victim) )
    {
        act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
        return;
    }
  
/*  Note: mob tells go through deaf mode just in case */

    if (IS_SET(victim->comm,COMM_AFK))
    {
        if (IS_NPC(victim))
        {
            act("$E is AFK, and not receiving tells.",ch,NULL,victim,TO_CHAR);
            return;
        }

        act("$E is AFK, but your tell will go through when $E returns.",
            ch,NULL,victim,TO_CHAR);
        sprintf(buf,"%s tells you {%c'%s'{x\n\r",PERS2(ch,victim),
            victim->colors[C_TELL],argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
        return;
    }

    sprintf( buf, "You tell %s {%c'%s'{x\n\r", PERS2(victim, ch),
            ch->colors[C_TELL], argument);
    send_to_char(buf,ch);
    sprintf( buf, "%s tells you {%c'%s'{x\n\r", PERS2(ch, victim),
            victim->colors[C_TELL], argument);
    buf[0] = UPPER(buf[0]);
    send_to_char(buf,victim);
    victim->reply	= ch;

    return;
}

void do_mpbson( CHAR_DATA *ch, char *argument )
{
    SET_BIT( ch->affected_by2, AFF2_BLINDSIGHT );
    return;
}
void do_mpbsoff( CHAR_DATA *ch, char *argument )
{
    REMOVE_BIT( ch->affected_by2, AFF2_BLINDSIGHT );
    return;
}

void do_mpzechoaround( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char arg[ MAX_INPUT_LENGTH ];

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR( arg ) || IS_NULLSTR( argument ) )
    {
	bug( "mpzechoaround: missing argument from vnum %d",
	    IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    if ( ( victim = get_char_room( ch, arg, FALSE ) ) == NULL )
    {
        bug( "mpzechoaround: cannot find person to echo around vnum %d",
            IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }

    if ( ch->in_room == NULL )
	return;

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING 
	&&   d->character->in_room != NULL 
        &&   d->character != victim
	&&   d->character->in_room->area == ch->in_room->area )
 	{
	    if ( IS_IMMORTAL(d->character) )
		send_to_char( "Mob echo> ", d->character );
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r", d->character );
	}
    }
    return;
}

void do_mpsreset( CHAR_DATA *ch, char *argument )
{
    if ( ch->in_room )
        reset_room_new( ch->in_room, TRUE );
    return;
}

void do_mpalert( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    
    sprintf(buf, "%s(%d) in room %d: %s",
    ( IS_NPC( ch ) ? ch->short_descr : ch->name ),
    ( IS_NPC( ch ) ? ch->pIndexData->vnum : 0 ),
    ( ch->in_room ? ch->in_room->vnum : 0 ),
      argument );
    wiznet(buf,ch,NULL,WIZ_MOBALERTS,0,0);   
    return;
}

void do_mpbankroll( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int amount;
    int maxtrans = MAX_MONEY_TRANSFER;
    bool isgold = TRUE;

    argument = one_argument( argument, arg );
    
    if ( !str_cmp( argument, "silver" ) )
    {
        isgold = FALSE;
        maxtrans = maxtrans * 100;
    }

    if ( (amount = atoi(arg)) < 0 )
    {
        bug( "mpbankroll: mob %d trying to get negative gold",
            IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;

    }

    if ( amount > maxtrans )
    {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf, "%s bankrolled %d %s room %d.",
            IS_NPC( ch ) ? ch->short_descr : ch->name, 
            amount, ( isgold ? "gold" : "silver" ),
            ( ch->in_room ) ? ch->in_room->vnum : -1 );
        wiznet( buf, NULL, NULL, WIZ_COMMERCE, 0, 0 );
        log_string( buf );
    }
    if ( isgold ) ch->gold = amount;
    else ch->silver = amount;

    return;
}

void do_mpsecho(CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char *letter,*name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;
 
    if ( argument[0] == '\0' )
    {
	bug( "mpsecho: missing argument from vnum %d",
	    IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }

    if ( ch->in_room == NULL )
	return;
    
    if ( ch->desc != NULL )
    {
      send_to_char(argument,ch);
      send_to_char("\n\r",ch);
    }
 
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

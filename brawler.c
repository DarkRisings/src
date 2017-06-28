/******************************************************************************
 *                                                                            *
 *   Brawler scoreborad and support code                                      *
 *   Written October 2011 by Glenn K. Lockwood                                * 
 *   for the Dark Risings codebase                                            *
 *                                                                            *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"
#include "brawler.h"
#include "recycle.h"

/*
 * Global variables 
 */
BRAWLER_DATA *brawler_list;
time_t last_save;

/*
 * Prototypes
 */ 
BRAWLER_DATA    *new_brawler( char *name );
BRAWLER_DATA    *new_brawler_data( void );
FIGHT_RECORD    *new_fight_record( void );

void free_brawler_data( BRAWLER_DATA *p );
void free_fight_record( FIGHT_RECORD *p );

void brawler_win( BRAWLER_DATA *winner, BRAWLER_DATA *loser );
void sort_brawlers( void );

/*******************************************************************************
 *
 *  Player interfaces to the brawler scoreboard
 *
 ******************************************************************************/

void do_scoreboard_hist( CHAR_DATA *ch, char *argument)
{
    return;
}

void do_brawllist( CHAR_DATA *ch, char *argument )
{
    FIGHT_RECORD *fr;
    BRAWLER_DATA *pBrawler;
    BUFFER* buffer;
    int count = 0;
    char buf[MAX_STRING_LENGTH], arg[MAX_INPUT_LENGTH];
    char buf2[256];
    struct tm *ltime;
    const char *outcomestr[] = {
      "Indeterminate",  /* FIGHT_UNKNOWN */
      "Win",            /* FIGHT_WIN     */
      "Loss",           /* FIGHT_LOSS    */
      "Wimp out"        /* FIGHT_WIMP    */
    };

/* Only brawlers can check out each others' stats */
    if ( !IS_BRAWLER(ch) && !IS_IMMORTAL(ch) )
    {
        send_to_char( "You aren't a Brawler.\n\r", ch );
        return;
    }

    one_argument(argument, arg);

    if ( !IS_NULLSTR( arg ) )
        pBrawler = get_brawler( arg );
    else
    {
        if ( IS_NPC(ch) )
        {
            send_to_char( "NPCs aren't Brawlers.\n\r", ch );
            return;
        }
        if ( (pBrawler = ch->pcdata->brawler) == NULL )
        {
            send_to_char( "You have no brawling record yet.\n\r", ch );
            return;
        }
    }

    if ( pBrawler == NULL )
    {
        send_to_char( "There are no records of a brawler with that name.\n\r", 
            ch );
        return;
    }

    if ( pBrawler->fights == NULL )
    {
        if ( !str_cmp( pBrawler->name, ch->name ) )
            send_to_char( "You have no brawls on record.\n\r", ch );
        else
        {
            sprintf(buf, "%s has no brawls on record.\n\r", pBrawler->name );
            buf[0] = UPPER(buf[0]);
            send_to_char( buf, ch );
        }
        return;
    }

    buffer = new_buf();

    sprintf( buf, "%s's Brawls:\n\r", pBrawler->name );
    buf[0] = UPPER(buf[0]);
    add_buf( buffer, buf );
    for ( fr = pBrawler->fights; fr != NULL; fr = fr->next )
    {
        ltime = localtime( &(fr->timestamp) );
        strftime( buf2, 256, "%b %e %Y %I:%M %p", ltime );
        sprintf( buf, "[%3d] %21s: %s against %s\n", ++count, buf2, 
            outcomestr[fr->outcome], (fr->opponent?fr->opponent->name:"?") );
        add_buf( buffer, buf );
        if ( count > 100 ) 
            break;
    }
    if ( pBrawler->banuntil > 0 )
    {
        ltime = localtime( &(pBrawler->banuntil) );
        strftime( buf2, 256, "%b %e %Y %I:%M %p", ltime );
        sprintf( buf, "%s is banned from Brawler until %s.\n\r", 
            pBrawler->name, buf2 );
        add_buf( buffer, buf );
    }
    page_to_char( buf_string(buffer), ch );
    free_buf(buffer);

    return;
}

void do_scoreboard( CHAR_DATA *ch, char *argument )
{
    BRAWLER_DATA *pBrawler;
    BUFFER* buffer;
    char buf[MAX_STRING_LENGTH];
    char buf1[256], buf2[256], buf3[256];
    int rank = 0;

    buffer = new_buf();

    sprintf( buf, "%5s %-12s %8s %8s %12s %12s %12s\n\r",
        "Rank",
        "Name",
        "W/L*",
        "W/L",
        "Wins",
        "Losses",
        "Wimpouts" );
    add_buf( buffer, buf );

    for ( pBrawler = brawler_list; pBrawler != NULL; pBrawler=pBrawler->next )
    {
        if ( IS_SET( pBrawler->flags, BFLAG_HIDE ) )
            continue;

        sprintf( buf1, "%d(%d)", pBrawler->score[BRAWL_WINS], 
            pBrawler->score[BRAWL_UNIQUE_WINS] );
        sprintf( buf2, "%d(%d)", pBrawler->score[BRAWL_LOSSES], 
            pBrawler->score[BRAWL_UNIQUE_LOSSES] );
        sprintf( buf3, "%d(%d)", pBrawler->score[BRAWL_WIMPS], 
            pBrawler->score[BRAWL_UNIQUE_WIMPS] );

        sprintf( buf, "%4d. %-12s %8.3f %8.3f %12s %12s %12s\n\r",
            ++rank,
            pBrawler->name, 
            (float)pBrawler->score[BRAWL_UNIQUE_WINS] / 
                ( pBrawler->score[BRAWL_UNIQUE_LOSSES] == 0 ? 1.0 : 
                ((float)pBrawler->score[BRAWL_UNIQUE_LOSSES]) ),
            (float)pBrawler->score[BRAWL_WINS] / 
                ( pBrawler->score[BRAWL_LOSSES] == 0 ? 1.0 : 
                ((float)pBrawler->score[BRAWL_LOSSES]) ),
            buf1,
            buf2,
            buf3 );
        add_buf( buffer, buf );
    }
    page_to_char( buf_string(buffer), ch );
    free_buf(buffer);

    return;
}

void do_victory( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    BRAWLER_DATA *winner, *loser;

    one_argument( argument, arg );

    if ( IS_NPC(ch) )
    {
        send_to_char( "NPCs cannot claim victory over anyone.\n\r", ch );
        return;
    }

    if ( !IS_BRAWLER(ch) )
    {
        send_to_char( "Only Brawlers can do that.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED2(ch, AFF_GHOST) )
    {
        send_to_char( "You're a ghost!\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
        sort_brawlers(); /* secret sort switch */
        send_to_char( "Claim victory over whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg, TRUE ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch || IS_NPC(victim) )
    {
        send_to_char( "You can't do that.\n\r", ch );
        return;
    }

    if ( !IS_BRAWLER(victim) )
    {
        send_to_char( "You can only claim victory over other Brawlers.\n\r",
            ch );
        return;
    }

    if ( victim->position != POS_UNCONSCIOUS )
    {
        act( "You must render $M unconscious first.", 
            ch, NULL, victim, TO_CHAR );
        return;
    }

    /* attach brawler data for winner and loser */
    winner = ch->pcdata->brawler;
    loser = victim->pcdata->brawler;
    if ( winner == NULL )
    {
        winner = new_brawler( ch->name );
        ch->pcdata->brawler = winner;
    }
    if ( loser == NULL )
    {
        loser = new_brawler( victim->name );
        victim->pcdata->brawler = loser;
    }

    if ( (current_time - loser->lastbrawl) < PULSE_TICK/PULSE_PER_SECOND )
    {
        act( "Someone has already claimed victory over $M!", ch, NULL,
            victim, TO_CHAR );
        return;
    }

    brawler_win( winner, loser );
        
    /* output text to all involved parties */
    /* to do: consider adding a brawler-wide global echo */
    act( "You claim victory over $N!",  ch, NULL, victim, TO_CHAR    );
    act_new( "$n has claimed victory over you!", 
        ch, NULL, victim, TO_VICT, POS_DEAD, 0 );
    act( "$n claims victory over $N!",  ch, NULL, victim, TO_NOTVICT );

    sprintf( buf, "%s claims victory over %s at %d",
        ch->name, victim->name, ch->in_room->vnum );
    log_string( buf );
    wiznet( buf, NULL, NULL, WIZ_DEATHS, 0, 0 );

    return;
}

void do_clobber( CHAR_DATA *ch, char *argument )
{
    BRAWLER_DATA *brawler1, *brawler2;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    if ( IS_NPC(ch) )
    {
        send_to_char( "NPCs cannot claim victory over anyone.\n\r", ch );
        return;
    }

    if ( !IS_BRAWLER(ch) )
    {
        send_to_char( "Only Brawlers can do that.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED2(ch, AFF_GHOST) )
    {
        send_to_char( "You're a ghost!\n\r", ch );
        return;
    }
    one_argument( argument, arg );

    if ( IS_NULLSTR( arg ) )
    {
        send_to_char( "Who would you like to brawl?\n\r", ch );
        return;
    }
    if ( (victim = get_char_room(ch,arg, TRUE)) == NULL )
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }
    else if ( victim == ch )
    {
        send_to_char( "Not much of a challenge, is there?\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) || !IS_BRAWLER(victim) )
    {
        send_to_char( "You can only clobber other Brawlers.\n\r",
            ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
        send_to_char( "You shouldn't be clobbering your beloved master.\n\r", 
            ch );
        return;
    }

    if ( is_safe( ch, victim ) )
        return;

    if ( (current_time - ch->pcdata->lastpk) < FIGHT_LAG )
    {
        send_to_char( "You're already fighting someone!\n\r", ch );
        return;
    }
    else if ( (current_time - victim->pcdata->lastpk) < FIGHT_LAG )
    {
        act( "$E is already fighting someone!", ch, NULL, victim, TO_CHAR );
        return;
    }
    else if ( ch->pcdata->brawling != NULL )
    {
        send_to_char( "You are already brawling!\n\r", ch );
        return;
    }
    else if ( victim->pcdata->brawling != NULL )
    {
        act( "$E is already brawling!", ch, NULL, victim, TO_CHAR );
        return;
    }

    /* attach brawler data */
    brawler1 = ch->pcdata->brawler;
    brawler2 = victim->pcdata->brawler;
    if ( brawler1 == NULL )
    {
        brawler1 = new_brawler( ch->name );
        ch->pcdata->brawler = brawler1;
    }
    if ( brawler2 == NULL )
    {
        brawler2 = new_brawler( victim->name );
        victim->pcdata->brawler = brawler2;
    }

    ch->pcdata->brawling = brawler2;
    victim->pcdata->brawling = brawler1;

    act( "You clobber $M with a flurry of blows!  The brawl is on!",
        ch, NULL, victim, TO_CHAR );
    act( "$n clobbers you with a flurry of blows!  The brawl is on!", 
        ch, NULL, victim, TO_VICT );
    act( "$n clobbers $N with a flurry of blows!  The brawl is on!",
        ch, NULL, victim, TO_NOTVICT );
    /* to do: consider adding a brawler-wide global echo */

    WAIT_STATE( ch, PULSE_VIOLENCE );
    check_killer( ch, victim );
    /* clobber 'em! */
    multi_hit( ch, victim, TYPE_UNDEFINED );
    multi_hit( ch, victim, TYPE_UNDEFINED );

    sprintf( buf, "%s clobbers %s and starts a brawl at %d", 
        ch->name, victim->name,
        ( ch->in_room->vnum ? ch->in_room->vnum : 0 ) );
    wiznet( buf, NULL, NULL, WIZ_DEATHS, 0, 0 );

    return;
}

/*
 * Commands to join/leave brawler
 */
void do_joinbrawle( CHAR_DATA* ch, char* argument )
{
    send_to_char( "You must type the full command to join the club.\n\r", ch );
    return;
}
void do_joinbrawler( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
        send_to_char( "Only players can join the Brawler club.\n\r", ch );
        return;
    }

    if ( !IS_IMMORTAL( ch ) && in_fightlag(ch) )
    {
        send_to_char( "Wait until you've calmed down first.\n\r", ch );
        return;
    }

    if ( IS_SET( ch->act_bits, PLR_NEWBIE ) )
    {
        send_to_char( "Newbies cannot be brawlers.\n\r", ch );
        return;
    }

    if ( ch->level < 20 )
    {
        send_to_char( "Join after you've completed more of your training.\n\r",
            ch );
        return;
    }

    if ( IS_SET( ch->act_bits, PLR_BRAWLER ) )
    {
        if( IS_IMMORTAL( ch ) )
        {
            send_to_char( "Brawler status removed.\n\r", ch );
            REMOVE_BIT( ch->act_bits, PLR_BRAWLER );
        }
        else
        {
            send_to_char("If you want to leave the club, use leavebrawler.\n\r",
                ch );
        }
    }
    else
    {
        if ( ch->pcdata->brawler != NULL )
        {
            char buf2[256];
            struct tm *ltime;
            if ( current_time < ch->pcdata->brawler->banuntil )
            {
                ltime = localtime( &(ch->pcdata->brawler->banuntil) );
                strftime( buf2, 256, "%b %e %Y %I:%M %p", ltime );
                sprintf( buf,
                    "You are forbidden from rejoining Brawler until %s.\n\r",
                    buf2 );
                send_to_char( buf, ch );
                return;
            }
            REMOVE_BIT( ch->pcdata->brawler->flags, BFLAG_HIDE );
        }

        SET_BIT( ch->act_bits, PLR_BRAWLER );
        REMOVE_BIT(ch->comm,COMM_NOBRAWLER);
        send_to_char( "Welcome to the Brawler club!\n\r", ch );
        /* to do: consider adding a brawler-wide global echo */
    }
    return;
}

void do_leavebrawle( CHAR_DATA* ch, char* argument )
{
    send_to_char( "You must type the full command to leave the club.\n\r", ch );
    return;
}

void do_leavebrawler( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
        send_to_char( "You're dumb.\n\r", ch );
        return;
    }

    if ( !IS_SET( ch->act_bits, PLR_BRAWLER ) )
    {
        send_to_char( "You aren't a member of Brawler to begin with.\n\r", ch );
        return;
    }

    if ( !IS_IMMORTAL( ch ) && in_fightlag(ch) )
    {
        send_to_char( "Wait until you've calmed down first.\n\r", ch );
        return;
    }

    if ( str_cmp(argument, ch->name) )
    {
        sprintf( buf,
            "Leaving brawler will wipe your name and scores off of the scoreboard, and you\n\rwill be banned from re-joining for thirty days.  If you are sure you want to\n\rleave brawler, type: leavebrawler %s\n\r", 
            ch->name );
        send_to_char( buf, ch );
        return;
    }

    send_to_char("Like the lowest of cowards, you have left Brawler.\n\r", ch);
    /* to do: consider adding a brawler-wide global echo */

    REMOVE_BIT( ch->act_bits, PLR_BRAWLER );
    SET_BIT( ch->comm, COMM_NOBRAWLER );

    if ( ch->pcdata->brawler == NULL )
        ch->pcdata->brawler = new_brawler( ch->name );

    SET_BIT( ch->pcdata->brawler->flags, BFLAG_HIDE);
    ch->pcdata->brawler->banuntil = current_time + 86400 * 30;

    return;
}

void do_brawlerkick( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    
    argument = one_argument( argument, arg );
    if ( IS_NULLSTR( arg ) )
    {
        send_to_char( "Syntax: brawlerkick <character>\n\r", ch );
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

    if ( !IS_SET( victim->act_bits, PLR_BRAWLER ) )
    {
        act( "$E isn't a member of Brawler to begin with.", 
            ch, NULL, victim, TO_CHAR );
        return;
    }

    send_to_char("You have been thrown out of the Brawler club!\n\r", victim );
    act( "$E has been removed from Brawler with prejudice.", 
        ch, NULL, victim, TO_CHAR );
    /* to do: consider adding a brawler-wide global echo */

    REMOVE_BIT( victim->act_bits, PLR_BRAWLER );
    SET_BIT( victim->comm, COMM_NOBRAWLER );

    if ( victim->pcdata->brawler == NULL )
        victim->pcdata->brawler = new_brawler( victim->name );

    SET_BIT( victim->pcdata->brawler->flags, BFLAG_HIDE);
    victim->pcdata->brawler->banuntil = current_time + 86400 * 30;

    return;
}


/*
 * Brawler channel
 */
void do_brawl( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( !IS_IMMORTAL( ch ) )
    {
        if( !IS_SET( ch->act_bits, PLR_BRAWLER ) )
        {
            send_to_char( "The [Brawl] channel is only for brawlers.\n\r", ch );
            SET_BIT( ch->comm, COMM_NOBRAWLER );
            return;
        }
    }
 
    if (argument[0] == '\0' )
    {
        if (IS_SET(ch->comm,COMM_NOBRAWLER))
        {
            send_to_char("[Brawl] channel is now ON.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_NOBRAWLER);
        }
        else
        {
            send_to_char("[Brawl] channel is now OFF.\n\r",ch);
            SET_BIT(ch->comm,COMM_NOBRAWLER);
        }
    }
    else
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
            send_to_char("You must turn off quiet mode first.\n\r",ch);
            return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
            send_to_char("The gods have revoked your channel priviliges.\n\r",
                ch);
            return;
        }

        if ( IS_SET( ch->act_bits, ACT_NOAPPROVE ) )
        {
            send_to_char( 
           "You must be approved to use this channel.  See 'help approval'\n\r",
                ch );
            return;
        }

        REMOVE_BIT(ch->comm,COMM_NOBRAWLER);
 
        sprintf( buf, "You [Brawl]: {%c'%s'{x\n\r", ch->colors[C_BRAWLER],
            argument );
        send_to_char( buf, ch );

        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            CHAR_DATA *victim;
    
            victim = d->original ? d->original : d->character;
 
            if ( d->connected == CON_PLAYING 
            &&   d->character != ch 
            &&   (IS_IMMORTAL(victim) || IS_SET(victim->act_bits, PLR_BRAWLER) )
            &&   !IS_SET(victim->comm,COMM_NOBRAWLER) 
            &&   !IS_SET(victim->comm,COMM_QUIET) )
            {
                sprintf( buf, "[Brawl] %s: {%c'%s'{x\n\r", 
                    capitalize( PERS2(ch, d->character)),
                    d->character->colors[C_BRAWLER],
                    argument );
                send_to_char(buf,d->character);
            }
        }
    }
    return;
}

/*******************************************************************************
 *
 *  Master routine to sort the scoreboard
 *
 ******************************************************************************/
void sort_brawlers(void)
{
    BRAWLER_DATA *p, *prev, *next;
    float kd1, kd2, k1, k2;
    bool flip;

    p = brawler_list;
    prev = NULL;
    while ( p != NULL && p->next != NULL )
    {
        next = p->next;
        flip = FALSE;

        /* Sort according to unique W/L ratio for now */
        kd1 = ( p->score[BRAWL_UNIQUE_LOSSES] > 0 
            ? (float) p->score[BRAWL_UNIQUE_WINS] / 
              (float) p->score[BRAWL_UNIQUE_LOSSES] 
            : (float) p->score[BRAWL_UNIQUE_WINS] );
        kd2 = ( next->score[BRAWL_UNIQUE_LOSSES] > 0 
            ? (float) next->score[BRAWL_UNIQUE_WINS] / 
              (float) next->score[BRAWL_UNIQUE_LOSSES]
            : (float) next->score[BRAWL_UNIQUE_WINS] );

        /* Non-unique W/L is second metric */
        k1 = ( p->score[BRAWL_LOSSES] > 0 
           ? (float) p->score[BRAWL_WINS] / (float) p->score[BRAWL_LOSSES] 
           : (float) p->score[BRAWL_WINS] );
        k2 = ( next->score[BRAWL_LOSSES] > 0 
           ? (float) next->score[BRAWL_WINS] / (float) next->score[BRAWL_LOSSES]
           : (float) next->score[BRAWL_WINS] );

        if ( kd1 < kd2 )
            flip = TRUE;
        /* additional sorting rules - extraneous, but fun */
        else if ( kd1 == kd2 )
        {
            if ( k1 < k2 )
                flip = TRUE;
            else if ( k1 == k2 )
            {
                if(p->score[BRAWL_UNIQUE_WINS] < next->score[BRAWL_UNIQUE_WINS])
                    flip = TRUE;
                else if ( p->score[BRAWL_UNIQUE_WINS]
                          == next->score[BRAWL_UNIQUE_WINS] )
                {
                    if ( p->score[BRAWL_WINS]
                          < next->score[BRAWL_WINS] )
                        flip = TRUE;
                    else if ( p->score[BRAWL_WINS] == next->score[BRAWL_WINS] )
                    {
                        if ( p->score[BRAWL_UNIQUE_LOSSES] 
                             < next->score[BRAWL_UNIQUE_LOSSES] )
                            flip = TRUE;
                        else if ( p->score[BRAWL_UNIQUE_LOSSES] 
                             == next->score[BRAWL_UNIQUE_LOSSES]
                             && p->score[BRAWL_LOSSES] 
                                > next->score[BRAWL_LOSSES] )
                                flip = TRUE;
                    }
                }
            }
        }

        if ( flip )
        {
            p->next    = next->next;
            next->next = p;
            if ( prev != NULL )
                prev->next = next;
            else
                brawler_list = next;
            /* start over from the beginning */
            prev = NULL;
            p = brawler_list;
        }
        else
        {
            prev = p;
            p = next;
        }
    }

    return;
}

/*******************************************************************************
 *
 * Brawler API
 *
 ******************************************************************************/
void brawler_win( BRAWLER_DATA *winner, BRAWLER_DATA *loser )
{
    FIGHT_RECORD *fr;

    if ( winner == NULL || loser == NULL )
    {
        bug( "brawler_win: NULL winner or loser", 0 );
        return;
    }

    winner->score[BRAWL_WINS]++;
    winner->lastbrawl = current_time;
    loser->score[BRAWL_LOSSES]++;
    loser->lastbrawl = current_time;

    /* look for previous fights with this outcome on the winner */
    for ( fr = winner->fights; fr != NULL; fr = fr->next )
        if ( fr->opponent == loser && fr->outcome == FIGHT_WIN )
            break;
    if ( fr == NULL )
        winner->score[BRAWL_UNIQUE_WINS]++;

    /* look for previous fights with this outcome on the loser */
    for ( fr = loser->fights; fr != NULL; fr = fr->next )
        if ( fr->opponent == winner && fr->outcome == FIGHT_LOSS )
            break;
    if ( fr == NULL )
        loser->score[BRAWL_UNIQUE_LOSSES]++;

    /* create a fight record for the winner */
    fr = new_fight_record();
    fr->opponent = loser;
    fr->outcome = FIGHT_WIN;
    fr->timestamp = current_time;
    fr->next = winner->fights;
    winner->fights = fr;

    /* create a fight record for the loser */
    fr = new_fight_record();
    fr->opponent = winner;
    fr->outcome = FIGHT_LOSS;
    fr->timestamp = current_time;
    fr->next = loser->fights;
    loser->fights = fr;

    sort_brawlers();

    save_brawlers();
    
    return;
}

/*
 *
void brawler_wimp( BRAWLER_DATA *wimp, BRAWLER_DATA *opponent )
{
    FIGHT_RECORD *fr;

    if ( wimp == NULL  | opponent == NULL)
    {
        bug( "brawler_wimp: NULL wimp or opponent", 0 );
        return;
    }

    wimp->score[BRAWL_WIMPS]++;
    wimp->lastbrawl = current_time;

    for ( fr = wimp->fights; fr != NULL; fr = fr->next )
        if ( fr->opponent == loser && fr->outcome == FIGHT_WIMP )
            break;
    if ( fr == NULL )
        wimp->score[BRAWL_UNIQUE_WIMPS]++;

    fr = new_fight_record();
    fr->opponent = opponent;
    fr->outcome = FIGHT_WIMP;
    fr->timestamp = current_time;
    fr->next = wimp->fights;
    wimp->fights = fr;

    fr = new_fight_record();
    fr->opponent = wimp;
    fr->outcome = FIGHT_DRAW;
    fr->timestamp = current_time;
    fr->next = opponent->fights;
    opponent->fights = fr;

    sort_brawlers();

    save_brawlers();
    
    return;
}
 *
 */

BRAWLER_DATA *get_brawler( char *name )
{
    BRAWLER_DATA *pBrawler;
    for (pBrawler = brawler_list; pBrawler != NULL; pBrawler=pBrawler->next)
    {
        if ( !str_cmp( name, pBrawler->name ) )
            return pBrawler;
    }
    return NULL;
}

BRAWLER_DATA *new_brawler( char *name )
{
    BRAWLER_DATA *pBrawler;
    
    /* don't double up on Brawlers by accident */
    if ( !IS_NULLSTR(name) && (pBrawler = get_brawler( name )) != NULL )
        return pBrawler;

    /* tack new brawler record at the bottom of the list to save on sorting */
    pBrawler = brawler_list;
    while ( pBrawler != NULL && pBrawler->next != NULL )
        pBrawler = pBrawler->next;
    if ( pBrawler == NULL )
    { 
        pBrawler = new_brawler_data();
        brawler_list = pBrawler;
    }
    else
    {
        pBrawler->next = new_brawler_data();
        pBrawler = pBrawler->next;
    }
    pBrawler->name = str_dup(name);
    pBrawler->next = NULL;

    return pBrawler;
}

/*******************************************************************************
 *
 *  Memory handling and recycling routines 
 *
 ******************************************************************************/
BRAWLER_DATA *brawler_data_free;
BRAWLER_DATA *new_brawler_data(void)
{
    BRAWLER_DATA *p;
    int i;
    if ( brawler_data_free == NULL )
        p = alloc_perm(sizeof(*p));
    else
    {
        p = brawler_data_free;
        brawler_data_free = p->next;
    }
    /* initialize everything here */
    p->name = &str_empty[0];
    p->fights = NULL;
    for ( i = 0; i < MAX_BRAWLER_SCORETYPES; i++ )
        p->score[i] = 0;
    p->lastbrawl = 0;
    p->banuntil = 0;
    p->flags = 0;

    return p;
}
void free_brawler_data( BRAWLER_DATA *p )
{
    FIGHT_RECORD *fr, *fr_next;

    p->next = brawler_data_free;
    brawler_data_free = p;
    free_string(p->name);
    for ( fr = p->fights; fr != NULL; fr = fr_next )
    {
        fr_next = fr->next;
        free_fight_record( fr );
    }
    return;
}

FIGHT_RECORD *fight_record_free;
FIGHT_RECORD *new_fight_record(void)
{
    FIGHT_RECORD *p;

    if ( fight_record_free == NULL )
        p = alloc_perm(sizeof(*p));
    else
    {
        p = fight_record_free;
        fight_record_free = p->next;
    }
    return p;
}

void free_fight_record( FIGHT_RECORD *p )
{
    p->next = fight_record_free;
    fight_record_free = p;
    return;
}

/*******************************************************************************
 *
 *  Brawler DB I/O code
 *
 ******************************************************************************/
/*
 * fread_string, but without the string allocation
 */
char *fread_string_tmp( FILE *fp )
{
    static char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int readin = 0;

    buf[0] = '\0';
    plast = &buf[0];

    do
    {
        c = getc( fp );
    } while ( isspace(c) );

    if ( (*plast++ = c) == '~' )
        return &buf[0];

    for ( ; ; )
    {
        switch ( *plast = getc(fp) )
        {
            default:
                plast++;
                break;

            case EOF:
                bug( "fread_string_tmp: EOF", 0 );
                return NULL;
                break;
 
            case '\n':
                plast++;
                *plast++ = '\r';
                break;
 
            case '\r':
                break;
 
            case '~':
                *plast = '\0';
                return buf;
        }
        if ( ++readin > MAX_STRING_LENGTH )
        {
            bug( "fread_string_tmp: buffer overflow", 0 );
            break;
        }
    }
    return NULL;
}

/*
 * Read scores for individual brawlers.  Only lasts a month.
 */
void load_brawler_scores( FILE *fp, int version )
{
    char *word;
    BRAWLER_DATA *p;

    for ( ; ; )
    {
        word = fread_word( fp );
        if ( !str_cmp( word, "Brawler" ) )
        {
            p = new_brawler_data();
            p->name = fread_string( fp );
            p->next = brawler_list;
            brawler_list = p;
        }
        else if ( !str_cmp( word, "WinRec" ) )
            fread_to_eol( fp );
        else if ( !str_cmp( word, "LossRec" ) )
            fread_to_eol( fp );
        else if ( !str_cmp( word, "WimpRec" ) )
            fread_to_eol( fp );
        else if ( !str_cmp( word, "ScWin" ) )
            p->score[BRAWL_WINS] = fread_number(fp);
        else if ( !str_cmp( word, "ScLose" ) )
            p->score[BRAWL_LOSSES] = fread_number(fp);
        else if ( !str_cmp( word, "ScWimp" ) )
            p->score[BRAWL_WIMPS] = fread_number(fp);
        else if ( !str_cmp( word, "ScUWin" ) )
            p->score[BRAWL_UNIQUE_WINS] = fread_number(fp);
        else if ( !str_cmp( word, "ScULose" ) )
            p->score[BRAWL_UNIQUE_LOSSES] = fread_number(fp);
        else if ( !str_cmp( word, "ScUWimp" ) )
            p->score[BRAWL_UNIQUE_WIMPS] = fread_number(fp);
        else if ( !str_cmp( word, "LastBrawl" ) )
            p->lastbrawl = (time_t) fread_number(fp);
        else if ( !str_cmp( word, "BanUntil" ) )
            p->banuntil = (time_t) fread_number(fp);
        else if ( !str_cmp( word, "Flags" ) )
            p->flags = fread_flag(fp);
        else if ( !str_cmp( word, "#DONE" ) )
            break;
        else
        {
            char buf[MAX_STRING_LENGTH];
            sprintf( buf, "load_brawler_scores: unknown field '%s'", word );
            bug( buf, 0 );
        }
    }
    return;
}

/*
 * Read brawler scores from the past.  Doesn't do anything right now.
 */
void load_brawler_history( FILE *fp, int version )
{
    char *word;
    for ( ; ; )
    {
        word = fread_word( fp );
        if ( !str_cmp( word, "#DONE" ) )
            break;
        else
            fread_to_eol( fp );
    }

    return;
}

/*
 * Load individual records of fights
 */
void load_brawler_fightrec( FILE *fp, int version )
{
    char *word;
    char name1[MAX_STRING_LENGTH], name2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    FIGHT_RECORD *pFight, *pf;
    BRAWLER_DATA *pBrawler;

    for ( ; ; )
    {
        word = fread_word( fp );
        if ( !str_cmp( word, "#DONE" ) )
            break;
        else if ( word[0] == 'R' )
        {
            bool attached = FALSE;
            pFight = new_fight_record();
            pFight->opponent = NULL;
            strcpy( name1, fread_string_tmp( fp ) );
            strcpy( name2, fread_string_tmp( fp ) );
            pFight->outcome = fread_number( fp );
            pFight->timestamp = (time_t) fread_number( fp );
            for ( pBrawler = brawler_list; pBrawler != NULL; 
                                                     pBrawler = pBrawler->next )
            {
                if ( !str_cmp( name1, pBrawler->name ) )
                {
                    if ( pBrawler->fights == NULL )
                        pBrawler->fights = pFight;
                    else
                    {
                        for ( pf = pBrawler->fights; pf->next != NULL; 
                                                                pf = pf->next );
                        pf->next = pFight;
                    }
                    pFight->next = NULL;
                    attached = TRUE;
                }
                else if ( !str_cmp( name2, pBrawler->name ) )
                    pFight->opponent = pBrawler;
            }
            if ( !attached )
            {
                sprintf( buf, 
                   "load_brawler_fightrec: couldn't find brawler %s (%s vs %s)",
                   name1, name1, name2 );
                bug( buf, 0 );
                free_fight_record( pFight );
            }
            if ( pFight->opponent == NULL )
            {
                sprintf( buf, 
                  "load_brawler_fightrec: couldn't find opponent %s (%s vs %s)",
                  name2, name1, name2 );
                bug( buf, 0 );
            }
        }
        else
        {
            sprintf(buf, "load_brawler_fightrec: Unknown identifier '%s'",
                word );
            bug( buf, 0 );
        }
    }

    return;
}

/*
 * Load brawler data from disk.
 * Just calls other routines to parse the different sections of the brawler file
 */ 
void load_brawler( void )
{
    FILE* fp;
    int vers = -1;

    brawler_list = NULL;
    last_save = 0;
    if ( (fp = fopen( BRAWLER_FILE, "r" )) == NULL )
        return;

    for ( ; ; )
    {
        char *word;
        if ( fread_letter( fp ) != '#' )
        {
            bug( "load_brawler: # not found.", 0 );
            break;
        }

        word = fread_word( fp );

        if      ( !str_cmp( word, "VERSION"  ) ) vers = fread_number(fp);
        else if ( !str_cmp( word, "BRAWLERS" ) ) load_brawler_scores(fp, vers);
        else if ( !str_cmp( word, "HISTORY"  ) ) load_brawler_history(fp, vers);
        else if ( !str_cmp( word, "FIGHTREC" ) ) load_brawler_fightrec(fp,vers);
        else if ( !str_cmp( word, "END" ) )      break;
        else
        {
            bug( "load_brawler: bad section name", 0 );
            break;
        }
    }
    fclose( fp );

    sort_brawlers();

    return;
}

/* 
 * Save brawler records to disk.  
 * Scoreboards are generated during runtime from this data.
 */
#define BRAWLERFILE_VERSION     1
void save_brawlers( void )
{
    FILE* fp;
    BRAWLER_DATA *pBrawler;
    FIGHT_RECORD *pFight;

    fclose( fpReserve );

    if ( ( fp = fopen( BRAWLER_FILE, "w" ) ) == NULL )
    {
        perror( BRAWLER_FILE );
        return;
    }

    fprintf( fp, "#VERSION %d\n\n", BRAWLERFILE_VERSION );

    /* list of active brawlers */
    fprintf( fp, "#BRAWLERS\n" );
    for ( pBrawler = brawler_list; pBrawler != NULL; pBrawler = pBrawler->next )
    {
        fprintf( fp, "Brawler %s~\n", pBrawler->name );
        fprintf( fp, 
        "ScWin %d\nScLose %d\nScWimp %d\nScUWin %d\nScULose %d\nScUWimp %d\n",
            pBrawler->score[BRAWL_WINS],
            pBrawler->score[BRAWL_LOSSES],
            pBrawler->score[BRAWL_WIMPS],
            pBrawler->score[BRAWL_UNIQUE_WINS],
            pBrawler->score[BRAWL_UNIQUE_LOSSES],
            pBrawler->score[BRAWL_UNIQUE_WIMPS] );
        fprintf( fp, "Flags %s\n", print_flags(pBrawler->flags) );
        fprintf( fp, "LastBrawl %d\nBanUntil %d\n\n", 
            (int) pBrawler->lastbrawl, (int) pBrawler->banuntil );
    }
    fprintf( fp, "#DONE\n\n" );

    /* list of fights */
    fprintf( fp, "#FIGHTREC\n" );
    for ( pBrawler = brawler_list; pBrawler != NULL; pBrawler = pBrawler->next )
    {
        for ( pFight = pBrawler->fights; pFight != NULL; pFight = pFight->next )
        {
            if ( pFight->opponent == NULL )
            {
                bug( "save_brawlers: whoa nelly, no opponent", 0 );
            }
            else
            {
                fprintf( fp, "R %s~ %s~ %d %d\n", pBrawler->name, 
                    pFight->opponent->name, pFight->outcome, 
                    (int) pFight->timestamp );
            }
        }
    }
    fprintf( fp, "#DONE\n\n" );

    /* monthly champions */
    fprintf( fp, "\n#HISTORY\n" );
    /* fprintf something here; does nothing for now */
    fprintf( fp, "#DONE\n\n" );

    fprintf( fp, "#END\n" );

    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );

    return;
}

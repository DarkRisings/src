#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"

#define BIG_BLUE_LINE_SINGLE "{D================================================================================{x\n\r"
#define BIG_BLUE_LINE_DOUBLE "{D================================================================================{x\n\r\n\r"

#define MAX_QUESTS_PER_PAGE 30

void journal_read( CHAR_DATA *ch, int page )
{
    QUEST_DATA *pQuest = NULL;
    JOURNAL_DATA *pJournal;
    AREA_DATA *pArea;
    BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    int qcount;
    bool complete = FALSE;
    bool found = FALSE;

    if ( page < 1 )
    {
        send_to_char( "That doesn't even make sense.\n\r", ch );
        return;
    }

    qcount = 0;
    /* seek out the correct quest */
    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        for ( pQuest = pArea->quests; pQuest; pQuest = pQuest->next )
        {
            if ( !pQuest->completevnum )
                continue;
            for ( pJournal = pQuest->journal; pJournal != NULL; 
                                                 pJournal = pJournal->next )
            {
                 if ( check_recog( ch, pJournal->vnum, pJournal->flag ) )
                 {
                     if ( ++qcount == page )
                         found = TRUE;
                     break;
                 }
            }
            if ( found ) break;
        }
        if ( found ) break;
    }

    if ( pQuest )
    {
        char tbuf[1024];
        output = new_buf();

        if (IS_DRMODE(ch)) add_buf(output,BIG_BLUE_LINE_SINGLE);

        sprintf( tbuf, "%%%ds%%s\n\r%s", 
            40-((int)strlen(pQuest->name))/2,
            (IS_DRMODE(ch) ? BIG_BLUE_LINE_DOUBLE : "") );
        sprintf( buf, tbuf, "", pQuest->name );
        add_buf(output, buf);

        add_buf(output, "Progress so far:\n\r\n\r");

        for ( pJournal = pQuest->journal; pJournal != NULL; 
                                                 pJournal = pJournal->next )
        {
            if ( check_recog( ch, pJournal->vnum, pJournal->flag ) )
            {
                if ( pJournal->vnum == pQuest->completevnum
                &&   pJournal->flag == pQuest->completeflag )
                    complete = TRUE;
                sprintf(buf, " * %s\n\r", pJournal->text );
                add_buf(output, buf);
            }
        }
        add_buf(output, "\n\r");
        if ( complete )
            add_buf(output, "   This quest has been completed.\n\r");
        if (IS_DRMODE(ch)) add_buf(output,BIG_BLUE_LINE_SINGLE);

        /* insert any related command stuff here */
        add_buf(output, "{Djournal list                    journal read <#>               journal completed{x\n\r");

        page_to_char(buf_string(output),ch);
        free_buf(output);
    }
    else
    {
        if ( qcount == 0 )
            send_to_char("You have no quests in your journal yet.\n\r", 
                ch );
        else if ( qcount == 1 )
            send_to_char("You only have one quest in your journal.\n\r",
                ch );
        else
        {
            sprintf(buf, "There are only %d quests in your journal.\n\r",
                qcount );
            send_to_char(buf, ch);
        }
    }
    return;
}

void journal_completed( CHAR_DATA *ch )
{
    QUEST_DATA *pQuest = NULL;
    JOURNAL_DATA *pJournal;
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH], tbuf[MAX_STRING_LENGTH];
    int totsum, xpsum, skillsum, mystsum;

    totsum = 0;
    xpsum = 0;
    skillsum = 0;
    mystsum = 0;

    /* Sum up completed quest types -- hardcoded types */
    for( pArea = area_first; pArea; pArea = pArea->next )
    {
        for ( pQuest = pArea->quests; pQuest; pQuest = pQuest->next )
        {
            if ( !pQuest->completevnum )
                continue;
            for ( pJournal = pQuest->journal; pJournal != NULL; 
                                                 pJournal = pJournal->next )
            {
                 if ( pJournal->vnum == pQuest->completevnum
                 &&   pJournal->flag == pQuest->completeflag
                 &&   check_recog( ch, pJournal->vnum, pJournal->flag ) )
                 {
                     totsum++;
                     if ( IS_SET(pQuest->flags,QTYPE_XP) )
                         xpsum++;
                     if ( IS_SET(pQuest->flags,QTYPE_SKILL) )
                         skillsum++;
                     if ( IS_SET(pQuest->flags,QTYPE_MYSTERY) )
                         mystsum++;
                 }
            }
        }
    }

    if (IS_DRMODE(ch)) send_to_char( BIG_BLUE_LINE_SINGLE, ch);

    sprintf( tbuf, "%%%ds%%s%%s\n\r%s", 40-((int)strlen(ch->name)+17)/2,
        (IS_DRMODE(ch) ? BIG_BLUE_LINE_DOUBLE : "") );
    sprintf( buf, tbuf, "", ch->name, "'s Quest Progress" );
    send_to_char(buf, ch);

    /* Hardcoded flag sums here */ 
    sprintf(buf, 
    "                      Mysteries solved: %d out of %d (%5.1f%%)\n\r", 
        mystsum, 
        global_quest_count[2],
        ( global_quest_count[2] > 0 ? 
            (float)(100 * mystsum / global_quest_count[0]) : 0.0 ) );
    send_to_char(buf, ch);

    sprintf(buf, 
    "                   XP quests completed: %d out of %d (%5.1f%%)\n\r", 
        xpsum, 
        global_quest_count[0],
        ( global_quest_count[0] > 0 ? 
            (float)(100 * xpsum / global_quest_count[0]) : 0.0 ) );
    send_to_char(buf, ch);

    sprintf(buf, 
    "                Skill quests completed: %d out of %d (%5.1f%%)\n\r", 
        skillsum, 
        global_quest_count[1],
        ( global_quest_count[1] > 0 ? 
            (float)(100 * skillsum / global_quest_count[0]) : 0.0 ) );
    send_to_char(buf, ch);

    sprintf(buf, 
    "\n\r                     Total quest score: %d out of %d (%5.1f%%)\n\r", 
        totsum, 
        global_quest_count[MAX_FLAG],
        ( global_quest_count[MAX_FLAG] > 0 ? 
            (float)(100 * totsum / global_quest_count[MAX_FLAG]) : 0.0 ) );
    send_to_char(buf, ch);

    if (IS_DRMODE(ch)) send_to_char( BIG_BLUE_LINE_SINGLE, ch);

    /* insert any related command stuff here */
    send_to_char("{Djournal list                    journal read <#>               journal completed{x\n\r", ch);

    return;
}

void journal_index( CHAR_DATA *ch, int page )
{
    QUEST_DATA *pQuest = NULL;
    JOURNAL_DATA *pJournal;
    AREA_DATA *pArea;
    BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    char tbuf[1024];
    char *jtag = "";
    int qcount, pcount, histep, offset;
    bool complete = FALSE, breakout = FALSE;

    /* do_journal ensures that provided page is a sensible value, but it does
       not ensure that player has anything written on that page */
    pcount = 0;
    for( pArea = area_first; pArea; pArea = pArea->next )
        for ( pQuest = pArea->quests; pQuest; pQuest = pQuest->next )
            if ( pQuest->completevnum )
            {
                pcount++;
                /* pagenum = qcount / MAX_QUESTS_PER_PAGE + 1 */
                /* pagenum = qcount / MAX_QUESTS_PER_PAGE     */
                /* offset = (pagenuma - 1) * MAX_QUESTS_PER_PAGE */
            }
    pcount = pcount / MAX_QUESTS_PER_PAGE + 1;

    if ( page < 1 )
    {
        send_to_char( "That doesn't even make sense.\n\r", ch );
        return;
    }
    else if ( page > pcount )
    {
        sprintf(buf, "You only have %d pages in your journal index.\n\r",
            pcount );
        send_to_char(buf, ch);
        return;
    }
    offset = (page-1)*MAX_QUESTS_PER_PAGE;

    output = new_buf();

    if (IS_DRMODE(ch)) add_buf(output,BIG_BLUE_LINE_SINGLE);

    sprintf( tbuf, "%%%ds%%s%%s\n\r%s", 40-((int)strlen(ch->name)+10)/2,
        ( IS_DRMODE(ch) ? BIG_BLUE_LINE_DOUBLE : "" ) );
    sprintf( buf, tbuf, "", ch->name, "'s Journal" );

    add_buf(output, buf);

    qcount = 0;
    for( pArea = area_first; pArea; pArea = pArea->next )
    {
        if ( breakout )
            break;
        for ( pQuest = pArea->quests; pQuest; pQuest = pQuest->next )
        {
            if ( breakout )
                break;
            if ( !pQuest->completevnum
            ||   ++qcount <= offset )
                continue;
            histep = -1;
            complete = FALSE;
            for ( pJournal = pQuest->journal; pJournal != NULL; 
                                                 pJournal = pJournal->next )
            {
                if ( check_recog( ch, pJournal->vnum, pJournal->flag ) )
                {
                    if ( pJournal->step > histep )
                    {
                        histep = pJournal->step;
                        jtag = pJournal->text;
                    }
                    if ( pJournal->vnum == pQuest->completevnum
                    &&   pJournal->flag == pQuest->completeflag )
                        complete = TRUE;
                }
            }
            if ( histep >= 0 )
            {
                sprintf( buf, "%3d. %s\n\r", qcount, pQuest->name ) ;
                add_buf(output,buf);
                if ( complete )
                    sprintf(buf, "     ({DCompleted{x) %s\n\r\n\r",
                        ( jtag != "" ? jtag : "???" ) );
                else
                    sprintf(buf, "     ({rStep %d{x) %s\n\r\n\r",
                        histep,
                        ( jtag != "" ? jtag : "???" ) );

                add_buf(output,buf);
            }
            if ( qcount == (page)*MAX_QUESTS_PER_PAGE )
               breakout = TRUE;
        }
    }

    if ( breakout )
    {
        sprintf(buf, 
"There are %d pages in the index.  To view the next one, use journal index %d.\n\r", pcount, page+1);
        add_buf(output,buf);
    }

    if (IS_DRMODE(ch)) add_buf(output,BIG_BLUE_LINE_SINGLE);

    /* insert any related command stuff here */
    add_buf(output, "{Djournal list                    journal read <#>               journal completed{x\n\r");

    page_to_char(buf_string(output),ch);
    free_buf(output);

    return;
}

void do_journal( CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char firstc;
    int page;

    if ( IS_NPC(ch) )
    {
        send_to_char( "Use mpstat to see your recogs.\n\r", ch );
        return;
    }

    argument = one_argument(argument, arg); 
    firstc = LOWER(arg[0]);

    /* "index" or "list" */
    if ( IS_NULLSTR(arg) || firstc == 'i' || firstc == 'l' )
    {
        if ( IS_NULLSTR(argument) )
            page = 1;
        else
            page = atoi(argument);
        journal_index( ch, page );
    }
    /* "read" */
    else if ( firstc == 'r' )
    {
        if ( IS_NULLSTR(argument) )
            page = 1;
        else
            page = atoi(argument);
        journal_read(ch, page );
    }
    /* "completed" or "progress" */
    else if ( firstc == 'c' || firstc == 'p' )
    {
        journal_completed( ch );
    }
    else
    {
        send_to_char("You can only LIST, READ, or check PROGRESS using your journal.\n\r", ch);
    }

    return;
}

/* If you compile with 128-bit integers, this may break */
int global_quest_count[MAX_FLAG+1];

void sort_journals ( AREA_DATA *pArea )
{
    QUEST_DATA          *pQuest;
    JOURNAL_DATA        *pJournal, *j_prev, *j_next;
    bool                swapped;

    for ( pQuest = pArea->quests; pQuest; pQuest = pQuest->next )
    {
        if ( (pJournal = pQuest->journal) == NULL )
            break;

        /* Bubble sort journal entries according to step */
        do
        {
            swapped = FALSE;
            j_prev = NULL;
            for ( pJournal = pQuest->journal; 
                  pJournal && pJournal->next; 
                  pJournal = pJournal->next )
            {
                if ( pJournal->step > pJournal->next->step )
                {
                    j_next = pJournal->next->next;
                    pJournal->next->next = pJournal;
                    if ( j_prev )
                        j_prev->next = pJournal->next;
                    else
                        pQuest->journal = pJournal->next;
                    pJournal->next = j_next;
                    swapped = TRUE;
                }
                j_prev = pJournal;
            }
        } while ( swapped );
    }
    return;
}

void sort_quest_db( void )
{
    QUEST_DATA          *pQuest;
    AREA_DATA           *pArea;
    int                 i, count;

    for ( i = 0; i < MAX_FLAG+1; i++ )
        global_quest_count[i] = 0;

    for( pArea = area_first; pArea; pArea = pArea->next )
    {
        sort_journals( pArea );

        /* Tally up the number of available quests in each category */
        for ( pQuest = pArea->quests; pQuest; pQuest = pQuest->next )
        {
            if ( pQuest->completevnum > 0 )
            {
                i = 1;
                global_quest_count[MAX_FLAG]++;
                for ( count = 0; count < MAX_FLAG; count++ )
                {
                    if ( IS_SET( pQuest->flags, i ) )
                        global_quest_count[count]++;
                    i *= 2;
                }
            }
        }
    }
    return;
}

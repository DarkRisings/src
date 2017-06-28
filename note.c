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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "clans.h"

/* globals from db.c for load_notes */
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif
extern FILE *                  fpArea;
extern char                    strArea[MAX_INPUT_LENGTH];

/* local procedures */
void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time);
void parse_note(CHAR_DATA *ch, char *argument, int type);
bool hide_note(CHAR_DATA *ch, NOTE_DATA *pnote);
bool is_note_race( CHAR_DATA* ch, NOTE_DATA* pnote );
bool is_note_class( CHAR_DATA* ch, NOTE_DATA *pnote );
bool is_note_special( CHAR_DATA* ch, NOTE_DATA* pnote );
bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote );

NOTE_DATA *note_list;
NOTE_DATA *idea_list;
NOTE_DATA *history_list;
NOTE_DATA *news_list;
NOTE_DATA *changes_list;
NOTE_DATA *trouble_list;

const char LASOMBRA_PAT[]   = "Sirath";
const char RAVNOS_PAT[]     = "Kiyanne";
const char TRAMUIR_PAT[]    = "Flocrian";
const char FEROCAI_PAT[]   = "Alumot";

int count_spool(CHAR_DATA *ch, NOTE_DATA *spool)
{
    int count = 0;
    NOTE_DATA *pnote;

    for (pnote = spool; pnote != NULL; pnote = pnote->next)
	if (!hide_note(ch,pnote))
	    count++;

    return count;
}

int count_total(CHAR_DATA *ch, NOTE_DATA *spool)
{
    int count = 0;
    NOTE_DATA *pnote;

    for (pnote = spool; pnote != NULL; pnote = pnote->next)
	{
	   	if (is_note_to(ch,pnote)) 
		{
			count++;
		}
	}
    return count;
}

void do_unread(CHAR_DATA *ch)
{
    char buf[MAX_STRING_LENGTH];
    int count;
    bool found = FALSE;

    if (IS_NPC(ch))
	return; 

    if(IS_IMMORTAL(ch) && ( count = count_spool(ch,trouble_list)) > 0 )
      {
	found = TRUE;
	sprintf( buf, "There %s %d new troublemaker%s waiting for your wrath.\n\r",
		 ( count > 1 ? "are" : "is" ), 
		 count,
		 ( count > 1 ? "s" : "" ) );
	send_to_char( buf, ch );
      }

    if (IS_IMMORTAL(ch) && (count = count_spool(ch,news_list)) > 0)
    {
	found = TRUE;
	sprintf(buf,"There %s %d new news article%s waiting.\n\r",
	    count > 1 ? "are" : "is",count, count > 1 ? "s" : "");
	send_to_char(buf,ch);
    }
    if ((count = count_spool(ch,changes_list)) > 0)
    {
	found = TRUE;
	sprintf(buf,"There %s %d change%s waiting to be read.\n\r",
	    count > 1 ? "are" : "is", count, count > 1 ? "s" : "");
        send_to_char(buf,ch);
    }

    if ((count = count_spool(ch,note_list)) > 0)
    {
	found = TRUE;
	sprintf(buf,"You have %d new note%s waiting.\n\r",
	    count, count > 1 ? "s" : "");
	send_to_char(buf,ch);
    }
    if ((count = count_spool(ch,idea_list)) > 0)
    {
	found = TRUE;
	sprintf(buf,"You have %d unread idea%s to peruse.\n\r",
	    count, count > 1 ? "s" : "");
	send_to_char(buf,ch);
    }
    if ((count = count_spool(ch,history_list)) > 0)
    {
	found = TRUE;
	sprintf(buf,"%d %s been added.\n\r",
	    count, count > 1 ? "histories have" : "history has");
	send_to_char(buf,ch);
    }

    if (!found)
	send_to_char("You have no unread notes.\n\r",ch);
}

void do_noteboard(CHAR_DATA *ch)
{
    char buf[MAX_STRING_LENGTH];
    int count;
	int unread;

    if (IS_NPC(ch))
	return; 

	send_to_char("{r +{D=================== {rNote Board {D==================={r+\n\r",ch);
	send_to_char("{D |--------------------------------------------------|\n\r",ch);
	send_to_char( "{D | {YBoard Name           Unread                Total {D|\n\r",ch);
	send_to_char("{D |--------------------------------------------------|\n\r", ch);
	
	if (IS_IMMORTAL(ch))
	{
		count = count_total(ch,trouble_list);
		unread = count_spool(ch,trouble_list);
		sprintf(buf,"{D | {wTroublemakers        %5d                 %5d {D|\n\r", unread, count);
		send_to_char(buf,ch);
	
		count = count_total(ch,news_list);
		unread = count_spool(ch,news_list);
		sprintf(buf,"{D | {wNews                 %5d                 %5d {D|\n\r", unread, count);
		send_to_char(buf,ch);
	}
		
	count = count_total(ch,changes_list);
	unread = count_spool(ch,changes_list);
	sprintf(buf,"{D | {wChanges              %5d                 %5d {D|\n\r", unread, count);
	send_to_char(buf,ch);
		
	count = count_total(ch,note_list);
	unread = count_spool(ch,note_list);
	sprintf(buf,"{D | {wNotes                %5d                 %5d {D|\n\r", unread, count);
	send_to_char(buf,ch);
	
	count = count_total(ch,history_list);
	unread = count_spool(ch,history_list);	
	sprintf(buf,"{D | {wHistories            %5d                 %5d {D|\n\r", unread, count);
	send_to_char(buf,ch);
	
	count = count_total(ch,idea_list);
	unread = count_spool(ch,idea_list);
	sprintf(buf,"{D | {wIdeas                %5d                 %5d {D|\n\r", unread, count);
	send_to_char(buf,ch);
	
	send_to_char("{r +{D=================================================={r+{x\n\r",ch);
}

void do_note(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_NOTE);
}

void do_idea(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_IDEA);
}

void do_history(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_HISTORY);
}

void do_news(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_NEWS);
}

void do_changes(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_CHANGES);
}


void do_trouble( CHAR_DATA* ch, char* argument )
{
  if( IS_IMMORTAL( ch ) )
    parse_note( ch, argument, NOTE_TROUBLE );
  else
    send_to_char( "Huh?\n\r", ch );
}

void save_notes(int type)
{
    FILE *fp;
    char *name;
    NOTE_DATA *pnote;

    switch (type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    name = NOTE_FILE;
	    pnote = note_list;
	    break;
	case NOTE_IDEA:
	    name = IDEA_FILE;
	    pnote = idea_list;
	    break;
	case NOTE_HISTORY:
	    name = HISTORY_FILE;
	    pnote = history_list;
	    break;
	case NOTE_NEWS:
	    name = NEWS_FILE;
	    pnote = news_list;
	    break;
	case NOTE_CHANGES:
	    name = CHANGES_FILE;
	    pnote = changes_list;
	    break;
    case NOTE_TROUBLE:
            name = TROUBLE_FILE;
            pnote = trouble_list;
            break;
    }

    fclose( fpReserve );
    if ( ( fp = fopen( name, "w" ) ) == NULL )
    {
	perror( name );
    }
    else
    {
	for ( ; pnote != NULL; pnote = pnote->next )
	{
	    fprintf( fp, "Sender  %s~\n", pnote->sender);
	    fprintf( fp, "Date    %s~\n", pnote->date);
	    fprintf( fp, "Stamp   %ld\n", pnote->date_stamp);
	    fprintf( fp, "To      %s~\n", pnote->to_list);
	    fprintf( fp, "Subject %s~\n", pnote->subject);
            if ( pnote->persist != 0 )
                fprintf( fp, "Persist %d\n",  pnote->persist );
	    fprintf( fp, "Text\n%s~\n",   pnote->text);
	}
	fclose( fp );
	fpReserve = fopen( NULL_FILE, "r" );
   	return;
    }
}
void load_notes(void)
{
    load_thread(NOTE_FILE,&note_list, NOTE_NOTE, 14*24*60*60);
    load_thread(IDEA_FILE,&idea_list, NOTE_IDEA, 28*24*60*60);
    load_thread(HISTORY_FILE,&history_list, NOTE_HISTORY, 0);
    load_thread(NEWS_FILE,&news_list, NOTE_NEWS, 0);
    load_thread(CHANGES_FILE,&changes_list,NOTE_CHANGES, 0);
    load_thread(TROUBLE_FILE, &trouble_list, NOTE_TROUBLE, 0 );
}

void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time)
{
    FILE *fp;
    char *word;
    char buf[MAX_STRING_LENGTH];
    NOTE_DATA *pnotelast;
 
    if ( ( fp = fopen( name, "r" ) ) == NULL )
	return;
	 
    pnotelast = NULL;
    for ( ; ; )
    {
	NOTE_DATA *pnote;
	char letter;
	 
	do
	{
	    letter = getc( fp );
            if ( feof(fp) )
            {
                fclose( fp );
                return;
            }
        }
        while ( isspace(letter) );
        ungetc( letter, fp );
 
        pnote           = alloc_perm( sizeof(*pnote) );
 
        word = fread_word(fp);
        if ( str_cmp( word, "sender" ) )
            break;
        pnote->sender   = fread_string( fp );
 
        word = fread_word(fp);
        if ( str_cmp( word, "date" ) )
            break;
        pnote->date     = fread_string( fp );
 
        word = fread_word(fp);
        if ( str_cmp( word, "stamp" ) )
            break;
        pnote->date_stamp = fread_number(fp);
 
        word = fread_word(fp);
        if ( str_cmp( word, "to" ) )
            break;
        pnote->to_list  = fread_string( fp );
 
        word = fread_word(fp);
        if ( str_cmp( word, "subject" ) )
            break;
        pnote->subject  = fread_string( fp );

        word = fread_word(fp);
        if ( !str_cmp( word, "persist" ) )
        {
            pnote->persist  = (fread_number(fp) == 0 ? FALSE : TRUE);
            word = fread_word(fp);
        }

        if ( str_cmp( word, "text" ) )
            break;
        pnote->text     = fread_string( fp );

        if ( !pnote->persist )
            if (free_time && pnote->date_stamp < current_time - free_time)
            {
                free_note(pnote);
                continue;
            }

	pnote->type = type;
 
        if (*list == NULL)
            *list           = pnote;
        else
            pnotelast->next     = pnote;
 
        pnotelast       = pnote;
    }
 
    strcpy( strArea, NOTE_FILE );
    fpArea = fp;
    sprintf(buf, "load_notes: bad keyword %s", word);
    bug( buf, 0 );
    exit( 1 );
    return;
}

void append_note(NOTE_DATA *pnote)
{
    NOTE_DATA **list;
    NOTE_DATA *last;

    switch(pnote->type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    list = &note_list;
	    break;
	case NOTE_IDEA:
	    list = &idea_list;
	    break;
	case NOTE_HISTORY:
	    list = &history_list;
	    break;
	case NOTE_NEWS:
	     list = &news_list;
	     break;
	case NOTE_CHANGES:
	     list = &changes_list;
	     break;
        case NOTE_TROUBLE:
             list = &trouble_list;
             break;

    }

    if (*list == NULL)
	*list = pnote;
    else
    {
	for ( last = *list; last->next != NULL; last = last->next);
	last->next = pnote;
    }

    save_notes(pnote->type);

/*  fclose(fpReserve);
    if ( ( fp = fopen(name, "a" ) ) == NULL )
    {
        perror(name);
    }
    else
    {
        fprintf( fp, "Sender  %s~\n", pnote->sender);
        fprintf( fp, "Date    %s~\n", pnote->date);
        fprintf( fp, "Stamp   %ld\n", pnote->date_stamp);
        fprintf( fp, "To      %s~\n", pnote->to_list);
        fprintf( fp, "Subject %s~\n", pnote->subject);
        fprintf( fp, "Text\n%s~\n", pnote->text);
        fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" ); */
}

bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote )
{
  
  if( pnote->type == NOTE_TROUBLE && !IS_IMMORTAL( ch ) )
    return FALSE;
  
  if ( !str_cmp( ch->name, pnote->sender ) )
    return TRUE;
  
  if ( is_name( "all", pnote->to_list ) )
    return TRUE;
  
  if ( IS_ADMIN(ch) && is_name( "admin", pnote->to_list ) )
    return TRUE;

  if ( IS_BRAWLER(ch) && is_name( "brawler", pnote->to_list ) )
    return TRUE;
  
  if ( IS_IMMORTAL(ch) && is_name( "immortal", pnote->to_list ) )
    return TRUE;
  
  if ( IS_IMMORTAL(ch) && is_name( "imm", pnote->to_list ) )
    return TRUE;
  
  if (ch->guild && is_name(guild_table[ch->guild].name,pnote->to_list))
    return TRUE;
  
  if ( is_name( ch->name, pnote->to_list ) )
    return TRUE;

  /* remove this upon replacing the clan code */
  if( is_name( ch->cln_name, pnote->to_list ) || is_name( ch->cln_symbol, pnote->to_list ) )
  	return TRUE;

  /* new clan code support */
  if( !IS_NPC( ch ) &&
      ch->pcdata->pcClan != NULL &&
      ( is_name( ch->pcdata->pcClan->name, pnote->to_list ) ||
	is_name( ch->pcdata->pcClan->symbol, pnote->to_list ) ) )
    {
      return TRUE;
    }

  if( !IS_NPC( ch ) && !IS_SET( ch->act_bits, ACT_NOAPPROVE ) )
  {
      if( is_note_race( ch, pnote ) )
          return TRUE;

      if ( is_note_class( ch, pnote ) )
          return TRUE;
  }

  if ( is_note_special( ch, pnote ) )
    return TRUE;
  
/* nasty brood notes hack */
    if ( is_name( "lasombra", pnote->to_list ) 
    &&   !str_cmp( LASOMBRA_PAT, ch->patriarch ) )
        return TRUE;
  
    if ( is_name( "ravnos", pnote->to_list ) 
    &&   !str_cmp( RAVNOS_PAT, ch->patriarch ) )
        return TRUE;

    if ( is_name( "tramuir", pnote->to_list )
    &&   !str_cmp( TRAMUIR_PAT, ch->patriarch ) )
        return TRUE;
		
	if ( is_name( "ferocai", pnote->to_list )
    &&   !str_cmp( FEROCAI_PAT, ch->patriarch ) )
        return TRUE;

  return FALSE;
}

bool is_note_race( CHAR_DATA* ch, NOTE_DATA* pnote )
{
  char* ch_race;

  ch_race = pc_race_table[ ch->race ].name;

  return is_name( ch_race, pnote->to_list );
}

bool is_note_class( CHAR_DATA *ch, NOTE_DATA *pnote)
{
    char *ch_class;
    if ( IS_NPC( ch ) )
        return FALSE;
    ch_class = class_table[ch->class].name;
    return is_name( ch_class, pnote->to_list );
}

bool is_note_guild( CHAR_DATA *ch, NOTE_DATA *pnote)
{
    if (ch->guild && is_name(guild_table[ch->guild].name,pnote->to_list))
    return TRUE;

    return FALSE;

}

bool is_note_personal( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    if ( is_name( ch->name, pnote->to_list ) )
    return TRUE;

    return FALSE;
}

bool is_note_vamp( CHAR_DATA* ch, NOTE_DATA* pnote )
{
  bool vamp_note = FALSE;

  if( is_name( "lasombra", pnote->to_list ) &&
      !strcasecmp( LASOMBRA_PAT, ch->patriarch ) )
    {
      vamp_note = TRUE;
    }

  if( is_name( "ravnos", pnote->to_list ) &&
      !strcasecmp( RAVNOS_PAT, ch->patriarch ) )
    {
      vamp_note = TRUE;
    }
	
	if( is_name( "ferocai", pnote->to_list ) &&
      !strcasecmp( FEROCAI_PAT, ch->patriarch ) )
    {
      vamp_note = TRUE;
    }

  return vamp_note;
}

bool is_note_special( CHAR_DATA* ch, NOTE_DATA* pnote )
{
    char *tags;
    char targ[MAX_STRING_LENGTH];
    bool found = FALSE;

    if ( IS_NPC( ch ) )
       return FALSE;

    if ( IS_NULLSTR( ch->pcdata->notetags ) )
       return FALSE;
 
    tags = ch->pcdata->notetags;
    tags = one_argument( tags, targ );

    while ( !IS_NULLSTR( targ ) )
    {
        if ( is_name( targ, pnote->to_list ) )
            found = TRUE;
        tags = one_argument( tags, targ );
    }

    return found;
}

void note_attach( CHAR_DATA *ch, int type )
{
    NOTE_DATA *pnote;

    if ( ch->pnote != NULL )
	return;

    pnote = new_note();

    pnote->next		= NULL;
    pnote->sender	= str_dup( ch->name );
    pnote->date		= str_dup( "" );
    pnote->to_list	= str_dup( "" );
    pnote->subject	= str_dup( "" );
    pnote->text		= str_dup( "" );
    pnote->type		= type;
    ch->pnote		= pnote;
    return;
}



void note_remove( CHAR_DATA *ch, NOTE_DATA *pnote, bool delete)
{
    char to_new[MAX_INPUT_LENGTH];
    char to_one[MAX_INPUT_LENGTH];
    NOTE_DATA *prev;
    NOTE_DATA **list;
    char *to_list;

    if (!delete)
    {
	/* make a new list */
        to_new[0]	= '\0';
        to_list	= pnote->to_list;
        while ( *to_list != '\0' )
        {
    	    to_list	= one_argument( to_list, to_one );
    	    if ( to_one[0] != '\0' && str_cmp( ch->name, to_one ) )
	    {
	        strcat( to_new, " " );
	        strcat( to_new, to_one );
	    }
        }
        /* Just a simple recipient removal? */
       if ( str_cmp( ch->name, pnote->sender ) && to_new[0] != '\0' )
       {
	   free_string( pnote->to_list );
	   pnote->to_list = str_dup( to_new + 1 );
	   return;
       }
    }
    /* nuke the whole note */

    switch(pnote->type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    list = &note_list;
	    break;
	case NOTE_IDEA:
	    list = &idea_list;
	    break;
	case NOTE_HISTORY:
	    list = &history_list;
	    break;
	case NOTE_NEWS:
	    list = &news_list;
	    break;
	case NOTE_CHANGES:
	    list = &changes_list;
	    break;
        case NOTE_TROUBLE:
            list = &trouble_list;
            break;
    }

    /*
     * Remove note from linked list.
     */
    if ( pnote == *list )
    {
	*list = pnote->next;
    }
    else
    {
	for ( prev = *list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == pnote )
		break;
	}

	if ( prev == NULL )
	{
	    bug( "Note_remove: pnote not found.", 0 );
	    return;
	}

	prev->next = pnote->next;
    }

    save_notes(pnote->type);
    free_note(pnote);
    return;
}

bool hide_note (CHAR_DATA *ch, NOTE_DATA *pnote)
{
  time_t last_read;

  if (IS_NPC(ch))
    return TRUE;

  switch (pnote->type)
    {
    default:
      return TRUE;
    case NOTE_NOTE:
      last_read = ch->pcdata->last_note;
      break;
    case NOTE_IDEA:
      last_read = ch->pcdata->last_idea;
      break;
    case NOTE_HISTORY:
      last_read = ch->pcdata->last_history;
      break;
    case NOTE_NEWS:
      last_read = ch->pcdata->last_news;
      break;
    case NOTE_CHANGES:
      last_read = ch->pcdata->last_changes;
      break;
    case NOTE_TROUBLE:
      last_read = ch->pcdata->last_trouble;
      break;
    }
    
  if (pnote->date_stamp <= last_read)
    return TRUE;
  
  if (!str_cmp(ch->name,pnote->sender))
    return TRUE;
  
  if (!is_note_to(ch,pnote))
    return TRUE;

    return FALSE;
}


void update_read(CHAR_DATA *ch, NOTE_DATA *pnote)
{
    time_t stamp;

    if (IS_NPC(ch))
	return;

    stamp = pnote->date_stamp;

    switch (pnote->type)
    {
        default:
            return;
        case NOTE_NOTE:
	    ch->pcdata->last_note = UMAX(ch->pcdata->last_note,stamp);
            break;
        case NOTE_IDEA:
	    ch->pcdata->last_idea = UMAX(ch->pcdata->last_idea,stamp);
            break;
        case NOTE_HISTORY:
	    ch->pcdata->last_history = UMAX(ch->pcdata->last_history,stamp);
            break;
        case NOTE_NEWS:
	    ch->pcdata->last_news = UMAX(ch->pcdata->last_news,stamp);
            break;
        case NOTE_CHANGES:
	    ch->pcdata->last_changes = UMAX(ch->pcdata->last_changes,stamp);
            break;
        case NOTE_TROUBLE:
            ch->pcdata->last_trouble = UMAX( ch->pcdata->last_trouble, stamp );
            break;
    }
}

void parse_note( CHAR_DATA *ch, char *argument, int type )
{
  BUFFER* buffer;
  char* buf2;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  NOTE_DATA *pnote = NULL;
  NOTE_DATA *notestart = NULL;
  NOTE_DATA **list;
  char *list_name;
  char *cmd_name;
  int vnum;
  int pnum;      /* Note tail position variable */
  int anum;
  int i, j, k;
  int vnumoffset = 0;
  bool tail;     /* Tail = TRUE, List = FALSE */

  buf2 = NULL;

  switch(type)
    {
    default:
      return;
    case NOTE_NOTE:
      list = &note_list;
      list_name = "notes";
      cmd_name = "note";
      break;
    case NOTE_IDEA:
      list = &idea_list;
      list_name = "ideas";
      cmd_name = "idea";
      break;
    case NOTE_HISTORY:
      list = &history_list;
      list_name = "histories";
      cmd_name = "history";
      break;
    case NOTE_NEWS:
      list = &news_list;
      list_name = "news";
      cmd_name = "news";
      break;
    case NOTE_CHANGES:
      list = &changes_list;
      list_name = "changes";
      cmd_name = "change";
      break;
    case NOTE_TROUBLE:
      list = &trouble_list;
      list_name = "troublemakers";
      cmd_name = "troublemaker";
      break;
    }

  argument = one_argument( argument, arg );
  smash_tilde( argument );

  if ( arg[0] == '\0' || !str_prefix( arg, "read" ) )
    {
      bool fAll;
      if ( IS_NPC(ch) ) return;

      if ( !str_cmp( argument, "all" ) )
        {
	  fAll = TRUE;
	  anum = 0;
        }
 
      else if ( argument[0] == '\0' || !str_prefix(argument, "next"))
        /* read next unread note */
        {
	  vnum = 0;
	  for ( pnote = *list; pnote != NULL; pnote = pnote->next)
            {
	      if (!hide_note(ch,pnote))
                {
		  sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
			   vnum,
			   pnote->sender,
			   pnote->subject,
			   pnote->date,
			   pnote->to_list);
		  send_to_char( buf, ch );
		  page_to_char( pnote->text, ch );
		  update_read(ch,pnote);
		  return;
                }
	      else if (is_note_to(ch,pnote))
		vnum++;
            }
	  sprintf(buf,"You have no unread %s.\n\r",list_name);
	  send_to_char(buf,ch);
	  return;
        }
 
      else if ( is_number( argument ) )
        {
	  fAll = FALSE;
	  anum = atoi( argument );
        }
      else
        {
	  send_to_char( "Read which number?\n\r", ch );
	  return;
        }
 
      vnum = 0;
      for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
	  if ( is_note_to( ch, pnote ) && ( vnum++ == anum || fAll ) )
            {
	      sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
		       vnum - 1,
		       pnote->sender,
		       pnote->subject,
		       pnote->date,
		       pnote->to_list
		       );
	      send_to_char( buf, ch );
	      page_to_char( pnote->text, ch );
	      update_read(ch,pnote);
	      return;
            }
        }
 
      sprintf(buf,"There aren't that many %s.\n\r",list_name);
      send_to_char(buf,ch);
      return;
    }

/* Rewritten 'note list' command by gkl, May 2006. */
  if ( !str_prefix( arg, "list" ) || !str_prefix( arg, "tail" ) )
    {
      vnum = 0;
      pnum = 0;
      k = 0;
      tail = FALSE;
      if ( IS_NPC(ch) ) return;

      if( !str_prefix( arg, "tail" ) )
        {
	  tail = TRUE;
	  
	  for ( pnote = *list; pnote != NULL; pnote = pnote->next )
            {
	      if ( is_note_to( ch, pnote ) )
		pnum++;
            }
	  
	  if( pnum < 10 )
	    pnum = 0;
	  else
	    pnum -= 10;
	  
	  switch(type)
            {
	    case NOTE_NOTE:     list = &note_list;      break;
	    case NOTE_IDEA:     list = &idea_list;      break;
	    case NOTE_HISTORY:  list = &history_list;   break;
	    case NOTE_NEWS:     list = &news_list;      break;
	    case NOTE_CHANGES:  list = &changes_list;   break;
	    case NOTE_TROUBLE:  list = &trouble_list;   break;
/*	    case NOTE_LORE:     list = &lore_list;      break;*/
	    default:            return;
            }
        }
      else /* tail doesn't allow one to specify starting note */
      {
          k = 0;
          /* This is a count for the end-of-page text.  Kind of
             inefficient; feel free to clean up later */
          for ( pnote = *list; pnote != NULL; pnote = pnote->next )
          {
              if ( is_note_to( ch, pnote ) )
                  k++;
          }


          /* The below line prevents overflows.  The constant of 32700
             may need to be adjusted if the number of notes ever exceeds
             this. */
          vnumoffset = UMAX(UMIN(atoi(argument), 327),0);
          vnumoffset = UMIN(UMAX((vnumoffset - 1) * MAX_NOTES_PER_LIST,0), 32700);
	  if ( vnumoffset < 1 )
	    vnumoffset = 0;
	  else
	  {
              j = 0;

	      for ( pnote = *list; pnote != NULL; pnote = pnote->next )
              {
                  if ( is_note_to( ch, pnote ) )
                      j++;
                  if ( j > vnumoffset )
		      break; /* This is bad coding practice.  Oops */
              }
              if ( pnote == NULL ) /* In case specified note doesn't exist */
              {
		  sprintf(buf, "There aren't that many %s.\n\r", list_name );
		  send_to_char( buf, ch );
		  return;
              }
	  }
      }

      if ( !tail && pnote != NULL )
	  notestart = pnote;
      else
	  notestart = *list;

      i = 0;
      for ( pnote = notestart; pnote != NULL; pnote = pnote->next )
	{
	  if ( is_note_to( ch, pnote ) )
          {
	      if ( i >= MAX_NOTES_PER_LIST )
		  break; /* More bad programming practice */

	      if( tail && ( vnum < pnum ) )
		{
		  vnum++;
		  continue;
		}
	      
				sprintf( buf, "[%3d%s] %s %s %s%s %s%s%s%s%s%s\n\r",
					vnum + vnumoffset, 
					hide_note(ch,pnote) ? ( pnote->persist ? "P" : " " ) : "N", 
					pnote->date,
					"-",
					pnote->sender,
					":",
					(int) is_note_race(ch,pnote) ? "(RACE) ": "",
					is_note_guild(ch,pnote) ? "(GUILD) ": "",
					is_note_personal(ch,pnote) ? "(PERSONAL) " : "",
					is_note_vamp(ch,pnote) ? "(BROOD) ": "",
					( IS_BRAWLER(ch) && is_name( "brawler", pnote->to_list ) ) ? "(BRAWLER) " : "",
					pnote->subject);
	      
	      if( buf2 == NULL )
		{
		  buf2 = strdup( buf );
		}
	      else
		{
		  buf2 = ( char* )( realloc( buf2, sizeof( char ) *
					     ( strlen( buf ) +
					       strlen( buf2 ) + 1 ) ) );
		  
		  strcat( buf2, buf );
		}
	      
	      vnum++;
	      i++;
	    }
	}

      if( buf2 != NULL )
      {
          if ( pnote != NULL && vnum ) /* More notes to be read; page cut off the rest */
          {
            sprintf(buf,
                "\n\rThere are %d pages of %s.  To view the next one, use %s list %d.\n\r",
                (k / MAX_NOTES_PER_LIST)+1, list_name, 
                cmd_name, vnumoffset / MAX_NOTES_PER_LIST + 2 );

            buf2 = ( char* )( realloc( buf2, sizeof( char ) *
                                           ( strlen( buf ) +
                                             strlen( buf2 ) + 1 ) ) );

            strcat( buf2, buf );
          }

	  page_to_char( buf2, ch );
	  free( buf2 );
	  buf2 = NULL;

      }
      
      if (!vnum)
	{
	  switch(type)
	    {
	    case NOTE_NOTE:	
	      send_to_char("There are no notes for you.\n\r",ch);
	      break;
	    case NOTE_IDEA:
	      send_to_char("There are no ideas for you.\n\r",ch);
	      break;
	    case NOTE_HISTORY:
	      send_to_char("There are no histories for you.\n\r",ch);
	      break;
	    case NOTE_NEWS:
	      send_to_char("There is no news for you.\n\r",ch);
	      break;
	    case NOTE_CHANGES:
	      send_to_char("There are no changes for you.\n\r",ch);
	      break;
	    case NOTE_TROUBLE:
	      send_to_char( "There are no troublemakers for you.  Lucky guy!\n\r", ch );
	      break;
/*	    case NOTE_LORE:
	      send_to_char( "There is no lore for you.\n\r", ch );
	      break;*/
	    }
	}
      
      return;
    }
  if ( !str_prefix( arg, "personal" ) )
    {
      vnum = 0;
      if ( IS_NPC(ch) ) return;

      for ( pnote = *list; pnote != NULL; pnote = pnote->next )
	{
          if (is_note_to(ch,pnote)) {
	    if ( is_note_personal( ch, pnote ) )
	      {
		sprintf( buf, "[%3d%s] %s - %s: %s%s%s%s%s\n\r",
			 vnum, 
/*                       hide_note(ch,pnote) ? " " : "N", */
                         hide_note(ch,pnote) ? ( pnote->persist ? "P" : " " ) : "N",
			 pnote->date,
			 pnote->sender,
			 (int) is_note_race(ch,pnote) ? "(RACE) " : "",
			 is_note_guild(ch,pnote) ? "(GUILD) ": "",
			 is_note_personal(ch,pnote) ? "(PERSONAL) " : "",
                         ( IS_BRAWLER(ch) && is_name( "brawler", pnote->to_list ) ) ? "(BRAWLER) " : "",
			 pnote->subject );
		send_to_char( buf, ch );

	      }
	    vnum++;
	  }
	}
      if (!vnum)
	{
	  switch(type)
	    {
	    case NOTE_NOTE:	
	      send_to_char("There are no notes for you.\n\r",ch);
	      break;
	    case NOTE_IDEA:
	      send_to_char("There are no ideas for you.\n\r",ch);
	      break;
	    case NOTE_HISTORY:
	      send_to_char("There are no histories for you.\n\r",ch);
	      break;
	    case NOTE_NEWS:
	      send_to_char("There is no news for you.\n\r",ch);
	      break;
	    case NOTE_CHANGES:
	      send_to_char("There are no changes for you.\n\r",ch);
	      break;
	    case NOTE_TROUBLE:
	      send_to_char( "There are no troublemakers for you.  Lucky you!\n\r", ch );
	      break;
/*	    case NOTE_LORE:
	      send_to_char( "There is no lore for you.\n\r", ch );
	      break;*/
	    }
	}
      return;
    }

/* Rewritten 'note guild'.  For details, see 'note list' above. */
  if ( !str_prefix( arg, "guild" ) )
    {
      vnum = 0;
      pnum = 0;

      if ( IS_NPC(ch) ) return;

      k = 0;

      for ( pnote = *list; pnote != NULL; pnote = pnote->next )
      {
          if ( is_note_to( ch, pnote ) && is_note_guild( ch, pnote ) )
              k++;
      }

      if ( !IS_NULLSTR(argument) && !str_prefix( argument, "count" ) )
      {
           sprintf(buf, "You have a total of %d guild %s over %d pages.\n\r",
               k, list_name, ( k / MAX_NOTES_PER_LIST ) + 1 );
           send_to_char( buf, ch );
           return;
      }

/* Very obscure overflow protection.  Increase if necessary. */ 
      vnumoffset = UMAX(UMIN(atoi(argument), 327),0);
      vnumoffset = UMIN(UMAX((vnumoffset - 1) * MAX_NOTES_PER_LIST,0), 32700);
      if ( vnumoffset < 1 )
          vnumoffset = 0;
      else
      {
          j = 0;
          for ( pnote = *list; pnote != NULL; pnote = pnote->next )
          {
              if ( is_note_to( ch, pnote ) )
              {
                  vnum++;
                  if ( is_note_guild( ch, pnote ) )
                      j++;
              }
              if ( j > vnumoffset )
                  break; /* This is bad coding practice.  Oops */
          }
          if ( pnote == NULL )
          {
              sprintf(buf, "There aren't that many guild %s.\n\r", list_name );
              send_to_char( buf, ch );
              return;
          }
      }

      i = 0; /* <-- incremented only for guild notes */
      if ( pnote != NULL )
      {
          vnum--;
          notestart = pnote;
      }
      else
          notestart = *list;
      for ( pnote = notestart; pnote != NULL; pnote = pnote->next )
      {
          if ( is_note_to( ch, pnote ) )
          {
              if ( is_note_guild( ch, pnote ) )
              {
                  if ( (i) >= MAX_NOTES_PER_LIST )
                       break;
					   
					sprintf( buf, "[%3d%s] %s %s %s%s %s%s%s%s%s%s\n\r",               
                       vnum, 
                       hide_note(ch,pnote) ? ( pnote->persist ? "P" : " " ) : "N",
                       pnote->date,
                      "-",
					  pnote->sender,
			         ":",
                       (int) is_note_race(ch,pnote) ? "(RACE) ": "",
                       is_note_guild(ch,pnote) ? "(GUILD) ": "",
                       is_note_personal(ch,pnote) ? "(PERSONAL) " : "",
                       is_note_vamp(ch,pnote) ? "(BROOD) ": "",
                       ( IS_BRAWLER(ch) && is_name( "brawler", pnote->to_list ) ) ? "(BRAWLER) " : "",
                       pnote->subject );

                  if( buf2 == NULL )
                      buf2 = strdup( buf );
                  else
                  {
                      buf2 = ( char* )( realloc( buf2, sizeof( char ) *
                          ( strlen( buf ) +
                          strlen( buf2 ) + 1 ) ) );

                      strcat( buf2, buf );
                  }
                  i++;
              }
              vnum++;
          }
      }

      if( buf2 != NULL )
      {
          if ( pnote != NULL && vnum ) /* More notes to be read; page cut off the rest */
          {
            sprintf(buf,
                "\n\rThere are %d pages of %s.  To view the next one, use %s guild %d.\n\r",
                (k / MAX_NOTES_PER_LIST)+1, list_name,
                cmd_name, vnumoffset / MAX_NOTES_PER_LIST + 2 );

            buf2 = ( char* )( realloc( buf2, sizeof( char ) *
                                           ( strlen( buf ) +
                                             strlen( buf2 ) + 1 ) ) );

            strcat( buf2, buf );
          }

          page_to_char( buf2, ch );
          free( buf2 );
          buf2 = NULL;

      }
      if (!vnum)
        {
          switch(type)
            {
            case NOTE_NOTE:
              send_to_char("There are no guild notes for you.\n\r",ch);
              break;
            case NOTE_IDEA:
              send_to_char("There are no guild ideas for you.\n\r",ch);
              break;
            case NOTE_HISTORY:
              send_to_char("There are no guild histories for you.\n\r",ch);
              break;
            case NOTE_NEWS:
              send_to_char("There is no guild news for you.\n\r",ch);
              break;
            case NOTE_CHANGES:
              send_to_char("There are no guild changes for you.\n\r",ch);
              break;
            case NOTE_TROUBLE:
              send_to_char( "There are no guild troublemakers for you.  Lucky guy!\n\r", ch );
              break;
            }
        }
      return;
    }

  if ( !str_prefix( arg, "unread" ) )
    {
      vnum = 0;
      if ( IS_NPC(ch) ) return;
      for ( pnote = *list; pnote != NULL; pnote = pnote->next )
	{
	  if ( is_note_to( ch, pnote ) )
	    {
              if (!hide_note(ch,pnote)){
				sprintf( buf, "[%3d%s] %s %s %s%s %s%s%s%s%s\n\r",
					vnum, 
                    hide_note(ch,pnote) ? ( pnote->persist ? "P" : " " ) : "N",
					pnote->date,
					"-",
					pnote->sender,
					":",
					(int) is_note_race(ch,pnote) ? "(RACE) " : "",
					is_note_guild(ch,pnote) ? "(GUILD) ": "",
					is_note_personal(ch,pnote) ? "(PERSONAL) " : "",
					( IS_BRAWLER(ch) && is_name( "brawler", pnote->to_list ) ) ? "(BRAWLER) " : "",
					pnote->subject );
					send_to_char( buf, ch );
	      }
	      vnum++;
	    }
	}
      if (!vnum)
	{
	  switch(type)
	    {
	    case NOTE_NOTE:	
	      send_to_char("There are no notes for you.\n\r",ch);
	      break;
	    case NOTE_IDEA:
	      send_to_char("There are no ideas for you.\n\r",ch);
	      break;
	    case NOTE_HISTORY:
	      send_to_char("There are no histories for you.\n\r",ch);
	      break;
	    case NOTE_NEWS:
	      send_to_char("There is no news for you.\n\r",ch);
	      break;
	    case NOTE_CHANGES:
	      send_to_char("There are no changes for you.\n\r",ch);
	      break;
	    case NOTE_TROUBLE:
	      send_to_char( "There are no troublemakers for you.  Lucky guy!\n\r", ch );
	      break;
	    }
	}
      return;
    }
    if ( !str_prefix( arg, "search" ) )
    {
      char searchstr[MAX_INPUT_LENGTH];
      vnum = 0;
      pnum = 0;

      if ( IS_NPC(ch) ) return;

      k = 0;

      argument = one_argument( argument, searchstr );
      for ( pnote = *list; pnote != NULL; pnote = pnote->next )
      {
          if ( is_note_to( ch, pnote ) 
          &&  (!str_infix( searchstr, pnote->to_list )
          ||   !str_infix( searchstr, pnote->sender )
          ||   !str_infix( searchstr, strip_color(pnote->subject) ) ) )
              k++;
      }

      if ( !IS_NULLSTR(argument) && !str_prefix( argument, "count" ) )
      {
           sprintf(buf, "A total of %d %s over %d pages match '%s'.\n\r",
               k, list_name, ( k / MAX_NOTES_PER_LIST ) + 1, searchstr );
           send_to_char( buf, ch );
           return;
      }

/* Very obscure overflow protection.  Increase if necessary. */ 
      vnumoffset = UMAX(UMIN(atoi(argument), 327),0);
      vnumoffset = UMIN(UMAX((vnumoffset - 1) * MAX_NOTES_PER_LIST,0), 32700);
      if ( vnumoffset < 1 )
          vnumoffset = 0;
      else
      {
          j = 0;
          for ( pnote = *list; pnote != NULL; pnote = pnote->next )
          {
              if ( is_note_to( ch, pnote ) )
              {
                  vnum++;
                  if ( !str_infix( searchstr, pnote->to_list )
                  ||   !str_infix( searchstr, pnote->sender )
                  ||   !str_infix( searchstr, strip_color(pnote->subject) ) )
                      j++;
              }
              if ( j > vnumoffset )
                  break;
          }
          if ( pnote == NULL )
          {
            sprintf(buf, "There are only %d search results over %d pages.\n\r",
               k, ( k / MAX_NOTES_PER_LIST ) + 1 );
            send_to_char( buf, ch );
            return;
          }
      }

      i = 0; /* <-- incremented only for positive search results */
      if ( pnote != NULL )
      {
          vnum--;
          notestart = pnote;
      }
      else
          notestart = *list;
      for ( pnote = notestart; pnote != NULL; pnote = pnote->next )
      {
          if ( is_note_to( ch, pnote ) )
          {
              if ( !str_infix( searchstr, pnote->to_list )
              ||   !str_infix( searchstr, pnote->sender )
              ||   !str_infix( searchstr, strip_color(pnote->subject) ) ) 
              {
                  if ( (i) >= MAX_NOTES_PER_LIST )
                       break;

					sprintf( buf, "[%3d%s] %s %s %s%s %s%s%s%s%s%s\n\r",
                     vnum, 
                     hide_note(ch,pnote) ? ( pnote->persist ? "P" : " " ) : "N",
                     pnote->date,
					"-",
                  pnote->sender,
					":",
                     (int) is_note_race(ch,pnote) ? "(RACE) ": "",
                     is_note_guild(ch,pnote) ? "(GUILD) ": "",
                     is_note_personal(ch,pnote) ? "(PERSONAL) " : "",
                     is_note_vamp(ch,pnote) ? "(BROOD) ": "",
                     ( IS_BRAWLER(ch) && is_name( "brawler", pnote->to_list ) ) 
                       ? "(BRAWLER) " : "",
                       pnote->subject );

                  if( buf2 == NULL )
                      buf2 = strdup( buf );
                  else
                  {
                      buf2 = ( char* )( realloc( buf2, sizeof( char ) *
                          ( strlen( buf ) +
                          strlen( buf2 ) + 1 ) ) );

                      strcat( buf2, buf );
                  }
                  i++;
              }
              vnum++;
          }
      }
      if( buf2 != NULL )
      {
          if ( pnote != NULL && vnum ) /* More notes to be read; cut off  */
          {
            sprintf(buf,
                "\n\rYour search returned %d results over %d pages.\n\rType '%s search %s %d' to continue.\n\r", k, 
                (k / MAX_NOTES_PER_LIST)+1,
                cmd_name, searchstr, vnumoffset / MAX_NOTES_PER_LIST + 2 );

            buf2 = ( char* )( realloc( buf2, sizeof( char ) *
                                           ( strlen( buf ) +
                                             strlen( buf2 ) + 1 ) ) );

            strcat( buf2, buf );
          }

          page_to_char( buf2, ch );
          free( buf2 );
          buf2 = NULL;

      }
      if (!vnum)
          send_to_char( "No results found.\n\r", ch );
      return;
    }

  if ( !str_prefix( arg, "remove" ) )
    {
      if ( IS_NPC(ch) ) return;
      if ( !is_number( argument ) )
        {
	  send_to_char( "Note remove which number?\n\r", ch );
	  return;
        }
 
      anum = atoi( argument );
      vnum = 0;
      for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
	  if ( is_note_to( ch, pnote ) && vnum++ == anum )
            {
	      note_remove( ch, pnote, FALSE );
	      send_to_char( "Ok.\n\r", ch );
	      return;
            }
        }
 
      sprintf(buf,"There aren't that many %s.",list_name);
      send_to_char(buf,ch);
      return;
    }
 
  if ( !str_prefix( arg, "delete" ) )
    {
      if ( IS_NPC(ch) ) return;
      if ( !IS_ADMIN( ch ) )
      {
         send_to_char( "Try 'note remove' instead.\n\r", ch );
         return;
      }
      if ( !is_number( argument ) )
        {
	  send_to_char( "Note delete which number?\n\r", ch );
	  return;
        }
 
      anum = atoi( argument );
      vnum = 0;
      for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
	  if ( is_note_to( ch, pnote ) && vnum++ == anum )
            {
	      note_remove( ch, pnote,TRUE );
	      send_to_char( "Ok.\n\r", ch );
	      return;
            }
        }

      sprintf(buf,"There aren't that many %s.",list_name);
      send_to_char(buf,ch);
      return;
    }

  if (!str_prefix(arg,"catchup"))
    {
      if ( IS_NPC(ch) ) return;
      switch(type)
	{
	case NOTE_NOTE:	
	  ch->pcdata->last_note = current_time;
	  break;
	case NOTE_IDEA:
	  ch->pcdata->last_idea = current_time;
	  break;
	case NOTE_HISTORY:
	  ch->pcdata->last_history = current_time;
	  break;
	case NOTE_NEWS:
	  ch->pcdata->last_news = current_time;
	  break;
	case NOTE_CHANGES:
	  ch->pcdata->last_changes = current_time;
	  break;
	case NOTE_TROUBLE:
	  ch->pcdata->last_trouble = current_time;
	  break;
	}
      return;
    }

  if ( !str_prefix( arg, "count" ) )
  {
       i = 0;
       for ( pnote = *list; pnote != NULL; pnote = pnote->next )
       {
           if ( is_note_to( ch, pnote ) )
               i++;
       }
       sprintf(buf, "You have a total of %d %s over %d pages.\n\r", 
           i, list_name, ( i / MAX_NOTES_PER_LIST ) + 1 );
       send_to_char( buf, ch );
       return;
  }

  /* below this point only certain people can edit notes */

  if( (type == NOTE_NEWS && !IS_TRUSTED(ch,ANGEL) )
      || (type == NOTE_CHANGES && !IS_TRUSTED(ch,ADMIN) )
      || (type == NOTE_TROUBLE && !IS_TRUSTED(ch,ANGEL) ) )
    {
      if ( IS_NPC(ch) ) return;
      sprintf(buf,"You aren't high enough level to write %s.\n\r",list_name);
      send_to_char(buf,ch);
      return;
    }

  if ( !str_cmp( arg, "+" ) )
    {
      note_attach( ch,type );
      if (ch->pnote->type != type)
	{
	  send_to_char(
		       "You already have a different note in progress.\n\r",ch);
	  return;
	}

/*    This is handled differently in desc + and deschalf +.  Odd. */
      if (strlen(ch->pnote->text)+strlen(argument) >= (MAX_STRING_LENGTH-MAX_INPUT_LENGTH))
	{
	  send_to_char( "Note too long.\n\r", ch );
	  return;
	}

      buffer = new_buf();

      add_buf(buffer,ch->pnote->text);
      add_buf(buffer,argument);
      add_buf(buffer,"\n\r");
      free_string( ch->pnote->text );
      ch->pnote->text = str_dup( buf_string(buffer) );
      free_buf(buffer);
      send_to_char( "Ok.\n\r", ch );
      return;
    }

  if (!str_cmp(arg,"-"))
    {
      int len;
      bool found = FALSE;

      note_attach(ch,type);
      if (ch->pnote->type != type)
        {
	  send_to_char(
		       "You already have a different note in progress.\n\r",ch);
	  return;
        }

      if (ch->pnote->text == NULL || ch->pnote->text[0] == '\0')
	{
	  send_to_char("No lines left to remove.\n\r",ch);
	  return;
	}

      strcpy(buf,ch->pnote->text);

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
		  free_string(ch->pnote->text);
		  ch->pnote->text = str_dup(buf);
		  return;
		}
	    }
	}
      buf[0] = '\0';
      free_string(ch->pnote->text);
      ch->pnote->text = str_dup(buf);
      return;
    }

  if ( !str_cmp( arg, "format" ) )
  {
      if ( IS_NPC( ch ) ) 
      {
           send_to_char( "NPCs can't do that.\n\r", ch );
           return;
      }
      note_attach( ch, type );
      ch->pnote->text = format_string( ch->pnote->text );
      send_to_char( "Note formatted.\n\r", ch );
      return;
  }

  if ( !str_cmp( arg, "desc" ) || !str_cmp( arg, "write") )
    {
      if(IS_NPC(ch)) 
      {
         send_to_char( "NPCs can't do that.\n\r", ch );
         return;
      }
      note_attach( ch, type );
      if ( argument[0] == '\0' )
	string_append( ch, &ch->pnote->text );
      return;
    }

  if ( !str_prefix( arg, "subject" ) )
    {
      note_attach( ch,type );
      if (ch->pnote->type != type)
        {
	  send_to_char(
		       "You already have a different note in progress.\n\r",ch);
	  return;
        }

      free_string( ch->pnote->subject );
      ch->pnote->subject = str_dup( argument );
      send_to_char( "Ok.\n\r", ch );
      return;
    }

  if ( !str_prefix( arg, "to" ) )
    {
      note_attach( ch,type );
      if (ch->pnote->type != type)
        {
	  send_to_char(
		       "You already have a different note in progress.\n\r",ch);
	  return;
        }
      free_string( ch->pnote->to_list );
      ch->pnote->to_list = str_dup( argument );

      if ( is_name( "all", ch->pnote->to_list ) )
      {
          sprintf(buf, "$N has started a%s %s to all.",
              !str_cmp(cmd_name, "idea") ? "n" : "", cmd_name );
          wiznet( buf, ch, NULL, WIZ_NOTES, 0, get_trust( ch ) );
      }

      send_to_char( "Ok.\n\r", ch );
      return;
    }

  if ( !str_prefix( arg, "clear" ) )
    {
      if ( ch->pnote != NULL )
	{
	  free_note(ch->pnote);
	  ch->pnote = NULL;
	}

      send_to_char( "Ok.\n\r", ch );
      return;
    }

  if ( !str_prefix( arg, "show" ) )
    {
      if ( ch->pnote == NULL )
	{
	  send_to_char( "You have no note in progress.\n\r", ch );
	  return;
	}

      if (ch->pnote->type != type)
	{
	  send_to_char("You aren't working on that kind of note.\n\r",ch);
	  return;
	}

    sprintf( buf, "%s%s: %s\n\rTo: %s\n\r",
      ch->pnote->sender,
        ( ch->pnote->persist ? " [P]" : "" ),
        ch->pnote->subject,
        ch->pnote->to_list );
      send_to_char( buf, ch );
      send_to_char( ch->pnote->text, ch );
      return;
    }

    if ( !str_prefix( arg, "persist" ) || !str_prefix( arg, "sticky" ) )
    {

        if ( !IS_IMMORTAL( ch ) )
        {
	    send_to_char( "Only immortals may post persistent notes.\n\r", ch );
            return;
        }

        if ( !is_number( argument ) || argument[0] == '\0' )
        {
            if ( ch->pnote == NULL )
            {
                send_to_char( "You have no note in progress.\n\r", ch );
                return;
            }

            if ( ch->pnote->persist )
            {
	        ch->pnote->persist = FALSE;
                send_to_char( "Persistency turned off.\n\r", ch );
            }
            else
            {
                ch->pnote->persist = TRUE;
                send_to_char( "Persistency turned on.\n\r", ch );
            }

            return;
        }

        anum = atoi( argument );
        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && vnum++ == anum )
            {
                if ( pnote->persist )
                {
                    pnote->persist = FALSE;
                    send_to_char( "Persistency turned off.\n\r", ch );
                }
                else
                {
                    pnote->persist = TRUE;
                    send_to_char( "Persistency turned on.\n\r", ch );
                }
                save_notes(pnote->type);
                return;
            }
        }

        sprintf(buf,"There aren't that many %s.\n\r",list_name);
        send_to_char(buf,ch);
 
 	return;
    }

   /* 
    * note copy - 6/8/2011 gkl 
    */
    if ( !str_prefix( arg, "copy" ) )
    {
      note_attach( ch, type );
      if (ch->pnote->type != type)
      {
	  send_to_char( "You already have a different note in progress.\n\r",
            ch);
	  return;
      }

      /* find the target note */
      if ( !is_number( argument ) )
      {
        send_to_char( "Copy which note?\n\r", ch );
        return;
      }
      anum = atoi( argument );
      vnum = 0;
      for ( pnote = *list; pnote != NULL; pnote = pnote->next )
      {
          if ( is_note_to( ch, pnote ) && vnum++ == anum )
              break;
      }
      if ( pnote == NULL )
      {
          sprintf(buf,"There aren't that many %s.",list_name);
          send_to_char(buf,ch);
          return;
      }

      /* copy the stuff */
      free_string( ch->pnote->date );
      ch->pnote->date    = str_dup( pnote->date );

      free_string( ch->pnote->to_list );
      ch->pnote->to_list = str_dup( pnote->to_list );

      free_string( ch->pnote->subject );
      ch->pnote->subject = str_dup( pnote->subject );

      free_string( ch->pnote->text );
      ch->pnote->text    = str_dup( pnote->text );

      if ( is_name( "all", ch->pnote->to_list ) )
      {
          sprintf(buf, "$N has copied a%s %s to all.",
              !str_cmp(cmd_name, "idea") ? "n" : "", cmd_name );
          wiznet( buf, ch, NULL, WIZ_NOTES, 0, get_trust( ch ) );
      }

      send_to_char( "Ok.\n\r", ch );
      return;
    }

    if ( !str_prefix( arg, "replace" ) )
    {
      NOTE_DATA *prev;

      if ( IS_NPC( ch ) )
          return;

      if (ch->level < 50)
      {
	send_to_char( "You must be level 50 to replace notes.\n\r", ch );
	return;
      }	
      if( IS_SET( ch->act_bits, ACT_NOAPPROVE ) )
      {
        send_to_char( "You must be approved to post.  See 'help approval'.\n\r",
          ch );
        return;
      }
      if ( ch->pnote == NULL )
	{
	  send_to_char( "You have no note in progress.\n\r", ch );
	  return;
	}
	
      if ( ch->pnote->type == NOTE_CHANGES
      &&   ch->level < LEVEL_IMMORTAL )
      {
	send_to_char("You can't post to that board.\n\r",ch);
	return;
      }
	  
      if (ch->pnote->type != type)
      {
        send_to_char("You aren't working on that kind of note.\n\r",ch);
        return;
      }

      if (!str_cmp(ch->pnote->to_list,""))
      {
        send_to_char( "You need to provide a recipient.\n\r", ch);
        return;
      }

      if (!str_cmp(ch->pnote->subject,""))
      {
        send_to_char("You need to provide a subject.\n\r",ch);
        return;
      }

      /* find the target note */
      if ( !is_number( argument ) )
      {
        send_to_char( "Replace which note?\n\r", ch );
        return;
      }
      anum = atoi( argument );
      vnum = 0;
      prev = NULL;
      for ( pnote = *list; pnote != NULL; pnote = pnote->next )
      {
          if ( is_note_to( ch, pnote ) && vnum++ == anum )
              break;
          prev = pnote;
      }
      if ( pnote == NULL )
      {
          sprintf(buf,"There aren't that many %s.",list_name);
          send_to_char(buf,ch);
          return;
      }
      if ( str_cmp( ch->name, pnote->sender )
      &&  !IS_ADMIN( ch ) )
      {
          send_to_char( "You can only replace notes you've posted.\n\r", ch );
          return;
      }

      ch->pnote->date			= str_dup( pnote->date );
      ch->pnote->date_stamp		= pnote->date_stamp;

      /* do the actual replacement.  replace pnote with ch->pnote */
      ch->pnote->next = pnote->next;
      if ( prev == NULL )
          *list = ch->pnote;
      else
          prev->next = ch->pnote;
      /* at this point, all references to the old pnote are now to ch->pnote */

      /* instead of destroying the old one, swap it back to ch->pnote */
      ch->pnote = pnote;
      ch->pnote->next = NULL;

      /* all done.  resume regular bs */
      if ( is_name( "all", ch->pnote->to_list ) )
      {
          sprintf(buf, "$N has replaced a%s %s to all.", 
              !str_cmp(cmd_name, "idea") ? "n" : "", cmd_name );
          wiznet( buf, ch, NULL, WIZ_NOTES, 0, get_trust( ch ) );
      }
      send_to_char(
          "Replaced.  You are now editing the note you just wiped out.\n\r",ch);
      return;
    }


    if ( !str_prefix( arg, "post" ) || !str_prefix(arg, "send"))
    {
      char *strtime;

      if (ch->level < 3){
	send_to_char( "You must be level 3 or higher to post.\n\r", ch );
	return;
      }	

      if( IS_SET( ch->act_bits, ACT_NOAPPROVE ) )
	{
	  send_to_char( "You must be approved to post.  See 'help approval'.\n\r", ch );
	  return;
	}

      if ( ch->pnote == NULL )
	{
	  send_to_char( "You have no note in progress.\n\r", ch );
	  return;
	}
	
      if ((ch->pnote->type == NOTE_CHANGES) && (ch->level < LEVEL_IMMORTAL)){
	send_to_char("You can't post to that board.\n\r",ch);
	return;
      }
	  
      if (ch->pnote->type != type)
        {
	  send_to_char("You aren't working on that kind of note.\n\r",ch);
	  return;
        }

      if (!str_cmp(ch->pnote->to_list,""))
	{
	  send_to_char(
		       "You need to provide a recipient (name, all, imp, admin or immortal).\n\r",
		       ch);
	  return;
	}

      if (!str_cmp(ch->pnote->subject,""))
	{
	  send_to_char("You need to provide a subject.\n\r",ch);
	  return;
	}

      ch->pnote->next			= NULL;
      strtime				= ctime( &current_time );
      strtime[strlen(strtime)-1]	= '\0';
      ch->pnote->date			= str_dup( strtime );
      ch->pnote->date_stamp		= current_time;

      if ( is_name( "all", ch->pnote->to_list ) )
      {
          sprintf(buf, "$N has posted a%s %s to all.", 
              !str_cmp(cmd_name, "idea") ? "n" : "", cmd_name );
          wiznet( buf, ch, NULL, WIZ_NOTES, 0, get_trust( ch ) );
      }

      if ( !IS_NPC( ch )
      &&   is_guild( ch )
      &&   is_name( guild_table[ch->guild].name, ch->pnote->to_list ) )
          ch->pcdata->guildnotes++;

      append_note(ch->pnote);
      ch->pnote = NULL;
      send_to_char("Posted.\n\r",ch);
      return;
    }

  send_to_char( "You can't do that.\n\r", ch );
  return;
}

/*
 * Manually free all notes before shutdown.  NEVER invoke this on a running 
 * game; it WILL irreparably destroy all boards!
 *
 * Used in conjunction with Valgrind profiling
 *
 * 6/8/2011 gkl
 *
void free_board_lists( void )
{
    NOTE_DATA **list;
    NOTE_DATA *pnote; 
    NOTE_DATA *note_next;
    int i, count, j;
    char buf[MAX_STRING_LENGTH];

    for ( i = 0; i <= 6; i++ )
    {
        switch(i)
        {
            default:
                continue;
            case NOTE_NOTE:
                list = &note_list;
                break;
            case NOTE_IDEA:
                list = &idea_list;
                break;
            case NOTE_HISTORY:
                list = &history_list;
                break;
            case NOTE_NEWS:
                list = &news_list;
                break;
            case NOTE_CHANGES:
                list = &changes_list;
                break;
            case NOTE_TROUBLE:
                list = &trouble_list;
                break;
        }
        count = 0;
        j = 0;
        for ( pnote = *list; pnote != NULL; pnote = note_next )
        {
            note_next = pnote->next;
            sprintf( buf, "Freeing %d/%d...\n", ++j, count );
            log_string(buf);
            free_note(pnote);
        }
    }
    return;
}
 *
 */

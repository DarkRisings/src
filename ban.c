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
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"


BAN_DATA* ban_list;

/* do functions and external function*/
void do_ban();
void do_permban();
void do_allow();
bool check_ban();

/* auxillary functions */
void save_bans();
void load_bans();
void insert_ban();
char* remove_ban();
bool is_banned();
void ban_site( CHAR_DATA* ch, char* argument, bool fPerm );
void show_bans();

void do_ban( CHAR_DATA* ch, char* argument )
{
  ban_site( ch, argument, FALSE );
}

void do_permban( CHAR_DATA* ch, char* argument )
{
  ban_site( ch, argument, TRUE );
}

void do_allow( CHAR_DATA* ch, char* argument )
{
  char arg[ MAX_INPUT_LENGTH ];
  char buf[ MAX_STRING_LENGTH ];
  char* errmsg;
  
  one_argument( argument, arg );
  
  errmsg = remove_ban( ch, arg );
  
  save_bans();
  sprintf( buf, "%s", errmsg );
  
  send_to_char( buf, ch );
}

bool check_ban( char* site, int type )
{
  return is_banned( site, type );
}

/*
 * converts string-type IP WWW.XXX.YYY.ZZZ to double WWWXXXYYY.ZZZ
 * fills in 0 for missing characters
 * ex.  "127.0.0.1" converts to 127000000.001
 */
double ban_sortkey( const char* ban_name )
{
  char* ban_copy;
  char* ban_copy_ref;
  char* tokens[ 4 ];
  char* temp_token;
  int i;
  int j;
  double value;
  
  ban_copy = strdup( ban_name );
  ban_copy_ref = ban_copy;

  for( i = 0; i < 4; i++ )
    {
      temp_token = strsep( &ban_copy, "." );


      if( temp_token == NULL )
	{
	  tokens[ i ] = strdup( "000" );
	}
      else if( ( atoi( temp_token ) == 0 ) )
	{
	  tokens[ i ] = strdup( "000" );
	}
      else
	{
	  if( strlen( temp_token ) == 3 )
	    tokens[ i ] = strdup( temp_token );
	  else
	    {
	      tokens[ i ] = ( char* )( malloc( sizeof( char ) * 4 ) );
	      
	      for( j = 0; j < 4; j++ )
		{
		  tokens[ i ][ j ] = '\0';
		}

	      tokens[ i ][ 0 ] = '0';

	      if( strlen( temp_token ) == 1 )
		{
		  tokens[ i ][ 1 ] = '0';
		}

	      strcat( tokens[ i ], temp_token );
	    }
	}
	      
    }

  value = atoi( tokens[ 0 ] ) * 1000000;
  value += atoi( tokens[ 1 ] ) * 1000;
  value += atoi( tokens[ 2 ] );
  value += ( (double)atoi( tokens[ 3 ] ) / ( double )(1000) );

  for( i = 0; i < 4; i++ )
    free( tokens[ i ] );

  free( ban_copy_ref );

  return value;

}


/*
 * saves the bans to a file
 */
void save_bans( void )
{
  BAN_DATA* ban_ptr;
  FILE* fp;
  bool found = FALSE;

  fclose( fpReserve );

  if( ( fp = fopen( BAN_FILE, "w" ) ) == NULL )
    {
      perror( BAN_FILE );
    }

  for( ban_ptr = ban_list; ban_ptr != NULL; ban_ptr = ban_ptr->next )
    {
      if( IS_SET( ban_ptr->ban_flags, BAN_PERMANENT ) )
	{
	  found = TRUE;
	  fprintf( fp, "%-20s %-2d %-8s %-10d %s~\n",
		   ban_ptr->name,
		   ban_ptr->level,
		   print_flags( ban_ptr->ban_flags ),
		   (int)ban_ptr->stamp,
		   ban_ptr->comment );
	}
    }

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );

  if( !found )
    unlink( BAN_FILE );
}


/*
 * loads the bans from a file
 */
void load_bans( void )
{
  FILE* fp;
  BAN_DATA* pban;
  
  ban_list = NULL;

  if( ( fp = fopen( BAN_FILE, "r" ) ) == NULL )
    {
      perror( BAN_FILE );
    }
  else
    {
      while( !feof( fp ) )
	{
	  pban = new_ban();

	  pban->name = str_dup( fread_word( fp ) );
	  pban->level = fread_number( fp );
	  pban->ban_flags = fread_flag( fp );
	  pban->stamp = fread_number( fp );
	  pban->comment = str_dup( fread_string( fp ) );
	  pban->next = NULL;

	  fread_to_eol( fp );

	  insert_ban( pban );
	}
    }
}



/*
 * inserts ban into linked list ban_list
 */
void insert_ban( BAN_DATA* new_ban )
{

  BAN_DATA* before_point;
  BAN_DATA* after_point;
  bool point_found;

  before_point = NULL;
  after_point = ban_list;
  point_found = FALSE;

  while( !point_found )
    {
      if( after_point == NULL )
	{
	  point_found = TRUE;
	}
      else if( ban_sortkey( new_ban->name ) < ban_sortkey( after_point->name ) )
	{
	  point_found = TRUE;
	}
      else
	{
	  before_point = after_point;
	  after_point = after_point->next;
	}
    }

  if( before_point == NULL )
    {
      ban_list = new_ban;
    }
  else
    {
      before_point->next = new_ban;
    }
  
  new_ban->next = after_point;
}



/*
 * removes ban from linked list ban_list
 * returns status message
 */
char* remove_ban( CHAR_DATA* ch, const char* host )
{
  BAN_DATA* pban;
  BAN_DATA* ban_before;
  bool done = FALSE;
  bool found = FALSE;
  char* errmsg = NULL;
  char wiz_buf[ MAX_STRING_LENGTH ];

  pban = ban_list;
  ban_before = NULL;

  while( !done )
    {
      if( pban == NULL )
	{
	  done = TRUE;

	  if( !found )
	    errmsg = strdup( "No such ban.\n\r" );
	}
      else if( strcmp( host, pban->name ) == 0 )
	{
	  done = TRUE;
	  
	  if( ch != NULL && ( pban->level > get_trust( ch ) ) )
	    errmsg = strdup( "You are not powerful enough.\n\r" );
	  else
	    found = TRUE;
		  
	}
      else
	{
	  ban_before = pban;
	  pban = pban->next;
	}
    }

  if( found )
    {
      errmsg = strdup( "Ban removed.\n\r" );
      if( ban_before != NULL )
	{
	  ban_before->next = pban->next;
	}
      else
	{
	  ban_list = pban->next;
	}

      sprintf( wiz_buf, "%s has unbanned %s%s.",
	       ch->name,
	       pban->name,
	       ( IS_SET( pban->ban_flags, BAN_SUFFIX ) ? "*" : "" ) );
      wiznet( wiz_buf, NULL, NULL, WIZ_BANS, 0, DEITY );

      free_ban( pban );
    }

  return errmsg;
      
}


/*
 * checks if a ban is present for ip_addr
 */
bool is_banned( const char* site, int type )
{
  BAN_DATA* pban;
  char* host;
  char* host_ref;
  bool done = FALSE;
  bool banned = FALSE;

  host = strdup( site );
  host_ref = host;

  pban = ban_list;

  while( !done && !banned && ( pban != NULL ) )
    {
      if( IS_SET( pban->ban_flags, BAN_SUFFIX ) )
	{
	  if( strstr( host, pban->name ) != NULL 
          &&  str_prefix(host, pban->name) )
	    {
	      done = TRUE;
	    }
	}
      else
	{
	  if( strcmp( host, pban->name ) == 0 )
	    {
	      done = TRUE;
	    }
	}

      if( done )
	{
	  if( IS_SET( pban->ban_flags, type ) )
	    {
	      banned = TRUE;
	    }
	  else
	    {
	      done = FALSE;
	    }
	}

      if( !banned )
	{
	  pban = pban->next;
	}
    }

  free( host_ref );
  return banned;
}


/*
 * performs an actual ban
 */
void ban_site( CHAR_DATA* ch, char* argument, bool fPerm )
{
  int type;
  char* name;
  bool suffix = FALSE;
  char* comment;
  BAN_DATA* pban;
  char buf[ MAX_STRING_LENGTH ];
  char arg1[ MAX_INPUT_LENGTH ];
  char arg2[ MAX_INPUT_LENGTH ];

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  comment = argument;

  if( arg1[ 0 ] == '\0' )
    {
      show_bans( ch );
      return;
    }

  if( arg2[ 0 ] == '\0' || !str_prefix( arg2, "all" ) )
    type = BAN_ALL;
  else if( !str_prefix( arg2, "newbies" ) )
    type = BAN_NEWBIES;
  else if( !str_prefix( arg2, "permit" ) )
    type = BAN_PERMIT;
  else
    {
      send_to_char( "Acceptable ban types are all, newbies, and permit.\n\r", ch );
      return;
    }

  name = arg1;
  
  if( name[ 0 ] == '*' )
    {
      send_to_char( "No more prefix banning.\n\r", ch );
      return;
    }
  
  if (name[strlen(name) - 1] == '*')
    {
      suffix = TRUE;
      name[strlen(name) - 1] = '\0';
    }
  
  if (strlen(name) == 0)
    {
      send_to_char("You have to ban SOMETHING.\n\r",ch);
      return;
    }
  
  if (strlen(comment) == 0)
    {
      send_to_char("You have to have some type of commentary on the ban.\n\r",ch);
      return;
    }
  
  if( is_banned( name, type ) )
    {
      send_to_char( "That site is already banned.\n\r", ch );
      return;
    }

  pban = new_ban();
  pban->name = str_dup( name );
  pban->level = get_trust( ch );
  pban->comment = str_dup( comment );
  pban->ban_flags = type;
  pban->stamp = time( 0 );
  
  if( suffix )
    SET_BIT( pban->ban_flags, BAN_SUFFIX );
  if( fPerm )
    SET_BIT( pban->ban_flags, BAN_PERMANENT );
  
  pban->next = NULL;
  
  insert_ban( pban );
  save_bans();
  sprintf( buf, "%s has been banned.\n\r", pban->name );
  send_to_char( buf, ch );

  sprintf( buf, "%s has %sbanned %s%s.",
	   ch->name,
	   ( IS_SET( pban->ban_flags, BAN_PERMANENT ) ? "perm-" : "" ),
	   pban->name,
	   ( IS_SET( pban->ban_flags, BAN_SUFFIX ) ? "*" : "" ) );
  wiznet( buf, NULL, NULL, WIZ_BANS, 0, DEITY );

  return;
      
}



/*
 * shows the ban list to ch
 */
void show_bans( CHAR_DATA* ch )
{
  BAN_DATA* pban;
  char timestamp[ 12 ];
  char buf[ MAX_STRING_LENGTH ];
  char buf2[ MAX_STRING_LENGTH ];
  BUFFER* buffer;

  if (ban_list == NULL)
    {
      send_to_char("No sites banned at this time.\n\r",ch);
      return;
    }
  buffer = new_buf();
  

  /* ONE OFF */
  save_bans();

  sprintf( buf, "%-17s %-12s %-5s  %-7s  %4s  %7s\n\r",
	   "ADDRESS",
	   "TIMESTAMP",
	   "LEVEL",
	   "TYPE",
	   "PERM",
	   "COMMENT" );

  add_buf( buffer, buf );

  add_buf( buffer, "=====================================================================================\n\r" );

  for( pban = ban_list; pban != NULL; pban = pban->next )
    {

      strftime( timestamp, 12, "%b %d %Y", localtime( &( pban->stamp ) ) );
      
      sprintf( buf2, "%s%s ",
	       pban->name,
	       IS_SET( pban->ban_flags, BAN_SUFFIX ) ? "*": " " );
      
      sprintf( buf, "%-17s %-12s  %5d  %-7s  %4s  %s\n\r",
	       buf2, timestamp, pban->level,
	       IS_SET( pban->ban_flags, BAN_NEWBIES ) ? "newbies" :
	       IS_SET( pban->ban_flags, BAN_PERMIT ) ? "permit" :
	       IS_SET( pban->ban_flags, BAN_ALL ) ? "all" : "",
	       IS_SET( pban->ban_flags, BAN_PERMANENT ) ? "perm" : "temp",
	       pban->comment );
      
      add_buf( buffer, buf );
    }
  
  page_to_char( buf_string(buffer), ch );
  free_buf(buffer);
  return;
}















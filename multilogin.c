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
#include <netinet/in.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "merc.h"
#include "recycle.h"

MULTILOGIN_DATA *multilogin_list;

extern MULTILOGIN_DATA* new_multilogin( void );
extern MULTILOGIN_DATA* free_multilogin( MULTILOGIN_DATA* multilogin );

void save_multilogins(void)
{
  MULTILOGIN_DATA *pmultilogin;
  FILE *fp;
  bool found = FALSE;

  fclose( fpReserve ); 
  if ( ( fp = fopen( MULTILOGIN_FILE, "w" ) ) == NULL )
    {
      perror( MULTILOGIN_FILE );
    }
  
  for (pmultilogin = multilogin_list; pmultilogin != NULL; pmultilogin = pmultilogin->next)
    {
      found = TRUE;
      fprintf(fp,"%-20s %-2d %-8s %s~\n",
	      pmultilogin->name,pmultilogin->level,
	      print_flags(pmultilogin->multilogin_flags),
	      pmultilogin->comment);
    }
  
  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );
  if (!found)
    unlink(MULTILOGIN_FILE);
}



/* converts a text IP AAA.BBB.CCC.DDD to a double AAA.BBBCCCDDD 
double calc_ip( const char* ip_addr )
{
  char* p_ip;
  char* p_ip_ref;
  double total;

  p_ip = strdup( ip_addr );
  p_ip_ref = p_ip;

  total = (double)( atoi( strsep( &p_ip, "." ) ) );
  total += ((double)( atoi( strsep( &p_ip, "." ) ) ) )/1000;
  total += ((double)( atoi( strsep( &p_ip, "." ) ) ) )/1000000;
  total += ((double)( atoi( p_ip ) ) )/1000000000;

  free( p_ip_ref );
  return total;
}
*/



/* rewritten by Kesavaram to sort on load */
/* 06 Jan 2002 */
void load_multilogins(void)
{
  FILE* fp;
  MULTILOGIN_DATA* pmultilogin;
  MULTILOGIN_DATA* ins_point;
  MULTILOGIN_DATA* prev_point;
  double pmulti_ip;
  double ins_ip;
  bool found;
 
  if( ( fp = fopen( MULTILOGIN_FILE, "r" ) ) == NULL )
    {
      return;
    }

  while( !feof( fp ) )
    {
      pmultilogin = new_multilogin();
  
      pmultilogin->name = str_dup( fread_word( fp ) );
      pmultilogin->level = fread_number( fp );
      pmultilogin->multilogin_flags = fread_flag( fp );
      pmultilogin->comment = str_dup( fread_string( fp ) );

      fread_to_eol( fp );

      if( multilogin_list == NULL )
	{
	  multilogin_list = pmultilogin;
	}
      else
	{
	  ins_point = multilogin_list;
	  prev_point = NULL;

	  pmulti_ip = ban_sortkey( pmultilogin->name );

	  found = FALSE;

	  while( !found )
	    {
	      if( ins_point == NULL )
		{
		  found = TRUE;
		}
	      else
		{
		  ins_ip = ban_sortkey( ins_point->name );

		  if( pmulti_ip < ins_ip )
		    {
		      found = TRUE;
		    }
		  else
		    {
		      prev_point = ins_point;
		      ins_point = ins_point->next;
		    }
		}
	    }

	  pmultilogin->next = ins_point;
	  
	  if( prev_point != NULL )
	    {
	      prev_point->next = pmultilogin;
	    }
	  else
	    {
	      multilogin_list = pmultilogin;
	    }
	}
    }
  
  fclose( fp );
}


bool check_multilogin(char *site,int type)
{
  MULTILOGIN_DATA* pmultilogin;
  char host[ MAX_STRING_LENGTH ];
  bool found = FALSE;

  strcpy( host, capitalize( site ) );
  host[ 0 ] = LOWER( host[ 0 ] );

  pmultilogin = multilogin_list;

  while( !found && pmultilogin != NULL )
  {
      if ( IS_SET(pmultilogin->multilogin_flags, MULTILOGIN_SUFFIX ) )
      {
          if ( strstr( host, pmultilogin->name ) != NULL
          &&   str_prefix( host, pmultilogin->name ) )
              found = TRUE;
      }
      else
      {
          if( !strcmp( host, pmultilogin->name ) )
	      found = TRUE;
      }

      pmultilogin = pmultilogin->next;
  }

  return found;

}
/*
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
*/

void multilogin_site(CHAR_DATA *ch, char *argument, bool fPerm)
{
    char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char *name;
    char *comment;
    BUFFER *buffer;
    MULTILOGIN_DATA *pmultilogin, *prev;
    bool prefix = FALSE,suffix = FALSE;
    int type;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);
    argument = one_argument(argument,arg3);

    if ( arg1[0] == '\0' )
    {
	if (multilogin_list == NULL)
	{
	    send_to_char("No sites listed at this time.\n\r",ch);
	    return;
  	}
	buffer = new_buf();

        add_buf(buffer,"Multilogin sites   level  type        comment\n\r");
	add_buf(buffer,"----------------------------------------------------\n\r");
        for (pmultilogin = multilogin_list;pmultilogin != NULL;pmultilogin= pmultilogin->next)
        {
	    sprintf(buf2,"%s%s%s",
		IS_SET(pmultilogin->multilogin_flags,MULTILOGIN_PREFIX) ? "*" : "",
		pmultilogin->name,
		IS_SET(pmultilogin->multilogin_flags,MULTILOGIN_SUFFIX) ? "*" : "");
	    sprintf(buf," %-15s    %-3d    %-9s   %s\n\r",
		buf2, pmultilogin->level,
		IS_SET(pmultilogin->multilogin_flags,MULTILOGIN_SCHOOL) ? "school" :
		IS_SET(pmultilogin->multilogin_flags,MULTILOGIN_HOME_LAN) ? "home LAN" :
		IS_SET(pmultilogin->multilogin_flags,MULTILOGIN_COMPANY) ? "company"
		: "unknown",
		pmultilogin->comment);
	    add_buf(buffer,buf);
        }

        page_to_char( buf_string(buffer), ch );
	free_buf(buffer);
        return;
    }

    /* find out what type of ban */
    if (!str_prefix(arg2, "?"))
    {
	send_to_char("Syntax: multilogin <IP> <type> <comment>\n\r",ch);
	send_to_char("	 acceptable types are school, company, home-lan",ch);
	send_to_char("   i.e. multilogin 122.32.41.18 school u-of-m\n\r",ch);
	return;
    }
    else if (arg2[0] == '\0' || !str_prefix(arg2,"school"))
	type = MULTILOGIN_SCHOOL;
    else if (!str_prefix(arg2,"company"))
	type = MULTILOGIN_COMPANY;
    else if (!str_prefix(arg2,"home-lan"))
	type = MULTILOGIN_HOME_LAN;
    else
    {
	send_to_char("Acceptable network types are home-lan, school and company.\n\r",
	    ch); 
	return;
    }

    name = arg1;

    if (name[0] == '*')
    {
	prefix = TRUE;
	name++;
    }

    if (name[strlen(name) - 1] == '*')
    {
	suffix = TRUE;
	name[strlen(name) - 1] = '\0';
    }

    if (strlen(name) == 0)
    {
	send_to_char("SOMETHING must be the IP.\n\r",ch);
	return;
    }

    comment = arg3;

    if (strlen(comment) == 0)
    {
	send_to_char("You have to have some type of commentary.\n\r",ch);
	return;
    }

    prev = NULL;
    for ( pmultilogin = multilogin_list; pmultilogin != NULL; prev = pmultilogin, pmultilogin = pmultilogin->next )
    {
        if (!str_cmp(name,pmultilogin->name))
        {
	    if (pmultilogin->level > get_trust(ch))
	    {
            	send_to_char( "That was set by a higher power.\n\r", ch );
            	return;
	    }
	    else
	    {
		if (prev == NULL)
		    multilogin_list = pmultilogin->next;
		else
		    prev->next = pmultilogin->next;
		free_multilogin(pmultilogin);
	    }
        }
    }

    pmultilogin = new_multilogin();
    pmultilogin->name = str_dup(name);
    pmultilogin->level = get_trust(ch);
    pmultilogin->comment = str_dup(comment);
    pmultilogin->multilogin_flags = type;

    if (prefix)
	SET_BIT(pmultilogin->multilogin_flags,MULTILOGIN_PREFIX);
    if (suffix)
	SET_BIT(pmultilogin->multilogin_flags,MULTILOGIN_SUFFIX);

    pmultilogin->next  = multilogin_list;
    multilogin_list    = pmultilogin;
    save_multilogins();
    sprintf(buf,"%s has been listed.\n\r",pmultilogin->name);
    send_to_char( buf, ch );
    return;
}

void do_multilogin(CHAR_DATA *ch, char *argument)
{
    multilogin_site(ch,argument,TRUE);
}

void do_unallow( CHAR_DATA *ch, char *argument )                        
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    MULTILOGIN_DATA *prev;
    MULTILOGIN_DATA *curr;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Remove which site from the multilogin list?\n\r",
		ch );
        return;
    }

    prev = NULL;
    for ( curr = multilogin_list; curr != NULL; prev = curr, curr = curr->next )
    {
        if ( !str_cmp( arg, curr->name ) )
        {
            if ( prev == NULL )
                multilogin_list   = multilogin_list->next;
            else
                prev->next = curr->next;

            free_multilogin(curr);
	    sprintf(buf,"Multilogin permission on %s lifted.\n\r",arg);
            send_to_char( buf, ch );
	    save_multilogins();
            return;
        }
    }

    send_to_char( "Site is not listed.\n\r", ch );
    return;
}



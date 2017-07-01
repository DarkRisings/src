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
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
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

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#include <sys/types.h>
#include <sys/time.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/telnet.h>

#include "merc.h"
#include "recycle.h"
#include "brawler.h"
#include "lookup.h"

/* command procedures needed */
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_skills	);
DECLARE_DO_FUN(do_outfit	);
DECLARE_DO_FUN(do_unread	);


#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	args( ( int  ) );
extern	int	malloc_verify	args( ( void ) );
#endif

const	char	echo_off_str	[] = { (char) IAC, (char) WILL, (char) TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { (char) IAC, (char) WONT, (char) TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { (char) IAC, (char) GA, '\0' };

DECLARE_SPELL_FUN( spell_null );

extern void free_board_lists( void );

/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    god;		/* All new chars are gods!	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
bool		    newlock;		/* Game is newlocked		*/
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* time of this pulse */	
bool		    MOBtrigger = TRUE;  /* act() switch                 */
long                queststatus = 0;
QUEST_INFO          global_quest_info = { "", 1, 50, 0 };
int quad; // ticks of quad

void	game_loop_unix		args( ( int control ) );
int	init_socket		args( ( int port ) );
void	init_descriptor		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );

/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
bool	check_playing2		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void    bust_a_prompt           args( ( CHAR_DATA *ch ) );


int main( int argc, char **argv )
{
    struct timeval now_time;
    int port;
    int control;
    FILE *debug;

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

#ifdef MEMWATCH    
    mwInit();
#endif

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time 	= (time_t) now_time.tv_sec;
    strcpy( str_boot_time, smash_crlf( ctime( &current_time ) ) );

    /*
     * Macintosh console initialization.
     */

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = 4000;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    exit( 1 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    exit( 1 );
	}
    }

    /*
     * Run the game.
     */
    control = init_socket( port );
    boot_db();
    sprintf( log_buf, "ROM is ready to rock on port %d.", port );
    log_string( log_buf );
    game_loop_unix( control );
    close (control);

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );

/*  free_board_lists(); */

    if ( (debug = fopen( "shutdown.txt", "r" )) == NULL )
        log_string( "Will reboot now." );
    else
    {
        log_string( "Shutting down now." );
        fclose( debug );
    }


    free( social_table );

#ifdef MEMWATCH
    mwTerm();
#endif

    exit( 0 );
    return 0;
}




int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close(fd);
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(__SVR4) && !defined(__sun)
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close(fd);
	    exit( 1 );
	}
    }
#endif

    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
	perror("Init socket: bind" );
	close(fd);
	exit(1);
    }


    if ( listen( fd, 3 ) < 0 )
    {
	perror("Init socket: listen");
	close(fd);
	exit(1);
    }

    return fd;
}



extern void substitute_prefix( DESCRIPTOR_DATA* d, char* argument );

void game_loop_unix( int control )
{
  static struct timeval null_time;
  struct timeval last_time;
  
  signal( SIGPIPE, SIG_IGN );
  gettimeofday( &last_time, NULL );
  current_time = (time_t) last_time.tv_sec;

  /* Main loop */
  while ( !merc_down )
    {
      fd_set in_set;
      fd_set out_set;
      fd_set exc_set;
      DESCRIPTOR_DATA *d;
      int maxdesc;
	
#if defined(MALLOC_DEBUG)
      if ( malloc_verify( ) != 1 )
	abort( );
#endif

      /*
       * Poll all active descriptors.
       */
      FD_ZERO( &in_set  );
      FD_ZERO( &out_set );
      FD_ZERO( &exc_set );
      FD_SET( control, &in_set );
      maxdesc	= control;
      for ( d = descriptor_list; d; d = d->next )
	{
	  maxdesc = UMAX( maxdesc, d->descriptor );
	  FD_SET( d->descriptor, &in_set  );
	  FD_SET( d->descriptor, &out_set );
	  FD_SET( d->descriptor, &exc_set );
	}

      if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	  perror( "Game_loop: select: poll" );
	  exit( 1 );
	}

      /*
       * New connection?
       */
      if( FD_ISSET( control, &in_set ) )
	init_descriptor( control );

      /*
       * Kick out the freaky folks.
       */
      for ( d = descriptor_list; d != NULL; d = d_next )
	{
	  d_next = d->next;   
	  if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
	      FD_CLR( d->descriptor, &in_set  );
	      FD_CLR( d->descriptor, &out_set );
	      if ( d->character && d->connected == CON_PLAYING)
		save_char_obj( d->character );
	      d->outtop	= 0;
	      close_socket( d );
	    }
	}

      /*
       * Process input.
       */
      for ( d = descriptor_list; d != NULL; d = d_next )
	{
	  if( d->next != NULL )
	    {
	      d_next = d->next;
	    }
	  else
	    {
	      d_next = NULL;
	    }

	  d->fcommand	= FALSE;

	  if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
	      if ( d->character != NULL )
		d->character->timer = 0;
	      if ( !read_from_descriptor( d ) )
		{
		  FD_CLR( d->descriptor, &out_set );
		  if ( d->character != NULL && d->connected == CON_PLAYING)
		    save_char_obj( d->character );
		  d->outtop	= 0;
		  close_socket( d );
		  continue;
		}
	    }

	  if (d->character != NULL && d->character->daze > 0) {
	    --d->character->daze;
            if ((d->character->position == POS_DEAD) &&
                (d->character->daze <= 0))
	      d->character->position = POS_SLEEPING;
            else if ((d->character->position == POS_UNCONSCIOUS) &&
		     (d->character->daze <= 0)){
	      d->character->position = POS_RESTING;
	      send_to_char("You awake from your stupor.\n\r",d->character);
	      d->character->hit = d->character->max_hit/3;
	      d->character->mana = d->character->max_mana/3;
	      d->character->move = d->character->max_move/3;
	    }
	  }

	  if ( d->character != NULL && d->character->wait > 0 )
	    {
	      --d->character->wait;
	      continue;
	    }

	  read_from_buffer( d );
	  if ( d->incomm[0] != '\0' )
	    {
	      d->fcommand	= TRUE;
	      stop_idling( d->character );

	      /* OLC */
	      if ( d->showstr_point )
		show_string( d, d->incomm );
	      else
		if ( d->pString )
		  string_add( d->character, d->incomm );
		else
		  switch ( d->connected )
		    {
		    case CON_PLAYING:
		      if ( !run_olc_editor( d ) )
    		        substitute_prefix( d, d->incomm );
		      break;
		    default:
		      nanny( d, d->incomm );
		      break;
		    }

	      d->incomm[0]	= '\0';
	    }
	}



      /*
       * Autonomous game motion.
       */
      update_handler( );



      /*
       * Output.
       */
      for ( d = descriptor_list; d != NULL; d = d_next )
	{
	  d_next = d->next;

	  if ( ( d->fcommand || d->outtop > 0 )
	       &&   FD_ISSET(d->descriptor, &out_set) )
	    {
	      if ( !process_output( d, TRUE ) )
		{
		  if ( d->character != NULL && d->connected == CON_PLAYING)
		    save_char_obj( d->character );
		  d->outtop	= 0;
		  close_socket( d );
		}
	    }
	}

    

      /*
       * Synchronize to a clock.
       * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
       * Careful here of signed versus unsigned arithmetic.
       */
      {
	struct timeval now_time;
	long secDelta;
	long usecDelta;
	
	gettimeofday( &now_time, NULL );
	usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
	  + 1000000 / PULSE_PER_SECOND;
	secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	while ( usecDelta < 0 )
	  {
	    usecDelta += 1000000;
	  secDelta  -= 1;
	  }
	
	while ( usecDelta >= 1000000 )
	  {
	    usecDelta -= 1000000;
	    secDelta  += 1;
	  }
	
	if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	  {
	    struct timeval stall_time;
	    
	    stall_time.tv_usec = usecDelta;
	    stall_time.tv_sec  = secDelta;
	    if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
	      {
		perror( "Game_loop: select: stall" );
		exit( 1 );
	    }
	  }
	
      }
      gettimeofday( &last_time, NULL );
      current_time = (time_t) last_time.tv_sec;
      
    }
  
  return;
}





void init_descriptor( int control )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
    int desc;
    int size;

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }

    /*
     * Cons a new descriptor.
     */
    dnew = new_descriptor();

    dnew->descriptor	= desc;
    dnew->connected	= CON_GET_NAME;
    dnew->ansi          = FALSE;
    dnew->showstr_head	= NULL;
    dnew->showstr_point = NULL;
    dnew->outsize	= 2000;
    dnew->pEdit		= NULL;			/* OLC */
    dnew->pString	= NULL;			/* OLC */
    dnew->editor	= 0;			/* OLC */
    dnew->outbuf	= alloc_mem( dnew->outsize );

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );
    }
    else
    {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	int addr;

	addr = ntohl( sock.sin_addr.s_addr );
	sprintf( buf, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
	sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	log_string( log_buf );
  
/* *** This causes issues when players from sites with misconfigured DNS log in
   (e.g., Rigby the admin)                                         2/11/2011 gkl
        struct hostent *from;
	from = gethostbyaddr( (char *) &sock.sin_addr,
	    sizeof(sock.sin_addr), AF_INET );
	dnew->hostn = str_dup( from ? from->h_name : buf );
*** */
  
        dnew->host = str_dup(buf);
    }
	
    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    if( ( !check_ban( dnew->host, BAN_PERMIT ) ) && ( check_ban( dnew->host, BAN_ALL ) ) )
      {
	write_to_descriptor( desc,
			     "Your site has been banned from this mud.\n\r", 0 );

	sprintf( buf, "Attempted login from banned IP %s.", dnew->host );
	close( desc );
	free_descriptor(dnew);
	wiznet( buf, NULL, NULL, WIZ_BANS, 0, DEITY );
        log_string(buf);
	return;
      }


    /*
     * Init descriptor data.
     */
    dnew->next			= descriptor_list;
    descriptor_list		= dnew;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting2;

/*
	if ( help_greeting2[0] == '.' )
	    write_to_buffer( dnew, "Do you want ANSI? (Y/N)", 0 );
*/
        if ( help_greeting2[0] == '.' )
            send_to_desc( help_greeting2+1, dnew );
        else
            send_to_desc( help_greeting2 , dnew );
    }

    return;
}




void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
	sprintf( log_buf, "Closing link to %s.", ch->name );
	log_string( log_buf );

		if ( ch->pet && ch->pet->in_room == NULL )
		{
			char_to_room(ch->pet,get_room_index(ROOM_VNUM_LIMBO) );
			extract_char( ch->pet, TRUE );
		}

	/* cut down on wiznet spam when rebooting */
	if ( dclose->connected == CON_PLAYING && !merc_down)
	{
	    act_new( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM,
                POS_SPRAWLED, ch->invis_level );
	    wiznet("Net death has claimed $N.",ch,NULL,WIZ_LINKS,0,ch->level);
	    ch->desc = NULL;
	    if ( !IS_NPC(ch) ) 
                SET_BIT( ch->act_bits, PLR_LINKDEAD );
	}
	else
	{
            player_quitting = TRUE;
	    free_char(dclose->original ? dclose->original : 
		dclose->character );
            player_quitting = FALSE;

	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    free_descriptor(dclose);

    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
	return FALSE;
    }

    /* Snarf input. */

    for ( ; ; )
    {
	int nRead;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     * 3-22-08 gkl This is actually rather pointless.  This wiznet
     * function has never ever ever in the six years I've immed here
     * ever revealed anything meaningful; it just annoys imms and
     * players alike.
     */
/*
    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
        else
	{
	    if (++d->repeat >= 75 && 
		d->character &&  
		d->connected == CON_PLAYING)
	      {
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
		
		wiznet("Spam spam spam $N spam spam spam spam spam!",
		       d->character,NULL,WIZ_SPAM,0,get_trust(d->character));
		
		if (d->incomm[0] == '!')
		  wiznet(d->inlast,d->character,NULL,WIZ_SPAM,0,
			 get_trust(d->character));
		else
		  wiznet(d->incomm,d->character,NULL,WIZ_SPAM,0,
			 get_trust(d->character));

		int j;
		for( j = 0; j < 3; j++ )
		  send_to_desc( "STOP THE SPAM!!!!!\n\r", d );

		WAIT_STATE( d->character, PULSE_VIOLENCE );

		d->repeat = 0;

	    }
	}
    }
*/

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
  extern bool merc_down;

  /*
   * Bust a prompt.
   */
  if ( !merc_down )
    {
      if ( d->showstr_point )
	{
	  char* temp_buffer_ptr;
	  char temp_buffer[ MAX_STRING_LENGTH ];
	  
	  temp_buffer_ptr = temp_buffer;

	  if( d->character )
	    {
	      colourconv( temp_buffer_ptr, "{R\n\r[Hit Enter to continue reading, or any other key to finish]{x\n\r", d->character );
	    }
	  else
	    {
	      temp_buffer_ptr = strdup( "[Hit Enter to continue reading, or any other key to finish]\n\r" );
	    }

	  write_to_buffer( d, temp_buffer_ptr, 0 );

	}
      else if ( fPrompt && d->pString && d->connected == CON_PLAYING )
	write_to_buffer( d, "> ", 2 );
      else if ( fPrompt && d->connected == CON_PLAYING )
	{
	  CHAR_DATA *ch;
	  CHAR_DATA *victim;

	  ch = d->character;

	  /* battle prompt */
	  if ((victim = ch->fighting) != NULL && can_see(ch,victim))
	    {
	      int percent;
	      char *pbuff;
	      char buf[MAX_STRING_LENGTH];
	      char buffer[MAX_STRING_LENGTH*2];
 
	      if (victim->max_hit > 0)
                percent = victim->hit * 100 / victim->max_hit;
	      else
                percent = -1;
 
	      if (percent >= 100)
                sprintf(buf,"{g%s is in excellent condition.{x\n\r",
			IS_NPC(victim) ? victim->short_descr : victim->name);
	      else if (percent >= 90)
                sprintf(buf,"{g%s has a few scratches.{x\n\r",
			IS_NPC(victim) ? victim->short_descr : victim->name);
	      else if (percent >= 75)
                sprintf(buf,"{g%s has some small wounds and bruises.{x\n\r",
			IS_NPC(victim) ? victim->short_descr : victim->name);
	      else if (percent >= 50)
                sprintf(buf,"{c%s has quite a few wounds.{x\n\r",
			IS_NPC(victim) ? victim->short_descr : victim->name);
	      else if (percent >= 30)
                sprintf(buf,"{c%s has some big nasty wounds and scratches.{x\n\r",
			IS_NPC(victim) ? victim->short_descr : victim->name);
	      else if (percent >= 15)
                sprintf(buf,"{r%s looks pretty hurt.{x\n\r",
			IS_NPC(victim) ? victim->short_descr : victim->name);
	      else if (percent >= 0)
                sprintf(buf,"{r%s is in awful condition.{x\n\r",
			IS_NPC(victim) ? victim->short_descr : victim->name);
	      else
                sprintf(buf,"{r%s is bleeding to death.{x\n\r",
			IS_NPC(victim) ? victim->short_descr : victim->name);
 
	      buf[2]	= UPPER( buf[2] );
	      pbuff	= buffer;
	      colourconv( pbuff, buf, d->character );
	      write_to_buffer( d, buffer, 0);
	    }


	  ch = d->original ? d->original : d->character;
	  if (!IS_SET(ch->comm, COMM_COMPACT) )
	    write_to_buffer( d, "\n\r", 2 );

	  
	  if ( IS_SET(ch->comm, COMM_PROMPT) )
            bust_a_prompt( d->character );

	  if (IS_SET(ch->comm,COMM_TELNET_GA))
	    write_to_buffer(d,go_ahead_str,0);
	}
    }

  /*
   * Short-circuit if nothing to write.
   */
  if ( d->outtop == 0 )
    return TRUE;
  
  /*
   * Snoop-o-rama.
   */
  if ( d->snoop_by != NULL )
    {
      if (d->character != NULL)
	write_to_buffer( d->snoop_by, d->character->name,0);
     
      write_to_buffer( d->snoop_by, "> ", 2 );
      write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }
  
  /*
   * OS-dependent output.
   */
  if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
    {
      d->outtop = 0;
      return FALSE;
    }
  else
    {
      d->outtop = 0;
      return TRUE;
    }
}

/* source: EOD, by John Booth <???> */
/* stick this in in comm.c somewhere */
/* Remember to include <stdargs.h> */

void printf_to_char (CHAR_DATA *ch, char *fmt, ...)
{
  char buf [MAX_STRING_LENGTH];
  va_list args;
  va_start (args, fmt);
  vsprintf (buf, fmt, args);
  va_end (args);
	
  send_to_char (buf, ch);
}



void bust_a_prompt( CHAR_DATA* ch )
{
  extern void olc_prompt(DESCRIPTOR_DATA *d);
  const char* dir_name[] = { "N", "E", "S", "W", "U", "D" };
  char parsed_prompt[ MAX_STRING_LENGTH ];
  char final_prompt[ 2 * MAX_STRING_LENGTH ];
  char temp_doors[ MAX_INPUT_LENGTH ];
  char* output_point = parsed_prompt;
  char* input_point;
  int door;
  int chars_appended;
  bool door_found;
  EXIT_DATA* pexit;
  
  *parsed_prompt = '\0';
  *final_prompt = '\0';

  if( !ch->prompt || ch->prompt[ 0 ] == '\0' )
    {
      sprintf( final_prompt, "{%c<%dhp {%c%dm {%c%dmv>{x %s",
	       ch->colors[C_HP],ch->hit, 
	       ch->colors[C_MANA],ch->mana, 
	       ch->colors[C_MOVES],ch->move, ch->prefix );
      send_to_char( final_prompt, ch );
      return;
    }

  if( IS_SET( ch->comm, COMM_AFK ) )
    {
      send_to_char( "<AFK> ", ch );
      return;
    }

  input_point = ch->prompt;

  while( *input_point != '\0' )
    {
      chars_appended = 0;

      if( *input_point != '%' )
	{
	  *output_point = *input_point;
	  chars_appended = 1;
	}
      else
	{
	  input_point++;

	  switch( *input_point )
	    {
	    case 'e':
	      {
		door_found = FALSE;
		temp_doors[ 0 ] = '\0';

		for( door = 0; door < 6; door++ )
		  {
		    pexit = ch->in_room->exit[ door ];

		    if( pexit &&
			pexit->u1.to_room &&
			( can_see_room( ch, pexit->u1.to_room ) ||
			  IS_AFFECTED( ch, AFF_INFRARED ) ) &&
			!IS_SET( pexit->exit_info, EX_CLOSED ) )
		      {
			door_found = TRUE;
			strcat( temp_doors, dir_name[ door ] );
		      }
		  }

		if( !door_found )
		  {
		    strcat( temp_doors, "none" );
		  }

		if( IS_AFFECTED( ch, AFF_BLIND ) ||
		    IS_AFFECTED3( ch, AFF_DIRT ) )
		  {
		    temp_doors[ 0 ] = '\0';
		    strcat( temp_doors, "BLIND" );
		  }

		if( IS_AFFECTED( ch, AFF_SLEEP ) )
		  {
		    temp_doors[ 0 ] = '\0';
		    strcat( temp_doors, "SLEEP" );
		  }
		    
		chars_appended = sprintf( output_point, "%s", temp_doors );
		break;
	      }
	    case 'f':
	      {
		if ( !IS_NPC(ch) 
                &&   ch->pcdata->lastfight )
                {
                  chars_appended = sprintf( output_point, "%d", 
                  FIGHT_LAG - (int)(current_time-ch->pcdata->lastfight));
                }
		break;
	      }
	    case 'p':
	      {
		if ( !IS_NPC(ch) 
                &&   ch->pcdata->lastpk )
                {
                  chars_appended = sprintf( output_point, "%d", 
                  FIGHT_LAG - (int)(current_time-ch->pcdata->lastpk));
                }
		break;
	      }

	    case 'l':
	      {
                if ( !IS_NPC( ch ) 
                &&   ch->pcdata->latelog
                &&   ch->logon > 0 )
                {
                  int latelog = (int)(current_time - ch->logon);
                  if ( latelog < 180 )
                      chars_appended = sprintf(output_point, "%d", 180-latelog);
                }
		break;
	      }
            case 'L':
            {
              if ( !IS_NPC(ch)
              &&   ch->pcdata->brawling )
                chars_appended = sprintf( output_point, "%s", 
                    ch->pcdata->brawling->name );
              break;
            }
	    case 'c':
	      {
		chars_appended = sprintf( output_point, "%s", "\n\r" );
		break;
	      }
	    case 'h':
	      {
              /* chars_appended = sprintf( output_point, "{%c%d{x", 
                    ch->colors[C_HP], ch->hit ); */
                chars_appended = sprintf( output_point, "%d", ch->hit );
		break;
	      }
	    case 'H':
	      {
              /* chars_appended = sprintf( output_point, "{%c%d{x", 
                    ch->colors[C_MAXHP],ch->max_hit ); */
                chars_appended = sprintf( output_point, "%d", ch->max_hit );
		break;
	      }
	    case 'm':
	      {
              /* chars_appended = sprintf( output_point, "{%c%d{x", 
                    ch->colors[C_MANA],ch->mana ); */
                chars_appended = sprintf( output_point, "%d", ch->mana );
		break;
	      }
	    case 'M':
	      {
              /* chars_appended = sprintf( output_point, "{%c%d{x", 
                    ch->colors[C_MAXMANA],ch->max_mana ); */
                chars_appended = sprintf( output_point, "%d", ch->max_mana );
		break;
	      }
	    case 'v':
	      {
              /* chars_appended = sprintf( output_point, "{%c%d{x", 
                    ch->colors[C_MOVES],ch->move ); */
                chars_appended = sprintf( output_point, "%d", ch->move );
		break;
	      }
	    case 'V':
	      {
              /* chars_appended = sprintf( output_point, "{%c%d{x", 
                    ch->colors[C_MAXMOVES],ch->max_move ); */
                chars_appended = sprintf( output_point, "%d", ch->max_move );
		break;
	      }
	    case 'x':
	      {
		chars_appended = sprintf( output_point, "%d", ch->exp );
		break;
	      }
	    case 'X':
	      {
		chars_appended = sprintf(output_point, "%d", IS_NPC(ch) ? 0 :
		 (ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);
		break;
	      }
	    case 'g':
	      {
		chars_appended = sprintf( output_point, "%ld", ch->gold);
		break;
	      }
	    case 's':
	      {
		chars_appended = sprintf( output_point, "%ld", ch->silver);
		break;
	      }
	    case 'r':
	      {
		if( ch->in_room != NULL )
		  chars_appended = sprintf( output_point, "%s", 
					    ( ( !IS_NPC( ch ) && IS_SET( ch->act_bits, PLR_HOLYLIGHT ) ) ||
					      ( !IS_AFFECTED( ch, AFF_BLIND ) &&
					        !IS_AFFECTED3( ch, AFF_DIRT ) &&
					        !room_is_dark( ch->in_room ) ) ) ? ch->in_room->name : "DARKNESS" );
		else
		  chars_appended = sprintf( output_point, " " );
	     
		break;
	      }
	    case 'R':
	      {
		if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
		  chars_appended = sprintf( output_point, "%d", ch->in_room->vnum );
		else
		  chars_appended = sprintf( output_point, " " );

		break;
	      }
	    case 'z':
	      {
		if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
		  chars_appended = sprintf( output_point, "%s", ch->in_room->area->name );
		else
		  chars_appended = sprintf( output_point, " " );

		break;
	      }
	    case '%':
	      {
		chars_appended = sprintf( output_point, "%%" );
		break;
	      }
	    case 'o':
	      {
		chars_appended = sprintf( output_point, "%s", olc_ed_name(ch) );
		break;
	      }
	    case 'O':
	      {
		chars_appended = sprintf( output_point, "%s", olc_ed_vnum(ch) );
		break;
	      }
            case 'b':
              {
		if( IS_SET( ch->shiftbits, PERM_VAMP ) || IS_SET( ch->shiftbits, TEMP_VAMP ) )
		    chars_appended = sprintf( output_point, "%d", ch->blood );
		else
		    chars_appended = sprintf( output_point, " " );

                break;
              }
	    case 'B':
	      {
		if( IS_SET( ch->shiftbits, PERM_VAMP ) || IS_SET( ch->shiftbits, TEMP_VAMP ) )
		    chars_appended = sprintf( output_point, "%d", ch->max_blood );
		else
		    chars_appended = sprintf( output_point, " " );
	
		break;
	      }		    
            case 'w': /* Shift flags */
            {
/*      This is done as an if/else because nobody should be shifted as more than
        one thing at a time.  If new shift flags are added, they MUST be added
        specifically here as well as to shift_flag_name in handler.c in order to
        show up in prompts. */
                if ( IS_SET(ch->shiftbits,SHIFT_POTENTIAL) && (ch->level > 29) )
                {
                    if ( IS_SET( ch->shiftbits, SHIFT_HALF) )
                        chars_appended = sprintf( output_point, "H" );
                    else if ( IS_SET( ch->shiftbits, SHIFT_FULL) )
                        chars_appended = sprintf( output_point, "F" );
                }
                else if ( IS_SET( ch->shiftbits, PERM_VAMP )
                     ||   IS_SET( ch->shiftbits, TEMP_VAMP ) )
                {
                    if ( IS_SET( ch->shiftbits, MIST_VAMP) )
                        chars_appended = sprintf( output_point, "M" );
                    else if ( IS_SET( ch->shiftbits, BAT_VAMP) )
                        chars_appended = sprintf( output_point, "B" );
                }

                break;
            }
            case 't': /* IC time */
            {
                chars_appended = sprintf( output_point, "%d%s",
                    (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
                    time_info.hour >= 12 ? "pm" : "am" );
 
                break;
            }
            case 'T': /* Server time (24 hour) */
            {
                chars_appended = sprintf( output_point, global_prompt_t );
                break;
            }
            case 'a': /* Server time (in am/pm) */
            {
                chars_appended = sprintf( output_point, global_prompt_a );
                break;
            }
			case 'u': /* current life energy*/
            {
				if (ch->race == race_lookup("seraph"))
                chars_appended = sprintf( output_point, "%d", ch->life_energy );
				else
				chars_appended = sprintf( output_point, " " );
                break;
            }
			case 'U': /* max life energy*/
            {
				if (ch->race == race_lookup("seraph"))
                chars_appended = sprintf( output_point, "%d", ch->max_life_energy );
				else
				chars_appended = sprintf( output_point, " " );
                break;
            }
	    default:
	      {
		*output_point = ' ';
		chars_appended = 1;
		break;
	      }
	    }
	}

      input_point++;
      output_point += chars_appended;
      *output_point = '\0';
    }

  colourconv( final_prompt, parsed_prompt, ch );
  write_to_buffer( ch->desc, final_prompt, 0 );

/* The write_to_buffer above can possibly disconnect the
 * player due to buffer overflow--in this case, the below
 * line will cause a segfault if ch->desc's validity is
 * not first re-checked.  It's happened.  7/12/08 gkl
 */
/* Let's just get rid of it altogether.  Builders hate it,
   and %o in the prompt does this anyway.  2/8/10 gkl
  if( ch->desc && ch->desc->editor )
    olc_prompt( ch->desc );
 */

  if( ch->prefix[ 0 ] != '\0' )
    write_to_buffer( ch->desc, ch->prefix, 0 );
 

}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
  /*
   * Find length in case caller didn't.
   */
  if ( length <= 0 )
    length = strlen(txt);

  /* kick bad descriptors */
  if( d == NULL )
    return;

  /*
   * Initial \n\r if needed.
   */
  if ( d->outtop == 0 && !d->fcommand )
    {
      d->outbuf[0]	= '\n';
      d->outbuf[1]	= '\r';
      d->outtop	= 2;
    }

  /*
   * Expand the buffer as needed.
   */
  while ( d->outtop + length >= d->outsize )
    {
      char *outbuf;

      if (d->outsize >= 32000)
	{
	  close_socket(d);
	  bug("Buffer overflow. Closing.\n\r",0);
	  return;
 	}
      outbuf      = alloc_mem( 2 * d->outsize );
      strncpy( outbuf, d->outbuf, d->outtop );
      free_mem( d->outbuf, d->outsize );
      d->outbuf   = outbuf;
      d->outsize *= 2;
    }

  /*
   * Copy.
   */
  /*  strcpy( d->outbuf + d->outtop, txt ); */
  strncpy( d->outbuf + d->outtop, txt, length );
  d->outtop += length;
  return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
  int iStart;
  int nWrite;
  int nBlock;
  bool success = TRUE;

  if ( length <= 0 )
    length = strlen(txt);

  for ( iStart = 0; iStart < length; iStart += nWrite )
    {
      nBlock = UMIN( length - iStart, 2048 );

      nWrite = write( desc, txt + iStart, nBlock );

      if( nWrite < 0 )
	{
	  perror( "write_to_descriptor()" );
	  success = FALSE;
	  break;
	}
    } 

  return success;
}

bool check_parse_name( char *name )
{
    /*
     * Reserved words.  --== ILLEGAL NAMES LIST ==--
     */
    if ( is_name( name, 
	"all auto imm immortal self someone something the you demise \
         balance circle loner honor none ass asshole fuck fucker fuckface\
         retard pussy dick bavor marsarken hitler nazi frodo hacker \
	 bitch crimson rain inferno everyone and guild romany family\
         vermillion dawn dawning or they those the love paradox mike\
         krystaline linda tolkien kerri admin imms immortals guilds\
         tyrin syrin lasombra ravnos ventrue toreador gangrel brujah\
	noferatu assamite tremere malkavian giovanni salubri tzimisce\
	setite brother bretheren realms realm lord lords ladies \
	noble house clan brethren mother sister scholars assassins\
	circles") )
	return FALSE;
	
    if (str_cmp(capitalize(name),"Alander") && (!str_prefix("Alander",name)
    || !str_suffix("Alander",name)))
	return FALSE;

    if (guild_lookup(name) != 0)
        return FALSE;

    /*
     * Length restrictions.
     */
     
    if ( strlen(name) <  2 )
	return FALSE;

    if ( strlen(name) > 12 )
	return FALSE;


    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll,adjcaps = FALSE,cleancaps = FALSE;
 	int total_caps = 0;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;

	    if ( isupper(*pc)) /* ugly anti-caps hack */
	    {
		if (adjcaps)
		    cleancaps = TRUE;
		total_caps++;
		adjcaps = TRUE;
	    }
	    else
		adjcaps = FALSE;

	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;

	if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

    return TRUE;
}


/*
 * Deal with sockets that haven't logged in yet.
 */

void nanny( DESCRIPTOR_DATA *d, char *argument )
{
  DESCRIPTOR_DATA *d_old, *d_next;
  CHAR_DATA *ch;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char pueblo_buf[MAX_STRING_LENGTH];
  char log_buf[ MAX_STRING_LENGTH ];
  char *pwdnew;
  char *p;
  int iClass,race,i,weapon, god,isn;
  bool fOld;

  while ( isspace(*argument) )
    argument++;

  ch = d->character;

  switch ( d->connected )
    {

    default:
      bug( "Nanny: bad d->connected %d.", d->connected );
      close_socket( d );
      return;

    case CON_GET_NAME:
      if ( argument[0] == '\0' )
	{
	  close_socket( d );
	  return;
	}

      argument[0] = UPPER(argument[0]);
      if ( !check_parse_name( argument ) )
	{
	  write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
	  return;
	}

      fOld = load_char_obj( d, argument );
      ch   = d->character;
	
      if( ch->position == POS_UNCONSCIOUS )
	{
	  ch->daze = PULSE_TICK;
	}
	    
      /*
        if (d->ansi == TRUE)
        {
	SET_BIT( ch->act_bits, PLR_COLOUR );
	REMOVE_BIT( ch->act_bits, PLR_PUEBLO );
        }
        else if (d->ansi == FALSE)
        {
	REMOVE_BIT( ch->act_bits, PLR_COLOUR);
	REMOVE_BIT( ch->act_bits, PLR_PUEBLO);
        }
        if (d->pueblo == TRUE)
        {
	SET_BIT( ch->act_bits, PLR_COLOUR );
	SET_BIT( ch->act_bits, PLR_PUEBLO );
        }
      */

      if (check_ban(d->host,BAN_PERMIT) && !IS_SET(ch->act_bits,PLR_PERMIT))
	{
	  sprintf( log_buf, "%s attempted to login from permitted IP %s.",
		   ch->name,
		   d->host );
	  wiznet( log_buf, NULL, NULL, WIZ_BANS, 0, DEITY );
          log_string(log_buf);

	  write_to_buffer(d,"Your site has been banned from this mud.\n\r",0);
	  close_socket(d);
	  return;
	}

      if ( check_reconnect( d, argument, FALSE ) )
	{
	  fOld = TRUE;
	}
      else
	{
	  if ( wizlock && !IS_IMMORTAL(ch)) 
	    {
	      write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
	      close_socket( d );
	      return;
	    }
	}

      if ( fOld  ==  TRUE)
	{
          if ( !global_config_aprilfools ) {
	  /* Old player */
	  write_to_buffer( d, "    |                                                              |\n\r", 0);
	  write_to_buffer( d, "    | This name is already in my record books, the dark figure     |\n\r", 0);
	  write_to_buffer( d, "    | says, to access thy mortal body thou must provide me with a  |\n\r", 0);
	  write_to_buffer( d, "    | password.                                                    |\n\r", 0);
	  write_to_buffer( d, "    |                                                              |\n\r", 0);
	  write_to_buffer( d, "    | What is thy password soul? ", 0 );
	  write_to_buffer( d, echo_off_str, 0 );
          }
          else
          {
	  write_to_buffer(d,"    |                                                              |\n\r", 0);
	  write_to_buffer(d,"    | The name thee wishes to use is not in my records. Thou must  |\n\r", 0);
	  write_to_buffer(d,"    | create thyself a mortal body. Before thou dost continues,    |\n\r", 0);
	  write_to_buffer(d,"    | thou must consider thy name. Thy name must be one that is    |\n\r", 0);
	  write_to_buffer(d,"    | relevant to the time in which the world lives within. Thou   |\n\r", 0);
	  write_to_buffer(d,"    | must not have a profain, nor a name that refers to an action.|\n\r", 0);
	  write_to_buffer(d,"    | Thou must consider this wisely, for the immortals of the     |\n\r", 0);
	  write_to_buffer(d,"    | land must approve thee for continued usage of thy name.      |\n\r", 0);
	  write_to_buffer(d,"    |                                                              |\n\r", 0);
	  write_to_buffer(d,"    |                                                              |\n\r", 0);
	  sprintf (buf,"    | Is this the name thou wishes to use?, %s (Y/N)? ", argument );
	  write_to_buffer( d, buf, 0 );
	  write_to_buffer( d, echo_off_str, 0 );
          }
	  d->connected = CON_GET_OLD_PASSWORD;
	  return;
	}
      else if  (fOld == 2){
	/* BAD PLAYER FILE */
	write_to_buffer(d, "{GThere seems to be a problem with thy body, please create anew and notify thine IMP.{x\n\r",0);
	close_socket(d);
	return;
      }	
      else
	{
	  /* CLOSE THE LOOP */
	  if (check_playing2(d,argument)){
	    write_to_buffer( d, 
"You are already in the creation process for this character in another\n\r"
"session.  If you think this is a mistake, try again in a few minutes\n\r"
"or log in with another character and notify an immortal to have your\n\r"
"other creation attempt cancelled.\n\r", 0 );
	    close_socket( d );
	    return;
	  }

	  /* New player */
	  if (newlock)
	    {
	      write_to_buffer( d, "The game is newlocked.\n\r", 0 );
	      close_socket( d );
	      return;
            }

	  if (check_ban(d->host,BAN_NEWBIES))
	    {
	      write_to_buffer(d,
			      "New players are not allowed from your site.\n\r",0);

	      sprintf( buf, "New character creation attempt from banned IP %s.",
		       d->host );
	      wiznet( buf, NULL, NULL, WIZ_BANS, 0, DEITY );
              log_string(buf);

	      close_socket(d);
	      return;
	    }
	
	  write_to_buffer(d,"    |                                                              |\n\r", 0);
	  write_to_buffer(d,"    | The name thee wishes to use is not in my records. Thou must  |\n\r", 0);
	  write_to_buffer(d,"    | create thyself a mortal body. Before thou dost continues,    |\n\r", 0);
	  write_to_buffer(d,"    | thou must consider thy name. Thy name must be one that is    |\n\r", 0);
	  write_to_buffer(d,"    | relevant to the time in which the world lives within. Thou   |\n\r", 0);
	  write_to_buffer(d,"    | must not have a profain, nor a name that refers to an action.|\n\r", 0);
	  write_to_buffer(d,"    | Thou must consider this wisely, for the immortals of the     |\n\r", 0);
	  write_to_buffer(d,"    | land must approve thee for continued usage of thy name.      |\n\r", 0);
	  write_to_buffer(d,"    |                                                              |\n\r", 0);
	  write_to_buffer(d,"    |                                                              |\n\r", 0);
	  sprintf (buf,"    | Is this the name thou wishes to use?, %s (Y/N)? ", argument );
	  write_to_buffer( d, buf, 0 );
	  d->connected = CON_CONFIRM_NEW_NAME;
	  return;
	}
      break;

    case CON_GET_OLD_PASSWORD:

      write_to_buffer( d, "\n\r", 2 );


      if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ))
	{
	  if( IS_SET( ch->act_bits, PLR_DENY ) )
	    {
	      sprintf( log_buf, "Bad password attempt on denied character %s@%s", ch->name, d->host );
	      wiznet( log_buf, NULL, NULL, WIZ_BANS, 0, DEITY );
	    }
	  else
	    {	  
              if ( !global_config_aprilfools || strcmp( argument, "y" ) )
              {
	      sprintf( log_buf, "Bad password attempt on %s@%s", ch->name, d->host );
	      wiznet( log_buf, NULL, NULL, WIZ_BANS, 0, DEITY );
              }
	    }

	  log_string( log_buf );
          if ( !global_config_aprilfools ) {
	  write_to_buffer( d, "Thy password is wrong.\n\r", 0 ); }
          else {
/*                                 1234|6789x123456789x123456789x123456789x123456789x123456789x12345  | */
              write_to_buffer( d, "\n\r", 0);
              write_to_buffer( d, "    |==============================================================|\n\r", 0);
              write_to_buffer( d, "    |                          APRIL FOOL'S!!                      |\n\r", 0);
              write_to_buffer( d, "    |==============================================================|\n\r", 0);
              write_to_buffer( d, "   \n\r", 0);
              write_to_buffer( d, "     Ha ha, thought your character was deleted, huh?  April fool's!\n\r", 0);
              write_to_buffer( d, "     To log your character in, just reconnect and enter your normal\n\r", 0);
              write_to_buffer( d, "     password when asked if you are sure you've entered the name\n\r", 0 );
              write_to_buffer( d, "     you want to use instead of Y or N.\n\r", 0 );
              write_to_buffer( d, "   \n\r", 0);
              write_to_buffer( d, "    |==============================================================|\n\r", 0);


          }
	  close_socket( d );
	  return;
	}

      if (IS_SET(ch->act_bits, PLR_DENY))
	{
	  sprintf( log_buf, "Denying access to %s@%s.", ch->name, d->host );
	  wiznet( log_buf, NULL, NULL, WIZ_BANS, 0, DEITY );
	  log_string( log_buf );
	  write_to_buffer( d, "You are denied access.  Have a nice day.\n\r", 0 );
	  close_socket( d );
	  return;
	}
 
      write_to_buffer( d, echo_on_str, 0 );

      if (check_playing(d,ch->name))
	return;

      if ( check_reconnect( d, ch->name, TRUE ) )
	return;

      if( ch->pcdata->fake_ip[ 0 ] == '\0' )
	{
	  sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
	  log_string( log_buf );
	  wiznet(log_buf,NULL,NULL,WIZ_SITES,0,ch->level);

	}
      else
	{
	  sprintf( log_buf, "%s@%s (REAL IP) has connected.", ch->name, d->host );
	  log_string( log_buf );
	  wiznet(log_buf,NULL,NULL,WIZ_SITES,0,MAX_LEVEL);

	  sprintf( log_buf, "%s@%s has connected.", ch->name, ch->pcdata->fake_ip );
	  log_string( log_buf );
	  wiznet(log_buf,NULL,NULL,WIZ_SITES,0,ch->level);

	}

      if( ch->pcdata->last_ip[ 0 ] != '\0' )
	{
	  sprintf( log_buf, "%s last logged from %s.", ch->name,
		   ch->pcdata->fake_ip[ 0 ] != '\0' ?
		   ch->pcdata->fake_ip :
		   ch->pcdata->last_ip );
	  wiznet( log_buf, NULL, NULL, WIZ_SITES, 0, ch->level );
	}

      free_string( ch->pcdata->last_ip );
      ch->pcdata->last_ip = str_dup( ch->pcdata->fake_ip[ 0 ] != '\0' ?
				     ch->pcdata->fake_ip :
				     d->host );
		  
        
      /*
        if (ch->desc->ansi)
	SET_BIT(ch->act_bits, PLR_COLOUR);
        else REMOVE_BIT(ch->act_bits, PLR_COLOUR);
      */

      if (IS_SET(ch->act_bits, PLR_PUEBLO)) {
        sprintf(pueblo_buf,"<img xch_sound=play xch_volume=%d src=\"%s%s\">"
                "<img src=\"%s%s\">",
		ch->pcdata->volume,PUEBLO_DIR,PUEBLO_START, PUEBLO_DIR, PUEBLO_S_IMG);
        send_to_char("</xch_mudtext><img xch_mode=html>",ch);
        send_to_char(pueblo_buf,ch);
        send_to_char("<br><img xch_mode=text>",ch);
      } else send_to_char("    |                                                              |\n\r", ch);

      if ( IS_IMMORTAL(ch) )
	{
	  send_to_char("    | The dark figure bows before you and says, '{gWelcome           {x|\n\r", ch);
	  sprintf(buf,"    | {g%14s, thine immortal station awaits thee.{x'         |\n\r",ch->name);
	  send_to_char(buf,ch);
	  send_to_char("    |                                                              |\n\r", ch);
	  send_to_char("    | You feel the universe begin to swirl as your senses are      |\n\r", ch);
	  send_to_char("    | bombarded by the swirling vortex surrounding you. As your    |\n\r", ch);
	  send_to_char("    | soul falls down to the realm of mortal men you think you can |\n\r", ch);
	  send_to_char("    | make out what appears to be a sign.............              |\n\r",ch);
	  send_to_char("\n\r", ch);
	  send_to_char("\n\r", ch);
	  do_help(ch,"greeting");
	  d->connected = CON_READ_GREETING;
 	}
      else
        {
          send_to_char("    | The dark figure fades from view and a voice booms '{gWelcome {x  |\n\r", ch); 
          sprintf(buf,"    | {g%14s, thine adventure awaits for thee.{x'             |\n\r",ch->name);
          send_to_char(buf,ch);
          send_to_char("    |                                                               |\n\r", ch);
          send_to_char("    | You feel the universe begin to swirl as your senses are       |\n\r", ch);
          send_to_char("    | bombarded by the swirling vortex surrounding you. As your     |\n\r", ch);
          send_to_char("    | soul falls down to the realm of mortal men you think you can  |\n\r", ch);
          send_to_char("    | make out what appears to be a sign.............               |\n\r", ch);
          send_to_char("\n\r", ch);
          send_to_char("\n\r", ch);
	  do_help(ch,"greeting");
	  d->connected = CON_READ_GREETING;
        }

      break;

      /* RT code for breaking link */
 
    case CON_BREAK_CONNECT:
      switch( *argument )
	{
	case 'y' : case 'Y':
	  for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
	    {
	      d_next = d_old->next;
	      if (d_old == d || d_old->character == NULL)
		continue;

	      if (str_cmp(ch->name,d_old->original ?
			  d_old->original->name : d_old->character->name))
		continue;

	      close_socket(d_old);
	    }
	  if (check_reconnect(d,ch->name,TRUE))
	    return;
	  write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);
	  if ( d->character != NULL )
            {
              player_quitting = TRUE;
	      free_char( d->character );
              player_quitting = FALSE;
	      d->character = NULL;
            }
	  d->connected = CON_GET_NAME;
	  break;

	case 'n' : case 'N':
	  write_to_buffer(d,"Name: ",0);
	  if ( d->character != NULL )
            {
              player_quitting = TRUE;
	      free_char( d->character );
              player_quitting = FALSE;
	      d->character = NULL;
            }
	  d->connected = CON_GET_NAME;
	  break;

	default:
	  write_to_buffer(d,"Please type Y or N? ",0);
	  break;
	}
      break;

    case CON_CONFIRM_NEW_NAME:
      switch ( *argument )
	{
	case 'y': case 'Y':
	  write_to_buffer( d, "    |                                                              |\n\r", 0);
	  write_to_buffer( d, "    | Thou must have a password to access thy mortal body.         |\n\r", 0);
	  write_to_buffer( d, "    |                                                              |\n\r", 0);
	  sprintf( buf, "    | What is the password thou wishes to use for %s? %s",
		   ch->name, echo_off_str );
	  SET_BIT(ch->act_bits,PLR_NEWBIE);
	  write_to_buffer( d, buf, 0 );
	  d->connected = CON_GET_NEW_PASSWORD;
	  if (ch->desc->ansi)
	    SET_BIT(ch->act_bits, PLR_COLOUR);
	  break;

	case 'n': case 'N':
	  write_to_buffer( d, "Ok, what IS thy name then? ", 0 );
	  free_char( d->character );
	  d->character = NULL;
	  d->connected = CON_GET_NAME;
	  break;

	default:
	  write_to_buffer( d, "Please type Yes or No? ", 0 );
	  break;
	}
      break;

    case CON_GET_NEW_PASSWORD:

      write_to_buffer( d, "\n\r", 2 );


      if ( strlen(argument) < 5 )
	{
	  write_to_buffer( d,"    | Thy password must be at least 5 characters long.             |\n\r", 0);
	  write_to_buffer( d,"    | What is thy Password? ", 0 );
	  return;
	}

      pwdnew = crypt( argument, ch->name );
      for ( p = pwdnew; *p != '\0'; p++ )
	{
	  if ( *p == '~' )
	    {
	      write_to_buffer( d,"    | Thy password is not acceptable choose again.                  |\n\r", 0);
	      write_to_buffer( d,"    | What is thy Password: ", 0 );
	      return;
	    }
	}

      free_string( ch->pcdata->pwd );
      ch->pcdata->pwd	= str_dup( pwdnew );
      write_to_buffer( d, "    |                                                              |\n\r", 0);
      write_to_buffer( d, "    | Please retype thy password: ", 0 );
      d->connected = CON_CONFIRM_NEW_PASSWORD;
      break;

    case CON_CONFIRM_NEW_PASSWORD:

      write_to_buffer( d, "\n\r", 2 );


      if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	  write_to_buffer( d, "    |                                                              |\n\r", 0);
	  write_to_buffer( d, "    | Thou's passwords do not match, please try again.             |\n\r", 0);
	  write_to_buffer( d, "    | Retype thy password:",0 );
	  d->connected = CON_GET_NEW_PASSWORD;
	  return;
	}

      write_to_buffer( d, echo_on_str, 0 );
      write_to_buffer( d, "    |                                                              |\n\r", 0);
      write_to_buffer( d, "    | Dost thou wish to end thy colorblindness soul? (Y/N)  ",0);
      d->connected = CON_ANSI;
      break;

    case CON_ANSI:
      switch ( *argument )
        {
        case 'y': case 'Y':
	  SET_BIT(ch->act_bits, PLR_COLOUR);
	  d->connected = CON_GET_NEW_RACE;
	  break;
 
        case 'n': case 'N':
	  break;
 
        default:
	  write_to_buffer( d, "Please type Yes or No? ", 0 );
	  break;
        }
 
      write_to_buffer( d, echo_on_str, 0 );
      write_to_buffer( d, "    |                                                              |\n\r", 0);
      write_to_buffer( d, "    | The dark figure leads you down a path towards what appears   |\n\r",0);
      write_to_buffer( d, "    | to be a small building floating in the void. The Dark Figure |\n\r",0);
      write_to_buffer( d, "    | opens the door to the building and leads you inside. As you  |\n\r",0);
      write_to_buffer( d, "    | look around you see hanging on the walls what appears to be  |\n\r",0);
      write_to_buffer( d, "    | lifeless bodies of different kinds. The Dark Figure turns to |\n\r",0);
      write_to_buffer( d, "    | you and begins to speak to you again.                        |\n\r",0);
      write_to_buffer( d, "    |                                                              |\n\r", 0);
      write_to_buffer( d, "    |                                                              |\n\r", 0);
      sprintf(buf,"    | {g'There are numerous races available to thee, %-14s  {x|\n\r",ch->name);
      send_to_char(buf,ch);
      sprintf(buf,"    | {gTo aid thee in thy selection thou can type help {Crace{g and     {x|\n\r");
      send_to_char(buf,ch);
      sprintf(buf,"    | {ghelp {Cracestat{g for more information on thy selected race.'    {x|\n\r");
      send_to_char(buf,ch);
      write_to_buffer( d, "\n\r", 0);
      write_to_buffer( d, "\n\r", 0);
      write_to_buffer( d, "\n\r", 0);
      sprintf(buf,"   {GPlease select a race suitable to thy name from the following list:{c\n\r");
      send_to_char(buf,ch);
      write_to_buffer(d,"\n\r", 0);
      write_to_buffer( d, " [",0 );
      for ( race = 1; race_table[race].name != NULL;  race++ )
        {
	  if( race_table[ race ].pc_race &&
	      race_table[ race ].visible &&
	      !race_table[ race ].remort_race )
	    {
	      write_to_buffer(d,race_table[race].name,0);
	      write_to_buffer(d," ",1);
	    }
        }
      write_to_buffer( d, "]\n\r",0 );
      write_to_buffer( d, "\n\r", 0);
      write_to_buffer( d, "\n\r", 0);
      sprintf(buf,"   {GWhat race dost thou wish to use?{x ");
      send_to_char(buf,ch);
      d->connected = CON_GET_NEW_RACE;
      break;
 
      break;
 
    case CON_GET_NEW_RACE:
      one_argument(argument,arg);

      if (!strcmp(arg,"help"))
	{
	  argument = one_argument(argument,arg);
	  if (argument[0] == '\0')
	    do_help(ch,"race help");
	  else
	    do_help(ch,argument);
	  write_to_buffer(d,"\n\r", 0);
	  sprintf(buf,"   {GWhat race dost thou wish to use?{x ");
	  send_to_char(buf,ch);
	  break;
  	}

      race = race_lookup(argument);

      if (race == 0 || !race_table[race].pc_race || race_table[race].remort_race)
	{
	  write_to_buffer(d,"\n\r", 0);
	  sprintf(buf,"   {GThat is not race that lives amongst this world.{x\n\r");
	  send_to_char(buf,ch);
	  sprintf(buf,"   {GThese are the races available to thee:{C\n\r");
	  send_to_char(buf,ch);
	  write_to_buffer(d,"\n\r", 0);
	  for ( race = 1; race_table[race].name != NULL; race++ )
            {
	      if( race_table[ race ].pc_race &&
		  race_table[ race ].visible &&
		  !race_table[ race ].remort_race )
		{
		  write_to_buffer(d,race_table[race].name,0);
		  write_to_buffer(d," ",1);
		}
            }
	  write_to_buffer(d,"\n\r", 0);
	  write_to_buffer(d,"\n\r", 0);

	  sprintf(buf,"   {GWhat race dost thou wish to use? {x");
	  send_to_char(buf,ch);
	  break;
	}

      ch->race = race;
      /* initialize stats */
      for (i = 0; i < MAX_STATS; i++)
	ch->perm_stat[i] = pc_race_table[race].stats[i];
      ch->affected_by = ch->affected_by|race_table[race].aff;
      ch->affected_by2 = ch->affected_by2|race_table[race].aff2;
      ch->affected_by3 = ch->affected_by3|race_table[race].aff3;
      ch->imm_flags	= ch->imm_flags|race_table[race].imm;
      ch->res_flags	= ch->res_flags|race_table[race].res;
      ch->vuln_flags	= ch->vuln_flags|race_table[race].vuln;
      ch->form	= race_table[race].form;
      ch->parts	= race_table[race].parts;

      /* suspended levelling defaults to off */
      if( IS_SET( ch->act_bits, PLR_SUSPEND_LEVEL ) )
	REMOVE_BIT( ch->act_bits, PLR_SUSPEND_LEVEL );


      /* add skills */
      for (i = 0; i < 5; i++)
	{
	  if (pc_race_table[race].skills[i] == NULL)
	    break;
	  group_add(ch,pc_race_table[race].skills[i],FALSE);
	}
      /* add cost */
      ch->pcdata->points = pc_race_table[race].points;
      ch->size = pc_race_table[race].size;

      write_to_buffer(d,"\n\r", 0);
      write_to_buffer(d,"\n\r", 0);
      sprintf( buf, "   {GThou must choose %s, whether thou art male or if thou art female.{x\n\r",ch->name);
      send_to_char(buf,ch);
      sprintf( buf, "   {GIf thou art one of the lichnee, thou mayest choose to be neither.{x" );
      send_to_char(buf,ch);
      write_to_buffer(d,"\n\r", 0);
      write_to_buffer(d,"\n\r", 0);

      if( ch->race == race_lookup( "lich" ) )
	sprintf(buf,"   {GWhat is thy sex, %s (M/F/N)?{x ", ch->name );
      else
	sprintf(buf,"   {GWhat is thy sex, %s (M/F)?{x ", ch->name );

      send_to_char(buf,ch );
      d->connected = CON_GET_NEW_SEX;
      break;
        

    case CON_GET_NEW_SEX:
      switch ( argument[0] )
	{
	case 'n':
	case 'N':
	  {
	    if( ch->race == race_lookup( "lich" ) )
	      {
		ch->sex = SEX_NEUTRAL;
		ch->pcdata->true_sex = SEX_NEUTRAL;
	      }
	    else
	      {
		sprintf(buf, "   {GThat's not a sex, %s\n\r{x", ch->name);
		send_to_char( buf, ch );
		write_to_buffer( d, "\n\r", 0 );
		sprintf(buf, "   {GWhat IS your sex?{x" );
		send_to_char( buf, ch );
		return;
	      }

	    break;
	  }
	case 'm': case 'M': ch->sex = SEX_MALE;    
	  ch->pcdata->true_sex = SEX_MALE;
	  break;
	case 'f': case 'F': ch->sex = SEX_FEMALE; 
	  ch->pcdata->true_sex = SEX_FEMALE;
	  break;
	default:
	  sprintf(buf, "   {GThat's not a sex, %s\n\r{x", ch->name);	
	  send_to_char(buf,ch);
	  write_to_buffer(d,"\n\r", 0);
	  sprintf(buf, "   {GWhat IS your sex %s? {x", ch->name );
	  send_to_char(buf,ch);
	  return;
	}

      write_to_buffer(d,"\n\r", 0);
      write_to_buffer(d,"\n\r", 0);
      write_to_buffer(d,"    | The Dark Figure leads you away from the house floating       |\n\r", 0);
      write_to_buffer(d,"    | in the void down another path leading to a small arena       |\n\r", 0);
      write_to_buffer(d,"    | floating freely in space. As you look around you notice      |\n\r", 0);
      write_to_buffer(d,"    | other ethereal beings practicing weaponry or reciting        |\n\r", 0);
      write_to_buffer(d,"    | arcane words from books spread about them. He turns to you   |\n\r", 0);
      write_to_buffer(d,"    | and speaks to you again.                                     |\n\r", 0);
      write_to_buffer(d,"    |                                                              |\n\r", 0);
      sprintf( buf, "    | {g'Thou must choose now %13s, what trade Thou wishes  {x|\n\r",ch->name);
      send_to_char(buf,ch);
      sprintf( buf, "    | {gto use upon the mortal realm. There are several options      {x|\n\r");
      send_to_char(buf,ch);
      sprintf( buf, "    | {gavailable to thee, %13s. To help thee choose, thou  {x|\n\r",ch->name);
      send_to_char(buf,ch);
      sprintf( buf, "    | {gcan type help {Cclass {gor help {Cclass2{g for more information to   {x|\n\r");
      send_to_char(buf,ch);
      sprintf( buf, "    | {gaid thee.'                                                   {x|\n\r");
      send_to_char(buf,ch);
      write_to_buffer(d,"\n\r", 0);
      write_to_buffer(d,"\n\r", 0);
      sprintf(buf,"   {gPlease choose thy trade, %s, from the following list:{c\n\r",ch->name);
      send_to_char(buf,ch);
      write_to_buffer(d,"\n\r", 0);
      strcpy( buf, " [" );
      for ( iClass = 0; iClass < MAX_CLASS ; iClass++ )
	{
	  if ( iClass > 0 )
	    strcat( buf, " " );
/*        if (iClass != class_lookup("avatar"))  */
	    strcat( buf, class_table[iClass].name );
	}
      strcat( buf, "] \n\r" );
      write_to_buffer( d, buf, 0 );
      write_to_buffer(d,"\n\r", 0);
      sprintf(buf,"   {gWhat is thy trade %s? {x", ch->name);
      send_to_char(buf,ch);
      d->connected = CON_GET_NEW_CLASS;
      break;

    case CON_GET_NEW_CLASS:

      one_argument( argument, arg );

      if (!strcmp(arg,"help"))
	{
	  argument = one_argument(argument,arg);

	  if (argument[0] == '\0')
	    do_help(ch,"class");
	  else
	    do_help(ch,argument);

	  write_to_buffer(d,"\n\r", 0);
	  sprintf(buf,"   {GWhat is thy trade %s? {x", ch->name);
	  send_to_char(buf,ch);
	  break;
  	}

      iClass = class_lookup(argument);

      if ( iClass == -1 /* || (iClass == class_lookup("avatar")) */ )
	{
	  sprintf( buf, "   {gThat's not a trade %s.\n\r   What IS thy trade, %s? {x",ch->name,ch->name);
	  send_to_char(buf,ch);
	  return;
	}

      ch->class = iClass;

      sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
      log_string( log_buf );
      wiznet("Newbie alert!  $N sighted.",ch,NULL,WIZ_NEWBIE,0,0);
      wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

      write_to_buffer( d, "\n\r", 2 );
      group_add(ch,"rom basics",TRUE);
      group_add(ch,class_table[ch->class].base_group,TRUE);
      ch->pcdata->learned[gsn_recall] = 100;
      ch->pcdata->learned[gsn_wands] = MIN_SKILL_PCT;
      ch->pcdata->learned[gsn_scrolls] = MIN_SKILL_PCT;
      ch->pcdata->learned[gsn_staves] = MIN_SKILL_PCT;
      write_to_buffer(d,"    |                                                              |\n\r", 0);
      write_to_buffer(d,"    |                                                              |\n\r", 0);
      write_to_buffer(d,"    | The Dark Figure leads you down a small path towards what     |\n\r", 0);
      write_to_buffer(d,"    | appears to be a small temple. He opens the door and leads    |\n\r", 0);
      write_to_buffer(d,"    | you inside. As you enter the small temple you see three      |\n\r", 0);
      write_to_buffer(d,"    | large tapestries, each depicting a grand scene involving     |\n\r", 0);
      write_to_buffer(d,"    | the different Gods of the realm.                             |\n\r", 0);
      write_to_buffer(d,"    |                                                              |\n\r", 0);
      sprintf(buf, "    | {g'Thou must now decide which of the three gods of the         {x|\n\r");
      send_to_char(buf,ch);
      sprintf(buf, "    | {grealm thou wishes to worship. If thou doest not know         {x|\n\r");
      send_to_char(buf,ch);
      sprintf(buf, "    | {gwhich god to choose thou can type help of each of the        {x|\n\r");
      send_to_char(buf,ch);
      sprintf(buf, "    | {ggods names.                                                  {x|\n\r");
      send_to_char(buf,ch);
      write_to_buffer(d,"    |                                                              |\n\r", 0);
      sprintf(buf, "     {g'Thy choices are as follows:                                 {c\n\r");
      send_to_char(buf,ch);
      send_to_char("\n\r",ch);        
      send_to_char("\n\r",ch);        

      if (ch->race == race_lookup("lich"))
      {
	  send_to_char("     {c[Syrin]",ch);
      }
      else if (ch->class == class_lookup("templar"))
      {
          send_to_char("     {c[Rhian Tyrin Syrin]", ch );
      }
      else
	{
	  send_to_char("     {c[Rhian Tyrin Syrin None]",ch);
	}

      write_to_buffer(d,"\n\r", 0);
      write_to_buffer(d,"\n\r", 0);
      sprintf(buf, "   {gWho do you choose to worship, %s?{x ",ch->name);
      send_to_char(buf,ch);
      d->connected = CON_GET_GOD;
      break;


    case CON_GET_GOD:
      one_argument(argument,arg);

      if (!strcmp(arg,"help"))
        {
	  argument = one_argument(argument,arg);
	  if (argument[0] == '\0')
	    do_help(ch,"gods");
	  else
	    do_help(ch,argument);
	  sprintf(buf, "   {gWho do you choose to worship, %s?{x ",ch->name);
	  send_to_char(buf,ch);
	  break;
        }

      god = god_lookup(argument);
      
      if ( god == -1 
      ||  ( ch->race == race_lookup( "lich" ) && god != 3 )
      ||  ( god == 0 && ch->class == class_lookup( "templar" ) ))
      {
          sprintf(buf, "   {gYou cannot worship that deity, %s{x\n\r",ch->name);
          send_to_char(buf,ch);
	  
	  if (ch->race == race_lookup("lich"))
	      send_to_char("     {c[Syrin]\n\r",ch);
          else if (ch->class == class_lookup("templar"))
              send_to_char("     {c[Rhian Tyrin Syrin]", ch );
	  else
	      send_to_char("     {c[Rhian Tyrin Syrin None]\n\r",ch);
        
	  write_to_buffer(d,"\n\r",0);
	  sprintf(buf, "   {gWho do you choose to worship, %s?{x ",ch->name);
	  send_to_char(buf,ch);
	  break;
      } 
        
      ch->pcdata->god = god;
 
      write_to_buffer(d,"    |                                                              |\n\r", 0);
      write_to_buffer(d,"    |                                                              |\n\r", 0);
      write_to_buffer(d,"    | The Dark Figure leads you to the center of the arena and     |\n\r",0);
      write_to_buffer(d,"    | waves his hands in the air. Around you books and weaponry    |\n\r",0);
      write_to_buffer(d,"    | appear. The Dark Figure turns and begins to speak to you     |\n\r",0);
      write_to_buffer(d,"    | again.                                                       |\n\r",0);
      write_to_buffer(d,"    |                                                              |\n\r", 0);
      sprintf(buf, "    | {g'Thou must decide now if thou wishes to gain specific        {x|\n\r");
      send_to_char(buf,ch);
      sprintf(buf, "    | {gknowledge in thy trade, or if thy wants to only gain the     {x|\n\r");
      send_to_char(buf,ch);
      sprintf(buf, "    | {gbasic knowledge for thy trade. Those who know not the realm  {x|\n\r");
      send_to_char(buf,ch);
      sprintf(buf, "    | {gof Rhia should seriously consider learning only the basics   {x|\n\r");
      send_to_char(buf,ch);
      sprintf(buf, "    | {gand save customizing for another day.  The basic skills and  {x|\n\r");
      send_to_char(buf,ch);
      sprintf(buf, "    | {gspells provided are sufficient for almost any task.          {x|\n\r");
      send_to_char(buf,ch);
      write_to_buffer(d,"    |                                                              |\n\r", 0);
      write_to_buffer(d,"\n\r",0);
/*      sprintf(buf, "   {gDo you wish to customize thy knowledge?  New players should not.{x (Y/N) ",ch->name);*/
      send_to_char("   {gDo you wish to customize thy knowledge?  New players should not.{x (Y/N) ",ch);
      d->connected = CON_DEFAULT_CHOICE;
      break;

    case CON_DEFAULT_CHOICE:
      write_to_buffer(d,"\n\r",2);
      switch ( argument[0] )
        {
        case 'y': case 'Y': 
	  write_to_buffer(d,"\n\r",0);
	  sprintf(buf,"{W\n\r");
	  send_to_char(buf,ch);
	  ch->gen_data = new_gen_data();
	  ch->gen_data->points_chosen = ch->pcdata->points;
	  do_help(ch,"group header");
	  sprintf(buf,"{W");
	  send_to_char(buf,ch);
	  list_group_costs(ch);
	  sprintf(buf,"{gYou already have the following skills:{x\n\r");
	  send_to_char(buf,ch);
	  sprintf(buf,"{m");
	  send_to_char(buf,ch);
	  do_skills(ch, "50");
	  sprintf(buf,"{x\n\r");
	  send_to_char(buf,ch);
	  do_help(ch,"menu choice");
	  d->connected = CON_GEN_GROUPS;
	  break;
        case 'n': case 'N': 
	  group_add(ch,class_table[ch->class].default_group,TRUE);
	  write_to_buffer( d, "\n\r", 2 );
	  write_to_buffer(d,"    |                                                              |\n\r",0);
	  write_to_buffer(d,"    | The Dark Figure leads you away from the arena to a small hut |\n\r",0);
	  write_to_buffer(d,"    | floating in the void of space. He opens the door and leads   |\n\r",0);
	  write_to_buffer(d,"    | you inside. As you look around the hut you notice the walls  |\n\r",0);
	  write_to_buffer(d,"    | are lined with many different types of weaponry. The Dark    |\n\r",0);
	  write_to_buffer(d,"    | Figure turns to you and begins to speak                      |\n\r",0);
	  write_to_buffer(d,"    |                                                              |\n\r",0);
	  sprintf( buf, "    | {gThy task is almost complete soul. Thou must choose now{x       |\n\r");
	  send_to_char(buf,ch); 
	  sprintf( buf, "    |{g what weapon thy wishes to use upon entering the mortal    {x   |\n\r");
	  send_to_char(buf,ch); 
	  sprintf( buf, "    | {grealm.{x                                                       |\n\r");
	  send_to_char(buf,ch); 
	  send_to_char("\n\r",ch);
	  sprintf( buf, "   {gPlease pick a weapon from the following choices:{c\n\r\n\r");
	  send_to_char(buf,ch);

	  buf[0] = '\0';

	  for ( i = 0; weapon_table[i].name != NULL; i++)
	    if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
	      {
		write_to_buffer(d,weapon_table[i].name,0);
		write_to_buffer(d," ",1);
	      }
	  send_to_char("{g\n\r",ch);
	  strcat(buf,"\n\r  What is Thy choice? ");
	  write_to_buffer(d,buf,0); 
	  d->connected = CON_PICK_WEAPON;
	  break;
        default:
	  sprintf( buf, "   {gPlease answer (Y/N)?{x ");
	  send_to_char(buf,ch); 
	  return;
        }
      break;

    case CON_PICK_WEAPON:

      weapon = weapon_lookup(argument);
      if (weapon == -1 || ch->pcdata->learned[*weapon_table[weapon].gsn] <= 0)
	{
	  sprintf(buf,"   {gThat's not a valid selection. Thy choices are:{c\n\r");
	  send_to_char(buf,ch);
	  buf[0] = '\0';
	  for ( i = 0; weapon_table[i].name != NULL; i++)
	    if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
	      {
		write_to_buffer(d,weapon_table[i].name,0);
		write_to_buffer(d," ",1);
	      }
	  send_to_char("{g\n\r",ch);
	  strcat(buf,"\n\r   What is Thy choice? ");
	  write_to_buffer(d,buf,0);
	  return;
	}

      ch->pcdata->learned[*weapon_table[weapon].gsn] = 40;

/*    Add code to set all level 1 spells to 40% here */
      for (isn = 0; isn < MAX_SKILL; isn++)
      {
          if (skill_table[isn].name == NULL )
              break;

          if (skill_table[isn].skill_level[ch->class] == 1
          &&  skill_table[isn].spell_fun != spell_null )
              ch->pcdata->learned[isn] = 40;
      }
/*                       */

      send_to_char("\n\r {y[Hit return to continue]{x\n\r",ch);
      d->connected = CON_READ_PREGREETING;
      break;

    case CON_GEN_GROUPS:
      send_to_char("\n\r",ch);
      if (!str_cmp(argument,"done"))
       	{
	  sprintf(buf,"{g     Thou hast used {m%d {gcreation points.{x\n\r",ch->pcdata->points);
	  send_to_char(buf,ch);
	  sprintf(buf,"{g     Thou dost need {m%d {gexperience points to level per level.{x\n\r",
		  exp_per_level(ch,ch->gen_data->points_chosen));
	  if (ch->pcdata->points < 40)
	    ch->train = (40 - ch->pcdata->points + 1) / 2;
	  free_gen_data(ch->gen_data);
	  ch->gen_data = NULL;
	  send_to_char(buf,ch);
	  write_to_buffer( d, "\n\r", 2 );
	  write_to_buffer(d,"    |                                                              |\n\r",0);
	  write_to_buffer(d,"    | The Dark Figure leads you away from the arena to a small hut |\n\r",0);
	  write_to_buffer(d,"    | floating in the void of space. He opens the door and leads   |\n\r",0);
	  write_to_buffer(d,"    | you inside. As you look around the hut you notice the walls  |\n\r",0);
	  write_to_buffer(d,"    | are lined with many different types of weaponry. The Dark    |\n\r",0);
	  write_to_buffer(d,"    | Figure turns to you and begins to speak.                     |\n\r",0);
	  write_to_buffer(d,"    |                                                              |\n\r",0);
	  sprintf( buf, "    | {gThy task is almost complete soul. Thou must choose now{x       |\n\r");
	  send_to_char(buf,ch); 
	  sprintf( buf, "    |{g what weapon thy wishes to use upon entering the mortal    {x   |\n\r");
	  send_to_char(buf,ch); 
	  sprintf( buf, "    | {grealm.{x                                                       |\n\r");
	  send_to_char(buf,ch); 
	  send_to_char("\n\r",ch);
	  sprintf( buf, "   {gPlease pick a weapon from the following choices:{c\n\r");
	  send_to_char(buf,ch);

	  buf[0] = '\0';

	  for ( i = 0; weapon_table[i].name != NULL; i++)
	    if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
	      {
		write_to_buffer(d,weapon_table[i].name,0);
		write_to_buffer(d," ",1);
	      }
	  send_to_char("{g\n\r",ch);
	  strcat(buf,"\n\r   What is Thy choice? ");
	  write_to_buffer(d,buf,0);
	  d->connected = CON_PICK_WEAPON;
	  break;
        }

      if (!parse_gen_groups(ch,argument))
        send_to_char(
		     "Choices are: list,learned,premise,add,drop,info,help, and done.\n\r"
		     ,ch);

      do_help(ch,"menu choice");
      break;

    case CON_CHOOSE_TERM:
      if ( !strncmp (argument, "PUEBLOCLIENT", 12 ) )
        {
	  d->ansi = TRUE;
	  d->pueblo = TRUE;
	  write_to_buffer( d, "\n\rPueblo Enabled...\n\r", 0 );
        }
      else
        {
	  switch ( argument[0] )
            {
            case 'y': case 'Y':
	      d->ansi = TRUE;
	      d->pueblo = FALSE;
	      write_to_buffer(d,"\n\r",0);
	      break;
            case 'n': case 'N':
	      d->ansi = FALSE;
	      d->pueblo = FALSE;
	      write_to_buffer(d,"\n\r",0);
	      break;
            default:
	      write_to_buffer( d, "Please answer (Y/N)? ", 0 );
	      return;
            }
        }

    case CON_READ_PREGREETING:
      send_to_char("    |                                                               |\n\r",ch);
      send_to_char("    | The Dark Figure leads you away from the hut down another      |\n\r",ch);
      send_to_char("    | path. You follow him for what seems like an eternity when     |\n\r",ch);
      send_to_char("    | suddenly you come to a dead end. The Dark Figure turns to     |\n\r",ch);
      send_to_char("    | you and begins to speak.                                      |\n\r",ch);
      send_to_char("    |                                                               |\n\r",ch);
      sprintf(buf,"    |{g 'Thou art ready to enter the realm of mortal men %12s.{x|\n\r", ch->name);
      send_to_char(buf,ch);
      send_to_char("    | {gTo aid thee in thy adventure in the realm, Thou can read      {x|\n\r",ch);
      send_to_char("    | {gseveral tomes after entering. The most useful of these tomes  {x|\n\r",ch);
      send_to_char("    | {gare, {chelp newbie, help newbie2, help etiquette, help rules,  {x|\n\r",ch);
      send_to_char("    | {gand {rhelp getting started. {gThese are my last words to thee     {x|\n\r",ch);
      send_to_char("    | {gsoul, please consider them wisely for the IMMORTALS of the    {x|\n\r",ch);
      send_to_char("    | {grealm will expect thee to know thy content.                   {x|\n\r",ch);
      send_to_char("    |                                                               {x|\n\r",ch);
      send_to_char("    | The Dark Figure fades from view and a voice booms '{gWelcome {x   |\n\r",ch);
      sprintf(buf,"    | {g%14s, thine adventure awaits for thee.{x'             |\n\r",ch->name);
      send_to_char(buf,ch);
      send_to_char("    |                                                               |\n\r", ch);
      send_to_char("    | You feel the universe begin to swirl as your senses are       |\n\r", ch);
      send_to_char("    | {gseveral tomes after entering. The most useful of these tomes  {x|\n\r",ch);
      send_to_char("    | {gare, {chelp newbie, help newbie2, help etiquette, help rules,  {x|\n\r",ch);
      send_to_char("    | {gand {rhelp getting started. {gThese are my last words to thee     {x|\n\r",ch);
      send_to_char("    | {gsoul, please consider them wisely for the IMMORTALS of the    {x|\n\r",ch);
      send_to_char("    | {grealm will expect thee to know thy content.                   {x|\n\r",ch);
      send_to_char("    |                                                               {x|\n\r",ch);
      send_to_char("    | The Dark Figure fades from view and a voice booms '{gWelcome {x   |\n\r",ch);
      sprintf(buf,"    | {g%14s, thine adventure awaits for thee.{x'             |\n\r",ch->name);
      send_to_char(buf,ch);
      send_to_char("    |                                                               |\n\r", ch);
      send_to_char("    | You feel the universe begin to swirl as your senses are       |\n\r", ch);
      send_to_char("    | bombarded by the swirling vortex surrounding you. As your     |\n\r", ch);
      send_to_char("    | soul falls down to the realm of mortal men you think you can  |\n\r", ch);
      send_to_char("    | make out what appears to be  a sign.............              |\n\r", ch);
      send_to_char("\n\r", ch);
      send_to_char("\n\r", ch);
      write_to_buffer( d, " ", 0);
      d->connected = CON_READ_GREETING;
      break;

    case CON_READ_GREETING:
      if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
        {
	  write_to_buffer( d, "Warning! Null password!\n\r",0 );
	  write_to_buffer( d, "Please report old password with bug.\n\r",0);
	  write_to_buffer( d,
			   "Type 'password null <new password>' to fix.\n\r",0);
        }

      ch->next	= char_list;
      char_list	= ch;
      d->connected	= CON_PLAYING;
      reset_char(ch);

      if ((IS_SET(ch->shiftbits, TEMP_VAMP)) || (IS_SET(ch-> shiftbits,PERM_VAMP)))
	{
	  if ((time_info.hour < 6) && (time_info.hour > 19)) 
            {
	      /* They quit in the day, its now night, add some affects */
	      vampire_enhance( ch );
            }
	  else if ((time_info.hour > 5) && (time_info.hour < 20) ) 
            {
              /* quit at night, its now day */
              vampire_unenhance( ch );
            }
	}

      if ((ch->level > 29) && (IS_SET(ch->shiftbits,SHIFT_POTENTIAL)))
	{

	  if ((time_info.hour < 6) && (time_info.hour > 19))
            {
              /*They quit during day and come back during permwere time*/
              were_chance( ch );
              shift_strip2( ch );
            }

	  else if ((IS_AFFECTED(ch,AFF_SHIFT_PERM))
		   && (time_info.hour < 20)
		   && (time_info.hour > 6)) 
            {
	      /*They quit during permwere time and come back during day*/
	      shift_strip(ch);
            }

	}

      if ( ch->level == 0 )
	{

	  /* set all new character spells/skills to minimum */
	  for( isn = 0; isn < MAX_SKILL; isn++ )
	    {
	      if( skill_table[ isn ].name == NULL )
		break;

	      if( ch->pcdata->learned[ isn ] > 0 &&
		  ch->pcdata->learned[ isn ] < MIN_SKILL_PCT )
		{
		  ch->pcdata->learned[ isn ] = MIN_SKILL_PCT;
		}
	    }

	  
	  /* WORKING */
	  ch->perm_stat[class_table[ch->class].attr_prime] += 3;

	  ch->level	= 1;
	  ch->exp	= exp_per_level(ch,ch->pcdata->points);
	  ch->hit	= ch->max_hit;
	  ch->mana	= ch->max_mana;
	  ch->move	= ch->max_move;
	  ch->train	 = 3;
	  ch->practice = 5;
	  set_title( ch, "the Adventurer" );
	  SET_BIT(ch->act_bits,PLR_AUTOEXIT);
	  SET_BIT(ch->comm, COMM_SHOW_AFFECTS );
/*        SET_BIT( ch->comm, COMM_NOBRAWLER ); */
          SET_BIT( ch->comm, COMM_DRMODE );
	  ch->wimpy = 5;
	  if (IS_SET(ch->act_bits, PLR_PUEBLO)) {
	    sprintf(pueblo_buf,"<img xch_sound=play xch_volume=%d src=\"%s%s\">"
		    "<img src=\"%s%s\">",
		    ch->pcdata->volume,PUEBLO_DIR,PUEBLO_START, PUEBLO_DIR, PUEBLO_S_IMG);
	    send_to_char("</xch_mudtext><img xch_mode=html>",ch);
	    send_to_char(pueblo_buf,ch);
	    send_to_char("<br><img xch_mode=text>",ch);
	  }
	  else send_to_char("\n\r",ch);
	  if (IS_SET(ch->act_bits, PLR_PUEBLO))
	    send_to_char("<xch_mudtext>{x",ch);
	  do_help(ch,"greeting");
	  do_outfit(ch,"");
	  obj_to_char(create_object(get_obj_index(OBJ_VNUM_MAP),0),ch);

	  char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
	  send_to_char("\n\r",ch);
	  send_to_char("\n\r",ch);
	}
      else if ( ch->in_room != NULL )
	{
	  char_to_room( ch, ch->in_room );
	}
      else if ( IS_IMMORTAL(ch) )
	{
	  char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
	}
      else
	{
	  char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}

      act_new( "{g$n has entered the realm of mortal men.{x", ch, NULL, NULL, 
          TO_ROOM, POS_SPRAWLED, ch->invis_level );
      do_look( ch, "auto" );

      if( !IS_NPC(ch) && IS_SET( ch->act_bits, PLR_LINKDEAD ) )
	REMOVE_BIT( ch->act_bits, PLR_LINKDEAD );

      wiznet("$N has left real life behind.",ch,NULL,
	     WIZ_LOGINS,WIZ_SITES,get_trust(ch));
      check_multiplay(ch);

      if (ch->pet != NULL)
	{
	  char_to_room(ch->pet,ch->in_room);
	  act("$n has entered the game.",ch->pet,NULL,NULL,TO_ROOM);
	}

      do_unread(ch,"");
      ch->logon = current_time;
      ch->pcdata->latelog = TRUE;
      break;
    }
 
  return;
}
 


/*
 * Parse a name for acceptability.
 */



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&&   (!fConn || ch->desc == NULL)
	&&   !str_cmp( d->character->name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
/*		This player quitting bit is pretty smart; it is located here to prevent
                players from duplicating their weapons when they reconnect from linkdeath. */
                player_quitting = TRUE;
		free_char( d->character );
                player_quitting = FALSE;
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
		send_to_char(
		    "'{GThou hast regained thy body, Thou must type replay to see thy missed tells.'{x\n\r", ch);
		act_new( "$n has reconnected.", ch, NULL, NULL, TO_ROOM,
                    POS_SPRAWLED, ch->invis_level );

		sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
		log_string( log_buf );
		wiznet("$N groks the fullness of $S link.",
		    ch,NULL,WIZ_LINKS,0,ch->level);
		d->connected = CON_PLAYING;
		REMOVE_BIT( ch->act_bits, PLR_LINKDEAD );
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    write_to_buffer( d, "'My records show this body is already occupied.'\n\r",0);
	    write_to_buffer( d, "'Dost thou wish to take it over?' (Y/N)?",0);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}

bool check_playing2( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    return TRUE;
	}
    }

    return FALSE;
}


void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    REMOVE_BIT( ch->act_bits, PLR_LINKDEAD );
    return;
}



/*
 * Write to one char.
 */
void send_to_char_bw( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
        write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}

/*
 * Write to one char, new colour version, by Lope.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    const	char 	*point;
    		char 	*point2;
    		char 	buf[ MAX_STRING_LENGTH*4 ] = "";
		int	skip = 0;

    point2 = buf;
    if( txt && ch->desc )
	{
	    if(IS_SET( ch->act_bits, PLR_COLOUR )
	      || IS_NPC(ch))
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			skip = colour( *point, ch, point2 );
			while( skip-- > 0 )
			    ++point2;
			continue;
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}			
		*point2 = '\0';
        	write_to_buffer( ch->desc, buf, point2 - buf );
	    }
	    else
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			continue;
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}
		*point2 = '\0';
        	write_to_buffer( ch->desc, buf, point2 - buf );
	    }
	}
    return;
}

void send_to_desc( const char *txt, DESCRIPTOR_DATA *d )
{
    const	char 	*point;
    		char 	*point2;
    		char 	buf[ MAX_STRING_LENGTH*4 ];
		int	skip = 0;

    buf[0] = '\0';
    point2 = buf;
    if( txt && d )
	{
	    if( d->ansi == TRUE )
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			skip = colour( *point, NULL, point2 );
			while( skip-- > 0 )
			    ++point2;
			continue;
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}			
		*point2 = '\0';
        	write_to_buffer( d, buf, point2 - buf );
	    }
	    else
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			continue;
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}
		*point2 = '\0';
        	write_to_buffer( d, buf, point2 - buf );
	    }
	}
    return;
} 

/*
 * Send a page to one char.
 */
void page_to_char_bw( const char *txt, CHAR_DATA *ch )
{
  if ( txt == NULL || ch->desc == NULL)


    if (ch->lines == 0 )
      {
	send_to_char(txt,ch);
	return;
      }
	
  ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
  strcpy(ch->desc->showstr_head,txt);
  ch->desc->showstr_point = ch->desc->showstr_head;
  show_string(ch->desc,"");

}

/*
 * page_to_char()
 * There's a problem with this function and coloring, since this version
 * swaps out color codes (two characters) for ansi codes (seven characters)
 * which, if txt is too large, will cause an overflow and crash.
 *
 * I put a fix in that should stop sigsevs, but it doesn't "fix" the
 * problem; it only fixes the symptom of crashing.
 * 
 * 7/30/06 gkl
 */
void page_to_char( const char* txt, CHAR_DATA* ch )
{
  const char* point = NULL;
  char *point2 = NULL;
  char buf[ MAX_STRING_LENGTH * 4 ];
  char tmp[1024];
  int skip = 0;
  int lostr = 1;
  bool color_on = FALSE;

  buf[0] = '\0';
  point2 = buf;

  color_on = ( IS_SET( ch->act_bits, PLR_COLOUR ) || IS_NPC( ch ) );

  if( txt && ch->desc )
    {
      tmp[0] = '\0';
      for ( point = txt; *point; point++ )
      {
          if ( *point == '{' )
          {
            point++;
            if ( color_on )
                skip = colour( *point, ch, tmp );
            while ( skip-- > 0 )
                lostr++;
            tmp[0] = '\0';
          }
          else
            lostr++;
      }

      if ( lostr > MAX_STRING_LENGTH * 4 )
      {
          sprintf(tmp, "page_to_char: overflowed buffer called by %s", ch->name );
          bug ( tmp, 0 );
          send_to_char( "Output too large.  Try refining your search "
                        "or as an immortal for help.\n\r", ch );
          return;
      }

      for( point = txt; *point; point++ )
	{
	  if( *point == '{' )
	    {
	      point++;

	      if( color_on )
		{
		  skip = colour( *point, ch, point2 );
		  while( skip-- > 0 )
		    point2++;
		}
	    }
	  else
	    {
	      *point2 = *point;
	      point2++;
	      *point2 = '\0';
	    }
	}

      *point2 = '\0';
      ch->desc->showstr_head  = alloc_mem( strlen( buf ) + 1 );
      strcpy( ch->desc->showstr_head, buf );
      ch->desc->showstr_point = ch->desc->showstr_head;
      show_string( ch->desc, "" );
    }
}

/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
  char* buffer = NULL;
  char arg[ MAX_INPUT_LENGTH ] = "";
  register char *scan = NULL;
  register char *chk = NULL;
  int lines = 0, toggle = 1;
  int show_lines = 0;
  size_t buffer_len = 0;

  one_argument( input, arg );
  
  if( arg[ 0 ] != '\0')
  {
      if (d->showstr_head)
      {
          free_mem(d->showstr_head,strlen(d->showstr_head));
          d->showstr_head = 0;
      }
      d->showstr_point  = 0;
      return;
  }
  
  if (d->character)
      show_lines = d->character->lines;
  else
      show_lines = 0;
 
  buffer_len = strlen( d->showstr_point ) + 1;

  buffer = ( char* )calloc( buffer_len, sizeof( char ) );

  for (scan = buffer; ; scan++, d->showstr_point++)
  {
      if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
      && (toggle = -toggle) < 0)
          lines++;
      else if (!*scan || (show_lines > 0 && lines >= show_lines))
      {
          *scan = '\0';
          write_to_buffer(d,buffer,strlen(buffer));
          
          /* this is a bug, but it appears that this bug is now required
             or else the [hit return to continue] part stops working.  fix
             this sometime. 2-nov-2013 gkl */
          for (chk = d->showstr_point; isspace(*chk); chk++);
          {
              if (!*chk)
              {
                  if (d->showstr_head)
                  {
                    free_mem(d->showstr_head,strlen(d->showstr_head));
                    d->showstr_head = 0;
                  }
                  d->showstr_point  = 0;
              }
          }
          if( buffer )
            free( buffer );

          return;
        }
    }
    free( buffer );
    return;
}

char * parse_act( const char *format, CHAR_DATA *ch, const void *arg1, 
	      const void *arg2, int type, CHAR_DATA *to )
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };

    CHAR_DATA 		*vch = ( CHAR_DATA * ) arg2;
    OBJ_DATA 		*obj1 = ( OBJ_DATA  * ) arg1;
    OBJ_DATA 		*obj2 = ( OBJ_DATA  * ) arg2;
    const 	char 	*str;
    char 		*i = NULL;
    char 		*point;
    char* pBuffer;
    static char 	buffer[ MAX_STRING_LENGTH*2 ];
    char 		buf[ MAX_STRING_LENGTH   ];
    bool		fColour = FALSE;

    point   = buf;
    str     = format;
    while( *str != '\0' )
    {
        if( *str != '$' )
            {
	      *point++ = *str++;
	      continue;
            }
 
	  fColour = TRUE;
	  ++str;
	  i = " <@@@> ";
	  if( !arg2 && *str >= 'A' && *str <= 'Z' )
            {
	      bug( "Act: missing arg2 for code %d.", *str );
	      i = " <@@@> ";
            }
	  else
            {
	      switch ( *str )
                {
                default:  bug( "Act: bad code %d.", *str );
		  i = " <@@@> ";                                break;
                case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];        break;
                case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];        break;
                case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];        break;
                case 'M': i = him_her [URANGE(0, vch ->sex, 2)];        break;
                case 's': i = his_her [URANGE(0, ch  ->sex, 2)];        break;
                case 'S': i = his_her [URANGE(0, vch ->sex, 2)];        break;

		case 't' : if (arg1) i = (char *) arg1;
		else bug("Act: bad code $t for 'arg1'",0);
		break;
		case 'T' : if (arg2) i = (char *) arg2;
		else bug("Act: bad code $T for 'arg2'",0);
		break;
		case 'n' : if (ch&&to) i = PERS( ch, to );
		else bug("Act: bad code $n for 'ch' or 'to'", 0);
		break;
		case 'N' : if (vch&&to) i = PERS( vch, to );
		else bug("Act: bad code $N for 'vch' or 'to'", 0);
		break;
                case 'p':
		  i = can_see_obj( to, obj1 )
		    ? obj1->short_descr
		    : "something";
		  break;
 
                case 'P':
		  i = can_see_obj( to, obj2 )
		    ? obj2->short_descr
		    : "something";
		  break;
 
                case 'd':
		  if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
                    {
		      i = "door";
                    }
		  else
                    {
		      i = (char *) arg2;
                    }
		  break;
                }
            }
 
	  ++str;
	  while ( ( *point = *i ) != '\0' )
	    ++point, ++i;
        }
 
      *point++ = '\n';
      *point++ = '\r';
      *point	 = '\0';
      buf[0]   = UPPER(buf[0]);
      pBuffer = buffer;
      colourconv( pBuffer, buf, to );

      return buffer;
}


/* quick sex fixer */
void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
    	ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}

void act (const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2,
	  int type)
{
    /* to be compatible with older code */
    act_new(format,ch,arg1,arg2,type,POS_SPRAWLED,0 );
}

/*
 * act_new functions as the lowest-level act function
 */
void act_new( const char *format, CHAR_DATA *ch, const void *arg1, 
              const void *arg2, int type, int min_pos, int min_level )
{
    CHAR_DATA *to;
    CHAR_DATA *vch = ( CHAR_DATA * ) arg2;
    char      *buffer;

    /*
     * Discard null and zero-length messages.
     */
    if( !format || !*format )
        return;

    if( ch == NULL || ch->in_room == NULL )
        return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
        if ( !vch )
        {
            bug( "Act: null vch with TO_VICT.", 0 );
            return;
        }
        if ( !vch->in_room )
            return;
        to = vch->in_room->people;
    }
 
    for ( ; to ; to = to->next_in_room )
    {
        if ( (to->desc == NULL && (!IS_NPC(to) || !HAS_TRIGGER(to, TRIG_ACT)))
        ||   to->position  < min_pos 
        ||   get_trust(to) < min_level )
            continue;
 
        if( ( type == TO_CHAR ) && to != ch )
            continue;
        if( type == TO_VICT && ( to != vch || to == ch ) )
            continue;
        if( type == TO_ROOM && to == ch )
            continue;
        if( type == TO_NOTVICT && (to == ch || to == vch) )
            continue;
 
        buffer = parse_act( format, ch, arg1, arg2, type, to );
        write_to_buffer( to->desc, buffer, 0 );
    }

    /* 
     * Loops again so mptriggers are guaranteed to appear after the act 
     * to all people in room 
     */
    if ( !IS_NPC( ch ) && MOBtrigger )
    {
        for ( to = ch->in_room->people; to; to = to->next_in_room )
        {
            if ( !IS_NPC(to)
            ||  !HAS_TRIGGER( to, TRIG_ACT )
            ||   to->position  < min_pos
            ||   get_trust(to) < min_level )
                continue;

            if ( ( type == TO_CHAR ) && to != ch )
                continue;
            if ( type == TO_VICT && ( to != vch || to == ch ) )
                continue;
            if ( type == TO_ROOM && to == ch )
                continue;
            if ( type == TO_NOTVICT && (to == ch || to == vch) )
                continue;

            buffer = parse_act( format, ch, arg1, arg2, type, to );
            mp_act_trigger( buffer, to, ch, arg1, arg2, TRIG_ACT );
        }
    }
    return;
}

int colour( char type, CHAR_DATA *ch, char *string )
{
    char	code[ 20 ];
    char	*p = '\0';

    if( ch && IS_NPC( ch ) && ch->desc == NULL )
	return( 0 );

/*  Special character codes go here. */
    if ( type == 's' )
        type = ch->colors[C_SAY];
    if ( type == 't' )
        type = ch->colors[C_TELL];

    switch( type )
    {
	default:
	    sprintf( code, CLEAR );
	    break;
	case 'x':
	    sprintf( code, CLEAR );
	    break;
	case 'b':
	    sprintf( code, C_BLUE );
	    break;
	case 'c':
	    sprintf( code, C_CYAN );
	    break;
	case 'g':
	    sprintf( code, C_GREEN );
	    break;
	case 'm':
	    sprintf( code, C_MAGENTA );
	    break;
	case 'r':
	    sprintf( code, C_RED );
	    break;
	case 'w':
	    sprintf( code, C_WHITE );
	    break;
	case 'y':
	    sprintf( code, C_YELLOW );
	    break;
	case 'B':
	    sprintf( code, C_B_BLUE );
	    break;
	case 'C':
	    sprintf( code, C_B_CYAN );
	    break;
	case 'G':
	    sprintf( code, C_B_GREEN );
	    break;
	case 'M':
	    sprintf( code, C_B_MAGENTA );
	    break;
	case 'R':
	    sprintf( code, C_B_RED );
	    break;
	case 'W':
	    sprintf( code, C_B_WHITE );
	    break;
	case 'Y':
	    sprintf( code, C_B_YELLOW );
	    break;
	case 'D':
	    sprintf( code, C_D_GREY );
	    break;
	case '*':
	    sprintf( code, "%c", 007 );
	    break;
	case '{':
	    sprintf( code, "%c", '{' );
	    break;
        case '-':
            sprintf( code, "%c", '~' );
            break;
    }

    p = code;
    while( *p != '\0' )
    {
	*string = *p++;
	*++string = '\0';
    }

    return( strlen( code ) );
}

void colourconv( char *buffer, const char *txt, CHAR_DATA *ch )
{
  const	char	*point;
  int	skip = 0;
  
  /* if( ch->desc && txt ) */

  if( txt )
    {
      if(( IS_SET( ch->act_bits, PLR_COLOUR ) ) || IS_NPC(ch))
	{
	  for( point = txt ; *point ; point++ )
	    {
	      if( *point == '{' )
		{
		  point++;
		  skip = colour( *point, ch, buffer );
		  while( skip-- > 0 )
		    ++buffer;
		  continue;
		}
	      *buffer = *point;
	      *++buffer = '\0';
	    }			
	  *buffer = '\0';
	}
      else
	{
	  for( point = txt ; *point ; point++ )
	    {
	      if( *point == '{' )
		{
		  point++;
		  continue;
		}
	      *buffer = *point;
	      *++buffer = '\0';
	    }
	  *buffer = '\0';
	}
    }
  return;
}

/* for glibc getline() */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "clans.h"
#include "lookup.h"
#include "interp.h"

Clan* clanList = NULL;

/* name, syntax, has victim?, imm only?, leader only?, function */
ClanSubcommand cmdList[] = 
  {
    { "establish", "establish <leader> <clan name> <symbol>", TRUE, TRUE, FALSE, clanEstablish },
    { "leader", "leader <new leader> <clan name>", TRUE, TRUE, FALSE, clanLeader },
    { "apply", "apply <leader>", TRUE, FALSE, FALSE, clanApply },
    { "accept", "accept <supplicant>", TRUE, FALSE, TRUE, clanAccept },
    { "remove", "remove <character>", TRUE, FALSE, TRUE, clanRemove },
    { "destroy", "destroy <clan>", FALSE, TRUE, FALSE, clanDestroy },
    { "list", "list", FALSE, TRUE, FALSE, clanListing },
    { "grant", "grant <member> <favor>", TRUE, FALSE, TRUE, clanGrant },
    { "revoke", "revoke <member> <favor>", TRUE, FALSE, TRUE, clanRevoke },
    { NULL, NULL, FALSE, FALSE, FALSE, NULL }
  };



void do_clan( CHAR_DATA* ch, char* argument )
{
  char buf[ MAX_INPUT_LENGTH ] = "";
  char arg1[ MAX_INPUT_LENGTH ] = "";
  char arg2[ MAX_INPUT_LENGTH ] = "";
  ClanSubcommand* nextCmd = NULL;
  CHAR_DATA* victim = NULL;
  bool success = FALSE;

  if( IS_NPC( ch ) )
    {
      send_to_char( "NPCs can't perform clan business.\n\r", ch );
      return;
    }

  if( IS_NULLSTR( argument ) )
    {
      send_to_char( "What clan function would you like to perform?\n\r", ch );
      sendClanCmdList( ch );
      return;
    }

  smash_tilde( argument );
  argument = one_argument( argument, arg1 );

  nextCmd = clanCmdLookup( arg1 );

  send_to_char( buf, ch );

  if( nextCmd == NULL )
    {
      send_to_char( "That is not a valid clan function. Syntax:\n\r", ch );
      sendClanCmdList( ch );
      return;
    }



  /* Most subcommands require a victim, but not all do. The command dispatcher requires */
  /* all the functions to have the same argument types, so we'll pass NULL into those   */
  /* subcommand functions that don't really need a victim */

  if( nextCmd->hasVictim == TRUE )
    {
      argument = one_argument( argument, arg2 );
      
      victim = get_char_world( ch, arg2, FALSE );
      if( victim == NULL )
	{
	  send_to_char( "That clan command requires a named player. Syntax:\n\r", ch );
	  sprintf( buf, "  clan %s\n\r", nextCmd->syntax );
	  send_to_char( buf, ch );
	  return;
	}

      if( IS_NPC( victim ) )
	{
	  send_to_char( "NPCs can't relate to clan business.\n\r", ch );
	  return;
	}
    }
  if( nextCmd->immOnly && !IS_IMMORTAL( ch ) )
    {
      send_to_char( "Huh?\n\r", ch );
      return;
    }

  success = ( nextCmd->csHandler )( ch, victim, argument );

  if( !success )
    send_to_char( "Clan command failure.\n\r", ch );

}



void do_clantalk( CHAR_DATA* ch, char* argument )
{
  char buf[ MAX_INPUT_LENGTH ] = "";
  DESCRIPTOR_DATA* d = NULL;
  CHAR_DATA* receiver = NULL;

  if( ch->pcdata->pcClan == NULL )
    {
      send_to_char( "You have to be in a clan to talk to one.\n\r", ch );
      return;
    }

  if( IS_NULLSTR( argument ) )
    {
      if( IS_SET( ch->comm, COMM_NOCLAN ) )
	{
	  send_to_char( "Clan talk is now ON.\n\r", ch );
	  REMOVE_BIT( ch->comm, COMM_NOCLAN );
	}
      else
	{
	  send_to_char( "Clan talk is now OFF.\n\r", ch );
	  SET_BIT( ch->comm, COMM_NOCLAN );
	}

      return;
    }

  sprintf( buf, "{B[%s]{x You: {%c'%s'{x\n\r", 
	   capitalize( ch->pcdata->pcClan->name ),
	   ch->colors[ C_CLANTALK ], 
	   argument );

  send_to_char( buf, ch );

  for( d = descriptor_list; d != NULL; d = d->next )
    {
      receiver = d->original ? d->original : d->character;

      if( d->connected == CON_PLAYING && receiver != ch && ch->pcdata->pcClan == receiver->pcdata->pcClan ) 
	{
	  sprintf( buf, "{B[%s]{x %s: {%c'%s'{x\n\r", 
		   capitalize( ch->pcdata->pcClan->name ), 
		   PERS2( ch, receiver ), 
		   receiver->colors[ C_CLANTALK ],
		   argument );

	  send_to_char( buf, receiver );
	}
    }
}



bool loadClans( void )
{
  FILE* clansFile = NULL;
  char newName[ MAX_INPUT_LENGTH ] = "";
  char newLeader[ MAX_INPUT_LENGTH ] = "";
  char newSymbol[ MAX_INPUT_LENGTH ] = "";
  time_t newTime;
  char* tempLine = NULL;
  int lineLength = 0;

  clansFile = fopen( CLANS_FILE, "r" );
  if( clansFile == NULL )
    {
      return FALSE;
    }

  while( !feof( clansFile ) )
    {
      if( getline( &tempLine, &lineLength, clansFile ) > 0 )
	{
	  sscanf( tempLine, "%[^,],%[^,],%[^,],%ld\n]", newName, newLeader, newSymbol, &newTime );
	  makeClan( newName, newLeader, newSymbol, newTime );

	  free( tempLine );
	  tempLine = NULL;

	}
    }

  return ( fclose( clansFile ) == 0 ? TRUE : FALSE );
}



bool saveClans( void )
{
  FILE* clansFile = NULL;
  Clan* currentClan = NULL;
  char buf[ MAX_INPUT_LENGTH ] = "";

  clansFile = fopen( CLANS_FILE, "w" );
  if( clansFile == NULL )
    return FALSE;

  for( currentClan = clanList; currentClan != NULL; currentClan = currentClan->nextClan )
    {
      sprintf( buf, "%s,%s,%s,%ld\n", 
	       currentClan->name, 
	       currentClan->leader, 
	       currentClan->symbol,
	       ( long int )( currentClan->established ) );

      if( fputs( buf, clansFile ) == EOF )
	return FALSE;
    }

  return ( fclose( clansFile ) == 0 ? TRUE : FALSE );

}



Clan* makeClan( const char* newName, const char* newLeader, const char* newSymbol, time_t newTime )
{
  Clan* theNewClan = NULL;

  if( IS_NULLSTR( newName ) || IS_NULLSTR( newLeader ) || IS_NULLSTR( newSymbol ) )
    return NULL;

  theNewClan = (Clan*)malloc( sizeof( Clan ) );
  if( theNewClan == NULL )
    {
      bug( "clanEstablish: memory allocation error.", 0 );
      return NULL;
    }

  theNewClan->name = str_dup( newName );
  theNewClan->leader = str_dup( newLeader );
  theNewClan->symbol = str_dup( newSymbol );
  theNewClan->established = newTime;

  theNewClan->nextClan = clanList;
  
  clanList = theNewClan;

  saveClans();

  return theNewClan;
}
  


bool eraseClan( Clan* oldClan )
{
  Clan* prevClanPtr;
  Clan* listClanPtr;

  if( oldClan == NULL )
    return FALSE;

  if( oldClan == clanList )
    {
      clanList = oldClan->nextClan;
    }
  else
    {
      listClanPtr = clanList;

      while( listClanPtr != NULL && listClanPtr != oldClan )
	{
	  prevClanPtr = listClanPtr;
	  listClanPtr = listClanPtr->nextClan;
	}

      prevClanPtr->nextClan = listClanPtr->nextClan;
    }

  free_string( oldClan->name );
  free_string( oldClan->leader );
  free_string( oldClan->symbol );

  free( oldClan );

  saveClans();

  return TRUE;
}



Clan* getClanByName( const char* desiredName )
{
  Clan* listPtr = NULL;

  if( IS_NULLSTR( desiredName ) || clanList == NULL )
    return NULL;

  for( listPtr = clanList; listPtr; listPtr = listPtr->nextClan )
    {
      if( listPtr->name && !str_prefix( listPtr->name, desiredName ) )
	return listPtr;
    }

  return NULL;
}



Clan* getClanBySymbol( const char* desiredSymbol )
{
  Clan* listPtr = NULL;

  if( IS_NULLSTR( desiredSymbol ) || clanList == NULL )
    return NULL;

  for( listPtr = clanList; listPtr; listPtr = listPtr->nextClan )
    {
      if( listPtr->symbol && !str_prefix( listPtr->symbol, desiredSymbol ) )
	return listPtr;
    }

  return NULL;
}



Clan* getClanByLeader( const char* desiredLeader )
{
  Clan* listPtr = NULL;
  
  if( IS_NULLSTR( desiredLeader ) || clanList == NULL )
    return NULL;

  for( listPtr = clanList; listPtr; listPtr = listPtr->nextClan )
    {
      if( listPtr->name && !str_cmp( desiredLeader, listPtr->leader ) )
	return listPtr;
    }

  return NULL;
}



ClanSubcommand* clanCmdLookup( const char* cmd )
{
  int i;
  ClanSubcommand* cmdPtr = cmdList;

  if( cmd == NULL )
    return NULL;

  /* name, syntax, immOnly, csHandler */
  for( i = 0; cmdPtr[ i ].name != NULL; i++ )
    {
      if( !str_prefix( cmdPtr[ i ].name, cmd ) )
	return &cmdPtr[ i ];
    }

  return NULL;
}



void sendClanCmdList( CHAR_DATA* ch )
{
  char buf[ MAX_INPUT_LENGTH ] = "";
  int i;
  bool iCmd;

  if( ch != NULL )
    {
      for( i = 0; cmdList[ i ].name != NULL; i++ )
	{
	  iCmd = FALSE;

	  if( IS_IMMORTAL( ch ) )
	    {
	      iCmd = TRUE;
	    }
	  else if( cmdList[ i ].immOnly == FALSE )
	    {
	      if( cmdList[ i ].leaderOnly )
		{
		  if( getClanByLeader( ch->name ) )
		    {
		      iCmd = TRUE;
		    }
		}
	      else
		{
		  iCmd = TRUE;
		}
	    }
	  
	  if( iCmd )
	    {
	      sprintf( buf, "  clan %s\n\r", cmdList[ i ].syntax );
	      send_to_char( buf, ch );
	    }
	}
    }
}


bool clanEstablish( CHAR_DATA* ch, CHAR_DATA* newLeader, char* argument )
{
  char buf[ MAX_INPUT_LENGTH ] = "";
  char newName[ MAX_INPUT_LENGTH ] = "";
  char newSymbol[ MAX_INPUT_LENGTH ] = "";
  Clan* newClan = NULL;

  if( ch == NULL )
    return FALSE;

  if( newLeader == NULL )
    {
      send_to_char( "You must specify a leader for the new clan.\n\r", ch );
      sprintf( buf, "Syntax: clan %s\n\r", clanCmdLookup( "establish" )->syntax );
      send_to_char( buf, ch );
      return FALSE;
    }

  if( getClanByLeader( newLeader->name ) )
    {
      send_to_char( "That person is already the leader of a clan.\n\r", ch );
      return FALSE;
    }

  if( IS_NULLSTR( argument ) )
    {
      send_to_char( "You must specify a name for the new clan.\n\r", ch );
      sprintf( buf, "Syntax: clan %s\n\r", clanCmdLookup( "establish" )->syntax );
      send_to_char( buf, ch );
      return FALSE;
    }

  argument = preserve_case_one_argument( argument, newName );
  argument = preserve_case_one_argument( argument, newSymbol );

  if( IS_NULLSTR( newSymbol ) )
    {
      send_to_char( "You must specify a symbol for the new clan.\n\r", ch );
      sprintf( buf, "Syntax: clan %s\n\r", clanCmdLookup( "establish" )->syntax );
      send_to_char( buf, ch );
      return FALSE;
    }

  if( guild_lookup( newName ) != 0 || getClanByName( newName ) || getClanBySymbol( newName ) )
    {
      send_to_char( "You can't use an extant guild, clan, or clan symbol as a new clan name.\n\r", ch );
      return FALSE;
    }

  if( guild_lookup( newSymbol ) != 0 || getClanByName( newSymbol ) || getClanBySymbol( newSymbol ) )
    {
      send_to_char( "You can't use an extant guild, clan, or clan symbol as a new clan's symbol.\n\r", ch );
      return FALSE;
    }
  
  newClan = makeClan( newName, newLeader->name, newSymbol, time( NULL ) );
  if( newClan == NULL )
    {
      send_to_char( "System error while making new clan.\n\r", ch );
      return FALSE;
    }
  else
    {
      if( ch == newLeader )
	{
	  sprintf( buf, "Clan %s has been established with you as its leader and %s as its symbol.\n\r",
		   newClan->name,
		   newClan->symbol );
	}
      else
	{
	  sprintf( buf, "You have established clan %s with %s as its leader and %s as its symbol.\n\r", 
		   newName, 
		   newLeader->name, 
		   newSymbol );

	  send_to_char( "You are now the leader of your clan!\n\r", newLeader );

	}
 
      send_to_char( buf, ch );

      newLeader->pcdata->pcClan = newClan;

      return TRUE;
    }
}



bool clanLeader( CHAR_DATA* ch, CHAR_DATA* newLeader, char* argument )
{
  char buf[ MAX_INPUT_LENGTH ] = "";
  Clan* theClan = NULL;

  if( ch == NULL )
    return FALSE;

  if( newLeader == NULL )
    {
      send_to_char( "You must specify a new leader for the clan.\n\r", ch );
      sprintf( buf, "Syntax: clan %s\n\r", clanCmdLookup( "leader" )->syntax );
      send_to_char( buf, ch );
      return FALSE;
    }

  if( IS_NULLSTR( argument ) )
    {
      send_to_char( "For what clan would you like to name a new leader?.\n\r", ch );
      sprintf( buf, "Syntax: clan %s\n\r", clanCmdLookup( "leader" )->syntax );
      send_to_char( buf, ch );
      return FALSE;
    }

  theClan = getClanByName( argument );
  if( theClan == NULL )
    {
      send_to_char( "There is no clan by that name.\n\r", ch );
      return FALSE;
    }

  if( theClan != newLeader->pcdata->pcClan )
    {
      sprintf( buf, "%s is not a member of clan %s.\n\r", newLeader->name, theClan->name );
      send_to_char( buf, ch );
      return FALSE;
    }

  free_string( theClan->leader );
  theClan->leader = str_dup( newLeader->name );

  sprintf( buf, "%s is the new leader of clan %s.\n\r", newLeader->name, theClan->name );
  send_to_char( buf, ch );

  send_to_char( "You are the new leader of your clan!\n\r", newLeader );

  return TRUE;
}



bool clanApply( CHAR_DATA* ch, CHAR_DATA* leader, char* argument )
{
  char buf[ MAX_INPUT_LENGTH ] = "";
  Clan* theClan = NULL;

  if( ch == NULL || leader == NULL )
    return FALSE;

  theClan = getClanByLeader( leader->name );
  if( theClan == NULL )
    {
      send_to_char( "That person doesn't lead a clan.\n\r", ch );
      return FALSE;
    }

  ch->pcdata->pcClanApply = theClan;

  sprintf( buf, "You have applied to %s for membership in clan %s\n\r", leader->name, theClan->name );
  send_to_char( buf, ch );

  sprintf( buf, "%s has applied for membership in your clan.\n\r", ch->name );
  send_to_char( buf, leader );

  return TRUE;
}



bool clanAccept( CHAR_DATA* ch, CHAR_DATA* supplicant, char* argument )
{
  char buf[ MAX_INPUT_LENGTH ] = "";

  if( ch == NULL || supplicant == NULL )
    return FALSE;

  if( getClanByLeader( ch->name ) == NULL )
    {
      send_to_char( "You have to lead a clan to accept someone into a clan.\n\r", ch );
      return FALSE;
    }

  if( ch->pcdata->pcClan != supplicant->pcdata->pcClanApply )
    {
      sprintf( buf, "%s hasn't applied to your clan.\n\r", supplicant->name );
      send_to_char( buf, ch );
      return FALSE;
    }

  supplicant->pcdata->pcClanApply = NULL;
  supplicant->pcdata->pcClan = ch->pcdata->pcClan;
  
  sprintf( buf, "You have accepted %s into your clan!\n\r", supplicant->name );
  send_to_char( buf, ch );

  sprintf( buf, "%s has accepted you into clan %s!\n\r", ch->name, supplicant->pcdata->pcClan->name );
  send_to_char( buf, supplicant );

  return TRUE;
}



bool clanRemove( CHAR_DATA* ch, CHAR_DATA* victim, char* argument )
{
  char buf[ MAX_INPUT_LENGTH ] = "";

  if( ch == NULL || victim == NULL )
    return FALSE;

  if( getClanByLeader( ch->name ) == NULL )
    {
      send_to_char( "You can't remove someone from a clan if you're not the leader.\n\r", ch );
      return FALSE;
    }

  if( ch->pcdata->pcClan != victim->pcdata->pcClan )
    {
      sprintf( buf, "You and %s are not members of the same clan.\n\r", victim->name );
      return FALSE;
    }

  victim->pcdata->pcClan = NULL;

  sprintf( buf, "You have removed %s from the clan!\n\r", victim->name );
  send_to_char( buf, ch );

  sprintf( buf, "%s has removed you from your clan!\n\r", ch->name );
  send_to_char( buf, victim );

  return TRUE;
}



bool clanDestroy( CHAR_DATA* ch, CHAR_DATA* victim, char* argument )
{
  char buf[ MAX_INPUT_LENGTH ] = "";
  DESCRIPTOR_DATA* d = NULL;
  CHAR_DATA* receiver = NULL;
  Clan* targetClan = NULL;

  if( ch == NULL )
    return FALSE;

  if( IS_NULLSTR( argument ) )
    {
      send_to_char( "Which clan would you like to destroy?\n\r", ch );
      return FALSE;
    }

  targetClan = getClanByName( argument );
  if( targetClan == NULL )
    {
      send_to_char( "No clan by that name exists.\n\r", ch );
      return FALSE;
    }

  /* remove target clan from active players */
  for( d = descriptor_list; d != NULL; d = d->next )
    {
      receiver = d->original ? d->original : d->character;

      if( receiver && receiver->pcdata->pcClan == targetClan )
	{
	  ch->pcdata->pcClan = NULL;

	  if( d->connected == CON_PLAYING && receiver != ch )
	    {
	      send_to_char( "Your clan has been dissolved!\n\r", receiver );
	      receiver->pcdata->pcClan = NULL;
	    }
	}
    }

  if( ch->pcdata->pcClan == targetClan )
    ch->pcdata->pcClan = NULL;

  sprintf( buf, "You have destroyed clan %s.\n\r", targetClan->name );
  send_to_char( buf, ch );
  eraseClan( targetClan );
 
  return TRUE;
}
		  


bool clanListing( CHAR_DATA* ch, CHAR_DATA* victim, char* argument )
{
  char formattedList[ MAX_STRING_LENGTH ] = "";
  char clanListPtrEstStr[ MAX_INPUT_LENGTH ] = "";
  char* formattedListPtr = formattedList;
  Clan* clanListPtr = clanList;
  int clanCount = 0;
  struct tm* clanListPtrEstablished = NULL;

  /* name-20 */
  /* leader-20 */
  /* established */
  /* symbol-10 */

  formattedListPtr += sprintf( formattedListPtr, "%-15s %-15s %-26s %-20s\n\r", "Clan Name", "Leader", "Established", "Symbol" );
  formattedListPtr += sprintf( formattedListPtr, "--------------------------------------------------------------------------------\n\r" );

  for( clanListPtr = clanList; clanListPtr != NULL; clanListPtr = clanListPtr->nextClan )
    {
      clanCount++;
      
      clanListPtrEstablished = localtime( &( clanListPtr->established ) );
      strftime( clanListPtrEstStr, MAX_INPUT_LENGTH, "%a %b %H:%M:%S %Y", clanListPtrEstablished );

      formattedListPtr += sprintf( formattedListPtr, "%-15s %-15s %-26s %-20s\n\r",
				   clanListPtr->name,
				   clanListPtr->leader,
				   clanListPtrEstStr,
				   clanListPtr->symbol );

    }
  
  page_to_char( formattedList, ch );
  
  printf_to_char( ch, "\n\rTotal clans: %d\n\r", clanCount );

  return TRUE;
}



bool clanGrant( CHAR_DATA* ch, CHAR_DATA* victim, char* argument )
{
  return FALSE;
}



bool clanRevoke( CHAR_DATA* ch, CHAR_DATA* victim, char* argument )
{
  return FALSE;
}

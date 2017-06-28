#include <stdio.h>
#include <string.h>

#include "merc.h"
#include "coalition.h"

void do_cln_talk( CHAR_DATA* ch, char* argument )
{
  char buf[ MAX_STRING_LENGTH ];
  DESCRIPTOR_DATA* d;
  CHAR_DATA* receiver;

  if( IS_NULLSTR( ch->cln_name ) ) 
    {
      send_to_char( "Huh?\n\r", ch );
      return;
    }

  sprintf( buf, "{B[%s]{x You: {D'%s'{x\n\r", capitalize( ch->cln_name ), argument );
  send_to_char( buf, ch );

  for( d = descriptor_list; d != NULL; d = d->next )
    {
      receiver = d->original ? d->original : d->character;

      if( d->connected == CON_PLAYING && receiver != ch && !str_cmp( ch->cln_name, receiver->cln_name ) )
	{
	  /* this is a hack, but there's no other convenient place to reset cln->leader_name */
	  /* on the clan members if it should change...so we check if the person chatting is */
	  /* is the leader and if the receiver has the same leader...fix if not */
	  if( !str_cmp( ch->name, ch->cln_leader ) )
	    {
	      if( str_cmp( ch->cln_leader, receiver->cln_leader ) )
		{
		  free_string( receiver->cln_leader );
		  receiver->cln_leader = str_dup( ch->cln_leader );
		}
	    }

	  sprintf( buf, "{B[%s]{x %s {D'%s'{x\n\r", capitalize( ch->cln_name ), PERS2( ch, receiver ), argument );
	  send_to_char( buf, receiver );
	}
    }
}


/* NOTE */
/* What happens when a clan leader is removed from the clan?  */
/* As of now, the clan will continue to function. Old members */
/* can keep working as a clan but cannot accept new members.  */
/* The Coalition imm can reestablish clan leadership by using */
/* "clan establish" again on a new leader. As long as the     */
/* name matches, the new clan leader will take over and can   */
/* accept new members. */

void do_clan( CHAR_DATA* ch, char* argument )
{
  char arg1[ MAX_INPUT_LENGTH ];
  ClanCommandHandler action = NULL;

  smash_tilde( argument );
  argument = one_argument( argument, arg1 );

  if( ch->guild != guild_lookup( "coalition" ) )
    {
      send_to_char( "Huh?\n\r", ch );
      return;
    }

  if( *arg1 == '\0' )
    {
      send_to_char( "What clan function would you like to perform?\n\r", ch );
      listClanFunctions( ch );
      return;
    }

  action = clanDispatch( arg1 );
  if( action == NULL )
    {
      send_to_char( "There is no such clan function.\n\r", ch );
      listClanFunctions( ch );
    }
  else
    {
      (*action)( ch, argument );
    }
}

void listClanFunctions( CHAR_DATA* ch )
{
  if( ch != NULL )
    {
      send_to_char( "You can:\n\r", ch );
      if( !IS_NULLSTR( ch->cln_name ) && !str_cmp( ch->name, ch->cln_leader ) )
	{
	  send_to_char( "clan accept\n\r", ch );
	}

      send_to_char( "clan apply\n\r", ch );

      if( IS_IMMORTAL( ch ) )
	{
	  send_to_char( "clan establish\n\r", ch );
	  send_to_char( "clan transfer\n\r", ch );
	}
    }
}

ClanCommandHandler clanDispatch( const char* cmd )
{
  int i;
  ClanCommand cmdList[] =
    {
      { "apply", clanApply },
      { "accept", clanAccept },
      { "establish", clanEstablish },
      { "transfer", clanTransfer },
      { NULL, NULL }
    };

  for( i = 0; cmdList[ i ].cchFunction != NULL; i++ )
    {
      if( !str_prefix( cmd, cmdList[ i ].name ) )
	{
	  return cmdList[ i ].cchFunction;
	}
    }

  return NULL;

}

void clanEstablish( CHAR_DATA* ch, char* argument )
{
  char newClanLeaderName[ MAX_INPUT_LENGTH ];
  char newClanName[ MAX_INPUT_LENGTH ];
  char newClanSymbol[ MAX_INPUT_LENGTH ];
  char buf[ MAX_INPUT_LENGTH ];	
  CHAR_DATA* newClanLeader = NULL;

  if( ch != NULL )
    {
      if( !IS_IMMORTAL( ch ) )
	{
	  send_to_char( "You can't do that.\n\r", ch );
	  return;
	}

      argument = one_argument( argument, newClanLeaderName );
      argument = one_argument( argument, newClanName );
      strcpy( newClanSymbol, argument );

      if( *newClanLeaderName == '\0' || *newClanName == '\0' || *newClanSymbol == '\0' )
	{
	  send_to_char( "Syntax:\n\r", ch );
	  send_to_char( "clan establish <char name> <clan name> <clan symbol>\n\r", ch );
	  return;
	}

      newClanLeader = get_char_world( ch, newClanLeaderName, FALSE );
      if( newClanLeader == NULL )
	{
	  send_to_char( "That person isn't here.\n\r", ch );
	  return;
	}

      if( newClanLeader->guild != guild_lookup( "coalition" ) )
	{
	  send_to_char( "That person isn't a member of the Coalition.\n\r", ch );
	  return;
	}

      if( !IS_NULLSTR( newClanLeader->cln_name ) )
	{
	  sprintf( buf, "The current clan name is %s\n\r", newClanLeader->cln_name );
	  send_to_char( buf, ch );
	  sprintf( buf, "WARNING: Removing %s from clan %s\n\r", newClanLeader->name, newClanLeader->cln_name );
	  send_to_char( buf, ch );
	}

      newClanLeader->cln_leader = str_dup( newClanLeader->name );
      newClanLeader->cln_name = str_dup( newClanName );
      newClanLeader->cln_symbol = str_dup( newClanSymbol );

      sprintf( buf, "You have established the Coalition clan %s with %s as its leader and %s as its symbol.\n\r", newClanLeader->cln_name, newClanLeader->cln_leader, newClanLeader->cln_symbol );
      send_to_char( buf, ch );

      sprintf( buf, "You are now the leader of clan %s, with %s as its symbol.\n\r", newClanLeader->cln_name, newClanLeader->cln_symbol );
      send_to_char( buf, newClanLeader );

    }
}

void clanApply( CHAR_DATA* ch, char* argument )
{
  char buf[ MAX_INPUT_LENGTH ];
  CHAR_DATA* clanLeader = NULL;

  if( ch == NULL )
    return;

  if( argument == NULL || *argument == '\0' )
    {
      send_to_char( "Syntax:\n\r", ch );
      send_to_char( "clan apply <leader's name>\n\r", ch );
      return;
    }

  clanLeader = get_char_world( ch, argument, FALSE );
  if( clanLeader == NULL )
    {
      send_to_char( "That person isn't here.\n\r", ch );
      return;
    }

  if( clanLeader->guild != guild_lookup( "coalition" ) )
    {
      send_to_char( "That person isn't a member of the Coalition.\n\r", ch );
      return;
    }

  if( IS_NULLSTR( clanLeader->cln_name ) )
    {
      send_to_char( "That person isn't leading a clan.\n\r", ch );
      return;
    }

  if( str_cmp( clanLeader->name, clanLeader->cln_leader ) )
    {
      sprintf( buf, "%s is not the leader of clan %s.\n\r", clanLeader->name, capitalize( clanLeader->cln_name ) );
      send_to_char( buf, ch );
      return;
    }

  ch->cln_apply = str_dup( clanLeader->name );

  sprintf( buf, "You have applied to %s for membership in clan %s.\n\r", clanLeader->name, capitalize( clanLeader->cln_name ) );
  send_to_char( buf, ch );

  sprintf( buf, "%s has applied for membership in your clan.\n\r", ch->name );
  send_to_char( buf, clanLeader );
}

void clanAccept( CHAR_DATA* ch, char* argument )
{
  char buf[ MAX_INPUT_LENGTH ];
  CHAR_DATA* newMember = NULL;

  if( ch == NULL )
    return;

  if( argument == NULL || *argument == '\0' )
    {
      send_to_char( "Syntax:\n\r", ch );
      send_to_char( "clan accept <applicant's name>\n\r", ch );
      return;
    }

  if( str_cmp( ch->name, ch->cln_leader ) )
    {
      send_to_char( "Only a clan leader can accept new members into the clan.\n\r", ch );
      return;
    }

  newMember = get_char_world( ch, argument, FALSE );
  if( newMember == NULL )
    {
      send_to_char( "That person isn't here.\n\r", ch );
      return;
    }

  if( newMember->guild != guild_lookup( "coalition" ) )
    {
      send_to_char( "That person isn't a member of the Coalition.\n\r", ch );
      return;
    }

  if( str_cmp( ch->name, newMember->cln_apply ) )
    {
      send_to_char( "That person doesn't seem to be applying to your clan.\n\r", ch );
      return;
    }

  free_string( newMember->cln_apply );
  newMember->cln_apply = &str_empty[0];
  newMember->cln_name = str_dup( ch->cln_name );
  newMember->cln_leader = str_dup( ch->name );
  newMember->cln_symbol = str_dup( ch->cln_symbol );

  sprintf( buf, "You have accepted %s into your clan.\n\r", newMember->name );
  send_to_char( buf, ch );

  sprintf( buf, "%s has accepted you into clan %s.\n\r", newMember->cln_leader, capitalize( newMember->cln_name ) );
  send_to_char( buf, newMember );
}

void clanTransfer( CHAR_DATA* ch, char* argument )
{
  char buf[ MAX_INPUT_LENGTH ];
  char newLeaderName[ MAX_INPUT_LENGTH ];
  char newMemberName[ MAX_INPUT_LENGTH ];
  CHAR_DATA* newLeader = NULL;
  CHAR_DATA* newMember = NULL;
  int coalitionNumber = guild_lookup( "coalition" );

  if( ch == NULL )
    return;

  if( !IS_IMMORTAL( ch ) )
    {
      send_to_char( "You can't do that.\n\r", ch );
      return;
    }

  argument = one_argument( argument, newMemberName );
  strcpy( newLeaderName, argument );

  if( *newMemberName == '\0' )
    {
      send_to_char( "Syntax:\n\r", ch );
      send_to_char( "clan transfer <member> <new leader's name> (to transfer)\n\r", ch );
      send_to_char( "clan transfer <member> (to remove)\n\r", ch );
      return;
    }

  newMember = get_char_world( ch, newMemberName, FALSE );
  if( newMember == NULL )
    {
      send_to_char( "The person you want to transfer isn't here.\n\r", ch );
      return;
    }

  if( newMember->guild != coalitionNumber )
    {
      send_to_char( "The person must be a member of the Coalition.\n\r", ch );
      return;
    }

  if( IS_NULLSTR( newMember->cln_name ) )
    {
      send_to_char( "The person isn't a member of a clan.\n\r", ch );
      return;
    }

  /* remove newMember from the clan */
  if( *newLeaderName == '\0' )
    {
      sprintf( buf, "You have removed %s from clan %s.\n\r", newMember->name, newMember->cln_name );
      send_to_char( buf, ch );
      
      free_string( newMember->cln_apply );
      free_string( newMember->cln_name );
      free_string( newMember->cln_leader );
      free_string( newMember->cln_symbol );

      newMember->cln_apply = &str_empty[ 0 ];
      newMember->cln_name = &str_empty[ 0 ];
      newMember->cln_leader = &str_empty[ 0 ];
      newMember->cln_symbol = &str_empty[ 0 ];

      send_to_char( "You have been removed from your clan!\n\r", newMember );
    }
  else /* transfer */
    {
      newLeader = get_char_world( ch, newLeaderName, FALSE );
      if( newLeader == NULL )
	{
	  send_to_char( "The clan leader you specified isn't here.\n\r", ch );
	  return;
	}

      if( newLeader->guild != coalitionNumber )
	{
	  send_to_char( "The new leader you specfied isn't in the Coalition.\n\r", ch );
	  return;
	}

      if( str_cmp( newLeader->name, newLeader->cln_leader ) )
	{
	  sprintf( buf, "%s isn't the leader of a clan.\n\r", newLeader->name );
	  return;
	}

      free_string( newMember->cln_apply );
      newMember->cln_apply = &str_empty[ 0 ];

      free_string( newMember->cln_name );
      free_string( newMember->cln_leader );
      free_string( newMember->cln_symbol );

      newMember->cln_name = str_dup( newLeader->cln_name );
      newMember->cln_leader = str_dup( newLeader->name );
      newMember->cln_symbol = str_dup( newLeader->cln_symbol );

      sprintf( buf, "You have transfered %s to clan %s.\n\r", newMember->name, capitalize( newMember->cln_name ) );
      send_to_char( buf, ch );

      sprintf( buf, "You are now a member of clan %s, with %s as your leader.\n\r", newMember->cln_name, newMember->cln_leader );
      send_to_char( buf, newMember );

      sprintf( buf, "%s has been accepted into your clan.\n\r", newMember->name );
      send_to_char( buf, newLeader );
    }
}

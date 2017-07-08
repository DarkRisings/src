#ifndef __CLANS_H
#define __CLANS_H

#include <time.h>
#include "merc.h"

/*******************/
/* Data Structures */
/*******************/
struct clanNode
{
  char* name;
  char* leader;
  char* symbol;
  time_t established;

  struct clanNode* nextClan;
};

typedef void (*ClanSubcommandHandler)( CHAR_DATA* ch, CHAR_DATA* victim, char* argument );

typedef struct
{
  const char* name;
  const char* syntax;
  bool hasVictim;
  bool immOnly;
  bool leaderOnly;
  ClanSubcommandHandler csHandler;
} ClanSubcommand;

/***********/
/* Globals */
/***********/
const static char CLANS_FILE[] = "clans.csv";
extern Clan* clanList;

/*********************/
/* in-game commands  */
/*********************/
void do_clan( CHAR_DATA* ch, char* argument );
void do_clantalk( CHAR_DATA* ch, char* argument );

/**************************************/
/* File operations to load/save clans */
/**************************************/
bool loadClans( void );
bool saveClans( void );
Clan* makeClan( const char* newName, const char* newLeader, const char* newSymbol, time_t newTime );
bool eraseClan( Clan* oldClan );


/********************/
/* Lookup functions */
/********************/
Clan* getClanByName( const char* desiredName );
Clan* getClanBySymbol( const char* desiredSymbol );
Clan* getClanByLeader( const char* name );

/***********************************/
/* Subcommand Lookup and Utilities */
/***********************************/
ClanSubcommand* clanCmdLookup( const char* cmd );
void sendClanCmdList( CHAR_DATA* ch );

/************************/
/* Subcommand Functions */
/************************/
bool clanEstablish( CHAR_DATA* ch, CHAR_DATA* newLeader, char* argument );
bool clanLeader( CHAR_DATA* ch, CHAR_DATA* newLeader, char* argument );
bool clanApply( CHAR_DATA* ch, CHAR_DATA* leader, char* argument );
bool clanAccept( CHAR_DATA* ch, CHAR_DATA* supplicant, char* argument );
bool clanRemove( CHAR_DATA* ch, CHAR_DATA* victim, char* argument );
bool clanDestroy( CHAR_DATA* ch, CHAR_DATA* leader, char* argument );
bool clanTalk( CHAR_DATA* ch, CHAR_DATA* victim, char* argument );
bool clanListing( CHAR_DATA* ch, CHAR_DATA* victim, char* argument );
bool clanGrant( CHAR_DATA* ch, CHAR_DATA* victim, char* argument );
bool clanRevoke( CHAR_DATA* ch, CHAR_DATA* victim, char* argument );

#endif /* __CLANS_H defined */

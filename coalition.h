#ifndef __COALITION_H
#define __COALITION_H

#include "merc.h"
#include "lookup.h"

/* in-game commands */
void do_cln_talk( CHAR_DATA* ch, char* argument );
void do_cclan( CHAR_DATA* ch, char* argument );

/* do_cclan sub-functions and dispatcher */

typedef void (*ClanCommandHandler)( CHAR_DATA* ch, char* argument );
typedef struct
{
	const char* name;
	ClanCommandHandler cchFunction;
} ClanCommand;


ClanCommandHandler clanDispatch( const char* cmd );

void listClanFunctions( CHAR_DATA* ch );
void clanEstablish( CHAR_DATA* actor, char* argument );
void clanApply( CHAR_DATA* actor, char* argument );
void clanAccept( CHAR_DATA* actor, char* argument );
void clanTransfer( CHAR_DATA* actor, char* argument );


#endif /* __COALITION_H defined */

#include "merc.h"

typedef struct pc_guild_member MEMBER;
typedef struct pc_guild_type PCGUILD;

//
//  The guild table is composed of these
//
struct pc_guild_type
{
	char name[MAX_GNAME + 1];	// The name of this guild
	char symbol[MAX_GSYMB + 1];	// The symbol of this guild
	MEMBER *leader;				// The leader of this guild
	int ident;					// guild have a unique identifier, like a vnum
	int levels;					// The total number of levels of this guild
	bool active;				// is guild approved and active 
	MEMBER *members;			// A list of the members
	MEMBER *applicants;			// A list of players applying to this guild
};

//
// Linked list of players, used for membership, application, etc
//
struct pc_guild_member
{
	MEMBER *next;
	char name[MAX_GNAME + 1];
	int levels;			
	char gtitle[MAX_GNAME + 1];
	bool isLeader;
	int rank;
};

typedef	void GuildCommand args((CHAR_DATA *ch, char *argument));

//
// table for holding all the guild commands
//
struct guild_cmd_type
{
	char * const cmd;
	char * const syntax;
	GuildCommand * function;
	int requiredRank;
	bool leaderOnly;
	bool immOnly;
	bool guildOnly;
};

//
// Local functions
//
MEMBER *new_member_elt(void);
MEMBER *append_member(MEMBER * list, MEMBER * elt);
MEMBER *get_member(int guild, char *name);
MEMBER *get_applicant(int guild, char *name);
bool is_in_guild(int guild, char *name);
bool is_applicant(int guild, char *name);
void load_new_guilds(void);
void load_guild(FILE *fp);
void save_guild(int guild);
void save_guilds();
void nullify_applicant(int guild, char *name);
void nullify_member(int guild, char *name);
void get_guild_cmd_list(CHAR_DATA *ch);
void do_guild_expel(CHAR_DATA *ch, char *name);
void do_guild_defect(CHAR_DATA* ch, char* argument);
void do_guild_disband(CHAR_DATA* ch, char* argument);
void do_guild_rank(CHAR_DATA* ch, char* argument);
void do_guild_title(CHAR_DATA* ch, char* argument);
void do_guild_who(CHAR_DATA* ch, char* argument);
void do_guild_list(CHAR_DATA* ch, char* argument);
void do_guild_establish(CHAR_DATA* ch, char* argument);
void do_guild_apply(CHAR_DATA *ch, char *argument);
void do_guild_approve(CHAR_DATA *ch, char *argument);
void do_guild_info(CHAR_DATA* ch, char* argument);
void do_guild_accept(CHAR_DATA *ch, char *argument);
void do_guild_prospects(CHAR_DATA* ch, char* argument);
void do_guild_symbol(CHAR_DATA *ch, char *symbol);
void do_guild_decline(CHAR_DATA *ch, char *name);
void do_guild_deny(CHAR_DATA *ch, char *argument);
void do_reload_guilds(CHAR_DATA* ch, char* argument);
void do_guild_leader(CHAR_DATA* ch, char* argument)
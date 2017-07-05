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

//
// Local functions
//
void load_new_guilds(void);
void load_guild(FILE *fp);
MEMBER *new_member_elt(void);
MEMBER *append_member(MEMBER * list, MEMBER * elt);
void do_guild_who(CHAR_DATA* ch, char* argument);
void do_guild_list(CHAR_DATA* ch);
void do_guild_establish(CHAR_DATA* ch, char* name, char* symbol);
bool is_in_guild(int guild, char *name);
void save_guild(int guild);
void save_guilds();
MEMBER *get_member(int guild, char *name);
void do_guild_apply(CHAR_DATA *ch, char *argument);
void do_guild_approve(CHAR_DATA *ch, char *argument);
void do_guild_info(CHAR_DATA *ch);
void do_guild_accept(CHAR_DATA *ch, char *argument);
void nullify_applicant(int guild, char *name);
void nullify_member(int guild, char *name);
void do_guild_expel(CHAR_DATA *ch, char *argument);
void do_guild_defect(CHAR_DATA *ch);
bool is_applicant(int guild, char *name);
void do_guild_disband(CHAR_DATA *ch);
bool can_perform_action(MEMBER *m, char *action);
void do_guild_rank(CHAR_DATA *ch, char *argument);

//
// External declarations
//
char *center_text(char *txtstr, int txtnum);
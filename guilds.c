#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "db.h"
#include "guilds.h"
#include "lookup.h"
#include "interp.h"

const struct pc_guild_type EmptyGuild[MAX_PC_GUILD];
struct pc_guild_type pc_guild_table[MAX_PC_GUILD];
int guild_number = 0;
extern char *center_text args((char *txtstr, int txtnum));

// Command, Function, Required Rank, Leader Only, Imm Only, Guild Only

const struct guild_cmd_type cmdTbl[] =
{
	{ "list", "guild list", do_guild_list, 0, FALSE, FALSE, FALSE},
	{ "establish", "guild establish <NAME> <SYMBOL> | IE: guild establish arcaenum {{C[Arcaenum]{{x", do_guild_establish, 0, FALSE, FALSE, FALSE },
	{ "apply", "guild apply <GUILD NAME>", do_guild_apply, 0, FALSE, FALSE, FALSE },
	{ "approve", "guild approve <GUILD NAME>", do_guild_approve, 0, FALSE, TRUE, FALSE },
	{ "info", "guild info <GUILD NAME>", do_guild_info, 0, FALSE, FALSE, FALSE },
	{ "who", "guild who <GUILD NAME>", do_guild_who, 0, FALSE, FALSE, FALSE },
	{ "accept", "guild accept <APPLICANT NAME>", do_guild_accept, 2, FALSE, FALSE, TRUE },
	{ "defect", "guild defect", do_guild_defect, 1, FALSE, FALSE, TRUE },
	{ "expel", "guild expel <MEMBER NAME>", do_guild_expel, 3, FALSE, FALSE, TRUE },
	{ "disband", "guild disband", do_guild_disband, 0, TRUE, FALSE, TRUE },
	{ "title", "guild title <MEMBER NAME> <TITLE STRING>", do_guild_title, 0, TRUE, FALSE, TRUE },
	{ "rank", "guild rank <MEMBER NAME> <RANK> | Rank being one of: 1, 2, 3 or leader", do_guild_rank, 0, TRUE, FALSE, TRUE },
	{ "prospects", "guild prospects", do_guild_prospects, 2, FALSE, FALSE, TRUE },
	{ "symbol", "guild symbol <NEW SYMBOL> | Can only be done BEFORE approval", do_guild_symbol, 0, TRUE, FALSE, TRUE },
	{ "decline", "guild decline <APPLICANT>", do_guild_decline, 2, FALSE, FALSE, TRUE },
	{ "leader", "guild leader <GUILD NAME> <NEW LEADER>", do_guild_leader, 0, FALSE, TRUE, FALSE },
	{ "deny", "guild deny <GUILD NAME>", do_guild_deny, 0, FALSE, TRUE, FALSE },
	{ "reload", "guild reload", do_reload_guilds, 0, FALSE, TRUE, FALSE },
	{ NULL, NULL, NULL, 0, FALSE, FALSE, FALSE },
};

void do_new_guild(CHAR_DATA* ch, char* argument)
{
	bool found = FALSE;
	int cmd = 0;
	char arg1[MAX_INPUT_LENGTH] = "";
	argument = one_argument(argument, arg1);
	MEMBER *m;
	m = get_member(ch->guild, ch->name);
	
	if (IS_NPC(ch)) {
		send_to_char("NPCs can't perform clan business.\n\r", ch);
		return;
	}

	if (IS_NULLSTR(arg1)) {
		send_to_char("What guild function would you like to perform?\n\r", ch);
		get_guild_cmd_list(ch);
		return;
	}

	while (cmdTbl[cmd].cmd != NULL)
	{
		if (arg1[0] == cmdTbl[cmd].cmd[0] && !str_prefix(arg1, cmdTbl[cmd].cmd)) {
			found = TRUE;
			break;
		}

		cmd++;
	}

	if (!found) {
		send_to_char("Invalid command.\n\r", ch);
		return;
	}

	if (cmdTbl[cmd].guildOnly) {
		if (!is_guild(ch)) {
			send_to_char("You're not in a guild.\n\r", ch);
			return;
		}

		if (cmdTbl[cmd].leaderOnly && !m->isLeader) {
			send_to_char("You're not the leader of your guild.\n\r", ch);
			return;
		}

		if (cmdTbl[cmd].requiredRank > m->rank) {
			send_to_char("You are not high enough rank to perform that action.\n\r", ch);
			return;
		}
	}

	if (cmdTbl[cmd].immOnly && !IS_IMMORTAL(ch)) {
		send_to_char("Invalid command.\n\r", ch);
		get_guild_cmd_list(ch);
		return;
	}

	/*
	* Dispatch the command.
	*/
	(*cmdTbl[cmd].function)(ch, argument);
	tail_chain();
}


void get_guild_cmd_list(CHAR_DATA *ch)
{
	int cmd = 0;
	bool hasCmd;
	char buf[MAX_INPUT_LENGTH] = "";
	MEMBER *m;
	m = get_member(ch->guild, ch->name);

	while (cmdTbl[cmd].cmd != NULL) {
		hasCmd = FALSE;

		if (cmdTbl[cmd].guildOnly) {
			if (is_guild(ch)) {
				hasCmd = TRUE;

				if (cmdTbl[cmd].leaderOnly && !m->isLeader) {
					hasCmd = FALSE;
				}

				if (cmdTbl[cmd].requiredRank > m->rank) {
					hasCmd = FALSE;
				}
			}
		} else {
			if (!cmdTbl[cmd].immOnly) {
				hasCmd = TRUE;
			}
		}


		if (IS_IMMORTAL(ch)) {
			hasCmd = TRUE;
		}

		if (hasCmd) {
			sprintf(buf, "%s\n\r", cmdTbl[cmd].syntax);
			send_to_char(buf, ch);
		}

		cmd++;
	}
}

void load_new_guilds()
{
	FILE *fp;
	FILE *gf;
	char buf[MAX_STRING_LENGTH];
	char* tempLine = NULL;
	size_t lineLength = 0;
	char name[MAX_STRING_LENGTH];

	sprintf(buf, "%s/%s", GUILD_DIR, GUILDINDEX);
	fp = fopen(buf, "r");

	if (fp == NULL) {
		bug("Guild index not found", 0);
		exit(1);
	}

	while (!feof(fp)) {
		if (getline(&tempLine, &lineLength, fp) > 0) {
			sscanf(tempLine, "%s[^\n]", name);
			sprintf(buf, "%s/%s.gld", GUILD_DIR, name);

			gf = fopen(buf, "r");

			if (gf == NULL) {
				bug("Guild not found", 0);
			} else {
				load_guild(gf);
				fclose(gf);
			}

			free(tempLine);
			tempLine = NULL;
		}
	}

	fclose(fp);
}


void save_guilds()
{
	int i;
	for (i = 0; i < guild_number + 1; i++) {
		save_guild(i);
	}
}

void save_guild(int guild)
{
	FILE *fp;
	MEMBER *m;
	MEMBER *a;
	char name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

	// reliable and easy way to lowercase a string
	one_argument(pc_guild_table[guild].name, name);
	sprintf(buf, "%s/%s.gld", GUILD_DIR, name);
	fp = fopen(buf, "w");

	if (fp != NULL) {
		fprintf(fp, "Iden %d\n", pc_guild_table[guild].ident);
		fprintf(fp, "Acti %d\n", pc_guild_table[guild].active);
		fprintf(fp, "Name %s\n", pc_guild_table[guild].name);
		fprintf(fp, "Symb %s\n", pc_guild_table[guild].symbol);
		fprintf(fp, "Levl %d\n", pc_guild_table[guild].levels);

		m = pc_guild_table[guild].members;
		while (m != NULL) {
			if (m->name[0] != '\0') {
				fprintf(fp, "Memb %s %d %d %d %s\n", m->name, m->levels, m->isLeader ? 1 : 0, m->rank, m->gtitle);
			}
			m = m->next;
		}

		a = pc_guild_table[guild].applicants;
		while (a != NULL) {
			if (a->name[0] != '\0') {
				fprintf(fp, "Appl %s %d %d %d %s\n", a->name, a->levels, 0, 1, "Applicant");
			}
			a = a->next;
		}

		fprintf(fp, "End ");
		fclose(fp);
	} else {
		log_string("Null file");
	}
}



void load_guild(FILE *fp)
{
	char *word;
	int id;
	int isLeader;
	bool foundLeader = FALSE;
	char buf[MAX_INPUT_LENGTH] = "";
	int levels = 0;

	MEMBER *m = NULL;

	for (; ; ) {
		word = feof(fp) ? "End" : fread_word(fp);
		switch (UPPER(word[0])) {
			case 'I':
				id = fread_number(fp);
				if (id == 0) {
					return;
				}
				if (guild_number < id) {
					guild_number = id;
				}
				pc_guild_table[id].ident = id;
				pc_guild_table[id].members = NULL;
				pc_guild_table[id].applicants = NULL;
			break;
			case 'N':
				strcpy(pc_guild_table[id].name, fread_word(fp));
				log_string(pc_guild_table[id].name);
			break;
			case 'S':
				// Don't wanna fuck with the who function too much
				// I need a space after the clan symbol ala <symbol> Name
				sprintf(buf, "%s ", fread_string_eol(fp));
				strcpy(pc_guild_table[id].symbol, buf);
			break;
			case 'M':
				// Red in a new member
				m = new_member_elt();
				strcpy(m->name, fread_word(fp));
				m->levels = fread_number(fp);
				isLeader = fread_number(fp);
				m->rank = fread_number(fp);
				strcpy(m->gtitle, fread_string_eol(fp));
				if (isLeader == 1) {
					m->isLeader = TRUE;
					foundLeader = TRUE;
					pc_guild_table[id].leader = m;
				}
				sprintf(buf, "%d", levels);
				levels = levels + m->levels;
				sprintf(buf, "%d", levels);
				pc_guild_table[id].members = append_member(pc_guild_table[id].members, m);
			break;
			case 'A':
				if (!str_cmp(word, "acti")) {
					int active;
					active = fread_number(fp);
					if (active == 1) {
						pc_guild_table[id].active = TRUE;
					}
				}

				if (!str_cmp(word, "appl")) {
					m = new_member_elt();
					strcpy(m->name, fread_word(fp));
					m->levels = fread_number(fp);
					isLeader = fread_number(fp);
					m->isLeader = FALSE;
					m->rank = fread_number(fp);
					strcpy(m->gtitle, fread_word(fp));
					pc_guild_table[id].applicants = append_member(pc_guild_table[id].applicants, m);
				}
			break;
			case 'E':
				// Finish the routine
				if (!str_cmp(word, "End")) {
					if (!foundLeader) {
						// Someone hosed the leader. This can either mean that
						// a player was deleted or the clan is disbanded. So
						// first check if there are any members...

						if (pc_guild_table[id].members != NULL) {
							// There are members, so first guy is it now. Gratz guy.
							m = new_member_elt();
							strcpy(m->name, pc_guild_table[id].members->name);
							strcpy(m->gtitle, pc_guild_table[id].members->gtitle);
							m->levels = pc_guild_table[id].members->levels;
							m->isLeader = TRUE;
							m->next = NULL;
							m->rank = 3;
							pc_guild_table[id].leader = m;
						} else {
							// If by this point there are NO members, this guild is disbanded
							// So just to be safe let's set active to 0 to make sure it
							// won't show up in the guild list
							pc_guild_table[id].active = FALSE;
						}
					}

					pc_guild_table[id].levels = levels;
					return;
				}
			break;
		}
	}
}

void reload_guild(int guild)
{
	FILE *gf;
	char buf[MAX_STRING_LENGTH];
	char fileBuf[MAX_STRING_LENGTH];
	char fileName[MAX_STRING_LENGTH];

	strcpy(fileBuf, pc_guild_table[guild].name);
	one_argument(fileBuf, fileName);

	sprintf(buf, "%s/%s.gld", GUILD_DIR, fileName);
	gf = fopen(buf, "r");

	if (gf == NULL) {
		bug("Guild not found", 0);
		exit(1);
	} else {
		load_guild(gf);
	}

}


void init_guild_table()
{
	pc_guild_table[GUILD_BOGUS].ident = GUILD_BOGUS;
	pc_guild_table[GUILD_BOGUS].levels = GUILD_BOGUS;
	pc_guild_table[GUILD_BOGUS].members = NULL;
	pc_guild_table[GUILD_BOGUS].applicants = NULL;
	pc_guild_table[GUILD_BOGUS].leader = NULL;
	strcpy(pc_guild_table[GUILD_BOGUS].name, GUILD_BOGUS_NAME);
	strcpy(pc_guild_table[GUILD_BOGUS].symbol, GUILD_BOGUS_SYMB);
}


void do_reload_guilds(CHAR_DATA* ch, char* argument) {
	int i;

	for (i = 0; i <= guild_number; i++) {
		pc_guild_table[i] = EmptyGuild[0];
	}

	guild_number = 0;
	load_new_guilds();
}

void do_guild_establish(CHAR_DATA* ch, char* argument)
{
	int id;
	int i;
	MEMBER* m;
	char symbol[MAX_INPUT_LENGTH] = "";
	char name[MAX_INPUT_LENGTH] = "";
	char buf[MAX_INPUT_LENGTH] = "";
	FILE *guildIndex;
	FILE *player;
	CHAR_DATA *gname;
	argument = one_argument(argument, name);
	strcpy(symbol, argument);

	if (IS_NULLSTR(name) || IS_NULLSTR(symbol)) {
		send_to_char("Syntax: guild establish <name> <symbol>\n\r", ch);
		return;
	}

	// Make sure this player isn't in a guild. 
	if (is_guild(ch)) {
		send_to_char("You're already in a guild.\n\r", ch);
		return;
	}

	// First we have to make sure that this name hasn't already
	// been taken. This covers inactive guilds as well.
	for (i = 0; i < guild_number + 1; i++) {
		if (!IS_NULLSTR(pc_guild_table[i].name) && !str_cmp(name, pc_guild_table[i].name)) {
			send_to_char("That guild name has already been taken. \n\r"
						 "It may be inactive and you may be able to claim it. Please see an imm.\n\r", ch);
			return;
		}

		if (!IS_NULLSTR(pc_guild_table[i].symbol) && !str_cmp(symbol, pc_guild_table[i].symbol)) {
			send_to_char("Please choose a unique symbol.\n\r", ch);
			return;
		}
	}

	// Make sure it's not a player name.
	if ((gname = get_char_world(ch, name, TRUE)) != NULL) {
		if (!IS_NPC(gname)) {
			send_to_char("Someone with that name is actively playing.\n\r", ch);
			return;
		}
	} else {
		sprintf(buf, "%s%s", PLAYER_DIR, capitalize(name));
		if ((player = fopen(buf, "r")) != NULL)
		{
			send_to_char("Invalid guild name.\n\r", ch);
			fclose(player);
			return;
		}
	}

	// Assign the guild an ID, and increase the
	// global guild number.
	id = guild_number + 1;
	guild_number = id;

	// Assign guild to char
	ch->guild = id;

	// Format symbol
	sprintf(buf, "%s ", symbol);
	strcpy(pc_guild_table[id].symbol, buf);

	// Set name
	strcpy(pc_guild_table[id].name, name);

	// Set identity
	pc_guild_table[id].ident = id;
	
	// Initialize applicants list
	pc_guild_table[id].applicants = NULL;

	// Set active state
	pc_guild_table[id].active = FALSE;

	// Set current levels
	pc_guild_table[id].levels = ch->level;

	// Initialize members list with leader as
	// first member
	m = new_member_elt();
	m->isLeader = TRUE;
	m->levels = ch->level;
	m->rank = 3;
	strcpy(m->name, ch->name);
	strcpy(m->gtitle, "Leader");
	pc_guild_table[id].members = m;
	pc_guild_table[id].leader = m;

	// Open guild index and write this guild to it
	sprintf(buf, "%s/%s", GUILD_DIR, GUILDINDEX);
	guildIndex = fopen(buf, "a");
	sprintf(buf, "\n%s", name);
	fprintf(guildIndex, buf);
	fclose(guildIndex);

	// Create the guild file
	sprintf(buf, "%s/%s.gld", GUILD_DIR, name);
	FILE *fp = fopen(buf, "ab+");
	fclose(fp);

	// Write to file
	save_guild(id);

	// Flavor text
	sprintf(buf, "Guild %s has been proposed with you as the leader.\n\r"
		"Your guild will be inactive until an immortal approves it.\n\r", symbol);
	send_to_char(buf, ch);
}

void do_guild_list(CHAR_DATA* ch, char* argument)
{
	int i;
	char buf[MAX_INPUT_LENGTH] = "";
	
	send_to_char("{D:{B================================================================={D:\n\r", ch);
	send_to_char(" Level     Name                Leader              Symbol         \n\r", ch);
	send_to_char("{D:{B-----------------------------------------------------------------{D:{x\n\r", ch);
	for (i = 0; i < guild_number + 1; i++) {
		if (!IS_NULLSTR(pc_guild_table[i].name)) {
			if (pc_guild_table[i].active == TRUE) {
				sprintf(buf, " %-10d%-20s%-20s%-30s\n\r", 
					pc_guild_table[i].levels,
					capitalize(pc_guild_table[i].name), 
					pc_guild_table[i].leader->name,
					pc_guild_table[i].symbol
				);

				send_to_char(buf, ch);
			}
		}
	}
	if (IS_IMMORTAL(ch)) {
		send_to_char("{D:{B-----------------------------------------------------------------{D:\n\r", ch);
		send_to_char(" INACTIVE GUILDS\n\r", ch);
		send_to_char("{D:{B-----------------------------------------------------------------{D:{x\n\r", ch);
		for (i = 0; i < guild_number + 1; i++) {
			if (!IS_NULLSTR(pc_guild_table[i].name)) {
				if (pc_guild_table[i].active == FALSE) {
					sprintf(buf, " %-10d%-20s%-20s%-30s\n\r",
						pc_guild_table[i].levels,
						capitalize(pc_guild_table[i].name),
						"NONE",
						pc_guild_table[i].symbol
					);
					send_to_char(buf, ch);
				}
			}
		}
	}

	send_to_char("{D:{B-----------------------------------------------------------------{D:{x\n\r", ch);
}

void do_guild_who(CHAR_DATA* ch, char* argument)
{
	char arg[MAX_INPUT_LENGTH] = "";
	char buf[MAX_INPUT_LENGTH] = "";
	char rankBuf[MAX_INPUT_LENGTH] = "";
	int guild;
	MEMBER *m = NULL;
	smash_tilde(argument);
	argument = one_argument(argument, arg);
	guild = guild_lookup(arg);

	if (IS_NULLSTR(arg)) {
		send_to_char("You must supply a guild name.\n\r", ch);
		return;
	}

	if (guild == GUILD_BOGUS) {
		sprintf(buf, "Guild %s not found.", arg);
		send_to_char(buf, ch);
		return;
	}

	
	m = pc_guild_table[guild].members;
	while (m != NULL) {

		if (m->isLeader) {
			sprintf(rankBuf, "(%s)", "Leader");
		} else {
			sprintf(rankBuf, "(Rank %d)", m->rank);
		}

		sprintf(buf, "[%s] %s %s\n\r", m->gtitle, m->name, rankBuf);
		send_to_char(buf, ch);
		m = m->next;
	}
}

void do_guild_leader(CHAR_DATA* ch, char* argument)
{
	char gname[MAX_INPUT_LENGTH] = "";
	char buf[MAX_INPUT_LENGTH] = "";
	char name[MAX_INPUT_LENGTH] = "";
	int guild;
	MEMBER *m = NULL;
	CHAR_DATA *leader;
	smash_tilde(argument);
	argument = one_argument(argument, gname);
	argument = one_argument(argument, name);
	guild = guild_lookup(gname);
	DESCRIPTOR_DATA *d;

	if (IS_NULLSTR(gname)) {
		send_to_char("You must supply a guild name.\n\r", ch);
		return;
	}

	if (IS_NULLSTR(gname)) {
		send_to_char("You must supply a player name, who is playing.\n\r", ch);
		return;
	}

	if (guild == GUILD_BOGUS) {
		sprintf(buf, "Guild %s not found.", gname);
		send_to_char(buf, ch);
		return;
	}

	if ((leader = get_char_world(ch, name, TRUE)) == NULL) {
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC(leader)) {
		send_to_char("Not on NPCs.\n\r", ch);
		return;
	}

	if (is_guild(leader) && leader->guild != guild) {
		send_to_char("They're in another guild.\n\r", ch);
		return;
	}

	m = pc_guild_table[guild].members;
	while (m != NULL) {
		if (m->isLeader) {
			m->isLeader = FALSE;
			m->rank = 3;
			break;
		}
	}

	if (is_in_guild(guild, leader->name)) {
		m = get_member(guild, leader->name);
		m->isLeader = TRUE;
	} else {
		leader->guild = guild;
		m = new_member_elt();
		m->isLeader = TRUE;
		m->levels = ch->level;
		m->rank = 3;
		strcpy(m->name, leader->name);
		strcpy(m->gtitle, "Leader");
		pc_guild_table[guild].members = append_member(pc_guild_table[guild].members, m);
	}

	for (d = descriptor_list; d; d = d->next)
	{
		if (d->connected == CON_PLAYING)
		{
			sprintf(buf, "{G%s {g has taken over leadership of {G%s{g.{x\n\r",
				leader->name,
				capitalize(pc_guild_table[guild].name)
			);
			send_to_char(buf, d->character);
		}
	}

	save_guild(guild);
	reload_guild(guild);
	
}

void do_guild_prospects(CHAR_DATA* ch, char* argument)
{
	char buf[MAX_INPUT_LENGTH] = "";
	MEMBER *applicant = NULL;

	send_to_char("Applying to your guild: \n\r", ch);
	applicant = pc_guild_table[ch->guild].applicants;
	while (applicant != NULL) {
		sprintf(buf, "%s\n\r", applicant->name);
		send_to_char(buf, ch);
		applicant = applicant->next;
	}
}

void do_guild_approve(CHAR_DATA *ch, char *argument) 
{
	int guild;
	CHAR_DATA *leader;
	char arg[MAX_INPUT_LENGTH] = "";
	char buf[MAX_INPUT_LENGTH] = "";
	smash_tilde(argument);
	argument = one_argument(argument, arg);
	guild = guild_lookup(arg);
	DESCRIPTOR_DATA *d;

	if (IS_NULLSTR(arg)) {
		send_to_char("You must supply a guild name.\n\r", ch);
		return;
	}

	if (guild == GUILD_BOGUS) {
		sprintf(buf, "Guild %s not found.", capitalize(arg));
		send_to_char(buf, ch);
		return;
	}

	pc_guild_table[guild].active = TRUE;

	sprintf(buf, "Guild %s approved.\n\r", capitalize(arg));
	send_to_char(buf, ch);

	if ((leader = get_char_world(ch, pc_guild_table[guild].leader->name, TRUE)) != NULL) {
		send_to_char("Your guild has been approved and is now active.\n\r", leader);
	}

	for (d = descriptor_list; d; d = d->next)
	{
		if (d->connected == CON_PLAYING)
		{
			sprintf(buf, "{gGuild {G%s {ghas been established, led by {G%s{g.{x\n\r", 
				capitalize(pc_guild_table[guild].name), 
				pc_guild_table[guild].leader->name);
			send_to_char(buf, d->character);
		}
	}

	save_guild(guild);
	return;
}

void do_guild_deny(CHAR_DATA *ch, char *argument)
{
	int guild;
	CHAR_DATA *leader;
	char arg[MAX_INPUT_LENGTH] = "";
	char buf[MAX_INPUT_LENGTH] = "";
	smash_tilde(argument);
	argument = one_argument(argument, arg);
	guild = guild_lookup(arg);
	MEMBER *m;
	MEMBER *a;
	CHAR_DATA *target;

	if (IS_NULLSTR(arg)) {
		send_to_char("You must supply a guild name.\n\r", ch);
		return;
	}

	if (guild == GUILD_BOGUS) {
		sprintf(buf, "Guild %s not found.", capitalize(arg));
		send_to_char(buf, ch);
		return;
	}

	pc_guild_table[guild].active = FALSE;

	m = pc_guild_table[guild].members;
	while (m != NULL) {
		if ((target = get_char_world(ch, m->name, TRUE)) != NULL) {
			target->guild = GUILD_BOGUS;
		}
		nullify_member(guild, m->name);
		m = m->next;
	}

	a = pc_guild_table[guild].applicants;
	while (a != NULL) {
		if ((target = get_char_world(ch, a->name, TRUE)) != NULL) {
			target->cln_apply[0] = '\0';
		}
		nullify_applicant(guild, a->name);
		a = a->next;
	}

	sprintf(buf, "Guild %s denied.\n\r", capitalize(arg));
	send_to_char(buf, ch);

	if ((leader = get_char_world(ch, pc_guild_table[guild].leader->name, TRUE)) != NULL) {
		send_to_char("Your guild has been denied.\n\r", leader);
		leader->guild = GUILD_BOGUS;
	}

	save_guild(guild);
	return;
}

void do_guild_info(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH] = "";
	int center_diff;

	center_diff = strlen(pc_guild_table[ch->guild].symbol) - strlen(strip_color(pc_guild_table[ch->guild].symbol));

	if (!ch->guild || ch->guild == GUILD_BOGUS) {
		send_to_char("You're not in a guild.\n\r", ch);
		return;
	}

	send_to_char("{D:{B================================================={D:{x\n\r", ch);
	send_to_char(center_text(pc_guild_table[ch->guild].symbol, (51 + center_diff)), ch);
	send_to_char("{x\n\r", ch);
	send_to_char("{D:{B================================================={D:{x\n\r", ch);
	send_to_char("\n\r", ch);
	send_to_char(center_text("LEADER", 50), ch);
	send_to_char("\n\r", ch);
	send_to_char(center_text(pc_guild_table[ch->guild].leader->name, 50), ch);
	send_to_char("\n\r", ch);
	send_to_char("\n\r", ch);
	send_to_char(center_text("LEVELS", 50), ch);
	send_to_char("\n\r", ch);
	sprintf(buf, "%d", pc_guild_table[ch->guild].levels);
	send_to_char(center_text(buf, 50), ch);
	send_to_char("\n\r", ch);
	send_to_char("\n\r", ch);
	send_to_char("{D:{B-------------------------------------------------{D:{x\n\r", ch);
}

void do_guild_apply(CHAR_DATA *ch, char *argument)
{
	int guild;
	char arg[MAX_INPUT_LENGTH] = "";
	char buf[MAX_INPUT_LENGTH] = "";
	smash_tilde(argument);
	argument = one_argument(argument, arg);
	guild = guild_lookup(arg);
	MEMBER *a;
	DESCRIPTOR_DATA *d;

	if (is_guild(ch)) {
		send_to_char("You're already in a guild.\n\r", ch);
		return;
	}

	if (guild == GUILD_BOGUS) {
		sprintf(buf, "Guild %s not found.", arg);
		send_to_char(buf, ch);
		return;
	}

	free_string(ch->cln_apply);
	ch->cln_apply = str_dup(arg);

	a = new_member_elt();
	strcpy(a->name, ch->name);
	strcpy(a->gtitle, "Member");
	a->isLeader = FALSE;
	a->levels = ch->level;
	a->rank = 1;

	pc_guild_table[guild].applicants = append_member(pc_guild_table[guild].applicants, a);
	save_guild(guild);

	sprintf(buf, "You have applied for membership to guild %s.\n\r", capitalize(arg));
	send_to_char(buf, ch);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING) {
			if (d->character->guild == guild) {
				sprintf(buf, "%s has applied for membership to the guild.\n\r", ch->name);
				send_to_char(buf, d->character);
			}
		}
	}
}

void do_guild_disband(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_INPUT_LENGTH] = "";
	MEMBER *m;
	MEMBER *a;
	CHAR_DATA *target;

	m = pc_guild_table[ch->guild].members;
	while (m != NULL) {
		if ((target = get_char_world(ch, m->name, TRUE)) != NULL) {
			if (target != ch) {
				target->guild = GUILD_BOGUS;
			}
		}
		nullify_member(ch->guild, m->name);
		m = m->next;
	}

	a = pc_guild_table[ch->guild].applicants;
	while (a != NULL) {
		if ((target = get_char_world(ch, a->name, TRUE)) != NULL) {
			target->cln_apply[0] = '\0';
		}
		nullify_applicant(ch->guild, a->name);
		a = a->next;
	}

	pc_guild_table[ch->guild].levels = 0;

	for (d = descriptor_list; d; d = d->next)
	{
		if (d->connected == CON_PLAYING)
		{
			sprintf(buf, "{R%s {rhas disbanded %s.{x\n\r", ch->name, capitalize(pc_guild_table[ch->guild].name));
			send_to_char(buf, d->character);
		}
	}

	save_guild(ch->guild);
	reload_guild(ch->guild);
	ch->guild = GUILD_BOGUS;
}

void do_guild_expel(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH] = "";
	char name[MAX_INPUT_LENGTH] = "";
	CHAR_DATA *victim;
	MEMBER *m;
	MEMBER *t;
	argument = one_argument(argument, name);

	if (name[0] == '\0') {
		send_to_char("Syntax: guild expel <char>\n\r", ch);
		return;
	}

	if (!is_in_guild(ch->guild, name)) {
		send_to_char("No member of that name.\n\r", ch);
		return;
	}

	t = get_member(ch->guild, name);
	m = get_member(ch->guild, ch->name);
	if (t->isLeader || t->rank >= m->rank) {
		send_to_char("I don't think so.\n\r", ch);
		return;
	}
	
	sprintf(buf, "You have expelled %s from the guild.\n\r", capitalize(name));
	send_to_char(buf, ch);

	if ((victim = get_char_world(ch, name, TRUE)) != NULL) {
		victim->guild = GUILD_BOGUS;
		sprintf(buf, "You have been expelled from guild %s.\n\r", pc_guild_table[ch->guild].name);
		send_to_char(buf, victim);
	}

	nullify_member(ch->guild, name);
	save_guild(ch->guild);
	reload_guild(ch->guild);
}

void do_guild_defect(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *leader;
	char buf[MAX_INPUT_LENGTH] = "";


	if (!str_cmp(ch->name, pc_guild_table[ch->guild].leader->name)) {
		send_to_char("You're the leader of the guild and cannot defect. \n\r"
			"You may {mpromote{x another guild member, or {mdisband{x the guild entirely.\n\r", ch);
		return;
	}

	nullify_member(ch->guild, ch->name);
	save_guild(ch->guild);
	reload_guild(ch->guild);

	sprintf(buf, "You defect from guild %s.\n\r", pc_guild_table[ch->guild].name);
	send_to_char(buf, ch);

	if ((leader = get_char_world(ch, pc_guild_table[ch->guild].leader->name, TRUE)) != NULL) {
		sprintf(buf, "%s has defected from the guild.\n\r", ch->name);
		send_to_char(buf, leader);
	}

	ch->guild = GUILD_BOGUS;
}

void do_guild_title(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];
	char title[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	MEMBER *m;
	argument = one_argument(argument, name);
	strcpy(title, argument);

	if (name[0] == '\0' || title[0] == '\0') {
		send_to_char("Syntax: guild title <char> <title>\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, name, TRUE)) == NULL) {
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (victim->guild != ch->guild && !IS_IMMORTAL(ch)) {
		send_to_char("You aren't their leader.\n\r", ch);
		return;
	}

	sprintf(buf, "%s", title);
	
	m = get_member(victim->guild, victim->name);
	if (m != NULL) {
		strcpy(m->gtitle, buf);
	}

	sprintf(buf, "They have achieved %s status within the guild.\n\r", title);
	send_to_char(buf, ch);
	sprintf(buf, "You have achieved %s status within the guild.\n\r", title);
	send_to_char(buf, victim);

	save_guild(ch->guild);
}

void do_guild_rank(CHAR_DATA *ch, char *argument)
{
	char name[MAX_STRING_LENGTH];
	char rank[MAX_STRING_LENGTH];
	CHAR_DATA *member;
	int newRank;
	MEMBER *t;
	MEMBER *c;

	argument = one_argument(argument, name);
	argument = one_argument(argument, rank);
	member = get_char_world(ch, name, TRUE);
	t = get_member(ch->guild, name);
	c = get_member(ch->guild, ch->name);

	if (t == NULL) {
		send_to_char("They're not in your guild.\n\r", ch);
		return;
	}
	
	if (!str_cmp(rank, "1") || !str_cmp(rank, "2") || !str_cmp(rank, "3")) {
		newRank = atoi(rank);

		if (newRank == t->rank) {
			send_to_char("But that's their current rank...\n\r", ch);
			return;
		} else if (newRank > t->rank) {
			send_to_char("They have been promoted.\n\r", ch);
			if (member != NULL) {
				send_to_char("You have been promoted.", member);
			}
		} else {
			send_to_char("They have been demoted.\n\r", ch);
			if (member != NULL) {
				send_to_char("You have been demoted.", member);
			}
		}

		t->rank = newRank;
	} else if (!str_cmp(rank, "leader")) {
		// swapping leaders
		c->isLeader = FALSE;
		c->rank = 3;

		t->isLeader = TRUE;
		t->rank = 3;

		save_guild(ch->guild);
	} else {
		send_to_char("Invalid argument.\n\r", ch);
	}
}

void do_guild_symbol(CHAR_DATA *ch, char *argument)
{
	char symbol[MAX_STRING_LENGTH];
	strcpy(symbol, argument);

	if (!is_guild(ch)) {
		send_to_char("You're not even in a guild, let alone a leader.\n\r", ch);
		return;
	}

	if (str_cmp(ch->name, pc_guild_table[ch->guild].leader->name) && !IS_IMMORTAL(ch)) {
		send_to_char("You are not the leader of your guild.\n\r", ch);
		return;
	}

	if (pc_guild_table[ch->guild].active) {
		send_to_char("Your guild has alread been established.\n\r", ch);
		return;
	}

	strcpy(pc_guild_table[ch->guild].symbol, symbol);
	save_guild(ch->guild);
	reload_guild(ch->guild);
	send_to_char("The guild symbol has been updated.\n\r", ch);
}

void do_guild_decline(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *applicant;
	char buf[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];
	MEMBER *a;
	MEMBER *m;
	argument = one_argument(argument, name);

	if (name[0] == '\0') {
		send_to_char("Syntax: guild accept <char>\n\r", ch);
		return;
	}

	a = get_applicant(ch->guild, name);
	m = get_member(ch->guild, ch->name);

	if (a == NULL) {
		send_to_char("They aren't applying to your guild, so I guess we're good.", ch);
		return;
	}

	sprintf(buf, "You have declined %s's application.\n\r", a->name);
	send_to_char(buf, ch);

	if ((applicant = get_char_world(ch, name, TRUE)) != NULL) {
		free_string(applicant->cln_apply);
		sprintf(buf, "%s has declined your application to %s.\n\r", ch->name, capitalize(pc_guild_table[ch->guild].name));
		send_to_char(buf, applicant);
	}

	nullify_applicant(ch->guild, a->name);
	save_guild(ch->guild);
	reload_guild(ch->guild);
}

void do_guild_accept(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *applicant;
	char buf[MAX_STRING_LENGTH];
	argument = one_argument(argument, arg);
	MEMBER *a;
	MEMBER *m;
	MEMBER *newMember;
	a = get_applicant(ch->guild, arg);
	m = get_member(ch->guild, ch->name);

	if (arg[0] == '\0') {
		send_to_char("Syntax: guild accept <char>\n\r", ch);
		return;
	}

	if (a == NULL) {
		send_to_char("They aren't applying to your guild. Perhaps ask?", ch);
		return;
	}

	sprintf(buf, "You have accepted %s into the guild.\n\r", a->name);
	send_to_char(buf, ch);

	newMember = new_member_elt();
	strcpy(newMember->name, a->name);
	strcpy(newMember->gtitle, a->gtitle);
	newMember->levels = a->levels;
	newMember->isLeader = FALSE;
	newMember->rank = a->rank;
	
	pc_guild_table[ch->guild].members = append_member(pc_guild_table[ch->guild].members, newMember);
	nullify_applicant(ch->guild, a->name);


	if ((applicant = get_char_world(ch, arg, TRUE)) != NULL) {
		applicant->guild = ch->guild;
		free_string(applicant->cln_apply);
		sprintf(buf, "You have been accepted into %s.\n\r", capitalize(pc_guild_table[ch->guild].name));
		send_to_char(buf, applicant);
	}

	save_guild(ch->guild);
	reload_guild(ch->guild);
}

MEMBER *get_applicant(int guild, char *name) {
	MEMBER *m = NULL;

	m = pc_guild_table[guild].applicants;
	while (m != NULL) {
		if (!str_cmp(m->name, name)) {
			break;
		}
		m = m->next;
	}

	return m;
}

MEMBER *get_member(int guild, char *name) {
	MEMBER *m = NULL;

	m = pc_guild_table[guild].members;
	while (m != NULL) {
		if (!str_cmp(m->name, name)) {
			break;
		}
		m = m->next;
	}

	return m;
}

MEMBER *new_member_elt()
{
	MEMBER *m = NULL;
	m = (MEMBER *)malloc(sizeof(struct pc_guild_member));
	if (m == NULL) {
		bug("new_member_elt() failed to allocate %d bytes.",
			sizeof(struct pc_guild_member));
		return (NULL);
	}

	m->next = NULL;
	m->isLeader = FALSE;
	m->name[0] = '\0';
	m->gtitle[0] = '\0';
	m->levels = 0;
	m->rank = 0;
	return (m);
}


MEMBER *append_member(MEMBER * list, MEMBER * elt)
{
	MEMBER *cur = list;

	while (cur && (cur->next)) {
		cur = cur->next;
	}

	if (list == NULL) {
		return (elt);
	}
	else {
		elt->next = NULL;
		cur->next = elt;
		return (list);
	}
}

bool is_in_guild(int guild, char *name) {
	MEMBER *m;
	m = pc_guild_table[guild].members;
	while (m != NULL) {
		if (!str_cmp(name, m->name)) {
			return TRUE;
		}
		m = m->next;
	}
	return FALSE;
}

bool is_applicant(int guild, char *name) {
	MEMBER *m;
	m = pc_guild_table[guild].applicants;
	while (m != NULL) {
		if (!str_cmp(name, m->name)) {
			return TRUE;
		}
		m = m->next;
	}
	return FALSE;
}

void nullify_member(int guild, char *name)
{
	MEMBER *m;
	m = pc_guild_table[guild].members;
	while (m != NULL) {
		if (!str_cmp(name, m->name)) {
			m->name[0] = '\0';
			m->levels = 0;
			m->rank = 0;
			m->gtitle[0] = '\0';
		}
		m = m->next;
	}
}

void nullify_applicant(int guild, char *name)
{
	MEMBER *m;
	m = pc_guild_table[guild].applicants;
	while (m != NULL) {
		if (!str_cmp(name, m->name)) {
			m->name[0] = '\0';
			m->levels = 0;
			m->rank = 0;
			m->gtitle[0] = '\0';
		}
		m = m->next;
	}
}
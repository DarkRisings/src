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

struct pc_guild_type pc_guild_table[MAX_PC_GUILD];
int guild_number = 0;

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
	int isLeader = 0;
	char name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

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
				if (m->isLeader) {
					isLeader = 1;
				}

				fprintf(fp, "Memb %s %d %d %s\n", m->name, m->levels, isLeader, m->rank);
			}
			m = m->next;
		}

		a = pc_guild_table[guild].applicants;
		while (a != NULL) {
			if (a->name[0] != '\0') {
				fprintf(fp, "Appl %s %d %d %s\n", a->name, a->levels, 0, "Applicant");
			}
			a = a->next;
		}

		fprintf(fp, "End ");
		fclose(fp);
	}
	else {
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
				sprintf(buf, "%s ", fread_word(fp));
				strcpy(pc_guild_table[id].symbol, buf);
			break;
			case 'L':
				pc_guild_table[id].levels = fread_number(fp);
			break;
			case 'M':
				m = new_member_elt();
				strcpy(m->name, fread_word(fp));
				m->levels = fread_number(fp);
				isLeader = fread_number(fp);
				strcpy(m->rank, fread_word(fp));
				if (isLeader == 1) {
					m->isLeader = TRUE;
					foundLeader = TRUE;
					pc_guild_table[id].leader = m;
				}

				if (pc_guild_table[id].members == NULL) {
					pc_guild_table[id].members = m;
				} else {
					append_member(pc_guild_table[id].members, m);
				}
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
					strcpy(m->rank, fread_word(fp));
					if (pc_guild_table[id].applicants == NULL) {
						pc_guild_table[id].applicants = m;
					}
					else {
						append_member(pc_guild_table[id].applicants, m);
					}
				}
			break;
			case 'E':
				if (!foundLeader) {
					// Someone hosed the leader. This can either mean that
					// a player was deleted or the clan is disbanded. So
					// first check if there are any members...

					if (pc_guild_table[id].members != NULL) {
						// There are members, so first guy is it now. Gratz guy.
						m = new_member_elt();
						strcpy(m->name, pc_guild_table[id].members->name);
						strcpy(m->rank, pc_guild_table[id].members->rank);
						m->levels = pc_guild_table[id].members->levels;
						m->isLeader = TRUE;
						m->next = NULL;
						pc_guild_table[id].leader = m;
					} else {
						// If by this point there are NO members, this guild is disbanded
						// So just to be save let's set active to 0 to make sure it
						// won't show up in the guild list
						pc_guild_table[id].active = FALSE;
					}
					
				}
				if (!str_cmp(word, "End")) {
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
	m->name[0] = '\0';
	m->levels = GUILD_BOGUS;
	m->rank[0] = '\0';
	m->isLeader = FALSE;
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
	} else {
		elt->next = NULL;
		cur->next = elt;
		return (list);
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

void do_new_guild(CHAR_DATA* ch, char* argument)
{
	char arg1[MAX_INPUT_LENGTH] = "";
	char arg2[MAX_INPUT_LENGTH] = "";
	char arg3[MAX_INPUT_LENGTH] = "";
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	strcpy(arg3, argument);


	if (IS_NPC(ch)) {
		send_to_char("NPCs can't perform clan business.\n\r", ch);
		return;
	}

	if (IS_NULLSTR(arg1)) {
		send_to_char("What guild function would you like to perform?\n\r", ch);
		return;
	}

	if (!strcmp(arg1, "list")) {
		do_guild_list(ch);
	} else if (!strcmp(arg1, "who")) {
		do_guild_who(ch, arg2);
	} else if (!strcmp(arg1, "establish")) {
		do_guild_establish(ch, arg2, arg3);
	}
}

void do_guild_establish(CHAR_DATA* ch, char* name, char* symbol)
{
	int id;
	int i;
	MEMBER* m;
	char buf[MAX_INPUT_LENGTH] = "";
	FILE *guildIndex;

	if (IS_NULLSTR(name) || IS_NULLSTR(symbol)) {
		send_to_char("Syntax: guild establish <name> <symbol>\n\r", ch);
		return;
	}

	// First we have to make sure that this name hasn't already
	// been taken. This covers inactive guilds as well.
	for (i = 0; i < guild_number + 1; i++) {
		if (!IS_NULLSTR(pc_guild_table[i].name) && !str_cmp(name, pc_guild_table[i].name)) {
			send_to_char("That guild name has already been taken."
						 "It may be inactive and you may be able to claim it. Please see an imm.\n\r", ch);
			return;
		}

		if (!IS_NULLSTR(pc_guild_table[i].symbol) && !str_cmp(symbol, pc_guild_table[i].symbol)) {
			send_to_char("Please choose a unique symbol.\n\r", ch);
			return;
		}
	}

	// Make sure this player isn't in a guild. 
	if (is_guild(ch)) {
		send_to_char("You're already in a guild.\n\r", ch);
		return;
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
	strcpy(m->name, ch->name);
	strcpy(m->rank, ch->title_guild);
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
	sprintf(buf, "Guild %s has been proposed with you as the leader."
		"Your guild will be inactive until an immportal approves it.\n\r", symbol);
	send_to_char(buf, ch);
}

void do_guild_list(CHAR_DATA* ch)
{
	int i;
	char buf[MAX_INPUT_LENGTH] = "";
	send_to_char(":=============================================:\n\r", ch);
	send_to_char(" Name                Symbol              Level\n\r", ch);
	send_to_char(":---------------------------------------------:\n\r", ch);
	for (i = 0; i < guild_number + 1; i++) {
		if (!IS_NULLSTR(pc_guild_table[i].name) && pc_guild_table[i].active == TRUE) {
			sprintf(buf, " %-20s", pc_guild_table[i].name);
			send_to_char(buf, ch);
			sprintf(buf, "%-20s", pc_guild_table[i].symbol);
			send_to_char(buf, ch);
			sprintf(buf, "%-5d\n\r", pc_guild_table[i].levels);
			send_to_char(buf, ch);
		}
	}
	send_to_char(":---------------------------------------------:\n\r", ch);
}

void do_guild_who(CHAR_DATA* ch, char* argument)
{
	char arg[MAX_INPUT_LENGTH] = "";
	char buf[MAX_INPUT_LENGTH] = "";
	bool clnFound = FALSE;
	int i;
	MEMBER *m = NULL;
	smash_tilde(argument);
	argument = one_argument(argument, arg);

	if (IS_NULLSTR(arg)) {
		send_to_char("You must supply a guild name.\n\r", ch);
		return;
	}

	for (i = 0; i < guild_number + 1; i++) {
		if (!IS_NULLSTR(pc_guild_table[i].name) && !str_cmp(pc_guild_table[i].name, arg)) {
			clnFound = TRUE;
			break;
		}
	}

	if (clnFound) {
		m = pc_guild_table[i].members;
		while (m != NULL) {
			sprintf(buf, "[%s] %s\n\r", m->isLeader ? "Leader" : m->rank, m->name);
			send_to_char(buf, ch);
			m = m->next;
		}
	} else {
		sprintf(buf, "Guild %s not found.", arg);
		send_to_char(buf, ch);
	}
}

void do_rank(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg1);
	strcpy(arg2, argument);

	if (!is_guild(ch)) {
		send_to_char("You're not even in a guild, let alone a leader.\n\r", ch);
		return;
	}

	if (str_cmp(ch->name, pc_guild_table[ch->guild].leader->name) && !IS_IMMORTAL(ch)) {
		send_to_char("You are not the leader of your guild.\n\r", ch);
		return;
	}

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Syntax: rank <char> <rank>\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, TRUE)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	free_string(victim->title_guild);
	smash_tilde(arg2);
	sprintf(buf, "%s", arg2);
	victim->title_guild = str_dup(buf);

	sprintf(buf, "They have achieved %s status within the guild.\n\r", arg2);
	send_to_char(buf, ch);
	sprintf(buf, "You have achieved %s status within the guild.\n\r", arg2);
	send_to_char(buf, victim);
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

void nullify_member(int guild, char *name)
{
	MEMBER *m;
	m = pc_guild_table[guild].members;
	while (m != NULL) {
		if (!str_cmp(name, m->name)) {
			m->name[0] = '\0';
			m->isLeader = FALSE;
			m->levels = 0;
			m->rank[0] = '\0';
		}
		m = m->next;
	}
}
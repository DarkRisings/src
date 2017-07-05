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
	} else if (!strcmp(arg1, "apply")) {
		do_guild_apply(ch, arg2);
	} else if (!strcmp(arg1, "approve")) {
		do_guild_approve(ch, arg2);
	} else if (!strcmp(arg1, "info")) {
		do_guild_info(ch);
	} else if (!strcmp(arg1, "accept")) {
		do_guild_accept(ch, arg2);
	} else if (!strcmp(arg1, "defect")) {
		do_guild_defect(ch);
	} else if (!strcmp(arg1, "expel")) {
		do_guild_expel(ch, arg2);
	} else if (!strcmp(arg1, "disband")) {
		do_guild_disband(ch);
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
				fprintf(fp, "Appl %s %d %d %s\n", a->name, a->levels, 1, "Applicant");
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
				sprintf(buf, "%s ", fread_word(fp));
				strcpy(pc_guild_table[id].symbol, buf);
			break;
			case 'M':
				// Red in a new member
				m = new_member_elt();
				strcpy(m->name, fread_word(fp));
				m->levels = fread_number(fp);
				isLeader = fread_number(fp);
				m->rank = fread_number(fp);
				strcpy(m->gtitle, fread_word(fp));
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
							strcpy(m->rank, pc_guild_table[id].members->rank);
							m->levels = pc_guild_table[id].members->levels;
							m->isLeader = TRUE;
							m->next = NULL;
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
	sprintf(buf, "%s ", strip_whitespace(symbol));
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
			sprintf(buf, "%3d\n\r", pc_guild_table[i].levels);
			send_to_char(buf, ch);
		}
	}
	send_to_char(":---------------------------------------------:\n\r", ch);
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
			sprintf(rankBuf, "%s\n\r", "Leader");
		} else {
			sprintf(rankBuf, "%d\n\r", m->rank);
		}

		sprintf(buf, "[%s] %s (Rank %s)\n\r", m->gtitle, m->name, rankBuf);
		send_to_char(buf, ch);
		m = m->next;
	}
}

void do_guild_prospects(CHAR_DATA* ch)
{
	char buf[MAX_INPUT_LENGTH] = "";
	MEMBER *m = NULL;
	MEMBER *applicant = NULL;

	if (!is_guild(ch)) {
		send_to_char("You're not even in a guild.\n\r", ch);
		return;
	}

	m = get_member(ch->guild, ch->name);

	if (!can_perform_action(m, "prospects") && !IS_IMMORTAL(ch)) {
		send_to_char("You're not high enough rank to perform that action.\n\r", ch);
		return;
	}

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

	if (IS_NULLSTR(arg)) {
		send_to_char("You must supply a guild name.\n\r", ch);
		return;
	}

	if (!IS_IMMORTAL(ch)) {
		send_to_char("Right. Let's just have total anarchy.\n\r", ch);
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

	save_guild(guild);
	return;
}

void do_guild_info(CHAR_DATA *ch)
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

void do_guild_disband(CHAR_DATA *ch)
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH] = "";
	char buf[MAX_INPUT_LENGTH] = "";
	MEMBER *m;
	MEMBER *a;

	if (!is_guild(ch)) {
		send_to_char("You're not even in a guild, let alone a leader.\n\r", ch);
		return;
	}

	if (str_cmp(ch->name, pc_guild_table[ch->guild].leader->name) && !IS_IMMORTAL(ch)) {
		send_to_char("You are not the leader of your guild.\n\r", ch);
		return;
	}

	m = pc_guild_table[ch->guild].members;
	while (m != NULL) {
		nullify_member(ch->guild, m->name);
		m = m->next;
	}

	a = pc_guild_table[ch->guild].applicants;
	while (m != NULL) {
		nullify_applicant(ch->guild, a->name);
		a = a->next;
	}

	for (d = descriptor_list; d; d = d->next)
	{
		if (d->connected == CON_PLAYING)
		{
			sprintf(buf, "%s has disbanded %s.\n\r", ch->name, capitalize(pc_guild_table[ch->guild].name));
			send_to_char(buf, d->character);
		}
	}

	save_guild(ch->guild);
	reload_guild(ch->guild);
	ch->guild = GUILD_BOGUS;
}

void do_guild_expel(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH] = "";
	char buf[MAX_INPUT_LENGTH] = "";
	smash_tilde(argument);
	argument = one_argument(argument, arg);
	CHAR_DATA *victim;
	MEMBER *m;

	if (arg[0] == '\0') {
		send_to_char("Syntax: guild expel <char>\n\r", ch);
		return;
	}

	if (!is_guild(ch)) {
		send_to_char("You're not even in a guild, let alone a leader.\n\r", ch);
		return;
	}

	m = get_member(ch->guild, ch->name);

	if (!can_perform_action(m, "expel") && !IS_IMMORTAL(ch)) {
		send_to_char("You are not high enough rank to perform that action.\n\r", ch);
		return;
	}

	if (!is_in_guild(ch->guild, arg)) {
		send_to_char("No member of that name.\n\r", ch);
		return;
	}
	
	sprintf(buf, "You have expelled %s from the guild.\n\r", capitalize(arg));
	send_to_char(buf, ch);

	if ((victim = get_char_world(ch, arg, TRUE)) != NULL) {
		sprintf(buf, "You have been expelled from guild %s.\n\r", pc_guild_table[ch->guild].name);
		send_to_char(buf, victim);
	}

	nullify_member(ch->guild, arg);
	save_guild(ch->guild);
	reload_guild(ch->guild);
}

void do_guild_defect(CHAR_DATA *ch)
{
	CHAR_DATA *leader;
	char buf[MAX_INPUT_LENGTH] = "";

	if (!is_guild(ch)) {
		send_to_char("You're not in a guild.\n\r", ch);
		return;
	}

	if (!str_cmp(ch->name, pc_guild_table[ch->guild].leader->name)) {
		send_to_char("You're the leader of the guild and cannot defect. "
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

void do_guild_rank(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	MEMBER *m;

	argument = one_argument(argument, arg1);
	strcpy(arg2, argument);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		send_to_char("Syntax: rank <char> <rank>\n\r", ch);
		return;
	}

	if (!is_guild(ch)) {
		send_to_char("You're not even in a guild, let alone a leader.\n\r", ch);
		return;
	}

	if (str_cmp(ch->name, pc_guild_table[ch->guild].leader->name) && !IS_IMMORTAL(ch)) {
		send_to_char("You are not the leader of your guild.\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, TRUE)) == NULL) {
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (victim->guild != ch->guild && !IS_IMMORTAL(ch)) {
		send_to_char("You aren't their leader.\n\r", ch);
		return;
	}

	sprintf(buf, "%s", arg2);
	
	m = get_member(victim->guild, victim->name);
	if (m != NULL) {
		strcpy(m->gtitle, buf);
	}

	sprintf(buf, "They have achieved %s status within the guild.\n\r", arg2);
	send_to_char(buf, ch);
	sprintf(buf, "You have achieved %s status within the guild.\n\r", arg2);
	send_to_char(buf, victim);

	save_guild(ch->guild);
}

void do_guild_rank(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int newRank;
	MEMBER *c;
	MEMBER *t;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (!is_guild(ch)) {
		send_to_char("You're not even in a guild, let alone a leader.\n\r", ch);
		return;
	}

	c = get_member(ch->guild, ch->name);

	if (!can_perform_action(c, "promote") && !IS_IMMORTAL(ch)) {
		send_to_char("You are not the leader of your guild.\n\r", ch);
		return;
	}

	t = get_member(ch->guild, arg1);

	if (t == NULL) {
		send_to_char("Their not in your guild.\n\r", ch);
		return;
	}
	
	if (!str_cmp(arg2, "1") || !str_cmp(arg2, "2") || !str_cmp(arg2, "3")) {
		newRank = atoi(arg2);

		if (newRank == t->rank) {
			send_to_char("But that's their current rank...\n\r", ch);
			return;
		} else if (newRank > t->rank) {
			send_to_char("They have been promoted.\n\r", ch);
		} else {
			send_to_char("They have been demoted.\n\r", ch);
		}

		t->rank = newRank;
	} else if (!str_cmp(arg2, "leader")) {
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
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	argument = one_argument(argument, arg);

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

	sprintf(buf, "%s", strip_whitespace(arg));
	strcpy(pc_guild_table[ch->guild].symbol, buf);

	send_to_char("The guild symbol has been updated.\n\r", ch);
}

void do_guild_decline(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *applicant;
	char buf[MAX_STRING_LENGTH];
	argument = one_argument(argument, arg);
	MEMBER *a;
	MEMBER *m;
	a = get_applicant(ch->guild, arg);

	if (arg[0] == '\0') {
		send_to_char("Syntax: guild accept <char>\n\r", ch);
		return;
	}

	if (!is_guild(ch)) {
		send_to_char("You're not even in a guild, let alone a leader.\n\r", ch);
		return;
	}

	m = get_member(ch->guild, ch->name);

	if (!can_perform_action(m, "decline") && !IS_IMMORTAL(ch)) {
		send_to_char("You are not high enough rank to perform that action.\n\r", ch);
		return;
	}

	if (a == NULL) {
		send_to_char("They aren't applying to your guild, so I guess we're good.", ch);
		return;
	}


	sprintf(buf, "You have declined %s's application.\n\r", a->name);
	send_to_char(buf, ch);

	if ((applicant = get_char_world(ch, arg, TRUE)) != NULL) {
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
	a = get_applicant(ch->guild, arg);
	m = get_member(ch->guild, ch->name);

	if (arg[0] == '\0') {
		send_to_char("Syntax: guild accept <char>\n\r", ch);
		return;
	}

	if (!is_guild(ch)) {
		send_to_char("You're not even in a guild, let alone a leader.\n\r", ch);
		return;
	}

	if (!can_perform_action(m, "accept") && !IS_IMMORTAL(ch)) {
		send_to_char("You are not high enough rank to perform this action.\n\r", ch);
		return;
	}

	if (a == NULL) {
		send_to_char("They aren't applying to your guild. Perhaps ask?", ch);
		return;
	}

	pc_guild_table[ch->guild].members = append_member(pc_guild_table[ch->guild].members, a);
	nullify_applicant(ch->guild, a->name);

	sprintf(buf, "You have accepted %s into the guild.\n\r", a->name);
	send_to_char(buf, ch);

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

bool can_perform_action(MEMBER *m, char *action) 
{
	if (m->isLeader) {
		return TRUE;
	}

	if (action == "accept" && m->rank < 2) {
		return FALSE;
	}

	if (action == "decline" && m->rank < 2) {
		return FALSE;
	}

	if (action == "prospects" && m->rank < 2) {
		return FALSE;
	}

	if (action == "expel" && m->rank < 3) {
		return FALSE;
	}

	return TRUE;
}
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,       *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                       *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael       *
 *  Chastain, Michael Quan, and Mitchell Tse.                   *
 *                                       *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                           *
 *                                       *
 *  Much time and thought has gone into this software and you are       *
 *  benefitting.  We hope that you share your changes too.  What goes       *
 *  around, comes around.                           *
 ***************************************************************************/

/***************************************************************************
 *    ROM 2.4 is copyright 1993-1996 Russ Taylor               *
 *    ROM has been brought to you by the ROM consortium           *
 *        Russ Taylor (rtaylor@efn.org)                   *
 *        Gabrielle Taylor                           *
 *        Brian Moore (zump@rom.org)                       *
 *    By using this code, you have agreed to follow the terms of the       *
 *    ROM license, in the file Rom24/doc/rom.license               *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"

/* When I originally was tasked to code jousting, I had (currently, have) no idea
** how much code it's actually going to take. So, I decided to create a new file
** to house it, why not, right? ... right?? HOW HARD CAN THIS BE? OH GOD.
**
** - Zalyriel (1/18/2014)
***
** Update (7/17/2014)
**
** Well I think I've done it. I can't speak to how clean it is, though. :p
** Just one long ass function.
**/


void do_jousting(CHAR_DATA * ch, char * argument)
{
char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
	char arg4 [MAX_INPUT_LENGTH];
	char buf [MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA * d;
    CHAR_DATA * jouster1 = NULL;
	CHAR_DATA * jouster2 = NULL;
	int value;

	int jouster1_count = 0;
	int jouster2_count = 0;

	smash_tilde(argument);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);
    strcpy(arg4, argument);


	/*
    * Snarf the value (which need not be numeric).
    */
    value = is_number(arg2) ? atoi(arg2) : -1;



	for (d = descriptor_list; d != NULL; d = d->next ) {
    if (d->connected == CON_PLAYING && !IS_NPC(d->character ) ) {
        if (d->character->jouster1 == 1)
			{
                jouster1 = d->character;
				jouster1_count++;
			}

			if (d->character->jouster2 == 1)
			{
                jouster2 = d->character;
				jouster2_count++;
			}
		}
	}

	if (jouster1_count == 0) {
        jouster1 = NULL;
    }

	if (jouster2_count == 0) {
        jouster2 = NULL;
    }


	if (arg1[0] == '\0' && arg2[0] == '\0' && arg3[0] == '\0' && arg4[0] == '\0') {
        send_to_char("Available Options:\n\r", ch);
        send_to_char("Joust attack # -- Sets your joust attack, numbers are 1 - 3\n\r", ch);
        send_to_char("Joust ready    -- Lets the immortal know that you are ready\n\r", ch);

        if (IS_IMMORTAL(ch)) {
            send_to_char("Immortal Commands:\n\r", ch);
            send_to_char("joust begin <name> <name> -- starts a joust between two players\n\r", ch);
            send_to_char("joust continue -- runs through the stages of the joust until someone wins\n\r", ch);
            send_to_char("joust end -- Ends the joust and clears all joust data\n\r", ch);
        }

        return;
    }


	/*** Set up the joust ***/
	if (!str_prefix(arg1, "begin")) {
        if (!IS_IMMORTAL(ch)) {
            send_to_char("You can't do that.\n\r", ch);
            return;
        }

        if (arg2[0] == '\0' || arg3[0] == '\0' || arg4[0] != '\0') {
            send_to_char("Bad syntax.\n\r", ch);
            return;
        }

        if (jouster1 != NULL || jouster2 != NULL) {
            send_to_char("It appears there is a joust in progress already. Use joust end.\n\r", ch);
            return;
        }

        // Locate the chars
        jouster1 = get_char_world(ch, arg2, FALSE);
        jouster2 = get_char_world(ch, arg3, FALSE);

        if (jouster1 == NULL || jouster2 == NULL) {
            send_to_char("They aren't here.\n\r", ch);
            return;
        }

        // Now store them.
        jouster1->jouster1 = 1;
		jouster2->jouster2 = 1;
		jouster1->in_joust = 1;
		jouster2->in_joust = 1;

		for (d = descriptor_list; d != NULL; d = d->next )
			{
                if (d->connected == CON_PLAYING && !IS_NPC(d->character ) )
				{
                    if (d->character->in_room->vnum == 1900 || d->character->in_joust == 1)
					{
                        if (d->character != ch)
						{
                            sprintf(buf, "%s has started a joust between %s and %s!{x\n\r",
                                ch->name, jouster1->name, jouster2->name);
							send_to_char(buf, d->character);
						}

						else
						{
                            sprintf(buf, "You have started a joust between %s and %s!{x\n\r",
                                jouster1->name, jouster2->name);
							send_to_char(buf, d->character);
						}
					}
				}
			}

		char_from_room(jouster1);
		char_to_room(jouster1, get_room_index(1901));

		char_from_room(jouster2);
		char_to_room(jouster2, get_room_index(1903));

		for (d = descriptor_list; d != NULL; d = d->next )
		{
            if (d->connected == CON_PLAYING && !IS_NPC(d->character ) )
			{
                if (d->character->in_room->vnum == 1900)
				{
                    sprintf(buf,
                        "%s and %s descend the stairs and take their place on a horse at either end of the arena.\n\r",
                        jouster1->name, jouster2->name);
					send_to_char(buf, d->character);
				}
			}
		}

		send_to_char("You descend the stairs and take your place on a horse at the west side of the arena.\n\r",
            jouster1);
		send_to_char("You descend the stairs and take your place on a horse at the east side of the arena.\n\r",
            jouster2);
		return;
	}

	/*** Set up the attacks ***/
	if (!str_prefix(arg1, "attack")) {
        if (ch->in_joust == 0) {
            send_to_char("You're not jousting.\n\r", ch);
            return;
        }

		if (jouster1 == NULL || jouster2 == NULL) {
            send_to_char("It appears you no longer have an opponent.\n\r", ch);
            return;
        }

		if (ch->joust_ready != 0) {
            send_to_char("You can't set your attack anymore! Wait until this round is over.\n\r", ch);
        }

		if (arg3[0] != '\0' && arg4[0] != '\0') {
            send_to_char("Syntax: Joust attack #. \n\r", ch);
            send_to_char("1 -  Lunge\n\r", ch); // Rock
            send_to_char("2 -  Strike\n\r", ch); // Paper
            send_to_char("3 -  Reposte\n\r", ch); // Scissors
            send_to_char("4 -  Feign\n\r", ch); // Lizard
            send_to_char("5 -  Shield Block\n\r", ch); // Spock
            send_to_char("See help joust attack for more information.\n\r", ch);
            return;
        }

		if (arg2[0] == '\0') {
            if (!IS_NULLSTR(ch->joust_attack) ) {
                send_to_char("Your current joust attack is:\n\r", ch);
                sprintf(buf, "{Y%s{x.\n\r", ch->joust_attack);
				send_to_char(buf, ch);
				return;
			} else {
                send_to_char("You have not yet set your attack position. See help joust attack for more information.\n\r",
                    ch);
            }
		}

		if (value != 1 && value != 2 && value != 3 && value != 4 && value != 5 && arg2[0] != '\0') {
            send_to_char("That is not a valid attack. You may choose from the following:\n\r", ch);
            send_to_char("1 -  Lunge\n\r", ch); // Rock
            send_to_char("2 -  Strike\n\r", ch); // Paper
            send_to_char("3 -  Reposte\n\r", ch); // Scissors
            send_to_char("4 -  Feign\n\r", ch); // Lizard
            send_to_char("5 -  Shield Block\n\r", ch); // Spock
            send_to_char("See help joust attack for more information.\n\r", ch);
            return;
        }

		ch->joust_attack_num = value;

		switch (value) {
            case 1:
                ch->joust_attack = "lunge";
			break;

            case 2:
                ch->joust_attack = "strike";
			break;

            case 3:
                ch->joust_attack = "reposte";
			break;

            case 4:
                ch->joust_attack = "feign";
			break;

            case 5:
                ch->joust_attack = "shield block";
			break;
        }

		sprintf(buf, "Your attack is now {Y%s{x{x\n\r", ch->joust_attack);
		send_to_char(buf, ch);

		return;
	}

	/*** Commence! ***/
	if (!str_prefix(arg1, "ready")) {

        if (ch->in_joust == 0) {
            send_to_char("You're not jousting.\n\r", ch);
            return;
        }

		if (jouster1 == NULL || jouster2 == NULL) {
            send_to_char("It appears you no longer have an opponent.\n\r", ch);
            return;
        }

		if (IS_NULLSTR(ch->joust_attack)) {
            send_to_char("Set an attack first!\n\r", ch);
            return;
        }

		if (arg2[0] != '\0' || arg3[0] != '\0' || arg4[0] != '\0') {
            send_to_char("Just 'joust ready' -- no further arguments necessary.\n\r", ch);
            return;
        }

		if (ch->joust_ready > 0) {
            send_to_char("You're already... ready.\n\r", ch);
            return;
        }

		ch->joust_ready = 1;

		send_to_char("You ready yourself for the joust!\n\r", ch);

		for (d = descriptor_list; d != NULL; d = d->next ) {
            if (d->connected == CON_PLAYING && !IS_NPC(d->character ) ) {
                if (d->character->in_room->vnum == 1900) {
                    if (d->character != ch) {
                        sprintf(buf, "%s is ready for the joust!{x\n\r", ch->name);
						send_to_char(buf, d->character);
					}
				}
			}
		}

		return;
	}

	// Immortal only 'joust continue' command. Why? Because I wanted to be able to add some
	// kind of pause between each 'stage' of the joust, and there's no easy way to do that while
	// also keeping jousting streamlined into one function. So I set this up, and the imm can control
	// the delays between flavor texts.
	if (!str_prefix(arg1, "continue")) {
        if (!IS_IMMORTAL(ch)) {
            send_to_char("You can't do that.\n\r", ch);
            return;
        }

        if (jouster1 == NULL && jouster2 == NULL) {
            send_to_char("It would appear as though there is no joust in progress, or both participants have left.\n\r",
                ch);
            return;
        } else {
            if (jouster1 == NULL || jouster2 == NULL) {
                send_to_char("It appears one of the jousters has bailed.\n\r", ch);
                return;
            }
        }

        if (arg2[0] != '\0' || arg3[0] != '\0' || arg4[0] != '\0') {
            send_to_char("Syntax: joust continue\n\r", ch);
            return;
        }

        // Check if they're both ready.
        if (jouster1->joust_ready == 0 || jouster2->joust_ready == 0) {
            send_to_char("The jousters do not seem to be ready.\n\r", ch);
            return;
        }

		// Stage one -- initiation
		if (jouster1->joust_ready == 1 && jouster2->joust_ready == 1){
            for (d = descriptor_list; d != NULL; d = d->next ) {
                if (d->connected == CON_PLAYING && !IS_NPC(d->character ) ) {
                    if (d->character->in_room->vnum == 1900 || d->character->in_joust == 1) {
                        send_to_char("A horn sounds above you and the horses begin to charge!  The joust begins!\n\r",
                            d->character);
					}
				}
			}

			jouster1->joust_ready = 2;
			jouster2->joust_ready = 2;

			// Now we are in stage two.
			return;
		} else if (jouster1->joust_ready == 2 && jouster2->joust_ready == 2) {
            char_from_room(jouster1);
            char_to_room(jouster1, get_room_index(1902));

            char_from_room(jouster2);
            char_to_room(jouster2, get_room_index(1902));

            if ((jouster1->joust_attack_num - jouster2->joust_attack_num) % 5 == 1 ||
        (jouster1->joust_attack_num - jouster2->joust_attack_num) % 5 == 2) {
                jouster1->joust_win = 1;
				jouster2->joust_win = 0;
			}

			else if ((jouster1->joust_attack_num - jouster2->joust_attack_num) % 5 == 0)
			{
                jouster2->joust_win = 0;
				jouster1->joust_win = 0;
			}

			else
			{
                jouster1->joust_win = 0;
				jouster2->joust_win = 1;
			}

			if (jouster1->joust_win == 0 && jouster2->joust_win == 0)
			{

                sprintf(buf,
                    "As your horse charges toward the center of the arena, you %s and %s %ss!  It's anybody's game!\n\r",
                    jouster1->joust_attack, jouster2->name, jouster2->joust_attack);
				send_to_char(buf, jouster1);

				sprintf(buf,
                    "As your horse charges toward the center of the arena, you %s and %s %ss!  It's anybody's game!\n\r",
                    jouster2->joust_attack, jouster1->name, jouster1->joust_attack);
				send_to_char(buf, jouster2);


				for (d = descriptor_list; d != NULL; d = d->next )
				{
                    if (d->connected == CON_PLAYING && !IS_NPC(d->character ) )
					{
                        if (d->character->in_room->vnum == 1900 && d->character->in_joust == 0)
						{
                            sprintf(buf,
                                "As %s's horse charges toward %s, %s %ss and %s %ss! It's anybody's game!{x\n\r",
                                jouster1->name, jouster2->name, jouster1->name, jouster1->joust_attack,
							jouster2->name, jouster2->joust_attack);
							send_to_char(buf, d->character);
						}
					}
				}

				if (number_percent() < 50) {
                    sprintf(buf, "{GYou dodge %s's attack just in time, and land a blow!{x\n\r", jouster1->name);
					send_to_char(buf, jouster2);

					for (d = descriptor_list; d != NULL; d = d->next )
					{
                        if (d->connected == CON_PLAYING && !IS_NPC(d->character ) )
						{
                            if (d->character->in_room->vnum == 1900 && d->character->in_joust == 0)
							{
                                sprintf(buf, "%s dodges %s's attack just in time, and lands a blow!{x\n\r",
                                    jouster2->name, jouster1->name);
								send_to_char(buf, d->character);
							}
						}
					}

					sprintf(buf, "{r%s dodges your attack just in time, and lands a blow!{x\n\r", jouster2->name);
					send_to_char(buf, jouster1);
					raw_damage(jouster1, jouster1, 200, TYPE_UNDEFINED, DAM_NONE, FALSE, TRUE);
				} else {
                    sprintf(buf, "{GYou dodge %s's attack just in time, and land a blow!{x\n\r", jouster2->name);
					send_to_char(buf, jouster1);

					for (d = descriptor_list; d != NULL; d = d->next ) {
                        if (d->connected == CON_PLAYING && !IS_NPC(d->character ) ) {
                            if (d->character->in_room->vnum == 1900 && d->character->in_joust == 0) {
                                sprintf(buf, "%s dodges %s's attack just in time, and lands a blow!{x\n\r",
                                    jouster1->name, jouster2->name);
								send_to_char(buf, d->character);
							}
						}
					}

					sprintf(buf, "{r%s dodges your attack just in time, and lands a blow!{x\n\r", jouster1->name);
					send_to_char(buf, jouster2);
					raw_damage(jouster2, jouster2, 200, TYPE_UNDEFINED, DAM_NONE, FALSE, TRUE);
				}
			}


			else if (jouster1->joust_win == 1 && jouster2->joust_win == 0)
			{
                sprintf(buf,
                    "{GAs your horse charges toward the center of the arena, you %s and %s %ss! You win!{x\n\r",
                    jouster1->joust_attack, jouster2->name, jouster2->joust_attack);
				send_to_char(buf, jouster1);

				sprintf(buf,
                    "{RAs your horse charges toward the center of the arena, you %s and %s %ss! You lose!{x\n\r",
                    jouster2->joust_attack, jouster1->name, jouster1->joust_attack);
				send_to_char(buf, jouster2);

				for (d = descriptor_list; d != NULL; d = d->next )
				{
                    if (d->connected == CON_PLAYING && !IS_NPC(d->character ) )
					{
                        if (d->character->in_room->vnum == 1900 && d->character->in_joust == 0)
						{
                            sprintf(buf, "As %s's horse charges toward %s, %s %ss and %s %ss, %s wins!{x\n\r",
                                jouster1->name, jouster2->name, jouster1->name, jouster1->joust_attack,
							jouster2->name, jouster2->joust_attack, jouster1->name);
							send_to_char(buf, d->character);
						}
					}
				}

				sprintf(buf, "{rYou feel a blinding pain as %s lands a blow!{x\n\r", jouster1->name);
				send_to_char(buf, jouster2);
				raw_damage(jouster2, jouster2, 200, TYPE_UNDEFINED, DAM_NONE, FALSE, TRUE);
			}

			else  if (jouster2->joust_win == 1 && jouster1->joust_win == 0)
			{
                sprintf(buf,
                    "{GAs your horse charges toward the center of the arena, you %s and %s %ss! You win!{x\n\r",
                    jouster2->joust_attack, jouster1->name, jouster1->joust_attack);
				send_to_char(buf, jouster2);

				sprintf(buf,
                    "{RAs your horse charges toward the center of the arena, you %s and %s %ss! You lose!{x\n\r",
                    jouster1->joust_attack, jouster2->name, jouster2->joust_attack);
				send_to_char(buf, jouster1);

				for (d = descriptor_list; d != NULL; d = d->next )
				{
                    if (d->connected == CON_PLAYING && !IS_NPC(d->character ) )
					{
                        if (d->character->in_room->vnum == 1900 && d->character->in_joust == 0)
						{
                            sprintf(buf, "As %s's horse charges toward %s, %s %ss and %s %ss, %s wins!{x\n\r",
                                jouster2->name, jouster1->name, jouster2->name, jouster2->joust_attack,
							jouster1->name, jouster1->joust_attack, jouster2->name);
							send_to_char(buf, d->character);
						}
					}
				}

				sprintf(buf, "{rYou feel a blinding pain as %s lands a blow!{x\n\r", jouster2->name);
				send_to_char(buf, jouster1);
				raw_damage(jouster1, jouster1, 200, TYPE_UNDEFINED, DAM_NONE, FALSE, TRUE);
			}

			else
			{
                send_to_char("Apparently all of the following are not true:", ch);
                send_to_char("Both jouster1 and jouster2 tied. Jouster1 lost and jouster2 won. Jouster2 won and jouster 2 lost.\n\r",
                    ch);
                send_to_char("So.... Zalyriel fucked something up.\n\r", ch);
            }

			// add another stage starting here. checks if one of the jousters is unconscious, if not, restarts the joust.
			jouster1->joust_ready = 3;
			jouster2->joust_ready = 3;
			return;
		}

		// Damage dealt, start a new round if no one died, end joust if there was a death/KO
		else if (jouster1->joust_ready == 3 && jouster2->joust_ready == 3)
		{
            if (jouster1->position == POS_UNCONSCIOUS ||
        jouster1->position == POS_DEAD ||
        IS_AFFECTED2(jouster1, AFF_GHOST))
			{
                for (d = descriptor_list; d != NULL; d = d->next )
				{
                    if (d->connected == CON_PLAYING && !IS_NPC(d->character ) )
					{
                        if (d->character->in_room->vnum == 1900 && d->character->in_joust == 0)
						{
                            sprintf(buf, "{Y%s wins the joust!{x\n\r", jouster2->name);
							send_to_char(buf, d->character);
						}
					}
				}

				send_to_char("{YYou win the joust!{x\n\r", jouster2);
				send_to_char("{RYou have lost the joust!{x\n\r", jouster1);

				char_from_room(jouster1);
				char_to_room(jouster1, get_room_index(1900));

				char_from_room(jouster2);
				char_to_room(jouster2, get_room_index(1900));

				for (d = descriptor_list; d != NULL; d = d->next )
				{
                    if (d->connected == CON_PLAYING && !IS_NPC(d->character ))
					{
                        if (d->character->in_joust == 1)
						{
                            d->character->joust_attack = &str_empty[0];
							d->character->joust_ready = 0;
							d->character->joust_attack_num = 0;
							d->character->in_joust = 0;
							d->character->jouster1 = 0;
							d->character->jouster2 = 0;
						}
					}
				}


				return;
			}

			else if (jouster2->position == POS_UNCONSCIOUS ||
                    jouster2->position == POS_DEAD ||
                    IS_AFFECTED2(jouster2, AFF_GHOST)){
                for (d = descriptor_list; d != NULL; d = d->next ){
                    if (d->connected == CON_PLAYING && !IS_NPC(d->character ) ){
                        if (d->character->in_room->vnum == 1900 && d->character->in_joust == 0){
                            sprintf(buf, "{Y%s wins the joust!{x\n\r", jouster1->name);
							send_to_char(buf, d->character);
						}
					}
				}

				send_to_char("{YYou win the joust!{x\n\r", jouster1);
				send_to_char("{RYou have lost the joust!{x\n\r", jouster2);

				char_from_room(jouster1);
				char_to_room(jouster1, get_room_index(1900));

				char_from_room(jouster2);
				char_to_room(jouster2, get_room_index(1900));

				for (d = descriptor_list; d != NULL; d = d->next ){
                    if (d->connected == CON_PLAYING && !IS_NPC(d->character )){
                        if (d->character->in_joust == 1){
                            d->character->joust_attack = &str_empty[0];
							d->character->joust_ready = 0;
							d->character->joust_attack_num = 0;
							d->character->in_joust = 0;
							d->character->jouster1 = 0;
							d->character->jouster2 = 0;
						}
					}
				}


				return;
			} else {
                for (d = descriptor_list; d != NULL; d = d->next ) {
                    if (d->connected == CON_PLAYING && !IS_NPC(d->character ) ) {
                        if (d->character->in_room->vnum == 1900 && d->character->in_joust == 0) {
                            send_to_char("Another horn rings out across the arena and the horses trot back into position.\n\r",
                                d->character);
						}
					}
				}

				send_to_char("Another horn rings out across the arena and your horse trots back into position.\n\r",
                    jouster1);
				send_to_char("Another horn rings out across the arena and your horse trots back into position.\n\r",
                    jouster2);

				char_from_room(jouster1);
				char_to_room(jouster1, get_room_index(1901));

				char_from_room(jouster2);
				char_to_room(jouster2, get_room_index(1903));

				jouster1->joust_ready = 0;
				jouster2->joust_ready = 0;
			}
		}

	    return;
	}

	if (!str_prefix(arg1, "end")) {

        if (!IS_IMMORTAL(ch)) {
            send_to_char("You can't do that.\n\r", ch);
            return;
        }

        if (arg2[0] != '\0' || arg3[0] != '\0' || arg4[0] != '\0') {
            send_to_char("Syntax: joust end\n\r", ch);
            return;
        }

        if (jouster1 == NULL && jouster2 == NULL) {
            send_to_char("There... doesn't appear to be a joust in progress that you can end.", ch);
            return;
        }

        for (d = descriptor_list; d != NULL; d = d->next ) {
            if (d->connected == CON_PLAYING && !IS_NPC(d->character )) {
                if (d->character->in_joust == 1) {
                    d->character->joust_attack = &str_empty[0];
                    d->character->joust_ready = 0;
                    d->character->joust_attack_num = 0;
                    d->character->in_joust = 0;
                    d->character->jouster1 = 0;
                    d->character->jouster2 = 0;
                }
            }
        }

		jouster1 = NULL;
		jouster2 = NULL;

		for (d = descriptor_list; d != NULL; d = d->next ){
            if (d->connected == CON_PLAYING && !IS_NPC(d->character ) ){
                if (d->character->in_room->vnum == 1900 && d->character != ch){
                    sprintf(buf, "%s has ended the joust.\n\r", ch->name);
					send_to_char(buf, d->character);
				}
			}
		}

		send_to_char("You have ended the joust.\n\r", ch);
		return;
	}

	if (!str_prefix(arg1, "info")) {

        if (arg2[0] != '\0' || arg3[0] != '\0' || arg4[0] != '\0') {
            send_to_char("Syntax: joust info\n\r", ch);
            return;
        }

        send_to_char("==============================================================================\n\r", ch);
        send_to_char("                               Current Jousters                               \n\r", ch);
        send_to_char("==============================================================================\n\r", ch);
        if (jouster1 != NULL) {
            sprintf(buf, "Jouster 1: %s -- %d HP\n\r", jouster1->name, jouster1->hit);
		send_to_char(buf, ch);
		}

        if (jouster2 != NULL) {
            sprintf(buf, "Jouster 2: %s -- %d HP\n\r", jouster2->name, jouster2->hit);
		send_to_char(buf, ch);
		}

        if (jouster1 == NULL && jouster2 == NULL) {
            send_to_char("There is currently no joust in progress.\n\r", ch);
        }
        send_to_char("==============================================================================\n\r", ch);

        return;
    }

}


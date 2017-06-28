#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN( do_help		);

void do_interest( CHAR_DATA* ch, char* argument )
{
  if( IS_ADMIN( ch ) && !str_cmp( argument, "reset" ) )
    {
      send_to_char( "You have reset the amount of gold and silver collected on fees.\n\r", ch );
      wiznet( "$N flushes out the fee counts.", ch, NULL, WIZ_COMMERCE, 0, get_trust(ch) );
      bank_interest_total_gold = 0;
      bank_interest_total_silver = 0;
      return;
    }
  
  if( IS_NPC( ch ) )
    {
      send_to_char( "Only players need worry about interest rates.\n\r", ch );
      return;
    }
  
  if( !IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
    {
      send_to_char( "You must be in a bank to find the current rates.\n\r", ch );
      return;
    }

  if( bank_interest == 0 )
    {
      send_to_char( "Your lucky day!  The bank is charging no fees.  Limited time offer!\n\r", ch );
    }
  else
    {
      printf_to_char( ch, "Otho reports that the current fee is %d%% on deposits.\n\r", bank_interest );
    }
  if( IS_ADMIN( ch ) )
    {
      printf_to_char( ch, "He has also skimmed a total of %ld gold and %ld silver off the top of deposits.\n\r",
		      bank_interest_total_gold, bank_interest_total_silver );
    }
}

void do_account (CHAR_DATA *ch, char *argument)
{
  long gold = 0, silver = 0;
  char buf[MAX_STRING_LENGTH];

  /* No NPC's, No pets, No imms,
   * No chainmail, No service! 
   */
  if (IS_NPC(ch))
    {
      send_to_char("Only players need money!\n\r", ch);
      return;
    }
  gold = ch->pcdata->gold_bank;
  silver = ch->pcdata->silver_bank;

  sprintf( buf,"Your Arinock Account records show:\n\rYou have in your beltpouch:\n\r"
	   "  Gold:   %10ld\n\r  Silver: %10ld\n\r" 
	   "In your Account:\n\r"
	   "  Gold:   %10ld\n\r  Silver: %10ld\n\r",ch->gold, ch->silver,gold, silver); 

  send_to_char(buf, ch);
  return;
}


void do_deposit (CHAR_DATA *ch, char *argument)
{
  long amount = 0;
  long deposit = 0;
  long interest = 0;

  long amountGold = 0;
  long depositGold = 0;
  long interestGold = 0;

  long amountSilver = 0;
  long depositSilver = 0;
  long interestSilver = 0;

  char arg1[ MAX_INPUT_LENGTH ] = "";
  char arg2[ MAX_INPUT_LENGTH ] = "";
  char buf[ MAX_STRING_LENGTH ] = "";

  if( IS_NPC( ch ) )
    {
      send_to_char( "Only players need money!\n\r", ch );
      return;
    }

  if( !IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
    {
      send_to_char( "You must be in a bank to make a deposit.\n\r", ch );
      return;
    }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if( IS_NULLSTR( arg1 ) )
    {
      send_to_char( "How much would you like to squirrel away?\n\r", ch );
      return;
    }

  if( !str_cmp( arg1, "all" ) )
    {
      amountGold = ch->gold;
      depositGold = amountGold * ( 100 - bank_interest ) / 100;
      interestGold = amountGold - depositGold;

      amountSilver = ch->silver;
      depositSilver = amountSilver * ( 100 - bank_interest ) / 100;
      interestSilver = amountSilver - depositSilver;
    }
  else if( is_number( arg1 ) )
    {
      amount = atoi( arg1 );
      deposit = amount * ( 100 - bank_interest ) / 100;
      interest = amount - deposit;

      if( !str_cmp( arg2, "gold" ) )
	{
	  if( ch->gold < amount )
	    {
	      send_to_char( "You don't have that much gold.\n\r", ch );
	      return;
	    }

	  amountGold = amount;
	  depositGold = deposit;
	  interestGold = interest;
	}
      else if( !str_cmp( arg2, "silver" ) )
	{
	  if( ch->silver < amount )
	    {
	      send_to_char( "You don't have that much silver.\n\r", ch );
	      return;
	    }

	  amountSilver = amount;
	  depositSilver = deposit;
	  interestSilver = interest;
	}
      else
	{
	  send_to_char( "You can deposit either gold or silver.\n\r", ch );
	  return;
	}
    }
  else /* you can either deposit all or deposit x gold/silver...this is neither */
    {
      send_to_char( "You can either 'deposit all' your coin, 'deposit X gold', or 'deposit X silver'\n\r", ch );
      return;
    }

  /* now process the deposit */

  if( depositGold == 0 && depositSilver == 0 )
    {
      send_to_char( "You don't have enough coin to deposit.\n\r", ch );
      return;
    }

  if( depositGold > 0 )
    {
      ch->gold -= amountGold;
      ch->pcdata->gold_bank += depositGold;

      act( "$n deposits some gold into $s account.", ch, NULL, NULL, TO_ROOM );

      sprintf( buf, "You have deposited %ld gold and paid %ld gold in fees.\n\r", depositGold, interestGold );
      send_to_char( buf, ch );

      if( LONG_MAX - bank_interest_total_gold < interestGold )
	{
	  bug( "Overflow on Otho's skimmed gold! Resetting from %ld!", bank_interest_total_gold );
	  bank_interest_total_gold = interestGold;
	}
      else
	{
	  bank_interest_total_gold += interestGold;
	}
    }

  if( depositSilver > 0 )
    {
      ch->silver -= amountSilver;
      ch->pcdata->silver_bank += depositSilver;

      act( "$n deposits some silver into $s account.", ch, NULL, NULL, TO_ROOM );

      sprintf( buf, "You have deposited %ld silver and paid %ld silver in fees.\n\r", depositSilver, interestSilver );
      send_to_char( buf, ch );

      if( LONG_MAX - bank_interest_total_silver < interestSilver )
	{
	  bug( "Overflow on Otho's skimmed silver! Resetting from %ld!", bank_interest_total_silver );
	  bank_interest_total_silver = interestSilver;
	}
      else
	{
	  bank_interest_total_silver += interestSilver;
	}
    }
}


void do_withdraw (CHAR_DATA *ch, char *argument)
{
  long amount = 0;

  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  /* No NPC's, No pets, No imms,
   * No chainmail, No service! 
   */
  if (IS_NPC(ch))
    {
      send_to_char("Only players need money!\n\r", ch);
      return;
    }

  if( !IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
    {
      send_to_char( "You must be at a bank to make a withdrawal.\n\r", ch );
      return;
    }
  else  /* In the Bank */
    {
      argument = one_argument( argument, arg1 );
      argument = one_argument( argument, arg2 );
      if (arg1[0] == '\0'|| arg2[0] == '\0')
	{
	  send_to_char("How much would you like to withdraw?\n\r", ch );
/*  send_to_char("Withdraw <value> gold\n\r",ch);
	  send_to_char("     Withdraw <value> silver\n\r",ch);
	  send_to_char("For more information Type 'Help Bank'.\n\r",ch);
*/
	  return;
	}
      if( IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
	{
            if( is_number( arg1 ) )
	      {
		amount = atoi(arg1);

		if ( amount <= 0 )
		  {
		    send_to_char( "You must withdraw a sensible amount!\n\r", ch );
/*                  send_to_char( "     For information Type 'Bank'.\n\r",ch);*/
		    return;
		  }
		if(!str_cmp( arg2, "gold")) 
		  {
		    if (ch->pcdata->gold_bank < amount)
		      {
			send_to_char("You don't have that much gold squirreled away.\n\r",ch);
			return;
		      }
		    else 
		      {
			ch->pcdata->gold_bank -= amount;
			ch->gold += amount;
			act("$n withdraws gold from $s account.", ch,NULL,NULL, TO_ROOM);
			sprintf( buf, "You have withdrawn %ld gold.\n\r   Account: %10ld.\n\r   Beltpouch: %8ld.\n\r", amount, ch->pcdata->gold_bank, ch->gold);
			send_to_char( buf, ch);
			return;
		      }
		  }  
		if(!str_cmp( arg2, "silver")) 
		  {
		    if (ch->pcdata->silver_bank < amount)
		      {
			send_to_char("You don't have that much silver squirreled away.\n\r",ch);
			return;
		      }
		    else 
		      {
			ch->pcdata->silver_bank -= amount;
			ch->silver += amount;
			act("$n withdraws silver from $s account.", ch,NULL,NULL, TO_ROOM);
			sprintf( buf, "You have withdrawn %ld silver.\n\r   Account: %10ld.\n\r   Beltpouch: %8ld.\n\r", amount, ch->pcdata->silver_bank, ch->silver);
			send_to_char( buf, ch);
			return;
		      }
		  }  
	      }
            else
	      {
		send_to_char("The type of currency must be stated after the amount.\n\r",ch);
	      }
	}

      else
	{
	  bug( "Do_withdraw: Bank doesn't exist.", 0 );
	  send_to_char( "Bank doesn't exist.\n\r", ch );
	  return;
	} 
    }
  return;
}


void do_change (CHAR_DATA *ch, char *argument)
{
   int amount = ch->silver, change = 0, remainder = 0;
   char buf [MAX_STRING_LENGTH];
  
   if( !IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
   {
      send_to_char("You must be in a bank to exchange currency.\n\r",ch);
      return;
   }
   
   
   else  /* In the Bank */
   {
      /* No NPC's, No pets, No imms,
       * No chainmail, No service! 
       */
      if (IS_NPC(ch))
      {
         send_to_char("Only players need to exchange currency!\n\r", ch);
         return;
      } 

      if( IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
      {
	  
	  if (amount < 100)
	  {
	    send_to_char("Nope.\n\r", ch);
        return;
	  }
                  change = amount/100;
				  /*THANKS ZEPANA
				 ** WITHOUT YOUR HELP SOME POOR INNOCENT SOUL MAY HAVE
				 ** BEEN SHORT CHANGED 40, PERHAPS EVEN 80 SILVER.
				 ** OMG WHAT IS MY LIFE ABOUT.
				  */
				  remainder = amount - (change*100);
                  ch->gold += change;
                  ch->silver -= amount;
				  ch->silver += remainder;
                  sprintf( buf, "You have changed %d silver.\n\rYou have received %d gold.\n\rYou have received %d silver.\n\r", amount, change, remainder);
                  send_to_char( buf, ch);
                  return;
            }
   } 
   
   return;
}

/*
 * Bank account data--for manipulating guild bank accounts and accumulated cash
 * while not logged in.
 *
 */
void acct_to_char( BANKACCT_DATA *acct, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    int net;

    if ( IS_NPC(ch) ) return;
    if ( (net = 100*acct->gold + acct->silver) == 0 ) return;

    if ( net > 0 )
    {
        if ( acct->gold == 0 ) 
            sprintf( buf, 
            "%ld silver has been deposited into your bank account.\n\r",
            acct->silver );
        else if ( acct->silver == 0 ) 
            sprintf( buf, 
             "%ld gold has been deposited into your bank account.\n\r",
            acct->gold );
        else
            sprintf( buf, 
       "%ld gold and %ld silver has been deposited into your bank account.\n\r",
            acct->gold, acct->silver );

    }
    else
    {
        if ( acct->gold == 0 ) sprintf( buf, 
            "%ld silver has been billed from your bank account.\n\r",
            -1 * acct->silver );
        else if ( acct->silver == 0 ) sprintf( buf, 
             "%ld gold has been billed from your bank account.\n\r",
            -1 * acct->gold );
        else
            sprintf( buf, 
          "%ld gold and %ld silver has been billed from your bank account.\n\r",
            -1 * acct->gold, -1 * acct->silver );
    }
    send_to_char( buf, ch );
    ch->pcdata->gold_bank += acct->gold;
    ch->pcdata->silver_bank += acct->silver;
    ch->pcdata->proceeds += net;
    acct->gold = 0;
    acct->silver = 0;
    global_bankacct_changed = TRUE;

    if ( ch->pcdata->gold_bank < 0 ) ch->pcdata->gold_bank = 0;
    if ( ch->pcdata->silver_bank < 0 ) ch->pcdata->silver_bank = 0;

    /* unlink the empty account */
    if ( acct == global_bankacct_list )
    {
        global_bankacct_list = acct->next;
        free_bankacct( acct );
    }
    else
    {
        BANKACCT_DATA *prev;
        for (prev = global_bankacct_list; prev != NULL; prev = prev->next)
            if ( prev->next && prev->next == acct )
                break;
        if ( prev == NULL )
        {
            bug( "acct_to_char: need to unlink acct, but can't find it", 0 );
        }
        else
        {
            prev->next = acct->next;
            free_bankacct( acct );
        }
    }

    return;
}

/* principal routine to interact with bankaccts */
BANKACCT_DATA *modify_acct( char *owner, long gold, long silver )
{
    BANKACCT_DATA *acct;
    int net;

    if ( (net = 100*gold + silver) == 0 ) return NULL;

    for ( acct = global_bankacct_list; acct != NULL; acct = acct->next )
        if ( is_name(owner, acct->owner) )
            break;

    /* if this is a new account, create it */
    if ( acct == NULL )
    {
        acct = new_bankacct();
        acct->owner  = str_dup( owner );
        acct->type   = ACCT_PERSONAL;
        acct->gold   = gold;
        acct->silver = silver;
        acct->next   = global_bankacct_list;
        global_bankacct_list = acct;
    }
    else
    {
        acct->gold += gold;
        acct->silver += silver;
    }
    global_bankacct_changed = TRUE;

    /* unlink if empty */
    if ( acct->type == ACCT_PERSONAL 
    &&   acct->gold == 0 
    &&   acct->silver == 0 )
    {
        if ( acct == global_bankacct_list )
        {
            global_bankacct_list = acct->next;
            free_bankacct( acct );
        }
        else
        {
            BANKACCT_DATA *prev;
            for (prev = global_bankacct_list; prev != NULL; prev = prev->next)
                if ( prev->next && prev->next == acct )
                    break;
            if ( prev == NULL )
                bug( "acct_to_char: can't find it zero acct to unlink", 0 );
            else
            {
                prev->next = acct->next;
                free_bankacct( acct );
            }
        }
        return NULL;
    }

    return acct;
}

/* load bank records from disk.  call only once on bootup */
void load_bankaccts( void )
{
    BANKACCT_DATA *acct;
    FILE* fp;

    if ( (fp = fopen( BANKACCT_FILE, "r" )) != NULL )
    {
        while ( !feof( fp ) )
        {
            acct = new_bankacct();
            acct->owner = str_dup( fread_word(fp) );
            acct->type = fread_number(fp);
            acct->gold = fread_number(fp);
            acct->silver = fread_number(fp);

            acct->next = global_bankacct_list;
            global_bankacct_list = acct;

            fread_to_eol( fp );
        }
    }
    return;
}

/* save bank records to disk */
void save_bankaccts( void )
{
    BANKACCT_DATA *acct;
    FILE* fp;
    bool found = FALSE;

    fclose( fpReserve );

    if ( ( fp = fopen( BANKACCT_FILE, "w" ) ) == NULL )
    {
        perror( BANKACCT_FILE );
        return;
    }

    for ( acct = global_bankacct_list; acct != NULL; acct = acct->next )
    {
        if ( (acct->gold + acct->silver) != 0 )
        {
            found = TRUE;
            fprintf( fp, "%-20s %3d %10ld %10ld\n",
                acct->owner,
                acct->type,
                acct->gold,
                acct->silver );
	}
    }

    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );

    if ( !found )
        unlink( BANKACCT_FILE );
    return;
}

/* show list of bank accounts to admin */
void do_bankaccts( CHAR_DATA *ch, char *argument )
{
    BANKACCT_DATA *acct;
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "%-20s %3s %10s %10s\n\r",
        "Owner", "Typ", "Gold", "Silver" );
    send_to_char( buf, ch );

    for ( acct = global_bankacct_list; acct != NULL; acct = acct->next )
    {
        if ( (acct->gold + acct->silver) != 0 )
        {
            sprintf( buf, "%-20s %3d %10ld %10ld\n\r",
                acct->owner,
                acct->type,
                acct->gold,
                acct->silver );
            send_to_char( buf, ch );
	}
    }

    return;
}

/* admins can modify any account record */
void do_modacct( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    char owner[MAX_INPUT_LENGTH];
    long amount;

    argument = one_argument(argument, owner);
    if ( IS_NULLSTR(owner) )
    {
        send_to_char("Syntax: modacct <owner> <amount> <gold|silver>\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);
    if ( (amount = atol(arg)) == 0 )
    {
        send_to_char( "There's no point.\n\r", ch );
        return;
    }

    if ( LOWER(*argument) == 'g' )
    {
        modify_acct( owner, amount, 0 );
        sprintf(arg, 
            "You arrange to have %ld gold deposited into %s's account.\n\r", 
            amount, owner);
    }
    else
    {
        modify_acct( owner, 0, amount );
        sprintf(arg, 
            "You arrange to have %ld silver deposited into %s's account.\n\r", 
            amount, owner);
    }
    send_to_char( arg, ch );
    return;
}

/* Withdraw money from guild account */
void do_gwithdraw (CHAR_DATA *ch, char *argument)
{
    BANKACCT_DATA *acct;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char crhack[] = "crimson";  /* because CR has a two-word name */
    char *owner;

    long amount = 0;

    if ( IS_NPC(ch) )
    {
        send_to_char( "Only players need money!\n\r", ch);
        return;
    }
    if ( !is_guild(ch) )
    {
        send_to_char( "You aren't in a guild.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( IS_NULLSTR(arg1) || IS_NULLSTR(arg2) )
    {
        send_to_char("How much would you like to withdraw?\n\r", ch );
        return;
    }

    amount = atol(arg1);

    if ( amount <= 0 )
    {
        send_to_char( "You must withdraw a sensible amount!\n\r", ch );
        return;
    }

    owner = guild_table[ch->guild].name;
    if ( !strcmp(owner,"crimson rain") )
        owner = crhack;
    for ( acct = global_bankacct_list; acct != NULL; acct = acct->next )
        if ( is_name(owner, acct->owner) )
            break;

    if ( acct == NULL )
    {
        send_to_char( "Your guild has no money in reserve.\n\r", ch );
        return;
    }

    if ( !str_prefix( arg2, "gold") )
    {
        if (acct->gold < amount)
        {
            send_to_char( "Your guild doesn't have that much gold.\n\r", ch );
            return;
        }
        else 
        {
            acct = modify_acct( owner, -1 * amount, 0 );
            ch->gold += amount;
            sprintf( buf, "You have withdrawn %ld gold.\n\r   Account: %10ld.\n\r   Beltpouch: %8ld.\n\r", 
                amount, 
                ( acct ? acct->gold : 0 ), 
                ch->gold );
            send_to_char( buf, ch);
            return;
        }
    }  
    else
    {
        if (acct->silver < amount)
        {
            send_to_char( "Your guild doesn't have that much silver.\n\r", ch );
            return;
        }
        else 
        {
            acct = modify_acct( owner, 0, -1 * amount );
            ch->silver += amount;
            sprintf( buf, "You have withdrawn %ld silver.\n\r   Account: %10ld.\n\r   Beltpouch: %8ld.\n\r", 
                amount, 
                (acct ? acct->silver : 0 ), 
                ch->silver );
            send_to_char( buf, ch);
            return;
        }
    }
    return;
}

void do_gdeposit (CHAR_DATA *ch, char *argument)
{
    BANKACCT_DATA *acct;
    long amount = 0;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char crhack[] = "crimson";  /* because CR has a two-word name */
    char *owner;

    if ( IS_NPC(ch) )
    {
        send_to_char( "Only players need money!\n\r", ch );
        return;
    }

    if ( !is_guild(ch) )
    {
        send_to_char( "You aren't in a guild.\n\r", ch );
        return;
    }
   
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("How much do you want to squirrel away?\n\r", ch );
        return;
    }

    owner = guild_table[ch->guild].name;
    if ( !strcmp(owner,"crimson rain") )
        owner = crhack;

    if ( (amount = atol(arg1)) < 1 )
    {
        send_to_char( "You must deposit a sensible amount!\n\r", ch );
        return;
    }

    if ( !str_prefix(arg2, "gold") )
    {
        if (ch->gold < amount)
        {
            send_to_char("You don't have that much gold.\n\r",ch);
            return;
        }
        else 
        {
            acct = modify_acct( owner, amount, 0 );
            if ( !acct )
            {
                bug( "do_gdeposit: acct doesn't exist after depositing", 0 );
                return;
            }
            acct->type = ACCT_GUILD;
            ch->gold -= amount;
            sprintf( buf, "You have deposited %ld gold.\n\r   Account: %10ld.\n\r   Beltpouch: %8ld.\n\r",
                amount, 
                acct->gold, 
                ch->gold );
	    send_to_char( buf, ch );
	    return;
	}
    }
    else
    {
        if (ch->silver < amount)
        {
            send_to_char("You don't have that much silver.\n\r",ch);
            return;
        }
        else 
        {
            acct = modify_acct( owner, 0, amount );
            if ( !acct )
            {
                bug( "do_gdeposit: acct doesn't exist after depositing", 0 );
                return;
            }
            acct->type = ACCT_GUILD;
            ch->silver -= amount;
            sprintf( buf, "You have deposited %ld silver.\n\r   Account: %10ld.\n\r   Beltpouch: %8ld.\n\r", 
                amount, 
                acct->silver, 
                ch->silver );
            send_to_char( buf, ch );
            return;
        }
    }
    return;
}

void do_gaccount( CHAR_DATA *ch, char *argument )
{
    BANKACCT_DATA *acct;
    long gold = 0, silver = 0;
    char buf[MAX_STRING_LENGTH];
    char crhack[] = "crimson";  /* because CR has a two-word name */
    char *owner;


    if ( IS_NPC(ch) )
      return;

    if ( !is_guild(ch) )
    {
        send_to_char( "You aren't in a guild.\n\r", ch );
        return;
    }

    owner = guild_table[ch->guild].name;
    if ( !strcmp(owner,"crimson rain") )
        owner = crhack;
    for ( acct = global_bankacct_list; acct != NULL; acct = acct->next )
        if ( is_name(owner, acct->owner) )
            break;
    if ( acct == NULL )
    {
        gold = 0;
        silver = 0;
    }
    else
    {
        gold = acct->gold;
        silver = acct->silver;
    }

    sprintf( buf,"Your Arinock Account records show:\n\rYou have in your beltpouch:\n\r"
	   "  Gold:   %10ld\n\r  Silver: %10ld\n\r" 
	   "In your guild account:\n\r"
	   "  Gold:   %10ld\n\r  Silver: %10ld\n\r",
           ch->gold, ch->silver, gold, silver ); 
    send_to_char(buf, ch);
    return;
}

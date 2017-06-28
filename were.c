/***************************************************************************
 * Were creatures by Sharon P. Goza 2/11/99
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"

void frog_unshift( CHAR_DATA* ch, char* argument )
{
  send_to_char( "Immortal wrath against you has subsided.\n\r", ch );
  act( "$n shimmers in bright red hues and returns to normal form.", ch, NULL, NULL, TO_ROOM );
}


void vampire_shift( CHAR_DATA *ch, char *argument );
void do_shift( CHAR_DATA *ch, char *argument )
{
  char buf[250];
  char arg1[MAX_INPUT_LENGTH];
  AFFECT_DATA af;
  int sn;


  if ( IS_NPC( ch ) )
  {
      send_to_char( "Mobiles can't change shape.\n\r", ch );
      return;
  }

  sn = skill_lookup( "Tyrins Wrath" );

  if( sn == -1 )
    {
      bug( "do_shift(): bad lookup for Tyrin's Wrath", 0 );
    }
  else if( is_affected( ch, sn ) )
    {
      send_to_char( "You cannot shift while Tyrin's wrath is upon you!\n\r", ch );
      return;
    }

   if (IS_SET(ch->shiftbits, TEMP_VAMP) || IS_SET(ch->shiftbits, PERM_VAMP))
   {
      vampire_shift(ch,argument);
      return;
   }

   argument = one_argument( argument, arg1 );

/* if ( !IS_SET(ch->shiftbits,SHIFT_POTENTIAL)
   || ch->pcdata->cleansings > 2 ) */
   if ( !IS_WERECREATURE( ch ) )
   {
       send_to_char("Try as you might, you cannot change forms.\n\r",ch);
       return;
   }

   if (arg1[0] == '\0'){
      send_to_char("Shift to half form or full form?\n\r",ch);
      return;
      }

   if (IS_SET(ch->shiftbits,SHIFT_HALF) || IS_SET(ch->shiftbits,SHIFT_FULL)){
      send_to_char("You are already in an alternate form.\n\r",ch);
      return;
      }
   
   if (IS_AFFECTED(ch,AFF_SHIFT_SICK)){
      send_to_char("You are too tired to shift.\n\r",ch);
      return;
      }

   if (!strcasecmp(arg1,"full")){
     /* FULL SHIFT */
     sprintf(buf,"$n shimmers and before you stands %s!",
        were_table[ch->race].short_full);
     act(buf,ch,NULL,NULL,TO_ROOM);

     sprintf(buf,"You shimmer and shift into %s!\n\r",
        were_table[ch->race].short_full);
     send_to_char(buf,ch);

     SET_BIT(ch->shiftbits,SHIFT_FULL);

     ch->max_hit += were_table[ch->race].hpadd;
     ch->max_mana += were_table[ch->race].manaadd;
     ch->res_flags |= were_table[ch->race].res_full;
     ch->vuln_flags |= were_table[ch->race].vuln_full;
     ch->affected_by |= were_table[ch->race].aff_full;
     /*ch->mod_stat[STAT_STR] += were_table[ch->race].str_full;
     ch->mod_stat[STAT_DEX] += were_table[ch->race].dex_full;*/
     
     af.where = TO_AFFECTS;
     af.type = -1;
     af.level = 30;
     af.duration = 15;
     af.modifier = 0;
     af.location = APPLY_NONE;
     af.bitvector = AFF_SHIFT_ON;
     affect_to_char(ch,&af);
     }
   else if (!strcasecmp(arg1,"half")){
     /* HALF SHIFT */
     sprintf(buf,"$n shimmers and before you stands %s!",
        were_table[ch->race].short_half);
     act(buf,ch,NULL,NULL,TO_ROOM);

     sprintf(buf,"You shimmer and shift into %s!\n\r",
        were_table[ch->race].short_half);
     send_to_char(buf,ch);

     SET_BIT(ch->shiftbits,SHIFT_HALF);

	/* SET WERE AFFECTS */
     ch->max_hit += were_table[ch->race].hpadd;
     ch->max_mana += were_table[ch->race].manaadd;
     ch->res_flags |= were_table[ch->race].res_half;
     ch->vuln_flags |= were_table[ch->race].vuln_half;
     ch->affected_by |= were_table[ch->race].aff_half;
     /*ch->mod_stat[STAT_STR] += were_table[ch->race].str_half;
     ch->mod_stat[STAT_DEX] += were_table[ch->race].dex_half;*/
     
     af.where = TO_AFFECTS;
     af.type = -1;
     af.level = 30;
     af.duration = 15;
     af.modifier = 0;
     af.location = APPLY_NONE;
     af.bitvector = AFF_SHIFT_ON;
     affect_to_char(ch,&af);
     }
   else {
     send_to_char("That shift is impossible.\n\r",ch);
     }
}

void were_chance(CHAR_DATA *ch)
{
  short chance;
 
  chance = number_range(1,100);
  
  if( ch->level > 50 )
    {
      return;
    }

  if ( IS_SET(ch->shiftbits, SHIFT_NOFORCE) )
  {
      send_to_char("The beast within you remains subdued.\n\r",ch);
      return;
  }
//REMOVE_BIT(ch->shiftbits,SHIFT_FULL);

  if (chance < 21)
/*  if ( chance ) */
    {
      send_to_char("The beast rages within you and you lose control!\n\r",ch);
      shift_strip(ch);
      were_night(ch);
      return;
    }
  else
    {
      send_to_char("The beast rages within but you retain control..for now...\n\r",ch);
      return;
    }
}


void were_night(CHAR_DATA *ch)
{
  char buf[250];
  AFFECT_DATA af;
  CHAR_DATA *vch;
  short chance;
  short pccount = 0, iter=0;

  chance = number_range(1,100);


  if (chance < 51)
    {
      /* FULL SHIFT */
      SET_BIT(ch->affected_by,AFF_SHIFT_PERM);
      sprintf(buf,"$n shimmers and before you stands %s!",
	      were_table[ch->race].short_full);
      act(buf,ch,NULL,NULL,TO_ROOM);
      
      sprintf(buf,"You shimmer and shift into %s!\n\r",
	      were_table[ch->race].short_full);
      send_to_char(buf,ch);

      SET_BIT(ch->shiftbits,SHIFT_FULL);

      ch->max_hit += were_table[ch->race].hpadd;
      ch->max_mana += were_table[ch->race].manaadd;
      ch->res_flags |= were_table[ch->race].res_full;
      ch->vuln_flags |= were_table[ch->race].vuln_full;
      ch->affected_by |= were_table[ch->race].aff_full;
      /*ch->mod_stat[STAT_STR] += were_table[ch->race].str_full;
	ch->mod_stat[STAT_DEX] += were_table[ch->race].dex_full;*/
     
      af.where = TO_AFFECTS;
      af.type = -1;
      af.level = 30;
      af.duration = -1;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_SHIFT_ON;
      affect_to_char(ch,&af);
      shift_strip2(ch);
    }
  else
    {
      /* HALF SHIFT */
      SET_BIT(ch->affected_by,AFF_SHIFT_PERM);
      sprintf(buf,"$n shimmers and before you stands %s!",
	      were_table[ch->race].short_half);
      act(buf,ch,NULL,NULL,TO_ROOM);
      
      sprintf(buf,"You shimmer and shift into %s!\n\r",
	      were_table[ch->race].short_half);
      send_to_char(buf,ch);
    
      SET_BIT(ch->shiftbits,SHIFT_HALF);
    
      /* SET WERE AFFECTS */
      ch->max_hit += were_table[ch->race].hpadd;
      ch->max_mana += were_table[ch->race].manaadd;
      ch->res_flags |= were_table[ch->race].res_half;
      ch->vuln_flags |= were_table[ch->race].vuln_half;
      ch->affected_by |= were_table[ch->race].aff_half;
      /*ch->mod_stat[STAT_STR] += were_table[ch->race].str_half;
	ch->mod_stat[STAT_DEX] += were_table[ch->race].dex_half;*/
     
      af.where = TO_AFFECTS;
      af.type = -1;
      af.level = 30;
      af.duration = -1;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_SHIFT_ON;
      affect_to_char(ch,&af);
      shift_strip2(ch);
    }

/* Nightshift autoattack code.  Nasty! */
    if ( !IS_SET(ch->act_bits, PLR_NEWBIE ) ) {
/*  Count how many PCs are in the room other than the player */
    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
        if ( !IS_NPC(vch) 
        &&    vch != ch 
        &&   !is_safe_verb( ch, vch, FALSE )
        &&   (abs(ch->level - vch->level) <=6)
        &&   !IS_IMMORTAL( vch )
        &&   !IS_SET(vch->act_bits, PLR_NEWBIE ) )
            pccount++;

    if ( pccount > 0 )
    {
        pccount = number_range(1,pccount);
        vch = ch->in_room->people;
        iter = 0;
/*      This loop goes through all of the people in an endless loop until the
        chance'th valid target is reached. */
        do
        {
            if ( vch == NULL ) vch = ch->in_room->people;

            if ( !IS_NPC(vch)
            &&    vch != ch
            &&   !is_safe_verb( ch, vch, FALSE )
            &&   (abs(ch->level - vch->level) <=6)
            &&   !IS_IMMORTAL( vch )
            &&   !IS_SET(vch->act_bits, PLR_NEWBIE ) )
                iter++;

            if ( iter < pccount )
                vch = vch->next_in_room;

            if ( iter > 1000 ) 
            {
                bug ( "were_night: runaway attack loop", 0 );
                return;
            }
        } while ( iter < pccount );

        act( "$n lunges at $N!", ch, NULL, vch, TO_NOTVICT );
        act( "You savagely lunge at $N!", ch, NULL, vch, TO_CHAR );
        act( "$n lunges at you!", ch, NULL, vch, TO_VICT );

        if ( !IS_NPC( ch ) && !IS_NPC( vch ) )
        {
            sprintf( buf, "%s forced to attack %s due to nightshift.", ch->name, vch->name );
            ch->pcdata->latelog = FALSE;
            vch->pcdata->latelog = FALSE;
            wiznet( buf, NULL, NULL, WIZ_DEATHS, 0, 0 );
            log_string(buf);
            multi_hit( vch, ch, TYPE_UNDEFINED );
        } else bug ( "were_night: ch or vch a mobile", 0 );
    }}
}


void unshift(CHAR_DATA *ch)
{
  AFFECT_DATA newaf;
  char buf[250];
  int sn;
  bool affected = FALSE;

  if (IS_NPC(ch))
    {
      send_to_char("Only PC's can shift and unshift at will.\n\r",ch);
      return;
    }
  
  if (IS_SET(ch->shiftbits,SHIFT_HALF)) 
    {
    
      ch->max_hit -= were_table[ch->race].hpadd;
      ch->max_mana -= were_table[ch->race].manaadd;
      
      ch->res_flags &= ~(were_table[ch->race].res_half);
      ch->vuln_flags &= ~(were_table[ch->race].vuln_half);
      ch->affected_by &= ~(were_table[ch->race].aff_half);
      /*ch->mod_stat[STAT_STR] -= were_table[ch->race].str_half;
	ch->mod_stat[STAT_DEX] -= were_table[ch->race].dex_half;*/
      sprintf(buf,"%s shimmers and returns to $s normal form.",
	      were_table[ch->race].short_half);
      act(buf,ch,NULL,NULL,TO_ROOM);
          
    send_to_char("You shimmer and return to your normal form.\n\r",ch);
  }
  else {
    
    ch->max_hit -= were_table[ch->race].hpadd;
    ch->max_mana -= were_table[ch->race].manaadd;
    
    ch->res_flags &= ~(were_table[ch->race].res_full);
    ch->vuln_flags &= ~(were_table[ch->race].vuln_full);
    ch->affected_by &= ~(were_table[ch->race].aff_full);
    /*ch->mod_stat[STAT_STR] -= were_table[ch->race].str_full;
      ch->mod_stat[STAT_DEX] -= were_table[ch->race].dex_full;*/
    sprintf(buf,"%s shimmers and returns to $s normal form.",
	    were_table[ch->race].short_full);
    act(buf,ch,NULL,NULL,TO_ROOM);
    
    send_to_char("You shimmer and return to your normal form.\n\r",ch);
  }
  
  /* RESTORE JUST IN CASE THEY WERE TAKEN AWAY */
  ch->res_flags |= race_table[ch->race].res;
  ch->vuln_flags |= race_table[ch->race].vuln;
  ch->affected_by |= race_table[ch->race].aff;
  
  sn = skill_lookup( "Tyrins wrath" );

  if( sn == -1 )
    {
      bug( "unshift(): bad lookup for Tyrin's wrath", 0 );
    }
  else
    {
      affected = is_affected( ch, sn );
    }

  newaf.where = TO_AFFECTS;
  newaf.type = -2;
  newaf.level = 30;
  newaf.duration = ( affected ? 5 : 10 );
  newaf.modifier = 0;
  newaf.location = APPLY_NONE;
  newaf.bitvector = AFF_SHIFT_SICK;
  affect_to_char(ch,&newaf);

  
  REMOVE_BIT(ch->affected_by, AFF_SHIFT_PERM);
  REMOVE_BIT(ch->shiftbits,SHIFT_FULL);
  REMOVE_BIT(ch->shiftbits,SHIFT_HALF);
  
}


void spell_unholyfire( int sn, int level, CHAR_DATA* ch, void* vo, int target )
{
    CHAR_DATA* victim = ( CHAR_DATA* )vo;
    AFFECT_DATA af;
    int dam = 0;
    static const sh_int dam_each[] =
    {
        0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,  30,  35,  40,  45,  50,  55,
       60,  65,  70,  75,  80,  82,  84,  86,  88,  90,
       92,  94,  96,  98, 100, 102, 104, 106, 108, 110,
      112, 114, 116, 118, 120, 122, 124, 126, 128, 130
    };

    if ( !IS_WERECREATURE( ch ) )
    {
        send_to_char( "You mispronounce an arcane syllable.\n\r", ch );
        return;
    }

    if ( !IS_SET( ch->shiftbits, SHIFT_HALF ) 
    &&   !IS_SET( ch->shiftbits, SHIFT_FULL ) )
    {
        send_to_char( 
     "You are not influenced enough by the moon to call forth unholy fire.\n\r",
            ch );
        return;
    }

    if ( ch == victim )
    {
        send_to_char( 
            "It would be folly to call down unholy fire upon yourself.\n\r", 
            ch );
        return;
    }


    level = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[ 0 ] ) - 1 );
    level = UMAX( 0, level );

    if ( !saves_spell( level, ch, victim, DAM_DISEASE ) 
    &&   !is_affected( victim, sn )
    && ( !IS_NPC( victim ) || !IS_SET( victim->act_bits, ACT_UNDEAD ) ) )
	{
        send_to_char( 
            "The unholy flame causes your skin to rebel in outrage!\n\r", 
            victim );
        act( 
          "Unholy fire envelops $n and $s skin erupts in rebellion against $m.",
	       victim, NULL, NULL, TO_ROOM );

        af.where = TO_AFFECTS;
        af.type = sn;
        af.level = level * 3 / 4;
        af.duration = level;
        af.location = APPLY_STR;
        af.modifier = -5;
        af.bitvector = AFF_PLAGUE;
        affect_join( victim, &af, FALSE );
	}

    dam = number_range( dam_each[ level ] / 2, dam_each[ level ] * 2 );
    damage( ch, victim, dam, sn, DAM_FIRE, TRUE );
    return;
}


void spell_fear( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    short chance;

    if ( !IS_IMMORTAL( ch ) && !IS_NPC( ch ) && !IS_WERECREATURE( ch ) )
    {
        send_to_char( "You mispronounce an arcane syllable.\n\r", ch );
        return;
    }
	
	if (!IS_SET(ch->shiftbits, SHIFT_HALF) &&
	!IS_SET(ch->shiftbits, SHIFT_FULL)) {
	send_to_char("You're not in the right form.\n\r",ch);
	return;
    }

    chance = number_range(1,100);

    victim = (CHAR_DATA *) vo;

    if ( IS_AFFECTED2(victim,AFF_FEAR) 
    ||   victim->race == race_lookup("kender") ) /* 7/3/2012 gkl */
        return; 

    if (saves_spell(level,ch,victim,DAM_MENTAL))
     {
       if(chance < 75)
       return;
     }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 4;
    af.location  = APPLY_HITROLL;
    af.modifier  = -5;
    af.bitvector = AFF_FEAR;
    affect_to_char2( victim, &af );

    af.location  = APPLY_DEX;
    af.modifier  = -2;
    affect_to_char2( victim, &af );

    send_to_char("You freeze in terror!\n\r",victim);
    if ( ch != victim )
        act("$N freezes in terror.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_call_spiders(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *mob;
  CHAR_DATA* victim;
  AFFECT_DATA af;
  MOB_INDEX_DATA *pmobindex;
  short indexnum = 10;

  victim = ( CHAR_DATA* )vo;

  if ( !IS_IMMORTAL( ch ) && !IS_NPC( ch ) && !IS_WERECREATURE( ch ) )
  {
    send_to_char( "You mispronounce an arcane syllable.\n\r", ch );
    return;
  }

  if (ch->pet != NULL || has_summonedpets(ch) )
    {
      send_to_char("You already have a creature following you.\n\r",ch);
      return;
    }

  pmobindex = get_mob_index(indexnum);

  if (!pmobindex){
        return;
        }

  if( IS_AFFECTED2( ch, AFF_SUMSICK ) )
    {
      send_to_char( "You cannot summon more spiders so quickly!\n\r", ch );
      return;
    }

    mob = create_mobile(pmobindex);
    char_to_room(mob,ch->in_room);

    mob->max_hit = .75 * ch->max_hit;
    mob->hit = mob->max_hit;
    mob->level = ch->level;

    act( "Spiders crawl out from every dark crevice and form a massive horde.",
        ch, NULL, NULL, TO_ROOM );
    act( "Spiders crawl out from every dark crevice and form a massive horde!",
        ch, NULL, NULL, TO_CHAR );
 
    add_follower(mob, ch);
    mob->leader = ch;
    ch->pet = mob;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( mob, &af );

    if( !IS_SET( ch->shiftbits, SHIFT_HALF ) && !IS_SET( ch->shiftbits, SHIFT_FULL ) )
    {
        spell_summon_sick(gsn_sumsick,level,ch,(void*)ch, TARGET_CHAR);
/*
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.whichaff = AFF3;
	af.duration = 7;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_WERESUMSICK;
	affect_to_char3( ch, &af );
*/
      }
   return;
}

void spell_summonmeerkats( int sn, int level, CHAR_DATA* ch, void* vo, int target )
{
  CHAR_DATA* mob = 0;
  MOB_INDEX_DATA* pmobindex = 0;
  short indexnum = 13;
  int i;
  AFFECT_DATA af;
  const static int DURATION = 10;
  char buf[MAX_STRING_LENGTH];

  if ( !IS_IMMORTAL( ch ) && !IS_NPC( ch ) && !IS_WERECREATURE( ch ) )
  {
    send_to_char( "You mispronounce an arcane syllable.\n\r", ch );
    return;
  }

  if( !IS_SET( ch->shiftbits, SHIFT_FULL ) && !IS_SET( ch->shiftbits, SHIFT_HALF ) )
    {
      send_to_char( "You are not in close enough touch with your inner meerkat.\n\r", ch );
      return;
    }
  if ( ch->pet != NULL || has_summonedpets(ch) )
  {
      send_to_char("You already have a creature following you.\n\r",ch);
      return;
  }

  if ( IS_AFFECTED2( ch, AFF_SUMSICK ) )
  {
      send_to_char( "You cannot summon more friends so quickly!\n\r", ch );
      return;
  }
 
  pmobindex = get_mob_index( indexnum );
  if( !pmobindex )
    {
      return;
    }

  send_to_char( "You summon a group of meerkats for playmates.\n\r", ch);
  act( "$n summons a group of meerkats out of sheer boredom.", ch, NULL, NULL, TO_ROOM );

  for( i = 0; i < 5; i++ )
    {
      mob = create_mobile( pmobindex );
      mob->max_hit = ch->max_hit * 3 / 15;
      mob->hit = mob->max_hit;
      mob->level = ch->level;

      char_to_room( mob, ch->in_room );
      add_follower( mob, ch );
      mob->leader = ch;
/*    ch->pet = mob;   <--- BAD BAD BAD!  MULTIPLE PETS = CRASHES! */
      SET_BIT( mob->act_bits, ACT_PET );
      SET_BIT( mob->affected_by2, AFF2_SUMMONEDPET );

      sprintf( buf, mob->name, ch->name );
      free_string( mob->name );
      mob->name = str_dup( buf );

      af.where     = TO_AFFECTS;
      af.type      = sn;
      af.level     = level;
      af.duration  = DURATION;
      af.location  = 0;
      af.modifier  = 0;
      af.bitvector = AFF_CHARM;
      affect_to_char( mob, &af );
    }
    spell_summon_sick(gsn_sumsick,level,ch,(void*)ch, TARGET_CHAR);
/*
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.whichaff = AFF2;
  af.duration = DURATION;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SUMSICK;
  affect_to_char2( ch, &af );
*/
}

void spell_call_water_spirit(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *mob;
  CHAR_DATA *victim;
  AFFECT_DATA af;
  MOB_INDEX_DATA *pmobindex;
  short indexnum = 11;

  victim = ( CHAR_DATA* )vo;

  if ( !IS_IMMORTAL( ch ) && !IS_NPC( ch ) && !IS_WERECREATURE( ch ) )
  {
    send_to_char( "You mispronounce an arcane syllable.\n\r", ch );
    return;
  }

  if (ch->pet != NULL || has_summonedpets( ch ))
    {
      send_to_char("You already have a creature following you.\n\r",ch);
      return;
    }


  pmobindex = get_mob_index(indexnum);

  if (!pmobindex)
    {
      return;
    }

  mob = create_mobile(pmobindex);
  char_to_room(mob,ch->in_room);

    mob->max_hit = .75 * ch->max_hit;
    mob->hit = mob->max_hit;
    mob->level = ch->level;

    act( "The mist in the air solidifes into $N.",
	ch, NULL, mob, TO_ROOM );
    act( "The mist in the air solidifes into $N!",
        ch, NULL, mob, TO_CHAR );

    add_follower(mob, ch);
    mob->leader = ch;
    ch->pet = mob;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( mob, &af );

    return;
}

void do_feed( CHAR_DATA *ch, char *argument)
{
    int chance, hp, mana;
    CHAR_DATA *victim;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
       send_to_char("You can't feed, you're DEAD!\n\r",ch);
       return;
       }

    if ((chance = get_skill(ch,gsn_feed)) == 0
    || (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_feed].were_level[ch->race]))
    {
        send_to_char("You don't have any fangs.\n\r",ch);
       return;
    }
   
    if (!IS_SET(ch->shiftbits, SHIFT_HALF) &&
	!IS_SET(ch->shiftbits, SHIFT_FULL)) {
	send_to_char("You're not in the right form.\n\r",ch);
	return;
    }


    if (argument[0] == '\0') {
       if ( ( victim = ch->fighting ) == NULL )
       {
	  send_to_char("Who do you want to feed on?\n\r",ch);
	  return;
       }
    }
    else if ( (victim = get_char_room(ch, argument, TRUE)) == NULL ) 
    {
	send_to_char("They aren't here.\n\r", ch );
	return;
    }
    
    if (victim == ch) {
	send_to_char("You can't feed on yourself.\n\r",ch);
	return;
    }

    if (IS_AFFECTED2(victim, AFF_GHOST)) {
      send_to_char("You pass right through them!\n\r", ch);
      return;
    }
    
    if ( is_safe( ch, victim ) )
      return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (saves_spell(ch->level, ch, victim, DAM_NEGATIVE)) {
        chance /= 2;
    }

    if (number_percent() < chance)
    {

        WAIT_STATE(ch,skill_table[gsn_feed].beats);
	act("$n feeds on your energy!",ch,NULL,victim,TO_VICT);
	act("You feed on $N's energy!",ch,NULL,victim,TO_CHAR);
	hp = 2 * number_range( 5, ch->level/2 );
	mana = number_range( 5, ch->level/2 );
	damage(ch,victim,hp,0,DAM_NEGATIVE,FALSE);
	ch->hit += hp;
	ch->hit = UMIN( ch->hit, ch->max_hit );
        ch->mana += mana;
        ch->mana =UMIN( ch->mana, ch->max_mana );
	check_improve(ch,gsn_feed,TRUE,2);
    }

    else
    {
        WAIT_STATE(ch,3 * skill_table[gsn_feed].beats / 2 );
        ch->move -= 15;
	damage(ch,victim,0,0,DAM_NEGATIVE,FALSE);
        send_to_char("You fail.\n\r",ch);
        check_improve(ch,gsn_feed,FALSE,2);
    }

}

void do_touch( CHAR_DATA *ch, char *argument)
{
    int chance, hp, mana;
    CHAR_DATA *victim;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
       send_to_char("You must be 'living' to use this!\n\r",ch);
       return;
       }

    if ((chance = get_skill(ch,gsn_touch)) == 0
    || (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_touch].were_level[ch->race]))
    {
        send_to_char("Huh?\n\r",ch);
       return;
    }
   
    if (!IS_SET(ch->shiftbits, SHIFT_HALF) &&
	!IS_SET(ch->shiftbits, SHIFT_FULL)) {
	send_to_char("You're not in the right form.\n\r",ch);
	return;
    }


    if (argument[0] == '\0') {
       if ( ( victim = ch->fighting ) == NULL )
       {
	  send_to_char("Whom would you like to bestow your touch upon?\n\r",ch);
	  return;
       }
    }
    else if ( (victim = get_char_room(ch, argument, TRUE)) == NULL ) 
    {
	send_to_char("They aren't here.\n\r", ch );
	return;
    }
    
    if (victim == ch) {
	send_to_char("You can't touch yourself with death.\n\r",ch);
	return;
    }

    if (IS_AFFECTED2(victim, AFF_GHOST)) {
      send_to_char("Your touch passes right through them!\n\r", ch);
      return;
    }
 
   if ( is_safe( ch, victim ) )
       return;

   if (victim->mana < 101)
     {
       send_to_char("You cannot drain anymore essence from your victim.\n\r",ch);
       return;
     }    
    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (saves_spell(ch->level, ch, victim, DAM_NEGATIVE)) {
        chance /= 2;
    }

    if (number_percent() < chance)
    {

        WAIT_STATE(ch,skill_table[gsn_feed].beats);
	act("$n's touch drains your energy!",ch,NULL,victim,TO_VICT);
	act("Your touch drains $N's energy!",ch,NULL,victim,TO_CHAR);
	hp = 2 * number_range( 5, ch->level/2 );
	mana = number_range( 5, ch->level/2 );
        victim->mana -= 100;
	damage(ch,victim,hp,0,DAM_NEGATIVE,FALSE);
	ch->hit += hp;
	ch->hit = UMIN( ch->hit, ch->max_hit );
        ch->mana += mana;
        ch->mana =UMIN( ch->mana, ch->max_mana );
	check_improve(ch,gsn_touch,TRUE,2);
    }

    else
    {
        WAIT_STATE(ch,3 * skill_table[gsn_touch].beats / 2 );
        ch->move -= 15;
	damage(ch,victim,0,0,DAM_NEGATIVE,FALSE);
        send_to_char("You fail.\n\r",ch);
        check_improve(ch,gsn_touch,FALSE,2);
    }

}

void spell_web( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {

  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( !IS_IMMORTAL( ch ) && !IS_NPC( ch ) && !IS_WERECREATURE( ch ) )
  {
    send_to_char( "You mispronounce an arcane syllable.\n\r", ch );
    return;
  }

  if ( IS_AFFECTED2(victim, AFF_WEB) )
    return;

  if (!IS_SET(ch->shiftbits, SHIFT_HALF) &&
      !IS_SET(ch->shiftbits, SHIFT_FULL)) {
       send_to_char("You're not in the right form.\n\r",ch);
      return;
     }

  if ( saves_spell( level, ch, victim, DAM_OTHER ) )
    return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 3;
    af.location  = APPLY_AC;
    af.modifier  = level;
    af.bitvector = AFF_WEB;
    affect_to_char2( victim, &af );
    send_to_char( "You are surrounded by sticky webs.\n\r", victim );
    act( "$n is surrounded by sticky webs.", victim, NULL, NULL, TO_ROOM );
    return;
}


void do_pounce( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    /*int victsize = 0;
    int charsize = 0;*/

    
    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    one_argument(argument,arg);
 
    if ( (chance = get_skill(ch,gsn_pounce)) == 0
    ||	 (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_pounce].were_level[ch->race]) )
    {	
	send_to_char("Pouncing?  What's that?\n\r",ch);
	return;
    }

    
    if (!IS_SET(ch->shiftbits, SHIFT_HALF) &&
	!IS_SET(ch->shiftbits, SHIFT_FULL)) {
	send_to_char("You're not in the right form.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg, TRUE)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    } 

    if (victim == ch)
    {
	send_to_char("How does one pounce themselves?\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;
    if(IS_SET(victim->imm_flags,IMM_LAGSKILLS)) {
       act("You can't seem to knock $M over!",ch,NULL,victim,TO_CHAR);
       return;
       }


    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 150;
    chance -= victim->carry_weight / 200;

    if (ch->level > victim->level)
	chance -= (ch->level-victim->level)+10;
    else
	chance += (ch->level-victim->level)+10;


    if (ch->size < victim->size)
	chance -= 10; /*((ch->size + charsize) - (victim->size +victsize))*15;*/
    else
	chance += 10;/*((ch->size + charsize) - (victim->size + victsize))*10; */


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= (get_curr_stat(victim,STAT_DEX));
    chance -= GET_AC(victim,AC_BASH) /25;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 1;

    /* level */
    chance += (ch->level - victim->level);

    /* flying */
    if (IS_AFFECTED(ch,AFF_FLYING))
       chance -= 1;

    if (!IS_NPC(victim) 
	&& chance < get_skill(victim,gsn_dodge) )
    {	/*
        act("$n tries to pounce on you, but you dodge it.",ch,NULL,victim,TO_VICT);
        act("$N dodges your pounce, you fall flat on your face.",ch,NULL,victim,TO_CHAR);
        WAIT_STATE(ch,12);
        return;*/
	chance -= 3 * (get_skill(victim,gsn_dodge) - chance);
    }

    /* now the attack */
    if (number_percent() < chance )
    {
    
	act("$n pounces on you!",
		ch,NULL,victim,TO_VICT);
	act("You pounce onto $N, and send $M sprawling!",ch,NULL,victim,TO_CHAR);
	act("$n pounces on $N sending them to the ground.",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_pounce,TRUE,1);

	WAIT_STATE(victim,PULSE_VIOLENCE*2);
	WAIT_STATE(ch,PULSE_VIOLENCE*2+6);
	damage(ch,victim,number_range(4,2 + 2 * ch->size + chance),gsn_pounce,
	    DAM_BASH,TRUE);

        if (victim->position > POS_FLATFOOTED && victim->in_room == ch->in_room)
            victim->position = POS_SPRAWLED;
	
    }
    else
    {
	damage(ch,victim,0,gsn_pounce,DAM_BASH,FALSE);
	act("You fall flat on your face!",
	    ch,NULL,victim,TO_CHAR);
	act("$n falls flat on $s face.",
	    ch,NULL,victim,TO_NOTVICT);
	act("You evade $n's pounce, causing $m to fall flat on $s face.",
	    ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_pounce,FALSE,1);
	WAIT_STATE(ch,12);
	ch->position = POS_SPRAWLED;
    }
	check_killer(ch,victim);
}


void do_charge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    int victsize = 0;
    int charsize = 0;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    one_argument(argument,arg);
 
    if ( (chance = get_skill(ch,gsn_charge)) == 0
    ||	 (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_charge].were_level[ch->race]))
    {	
	send_to_char("There is no credit in these times.\n\r",ch);
	return;
    }
 
    if (!IS_SET(ch->shiftbits, SHIFT_HALF) &&
	!IS_SET(ch->shiftbits, SHIFT_FULL)) {
	send_to_char("You're not in the right form.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg, TRUE)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    } 

    if (victim == ch)
    {
	send_to_char("You obviously need to seek counseling.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;


    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if(IS_SET(victim->imm_flags,IMM_LAGSKILLS)) {
       act("You can't seem to knock $M over!",ch,NULL,victim,TO_CHAR);
       return;
}
    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 150;
    chance -= victim->carry_weight / 200;

    if (ch->level > victim->level)
	charsize = (ch->level-victim->level)/3;
    else
	victsize = (victim->level-ch->level)/3;


    if (ch->size < victim->size)
	chance += ((ch->size + charsize) - (victim->size + victsize)) * 15;
    else
	chance += ((ch->size + charsize) - (victim->size + victsize)) * 10; 


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;
    chance -= GET_AC(victim,AC_BASH) /25;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 1;

    /* level */
    chance += (ch->level - victim->level);

    /* flying */
    if (IS_AFFECTED(ch,AFF_FLYING))
       chance -= 1;

    if (!IS_NPC(victim) 
	&& chance < get_skill(victim,gsn_dodge) )
    {	/*
        act("$n tries to charge you but you deftly dodge it.",ch,NULL,victim,TO_VICT);
        act("$N dodges your charge, you fall flat on your face.",ch,NULL,victim,TO_CHAR);
        WAIT_STATE(ch,12);
        return;*/
	chance -= 3 * (get_skill(victim,gsn_dodge) - chance);
    }

    /* now the attack */
    if (number_percent() < chance )
    {
    
	act("$n charges towards you!",
		ch,NULL,victim,TO_VICT);
	act("You charge $N, and send $M flying!",ch,NULL,victim,TO_CHAR);
	act("$n charges $N sending them flying!",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_charge,TRUE,1);

	WAIT_STATE(victim, 24);
	WAIT_STATE(ch,30);
	damage(ch,victim,number_range(4,2 + 2 * ch->size + chance),gsn_charge,
	    DAM_BASH,TRUE);

        if (victim->position > POS_FLATFOOTED && victim->in_room == ch->in_room)
            victim->position = POS_SPRAWLED;
	
    }
    else
    {
	damage(ch,victim,0,gsn_charge,DAM_BASH,FALSE);
	act("You miss your charge and fall flat on your face!",
	    ch,NULL,victim,TO_CHAR);
	act("$n falls flat on $s face.",
	    ch,NULL,victim,TO_NOTVICT);
	act("You evade $n's charge, causing $m to fall flat on $s face.",
	    ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_charge,FALSE,1);
	WAIT_STATE(ch,12);
	ch->position = POS_SPRAWLED;
    }
	check_killer(ch,victim);
}


void do_maul( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    int victsize = 0;
    int charsize = 0;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    one_argument(argument,arg);
 
    if ( (chance = get_skill(ch,gsn_maul)) == 0
    ||	 (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_maul].were_level[ch->race]))
    {	
	send_to_char("You lack the proper equipment.\n\r",ch);
	return;
    }
 
    if (!IS_SET(ch->shiftbits, SHIFT_HALF) &&
	!IS_SET(ch->shiftbits, SHIFT_FULL)) {
	send_to_char("You're not in the right form.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg, TRUE)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    } 

    if (victim == ch)
    {
	send_to_char("You must be into masochism.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;


    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if(IS_SET(victim->imm_flags,IMM_LAGSKILLS)) {
       act("You can't seem to knock $M over!",ch,NULL,victim,TO_CHAR);
       return;
}
    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 150;
    chance -= victim->carry_weight / 200;

    if (ch->level > victim->level)
	charsize = (ch->level-victim->level)/3;
    else
	victsize = (victim->level-ch->level)/3;


    if (ch->size < victim->size)
	chance += ((ch->size + charsize) - (victim->size + victsize)) * 15;
    else
	chance += ((ch->size + charsize) - (victim->size + victsize)) * 10; 


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= (get_curr_stat(victim,STAT_DEX) * 4)/2;
    chance -= GET_AC(victim,AC_BASH) /25;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 1;

    /* level */
    chance += (ch->level - victim->level);

    /* flying */
    if (IS_AFFECTED(ch,AFF_FLYING))
       chance -= 1;

    if (!IS_NPC(victim) 
	&& chance < get_skill(victim,gsn_dodge) )
    {	/*
        act("$n tries to maul you but you deftly dodge it.",ch,NULL,victim,TO_VICT);
        act("$N dodges your attack, you fall flat on your face.",ch,NULL,victim,TO_CHAR);
        WAIT_STATE(ch,12);
        return;*/
	chance -= 3 * (get_skill(victim,gsn_dodge) - chance);
    }

    /* now the attack */
    if (number_percent() < chance )
    {
    
	act("$n charges towards you!",
		ch,NULL,victim,TO_VICT);
	act("You maul $N, and send $M to the ground!",ch,NULL,victim,TO_CHAR);
	act("$n mauls $N sending them to the ground!",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_maul,TRUE,1);

	WAIT_STATE(victim, 24);
	WAIT_STATE(ch,33);
	damage(ch,victim,number_range(4,2 + 2 * ch->size + chance),gsn_maul,
	    DAM_BASH,TRUE);

        if (victim->position > POS_FLATFOOTED && victim->in_room == ch->in_room)
            victim->position = POS_SPRAWLED;
	
    }
    else
    {
	damage(ch,victim,0,gsn_maul,DAM_BASH,FALSE);
	act("You miss your charge and fall flat on your face!",
	    ch,NULL,victim,TO_CHAR);
	act("$n falls flat on $s face.",
	    ch,NULL,victim,TO_NOTVICT);
	act("You evade $n's charge, causing $m to fall flat on $s face.",
	    ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_maul,FALSE,1);
	WAIT_STATE(ch,12);
	ch->position = POS_SPRAWLED;
    }
	check_killer(ch,victim);
}


void do_hamstring( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

   
     if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_hamstring)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch) 
	  && ch->level < skill_table[gsn_hamstring].were_level[ch->race]))
    {
	send_to_char("Seems silly to try and bite someones ankles doesnt it?\n\r",ch);
	return;
    }

    if (!IS_SET(ch->shiftbits, SHIFT_HALF) &&
	!IS_SET(ch->shiftbits, SHIFT_FULL)) {
	send_to_char("You're not in the right form.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,arg, TRUE)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    /*if (IS_AFFECTED(victim,AFF_FLYING))
    {
	act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR);
	return;
    }*/

    if (victim->position < POS_FIGHTING)
    {
	act("$N is already hamstringed.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You fall flat on your face!\n\r",ch);
	WAIT_STATE(ch,12);
	act("$n trips over $s own feet!",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if(IS_SET(victim->imm_flags,IMM_LAGSKILLS)) {
       act("You can't seem to knock $M over!",ch,NULL,victim,TO_CHAR);
       return;
}
    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 10;

    /* level */
    chance += (ch->level - victim->level) * 2;


    /* now the attack */
    if (number_percent() < chance)
    {
	act("$n hamstrings you and you go down!",ch,NULL,victim,TO_VICT);
	act("You hamstring $N and $N goes down!",ch,NULL,victim,TO_CHAR);
	act("$n hamstrings $N, sending $M to the ground.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_hamstring,TRUE,1);

	WAIT_STATE(victim,PULSE_VIOLENCE*2);
        WAIT_STATE(ch,PULSE_VIOLENCE*2+4);
	damage(ch,victim,number_range(4, 2 +  2 *victim->size + chance),
            gsn_hamstring, DAM_BASH,TRUE);

        if (victim->position > POS_FLATFOOTED && victim->in_room == ch->in_room)
	    victim->position = POS_SPRAWLED;
    }
    else
    {
	damage(ch,victim,0,gsn_hamstring,DAM_BASH,TRUE);
	WAIT_STATE(ch,12);
	check_improve(ch,gsn_hamstring,FALSE,1);
    } 
	check_killer(ch,victim);
}

void do_wing( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
	send_to_char("You're a ghost!\n\r",ch);
	return;
	}
    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_wing)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT))
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_wing].were_level[ch->race]))
    {
	send_to_char("You stand there foolishly flapping your arms.\n\r",ch);
	return;
    }

    if (!IS_SET(ch->shiftbits, SHIFT_HALF) &&
	!IS_SET(ch->shiftbits, SHIFT_FULL)) {
	send_to_char("You're not in the right form.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg, TRUE)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("$E's already been blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (IS_AFFECTED3(victim,AFF_DIRT))
    {
	act("$E's already been blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;


    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }
if(IS_SET(victim->imm_flags,IMM_LAGSKILLS)) {
       act("You can't seem to affect $S vision in that way!",ch,NULL,victim,TO_CHAR);
       return;
}

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):		chance +=  5;	break;
	case(SECT_CITY):		chance +=  5;	break;
	case(SECT_SNOW):		chance +=  5;	break;
	case(SECT_FIELD):		chance += 10;	break;
	case(SECT_FOREST):		chance += 10;   break;
	case(SECT_HILLS):		chance +=  5;   break;
	case(SECT_MOUNTAIN):		chance +=  5;	break;
	case(SECT_WATER_SWIM):		chance +=  5;	break;
	case(SECT_WATER_NOSWIM):	chance +=  5;	break;
	case(SECT_AIR):			chance +=  5;  	break;
	case(SECT_LAVA):		chance +=  5;  	break;
	case(SECT_DESERT):		chance += 10;   break;
    }

    if (chance == 0)
    {
	send_to_char("There isn't any dirt to kick here.\n\r",ch);
	return;
    }

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("$n is blinded by the debris!",victim,NULL,NULL,TO_ROOM);
	act("$n sends up a gust of wind, blinding you!",ch,NULL,victim,TO_VICT);
        damage(ch,victim,number_range(6,10),gsn_wing,DAM_NONE,FALSE);
	send_to_char("You can't see a thing!\n\r",victim);
	check_improve(ch,gsn_wing,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_wing].beats);

	af.where	= TO_AFFECTS;
        af.whichaff     = AFF3;
	af.type 	= gsn_wing;
	af.level 	= ch->level;
	af.duration	= 1;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_DIRT;

	affect_to_char3(victim,&af);
    }
    else
    {
	damage(ch,victim,0,gsn_wing,DAM_NONE,TRUE);
	check_improve(ch,gsn_wing,FALSE,2);
	WAIT_STATE(ch,skill_table[gsn_wing].beats);
    }
	check_killer(ch,victim);
}

void do_fury( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;

    if (IS_AFFECTED2(ch,AFF_GHOST)){
       send_to_char("You can't do that; you're DEAD!\n\r",ch);
       return;
       }

    if ((chance = get_skill(ch,gsn_fury)) == 0
    || (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    || (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_fury].were_level[ch->race]))
    {
        send_to_char("You fail to summon the beast within.\n\r",ch);
       return;
    }

    if (IS_AFFECTED2(ch,AFF_FURY) || is_affected(ch,gsn_fury)
    ||  is_affected(ch,skill_lookup("fury")))
    {
        send_to_char("You have already summoned the beast within.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
        send_to_char("You're feeling too calm to summon the beast within.\n\r",ch);
        return;
    }

    if (ch->mana < 50)
    {
        send_to_char("You don't have enough energy to summon the beast within.\n\r",ch);
        return;
    }

    /*modifiers*/

    /*fighting*/
    if (ch->position == POS_FIGHTING)
        chance += 10;

    /*damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
        AFFECT_DATA af;

        WAIT_STATE(ch,PULSE_VIOLENCE);
        ch->mana -= 50;

        /*heal a little damage*/
        ch->hit += ch->level * 2;
        ch->hit = UMIN(ch->hit,ch->max_hit);

        send_to_char("You summon the beast within you!\n\r",ch);
        act("$n is consumed by rage!",ch,NULL,NULL,TO_ROOM);
        check_improve(ch,gsn_fury,TRUE,2);

        af.where     = TO_AFFECTS;
        af.type      = gsn_fury;
        af.level     = ch->level;
        af.duration  = number_fuzzy(ch->level / 5);
        af.modifier  = UMAX(1, ch->level/10);
        af.bitvector = AFF_FURY;

        af.location  = APPLY_HITROLL;
        affect_to_char2(ch,&af);

        af.location  = APPLY_DAMROLL;
        affect_to_char2(ch,&af);

        af.modifier  = - ch->level;
        af.location  = APPLY_AC;
        affect_to_char2(ch,&af);
    }

    else
    {
        WAIT_STATE(ch,PULSE_VIOLENCE);
        ch->mana -= 25;

        send_to_char("You fail to summon the beast within.\n\r",ch);
        check_improve(ch,gsn_fury,FALSE,2);
    }
}

void spell_klaive( int sn, int level, CHAR_DATA *ch, void *vo, int target ) 
{

  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;

  if ( !IS_IMMORTAL( ch ) && !IS_NPC( ch ) && !IS_WERECREATURE( ch ) )
  {
    send_to_char( "You mispronounce an arcane syllable.\n\r", ch );
    return;
  }

  if ( ( pObjIndex = get_obj_index( 1214) )  == NULL )
    {
    send_to_char( "No object has that vnum.\n\r", ch );
    return;
    }

   obj = create_object(pObjIndex, level);
   obj->value[1] = ch->level/5;
   obj->value[2] = 7;
   obj->timer = 10; 

   obj_to_char(obj,ch);
   send_to_char("You have created a klaive!\n\r",ch);

}

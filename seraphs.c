/*********************************************************************************************
 *
 * Seraphs by Lynne Kirsch, Josh Davis, Sam Green & Nick Shoemake. @2014
 * 
**********************************************************************************************/

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


void do_lifeenergy(CHAR_DATA *ch) 
{
	int energy;
	int maxenergy;
	energy = ch->life_energy;
	maxenergy = ch->max_life_energy;
	char buf [MAX_STRING_LENGTH];

	if (ch->race != race_lookup("seraph"))
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}
				
	send_to_char("\n\r",ch );
	send_to_char(" {C+{D-------------{cLife Energy{D-------------{C+{x\n\r",ch );
	send_to_char(" {D|                                     {D|\n\r",ch );
	sprintf( buf, " {D|  {wYour maximum energy level is: {C%3d  {D|{x\n\r", maxenergy); send_to_char( buf, ch);
	sprintf( buf, " {D|  Your current energy level is: {c%3d  {D|{x\n\r", energy); send_to_char( buf, ch);
	send_to_char(" {D|                                     {D|\n\r",ch );
	
	if (energy > 124)
	{
		send_to_char(" {D|  Sanity: {C=========================  {D|\n\r",ch );
	} 
				
	else 
	{
		switch(energy)
		{
						case 124: case 123: case 122: case 121: case 120: 
						send_to_char(" {D|  Sanity: {C========================{c=  {D|\n\r",ch );
						break;
				
						case 119: case 118: case 117: case 116: case 115: 
						send_to_char(" {D|  Sanity: {C======================={c==  {D|\n\r",ch );
						break;
				
						case 114: case 113: case 112: case 111: case 110: 
						send_to_char(" {D|  Sanity: {C======================{c===  {D|\n\r",ch );
						break;
				
						case 109: case 108: case 107: case 106: case 105: 
						send_to_char(" {D|  Sanity: {C====================={c====  {D|\n\r",ch );
						break;
				
						case 104: case 103: case 102: case 101: case 100: 
						send_to_char(" {D|  Sanity: {C===================={c=====  {D|\n\r",ch );
						break;
						
						case 99: case 98: case 97: case 96: case 95: 
						send_to_char(" {D|  Sanity: {C==================={c======  {D|\n\r",ch );
						break;
						
						case 94: case 93: case 92: case 91: case 90: 
						send_to_char(" {D|  Sanity: {C=================={c=======  {D|\n\r",ch );
						break;
						
						case 89: case 88: case 87: case 86: case 85: 
						send_to_char(" {D|  Sanity: {C================={c========  {D|\n\r",ch );
						break;
						
						case 84: case 83: case 82: case 81: case 80: 
						send_to_char(" {D|  Sanity: {C================{c=========  {D|\n\r",ch );
						break;
						
						case 79: case 78: case 77: case 76: case 75: 
						send_to_char(" {D|  Sanity: {C==============={c==========  {D|\n\r",ch );
						break;
						
						case 74: case 73: case 72: case 71: case 70: 
						send_to_char(" {D|  Sanity: {C=============={c===========  {D|\n\r",ch );
						break;
						
						case 69: case 68: case 67: case 66: case 65: 
						send_to_char(" {D|  Sanity: {C============={c============  {D|\n\r",ch );
						break;
						
						case 64: case 63: case 62: case 61: case 60: 
						send_to_char(" {D|  Sanity: {C============{c=============  {D|\n\r",ch );
						break;
						
						case 59: case 58: case 57: case 56: case 55: 
						send_to_char(" {D|  Sanity: {C==========={c==============  {D|\n\r",ch );
						break;
						
						case 54: case 53: case 52: case 51: case 50: 
						send_to_char(" {D|  Sanity: {C=========={c===============  {D|\n\r",ch );
						break;
						
						case 49: case 48: case 47: case 46: case 45: 
						send_to_char(" {D|  Sanity: {C========={c================  {D|\n\r",ch );
						break;
						
						case 44: case 43: case 42: case 41: case 40: 
						send_to_char(" {D|  Sanity: {C========{c=================  {D|\n\r",ch );
						break;
						
						case 39: case 38: case 37: case 36: case 35: 
						send_to_char(" {D|  Sanity: {C======={c==================  {D|\n\r",ch );
						break;
						
						case 34: case 33: case 32: case 31: case 30: 
						send_to_char(" {D|  Sanity: {C======{c===================  {D|\n\r",ch );
						break;
						
						case 29: case 28: case 27: case 26: case 25: 
						send_to_char(" {D|  Sanity: {R====={r====================  {D|\n\r",ch );
						break;
						
						case 24: case 23: case 22: case 21: case 20: 
						send_to_char(" {D|  Sanity: {R===={r=====================  {D|\n\r",ch );
						break;
						
						case 19: case 18: case 17: case 16: case 15: 
						send_to_char(" {D|  Sanity: {R==={r======================  {D|\n\r",ch );
						break;
						
						case 14: case 13: case 12: case 11: case 10: 
						send_to_char(" {D|  Sanity: {R=={r=======================  {D|\n\r",ch );
						break;
						
						case 9: case 8: case 7: case 6: case 5: 
						send_to_char(" {D|  Sanity: {R={r========================  {D|\n\r",ch );
						break;
						
						case 4: case 3: case 2: case 1: case 0: 
						send_to_char(" {D|  Sanity: {R!{D========================  {D|\n\r",ch );
						break;
		}
	}

	send_to_char(" {D|                                     {D|\n\r",ch );
	send_to_char(" {C+{D-------------------------------------{C+{x\n\r",ch );
				  
	return;
}

void do_makeseraph(CHAR_DATA *ch, char *argument )
{

	CHAR_DATA *victim;
	int i;
	int race;
    int old_race;
	char arg[MAX_INPUT_LENGTH];
	one_argument( argument, arg );
	 
	 
	if ( arg[0] == '\0' ) {
		send_to_char( "Who would you like to make a Seraph?\n\r", ch );
		return;
	}
		
	if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL ) {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}
		
	if ( IS_NPC(victim) ) {
		send_to_char( "Not on NPC's.\n\r", ch );
		return;
	}

	old_race = victim->race;
	race = race_lookup("seraph");
 
	// Set to level 1, reset hp/ma/mv
	victim->level = 1;
	victim->max_hit = 120;
	victim->hit = 120;
	victim->max_mana = 200;
	victim->mana = 200;
	victim->max_move = 100;
	victim->move = 100;
	victim->pcdata->perm_hit = 120;
	victim->pcdata->perm_mana = 200;
	victim->pcdata->perm_move = 100;
  
	//set the gods and other frilly shit
	victim->pcdata->god = 0;
  
	//Make them a Seraph
	victim->race = race;
	victim->life_energy = 300;
	victim->max_life_energy = 300;
	victim->enochian = 1;
  
	//unfuck the stats
	for( i = 0; i < MAX_STATS; i++ )
	{
		victim->perm_stat[ i ] = pc_race_table[ race ].stats[ i ];
	}
	
	//apply racial shit
	victim->affected_by = 0 | race_table[ race ].aff;
	victim->affected_by2 = 0 | race_table[ race ].aff2;
	victim->affected_by3 = 0 | race_table[ race ].aff3;
	victim->imm_flags = 0 | race_table[ race ].imm;
	victim->res_flags = 0 | race_table[ race ].res;
	victim->vuln_flags = 0 | race_table[ race ].vuln;
	victim->form = race_table[ race ].form;
	victim->parts = race_table[ race ].parts;
	victim->size = pc_race_table[ race ].size;

	//reset the skills
	for( i = 0; i < 5; i++ )
	{
		if( pc_race_table[ old_race ].skills[ i ] != NULL )
		{
			group_remove( victim, pc_race_table[ old_race ].skills[ i ] );
		}
			
		if( pc_race_table[ race ].skills[ i ] != NULL )
		{
			group_add( victim, pc_race_table[ race ].skills[ i ], FALSE );
		}
	}
	
	send_to_char("{CYou surge with newfound divinity.{x\n\r",victim);
	send_to_char("You have made them a Seraph.\n\r",ch);
		
return;
}

void do_project( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
	
	if (ch->race != race_lookup("seraph"))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
    
    if ( argument[0] == '\0' )
    {
		send_to_char( "What would you like to project?\n\r", ch );
		return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
		if ( d->connected == CON_PLAYING &&   d->character->in_room == ch->in_room )
		{
			send_to_char( argument, d->character );
			send_to_char( "\n\r",   d->character );
		}
    }

    return;
}

void spell_angelic_lance( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{

	OBJ_DATA *lance;
  
	act("$n bows $s head momentarily and wills an angelic lance into existence!", ch, NULL, ch, TO_ROOM);
	send_to_char("You bow your head in concentration and manifest an angelic lance!\n\r",ch);

	lance = create_object(get_obj_index(OBJ_VNUM_SLANCE), 0);
	obj_to_char(lance,ch);
	lance->timer    = 15;
}

void spell_godspeed( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    AFFECT_DATA af;

    if( IS_AFFECTED3( ch, AFF3_GODSPEED) )
    {
          send_to_char("You already possess the speed of the gods.\n\r",ch);
		  return;
    }
	
	if( IS_AFFECTED3( ch, AFF3_FATIGUE ) )
    {
      send_to_char( "You are far too tired to summon divine agility.\n\r", ch );
      return;
    }
	
  af.where	 = TO_AFFECTS;
  af.type      = sn;
  af.level	 = level;
  af.duration  = 3;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF3_GODSPEED;
  affect_to_char3(ch, &af );
  act( "$n summons forth the agility of the gods!", ch, NULL, NULL, TO_ROOM );
  send_to_char( "You feel invigorated.\n\r", ch );
    return;
}

void spell_tabula_rasa( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
	 if( is_affected(victim,skill_lookup("tabula rasa")))
    {
          send_to_char("They are already oppressed by divine forces.\n\r",ch);
		  return;
    }
 
    if (saves_spell(level,ch,victim,DAM_OTHER) 
    ||  IS_SET(victim->imm_flags,IMM_MAGIC) )
    {
		if (victim != ch)
		send_to_char("You fail to invoke the fury of the gods.\n\r",ch);
        send_to_char("You feel a slight tingle across your skin.\n\r",victim);
        return;
    }
 

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "A divine force opresses your magical knowledge.\n\r", victim );
	
	if (victim != ch) {
    act( "Divine oppression overcomes $n.", victim, NULL, ch,   TO_CHAR );
	}
	
    return;
}

void spell_hallowed_discord( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
	
	
 	 if( is_affected(victim,skill_lookup("hallowed discord")))
    {
          send_to_char("They are already inhibited by hallowed discord.\n\r",ch);
		  return;
    }
 
    if (saves_spell(level,ch,victim,DAM_OTHER) 
    ||  IS_SET(victim->imm_flags,IMM_MAGIC) )
    {
		if (victim != ch)
		send_to_char("You fail.\n\r",ch);
        send_to_char("They failed.\n\r",victim);
        return;
    }
 

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You are gripped by the crushing weight of hallowed discord.\n\r", victim );
	
	if (victim != ch) {
    act( "Hallowed discord grips $n.", victim, NULL, ch,   TO_CHAR );
	}
	
    return;
}

void do_bestow(CHAR_DATA *ch, char *argument ) {

	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	one_argument( argument, arg );
	
		if (ch->race != race_lookup("seraph"))
		{
			send_to_char("Huh?\n\r", ch);
			return;
		}
		
	
		if ( arg[0] == '\0' )
		{
			send_to_char( "Who would you like to bestow Enochian upon?\n\r", ch );
			return;
		}
		
		if ( ( victim = get_char_world( ch, arg, FALSE ) ) == NULL )
		{
			send_to_char( "They aren't here.\n\r", ch );
			return;
		}
		
		if ( IS_NPC(victim) )
		{
			send_to_char( "Not on NPC's.\n\r", ch );
			return;
		}
		
		if ( victim->race != race_lookup("seraph") )
		{	
			if (victim->enochian == 0) 
			{
				victim->enochian = 1;
				send_to_char("{CYou have been given the knowledge of the Enochian tongue.{x\n\r",victim);
				send_to_char("You have bestowed the knowledge of Enochian upon them.\n\r",ch);
				return;
			}
		
			if (victim->enochian == 1) 
			{
				victim->enochian = 0;
				send_to_char("{rThe knowledge of the Enochian tongue drains from your memories.{x\n\r",victim);
				send_to_char("You have rescinded the knowledge of Enochian from them.\n\r",ch);
				return;
			}
		}
		
		else 
		{
			send_to_char("You cannot perform this action on another Seraph.\n\r",ch);
			return;
		}
	
}

/******************************************************************************
 *                                                                            *
 *   Templar class-specific spells and skills                                 *
 *   Written May 2009 by Glenn K. Lockwood                                    * 
 *   for the Dark Risings codebase                                            *
 *                                                                            *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"

bool check_social args( ( CHAR_DATA *victim, char *command, char *argument ) );
bool saves_dispel args( ( int dis_level, int spell_level, int duration) );
bool check_dispel args( ( int dis_level, CHAR_DATA* ch, int sn ) );

#define VNUM_ITEM_SPEAR_OF_RECKONING    26
void spell_spear( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{

    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;

    if (( pObjIndex = get_obj_index( VNUM_ITEM_SPEAR_OF_RECKONING )) == NULL )
    {
        send_to_char("Something horrible has happened"
            " and you should notify an IMP immediately.\n\r", ch);
        return;
    }

    obj = create_object(pObjIndex,level);
    obj->level    = level;
    obj->value[1] = level/5;
    obj->value[2] = 7;
    obj->timer    = 10;

    obj_to_char(obj,ch);
    act( "$p appears before you in a flash of brilliant light!", 
      ch, obj, NULL, TO_CHAR );
    act( "$p appears before $n in a flash of brilliant light!", 
      ch, obj, NULL, TO_ROOM );
    return;
}

void spell_ordain( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int chgod, victgod;
    char godname[24], victname[24];

    if ( IS_NPC( ch ) )
      chgod = GOD_MOB;
    else
      chgod = ch->pcdata->god;

    if ( IS_NPC( victim ) )
      victgod = GOD_MOB;
    else
      victgod = victim->pcdata->god;
    strcpy(godname,god_table[chgod].descr);
    strcpy(victname,god_table[victgod].descr);

    if ( !IS_NPC( ch ) && !IS_IMMORTAL( ch ) )
    {
      if ( chgod != victgod )
      {
        act("To ordain someone who does not follow $t is blasphemy!",
          ch, godname, NULL, TO_CHAR );
        return;
      }
    }
    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          act("You are already ordained by $t.", ch, godname, victim, TO_CHAR);
        else
          act("$N is already ordained by $t.",ch,victname,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.modifier  = ch->level;
    af.bitvector = 0;

    af.location  = APPLY_HIT;
    affect_to_char( victim, &af );

    af.location  = APPLY_MANA;
    affect_to_char( victim, &af );

    af.location  = APPLY_MOVE;
    affect_to_char( victim, &af );

    if ( ch == victim )
      act("You are ordained by the power of $t.", ch, godname, NULL, TO_CHAR);
    else
    {
      act("You ordain $N by the power of $t.",ch,godname,victim,TO_CHAR);
      act("$n ordains you by the power of $t.",ch,godname,victim,TO_VICT);
    }

    return;
}

#define DAMN_BLIND     1
#define DAMN_WEAKEN    2
#define DAMN_SLOW      4
#define DAMN_CURSE     8
void spell_judgment( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    short chance, emergency = 0;
    char godname[24];
    bool found = FALSE;
    int chgod, already = 0;

    victim = (CHAR_DATA *) vo;

    if ( IS_NPC( ch ) )
      chgod = GOD_MOB;
    else
      chgod = ch->pcdata->god;
    strcpy(godname,god_table[chgod].descr);

    if ( IS_AFFECTED(victim, AFF_BLIND) 
    ||   IS_AFFECTED3(victim, AFF_DIRT) )
      SET_BIT(already,DAMN_BLIND);

    if ( is_affected(victim,gsn_psy_drain)
    ||   is_affected(victim,gsn_weaken) )
      SET_BIT(already,DAMN_WEAKEN);

    if ( IS_AFFECTED(victim,AFF_SLOW))
      SET_BIT(already,DAMN_SLOW);

    if ( IS_AFFECTED(victim,AFF_CURSE) )
      SET_BIT(already,DAMN_CURSE);

    if ( IS_SET(already,DAMN_BLIND )
    &&   IS_SET(already,DAMN_WEAKEN)
    &&   IS_SET(already,DAMN_SLOW  )
    &&   IS_SET(already,DAMN_CURSE ) )
    {
      if ( ch != victim )
        act("$E has already been judged by $t.", ch, godname, victim, TO_CHAR );
      else
        send_to_char( "You have already been judged!\n\r", ch );
      return;
    }

 /* The spell should try to hurt victim.  First allow a save */
    if ( saves_spell(level,ch,victim,DAM_OTHER ) && number_range(1,100) > 10 )
    {
      if ( ch != victim )
        act("$E avoids the judgment of $t.", ch, godname, victim, TO_CHAR );
      else
        send_to_char("You avoid judgment.\n\r", ch );
      return;
    }

 /* If the victim doesn't save, solemn vow might still kick in */
    if ( IS_AFFECTED3(victim,AFF3_SOLEMN)
    &&   number_percent() > CONFIG_SOLEMN_VOWS_CHANCE )
    {
      send_to_char("The power of your vows shield you from taint.\n\r", victim);
      if ( ch != victim )
        act_new("$N's vows protect $M from your spell.", ch, NULL, victim, 
	  TO_CHAR, POS_SLEEPING, 0);
      return;
    }

 /* Afflict the victim with some malady */
    do
    {
      if ( emergency++ > 100 )
      {
        bug( "spell_judgment: aff loop exceeded 100!", 0 );
        send_to_char("Spell failed.\n\r", ch );
        return;
      }

      chance = number_percent();

      if ( chance > 75 && !IS_SET( already, DAMN_BLIND ) )
      {
        found = TRUE;
        af.where     = TO_AFFECTS;
        af.type      = gsn_blindness;
        af.level     = level;
        af.location  = APPLY_HITROLL;
        af.modifier  = -4;
        af.duration  = level/5;
        af.bitvector = AFF_BLIND;
        affect_to_char( victim, &af );
        if ( IS_AFFECTED2(victim, AFF2_BLINDSIGHT) )
        {
          if ( ch != victim )
            act("The judgment of $t strikes at $N's vision!", 
              ch, godname, victim, TO_CHAR);
          act_new("The judgment of $t strikes at your vision!", 
            victim, godname, NULL, TO_CHAR, POS_SLEEPING, 0);
          act("$N's vision appears to become clouded.",
            ch, NULL, victim, TO_NOTVICT );
        }
        else
        {
          if ( ch != victim )
            act("The judgment of $t strikes $N blind!", 
              ch, godname, victim, TO_CHAR);
          act_new("The judgment of $t strikes you blind!", 
            victim, godname, NULL, TO_CHAR, POS_SLEEPING, 0);
          act("$N appears to be blinded.",
            ch, NULL, victim, TO_NOTVICT );
        }
      }
      else if ( chance > 50 && !IS_SET( already, DAMN_WEAKEN ) )
      {
        found = TRUE;
        af.where     = TO_AFFECTS;
        af.type      = gsn_weaken;
        af.level     = level;
        af.duration  = level / 2;
        af.location  = APPLY_STR;
        af.modifier  = -1 * (level / 5);
        af.bitvector = AFF_WEAKEN;
        affect_to_char( victim, &af );
        if ( ch != victim )
          act("The judgment of $t renders $N feeble!", 
            ch, godname, victim, TO_CHAR);
        act_new("The judgment of $t renders you feeble!", 
          victim, godname, NULL, TO_CHAR, POS_SLEEPING, 0);
        act("$N looks tired and weak.",
          ch, NULL, victim, TO_NOTVICT );
      }
      else if ( chance > 25 && !IS_SET( already, DAMN_SLOW ) )
      {
        found = TRUE;
        if (IS_AFFECTED(victim,AFF_HASTE))
        {
          if (!check_dispel(level,victim,skill_lookup("haste"))
          ||  !check_dispel(level,victim,skill_lookup("adrenaline rush")))
          {
            if ( ch != victim )
              act("$N narrowly avoids the judgment of $t!", 
                ch, godname, victim, TO_CHAR);
            act_new("You feel the judgment of $t momentarily slow you down.", 
              victim, godname, NULL, TO_CHAR, POS_SLEEPING, 0);
          }
          else
          {
            if ( ch != victim )
              act("The judgment of $t tears away at $N's speed!", 
                ch, godname, victim, TO_CHAR);
            act_new("The judgment of $t tears away at your speed!", 
              victim, godname, NULL, TO_CHAR, POS_SLEEPING, 0);
          }
        }
        else
        {
          af.where     = TO_AFFECTS;
          af.type      = gsn_slow;
          af.level     = level;
          af.duration  = level/2;
          af.location  = APPLY_DEX;
          af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32);
          af.bitvector = AFF_SLOW;
          affect_to_char( victim, &af );
          if ( ch != victim )
            act("The judgment of $t renders $N sluggish!", 
              ch, godname, victim, TO_CHAR);
          act_new("The judgment of $t renders you sluggish!", 
            victim, godname, NULL, TO_CHAR, POS_SLEEPING, 0);
          act("$N starts to move in slow motion.",
            ch, NULL, victim, TO_NOTVICT );
        }
      }
      else if ( !IS_SET( already, DAMN_CURSE ) )
      {
        found = TRUE;
        af.where     = TO_AFFECTS;
        af.type      = gsn_curse;
        af.level     = level;
        af.duration  = level/3;
        af.location  = APPLY_HITROLL;
        af.modifier  = -1 * (level / 8);
        af.bitvector = AFF_CURSE;
        affect_to_char( victim, &af );

        af.location  = APPLY_SAVING_SPELL;
        af.modifier  = level / 8;
        affect_to_char( victim, &af );

        if ( ch != victim )
          act("The judgment of $t strickens $N with damnation!", 
            ch, godname, victim, TO_CHAR);
        act_new("The judgment of $t strickens you with damnation!", 
          victim, godname, NULL, TO_CHAR, POS_SLEEPING, 0);
        act("$N looks very uncomfortable.",
          ch, NULL, victim, TO_NOTVICT );

      }
       
    } while ( !found );

    return;
}
#undef DAMN_BLIND
#undef DAMN_WEAKEN
#undef DAMN_SLOW
#undef DAMN_CURSE 

void spell_harrow( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char godname[24];
    int dam, chgod;
    bool knocked = FALSE;

    if ( IS_NPC( ch ) )
      chgod = GOD_MOB;
    else
      chgod = ch->pcdata->god;
    strcpy(godname,god_table[chgod].descr);

/* need to sprawl BEFORE damage is done so that the the victim is already
   lagged when the damage is done.  this prevents wimpying out due to the
   damage of the knockdown */
    if ( victim != ch
    &&   victim->position > POS_FLATFOOTED
    &&  !IS_SET(victim->imm_flags,IMM_LAGSKILLS)
    &&  victim->in_room == ch->in_room )
    {
      if ( number_percent() < 25 )
      {
        /* set the lag */
        WAIT_STATE(victim,PULSE_VIOLENCE*2);
        victim->position = POS_SPRAWLED;
        WAIT_STATE(ch,PULSE_VIOLENCE*2+4);
        knocked = TRUE;
      }
    }

    /* do the damage */
    dam = dice(6 + level / 2, 8);
    damage( ch, victim, dam, sn, DAM_HOLY, TRUE );

    /* send the knockdown messages */
    if ( knocked )
    {
        act("The force of $t drives you to your knees!",
            ch, godname, victim, TO_VICT);
        act("The force of $t drives $n to $s knees!",
            victim, godname, NULL, TO_ROOM);
    }

    return;
}

void do_lay_on_hands( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    int chance;

    if (IS_AFFECTED2(ch,AFF_GHOST))
    {
        send_to_char("You're a ghost!\n\r", ch);
        return;
    }

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char("Who do you want to touch?\n\r",ch);
        return;
    }

    if ( (victim = get_char_room(ch, argument, TRUE)) == NULL ) 
    {
        send_to_char("They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        if ( !check_social( ch, "adjust", "" ) )
        {
            send_to_char("That would not be very proper.\n\r", ch );
            return;
        }
        return;
    }

    if ((!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_lay_on_hands].skill_level[ch->class] )
    ||  (chance = get_skill(ch, gsn_lay_on_hands)) < 1 )
    {
        act("You lay your hands on $M.", ch, NULL, victim, TO_CHAR );
        check_social( ch, "slap", argument );

        return;
    }
      
    if (ch->mana < 50)
    {
        send_to_char("You don't have enough mana.\n\r",ch);
        return;
    }
   
    if (IS_AFFECTED2(victim, AFF_GHOST))
    {
        act("Your hand passes right through $M!", ch, NULL, victim, TO_CHAR );
        return;
    }
  
    if ( is_affected(ch,gsn_exhaustion))
    {
        act("You are too exhausted to heal $M right now.", 
          ch, NULL, victim, TO_CHAR );
        return;
    }
    if ( number_percent( ) <= chance )
    {
        ch->mana = UMAX(0,ch->mana-50);
        victim->hit = UMIN(victim->max_hit, victim->hit + 200);

        act("You gently lay your hands on $N, healing $S wounds.", 
          ch, NULL, victim, TO_CHAR);   

       act("A warm feeling fills your body as $n gently touches your forehead.",
          ch, NULL, victim, TO_VICT);

        act("$n's hands glow as $e gently touches $N's forehead.", 
          ch, NULL, victim, TO_NOTVICT);

        af.where     = TO_AFFECTS;
        af.type      = gsn_exhaustion;
        af.level     = ch->level;
        af.duration  = 2;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = 0;
        affect_to_char( ch, &af );

        check_improve( ch, gsn_lay_on_hands, TRUE, 1 );
    }
    else
    {
        if ( number_range(1,1000) == 1 )
        {
            act("You attempt to lay your hands on $M but slip!",
              ch, NULL, victim, TO_CHAR );
            check_social( ch, "slap", argument );
        }
        else
        {
            act("You gently lay your hands on $N, but nothing happens!", 
              ch, NULL, victim, TO_CHAR);   
            act("$n gently touches your forehead, but nothing happens.",
              ch, NULL, victim, TO_VICT);
            act("$n gently touches $N's forehead.", 
              ch, NULL, victim, TO_NOTVICT);
        }
        check_improve( ch, gsn_lay_on_hands, FALSE, 1 );
    }

    WAIT_STATE( ch, skill_table[gsn_lay_on_hands].beats );

    return;
}

#define PURE_POISON   1
#define PURE_PLAGUE   2
#define PURE_UNHOLY   4
#define PURE_DECAY    8
#define PURE_ATROPHY 16
#define PURE_WILT    32
#define PURE_BLISTER 64
/*
 * This skill uses its own aff-handling subroutines because it needs
 * to know how long the spell it cancels was going to last.  This is
 * prone to breakage if the affects code changes
 *
 * 5/23/2009 gkl
 */
void do_purify( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *af, *taf = NULL;
    int target, chance;
    int affected = 0, count = 0, cycleup = 0;

    if (IS_AFFECTED2(ch,AFF_GHOST))
    {
        send_to_char("You're a ghost!\n\r", ch);
        return;
    }

    if ((!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_purify].skill_level[ch->class] )
    ||  (chance = get_skill(ch, gsn_purify)) < 1 )
    {
        send_to_char("You do not know how to do that.\n\r", ch );
        return;
    }

/*  first loop -- see what eligible affs exist */
    for ( af = ch->affected; af != NULL; af = af->next )
    {
        if ( af->type == gsn_atrophy )
        {
          count++;
          affected |= PURE_ATROPHY;
          taf = af;
        }
        else if ( af->type == gsn_wilt )
        {
          count++;
          affected |= PURE_WILT;
          taf = af;
        }
        else if ( af->type == gsn_blister )
        {
          count++;
          affected |= PURE_BLISTER;
          taf = af;
        }
        else if ( af->type == gsn_decay )
        {
          count++;
          affected |= PURE_DECAY;
          taf = af;
        }
    }

    if ( count == 0 )
    {
      send_to_char("Your body is already free from impurity.\n\r",ch);
      return;
    }

    if ( number_percent( ) > chance )
    {
      send_to_char("You are unable to purify your body of taint.\n\r", ch );
      check_improve( ch, gsn_purify, FALSE, 1 );
      WAIT_STATE( ch, skill_table[gsn_purify].beats );
      return;
    }

    if ( count > 1 ) /* find random aff */
    {
      target = number_range(1,count);
      for ( af = ch->affected; af != NULL; af = af->next )
      {
        if ( ( af->type == gsn_atrophy
        ||   af->type == gsn_wilt
        ||   af->type == gsn_blister
        ||   af->type == gsn_decay )
        &&  ++cycleup == target )
        {
          taf = af;
          break;
        }
      }
    }
/*  else, only one aff was eligible and it is already taf */

    if ( taf == NULL )
    {
      send_to_char("Your body is already free from impurity.\n\r",ch);
      bug( "do_purify: count>0 but could not find taf", 0 );
      return;
    }

    affected = taf->type;
    count = taf->duration;
    if (!saves_dispel(ch->level,taf->level,taf->duration))
    {
      send_to_char("You drive the taint from your body!\n\r", ch );
      act("$n drives the taint from $s body!", ch, NULL, NULL, TO_ROOM);
      affect_strip(ch,affected);
      if ( skill_table[affected].msg_off )
      {
        send_to_char( skill_table[affected].msg_off, ch );
        send_to_char( "\n\r", ch );
      }
/*    This is going to hurt */ 
      cycleup = 10 * number_range(count+1,count+3);
      raw_damage( ch, ch, cycleup, gsn_purify, DAM_HOLY, TRUE, FALSE );
    }
    else
    {
      send_to_char("You feel the taint within you weaken!\n\r", ch );
      taf->level--;
    }
       
    check_improve( ch, gsn_purify, TRUE, 1 );
    WAIT_STATE( ch, skill_table[gsn_purify].beats );

    return;
}
#undef PURE_POISON
#undef PURE_PLAGUE
#undef PURE_UNHOLY
#undef PURE_DECAY
#undef PURE_ATROPHY
#undef PURE_WILT
#undef PURE_BLISTER

void do_solemn( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
    int duration, chgod, chance;
    char godname[24];

    if ( IS_NPC( ch ) )
      chgod = GOD_MOB;
    else
      chgod = ch->pcdata->god;
    strcpy(godname,god_table[chgod].descr);


    if (IS_AFFECTED2(ch,AFF_GHOST))
    {
        send_to_char("You should be praying for your life back first!\n\r", ch);
        return;
    }

    if ((!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_solemn_vow].skill_level[ch->class] )
    ||  (chance = get_skill(ch, gsn_solemn_vow)) < 1 )
    {
        send_to_char("You do not know how to do that.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED3( ch, AFF3_SOLEMN ) )
    {
      send_to_char("You have already undertaken a solemn vow.\n\r", ch);
      return;
    }

    if ( number_percent() > chance )
    {
        send_to_char(
            "You attempt to recite your solemn vows but miss a word.\n\r", ch);
        check_improve( ch, gsn_solemn_vow, FALSE, 1 );
        WAIT_STATE( ch, skill_table[gsn_solemn_vow].beats );
        return;
    }

    duration = get_curr_stat(ch,STAT_CON)/4;

    af.where     = TO_AFFECTS;
    af.type      = gsn_solemn_vow;
    af.level     = ch->level;
    af.duration  = duration;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF3_SOLEMN;
    affect_to_char3( ch, &af );

    act("You renew your solemn vow to $t.", 
      ch, godname, NULL, TO_CHAR);
    act("$n recites a few words and renews $s solemn vow to $t.", 
      ch, godname, NULL, TO_ROOM);

    check_improve( ch, gsn_solemn_vow, TRUE, 1 );     
    WAIT_STATE( ch, skill_table[gsn_solemn_vow].beats );

    return;
}

void spell_create_rose( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *rose;
    int chgod;

    if ( IS_NPC( ch ) )
       chgod = GOD_MOB;
    else
       chgod = ch->pcdata->god;

    rose = create_object(get_obj_index(god_table[chgod].rose), 0);
    
    act( "You create $p.", ch, rose, NULL, TO_CHAR );
    act( "$n creates $p.", ch, rose, NULL, TO_ROOM );

    obj_to_char(rose,ch);

    return;
}

void spell_inquisition( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    char godname[24];
    int chgod;

    victim = (CHAR_DATA *) vo;

    if ( IS_NPC( ch ) )
      chgod = GOD_MOB;
    else
      chgod = ch->pcdata->god;
    strcpy(godname,god_table[chgod].descr);

    if ( ch != victim )
      act("You compel $N to bare $S soul to $t!", 
        ch, godname, victim, TO_CHAR);
      act("$N compels you to bare your soul to $t!",
        victim, godname, ch, TO_CHAR);

    raw_damage( ch, ch, number_fuzzy(5), sn, DAM_HOLY, TRUE, FALSE );
    spell_dispel_magic(gsn_dispel_magic,level,ch,vo,target);

    return;
}

void do_divine_focus( CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;
    char godname[24];
    int chgod, chance, applied;

    if ( IS_NPC( ch ) )
      chgod = GOD_MOB;
    else
      chgod = ch->pcdata->god;
    strcpy(godname,god_table[chgod].descr);

    if (!IS_NPC(ch) && IS_AFFECTED2(ch,AFF_GHOST))
    {
       send_to_char("You should pray for life before all else.\n\r",ch);
       return;
    }

    chance = get_skill(ch,gsn_divine_focus);
    if (chance == 0
    || (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    || (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_divine_focus].skill_level[ch->class]))
    {
       act("$t requires more from you first.", ch, godname, NULL, TO_CHAR);
       return;
    }

    if (ch->mana < 50)
    {
      act("You cannot muster up enough will to focus on $t.",
       ch, godname, NULL, TO_CHAR);
      return;
    }

    if (IS_AFFECTED(ch,AFF_BERSERK))
    {
      act("You are already focused on the divinity of $t.",
        ch,godname,NULL,TO_CHAR);
      return;
    }

/*  Modifiers--behaves in many ways opposite to battle focus */
    if (IS_AFFECTED(ch,AFF_CALM))
        chance += 10;
    if (ch->position == POS_FIGHTING)
        chance -= 10;

    WAIT_STATE( ch, skill_table[gsn_divine_focus].beats );

    if ( number_percent() > chance )
    {
        if ( number_range(1,2) == 1 )
          send_to_char("A moment of uncertainty disrupts your focus.\n\r", ch );        else
          act("You fail to focus your thoughts on $t with your complete being.",
            ch, godname, NULL, TO_CHAR);
        check_improve( ch, gsn_divine_focus, FALSE, 1 );
        return;
    }

    ch->mana -= 50;

    ch->hit += ch->level * 2;
    ch->hit = UMIN(ch->hit,ch->max_hit);

    act("The power of $t gives you renewed strength!", 
      ch, godname, NULL, TO_CHAR);
    act("A$t aura surrounds $n as $e is granted divine favor!",
      ch, god_table[chgod].aura, NULL, TO_ROOM);

    check_improve(ch,gsn_divine_focus,TRUE,2);

    applied = 0;
    af.where     = TO_AFFECTS;
    af.type      = gsn_divine_focus;
    af.level     = ch->level;
    af.duration  = ch->level / 5;
    af.bitvector = AFF_BERSERK;

 /* The strength portion of this is not stackable */
    if (!is_affected(ch,skill_lookup("enhanced strength"))
    &&  !is_affected(ch,skill_lookup("giant strength")))
    {
      af.location  = APPLY_STR;
      af.modifier  = UMAX(1, ch->level/12);
      affect_to_char(ch,&af);
      applied++;
    }

 /* However, the dex portion is (which is the same case as rage) */
    af.bitvector = ( applied == 0 ? AFF_BERSERK : AFF_HASTE );
    af.location  = APPLY_DEX;
    af.modifier  = UMAX(1, ch->level/12);
    affect_to_char(ch,&af);
    applied++;

 /* But the con portion cannot stack with adrenaline rush */
    if (!is_affected(ch,skill_lookup("adrenaline rush")))
    {
      af.bitvector = ( applied == 0 ? AFF_BERSERK : AFF_HASTE );
      af.location  = APPLY_CON;
      af.modifier  = UMAX(1, ch->level/20);
      affect_to_char(ch, &af);
      applied++;
    }
    if ( applied < 2 )
    {
      af.bitvector = AFF_HASTE;
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      affect_to_char(ch,&af);
    }

    return;
}

/*
 * Reckoning, which came from the original damnation
 * Removed 5/23/2009 gkl
 *
void spell_reckoning( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    short chance;

    chance = number_range(0,100);

    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED3(victim,AFF_RECKONING))
       {
            send_to_char("Their day of reckoning is already upon them.\n\r",ch);     
            return;
       }
 
   if ( saves_spell( level, ch, victim, DAM_OTHER ) ) 
        {
          if (chance > 20)
             {
               act("Your opponent narrowly avoids their day of reckoning.",ch, 0, 0, TO_CHAR);
               return;
             }
        }

    af.where     = TO_AFFECTS;
    af.type      = gsn_reckoning;
    af.level     = level;
    af.duration  = 4;
    af.location  = APPLY_HIT;
    af.modifier  = -25;
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );


    af.whichaff  = AFF2;
    af.location  = APPLY_SAVES;
    af.modifier  = level/5;
    af.bitvector = AFF_RECKONING;
    affect_to_char3( victim, &af );
    send_to_char("Your time of reckoning is at hand, evil doer!\n\r",victim);   
    if ( ch != victim )
        act("$N's time of reckoning has arrived!",ch,NULL,victim,TO_CHAR);
    return;

}
 *
 *
 */

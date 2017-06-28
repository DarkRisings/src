/*******************************************************************************
 * Healer Functionality
 * Rewritten for Dark Risings
 *
 * November 6, 2011                             
 * Glenn K. Lockwood
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "magic.h"

struct healer_spell
{
    char      * name;
    char      * words;
    char      * descr;
    SPELL_FUN * spell;
    sh_int    * sn;
    int         cost;
    bool        show;   /* set to FALSE to hide spell on list of services */
};

const struct healer_spell healer_table[] =
{
    {   
        "light",                "judicandus dies",      
        "cure light wounds",
        spell_cure_light,       &gsn_cure_light,
        1000,                   TRUE 
    },

    {   
        "serious",              "judicandus gzfuajg",      
        "cure serious wounds",
        spell_cure_serious,     &gsn_cure_serious,
        1500,                   TRUE 
    },

    {   
        "critical",             "judicandus qfuhuqar",      
        "cure critical wounds",
        spell_cure_critical,    &gsn_cure_critical,
        2500,                   TRUE 
    },

    {   
        "heal",                 "pzar",
        "healing spell",
        spell_heal,             &gsn_heal,
        5000,                   TRUE 
    },

    {   
        "blindness",            "judicandus noselacri",
        "cure blindness",
        spell_cure_blindness,   &gsn_cure_blindness,
        2000,                   TRUE 
    },

    {   
        "disease",              "judicandus eugzagz",
        "cure disease",
        spell_cure_disease,     &gsn_cure_disease,
        1500,                   TRUE 
    },

    {   
        "poison",               "judicandus sausabru",
        "cure poison",
        spell_cure_poison,      &gsn_cure_poison,
        2500,                   TRUE 
    },

    {   
        "uncurse",              "candussido judifgz",
        "remove curse",
        spell_remove_curse,     &gsn_remove_curse,
        5000,                   TRUE 
    },

    {   
        "curse",                "candussido judifgz",
        "remove curse",
        spell_remove_curse,     &gsn_remove_curse,
        5000,                   FALSE
    },

    {   
        "refresh",              "candusima",
        "restore movement",
        spell_refresh,          &gsn_refresh,
         500,                   TRUE 
    },

    {   
        "moves",                "candusima",
        "restore movement",
        spell_refresh,          &gsn_refresh,
         500,                   FALSE
    },

    {   
        "resurrect",            "ilfe",      
        "returns life to a ghost",
        spell_resurrect,        &gsn_resurrect,
        0,                      TRUE 
    },

    {   
        "mana",                 "energizer",
        "restore mana",
        NULL,                   NULL,
        2000,                   TRUE 
    },

    {   
        "energize",             "energizer",
        "restore mana",
        NULL,                   NULL,
        2000,                   FALSE
    },

    {   
        "limb",                 "candusghacandus ruwb",
        "restore limb",
        spell_restore_limb,     &gsn_restore_limb,
        5000,                   TRUE
    },

    {   
        "wither",               "candusghacandus ruwb",
        "restore limb",
       *spell_restore_limb,     &gsn_restore_limb,
        5000,                   FALSE
    },

    {   
        NULL,                   NULL,
        NULL,                   NULL,
           0,                   FALSE
    }
};

void do_heal(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int i;

    /* check for healer */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && IS_SET(mob->act_bits, ACT_IS_HEALER) )
            break;
    }
 
    if ( mob == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    one_argument(argument,arg);

    if ( IS_NULLSTR(arg) )
    {
        act("$N says 'I offer the following spells:'",ch,NULL,mob,TO_CHAR);
        for ( i = 0; healer_table[i].name != NULL; i++ )
        {
            if ( healer_table[i].show )
            {
            sprintf(buf2,"%s: %s", healer_table[i].name, healer_table[i].descr);
            sprintf(buf, "  %-35s%2d gold\n\r", buf2, healer_table[i].cost/100);
            send_to_char(buf, ch);
            }
        }
        send_to_char( " Type heal <type> to be healed.\n\r", ch );
        return;
    }

    for ( i = 0; healer_table[i].name != NULL; i++ )
        if ( !str_prefix( arg, healer_table[i].name ) )
            break;

    if ( healer_table[i].name == NULL )
    {
        act( "$N says 'Type 'heal' for a list of spells.'", 
            ch, NULL, mob, TO_CHAR );
        return;
    }

    if (healer_table[i].cost > (ch->gold * 100 + ch->silver))
    {
        act( "$N says 'You do not have enough gold for my services.'",
            ch, NULL, mob, TO_CHAR );
        return;
    }

    WAIT_STATE( ch, PULSE_VIOLENCE );

    deduct_cost( ch, healer_table[i].cost );
    mob->gold   += healer_table[i].cost / 100;
    mob->silver += healer_table[i].cost % 100;

    act("$n utters the words '$T'.", mob, NULL, healer_table[i].words, TO_ROOM);
  
    if (healer_table[i].spell == NULL)  /* restore mana trap...kinda hackish */
    {
        ch->mana += (dice(2,8) + mob->level / 3)*2;
        ch->mana = UMIN(ch->mana,ch->max_mana);
        send_to_char("A warm glow passes through you.\n\r",ch);
        return;
    }

    if ( healer_table[i].sn == NULL )
        return;
    
    (*healer_table[i].spell) (*healer_table[i].sn, mob->level, mob, ch, 
                                                                TARGET_CHAR);
    return;
}

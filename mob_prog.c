/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  MOBprograms for ROM 2.4 v0.98g (C) M.Nylander 1996                     *
 *  Based on MERC 2.2 MOBprograms concept by N'Atas-ha.                    *
 *  Written and adapted to ROM 2.4 by                                      *
 *          Markku Nylander (markku.nylander@uta.fi)                       *
 *  This code may be copied and distributed as per the ROM license.        *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <ctype.h>
#include <regex.h>
#include "merc.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"
#include "mob_cmds.h"

extern int flag_lookup( const char *word, const struct flag_type *flag_table );
extern MPDELAY_DATA      *new_mpdelay  args( (void) );
extern void               free_mpdelay args( (MPDELAY_DATA *mpdelay) );

bool is_mprecog( CHAR_DATA *ch, char *argument );


/*
 * These defines correspond to the entries in fn_keyword[] table.
 * If you add a new if_check, you must also add a #define here.
 */
#define CHK_RAND   	(0)
#define CHK_MOBHERE     (1)
#define CHK_OBJHERE     (2)
#define CHK_MOBEXISTS   (3)
#define CHK_OBJEXISTS   (4)
#define CHK_PEOPLE      (5)
#define CHK_PLAYERS     (6)
#define CHK_MOBS        (7)
#define CHK_CLONES      (8)
#define CHK_ORDER       (9)
#define CHK_HOUR        (10)
#define CHK_ISPC        (11)
#define CHK_ISNPC       (12)
#define CHK_ISIMMORT    (13)
#define CHK_ISCHARM     (14)
#define CHK_ISFOLLOW    (15)
#define CHK_ISACTIVE    (16)
#define CHK_ISDELAY     (17)
#define CHK_ISVISIBLE   (18)
#define CHK_HASTARGET   (19)
#define CHK_ISTARGET    (20)
#define CHK_EXISTS      (21)
#define CHK_AFFECTED    (22)
#define CHK_ACT         (23)
#define CHK_OFF         (24)
#define CHK_IMM         (25)
#define CHK_CARRIES     (26)
#define CHK_WEARS       (27)
#define CHK_HAS         (28)
#define CHK_USES        (29)
#define CHK_NAME        (30)
#define CHK_POS         (31)
#define CHK_GUILD       (32)
#define CHK_RACE        (33)
#define CHK_CLASS       (34)
#define CHK_OBJTYPE     (35)
#define CHK_VNUM        (36)
#define CHK_HPCNT       (37)
#define CHK_ROOM        (38)
#define CHK_SEX         (39)
#define CHK_LEVEL       (40)
#define CHK_ALIGN       (41)
#define CHK_MONEY       (42)
#define CHK_OBJVAL0     (43)
#define CHK_OBJVAL1     (44)
#define CHK_OBJVAL2     (45)
#define CHK_OBJVAL3     (46)
#define CHK_OBJVAL4     (47)
#define CHK_GRPSIZE     (48)
#define CHK_PRAND	(49)
#define CHK_ISCONTAINED (50)
#define CHK_HASANYWHERE (51)
#define CHK_PPM         (52)
#define CHK_RECOG       (53)
#define CHK_DEITY       (54)
#define CHK_PCEXISTS    (55)
#define CHK_ARRIVED     (56)
#define CHK_WASIN       (57)
#define CHK_SKILL       (58)
#define CHK_FIGHTLAG    (59)
#define CHK_INPK        (60)
#define CHK_PCHERE      (61)
#define CHK_PCAROUND    (62)
#define CHK_ISVAMP  	(63)
#define CHK_ISWERE   	(64)
#define CHK_BAKING 		(65)
#define CHK_TAILORING 	(66)
#define CHK_FLETCHING 	(67)
#define CHK_AFFECTED2   (68)
#define CHK_AFFECTED3   (69)

/*
 * These defines correspond to the entries in fn_evals[] table.
 */
#define EVAL_EQ            0
#define EVAL_GE            1
#define EVAL_LE            2
#define EVAL_GT            3
#define EVAL_LT            4
#define EVAL_NE            5

/*
 * if-check keywords:
 */
const char * fn_keyword[] =
{
    "rand",		/* 0) if rand 30	- if random number < 30 */
    "mobhere",		/* 1) if mobhere fido	- is there a 'fido' here */
    "objhere",		/* 2) if objhere bottle- is there a 'bottle' here */
			/* if mobhere 1233	- is there mob vnum 1233 here */
			/* if objhere 1233	- is there obj vnum 1233 here */
    "mobexists",	/* 3) if mobexists fido	- is there a fido somewhere */
    "objexists",	/* 4) if objexists sword- is there a sword somewhere */

    "people",		/* 5) if people > 4 - does room contain > 4 people */
    "players",		/* 6) if players > 1- does room contain > 1 pcs */
    "mobs",		/* 7) if mobs >     - does room contain > 2 mobiles */
    "clones",		/* 8) if clones > 3 - are there > 3 mobs of same vnum here */
    "order",		/* 9) if order == 0	- is mob the first in room */
    "hour",		/* 10) if hour > 11	- is the time > 11 o'clock */


    "ispc",		/* 11) if ispc $n 		- is $n a pc */
    "isnpc",		/* 12) if isnpc $n 		- is $n a mobile */
    "isimmort",		/* 13) if isimmort $n	- is $n immortal */
    "ischarm",		/* 14) if ischarm $n	- is $n charmed */
    "isfollow",		/* 15) if isfollow $n	- is $n following someone */
    "isactive",		/* 16) if isactive $n	- is $n's position > SLEEPING */
    "isdelay",		/* 17) if isdelay $i	- does $i have mobprog pending */
    "isvisible",	/* 18) if isvisible $n	- can mob see $n */
    "hastarget",	/* 19) if hastarget $i	- does $i have a valid target */
    "istarget",		/* 20) if istarget $n	- is $n mob's target */
    "exists",		/* 21) if exists $n	- does $n exist somewhere */

    "affected",		/* 22) if affected $n blind - is $n affected by blind */
    "act",		/* 23) if act $i sentinel - is $i flagged sentinel */
    "off",              /* 24) if off $i berserk- is $i flagged berserk */
    "imm",              /* 25) if imm $i fire	- is $i immune to fire */
    "carries",		/* 26) if carries $n sword - does $n have a 'sword' IN INVENTORY */
			/* if carries $n 1233	- does $n have obj vnum 1233 IN INVENTORY */
    "wears",		/* 27) if wears $n lantern -is $n wearing a 'lantern' */
			/* if wears $n 1233	- is $n wearing obj vnum 1233 */
    "has",    		/* 28) if has $n weapon	- does $n have obj of type weapon */
    "uses",		/* 29) if uses $n armor	- is $n wearing obj of type armor */
    "name",		/* 30) if name $n puff	- is $n's name 'puff' */
    "pos",		/* 31) if pos $n standing	- is $n standing */
    "clan",		/* 32) if clan $n 'whatever'- does $n belong to clan 'whatever' */
    "race",		/* 33) if race $n dragon - is $n of 'dragon' race */
    "class",		/* 34) if class $n mage	- is $n's class 'mage' */
    "objtype",		/* 35) if objtype $p scroll	- is $p a scroll */

    "vnum",		/* 36) if vnum $i == 1233  	- virtual number check */
    "hpcnt",		/* 37) if hpcnt $i > 30	- hit point percent check */
    "room",		/* 38) if room $i == 1233	- room virtual number */
    "sex",		/* 39) if sex $i == 0	- sex check */
    "level",		/* 40) if level $n < 5	- level check */
    "align",		/* 41) alignment check */
    "money",		/* 42) if money $n */
    "objval0",		/* 43) if objval0 > 1000 - object value[] checks 0..4 */
    "objval1",		/* 44) */
    "objval2",		/* 45) */
    "objval3", 		/* 46) */
    "objval4",          /* 47) */
    "grpsize",		/* 48) if grpsize $n > 6	- group size check */
    "prand",		/* 49) if prand, works like rand but uses persistent number */
    "iscontained",	/* 50) if ch carries object vnum in a container */
    "hasanywhere",	/* 51) if ch carries object vnum/name anywhere on person (carries||contains) */
    "ppm",              /* 52) if prand, except the range is 1 to 1,000,000 instead of 1 to 100 */
    "recog",            /* 53) if recog $n [<set vnum> <flag>|<flag alias>] */
    "deity",            /* 54) if deity $n <syrin/tyrin/rhian/none> */
    "pcexists",         /* 55) same as mobexists, but for pcs */
    "arrived",          /* 56) used only for greet/grall triggers--if $n arrived from <direction> */
    "wasin",            /* 57) also used only in greet/grall; if $n came from room <vnum> */
    "skill",            /* 58) if $n's skill <skill> is <gt/lt/eq> # */
    "fightlag",         /* 59) is $n is in fightlag, pk or otherwise */
    "inpk",             /* 60) if $n is in pk */
    "pchere",           /* 61) if $n is in the room (like mobhere for PCs) */
    "pcaround",         /* 62) if PCs are in any adjacent rooms */
    "isvamp",           /* 63) if character is vamp */
    "iswere",           /* 64) if character is were */
    "baking",		/* 65) if baking $n > 49	- trade check */
    "tailoring",	/* 66) if fletching $n > 49	- trade check */
    "fletching",	/* 67) if tailoring $n > 49	- trade check */
    "affected2",        /* 68) same as affected for aff2 */
    "affected3",        /* 69) same as affected for aff3 */
    
    "\n"		/* Table terminator */
};

const char *fn_evals[] =
{
    "==",
    ">=",
    "<=",
    ">",
    "<",
    "!=",
    "\n"
};

/*
 * Return a valid keyword from a keyword table
 */
int keyword_lookup( const char **table, char *keyword )
{
    register int i;
    for( i = 0; table[i][0] != '\n'; i++ )
        if( !str_cmp( table[i], keyword ) )
            return( i );
    return -1;
}

/*
 * Perform numeric evaluation.
 * Called by cmd_eval()
 */
int num_eval( int lval, int oper, int rval )
{
    switch( oper )
    {
        case EVAL_EQ:
             return ( lval == rval );
        case EVAL_GE:
             return ( lval >= rval );
        case EVAL_LE:
             return ( lval <= rval );
        case EVAL_NE:
             return ( lval != rval );
        case EVAL_GT:
             return ( lval > rval );
        case EVAL_LT:
             return ( lval < rval );
        default:
             bug( "num_eval: invalid oper", 0 );
             return 0;
    }
}

/*
 * ---------------------------------------------------------------------
 * UTILITY FUNCTIONS USED BY CMD_EVAL()
 * ----------------------------------------------------------------------
 */

/*
 * Get a random PC in the room (for $r parameter)
 */
CHAR_DATA *get_random_char( CHAR_DATA *mob )
{
    CHAR_DATA *vch, *victim = NULL;
    int now = 0, highest = 0;
    for( vch = mob->in_room->people; vch; vch = vch->next_in_room )
    {
        if ( mob != vch 
        &&   !IS_NPC( vch ) 
        &&   can_see( mob, vch )
        &&   ( now = number_percent() ) > highest )
        {
            victim = vch;
            highest = now;
        }
    }
    return victim;
}

/* 
 * How many other players / mobs are there in the room
 * iFlag: 0: all, 1: players, 2: mobiles 3: mobs w/ same vnum 4: same group
 */
int count_people_room( CHAR_DATA *mob, int iFlag )
{
    CHAR_DATA *vch;
    int count;
    for ( count = 0, vch = mob->in_room->people; vch; vch = vch->next_in_room )
	if ( mob != vch 
	&&   (iFlag == 0
	  || (iFlag == 1 && !IS_NPC( vch )) 
	  || (iFlag == 2 && IS_NPC( vch ))
	  || (iFlag == 3 && IS_NPC( mob ) && IS_NPC( vch ) 
	     && mob->pIndexData->vnum == vch->pIndexData->vnum )
	  || (iFlag == 4 && is_same_group( mob, vch )) )
	&& can_see( mob, vch ) ) 
	    count++;
    return ( count );
}

/*
 * How many players are in all immediately adjacent rooms?
 */
int count_neighbors( CHAR_DATA *mob )
{
    CHAR_DATA *vch;
    EXIT_DATA *pExit;
    sh_int door;
    int count = 0;
    for ( door = 0; door < 6; door++ )
    {
        if ((pExit = mob->in_room->exit[door]) != NULL)
        {
            for (vch = pExit->u1.to_room->people; vch; vch = vch->next_in_room)
            {
                if ( !IS_NPC(vch) && can_see( mob, vch ) )
                    count++;
            }
        }
    }
    return count;
}

/*
 * Get the order of a mob in the room. Useful when several mobs in
 * a room have the same trigger and you want only the first of them
 * to act 
 */
int get_order( CHAR_DATA *ch )
{
    CHAR_DATA *vch;
    int i;

    if ( !IS_NPC(ch) )
	return 0;
    for ( i = 0, vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
	if ( vch == ch )
	    return i;
	if ( IS_NPC(vch) 
	&&   vch->pIndexData->vnum == ch->pIndexData->vnum )
	    i++;
    }
    return 0;
}

/*
 * Check if ch has a given item or item type
 * vnum: item vnum or -1
 * item_type: item type or -1
 * fWear: TRUE: item must be worn, FALSE: don't care
 */
bool has_item( CHAR_DATA *ch, sh_int vnum, sh_int item_type, bool fWear )
{
    OBJ_DATA *obj;
    for ( obj = ch->carrying; obj; obj = obj->next_content )
	if ( ( vnum < 0 || obj->pIndexData->vnum == vnum )
	&&   ( item_type < 0 || obj->pIndexData->item_type == item_type )
	&&   ( !fWear || obj->wear_loc != WEAR_NONE ) )
	    return TRUE;
    return FALSE;
}

/* The opposite of has_item( *, *, *, FALSE ) in that it is only true
   if the character has the item in inventory - not in container, not worn.
   Only accepts a vnum argument, as get_obj_carry() handles item names
   as arguments  */
bool has_item_inventory( CHAR_DATA *ch, sh_int vnum )
{
    OBJ_DATA *obj;
    for ( obj = ch->carrying; obj; obj = obj->next_content )
        if ( obj->pIndexData->vnum == vnum 
        &&   obj->wear_loc == WEAR_NONE )
            return TRUE;
    return FALSE;
}

/*
 * Recursive function which looks for an item in the possession of
 * a person - only returns true if the item is in a container though
 */

bool has_incontainer( OBJ_DATA *obj_list, sh_int vnum, sh_int cont_vnum, char *cont_name )
{
    OBJ_DATA *obj;
    bool found = FALSE;

    for ( obj = obj_list; obj; obj = obj->next_content )
    {
/*      So $o is contained in something */
        if ( obj->pIndexData->vnum == vnum && obj->in_obj )
        {
            if ( cont_vnum == -1 && is_name( cont_name, obj->in_obj->name ) )
                return TRUE;
            if ( cont_vnum  < -1 || obj->in_obj->pIndexData->vnum == cont_vnum )
                return TRUE;
        }
        if ( obj->contains )
            found = has_incontainer( obj->contains, vnum, cont_vnum, cont_name );
        if ( found )
            return TRUE;
    }
    return FALSE;
}

bool has_incontainer_name( OBJ_DATA *obj_list, char *argument, sh_int cont_vnum, char *cont_name )
{
    OBJ_DATA *obj;
    bool found = FALSE;
    for ( obj = obj_list; obj; obj = obj->next_content )
    {
/*      Found the right object, now to check to see if the container is right */
        if ( is_name( argument, obj->name ) && obj->in_obj )
        {
            if ( cont_vnum == -1 && is_name( cont_name, obj->in_obj->name ) )
                return TRUE;
            if ( cont_vnum  < -1 || obj->in_obj->pIndexData->vnum == cont_vnum )
                return TRUE;
        }
        if ( obj->contains )
            found = has_incontainer_name( obj->contains, argument, cont_vnum, cont_name );
        if ( found )
            return TRUE;
    }
    return FALSE;
}

/*
 * Check if there's a mob with given vnum in the room
 */
bool get_mob_vnum_room( CHAR_DATA *ch, sh_int vnum )
{
    CHAR_DATA *mob;
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
	if ( IS_NPC( mob ) && mob->pIndexData->vnum == vnum )
	    return TRUE;
    return FALSE;
}

bool is_mob_room( CHAR_DATA *ch, char *argument, bool ch_pc )
{
    CHAR_DATA *rch;

    if( IS_NULLSTR( argument ) )
        return FALSE;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if( rch->in_room == NULL )
            continue;

        if ( ((ch_pc && !IS_NPC(rch)) || (!ch_pc && IS_NPC(rch) ))
        && is_name( argument, rch->name ) )
            return TRUE;

    }
    return FALSE;
}

/* Returns true if mobile is loaded somewhere in the world */
bool is_mob_world( char *argument )
{
    CHAR_DATA *wch;

    if( IS_NULLSTR( argument ) )
        return FALSE;

    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
        if( wch->in_room == NULL )
            continue;
        if( IS_NPC(wch) && is_name( argument, wch->name ) )
            return TRUE;
    }
    return FALSE;
}

bool is_pc_world( char *argument )
{
    CHAR_DATA *wch;

    if( IS_NULLSTR( argument ) )
        return FALSE;

    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
        if( wch->in_room == NULL )
            continue;
        if( !IS_NPC(wch) && is_name( argument, wch->name ) )
            return TRUE;
    }
    return FALSE;
}

/*
 * Check if there's an object with given vnum in the room
 */
bool get_obj_vnum_room( CHAR_DATA *ch, sh_int vnum )
{
    OBJ_DATA *obj;
    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	if ( obj->pIndexData->vnum == vnum )
	    return TRUE;
    return FALSE;
}

/* ---------------------------------------------------------------------
 * CMD_EVAL
 * This monster evaluates an if/or/and statement
 * There are five kinds of statement:
 * 1) keyword and value (no $-code)	    if random 30
 * 2) keyword, comparison and value	    if people > 2
 * 3) keyword and actor		    	    if isnpc $n
 * 4) keyword, actor and value		    if carries $n sword
 * 5) keyword, actor, comparison and value  if level $n >= 10
 *
 *----------------------------------------------------------------------
 */
int cmd_eval( sh_int vnum, char *line, int check,
	CHAR_DATA *mob, CHAR_DATA *ch, 
	const void *arg1, const void *arg2, CHAR_DATA *rch )
{
    CHAR_DATA *lval_char = mob;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    OBJ_DATA  *lval_obj = NULL;

    char *original, buf[MAX_INPUT_LENGTH], code;
    char buf2[MAX_INPUT_LENGTH];
    int lval = 0, oper = 0, rval = -1, cvn;
    int door;

    original = line;
    line = one_argument( line, buf );
    if ( buf[0] == '\0' || mob == NULL )
	return FALSE;

    /*
     * If this mobile has no target, let's assume our victim is the one
     *
     * On second thought, let's not.  This is a ridiculous assumption to make (5/26/06 gkl)
    if ( mob->mprog_target == NULL )
	mob->mprog_target = ch;
     */
    switch ( check )
    {
	/*
	 * Case 1: keyword and value
	 */
	case CHK_RAND:
	    return( atoi( buf ) < number_percent() );
        case CHK_PRAND:
            if ( mob->prand < 0 )
	        mob->prand = number_percent();
            return ( atoi( buf ) < mob->prand );
        case CHK_PPM:
            if ( mob->prand < 0 )
                mob->prand = number_range(1,1000000);
            return ( atoi ( buf ) < mob->prand );
	case CHK_MOBHERE:
	    if ( is_number( buf ) )
		return( get_mob_vnum_room( mob, atoi(buf) ) );
	    else
                return( is_mob_room( mob, buf, FALSE ) );
	case CHK_OBJHERE:
	    if ( is_number( buf ) )
		return( get_obj_vnum_room( mob, atoi(buf) ) );
	    else
		return( (bool) (get_obj_here( mob, buf, FALSE) != NULL) );
        case CHK_MOBEXISTS:
//	    return( (bool) (get_char_world( mob, buf, FALSE) != NULL) );
            return( is_mob_world( buf ) );
        case CHK_PCEXISTS:
            return( is_pc_world( buf ) );
	case CHK_OBJEXISTS:
	    return( (bool) (get_obj_world( mob, buf, FALSE) != NULL) );
	/*
	 * Case 2 begins here: We sneakily use rval to indicate need
	 * 		       for numeric eval...
	 */
	case CHK_PEOPLE:
	    rval = count_people_room( mob, 0 ); break;
	case CHK_PLAYERS:
	    rval = count_people_room( mob, 1 ); break;
	case CHK_MOBS:
	    rval = count_people_room( mob, 2 ); break;
	case CHK_CLONES:
	    rval = count_people_room( mob, 3 ); break;
	case CHK_ORDER:
	    rval = get_order( mob ); break;
	case CHK_HOUR:
	    rval = time_info.hour; break;
        case CHK_PCAROUND:
            rval = count_neighbors( mob ); break;
	default:;
    }

    /*
     * Case 2 continued: evaluate expression
     */
    if ( rval >= 0 )
    {
	if ( (oper = keyword_lookup( fn_evals, buf )) < 0 )
	{
	    sprintf( buf, "Cmd_eval: prog %d syntax error(2) '%s'",
		vnum, original );
	    bug( buf, 0 );
	    return FALSE;
	}
	one_argument( line, buf );
	lval = rval;
	rval = atoi( buf );
	return( num_eval( lval, oper, rval ) );
    }

    /*
     * Case 3,4,5: Grab actors from $* codes
     */
    if ( buf[0] != '$' || buf[1] == '\0' )
    {
	sprintf( buf, "Cmd_eval: prog %d syntax error(3) '%s'",
		vnum, original );
	bug( buf, 0 );
        return FALSE;
    }
    else
        code = buf[1];
    switch( code )
    {
    	case 'i':
            lval_char = mob; break;
        case 'n':
            lval_char = ch; break;
        case 't':
            lval_char = vch; break;
        case 'r':
            lval_char = rch == NULL ? get_random_char( mob ) : rch ; break;
        case 'o':
            lval_obj = obj1; break;
        case 'p':
            lval_obj = obj2; break;
	case 'q':
	    lval_char = mob->mprog_target; break;
        case 'f':
            lval_char = mob->master; break;
	default:
	    sprintf( buf, "Cmd_eval: prog %d syntax error(4) '%s'",
		vnum, original );
	    bug( buf, 0 );
	    return FALSE;
    }
    /*
     * From now on, we need an actor, so if none was found, bail out
     */
    if ( lval_char == NULL && lval_obj == NULL )
    	return FALSE;

    /*
     * Case 3: Keyword, comparison and value
     */
    switch( check )
    {
	case CHK_ISPC:
            return( lval_char != NULL && !IS_NPC( lval_char ) );
	case CHK_ISVAMP:
		return( lval_char != NULL && (IS_SET( lval_char->shiftbits, PERM_VAMP ) ||
															IS_SET( lval_char->shiftbits, TEMP_VAMP ))  );
		case CHK_ISWERE:
            return( lval_char != NULL && IS_WERECREATURE(lval_char) );													
        case CHK_ISNPC:
            return( lval_char != NULL && IS_NPC( lval_char ) );
	case CHK_ISIMMORT:
            return( lval_char != NULL && IS_IMMORTAL( lval_char ) );
        case CHK_ISCHARM: /* A relic from MERC 2.2 MOBprograms */
            return( lval_char != NULL && IS_AFFECTED( lval_char, AFF_CHARM ) );
        case CHK_ISFOLLOW:
            return( lval_char != NULL && lval_char->master != NULL 
		 && lval_char->master->in_room == lval_char->in_room );
	case CHK_ISACTIVE:
	    return( lval_char != NULL && lval_char->position > POS_SLEEPING );
	case CHK_ISDELAY:
	    return( lval_char != NULL && lval_char->mprog_delay > 0 );
	case CHK_ISVISIBLE:
            switch( code )
            {
                default :
                case 'i':
                case 'n':
                case 't':
                case 'r':
		case 'q':
                case 'f':
	    	    return( lval_char != NULL && can_see( mob, lval_char ) );
		case 'o':
		case 'p':
	    	    return( lval_obj != NULL && can_see_obj( mob, lval_obj ) );
	    }
	case CHK_HASTARGET:
	    return( lval_char != NULL && lval_char->mprog_target != NULL
		&&  lval_char->in_room == lval_char->mprog_target->in_room );
	case CHK_ISTARGET:
	    return( lval_char != NULL && mob->mprog_target == lval_char );
        case CHK_RECOG:
/*      if recog $n <set vnum> <flag>, but <flag> is handled by is_mprecog */
            return( is_mprecog( lval_char, line ) );
	case CHK_FIGHTLAG:
	    return( lval_char != NULL && in_fightlag(lval_char) );
	case CHK_INPK:
	    return( lval_char != NULL 
                 && !IS_NPC(lval_char)
                 && (current_time - ch->pcdata->lastpk) < FIGHT_LAG );
        case CHK_PCHERE:
            return( is_mob_room( mob, lval_char->name, TRUE ) );


	default:;
     }

     /* 
      * Case 4: Keyword, actor and value
      */
    line = one_argument( line, buf );
    switch( check )
      {
      case CHK_AFFECTED:
	return( lval_char != NULL 
		&&  IS_SET(lval_char->affected_by, flag_lookup(buf, affect_flags)) );
      case CHK_AFFECTED2:
	return( lval_char != NULL &&
		IS_SET( lval_char->affected_by2, flag_lookup( buf, affect2_flags ) ) );
      case CHK_AFFECTED3:
	return( lval_char != NULL &&
		IS_SET( lval_char->affected_by3, flag_lookup( buf, affect3_flags ) ) );
      case CHK_ACT:
	    return( lval_char != NULL 
		&&  IS_SET(lval_char->act_bits, flag_lookup(buf, act_flags)) );
	case CHK_IMM:
	    return( lval_char != NULL 
		&&  IS_SET(lval_char->imm_flags, flag_lookup(buf, imm_flags)) );
	case CHK_OFF:
	    return( lval_char != NULL 
		&&  IS_SET(lval_char->off_flags, flag_lookup(buf, off_flags)) );
	case CHK_CARRIES:
	    if ( is_number( buf ) )
		return( lval_char != NULL && has_item_inventory( lval_char, atoi(buf) ) );
	    else
		return( lval_char != NULL && (get_obj_carry( lval_char, buf, lval_char, FALSE ) != NULL) );
        case CHK_USES:
	case CHK_WEARS:
	    if ( is_number( buf ) )
		return( lval_char != NULL && has_item( lval_char, atoi(buf), -1, TRUE ) );
	    else
		return( lval_char != NULL && (get_obj_wear( lval_char, buf, FALSE ) != NULL) );
        case CHK_ISCONTAINED:
/*      if iscontained <$n> <$object name/vnum> <container name/vnum> */
            one_argument( line, buf2 );
            if ( !IS_NULLSTR( buf2 ) )  /* then they specified a container */
                if ( is_number( buf2 ) )
                    cvn = atoi( buf2 ); /* cvn is the container's vnum */
                else
                    cvn = -1;           /* cvn=-1 indicates cont_name should be used */
            else
                cvn = -2;               /* cvn=-2 means no container specified */

            if ( is_number( buf ) )     /* if obj vnum is specified */
                return( lval_char != NULL 
                     && has_incontainer( lval_char->carrying, atoi(buf), (sh_int) cvn, buf2 ) );
            else                        /* if obj name is specified */
                return( lval_char != NULL 
                     && has_incontainer_name( lval_char->carrying, buf, (sh_int) cvn, buf2 ) );
        case CHK_HAS:
        case CHK_HASANYWHERE:
            if ( lval_char == NULL ) return FALSE;
            if ( is_number( buf ) )
                return ( has_item( lval_char, atoi(buf), -2, FALSE ) 
                      || has_incontainer( lval_char->carrying, atoi(buf), -2, NULL ) );
            else
                return ( (get_obj_carry( lval_char, buf, lval_char, FALSE ) != NULL)
                      || (get_obj_wear( lval_char, buf, FALSE ) != NULL)
                      || has_incontainer_name( lval_char->carrying, buf, -2, NULL ) );
        case CHK_ARRIVED:
            if ( lval_char == NULL || lval_char->was_in_room == NULL ) return FALSE;

            if ( !str_cmp( buf, "n" ) || !str_cmp( buf, "north" ) ) door = 0;
            else if ( !str_cmp( buf, "e" ) || !str_cmp( buf, "east"  ) ) door = 1;
            else if ( !str_cmp( buf, "s" ) || !str_cmp( buf, "south" ) ) door = 2;
            else if ( !str_cmp( buf, "w" ) || !str_cmp( buf, "west"  ) ) door = 3;
            else if ( !str_cmp( buf, "u" ) || !str_cmp( buf, "up"    ) ) door = 4;
            else if ( !str_cmp( buf, "d" ) || !str_cmp( buf, "down"  ) ) door = 5;
            else 
                return FALSE;
            if ( mob->in_room->exit[door] == NULL ) return FALSE;
            return ( mob->in_room->exit[door]->u1.to_room == lval_char->was_in_room );
        case CHK_WASIN:
            if ( lval_char == NULL || lval_char->was_in_room == NULL ) return FALSE;
            door = atoi(buf);
            return ( door > 0 && lval_char->was_in_room->vnum == door );

/*	case CHK_HAS:
	    return( lval_char != NULL && has_item( lval_char, -1, item_lookup(buf), FALSE ) );
	case CHK_USES:
	    return( lval_char != NULL && has_item( lval_char, -1, item_lookup(buf), TRUE ) );
*/
	case CHK_NAME:
            switch( code )
            {
                default :
                case 'i':
                case 'n':
                case 't':
                case 'r':
		case 'q':
                case 'f':
		    return( lval_char != NULL && is_name( buf, lval_char->name ) );
		case 'o':
		case 'p':
		    return( lval_obj != NULL && is_name( buf, lval_obj->name ) );
	    }
	case CHK_POS:
	    return( lval_char != NULL && lval_char->position == position_lookup( buf ) );
	case CHK_GUILD:
	    return( lval_char != NULL && lval_char->guild == guild_lookup( buf ) );
	case CHK_RACE:
	    return( lval_char != NULL && lval_char->race == race_lookup( buf ) );
        case CHK_CLASS:
            return( lval_char != NULL && lval_char->class == class_lookup( buf ) );
	case CHK_OBJTYPE:
	    return( lval_obj != NULL && lval_obj->item_type == item_lookup( buf ) );
        case CHK_DEITY:
            if ( IS_NPC( lval_char ) ) 
                return FALSE;
            else
                return ( lval_char->pcdata->god == god_lookup( buf ) );
	default:;
    }

    /*
     *  Case 4.5: keyword, actor, value, comparator, value
     */
    if ( check == CHK_SKILL )
    {
        /* lval_char = $n, buf = skill name, line = comparator, value */
        char skill[MAX_STRING_LENGTH];
        int sn;
        if ( IS_NPC(lval_char) ) return FALSE;
        if ( (sn = skill_lookup(buf)) < 0 )
        {
            sprintf( buf, "cmd_eval: prog %d no such skill '%s'",
              vnum, skill);
            bug( buf, 0 );
            return FALSE;
        }
        else
        {
            /* NOTE: this ifcheck does ***NOT*** work on wereskills */
            if ( ch->level < skill_table[sn].skill_level[ch->class] )
                lval = 0;
            else
                lval = lval_char->pcdata->learned[sn];
        }

        line = one_argument( line, buf  );
        if ( (oper = keyword_lookup( fn_evals, buf )) < 0 )
        {
            sprintf( buf, "Cmd_eval: prog %d invalid comparator: '%s'",
              vnum, original );
            bug( buf, 0 );
            return FALSE;
        }
        one_argument( line, buf );
        rval = atoi( buf );
        return num_eval( lval, oper, rval );
    }

    /*
     * Case 5: Keyword, actor, comparison and value
     */
    if ( (oper = keyword_lookup( fn_evals, buf )) < 0 )
    {
	sprintf( buf, "Cmd_eval: prog %d syntax error(5): '%s'",
		vnum, original );
	bug( buf, 0 );
	return FALSE;
    }
    one_argument( line, buf );
    rval = atoi( buf );

    switch( check )
    {
	case CHK_VNUM:
	    switch( code )
            {
                default :
                case 'i':
                case 'n':
                case 't':
                case 'r':
		case 'q':
                case 'f':
                    if( lval_char != NULL && IS_NPC( lval_char ) )
                        lval = lval_char->pIndexData->vnum;
                    break;
                case 'o':
                case 'p':
                     if ( lval_obj != NULL )
                        lval = lval_obj->pIndexData->vnum;
            }
            break;
	case CHK_HPCNT:
	    if ( lval_char != NULL ) lval = (lval_char->hit * 100)/(UMAX(1,lval_char->max_hit)); break;
	case CHK_ROOM:
	    if ( lval_char != NULL && lval_char->in_room != NULL )
		lval = lval_char->in_room->vnum; break;
        case CHK_SEX:
	    if ( lval_char != NULL ) lval = lval_char->sex; break;
        case CHK_LEVEL:
            if ( lval_char != NULL ) lval = lval_char->level; break;
		case CHK_BAKING:
            if ( lval_char != NULL ) lval = lval_char->pcdata->baking; break;
		case CHK_FLETCHING:
            if ( lval_char != NULL ) lval = lval_char->pcdata->fletching; break;
		case CHK_TAILORING:
            if ( lval_char != NULL ) lval = lval_char->pcdata->tailoring; break;
	case CHK_MONEY:  /* Money is converted to silver... */
	    if ( lval_char != NULL ) 
		lval = lval_char->silver + (lval_char->gold * 100); break;
	case CHK_OBJVAL0:
            if ( lval_obj != NULL ) lval = lval_obj->value[0]; break;
        case CHK_OBJVAL1:
            if ( lval_obj != NULL ) lval = lval_obj->value[1]; break;
        case CHK_OBJVAL2: 
            if ( lval_obj != NULL ) lval = lval_obj->value[2]; break;
        case CHK_OBJVAL3:
            if ( lval_obj != NULL ) lval = lval_obj->value[3]; break;
	case CHK_OBJVAL4:
	    if ( lval_obj != NULL ) lval = lval_obj->value[4]; break;
	case CHK_GRPSIZE:
	    if( lval_char != NULL ) lval = count_people_room( lval_char, 4 ); break;
	default:
            return FALSE;
    }
    return( num_eval( lval, oper, rval ) );
}

/*
 * ------------------------------------------------------------------------
 * EXPAND_ARG
 * This is a hack of act() in comm.c. I've added some safety guards,
 * so that missing or invalid $-codes do not crash the server
 * ------------------------------------------------------------------------
 */
void expand_arg( char *buf, 
	const char *format, 
	CHAR_DATA *mob, CHAR_DATA *ch, 
	const void *arg1, const void *arg2, CHAR_DATA *rch,
        bool serious )
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
    const char *someone = "someone";
    const char *something = "something";
    const char *someones = "someone's";
 
    char fname[MAX_INPUT_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;
 
    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
        return;

    point   = buf;
    str     = format;
    while ( *str != '\0' )
    {
    	if ( *str != '$' )
        {
            *point++ = *str++;
            continue;
        }
        ++str;

        switch ( *str )
        {
            default:  bug( "Expand_arg: bad code %d.", *str );
                          i = " <@@@> ";                        break;
            case 'i':
		one_argument( mob->name, fname );
		i = fname;                         		break;
            case 'I': i = mob->short_descr;                     break;
            case 'n': 
		i = someone;
		if ( ch != NULL && (serious || can_see( mob, ch )) )
		{
            	    one_argument( ch->name, fname );
		    i = capitalize(fname);
		}						break;
            case 'N': 
	    	i = (ch != NULL && (serious || can_see( mob, ch )) )
		? ( IS_NPC( ch ) ? ch->short_descr : ch->name )
		: someone;                         		break;
            case 't': 
		i = someone;
		if ( vch != NULL && (serious || can_see( mob, vch )) )
		{
            	     one_argument( vch->name, fname );
		     i = capitalize(fname);
		}						break;
            case 'T': 
	    	i = (vch != NULL && (serious || can_see( mob, vch )) )
		? ( IS_NPC( vch ) ? vch->short_descr : vch->name )
		: someone;                         		break;
            case 'r': 
		if ( rch == NULL ) 
		    rch = get_random_char( mob );
		i = someone;
		if( rch != NULL && (serious || can_see( mob, rch )) )
		{
                    one_argument( rch->name, fname );
		    i = capitalize(fname);
		} 						break;
            case 'R': 
		if ( rch == NULL ) 
		    rch = get_random_char( mob );
		i  = ( rch != NULL && (serious || can_see( mob, rch )) )
		? ( IS_NPC( ch ) ? ch->short_descr : ch->name )
		:someone;					break;
	    case 'q':
		i = someone;
		if ( mob->mprog_target != NULL 
                &&   (serious || can_see( mob, mob->mprog_target )) )
	        {
		    one_argument( mob->mprog_target->name, fname );
		    i = capitalize( fname );
		} 						break;
	    case 'Q':
	    	i = (mob->mprog_target != NULL 
                && (serious || can_see( mob, mob->mprog_target )) )
		? ( IS_NPC( mob->mprog_target ) ? mob->mprog_target->short_descr : mob->mprog_target->name )
		: someone;                         		break;
            case 'j': i = he_she  [URANGE(0, mob->sex, 2)];     break;
            case 'e': 
	    	i = (ch != NULL && (serious || can_see( mob, ch )) )
		? he_she  [URANGE(0, ch->sex, 2)]        
		: someone;					break;
            case 'E': 
	    	i = (vch != NULL && (serious || can_see( mob, vch )) )
		? he_she  [URANGE(0, vch->sex, 2)]        
		: someone;					break;
            case 'J': 
		i = (rch != NULL && (serious || can_see( mob, rch )) )
		? he_she  [URANGE(0, rch->sex, 2)]        
		: someone;					break;
	    case 'X':
		i = (mob->mprog_target != NULL 
                && (serious || can_see( mob, mob->mprog_target)) )
		? he_she  [URANGE(0, mob->mprog_target->sex, 2)]
		: someone;					break;
            case 'k': i = him_her [URANGE(0, mob->sex, 2)];	break;
            case 'm': 
	    	i = (ch != NULL && (serious || can_see( mob, ch )) )
		? him_her [URANGE(0, ch  ->sex, 2)]
		: someone;        				break;
            case 'M': 
	    	i = (vch != NULL && (serious || can_see( mob, vch )) )
		? him_her [URANGE(0, vch ->sex, 2)]        
		: someone;					break;
            case 'K': 
		if ( rch == NULL ) 
		    rch = get_random_char( mob );
		i = (rch != NULL && (serious || can_see( mob, rch )) )
		? him_her [URANGE(0, rch ->sex, 2)]
		: someone;					break;
            case 'Y': 
	    	i = (mob->mprog_target != NULL 
                && (serious || can_see( mob, mob->mprog_target )) )
		? him_her [URANGE(0, mob->mprog_target->sex, 2)]        
		: someone;					break;
            case 'l': i = his_her [URANGE(0, mob ->sex, 2)];    break;
            case 's': 
	    	i = (ch != NULL && (serious || can_see( mob, ch )) )
		? his_her [URANGE(0, ch ->sex, 2)]
		: someones;					break;
            case 'S': 
	    	i = (vch != NULL && (serious || can_see( mob, vch )) )
		? his_her [URANGE(0, vch ->sex, 2)]
		: someones;					break;
            case 'L': 
		if ( rch == NULL ) 
		    rch = get_random_char( mob );
		i = ( rch != NULL && (serious || can_see( mob, rch )) ) 
		? his_her [URANGE(0, rch ->sex, 2)]
		: someones;					break;
            case 'Z': 
	    	i = (mob->mprog_target != NULL 
                && (serious || can_see( mob, mob->mprog_target )) )
		? his_her [URANGE(0, mob->mprog_target->sex, 2)]
		: someones;					break;
	    case 'o':
		i = something;
		if ( obj1 != NULL && can_see_obj( mob, obj1 ) )
		{
            	    one_argument( obj1->name, fname );
                    i = fname;
		} 						break;
            case 'O':
                i = (obj1 != NULL && can_see_obj( mob, obj1 ))
                ? obj1->short_descr
                : something;					break;
            case 'p':
		i = something;
		if ( obj2 != NULL && can_see_obj( mob, obj2 ) )
		{
            	    one_argument( obj2->name, fname );
            	    i = fname;
		} 						break;
            case 'P':
            	i = (obj2 != NULL && can_see_obj( mob, obj2 ))
                ? obj2->short_descr
                : something;					break;
            case '#':
                sprintf(buf2,"%d",mob->prand);
                i = buf2;                                       break;
            case 'f': 
		i = someone;
		if ( mob->master != NULL 
                &&  (serious || can_see( mob, mob->master )) )
		{
            	    one_argument( mob->master->name, fname );
		    i = capitalize(fname);
		}						break;
            case 'F': 
	    	i = (mob->master != NULL 
                  && (serious || can_see( mob, mob->master )) )
		? ( IS_NPC( mob->master ) ? mob->master->short_descr : mob->master->name )
		: someone;                         		break;

        }
 
        ++str;
        while ( ( *point = *i ) != '\0' )
            ++point, ++i;
 
    }
    *point = '\0';
 
    return;
}    

/*
 * ------------------------------------------------------------------------
 *  PROGRAM_FLOW
 *  This is the program driver. It parses the mob program code lines
 *  and passes "executable" commands to interpret()
 *  Lines beginning with 'mob' are passed to mob_interpret() to handle
 *  special mob commands (in mob_cmds.c)
 *-------------------------------------------------------------------------
 */

#define MAX_NESTED_LEVEL 12 /* Maximum nested if-else-endif's (stack size) */
#define BEGIN_BLOCK       0 /* Flag: Begin of if-else-endif block */
#define IN_BLOCK         -1 /* Flag: Executable statements */
#define END_BLOCK        -2 /* Flag: End of if-else-endif block */
#define MAX_CALL_LEVEL    5 /* Maximum nested calls */

void program_flow( 
        sh_int pvnum,  /* For diagnostic purposes */
	char *source,  /* the actual MOBprog code */
	CHAR_DATA *mob, CHAR_DATA *ch, const void *arg1, const void *arg2 )
{
    CHAR_DATA *rch = NULL;
    char *code, *line;
    char buf[MAX_STRING_LENGTH];
    char control[MAX_INPUT_LENGTH], data[MAX_STRING_LENGTH];

    static int call_level; /* Keep track of nested "mpcall"s */

    int level, eval, check;
    int state[MAX_NESTED_LEVEL], /* Block state (BEGIN,IN,END) */
	cond[MAX_NESTED_LEVEL];  /* Boolean value based on the last if-check */

    sh_int mvnum = mob->pIndexData->vnum;

    if( ++call_level > MAX_CALL_LEVEL )
    {
        bug( "MOBprogs: MAX_CALL_LEVEL exceeded, vnum %d", 
            mob->pIndexData->vnum );
        call_level--;
	return;
    }

    /*
     * Reset "stack"
     */
    for ( level = 0; level < MAX_NESTED_LEVEL; level++ )
    {
    	state[level] = IN_BLOCK;
        cond[level]  = TRUE;
    }
    level = 0;

    code = source;
    /*
     * Parse the MOBprog code
     */
    while ( *code )
    {
	bool first_arg = TRUE;
	char *b = buf, *c = control, *d = data;

	/*
	 * Get a command line. We sneakily get both the control word
	 * (if/and/or) and the rest of the line in one pass.
	 */
	while( isspace( *code ) && *code ) code++;
	while ( *code )
	{
	    if ( *code == '\n' || *code == '\r' )
		break;
	    else if ( isspace(*code) )
	    {
		if ( first_arg )
		    first_arg = FALSE;
		else
		    *d++ = *code;
	    }
	    else
	    {
		if ( first_arg )
		   *c++ = *code;
		else
		   *d++ = *code;
	    }
	    *b++ = *code++;
	}
	*b = *c = *d = '\0';

	if ( buf[0] == '\0' )
	    break;
	if ( buf[0] == '*' ) /* Comment */
	    continue;

        line = data;
	/* 
	 * Match control words
	 */
	if ( !str_cmp( control, "if" ) )
	{
	    if ( state[level] == BEGIN_BLOCK )
	    {
		sprintf( buf, 
                    "Mobprog: misplaced if statement, mob %d prog %d",
                    mvnum, pvnum );
		bug( buf, 0 );
                call_level--;
		return;
	    }
	    state[level] = BEGIN_BLOCK;
            if ( ++level >= MAX_NESTED_LEVEL )
            {
		sprintf( buf, 
                    "Mobprog: Max nested level exceeded, mob %d prog %d",
                    mvnum, pvnum );
		bug( buf, 0 );
                call_level--;
		return;
	    }
	    if ( level && cond[level-1] == FALSE ) 
	    {
		cond[level] = FALSE;
		continue;
	    }
	    line = one_argument( line, control );
	    if ( ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
	    {
		cond[level] = 
                    cmd_eval( pvnum, line, check, mob, ch, arg1, arg2, rch );
	    }
	    else
	    {
		sprintf( buf, "Mobprog: invalid if_check (if), mob %d prog %d",
			mvnum, pvnum );
		bug( buf, 0 );
                call_level--;
		return;
	    }
	    state[level] = END_BLOCK;
    	}
	else if ( !str_cmp( control, "or" ) )
	{
	    if ( !level || state[level-1] != BEGIN_BLOCK )
	    {
		sprintf( buf, "Mobprog: or without if, mob %d prog %d",
			mvnum, pvnum );
		bug( buf, 0 );
                call_level--;
		return;
	    }
	    if ( level && cond[level-1] == FALSE ) continue;
	    line = one_argument( line, control );
	    if ( ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
	    {
		eval = cmd_eval( pvnum, line, check, mob, ch, arg1, arg2, rch );
	    }
	    else
            {
		sprintf( buf, "Mobprog: invalid if_check (or), mob %d prog %d",
			mvnum, pvnum );
		bug( buf, 0 );
                call_level--;
		return;
            }
            cond[level] = (eval == TRUE) ? TRUE : cond[level];
    	}
	else if ( !str_cmp( control, "and" ) )
	{
	    if ( !level || state[level-1] != BEGIN_BLOCK )
	    {
		sprintf( buf, "Mobprog: and without if, mob %d prog %d",
			mvnum, pvnum );
		bug( buf, 0 );
                call_level--;
		return;
	    }
	    if ( level && cond[level-1] == FALSE ) continue;
	    line = one_argument( line, control );
	    if ( ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
	    {
		eval = cmd_eval( pvnum, line, check, mob, ch, arg1, arg2, rch );
	    }
	    else
	    {
		sprintf( buf, "Mobprog: invalid if_check (and), mob %d prog %d",
			mvnum, pvnum );
		bug( buf, 0 );
                call_level--;
		return;
	    }
	    cond[level] = (cond[level] == TRUE) && (eval == TRUE) ? TRUE : FALSE;
    	}
	else if ( !str_cmp( control, "endif" ) )
	{
	    if ( !level || state[level-1] != BEGIN_BLOCK )
	    {
		sprintf( buf, "Mobprog: endif without if, mob %d prog %d",
			mvnum, pvnum );
		bug( buf, 0 );
                call_level--;
		return;
	    }
	    cond[level] = TRUE;
	    state[level] = IN_BLOCK;
            state[--level] = END_BLOCK;
        }
	else if ( !str_cmp( control, "else" ) )
	{
	    if ( !level || state[level-1] != BEGIN_BLOCK )
	    {
		sprintf( buf, "Mobprog: else without if, mob %d prog %d",
			mvnum, pvnum );
		bug( buf, 0 );
                call_level--;
		return;
	    }
	    if ( level && cond[level-1] == FALSE ) continue;
            state[level] = IN_BLOCK;
            cond[level] = (cond[level] == TRUE) ? FALSE : TRUE;
        }
    	else if ( cond[level] == TRUE
	&& ( !str_cmp( control, "break" ) || !str_cmp( control, "end" ) ) )
	{
	    call_level--;
            return;
	}
	else if ( !str_cmp( control, "pause" ) )
	{
	    MPDELAY_DATA *mpd;

	    if ( level != 0 )
	    {
		sprintf(buf, "Mobprog: wait within an if, mob %d prog %d",
		    mvnum, pvnum );
		bug( buf, 0 );
		call_level--;
		return;
	    }

	    mpd = new_mpdelay();

	    mpd->pvnum = pvnum;
	    mpd->code = str_dup(code);
	    mpd->mob = mob;
	    mpd->ch = ch;
	    mpd->arg1 = arg1;
            mpd->arg2 = arg2;

	    mpd->next = mpdelay_list;
	    mpdelay_list = mpd;

	    call_level--;
	    return;

	}
	else if ( (!level || cond[level] == TRUE) && buf[0] != '\0' )
	{
            /* need to trap cases where the mob is destroyed by its own prog */
            if ( mob->in_room == NULL )
            {
                bug( "MOBprogs: trapped dead mob running mprog vnum %d",
                    mob->pIndexData->vnum );
                call_level--;
                return;
            }

	    state[level] = IN_BLOCK;

            /*
             * handle mob commands that need to be interpreted without respect
             * to visibility.  pre-interpret mob commands and look up how their
             * arguments should be expanded
             */
            {
                int cmd;
                bool found = FALSE;
                char command[MAX_INPUT_LENGTH];
                one_argument( data, command );
                if ( !IS_NULLSTR(command) && !str_cmp( control, "mob" ) )
                {
                    for ( cmd = 0; mob_cmd_table[cmd].name[0] != '\0'; cmd++ )
                    {
                        if ( command[0] == mob_cmd_table[cmd].name[0]
                        &&   !str_prefix( command, mob_cmd_table[cmd].name ) )
                        {
                            expand_arg( data, buf, mob, ch, arg1, arg2, rch,
                                mob_cmd_table[cmd].serious );
                            found = TRUE;
                        }
                    }
                }

                if ( !found )
                    expand_arg( data, buf, mob, ch, arg1, arg2, rch, FALSE );
            }

	    if ( !str_cmp( control, "mob" ) )
	    {
		/* 
		 * Found a mob restricted command, pass it to mob interpreter
		 */
		line = one_argument( data, control );
		mob_interpret( mob, line );
	    }
            else if ( !str_cmp( control, "hardcall" ) )
            {
                MPROG_CODE *prg;
                line = one_argument( line, control );
                line = one_argument( line, control );
                if ( IS_NULLSTR(control) )
                {
                    sprintf( buf, 
                        "hardcall: missing arguments from mob %d prog %d",
                        mvnum, pvnum );
                    bug ( buf, 0 );
                    call_level--;
                    return;
                }
                if ( ( prg = get_mprog_index( atoi(control) ) ) == NULL )
                {
                    sprintf( buf, 
                      "hardcall: bad call vnum from mob %d prog %d (l=%s,c=%s)",
                      mvnum, pvnum,line,control );
                    bug ( buf, 0 );
                    call_level--;
                    return;
                }

                program_flow( prg->vnum, prg->code, mob, ch, arg1, arg2 );
            }
	    else
	    {
		/* 
		 * Found a normal mud command, pass it to interpreter
		 */
		interpret( mob, data );
	    }
	}
    }
    call_level--;
    return;
}

/* 
 * ---------------------------------------------------------------------
 * Trigger handlers. These are called from various parts of the code
 * when an event is triggered.
 * ---------------------------------------------------------------------
 */

/*
 * A general purpose string trigger. Matches argument to a string trigger
 * phrase.
 */
void mp_act_trigger( 
	char *argument, CHAR_DATA *mob, CHAR_DATA *ch, 
	const void *arg1, const void *arg2, int type )
{
    MPROG_LIST *prg;
    MPROG_LIST *eprg;
    bool found = FALSE;
    eprg = NULL;

    for ( prg = mob->pIndexData->mprogs; prg != NULL; prg = prg->next )
    {
    	if (prg->trig_type == type && !str_infix(prg->trig_phrase, argument) )
        {
	    program_flow( prg->vnum, prg->code, mob, ch, arg1, arg2 );
            found = TRUE;
	    break;
	}
        else if ( type == TRIG_SPEECH && prg->trig_type == TRIG_SELSE )
            eprg = prg;
    }

    if ( type == TRIG_SPEECH && !found && eprg != NULL )
        program_flow( eprg->vnum, eprg->code, mob, ch, arg1, arg2 );
    return;
}

/*
 * A general purpose percentage trigger. Checks if a random percentage
 * number is less than trigger phrase
 */
bool mp_percent_trigger( 
	CHAR_DATA *mob, CHAR_DATA *ch, 
	const void *arg1, const void *arg2, int type )
{
    MPROG_LIST *prg;

    for ( prg = mob->pIndexData->mprogs; prg != NULL; prg = prg->next )
    {
    	if ( prg->trig_type == type 
	&&   number_percent() <= atoi( prg->trig_phrase ) )
        {
	    program_flow( prg->vnum, prg->code, mob, ch, arg1, arg2 );
	    return ( TRUE );
	}
    }
    return ( FALSE );
}

bool mp_ppm_trigger(
        CHAR_DATA *mob, CHAR_DATA *ch,
        const void *arg1, const void *arg2, int type )
{
    MPROG_LIST *prg;

    for ( prg = mob->pIndexData->mprogs; prg != NULL; prg = prg->next )
    {
        if ( prg->trig_type == type
        &&   number_range(1,1000000) <= atoi( prg->trig_phrase ) )
        {
            program_flow( prg->vnum, prg->code, mob, ch, arg1, arg2 );
            return ( TRUE );
        }
    }
    return ( FALSE );
}

void mp_bribe_trigger( CHAR_DATA *mob, CHAR_DATA *ch, int amount )
{
    MPROG_LIST *prg, *hprg;
    int pamt, maxamt = 0;
    hprg = NULL;

/*  Rewritten 8/26/08 gkl to execute only the BRIBE trigger closest
    to the amount given to the mob */
    for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
    {
	if ( prg->trig_type == TRIG_BRIBE
        && ( pamt = atoi(prg->trig_phrase) ) <= amount
        &&   pamt > maxamt )
        {
            maxamt = pamt;
            hprg = prg;
        }
    }
    if ( hprg != NULL )
        program_flow( hprg->vnum, hprg->code, mob, ch, NULL, NULL );

/*
    for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
    {
	if ( prg->trig_type == TRIG_BRIBE
	&&   amount == atoi( prg->trig_phrase ) )
	{
	    program_flow( prg->vnum, prg->code, mob, ch, NULL, NULL );
	    break;
	}
    }
*/
    return;
}

bool mp_exit_trigger( CHAR_DATA *ch, int dir )
{
    CHAR_DATA *mob;
    MPROG_LIST   *prg;

    for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
    {    
	if ( IS_NPC( mob )
	&&   ( HAS_TRIGGER(mob, TRIG_EXIT) || HAS_TRIGGER(mob, TRIG_EXALL) ) )
	{
	    for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
	    {
		/*
		 * Exit trigger works only if the mobile is not busy
		 * (fighting etc.). If you want to be sure all players
		 * are caught, use ExAll trigger
		 */
		if ( prg->trig_type == TRIG_EXIT
		&&  dir == atoi( prg->trig_phrase )
		&&  mob->position == mob->pIndexData->default_pos
		&&  can_see( mob, ch ) )
		{
		    program_flow( prg->vnum, prg->code, mob, ch, NULL, NULL );
		    return TRUE;
		}
		else
		if ( prg->trig_type == TRIG_EXALL
		&&   dir == atoi( prg->trig_phrase ) )
		{
		    program_flow( prg->vnum, prg->code, mob, ch, NULL, NULL );
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}

void mp_give_trigger( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj )
{

    char        buf[MAX_INPUT_LENGTH], *p;
    MPROG_LIST  *prg;
    MPROG_LIST  *eprg;
    bool found = FALSE;

    eprg = NULL;

    for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
    {
	if ( prg->trig_type == TRIG_GIVE )
	{
	    p = prg->trig_phrase;
	    /*
	     * Vnum argument
	     */
	    if ( is_number( p ) )
	    {
		if ( obj->pIndexData->vnum == atoi(p) )
		{
		    program_flow(prg->vnum, prg->code, mob, ch, (void *) obj, NULL);
                    found = TRUE;
		    return;
		}
	    }
	    /*
	     * Object name argument, e.g. 'sword'
	     */
	    else
	    {
	    	while( *p )
	    	{
		    p = one_argument( p, buf );

		    if ( is_name( buf, obj->name )
		    ||   !str_cmp( "all", buf ) )
		    {
		    	program_flow(prg->vnum, prg->code, mob, ch, (void *) obj, NULL);
                        found = TRUE;
		    	return;
		    }
		}
	    }
	}
        else if ( prg->trig_type == TRIG_GELSE )
            eprg = prg;
    }
    if ( !found && eprg != NULL )
        program_flow(eprg->vnum, eprg->code, mob, ch, (void *) obj, NULL );
}

void mp_greet_trigger( CHAR_DATA *ch )
{
    CHAR_DATA *mob;

    for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
    {    
	if ( IS_NPC( mob )
	&& ( HAS_TRIGGER(mob, TRIG_GREET) || HAS_TRIGGER(mob,TRIG_GRALL) ) )
	{
	    /*
	     * Greet trigger works only if the mobile is not busy
	     * (fighting etc.). If you want to catch all players, use
	     * GrAll trigger
	     */
	    if ( HAS_TRIGGER( mob,TRIG_GREET )
	    &&   mob->position == mob->pIndexData->default_pos
	    &&   can_see( mob, ch ) )
                mp_percent_trigger( mob, ch, NULL, NULL, TRIG_GREET ); 
            else if ( HAS_TRIGGER( mob, TRIG_GRALL ) )
                mp_percent_trigger( mob, ch, NULL, NULL, TRIG_GRALL );
	}
    }
    return;
}

void mp_spawn_trigger( CHAR_DATA *ch )
{
   MPROG_LIST *prg;
   for ( prg = ch->pIndexData->mprogs; prg != NULL; prg = prg->next )
        if ( prg->trig_type == TRIG_SPAWN )
        {
            program_flow( prg->vnum, prg->code, ch, NULL, NULL, NULL );
            break;
        }

    return;
}

void mp_hprct_trigger( CHAR_DATA *mob, CHAR_DATA *ch )
{
    MPROG_LIST *prg;

    for ( prg = mob->pIndexData->mprogs; prg != NULL; prg = prg->next )
	if ( ( prg->trig_type == TRIG_HPCNT )
	&& ( (100 * mob->hit / mob->max_hit) < atoi( prg->trig_phrase ) ) )
	{
	    program_flow( prg->vnum, prg->code, mob, ch, NULL, NULL );
	    break;
	}
}

void do_mplistmobs( CHAR_DATA* ch, char* argument )
{
  char mpRegexStr[ MAX_STRING_LENGTH ] = "";
  char assignments[ MAX_STRING_LENGTH ] = "";
  char unassigned[ MAX_STRING_LENGTH ] = "";
  char called[ MAX_STRING_LENGTH ] = "";
  char* pAssignments, *pUnassigned, *pCalled;
  int minvnum, maxvnum;
  int i;
  int mpcount = 0, uacount = 0, ccount;
  regex_t mpRegex;
  MOB_INDEX_DATA* pMob = NULL;
  MPROG_LIST* pMobList;
  MPROG_CODE* pCode;
  MPROG_CODE* pGlobalList;
  bool matched = FALSE;
  bool full = FALSE;

  if ( IS_NPC(ch) || !ch->in_room || !ch->in_room->area )
  {
      send_to_char( "No no no, you can't do that.\n\r", ch );
      return;
  }

  minvnum = ch->in_room->area->min_vnum;
  maxvnum = ch->in_room->area->max_vnum;


  if ( !str_prefix( argument, "full" ) )
      full = TRUE;

  pAssignments = &assignments[ 0 ];
  pUnassigned = &unassigned[ 0 ];
  pCalled = &called[ 0 ];

/*** for each mobile in area, list off its assigned progs *********************/
  pAssignments += sprintf( pAssignments, 
      "Current Mobile-Mobprog Assignments in this area:\n\r\n\r" );

  for( i = minvnum; i < maxvnum; i++, mpcount = 0 )
  {
    pMob = get_mob_index( i );
    if ( pMob != NULL && pMob->mprog_flags )
    {
      pAssignments += sprintf( pAssignments, "%d (%s):\n\r%5s", 
                                                     i, pMob->short_descr, "" );

      for ( pMobList=pMob->mprogs; pMobList != NULL; pMobList=pMobList->next )
      {
        pAssignments += sprintf( pAssignments, "%d ", pMobList->vnum );
        mpcount++;
	      
        if( mpcount > 10 )
	{
	  pAssignments += sprintf( pAssignments, "\n\r%5s", "" );
	  mpcount = 0;
	}
      }

      pAssignments += sprintf( pAssignments, "\n\r\n\r" );
    }
  }
  
/*** for each mobprog in area, check against world for calls ******************/
  pCalled += sprintf( pCalled, 
    "Current Mobprogs Called by Mobprogs in this area:\n\r\n\r" );

  for( i = minvnum; i < maxvnum; i++, ccount = 0, matched = FALSE )
  {
/*  sprintf( mpRegexStr, "[Cc][Aa][Ll][Ll][ ]+%d", i ); */
    sprintf( mpRegexStr, "mob  *call  *%d[^01-9]", i );
    regcomp( &mpRegex, mpRegexStr, REG_ICASE|REG_NEWLINE );
    if ( ( pCode = get_mprog_index( i ) ) != NULL )
      pCode->called = FALSE;
    else if ( !full )   /* Only scan for non-existent mprogs if told to */
    {
      regfree( &mpRegex );
      continue;
    }

    for (pGlobalList = mprog_list; pGlobalList; pGlobalList = pGlobalList->next)
    {
      if ( regexec( &mpRegex, pGlobalList->code, 0, NULL, 0 ) != REG_NOMATCH )
      {
        if( !matched )
	{
	  matched = TRUE;
	  pCalled += sprintf( pCalled, "%d %sis called by ", i,
              ( pCode ? "" : "(which doesn't exist) " ) );
          if ( pCode )
            pCode->called = TRUE;
	}
        pCalled += sprintf( pCalled, "%d ", pGlobalList->vnum );
        ccount++;
      }
    }
    if ( matched )
      pCalled += sprintf( pCalled, "\n\r" );

    regfree( &mpRegex );
  }

/*** for each mobprog in area, list off unassigned ****************************/
  pUnassigned += sprintf( pUnassigned, 
    "Current Mobprogs Unassigned to Mobiles in this area:\n\r\n\r" );

  for( i = minvnum; i < maxvnum; i++, uacount = 0 )
  {
    pCode = get_mprog_index( i );

    if( pCode != NULL && pCode->assigned == 0 && !pCode->called )
    {
      pUnassigned += sprintf( pUnassigned, "%d ", i );
      uacount++;

      if( uacount > 10 )
      {
        pUnassigned += sprintf( pUnassigned, "\n\r%5s", "" );
        uacount = 0;
      }
    }
  }

  pUnassigned += sprintf( pUnassigned, "\n\r\n\r" );

  send_to_char( assignments, ch );
  send_to_char( "--------------------------------------------------------------------------------\n\r", ch );
  send_to_char( called, ch );
  send_to_char( "--------------------------------------------------------------------------------\n\r", ch );
  send_to_char( unassigned, ch );
  send_to_char( "\n\rRemember that this list does not include mob invokes!\n\r",
    ch);
  return;
}

bool check_recog( CHAR_DATA *ch, int snum, int bit )
{
    MPRECOG_DATA *mpr;

    for ( mpr = ch->mprecog; mpr; mpr = mpr->next )
        if ( mpr->vnum == snum ) break;

    if ( mpr != NULL && IS_SET( mpr->flags, bit) )
        return TRUE;
    else
        return FALSE;
}
      
bool is_mprecog( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int bit;

    if ( ch == NULL || IS_NULLSTR( argument ) )
        return FALSE;

    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );

    if ( !is_number( arg ) )
    {
        /*  For MPRECOG flag aliases in the future */
    }
    else
    {
        /*  Specifying both set number and flag */
        if ((bit = flag_lookup( arg2, mprecog_flags )) == 0)
            return FALSE;
        return check_recog( ch, atoi(arg), bit );
    }
    return FALSE;
}

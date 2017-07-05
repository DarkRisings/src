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
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@efn.org)				   *
*	    Gabrielle Taylor						   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "brawler.h"
#include "clans.h"
#include "guilds.h" 
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif


int rename(const char *oldfname, const char *newfname);
extern MPRECOG_DATA *new_mprecog(void);
extern struct pc_guild_type pc_guild_table[MAX_PC_GUILD];

char *print_flags(int flag)
{
    int count, pos = 0;
    static char buf[52];


    for (count = 0; count < 32;  count++)
    {
        if (IS_SET(flag,1<<count))
        {
            if (count < 26)
                buf[pos] = 'A' + count;
            else
                buf[pos] = 'a' + (count - 26);
            pos++;
        }
    }

    if (pos == 0)
    {
        buf[pos] = '0';
        pos++;
    }

    buf[pos] = '\0';

    return buf;
}


/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];


/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			    FILE *fp, int iNest ) );
void	fwrite_pet	args( ( CHAR_DATA *pet, FILE *fp) );
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fread_pet	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );



/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;

    if ( IS_NPC(ch) )
	return;

    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    fclose( fpReserve );
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Save_char_obj: fopen", 0 );
	perror( strsave );
    }
    else
    {
	fwrite_char( ch, fp );
	if ( ch->carrying != NULL )
	    fwrite_obj( ch, ch->carrying, fp, 0 );
	/* save the pets */
/*
	if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
	    fwrite_pet(ch->pet,fp);
*/
	fprintf( fp, "#END\n" );
    }
    fclose( fp );
    rename(TEMP_FILE,strsave);
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Write the char.
 */
#define PFILE_VERSION 8
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
  AFFECT_DATA *paf;
  int sn, gn;

  fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"	);

  fprintf( fp, "Name %s~\n",	ch->name		);
  fprintf( fp, "Id   %ld\n",    ch->id			);
  fprintf( fp, "LogO %ld\n",	current_time		);
  fprintf( fp, "Vers %d\n",     PFILE_VERSION		);
  
  if (ch->short_descr[0] != '\0')
    fprintf( fp, "ShD  %s~\n",	ch->short_descr	);
    
  if( ch->long_descr[0] != '\0')
    fprintf( fp, "LnD  %s~\n",	ch->long_descr	);
    
  if (ch->description[0] != '\0')
    fprintf( fp, "Desc %s~\n",	ch->description	);
  
  if (ch->prompt != NULL || !strcmp( ch->prompt, default_prompt ) )
    fprintf( fp, "Prom %s~\n",      ch->prompt  	);
    
  fprintf( fp, "Race %s~\n", pc_race_table[ch->race].name );
  
  if (ch->guild)
    fprintf( fp, "Guild %s~\n",pc_guild_table[ch->guild].name);
    
  if (ch->image[0] != '\0')
    fprintf( fp, "Image %s~\n",	ch->image	);
    
  if (ch->shiftimage1[0] != '\0')
    fprintf( fp, "ShImg1 %s~\n",	ch->shiftimage1	);
    
  if (ch->shiftimage2[0] != '\0')
    fprintf( fp, "ShImg2 %s~\n",	ch->shiftimage2	);
  
  fprintf( fp, "Sex  %d\n",	ch->sex			);
  fprintf( fp, "Cla  %d\n",	ch->class		);
  fprintf( fp, "Levl %d\n",	ch->level		);
  if (ch->trust != 0)
    fprintf( fp, "Tru  %d\n",	ch->trust	);
  fprintf( fp, "Sec  %d\n",    ch->pcdata->security	);	/* OLC */
  fprintf( fp, "Plyd %d\n",
	   ch->played + (int) (current_time - ch->logon)	);
  fprintf( fp, "Immtime %d\n",(int)ch->pcdata->wizitime);
  fprintf( fp, "Vistime %d\n", (int)ch->pcdata->vistime);

  if ( ch->pcdata->approvals > 0
  ||   ch->pcdata->unapprovals > 0
  ||   ch->pcdata->immtalks > 0 
  ||   ch->pcdata->questcount > 0 )
  {
      fprintf( fp, "KPI1 %d %d %d %d\n",
        ch->pcdata->approvals,
        ch->pcdata->unapprovals,
        ch->pcdata->immtalks, 
        ch->pcdata->questcount);
  }
  if ( ch->pcdata->guildnotes > 0
  ||   ch->pcdata->gdts > 0
  ||   ch->pcdata->ics > 0
  ||   ch->pcdata->oocs > 0 )
  {
      fprintf( fp, "KPI2 %d %d %d %d\n",
        ch->pcdata->guildnotes,
        ch->pcdata->gdts,
        ch->pcdata->ics,
        ch->pcdata->oocs );
  }

  fprintf( fp, "RestoreChannels %d\n", (int)ch->pcdata->restore_channels );
  fprintf( fp, "RestoreEmotes %d\n", (int)ch->pcdata->restore_emotes );
  fprintf( fp, "RestoreShouts %d\n", (int)ch->pcdata->restore_shouts );
  fprintf( fp, "RestoreTells %d\n", (int)ch->pcdata->restore_tells );
  fprintf( fp, "Not  %ld %ld %ld %ld %ld\n",		
	   ch->pcdata->last_note,ch->pcdata->last_idea,ch->pcdata->last_history,
	   ch->pcdata->last_news,ch->pcdata->last_changes	);
  fprintf( fp, "Trouble %ld\n", ch->pcdata->last_trouble );
   fprintf( fp, "Gossip %ld\n", ch->pcdata->last_gossip );
/*fprintf( fp, "LastLore %ld\n", ch->pcdata->last_lore );*/
  if ( ch->lines != PAGELEN )
      fprintf( fp, "Scro %d\n", 	ch->lines		);
  fprintf( fp, "Room %d\n",
	   (  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
	      && ch->was_in_room != NULL )
	   ? ch->was_in_room->vnum
	   : ch->in_room == NULL ? 3001 : ch->in_room->vnum );
  
  fprintf( fp, "HMV  %d %d %d %d %d %d\n",
	   ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
  if (ch->gold > 0)
    fprintf( fp, "Gold %ld\n",	ch->gold		);
  else
    fprintf( fp, "Gold %d\n", 0			); 
  if (ch->silver > 0)
    fprintf( fp, "Silv %ld\n",ch->silver		);
  else
    fprintf( fp, "Silv %d\n",0			);
  /* Gothar Bank Services -1997*/
  if (ch->pcdata->gold_bank > 0)
    fprintf( fp, "Gold_bank %ld\n",ch->pcdata->gold_bank);
  else
    fprintf( fp, "Gold_bank %d\n", 0);
  if (ch->pcdata->silver_bank > 0)
    fprintf( fp, "Silv_bank %ld\n",ch->pcdata->silver_bank);
  else
    fprintf( fp, "Silv_bank %d\n",0);
  
  fprintf( fp, "Exp  %d\n",	ch->exp			);
  if (ch->act_bits != 0)
    fprintf( fp, "Act  %s\n",   print_flags(ch->act_bits));
  if (ch->affected_by != 0)
    fprintf( fp, "AfBy %s\n",   print_flags(ch->affected_by));
  
  /***** SAVE AFF 2's ****/
  if (ch->affected_by2 != 0)
    fprintf( fp, "AfBy2 %s\n",   print_flags(ch->affected_by2));
  
  /***** SAVE AFF 3's ****/
  if (ch->affected_by3 != 0)
    fprintf( fp, "AfBy3 %s\n",   print_flags(ch->affected_by3));
  
    /***** SAVE SERAPHS ****/
    fprintf( fp, "LifeEnergy %d\n",	ch->life_energy );
    fprintf( fp, "MaxLifeEnergy %d\n",   ch->max_life_energy );
	fprintf( fp, "Enochian %d\n",   ch->enochian);
  
  /***** SAVE WERE's ****/
  if (ch->shiftbits != 0)
    fprintf( fp, "Shift %s\n",   print_flags(ch->shiftbits));
  if (ch->shiftdesc1[0] != '\0')
    fprintf( fp, "ShiftDesc1 %s~\n",	ch->shiftdesc1	);
  if (ch->shiftdesc2[0] != '\0')
    fprintf( fp, "ShiftDesc2 %s~\n",	ch->shiftdesc2	);
  
  /***** COALITION *****/
  if( !IS_NULLSTR( ch->cln_name ) )
    fprintf( fp, "cclan %s~\n",	ch->cln_name	);
 
  if( !IS_NULLSTR( ch->cln_leader ) )
    fprintf( fp, "ccleader %s~\n",	ch->cln_leader	);
  
  if( !IS_NULLSTR( ch->cln_symbol ) )
    fprintf( fp, "ccsymbol %s~\n",	ch->cln_symbol	);
	
  if( !IS_NULLSTR( ch->cln_apply ) )
    fprintf( fp, "ccapply %s~\n",	ch->cln_apply	);
		
  /***** SAVE VAMP's ****/
  if (ch->patriarch[0] != '\0')
    fprintf( fp, "Patr %s\n",	ch->patriarch	);
  if ( IS_VAMPIRE(ch) ) 
  {
    fprintf( fp, "Bloodpool %d\n",	ch->blood );
    fprintf( fp, "MaxBloodpool %d\n",   ch->max_blood );
    fprintf( fp, "BloodpoolPercent %d\n", ch->blood_percentage );
  }
  
  for ( sn = 0; sn < MAX_COLOR; sn++ )
      if ( ch->colors[sn] != colordefs[sn].color )
          fprintf(fp,"Color %d %c\n", sn, ch->colors[sn]); 
		  
	if(IS_IMMORTAL(ch)) {
	fprintf( fp, "Availability %d\n",   ch->availability);
	}
 
  fprintf( fp, "Comm %s\n",       print_flags(ch->comm));
  if (ch->wiznet_flags)
    fprintf( fp, "Wizn %s\n",   print_flags(ch->wiznet_flags));
  if (ch->invis_level)
    fprintf( fp, "Invi %d\n", 	ch->invis_level	);
  if (ch->incog_level)
    fprintf(fp,"Inco %d\n",ch->incog_level);
  if (ch->rinvis_level)
    fprintf(fp, "RInvis %d\n",ch->rinvis_level);
  fprintf( fp, "Pos  %d\n",	
	   ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
  if (ch->practice != 0)
    fprintf( fp, "Prac %d\n",	ch->practice	);
  if (ch->train != 0)
    fprintf( fp, "Trai %d\n",	ch->train	);
  if (ch->saving_throw != 0)
    fprintf( fp, "Save  %d\n",	ch->saving_throw);
  fprintf( fp, "Alig  %d\n",	ch->alignment		);
  if (ch->hitroll != 0)
    fprintf( fp, "Hit   %d\n",	ch->hitroll	);
  if (ch->damroll != 0)
    fprintf( fp, "Dam   %d\n",	ch->damroll	);
  fprintf( fp, "ACs %d %d %d %d\n",	
	   ch->armor[0],ch->armor[1],ch->armor[2],ch->armor[3]);
  if (ch->wimpy !=0 )
    fprintf( fp, "Wimp  %d\n",	ch->wimpy	);
  fprintf( fp, "Attr %d %d %d %d %d\n",
	   ch->perm_stat[STAT_STR],
	   ch->perm_stat[STAT_INT],
	   ch->perm_stat[STAT_WIS],
	   ch->perm_stat[STAT_DEX],
	   ch->perm_stat[STAT_CON] );
  
  fprintf (fp, "AMod %d %d %d %d %d\n",
	   ch->mod_stat[STAT_STR],
	   ch->mod_stat[STAT_INT],
	   ch->mod_stat[STAT_WIS],
	   ch->mod_stat[STAT_DEX],
	   ch->mod_stat[STAT_CON] );
  
  if ( IS_NPC(ch) )
    {
      fprintf( fp, "Vnum %d\n",	ch->pIndexData->vnum	);
    }
  else
    {
      if( ch->pcdata->fake_ip[ 0 ] != '\0' )
	{
	  fprintf( fp, "FakeIP %s~\n", ch->pcdata->fake_ip );
	  fprintf( fp, "LastIP %s~\n", ch->pcdata->fake_ip );
	}
      else if( ch->pcdata->last_ip[ 0 ] != '\0' )
	{
	  fprintf( fp, "LastIP %s~\n", ch->pcdata->last_ip );
	}

      fprintf( fp, "Pass %s~\n",	ch->pcdata->pwd		);

      if ( !IS_NULLSTR(ch->pcdata->bamfin) )
          fprintf( fp, "Bin  %s~\n",	ch->pcdata->bamfin      );
      if ( !IS_NULLSTR(ch->pcdata->bamfout) )
          fprintf( fp, "Bout %s~\n",	ch->pcdata->bamfout     );

      if ( !IS_NULLSTR(ch->pcdata->transin) )
          fprintf( fp, "Tin  %s~\n",	ch->pcdata->transin     );
      if ( !IS_NULLSTR(ch->pcdata->transout) )
          fprintf( fp, "Tout %s~\n",	ch->pcdata->transout    );

      if ( !IS_NULLSTR(ch->pcdata->pretitle) )
          fprintf( fp, "Pret  %s~\n",	ch->pcdata->pretitle    );
      if ( !IS_NULLSTR(ch->pcdata->notetags) )
          fprintf( fp, "NTags %s~\n",	ch->pcdata->notetags    );
      if ( !IS_NULLSTR( ch->pcdata->famshort ) )
          fprintf( fp, "FamS %s~\n",    ch->pcdata->famshort    );
      if ( !IS_NULLSTR( ch->pcdata->famlong ) )
          fprintf( fp, "FamL %s~\n",    ch->pcdata->famlong     );
      if ( !IS_NULLSTR( ch->pcdata->famname ) )
          fprintf( fp, "FamN %s~\n",    ch->pcdata->famname     );
      if ( !IS_NULLSTR( ch->pcdata->famdesc ) )
          fprintf( fp, "FamD %s~\n",    ch->pcdata->famdesc     );
      fprintf( fp, "FamG %d\n",        ch->pcdata->famsex       );
      if ( ch->pcdata->proceeds > 0 )
          fprintf( fp, "Proceeds %d\n", ch->pcdata->proceeds    );
   
      fprintf( fp, "Titl %s~\n",	ch->pcdata->title	);
      fprintf( fp, "Guildtitl %s~\n",	ch->title_guild	);
      fprintf( fp, "GuildTime %d\n", ch->pcdata->guild_time );
      fprintf( fp, "GuildLogs %d\n", ch->pcdata->guild_logs );
      fprintf( fp, "GuildDate %d\n", (int)ch->pcdata->guild_date );
      fprintf( fp, "God %d\n",	ch->pcdata->god	);
      if ( ch->pcdata->cleansings > 0 )
          fprintf( fp, "Cleansings %d\n", ch->pcdata->cleansings );
      if ( ch->pcdata->last_cleanse > 0 )
          fprintf( fp, "LastCleanse %d\n", (int)ch->pcdata->last_cleanse );
      fprintf( fp, "Pnts %d\n",   	ch->pcdata->points      );
      fprintf( fp, "TSex %d\n",	ch->pcdata->true_sex	);
      fprintf( fp, "LLev %d\n",	ch->pcdata->last_level	);
      fprintf( fp, "HMVP %d %d %d\n", ch->pcdata->perm_hit, 
	       ch->pcdata->perm_mana,
	       ch->pcdata->perm_move);
      fprintf( fp, "Cnd  %d %d %d %d\n",
	       ch->pcdata->condition[0],
	       ch->pcdata->condition[1],
	       ch->pcdata->condition[2],
	       ch->pcdata->condition[3] );
      /* TRADE SKILLS GO HERE */
      fprintf( fp, "Trade %d\n", ch->pcdata->tailoring);
      fprintf( fp, "Bake %d\n", ch->pcdata->baking);
      fprintf( fp, "Fletch %d\n", ch->pcdata->fletching);

      if( ch->pcdata->pcClan != NULL )
	fprintf( fp, "PCClan %s~\n", ch->pcdata->pcClan->name );

      for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	  if ( skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0 )
	    {
	      fprintf( fp, "Sk %d '%s'\n",
		       ch->pcdata->learned[sn], skill_table[sn].name );
	    }
	}
      
      for ( gn = 0; gn < MAX_GROUP; gn++ )
	{
	  if ( group_table[gn].name != NULL && ch->pcdata->group_known[gn])
	    {
	      fprintf( fp, "Gr '%s'\n",group_table[gn].name);
	    }
	}
      if ( ch->mprecog != NULL ) 
      {
          MPRECOG_DATA *mpr;
          INT_LIST *mprt;
          for ( mpr = ch->mprecog ; mpr != NULL; mpr = mpr->next )
          {
              if ( mpr->flags != 0 )
              fprintf( fp, "MPR %d %s %d\n",
                mpr->vnum,
                print_flags(mpr->flags), 
                ((mpr->temp) ? 1 : 0 ));
              
              for ( mprt = mpr->timer; mprt != NULL; mprt = mprt->next )
              {
                fprintf( fp, "MPRT %d %d %d\n", mpr->vnum, mprt->value[0], 
                                                           mprt->value[1] );
              }
          }
      }
    }

  char scratch[ MAX_INPUT_LENGTH ] = "";
  for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
      if (paf->type>= MAX_SKILL)
	continue;

/* This is pretty bad, but so is the below code */
      if ( paf->type == -5 )
        continue;

      switch( paf->type )
	{
	case -1:
	  sprintf( scratch, "%s", "wereon" );
	  break;

	case -2:
	  sprintf( scratch, "%s", "wereoff" );
	  break;

	case -3:
	  sprintf( scratch, "%s", "vampon" );
	  break;

	case -4:
	  sprintf( scratch, "%s", "veilwait" );
	  break;
/* For the sake of simplicity, offering just isn't saved
	case -5:
	  sprintf( scratch, "%s", "offer" );
	  break;
 -- May 12, 2006 gkl */

	case -6:
	  sprintf( scratch, "%s", "frog" );
	  break;

	case -20:
	  sprintf( scratch, "%s", "torpor" );
	  break;

	default:
	  sprintf( scratch, "%s", skill_table[ paf->type ].name );
	  break;
	}

      fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %3d %10d\n",
	       scratch,
	       paf->where,
	       paf->level,
	       paf->duration,
	       paf->modifier,
	       paf->location,
	       paf->whichaff,
	       paf->bitvector
	       );



      /*      
      if (paf->type == -20)
	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %3d %10d\n",
		 "torpor",
		 paf->where,
		 paf->level,
		 paf->duration,
		 paf->modifier,
		 paf->location,
		 paf->whichaff,
		 paf->bitvector
		 );
      
      else if (paf->type == -1)
	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %3d %10d\n",
		 "wereon",
		 paf->where,
		 paf->level,
		 paf->duration,
		 paf->modifier,
		 paf->location,
		 paf->whichaff,
		 paf->bitvector
		 );
      else if (paf->type == -2)
	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %3d %10d\n",
		 "wereoff",
		 paf->where,
		 paf->level,
		 paf->duration,
		 paf->modifier,
		 paf->location,
		 paf->whichaff,
		 paf->bitvector
		 );
      else if (paf->type == -3)
	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %3d %10d\n",
		 "vampon",
		 paf->where,
		 paf->level,
		 paf->duration,
		 paf->modifier,
		 paf->location,
		 paf->whichaff,
		 paf->bitvector
		 );
      else if (paf->type == -4)
	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %3d %10d\n",
		 "veilwait",
		 paf->where,
		 paf->level,
		 paf->duration,
		 paf->modifier,
		 paf->location,
		 paf->whichaff,
		 paf->bitvector
		 );
      else if (paf->type == -5)
	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %3d %10d\n",
		 "offer",
		 paf->where,
		 paf->level,
		 paf->duration,
		 paf->modifier,
		 paf->location,
		 paf->whichaff,
		 paf->bitvector
		 );
      else if (paf->type == -6)
	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %3d %10d\n",
		 "frog",
		 paf->where,
		 paf->level,
		 paf->duration,
		 paf->modifier,
		 paf->location,
		 paf->whichaff,
		 paf->bitvector
		 );
      else
	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %3d %10d\n",
		 skill_table[paf->type].name,
		 paf->where,
		 paf->level,
		 paf->duration,
		 paf->modifier,
		 paf->location,
		 paf->whichaff,
		 paf->bitvector
		 );
      */
    }
  
  fprintf( fp, "End\n\n" );
  return;
}

/* write a pet */
void fwrite_pet( CHAR_DATA *pet, FILE *fp)
{
    AFFECT_DATA *paf;
    
    fprintf(fp,"#PET\n");
    
    fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);
    
    fprintf(fp,"Name %s~\n", pet->name);
    fprintf(fp,"LogO %ld\n", current_time);
    if (pet->short_descr != pet->pIndexData->short_descr)
    	fprintf(fp,"ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
    	fprintf(fp,"LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
    	fprintf(fp,"Desc %s~\n", pet->description);
    if (pet->race != pet->pIndexData->race)
    	fprintf(fp,"Race %s~\n", race_table[pet->race].name);
    if (pet->guild)
        fprintf( fp, "Guild %s~\n",guild_table[pet->guild].name);
    fprintf(fp,"Sex  %d\n", pet->sex);
    if (pet->level != pet->pIndexData->level)
    	fprintf(fp,"Levl %d\n", pet->level);
    fprintf(fp, "HMV  %d %d %d %d %d %d\n",
    	pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
    if (pet->gold > 0)
    	fprintf(fp,"Gold %ld\n",pet->gold);
    if (pet->silver > 0)
	fprintf(fp,"Silv %ld\n",pet->silver);
    if (pet->exp > 0)
    	fprintf(fp, "Exp  %d\n", pet->exp);
    if (pet->act_bits != pet->pIndexData->act)
    	fprintf(fp, "Act  %s\n", print_flags(pet->act_bits));
    if (pet->affected_by != pet->pIndexData->affected_by)
    	fprintf(fp, "AfBy %s\n", print_flags(pet->affected_by));

/* AFF 2 */	
    if (pet->affected_by2 != pet->pIndexData->affected_by2)
    	fprintf(fp, "AfBy2 %s\n", print_flags(pet->affected_by2));
/* AFF 3 */
    if (pet->affected_by3 != pet->pIndexData->affected_by3)
    	fprintf(fp, "AfBy3 %s\n", print_flags(pet->affected_by3));

    if (pet->comm != 0)
    	fprintf(fp, "Comm %s\n", print_flags(pet->comm));
    fprintf(fp,"Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
    	fprintf(fp, "Save %d\n", pet->saving_throw);
    if (pet->alignment != pet->pIndexData->alignment)
    	fprintf(fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->pIndexData->hitroll)
    	fprintf(fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
    	fprintf(fp, "Dam  %d\n", pet->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n",
    	pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);
    fprintf(fp, "Attr %d %d %d %d %d\n",
    	pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
    	pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
    	pet->perm_stat[STAT_CON]);
    fprintf(fp, "AMod %d %d %d %d %d\n",
    	pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
    	pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
    	pet->mod_stat[STAT_CON]);
    
    for ( paf = pet->affected; paf != NULL; paf = paf->next )
    {
    	if (paf->type < 0 || paf->type >= MAX_SKILL)
    	    continue;
    	    
    	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %3d %10d\n",
    	    skill_table[paf->type].name,
    	    paf->where, paf->level, paf->duration, paf->modifier,
			paf->location,
			paf->whichaff,
    	    paf->bitvector);
    }
    
    fprintf(fp,"End\n");
    return;
}
    
/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL )
	fwrite_obj( ch, obj->next_content, fp, iNest );

    /*
     * Castrate storage characters.
     */
    if ( obj->item_type == ITEM_KEY )
	return;

    fprintf( fp, "#O\n" );
    fprintf( fp, "Vnum %d\n",   obj->pIndexData->vnum        );
    if (!obj->pIndexData->new_format)
	fprintf( fp, "Oldstyle\n");
    if (obj->enchanted)
	fprintf( fp,"Enchanted\n");
    fprintf( fp, "Nest %d\n",	iNest	  	     );

    /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name)
    	fprintf( fp, "Name %s~\n",	obj->name		     );
    if ( obj->short_descr != obj->pIndexData->short_descr)
        fprintf( fp, "ShD  %s~\n",	obj->short_descr	     );
    if ( obj->description != obj->pIndexData->description)
        fprintf( fp, "Desc %s~\n",	obj->description	     );
    if ( obj->extra_flags != obj->pIndexData->extra_flags)
        fprintf( fp, "ExtF %d\n",	obj->extra_flags	     );
    if ( obj->wear_flags != obj->pIndexData->wear_flags)
        fprintf( fp, "WeaF %d\n",	obj->wear_flags		     );
    if ( obj->item_type != obj->pIndexData->item_type)
        fprintf( fp, "Ityp %d\n",	obj->item_type		     );
    if ( obj->weight != obj->pIndexData->weight)
        fprintf( fp, "Wt   %d\n",	obj->weight		     );
    if ( obj->condition != obj->pIndexData->condition)
	fprintf( fp, "Cond %d\n",	obj->condition		     );
    if ( strlen( obj->donor ) > 0 )
      fprintf( fp, "Donor %s~\n",        obj->donor );


    /* variable data */

    fprintf( fp, "Wear %d\n",   obj->wear_loc                );
    if (obj->level != obj->pIndexData->level)
        fprintf( fp, "Lev  %d\n",	obj->level		     );
    if (obj->timer != 0)
        fprintf( fp, "Time %d\n",	obj->timer	     );
    fprintf( fp, "Cost %d\n",	obj->cost		     );
    if (obj->value[0] != obj->pIndexData->value[0]
    ||  obj->value[1] != obj->pIndexData->value[1]
    ||  obj->value[2] != obj->pIndexData->value[2]
    ||  obj->value[3] != obj->pIndexData->value[3]
    ||  obj->value[4] != obj->pIndexData->value[4]) 
    	fprintf( fp, "Val  %d %d %d %d %d\n",
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	    obj->value[4]	     );

    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
    case ITEM_PILL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1 '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2 '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}
	if ( obj->value[4] > 0 )
	{
	    fprintf( fp, "Spell 4 '%s'\n", 
		skill_table[obj->value[4]].name );
	}


	break;

    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;
    case ITEM_DRINK_CON:
    case ITEM_FOUNTAIN:
	if ( obj->value[4] > 0 )
	{
	    fprintf( fp, "Spell 4 '%s'\n", 
		skill_table[obj->value[4]].name );
	}
	if ( obj->value[4] < 0 )
	{
	    fprintf( fp, "Spelln 4 '%s'\n", 
		skill_table[-1*obj->value[4]].name );
	}

        break;

    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type >= MAX_SKILL)
	    continue;
        fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %3d %10d\n",
            skill_table[paf->type].name,
            paf->where,
            paf->level,
            paf->duration,
            paf->modifier,
            paf->location,
            paf->whichaff,
            paf->bitvector
            );
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
	fprintf( fp, "ExDe %s~ %s~\n",
	    ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
	fwrite_obj( ch, obj->contains, fp, iNest + 1 );

    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
    char strsave[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    int stat;

    ch = new_char();
    ch->pcdata = new_pcdata();

    d->character			= ch;
    ch->desc				= d;
    ch->name				= str_dup( name );
    ch->id				= get_pc_id();
    ch->race				= race_lookup("human");
    ch->act_bits			= PLR_NOSUMMON;
    ch->comm				= COMM_COMBINE 
					| COMM_PROMPT;
    ch->prompt 				= str_dup( default_prompt );
    ch->mprecog                         = NULL;
/*  PCDATA initialization should be in new_pcdata() in recycle.c
    but since that function is only ever called here, I guess this
    is okay...  just inconsistent. */
    if ( global_config_aprilfools ) 
        ch->pcdata->aprilfools  = TRUE;
    else
        ch->pcdata->aprilfools  = FALSE;
    ch->pcdata->confirm_delete		= FALSE;
    ch->pcdata->latelog			= TRUE;
    ch->pcdata->fake_ip                 = str_dup( "" );
    ch->pcdata->last_ip                 = str_dup( "" );
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->pretitle		= str_dup( "" );
    ch->pcdata->famshort		= str_dup( "" );
    ch->pcdata->famlong 		= str_dup( "" );
    ch->pcdata->famname 		= str_dup( "" );
    ch->pcdata->famdesc 		= str_dup( "" );
    ch->pcdata->notetags                = str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->pcdata->transin			= str_dup( "" );
    ch->pcdata->transout		= str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
    ch->pcdata->god		        = 1;
    ch->pcdata->push_count              = 0;
    ch->pcdata->proceeds                = 0;
    ch->title_guild		        = str_dup( "" );
    for (stat =0; stat < MAX_STATS; stat++)
	ch->perm_stat[stat]		= 13;
    ch->pcdata->condition[COND_THIRST]	= 48; 
    ch->pcdata->condition[COND_FULL]	= 48;
    ch->pcdata->condition[COND_HUNGER]	= 48;
    ch->pcdata->security		= 0;	/* OLC */
    ch->pcdata->volume			= 50;
    ch->pcdata->tailoring		= 1;    
    ch->pcdata->baking			= 1;
    ch->pcdata->fletching		= 1;
    ch->pcdata->wizitime		= 1;
    ch->pcdata->vistime			= 1;

    ch->pcdata->approvals               = 0;
    ch->pcdata->unapprovals             = 0;
    ch->pcdata->guildnotes              = 0;
    ch->pcdata->gdts                    = 0;
    ch->pcdata->immtalks                = 0;
    ch->pcdata->questcount              = 0;
    ch->pcdata->ics                     = 0;
    ch->pcdata->oocs                    = 0;

    ch->pcdata->guild_time              = 0;
    ch->pcdata->guild_logs              = 0;
    ch->pcdata->guild_date              = 0;
    ch->pcdata->quest_team_score        = 0;
    ch->pcdata->quest_team_captain      = str_dup( "" );
    ch->pcdata->restore_channels        = 0;
    ch->pcdata->restore_emotes          = 0;
    ch->pcdata->restore_shouts          = 0;
    ch->pcdata->restore_tells           = 0;
    ch->pcdata->cleansings              = 0;
    ch->pcdata->cleanse_timer           = 0;
    ch->pcdata->offering                = NULL;
    ch->pcdata->verifytrain             = FALSE;
    ch->pcdata->skillbonus              = 0;
    ch->pcdata->famsex                  = SEX_NEUTRAL;
    ch->pcdata->pcClan                  = NULL;

    found = FALSE;
    fclose( fpReserve );
    
    #if defined(unix)
    /* decompress if .gz file exists */
    sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	sprintf(buf,"gzip -dfq %s",strsave);
	system(buf);
    }
    #endif

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
	    else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "O"      ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );

    if( !IS_NPC( ch ) && is_guild( ch ) && ch->pcdata->guild_date == 0 )
      {
	ch->pcdata->guild_date = current_time;
      }

    /* initialize race */
    if (found)
    {
	int i;
        BRAWLER_DATA *pBrawler;

        if ((ch->race > MAX_PC_RACE) || (ch->level == 0)){
           /* BAD PLAYER FILE */
           return 2;
           }

	if (ch->race == 0)
	    ch->race = race_lookup("human");

	ch->size = pc_race_table[ch->race].size;
	ch->dam_type = 17; /*punch */

	for (i = 0; i < 5; i++)
	{
	    if (pc_race_table[ch->race].skills[i] == NULL)
		break;
	    group_add(ch,pc_race_table[ch->race].skills[i],FALSE);
	}
	ch->affected_by = ch->affected_by|race_table[ch->race].aff;
	ch->affected_by2 = ch->affected_by2|race_table[ch->race].aff2;
	ch->affected_by3 = ch->affected_by3|race_table[ch->race].aff3;
	ch->imm_flags	= ch->imm_flags | race_table[ch->race].imm;
	ch->res_flags	= ch->res_flags | race_table[ch->race].res;
	ch->vuln_flags	= ch->vuln_flags | race_table[ch->race].vuln;
	ch->form	= race_table[ch->race].form;
	ch->parts	= race_table[ch->race].parts;

        /* initialize brawler */
        ch->pcdata->brawler = NULL;
        ch->pcdata->brawling = NULL;
        for (pBrawler=brawler_list; pBrawler != NULL; pBrawler=pBrawler->next)
            if ( !str_cmp(pBrawler->name, ch->name) )
                ch->pcdata->brawler = pBrawler;
    }

    if ( found && !IS_NPC(ch) )
    {   /* version 6 = pcdata->guild_time fixed */
        if ( ch->pcdata->version < 6) ch->pcdata->guild_time = 0;
        /* version 7 = colorless prompt options %h/%H/%m/%M/%v/%V */
        /*             monks also got divine focus */
        if ( ch->pcdata->version < 7 )
        {
            /*
             * Convert old prompt coloration to manual color codes
             */
            char newprompt[2*MAX_STRING_LENGTH];
            char *ps = newprompt;
            int i = 0;
            while ( ch->prompt[i] != '\0' )
            {
                char next = ch->prompt[i+1];
                if ( ch->prompt[i] == '%' 
                && (   next == 'h' || next == 'H'
                || next == 'm' || next == 'M'
                || next == 'v' || next == 'V' ) )
                {
                    *ps++ = '{';
                    switch ( next )
                    {
                        case 'h':
                            *ps++ = ch->colors[C_HP];
                            break;
                        case 'H':
                            *ps++ = ch->colors[C_MAXHP];
                            break;
                        case 'm':
                            *ps++ = ch->colors[C_MANA];
                            break;
                        case 'M':
                            *ps++ = ch->colors[C_MAXMANA];
                            break;
                        case 'v':
                            *ps++ = ch->colors[C_MOVES];
                            break;
                        case 'V':
                            *ps++ = ch->colors[C_MAXMOVES];
                            break;
                    }
                    *ps++ = ch->prompt[i++]; /* the % character */
                    *ps++ = ch->prompt[i++]; /* the hHmMvV character */
                    *ps++ = '{';
                    *ps++ = 'x';
                }
                else
                    *ps++ = ch->prompt[i++];
            }
            newprompt[MAX_STRING_LENGTH-1] = '\0';
            free_string( ch->prompt );
            ch->prompt = str_dup( newprompt );
            /*
             * Grant monks divine focus
             */
            if ( ch->class == class_lookup( "monk" ) )
            {
                 int pct;
                 if ( ch->level < LEVEL_HERO )
                     pct = 1;
                 else
                     pct = 75;
                 sprintf(buf, "Granting %s divine focus at %d percent",
                     ch->name, pct );
                 bug( buf, 0 );
                 ch->pcdata->learned[gsn_divine_focus] = pct;
            }
        }
        /* version 8 = C_EMOTE, C_HP, and C_MAXHP were changed to C_NB, 
           C_SHOUT, and C_YELL */
        if ( ch->pcdata->version < 8 )
        {
          ch->colors[C_NB]      = colordefs[C_NB].color;
          ch->colors[C_SHOUT]   = colordefs[C_SHOUT].color;
          ch->colors[C_YELL]    = colordefs[C_YELL].color;
        }
    }

    if( found && IS_SET( ch->shiftbits, SHIFT_FULL ) )
      {
	ch->max_hit += were_table[ch->race].hpadd;
	ch->max_mana += were_table[ch->race].manaadd;
	ch->res_flags |= were_table[ch->race].res_full;
	ch->vuln_flags |= were_table[ch->race].vuln_full;
	ch->affected_by |= were_table[ch->race].aff_full;
      }

    if( found && IS_SET( ch->shiftbits, SHIFT_HALF ) )
      {
	ch->max_hit += were_table[ch->race].hpadd;
	ch->max_mana += were_table[ch->race].manaadd;
	ch->res_flags |= were_table[ch->race].res_half;
	ch->vuln_flags |= were_table[ch->race].vuln_half;
	ch->affected_by |= were_table[ch->race].aff_half;
      }

    if( found && IS_SET( ch->shiftbits, BAT_VAMP ) )
      {
	ch->max_hit += ((sh_int)50);
        ch->max_mana += ((sh_int)100);
	ch->res_flags |= RES_MAGIC;
        ch->vuln_flags |= VULN_WOOD|VULN_HOLY;
        ch->affected_by |= AFF_FLYING|AFF_SNEAK|AFF_INFRARED;
      }

    if( found && IS_SET( ch->shiftbits, MIST_VAMP ) )
      {
	ch->max_hit += ((sh_int)100);
	ch->max_mana += ((sh_int)50);
	ch->res_flags |= RES_PIERCE|RES_BASH|RES_SLASH;
        ch->vuln_flags |= VULN_WOOD|VULN_HOLY;
	ch->affected_by |= AFF_PASS_DOOR;
      }

    /*
    if (found && 
	(IS_SET(ch->shiftbits, SHIFT_FULL) || IS_SET(ch->shiftbits,SHIFT_HALF)))
      {
       ch->max_hit += were_table[ch->race].hpadd;
       ch->max_mana += were_table[ch->race].manaadd;
     } 
    */

    if (!found) {
	/* New player, check the were */
	if (number_range(1,100) < 8)
	   ch->shiftbits = SHIFT_POTENTIAL;
	else
	   ch->shiftbits = 0;
	SET_BIT(ch->act_bits, ACT_NOAPPROVE);
       }
    return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

/* provided to free strings */
#if defined(KEYS)
#undef KEYS
#endif

#define KEYS( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    free_string(field);			\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_char( CHAR_DATA *ch, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    int lastlogoff = current_time;
    int percent;

    sprintf(buf,"Loading %s.",ch->name);
    log_string(buf);

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
		KEY( "Availability",  ch->availability,      fread_number( fp ) );
	    KEY( "Act",		ch->act_bits,		fread_flag( fp ) );
	    KEY( "AffectedBy",	ch->affected_by,	fread_flag( fp ) );
	    KEY( "AffectedBy2",	ch->affected_by2,	fread_flag( fp ) );
	    KEY( "AffectedBy3",	ch->affected_by3,	fread_flag( fp ) );
	    KEY( "AfBy",	ch->affected_by,	fread_flag( fp ) );
	    KEY( "AfBy2",	ch->affected_by2,	fread_flag( fp ) );
	    KEY( "AfBy3",	ch->affected_by3,	fread_flag( fp ) );
	    KEY( "Alignment",	ch->alignment,		fread_number( fp ) );
	    KEY( "Alig",	ch->alignment,		fread_number( fp ) );

	    if (!str_cmp( word, "AC") || !str_cmp(word,"Armor"))
	    {
		fread_to_eol(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word,"ACs"))
	    {
		int i;

		for (i = 0; i < 4; i++)
		    ch->armor[i] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < -2)
		    bug("Fread_char: unknown skill.",0);
		else
		    paf->type = sn;

		paf->level	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->whichaff	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= ch->affected;
		ch->affected	= paf;
		fMatch = TRUE;
		break;
	    }

            if (!str_cmp(word, "Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));

		/* changed for veil and torpor */

		if( sn == NO_SKILL )
		  {
		    bug( "Fread_char: unknown skill.", 0 );
		  }
		else
		  {
		    paf->type = sn;
		  }

		/*
               if (sn < -3)
                    bug("Fread_char: unknown skill.",0);
                else
                    paf->type = sn;
		*/

                paf->where  = fread_number(fp);
                paf->level      = fread_number( fp );
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->whichaff   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->next       = ch->affected;
                ch->affected    = paf;
                fMatch = TRUE;
                break;
            }

	    if ( !str_cmp( word, "AttrMod"  ) || !str_cmp(word,"AMod"))
	    {
		int stat;
		for (stat = 0; stat < MAX_STATS; stat ++)
		   ch->mod_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrPerm" ) || !str_cmp(word,"Attr"))
	    {
		int stat;

		for (stat = 0; stat < MAX_STATS; stat++)
		    ch->perm_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'B':
	    KEY( "Bake",	ch->pcdata->baking,     fread_number( fp ) );
	    KEY( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEY( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    KEY( "Bin",		ch->pcdata->bamfin,	fread_string( fp ) );
	    KEY( "Bout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    KEY( "Bloodpool",   ch->blood,          fread_number( fp ) );
	    KEY( "BloodpoolPercent",   ch->blood_percentage, fread_number( fp ) );
	    break;

	case 'C':
            KEY( "Cleansings",  ch->pcdata->cleansings, fread_number( fp ) ); 
			KEY( "ccleader",	ch->cln_leader,	fread_string( fp ) );
			KEY( "cclan",	ch->cln_name,	fread_string( fp ) );
			KEY( "ccsymbol",	ch->cln_symbol,	fread_string( fp ) );
			KEY( "ccapply",	ch->cln_apply,	fread_string( fp ) );
	    KEY( "Class",	ch->class,		fread_number( fp ) );
	    KEY( "Cla",		ch->class,		fread_number( fp ) );
	    KEY( "Clan",	ch->guild,	guild_lookup(fread_string(fp))); 
	    KEY( "C_OOC",       ch->colors[C_OOC],	fread_letter( fp ) );
	    KEY( "C_IC",        ch->colors[C_IC],	fread_letter( fp ) );
	    KEY( "C_GT",        ch->colors[C_GT],	fread_letter( fp ) );
	    KEY( "C_TELL",      ch->colors[C_TELL],	fread_letter( fp ) );
	    KEY( "C_REPLY",     ch->colors[C_REPLY],	fread_letter( fp ) );
	    KEY( "C_GDT",       ch->colors[C_GDT],	fread_letter( fp ) );
	    KEY( "C_SAY",       ch->colors[C_SAY],	fread_letter( fp ) );
/*          KEY( "C_EMOTE",     ch->colors[C_EMOTE],	fread_letter( fp ) ); */
/*          KEY( "C_HP",        ch->colors[C_HP],	fread_letter( fp ) ); */
/*          KEY( "C_MAXHP",     ch->colors[C_MAXHP],    fread_letter( fp ) ); */
/*          KEY( "C_MANA",      ch->colors[C_MANA],     fread_letter( fp ) ); */
/*          KEY( "C_MAXMANA",   ch->colors[C_MAXMANA],  fread_letter( fp ) ); */
/*          KEY( "C_MOVES",     ch->colors[C_MOVES],    fread_letter( fp ) ); */
/*          KEY( "C_MAXMOVES",  ch->colors[C_MAXMOVES], fread_letter( fp ) ); */
            if ( !str_cmp( word, "C_EMOTE" ) 
            ||   !str_cmp( word, "C_HP" )
            ||   !str_cmp( word, "C_MAXHP" )
            ||   !str_cmp( word, "C_MANA" )
            ||   !str_cmp( word, "C_MAXMANA" )
            ||   !str_cmp( word, "C_MOVES" )
            ||   !str_cmp( word, "C_MAXMOVES" ) )
            {
                /* discard old color settings; ch will fall back to default */
                fread_to_eol( fp );
                fMatch = TRUE;
                break;
            }
	    KEY( "C_IMM",	ch->colors[C_IMM],      fread_letter( fp ) );
	    KEY( "C_YBATTLE",   ch->colors[C_YBATTLE],  fread_letter( fp ) );
	    KEY( "C_TBATTLE",   ch->colors[C_TBATTLE],  fread_letter( fp ) );
	    KEY( "C_MINION",    ch->colors[C_MINION],   fread_letter( fp ) );
	    KEY( "C_AUCTION",   ch->colors[C_AUCTION],  fread_letter( fp ) );
	    KEY( "C_ADMIN",     ch->colors[C_ADMIN],	fread_letter( fp ) );
	    KEY( "C_WIZNET",    ch->colors[C_WIZNET],   fread_letter( fp ) );
	    KEY( "C_VSAY",      ch->colors[C_VSAY],     fread_letter( fp ) );
	    KEY( "C_BRAWLER",   ch->colors[C_BRAWLER],  fread_letter( fp ) );
            KEY( "C_QSAY",      ch->colors[C_QSAY],     fread_letter( fp ) );
            KEY( "C_QTSAY",     ch->colors[C_QTSAY],    fread_letter( fp ) );
            if ( !str_cmp( word, "Color" ) )
            {
                percent = fread_number( fp );
                if (percent < MAX_COLOR)
                    ch->colors[percent] = fread_letter(fp);
                else
                {
                    bug( "Fread_char: unknown color index. ", 0 );
                    fread_letter(fp);
                }
                fMatch = TRUE;
                break;
            }

	    if ( !str_cmp( word, "Condition" ) || !str_cmp(word,"Cond"))
	    {
		ch->pcdata->condition[0] = fread_number( fp );
		ch->pcdata->condition[1] = fread_number( fp );
		ch->pcdata->condition[2] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
            if (!str_cmp(word,"Cnd"))
            {
                ch->pcdata->condition[0] = fread_number( fp );
                ch->pcdata->condition[1] = fread_number( fp );
                ch->pcdata->condition[2] = fread_number( fp );
		ch->pcdata->condition[3] = fread_number( fp );
                fMatch = TRUE;
                break;
            }
	    KEY("Comm",		ch->comm,		fread_flag( fp ) ); 
          
	    break;

	case 'D':
	    KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
	    KEY( "Dam",		ch->damroll,		fread_number( fp ) );
	    KEY( "Description",	ch->description,	fread_string( fp ) );
	    KEY( "Desc",	ch->description,	fread_string( fp ) );


	    break;

	case 'E':
		KEY( "Enochian",  ch->enochian,      fread_number( fp ) );
	    if ( !str_cmp( word, "End" ) )
	    {
    		/* adjust hp mana move up  -- here for speed's sake */
    		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);

		percent = UMIN(percent,100);
 
    		if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
    		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
    		{
        	    ch->hit	+= (ch->max_hit - ch->hit) * percent / 100;
        	    ch->mana    += (ch->max_mana - ch->mana) * percent / 100;
        	    ch->move    += (ch->max_move - ch->move)* percent / 100;
    		}

		return;
	    }
	    KEY( "Exp",		ch->exp,		fread_number( fp ) );
	    break;

	case 'F':

	    KEY( "FamD",	ch->pcdata->famdesc, 	fread_string( fp ) );
	    KEY( "FamL",	ch->pcdata->famlong, 	fread_string( fp ) );
	    KEY( "FamN",	ch->pcdata->famname, 	fread_string( fp ) );
	    KEY( "FamS",	ch->pcdata->famshort,	fread_string( fp ) );
	    KEY( "FamG",	ch->pcdata->famsex,     fread_number( fp ) );
	    KEY( "Feed",	ch->blood,		fread_number( fp ) );
	    KEY( "Fletch",	ch->pcdata->fletching,  fread_number( fp ) );
	    KEY( "FakeIP",      ch->pcdata->fake_ip,    fread_string( fp ) );

	case 'G':
            KEY( "God", ch->pcdata->god,                fread_number(fp));
			KEY( "Gold",	ch->gold,		fread_number( fp ) );
            KEY( "Gold_bank", ch->pcdata->gold_bank,  fread_number(fp));
			KEY( "Guild", ch->guild, guild_lookup( fread_string( fp ) ));
            KEY( "Guildtitl", ch->title_guild,        fread_string(fp));
			KEY( "GuildTime", ch->pcdata->guild_time, fread_number(fp));
			KEY( "GuildLogs", ch->pcdata->guild_logs, fread_number(fp));
			KEY( "GuildDate", ch->pcdata->guild_date, fread_number(fp));
			KEY( "Gossip",	ch->pcdata->last_gossip,	fread_number( fp ) );
            if ( !str_cmp( word, "Group" )  || !str_cmp(word,"Gr"))
            {
                int gn;
                char *temp;
 
                temp = fread_word( fp ) ;
                gn = group_lookup(temp);
                /* gn    = group_lookup( fread_word( fp ) ); */
                if ( gn < 0 )
                {
                    fprintf(stderr,"%s",temp);
                    bug( "Fread_char: unknown group. ", 0 );
                }
                else
		    gn_add(ch,gn);
                fMatch = TRUE;
            }
	    break;

	case 'H':
	    KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );
	    KEY( "Hit",		ch->hitroll,		fread_number( fp ) );

	    if ( !str_cmp( word, "HpManaMove" ) || !str_cmp(word,"HMV"))
	    {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->mana	= fread_number( fp );
		ch->max_mana	= fread_number( fp );
		ch->move	= fread_number( fp );
		ch->max_move	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

            if ( !str_cmp( word, "HpManaMovePerm" ) || !str_cmp(word,"HMVP"))
            {
                ch->pcdata->perm_hit	= fread_number( fp );
                ch->pcdata->perm_mana   = fread_number( fp );
                ch->pcdata->perm_move   = fread_number( fp );
                fMatch = TRUE;
                break;
            }
      
	    break;

	case 'I':
	    KEY( "Id",		ch->id,			fread_number( fp ) );
	    KEYS( "Image",	ch->image,		fread_string( fp ) );
	    KEY("Immtime",	ch->pcdata->wizitime,	fread_number( fp ) );
	    KEY( "InvisLevel",	ch->invis_level,	fread_number( fp ) );
	    KEY( "Inco",	ch->incog_level,	fread_number( fp ) );
	    KEY( "Invi",	ch->invis_level,	fread_number( fp ) );
	    break;
        case 'K':
            if (!str_cmp(word, "KPI1"))
            {
                ch->pcdata->approvals   = fread_number(fp);
                ch->pcdata->unapprovals = fread_number(fp);
                ch->pcdata->immtalks    = fread_number(fp);
                ch->pcdata->questcount  = fread_number(fp);
                fMatch = TRUE;
                break;
            }
            if (!str_cmp(word, "KPI2"))
            {
                ch->pcdata->guildnotes  = fread_number(fp);
                ch->pcdata->gdts        = fread_number(fp);
                ch->pcdata->ics         = fread_number(fp);
                ch->pcdata->oocs        = fread_number(fp);
                fMatch = TRUE;
                break;
            }
            break;
	
	case 'J':
	KEY( "JousterOne",	ch->jouster1,	fread_number( fp ) );
	KEY( "JousterTwo",	ch->jouster2,	fread_number( fp ) );
	KEY( "JoustAttack",	ch->joust_attack,	fread_string( fp ) );
	KEY( "JoustAttackNum",	ch->joust_attack_num,	fread_number( fp ) );
	KEY( "Jousting",	ch->in_joust,	fread_number( fp ) );
	KEY( "JoustWin",	ch->joust_win,	fread_number( fp ) );
	break;
	
	case 'L':
/*	    KEY( "LastLore",    0,                      fread_number( fp ) );*/
		KEY( "LifeEnergy",  ch->life_energy,      fread_number( fp ) );
	    KEY( "LastCleanse",	ch->pcdata->last_cleanse, 
                                               (time_t) fread_number( fp ) );
	    KEY( "LastLevel",	ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "LLev",	ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "Level",	ch->level,		fread_number( fp ) );
	    KEY( "Lev",		ch->level,		fread_number( fp ) );
	    KEY( "Levl",	ch->level,		fread_number( fp ) );
	    KEY( "LogO",	ch->pcdata->lastlogoff,	fread_number( fp ) );
	    KEY( "LongDescr",	ch->long_descr,		fread_string( fp ) );
	    KEY( "LnD",		ch->long_descr,		fread_string( fp ) );
	    KEY( "LastIP",      ch->pcdata->last_ip,    fread_string( fp ) );
	    break;
	case 'M':
			KEY( "MaxLifeEnergy",  ch->max_life_energy,      fread_number( fp ) );
            KEY( "MaxBloodpool",  ch->max_blood,      fread_number( fp ) );
            if (!str_cmp(word, "MPR"))
            {
                MPRECOG_DATA *mpr;
                mpr = ch->mprecog;
                ch->mprecog = new_mprecog();
                ch->mprecog->next = mpr;
                ch->mprecog->vnum       = fread_number( fp );
                ch->mprecog->flags      = fread_flag( fp );
                ch->mprecog->temp       = ( ( fread_number( fp ) == 0 ) ? FALSE : TRUE );
                fMatch = TRUE;
                break;
            }
            /* MPRT must come AFTER the MPR has already been read */
            if (!str_cmp(word, "MPRT"))
            {
                MPRECOG_DATA *mpr;
                INT_LIST *mprt;
                int vnum, flag, timer;
		bool found = FALSE;
                vnum  = fread_number( fp );
                flag  = fread_number( fp );
                timer = fread_number( fp );
                for ( mpr = ch->mprecog ; mpr != NULL; mpr = mpr->next )
                {
                    if ( mpr->vnum == vnum ) 
                    {
                        mprt = new_int_list();
                        mprt->value[0] = flag;
                        mprt->value[1] = timer;
                        mprt->next = mpr->timer;
                        mpr->timer = mprt;
			found = TRUE;
                    }
                }
		if ( !found )
                    bug( "fread_char: orphan MPRT found", 0);
                fMatch = TRUE;
		break;
            }

            break;

	case 'N':
	    KEYS( "Name",	ch->name,		fread_string( fp ) );
	    KEY( "Note",	ch->pcdata->last_note,	fread_number( fp ) );
	    if (!str_cmp(word,"Not"))
	    {
		ch->pcdata->last_note			= fread_number(fp);
		ch->pcdata->last_idea			= fread_number(fp);
		ch->pcdata->last_history		= fread_number(fp);
		ch->pcdata->last_news			= fread_number(fp);
		ch->pcdata->last_changes		= fread_number(fp);
		fMatch = TRUE;
		break;
	    }
  	    KEY( "NTags",	ch->pcdata->notetags,	fread_string( fp ) );

	    break;

	case 'P':
	    KEY( "Password",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Pass",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Patr",	ch->patriarch,	str_dup(fread_word( fp )) );
	    KEY( "Played",	ch->played,		fread_number( fp ) );
	    KEY( "Plyd",	ch->played,		fread_number( fp ) );
	    KEY( "Points",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Pnts",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Position",	ch->position,		fread_number( fp ) );
	    KEY( "Pos",		ch->position,		fread_number( fp ) );
	    KEY( "Practice",	ch->practice,		fread_number( fp ) );
	    KEY( "Prac",	ch->practice,		fread_number( fp ) );
	    KEY( "Pret",	ch->pcdata->pretitle,	fread_string( fp ) );
            KEY( "Proceeds",    ch->pcdata->proceeds,   fread_number( fp ) );
            KEYS( "Prompt",     ch->prompt,             fread_string( fp ) );
 	    KEYS( "Prom",	ch->prompt,		fread_string( fp ) );
	    KEY( "PCClan",     ch->pcdata->pcClan,     getClanByName( fread_string( fp ) ) );
	    break;

	case 'R':
	    KEY( "Race",        ch->race,	
				race_lookup(fread_string( fp )) );

	    if ( !str_cmp( word, "Room" ) )
	    {
		ch->in_room = get_room_index( fread_number( fp ) );
		if ( ch->in_room == NULL )
		    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
		fMatch = TRUE;
		break;
	    }
	    KEY( "RestoreChannels", ch->pcdata->restore_channels, fread_number( fp ) );
	    KEY( "RestoreEmotes", ch->pcdata->restore_emotes, fread_number( fp ) );
	    KEY( "RestoreShouts", ch->pcdata->restore_shouts, fread_number( fp ) );
	    KEY( "RestoreTells", ch->pcdata->restore_tells, fread_number( fp ) );
            KEY( "RInvis",      ch->rinvis_level,       fread_number( fp ) );

	    break;

	case 'S':
	    KEY( "SavingThrow",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Save",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Scro",	ch->lines,		fread_number( fp ) );
	    KEY( "Sex",		ch->sex,		fread_number( fp ) );
	    KEY( "ShortDescr",	ch->short_descr,	fread_string( fp ) );
	    KEYS( "ShImg1",	ch->shiftimage1, fread_string( fp ) );
	    KEYS( "ShImg2",	ch->shiftimage2, fread_string( fp ) );
	    KEY( "Shift",	ch->shiftbits,	fread_flag( fp ) );
	    KEY( "ShiftDesc1",	ch->shiftdesc1,	fread_string( fp ) );
	    KEY( "ShiftDesc2",	ch->shiftdesc2,	fread_string( fp ) );
	    KEY( "ShD",		ch->short_descr,	fread_string( fp ) );
	    KEY( "Sec",         ch->pcdata->security,	fread_number( fp ) );	/* OLC */
          KEY( "Silv",        ch->silver,             fread_number( fp ) );
	    KEY( "Silv_bank", ch->pcdata->silver_bank,  fread_number(fp));


	    if ( !str_cmp( word, "Skill" ) || !str_cmp(word,"Sk"))
	    {
		int sn;
		int value;
		char *temp;

		value = fread_number( fp );
		temp = fread_word( fp ) ;
		sn = skill_lookup(temp);
		/* sn    = skill_lookup( fread_word( fp ) ); */
		if ( sn < -2 )
		{
                    /* buddha palm renamed to burning palm-- 4/11/2012 gkl */
                    if ( !str_cmp(temp,"buddha palm") )
		        ch->pcdata->learned[gsn_burning_palm] = value;
                    else
		        bug( "Fread_char: unknown skill. ", 0 );
		}
		else
		  {
		    /* update to reflect minimum skill/spell */
		    if( value > 0 && value < MIN_SKILL_PCT )
		      value = MIN_SKILL_PCT;

		    ch->pcdata->learned[sn] = value;
		  }
		fMatch = TRUE;
	    }
	    break;

	case 'T':
	  KEY( "Trouble", ch->pcdata->last_trouble, fread_number( fp ) );
            KEY( "TrueSex",     ch->pcdata->true_sex,  	fread_number( fp ) );
	    KEY( "TSex",	ch->pcdata->true_sex,   fread_number( fp ) );
	    KEY( "Trai",	ch->train,		fread_number( fp ) );
	    KEY( "Trade",	ch->pcdata->tailoring,  fread_number( fp ) );
	    KEY( "Trust",	ch->trust,		fread_number( fp ) );
	    KEY( "Tru",		ch->trust,		fread_number( fp ) );
         
	    if ( !str_cmp( word, "Title" )  || !str_cmp( word, "Titl"))
	    {
		ch->pcdata->title = fread_string( fp );
    		if (ch->pcdata->title[0] != '.' && ch->pcdata->title[0] != ',' 
		&&  ch->pcdata->title[0] != '!' && ch->pcdata->title[0] != '?')
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    free_string( ch->pcdata->title );
		    ch->pcdata->title = str_dup( buf );
		}
		fMatch = TRUE;
		break;
	    }
	    KEY( "Tin",         ch->pcdata->transin,    fread_string( fp ) );
	    KEY( "Tout",        ch->pcdata->transout,   fread_string( fp ) );

	    break;

	case 'V':
/*          KEY( "Version",     ch->pcdata->version,    fread_number ( fp ) );*/
            if ( !str_cmp( word, "Vers" ) )
            {
                ch->pcdata->version = fread_number(fp);
                /* for version < 7, pre-set the default color settings to the 
                old values so that the prompt coloring conversion can be done 
                properly */
                if ( ch->pcdata->version < 7 )
                {
                    ch->colors[C_HP]       = 'y';
                    ch->colors[C_MAXHP]    = 'c';
                    ch->colors[C_MANA]     = 'c';
                    ch->colors[C_MAXMANA]  = 'r';
                    ch->colors[C_MOVES]    = 'r';
                    ch->colors[C_MAXMOVES] = 'C';
                }
                fMatch = TRUE;
                break;
            }

	    KEY( "Vistime",	ch->pcdata->vistime,    fread_number ( fp ) );
	    
	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wimp",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wizn",	ch->wiznet_flags,		fread_flag( fp ) );
	    break;
	}

	if ( !fMatch )
	{
    	    sprintf(buf2,"Fread_char: %s no match.", word );
	    bug(buf2,0);
	    fread_to_eol( fp );
	} else {
		if (ch->guild != GUILD_BOGUS && !is_in_guild(ch->guild, ch->name)) {
			ch->guild = GUILD_BOGUS;
		}

		if (!IS_NULLSTR(ch->cln_apply) && !is_applicant(guild_lookup(ch->cln_apply), ch->name)) {
			free_string(ch->cln_apply);
		}
	}
    }
}

/* load a pet from the forgotten reaches */
void fread_pet( CHAR_DATA *ch, FILE *fp )
{
    char *word;
    CHAR_DATA *pet;
    bool fMatch;
    int lastlogoff = current_time;
    int percent;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "END" : fread_word(fp);
    if (!str_cmp(word,"Vnum"))
    {
    	int vnum;
    	
    	vnum = fread_number(fp);
    	if (get_mob_index(vnum) == NULL)
	{
    	    bug("Fread_pet: bad vnum %d.",vnum);
	    pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
	}
    	else
    	    pet = create_mobile(get_mob_index(vnum));
    }
    else
    {
        bug("Fread_pet: no vnum in file.",0);
        pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
    }
    
    for ( ; ; )
    {
    	word 	= feof(fp) ? "END" : fread_word(fp);
    	fMatch = FALSE;
    	
    	switch (UPPER(word[0]))
    	{
    	case '*':
    	    fMatch = TRUE;
    	    fread_to_eol(fp);
    	    break;
    		
    	case 'A':
    	    KEY( "Act",		pet->act_bits,		fread_flag(fp));
    	    KEY( "AfBy",	pet->affected_by,	fread_flag(fp));
    	    KEY( "AfBy2",	pet->affected_by2,	fread_flag(fp));
    	    KEY( "AfBy3",	pet->affected_by3,	fread_flag(fp));
    	    KEY( "Alig",	pet->alignment,		fread_number(fp));
    	    
    	    if (!str_cmp(word,"ACs"))
    	    {
    	    	int i;
    	    	
    	    	for (i = 0; i < 4; i++)
    	    	    pet->armor[i] = fread_number(fp);
    	    	fMatch = TRUE;
    	    	break;
    	    }
    	    
    	    if (!str_cmp(word,"AffD"))
    	    {
    	    	AFFECT_DATA *paf;
    	    	int sn;
    	    	
    	    	paf = new_affect();
    	    	
    	    	sn = skill_lookup(fread_word(fp));
    	     	if (sn < -2)
    	     	    bug("Fread_char: unknown skill.",0);
    	     	else
    	     	   paf->type = sn;
    	     	   
    	     	paf->level	= fread_number(fp);
    	     	paf->duration	= fread_number(fp);
    	     	paf->modifier	= fread_number(fp);
    	     	paf->location	= fread_number(fp);
    	     	paf->whichaff	= fread_number(fp);
    	     	paf->bitvector	= fread_number(fp);
    	     	paf->next	= pet->affected;
    	     	pet->affected	= paf;
    	     	fMatch		= TRUE;
    	     	break;
    	    }

            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < -2)
                    bug("Fread_char: unknown skill.",0);
                else
                   paf->type = sn;
 
		paf->where	= fread_number(fp);
                paf->level      = fread_number(fp);
                paf->duration   = fread_number(fp);
                paf->modifier   = fread_number(fp);
                paf->location   = fread_number(fp);
                paf->whichaff   = fread_number(fp);
                paf->bitvector  = fread_number(fp);
                paf->next       = pet->affected;
                pet->affected   = paf;
                fMatch          = TRUE;
                break;
            }
    	     
    	    if (!str_cmp(word,"AMod"))
    	    {
    	     	int stat;
    	     	
    	     	for (stat = 0; stat < MAX_STATS; stat++)
    	     	    pet->mod_stat[stat] = fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	    }
    	     
    	    if (!str_cmp(word,"Attr"))
    	    {
    	         int stat;
    	         
    	         for (stat = 0; stat < MAX_STATS; stat++)
    	             pet->perm_stat[stat] = fread_number(fp);
    	         fMatch = TRUE;
    	         break;
    	    }
    	    break;
    	     
    	 case 'C':
             KEY( "Clan",       pet->guild,       guild_lookup(fread_string(fp)));
    	     KEY( "Comm",	pet->comm,		fread_flag(fp));
    	     break;
    	     
    	 case 'D':
    	     KEY( "Dam",	pet->damroll,		fread_number(fp));
    	     KEY( "Desc",	pet->description,	fread_string(fp));
    	     break;
    	     
    	 case 'E':
    	     if (!str_cmp(word,"End"))
	     {
		pet->leader = ch;
		pet->master = ch;
		ch->pet = pet;
    		/* adjust hp mana move up  -- here for speed's sake */
    		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);
 
    		if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
    		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
    		{
		    percent = UMIN(percent,100);
    		    pet->hit	+= (pet->max_hit - pet->hit) * percent / 100;
        	    pet->mana   += (pet->max_mana - pet->mana) * percent / 100;
        	    pet->move   += (pet->max_move - pet->move)* percent / 100;
    		}
    	     	return;
	     }
    	     KEY( "Exp",	pet->exp,		fread_number(fp));
    	     break;
    	     
    	 case 'G':
    	     KEY( "Gold",	pet->gold,		fread_number(fp));
	     KEY( "Guild",      pet->guild,             guild_lookup( fread_string( fp ) ) );
    	     break;
    	     
    	 case 'H':
    	     KEY( "Hit",	pet->hitroll,		fread_number(fp));
    	     
    	     if (!str_cmp(word,"HMV"))
    	     {
    	     	pet->hit	= fread_number(fp);
    	     	pet->max_hit	= fread_number(fp);
    	     	pet->mana	= fread_number(fp);
    	     	pet->max_mana	= fread_number(fp);
    	     	pet->move	= fread_number(fp);
    	     	pet->max_move	= fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	     }
    	     break;
    	     
     	case 'L':
    	     KEY( "Levl",	pet->level,		fread_number(fp));
    	     KEY( "LnD",	pet->long_descr,	fread_string(fp));
	     KEY( "LogO",	lastlogoff,		fread_number(fp));
    	     break;
    	     
    	case 'N':
    	     KEY( "Name",	pet->name,		fread_string(fp));
    	     break;
    	     
    	case 'P':
    	     KEY( "Pos",	pet->position,		fread_number(fp));
    	     break;
    	     
	case 'R':
    	    KEY( "Race",	pet->race, race_lookup(fread_string(fp)));
    	    break;
 	    
    	case 'S' :
    	    KEY( "Save",	pet->saving_throw,	fread_number(fp));
    	    KEY( "Sex",		pet->sex,		fread_number(fp));
    	    KEY( "ShD",		pet->short_descr,	fread_string(fp));
            KEY( "Silv",        pet->silver,            fread_number( fp ) );
    	    break;
    	    
    	}
    }
}

extern	OBJ_DATA	*obj_free;

void fread_obj( CHAR_DATA *ch, FILE *fp )
{
    OBJ_DATA *obj;
    char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    bool first;
    bool new_format;  /* to prevent errors */
    bool make_new;    /* update object */
    
    fVnum = FALSE;
    obj = NULL;
    first = TRUE;  /* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    word   = feof( fp ) ? "End" : fread_word( fp );
    if (!str_cmp(word,"Vnum" ))
    {
        int vnum;
	first = FALSE;  /* fp will be in right place */
 
        vnum = fread_number( fp );
        if (  get_obj_index( vnum )  == NULL )
	{
            bug( "Fread_obj: bad vnum %d.", vnum );
	}
        else
	{
	    obj = create_object(get_obj_index(vnum),-1);
	    new_format = TRUE;
	}
	    
    }

    if (obj == NULL)  /* either not found or old style */
    {
    	obj = new_obj();
    	obj->name		= str_dup( "" );
    	obj->short_descr	= str_dup( "" );
    	obj->description	= str_dup( "" );
    }

    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;

    for ( ; ; )
    {
	if (first)
	    first = FALSE;
	else
	    word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if (!str_cmp(word,"AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < -2)
		    bug("Fread_obj: unknown skill.",0);
		else
		    paf->type = sn;

		paf->level	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->whichaff	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= obj->affected;
		obj->affected	= paf;
		fMatch		= TRUE;
		break;
	    }
            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < -2)
                    bug("Fread_obj: unknown skill.",0);
                else
                    paf->type = sn;
 
		paf->where	= fread_number( fp );
                paf->level      = fread_number( fp );
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->whichaff   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->next       = obj->affected;
                obj->affected   = paf;
                fMatch          = TRUE;
                break;
            }
	    break;

	case 'C':
	    KEY( "Cond",	obj->condition,		fread_number( fp ) );
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    KEY( "Desc",	obj->description,	fread_string( fp ) );
	    KEY( "Donor",       obj->donor,             fread_string( fp ) );
	    break;

	case 'E':

	    if ( !str_cmp( word, "Enchanted"))
	    {
		obj->enchanted = TRUE;
	 	fMatch 	= TRUE;
		break;
	    }

	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtF",	obj->extra_flags,	fread_number( fp ) );

	    if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe"))
	    {
		EXTRA_DESCR_DATA *ed;

		ed = new_extra_descr();

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !fNest || ( fVnum && obj->pIndexData == NULL ) )
		{
		    bug( "Fread_obj: incomplete object.", 0 );
		    free_obj(obj);
		    return;
		}
		else
	        {
		    if ( !fVnum )
		    {
			free_obj( obj );
			obj = create_object( get_obj_index( OBJ_VNUM_DUMMY ), 0 );
		    }

		    if (!new_format)
		    {
		    	obj->next	= object_list;
		    	object_list	= obj;
		    	obj->pIndexData->count++;
		    }

		    if (!obj->pIndexData->new_format 
		    && obj->item_type == ITEM_ARMOR
		    &&  obj->value[1] == 0)
		    {
			obj->value[1] = obj->value[0];
			obj->value[2] = obj->value[0];
		    }
		    if (make_new)
		    {
			int wear;
			
			wear = obj->wear_loc;
			extract_obj(obj);

			obj = create_object(obj->pIndexData,0);
			obj->wear_loc = wear;
		    }
		    if ( iNest == 0 || rgObjNest[iNest] == NULL )
			obj_to_char_init( obj, ch );
		    else
			obj_to_obj( obj, rgObjNest[iNest-1] );
		    return;
		}
	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    KEY( "Ityp",	obj->item_type,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    KEY( "Lev",		obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

   	case 'O':
	    if ( !str_cmp( word,"Oldstyle" ) )
	    {
		if (obj->pIndexData != NULL && obj->pIndexData->new_format)
		    make_new = TRUE;
		fMatch = TRUE;
	    }
	    break;
		    

	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		obj->short_descr,	fread_string( fp ) );

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 4 )
		{
		    bug( "Fread_obj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }
	    if ( !str_cmp( word, "Spelln" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 4 )
		{
		    bug( "Fread_obj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = -1 * sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    KEY( "Time",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals"))
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		   obj->value[0] = obj->pIndexData->value[0];
                fix_obj_values( obj );
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Val" ) )
	    {
		obj->value[0] 	= fread_number( fp );
	 	obj->value[1]	= fread_number( fp );
	 	obj->value[2] 	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		obj->value[4]	= fread_number( fp );
                fix_obj_values( obj );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		    bug( "Fread_obj: bad vnum %d.", vnum );
		else
		    fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WeaF",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    KEY( "Wt",		obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    bug( "Fread_obj: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
}

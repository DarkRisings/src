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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"

/* Generic integer linked-list */
INT_LIST *int_list_free;
INT_LIST *new_int_list(void)
{
    static INT_LIST int_list_zero;
    INT_LIST *int_list;

    if (int_list_free == NULL)
        int_list = alloc_perm(sizeof(*int_list));
    else
    {
        int_list = int_list_free;
        int_list_free = int_list->next;
    }

    /* Might as well initialize to something */
    int_list->value[0] = 0;
    int_list->value[1] = 0;
    *int_list = int_list_zero;
    return int_list;
}
void free_int_list(INT_LIST *int_list)
{
    int_list->next = int_list_free;
    int_list_free = int_list;
}

/* stuff for quests and quest journal entries */
QUEST_DATA *quest_free;
QUEST_DATA *new_quest_data(void)
{
    static QUEST_DATA quest_zero;
    QUEST_DATA *quest;
    if ( quest_free == NULL )
        quest = alloc_perm( sizeof(*quest) );
    else
    {
        quest = quest_free;
        quest_free = quest->next;
    }
    *quest =  quest_zero;
    VALIDATE(quest);
    quest->name = str_dup("");
    return quest;
}
void free_quest_data(QUEST_DATA *quest)
{
    JOURNAL_DATA *journal, *journal_next;
    if ( !IS_VALID(quest) )
        return;
    INVALIDATE(quest);
    for ( journal = quest->journal; journal != NULL; journal = journal_next )
    {
        journal_next = journal->next;
        free_journal_data( journal );
    }
    free_string( quest->name );
    quest->next = quest_free;
    quest_free = quest;

}
JOURNAL_DATA *journal_free;
JOURNAL_DATA *new_journal_data( void )
{
    static JOURNAL_DATA journal_zero;
    JOURNAL_DATA *journal;
    if ( journal_free == NULL )
        journal = alloc_perm( sizeof(*journal) );
    else
    {
        journal = journal_free;
        journal_free = journal->next;
    }
    *journal = journal_zero;
    VALIDATE(journal);
    journal->text = str_dup("");
    journal->flag = 0;
    journal->vnum = 0;
    journal->step = 0;
    return journal;
}
void free_journal_data(JOURNAL_DATA *journal)
{
    if ( !IS_VALID(journal) )
        return;
    INVALIDATE(journal);
    
    free_string( journal->text );
    journal->quest = NULL;
    journal->next = journal_free;
    journal_free = journal;

}
/* stuff for recycling mprog delay structures */
MPDELAY_DATA *mpdelay_free;
MPDELAY_DATA *new_mpdelay(void)
{
    static MPDELAY_DATA mpdelay_zero;
    MPDELAY_DATA *mpdelay;

    if (mpdelay_free == NULL)
        mpdelay = alloc_perm(sizeof(*mpdelay));
    else
    {
        mpdelay = mpdelay_free;
        mpdelay_free = mpdelay_free->next;
    }

    *mpdelay = mpdelay_zero;
    VALIDATE(mpdelay);
    return mpdelay;
}
void free_mpdelay(MPDELAY_DATA *mpdelay)
{
    if (!IS_VALID(mpdelay))
        return;

    INVALIDATE(mpdelay);
    free_string( mpdelay->code );

    mpdelay->execute = FALSE;
    mpdelay->next = mpdelay_free;
    mpdelay_free = mpdelay;
}

MPRECOG_DATA *mprecog_free;
MPRECOG_DATA *new_mprecog(void)
{
    static MPRECOG_DATA mprecog_zero;
    MPRECOG_DATA *mprecog;

    if (mprecog_free == NULL)
        mprecog = alloc_perm(sizeof(*mprecog));
    else
    {
        mprecog = mprecog_free;
        mprecog_free = mprecog_free->next;
    }

    *mprecog = mprecog_zero;
    VALIDATE(mprecog);
/*  Just in case! */
    mprecog->vnum = 0;
    mprecog->flags = 0;
    mprecog->temp = FALSE;
    mprecog->next = NULL;
    return mprecog;
}
void free_mprecog(MPRECOG_DATA *mprecog)
{
    INT_LIST *mprt, *mprt_next;

    if (!IS_VALID(mprecog))
        return;

    INVALIDATE(mprecog);
    mprecog->vnum = 0;
    mprecog->temp = FALSE;
    mprecog->flags = 0;

    for ( mprt = mprecog->timer; mprt != NULL; mprt = mprt_next )
    {
        mprt_next = mprt->next;
        free_int_list( mprt );
    }

    mprecog->next = mprecog_free;
    mprecog_free = mprecog;
}

/* stuff for recyling notes */
NOTE_DATA *note_free;

NOTE_DATA *new_note()
{
    NOTE_DATA *note;

    if (note_free == NULL)
	note = alloc_perm(sizeof(*note));
    else
    { 
	note = note_free;
	note_free = note_free->next;
    }
    VALIDATE(note);
    note->persist = FALSE;
    return note;
}

void free_note(NOTE_DATA *note)
{
    if (!IS_VALID(note))
	return;

    free_string( note->text    );
    free_string( note->subject );
    free_string( note->to_list );
    free_string( note->date    );
    free_string( note->sender  );
    INVALIDATE(note);

    note->persist = FALSE;
    note->next    = note_free;
    note_free     = note;
}


BADNAME_DATA *badname_free;

BADNAME_DATA *new_badname( void )
{
    static BADNAME_DATA badname_zero;
    BADNAME_DATA *badname;

    if (badname_free == NULL)
        badname = alloc_perm(sizeof(*badname));
    else
    {
        badname = badname_free;
        badname_free = badname_free->next;
    }

    *badname = badname_zero;
    VALIDATE(badname);
    badname->name = &str_empty[0];
    badname->type = BADNAME_EXACT;
    return badname;
}

void free_badname(BADNAME_DATA *badname)
{
    if (!IS_VALID(badname))
        return;
    free_string(badname->name);
    INVALIDATE(badname);
    badname->next = badname_free;
    badname_free = badname;

    return;
}

MULTILOGIN_DATA *multilogin_free;

MULTILOGIN_DATA *new_multilogin(void)
{
    static MULTILOGIN_DATA multilogin_zero;
    MULTILOGIN_DATA *multilogin;

    if (multilogin_free == NULL)
        multilogin = alloc_perm(sizeof(*multilogin));
    else
    {
        multilogin = multilogin_free;
        multilogin_free = multilogin_free->next;
    }

    *multilogin = multilogin_zero;
    VALIDATE(multilogin);
    multilogin->name = &str_empty[0];
    multilogin->comment = &str_empty[0];
    return multilogin;
}
     
void free_multilogin(MULTILOGIN_DATA *multilogin)
{
    if (!IS_VALID(multilogin))
        return;
    
    free_string(multilogin->name);
    free_string(multilogin->comment);
    INVALIDATE(multilogin);

    multilogin->next = multilogin_free;
    multilogin_free = multilogin;
}


/* stuff for recycling ban structures */
BAN_DATA *ban_free;

BAN_DATA *new_ban(void)
{
    static BAN_DATA ban_zero;
    BAN_DATA *ban;

    if (ban_free == NULL)
	ban = alloc_perm(sizeof(*ban));
    else
    {
	ban = ban_free;
	ban_free = ban_free->next;
    }

    *ban = ban_zero;
    VALIDATE(ban);
    ban->name = &str_empty[0];
    ban->comment = &str_empty[0];
    return ban;
}

void free_ban(BAN_DATA *ban)
{
    if (!IS_VALID(ban))
	return;

    free_string(ban->name);
    free_string(ban->comment);
    INVALIDATE(ban);

    ban->next = ban_free;
    ban_free = ban;
}

/* recycling bank accounts */
BANKACCT_DATA *bankacct_free;
BANKACCT_DATA *new_bankacct( void )
{
    static BANKACCT_DATA bankacct_zero =
    {
        NULL,
        FALSE,
        &str_empty[0],
        0,
        0,
        0
    };
    BANKACCT_DATA *acct;

    if (bankacct_free == NULL)
	acct = alloc_perm(sizeof(*acct));
    else
    {
	acct = bankacct_free;
	bankacct_free = bankacct_free->next;
    }

    *acct = bankacct_zero;
    VALIDATE(acct);
    return acct;
}
void free_bankacct(BANKACCT_DATA *acct)
{
    if ( !IS_VALID(acct) )
	return;

    free_string(acct->owner);
    INVALIDATE(acct);

    acct->next = bankacct_free;
    bankacct_free = acct;
}


/* stuff for recycling descriptors */
DESCRIPTOR_DATA *descriptor_free;

DESCRIPTOR_DATA *new_descriptor(void)
{
    static DESCRIPTOR_DATA d_zero;
    DESCRIPTOR_DATA *d;

    if (descriptor_free == NULL)
	d = alloc_perm(sizeof(*d));
    else
    {
	d = descriptor_free;
	descriptor_free = descriptor_free->next;
    }
	
    *d = d_zero;
    VALIDATE(d);
    return d;
}

void free_descriptor(DESCRIPTOR_DATA *d)
{
    if (!IS_VALID(d))
	return;

    free_string( d->host );
    free_string( d->showstr_head );
    free_mem( d->outbuf, d->outsize );
    INVALIDATE(d);
    d->next = descriptor_free;
    descriptor_free = d;
}

/* stuff for recycling gen_data */
GEN_DATA *gen_data_free;

GEN_DATA *new_gen_data(void)
{
    static GEN_DATA gen_zero;
    GEN_DATA *gen;

    if (gen_data_free == NULL)
	gen = alloc_perm(sizeof(*gen));
    else
    {
	gen = gen_data_free;
	gen_data_free = gen_data_free->next;
    }
    *gen = gen_zero;
    VALIDATE(gen);
    return gen;
}

void free_gen_data(GEN_DATA *gen)
{
    if (!IS_VALID(gen))
	return;

    INVALIDATE(gen);

    gen->next = gen_data_free;
    gen_data_free = gen;
} 

/* stuff for recycling extended descs */
EXTRA_DESCR_DATA *extra_descr_free;

EXTRA_DESCR_DATA *new_extra_descr(void)
{
    EXTRA_DESCR_DATA *ed;

    if (extra_descr_free == NULL)
	ed = alloc_perm(sizeof(*ed));
    else
    {
	ed = extra_descr_free;
	extra_descr_free = extra_descr_free->next;
    }

    ed->keyword = &str_empty[0];
    ed->description = &str_empty[0];
    VALIDATE(ed);
    return ed;
}

void free_extra_descr(EXTRA_DESCR_DATA *ed)
{
    if (!IS_VALID(ed))
	return;

    free_string(ed->keyword);
    free_string(ed->description);
    INVALIDATE(ed);
    
    ed->next = extra_descr_free;
    extra_descr_free = ed;
}


/* stuff for recycling affects */
AFFECT_DATA *affect_free;

AFFECT_DATA *new_affect(void)
{
    static AFFECT_DATA af_zero;
    AFFECT_DATA *af;

    if (affect_free == NULL)
	af = alloc_perm(sizeof(*af));
    else
    {
	af = affect_free;
	affect_free = affect_free->next;
    }

    *af = af_zero;

    VALIDATE(af);
    return af;
}

void free_affect(AFFECT_DATA *af)
{
    if (!IS_VALID(af))
    {
        log_string( "free_affect: got invalid af" );
	return;
    }

    INVALIDATE(af);
    af->next = affect_free;
    affect_free = af;
}

/* stuff for recycling objects */
OBJ_DATA *obj_free;

OBJ_DATA *new_obj(void)
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;

    if (obj_free == NULL)
	obj = alloc_perm(sizeof(*obj));
    else
    {
	obj = obj_free;
	obj_free = obj_free->next;
    }
    *obj = obj_zero;
    VALIDATE(obj);

    return obj;
}

void free_obj(OBJ_DATA *obj)
{
    AFFECT_DATA *paf, *paf_next;
    EXTRA_DESCR_DATA *ed, *ed_next;

    if (!IS_VALID(obj))
	return;

    for (paf = obj->affected; paf != NULL; paf = paf_next)
    {
	paf_next = paf->next;
	free_affect(paf);
    }
    obj->affected = NULL;

    for (ed = obj->extra_descr; ed != NULL; ed = ed_next )
    {
	ed_next = ed->next;
	free_extra_descr(ed);
     }
     obj->extra_descr = NULL;
   
    free_string( obj->name        );
    free_string( obj->description );
    free_string( obj->short_descr );
    free_string( obj->owner       );
    free_string( obj->material    );
    free_string( obj->donor       );
    INVALIDATE(obj);

    obj->next   = obj_free;
    obj_free    = obj; 
}


/* stuff for recyling characters */
CHAR_DATA *char_free;
CHAR_DATA *new_char (void)
{
    static CHAR_DATA ch_zero;
    CHAR_DATA *ch;
    int i;

    if (char_free == NULL)
	ch = alloc_perm(sizeof(*ch));
    else
    {
	ch = char_free;
	char_free = char_free->next;
    }

    *ch				= ch_zero;
    VALIDATE(ch);
    ch->name                    = &str_empty[0];
    ch->short_descr             = &str_empty[0];
    ch->long_descr              = &str_empty[0];
    ch->description             = &str_empty[0];
    ch->image			= &str_empty[0];
    ch->charmed_by              = &str_empty[0];
    ch->write_buffer		= &str_empty[0];
    ch->shiftimage1		= &str_empty[0];
    ch->shiftimage2		= &str_empty[0];
    ch->shiftdesc1              = &str_empty[0];
    ch->shiftdesc2              = &str_empty[0];
    ch->patriarch		= &str_empty[0];
	ch->cln_name		= &str_empty[0];
	ch->cln_leader		= &str_empty[0];
	ch->cln_symbol		= &str_empty[0];
	ch->cln_apply		= &str_empty[0];
	ch->jouster1 = 0;
	ch->jouster2 = 0;
	ch->joust_attack = &str_empty[0];
	ch->joust_attack_num = 0;
	ch->in_joust = 0;
	ch->joust_win = 0;
    ch->blood    		= 0;
    ch->max_blood               = 250;
    ch->blood_percentage        = 100;
/*  ch->fightlag		= 0; */
    ch->wstimer                 = 0;
    ch->kicktimer               = 0;
    ch->timer                   = 0;
    ch->prompt                  = &str_empty[0];
    ch->prefix			= &str_empty[0];
    ch->logon                   = current_time;
    ch->lines                   = PAGELEN;
    for (i = 0; i < 4; i++)
        ch->armor[i]            = 100;
    ch->position                = POS_STANDING;
    ch->hit                     = 20;
    ch->max_hit                 = 20;
    ch->mana                    = 100;
    ch->max_mana                = 100;
    ch->move                    = 100;
    ch->max_move                = 100;
    ch->affected_by             = 0;
    ch->affected_by2            = 0;
    ch->affected_by3            = 0;
    ch->charmies		= 0;
    ch->purge_on_areset         = FALSE; /* For marker mobs */
    ch->prand			= -1;
    ch->mprog_target            = NULL;
    for (i = 0; i < MAX_STATS; i ++)
    {
        ch->perm_stat[i] = 13;
        ch->mod_stat[i] = 0;
    }

    for ( i = 0; i < MAX_COLOR; i++ )
        ch->colors[i] = colordefs[i].color;

    return ch;
}


void free_char (CHAR_DATA *ch)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    MPRECOG_DATA *mpr;
    MPRECOG_DATA *mpr_next; 

    if (!IS_VALID(ch))
	return;

    if (IS_NPC(ch))
	mobile_count--;

    for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	extract_obj(obj);
    }

    for (paf = ch->affected; paf != NULL; paf = paf_next)
    {
	paf_next = paf->next;
	affect_remove(ch,paf);
    }
    for (mpr = ch->mprecog; mpr != NULL; mpr = mpr->next )
    {
        mpr_next = mpr->next;
        free_mprecog( mpr );
    }

    free_string(ch->name);
    free_string(ch->short_descr);
    free_string(ch->long_descr);
    free_string(ch->description);
    free_string(ch->image);
    free_string(ch->shiftimage1);
    free_string(ch->shiftimage2);
    free_string(ch->write_buffer);
    free_string(ch->charmed_by);
    free_string(ch->prompt);
    free_string(ch->prefix);
    free_string(ch->shiftdesc1);
    free_string(ch->shiftdesc2);
    free_string(ch->patriarch);
    free_string(ch->title_guild);
    free_string(ch->material);
	free_string(ch->cln_name);
	free_string(ch->cln_leader);
	free_string(ch->cln_symbol);
	free_string(ch->cln_apply);
	free_string(ch->joust_attack);

    if (ch->pcdata != NULL)
    	free_pcdata(ch->pcdata);

    ch->next = char_free;
    char_free  = ch;

    INVALIDATE(ch);
    return;
}

PC_DATA *pcdata_free;

PC_DATA *new_pcdata(void)
{
    static PC_DATA pcdata_zero;
    PC_DATA *pcdata;

    if (pcdata_free == NULL)
	pcdata = alloc_perm(sizeof(*pcdata));
    else
    {
	pcdata = pcdata_free;
	pcdata_free = pcdata_free->next;
    }

    *pcdata = pcdata_zero;

    pcdata->buffer = new_buf();
    
    VALIDATE(pcdata);
    return pcdata;
}
	

void free_pcdata(PC_DATA *pcdata)
{
    if (!IS_VALID(pcdata))
	return;
   /* Null tradeskills */
    pcdata->tailoring   = 0;
    pcdata->baking      = 0;
    pcdata->fletching   = 0;
    pcdata->wizitime    = 0;
    pcdata->vistime     = 0;
    pcdata->approvals   = 0;
    pcdata->unapprovals = 0;
    pcdata->guildnotes  = 0;
    pcdata->gdts        = 0;
    pcdata->immtalks    = 0;
    pcdata->ics         = 0;
    pcdata->oocs        = 0;
    pcdata->questcount  = 0;
    pcdata->famsex      = 0;
    pcdata->proceeds    = 0;
    pcdata->lastfight   = 0;
    pcdata->lastpk      = 0;
    pcdata->restore_channels = 0;
    pcdata->restore_emotes = 0;
    pcdata->restore_shouts = 0;
    pcdata->restore_tells = 0;
    pcdata->cleansings = 0;
    pcdata->cleanse_timer = 0;
    pcdata->last_cleanse = 0;
    pcdata->offering = NULL; /* Just to be safe */
    pcdata->verifytrain = FALSE;

    pcdata->pcClan = NULL;
    pcdata->pcClanApply = NULL;

    free_string(pcdata->pwd);
    free_string(pcdata->bamfin);
    free_string(pcdata->bamfout);
    free_string(pcdata->transin);
    free_string(pcdata->transout);
    free_string(pcdata->title);
    free_string(pcdata->pretitle);
    free_string(pcdata->quest_team_captain);
    free_string(pcdata->fake_ip);
    free_string(pcdata->last_ip);
    free_string(pcdata->notetags);
    free_string(pcdata->famshort);
    free_string(pcdata->famlong);
    free_string(pcdata->famname);
    free_string(pcdata->famdesc);
    free_buf(pcdata->buffer);
       
    INVALIDATE(pcdata);
    pcdata->next = pcdata_free;
    pcdata_free = pcdata;

    return;
}

	


/* stuff for setting ids */
long	last_pc_id;
long	last_mob_id;

long get_pc_id(void)
{
    int val;

    val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
    last_pc_id = val;
    return val;
}

long get_mob_id(void)
{
    last_mob_id++;
    return last_mob_id;
}

MEM_DATA *mem_data_free;

/* procedures and constants needed for buffering */

BUFFER *buf_free;

MEM_DATA *new_mem_data(void)
{
    MEM_DATA *memory;
  
    if (mem_data_free == NULL)
	memory = alloc_mem(sizeof(*memory));
    else
    {
	memory = mem_data_free;
	mem_data_free = mem_data_free->next;
    }

    memory->next = NULL;
    memory->id = 0;
    memory->reaction = 0;
    memory->when = 0;
    VALIDATE(memory);

    return memory;
}

void free_mem_data(MEM_DATA *memory)
{
    if (!IS_VALID(memory))
	return;

    memory->next = mem_data_free;
    mem_data_free = memory;
    INVALIDATE(memory);
}



/* buffer sizes */
const int buf_size[MAX_BUF_LIST] =
{
    16,32,64,128,256,1024,2048,4096,8192,16384
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int get_size (int val)
{
    int i;

    for (i = 0; i < MAX_BUF_LIST; i++)
	if (buf_size[i] >= val)
	{
	    return buf_size[i];
	}
    
    return -1;
}

BUFFER *new_buf()
{
    BUFFER *buffer;

    if (buf_free == NULL) 
	buffer = alloc_perm(sizeof(*buffer));
    else
    {
	buffer = buf_free;
	buf_free = buf_free->next;
    }

    buffer->next	= NULL;
    buffer->state	= BUFFER_SAFE;
    buffer->size	= get_size(BASE_BUF);

    buffer->string	= alloc_mem(buffer->size);
    buffer->string[0]	= '\0';
    VALIDATE(buffer);

    return buffer;
}

BUFFER *new_buf_size(int size)
{
    BUFFER *buffer;
 
    if (buf_free == NULL)
        buffer = alloc_perm(sizeof(*buffer));
    else
    {
        buffer = buf_free;
        buf_free = buf_free->next;
    }
 
    buffer->next        = NULL;
    buffer->state       = BUFFER_SAFE;
    buffer->size        = get_size(size);
    if (buffer->size == -1)
    {
        bug("new_buf: buffer size %d too large.",size);
        exit(1);
    }
    buffer->string      = alloc_mem(buffer->size);
    buffer->string[0]   = '\0';
    VALIDATE(buffer);
 
    return buffer;
}


void free_buf(BUFFER *buffer)
{
    if (!IS_VALID(buffer))
	return;

    free_mem(buffer->string,buffer->size);
    buffer->string = NULL;
    buffer->size   = 0;
    buffer->state  = BUFFER_FREED;
    INVALIDATE(buffer);

    buffer->next  = buf_free;
    buf_free      = buffer;
}


bool add_buf(BUFFER *buffer, char *string)
{
    int len;
    char *oldstr;
    int oldsize;

    oldstr = buffer->string;
    oldsize = buffer->size;

    if (buffer->state == BUFFER_OVERFLOW) /* don't waste time on bad strings! */
	return FALSE;

    len = strlen(buffer->string) + strlen(string) + 1;

    while (len >= buffer->size) /* increase the buffer size */
    {
	buffer->size 	= get_size(buffer->size + 1);
	{
	    if (buffer->size == -1) /* overflow */
	    {
		buffer->size = oldsize;
		buffer->state = BUFFER_OVERFLOW;
		bug("buffer overflow past size %d",buffer->size);
		return FALSE;
	    }
  	}
    }

    if (buffer->size != oldsize)
    {
	buffer->string	= alloc_mem(buffer->size);

	strcpy(buffer->string,oldstr);
	free_mem(oldstr,oldsize);
    }

    strcat(buffer->string,string);
    return TRUE;
}


void clear_buf(BUFFER *buffer)
{
    buffer->string[0] = '\0';
    buffer->state     = BUFFER_SAFE;
}


char *buf_string(BUFFER *buffer)
{
    return buffer->string;
}

/* stuff for recycling mobprograms */
MPROG_LIST *mprog_free;

MPROG_LIST *new_mprog(void)
{
   static MPROG_LIST mp_zero;
   MPROG_LIST *mp;

   if (mprog_free == NULL)
       mp = alloc_perm(sizeof(*mp));
   else
   {
       mp = mprog_free;
       mprog_free=mprog_free->next;
   }

   *mp = mp_zero;
   mp->vnum             = 0;
   mp->trig_type        = 0;
   mp->code             = str_dup("");
   VALIDATE(mp);
   return mp;
}

void free_mprog(MPROG_LIST *mp)
{
   if (!IS_VALID(mp))
      return;

   INVALIDATE(mp);
   mp->next = mprog_free;
   mprog_free = mp;
}

HELP_AREA * had_free;

HELP_AREA * new_had ( void )
{
	HELP_AREA * had;

	if ( had_free )
	{
		had		= had_free;
		had_free	= had_free->next;
	}
	else
		had		= alloc_perm( sizeof( *had ) );

	return had;
}

HELP_DATA * help_free;

HELP_DATA * new_help ( void )
{
	HELP_DATA * help;

	if ( help_free )
	{
		help		= help_free;
		help_free	= help_free->next;
	}
	else
		help		= alloc_perm( sizeof( *help ) );

	return help;
}

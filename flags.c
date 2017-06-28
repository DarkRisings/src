#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "tables.h"

extern int flag_lookup( const char* name, const struct flag_type* flag_table );

void print_flag_syntax( CHAR_DATA* ch )
{
  if( ch != NULL )
    {
      send_to_char( "\n\rSyntax:\n\r", ch );
      send_to_char( "  flag mob  <name> <field> <flags>\n\r", ch );
      send_to_char( "  flag char <name> <field> <flags>\n\r", ch );
      send_to_char( "  mob  flags: aff, aff2, aff3, imm, res, vuln, act, form, off, part\n\r", ch );
      send_to_char( "  char flags: aff, aff2, aff3, imm, res, vuln, plr, comm\n\r", ch );
      send_to_char( "\n\r flag behaves as a toggle.\n\r", ch );
    }
}  



void do_flag( CHAR_DATA* ch, char* argument )
{
  char entity_type[ MAX_INPUT_LENGTH ];
  char entity_name[ MAX_INPUT_LENGTH ];
  char entity_field[ MAX_INPUT_LENGTH ];
  char name[ MAX_INPUT_LENGTH ];
  CHAR_DATA* victim;
  int i;
  int table_index = 0;
  long* ch_flags;
  const struct flag_type* flag_table;

  argument = one_argument( argument, entity_type );
  argument = one_argument( argument, entity_name );
  argument = one_argument( argument, entity_field );

  if( *entity_type == '\0' )
    {
      print_flag_syntax( ch );
      return;
    }

  if( *entity_name == '\0' )
    {
      send_to_char( "Who?\n\r", ch );
      return;
    }

  if( *entity_field == '\0' )
    {
      send_to_char( "Which field do you wish to change?\n\r", ch );
      return;
    }


  if( !str_prefix( entity_type, "mob" ) || !str_prefix( entity_type, "char" ) )
    {
      victim = get_char_world( ch, entity_name, FALSE );

      if( victim == NULL )
	{
	  send_to_char( "You can't find them.\n\r", ch );
	  return;
	}

      if( !str_prefix( entity_field, "aff" )  )
	{
	  ch_flags = &victim->affected_by;
	  flag_table= affect_flags;
	}
      else if( !str_prefix( entity_field, "aff2" ) )
	{
	  ch_flags = &victim->affected_by2;
	  flag_table = affect2_flags;
	}
      else if( !str_prefix( entity_field, "aff3" ) )
	{
	  ch_flags = &victim->affected_by3;
	  flag_table = affect3_flags;
	}
      else if( !str_prefix( entity_field, "imm" ) )
	{
	  ch_flags = &victim->imm_flags;
	  flag_table = imm_flags;
	}
      else if( !str_prefix( entity_field, "res" ) )
	{
	  ch_flags = &victim->res_flags;
	  flag_table = imm_flags;
	}
      else if( !str_prefix( entity_field, "vuln" ) )
	{
	  ch_flags = &victim->vuln_flags;
	  flag_table = imm_flags;
	}
      else
	{
	  if( IS_NPC( victim ) )
	    {
	      if( !str_prefix( entity_field, "act" ) )
		{
		  ch_flags = &victim->act_bits;
		  flag_table = act_flags;
		}
	      else if( !str_prefix( entity_field, "form" ) )
		{
		  ch_flags = &victim->form;
		  flag_table = form_flags;
		}
	      else if( !str_prefix( entity_field, "off" ) )
		{
		  ch_flags = &victim->off_flags;
		  flag_table = off_flags;
		}
	      else if( !str_prefix( entity_field, "part" ) )
		{
		  ch_flags = &victim->parts;
		  flag_table = part_flags;
		}
	      else
		{
		  send_to_char( "Not on NPCs.\n\r", ch );
		  return;
		}
	    }
	  else /* PC-only flags */
	    {
	      if( !str_prefix( entity_field, "plr" ) )
		{
		  ch_flags =  &victim->act_bits;
		  flag_table = plr_flags;
		}
	      else if( !str_prefix( entity_field, "comm" ) )
		{
		  ch_flags = &victim->comm;
		  flag_table = comm_flags;
		}
	      else
		{
		  send_to_char( "Not on PCs.\n\r", ch );
		  return;
		}
	    }
	}

      do
	{
	  argument = one_argument( argument, name );

	  if( *name == '\0' )
	    {
	      send_to_char( "Available flags:\n\r", ch );
	      
	      for( i = 0; flag_table[ i ].name != NULL; i++ )
		{
		  if( flag_table[ i ].settable )
		    {
		      printf_to_char( ch, "  %s\n\r", flag_table[ i ].name );
		    }
		}
	    }
	  else
	    {
	      i = flag_lookup( name, flag_table );

	      if( i ==  -99 )
		{
		  printf_to_char( ch, "%s does not exist for that field.\n\r", name );
		}
	      else
		{
		  table_index = 0;
		  while( flag_table[ table_index ].name != NULL &&
			 flag_table[ table_index ].bit != i )
		    table_index++;

		  if( flag_table[ table_index ].settable )
		    {
		      if( IS_SET( *ch_flags, flag_table[ table_index ].bit ) )
			{
			  printf_to_char( ch, "Turning off %s.\n\r", flag_table[ table_index ].name );
			  REMOVE_BIT( *ch_flags, flag_table[ table_index ].bit );
			}
		      else
			{
			  printf_to_char( ch, "Turning on %s.\n\r", flag_table[ table_index ].name );
			  SET_BIT( *ch_flags, flag_table[ table_index ].bit );
			}
		    }
		  else
		    {
		      printf_to_char( ch, "You cannot set %s with this command.\n\r", flag_table[ table_index ].name );
		    }
		}
	    }
	}
      while( *argument != '\0' );

   }
}



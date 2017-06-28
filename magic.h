/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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


/*
 * Spell functions.
 * Defined in magic.c.
 */

#define IS_WIZI_AREA(ch,vch)  (get_trust(vch) >= LEVEL_IMMORTAL && \
             (vch->invis_level > ch->level || vch->rinvis_level > ch->level))

/* gkl spells */
DECLARE_SPELL_FUN(      spell_dark_grimoire     );
DECLARE_SPELL_FUN(      spell_white_grimoire    );
DECLARE_SPELL_FUN(      spell_forget            );
DECLARE_SPELL_FUN(      spell_fish              );
DECLARE_SPELL_FUN(      spell_cyclone           );
DECLARE_SPELL_FUN(      spell_immolation        );
DECLARE_SPELL_FUN(      spell_sanguination      );
DECLARE_SPELL_FUN(      spell_infusion          );
DECLARE_SPELL_FUN(      spell_dark_risings      );
DECLARE_SPELL_FUN(      spell_gypsy_curse       );
DECLARE_SPELL_FUN(      spell_festive_spirit    );
DECLARE_SPELL_FUN(      spell_spear             );
DECLARE_SPELL_FUN(      spell_judgment          );
DECLARE_SPELL_FUN(      spell_harrow            );
DECLARE_SPELL_FUN(      spell_inquisition       );
DECLARE_SPELL_FUN(      spell_green_thumb       );
DECLARE_SPELL_FUN(      spell_familiar_link     );
DECLARE_SPELL_FUN(      spell_conjure_familiar  );
DECLARE_SPELL_FUN(      spell_life_transfer		);
DECLARE_SPELL_FUN(      spell_mana_sear  		);
DECLARE_SPELL_FUN(      spell_fortune           );
DECLARE_SPELL_FUN(      spell_haunt             );
DECLARE_SPELL_FUN(      spell_pestilence        );
DECLARE_SPELL_FUN(      spell_somnambulance     );
DECLARE_SPELL_FUN(      spell_conjure_bats      );
DECLARE_SPELL_FUN(      spell_wyrm_venom        );
DECLARE_SPELL_FUN(      spell_sun_skeletals     );
DECLARE_SPELL_FUN(      spell_natures_pestil    );
DECLARE_SPELL_FUN(      spell_abjuration        );
DECLARE_SPELL_FUN(      spell_famine            );
DECLARE_SPELL_FUN(      spell_aura_pain         );
DECLARE_SPELL_FUN(      spell_profane_word      );
DECLARE_SPELL_FUN(      spell_contrition        );

/*            */
DECLARE_SPELL_FUN(      spell_summonmeerkats    );
DECLARE_SPELL_FUN(      spell_unholyfire        );
DECLARE_SPELL_FUN(      spell_epiclesis         );
DECLARE_SPELL_FUN(      spell_bolas             );
DECLARE_SPELL_FUN(      spell_pinch             );
DECLARE_SPELL_FUN(      spell_wild_dispel 	);
DECLARE_SPELL_FUN(      spell_wildsummon	);
DECLARE_SPELL_FUN(      spell_vanish		);
DECLARE_SPELL_FUN(      spell_supermob		);
DECLARE_SPELL_FUN(      spell_bloodrush		);
DECLARE_SPELL_FUN(      spell_peach		);
DECLARE_SPELL_FUN(      spell_cheese		);
DECLARE_SPELL_FUN(      spell_banana		);
DECLARE_SPELL_FUN(      spell_sour		);
DECLARE_SPELL_FUN(      spell_carnivore		);
DECLARE_SPELL_FUN(      spell_chocalate		);
DECLARE_SPELL_FUN(      spell_apple		);
DECLARE_SPELL_FUN(      spell_plum		);
DECLARE_SPELL_FUN(      spell_strawberry	);
DECLARE_SPELL_FUN(      spell_devotion		);
DECLARE_SPELL_FUN(      spell_ordain		);
DECLARE_SPELL_FUN(      spell_veil              );
/*DECLARE_SPELL_FUN(      spell_reckoning		);*/
DECLARE_SPELL_FUN(      spell_scry		);
DECLARE_SPELL_FUN(      spell_lifetap		);
DECLARE_SPELL_FUN(      spell_life_transfer		);
DECLARE_SPELL_FUN(      spell_blister		);
DECLARE_SPELL_FUN(      spell_headache		);
DECLARE_SPELL_FUN(      spell_shadow_step	);
DECLARE_SPELL_FUN(      spell_animate_dead	);
DECLARE_SPELL_FUN(      spell_restore_limb	);
DECLARE_SPELL_FUN(      spell_wither_limb	);
DECLARE_SPELL_FUN(      spell_atrophy		);
DECLARE_SPELL_FUN(      spell_wilt		);
DECLARE_SPELL_FUN(      spell_decay		);
DECLARE_SPELL_FUN(	spell_null		);
DECLARE_SPELL_FUN(	spell_magicward	);
DECLARE_SPELL_FUN(	spell_confusion	);
DECLARE_SPELL_FUN(	spell_divine_essence    );
DECLARE_SPELL_FUN(	spell_klaive	);
DECLARE_SPELL_FUN(	spell_summon_animal	);
DECLARE_SPELL_FUN(	spell_summon_sick	);
DECLARE_SPELL_FUN(	spell_chaosdemon	);
DECLARE_SPELL_FUN(	spell_wildfire	);
DECLARE_SPELL_FUN(	spell_wildaura	);
DECLARE_SPELL_FUN(	spell_wildshield	);
DECLARE_SPELL_FUN(	spell_wildheal);
DECLARE_SPELL_FUN(	spell_wildenhance	);
DECLARE_SPELL_FUN(	spell_acid_blast	);
DECLARE_SPELL_FUN(	spell_resurrect	);
DECLARE_SPELL_FUN(	spell_web		);
DECLARE_SPELL_FUN(	spell_armor		);
DECLARE_SPELL_FUN(	spell_mirror		);
DECLARE_SPELL_FUN(	spell_skulls		);
DECLARE_SPELL_FUN(	spell_bless		);
DECLARE_SPELL_FUN(	spell_blindness		);
DECLARE_SPELL_FUN(	spell_burning_hands	);
DECLARE_SPELL_FUN(	spell_call_lightning	);
DECLARE_SPELL_FUN(      spell_calm		);
DECLARE_SPELL_FUN(      spell_cancellation	);
DECLARE_SPELL_FUN(	spell_change_sex	);
DECLARE_SPELL_FUN(      spell_chain_lightning   );
DECLARE_SPELL_FUN(	spell_charm_person	);
DECLARE_SPELL_FUN(	spell_chill_touch	);
DECLARE_SPELL_FUN(	spell_circle_of_protection);
DECLARE_SPELL_FUN(	spell_colour_spray	);
DECLARE_SPELL_FUN(	spell_continual_light	);
DECLARE_SPELL_FUN(	spell_control_weather	);
DECLARE_SPELL_FUN(	spell_create_food	);
DECLARE_SPELL_FUN(	spell_create_rose	);
DECLARE_SPELL_FUN(	spell_create_spring	);
DECLARE_SPELL_FUN(	spell_create_water	);
DECLARE_SPELL_FUN(	spell_cure_blindness	);
DECLARE_SPELL_FUN(	spell_cure_critical	);
DECLARE_SPELL_FUN(      spell_cure_disease	);
DECLARE_SPELL_FUN(	spell_cure_light	);
DECLARE_SPELL_FUN(	spell_cure_poison	);
DECLARE_SPELL_FUN(	spell_cure_serious	);
DECLARE_SPELL_FUN(	spell_curse		);
DECLARE_SPELL_FUN(      spell_smite		);
DECLARE_SPELL_FUN(	spell_detect_hidden	);
DECLARE_SPELL_FUN(	spell_detect_invis	);
DECLARE_SPELL_FUN(	spell_detect_magic	);
DECLARE_SPELL_FUN(	spell_detect_poison	);
DECLARE_SPELL_FUN(	spell_detect_were	);
DECLARE_SPELL_FUN(	spell_dispel_magic	);
DECLARE_SPELL_FUN(	spell_earthquake	);
DECLARE_SPELL_FUN(	spell_enchant_armor	);
DECLARE_SPELL_FUN(	spell_enchant_weapon	);
DECLARE_SPELL_FUN(	spell_energy_drain	);
DECLARE_SPELL_FUN(	spell_entrance		);
DECLARE_SPELL_FUN(	spell_evil		);
DECLARE_SPELL_FUN(	spell_faerie_fire	);
DECLARE_SPELL_FUN(	spell_faerie_fog	);
DECLARE_SPELL_FUN(	spell_farsight		);
DECLARE_SPELL_FUN(	spell_fireball		);
DECLARE_SPELL_FUN(	spell_fireproof		);
DECLARE_SPELL_FUN(	spell_flamestrike	);
DECLARE_SPELL_FUN(	spell_floating_disc	);
DECLARE_SPELL_FUN(	spell_fly		);
DECLARE_SPELL_FUN(      spell_frenzy		);
DECLARE_SPELL_FUN(	spell_gate		);
DECLARE_SPELL_FUN(	spell_giant_strength	);
DECLARE_SPELL_FUN(      spell_haste		);
DECLARE_SPELL_FUN(	spell_heal		);
DECLARE_SPELL_FUN(      spell_holy_word		);
DECLARE_SPELL_FUN(	spell_identify		);
DECLARE_SPELL_FUN(	spell_infravision	);
DECLARE_SPELL_FUN(	spell_invis		);
DECLARE_SPELL_FUN(	spell_know_alignment	);
DECLARE_SPELL_FUN(	spell_lightning_bolt	);
DECLARE_SPELL_FUN(	spell_locate_object	);
DECLARE_SPELL_FUN(	spell_locate_person	);
DECLARE_SPELL_FUN(	spell_magic_missile	);
DECLARE_SPELL_FUN(      spell_mass_healing	);
DECLARE_SPELL_FUN(	spell_mass_invis	);
DECLARE_SPELL_FUN(	spell_nexus		);
DECLARE_SPELL_FUN(	spell_pass_door		);
DECLARE_SPELL_FUN(      spell_plague		);
DECLARE_SPELL_FUN(	spell_poison		);
DECLARE_SPELL_FUN(	spell_portal		);
DECLARE_SPELL_FUN(	spell_ray       	);
DECLARE_SPELL_FUN(	spell_recharge		);
DECLARE_SPELL_FUN(	spell_refresh		);
DECLARE_SPELL_FUN(	spell_reflect		);
DECLARE_SPELL_FUN(	spell_remove_curse	);
DECLARE_SPELL_FUN(	spell_sanctuary		);
DECLARE_SPELL_FUN(	spell_shadow		);
DECLARE_SPELL_FUN(	spell_shadows		);
DECLARE_SPELL_FUN(	spell_shocking_grasp	);
DECLARE_SPELL_FUN(	spell_shield		);
DECLARE_SPELL_FUN(	spell_sleep		);
DECLARE_SPELL_FUN(	spell_slow		);
DECLARE_SPELL_FUN(	spell_stone_skin	);
DECLARE_SPELL_FUN(	spell_summon		);
DECLARE_SPELL_FUN(	spell_teleport		);
DECLARE_SPELL_FUN(	spell_terror		);
DECLARE_SPELL_FUN(	spell_ventriloquate	);
DECLARE_SPELL_FUN(	spell_weaken		);
DECLARE_SPELL_FUN(	spell_word_of_recall	);
DECLARE_SPELL_FUN(	spell_acid_breath	);
DECLARE_SPELL_FUN(	spell_fire_breath	);
DECLARE_SPELL_FUN(	spell_frost_breath	);
DECLARE_SPELL_FUN(	spell_gas_breath	);
DECLARE_SPELL_FUN(	spell_lightning_breath	);
DECLARE_SPELL_FUN(	spell_general_purpose	);
DECLARE_SPELL_FUN(	spell_high_explosive	);
DECLARE_SPELL_FUN(      spell_blood_cool        );
/*psi spells*/
DECLARE_SPELL_FUN(      spell_awe               );
DECLARE_SPELL_FUN(      spell_displace          );
DECLARE_SPELL_FUN(      spell_steel_skin        );
DECLARE_SPELL_FUN(      spell_mental_barrier    );
DECLARE_SPELL_FUN(      spell_biofeedback       );
DECLARE_SPELL_FUN(      spell_enhanced_strength );
DECLARE_SPELL_FUN(      spell_levitation        );
DECLARE_SPELL_FUN(      spell_adrenaline_rush   );
DECLARE_SPELL_FUN(	spell_psionic_blast	);
DECLARE_SPELL_FUN(	spell_mind_blast	);
DECLARE_SPELL_FUN(	spell_infliction	);
DECLARE_SPELL_FUN(	spell_psychic_crush	);
DECLARE_SPELL_FUN(      spell_psychic_drain     );
DECLARE_SPELL_FUN(      spell_warp_time         );
DECLARE_SPELL_FUN(      spell_mind_whip         );
DECLARE_SPELL_FUN(      spell_domination        );
DECLARE_SPELL_FUN(      spell_psychic_heal      );
DECLARE_SPELL_FUN(      spell_major_heal        );
DECLARE_SPELL_FUN(	spell_cellular_purge	);
DECLARE_SPELL_FUN(      spell_astral_projection );
DECLARE_SPELL_FUN(      spell_mental_fortress   );
/* bard spells */
DECLARE_SPELL_FUN(      spell_battlecry         );
DECLARE_SPELL_FUN(      spell_battle_wail       );
DECLARE_SPELL_FUN(      spell_deathsong         );
DECLARE_SPELL_FUN(      spell_battle_sorrow     );
DECLARE_SPELL_FUN(	spell_soothe		);
DECLARE_SPELL_FUN(	spell_echoes		);
/*druid spells*/
DECLARE_SPELL_FUN(      spell_natures_wrath     );
/*new mage spells*/
DECLARE_SPELL_FUN(      spell_water_bolt        );
DECLARE_SPELL_FUN(      spell_flashfire         );
DECLARE_SPELL_FUN(      spell_thunderclap       );
DECLARE_SPELL_FUN(      spell_winter_blast      );
DECLARE_SPELL_FUN(      spell_meteor            );
DECLARE_SPELL_FUN(      spell_field             );
DECLARE_SPELL_FUN(      spell_wrath             );
/*werespells*/
DECLARE_SPELL_FUN(      spell_blade             );
DECLARE_SPELL_FUN(      spell_psiblade          );
DECLARE_SPELL_FUN(      spell_hammer            );
DECLARE_SPELL_FUN(      spell_call_spiders      );
DECLARE_SPELL_FUN(      spell_call_water_spirit );
DECLARE_SPELL_FUN(       spell_fear             );
/*vamp spells*/
DECLARE_SPELL_FUN(      spell_bboil             );
DECLARE_SPELL_FUN(      spell_blade2            );
DECLARE_SPELL_FUN(      spell_brimstone         );
DECLARE_SPELL_FUN(      spell_celerity          );
DECLARE_SPELL_FUN(      spell_chimestry         );
DECLARE_SPELL_FUN(      spell_interred          );
DECLARE_SPELL_FUN(      spell_skeletons         );
DECLARE_SPELL_FUN(      spell_focus             );
DECLARE_SPELL_FUN(      spell_silence           );
/*guild spells*/
DECLARE_SPELL_FUN(      spell_guildspell        );
DECLARE_SPELL_FUN(      spell_kit               );
DECLARE_SPELL_FUN(      spell_key               );
DECLARE_SPELL_FUN(	spell_smoke		);
DECLARE_SPELL_FUN(      spell_twrath            );
DECLARE_SPELL_FUN(      spell_wanted            );
/*warlock spells*/
DECLARE_SPELL_FUN(      spell_sorrow            );
DECLARE_SPELL_FUN(      spell_chaos             );
DECLARE_SPELL_FUN(      spell_torment           );
DECLARE_SPELL_FUN(      spell_ravage            );
DECLARE_SPELL_FUN(      spell_betray            );
/*avatar/exalted spells*/
DECLARE_SPELL_FUN(      spell_animae            );
/*Seraph Spells*/
DECLARE_SPELL_FUN(      spell_angelic_lance     );
DECLARE_SPELL_FUN(      spell_godspeed          );
DECLARE_SPELL_FUN(      spell_tabula_rasa       );
DECLARE_SPELL_FUN(      spell_hallowed_discord       );
/*vorpal*/
DECLARE_SPELL_FUN(      spell_vorpalspell         );

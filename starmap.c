#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"

#define NUM_DAYS 35
/* Match this to the number of days per month; this is the moon cycle */
#define NUM_MONTHS 17
/* Match this to the number of months defined in month_name[].  */
#define MAP_WIDTH 72
#define SHOW_WIDTH MAP_WIDTH/2
#define MAP_HEIGHT 9
/* Should be the string length and number of the constants below.*/
const char * star_map[] =
{
"   W.N     ' .     :. M,N     :  y:.,N    `  ,       B,N      .      .  ",
" W. :.N .      G,N  :M.: .N  :` y.N    .      :     B:   .N       :     ",
"    W:N    G.N:       M:.,N:.:   y`N      ,    c.N           .:    `    ",
"   W.`:N       '. G.N  `  : ::.      y.N      c'N      B.N R., ,N       ",
" W:'  `:N .  G. N    `  :    .y.N:.          ,     B.N      :  R:   . .N",
":' '.   .    G:.N      .'   '   :::.  ,  c.N   :c.N    `        R`.N    ",
"      :       `        `        :. ::. :     '  :        ,   , R.`:N    ",
"  ,       G:.N              `y.N :. ::.c`N      c`.N   '        `      .",
"     ..        G.:N :           .:   c.N:.    .              .          "
};

/***************************CONSTELLATIONS*******************************
  Lupus     Gigas      Pyx      Enigma   Centaurus    Terken    Raptus
   The       The       The       The       The         The       The  
White Wolf  Giant     Pixie     Sphinx    Centaur      Drow     Raptor
*************************************************************************/      
const char * sun_map[] =
{
"\\'|'/",
"- O -",
"/.|.\\"
};
const char * moon_map[] =
{
" @@@ ",
"@@@@@",
" @@@ "
};

void look_sky ( CHAR_DATA * ch )
{
    static char buf[MAX_STRING_LENGTH];
    static char buf2[4];
    int starpos, sunpos, moonpos, moonphase, i, linenum;
#ifdef DEBUG
        Debug ("look_sky");
#endif

   send_to_char("You gaze up towards the heavens and see:\n\r",ch);

   sunpos  = (MAP_WIDTH * (24 - time_info.hour) / 24);
   moonpos = (sunpos + time_info.day * MAP_WIDTH / NUM_DAYS) % MAP_WIDTH;
   if ((moonphase = ((((MAP_WIDTH + moonpos - sunpos ) % MAP_WIDTH ) +
                      (MAP_WIDTH/16)) * 8 ) / MAP_WIDTH)
                         > 4) moonphase -= 8;
   starpos = (sunpos + MAP_WIDTH * time_info.month / NUM_MONTHS) % MAP_WIDTH;
   /* The left end of the star_map will be straight overhead at midnight during 
      month 0 */

   for ( linenum = 0; linenum < MAP_HEIGHT; linenum++ )
   {
     if ((time_info.hour >= 6 && time_info.hour <= 18) &&
         (linenum < 3 || linenum >= 6))
       continue;
     sprintf(buf,"{W|{x");
     for ( i = MAP_WIDTH/4; i <= 3*MAP_WIDTH/4; i++)
     {
       /* plot moon on top of anything else...unless new moon & no eclipse */
       if ((time_info.hour >= 6 && time_info.hour <= 18)  /* daytime? */
        && (moonpos >= MAP_WIDTH/4 - 2) && (moonpos <= 3*MAP_WIDTH/4 + 2) /* in sky? */
        && ( i >= moonpos - 2 ) && (i <= moonpos + 2) /* is this pixel near moon? */
        && ((sunpos == moonpos && time_info.hour == 12) || moonphase != 0  ) /*no eclipse*/
        && (moon_map[linenum-3][i+2-moonpos] == '@'))
       {
         if ((moonphase < 0 && i - 2 - moonpos >= moonphase) ||
             (moonphase > 0 && i + 2 - moonpos <= moonphase))
           strcat(buf,"{W@");
         else
           strcat(buf," ");
       }
       else
       if ((linenum >= 3) && (linenum < 6) && /* nighttime */
           (moonpos >= MAP_WIDTH/4 - 2) && (moonpos <= 3*MAP_WIDTH/4 + 2) /* in sky? */
        && ( i >= moonpos - 2 ) && (i <= moonpos + 2) /* is this pixel near moon? */
        && (moon_map[linenum-3][i+2-moonpos] == '@'))
       {
         if ((moonphase < 0 && i - 2 - moonpos >= moonphase) ||
             (moonphase > 0 && i + 2 - moonpos <= moonphase))
           strcat(buf,"{W@");
         else
           strcat(buf," ");
       }
       else /* plot sun or stars */
       {
         if (time_info.hour>=6 && time_info.hour<=18) /* daytime */
         {
           if ( i >= sunpos - 2 && i <= sunpos + 2 )
           {
             sprintf(buf2,"{y%c",sun_map[linenum-3][i+2-sunpos]);
             strcat(buf,buf2);
           }
           else
             strcat(buf," ");
         }
         else
         {
           switch (star_map[linenum][(MAP_WIDTH + i - starpos)%MAP_WIDTH])
           {
             default     : strcat(buf," ");    break;
             case '.'    : strcat(buf,".");    break;
             case ','    : strcat(buf,",");    break;
             case ':'    : strcat(buf,":");    break;
             case '`'    : strcat(buf,"`");    break;
             case 'R'    : strcat(buf,"{R ");  break;
             case 'G'    : strcat(buf,"{G ");  break;
             case 'B'    : strcat(buf,"{B ");  break;
             case 'W'    : strcat(buf,"{W ");  break;
             case 'M'    : strcat(buf,"{M ");  break;
             case 'N'    : strcat(buf,"{x ");  break;
             case 'y'    : strcat(buf,"{y ");  break;
             case 'c'    : strcat(buf,"{c ");  break;
           }
         }
       }
     }
     strcat(buf,"{W|{x\n\r");
     send_to_char(buf,ch);
   }
}




/******************************************************************************
 *                                                                            *
 *   Brawler scoreborad and support code                                      *
 *   Written October 2011 by Glenn K. Lockwood                                * 
 *   for the Dark Risings codebase                                            *
 *                                                                            *
 ******************************************************************************/
#define BRAWLER_FILE                    "brawlers.txt"
#define BRAWL_WINS                      0
#define BRAWL_LOSSES                    1
#define BRAWL_WIMPS                     2
#define BRAWL_UNIQUE_WINS               3
#define BRAWL_UNIQUE_LOSSES             4
#define BRAWL_UNIQUE_WIMPS              5
#define MAX_BRAWLER_SCORETYPES          6

#define FIGHT_UNKNOWN                   0
#define FIGHT_WIN                       1
#define FIGHT_LOSS                      2
#define FIGHT_WIMP                      3
#define FIGHT_DRAW                      4

#define BFLAG_HIDE                      (A)
/*
 * Global lists
 */ 
extern  BRAWLER_DATA            *brawler_list;

/*
 * Structs
 */
struct brawler_data
{
    BRAWLER_DATA *next;
    FIGHT_RECORD *fights;
    char *name;
    int score[MAX_BRAWLER_SCORETYPES];
    int flags;
    time_t lastbrawl;   /* for tracking brawler activity */
    time_t banuntil;  /* for banning brawlers from re-joining */
};

struct fight_record
{
    FIGHT_RECORD *next;
    BRAWLER_DATA *opponent;
    int outcome;
    time_t timestamp;
};

/*
 * Global routines
 */
void save_brawlers              args( ( void ) );
void load_brawler               args( ( void ) );
BRAWLER_DATA *get_brawler       args( ( char *name ) );
void brawler_wimp( BRAWLER_DATA *wimp, BRAWLER_DATA *opponent);

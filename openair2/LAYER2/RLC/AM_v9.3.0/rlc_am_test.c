#define RLC_AM_MODULE
#define RLC_AM_TEST_C
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <sys/time.h>
#include <curses.h>


#include "rtos_header.h"
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "rlc.h"
#include "rlc_am.h"
#include "list.h"
#include "LAYER2/MAC/extern.h"






int print();
void rlc_window(WINDOW *win, int starty, int startx, int lines, int cols,
       int tile_width, int tile_height);


//-----------------------------------------------------------------------------
int print(void)
//-----------------------------------------------------------------------------
{   int count;
    int y = 2, x = 2, w = 4, h = 2;
    static int solution = 1;

    mvprintw(0, 0, "AM 1: VT(A) %04d VT(S) %04d VR(R) %04d VR(H) %04d", am_tx.vt_a, am_tx.vt_s, am_tx.vr_r, am_tx.vr_h);
    mvprintw(2, 0, "AM 2: VT(A) %04d VT(S) %04d VR(R) %04d VR(H) %04d", am_rx.vt_a, am_rx.vt_s, am_rx.vr_r, am_rx.vr_h);
    wrefresh(stdscr);
    /*rlc_window(stdscr, y, x, 2, 32, w, h);
    for(count = 1; count <= 1024; ++count)
    {   int tempy = y + (count - 1) * h + h / 2;
        int tempx = x + (count - 1) * w + w / 2;
        mvaddch(tempy, tempx, QUEEN_CHAR);
    }*/
    refresh();
    mvprintw(LINES - 2, 0, "Press Any Key to See next solution (F1 to Exit)");
}

//-----------------------------------------------------------------------------
void rlc_window(WINDOW *win, int starty, int startx, int lines, int cols, int tile_width, int tile_height)
//-----------------------------------------------------------------------------
{   int endy, endx, i, j;

    endy = starty + lines * tile_height;
    endx = startx + cols  * tile_width;

    for(j = starty; j <= endy; j += tile_height)
        for(i = startx; i <= endx; ++i)
            mvwaddch(win, j, i, ACS_HLINE);
    for(i = startx; i <= endx; i += tile_width)
        for(j = starty; j <= endy; ++j)
            mvwaddch(win, j, i, ACS_VLINE);
    mvwaddch(win, starty, startx, ACS_ULCORNER);
    mvwaddch(win, endy, startx, ACS_LLCORNER);
    mvwaddch(win, starty, endx, ACS_URCORNER);
    mvwaddch(win,   endy, endx, ACS_LRCORNER);
    for(j = starty + tile_height; j <= endy - tile_height; j += tile_height)
    {   mvwaddch(win, j, startx, ACS_LTEE);
        mvwaddch(win, j, endx, ACS_RTEE);
        for(i = startx + tile_width; i <= endx - tile_width; i += tile_width)
            mvwaddch(win, j, i, ACS_PLUS);
    }
    for(i = startx + tile_width; i <= endx - tile_width; i += tile_width)
    {   mvwaddch(win, starty, i, ACS_TTEE);
        mvwaddch(win, endy, i, ACS_BTEE);
    }
    wrefresh(win);
}







#define TEST1
#define TEST2
#define TEST3
#define TEST4
#define TEST5
#define TEST6
#define TEST7

#define TEST_MAX_SEND_SDU 8192
#define TARGET_MAX_RX_ERROR_RATE 10
#define TARGET_MAX_TX_ERROR_RATE 10
static int  random_sdu;
static int  random_nb_frames;
static int  random_tx_pdu_size;
static int  random_rx_pdu_size;
static int  target_tx_error_rate;
static int  target_rx_error_rate;
static int  tx_packets = 0;
static int  dropped_tx_packets = 0;
static int  rx_packets = 0;
static int  dropped_rx_packets = 0;
static int  drop_rx = 0;
static int  drop_tx = 0;
static int  mui = 0;
static int  send_sdu_ids[TEST_MAX_SEND_SDU][2];
static int  send_id_write_index[2];
static int  send_id_read_index[2];
static u8_t buffer[32];
static s8_t *sdus[] = {"En dépit de son volontarisme affiché, le premier ministre est de plus en plus décrié pour son incompétence. La tension politique et dans l'opinion publique est encore montée d'un cran au Japon, sur fond d'inquiétantes nouvelles, avec du plutonium détecté dans le sol autour de la centrale de Fukushima. Le premier ministre Naoto Kan a solennellement déclaré que son gouvernement était «en état d'alerte maximum». Tout en reconnaissant que la situation restait «imprévisible». Ce volontarisme affiché par le premier ministre - que Nicolas Sarkozy rencontrera demain lors d'une visite au Japon - ne l'a pas empêché d'être la cible de violentes critiques de la part de parlementaires sur sa gestion de la crise. Attaqué sur le manque de transparence, il a assuré qu'il rendait publiques toutes les informations en sa possession. Un député de l'opposition, Yosuke Isozaki, a aussi reproché à Naoto Kan de ne pas avoir ordonné l'évacuation des populations dans la zone comprise entre 20 et 30 km autour de la centrale. «Peut-il y avoir quelque chose de plus irresponsable que cela ?», a-t-il lancé. Pour l'heure, la zone d'évacuation est limitée à un rayon de 20 km, seul le confinement étant recommandé pour les 10 km suivants. Sur ce sujet, les autorités japonaises ont été fragilisées mardi par les déclarations de Greenpeace, affirmant que ses experts avaient détecté une radioactivité dangereuse à 40 km de la centrale. L'organisation écologiste a appelé à une extension de la zone d'évacuation, exhortant Tokyo à «cesser de privilégier la politique aux dépens de la science». L'Agence japonaise de sûreté nucléaire a balayé ces critiques.",

"La pâquerette (Bellis perennis) est une plante vivace des prés, des pelouses, des bords de chemins et des prairies, haute de dix à vingt centimètres, de la famille des Astéracées, dont les fleurs naissent sur des inflorescences appelées capitules : celles du pourtour, que l'on croit à tort être des pétales, appelées fleurs ligulées, parce qu'elles ont la forme d'une languette, ou demi-fleurons, sont des fleurs femelles, dont la couleur varie du blanc au rose plus ou moins prononcé ; celles du centre, jaunes, appelées fleurs tubuleuses, parce que leur corolle forme un tube, ou fleurons, sont hermaphrodites. Ainsi, contrairement à l'opinion populaire, ce qu'on appelle une « fleur » de pâquerette n'est en réalité pas « une » fleur mais un capitule portant des fleurs très nombreuses.Leurs fruits s'envolent grâce au vent et dégagent des odeurs qui attirent les insectes.Une variété muricole peut pousser sur des murs humides verticaux.Les pâquerettes sont des fleurs rustiques et très communes en Europe, sur les gazons, les prairies, les chemins et les zones d'herbe rase.Elles ont la particularité, comme certaines autres fleurs de plantes herbacées, de se fermer la nuit et de s'ouvrir le matin pour s'épanouir au soleil ; elles peuvent aussi se fermer pendant les averses, voire un peu avant, ce qui permet dans les campagnes de prédire la pluie légèrement à l'avance.",

"La pâquerette",
" (Bellis perennis)",
" est une plante vivace des prés,",
" des pelouses,",
" des bords de chemins et des prairies,",
"haute de dix à",
" vingt centimètres",
", de la",
" famille des",
" Astéracées",
", dont",
" les",
" fleurs",
" naissent",
" sur",
" des",
" inflorescences",
" appelées",
" capitules",
" : celles",
" du pourtour",
", que l'on",
" croit à",
" tort",
" être",
" des pétales",
", appelées",
" fleurs ligulées",
", parce qu'elles",
" ont la forme d'une languette, ou demi-fleurons, sont",
" des fleurs femelles,",
" dont la couleur varie du blanc au rose plus ou moins prononcé ; celles du centre, jaunes,",
" appelées ",
"fleurs tubuleuses",
", parce que leur corolle forme un tube, ou fleurons, sont hermaphrodites."
};

/*  Returns the x-y size of the terminal  */

//-----------------------------------------------------------------------------
void rlc_am_v9_3_0_test_windows()
//-----------------------------------------------------------------------------
{
   rlc_am_entity_t am1;
   rlc_am_entity_t am2;
   unsigned int    i;
   unsigned int    j;

   rlc_am_init(&am1);
   rlc_am_init(&am2);

   // TX window with vt_ms > vt_a
   for (j = 0; j < RLC_AM_SN_MODULO-RLC_AM_WINDOW_SIZE; j++) {
       am1.vt_a = j;
       am1.vt_s = j;
       am1.vt_ms = (am1.vt_a + RLC_AM_WINDOW_SIZE) & RLC_AM_SN_MASK;
       for (i = 0 ; i < am1.vt_a; i ++) {
           assert(rlc_am_in_tx_window(&am1, i) == 0);
       }
       for (i = j ; i < am1.vt_ms; i ++) {
           assert(rlc_am_in_tx_window(&am1, i) > 0);
       }
       for (i = am1.vt_ms ; i <= 0xFFFF; i ++) {
           //printf("assert(rlc_am_in_tx_window(&am1, %d) == 0)) vt(s)=%d vt(ms)=%d\n", i, am1.vt_s, am1.vt_ms);
           assert(rlc_am_in_tx_window(&am1, i) == 0);
       }
   }
   // TX window with vt_ms < vt_a
   for (j = RLC_AM_SN_MODULO-RLC_AM_WINDOW_SIZE; j < RLC_AM_SN_MODULO; j++) {
       am1.vt_a = j;
       am1.vt_s = j;
       am1.vt_ms = (am1.vt_a + RLC_AM_WINDOW_SIZE) & RLC_AM_SN_MASK;
       for (i = 0 ; i < am1.vt_ms; i ++) {
           assert(rlc_am_in_tx_window(&am1, i) > 0);
       }
       for (i = j ; i < am1.vt_a; i ++) {
           assert(rlc_am_in_tx_window(&am1, i) == 0);
       }
       for (i = am1.vt_a ; i < RLC_AM_SN_MODULO; i ++) {
           assert(rlc_am_in_tx_window(&am1, i) > 0);
       }
       for (i = RLC_AM_SN_MODULO ; i < 0xFFFF; i ++) {
           assert(rlc_am_in_tx_window(&am1, i) == 0);
       }
   }

   // RX window with vr_mr > vr_r
   for (j = 0; j < RLC_AM_SN_MODULO-RLC_AM_WINDOW_SIZE; j++) {
       am1.vr_r = j;
       am1.vr_mr = (am1.vr_r + RLC_AM_WINDOW_SIZE) & RLC_AM_SN_MASK;
       for (i = 0 ; i < am1.vr_r; i ++) {
           assert(rlc_am_in_rx_window(&am1, i) == 0);
       }
       for (i = j ; i < am1.vr_mr; i ++) {
           //printf("assert(rlc_am_in_rx_window(&am1, %d) == 0)) vr(r)=%d vr(mr)=%d\n", i, am1.vr_r, am1.vr_mr);
           assert(rlc_am_in_rx_window(&am1, i) > 0);
       }
       for (i = am1.vr_mr ; i <= 0xFFFF; i ++) {
           assert(rlc_am_in_rx_window(&am1, i) == 0);
       }
   }
   // RX window with vr_mr < vr_r
   for (j = RLC_AM_SN_MODULO-RLC_AM_WINDOW_SIZE; j < RLC_AM_SN_MODULO; j++) {
       am1.vr_r = j;
       am1.vr_mr = (am1.vr_r + RLC_AM_WINDOW_SIZE) & RLC_AM_SN_MASK;
       for (i = 0 ; i < am1.vr_mr; i ++) {
           assert(rlc_am_in_rx_window(&am1, i) > 0);
       }
       for (i = j ; i < am1.vr_r; i ++) {
           assert(rlc_am_in_rx_window(&am1, i) == 0);
       }
       for (i = am1.vr_r ; i < RLC_AM_SN_MODULO; i ++) {
           assert(rlc_am_in_rx_window(&am1, i) > 0);
       }
       for (i = RLC_AM_SN_MODULO ; i < 0xFFFF; i ++) {
           assert(rlc_am_in_rx_window(&am1, i) == 0);
       }
   }

}
//-----------------------------------------------------------------------------
void rlc_am_v9_3_0_test_read_write_bit_field()
//-----------------------------------------------------------------------------
{
  unsigned int bit_pos_write       = 0; // range from 0 (MSB/left) to 7 (LSB/right)
  u8_t*        byte_pos_write      = buffer;

  unsigned int bit_pos_read       = 0; // range from 0 (MSB/left) to 7 (LSB/right)
  u8_t*        byte_pos_read      = buffer;
  u16_t        read_value;

  memset (buffer, 0, 1024);
  // byte 0
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 1);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 1);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 1);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 1);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0);
  assert(buffer[0] == 0x96);
  assert(buffer[1] == 0x00);

  // byte 1
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 1);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 1);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 1);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 1);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 1);
  assert(buffer[0] == 0x96);

  assert(buffer[1] == 0xD9);
  assert(buffer[2] == 0x00);

  // byte 2
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 1);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 1);
  assert(buffer[0] == 0x96);
  assert(buffer[1] == 0xD9);

  assert(buffer[2] == 0x11);
  assert(buffer[3] == 0x00);
  // byte 3 & 4
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 2, 0x2);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 2, 0x1);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 2, 0x2);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0x0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 2, 0x3);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0x0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 2, 0x2);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 2, 0x1);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 2, 0x3);
  assert(buffer[0] == 0x96);
  assert(buffer[1] == 0xD9);
  assert(buffer[2] == 0x11);

  assert(buffer[3] == 0x99);
  assert(buffer[4] == 0xA7);
  assert(buffer[5] == 0x00);
  // byte 5 & 6
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 3, 0x7);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 3, 0x5);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 3, 0x1);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 3, 0x0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 3, 0x3);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0x0);
  assert(buffer[0] == 0x96);
  assert(buffer[1] == 0xD9);
  assert(buffer[2] == 0x11);
  assert(buffer[3] == 0x99);
  assert(buffer[4] == 0xA7);

  assert(buffer[5] == 0xF4);
  assert(buffer[6] == 0x86);
  assert(buffer[7] == 0x00);
  // byte 7 & 8 & 9
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 4, 0xC);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 4, 0xD);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 1, 0x0);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 4, 0xF);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 4, 0xA);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 4, 0xB);
  rlc_am_write8_bit_field(&byte_pos_write, &bit_pos_write, 3, 0x0);
  assert(buffer[0] == 0x96);
  assert(buffer[1] == 0xD9);
  assert(buffer[2] == 0x11);
  assert(buffer[3] == 0x99);
  assert(buffer[4] == 0xA7);
  assert(buffer[5] == 0xF4);
  assert(buffer[6] == 0x86);

  assert(buffer[7] == 0xCD);
  assert(buffer[8] == 0x7D);
  assert(buffer[9] == 0x58);
  assert(buffer[10] == 0x00);
  // byte 10 & 11 & 12 & 13 & 14
  rlc_am_write16_bit_field(&byte_pos_write, &bit_pos_write, 10, 0x2AB);
  rlc_am_write16_bit_field(&byte_pos_write, &bit_pos_write, 10, 0x1BA);
  rlc_am_write16_bit_field(&byte_pos_write, &bit_pos_write, 10, 0x2AF);
  rlc_am_write16_bit_field(&byte_pos_write, &bit_pos_write, 10, 0x134);
  assert(buffer[0]  == 0x96);
  assert(buffer[1]  == 0xD9);
  assert(buffer[2]  == 0x11);
  assert(buffer[3]  == 0x99);
  assert(buffer[4]  == 0xA7);
  assert(buffer[5]  == 0xF4);
  assert(buffer[6]  == 0x86);
  assert(buffer[7]  == 0xCD);
  assert(buffer[8]  == 0x7D);
  assert(buffer[9]  == 0x58);

  assert(buffer[10] == 0xAA);
  assert(buffer[11] == 0xDB);
  assert(buffer[12] == 0xAA);
  assert(buffer[13] == 0xBD);
  assert(buffer[14] == 0x34);
  assert(buffer[15] == 0x00);
  // byte 15 - 29
  rlc_am_write16_bit_field(&byte_pos_write, &bit_pos_write, 15, 0x701F);
  rlc_am_write16_bit_field(&byte_pos_write, &bit_pos_write, 15, 0x612E);
  rlc_am_write16_bit_field(&byte_pos_write, &bit_pos_write, 15, 0x523D);
  rlc_am_write16_bit_field(&byte_pos_write, &bit_pos_write, 15, 0x434C);
  rlc_am_write16_bit_field(&byte_pos_write, &bit_pos_write, 15, 0x345B);
  rlc_am_write16_bit_field(&byte_pos_write, &bit_pos_write, 15, 0x256A);
  rlc_am_write16_bit_field(&byte_pos_write, &bit_pos_write, 15, 0x1679);
  rlc_am_write16_bit_field(&byte_pos_write, &bit_pos_write, 15, 0x0788);
  assert(buffer[0]  == 0x96);
  assert(buffer[1]  == 0xD9);
  assert(buffer[2]  == 0x11);
  assert(buffer[3]  == 0x99);
  assert(buffer[4]  == 0xA7);
  assert(buffer[5]  == 0xF4);
  assert(buffer[6]  == 0x86);
  assert(buffer[7]  == 0xCD);
  assert(buffer[8]  == 0x7D);
  assert(buffer[9]  == 0x58);
  assert(buffer[10] == 0xAA);
  assert(buffer[11] == 0xDB);
  assert(buffer[12] == 0xAA);
  assert(buffer[13] == 0xBD);
  assert(buffer[14] == 0x34);

  assert(buffer[15] == 0xE0);
  assert(buffer[16] == 0x3F);
  assert(buffer[17] == 0x84);
  assert(buffer[18] == 0xBA);
  assert(buffer[19] == 0x91);
  assert(buffer[20] == 0xEC);
  assert(buffer[21] == 0x34);
  assert(buffer[22] == 0xC6);
  assert(buffer[23] == 0x8B);
  assert(buffer[24] == 0x69);
  assert(buffer[25] == 0x5A);
  assert(buffer[26] == 0x8B);
  assert(buffer[27] == 0x3C);
  assert(buffer[28] == 0x87);
  assert(buffer[29] == 0x88);
  assert(buffer[30] == 0x00);

  // 0x96 0xD9
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b0
  assert(read_value == 1);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b1
  assert(read_value == 0);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b2
  assert(read_value == 0);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b3
  assert(read_value == 1);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b4
  assert(read_value == 0);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b5
  assert(read_value == 1);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b6
  assert(read_value == 1);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b7
  assert(read_value == 0);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b8
  assert(read_value == 1);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b9
  assert(read_value == 1);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b10
  assert(read_value == 0);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b11
  assert(read_value == 1);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b12
  assert(read_value == 1);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b13
  assert(read_value == 0);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b14
  assert(read_value == 0);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);// b15
  assert(read_value == 1);

  //  0x11 0x99
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 2);
  assert(read_value == 0);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 2);
  assert(read_value == 1);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 2);
  assert(read_value == 0);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);
  assert(read_value == 0);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 2);
  assert(read_value == 3);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 2);
  assert(read_value == 0);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);
  assert(read_value == 1);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 2);
  assert(read_value == 2);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 2);
  assert(read_value == 1);

  //  0xA7 0xF4 0x86 0xCD 0x7D;
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 10);
  assert(read_value == 0x29F);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 10);
  assert(read_value == 0x348);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 10);
  assert(read_value == 0x1B3);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 10);
  assert(read_value == 0x17D);

  //  0x58 0xAA 0xDB 0xAA 0xBD 0x34
  //  0xE0 0x3F 0x84 0xBA 0x91 0xEC

  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);
  assert(read_value == 0);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 10);
  assert(read_value == 0x2C5);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 10);
  assert(read_value == 0x15B);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 10);
  assert(read_value == 0x1D5);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 10);
  assert(read_value == 0x17A);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 1);
  assert(read_value == 0);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 10);
  assert(read_value == 0x34E);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 10);
  assert(read_value == 0x00F);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 10);
  assert(read_value == 0x384);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 10);
  assert(read_value == 0x2EA);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 10);
  assert(read_value == 0x11E);

  bit_pos_read  = 0;
  byte_pos_read = buffer;
  // 0x96 0xD9 0x11 0x99 0xA7 0xF4 0x86 0xCD 0x7D 0x58 0xAA 0xDB 0xAA 0xBD 0x34
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 15);
  assert(read_value == 0x4B6C);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 15);
  assert(read_value == 0x4466);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 15);
  assert(read_value == 0x34FE);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 15);
  assert(read_value == 0x486C);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 15);
  assert(read_value == 0x6BEA);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 15);
  assert(read_value == 0x62AB);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 15);
  assert(read_value == 0x3755);
  read_value = rlc_am_read_bit_field(&byte_pos_read, &bit_pos_read, 15);
  assert(read_value == 0x3D34);
}
//-----------------------------------------------------------------------------
void rlc_am_v9_3_0_test_reset_sdus()
//-----------------------------------------------------------------------------
{
    int i, j;
    for (j = 0; j < 2; j++) {
        for (i = 0; i < TEST_MAX_SEND_SDU; i++) {
           send_sdu_ids[i][j]= -1;
        }
        send_id_write_index[j] = 0;
        send_id_read_index[j]  = 0;
    }
}
//-----------------------------------------------------------------------------
void rlc_am_v9_3_0_test_send_sdu(rlc_am_entity_t *am_txP, int sdu_indexP)
//-----------------------------------------------------------------------------
{
  mem_block_t *sdu;
  sdu = get_free_mem_block (strlen(sdus[sdu_indexP]) + 1 + sizeof (struct rlc_am_data_req_alloc));

  if (sdu != NULL) {
      // PROCESS OF COMPRESSION HERE:
      printf("[FRAME %05d][RLC][MOD %02d][RB %02d] TX SDU %d %04d bytes\n",mac_xface->frame,am_txP->module_id, am_txP->rb_id, sdu_indexP, strlen(sdus[sdu_indexP]) + 1);
      memset (sdu->data, 0, sizeof (struct rlc_am_data_req_alloc));
      strcpy (&sdu->data[sizeof (struct rlc_am_data_req_alloc)],sdus[sdu_indexP]);

      ((struct rlc_am_data_req *) (sdu->data))->data_size = strlen(sdus[sdu_indexP])+ 1;
      ((struct rlc_am_data_req *) (sdu->data))->conf = 1;
      ((struct rlc_am_data_req *) (sdu->data))->mui  = mui++;
      ((struct rlc_am_data_req *) (sdu->data))->data_offset = sizeof (struct rlc_am_data_req_alloc);
      rlc_am_data_req(am_txP, sdu);

      send_sdu_ids[send_id_write_index[am_txP->rb_id]++][am_txP->rb_id] = sdu_indexP;
      assert(send_id_write_index[am_txP->rb_id] < TEST_MAX_SEND_SDU);
  } else {
    printf("Out of memory error\n");
    exit(-1);
  }
}
//-----------------------------------------------------------------------------
void rlc_am_v9_3_0_test_mac_rlc_loop (struct mac_data_ind *data_indP,  struct mac_data_req *data_requestP, int* drop_countP, int *tx_packetsP, int* dropped_tx_packetsP) //-----------------------------------------------------------------------------
{


  mem_block_t* tb_src;
  mem_block_t* tb_dst;
  unsigned int tb_size;

  data_indP->no_tb = 0;
  while (data_requestP->data.nb_elements > 0) {
    tb_src = list_remove_head (&data_requestP->data);
    if (tb_src != NULL) {
        tb_size = ((struct mac_tb_req *) (tb_src->data))->tb_size_in_bits >> 3;
        printf("[RLC-LOOP] FOUND TB SIZE IN BITS %d IN BYTES %d sizeof (mac_rlc_max_rx_header_size_t) %d\n",
                   ((struct mac_tb_req *) (tb_src->data))->tb_size_in_bits,
                   tb_size, sizeof (mac_rlc_max_rx_header_size_t));

        *tx_packetsP = *tx_packetsP + 1;
        if (*drop_countP == 0) {
            tb_dst  = get_free_mem_block(sizeof (mac_rlc_max_rx_header_size_t) + tb_size);
            if (tb_dst != NULL) {
                ((struct mac_tb_ind *) (tb_dst->data))->first_bit        = 0;
                ((struct mac_tb_ind *) (tb_dst->data))->data_ptr         = &tb_dst->data[sizeof (mac_rlc_max_rx_header_size_t)];
                ((struct mac_tb_ind *) (tb_dst->data))->size             = tb_size;
                ((struct mac_tb_ind *) (tb_dst->data))->error_indication = 0;

                memcpy(((struct mac_tb_ind *) (tb_dst->data))->data_ptr,
                    &((struct mac_tb_req *) (tb_src->data))->data_ptr[0],
                    tb_size);

                list_add_tail_eurecom(tb_dst, &data_indP->data);
                data_indP->no_tb  += 1;
            } else {
               printf("Out of memory error\n");
               exit(-1);
            }
        } else {
            printf("[RLC-LOOP] DROPPING 1 TB\n");
            *drop_countP = *drop_countP - 1;
            *dropped_tx_packetsP = *dropped_tx_packetsP + 1;
        }
        free_mem_block(tb_src);
        if (data_indP->no_tb > 0) {
            printf("[RLC-LOOP] Exchange %d TBs\n",data_indP->no_tb);
        }
    }
  }
}
//-----------------------------------------------------------------------------
void rlc_am_v9_3_0_test_exchange_pdus(rlc_am_entity_t *am_txP,
                                      rlc_am_entity_t *am_rxP,
                                      u16_t           bytes_txP,
                                      u16_t           bytes_rxP)
//-----------------------------------------------------------------------------
{
  struct mac_data_req    data_request_tx;
  struct mac_data_req    data_request_rx;
  struct mac_data_ind    data_ind_tx;
  struct mac_data_ind    data_ind_rx;
  struct mac_status_ind  tx_status;
  struct mac_status_resp mac_rlc_status_resp_tx;
  struct mac_status_resp mac_rlc_status_resp_rx;


  memset(&data_request_tx, 0, sizeof(struct mac_data_req));
  memset(&data_request_rx, 0, sizeof(struct mac_data_req));
  memset(&data_ind_tx,     0, sizeof(struct mac_data_ind));
  memset(&data_ind_rx,     0, sizeof(struct mac_data_ind));
  memset(&tx_status,       0, sizeof(struct mac_status_ind));
  memset(&mac_rlc_status_resp_tx, 0, sizeof(struct mac_status_resp));
  memset(&mac_rlc_status_resp_rx, 0, sizeof(struct mac_status_resp));

  mac_rlc_status_resp_tx = rlc_am_mac_status_indication(am_txP, bytes_txP, tx_status);
  data_request_tx        = rlc_am_mac_data_request(am_txP);
  mac_rlc_status_resp_rx = rlc_am_mac_status_indication(am_rxP, bytes_rxP, tx_status);
  data_request_rx        = rlc_am_mac_data_request(am_rxP);


  rlc_am_v9_3_0_test_mac_rlc_loop(&data_ind_rx, &data_request_tx, &drop_tx, &tx_packets, &dropped_tx_packets);
  rlc_am_v9_3_0_test_mac_rlc_loop(&data_ind_tx, &data_request_rx, &drop_rx, &rx_packets, &dropped_rx_packets);
  rlc_am_mac_data_indication(am_rxP, data_ind_rx);
  rlc_am_mac_data_indication(am_txP, data_ind_tx);
  mac_xface->frame += 1;

  //rlc_am_tx_buffer_display(am_txP,NULL);
  //assert(am_txP->t_status_prohibit.time_out != 1);
  //assert(am_rxP->t_status_prohibit.time_out != 1);
  //assert(!((am_txP->vt_a == 954) && (am_txP->vt_s == 53)));
  //assert(mac_xface->frame <= 151);
  //check_mem_area(NULL);
  //display_mem_load();
}
//-----------------------------------------------------------------------------
void rlc_am_v9_3_0_test_data_conf(module_id_t module_idP, rb_id_t rb_idP, mui_t muiP, rlc_tx_status_t statusP)
//-----------------------------------------------------------------------------
{
    if (statusP == RLC_SDU_CONFIRM_YES) {
        printf("[FRAME %05d][RLC][MOD %02d][RB %02d]  CONFIRM SEND SDU MUI %05d\n",mac_xface->frame,module_idP, rb_idP, muiP);
    } else {
        printf("[FRAME %05d][RLC][MOD %02d][RB %02d]  CONFIRM LOST SDU MUI %05d\n",mac_xface->frame,module_idP, rb_idP, muiP);
    }
}
//-----------------------------------------------------------------------------
void rlc_am_v9_3_0_test_data_ind (module_id_t module_idP, rb_id_t rb_idP, sdu_size_t sizeP, mem_block_t *sduP)
//-----------------------------------------------------------------------------
{
    int i;
    for (i = 0; i < 37; i++) {
        if (strcmp(sdus[i], sduP->data) == 0) {
            printf("[FRAME %05d][RLC][MOD %02d][RB %02d] RX SDU %d %04d bytes\n",mac_xface->frame,module_idP, rb_idP, i, sizeP);
            assert(TEST_MAX_SEND_SDU > send_id_read_index[rb_idP]);
            assert(send_id_write_index[rb_idP^1] > send_id_read_index[rb_idP]);

            if (send_sdu_ids[send_id_read_index[rb_idP]][rb_idP^1] != i) {

                printf("[FRAME %05d][RLC][MOD %d][RB %d][DATA-IND] send_sdu_ids[%d] = %d\n",mac_xface->frame,module_idP, rb_idP,  send_id_read_index[rb_idP]-2, send_sdu_ids[send_id_read_index[rb_idP]-2][rb_idP^1]);

                printf("[FRAME %05d][RLC][MOD %d][RB %d][DATA-IND] send_sdu_ids[%d] = %d\n",mac_xface->frame,module_idP, rb_idP,  send_id_read_index[rb_idP]-1, send_sdu_ids[send_id_read_index[rb_idP]-1][rb_idP^1]);

                printf("[FRAME %05d][RLC][MOD %d][RB %d][DATA-IND] send_sdu_ids[%d] = %d\n",mac_xface->frame,module_idP, rb_idP,  send_id_read_index[rb_idP], send_sdu_ids[send_id_read_index[rb_idP]][rb_idP^1]);

                printf("[FRAME %05d][RLC][MOD %d][RB %d][DATA-IND] send_id_read_index = %d sdu sent = %d\n",mac_xface->frame,module_idP, rb_idP,  send_id_read_index[rb_idP], i);
            }
            assert(send_sdu_ids[send_id_read_index[rb_idP]][rb_idP^1] == i);
            send_id_read_index[rb_idP] += 1;
            free_mem_block(sduP);
            return;
        }
    }
    printf("[FRAME %05d][RLC][MOD %d][RB %d] RX UNKNOWN SDU %04d bytes\n",mac_xface->frame,module_idP, rb_idP,  sizeP);
    free_mem_block(sduP);
    assert(1==2);
}
//-----------------------------------------------------------------------------
void rlc_am_v9_3_0_test_tx_rx()
//-----------------------------------------------------------------------------
{
  u16_t                 max_retx_threshold = 255;
  u16_t                 poll_pdu           = 8;
  u16_t                 poll_byte          = 1000;
  u32_t                 t_poll_retransmit  = 15;
  u32_t                 t_reordering       = 5000;
  u32_t                 t_status_prohibit  = 10;
  int                   i,j,r;

  srand (0);

  rlc_am_init(&am_tx);
  rlc_am_init(&am_rx);
  rlc_am_set_debug_infos(&am_tx, 0, 0, 1);
  rlc_am_set_debug_infos(&am_rx, 1, 1, 1);

  rlc_am_configure(&am_tx, max_retx_threshold, poll_pdu, poll_byte, t_poll_retransmit, t_reordering, t_status_prohibit);
  rlc_am_configure(&am_rx, max_retx_threshold, poll_pdu, poll_byte, t_poll_retransmit, t_reordering, t_status_prohibit);

#ifdef TEST1
  srand (0);
  rlc_am_v9_3_0_test_reset_sdus();
  rlc_am_v9_3_0_test_send_sdu(&am_tx, 1);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 8000, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 8000, 200);
  rlc_am_v9_3_0_test_send_sdu(&am_tx, 2);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 8000, 200);
  rlc_am_v9_3_0_test_send_sdu(&am_tx, 1);
  rlc_am_v9_3_0_test_send_sdu(&am_tx, 2);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 8000, 200);
  rlc_am_v9_3_0_test_send_sdu(&am_tx, 1);
  rlc_am_v9_3_0_test_send_sdu(&am_tx, 2);
  rlc_am_v9_3_0_test_send_sdu(&am_tx, 3);
  rlc_am_v9_3_0_test_send_sdu(&am_tx, 4);
  rlc_am_v9_3_0_test_send_sdu(&am_tx, 5);
  rlc_am_v9_3_0_test_send_sdu(&am_tx, 6);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 1000, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 1000, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 1000, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 1000, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 1000, 200);
  rlc_am_rx_list_display(&am_tx, "RLC-AM TX:");
  rlc_am_rx_list_display(&am_rx, "RLC-AM RX:");
  assert (send_id_read_index[1] == send_id_write_index[0]);
    printf("\n\n\n\n\n\n-----------------------------------------------------------------------------------------rlc_am_v9_3_0_test 1: END OF SIMPLE TEST SEVERAL SDUs IN PDU\n\n\n\n");

    rlc_am_v9_3_0_test_reset_sdus();
    // RANDOM TESTS
    for (i = send_id_write_index[0]; send_id_write_index[0] < TEST_MAX_SEND_SDU-12; i++) {
        rlc_am_v9_3_0_test_send_sdu(&am_tx, 1);
        rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 8000, 200);
        rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 8000, 200);
        rlc_am_v9_3_0_test_send_sdu(&am_tx, 2);
        rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 8000, 200);
        rlc_am_v9_3_0_test_send_sdu(&am_tx, 1);
        rlc_am_v9_3_0_test_send_sdu(&am_tx, 2);
        rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 8000, 200);
        rlc_am_v9_3_0_test_send_sdu(&am_tx, 1);
        rlc_am_v9_3_0_test_send_sdu(&am_tx, 2);
        rlc_am_v9_3_0_test_send_sdu(&am_tx, 3);
        rlc_am_v9_3_0_test_send_sdu(&am_tx, 4);
        rlc_am_v9_3_0_test_send_sdu(&am_tx, 5);
        rlc_am_v9_3_0_test_send_sdu(&am_tx, 6);
        for (i = 0; i < 50; i++) {
            rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 200, 200);
        }
        assert (send_id_read_index[1] == send_id_write_index[0]);
    }
    rlc_am_rx_list_display(&am_tx, "RLC-AM TX:");
    rlc_am_rx_list_display(&am_rx, "RLC-AM RX:");
    assert (send_id_read_index[1] == send_id_write_index[0]);
    printf("\n\n\n\n\n\n-----------------------------------------------------------------------------------------rlc_am_v9_3_0_test 1: END OF TEST SEVERAL SDUs IN PDU\n\n\n\n");
#endif
#ifdef TEST2
  srand (0);
  rlc_am_v9_3_0_test_reset_sdus();
  // BIG SDU SMALL PDUS NO ERRORS
  rlc_am_v9_3_0_test_send_sdu(&am_tx, 1);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 3, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 3, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 3, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 4, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 5, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 6, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 7, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 8, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 9, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 10, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 11, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 12, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 13, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 16, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 17, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 18, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 19, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 20, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 21, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 22, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 23, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 24, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 25, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 26, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 27, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 28, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 29, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 2000, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 2000, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 2000, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 2000, 200);

  rlc_am_v9_3_0_test_send_sdu(&am_tx, 1);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 30, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 31, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 32, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 33, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 34, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 35, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 36, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 37, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 38, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 39, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 40, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 41, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 42, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 43, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 44, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 45, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 46, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 47, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 48, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 49, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 50, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 51, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 52, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 53, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 54, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 55, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 56, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 57, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 58, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 59, 200);
  //rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 60, 200); if uncomment: error because too many segments of SDU
  for (i = 0; i < 24; i++) {
      rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 2000, 200);
  }
  rlc_am_rx_list_display(&am_tx, "RLC-AM TX:");
  rlc_am_rx_list_display(&am_rx, "RLC-AM RX:");
  assert (send_id_read_index[1] == send_id_write_index[0]);
  printf("\n\n\n\n\n\n-----------------------------------------------------------------------------------------rlc_am_v9_3_0_test 2: END OF TEST BIG SDU SMALL PDUs\n\n\n\n");
#endif
#ifdef TEST3
  srand (0);
  rlc_am_v9_3_0_test_reset_sdus();
  // BIG SDU SMALL PDUS  ERRORs  ()
  rlc_am_v9_3_0_test_send_sdu(&am_tx, 1);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 3, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 3, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 3, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 4, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 5, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 6, 200);
  drop_tx = 1;
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 7, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 8, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 9, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 10, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 11, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 12, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 13, 200);
  drop_tx = 2;
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 16, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 17, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 18, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 19, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 20, 200);
  drop_tx = 4;
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 21, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 22, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 23, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 24, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 25, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 26, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 300, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 300, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 300, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 300, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 300, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 300, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 300, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 300, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 300, 200);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 300, 200);
  for (i = 0; i < 30; i++)
      rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 500, 200);

  // Purge
  for (i = 0; i < 24; i++) {
      rlc_am_v9_3_0_test_send_sdu(&am_tx, i);
      rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 300, 200);
  }
  for (i = 0; i < 100; i++) {
      rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 300, 200);
  }
  rlc_am_rx_list_display(&am_tx, "RLC-AM TX:");
  rlc_am_rx_list_display(&am_rx, "RLC-AM RX:");
  assert (send_id_read_index[1] == send_id_write_index[0]);
  printf("\n\n\n\n\n\n-----------------------------------------------------------------------------------------rlc_am_v9_3_0_test 3: END OF TEST BIG SDU SMALL PDUs WITH ERRORS ON PHY LAYER\n\n\n\n");
#endif
#ifdef TEST4
  srand (0);
  rlc_am_v9_3_0_test_reset_sdus();
  for (i = 2 ; i < 37 ; i++) {
      rlc_am_v9_3_0_test_send_sdu(&am_tx, i);
  }
  for (i = 2 ; i < 37 ; i++) {
      rlc_am_v9_3_0_test_send_sdu(&am_tx, i);
  }
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 30, 100);
  drop_tx = 4;
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 100, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 100, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 100, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 300, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 20, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 20, 100);
  drop_tx = 4;
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 20, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 15, 100);

  t_poll_retransmit = 6;
  rlc_am_configure(&am_tx, max_retx_threshold, poll_pdu, poll_byte, t_poll_retransmit, t_reordering, t_status_prohibit);

  for (i = 0 ; i < 50 ; i++) {
      rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 100, 100);
  }
  //exit(0);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 400, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 400, 100);
  //exit(0);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 400, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  //exit(0);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 14, 100);
  rlc_am_rx_list_display(&am_tx, "RLC-AM TX:");
  rlc_am_rx_list_display(&am_rx, "RLC-AM RX:");
  assert (send_id_read_index[1] == send_id_write_index[0]);
  printf("\n\n\n\n\n\n-----------------------------------------------------------------------------------------rlc_am_v9_3_0_test 4: END OF TEST SEVERAL SDUS IN PDUs WITH ERRORS ON PHY LAYER\n\n\n\n");
  assert (send_id_write_index[0] < TEST_MAX_SEND_SDU);
#endif
#ifdef TEST5
  srand (0);
  rlc_am_v9_3_0_test_reset_sdus();
  // RANDOM TESTS
  for (i = 0; send_id_write_index < TEST_MAX_SEND_SDU-1; i++) {
  //for (i = 0; send_id_write_index < 434; i++) {
      printf("AM.TX SDU %d\n", am_tx.nb_sdu);
      if (am_tx.nb_sdu < (RLC_AM_SDU_CONTROL_BUFFER_SIZE - 16)) {
          random_sdu = rand() % 37;
          rlc_am_v9_3_0_test_send_sdu(&am_tx, random_sdu);
      }
      //random_nb_frames   = (rand() % 2) + 1;
      random_nb_frames   = 1;
      for (j = 0; j < random_nb_frames; j++) {
          random_tx_pdu_size = rand() % RLC_SDU_MAX_SIZE;
          random_rx_pdu_size = rand() % RLC_SDU_MAX_SIZE;
          rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, random_tx_pdu_size, random_rx_pdu_size);
      }
  }
  rlc_am_rx_list_display(&am_tx, "RLC-AM TX:");
  rlc_am_rx_list_display(&am_rx, "RLC-AM RX:");
  printf("send_id_read_index %d send_id_write_index %d\n", send_id_read_index, send_id_write_index);
  for (j = 0; j < 100; j++) {
      random_tx_pdu_size = rand() % RLC_SDU_MAX_SIZE;
      random_rx_pdu_size = rand() % RLC_SDU_MAX_SIZE;
      rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, random_tx_pdu_size, random_rx_pdu_size);
  }
  rlc_am_rx_list_display(&am_tx, "RLC-AM TX:");
  rlc_am_rx_list_display(&am_rx, "RLC-AM RX:");
  assert (send_id_read_index[1] == send_id_write_index[0]);
  printf("\n\n\n\n\n\n-----------------------------------------------------------------------------------------rlc_am_v9_3_0_test 5: END OF TEST RANDOM TX ONLY  WITH NO ERRORS ON PHY LAYER\n\n\n\n");
#endif
    for (r = 0; r < 128; r++) {
        srand (r);
#ifdef TEST6

        for (target_tx_error_rate = 0; target_tx_error_rate < TARGET_MAX_TX_ERROR_RATE; target_tx_error_rate++) {
            for (target_rx_error_rate = 0; target_rx_error_rate < TARGET_MAX_RX_ERROR_RATE; target_rx_error_rate++) {
                tx_packets = 0;
                dropped_tx_packets = 0;
                rx_packets = 0;
                dropped_rx_packets = 0;
                rlc_am_v9_3_0_test_reset_sdus();
                // RANDOM TESTS
                for (i = send_id_write_index[0]; send_id_write_index[0] < TEST_MAX_SEND_SDU-1; i++) {
                    printf("AM.TX SDU %d\n", am_tx.nb_sdu);
                    if (am_tx.nb_sdu < (RLC_AM_SDU_CONTROL_BUFFER_SIZE - 2)) {
                        random_sdu = rand() % 37;
                        rlc_am_v9_3_0_test_send_sdu(&am_tx, random_sdu);
                    }
                    random_nb_frames   = (rand() % 10) + 1;
                    //random_nb_frames   = 1;
                    for (j = 0; j < random_nb_frames; j++) {
                        random_tx_pdu_size = (rand() % RLC_SDU_MAX_SIZE)  / ((rand () % 4)+1);
                        random_rx_pdu_size = (rand() % RLC_SDU_MAX_SIZE)  / ((rand () % 4)+1);
                        rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, random_tx_pdu_size, random_rx_pdu_size);
                    }
                    int dropped = (rand() % 3);
                    if ((dropped == 0) && (tx_packets > 0)){
                        if ((((dropped_tx_packets + 1)*100) / tx_packets) <= target_tx_error_rate) {
                            drop_tx = 1;
                        }
                    }
                    dropped = (rand() % 3);
                    if ((dropped == 0) && (rx_packets > 0)){
                        if ((((dropped_rx_packets + 1)*100) / rx_packets) <= target_rx_error_rate) {
                            drop_rx = 1;
                        }
                    }
                }
                for (j = 0; j < 400; j++) {
                    rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, 500, 500);
                }
                printf("\n\n\n\n\n\n-----------------------------------------------------------------------------------------rlc_am_v9_3_0_test 6: END OF TEST RANDOM (SEED=%d BLER TX=%d BLER RX=%d) TX ONLY WITH ERRORS ON PHY LAYER:\n\n\n\n",r, target_tx_error_rate, target_rx_error_rate);
                rlc_am_rx_list_display(&am_tx, "RLC-AM TX:");
                rlc_am_rx_list_display(&am_rx, "RLC-AM RX:");
                assert (send_id_read_index[1] == send_id_write_index[0]);
                printf("REAL BLER TX=%d (TARGET=%d) BLER RX=%d (TARGET=%d) \n",(dropped_tx_packets*100)/tx_packets, target_tx_error_rate, (dropped_rx_packets*100)/rx_packets, target_rx_error_rate);
            }
        }
#endif
#ifdef TEST7
        for (target_tx_error_rate = 0; target_tx_error_rate < TARGET_MAX_TX_ERROR_RATE; target_tx_error_rate++) {
            for (target_rx_error_rate = 0; target_rx_error_rate < TARGET_MAX_RX_ERROR_RATE; target_rx_error_rate++) {
                tx_packets = 0;
                dropped_tx_packets = 0;
                rx_packets = 0;
                dropped_rx_packets = 0;
                rlc_am_v9_3_0_test_reset_sdus();
                for (i = send_id_write_index[0]; send_id_write_index[0] < TEST_MAX_SEND_SDU-1; i++) {
                    if (am_tx.nb_sdu < (RLC_AM_SDU_CONTROL_BUFFER_SIZE - 2)) {
                        random_sdu = rand() % 37;
                        rlc_am_v9_3_0_test_send_sdu(&am_tx, random_sdu);
                        if (am_rx.nb_sdu < (RLC_AM_SDU_CONTROL_BUFFER_SIZE - 2)) {
                            random_sdu = rand() % 37;
                            rlc_am_v9_3_0_test_send_sdu(&am_rx, random_sdu);
                        } else {
                            i = i-1;
                        }
                    } else {
                        if (am_rx.nb_sdu < (RLC_AM_SDU_CONTROL_BUFFER_SIZE - 2)) {
                            random_sdu = rand() % 37;
                            rlc_am_v9_3_0_test_send_sdu(&am_rx, random_sdu);
                        } else {
                            i = i-1;
                        }
                    }
                    random_nb_frames   = rand() % 4;
                    for (j = 0; j < random_nb_frames; j++) {
                        random_tx_pdu_size = (rand() % RLC_SDU_MAX_SIZE)  / ((rand () % 4)+1);
                        random_rx_pdu_size = (rand() % RLC_SDU_MAX_SIZE)  / ((rand () % 4)+1);
                        rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, random_tx_pdu_size, random_rx_pdu_size);
                    }
                    int dropped = (rand() % 3);
                    if ((dropped == 0) && (tx_packets > 0)){
                        if ((((dropped_tx_packets + 1)*100) / tx_packets) <= target_tx_error_rate) {
                            drop_tx = 1;
                        }
                    }
                    dropped = (rand() % 3);
                    if ((dropped == 0) && (rx_packets > 0)){
                        if ((((dropped_rx_packets + 1)*100) / rx_packets) <= target_rx_error_rate) {
                            drop_rx = 1;
                        }
                    }
                }
                for (j = 0; j < 100; j++) {
                    random_tx_pdu_size = (rand() % RLC_SDU_MAX_SIZE)  / ((rand () % 4)+1);
                    random_rx_pdu_size = (rand() % RLC_SDU_MAX_SIZE)  / ((rand () % 4)+1);
                    rlc_am_v9_3_0_test_exchange_pdus(&am_tx, &am_rx, random_tx_pdu_size, random_rx_pdu_size);
                }
                printf("\n\n\n\n\n\n-----------------------------------------------------------------------------------------rlc_am_v9_3_0_test 7: END OF TEST RANDOM (SEED=%d BLER TX=%d BLER RX=%d ) TX RX WITH ERRORS ON PHY LAYER:\n\n\n\n",r, target_tx_error_rate, target_rx_error_rate);
                rlc_am_rx_list_display(&am_tx, "RLC-AM TX:");
                rlc_am_rx_list_display(&am_rx, "RLC-AM RX:");
                assert (send_id_read_index[1] == send_id_write_index[0]);
                assert (send_id_read_index[0] == send_id_write_index[1]);
                printf("REAL BLER TX=%d (TARGET=%d) BLER RX=%d (TARGET=%d) \n",(dropped_tx_packets*100)/tx_packets, target_tx_error_rate, (dropped_rx_packets*100)/rx_packets, target_rx_error_rate);

            }
        }
#endif
    }
}
//-----------------------------------------------------------------------------
void rlc_am_v9_3_0_test_print_trace (void)
//-----------------------------------------------------------------------------
{
  void *array[100];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace (array, 100);
  strings = backtrace_symbols (array, size);

  printf ("Obtained %d stack frames.\n", size);

  for (i = 0; i < size; i++)
    printf ("%s\n", strings[i]);

  free (strings);
}

//-----------------------------------------------------------------------------
void rlc_am_v9_3_0_test(void)
//-----------------------------------------------------------------------------
{
//     initscr();
//     cbreak();
//     keypad(stdscr, TRUE);



    // under test
    pool_buffer_init();
    rlc_am_v9_3_0_test_tx_rx();

    // already tested
    rlc_am_v9_3_0_test_windows();
    rlc_am_v9_3_0_test_read_write_bit_field();
    printf("rlc_am_v9_3_0_test: END OF TESTS\n");
    endwin();
    exit(0);
}
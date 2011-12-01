#include "PHY/defs.h"
#include "PHY/extern.h"

u16 NCS_fdd_unrestricted[16] = {0,13,15,18,22,26,32,38,46,59,76,93,119,167,279,419};
u16 NCS_fdd_restricted[15]   = {15,18,22,26,32,38,46,55,68,82,100,128,158,202,237};
u16 NCS_tdd[7]               = {2,4,6,8,10,12,15};

typedef struct {
  u8 f_ra;
  u8 t0_ra;
  u8 t1_ra;
  u8 t2_ra;
} PRACH_TDD_PREAMBLE_MAP_elem;
typedef struct {
  u8 num_prach;
  PRACH_TDD_PREAMBLE_MAP_elem map[6];
} PRACH_TDD_PREAMBLE_MAP;

// This is table 5.7.1-4 from 36.211
PRACH_TDD_PREAMBLE_MAP tdd_preamble_map[64][7] = { 
  // TDD Configuration Index 0
  { {1,{{0,1,0,2}}},{1,{{0,1,0,1}}}, {1,{{0,1,0,0}}}, {1,{{0,1,0,2}}}, {1,{{0,1,0,1}}}, {1,{{0,1,0,0}}}, {1,{{0,1,0,2}}}},
  // TDD Configuration Index 1
  { {1,{{0,2,0,2}}},{1,{{0,2,0,1}}}, {1,{{0,2,0,0}}}, {1,{{0,2,0,2}}}, {1,{{0,2,0,1}}}, {1,{{0,2,0,0}}}, {1,{{0,2,0,2}}}},
  // TDD Configuration Index 2
  { {1,{{0,1,1,2}}},{1,{{0,1,1,1}}}, {1,{{0,1,1,0}}}, {1,{{0,1,0,1}}}, {1,{{0,1,0,0}}}, {0,{{0,0,0,0}}}, {1,{{0,1,1,1}}}},
  // TDD Configuration Index 3
  { {1,{{0,0,0,2}}},{1,{{0,0,0,1}}}, {1,{{0,0,0,0}}}, {1,{{0,0,0,2}}}, {1,{{0,0,0,1}}}, {1,{{0,0,0,0}}}, {1,{{0,0,0,2}}}},
  // TDD Configuration Index 4
  { {1,{{0,0,1,2}}},{1,{{0,0,1,1}}}, {1,{{0,0,1,0}}}, {1,{{0,0,0,1}}}, {1,{{0,0,0,0}}}, {0,{{0,0,0,0}}}, {1,{{0,0,1,1}}}},
  // TDD Configuration Index 5
  { {1,{{0,0,0,1}}},{1,{{0,0,0,0}}}, {0,{{0,0,0,0}}}, {1,{{0,0,0,0}}}, {0,{{0,0,0,1}}}, {0,{{0,0,0,0}}}, {1,{{0,0,0,1}}}},
  // TDD Configuration Index 6
  { {2,{{0,0,0,2},{0,0,1,2}}}, {2,{{0,0,0,1},{0,0,1,1}}}, {2,{{0,0,0,0},{0,0,1,0}}}, {2,{{0,0,0,1},{0,0,0,2}}}, {2,{{0,0,0,0},{0,0,0,1}}}, {2,{{0,0,0,0},{1,0,0,0}}}, {2,{{0,0,0,2},{0,0,1,1}}}},
  // TDD Configuration Index 7
  { {2,{{0,0,0,1},{0,0,1,1}}}, {2,{{0,0,0,0},{0,0,1,0}}}, {0,{{0,0,0,0},{0,0,0,0}}}, {2,{{0,0,0,1},{0,0,0,2}}}, {0,{{0,0,0,0},{0,0,0,0}}}, {0,{{0,0,0,0},{0,0,0,0}}}, {2,{{0,0,0,1},{0,0,1,0}}}},
  // TDD Configuration Index 8
  { {2,{{0,0,0,0},{0,0,1,0}}}, {0,{{0,0,0,0},{0,0,0,0}}}, {0,{{0,0,0,0},{0,0,0,0}}}, {2,{{0,0,0,0},{0,0,0,1}}}, {0,{{0,0,0,0},{0,0,0,0}}}, {0,{{0,0,0,0},{0,0,0,0}}}, {2,{{0,0,0,0},{0,0,1,1}}}},
  // TDD Configuration Index 9
  { {3,{{0,0,0,1},{0,0,0,2},{0,0,1,2}}}, {3,{{0,0,0,0},{0,0,0,1},{0,0,1,1}}}, {3,{{0,0,0,0},{0,0,1,0},{1,0,0,0}}}, {3,{{0,0,0,0},{0,0,0,1},{0,0,0,2}}}, {3,{{0,0,0,0},{0,0,0,1},{1,0,0,1}}}, {3,{{0,0,0,0},{1,0,0,0},{2,0,0,0}}}, {3,{{0,0,0,1},{0,0,0,2},{0,0,1,1}}}},
  // TDD Configuration Index 10
  { {3,{{0,0,0,0},{0,0,1,0},{0,0,1,1}}}, {3,{{0,0,0,1},{0,0,1,0},{0,0,1,1}}}, {3,{{0,0,0,0},{0,0,1,0},{1,0,1,0}}}, {0,{{0,0,0,0},{0,0,0,0},{0,0,0,0}}}, {3,{{0,0,0,0},{0,0,0,1},{1,0,0,0}}}, {0,{{0,0,0,0},{0,0,0,0},{0,0,0,0}}}, {3,{{0,0,0,0},{0,0,0,2},{0,0,1,0}}}},
  // TDD Configuration Index 11
  { {0,{{0,0,0,0},{0,0,0,0},{0,0,0,0}}}, {3,{{0,0,0,0},{0,0,0,1},{0,0,1,0}}}, {0,{{0,0,0,0},{0,0,0,0},{0,0,0,0}}}, {0,{{0,0,0,0},{0,0,0,0},{0,0,0,0}}}, {0,{{0,0,0,0},{0,0,0,0},{0,0,0,0}}}, {0,{{0,0,0,0},{0,0,0,0},{0,0,0,0}}}, {3,{{0,0,0,1},{0,0,1,0},{0,0,1,1}}}}
};



u16 prach_root_sequence_map0_3[838] = { 129, 710, 140, 699, 120, 719, 210, 629, 168, 671, 84, 755, 105, 734, 93, 746, 70, 769, 60, 779,
					2, 837, 1, 838,
					56, 783, 112, 727, 148, 691,
					80, 759, 42, 797, 40, 799,
					35, 804, 73, 766, 146, 693,
					31, 808, 28, 811, 30, 809, 27, 812, 29, 810,
					24, 815, 48, 791, 68, 771, 74, 765, 178, 661, 136, 703,
					86, 753, 78, 761, 43, 796, 39, 800, 20, 819, 21, 818,
					95, 744, 202, 637, 190, 649, 181, 658, 137, 702, 125, 714, 151, 688,
					217, 622, 128, 711, 142, 697, 122, 717, 203, 636, 118, 721, 110, 729, 89, 750, 103, 736, 61,
					778, 55, 784, 15, 824, 14, 825,
					12, 827, 23, 816, 34, 805, 37, 802, 46, 793, 207, 632, 179, 660, 145, 694, 130, 709, 223, 616,
					228, 611, 227, 612, 132, 707, 133, 706, 143, 696, 135, 704, 161, 678, 201, 638, 173, 666, 106,
					733, 83, 756, 91, 748, 66, 773, 53, 786, 10, 829, 9, 830,
					7, 832, 8, 831, 16, 823, 47, 792, 64, 775, 57, 782, 104, 735, 101, 738, 108, 731, 208, 631, 184,
					655, 197, 642, 191, 648, 121, 718, 141, 698, 149, 690, 216, 623, 218, 621,
					152, 687, 144, 695, 134, 705, 138, 701, 199, 640, 162, 677, 176, 663, 119, 720, 158, 681, 164,
					675, 174, 665, 171, 668, 170, 669, 87, 752, 169, 670, 88, 751, 107, 732, 81, 758, 82, 757, 100,
					739, 98, 741, 71, 768, 59, 780, 65, 774, 50, 789, 49, 790, 26, 813, 17, 822, 13, 826, 6, 833,
					5, 834, 33, 806, 51, 788, 75, 764, 99, 740, 96, 743, 97, 742, 166, 673, 172, 667, 175, 664, 187,
					652, 163, 676, 185, 654, 200, 639, 114, 725, 189, 650, 115, 724, 194, 645, 195, 644, 192, 647,
					182, 657, 157, 682, 156, 683, 211, 628, 154, 685, 123, 716, 139, 700, 212, 627, 153, 686, 213,
					626, 215, 624, 150, 689,
					225, 614, 224, 615, 221, 618, 220, 619, 127, 712, 147, 692, 124, 715, 193, 646, 205, 634, 206,
					633, 116, 723, 160, 679, 186, 653, 167, 672, 79, 760, 85, 754, 77, 762, 92, 747, 58, 781, 62,
					777, 69, 770, 54, 785, 36, 803, 32, 807, 25, 814, 18, 821, 11, 828, 4, 835,
					3, 836, 19, 820, 22, 817, 41, 798, 38, 801, 44, 795, 52, 787, 45, 794, 63, 776, 67, 772, 72,
					767, 76, 763, 94, 745, 102, 737, 90, 749, 109, 730, 165, 674, 111, 728, 209, 630, 204, 635, 117,
					722, 188, 651, 159, 680, 198, 641, 113, 726, 183, 656, 180, 659, 177, 662, 196, 643, 155, 684,
					214, 625, 126, 713, 131, 708, 219, 620, 222, 617, 226, 613,
					230, 609, 232, 607, 262, 577, 252, 587, 418, 421, 416, 423, 413, 426, 411, 428, 376, 463, 395,
					444, 283, 556, 285, 554, 379, 460, 390, 449, 363, 476, 384, 455, 388, 451, 386, 453, 361, 478,
					387, 452, 360, 479, 310, 529, 354, 485, 328, 511, 315, 524, 337, 502, 349, 490, 335, 504, 324,
					515,
					323, 516, 320, 519, 334, 505, 359, 480, 295, 544, 385, 454, 292, 547, 291, 548, 381, 458, 399,
					440, 380, 459, 397, 442, 369, 470, 377, 462, 410, 429, 407, 432, 281, 558, 414, 425, 247, 592,
					277, 562, 271, 568, 272, 567, 264, 575, 259, 580,
					237, 602, 239, 600, 244, 595, 243, 596, 275, 564, 278, 561, 250, 589, 246, 593, 417, 422, 248,
					591, 394, 445, 393, 446, 370, 469, 365, 474, 300, 539, 299, 540, 364, 475, 362, 477, 298, 541,
					312, 527, 313, 526, 314, 525, 353, 486, 352, 487, 343, 496, 327, 512, 350, 489, 326, 513, 319,
					520, 332, 507, 333, 506, 348, 491, 347, 492, 322, 517,
					330, 509, 338, 501, 341, 498, 340, 499, 342, 497, 301, 538, 366, 473, 401, 438, 371, 468, 408,
					431, 375, 464, 249, 590, 269, 570, 238, 601, 234, 605,
					257, 582, 273, 566, 255, 584, 254, 585, 245, 594, 251, 588, 412, 427, 372, 467, 282, 557, 403,
					436, 396, 443, 392, 447, 391, 448, 382, 457, 389, 450, 294, 545, 297, 542, 311, 528, 344, 495,
					345, 494, 318, 521, 331, 508, 325, 514, 321, 518,
					346, 493, 339, 500, 351, 488, 306, 533, 289, 550, 400, 439, 378, 461, 374, 465, 415, 424, 270,
					569, 241, 598,
					231, 608, 260, 579, 268, 571, 276, 563, 409, 430, 398, 441, 290, 549, 304, 535, 308, 531, 358,
					481, 316, 523,
					293, 546, 288, 551, 284, 555, 368, 471, 253, 586, 256, 583, 263, 576,
					242, 597, 274, 565, 402, 437, 383, 456, 357, 482, 329, 510,
					317, 522, 307, 532, 286, 553, 287, 552, 266, 573, 261, 578,
					236, 603, 303, 536, 356, 483,
					355, 484, 405, 434, 404, 435, 406, 433,
					235, 604, 267, 572, 302, 537,
					309, 530, 265, 574, 233, 606,
					367, 472, 296, 543,
					336, 503, 305, 534, 373, 466, 280, 559, 279, 560, 419, 420, 240, 599, 258, 581, 229, 610};

u16 prach_root_sequence_map4[838] = {  1,138,2,137,3,136,4,135,5,134,6,133,7,132,8,131,9,130,10,129,
				       11,128,12,127,13,126,14,125,15,124,16,123,17,122,18,121,19,120,20,119,
				       21,118,22,117,23,116,24,115,25,114,26,113,27,112,28,111,29,110,30,109,
				       31,108,32,107,33,106,34,105,35,104,36,103,37,102,38,101,39,100,40,99,
				       41,98,42,97,43,96,44,95,45,94,46,93,47,92,48,91,49,90,50,89,
				       51,88,52,87,53,86,54,85,55,84,56,83,57,82,58,81,59,80,60,79,
				       61,78,62,77,63,76,64,75,65,74,66,73,67,72,68,71,69,70};

u16 du[838];

// This function finds the
void fill_du(u8 prach_fmt) {

  u16 iu,u,p;
  u16 N_ZC;
  u16 *prach_root_sequence_map;

  if (prach_fmt<4) {
    N_ZC = 839;
    prach_root_sequence_map = prach_root_sequence_map0_3;
  }
  else {
    N_ZC = 139;
    prach_root_sequence_map = prach_root_sequence_map4;
  }

  for (iu=1;iu<(N_ZC-1);iu++) {

    u=prach_root_sequence_map[iu];
    p=1;
    while (((u*p)%N_ZC)!=1)
      p++;
    du[u] = (p<(N_ZC>>1)) ? p : (N_ZC-p);
  }
  
}

u8  get_prach_fmt(u8 prach_ConfigIndex,u8 frame_type) {

  if (frame_type == 0) // FDD
    return(prach_ConfigIndex>>4);

  else {
    if (prach_ConfigIndex < 20)
      return (0);
    if (prach_ConfigIndex < 30)
      return (1);
    if (prach_ConfigIndex < 40)
      return (2);
    if (prach_ConfigIndex < 48)
      return (3);
    else
      return (4);
  }
}


void init_prach(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 subframe,u16 preamble_index, u16 Nf,u8 tdd_mapindex) {

  u8 frame_type         = phy_vars_ue->lte_frame_parms.frame_type;
  u8 tdd_config         = phy_vars_ue->lte_frame_parms.tdd_config;
  u16 rootSequenceIndex = 0;//phy_vars_ue->lte_frame_parms.prach_config_common.prach_ConfigInfo.rootSequenceIndex; 
  u8 prach_ConfigIndex  = phy_vars_ue->lte_frame_parms.prach_config_common.prach_ConfigInfo.prach_ConfigIndex; 
  u8 Ncs_config         = phy_vars_ue->lte_frame_parms.prach_config_common.prach_ConfigInfo.zeroCorrelationZoneConfig;
  u8 restricted_set     = phy_vars_ue->lte_frame_parms.prach_config_common.prach_ConfigInfo.highSpeedFlag;
  u8 n_ra_prboffset     = phy_vars_ue->lte_frame_parms.prach_config_common.prach_ConfigInfo.prach_FreqOffset;
  u8 n_ra_prb;
  //  u32 prachF            = phy_vars_ue->lte_ue_prach_vars[eNB_id].prachF;
  //  u32 prach             = phy_vars_ue->lte_ue_prach_vars[eNB_id].prach;

  u16 preamble_offset;

  u8 prach_fmt = get_prach_fmt(prach_ConfigIndex,frame_type);
  u8 Nsp=2;
  u8 f_ra,t1_ra;
  u16 N_ZC = (prach_fmt <4)?839:139;

  if (frame_type == 0) { // FDD
    n_ra_prb = n_ra_prboffset;
  }
  else { // TDD
    // adjust n_ra_prboffset for frequency multiplexing (p.36 36.211)

    f_ra = tdd_preamble_map[prach_ConfigIndex][tdd_config].map[tdd_mapindex].f_ra;

    if (prach_fmt < 4) {
      if ((f_ra&1) == 0) {
	n_ra_prb = n_ra_prboffset + 6*(f_ra>>1);
      }    
      else {
	n_ra_prb = phy_vars_ue->lte_frame_parms.N_RB_UL - 6 - n_ra_prboffset + 6*(f_ra>>1);
      }
    }
    else {
      if ((tdd_config >2) && (tdd_config<6)) 
	Nsp = 2;
      t1_ra = tdd_preamble_map[prach_ConfigIndex][tdd_config].map[0].t1_ra;

      if ((((Nf&1)*(2-Nsp)+t1_ra)&1) == 0) {
	n_ra_prb = 6*f_ra;
      }
      else {
	n_ra_prb = phy_vars_ue->lte_frame_parms.N_RB_UL - 6*(f_ra+1);
      }
    }
  }

  /*
  // now generate PRACH signal
  nsymb = (frame_parms->Ncp==0) ? 14:12;
  subframe_offset = (unsigned int)frame_parms->ofdm_symbol_size*(l+(subframe*nsymb));
  
  k = 7 + (12*n_ra_prb) - 6*phy_vars_ue->lte_frame_parms.N_RB_UL;
  if (k<0)
    k+=phy_vars_ue->lte_frame_parms.N_RB_UL;
  k*=12;


  if (N_ZC==189)
    Xu=Xu_839[preamble_offset];
  else
    Xu=Xu_139[preamble_offset];

  for (offset=0;offset<N_ZC;offset++) {
    prachF[k++]=Xu[offset];
    if (k>(12*phy_vars_ue->lte_frame_parms.N_RB_DL))
      k=0;
  }
  // do IDFT
  switch (phy_vars_ue->lte_frame_parms.N_RB_UL) {
  case 6:
    if (prach_fmt == 4)
      fft(prachF,prach,twiddleifft256,rev256,8,4,0);
    else
      ifft1536(prachF,prach);
    break;
  case 15:
    if (prach_fmt == 4)
      fft(prachF,prach,twiddleifft512,rev512,9,4,0);
    else
      ifft1536(prachF,prach);
    break;
  case 25:
    if (prach_fmt == 4)
      fft(prachF,prach,twiddleifft1024,rev1024,10,5,0);
    else
      ifft6144(prachF,prach);
    break;
  case 50:
    if (prach_fmt == 4)
      fft(prachF,prach,twiddleifft2048,rev2048,11,5,0);
    else
      ifft12288(prachF,prach);
    break;
  case 75:
    if (prach_fmt == 4)
      ifft3072(prachF,prach);
    else
      ifft18432(prachF,prach);
    break;
  case 100:
    if (prach_fmt == 4)
      fft(prachF,prach,twiddleifft4096,rev4096,12,6,0);
    else
      ifft24576(prachF,prach);
    break;
  }
  */

}


void generate_prach(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u8 subframe,u16 preamble_index, u16 Nf, u8 tdd_mapindex) {

  u8 frame_type         = phy_vars_ue->lte_frame_parms.frame_type;
  u8 tdd_config         = phy_vars_ue->lte_frame_parms.tdd_config;
  u16 rootSequenceIndex = 0;//phy_vars_ue->lte_frame_parms.prach_config_common.prach_ConfigInfo.rootSequenceIndex; 
  u8 prach_ConfigIndex  = phy_vars_ue->lte_frame_parms.prach_config_common.prach_ConfigInfo.prach_ConfigIndex; 
  u8 Ncs_config         = phy_vars_ue->lte_frame_parms.prach_config_common.prach_ConfigInfo.zeroCorrelationZoneConfig;
  u8 restricted_set     = phy_vars_ue->lte_frame_parms.prach_config_common.prach_ConfigInfo.highSpeedFlag;
  u8 n_ra_prboffset     = phy_vars_ue->lte_frame_parms.prach_config_common.prach_ConfigInfo.prach_FreqOffset;
  //  u32 prachF            = phy_vars_ue->lte_ue_prach_vars[eNB_id].prachF;
  //  u32 prach             = phy_vars_ue->lte_ue_prach_vars[eNB_id].prach;
  u8 n_ra_prb;
  u16 NCS;
  u16 preamble_offset,preamble_shift;
  u16 preamble_index0,n_shift_ra,n_shift_ra_bar;
  u16 d_start,n_group_ra,numshift;

  u8 prach_fmt = get_prach_fmt(prach_ConfigIndex,frame_type);
  u8 Nsp=2;
  u8 f_ra,t1_ra;
  u16 N_ZC = (prach_fmt <4)?839:139;
  u8 not_found;

  if (frame_type == 0) { // FDD

    // First compute physical root sequence
    if (restricted_set == 0) {
      if (Ncs_config>15) {
	msg("[PHY] generate_prach.c : FATAL, Illegal Ncs_config for unrestricted format %d\n",Ncs_config);
	mac_xface->macphy_exit("");
      }
      NCS = NCS_fdd_unrestricted[Ncs_config];

    }
    else {
      if (Ncs_config>14) {
	msg("[PHY] generate_prach.c : FATAL, Illegal Ncs_config for restricted format %d\n",Ncs_config);
	mac_xface->macphy_exit("");
      }
      NCS = NCS_fdd_restricted[Ncs_config];
    }

    n_ra_prb = n_ra_prboffset;
  }
  else { // TDD
    // adjust n_ra_prboffset for frequency multiplexing (p.36 36.211)

    f_ra = tdd_preamble_map[prach_ConfigIndex][tdd_config].map[tdd_mapindex].f_ra;

    if (prach_fmt < 4) {
      if ((f_ra&1) == 0) {
	n_ra_prb = n_ra_prboffset + 6*(f_ra>>1);
      }    
      else {
	n_ra_prb = phy_vars_ue->lte_frame_parms.N_RB_UL - 6 - n_ra_prboffset + 6*(f_ra>>1);
      }
    }
    else {
      if ((tdd_config >2) && (tdd_config<6)) 
	Nsp = 2;
      t1_ra = tdd_preamble_map[prach_ConfigIndex][tdd_config].map[0].t1_ra;

      if ((((Nf&1)*(2-Nsp)+t1_ra)&1) == 0) {
	n_ra_prb = 6*f_ra;
      }
      else {
	n_ra_prb = phy_vars_ue->lte_frame_parms.N_RB_UL - 6*(f_ra+1);
      }
    }
  }

  // This is the relative offset in the root sequence table (5.7.2-4 from 36.211) for the given preamble index
  preamble_offset = ((NCS==0)? preamble_index : (preamble_index/(N_ZC/NCS)));
  
  if (restricted_set == 0) {
    // This is the \nu corresponding to the preamble index 
    preamble_shift  = (NCS==0)? 0 : (preamble_offset % (N_ZC/NCS));
    preamble_shift *= NCS;
    
    // This is the offset in the root sequence table (5.7.2-4 from 36.211)
    preamble_offset += rootSequenceIndex;
  }
  else { // This is the high-speed case
    not_found=1;
    preamble_index0=preamble_index;
    // set preamble_offset to initial rootSequenceIndex and look if we need more root sequences for this
    // preamble index and find the corresponding cyclic shift
    preamble_offset = rootSequenceIndex;
    while (not_found == 1) {
      if ( (du[rootSequenceIndex]<(N_ZC/3)) && (du[rootSequenceIndex]>NCS) ) {
	n_shift_ra     = du[rootSequenceIndex]/NCS;
	d_start        = (du[rootSequenceIndex]<<1) + (n_shift_ra * NCS);
	n_group_ra     = N_ZC/d_start;
	n_shift_ra_bar = max(0,(N_ZC-(du[rootSequenceIndex]<<1)- (n_group_ra*d_start))/N_ZC);
      }
      else if  ( (du[rootSequenceIndex]>=(N_ZC/3)) && (du[rootSequenceIndex]<=((N_ZC - NCS)>>1)) ) {
	n_shift_ra     = (N_ZC - (du[rootSequenceIndex]<<1))/NCS;
	d_start        = N_ZC - (du[rootSequenceIndex]<<1) + (n_shift_ra * NCS);
	n_group_ra     = du[rootSequenceIndex]/d_start;
	n_shift_ra_bar = min(n_shift_ra,max(0,(du[rootSequenceIndex]- (n_group_ra*d_start))/NCS));
      }
      else {
	n_shift_ra     = 0;
	n_shift_ra_bar = 0;
      }
      // This is the number of cyclic shifts for the current rootSequenceIndex
      numshift = (n_shift_ra*n_group_ra) + n_shift_ra_bar;
      if (preamble_index0 < numshift) {
	not_found      = 0;
	preamble_shift = (d_start * (preamble_index0/n_shift_ra)) + ((preamble_index0%n_shift_ra)*NCS);
      }
      else {  // skip to next rootSequenceIndex and recompute parameters
	preamble_offset++;
	preamble_index0 -= numshift;
      }
    }
  }
  /*
  // now generate PRACH signal
  nsymb = (frame_parms->Ncp==0) ? 14:12;
  subframe_offset = (unsigned int)frame_parms->ofdm_symbol_size*(l+(subframe*nsymb));
  
  k = 7 + (12*n_ra_prb) - 6*phy_vars_ue->lte_frame_parms.N_RB_UL;
  if (k<0)
    k+=phy_vars_ue->lte_frame_parms.N_RB_UL;
  k*=12;

  if (N_ZC==189)
    Xu=Xu_839[preamble_offset];
  else
    Xu=Xu_139[preamble_offset];

  for (offset=0;offset<N_ZC;offset++) {
    prachF[k++]=Xu[offset];
    if (k>(12*phy_vars_ue->lte_frame_parms.N_RB_DL))
      k=0;
  }
  // do IDFT
  */
}

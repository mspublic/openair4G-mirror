void generate_phich_tdd(LTE_FRAME_PARMS frame_parms,unsigned char pnumber,unsigned char HI) {

  unsigned int d[12],d2[12];
  char z = (HI<<1)-1;

  // 
  // scrambling (later)

  memset(d,0,12*sizeof(unsigned int));

  if (frame_parms->Ncp == 1) { // Normal Cyclic Prefix
    switch (pnumber) {
    case 0: // +1 +1 +1 +1
      break;
    case 1: // +1 -1 +1 -1
      break;
    case 2: // +1 +1 -1 -1
      break;
    case 3: // +1 -1 -1 +1
      break;
    case 4: // +j +j +j +j
      break;
    case 5: // +j -j +j -j
      break;
    case 6: // +j +j -j -j
      break;
    case 7: // +j -j -j +j
      break;
    default:
      msg("phich_coding.c: Illegal PHICH Number\n");
      }
    else {
      switch (pnumber) {
      case 0: // +1 +1 
	((short*)&d)[0] = z;
	((short*)&d)[2] = z;
	((short*)&d)[8] = z;
	((short*)&d)[10] = z;
	((short*)&d)[16] = z;
	((short*)&d)[18] = z;
	break;
      case 1: // +1 -1 
	((short*)&d)[4] = z;
	((short*)&d)[6] = -z;
	((short*)&d)[12] = z;
	((short*)&d)[14] = -z;
	((short*)&d)[20] = z;
	((short*)&d)[22] = -z;
	break;
      case 2: // +j +j 
	((short*)&d)[1] = z;
	((short*)&d)[3] = z;
	((short*)&d)[9] = z;
	((short*)&d)[11] = z;
	((short*)&d)[17] = z;
	((short*)&d)[19] = z;
	break;
      case 3: // +j -j 
	((short*)&d)[5] = z;
	((short*)&d)[7] = -z;
	((short*)&d)[13] = z;
	((short*)&d)[15] = -z;
	((short*)&d)[21] = z;
	((short*)&d)[23] = -z;
	break;
      default:
	msg("phich_coding.c: Illegal PHICH Number\n");
      }
      
    }
    
    if (lte_frame_parms->nb_antennas_tx == 2) {
      // do Alamouti precoding here

    }
}

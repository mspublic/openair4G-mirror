unsigned char log2_approx(unsigned int x) {

  int i;
  unsigned char l2;

  l2=0;
  for (i=0;i<31;i++)
    if ((x&(1<<i)) != 0)
      //     l2 = i+1;
      l2 = i;

  //  printf("log2_approx = %d\n",l2);
  return(l2);
}
 

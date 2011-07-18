#ifndef __COMPLEX_H__
#define __COMPLEX_H__

#ifdef USER_MODE
struct complex
{
  double r;
  double i;
};

struct complexf
{
	float r;
	float i;
};
#endif //USER_MODE

typedef struct complex16
{
  short r;
  short i;	
}complex16;

typedef struct complex32
{
  int r;
  int i;
}complex32;

#endif //__COMPLEX_H__

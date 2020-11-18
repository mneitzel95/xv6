// Test that fork fails gracefully.
// Tiny executable so that the limit can be filling the proc table.

#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"
#include "types.h"
//#include "defs.h"

uint
random(void)
{
  // Take from http://stackoverflow.com/questions/1167253/implementation-of-rand
  static unsigned int z1 = 12345, z2 = 12345, z3 = 12345, z4 = 12345;
  unsigned int b;
  b  = ((z1 << 6) ^ z1) >> 13;
  z1 = ((z1 & 4294967294U) << 18) ^ b;
  b  = ((z2 << 2) ^ z2) >> 27;
  z2 = ((z2 & 4294967288U) << 2) ^ b;
  b  = ((z3 << 13) ^ z3) >> 21;
  z3 = ((z3 & 4294967280U) << 7) ^ b;
  b  = ((z4 << 3) ^ z4) >> 12;
  z4 = ((z4 & 4294967168U) << 13) ^ b;

  return (z1 ^ z2 ^ z3 ^ z4) / 2;
}

int
randomrange(int lo, int hi)
{
  if (hi < lo) {
    int tmp = lo;
    lo = hi;
    hi = tmp;
  }
  int range = hi - lo + 1;
  return random() % (range) + lo;
}

#define N  1000

/*void
printf(int fd, const char *s, ...)
{
  write(fd, s, strlen(s));
}*/

static void
putc(int fd, char c)
{
  write(fd, &c, 1);
}

static void
printint(int fd, int xx, int base, int sgn)
{
  static char digits[] = "0123456789ABCDEF";
  char buf[16];
  int i, neg;
  uint x;

  neg = 0;
  if (sgn && xx < 0) {
    neg = 1;
    x = -xx;
  } else {
    x = xx;
  }

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while ((x /= base) != 0);
  if (neg)
    buf[i++] = '-';

  while (--i >= 0)
    putc(fd, buf[i]);
}

// Print to the given fd. Only understands %d, %x, %p, %s.
void
printf(int fd, const char *fmt, ...)
{
  char *s;
  int c, i, state;
  uint *ap;

  state = 0;
  ap = (uint*)(void*)&fmt + 1;
  for (i = 0; fmt[i]; i++) {
    c = fmt[i] & 0xff;
    if (state == 0) {
      if (c == '%') {
        state = '%';
      } else {
        putc(fd, c);
      }
    } else if (state == '%') {
      if (c == 'd') {
        printint(fd, *ap, 10, 1);
        ap++;
      } else if (c == 'x' || c == 'p') {
        printint(fd, *ap, 16, 0);
        ap++;
      } else if (c == 's') {
        s = (char*)*ap;
        ap++;
        if (s == 0)
          s = "(null)";
        while (*s != 0) {
          putc(fd, *s);
          s++;
        }
      } else if (c == 'c') {
        putc(fd, *ap);
        ap++;
      } else if (c == '%') {
        putc(fd, c);
      } else {
        // Unknown % sequence.  Print it to draw attention.
        putc(fd, '%');
        putc(fd, c);
      }
      state = 0;
    }
  }
}

void
forktest(void)
{
  int n, pid;

  printf(1, "fork test\n");

  for(n=0; n<N; n++){
    pid = fork();
    if(pid < 0)
      break;
    if(pid == 0)
      exit();
  }

  if(n == N){
    printf(1, "fork claimed to work N times!\n", N);
    exit();
  }

  for(; n > 0; n--){
    if(wait() < 0){
      printf(1, "wait stopped early\n");
      exit();
    }
  }

  if(wait() != -1){
    printf(1, "wait got too many\n");
    exit();
  }

  printf(1, "fork test OK\n");
}

int
main(int argc, char *argv[])
{

  int size = 0;
  size =  atoi(argv[1]);
  //size = size +1;
  int array[size];
  int storage[size];
  int tracer = 0;
  /*array[1] = randomrange(0,10000);
  array[0] = randomrange(0,10000);
  array[2] = randomrange(0,10000);
  printf(1, "fork test %d\n", array[0]);
  printf(1, "fork test %d\n", array[1]);
  printf(1, "fork test %d\n", array[2]);*/
  printf(1, "lab02 starts\n");

  int n, pid;

  for(n=0; n<size; n++){
    pid = fork();
    if(pid < 0)
      break;
    else if(pid == 0){
	    for(int i=0; i <= n; i++)
	    array[n]= randomrange(0,10000);
      printf(1, "Hello, I am Child %d", n+1);
      printf(1, " with random number %d\n", array[n]);
      //sleep(5);
      exit();
    }
    else{
	storage[tracer] = wait();
	tracer++;
    }
  }

  /*if(n == size){
    printf(1, "fork claimed to work N times!\n", size);
    exit();
  }*/

  for(n=1; n < size; n++){
     storage[tracer] = wait();
     tracer++;
	  /*if(wait() < 0){
      printf(1, "wait stopped early\n");
      exit();
    }*/
  }
  
  tracer = 0;

  for(int j = 0; j < size; j++){
	  array[j] = randomrange(0, 10000);
  printf(1, "Child id: %d\n", storage[j]);
  	if (tracer < array[j]){
  		tracer = array[j];
  	}
  }
  printf(1, "The biggest number is %d\n", tracer);


  if(wait() != -1){
    printf(1, "wait got too many\n", storage[0]);
    exit();
  }

  printf(1, "lab02 OK\n");
  //forktest();
  exit();
}

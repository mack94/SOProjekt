// Autor: Maciej Makówka 
// POSIX 

#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h> 
#include <signal.h> 
#include <stddef.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <semaphore.h> 
#include <sys/types.h>
#include <unistd.h>
#include <time.h> 
// moze bedzie potrzebne
#include <mqueue.h>  // funkcja zwiazane z posixowymi kolejkami komunikatow
#include <sys/mman.h> // posixowa pamiec wspolna 

//#include <pthread.h>

#define LICZBA_TOROW 10 
#define EXPRESS_PRIOR 0
#define TOWAROWY_PRIOR 1
#define ZWYKLY_PRIOR 2 
#define NATEZENIE 10 
#define DLUGOSC_TUNELU 100 
#define EXPRESS_V 60 // max predkosc ekspresu w tunelu 
#define TOWAR_V 15 // max predkosc towarowego w tunelu
#define ZWYKLY_V 50 // max predkosc zwyklego w tunelu 

struct node { // kazdy tor potrzebuje swojej kolejki... 
  pid_t pociagID; 
  struct node *ptr;
}; 

struct tory {
  
  struct node *front[LICZBA_TOROW]; 
  struct node *rear[LICZBA_TOROW]; 
  int ilePociagowNaTorze[LICZBA_TOROW];
  
};

struct struktura_oczekujacych {

  pid_t front[LICZBA_TOROW];
  sem_t semaphore[LICZBA_TOROW]; 
  
};
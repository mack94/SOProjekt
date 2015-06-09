#include "header.h" 

int tor;
int priorytet;

sem_t *semafor; 

struct node *temp;
struct tory *stanTorow;  

struct struktura_oczekujacych *oczekujace; 
int tory_w_pamieci_wspolnej;

void zamelduj_opuszczenie_tunelu();
void jedz();


void zamelduj_opuszczenie_tunelu() {
  
  //printf("Dequeue: %d, Pociagow na torze: %d\n", tor, pamiec->ilePociagowNaTorze[tor]); 
  //dequeue(tor);
  
  int ret; 
  ret = kill(getppid(), SIGUSR1); // wysylam sygnal do zarzadcy ze wyjechalem wlasnie z tunelu
  
  while (ret < 0) {
    printf ("Niepowodzenie podczas wysylania sygnalu do zarzadcy! Powtarzam proces informowania \n");
    sleep(1); 
    ret = kill(getppid(), SIGUSR1); // wysylam sygnal do zarzadcy ze wyjechalem wlasnie z tunelu
  }
  
}

void jedz() {
  
  int czas_przejazdu; 
  
  if (priorytet == 1)
    czas_przejazdu = (int) DLUGOSC_TUNELU / EXPRESS_V; 
  if (priorytet == 2)
    czas_przejazdu = (int) DLUGOSC_TUNELU / TOWAR_V; 
  if (priorytet == 3)
    czas_przejazdu = (int) DLUGOSC_TUNELU / ZWYKLY_V; 
  
  printf("\x1b[31m *Pociag TLK%d jedzie przez tunel - 1 \x1b[0m\n", getpid()); 
  // t = s/v 
  //usleep(1000000) - 1sec
  usleep(1000000/5*2*czas_przejazdu); 
  printf("\x1b[32m ***->Wyjechalem z tunelu - meldunek od TLK%d \x1b[0m\n", getpid()); 
  
  zamelduj_opuszczenie_tunelu(); //test 
  
}

int main(int argc, char* argv[]) {
   
  
  // tworzymy obiekt pamięci wspolnej 
  tory_w_pamieci_wspolnej = shm_open("/pamiec", O_RDWR, 0777); 
  
  if (tory_w_pamieci_wspolnej < 0) {
    printf("Wystapil blad podczas otwierania pamięci wspólnej - pociagi.c \n"); 
    exit (-1);
  }
  
  ftruncate(tory_w_pamieci_wspolnej, sizeof(struct tory));
  
  //odwzorowujemy w pamieci procesu 
  oczekujace = mmap(NULL, sizeof(struct struktura_oczekujacych), PROT_READ | PROT_WRITE, MAP_SHARED, tory_w_pamieci_wspolnej, 0);

  if (oczekujace == MAP_FAILED) {
    printf("Wystąpil błąd podczas mapowania pamięci! \n"); 
    exit(-1); 
  }
  
  priorytet = 0; 
  priorytet = atoi(argv[0]); 
  
  tor = 0; 
  tor = atoi(argv[1]); 
  
  // otwieramy semafor
  char name[64]; 
  sprintf(name, "/tor_%d", tor); 
  
  semafor = sem_open(name, O_CREAT, 0644, 0); 
  
  if (semafor < 0) {
    printf("Wystąpił błąd podczas otwierania semafora! \n"); 
    exit (-1);
  }
  
  printf("Pociag: TLK%d, o priorytecie: %d oczekuje na wjazd do tunelu na torze: %d\n", getpid(), priorytet, tor); 

  sem_wait(semafor); 

  jedz(); 
  
  sem_close(semafor); 
 
  _exit(0);
  
}

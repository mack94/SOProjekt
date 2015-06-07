#include "header.h" 

int tor;
int priorytet;

sem_t *semafor; 

struct node *temp;
struct tory *stanTorow;  

struct struktura_oczekujacych *oczekujace; 
int tory_w_pamieci_wspolnej;


void dequeue(int numerToru) {
  

  
}

int frontElement(int numerToru) {
  

  return 0;
}

void zamelduj_opuszczenie_tunelu() {
  
  //printf("Dequeue: %d, Pociagow na torze: %d\n", tor, pamiec->ilePociagowNaTorze[tor]); 
  //dequeue(tor);
  
  int ret; 
  ret = kill(getppid(), SIGUSR1); // wysylam sygnal do zarzadcy ze wyjechalem wlasnie z tunelu
  
  //printf("ret: %d\n", ret); 
  
}

void jedz() {
  
  printf("Pociag TLK%d jedzie przez tunel - 1\n", getpid()); 
  sleep(1); 
  printf("Pociag TLK%d jedzie przez tunel - 2\n", getpid()); 
  sleep(1); 
  printf("Pociag TLK%d jedzie przez tunel - 3\n", getpid()); 
  sleep(1); 
  printf("Wyjechalem z tunelu - meldunek od TLK%d\n", getpid()); 
  
  zamelduj_opuszczenie_tunelu(); //test 
  
}

void oczekuj() {
  
}


int main(int argc, char* argv[]) {
   
  
  // tworzymy obiekt pamięci wspolnej 
  tory_w_pamieci_wspolnej = shm_open("/pamiec", O_RDWR, 0777); 
  ftruncate(tory_w_pamieci_wspolnej, sizeof(struct tory));
  
  //odwzorowujemy w pamieci procesu 
  oczekujace = mmap(NULL, sizeof(struct struktura_oczekujacych), PROT_READ | PROT_WRITE, MAP_SHARED, tory_w_pamieci_wspolnej, 0);

  
  if (oczekujace == MAP_FAILED) {
    printf("*****Some error in mmap occured******\n"); 
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
  
  printf("Pociag: TLK%d, o priorytecie: %d oczekuje na wjazd do tunelu na torze: %d\n", getpid(), priorytet, tor); 
      //printf("frontElement %d\n", oczekujace->front[0]);
      //printf("frontElement %d\n", oczekujace->front[1]);
      //printf("frontElement %d\n", oczekujace->front[2]);
      //printf("frontElement %d\n", oczekujace->front[3]);
      //printf("frontElement %d\n", oczekujace->front[4]);
      //printf("frontElement %d\n", oczekujace->front[5]);
      //printf("frontElement %d\n", oczekujace->front[6]);
      //printf("frontElement %d\n", oczekujace->front[7]);
      //printf("frontElement %d\n", oczekujace->front[8]);
      //printf("frontElement %d\n\n", oczekujace->front[9]);
  

  sem_wait(semafor); 

  jedz(); 

  oczekuj(); // test
  
  sem_close(semafor); 
  sem_unlink(name); 
  
  exit(0);
  
}

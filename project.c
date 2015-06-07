#include "header.h" 

int stopowa = 0; 

pthread_t thread1; 
pthread_t thread2; 

sem_t *semafor; 
sem_t *zbior_semaforow[LICZBA_TOROW]; 

struct sigaction sa; 

struct node *temp; 
struct tory *stanTorow;  

struct struktura_oczekujacych *oczekujace; 
int tory_w_pamieci_wspolnej;

void utworzKolejke() {
  
  stanTorow = malloc(sizeof(struct tory));
  
  int i;
  for(i = 0; i < LICZBA_TOROW; i++) {
    
    oczekujace->front[i] = 0;
    stanTorow->front[i] = stanTorow->rear[i] = NULL; 
    stanTorow->ilePociagowNaTorze[i] = 0; 
    
  }
  
}

void enqueue(pid_t nowyPociagID, int numerToru) {
  
  if (stanTorow->rear[numerToru] == NULL) {
    
    stanTorow->rear[numerToru] = (struct node*) malloc(1*sizeof(struct node)); 
    stanTorow->rear[numerToru]->ptr = NULL; 
    stanTorow->rear[numerToru]->pociagID = nowyPociagID; 
    stanTorow->front[numerToru] = stanTorow->rear[numerToru];
    oczekujace->front[numerToru] = nowyPociagID; 
    
  } else {
    
    temp = (struct node*) malloc(1*sizeof(struct node)); 
    stanTorow->rear[numerToru]->ptr = temp; 
    temp->pociagID = nowyPociagID;
    temp->ptr = NULL; 
    
    stanTorow->rear[numerToru] = temp;
    
  }
  
  stanTorow->ilePociagowNaTorze[numerToru]++; 
  
}

void dequeue(int numerToru) {
  
  temp = stanTorow->front[numerToru];
  
  if (temp == NULL) {
    
    return;
    
  } else {
    
    if (temp->ptr != NULL) {
      
      temp = temp->ptr;
      free(stanTorow->front[numerToru]);
      stanTorow->front[numerToru] = temp;
      oczekujace->front[numerToru] = temp->pociagID; 
      
    } else {
      
      free(stanTorow->front[numerToru]);
      stanTorow->front[numerToru] = NULL; 
      stanTorow->rear[numerToru] = NULL; 
      oczekujace->front[numerToru] = 0; 
      
    }
    
  }
  
  stanTorow->ilePociagowNaTorze[numerToru]--; 
  
}

int frontElement(int numerToru) {
  
  if ((stanTorow->front[numerToru] != NULL) && (stanTorow->rear[numerToru] != NULL))
    return (stanTorow->front[numerToru]->pociagID); 
  else
    return 0; 
  
}

void inicjalizujTory(int n) { // n - liczba torow do zainicjalizowania 
  // tory są reprezentowane tylko poprzez semafory i ich stan. 
  
  int i; 
  for(i = 0; i < n; i++) {
    
    // tory parzyste - poruszanie sie w kierunku + 
    // tory nieparzyste - poruszanie sie w kierunku - 
    printf("Inicjalizacja toru: %d\n", i); 
    
    char name[64]; 
    sprintf(name, "/tor_%d", i); 
    
    zbior_semaforow[i] = sem_open(name, O_CREAT, 0644, 0); 
    
    if (zbior_semaforow[i] == SEM_FAILED) {
      printf("************Blad podczas inicjalizowania semaforow - project.c**********\n"); 
    }
    
    printf("Zainicjalizowano tor: %d\n", i); 
    
  }
  
}

void generujPociag() {
  
  srand(time(NULL)); 
  
  while(true) {
    
    sleep((int) rand() % 2 + 1); 
    //TODO: to jednak tutaj musze losowac tor i priorytet 
    srand(time(NULL)); 
    pid_t pociagID; 
    int priorytet;
    int tor;
    char* temp; 
    char* dane[2]; // dane[0] - priorytet, dane[1] - tor 
    
    priorytet = (rand() % 3 + 1); 
    tor = (rand() % LICZBA_TOROW); 
    
    dane[0] = malloc(sizeof(char));
    dane[1] = malloc(sizeof(char));
    
    temp = malloc(sizeof(char));
    sprintf(temp, "%d", priorytet);
    dane[0] = temp; 

    temp = malloc(sizeof(char));
    sprintf(temp, "%d", tor);
    dane[1] = temp; 
      
    //printf("PRIOR: %s and TOR: %s\n", dane[0], dane[1]); 
    
    pociagID = fork(); // TODO: obsluga bledow
    
    if (pociagID != 0) { 	// jestesmy w procesie macierzystym
      
      enqueue(pociagID, tor); //TODO: na okreslony tor go ladowac a nie 5
      
      
    } else {		// uruchomiony proces potomny
      execvp("./pociag", dane); 
      abort(); 
    }
    
  }
  
}

void obsluz_sygnal(int signo, siginfo_t *siginfo, void *context) {
  
  printf("Dostalem sygnal SIGUSR1\n"); 
  
  stopowa = 0; 
  
//  pid_t pochodzenie; 
//  pochodzenie = siginfo->si_pid; 
  
//  kill(SIGKILL, pochodzenie); 
  
  return; 
  
}

void pusc_pociag() {
  
  int i;
  int wybranyTorDoPuszczenia = -1; 
  
  for(i = 0; i < LICZBA_TOROW; i++) {
    
    if (oczekujace->front[i] != 0)
      break; 
    
  }
  
  wybranyTorDoPuszczenia = i; 
  
  if (wybranyTorDoPuszczenia < 10 && wybranyTorDoPuszczenia >= 0) {
    
    printf("Wybrany: %d\n", wybranyTorDoPuszczenia);     
    sem_post(zbior_semaforow[wybranyTorDoPuszczenia]); 
    stopowa = 1;
    
    dequeue(wybranyTorDoPuszczenia); 
  }
  
  while(stopowa == 1) {
  
    
  }
  
  return; 
  
}

void zarzadzaj() {

      // obsluga sygnalu 
  sa.sa_handler = &obsluz_sygnal; 
    //sa.sa_flags = SA_SIGINFO;
  sigfillset(&sa.sa_mask); 
    
  sigaction(SIGUSR1, &sa, NULL);
  
  while(true) {
    
    pusc_pociag(); 
    
   sleep(1); 
  }

}

void posprzataj() {
  
  int i;
  for(i = 0; i < LICZBA_TOROW; i++) {
    
    char name[64]; 
    sprintf(name, "/tor_%d", i); 
    
    sem_close(zbior_semaforow[i]); 
    sem_unlink(name); 
      
  }
  
  //free(oczekujace);
  shm_unlink("/pamiec"); 
  
}

void wylacz() {
  exit(0); 
}

int main(int argc, char* argv[]) {
 
  atexit(posprzataj); 
  signal(SIGTSTP, wylacz);
  
  // tworzymy obiekt pamięci wspolnej 
  tory_w_pamieci_wspolnej = shm_open("/pamiec", O_RDWR | O_CREAT, 0644); 
  ftruncate(tory_w_pamieci_wspolnej, sizeof(struct tory)); 
  
  //odwzorowujemy w pamieci procesu 
  oczekujace = (struct tory *) mmap(NULL, sizeof(struct struktura_oczekujacych), PROT_READ | PROT_WRITE, MAP_SHARED, tory_w_pamieci_wspolnej, 0);
  
  utworzKolejke();
  
  inicjalizujTory(LICZBA_TOROW); 
  
  sleep(1);
  //TODO: obsluga bledow
  pthread_create(&thread1, NULL, zarzadzaj, NULL);
  pthread_create(&thread2, NULL, generujPociag, NULL); 
  
  pthread_join(thread1, NULL); 
  pthread_join(thread2, NULL); 
   
  return 0; 
}

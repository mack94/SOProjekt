#include "header.h" 

/*********************************************************************/

pthread_t thread1; 
pthread_t thread2; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

//sem_t *semafor;
sem_t *zbior_semaforow[LICZBA_TOROW]; 

struct sigaction sa; 

struct node *temp; 
struct tory *stanTorow;  

struct struktura_oczekujacych *oczekujace; 
int tory_w_pamieci_wspolnej;

/*********************************************************************/
void utworzKolejke();
void enqueue(pid_t nowyPociagID, int numerToru, int priorytet);
void dequeue(int numerToru);
int frontElement(int numerToru);
void inicjalizujTory(int n);
void generujPociag();
void obsluz_sygnal(int signo, siginfo_t *siginfo, void *context);
void pusc_pociag();
int ktory_pociag_wpuscic();
void zarzadzaj();
void posprzataj();
void wylacz();
/*********************************************************************/

void utworzKolejke() {
  
  stanTorow = malloc(sizeof(struct tory));
  
  int i;
  for(i = 0; i < LICZBA_TOROW; i++) {
    
    oczekujace->front[i] = 0;
    oczekujace->czas[i] = time(NULL);
    oczekujace->priorytet[i] = 0;
    stanTorow->front[i] = stanTorow->rear[i] = NULL; 
    stanTorow->ilePociagowNaTorze[i] = 0; 
    
  }
  
}

/*********************************************************************/
void enqueue(pid_t nowyPociagID, int numerToru, int priorytet) {
  
  time_t aktualny_czas;
  
  aktualny_czas = time(NULL); 
  
  if (stanTorow->rear[numerToru] == NULL) {
    
    stanTorow->rear[numerToru] = (struct node*) malloc(1*sizeof(struct node)); 
    stanTorow->rear[numerToru]->ptr = NULL; 
    stanTorow->rear[numerToru]->pociagID = nowyPociagID; 
    stanTorow->rear[numerToru]->priorytet = priorytet; 
    stanTorow->rear[numerToru]->czas = aktualny_czas; 
    stanTorow->front[numerToru] = stanTorow->rear[numerToru];
    oczekujace->front[numerToru] = nowyPociagID; 
    oczekujace->czas[numerToru] = aktualny_czas;
    oczekujace->priorytet[numerToru] = priorytet; 
    
  } else {
    
    temp = (struct node*) malloc(1*sizeof(struct node)); 
    stanTorow->rear[numerToru]->ptr = temp; 
    temp->pociagID = nowyPociagID;
    temp->priorytet = priorytet; 
    temp->czas = aktualny_czas;     
    temp->ptr = NULL; 
    
    stanTorow->rear[numerToru] = temp;
    
  }
  
  stanTorow->ilePociagowNaTorze[numerToru]++; 
  
}
/*********************************************************************/
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
      oczekujace->czas[numerToru] = temp->czas;
      oczekujace->priorytet[numerToru] = temp->priorytet; 
      //printf("=============1 : %d  <- na torze : %d \n ", temp->pociagID, numerToru);
      
    } else {
      
      free(stanTorow->front[numerToru]);
      stanTorow->front[numerToru] = NULL; 
      stanTorow->rear[numerToru] = NULL; 
      oczekujace->front[numerToru] = frontElement(numerToru); 
      oczekujace->czas[numerToru] = time(NULL);
      oczekujace->priorytet[numerToru] = 0;       
      //printf("=============2 : %d <- na torze : %d \n", frontElement(numerToru), numerToru);
      
    }
    
  }
  
  stanTorow->ilePociagowNaTorze[numerToru]--; 
  
}
/*********************************************************************/
int frontElement(int numerToru) {
  
  if ((stanTorow->front[numerToru] != NULL) && (stanTorow->rear[numerToru] != NULL))
    return (stanTorow->front[numerToru]->pociagID); 
  else
    return 0; 
  
}
/*********************************************************************/
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
      printf("Wystąpił błąd podczas inicjalizowania semaforow - project.c \n"); 
      exit (-1); 
    }
    
    printf("Zainicjalizowano tor: %d\n", i); 
    
  }
  
}
/*********************************************************************/
void generujPociag() {
  
  srand(time(NULL)); 
  
  while(true) {
    
    sleep((int) rand() % GEN_TIME + 1); 
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
      
      enqueue(pociagID, tor, priorytet); //TODO: na okreslony tor go ladowac a nie 5
      
      
    } else {		// uruchomiony proces potomny
      execvp("./pociag", dane); 
      abort(); 
    }
    
  }
  
}
/*********************************************************************/
void obsluz_sygnal(int signo, siginfo_t *siginfo, void *context) {

  //sem_post(semafor);
  pthread_mutex_unlock(&mutex);
  
  return; 
  
}
/*********************************************************************/
void pusc_pociag() {
  
  int wybranyTorDoPuszczenia = -1; 
  
  wybranyTorDoPuszczenia = ktory_pociag_wpuscic(); 
  
  //printf("WYBRANIEC: %d\n", wybranyTorDoPuszczenia); 
  
  if (wybranyTorDoPuszczenia < 10 && wybranyTorDoPuszczenia >= 0) {
    sem_post(zbior_semaforow[wybranyTorDoPuszczenia]); 
    //sem_wait(semafor); 
    pthread_mutex_lock(&mutex); 
    dequeue(wybranyTorDoPuszczenia); 
  }
  
  return; 
  
}
/*********************************************************************/
int ktory_pociag_wpuscic() {
  
  int i;
  double P = 0; 
  double P_temp = 0; 
  int priorytet;
  int numerToruZKtoregoPuszczamy = -1; 
  time_t aktualny_czas;
  double czas_oczekiwania; 
  
  aktualny_czas = time(NULL); 
  
  // P = czas_oczekiwania / priorytet -> max
  // czym wieksze jest P, tym bardziej powinnismy puscic pociag.  
  
  for(i = 0; i < LICZBA_TOROW; i++) {
    
    czas_oczekiwania = difftime(aktualny_czas, oczekujace->czas[i]); 
    priorytet = oczekujace->priorytet[i]; 
    
    if (priorytet == 0)
      continue; 
    
    P_temp = czas_oczekiwania / priorytet * 1.0;
    //printf("P_temp: %f\n", P_temp);
    
    if (P_temp > P) {
      
      P = P_temp; 
      numerToruZKtoregoPuszczamy = i; 
      
    }
    
  }
  
  return numerToruZKtoregoPuszczamy;
  
}
/*********************************************************************/
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
/*********************************************************************/
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
/*********************************************************************/
void wylacz() {
  exit(0); 
}
/*********************************************************************/
int main(int argc, char* argv[]) {
 
  atexit(posprzataj); 
  signal(SIGTSTP, wylacz);
  
  // tworzymy obiekt pamięci wspolnej 
  tory_w_pamieci_wspolnej = shm_open("/pamiec", O_RDWR | O_CREAT, 0644); 
  
  if (tory_w_pamieci_wspolnej < 0) {
    printf("Wystąpił błąd podczas tworzenia pamięci wspólnej! \n"); 
    exit (-1); 
  }
  
  ftruncate(tory_w_pamieci_wspolnej, sizeof(struct tory)); 
  
  //odwzorowujemy w pamieci procesu 
  oczekujace = mmap(NULL, sizeof(struct struktura_oczekujacych), PROT_READ | PROT_WRITE, MAP_SHARED, tory_w_pamieci_wspolnej, 0);

  if (oczekujace == MAP_FAILED) {
    printf("Wystąpil błąd podczas mapowania pamięci! \n"); 
    exit(-1); 
  }
  
  
  utworzKolejke();
  
  inicjalizujTory(LICZBA_TOROW); 
  
  // zainicjalizuj mutex, odpowiedzialny za oczekiwanie gdy pociąg jest w tunelu
  //semafor = sem_open("/syncsem0", O_CREAT, 0644, 1); // to mial byc semafor 'binarny'
  pthread_mutex_lock(&mutex); 
  
  sleep(1);
  //TODO: obsluga bledow
  pthread_create(&thread1, NULL, (void *) &zarzadzaj, NULL);
  pthread_create(&thread2, NULL, (void *)&generujPociag, NULL); 
  
  pthread_join(thread1, NULL); 
  pthread_join(thread2, NULL); 
   
  return 0; 
}

/*
    Prosty algorytm genetyczny do szukania optymalnych rozwi¹zañ wœród wektorów liczb rzeczywistych
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "alg_genetyczny.h"

/*
   losowanie liczby z rozkladu normalnego o zadanej sredniej i wariancji
*/
float Randn(float srednia,float wariancja)
{
  long liczba_iter = 10;  // im wiecej iteracji tym rozklad lepiej przyblizony
  float suma = 0;
  for (long i=0;i<liczba_iter;i++)
    suma += (float)rand()/RAND_MAX;
  return (suma-(float)liczba_iter/2)*sqrt(12*wariancja/liczba_iter) + srednia;
}



AlgGenetyczny::AlgGenetyczny(long _liczba_rozw, long _dlugosc_ciagu)
{
  liczba_rozw = _liczba_rozw;
  dlugosc_ciagu = _dlugosc_ciagu;

  popul_rozw = new float*[liczba_rozw];             // pamiêæ dla populacji
  for (long i=0;i<liczba_rozw;i++)
  {
    popul_rozw[i] = new float[dlugosc_ciagu];
    for (long j=0;j<dlugosc_ciagu;j++)
      popul_rozw[i][j] = Randn(0,1);               // losowanie parametrów ci¹gu w pocz¹tkowej populacji rozwi¹zañ
  }

  prawd_krzyz = 0.5;               // prawdopodobieñstwo krzy¿owania dwóch rozwi¹zañ
  prawd_mut = 0.05;                // prawdopodobieñstwo mutacji pojedynczego parametru rozwi¹zania
  wielkosc_mutacji = 0.3;          // zakres mutacji -> czy ma to byæ du¿a czy ma³a zmiana
  czy_max_przechodzi = 1;          // czy najlepsze rozwi¹zanie w populacji przechodzi bez zmian 
}
  
// UWAGA! Oceny powinny byæ dodatnie ( > 0) i tym wiêksze im lepsze rozwi¹zanie!
void AlgGenetyczny::epoka_uczenia(float *oceny_rozw)
{
  // szukanie najlepszego rozwi¹zania:
  float ocena_max = -1e10;
  long indeks_max = 0;
  for (int i=0;i<liczba_rozw;i++)
    if (ocena_max < oceny_rozw[i])
    {
      ocena_max = oceny_rozw[i];
      indeks_max = i;
    }


  // reprodukcja + selekcja -> by wybraæ wiêcej kopii lepszych rozwi¹zañ do kolejnej populacji, kosztem gorszych
  // tworzê tablicê skumulowanych wartoœci ocen:
  float *oceny_skum = new float[liczba_rozw];
  oceny_skum[0] = oceny_rozw[0];
  for (int i=1;i<liczba_rozw;i++)
    oceny_skum[i] = oceny_skum[i-1] + oceny_rozw[i];

  float **nowa_popul_rozw = new float*[liczba_rozw];            // pamiêæ dla nowej populacji
  for (int i=0;i<liczba_rozw;i++) nowa_popul_rozw[i] = new float[dlugosc_ciagu];

  for (int i=0;i<liczba_rozw;i++)
  {
    float los = (float)rand()/RAND_MAX *oceny_skum[liczba_rozw-1];
    long indeks = -1;

    if (los <= oceny_skum[0]) indeks = 0;
    else
    {
        long k,l = 0, p = liczba_rozw-1;

        // wyszukiwanie binarne miejsca na ruletce, w które trafi³ los:
        while (indeks == -1)
        {
          k = (l+p)/2;
          if (los > oceny_skum[k]) l = k+1;
          else p = k-1;
          if ((los <= oceny_skum[k])&&(los > oceny_skum[k-1]))
            indeks = k;
        }
    }
    // utworzenie nowej kopii rozwi¹zania i umieszczenie jej w nowej populacji:
    for (long j=0;j<dlugosc_ciagu;j++)
      nowa_popul_rozw[i][j] = popul_rozw[indeks][j];
  } // for po rozwi¹zaniach


  float **stara_popul_rozw = popul_rozw;
  popul_rozw = nowa_popul_rozw;                                 // nowa populacja zastêpuje star¹


  // krzy¿owanie -> wymiana wartoœci parametrów pomiêdzy dwoma losowymi rozwi¹zaniami:
  for (int i=0;i<liczba_rozw-1;i++)
  {
    float los = (float)rand()/RAND_MAX;             // losowanie czy rozwi¹zanie ma byæ skrzy¿owane z nastêpnym z tablicy

    if ((los < prawd_krzyz)&&(dlugosc_ciagu > 1))                     
    {
      long punkt_przec = 1+rand()%(dlugosc_ciagu-1); // losowanie punktu przeciêcia 
      for (long j=0;j<punkt_przec;j++)
      {
        float pom = popul_rozw[i][j];                // wymiana fragmentów ci¹gów parametrów 
        popul_rozw[i][j] = popul_rozw[i+1][j];
        popul_rozw[i+1][j] = pom;
      }
    }
  }

  
  // mutacja -> niewielka zmiana przypadkowych parametrów 
  for (int i=0;i<liczba_rozw-1;i++)
    for (long j=0;j<dlugosc_ciagu;j++)
    {
      bool czy_mut = ((float)rand()/RAND_MAX < prawd_mut);    // losowanie czy j-ty parametr i-tego rozwi¹zania ma byæ mutowany
      if (czy_mut)                                            // jeœli tak
      {
        int typ_mut = rand()%3;
        if (typ_mut == 0)                                    
          popul_rozw[i][j] *= 1+Randn(0,wielkosc_mutacji);    // to z pewnym prawdopodobieñstwem przemna¿am przez losow¹ wartoœæ
        else if (typ_mut == 1)
          popul_rozw[i][j] /= 1+Randn(0,wielkosc_mutacji);    // dzielê
        else
          popul_rozw[i][j] += Randn(0,wielkosc_mutacji);      // lub dodajê losow¹ wartoœæ
      }
    }


  // tzw. model elitarny -> najlepsze rozwi¹zanie przechodzi do populacji potomnej bez zmian, tzn.
  // z pominiêciem krzy¿owania i mutacji
  if (czy_max_przechodzi) 
    for (long j=0;j<dlugosc_ciagu;j++)
      popul_rozw[0][j] = stara_popul_rozw[indeks_max][j];
  

  for (int i=0;i<liczba_rozw;i++) delete stara_popul_rozw[i];   // usuniêcie starej populacji
  delete stara_popul_rozw;
  delete oceny_skum;
}
      
      
/*
   Funkcja interpretuj¹ca wektor liczb rzeczywistych jako parametry (wagi) sieci neuronowej
   obliczajacej wyjœcia na podstawie wejœæ.
   Uwagi:
   - wyjœæ musi byæ tyle, ile neuronów w ostatniej warstwie 
     (tzn. = liczby_neuronow[liczba_warstw-1], pamiêæ trzeba zarezerwowaæ wczeœniej)
   - liczba wag musi odpowiadaæ liczbie wag wynikaj¹cej ze struktury sieci. Mo¿na
     j¹ wczeœniej wyznaczyæ za pomoc¹ funkcji liczba_wag_sieci_neuronowej()
   - tablica liczby_neuronow[] nie zawiera liczby wejœæ (wejœcia nie s¹ traktowane jako neurony
*/
void aproks_neuronowa(float *wyjscia, float *wejscia, int liczba_wejsc, float *wagi, 
                      int *liczby_neuronow, int liczba_warstw_neuronow)
{
  float **y = new float*[liczba_warstw_neuronow];    // pamiêæ dla sygna³ów wyjœciowych neuronów 
  for (int i=0;i<liczba_warstw_neuronow;i++) y[i] = new float[liczby_neuronow[i]];
  long n=0;                                 // licznik wag

  for (int i=0;i<liczba_warstw_neuronow;i++)
  {
    for (int j=0;j<liczby_neuronow[i];j++)
    {
      y[i][j] = 0;                          // wyjœcie j-tego neurony w i-tej warstwie
      if (i==0)                             // dla pierwszej warstwy neuronów
        for (int k=0;k<liczba_wejsc;k++) y[i][j] += wagi[n++]*wejscia[k];     
      else                                  // dla kolejnych warstw
        for (int k=0;k<liczby_neuronow[i-1];k++) y[i][j] += wagi[n++]*y[i-1][k];
      y[i][j] += wagi[n++];                 // wartoœæ progowa - wyraz wolny
      
      if (i < liczba_warstw_neuronow-1)     // zastosowanie funkcji aktywacji tylko w przypadku wartstw ukrytych 
        y[i][j] = 1/(1 + exp(-y[i][j]));    // funkcja ci¹g³a - sigmoidalna
        //y[i][j] = (y[i][j] > 0 ? 1 : 0);    // funkcja progowa - o ostrym skoku  

      if (i == liczba_warstw_neuronow-1) 
        wyjscia[j] = y[i][j];               // wyjœcia z ostatniej warstwy traktowane s¹ jako wyjœcia sieci
    }
  }

  for (int i=0;i<liczba_warstw_neuronow;i++) delete y[i];
  delete y;
}

/*
   Funkcja zwraca liczbê wag - parametrów sieci neuronowej o podanej na wejœciu strukturze
*/
long liczba_wag_sieci_neuronowej(int liczba_wejsc,int *liczby_neuronow, int liczba_warstw_neuronow)
{
  long liczba = (liczba_wejsc+1)*liczby_neuronow[0];
  for (int i=1;i<liczba_warstw_neuronow;i++)
    liczba += (liczby_neuronow[i-1]+1)*liczby_neuronow[i];
  
  return liczba;
}




// Szukanie minimum funkcji za pomoc¹ algorytmu genetycznego - dla sprawdzenia
void uczenieAG3()
{
  FILE *f = fopen("szukanie_minimum_funkcji.txt","w");
  long liczba_epok = 20000;

  const long liczba_osobnikow = 130;
  const long dlugosc_ciagu = 2;

  AlgGenetyczny ag(liczba_osobnikow,dlugosc_ciagu);

  float oceny[liczba_osobnikow];
  float ocena_max = -1e10;

  printf("ALGORYTM GENETYCZNY\n");
  fprintf(f,"ALGORYTM GENETYCZNY\n");
  printf("Szukanie minimum funkcji f(x,y) = sin(x*3)/(abs(x)+1) + sin(y*5-1)/(abs(y/2-1)+1) + ((x-5)^2+(y-5)^2)/50\n");
  printf("liczba_epok = %d, liczba osobnikow = %d\n",liczba_epok, liczba_osobnikow);
  fprintf(f,"Szukanie minimum funkcji f(x,y) = sin(x*3)/(abs(x)+1) + sin(y*5-1)/(abs(y/2-1)+1) + ((x-5)^2+(y-5)^2)/50\n");
  fprintf(f,"liczba_epok = %d, liczba osobnikow = %d\n",liczba_epok, liczba_osobnikow);

  for (long ep=0;ep<liczba_epok;ep++)
  {
    // ocena poszczegolnych rozwiazan z rozszerzonej populacji (N osobnikow pop.podst. + M potomkow)
    for (long oso=0;oso<liczba_osobnikow;oso++)
    {
      float *ciag = ag.popul_rozw[oso];
      float x = ciag[0], y = ciag[1];
	    float fun2 = (sin(x*3)/(fabs(x)+1) + sin(y*5-1)/(fabs(y/2-1)+1) + ((x-5)*(x-5)+(y-5)*(y-5))/50);
      float ocena = 2 - fun2;
      if (ocena < 0) ocena = 0.00001;
      oceny[oso] = ocena;

      if (ocena_max < ocena)
      {
        ocena_max = ocena;
        printf("ep = %d, Rekord, ocena = %f, fun2 = %f dla ciagu = (%f, %f)\n",ep,ocena_max,fun2,x,y);
        fprintf(f,"ep = %d, Rekord, ocena = %f, fun2 = %f dla ciagu = (%f, %f)\n",ep,ocena_max,fun2,x,y);
      }
    }

    ag.epoka_uczenia(oceny);
  } // for po epokach
  fclose(f);
}

// przyk³ad genetycznego doboru wag sztucznej sieci neuronowej
void uczenieAG_SSN()
{
  FILE *f = fopen("aproksymacja_funkcji_dwoch_zmiennych.txt","w");
  long liczba_epok = 20000;

  const long liczba_osobnikow = 130;
  const int liczba_wejsc = 2;
  int liczby_neuronow[] = {10, 5, 1};
  int liczba_warstw = sizeof(liczby_neuronow)/sizeof(int);
  const long dlugosc_ciagu = liczba_wag_sieci_neuronowej(liczba_wejsc,liczby_neuronow,liczba_warstw);


  AlgGenetyczny ag(liczba_osobnikow,dlugosc_ciagu);

  float oceny[liczba_osobnikow];
  float ocena_max = -1e10;
  float *wejscia = new float[liczba_wejsc], wyjscie; 

  printf("ALGORYTM GENETYCZNY DO DOBORU WAG SSN\n");
  fprintf(f,"ALGORYTM GENETYCZNY DO DOBORU WAG SSN\n");
  printf("Aproksymacja funkcji f(x,y) = sin(x*3)/(abs(x)+1) + sin(y*5-1)/(abs(y/2-1)+1) + ((x-5)^2+(y-5)^2)/50\n");
  printf("liczba_epok = %d, liczba osobnikow = %d, liczba wag = %d\n",liczba_epok, liczba_osobnikow,dlugosc_ciagu);
  fprintf(f,"Aproksymacja funkcji f(x,y) = sin(x*3)/(abs(x)+1) + sin(y*5-1)/(abs(y/2-1)+1) + ((x-5)^2+(y-5)^2)/50\n");
  fprintf(f,"liczba_epok = %d, liczba osobnikow = %d, liczba wag = %d\n",liczba_epok, liczba_osobnikow,dlugosc_ciagu);

  for (long ep=0;ep<liczba_epok;ep++)
  {
    // ocena poszczegolnych rozwiazan z rozszerzonej populacji (N osobnikow pop.podst. + M potomkow)
    for (long oso=0;oso<liczba_osobnikow;oso++)
    {
      float *ciag = ag.popul_rozw[oso];
      float suma_roznic = 0;              // suma ró¿nic wartoœci funkcji i aproksymacji 
      long liczba_punktow = 0;
      for (float x=-5; x < 5; x += 0.2)      // dla siatki punktów
        for (float y=-5; y < 5; y += 0.2)
        {
          float fun2 = (sin(x*3)/(fabs(x)+1) + sin(y*5-1)/(fabs(y/2-1)+1) + ((x-5)*(x-5)+(y-5)*(y-5))/50);
          wejscia[0] = x;
          wejscia[1] = y;

          aproks_neuronowa(&wyjscie, wejscia, liczba_wejsc, ciag, 
                      liczby_neuronow, liczba_warstw);

          suma_roznic += fabs(wyjscie - fun2);           
          liczba_punktow++;
        }
	    float sr_roznica = suma_roznic/liczba_punktow; 
      oceny[oso] = 1/(sr_roznica*sr_roznica + 0.05);

      if (ocena_max < oceny[oso])
      {
        ocena_max = oceny[oso];
        printf("ep = %d, Rekord, ocena = %f, srednia roznica = %f\n",ep,ocena_max,suma_roznic/liczba_punktow);
        fprintf(f,"ep = %d, Rekord, ocena = %f, srednia_roznica = %f\n",ep,ocena_max,suma_roznic/liczba_punktow);
      }
    } // po osobnikach - rozwi¹zaniach

    ag.epoka_uczenia(oceny);
  } // for po epokach
  fclose(f);
}



import java.util.Random;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author J
 */
public class Srodowisko {

   // tablica pol:
   //  0 - puste
   //  1 - przeszkoda
   //  2 - nagroda
   //  3 - kara
   //  4 - biegun

   /*int pola[][] = {{0,0,1,0,0,0,0,2},
                   {0,0,1,1,0,1,1,2},
                   {0,0,0,0,0,0,1,2},
                   {0,0,1,1,1,0,1,0},
                   {0,0,1,0,0,0,0,2} };*/
   /*int pola[][] = {{0,0,0,0,0,0,0,0,2},
                   {0,0,0,1,1,0,1,1,2},
                   {0,0,0,0,0,0,0,3,2},
                   {0,0,0,1,1,1,0,1,0},
                   {0,0,0,0,0,0,0,0,2} };*/
   /*int pola[][] = {{0,0,0,0,0,0,0,0,0,2},
                   {0,0,0,0,1,1,1,1,1,2},
                   {0,0,0,0,0,0,0,0,0,2},
                   {0,0,0,4,1,1,1,1,1,2},
                   {0,0,0,4,0,0,0,0,0,0},
                   {0,0,0,0,1,1,1,1,1,2},
                  {0,0,0,0,0,0,0,0,0,2} }; */
   /*int pola[][] = {{0,0,1,4,4,3,0,0,0,0,0,0,0,0,0,2},
                   {0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,2},
                   {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,2},
                   {0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,2},
                   {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
                   {0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,2},
                   {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,2} };*/
   /*int pola[][] = {{0,0,0,0,0,3,0,2},
                   {0,0,4,4,0,0,0,2},
                   {0,0,3,3,0,0,0,2},
                   {0,0,0,0,0,0,0,0},
                   {0,0,0,0,0,0,0,0} };*/
   int pola[][] = {{0,0,1,2,0,0,0,0,0,0,0,4,0},  // są dwie drogi do nagrody. W górnej jest niebezpieczeńswo kary, więc agent uczony w środowisku
                   {0,0,4,1,1,1,1,1,1,0,0,4,0},  // jednoagentowym zwyklę ją omija. Gdy podczas uczenia występują interakcje z innymi agentami,
                   {0,0,0,0,0,0,0,0,0,0,0,0,0},  // agenty uczone powinny podzielić się na dwie grupy - dolną i górną, jako że kolizje z innymi
                   {0,0,0,0,0,0,1,0,1,1,0,3,0},  // agentami też kosztują
                   {0,0,0,0,0,0,0,0,0,0,0,0,0},
                   {0,0,0,1,1,1,1,1,1,0,0,0,0},
                   {0,0,1,2,0,0,0,0,0,0,0,0,0}};
   /*int pola[][] = {{0,0,1,0,0,0,4,1,0,2},  // którą drogę (dolną czy górną) wybierze agent w zal. od stopnia losowości (proc_wbok, proc_wtyl)?
                   {0,0,1,0,1,0,4,1,0,2},
                   {0,0,1,0,1,0,0,1,0,0},
                   {0,0,1,0,1,0,0,0,0,0},
                   {0,0,0,0,1,4,4,0,1,0},
                   {0,0,1,0,3,3,3,0,3,2},
                   {0,0,0,0,0,0,0,0,0,2} };*/
   /*int pola[][] = {{0,0,0,0,0,0,0,0,0,3,3,0,0,0,2},
                   {0,3,0,0,4,1,0,0,0,0,0,0,0,3,2},
                   {0,0,0,0,4,1,0,0,0,1,0,0,0,0,2},
                   {0,0,0,0,1,0,0,0,0,0,1,0,0,0,2},
                   {0,0,0,1,0,0,1,1,0,0,0,0,0,0,2},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
                   {0,0,0,1,1,1,1,3,0,0,0,0,0,0,0},
                   {0,0,0,0,0,0,0,0,0,3,0,0,0,0,0},
                   {0,0,3,3,0,0,0,0,0,1,1,1,0,4,0},
                   {0,0,4,4,0,0,0,0,0,0,0,0,0,4,0}};*/
  
   int liczba_wierszy = pola.length;
   int liczba_kolumn = pola[0].length;
   int proc_wbok = 10;     // prawdopodobienstwo w [%] pojscia w bok od wybranego kierunku
   int proc_wtyl = 5;      // -||-  pojscia w tyl od wybranego kierunku
   int wartosc_nagrody_stalej = 40;
   int wartosc_nagrody_za_zamkniecie = 1000;  // wartość nagrody za zamknięcie obwodu
   int wartosc_kary_za_krok = -1;
   int wartosc_kary_za_kolizje_ze_sciana = -3;
   int wartosc_kary_za_kolizje_z_agentem = -15;
   int wartosc_kary = -60;    // kara za wjechanie na czerwone pole


   // Funkcja modelujaca ruch zbioru obiektow nalezacych do pewnego agenta
   // w srodowisku: na podstawie
   // aktualnego stanu i akcji agenta (ruch zbioru obiektow) srodowisko zwraca
   // nowy stan w postaci nowych polozen obiektow i nagrod: 
   // {{x1,y1,r1},{x2,y2,r2},...,{xn,yn,rn}}
   // na wejscu: akcja w postaci tablicy posuniec obiektow:
   // numery akcji: 0 - brak ruchu, 1 - w prawo, 2 - w dół, 3 - w lewo, 4 - do góry,
   int[] ruch_agenta(int stan[], int akcja, int stany_agentow[][], int liczba_agentow)
   {
       int[] stan_i_nagroda = new int[3];
       Random loteria=new Random();
      
       int nowy_stan[] = new int[2];
     
       // losowanie przejścia docelowego:
       // (agent moze pojsc przypadkowo w innym kierunku niz zamierzal)
       int akcja_rzecz = akcja;
       int proc_los = loteria.nextInt(100);
       if (proc_los < proc_wtyl)
       {
           if (akcja==1) akcja_rzecz = 3;
           else if (akcja==2) akcja_rzecz = 4;
           else if (akcja==3) akcja_rzecz = 1;
           else if (akcja==4) akcja_rzecz = 2;
       }
       else if (proc_los < proc_wbok)
       {
           int lewo = loteria.nextInt(2);
           if (akcja==1) akcja_rzecz = (lewo == 0 ? 2 : 4);
           else if (akcja==2) akcja_rzecz = (lewo == 0 ? 3 : 1);
           else if (akcja==3) akcja_rzecz = (lewo == 0 ? 4 : 2);
           else if (akcja==4) akcja_rzecz = (lewo == 0 ? 1 : 3);
       }

       // wykonanie akcji -> przejscie do nowego stanu
       int wiersz = stan[0], kolumna = stan[1];

       if (akcja_rzecz == 1) kolumna++;                 // w prawo
       else if (akcja_rzecz == 2) wiersz++;             // do dołu
       else if (akcja_rzecz == 3) kolumna--;            // w lewo
       else if (akcja_rzecz == 4) wiersz--;             // w górę
       if ((wiersz >=0)&&(wiersz < liczba_wierszy)&&    // sprawdzenie czy nie wychodzi poza bande
           (kolumna >=0)&&(kolumna < liczba_kolumn)&&
           (pola[wiersz][kolumna] != 1))                // sprawdzenie czy w polu nie ma sciany
       {
         nowy_stan[0] = wiersz;
         nowy_stan[1] = kolumna;      
       }
       else            // cofnięcie do stanu poprzedniego
       {
         nowy_stan[0] = stan[0];
         nowy_stan[1] = stan[1];
         stan_i_nagroda[2] += wartosc_kary_za_kolizje_ze_sciana;
       }


       // sprawdzenie czy nie ma kolizji z innym agentem:
       for (int i=0;i<liczba_agentow;i++)
           if ((stany_agentow[i][0] != stan[0])||(stany_agentow[i][1] != stan[1])) // jeśli nie jest to nasz agent
           {
               if ((stany_agentow[i][0] == nowy_stan[0])&&(stany_agentow[i][1] == nowy_stan[1])) // jeśli kolizja
               {
                   nowy_stan[0] = stan[0];           // cofnięcie do stanu poprzedniego
                   nowy_stan[1] = stan[1];
                   stan_i_nagroda[2] += wartosc_kary_za_kolizje_z_agentem;
                   break;
               }
           }
     

       // nagroda lub kara za wykonana akcje oraz przejscie do nowego stanu:
       if (pola[nowy_stan[0]][nowy_stan[1]] == 2) stan_i_nagroda[2] = wartosc_nagrody_stalej;
       else if (pola[nowy_stan[0]][nowy_stan[1]]==3) stan_i_nagroda[2] = wartosc_kary;
       stan_i_nagroda[2] += wartosc_kary_za_krok;

       // nagroda za zamknięcie obwodu (jeśli dwóch agentów zajmie dwa sąsiednie bieguny):
       if (pola[nowy_stan[0]][nowy_stan[1]] == 4)
       {
           for (int i=0;i<liczba_agentow;i++)
               if (((stany_agentow[i][0] != stan[0])||(stany_agentow[i][1] != stan[1]))&& // jeśli nie jest to ten agent
               (pola[stany_agentow[i][0]][stany_agentow[i][1]] == 4)&&
               (Math.abs(stany_agentow[i][0]-nowy_stan[0])+Math.abs(stany_agentow[i][1]-nowy_stan[1]) == 1))
                   stan_i_nagroda[2] += wartosc_nagrody_za_zamkniecie;
       }


       // sprawdzenie czy obiekt agenta nie znajduje się w ostatniej kolumnie lub zdobył nagrodę. 
       // Jeśli tak, to przejście do pierwszej kolumny
       if ((nowy_stan[1] >= liczba_kolumn-1)||(stan_i_nagroda[2] > 0))
       {
           nowy_stan[1] = 0;                                // przejscie na poczatek planszy
           nowy_stan[0] = loteria.nextInt(liczba_wierszy);  // do losowego wiersza
       }

       stan_i_nagroda[0] = nowy_stan[0];
       stan_i_nagroda[1] = nowy_stan[1];


       return stan_i_nagroda;
   }

   int[] losowanie_stanu_pocz(int stany_innych[][], int liczba_innych)
   {
       // wybor stanow poczatkowych obiektow (tak by nie kolidowaly z innymi obiektami
        // agenta naszego i obcych agentow:
        int puste_pola[] = new int[liczba_wierszy];
        int liczba_pustych_pol = 0;
        int agenty_w_kol0[] = new int[liczba_wierszy];
        for (int i=0;i<liczba_wierszy;i++) agenty_w_kol0[i] = 0;   // wyzerowanie tablicy zajętości pól pierwszej kolumny
        for (int i=0;i<liczba_innych;i++) 
            if (stany_innych[i][1] == 0) agenty_w_kol0[stany_innych[i][0]] = i;   // umieszczenie numeru agenta znajdujacego się w pierwszej kolumnie
        for (int i=0;i<liczba_wierszy;i++)                         // utowrzenie tablicy pustych pól w pierwszej kolum,nie
            if ((pola[i][0] != 1)&&(agenty_w_kol0[i] == 0))        // jeśli nie ma w polu i przeszkody ani innego agenta         
            {
              puste_pola[liczba_pustych_pol] = i;
              liczba_pustych_pol++;
            }
        
        int stan[] = new int[2];
        if (liczba_pustych_pol == 0) {   // mie da się umieścic agenta  
            stan[0] = -1;
            stan[1] = -1;
        }
        else
        {
            Random loteria = new Random();
            stan[0] = loteria.nextInt(liczba_pustych_pol);
            stan[1] = 0;
        }

        return stan;
   }


   // Ocena  strategii
   double ocen_strategie(int[][] strategia, int liczba_epizodow, int liczba_krokow_max)
   {
      int stan[] = new int[2];   // stan agenta (numer wiersza i kolumny)
      int liczba_innych_agentow = 0;
      int polozenia_innych[][] = new int[liczba_innych_agentow][2]; // położenia innych agentów

      Random loteria = new Random();

      float suma_nagrod = 0;
      for (int i = 0;i<liczba_epizodow;i++)
      {
          // losowanie stanu początkowego -> położenie agenta w pierwszej kolumnnie
          stan[0] = loteria.nextInt(liczba_wierszy);
          stan[1] = 0;
          int nr_kroku = 0;

          while ((stan[1] != liczba_kolumn-1)&&(nr_kroku < liczba_krokow_max))
          {
              int akcja = strategia[stan[0]][stan[1]];

              //int[] stan_i_nagroda = s.ruch_agenta(stan,akcja,stany_obcych_agentow,liczba_obcych_agentow);
              int[] stan_i_nagroda = ruch_agenta(stan,akcja,polozenia_innych,0);

              stan[0] = stan_i_nagroda[0];
              stan[1] = stan_i_nagroda[1];
              suma_nagrod += stan_i_nagroda[2];

              nr_kroku ++;
          }
      }
      return  suma_nagrod/liczba_epizodow;
  }  // koniec funkcji oceny strategii
}

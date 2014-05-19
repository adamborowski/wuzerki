
import java.util.Random;
import java.lang.Math.*;
import java.io.*;
/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author J
 */
public class AlgGenetyczny {
  int liczba_rozw = 100;                    // liczba rozwiązań w populacji
  double prawd_krzyz = 0.3;           // prawdopodobieństwo krzyżowania dwóch rozwiązań
  double prawd_mut_rozw = 0.2;            // prawdopodobieństwo mutacji rozwiązania
  double prawd_mut_par = 0.15;            // prawdopodobieństwo mutacji każdego parametru rozwiązania
  double wsp_selekcji = 1;            // współczynnik nacisku selekcyjnego: im większy, tym więcej kopii lepszych kosztem gorszych
  int czy_max_przechodzi = 1;         // czy najlepsze rozwiązanie przechodzi do następnego pokolenia bez zmian



  int liczba_epizodow_oceny = 200;     // liczba epizodów (przejść labiryntu), na podstawie których obliczana jest ocena

  Srodowisko s = new Srodowisko();    // środowisko obliczające nagrody oraz wyznaczające nowy stan

  int liczba_krokow_max = (s.liczba_kolumn + s.liczba_wierszy)*4;   // maks. liczba kroków w każdym epizodzie po to, by nie tracić czasu na
                                                                    // bezsensowne błądzenie agenta, gdy jeszcze nie umie chodzić
  Rozw popul_rozw[] = new Rozw[liczba_rozw];             // populacja rozwiązań
  double fittness[] = new double[liczba_rozw];           // wartości przystosowania poszczególnych rozwiązań w populacji 
  
  Rozw rozw_max = new Rozw();                            // najlepsze rozwiązanie w całej ewolucji

  int liczba_epok = 0;

  Random loteria = new Random();                   // generator liczb losowych
  int id_rozw = 0;                                 // unikatowy numer rozwiązania
  File plik = new File("ag_log.txt");
  FileWriter fos;// = new FileOutputStream("test.txt");

  AlgGenetyczny()
  {
      System.out.println("konstr AlgGenetyczny");
      
      /*try{
          fos = new FileWriter("test.txt");
      }
      catch(IOException ex){
          ex.printStackTrace();
      }*/
      for (int i=0;i<liczba_rozw;i++) popul_rozw[i] = new Rozw();


      rozw_max.ocena = -1e10;
  }
  
  void epoka_uczenia()
  {
      
      for (int i=0;i<liczba_rozw;i++)
          if (popul_rozw[i].czy_oceniony == 0)
          {
              popul_rozw[i].ocena = s.ocen_strategie(popul_rozw[i].strategia,
                      liczba_epizodow_oceny,liczba_krokow_max);  // wyznaczenie oceny dla każdego rozwiązania
              popul_rozw[i].czy_oceniony = 1;
              //System.out.println("oceniono rozwiazanie "+i+" o id = "+popul_rozw[i].id+" ocena = "+popul_rozw[i].ocena);
          }
      
      // obliczenie przystosowania - fittness, tak by każda wartość przystosowania była >= 0 i tym większa,
      // im rozwiązanie lepsze
      double ocena_min = 1e10, ocena_max = -1e10, ocena_sr = 0;
      int indeks_max = -1;                                // indeks rozwiązania o najwyższej ocenie
      for (int i=0;i<liczba_rozw;i++)
      {
          if (ocena_min > popul_rozw[i].ocena) 
              ocena_min = popul_rozw[i].ocena;
          if (ocena_max < popul_rozw[i].ocena) 
          {
              ocena_max = popul_rozw[i].ocena;
              indeks_max = i;
              if (rozw_max.ocena < ocena_max)            // sprawdzenie czy znaleziono nowy rekord
              {
                  rozw_max.kopiuj_obiekt(popul_rozw[i]);              // kopiuje rozwiązanie do rozw. najlepszego
                  System.out.println("nowy rekord id = "+rozw_max.id+ " w ep."+liczba_epok+", sr. nagroda = "+ocena_max);
                  for (int p=0;p < s.liczba_wierszy;p++)
                  {
                      for (int q=0;q<s.liczba_kolumn;q++)
                          System.out.print(" "+rozw_max.strategia[p][q]);
                      System.out.println("");
                  }
              }
          }
      }
            
      for (int i=0;i<liczba_rozw;i++)
          fittness[i] = Math.pow(popul_rozw[i].ocena - ocena_min, wsp_selekcji);        // obliczenie wartości przystosowania
      
      reprodukcja();
      krzyzowanie();
      mutacja();
      
      if (czy_max_przechodzi == 1)              // model elitarny - najlepsze rozwiązanie przechodzi do nast. pokolenia bez zmian
          popul_rozw[0] = rozw_max;
      

      //System.out.println("epoka = "+liczba_epok);
      liczba_epok++;
  }
  
  void reprodukcja()
  {
      // tworzę tablicę skumulowanych wartości przystosowania:
      double fittness_skum[] = new double[liczba_rozw];
      fittness_skum[0] = fittness[0];
      for (int i=1;i<liczba_rozw;i++)
          fittness_skum[i] = fittness_skum[i-1] + fittness[i];

      Rozw nowa_popul_rozw[] = new Rozw[liczba_rozw];
      for (int i=0;i<liczba_rozw;i++) nowa_popul_rozw[i] = new Rozw();

      for (int i=0;i<liczba_rozw;i++)
      {
          double los = loteria.nextFloat()*fittness_skum[liczba_rozw-1];
          int indeks = -1;
          //System.out.printf("los = %f, fit_min = %f, fit_max = %f\n",los,fittness_skum[0],fittness_skum[liczba_rozw-1]);

          if (los <= fittness_skum[0]) indeks = 0;
          else
          {
              int k,l = 0, p = liczba_rozw-1;

              // wyszukiwanie binarne miejsca na ruletce:
              while (indeks == -1)
              {
                k = (l+p)/2;
                if (los > fittness_skum[k]) l = k+1;
                else p = k-1;
                //System.out.printf("l = %d, p = %d, k = %d, \n",l,p,k);
                if ((los <= fittness_skum[k])&&(los > fittness_skum[k-1]))
                    indeks = k;
              }
          }
          // utworzenie nowej kopii rozwiązania i umieszczenie jej w nowej populacji:
          nowa_popul_rozw[i].kopiuj_obiekt(popul_rozw[indeks]);
      } // for po rozwiązaniach

      popul_rozw = nowa_popul_rozw;           // nowa populacja zastępuje starą
  }
  
  void krzyzowanie()
  {
      for (int i=0;i<liczba_rozw;i++)
      {
          if (loteria.nextFloat() < prawd_krzyz)
          {
              int drugi = loteria.nextInt(liczba_rozw);      // losowanie drugiego rozwiązania
              int czy_zam_wierszy = loteria.nextInt(2);      // czy zamiana wierszy ( w przeciwnym wypadku kolumn)
              if (czy_zam_wierszy == 1) {
                  int miejsce_przec = 1 + loteria.nextInt(s.liczba_wierszy-1);
                  for (int j=0;j<miejsce_przec;j++)
                      for (int k=0;k<s.liczba_kolumn;k++){
                          int pom = popul_rozw[i].strategia[j][k];
                          popul_rozw[i].strategia[j][k] = popul_rozw[drugi].strategia[j][k];
                          popul_rozw[drugi].strategia[j][k] = pom;
                      }
              }
              else
              {
                  int miejsce_przec = 1 + loteria.nextInt(s.liczba_kolumn-1);
                  for (int j=0;j<miejsce_przec;j++)
                      for (int k=0;k<s.liczba_wierszy;k++){
                          int pom = popul_rozw[i].strategia[k][j];
                          popul_rozw[i].strategia[k][j] = popul_rozw[drugi].strategia[k][j];
                          popul_rozw[drugi].strategia[k][j] = pom;
                      }
              }


              popul_rozw[i].czy_oceniony = 0;
              popul_rozw[drugi].czy_oceniony = 0;
              popul_rozw[i].id = id_rozw; id_rozw++;
              popul_rozw[drugi].id = id_rozw; id_rozw++;
          }
      }
  }
  
  void mutacja()
  {
      for (int i=0;i<liczba_rozw;i++)
      {
          if (loteria.nextFloat() < prawd_mut_rozw)
          {
              for (int j=0;j<s.liczba_wierszy;j++)
                  for (int k=0;k<s.liczba_kolumn;k++)
                      if (loteria.nextFloat() < prawd_mut_par)
                          popul_rozw[i].strategia[j][k] = 1+loteria.nextInt(4);       // losuję numer akcji
              popul_rozw[i].czy_oceniony = 0;
              popul_rozw[i].id = id_rozw; id_rozw++;
          }
      }
  }
  
  // klasa rozwiązania - strategii i związane z nią metody - np. obliczania oceny
  class Rozw implements Cloneable {
      double ocena;            // ocena
      int czy_oceniony;       // informacja czy rozwiązanie było już ocenianiane
      int strategia[][];          // numery akcji w każdym ze stanów (każdy stan to położenie na planszy 2D)
      int id;

      Rozw()
      {
          //System.out.println("konstr Rozw");
          Random loteria = new Random();                   // generator liczb losowych
          strategia = new int[s.liczba_wierszy][s.liczba_kolumn];
          for (int i = 0;i<s.liczba_wierszy;i++)      // losowanie strategii początkowej
              for (int j = 0;j<s.liczba_kolumn;j++)
                  strategia[i][j] = 1+loteria.nextInt(4);
          czy_oceniony = 0;
          id = id_rozw; id_rozw++;
      }

      

      void kopiuj_obiekt(Rozw wzor)
      {
          for (int i = 0;i<s.liczba_wierszy;i++)      // losowanie strategii początkowej
              for (int j = 0;j<s.liczba_kolumn;j++)
                  strategia[i][j] = wzor.strategia[i][j];
          //System.arraycopy(s, id, s, id, id)
          czy_oceniony = wzor.czy_oceniony;
          id = wzor.id;
          ocena = wzor.ocena;
      }
  } // koniec klasy wewnętrznej Rozw (rozwiązanie)


}

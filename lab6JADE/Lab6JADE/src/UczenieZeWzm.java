import java.util.Random;
import java.lang.Math.*;
import java.io.*;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author 
 */
public class UczenieZeWzm {
    //long liczba_epizodow = 1000000;            // liczba epizodow uczenia
    //long co_ile_test = 10000;
    //long liczba_epiz_testu = 500;

    double alfa = 0.1;                // szybkosc uczenia
    double gamma = 0.98;              // wspolczynnik dyskontowania
    double epsylon = 0.1;             // wspolczynnik eksploracji podczas uczenia

    //boolean czy_softmax = false;             // czy eksploracja softmax (jeśli nie, to epsylon-zachłanna)
    double wsp_selekcji = 1.0;        // im większy, tym częstszy wybór akcji o większej wartości w softmax-sie

    float Q[][][];
    int strategia[][];          // numery akcji w każdym ze stanów (każdy stan to położenie na planszy 2D)
    int strategie_wszystkie[][][]; // strategie innych agentów
    int liczba_agentow = 1;
    
    int moj_numer;
    Srodowisko s = new Srodowisko();    // środowisko obliczające nagrody oraz wyznaczające nowy stan
    Random loteria = new Random();                   // generator liczb losowych

    int liczba_krokow_max = (s.liczba_kolumn + s.liczba_wierszy)*10;// maksymalna liczba krokow w epizodzie

    long numer_epizodu = 0;

    UczenieZeWzm()
    {
        Q = new float[s.liczba_wierszy][s.liczba_kolumn][5];
        for (int i = 0;i<s.liczba_wierszy;i++)
              for (int j = 0;j<s.liczba_kolumn;j++)
                  for (int a=0;a<5;a++)
                      Q[i][j][a] = 0;
        strategia = new int[s.liczba_wierszy][s.liczba_kolumn];
        strategie_wszystkie = new int[50][s.liczba_wierszy][s.liczba_kolumn];
    }
    
    void epizod_uczenia_Qlearning()
    {
        int stan[] = new int[2];     // stan agenta (numer wiersza i kolumny)
        int stan_nast[] = new int[2];   // stan następny
        //System.out.printf("moj_num = %d, liczba_ag = %d\n",moj_numer,liczba_agentow);
        
        int polozenia_agentow[][] = new int[50][2]; // położenia agentów
        
        // losujemy położenia początkowe wszystkich symulowanych agentów:
        for (int ag=0;ag<liczba_agentow;ag++)
        {
            polozenia_agentow[ag] = s.losowanie_stanu_pocz(polozenia_agentow, ag);
            //System.out.printf("dla ag. %d wylosowano następujące liczby: [%d, %d]",ag,polozenia_agentow[ag][0],polozenia_agentow[ag][1]);
        }
        stan[0] = polozenia_agentow[moj_numer][0];  // stan naszego agenta
        stan[1] = polozenia_agentow[moj_numer][1];

        int nr_kroku = 0;
        float suma_nagrod = 0;
        
        // ustalenie akcji o najwyższej wartości w aktualnym przybliżeniu optymalnej strategii:
        double Qmax = -1e10;
        int nr_akcji_Qmax = -1;
        for (int j=1;j<5;j++) {
            if (Qmax < Q[stan[0]][stan[1]][j]) {
                Qmax = Q[stan[0]][stan[1]][j];
                nr_akcji_Qmax = j;
            }
        }

        double nagroda = -1;
       
        while (nr_kroku < liczba_krokow_max)
        {
            // losowanie eksploracji (jest niezbędna ze względu na konieczność sprawdzenia akcji, które nie mają jeszcze wiarygodnej oceny):
            boolean eksploracja = (loteria.nextFloat() < epsylon);  // ekspolarcja epsylon-zachłanna

            //int nr_akcji_wyb = ...                       // wybór najlepszej znanej akcji
            //................
            //................
            //................
            //................
            int nr_akcji_wyb = 1;       // na razie akcja dająca szansę na ruch w prawo

            // wykonanie wybranej akcji:
            int[] stan_i_nagroda = s.ruch_agenta(stan,nr_akcji_wyb,polozenia_agentow,0);

            stan_nast[0] = stan_i_nagroda[0];
            stan_nast[1] = stan_i_nagroda[1];
            nagroda = stan_i_nagroda[2];
            suma_nagrod += nagroda;

            // ustalenie akcji o najwyższej wartości w następnym stanie:
            Qmax = -1e10;
            for (int i=1;i<5;i++)
                if (Qmax < Q[stan_nast[0]][stan_nast[1]][i]){
                    Qmax = Q[stan_nast[0]][stan_nast[1]][i];
                    nr_akcji_Qmax = i;
                }


            // modyfikacja wartości użyteczności poprzedniej akcji:
            //Q[stan[0]][stan[1]][nr_akcji_wyb] =
            //................
            //................
            //................
            //................

            // przejście do kolejnego stanu:
            stan[0] = stan_nast[0];      // numer wiersza
            stan[1] = stan_nast[1];      // numer kolumny

            nr_kroku ++;
        } // while po krokach epizodu
       numer_epizodu++;
    } // metoda epizod_uczenia_Qlearning()

    void tworz_strategie()
    {
        // tworzenie strategii na podstawie tablicy Q
        for (int i = 0;i<s.liczba_wierszy;i++)
              for (int j = 0;j<s.liczba_kolumn;j++)
              {
                  double Qmax = -1e10;
                  int nr_akcji_Qmax = -1;
                  for (int a=1;a<5;a++) {
                      if (Qmax < Q[i][j][a]) {
                          Qmax = Q[i][j][a];
                          nr_akcji_Qmax = a;
                      }
                  }
                  strategia[i][j] = nr_akcji_Qmax;
              }
    }
    
    void zapis_Q_do_pliku(int numer_agenta)
    {
        FileOutputStream fos = null;


        File file = new File("Qagenta" +numer_agenta+ ".txt");

        try {
            fos = new FileOutputStream(file);

        } catch (FileNotFoundException fnfe) {
            System.out.println("Brak pliku!");
        }

        DataOutputStream dos = new DataOutputStream(fos);

        try {    //zapisywanie liczb z i-tej kolumny do jednego pliku
            for (int w = 0; w < s.liczba_wierszy; w++) {
                for (int k = 0; k < s.liczba_kolumn; k++) {
                    dos.writeBytes("nr wiersza " + w + ", nr kolumny " + k + ", śr. suma nagród Q = [");
                    for (int a = 0; a < 5; a++) {
                        dos.writeBytes(Q[w][k][a] + ", ");
                    }
                    dos.writeBytes("]\n");
                }
            }
            dos.writeBytes("\nstrategia:\n");
            for (int w = 0; w < s.liczba_wierszy; w++) {
                for (int k = 0; k < s.liczba_kolumn; k++) {
                    dos.writeBytes(" "+strategia[w][k]);
                }
                dos.writeBytes("\n");
            }
        } catch (IOException ioe) {
            System.out.println("Blad zapisu!");

        }

    }
}

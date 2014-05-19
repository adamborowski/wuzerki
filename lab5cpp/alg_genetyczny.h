/*
    Prosty algorytm genetyczny do szukania optymalnych rozwi¹zañ wœród wektorów liczb rzeczywistych
*/

class AlgGenetyczny 
{
public:
  long liczba_rozw;                  // liczba rozwi¹zañ w populacji
  long dlugosc_ciagu;                // d³ugoœæ rozwi¹zania - rozmiar wektora parametrów

  float prawd_krzyz;               // prawdopodobieñstwo krzy¿owania dwóch rozwi¹zañ
  float prawd_mut;                 // prawdopodobieñstwo mutacji parametru
  float wielkosc_mutacji;          // zakres mutacji -> czy ma to byæ du¿a czy ma³a zmiana
  int czy_max_przechodzi;          // czy najlepsze rozwi¹zanie przechodzi do nastêpnego pokolenia bez zmian

  float **popul_rozw;              // pupulacja rozwi¹zañ - tablica wektorów parametrów

  AlgGenetyczny(long _liczba_rozw, long _dlugosc_ciagu);
  void epoka_uczenia(float *oceny_rozw);
};


/*
   Funkcja interpretuj¹ca wektor liczb rzeczywistych jako parametry (wagi) sieci neuronowej
   obliczajacej wyjœcia na podstawie wejœæ.
*/
void aproks_neuronowa(float *wyjscia, float *wejscia, int liczba_wejsc, float *wagi, 
                      int *liczby_neuronow, int liczba_warstw_neuronow);

/*
   Funkcja zwraca liczbê wag - parametrów sieci neuronowej o podanej na wejœciu strukturze
*/
long liczba_wag_sieci_neuronowej(int liczba_wejsc,int *liczby_neuronow, int liczba_warstw_neuronow);

// Szukanie minimum funkcji za pomoc¹ algorytmu genetycznego - dla sprawdzenia
void uczenieAG3();

// przyk³ad genetycznego doboru wag sztucznej sieci neuronowej
void uczenieAG_SSN();


/*
    Prosty algorytm genetyczny do szukania optymalnych rozwi�za� w�r�d wektor�w liczb rzeczywistych
*/

class AlgGenetyczny 
{
public:
  long liczba_rozw;                  // liczba rozwi�za� w populacji
  long dlugosc_ciagu;                // d�ugo�� rozwi�zania - rozmiar wektora parametr�w

  float prawd_krzyz;               // prawdopodobie�stwo krzy�owania dw�ch rozwi�za�
  float prawd_mut;                 // prawdopodobie�stwo mutacji parametru
  float wielkosc_mutacji;          // zakres mutacji -> czy ma to by� du�a czy ma�a zmiana
  int czy_max_przechodzi;          // czy najlepsze rozwi�zanie przechodzi do nast�pnego pokolenia bez zmian

  float **popul_rozw;              // pupulacja rozwi�za� - tablica wektor�w parametr�w

  AlgGenetyczny(long _liczba_rozw, long _dlugosc_ciagu);
  void epoka_uczenia(float *oceny_rozw);
};


/*
   Funkcja interpretuj�ca wektor liczb rzeczywistych jako parametry (wagi) sieci neuronowej
   obliczajacej wyj�cia na podstawie wej��.
*/
void aproks_neuronowa(float *wyjscia, float *wejscia, int liczba_wejsc, float *wagi, 
                      int *liczby_neuronow, int liczba_warstw_neuronow);

/*
   Funkcja zwraca liczb� wag - parametr�w sieci neuronowej o podanej na wej�ciu strukturze
*/
long liczba_wag_sieci_neuronowej(int liczba_wejsc,int *liczby_neuronow, int liczba_warstw_neuronow);

// Szukanie minimum funkcji za pomoc� algorytmu genetycznego - dla sprawdzenia
void uczenieAG3();

// przyk�ad genetycznego doboru wag sztucznej sieci neuronowej
void uczenieAG_SSN();


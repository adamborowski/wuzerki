/************************************************************
Interakcja:
Wysy�anie, odbi�r komunikat�w, interakcja z innymi
uczestnikami WZR, sterowanie wirtualnymi obiektami  
*************************************************************/

#include <windows.h>
#include <time.h>
#include <stdio.h>   
#include "interakcja.h"
#include "obiekty.h"
#include "grafika.h"

#include "siec.h"

FILE *f = fopen("plik_wzr.txt","w");     // plik do zapisu informacji testowych

ObiektRuchomy *pMojObiekt;               // obiekt przypisany do tej aplikacji

Teren teren;
int iLiczbaCudzychOb = 0;
ObiektRuchomy *CudzeObiekty[1000];  // obiekty z innych aplikacji lub inne obiekty niz pMojObiekt
int IndeksyOb[1000];                // tablica indeksow innych obiektow ulatwiajaca wyszukiwanie

float fDt;                          // sredni czas pomiedzy dwoma kolejnymi cyklami symulacji i wyswietlania
long czas_cyklu_WS,licznik_sym;     // zmienne pomocnicze potrzebne do obliczania fDt
float sr_czestosc;                  // srednia czestosc wysylania ramek w [ramkach/s] 
long czas_start = clock();          // czas od poczatku dzialania aplikacji  

multicast_net *multi_reciv;         // wsk do obiektu zajmujacego sie odbiorem komunikatow
multicast_net *multi_send;          //   -||-  wysylaniem komunikatow

HANDLE threadReciv;                 // uchwyt w�tku odbioru komunikat�w
extern HWND okno;       
int SHIFTwcisniety = 0;            

int liczba_prob_podniesienia = 0;

bool is_agent = true;				// czy agent
int agent_target = -1;				// cel agenta, indeks tablicy cel�w agenta / obiekt�w terenu
int deny_target = -1;				// zakazany cel


float odleglosc = 10000000.0;
float odlegloscNowa = 0.0;
int index = -1;

// sytuacja I - kupuj�cy chce kupi� paliwo i czeka
int id_sprzedawcy = -1;
float ile_sprzedac_paliwa = 0.f;
bool czy_wyslane_potwierdzenie = false;
bool czekaj = false;
bool w_trakcie_realizacji = false;

// sytuacja II - sprzedaj�cy chce sprzeda� paliwo i jedzie
bool czekaj_2 = false;

//
float za_jaka_cene_chce_kupic = 0.f;
const float CENA_PALIWA = 10.f;

// Parametry widoku:
Wektor3 kierunek_kamery = Wektor3(10,-3,-11);   // kierunek patrzenia
Wektor3 pol_kamery = Wektor3(-5,3,10);          // po�o�enie kamery
Wektor3 pion_kamery = Wektor3(0,1,0);           // kierunek pionu kamery             
bool sledzenie = 0;                             // tryb �ledzenia obiektu przez kamer�
float oddalenie = 1.0;                          // oddalenie lub przybli�enie kamery
float zoom = 1.0;                               // zmiana k�ta widzenia
float kat_kam_z = 0;                            // obr�t kamery g�ra-d�
bool sterowanie_myszkowe = 0;                   // sterowanie pojazdem za pomoc� myszki
int kursor_x, kursor_y;                         // polo�enie kursora myszki w chwili w��czenia sterowania

int opoznienia = 0;                
bool podnoszenie_przedm = 0;        // czy mozna podnosic przedmioty
bool rejestracja_uczestnikow = 1;   // rejestracja trwa do momentu wzi�cia przedmiotu przez kt�regokolwiek uczestnika,
// w przeciwnym razie trzeba by przesy�a� ca�y stan �rodowiska nowicjuszowi
float czas_odnowy_przedm = 90;      // czas w [s] po kt�rym wzi�te przedmioty odnawiaj� si�
bool czy_umiejetnosci = 0;          // czy zr�nicowanie umiej�tno�ci (dla ka�dego pojazdu losowane s� umiej�tno�ci
// zbierania got�wki i paliwa)

extern float WyslaniePrzekazu(int ID_adresata, int typ_przekazu, float wartosc_przekazu, float cena_jednostkowa);

enum typy_ramek {STAN_OBIEKTU, WZIECIE_PRZEDMIOTU, ODNOWIENIE_SIE_PRZEDMIOTU, KOLIZJA, PRZEKAZ, 
	PROSBA_O_ZAMKNIECIE, NEGOCJACJE_HANDLOWE, CHEC_KUPNA, CHEC_SPRZEDAZY, POTWIERDZAM_SPRZEDAZ, POTWIERDZAM_KUPNO};

enum typy_przekazu {GOTOWKA, PALIWO};
int typ=0;
struct Ramka
{  
	int typ_ramki;
	long czas_wyslania;    
	int iID_adresata;      // nr ID adresata wiadomo�ci (pozostali uczestnicy powinni wiadomo�� zignorowa�)

	int nr_przedmiotu;     // nr przedmiotu, kt�ry zosta� wzi�ty lub odzyskany
	Wektor3 wdV_kolid;     // wektor pr�dko�ci wyj�ciowej po kolizji (uczestnik o wskazanym adresie powinien 
	// przyj�� t� pr�dko��)  

	int typ_przekazu;        // got�wka, paliwo
	float wartosc_przekazu;  // ilo�� got�wki lub paliwa 
	float cena_przekazu;  // jednostkowa cena przekazu
	int nr_druzyny;

	StanObiektu stan;
};


//******************************************
// Funkcja obs�ugi w�tku odbioru komunikat�w 
DWORD WINAPI WatekOdbioru(void *ptr)
{
	multicast_net *pmt_net=(multicast_net*)ptr;  // wska�nik do obiektu klasy multicast_net
	int rozmiar;                                 // liczba bajt�w ramki otrzymanej z sieci
	Ramka ramka;
	StanObiektu stan;

	while(1)
	{
		rozmiar = pmt_net->reciv((char*)&ramka,sizeof(Ramka));   // oczekiwanie na nadej�cie ramki 
		switch (ramka.typ_ramki) 
		{
		case STAN_OBIEKTU:           // podstawowy typ ramki informuj�cej o stanie obiektu              
			{
				stan = ramka.stan;
				//fprintf(f,"odebrano stan iID = %d, ID dla mojego obiektu = %d\n",stan.iID,pMojObiekt->iID);
				int niewlasny = 1;
				if ((stan.iID != pMojObiekt->iID))          // je�li to nie m�j w�asny obiekt
				{

					if ((rejestracja_uczestnikow)&&(IndeksyOb[stan.iID] == -1))        // nie ma jeszcze takiego obiektu w tablicy -> trzeba go stworzy�
					{
						CudzeObiekty[iLiczbaCudzychOb] = new ObiektRuchomy();   
						IndeksyOb[stan.iID] = iLiczbaCudzychOb;     // wpis do tablicy indeksowanej numerami ID
						// u�atwia wyszukiwanie, alternatyw� mo�e by� tabl. rozproszona                                                                                                           
						iLiczbaCudzychOb++;     
						CudzeObiekty[IndeksyOb[stan.iID]]->ZmienStan(stan);   // aktualizacja stanu obiektu obcego 			
					}        
					else if (IndeksyOb[stan.iID] != -1)
						CudzeObiekty[IndeksyOb[stan.iID]]->ZmienStan(stan);   // aktualizacja stanu obiektu obcego 	
					else                                                    
					{
						Ramka ramka;
						ramka.typ_ramki = PROSBA_O_ZAMKNIECIE;                // ��danie zamki�cia aplikacji
						ramka.iID_adresata = stan.iID;
						int iRozmiar = multi_send->send_delayed((char*)&ramka,sizeof(Ramka));
					}
				}
				break;
			}
		case WZIECIE_PRZEDMIOTU:            // ramka informuj�ca, �e kto� wzi�� przedmiot o podanym numerze
			{
				if ((ramka.nr_przedmiotu < teren.liczba_przedmiotow)&&(stan.iID != pMojObiekt->iID))
				{
					teren.p[ramka.nr_przedmiotu].do_wziecia = 0;
					teren.p[ramka.nr_przedmiotu].czy_ja_wzialem = 0;
					rejestracja_uczestnikow = 0;
				}
				break;
			}
		case ODNOWIENIE_SIE_PRZEDMIOTU:       // ramka informujaca, �e przedmiot wcze�niej wzi�ty pojawi� si� znowu w tym samym miejscu
			{
				if (ramka.nr_przedmiotu < teren.liczba_przedmiotow)
					teren.p[ramka.nr_przedmiotu].do_wziecia = 1;
				break;
			}
		case KOLIZJA:                       // ramka informuj�ca o tym, �e obiekt uleg� kolizji
			{
				if (ramka.iID_adresata == pMojObiekt->iID)  // ID pojazdu, kt�ry uczestniczy� w kolizji zgadza si� z moim ID 
				{
					pMojObiekt->wdV_kolid = ramka.wdV_kolid; // przepisuje poprawk� w�asnej pr�dko�ci
					pMojObiekt->iID_kolid = pMojObiekt->iID; // ustawiam nr. kolidujacego jako w�asny na znak, �e powinienem poprawi� pr�dko��
				}
				break;
			}
		case PRZEKAZ:                       // ramka informuj�ca o przelewie pieni�nym lub przekazaniu towaru    
			{
				if (ramka.iID_adresata == pMojObiekt->iID)  // ID pojazdu, ktory otrzymal przelew zgadza si� z moim ID 
				{       
					if (ramka.typ_przekazu == GOTOWKA)
					{
						pMojObiekt->pieniadze += ramka.wartosc_przekazu; 					
						printf("Dostalem pieniadze: %f", ramka.wartosc_przekazu);
					}
					else if (ramka.typ_przekazu == PALIWO)
					{
						pMojObiekt->ilosc_paliwa += ramka.wartosc_przekazu;

						if (czekaj)
						{
							// wys�anie got�wki
							float ilosc_p =  0;
							ilosc_p = WyslaniePrzekazu(id_sprzedawcy, GOTOWKA, ramka.wartosc_przekazu * ramka.cena_przekazu, 0.f);

							if (ilosc_p < 1) {
								printf("Gotowki nie da�o si� przekaza�, bo by� mo�e najbli�szy obiekt ruchomy znajduje si� zbyt daleko.");
							}
							else {
								printf("przeslalem %f gotowki do obiektu nr %d\n", ilosc_p, ramka.stan.iID);
							}
						}
					}

					// nale�a�oby jeszcze przelew potwierdzi� (w UDP ramki mog� by� gubione!)
					czekaj = false;
					w_trakcie_realizacji = false;
					czy_wyslane_potwierdzenie = false;
				}
				break;
			}
		case PROSBA_O_ZAMKNIECIE:           // ramka informuj�ca, �e powiniene� si� zamkn��
			{
				if (ramka.iID_adresata == pMojObiekt->iID)
				{   
					SendMessage(okno,WM_DESTROY,0,100);
				}
				break;
			}
		case CHEC_SPRZEDAZY:
		{
			// sytuacja II
			// perspektywa kupca
			if (pMojObiekt->ilosc_paliwa < 30.f && !czy_wyslane_potwierdzenie) {
				//@todo: doliczanie dojazdu do ceny
				if (ramka.cena_przekazu < CENA_PALIWA)
				{
					za_jaka_cene_chce_kupic = ramka.cena_przekazu;

					Ramka potwierdzam;
					potwierdzam.typ_ramki = CHEC_KUPNA;
					potwierdzam.cena_przekazu = za_jaka_cene_chce_kupic;
					potwierdzam.wartosc_przekazu = ramka.wartosc_przekazu;
					potwierdzam.stan = pMojObiekt->Stan();
					potwierdzam.iID_adresata = ramka.stan.iID;

					int iRozmiar = multi_send->send_delayed((char*)&potwierdzam, sizeof(Ramka));

					printf("Odebralem ramke chce sprzedac\n");
				}

			}
			break;
		}
		case CHEC_KUPNA:
			{
				// perspektywa sprzedawcy
				if (pMojObiekt->ilosc_paliwa - ramka.wartosc_przekazu > 15.f && !w_trakcie_realizacji) {
					//@todo: doliczanie dojazdu do ceny
					printf("czy %d == %d", pMojObiekt->iID, ramka.iID_adresata);
					if (ramka.cena_przekazu > CENA_PALIWA || pMojObiekt->iID == ramka.iID_adresata)
					{
						za_jaka_cene_chce_kupic = ramka.cena_przekazu;

						// utworzenie ramki potwierdzaj�cej ch�� sprzeda�y
						Ramka potwierdzam;
						potwierdzam.iID_adresata = ramka.stan.iID;
						potwierdzam.typ_ramki = POTWIERDZAM_SPRZEDAZ;
						potwierdzam.stan = pMojObiekt->Stan(); 

						int iRozmiar = multi_send->send_delayed((char*)&potwierdzam,sizeof(Ramka));

						ile_sprzedac_paliwa = ramka.wartosc_przekazu;
						printf("Odebralem ramke chce kupic\n");

					}

				}
				break;
			}
		case POTWIERDZAM_SPRZEDAZ:
			{
				// perspektywa kupca
				if (ramka.iID_adresata == pMojObiekt->iID && !czy_wyslane_potwierdzenie)
				{
					id_sprzedawcy = ramka.stan.iID;

					// utworzenie ramki potwierdzaj�cej ch�� sprzeda�y
					Ramka potwierdzam;
					potwierdzam.iID_adresata = ramka.stan.iID;
					potwierdzam.typ_ramki = POTWIERDZAM_KUPNO;
					potwierdzam.stan = pMojObiekt->Stan(); 

					int iRozmiar = multi_send->send_delayed((char*)&potwierdzam,sizeof(Ramka));

					czy_wyslane_potwierdzenie = true;

					// oczekiwanie na sprzedawc�
					czekaj = true;
					printf("Odebralem ramke potwierdzam sprzedaz\n");
				}
			}
			break;
		case POTWIERDZAM_KUPNO:
		{
			// perspektywa sprzedawcy
			if (ramka.iID_adresata == pMojObiekt->iID && !w_trakcie_realizacji)
			{
				agent_target = ramka.stan.iID;

				w_trakcie_realizacji = true;

				printf("Odebralem ramke potwierdzam kupno\n");
			}
			break;
		}
		case NEGOCJACJE_HANDLOWE:
			{
				// ------------------------------------------------------------------------
				// --------------- MIEJSCE #1 NA NEGOCJACJE HANDLOWE  ---------------------
				// (szczeg�y na stronie w instrukcji do zadania)




				// ------------------------------------------------------------------------

				break;
			}

		} // switch po typach ramek
	}  // while(1)
	return 1;
}

// *****************************************************************
// ****    Wszystko co trzeba zrobi� podczas uruchamiania aplikacji
// ****    poza grafik�   
void PoczatekInterakcji()
{
	DWORD dwThreadId;

	pMojObiekt = new ObiektRuchomy();    // tworzenie wlasnego obiektu

	for (long i=0;i<1000;i++)            // inicjacja indeksow obcych obiektow
		IndeksyOb[i] = -1;

	czas_cyklu_WS = clock();             // pomiar aktualnego czasu

	// obiekty sieciowe typu multicast (z podaniem adresu WZR oraz numeru portu)
	multi_reciv = new multicast_net("192.168.0.3",10001);      // obiekt do odbioru ramek sieciowych
	multi_send = new multicast_net("192.168.0.2",10001);       // obiekt do wysy�ania ramek

	if (opoznienia)
	{
		float srednie_opoznienie = 3*(float)rand()/RAND_MAX, wariancja_opoznienia = 2;
		multi_send->PrepareDelay(srednie_opoznienie,wariancja_opoznienia);
	}

	// uruchomienie watku obslugujacego odbior komunikatow
	threadReciv = CreateThread(
		NULL,                        // no security attributes
		0,                           // use default stack size
		WatekOdbioru,                // thread function
		(void *)multi_reciv,               // argument to thread function
		0,                           // use default creation flags
		&dwThreadId);                // returns the thread identifier

}


// *****************************************************************
// ****    Wszystko co trzeba zrobi� w ka�dym cyklu dzia�ania 
// ****    aplikacji poza grafik� 
void Cykl_WS()
{
	licznik_sym++;  

	// obliczenie �redniego czasu pomi�dzy dwoma kolejnnymi symulacjami po to, by zachowa�  fizycznych 
	if (licznik_sym % 50 == 0)          // je�li licznik cykli przekroczy� pewn� warto��, to
	{                                   // nale�y na nowo obliczy� �redni czas cyklu fDt
		char text[200];
		long czas_pop = czas_cyklu_WS;
		czas_cyklu_WS = clock();
		float fFps = (50*CLOCKS_PER_SEC)/(float)(czas_cyklu_WS-czas_pop);
		if (fFps!=0) fDt=1.0/fFps; else fDt=1;

		sprintf(text,"[%d], %0.0f fps  %0.2fms, paliwo = %0.2f, gotowka = %d, F1-pomoc",pMojObiekt->iID, fFps,1000.0/fFps,pMojObiekt->ilosc_paliwa,pMojObiekt->pieniadze);
		SetWindowText(okno,text); // wy�wietlenie aktualnej ilo�ci klatek/s w pasku okna			
	}   

	pMojObiekt->Symulacja(fDt);                    // symulacja w�asnego obiektu


	if ((pMojObiekt->iID_kolid > -1)&&             // wykryto kolizj� - wysy�am specjaln� ramk�, by poinformowa� o tym drugiego uczestnika
		(pMojObiekt->iID_kolid != pMojObiekt->iID)) // oczywi�cie wtedy, gdy nie chodzi o m�j pojazd
	{
		Ramka ramka;
		ramka.typ_ramki = KOLIZJA;
		ramka.iID_adresata = pMojObiekt->iID_kolid;
		ramka.wdV_kolid = pMojObiekt->wdV_kolid;
		int iRozmiar = multi_send->send_delayed((char*)&ramka,sizeof(Ramka));

		char text[128];
		sprintf(text,"Kolizja z obiektem o ID = %d",pMojObiekt->iID_kolid);
		SetWindowText(okno,text);

		pMojObiekt->iID_kolid = -1;
	}

	// wyslanie komunikatu o stanie obiektu przypisanego do aplikacji (pMojObiekt):    
	if (licznik_sym % 1 == 0)      
	{
		Ramka ramka;
		ramka.typ_ramki = STAN_OBIEKTU;
		ramka.stan = pMojObiekt->Stan();         // stan w�asnego obiektu 
		int iRozmiar = multi_send->send_delayed((char*)&ramka,sizeof(Ramka));
	} 

	long czas_akt = clock();

	// sprawdzam, czy nie najecha�em na monet� lub beczk� z paliwem. Je�li tak, to zdobywam pieni�dze lub paliwo oraz wysy�am innym uczestnikom 
	// informacj� o zabraniu beczki: (wcze�niej trzeba wcisn�� P) 
	for (long i=0;i<teren.liczba_przedmiotow;i++)
	{
		if ((teren.p[i].do_wziecia == 1)&&(podnoszenie_przedm)&&
			((teren.p[i].wPol - pMojObiekt->wPol + Wektor3(0,pMojObiekt->wPol.y - teren.p[i].wPol.y,0)).dlugosc() < pMojObiekt->promien))
		{

			long wartosc = teren.p[i].wartosc;

			if (teren.p[i].typ == MONETA)
			{
				if (czy_umiejetnosci)
					wartosc = (long)(float)wartosc*pMojObiekt->umiejetn_zb_monet;   
				pMojObiekt->pieniadze += wartosc;   
			}
			else
			{
				if (czy_umiejetnosci)
					wartosc = (float)wartosc*pMojObiekt->umiejetn_zb_paliwa; 
				pMojObiekt->ilosc_paliwa += wartosc;      
			}

			agent_target = -1;
			odleglosc = 10000000.0;
			teren.p[i].do_wziecia = 0;
			teren.p[i].czy_ja_wzialem = 1;
			teren.p[i].czas_wziecia = clock();
			rejestracja_uczestnikow = 0;     // koniec rejestracji nowych uczestnik�w
			liczba_prob_podniesienia = 0;
			deny_target = -1;

			Ramka ramka;
			ramka.typ_ramki = WZIECIE_PRZEDMIOTU; 
			ramka.nr_przedmiotu = i;
			ramka.stan = pMojObiekt->Stan(); 
			int iRozmiar = multi_send->send_delayed((char*)&ramka,sizeof(Ramka));
		}
		else if ((teren.p[i].do_wziecia == 0)&&(teren.p[i].czy_ja_wzialem)&&(teren.p[i].czy_odnawialny)&&
			(czas_akt - teren.p[i].czas_wziecia >= czas_odnowy_przedm*CLOCKS_PER_SEC))
		{                              // je�li min�� pewnien okres czasu przedmiot mo�e zosta� przywr�cony
			teren.p[i].do_wziecia = 1;
			Ramka ramka;
			ramka.typ_ramki = ODNOWIENIE_SIE_PRZEDMIOTU; 
			ramka.nr_przedmiotu = i;
			int iRozmiar = multi_send->send_delayed((char*)&ramka,sizeof(Ramka));
		}
	} // for po przedmiotach

	// --------------------------------------------------------------------
	// --------------- MIEJSCE NA ALGORYTM STEROWANIA ---------------------
	// (dob�r si�y F w granicach (-2000 N, 4000 N), k�ta skr�tu k� alfa (-pi/4, pi/4) oraz
	// decyzji o hamowaniu ham w zale�no�ci od sytuacji)



	if(is_agent)
	{
		//if (agent_target == -1)
		//{
		// for(int i =0; i< teren.liczba_przedmiotow; i++)
		// {
		//  if(pMojObiekt->ilosc_paliwa < 19.0)
		//	  typ = PALIWO;
		//  else
		//	  typ = MONETA;
		//  if(teren.p[i].typ == typ && teren.p[i].do_wziecia)
		//  {		  
		//	  odlegloscNowa = (teren.p[i].wPol - pMojObiekt->wPol + Wektor3(0,pMojObiekt->wPol.y - teren.p[i].wPol.y,0)).dlugosc();
		//	  if(odleglosc > odlegloscNowa)
		//	  {
		//		  odleglosc = odlegloscNowa;
		//		  index = i;
		//	  }
		//  }
		// }
		// agent_target = index;		 
		// printf("Index obiektu: %d, wartosc: %d\n", index, teren.p[index].wartosc);

		//}

		if (czekaj)
		{
			// perspektywa kupca
			if (licznik_sym % 50 == 0)
			{
				printf("czekam\n");

				if (!czy_wyslane_potwierdzenie) {
					// kupiec
					float ile = 5.f;
					za_jaka_cene_chce_kupic = CENA_PALIWA + 5.f;
					//czy_wyslane_potwierdzenie = false;

					Ramka potwierdzam;
					potwierdzam.typ_ramki = CHEC_KUPNA;
					potwierdzam.cena_przekazu = za_jaka_cene_chce_kupic;
					potwierdzam.wartosc_przekazu = ile;
					potwierdzam.stan = pMojObiekt->Stan();

					int iRozmiar = multi_send->send_delayed((char*)&potwierdzam, sizeof(Ramka));
				}
			}

			if (pMojObiekt->wV.dlugosc() > 3.0)
			{
				pMojObiekt->F= -3000.0;
			}
			else if (pMojObiekt->wV.dlugosc() > 0.5)
			{
				pMojObiekt->F= -1000.0;
			}
			else
			{
				pMojObiekt->F = 0;
				pMojObiekt->wV = Wektor3(0,0,0);
			}


		}
		else if (w_trakcie_realizacji)
		{
			// perspektywa sprzedawcy
			ObiektRuchomy * kupujacy = CudzeObiekty[IndeksyOb[agent_target]];
			Wektor3 t = kupujacy->wPol-pMojObiekt->wPol;
			Wektor3 w_przod = pMojObiekt->qOrient.obroc_wektor(Wektor3(1,0,0)); 

			float cos = (t^w_przod)/(t.dlugosc()*w_przod.dlugosc());
			float kat = acos(cos);

			if(cos > 0.95)
			{
				pMojObiekt->alfa = 0;
			}
			else
			{
				if(kat<PI)		  
					pMojObiekt->alfa = -PI/3;
				else
					pMojObiekt->alfa = +PI/3;
			}

			if (pMojObiekt->wV.dlugosc() < 8.0)
				pMojObiekt->F=1500.0;
			else
				pMojObiekt->F=0.0;

			if((kupujacy->wPol - pMojObiekt->wPol + Wektor3(0,pMojObiekt->wPol.y - kupujacy->wPol.y,0)).dlugosc() < 5.0*pMojObiekt->promien)
			{
				//printf("Predkosc: %f\n", pMojObiekt->wV.dlugosc());

				if (pMojObiekt->wV.dlugosc() > 3.0)
					pMojObiekt->F = -3000.0;
				else if (pMojObiekt->wV.dlugosc() < 1.0)
				{
					pMojObiekt->F = 0;
				}

				float ilosc_p =  0;
				ilosc_p = WyslaniePrzekazu(kupujacy->iID, PALIWO, ile_sprzedac_paliwa, za_jaka_cene_chce_kupic);
				
				
				if (ilosc_p < 1.0) 
				{
					printf("Paliwa nie da�o si� przekaza�, bo by� mo�e najbli�szy obiekt ruchomy znajduje si� zbyt daleko.\n");
				}
				else
				{
					printf("Wyslalem: %f\n", ile_sprzedac_paliwa);
					w_trakcie_realizacji = false;
					agent_target=-1;
				}
			}

			if (licznik_sym % 50 == 0)
			{
				printf("Predkosc: %f, kat %f, cos %f\n", pMojObiekt->wV.dlugosc(), kat, cos);
			}
		}
		else if (agent_target >= 0 && teren.p[agent_target].do_wziecia)
		{

			Wektor3 t = teren.p[agent_target].wPol-pMojObiekt->wPol;
			Wektor3 w_przod = pMojObiekt->qOrient.obroc_wektor(Wektor3(1,0,0)); 

			float cos = (t^w_przod)/(t.dlugosc()*w_przod.dlugosc());
			float kat = acos(cos);

			if(cos > 0.95)
			{
				pMojObiekt->alfa = 0;
			}
			else
			{
				if(kat<PI)		  
					pMojObiekt->alfa = -PI/2-.1f;
				else
					pMojObiekt->alfa = +PI/2-.1f;
			}

			if (pMojObiekt->wV.dlugosc() < 8.0)
				pMojObiekt->F=1500.0;
			else
				pMojObiekt->F=0.0;

			if((teren.p[agent_target].wPol - pMojObiekt->wPol + Wektor3(0,pMojObiekt->wPol.y - teren.p[agent_target].wPol.y,0)).dlugosc() < 5.0*pMojObiekt->promien)
			{
				//printf("Predkosc: %f\n", pMojObiekt->wV.dlugosc());

				if(pMojObiekt->wV.dlugosc() > 3.0)
					pMojObiekt->F= -1500.0;

				podnoszenie_przedm = 1;
				++liczba_prob_podniesienia;

				if (liczba_prob_podniesienia > 1000) {
					deny_target = agent_target;
					agent_target = -1;
				}
			}
			else
			{
				podnoszenie_przedm = 0;
			}

			if (licznik_sym % 50 == 0)
			{
				printf("Predkosc: %f, kat %f, cos %f, liczba prob %d\n", pMojObiekt->wV.dlugosc(), kat, cos, liczba_prob_podniesienia);
			}
		}
		else
		{
			agent_target = -1;
			odleglosc = 10000000.0;

			// je�li mam za du�o paliwo, to sprzedaj
			if (pMojObiekt->ilosc_paliwa > 100.f) 
			{
				if (licznik_sym % 50 == 0)
				{
					float nasza_cena = CENA_PALIWA - 5.f;
					czy_wyslane_potwierdzenie = false;

					ile_sprzedac_paliwa = 10.f;

					Ramka potwierdzam;
					potwierdzam.typ_ramki = CHEC_SPRZEDAZY;
					potwierdzam.cena_przekazu = nasza_cena;
					potwierdzam.wartosc_przekazu = ile_sprzedac_paliwa;
					potwierdzam.stan = pMojObiekt->Stan();

					int iRozmiar = multi_send->send_delayed((char*)&potwierdzam, sizeof(Ramka));

					printf("chce sprzedac %f paliwa za %f\n", ile_sprzedac_paliwa, nasza_cena);
				}

			}
			// jesli mam za malo paliwo, to kup
			else if (pMojObiekt->ilosc_paliwa < 10.f)
			{
				// kupiec
				float ile = 5.f;
				za_jaka_cene_chce_kupic = CENA_PALIWA + 5.f;
				czy_wyslane_potwierdzenie = false;

				Ramka potwierdzam;
				potwierdzam.typ_ramki = CHEC_KUPNA;
				potwierdzam.cena_przekazu = za_jaka_cene_chce_kupic;
				potwierdzam.wartosc_przekazu = ile;
				potwierdzam.stan = pMojObiekt->Stan(); 

				int iRozmiar = multi_send->send_delayed((char*)&potwierdzam, sizeof(Ramka));

				printf("chce kupic %f paliwa za %f\n", ile, za_jaka_cene_chce_kupic);

				czekaj = true;
			} else {
				// w przeciwnym razie zbieraj dalej

				for(int i =0; i< teren.liczba_przedmiotow; i++)
				{
					if(pMojObiekt->ilosc_paliwa < 150.0)
						typ = PALIWO;
					else
						typ = MONETA;
					if(teren.p[i].typ == typ && teren.p[i].do_wziecia)
					{		  
						odlegloscNowa = (teren.p[i].wPol - pMojObiekt->wPol + Wektor3(0,pMojObiekt->wPol.y - teren.p[i].wPol.y,0)).dlugosc();
						if(odleglosc > odlegloscNowa && i != deny_target)
						{
							odleglosc = odlegloscNowa;
							index = i;
						}
					}
				}
			}

			agent_target = index;

			if (agent_target >= 0)
				printf("Index obiektu: %d, wartosc: %d, do wzi�cia: %d\n", index, teren.p[index].wartosc, teren.p[index].do_wziecia);
		}
	}

	// --------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// --------------- MIEJSCE #2 NA NEGOCJACJE HANDLOWE  ---------------------
	// (szczeg�y na stronie w instrukcji do zadania)




	// ------------------------------------------------------------------------
}

// *****************************************************************
// ****    Wszystko co trzeba zrobi� podczas zamykania aplikacji
// ****    poza grafik� 
void ZakonczenieInterakcji()
{
	fprintf(f,"Koniec interakcji\n");
	fclose(f);
}

// Funkcja wysylajaca ramke z przekazem, zwraca zrealizowan� warto�� przekazu
float WyslaniePrzekazu(int ID_adresata, int typ_przekazu, float wartosc_przekazu, float cena_jednostkowa)
{
	Ramka ramka;
	ramka.typ_ramki = PRZEKAZ;
	ramka.iID_adresata = ID_adresata;
	ramka.typ_przekazu = typ_przekazu;
	ramka.wartosc_przekazu = wartosc_przekazu;
	ramka.cena_przekazu = cena_jednostkowa;

	// tutaj nale�a�oby uzyska� potwierdzenie przekazu zanim sumy zostan� odj�te
	if (typ_przekazu == GOTOWKA)
	{
		if (pMojObiekt->pieniadze < wartosc_przekazu) 
			ramka.wartosc_przekazu = pMojObiekt->pieniadze;
		pMojObiekt->pieniadze -= ramka.wartosc_przekazu;
	}
	else if (typ_przekazu == PALIWO)
	{
		// odszukanie adresata, sprawdzenie czy jest odpowiednio blisko:
		int indeks_adresata = -1;
		for (int i=0;i<iLiczbaCudzychOb;i++)
			if (CudzeObiekty[i]->iID == ID_adresata) {indeks_adresata = i; break;}
			if ((CudzeObiekty[indeks_adresata]->wPol - pMojObiekt->wPol).dlugosc() > 
				CudzeObiekty[indeks_adresata]->dlugosc + pMojObiekt->dlugosc)
				ramka.wartosc_przekazu = 0;
			else
			{
				if (pMojObiekt->ilosc_paliwa < wartosc_przekazu)
					ramka.wartosc_przekazu = pMojObiekt->ilosc_paliwa;
				pMojObiekt->ilosc_paliwa -= ramka.wartosc_przekazu; 
			}
	}

	if (ramka.wartosc_przekazu > 0)
		int iRozmiar = multi_send->send_delayed((char*)&ramka,sizeof(Ramka));  

	return ramka.wartosc_przekazu;
}


// ************************************************************************
// ****    Obs�uga klawiszy s�u��cych do sterowania obiektami lub
// ****    widokami 
void KlawiszologiaSterowania(UINT kod_meldunku, WPARAM wParam, LPARAM lParam)
{

	switch (kod_meldunku) 
	{

	case WM_LBUTTONDOWN: //reakcja na lewy przycisk myszki
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			if (sterowanie_myszkowe)
				pMojObiekt->F = 4100.0;        // si�a pchaj�ca do przodu
			break;
		}
	case WM_RBUTTONDOWN: //reakcja na prawy przycisk myszki
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			if (sterowanie_myszkowe)
				pMojObiekt->F = -2100.0;        // si�a pchaj�ca do tylu
			break;
		}
	case WM_MBUTTONDOWN: //reakcja na �rodkowy przycisk myszki : uaktywnienie/dezaktywacja sterwania myszkowego
		{
			sterowanie_myszkowe = 1 - sterowanie_myszkowe;
			kursor_x = LOWORD(lParam);
			kursor_y = HIWORD(lParam);
			break;
		}
	case WM_LBUTTONUP: //reakcja na puszczenie lewego przycisku myszki
		{   
			if (sterowanie_myszkowe)
				pMojObiekt->F = 0.0;        // si�a pchaj�ca do przodu
			break;
		}
	case WM_RBUTTONUP: //reakcja na puszczenie lewy przycisk myszki
		{
			if (sterowanie_myszkowe)
				pMojObiekt->F = 0.0;        // si�a pchaj�ca do przodu
			break;
		}
	case WM_MOUSEMOVE:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			if (sterowanie_myszkowe)
			{
				float kat_skretu = (float)(kursor_x - x)/20;
				if (kat_skretu > 45) kat_skretu = 45;
				if (kat_skretu < -45) kat_skretu = -45;	
				pMojObiekt->alfa = PI*kat_skretu/180;
			}
			break;
		}
	case WM_KEYDOWN:
		{

			switch (LOWORD(wParam))
			{
			case VK_SHIFT:
				{
					SHIFTwcisniety = 1; 
					break; 
				}        
			case VK_SPACE:
				{
					pMojObiekt->ham = 1.0;       // stopie� hamowania (reszta zale�y od si�y docisku i wsp. tarcia)
					break;                       // 1.0 to maksymalny stopie� (np. zablokowanie k�)
				}
			case VK_UP:
				{

					pMojObiekt->F = 4100.0;        // si�a pchaj�ca do przodu
					break;
				}
			case VK_DOWN:
				{
					pMojObiekt->F = -2100.0;        // sila pchajaca do tylu
					break;
				}
			case VK_LEFT:
				{
					if (SHIFTwcisniety) pMojObiekt->alfa = PI*35/180;
					else pMojObiekt->alfa = PI*15/180;

					break;
				}
			case VK_RIGHT:
				{
					if (SHIFTwcisniety) pMojObiekt->alfa = -PI*35/180;
					else pMojObiekt->alfa = -PI*15/180;
					break;
				}
			case 'W':   // przybli�enie widoku
				{
					//pol_kamery = pol_kamery - kierunek_kamery*0.3;
					if (oddalenie > 0.5) oddalenie /= 1.2;
					else oddalenie = 0;  
					break; 
				}     
			case 'S':   // oddalenie widoku
				{
					//pol_kamery = pol_kamery + kierunek_kamery*0.3; 
					if (oddalenie > 0) oddalenie *= 1.2;
					else oddalenie = 0.5;   
					break; 
				}    
			case 'Q':   // widok z g�ry
				{
					pol_kamery = Wektor3(0,100,0);
					kierunek_kamery = Wektor3(0,-1,0.01);
					pion_kamery = Wektor3(0,0,-1);
					break;
				}
			case 'E':   // obr�t kamery ku g�rze (wzgl�dem lokalnej osi z)
				{
					kat_kam_z += PI*5/180; 
					break; 
				}    
			case 'D':   // obr�t kamery ku do�owi (wzgl�dem lokalnej osi z)
				{
					kat_kam_z -= PI*5/180;  
					break;
				}
			case 'A':   // w��czanie, wy��czanie trybu �ledzenia obiektu
				{
					sledzenie = 1 - sledzenie;
					break;
				}
			case 'Z':   // zoom - zmniejszenie k�ta widzenia
				{
					zoom /= 1.1;
					RECT rc;
					GetClientRect (okno, &rc);
					ZmianaRozmiaruOkna(rc.right - rc.left,rc.bottom-rc.top);
					break;
				}
			case 'X':   // zoom - zwi�kszenie k�ta widzenia
				{
					zoom *= 1.1;
					RECT rc;
					GetClientRect (okno, &rc);
					ZmianaRozmiaruOkna(rc.right - rc.left,rc.bottom-rc.top);
					break;
				}
			case 'P':   // podnoszenie przedmiot�w
				{
					//Wektor3 w_przod = pMojObiekt->qOrient.obroc_wektor(Wektor3(1,0,0));
					//+ Wector3(0,pMojObiekt->wPol.y - teren.p[i].wPol.y,0)
					podnoszenie_przedm = 1 - podnoszenie_przedm;
					break;
				}
			case 'F':  // przekazanie 10 kg paliwa pojazdowi, kt�ry znajduje si� najbli�ej
				{
					float min_odleglosc = 1e10;
					int indeks_min = -1;
					for (int i=0;i<iLiczbaCudzychOb;i++)
					{
						if (min_odleglosc > (CudzeObiekty[i]->wPol - pMojObiekt->wPol).dlugosc() )
						{
							min_odleglosc = (CudzeObiekty[i]->wPol - pMojObiekt->wPol).dlugosc();
							indeks_min = i;
						}
					}

					float ilosc_p =  0;
					if (indeks_min > -1)
						ilosc_p = WyslaniePrzekazu(CudzeObiekty[indeks_min]->iID, PALIWO, 10, 0.f);

					if (ilosc_p == 0) 
						MessageBox(okno,"Paliwa nie da�o si� przekaza�, bo by� mo�e najbli�szy obiekt ruchomy znajduje si� zbyt daleko.",
						"Nie da�o si� przekaza� paliwa!",MB_OK);
					break;
				}
			case 'G':  // przekazanie 100 jednostek gotowki pojazdowi, kt�ry znajduje si� najbli�ej
				{
					float min_odleglosc = 1e10;
					int indeks_min = -1;
					for (int i=0;i<iLiczbaCudzychOb;i++)
					{
						if (min_odleglosc > (CudzeObiekty[i]->wPol - pMojObiekt->wPol).dlugosc() )
						{
							min_odleglosc = (CudzeObiekty[i]->wPol - pMojObiekt->wPol).dlugosc();
							indeks_min = i;
						}
					}

					float ilosc_p =  0;
					if (indeks_min > -1)
						ilosc_p = WyslaniePrzekazu(CudzeObiekty[indeks_min]->iID, GOTOWKA, 100, 0.f);

					if (ilosc_p == 0) 
						MessageBox(okno,"Got�wki nie da�o si� przekaza�, bo by� mo�e najbli�szy obiekt ruchomy znajduje si� zbyt daleko.",
						"Nie da�o si� przekaza� got�wki!",MB_OK);
					break;
				}
			} // switch po klawiszach

			break;
		}
	case WM_KEYUP:
		{
			switch (LOWORD(wParam))
			{
			case VK_SHIFT:
				{
					SHIFTwcisniety = 0; 
					break; 
				}        
			case VK_SPACE:
				{
					pMojObiekt->ham = 0.0;
					break;
				}
			case VK_UP:
				{
					pMojObiekt->F = 0.0;

					break;
				}
			case VK_DOWN:
				{
					pMojObiekt->F = 0.0;
					break;
				}
			case VK_LEFT:
				{
					pMojObiekt->Fb = 0.00;
					pMojObiekt->alfa = 0;	
					break;
				}
			case VK_RIGHT:
				{
					pMojObiekt->Fb = 0.00;
					pMojObiekt->alfa = 0;	
					break;
				}

			}

			break;
		}

	} // switch po komunikatach
}

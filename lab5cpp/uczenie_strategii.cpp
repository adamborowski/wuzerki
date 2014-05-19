/*
    Uczenie strategii negocjacyjnych
    Za³ó¿my, ¿e sprzedawca ma towar po cenie hurtowej = 200 i chce sprzedaæ go jak najdro¿ej
    uwzglêdniaj¹c koszty. Jeœli nie sprzeda to traci (gdy¿ negocjacje i magazynowanie generuj¹ koszty)
    Kupiec musi zdobyæ towar. Chce kupiæ go jak najtaniej od sprzedawcy. Jeœli to siê nie uda, to musi 
    kupiæ towar w sklepie za 2 razy wy¿sz¹ cenê od hurtowej = 400.


*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "alg_genetyczny.h"

FILE *f = fopen("plik.txt","w");


float koszt_kroku = 1;
float koszt_nawiazania = 10;

enum TypyAgentow {KUPIEC, SPRZEDAWCA};



// Klasa abstrakcyjna - agent negocjacyjny
class AgentN
{
public:
  int nr_kroku; 
  float oferty_sprzed[1000];
  float oferty_kupna[1000];
  float koszty;
  int typ_agenta;

  AgentN::AgentN(){}
  virtual void poczatek_negocjacji(){};

  virtual float akcja(float cena_moja_pop,float cena_oponenta,int nr_kroku){return 0;};

  float krok_negocjacji(float cena_oponenta)
  {
    float cena_moja_pop = 0;
    if (nr_kroku == 0) 
      koszty = koszt_nawiazania;
    else 
    {
      koszty += koszt_kroku;    
      cena_moja_pop = (typ_agenta == KUPIEC ? oferty_kupna[nr_kroku-1] : oferty_sprzed[nr_kroku-1]);
    }
    
    float cena_moja = akcja(cena_moja_pop, cena_oponenta, nr_kroku);

    if (typ_agenta == KUPIEC)
    {
      if ((cena_moja > -1)&&(cena_oponenta <= cena_moja))     // jeœli cena kupuj¹cego wiêksza lub równa mojej cenie    
        cena_moja = cena_oponenta;                     // wyrownanie ceny - zgoda 
      oferty_sprzed[nr_kroku] = cena_oponenta;
      oferty_kupna[nr_kroku] = cena_moja;
    }
    else
    {
      if ((cena_moja > -1)&&(cena_oponenta >= cena_moja))     // jeœli cena kupuj¹cego wiêksza lub równa mojej cenie    
        cena_moja = cena_oponenta;                     // wyrownanie ceny - zgoda 
      oferty_sprzed[nr_kroku] = cena_moja;
      oferty_kupna[nr_kroku] = cena_oponenta;
    }

    nr_kroku ++;
    return cena_moja;
  }
  
};


// Klasa prostego agenta sprzedawcy jakiegoœ towaru: przyjmuje mar¿ê 50%,
// a nastêpnie negocjuje cenê obni¿aj¹c j¹ maksymalnie o 5 jednostek
// maksymalnie 7 razy
class ProstyAgentSprzedawca: public AgentN
{
public:
  float cena_hurtowa;
  virtual void poczatek_negocjacji()
  {
    nr_kroku = 0;
    cena_hurtowa = 150+100*(float)rand()/RAND_MAX;
  }

  ProstyAgentSprzedawca::ProstyAgentSprzedawca()
  { 
    typ_agenta = SPRZEDAWCA;
  }

  virtual float akcja(float cena_moja_pop,float cena_oponenta,int nr_kroku)
  {
    float cena_moja;
    if (nr_kroku == 0) 
      cena_moja = cena_hurtowa*1.5;    
    else 
    {
      if (nr_kroku >= 7)
        cena_moja = -1;
      else
        cena_moja = cena_moja_pop - 5;
    }
	if ((cena_moja != -1)&&(cena_moja < cena_hurtowa)) cena_moja = cena_hurtowa;
    return cena_moja;
  }
};

// Klasa prostego agenta kupca, który negocjuje cenê towaru proponuj¹c
// pocz¹tkowo 70% ceny ustalonej przez sprzedawcê, a nastêpnie wybiera
// cenê œredni¹ pomiêdzy swoj¹ ostatni¹ ofert¹, a bie¿¹c¹ ofert¹ sprzedawcy, maksymalnie 20 razy
class ProstyAgentKupiec: public AgentN
{
public:
  float cena_sklepowa;

  virtual void poczatek_negocjacji()
  {
    nr_kroku = 0;
    cena_sklepowa = 300+200*(float)rand()/RAND_MAX;
  };

  ProstyAgentKupiec::ProstyAgentKupiec()
  { 
    typ_agenta = KUPIEC;
  }

  virtual float akcja(float cena_moja_pop,float cena_oponenta,int nr_kroku)
  {
    float cena_moja;
    if (nr_kroku == 0) 
      cena_moja = cena_oponenta*0.7;    
    else 
    {
      if (nr_kroku >= 20)
        cena_moja = -1;
      else
        cena_moja = cena_moja_pop + (cena_oponenta - cena_moja_pop)/2;
    }
	if (cena_moja > cena_sklepowa) cena_moja = cena_sklepowa;
    return cena_moja;
  }

};


/*
   Klasa agenta sprzedawcy realizuj¹ca ten sam schemat strategii, co prosty sprzedawca, z t¹ ró¿nic¹, ¿e
   niektóre kluczowe parametry strategii zosta³y sparametryzowane i dziêki temu mog¹ podlegaæ adaptacji - uczeniu.
   Do uczenia wykorzystano algorytm genetyczny, który szuka optymalnych parametrów strategii w stosunku do 
   strategii oponenta przekazywanej na wejœciu metody uczenie()
*/
class ProstyAgentSprzedawcaUczony: public AgentN
{
public:
  float cena_hurtowa;

  // paramerty podlegajace uczeniu:
  float marza, obnizka, liczba_krokow_max;

  virtual void poczatek_negocjacji()
  {
    nr_kroku = 0;
    cena_hurtowa = 150+100*(float)rand()/RAND_MAX;
  }

  ProstyAgentSprzedawcaUczony::ProstyAgentSprzedawcaUczony()
  { 
    typ_agenta = SPRZEDAWCA;

    marza = 0.5;
    obnizka = 5;
    liczba_krokow_max = 7;
  }


  virtual float akcja(float cena_moja_pop,float cena_oponenta,int nr_kroku)
  {
    float cena_moja;
    if (nr_kroku == 0) 
      cena_moja = cena_hurtowa*(1 + marza);    
    else 
    {
      if (nr_kroku >= liczba_krokow_max)
        cena_moja = -1;
      else
        cena_moja = cena_moja_pop - obnizka;      
    }
    if ((cena_moja != -1)&&(cena_moja < cena_hurtowa)) cena_moja = cena_hurtowa;

    return cena_moja;
  }

  float symulacja_negocjacji(bool *czy_udane, AgentN *kupiec)
  {
    bool koniec = false;
	  this->poczatek_negocjacji();
	  kupiec->poczatek_negocjacji();

    float oferta_kupna, oferta_sprz = this->krok_negocjacji(0);
    //printf("Poczatkowa oferta sprzedawcy = %f\n",oferta_sprz);
    while (!koniec)
    {
      oferta_kupna = kupiec->krok_negocjacji(oferta_sprz);
      if ((oferta_kupna == oferta_sprz)||(oferta_kupna == -1))
        break; 

      oferta_sprz = this->krok_negocjacji(oferta_kupna);
      
      if ((oferta_kupna == oferta_sprz)||(oferta_sprz == -1))
        break;

      if ((kupiec->nr_kroku == 1000)||(this->nr_kroku == 1000))
        koniec = true;      
    }

    if (oferta_kupna == oferta_sprz)
    {
      (*czy_udane) = 1;
      return oferta_kupna + koszty;
    }
    else
    {
      (*czy_udane) = 0;
      return koszty;
    }
  }


  void uczenie(AgentN *oponent,long liczba_epok)
  {
    const long liczba_osobnikow = 50;
    const long dlugosc_ciagu = 3;
    bool czy_komunikaty = 1;
	  int liczba_prob = 10;

    AlgGenetyczny ag(liczba_osobnikow,dlugosc_ciagu);

    float oceny[liczba_osobnikow];
    float ocena_max = -1e10;
    float ciag_max[dlugosc_ciagu];
    
    
    if (czy_komunikaty)
    {
      printf("ALGORYTM GENETYCZNY\n");
      fprintf(f,"ALGORYTM GENETYCZNY\n");
      printf("Szukanie optymalnej strategii negocjacji sprzedawcy w stosunku do zadanej strategii kupca\n");
      printf("liczba_epok = %d, liczba osobnikow = %d\n",liczba_epok, liczba_osobnikow);
      fprintf(f,"Szukanie optymalnej strategii negocjacji sprzedawcy w stosunku do zadanej strategii kupca\n");
      fprintf(f,"liczba_epok = %d, liczba osobnikow = %d\n",liczba_epok, liczba_osobnikow);
    }    

    for (long ep=0;ep<liczba_epok;ep++)
    {
      // ocena poszczegolnych rozwiazan: 
      for (long oso=0;oso<liczba_osobnikow;oso++)
      {
        float *ciag = ag.popul_rozw[oso];
        marza = ciag[0];
        obnizka = ciag[1];
        liczba_krokow_max = ciag[2];

		    float ocena = 0;

		    for (long pr=0;pr<liczba_prob;pr++)
		    {
			    bool czy_udane = 1;
			    float wartosc = symulacja_negocjacji(&czy_udane, oponent);	      
			    ocena += (wartosc - cena_hurtowa)/liczba_prob;
		    }
        
        if (ocena < 0) ocena = 0.00001;
        oceny[oso] = ocena;

        if (ocena_max < ocena)
        {
          ocena_max = ocena;
          for (int i=0;i<dlugosc_ciagu;i++) ciag_max[i] = ciag[i];
          if (czy_komunikaty)
          {
            printf("ep = %d, Rekord, ocena = %f, dla ciagu = (%f, %f, %f)\n",
              ep,ocena,marza,obnizka,liczba_krokow_max);
            fprintf(f,"ep = %d, Rekord, ocena = %f, dla ciagu = (%f, %f, %f)\n",
              ep,ocena,marza,obnizka,liczba_krokow_max);
          }
        }
      } // po osobnikach - rozwi¹zaniach

      ag.epoka_uczenia(oceny);

    } // for po epokach
    marza = ciag_max[0];
    obnizka = ciag_max[1];
    liczba_krokow_max = ciag_max[2];
  } // koniec metdy uczenie

};



/*
   Klasa agenta kupca realizuj¹ca ten sam schemat strategii, co prosty kupiec, z t¹ ró¿nic¹, ¿e
   niektóre kluczowe parametry strategii zosta³y sparametryzowane i dziêki temu mog¹ podlegaæ adaptacji - uczeniu.
   Do uczenia wykorzystano algorytm genetyczny, który szuka optymalnych parametrów strategii w stosunku do 
   strategii oponenta przekazywanej na wejœciu metody uczenie()
*/
class ProstyAgentKupiecUczony: public AgentN
{
public:
  float cena_sklepowa; 

  float wstepna_obnizka, prop_roznicy,liczba_krokow_max;
  virtual void poczatek_negocjacji()
  {
    nr_kroku = 0;
    cena_sklepowa = 300+200*(float)rand()/RAND_MAX;
  };

  ProstyAgentKupiecUczony::ProstyAgentKupiecUczony()
  { 
    typ_agenta = KUPIEC;

    wstepna_obnizka = 0.7;
    prop_roznicy = 0.5;
    liczba_krokow_max = 20;
  }

  virtual float akcja(float cena_moja_pop,float cena_oponenta,int nr_kroku)
  {
    float cena_moja;
    if (nr_kroku == 0) 
      cena_moja = cena_oponenta*wstepna_obnizka;    
    else 
    {
      if (nr_kroku >= liczba_krokow_max)
        cena_moja = -1;
      else
        cena_moja = cena_moja_pop + (cena_oponenta - cena_moja_pop)*prop_roznicy;
    }

    if (cena_moja > cena_sklepowa) cena_moja = cena_sklepowa;
    return cena_moja;
  }

  float symulacja_negocjacji(bool *czy_udane, AgentN *sprzedawca)
  {
    bool koniec = false;
	  this->poczatek_negocjacji();
	  sprzedawca->poczatek_negocjacji();

    float oferta_kupna, oferta_sprz = sprzedawca->krok_negocjacji(0);
    //printf("Poczatkowa oferta sprzedawcy = %f\n",oferta_sprz);
    while (!koniec)
    {
      oferta_kupna = krok_negocjacji(oferta_sprz);
      if ((oferta_kupna == oferta_sprz)||(oferta_kupna == -1))
        break; 

      oferta_sprz = sprzedawca->krok_negocjacji(oferta_kupna);
      
      if ((oferta_kupna == oferta_sprz)||(oferta_sprz == -1))
        break;

      if ((sprzedawca->nr_kroku == 1000)||(nr_kroku == 1000))
        koniec = true;      
    }

    if (oferta_kupna == oferta_sprz)
    {
      (*czy_udane) = 1;
      return oferta_kupna + koszty;
    }
    else
    {
      (*czy_udane) = 0;
      return koszty;
    }
  }


  void uczenie(AgentN *oponent, long liczba_epok)
  {

    const long liczba_osobnikow = 50;
    const long dlugosc_ciagu = 3;
    bool czy_komunikaty = 1;
	  int liczba_prob = 10;

    AlgGenetyczny ag(liczba_osobnikow,dlugosc_ciagu);

    float oceny[liczba_osobnikow];
    float ocena_max = -1e10;
    float ciag_max[dlugosc_ciagu];


    if (czy_komunikaty)
    {
      printf("ALGORYTM GENETYCZNY\n");
      fprintf(f,"ALGORYTM GENETYCZNY\n");
      printf("Szukanie optymalnej strategii negocjacji kupca w stosunku do zadanej strategii sprzedawcy\n");
      printf("liczba_epok = %d, liczba osobnikow = %d\n",liczba_epok, liczba_osobnikow);
      fprintf(f,"Szukanie optymalnej strategii negocjacji kupca w stosunku do zadanej strategii sprzedawcy\n");
      fprintf(f,"liczba_epok = %d, liczba osobnikow = %d\n",liczba_epok, liczba_osobnikow);
    }


    for (long ep=0;ep<liczba_epok;ep++)
    {
      // ocena poszczegolnych rozwiazan z rozszerzonej populacji (N osobnikow pop.podst. + M potomkow)
      for (long oso=0;oso<liczba_osobnikow;oso++)
      {
        float *ciag = ag.popul_rozw[oso];
        wstepna_obnizka = ciag[0];
        prop_roznicy = ciag[1];
        liczba_krokow_max = ciag[2];

		    float ocena = 0;
		    for (long pr=0;pr<liczba_prob;pr++)
		    {
			    bool czy_udane = 1;
			    float wartosc = symulacja_negocjacji(&czy_udane, oponent);	      
			    ocena += 1000/(0.01+wartosc + (czy_udane == 0)*cena_sklepowa);
		    }
        if (ocena < 0) ocena = 0.00001;
        oceny[oso] = ocena;

        if (ocena_max < ocena)
        {
          ocena_max = ocena;
          for (int i=0;i<dlugosc_ciagu;i++) ciag_max[i] = ciag[i];
          if (czy_komunikaty)
          {
            printf("ep = %d, Rekord, ocena = %f, dla ciagu = (%f, %f, %f)\n",
              ep,ocena,wstepna_obnizka,prop_roznicy,liczba_krokow_max);
            fprintf(f,"ep = %d, Rekord, ocena = %f, dla ciagu = (%f, %f, %f)\n",
              ep,ocena,wstepna_obnizka,prop_roznicy,liczba_krokow_max);
          }
        }
      }

      ag.epoka_uczenia(oceny);
    } // for po epokach
    wstepna_obnizka = ciag_max[0];
    prop_roznicy = ciag_max[1];
    liczba_krokow_max = ciag_max[2];
  }
};



void prosty_proces_negocjacji()
{

  ProstyAgentKupiecUczony kupiec;
  ProstyAgentSprzedawcaUczony sprzedawca;


  // uczenie naprzemienne:
  //kupiec.uczenie(&sprzedawca,10000);
  //sprzedawca.uczenie(&kupiec,10000);


  // uczenie równoleg³e:
  for (long i=0;i<10000;i++)
  {   
    sprzedawca.uczenie(&kupiec,1); 
    kupiec.uczenie(&sprzedawca,1);
  }

  
  

  kupiec.poczatek_negocjacji();
  sprzedawca.poczatek_negocjacji();
  

  bool koniec = false;
  float oferta_kupna, oferta_sprz = sprzedawca.krok_negocjacji(0);
  printf("\n\nPrzyk³adowy proces negocjacji: \n\nPoczatkowa oferta sprzedawcy = %f\n",oferta_sprz);
  fprintf(f,"\n\nPrzyk³adowy proces negocjacji: \n\nPoczatkowa oferta sprzedawcy = %f\n",oferta_sprz);
  while (!koniec)
  {
    oferta_kupna = kupiec.krok_negocjacji(oferta_sprz);
    printf("w kroku %d oferta kupca = %f\n",kupiec.nr_kroku-1,oferta_kupna);
    fprintf(f,"w kroku %d oferta kupca = %f\n",kupiec.nr_kroku-1,oferta_kupna);
    if (oferta_kupna == oferta_sprz)
    {
      printf("Zgoda kupca na kupno za cene %f\n",oferta_kupna);
      fprintf(f,"Zgoda kupca na kupno za cene %f\n",oferta_kupna);
      break; 
    }
    else if (oferta_kupna == -1)
    {
      printf("Kupiec zrywa negocjacje!\n",oferta_kupna);
      fprintf(f,"Kupiec zrywa negocjacje!\n",oferta_kupna);
      break; 
    }

    oferta_sprz = sprzedawca.krok_negocjacji(oferta_kupna);
    printf("w kroku %d oferta sprzedawcy = %f\n",sprzedawca.nr_kroku-1,oferta_sprz);
    fprintf(f,"w kroku %d oferta sprzedawcy = %f\n",sprzedawca.nr_kroku-1,oferta_sprz);
    
    if (oferta_kupna == oferta_sprz)
    {
      printf("Zgoda sprzedawcy na sprzedaz za cene %f\n",oferta_sprz);
      fprintf(f,"Zgoda sprzedawcy na sprzedaz za cene %f\n",oferta_sprz);
      break; 
    }
    else if (oferta_sprz == -1)
    {
      printf("Sprzedawca zrywa negocjacje!\n");
      fprintf(f,"Sprzedawca zrywa negocjacje!\n");
      break; 
    }

    if ((sprzedawca.nr_kroku == 1000)||(kupiec.nr_kroku == 1000))
      koniec = true;
  }

  if (oferta_kupna == oferta_sprz){
    printf("Transakcja doszla do skutku po cenie = %f\n",oferta_kupna);
    printf("Uwzgledniajac koszty sprzedawca uzyskal %f (zysk = %f), kupiec zaplacil %f (zysk = %f)\n",
      oferta_kupna - sprzedawca.koszty, 
      oferta_kupna - sprzedawca.koszty - sprzedawca.cena_hurtowa,
      oferta_kupna + kupiec.koszty,
	    kupiec.cena_sklepowa - oferta_kupna - kupiec.koszty );
    fprintf(f,"Transakcja doszla do skutku po cenie = %f\n",oferta_kupna);
    fprintf(f,"Uwzgledniajac koszty sprzedawca uzyskal %f (zysk = %f), kupiec zaplacil %f (zysk = %f)\n",
      oferta_kupna - sprzedawca.koszty, 
      oferta_kupna - sprzedawca.koszty - sprzedawca.cena_hurtowa,
      oferta_kupna + kupiec.koszty,
	    kupiec.cena_sklepowa - oferta_kupna - kupiec.koszty );
  }
  else
  {
    printf("Negocjacje zawiodly.\n");
    fprintf(f,"Negocjacje zawiodly.\n");
  }
  printf("Koszty: kupiec - %f, sprzedawca - %f\n",kupiec.koszty,sprzedawca.koszty);
  fprintf(f,"Koszty: kupiec - %f, sprzedawca - %f\n",kupiec.koszty,sprzedawca.koszty);

  long liczba_prob = 50;
  float sr_zysk_sprz = 0, sr_zysk_kupca = 0;
  for (long pr=0;pr<liczba_prob;pr++)
  {
	  kupiec.poczatek_negocjacji();
	  sprzedawca.poczatek_negocjacji();
  

	  bool koniec = false;
	  float oferta_kupna, oferta_sprz = sprzedawca.krok_negocjacji(0);
	  while (!koniec)
	  {
		oferta_kupna = kupiec.krok_negocjacji(oferta_sprz);

		if (oferta_kupna == oferta_sprz)
		  break; 
		else if (oferta_kupna == -1)
		  break; 

		oferta_sprz = sprzedawca.krok_negocjacji(oferta_kupna);

		if (oferta_kupna == oferta_sprz)
		  break; 
		else if (oferta_sprz == -1)
		  break; 

		if ((sprzedawca.nr_kroku == 1000)||(kupiec.nr_kroku == 1000))
		  koniec = true;
	  }

    sr_zysk_sprz += (oferta_kupna - sprzedawca.cena_hurtowa - kupiec.koszty)/liczba_prob;
	  sr_zysk_kupca += (kupiec.cena_sklepowa - oferta_kupna - kupiec.koszty)/liczba_prob;
  }

  printf("po %d probach negocjacji sredni zysk: kupca = %f, sprzedawcy = %f\n",liczba_prob,sr_zysk_kupca,sr_zysk_sprz);
  fprintf(f,"po %d probach negocjacji sredni zysk: kupca = %f, sprzedawcy = %f\n",liczba_prob,sr_zysk_kupca,sr_zysk_sprz);
}

int main()
{
  srand(clock());

  //uczenieAG3(); 
  //uczenieAG_SSN();

  prosty_proces_negocjacji();

  while(1){}

  fclose(f);
  return 0;
}
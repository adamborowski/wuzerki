#ifndef __glosowanie
#define __glosowanie

class Glosowanie
{
public:
	
	Glosowanie(int nrDruzyny, int iloscGlosujacych);
	int nrDruzyny;
	int werdykt;
	int idProszacego;
	int iloscGlosujacych;
	int timeout;
	bool czyPrzyjety();
	void dodajGlos(int werdykt);
	bool czyKoniec();
};

#endif
#include "glosowanie.h"

Glosowanie::Glosowanie(int nrDruzyny, int iloscGlosujacych):
	nrDruzyny(nrDruzyny),
	iloscGlosujacych(iloscGlosujacych),
	werdykt(0),
	timeout(200)
{

}
bool Glosowanie::czyPrzyjety(){
	return werdykt>0;
}
void Glosowanie::dodajGlos(int werdykt){
	this->werdykt+=werdykt;
	iloscGlosujacych--;
}
bool Glosowanie::czyKoniec()
{
	return timeout<=0|| iloscGlosujacych==0;
}
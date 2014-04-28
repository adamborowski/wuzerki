/*
 *  Klasa sprzedawcy książek.
 *  Sprzedawca dysponuje katalogiem książek oraz dwoma klasami zachowań:
 *  - OfferRequestsServer - obsługa odpowiedzi na oferty klientów
 *  - PurchaseOrdersServer - obsługa zamówienia klienta
 */

import jade.core.Agent;
import jade.core.behaviours.*;
import jade.lang.acl.*;
import java.util.*;
import java.lang.*;

public class BookSellerAgent extends Agent {

    // Katalog książek na sprzedaż:
    private Hashtable catalogue;
    int TODO_ostatniaProponowanaCena=0;

    // Inicjalizacja klasy agenta:
    protected void setup() {
        // Tworzenie katalogu książek jako tablicy rozproszonej
        catalogue = new Hashtable();

        Random randomGenerator = new Random();    // generator liczb losowych

        catalogue.put("Zamek", 35 + randomGenerator.nextInt(10));       // nazwa książki jako klucz, cena jako wartość
        catalogue.put("Proces", 20);
        catalogue.put("Opowiadania", 15);

        doWait(2000);                     // czekaj 2 sekundy

        System.out.println("Witam! Agent-sprzedawca " + getAID().getName() + " jest gotów.");

        // Dodanie zachowania obsługującego odpowiedzi na oferty klientów (kupujących książki):
        addBehaviour(new OfferRequestsServer());

        // Dodanie zachowania obsługującego zamówienie klienta:
        addBehaviour(new PurchaseOrdersServer());
    }

    // Metoda realizująca zakończenie pracy agenta:
    protected void takeDown() {
        System.out.println("Agent-sprzedawca " + getAID().getName() + " zakończył działalność.");
    }

    /**
     * Inner class OfferRequestsServer. This is the behaviour used by
     * Book-seller agents to serve incoming requests for offer from buyer
     * agents. If the requested book is in the local catalogue the seller agent
     * replies with a PROPOSE message specifying the price. Otherwise a REFUSE
     * message is sent back.
     */
    class OfferRequestsServer extends CyclicBehaviour {

        public void action() {
            // Tworzenie szablonu wiadomości (wstępne określenie tego, co powinna zawierać wiadomość)
            MessageTemplate mt = MessageTemplate.MatchPerformative(ACLMessage.CFP);
            // Próba odbioru wiadomości zgodnej z szablonem:
            ACLMessage msg = myAgent.receive(mt);
            if (msg != null) {  // jeśli nadeszła wiadomość zgodna z ustalonym wcześniej szablonem
                String title = msg.getContent();  // odczytanie tytułu zamawianej książki

                System.out.println("Agent-sprzedawca " + getAID().getName() + " otrzymał wiadomość: "
                        + title);
                ACLMessage reply = msg.createReply();               // tworzenie wiadomości - odpowiedzi
                Integer price = (Integer) catalogue.get(title);     // ustalenie ceny dla podanego tytułu
                TODO_ostatniaProponowanaCena=price;
                if (price != null) {                                // jeśli taki tytuł jest dostępny
                    reply.setPerformative(ACLMessage.PROPOSE);            // ustalenie typu wiadomości (propozycja)
                    reply.setContent(String.valueOf(price.intValue()));   // umieszczenie ceny w polu zawartości (content)
                    System.out.println("Agent-sprzedawca " + getAID().getName() + " odpowiada: "
                            + price.intValue());
                } else {                                              // jeśli tytuł niedostępny
                    // The requested book is NOT available for sale.
                    reply.setPerformative(ACLMessage.REFUSE);         // ustalenie typu wiadomości (odmowa)
                    reply.setContent("niedostępny");                  // treść wiadomości
                }
                myAgent.send(reply);                                // wysłanie odpowiedzi
            } else // jeśli wiadomość nie nadeszła, lub była niezgodna z szablonem
            {
                block();                 // blokada metody action() dopóki nie pojawi się nowa wiadomość
            }
        }
    } // Koniec klasy wewnętrznej będącej rozszerzeniem klasy CyclicBehaviour

    class PurchaseOrdersServer extends CyclicBehaviour {

        public void action() {
            ACLMessage msg = myAgent.receive();
            if (msg != null) {
                // Message received. Process it
                if(msg.getPerformative()==ACLMessage.PROPOSE){
                    TODO_ostatniaProponowanaCena-=1;
                    ACLMessage reply = msg.createReply();
                    reply.setContent(String.valueOf(TODO_ostatniaProponowanaCena));
                    
                    System.out.println("Agent-sprzedawca " + getAID().getName() + " odpowiada na negocjację {"+msg.getContent()+"} => {"+TODO_ostatniaProponowanaCena+"}");
                    myAgent.send(reply);
                }
                else if (msg.getPerformative() == ACLMessage.ACCEPT_PROPOSAL) {
                    ACLMessage reply = msg.createReply();
                    String title=reply.getContent();
                    reply.setPerformative(ACLMessage.INFORM);
                    System.out.println("Agent-sprzedawca " + getAID().getName() + " sprzedał książkę: " + title);
                    myAgent.send(reply);
                }
            }
        }
    } // Koniec klasy wewnętrznej będącej rozszerzeniem klasy CyclicBehaviour
} // Koniec klasy będącej rozszerzeniem klasy Agent

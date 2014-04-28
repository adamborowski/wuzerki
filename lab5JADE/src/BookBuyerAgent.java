/*
 *  Klasa agenta kupującego książki
 */

import jade.core.Agent;
import jade.core.AID;
import jade.core.behaviours.*;
import jade.lang.acl.*;
import java.util.*;

// Przykładowa klasa zachowania:
class MyOwnBehaviour extends Behaviour {

    protected MyOwnBehaviour() {
    }

    public void action() {
    }

    public boolean done() {
        return false;
    }
}

public class BookBuyerAgent extends Agent {

    private String targetBookTitle;    // tytuł kupowanej książki przekazywany poprzez argument wejściowy
    // lista znanych agentów sprzedających książki
    private AID[] sellerAgents = {new AID("seller1", AID.ISLOCALNAME),
        new AID("seller2", AID.ISLOCALNAME)};

    // Inicjalizacja klasy agenta:
    protected void setup() {

        //doWait(5000);   // Oczekiwanie na uruchomienie agentów sprzedających
        System.out.println("Cześć! Agent-kupiec " + getAID().getName() + " obecny!");

        Object[] args = getArguments();  // lista argumentów wejściowych (tytuł książki)

        if (args != null && args.length > 0) // jeśli podano tytuł książki
        {
            targetBookTitle = (String) args[0];
            System.out.println("Spróbuję kupić " + targetBookTitle);

            addBehaviour(new RequestPerformer());  // dodanie głównej klasy zachowań - kod znajduje się poniżej

            // Wybrane inne klasy zachowań:
            // wlasna klasa zachowania:
        /*addBehaviour(new MyOwnBehaviour());


             // gotowe klasy zachowania:
             // zachowanie uruchamiające się co stałą ilość czasu
             addBehaviour(new TickerBehaviour(this, 2000)
             {
             protected void onTick()
             {
             //myAgent.addBehaviour(new RequestPerformer());
             System.out.printf("komunikat onTick\n");
             }
             } );

             // zachowanie które pojawia się po podanym czasie (10000 milisekund)
             addBehaviour(new WakerBehaviour(this, 10000) {
             protected void handleElapsedTimeout() {
             // perform operation X
             System.out.printf("komunikat Waker\n");
             }
             } );
             */
        } else {
            // Jeśli nie przekazano poprzez argument tytułu książki, agent kończy działanie:
            System.out.println("Nie podano tytułu książki w argumentach wejściowych kupca!");
            doDelete();
        }
    }

    // Metoda realizująca zakończenie pracy agenta:
    protected void takeDown() {
        System.out.println("Agent-kupiec " + getAID().getName() + " kończy działanie.");
    }

    public enum Step {

        WYSLANIE_OFERTY_KUPNA, ODBIOR_OFERT_SPRZEDAZY_LUB_ODMOWY, WYSLANIE_NOWEJ_CENY, ODBIOR_NOWEJ_CENY, WYSLANIE_ZAMOWIENIA, ODBIOR_POTWIERDZENIA_ZAMOWIENIA, KONIEC
    };

    /**
     * Inner class RequestPerformer. This is the behaviour used by Book-buyer
     * agents to request seller agents the target book.
     */
    private class RequestPerformer extends Behaviour {

        private AID bestSeller;     // agent sprzedający z najkorzystniejszą ofertą
        private int bestPrice;      // najlepsza cena
        private int currentPrice;
        private int repliesCnt = 0; // liczba odpowiedzi od agentów
        private MessageTemplate mt; // szablon odpowiedzi
        private Step step = Step.WYSLANIE_OFERTY_KUPNA;       // krok
        private int numNegotiations=0;

        public void action() {
            switch (step) {
                case WYSLANIE_OFERTY_KUPNA:      // wysłanie oferty kupna
                    System.out.print(" Oferta kupna (CFP) jest wysyłana do: ");
                    for (int i = 0; i < sellerAgents.length; ++i) {
                        System.out.print(sellerAgents[i] + " ");
                    }
                    System.out.println();

                    // Tworzenie wiadomości CFP do wszystkich sprzedawców:
                    ACLMessage cfp = new ACLMessage(ACLMessage.CFP);
                    for (int i = 0; i < sellerAgents.length; ++i) {
                        cfp.addReceiver(sellerAgents[i]);          // dodanie adresate
                    }
                    cfp.setContent(targetBookTitle);             // wpisanie zawartości - tytułu książki
                    cfp.setConversationId("book-trade");         // wpisanie specjalnego identyfikatora korespondencji
                    cfp.setReplyWith("cfp" + System.currentTimeMillis()); // dodatkowa unikatowa wartość, żeby w razie odpowiedzi zidentyfikować adresatów
                    myAgent.send(cfp);                           // wysłanie wiadomości

                    // Utworzenie szablonu do odbioru ofert sprzedaży tylko od wskazanych sprzedawców:
                    mt = MessageTemplate.and(MessageTemplate.MatchConversationId("book-trade"),
                            MessageTemplate.MatchInReplyTo(cfp.getReplyWith()));
                    step = Step.ODBIOR_OFERT_SPRZEDAZY_LUB_ODMOWY;     // przejście do kolejnego kroku
                    break;
                case ODBIOR_OFERT_SPRZEDAZY_LUB_ODMOWY: // odbiór ofert sprzedaży/odmowy od agentów-sprzedawców
                {
                    ACLMessage reply = myAgent.receive(mt);      // odbiór odpowiedzi
                    if (reply != null) {
                        if (reply.getPerformative() == ACLMessage.PROPOSE) // jeśli wiadomość jest typu PROPOSE
                        {
                            int price = Integer.parseInt(reply.getContent());  // cena książki
                            if (bestSeller == null || price < bestPrice) // jeśli jest to najlepsza oferta
                            {
                                bestPrice = price;
                                currentPrice = (int) (bestPrice * 0.7);
                                bestSeller = reply.getSender();
                            }
                        }
                        repliesCnt++;                                        // liczba ofert
                        if (repliesCnt >= sellerAgents.length) // jeśli liczba ofert co najmniej liczbie sprzedawców
                        {
                            numNegotiations=0;
                            step = Step.WYSLANIE_NOWEJ_CENY;
                        }
                    } else {
                        block();
                    }
                    break;
                }
                case WYSLANIE_NOWEJ_CENY: {
                    numNegotiations++;
                    ACLMessage propozycjaCeny = new ACLMessage(ACLMessage.PROPOSE);
                    propozycjaCeny.addReceiver(bestSeller);
                    propozycjaCeny.setConversationId("book-trade");
                    
                    propozycjaCeny.setContent(String.valueOf(currentPrice));
                   // System.out.println(getAID()+" buyer chce "+propozycjaCeny.getContent());
                    propozycjaCeny.setReplyWith("order" + System.currentTimeMillis());
                    myAgent.send(propozycjaCeny);
                    mt = MessageTemplate.and(MessageTemplate.MatchConversationId("book-trade"),
                            MessageTemplate.MatchInReplyTo(propozycjaCeny.getReplyWith()));
                    step = Step.ODBIOR_NOWEJ_CENY;
                    break;
                }
                case ODBIOR_NOWEJ_CENY: {
                    ACLMessage reply = myAgent.receive(mt);      // odbiór odpowiedzi
                    if (reply != null) {
                       int responsePrice = Integer.parseInt(reply.getContent());  // cena książki
                       if(responsePrice<=currentPrice|| numNegotiations==7){
                           //jehuu! akceptujemy
                           step = Step.WYSLANIE_ZAMOWIENIA;
                       }
                       else{
                           //cena nie zadawala mnie, mogę negocjować, zatem powtórzę Step
                           step = Step.WYSLANIE_NOWEJ_CENY;
                           currentPrice=(currentPrice+responsePrice)/2;
                       }
                    } else {
                        block();
                    }
                    break;
                }
                case WYSLANIE_ZAMOWIENIA: // wysłanie zamówienia do sprzedawcy, który złożył najlepszą ofertę
                {
                    ACLMessage order = new ACLMessage(ACLMessage.ACCEPT_PROPOSAL);
                    order.addReceiver(bestSeller);
                    order.setContent(targetBookTitle);
                    order.setConversationId("book-trade");
                    order.setReplyWith("order" + System.currentTimeMillis());
                    myAgent.send(order);
                   
                    mt = MessageTemplate.and(MessageTemplate.MatchConversationId("book-trade"),
                            MessageTemplate.MatchInReplyTo(order.getReplyWith()));
                    step = Step.ODBIOR_POTWIERDZENIA_ZAMOWIENIA;
                    break;
                }
                case ODBIOR_POTWIERDZENIA_ZAMOWIENIA: // odbiór odpowiedzi na zamównienie
                {
                    ACLMessage reply = myAgent.receive(mt);
                    if (reply != null) {
                        if (reply.getPerformative() == ACLMessage.INFORM) {
                            System.out.println(targetBookTitle + " successfully purchased.");
                            System.out.println("Price = " + currentPrice);
                            myAgent.doDelete();
                        }
                        step = Step.KONIEC;
                    } else {
                        block();
                    }
                    break;
                }
            }  // switch
        } // action

        public boolean done() {
            return ((step == Step.WYSLANIE_ZAMOWIENIA && bestSeller == null) || step == Step.KONIEC);
        }
    } // Koniec wewnętrznej klasy RequestPerformer
}

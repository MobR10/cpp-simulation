# cpp-simulation

WORK IN PROGRESS

Functionalities:

- O logica de alocare a pachetelor catre un agent in functie de bateria lui, de pachetele deja in posesia lui si de viteza maxima pe care o poate avea;

- Un BFS pentru a determina o distanta estimativa intre 2 puncte folosit in logica de alocare a pachetelor

Validarea hartii generate cu BFS

Logica de operare a agentilor per tick(de exemplu, orice agent care nu este 100% incarcat, de fiecare data cand trece prin baza sau printr-o statie, asteapta acolo pana se incarca complet pentru a-si continua drumul)

Foloseste algoritmul A* pentru a determina distanta minima intre 2 puncte, evitand obstacole. Il foloseste pentru a determina path-ul de la agent la client sau de la agent la baza si tine cont de modul de deplasare aerian sau terestru al agentului

In genesis sunt modulele responsabile de crearea hartii

In directorul agents sunt modulele cu logica agentilor



In directorul root sunt modulele hivemind, care detine harta, logica de distribuire a pachetelor si informatii despre configurarea simularii, agentii, numarul lor, numar de pachete etc, si pathfinder, modulul care contine algoritmul A* si BFS pentru distanta estimativa.

also, ca design pattern, am folosit doar strategy. 
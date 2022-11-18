# Sprawozdanie do zadania: Pojedynek

## ***Spis Treści***
- [Treść zadania](#treść-zadania)
- [Opis problemu](#opis-problemu)
- [Opis Procesu](#opis-procesu)
  - [Wstęp](#wstęp)
  - [Pseudokod](#pseudokod)
  - [Wygląd komunikatów](#wygląd-komunikatów)
- [Rozwiązanie dostępu do sekcji krytycznych](#rozwiązanie-dostępu-do-sekcji-krytycznych)
  - [Rozwiązanie problemu parowania](#rozwiązanie-problemu-parowania)
  - [Rozwiązanie problemu Sekundantów i sal szpitalnych](#rozwiązanie-problemu-sekundantów-i-sal-szpitalnych)
  - [Rozwiązanie pojedynków](#rozwiązanie-pojedynków)

## ***Treść zadania***
W alternatywnej rzeczywistości o technologii zbliżonej do wojen napoleońskich, weterani wojen znudzeni pokojem postanowili urządzić sobie pojedynki. W tym celu najpierw zgłaszają swój udział, dobierają się w pary, następnie dobierają sekundantów, przeprowadzają pojedynek, po którym przegrany trafia pod opiekę szpitalną i na jakiś czas nie może brać udziału w dalszych pojedynkach.

Danych jest 'W' weteranów. Sekundantów jest 'S' i jest im wszystko jedno, komu sekundują. Sal szpitalnych jest 'SZ' i są nierozróżnialne.
<br><br><br>

## ***Opis problemu***
Celem zadania było obsługa sekcji krytycznych, które mogą mieć różne wielkości oraz łączenie się w pary (zawodników do pojedynków).
Dane są trzy zmienne:
 * Weterani - Procesy, 
 * Sekundanci - z uwagi na charakter zadania, czyli pojedynek może się rozegrać gdy obie osoby posiadają sekundanta - wymagana ilość sekundantów to (n/2)+1, w przypadku mniejszej ilości może dojść do sytuacji, że dla każdej z par tylko jedna osoba będzie posiadała sekundanta, co  spowoduje, że żaden pojedynek się nie rozpocznie, nie zakończy i nie zostaną zwolnione zasoby sekundantów.
 * Sale szpitalne
<br><br><br>


## ***Opis Procesu***

### ***Wstęp***
Żaden z procesów nie jest wyróżnionym ani wiodący. Założeniem było brak faworyzacji oraz całkowite rozproszenie. 
Kod procesu składa się z głównej pętli życia zawodnika, gdzie czynności jakie jest w stanie wykonać to:
 * Szukanie rywala
 * Szukanie sekundanta
 * Walka
 * Leczenie po przegranej walce
Algorytm dodatkowo składa się z Kontrolera komunikacji i weryfikatora odpowiedzi - te funkcje mają na celu jedynie wysyłać żądania i obsługiwać odpowiedzi na komunikaty przychodzące.
Opisane niżej rozwiązania poszczególnych problemów zrealizowane są za pomocą algorytmu Ricarta-Agrawali, korzystającego z zegara Lamporta jako wyznacznika priorytetów, a remisy rozstrzygane są wartością identyfikatora (większy wygrywa). 
<br><br>

### ***Pseudokod***
```
Blok główny:
dopuki proces nie zostanie zatrzymany:
       Szukaj rywala()
       Szukaj sekundanta()
       Walcz()
       jeśli poległ: 
              Lecz się()
              

Szukaj rywala():
       powtarzaj i ← 0 do n:
              jeśli i to mój identyfikator: pomiń
              wyślij REQ walki do i
              dopóki nie otrzymano ACK lub NACK od i:
                     przetwarzaj przychodzące żądania
              jeśli otrzymano ACK lub NACK:
                     zapisz rywala

Szukaj sekundanta():
       inicjuj liczbę c = 0 odebranych komunikatów do żądania
       powtarzaj i ← 0 do n:
              jeśli i to mój ID: pomiń
              wyślij REQ walki do i
       
       powtarzaj dopuki c < ilość procesów -1:
              jeśli REQ nie dotyczy mojej sekcji:
                     jeśli sekcja dotyczy rywala, to NACK
                     w przeciwnym wypadku, ACK
              jeśli REQ dotyczy mojej sekcji:
                     jeśli odebrany Lamport > wysłany Lamport, to ACK
                     jeśli odebrany Lamport == wysłany Lamport i SourceID > mój ID, to ACK
                     w p.p., zapisz proces w buforze Sekundantów
       jeśli ilość sekundantów >  ilość odmówień, przydziel zasób
       w przeciwnym przypadku ponów żądanie 

Walcz():
       l = losuj liczbę od 1 do 1000
       wyślij liczbę rywalowi
       odbierz liczbę = w od rywala
       jeśli l > w, wygrana
       w przeciwnym razie przegrana
       Wyślij ACK do procesów w buforze sekundantów

Lecz się():
       inicjuj liczbę c = 0 odebranych komunikatów do żądania
       powtarzaj i ← 0 do n:
              jeśli i to mój ID: pomiń
              wyślij REQ walki do i
       
       powtarzaj dopóki c < ilość procesów -1:
              jeśli REQ nie dotyczy mojej sekcji:
                     jeśli sekcja dotyczy rywala, to NACK
                     w przeciwnym wypadku, ACK
              jeśli REQ dotyczy mojej sekcji:
                     jeśli odebrany Lamport > wysłany Lamport, to ACK
                     jeśli odebrany Lamport == wysłany Lamport i SourceID > mój ID, to ACK
                     w p.p., zapisz proces w buforze SalSzpitalnych
       jeśli ilość SalSzpitalnych >  ilość odmówień, przydziel zasób
       w przeciwnym przypadku ponów żądanie 
       losuj czas leczenia cz = random(1 do 3) sekund
       czekaj cz
       Wyślij ACK do procesów w buforze SalSzpitalnych

```
<br><br>

### ***Wygląd komunikatów***
Przesyłana wiadomość zawsze składa sie z trzech elementów:
* msg[0] - zegar lamporta
* msg[1] - czynność
  * ***0*** - szuka rywala
  * ***1*** - szuka sekundanta
  * ***2*** - walczy
  * ***3*** - szuka sali szpitalnej
* msg[2] - typ wiadomości
  * ***-1*** - NACK
  * ***0*** - REQ
  * ***1*** - ACK
<br><br><br>


## ***Rozwiązanie dostępu do sekcji krytycznych***

### ***Rozwiązanie problemu parowania***
Wyznaczenie pary zawodników odbywa się przez komunikację REQ i ACK/NACK. Proces ubiegający się o przeciwnika wysyła wiadomość REQ z informacją o chęci walki. Przeciwnik podejmuje decyzję, czy jest zainteresowany walką, czy też nie i wysyła odpowiedni komunikat (odpowiednio ACK lub NACK). 
Złożoność komunikacyjna wynosi 2n, a czasowa 2.
<br><br>

### ***Rozwiązanie problemu Sekundantów i sal szpitalnych***
Proces ubiegający się o dostęp do sekcji krytycznej rozsyła do pozostałych procesów prośbę wraz z lokalną, bieżącą wartością zegara Lamporta. Proces otrzymujący żądanie (REQ) ma do dyspozycji kilka opcji:
 * Jeśli nie ubiega się o daną sekcję to wysyła akceptację (ACK)
 * Jeśli ubiega się o daną sekcję i jego priorytet jest większy to zapisuje proces żądający do buforu 
 * Jeśli ubiega się o daną sekcję i jego priorytet jest niższy to wysyła akceptację (ACK)

 Dozwolona jest również ścieżka skrócona, która sprawdza, czy właśnie otrzymane żądanie do tego samego zasobu ma wyższy priorytet niż wysłane przez proces ubiegający się. Jeśli tak, proces nie czeka na akceptacje tylko już wie, że ją dostanie.
 Dzięki czemu ze złożoności komunikacyjnej równej 2n chwilowo robi się n, a czasowej 2, równa 1.
 <br><br>

### ***Rozwiązanie pojedynków***
Z uwagi na charakter działania, zdecydowano się do wyłaniania zwycięzcy losowo, a nie na podstawie priorytetu. Każdy z zawodników losuje liczbę i przesyła ją do rywala. Wygrywa ten, który wylosował większą z nich.
Złożoność komunikacyjna i czasowa wynoszą 1.
 <br><br><br>

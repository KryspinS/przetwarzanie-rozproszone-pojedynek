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
SZUKANIE PARTNERA DO WALKI:
N - liczba procesów

Pi:
	ustaw priorytet = Lamport
	roześlij {REQ, priorytet}
	ustaw licznik odp = 0
	dopóki odp = 0:
		dopóki dop < N odbieraj AKC(REQ od Pn) lub NAKC:
			odp++
			jeśli AKC(REQ): zapisz priorytet Pj w buf[]

		sortuj buf min => max
		paruj buf[1]:buf[2] itd.
		jeśli Pi nie ma pary odp = 0


Pj:
	ustaw priorytet = Lamporta
	jeśli równeż szuka: (rozesłano już REQ, zachowanie jak dla Pi)
	jeśli nie szuka {NACK, priorytet}
		zapisz w buf[] {Pi, priorytet}



Pytanie o zasób Sekundant/Szpital:
N - liczna procesów
S - ograniczenie zasobu
Pi:
	ustaw priorytet = Lamport
	ustaw akc = 0
	roześlij {REQ, priorytet}
	dopóki akc-N >= S
		jeśli odebrano AKC: akc++

Pj:
	jeśli nie ubiega się o zasób: AKC
	jeśli ubiega się o zasób:
		ustaw priorytet = Lamport
		jeśli priorytet < Pi[priotytet]: wyślij AKC
		jeśli priorytet > Pi[priotytet]: 
			dodaj do buf_czekujacych[] = Pj



Zwalnianie zasobu
Pi:
	jeśli zwolniono zasób:
		wyślij do procesów z buf_czekujacych[]: AKC
```
<br><br>

### ***Wygląd komunikatów***
//TODO: porawa wyglądu komunikatów
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
Proces ubiegający się o rywala tylko raz rozsyła żądanie {REQ, priorytetem} do pozostałych procesów, jeśli okaże się, że odebrał odpowiedzi od wszystkich i nie zakwalifikował się do pary, wówczas zeruje licznik odpowiedzi i nadal nasłuchuje, bo wie, że w kolejnej kolejce przeszukiwania dostanie żądanie (REQ) od innych procesów, a pozostałe procesu mają zapamiętaną informację o tym, że Pi oczekuje / nie zakwalifikował się poprzednio - dodatkowo będzie on na szczycie listy i tym razem na pewno otrzyma parę (dlatego lista min -> max).

Jednocześnie proces Pj, który otrzymał już REQ od Pi i też szuka partnera rozesłał już swoje REQ (umowne ACK, bo nie musi dodatkowo wysyłać ACK), jedynie w przypadku braku zainteresowania zostanie przesłany NACK.
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
Z uwagi na charakter działania, zdecydowano się do wyłaniania zwycięzcy losowo, a nie na podstawie priorytetu. Każdy z zawodników losuje liczbę i przesyła ją do rywala. Wygrywa ten, który wylosował większą z nich - remisy rozstrzyga identyfikator procesu (starszy doświadczony strzelec wie lepiej jak oparzeć ranę).
Złożoność komunikacyjna i czasowa wynoszą 1.
 <br><br><br>

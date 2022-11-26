# Sprawozdanie do zadania: Pojedynek

## ***Spis Treści***
- [Sprawozdanie do zadania: Pojedynek](#sprawozdanie-do-zadania-pojedynek)
	- [***Spis Treści***](#spis-treści)
	- [***Treść zadania***](#treść-zadania)
	- [***Opis problemu***](#opis-problemu)
	- [***Opis Procesu***](#opis-procesu)
		- [***Wstęp***](#wstęp)
			- [*Plik main*](#plik-main)
			- [*Plik watek\_glowny*](#plik-watek_glowny)
			- [*Plik watek\_komunikacyjny*](#plik-watek_komunikacyjny)
			- [*Plik util*](#plik-util)
		- [***Pseudokod***](#pseudokod)
		- [***Wygląd komunikatów***](#wygląd-komunikatów)
	- [***Rozwiązanie dostępu do sekcji krytycznych***](#rozwiązanie-dostępu-do-sekcji-krytycznych)
		- [***Rozwiązanie problemu parowania***](#rozwiązanie-problemu-parowania)
		- [***Rozwiązanie problemu Sekundantów i sal szpitalnych***](#rozwiązanie-problemu-sekundantów-i-sal-szpitalnych)
		- [***Rozwiązanie pojedynków***](#rozwiązanie-pojedynków)

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
Kod procesu został podzielony na cztery pliki funkcyjne:
 * main
 * watek_glowny
 * watek_komunikacyjny
 * util
<br><br>

#### *Plik main*
Plik main zawiera inicjację wszystkich zmiennych globalnych oraz pętli głównej. Zbiera również informację o ilości sekundantów i sal szpitalnych, a także weryfikuje poziom wsparcia dla wątków. Zawiera również blok zakończenia - zwalniania pamięci i poprawnego zakończenia procesów.
<br><br>

#### *Plik watek_glowny*
Skrypt zawiera główną pętlę życia zawodnika oraz poszczególne funkcję jego działań takie jak:
 * dobieranie rywala
 * szukanie sekundanta
 * walka
 * leczenie

Poszczególne rozwiązania powyższych problemów funkcjonalnych zostały opisane w dalszej części dokumentu.
<br><br>

#### *Plik watek_komunikacyjny*
W pliku znajduje się obsługa otrzymywanych komunikatów od innych procesów i ich obsługa przez bieżący proces.
<br><br>

#### *Plik util*
Skrypt zawiera szereg funkcji wspierających proces. Znajdują się tam m.in. skrypt sortujący i wybierający rywala, skrypty zapisujące wartości zmiennych globalnych, funkcja do wysyłania wiadomości, czy inicjująca pakiet.
<br><br><br>

### ***Pseudokod***
```
DOBIERANIE PARTNERA DO WALKI:
N - liczba procesów

Pi:
	ustaw priorytet = Lamport
	roześlij {REQ, priorytet}
	ustaw licznik odp = 0
	dopóki odp = 0
	{
		dopóki dop < N odbieraj AKC(REQ od Pn) lub NAKC
		{
			odp++
			jeśli AKC(REQ): zapisz priorytet Pj w buf[]
		}

		sortuj buf min => max
		paruj buf[1]:buf[2] itd.
		jeśli Pi nie ma pary odp = 0
	}


Pj:
	ustaw priorytet = Lamporta
	jeśli również szuka: (rozesłano już REQ, zachowanie jak dla Pi)
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
		jeśli priorytet < Pi[priorytet]: wyślij AKC
		jeśli priorytet > Pi[priorytet]: 
			dodaj do buf_czekujacych[] = Pj



Zwalnianie zasobu
Pi:
	jeśli zwolniono zasób:
		wyślij do procesów z buf_czekujacych[]: AKC
```
<br><br>

### ***Wygląd komunikatów***
Przesyłana wiadomość zawsze składa się z trzech obowiązkowych elementów i jednego opcjonalnego (używanego w walce):
 * priorytet
 * źródło
 * czego dotyczy komunikat:
   * szukania rywala
   * szukania sekundanta
   * walki
   * leczenia się
 * wartość (opcjonalnie)

Dodatkowo każdy komunikat zawiera jeden z poniższych tagów:
 * REQ (żądanie)
 * ACK (akceptacja)
 * NACK (brak akceptacji - używany tylko przy szukaniu rywali)
 * FREE (zwalnianie zasobu)
<br><br><br>


## ***Rozwiązanie dostępu do sekcji krytycznych***

### ***Rozwiązanie problemu parowania***
Proces ubiegający się o rywala tylko raz rozsyła żądanie {REQ, priorytetem} do pozostałych procesów. Po odebraniu odpowiedzi od wszystkich pozostałych procesów sortuje listę rywali względem priorytetu od najmniejszej do największej wartości i względem niej dobiera w pary.

Każdy procese weryfikuje, czy jest elementem nieparzystym, jeśli tak, to sprawdza, czy jest na końcu listy, jeśli tak to zeruje licznik odpowiedzi i dalej nasłuchuje, w przeciwnym przypadku, jeśli nie jest ostatni, to bierze ostatniego za rywala, a jeśli jest parzysty to bierze poprzednika. Parzystość liczona jest od pierwszego rywala, który zaakceptował wyzwanie.

W przypadku kiedy proces nie zakwalifikował się do walki w tym rozdaniu i nadal nasłuchuje za rywalami, ci zawierają informację o oczekującym procesię oraz jego pierwotnym priorytecie, który teraz (przy najbliższym szukaniu) zostanie ustawiony na szczycie listy i z pewnością spowoduje kwalifikację do walki (zachowuje postęp i nie nastąpi sytuacja, gdzie proces będzie wiecznie czekał na rywala)

Jednocześnie proces Pj, który otrzymał już REQ od Pi i też szuka partnera rozesłał już swoje REQ (umowne ACK, bo nie musi dodatkowo wysyłać ACK), jedynie w przypadku braku zainteresowania zostanie przesłany NACK.
Złożoność komunikacyjna wynosi n i czasowa 1 (w najgorszym przypadku komunikacyjna 2n i czasowa 2).
<br><br>

### ***Rozwiązanie problemu Sekundantów i sal szpitalnych***
Proces ubiegający się o dostęp do sekcji krytycznej rozsyła do pozostałych procesów prośbę wraz z lokalną, bieżącą wartością zegara Lamporta. Proces otrzymujący żądanie (REQ) ma do dyspozycji kilka opcji:
 * Jeśli nie ubiega się o daną sekcję to wysyła akceptację (ACK)
 * Jeśli ubiega się o daną sekcję i jego priorytet jest większy to zapisuje proces żądający do buforu 
 * Jeśli ubiega się o daną sekcję i jego priorytet jest niższy to wysyła akceptację (ACK)

 Dozwolona jest również ścieżka skrócona, która sprawdza, czy właśnie otrzymane żądanie do tego samego zasobu ma wyższy priorytet niż wysłane przez proces ubiegający się. Jeśli tak, proces nie czeka na akceptacje tylko już wie, że ją dostanie.
 Dzięki czemu ze złożoności komunikacyjnej równej 2n chwilowo robi się n, a czasowej 2, 1.
 <br><br>

### ***Rozwiązanie pojedynków***
Z uwagi na charakter działania, zdecydowano się do wyłaniania zwycięzcy losowo, a nie na podstawie priorytetu. Każdy z zawodników losuje liczbę i przesyła ją do rywala. Wygrywa ten, który wylosował większą z nich - remisy rozstrzyga identyfikator procesu (starszy doświadczony strzelec wie lepiej jak opatrzeć ranę i wygrywa).
Złożoność komunikacyjna i czasowa wynoszą 1.
 <br><br><br>

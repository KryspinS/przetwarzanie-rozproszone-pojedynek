clear
mpicc Zawodnik.c -o Zawodnik 
read -p 'Podaj liczbe sekundantow: ' sek
read -p 'Podaj liczbe sal szpitalnych: ' szp
read -p 'Podaj liczbe zawodnikow: ' z
mpirun -n $z Zawodnik $sek $szp 0  | tee output.log


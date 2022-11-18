clear
mpicc Zawodnik.c -o Zawodnik 
read -p 'Podaj liczbe sekundantow: ' sek
read -p 'Podaj liczbe sal szpitalnych: ' szp
mpirun Zawodnik $sek $szp 1 | tee output.log


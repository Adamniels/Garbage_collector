# Garbage Collector-projekt

Detta projekt började som ett skolprojekt inom minneshantering och garbage collection i C.
Ursprungligen var projektet inte färdigt och saknade fungerande tester och exempeldemos som funkade.
Jag har därefter byggt vidare, fixat dem värsta buggarn, fixat testfallen och lagt till enkla demonstrationsprogram
för att visa hur garbage collectorn kan användas.

## Projektstruktur

```
Garbage_collector/
│── src/ # Källkod
│ ├── allocation.c # Minnesallokering
│ ├── heap.c # Hantering av heap
│ ├── compacting.c # Minneskopmaktering
│ ├── find_roots.c # Identifiering av rötter
│ └── lib/ # Stödjande bibliotek
│── test/ # Enhetstester
│── demos/ # Exempelprogram
│── Makefile # Byggskript
│── README.md # Dokumentation
│── TODO.md # Uppgiftslista
```

## Komma igång

1. **Kloning av repo**

   - klona repot

2. **Installation av beroenden**

   - En C-kompilator (t.ex. `gcc`)
   - CUnit (om du vill köra enhetstester). På Ubuntu:  
     `sudo apt-get update`  
     `sudo apt-get install gcc make libcunit1 libcunit1-doc libcunit1-dev`
   - Valgrind (valfritt för minnesanalys):  
     `sudo apt-get install valgrind`

3. **Kompilera projektet**  
   `make`  
   Detta genererar nödvändiga objektfiler i `obj/` samt kompilerar källkoden.

## Hur man kör olika delar

### 1. Köra enhetstester

- **Kompilera testfiler**:  
  `make compile_tests`

- **Köra tester**:  
  `make test` följt av `./unit_tests`  
  (Eller bara `make test` för att göra båda stegen på en gång.)

### 2. Köra Valgrind (minnesanalys) på tester

- **Automatiserat med Make**:  
  `make memtest`

- **Direkt via Valgrind**:  
  `valgrind --leak-check=full ./unit_tests`

### 3. Köra demos

- **linked_list_demo**:  
  `make demo_linked_list`  
  `./demo_linked_list`

- **demo_from_test**:  
  `make demo_from_test`  
  `./demo_from_test`

## Kort om implementationen

- **find_roots.c**: Ansvarar för att hitta “rötter” (pekare) i stack och globala variabler.
- **compacting.c**: Kompaktering av minnet för att undvika fragmentering.
- **allocation.c**: Hanterar anpassade allokeringsfunktioner.
- **heap.c**: Sköter heap-logiken samt datastrukturer för garbage collection.
- **test/**: Innehåller enhetstester för att validera funktionaliteten (skrivna med t.ex. CUnit).

## Fortsatt arbete

- [x] Fullständig testsvit som nu är fixad
- [x] Fungerande demoprogram
- [ ] Införa en `DEBUG`-build för felsökning
- [ ] Höja test coverage och kommentera koden mer utförligt
- [ ] Eventuell engelskspråkig dokumentation
- [ ] bygga ett större program som testar att GC verkligen funkar som den ska

# TODO:

## Prioriterade uppgifter

- [ ] gå tillbaka till compacting och se att allt funkar
  - [ ] fixa bug där jag hittar för många pekare
        har type en lösning nu, där jag jämför om den har korupterats eller inte, frågan är bara om jag
        ska strunta i den helt som jag gör nu eller faktiskt använda den på något sätt.
        frågan är om den kan skriva över en viktig/riktig pekare eller inte????
    - [x] skriva ett demo som gör samma sak som testet och se om det funkar där
    - [ ] ska använda allokerings kartan för att få bort detta beteende,
          antingen om jag ändra på hur den funkar så att den bara sätter den till
          själva starten av objektet

## Kommande uppgifter

- [ ] write a demo program
- [ ] skriva om makefilen
- [ ] skriva om att jag har byggt vidare på ett icke färdigt skolprojekt

måste gå in och kolla varför pekare i testet andra gången fortf har alloc map index 1, det borde ligga på andra sidan

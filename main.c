#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define VSTUP "prenosy.dat" /* Vstupní soubor */
#define VYSTUP "prenosy.html" /* Výstupní soubor */
#define VELIKOST_BUFFERU 100 /* Velikost pro dočasný buffer */

/* Struktura času */
typedef struct {
    int hodina;
    int minuta;
} CAS;

/* Struktura s datem */
typedef struct {
    int den;
    int mesic;
    int rok;
} DATUM;

/* Struktura přenosu */
typedef struct {
    DATUM datum;
    CAS cas;
    char uzivatel[20 + 1];
    char ip[20 + 1];
    char soubor[20 + 1];
    int data;
} PRENOS;

/* Převede formát `[xxxxxx]` na `[xxxxxx]` (vytvoří číslo bez hranatých závorek)*/
void ziskejDatoveCislo(const char *vstup_text, char *vystup_text) {
    int i = 0, j = 0;

    while (vstup_text[i] != '\0') {
        if (isdigit(vstup_text[i])) {
            vystup_text[j++] = vstup_text[i];
        }
        i++;
    }
    vystup_text[j] = '\0';
}

/* Zjistí, kolik je v souboru řádku a na základě toho vrací číslo */
int zjistiPocetPrenosu(const char *vstup) {
    FILE *soubor = fopen(vstup, "r");

    if (soubor == NULL) {
        printf("Chyba pri otevirani souboru %s.\n", vstup);
        exit(EXIT_FAILURE);
    }
    char znak;
    char posledniZnak = EOF;
    int pocetRadek = 0;
    while ((znak = (char) fgetc(soubor)) != EOF) {
        if (znak == '\n') {
            pocetRadek++;
        }
        posledniZnak = znak;
    }
    if (posledniZnak != '\n' && posledniZnak != EOF) {
        pocetRadek++;
    }
    if (fclose(soubor) == EOF) {
        printf("Chyba pri zavirani souboru %s.\n", vstup);
        exit(EXIT_FAILURE);
    }
    return pocetRadek;
}

/* Načte data ze souboru do pole struktur `prenosy` */
void nactiData(PRENOS *prenosy, const char *vstup) {
    FILE *soubor = fopen(vstup, "r");
    int i = 0;
    char buffer[VELIKOST_BUFFERU];

    if (soubor == NULL) {
        printf("Chyba pri otevirani souboru %s.\n", vstup);
        exit(EXIT_FAILURE);
    }
    /* Čte jednotlivé řádky ze souboru a z nich pak extrahuje data */
    while (fgets(buffer, VELIKOST_BUFFERU, soubor) != NULL) {
        sscanf(buffer, "%d.%d.%d %d:%d %49[^@]@%15s %49s [%ld]", &prenosy[i].datum.den, &prenosy[i].datum.mesic,
               &prenosy[i].datum.rok, &prenosy[i].cas.hodina, &prenosy[i].cas.minuta, &prenosy[i].uzivatel,
               &prenosy[i].ip, &prenosy[i].soubor, &prenosy[i].data);
        i++;
    }
    if (fclose(soubor) == EOF) {
        printf("Chyba pri zavirani souboru %s.\n", vstup);
        exit(EXIT_FAILURE);
    }
}

/* Test, jestli první oktet IP adresy je 10 */
int ipZacinaCislem(const char *cislo, PRENOS prenos) {
    if (strncmp(prenos.ip, cislo, strlen(cislo)) == 0) {
        return 1;
    }
    return 0;
}

/* Vypisuje přenosy do tabulky na obrazovku konzole */
void vypisNaObrazovku(PRENOS *prenosy, int pocetPrenosu) {
    int prenesenaDataObjem = 0;
    PRENOS nejvetsiSoubor = prenosy[0]; // Předpokládáme, že první soubor je největší
    int pocetPrenosuNejvetsihoSouboru = 0;

    printf("P R E N O S Y\n");
    printf("     datum   cas        uzivatel       ip adresa          soubor        data\n");
    printf("----------------------------------------------------------------------------\n");

    for (int i = 0; i < pocetPrenosu; i++) {
        printf("%02d.%02d.%04d %02d:%02d    %12s   %13s    %12s    %8d\n",
               prenosy[i].datum.den, prenosy[i].datum.mesic, prenosy[i].datum.rok,
               prenosy[i].cas.hodina, prenosy[i].cas.minuta,
               prenosy[i].uzivatel, prenosy[i].ip, prenosy[i].soubor, prenosy[i].data);

        prenesenaDataObjem += prenosy[i].data;
        /* Zjišťování největšího souboru */
        if (prenosy[i].data > nejvetsiSoubor.data) {
            nejvetsiSoubor = prenosy[i];
            /* Resetuje počet na 1 */
            pocetPrenosuNejvetsihoSouboru = 1;
        } else if (prenosy[i].data == nejvetsiSoubor.data) {
            /* Pokud je stejná velikost, zvyšuje počet výskytů tohoto souboru */
            if (strcmp(prenosy[i].soubor, nejvetsiSoubor.soubor) == 0) {
                pocetPrenosuNejvetsihoSouboru++;
            }
        }
    }
    printf("\n");
    printf("Pocet zaznamu: %d\n", pocetPrenosu);
    printf("Objem prenesenych dat: %d bytu\n", prenesenaDataObjem);
    printf("Nejvetsi soubor %s o velikosti %d bytu byl prenesen %d krat.\n",
           nejvetsiSoubor.soubor, nejvetsiSoubor.data, pocetPrenosuNejvetsihoSouboru);
}

/* Zapisuje data do HTML tabulky, kterou lze následně zobrazit v prohlížeči */
void vypisDoSouboru(PRENOS *prenosy, int pocetPrenosu, const char *vystup) {
    FILE *fw = fopen(vystup, "w");
    int validni = 0;

    if (fw == NULL) {
        printf("Chyba pri otevirani souboru %s.\n", vystup);
        exit(EXIT_FAILURE);
    }
    printf("\nByl vytvoren soubor %s.\n", vystup);

    fprintf(fw, "<h1>Prenosy</h1>\n");
    fprintf(fw, "<table border=\"1\"><tr><th>poradi</th><th>uzivatel</th><th>ip</th><th>datum</th><th>cas</th>"
            "<th>soubor</th><th>data</th></tr>\n");
    for (int i = 0; i < pocetPrenosu; i++) {
        /* Vypisuje pouze přenos, který má více než 500 000 bytů a zároveň je první oktet jeho IP 10 */
        if (prenosy[i].data > 500000 && ipZacinaCislem("10", prenosy[i])) {
            validni++;
            fprintf(
                fw, "<tr><td>%d</td><td>%s</td><td>%s</td><td>%d-%02d-%02d</td>"
                "<td>%02d:%02d</td><td>%s</td><td>%d</td>"
                "</tr>\n",
                validni, prenosy[i].uzivatel, prenosy[i].ip, prenosy[i].datum.rok,
                prenosy[i].datum.mesic, prenosy[i].datum.den, prenosy[i].cas.hodina,
                prenosy[i].cas.minuta, prenosy[i].soubor, prenosy[i].data);
        }
    }
    fprintf(fw, "</table>\n");
    if (fclose(fw) == EOF) {
        printf("Chyba pri zavirani souboru %s.\n", vystup);
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    /* Zjistí počet přenosů v souboru */
    int pocetPrenosu = zjistiPocetPrenosu(VSTUP);
    /* Dynamicky vytvoří pole pro přenosy na základě jejich počtu */
    PRENOS *prenosy = malloc(sizeof(PRENOS) * pocetPrenosu);
    /* Testuje, jestli se paměť správně přidělila */
    if (prenosy == NULL) {
        printf("Chyba pri pridelovani pameti.\n");
        exit(EXIT_FAILURE);
    }
    /* Načte data do dynamického pole `prenosy` */
    nactiData(prenosy, VSTUP);
    /* Vypíše tabulku do konzole */
    vypisNaObrazovku(prenosy, pocetPrenosu);
    /* Vytvoří HTML tabulku */
    vypisDoSouboru(prenosy, pocetPrenosu, VYSTUP);
    /* Uvolní dynamicky alokovanou paměť */
    free(prenosy);
    prenosy = NULL;
    return 0;
}
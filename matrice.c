#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#define TAILLE_MIN 2
#define TAILLE_MAX 10
#define VALEUR_MAX 9

int TAILLE_BUFFER;

typedef struct DonneesThread {
    int ligne;
    int colonne;
    int donnee;
} DonneesThread;

sem_t plein, vide, mutex;

int LignesMatriceA, ColonnesMatriceB, LignesA_ColonnesB;

int matriceA[TAILLE_MAX][TAILLE_MAX];
int matriceB[TAILLE_MAX][TAILLE_MAX];
int matriceResultat[TAILLE_MAX][TAILLE_MAX];

int ip = 0, ic = 0;
DonneesThread *tampon;

void generer() {
    srand(time(NULL));

    LignesMatriceA = rand() % 9;
    ColonnesMatriceB = rand() % 9;
    LignesA_ColonnesB = rand() % 9;

    TAILLE_BUFFER = ColonnesMatriceB;

    for (int i = 0; i < LignesMatriceA; ++i) {
        for (int j = 0; j < LignesA_ColonnesB; ++j) {
            matriceA[i][j] = rand() % (VALEUR_MAX + 1);
        }
    }

    for (int i = 0; i < LignesA_ColonnesB; ++i) {
        for (int j = 0; j < ColonnesMatriceB; ++j) {
            matriceB[i][j] = rand() % (VALEUR_MAX + 1);
        }
    }

}



void *producteur(void *arg) {
    DonneesThread *args = (DonneesThread *)arg;

    args->donnee = 0;
    for (int i = 0; i < LignesA_ColonnesB; i++) {
        args->donnee += matriceA[args->ligne][i] * matriceB[i][args->colonne];
    }

    sem_wait(&vide);
    sem_wait(&mutex);

    tampon[ip] = *args;
    ip = (ip + 1) % TAILLE_BUFFER;

    sem_post(&plein);
    sem_post(&mutex);
}

void *consommateur(void *arg) {
    sem_wait(&plein);
    sem_wait(&mutex);

    matriceResultat[tampon[ic].ligne][tampon[ic].colonne] = tampon[ic].donnee;
    ic = (ic + 1) % TAILLE_BUFFER;

    sem_post(&vide);
    sem_post(&mutex);
}

void afficherMatriceResultat() {
    printf("Le rÃ©sultat est :\n\n");
    for (int i = 0; i < LignesMatriceA; i++) {
        for (int j = 0; j < ColonnesMatriceB; j++) {
            printf("%d\t", matriceResultat[i][j]);
        }
        printf("\n");
    }
}

int main() {
    generer();

    int tailleResultat = LignesMatriceA * ColonnesMatriceB;
    tampon = malloc(TAILLE_BUFFER * sizeof(DonneesThread));

    sem_init(&mutex, 0, 1);
    sem_init(&plein, 0, 0);
    sem_init(&vide, 0, TAILLE_BUFFER);

    pthread_t producteurs[tailleResultat];
    pthread_t consommateurs[tailleResultat];

    int lignesMatriceA = 0;
    int colonnesMatriceB = 0;

    DonneesThread vecteur[tailleResultat];

    for (int i = 0; i < tailleResultat; i++) {
        vecteur[i].ligne = lignesMatriceA;
        vecteur[i].colonne = colonnesMatriceB;

        colonnesMatriceB++;

        if (colonnesMatriceB == ColonnesMatriceB) {
            colonnesMatriceB = 0;
            lignesMatriceA++;
        }
    }

    for (int i = 0; i < tailleResultat; i++) {
        pthread_create(&producteurs[i], NULL, &producteur, (void *)&vecteur[i]);
    }

    for (int i = 0; i < tailleResultat; i++) {
        pthread_create(&consommateurs[i], NULL, &consommateur, (void *)&vecteur[i]);
    }

    for (int i = 0; i < tailleResultat; i++) {
        pthread_join(producteurs[i], NULL);
    }

    for (int i = 0; i < tailleResultat; i++) {
        pthread_join(consommateurs[i], NULL);
    }

    afficherMatriceResultat();

    free(tampon);

    return 0;
}

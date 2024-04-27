#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 5000
#define LG_MESSAGE 256

// Fonction pour afficher les erreurs Winsock
void afficherErreur(int codeErreur) {
    wchar_t *messageErreur = NULL;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, codeErreur, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPWSTR)&messageErreur, 0, NULL);
    wprintf(L"Erreur : %ls\n", messageErreur);
    LocalFree(messageErreur);
}

int main() {
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 0), &WSAData);
    SOCKET socketEcoute;
    int iResult;

    // Crée un socket de communication
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if (socketEcoute == INVALID_SOCKET) {
        afficherErreur(WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // On prépare l’adresse d’attachement locale
    struct sockaddr_in pointDeRencontreLocal;

    // Renseigne la structure sockaddr_in avec les informations locales du serveur
    pointDeRencontreLocal.sin_family = AF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    pointDeRencontreLocal.sin_port = htons(PORT);

    // On attache le socket à une adresse et un port spécifiques
    iResult = bind(socketEcoute, (SOCKADDR *) &pointDeRencontreLocal, sizeof(pointDeRencontreLocal));
    if (iResult == SOCKET_ERROR) {
        afficherErreur(WSAGetLastError());
        closesocket(socketEcoute);
        WSACleanup();
        return 1;
    }
    printf("Socket attachée avec succès !\n");

    // On fixe la taille de la file d’attente pour les demandes de connexion
    if (listen(socketEcoute, SOMAXCONN) == SOCKET_ERROR) {
        afficherErreur(WSAGetLastError());
        closesocket(socketEcoute);
        WSACleanup();
        return 1;
    }
    printf("Socket placée en écoute passive ...\n");

    SOCKET socketDialogue;
    struct sockaddr_in pointDeRencontreDistant;
    int longueurAdresse = sizeof(pointDeRencontreDistant);
    char messageEnvoi[LG_MESSAGE];
    char messageRecu[LG_MESSAGE];
    int ecrits, lus;

    // boucle d’attente de connexion
    while (1) {
        memset(messageEnvoi, 0x00, LG_MESSAGE * sizeof(char));
        memset(messageRecu, 0x00, LG_MESSAGE * sizeof(char));
        printf("Attente d’une demande de connexion (quitter avec Ctrl-C)\n\n");

        // Attente d'une connexion entrante
        socketDialogue = accept(socketEcoute, (SOCKADDR *) &pointDeRencontreDistant, &longueurAdresse);
        if (socketDialogue == INVALID_SOCKET) {
            afficherErreur(WSAGetLastError());
            closesocket(socketEcoute);
            WSACleanup();
            return 1;
        }

        // Réception des données du client
        lus = recv(socketDialogue, messageRecu, sizeof(messageRecu), 0);
        if (lus > 0)
            printf("Message reçu du client : %s (%d octets)\n\n", messageRecu, lus);
        else if (lus == 0)
            printf("Socket fermée\n");
        else
            afficherErreur(WSAGetLastError());

        // Envoi de données vers le client
        sprintf(messageEnvoi, "ok\n");
        ecrits = send(socketDialogue, messageEnvoi, (int) strlen(messageEnvoi), 0);
        if (ecrits == SOCKET_ERROR) {
            afficherErreur(WSAGetLastError());
            closesocket(socketDialogue);
            WSACleanup();
            return 1;
        }
        printf("Message %s envoyé (%d octets)\n\n", messageEnvoi, ecrits);

        // Fermeture de la socket de dialogue
        closesocket(socketDialogue);
    }

    // Fermeture de la socket d'écoute
    iResult = closesocket(socketEcoute);
    if (iResult == SOCKET_ERROR) {
        afficherErreur(WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Nettoyage de Winsock
    WSACleanup();
    return 0;
}

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
    WSAStartup(MAKEWORD(2, 0), &WSAData); // Initialisation de Winsock

    SOCKET descripteurSocket;
    int iResult;

    descripteurSocket = socket(AF_INET, SOCK_STREAM, 0); // Création d'un socket
    if (descripteurSocket == INVALID_SOCKET) {
        afficherErreur(WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Préparation de l'adresse du serveur distant
    struct sockaddr_in pointDeRencontreDistant;
    pointDeRencontreDistant.sin_family = AF_INET;
    pointDeRencontreDistant.sin_addr.s_addr = inet_addr("127.0.0.1"); // Adresse IP du serveur
    pointDeRencontreDistant.sin_port = htons(PORT); // Port du serveur

    // Connexion au serveur
    iResult = connect(descripteurSocket, (SOCKADDR *)&pointDeRencontreDistant, sizeof(pointDeRencontreDistant));
    if (iResult == SOCKET_ERROR) {
        afficherErreur(WSAGetLastError());

        iResult = closesocket(descripteurSocket);
        if (iResult == SOCKET_ERROR) {
            afficherErreur(WSAGetLastError());
        }
        WSACleanup();
        return 1;
    }
    printf("Connexion au serveur réussie avec succès !\n");

    char messageEnvoi[LG_MESSAGE];
    char messageRecu[LG_MESSAGE];
    int ecrits, lus;

    // Envoi d'un message au serveur
    sprintf(messageEnvoi, "Hello world !\n");
    ecrits = send(descripteurSocket, messageEnvoi, (int)strlen(messageEnvoi), 0);
    if (ecrits == SOCKET_ERROR) {
        afficherErreur(WSAGetLastError());
        closesocket(descripteurSocket);
        WSACleanup();
        return 1;
    }
    printf("Message \"%s\" envoyé avec succès (%d octets)\n\n", messageEnvoi, ecrits);

    // Réception d'une réponse du serveur
    lus = recv(descripteurSocket, messageRecu, sizeof(messageRecu), 0);
    if (lus > 0)
        printf("Message reçu du serveur : %s (%d octets)\n\n", messageRecu, lus);
    else if (lus == 0)
        printf("La socket a été fermée par le serveur !\n");
    else
        afficherErreur(WSAGetLastError());

    // Fermeture de la socket
    iResult = closesocket(descripteurSocket);
    if (iResult == SOCKET_ERROR) {
        afficherErreur(WSAGetLastError());
        WSACleanup();
        return 1;
    }

    WSACleanup(); // Nettoyage de Winsock
    return 0;
}

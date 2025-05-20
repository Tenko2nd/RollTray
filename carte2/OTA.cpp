#include "OTA.h"

// --- Configuration Spécifique à l'OTA ---

const char* versionInfoUrl = "https://raw.githubusercontent.com/Tenko2nd/RollTray/carte2/version_c2.txt";

// Version actuelle du firmware compilée dans ce sketch
const char* currentFirmwareVersion_OTA = "0.0.2"; 

// --- Certificat Racine PEM pour raw.githubusercontent.com ---
const char* github_raw_root_ca_pem_OTA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB\n" \
"iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n" \
"cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n" \
"BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw\n" \
"MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV\n" \
"BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU\n" \
"aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy\n" \
"dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK\n" \
"AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B\n" \
"3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY\n" \
"tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/\n" \
"Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2\n" \
"VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT\n" \
"79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6\n" \
"c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT\n" \
"Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l\n" \
"c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee\n" \
"UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE\n" \
"Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd\n" \
"BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G\n" \
"A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF\n" \
"Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO\n" \
"VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3\n" \
"ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs\n" \
"8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR\n" \
"iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze\n" \
"Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ\n" \
"XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/\n" \
"qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB\n" \
"VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB\n" \
"L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG\n" \
"jjxDah2nGN59PRbxYvnKkKj9\n" \
"-----END CERTIFICATE-----";


// Fonction internes

// Réalise la MAJ OTA
void performFirmwareUpdate_OTA(String firmwareDownloadUrl) {
    WiFiClientSecure clientSecure;
    clientSecure.setCACert(github_raw_root_ca_pem_OTA);

    HTTPClient http;

    Serial.print("(OTA) Tentative de connexion au serveur de firmware (HTTPS): ");
    Serial.println(firmwareDownloadUrl);

    if (http.begin(clientSecure, firmwareDownloadUrl)) {
        Serial.println("(OTA) Début du téléchargement du firmware...");
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK) {
            int contentLength = http.getSize();
            if (contentLength > 0) {
                Serial.print("(OTA) Taille du firmware: "); Serial.println(contentLength);
                bool canBegin = Update.begin(contentLength);
                if (canBegin) {
                    Serial.println("(OTA) Début de l'écriture OTA...");
                    WiFiClient& stream = http.getStream();
                    size_t written = Update.writeStream(stream);

                    if (written == contentLength) {
                        Serial.println("(OTA) Écriture OTA terminée avec succès.");
                    } else {
                        Serial.println("(OTA) Erreur d'écriture OTA: Écrit seulement " + String(written) + "/" + String(contentLength));
                    }

                    if (Update.end(true)) {
                        Serial.println("(OTA) Finalisation OTA réussie!");
                        if (Update.isFinished()) {
                            Serial.println("(OTA) Mise à jour complétée. Redémarrage...");
                            ESP.restart();
                        } else {
                            Serial.println("(OTA) Erreur: Mise à jour non marquée comme terminée.");
                        }
                    } else {
                        Serial.println("(OTA) Erreur lors de la finalisation de l'Update. Erreur #: " + String(Update.getError()));
                        Serial.print("(OTA) Détail Erreur Update: "); Serial.println(Update.errorString());
                    }
                } else {
                    Serial.println("(OTA) Échec de Update.begin(). Pas assez d'espace ou autre erreur.");
                    Serial.print("(OTA) Détail Erreur Update: "); Serial.println(Update.errorString());
                }
            } else {
                Serial.println("(OTA) Taille du contenu nulle ou erreur serveur lors du téléchargement du firmware.");
            }
        } else {
            Serial.printf("(OTA) Échec du téléchargement du firmware, Code HTTP: %d. Erreur: %s\n", httpCode, http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        Serial.printf("(OTA) Échec de http.begin() pour le téléchargement du firmware: %s\n", firmwareDownloadUrl.c_str());
    }
}

// Fonction principale
void handleOTAUpdates() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("(OTA) WiFi non connecté. Impossible de vérifier les mises à jour.");
        return;
    }

    Serial.println("-------------------------------------");
    Serial.println("(OTA) Vérification des mises à jour firmware...");
    Serial.print("(OTA) Version actuelle sur l'appareil: "); Serial.println(currentFirmwareVersion_OTA);

    WiFiClientSecure clientSecure;
    clientSecure.setCACert(github_raw_root_ca_pem_OTA);

    HTTPClient http;

    Serial.print("(OTA) URL de vérification de version (HTTPS): "); Serial.println(versionInfoUrl);
    if (http.begin(clientSecure, versionInfoUrl)) {
        Serial.println("(OTA) Tentative de récupération des informations de version...");
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            payload.trim();
            Serial.print("(OTA) Réponse du serveur de version: [\n"); Serial.print(payload); Serial.println("\n]");

            int newlineIndex = payload.indexOf('\n');
            String serverVersion = "";
            String firmwareDirectUrl = "";

            if (newlineIndex != -1) {
                serverVersion = payload.substring(0, newlineIndex);
                firmwareDirectUrl = payload.substring(newlineIndex + 1);
                serverVersion.trim();
                firmwareDirectUrl.trim();
            } else {
                serverVersion = payload;
                serverVersion.trim();
            }

            Serial.print("(OTA) Version lue sur le serveur: ["); Serial.print(serverVersion); Serial.println("]");
            if (firmwareDirectUrl.length() > 0) {
                Serial.print("(OTA) URL du firmware lue sur le serveur: ["); Serial.print(firmwareDirectUrl); Serial.println("]");
            }

            if (serverVersion.length() > 0 && serverVersion.compareTo(currentFirmwareVersion_OTA) > 0) {
                if (firmwareDirectUrl.length() > 0 && firmwareDirectUrl.startsWith("https://")) {
                    Serial.println("(OTA) Nouvelle version disponible. Lancement de la mise à jour...");
                    performFirmwareUpdate_OTA(firmwareDirectUrl);
                } else {
                    Serial.println("(OTA) Nouvelle version détectée, mais l'URL du firmware est manquante ou invalide.");
                }
            } else if (serverVersion.length() == 0) {
                Serial.println("(OTA) Le fichier de version sur le serveur est vide ou mal formaté.");
            } else {
                Serial.println("(OTA) Pas de nouvelle version disponible.");
            }
        } else {
            Serial.printf("(OTA) Échec de la récupération des infos de version, Code HTTP: %d. Erreur: %s\n", httpCode, http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        Serial.printf("(OTA) Échec de http.begin() pour les infos de version: %s\n", versionInfoUrl);
    }
    Serial.println("-------------------------------------");
}
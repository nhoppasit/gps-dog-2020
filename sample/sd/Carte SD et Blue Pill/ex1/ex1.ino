/****************************************************************
 
  Exemple d'écriture et lecture de carte SD avec une Blue Pill
  Un bouton poussoir ajoute une ligne dans un fichier, l'autre
  bouton lit le fichier et communique son contenu dans le moniteur série.
  
  Plus d'infos:
  https://electroniqueamateur.blogspot.com/2019/02/carte-sd-et-blue-pill-stm32duino.html

*****************************************************************/

// blibliothèques nécessaires pour la carte SD
// (déjà installées dans l'IDE)
#include <SPI.h>
#include <SD.h>

const int boutonEcr = PB12; // broche du bouton "enregistrer"
const int boutonLire = PB13; // broche du bouton "lire"

// variables utiles pour la précédure anti-rebond
int etatBoutonEcr, etatBoutonLire;   //état actuel des boutons
int ancEtatBoutonEcr = 0, ancEtatBoutonLire = 0;   // état précédent des 2 boutons
unsigned long heureRebondEcr = 0, heureRebondLire = 0;  // à quel moment à eu lieu le plus récent rebond
unsigned long delaiRebond = 50;

File monFichier;
int compteur = 0;

void setup() {

  Serial.begin(9600); // initialisation de la communication série
  delay(100); // pour éviter que des caractères étranges apparaissent dans le moniteur série

  // les deux boutons définis en entrée
  pinMode(boutonEcr, INPUT);
  pinMode(boutonLire, INPUT);

  Serial.print("Initialisation de la carte SD...");

  if (!SD.begin(PA4)) {  // "PA4" est la broche de la Blue Pill branchée au CS du lecteur de carte
    Serial.println("echec!");
    while (1);
  }
  Serial.println("reussie");
}

void loop() {

  int resultat;

  // bouton "enregisrer"

  resultat  = digitalRead(boutonEcr);

  if (resultat != ancEtatBoutonEcr) {
    heureRebondEcr = millis();
  }

  if ((millis() - heureRebondEcr) > delaiRebond) {
    if (resultat != etatBoutonEcr) {
      etatBoutonEcr = resultat;

      if (etatBoutonEcr == 1) {
        // on a appuyé sur le bouton enregistrer
        // ouverture (ou création) du fichier "Archives.txt")
        monFichier = SD.open("Archives.txt", FILE_WRITE);
        // si l'ouverture a réussi, on écrit à l'intérieur
        if (monFichier) {
          Serial.print("Ecriture dans le fichier Archives.txt ... ");  // dans le moniteur série
          monFichier.print("Valeur du compteur:  ");  // dans le fichier
          monFichier.println(compteur);
          compteur++;
          // on referme le fichier
          monFichier.close();
          Serial.println("reussie.");
        } else {
          // si l'ouverture du fichier a échoué:
          Serial.println("Erreur d'ouverture du fichier Archives.txt");
        }
      }
    }
  }

  ancEtatBoutonEcr = resultat;


  // bouton "lecture"

  resultat  = digitalRead(boutonLire);

  if (resultat != ancEtatBoutonLire) {
    heureRebondLire = millis();
  }

  if ((millis() - heureRebondLire) > delaiRebond) {
    if (resultat != etatBoutonLire) {
      etatBoutonLire = resultat;

      if (etatBoutonLire == 1) {
        // on a appuyé sur le bouton lire, alors on ouvre le fichier
        monFichier = SD.open("Archives.txt");
        if (monFichier) {
          Serial.println("Contenu du fichier Archives.txt:");
          // on lit le contenu du fichier jusqu'à la fin
          while (monFichier.available()) {
            Serial.write(monFichier.read()); // transcription dans le moniteur série
          }
          // fermeture du fichier:
          monFichier.close();
        } else {
          // en cas d'erreur lors de l'ouverture du fichier:
          Serial.println("Erreur d'ouverture du fichier Archives.txt");
        }
      }
    }
  }

  ancEtatBoutonLire = resultat;

}


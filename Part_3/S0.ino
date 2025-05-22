#include <SPI.h> // Dołączenie biblioteki do komunikacji SPI
#include <LoRa.h> // Dołączenie biblioteki LoRa

// Definicje pinów dla modułu LoRa (dostosuj do swojego modułu)
// Standardowe piny SPI dla Arduino Uno:
// MOSI: 11
// MISO: 12
// SCK: 13
#define NSS_PIN 10 // Pin Chip Select (Slave Select)
#define NRESET_PIN 9 // Pin Reset modułu LoRa
#define DIO0_PIN 2 // Pin DIO0 (wymagany do obsługi przerwań, np. TxDone)

// Licznik wysłanych pakietów
int counter = 0;

void setup() {
  // Inicjalizacja komunikacji szeregowej z komputerem (przez USB)
  Serial.begin(9600);
  while (!Serial); // Czekaj na otwarcie portu szeregowego (szczególnie dla Arduino Leonardo, Micro, itp.)

  Serial.println("Start programu - Nadajnik LoRa");

  // Ustawienie pinów dla modułu LoRa
  LoRa.setPins(NSS_PIN, NRESET_PIN, DIO0_PIN);

  // Inicjalizacja modułu LoRa na odpowiedniej częstotliwości
  // Typowe częstotliwości:
  // - 868E6 (868 MHz) - Europa
  if (!LoRa.begin(868E6)) { // Tutaj ustawiono 868 MHz
    Serial.println("Błąd inicjalizacji modułu LoRa!");
    while (1); // Zatrzymaj program w przypadku błędu
  }
  Serial.println("Moduł LoRa zainicjalizowany pomyślnie!");

  // Możesz dostosować parametry transmisji LoRa, jeśli potrzebujesz:
  // LoRa.setSpreadingFactor(7);       // Wartości od 6 do 12. Domyślnie 7.
  // LoRa.setSignalBandwidth(125E3);   // Dostępne: 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, 500E3. Domyślnie 125E3.
  // LoRa.setCodingRate4(5);         // Wartości od 5 do 8 (oznaczają 4/5, 4/6, 4/7, 4/8). Domyślnie 5.
  // LoRa.setPreambleLength(8);        // Domyślnie 8.
  // LoRa.setSyncWord(0xF3);           // Domyślnie 0x12. Musi być taki sam na nadajniku i odbiorniku.
  // LoRa.setTxPower(17);              // Moc nadawania w dBm (np. od 2 do 20, zależnie od modułu). Domyślnie 17.
}

void loop() {
  Serial.print("Wysyłanie pakietu: ");
  Serial.println(counter);

  // Rozpocznij tworzenie pakietu LoRa
  LoRa.beginPacket();
  // Dodaj dane do pakietu (mogą to być stringi, liczby, etc.)
  LoRa.print("Witaj LoRa! ");
  LoRa.print(counter);
  // Zakończ i wyślij pakiet
  LoRa.endPacket();

  // Zwiększ licznik
  counter++;

  // Czekaj 5 sekund przed wysłaniem kolejnego pakietu
  delay(5000);
}
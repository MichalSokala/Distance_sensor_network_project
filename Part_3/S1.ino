#include <SPI.h>  // Dołączenie biblioteki do komunikacji SPI
#include <LoRa.h> // Dołączenie biblioteki LoRa

// Definicje pinów dla modułu LoRa (dostosuj do swojego modułu)
// Standardowe piny SPI dla Arduino Uno:
// MOSI: 11
// MISO: 12
// SCK: 13
#define NSS_PIN 10    // Pin Chip Select (Slave Select)
#define NRESET_PIN 9  // Pin Reset modułu LoRa
#define DIO0_PIN 2    // Pin DIO0 (używany do przerwania RxDone)

void BlinkLED() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(5000);
  digitalWrite(LED_BUILTIN, LOW);
}


void setup() {
  // Inicjalizacja komunikacji szeregowej z komputerem (przez USB)
  Serial.begin(9600);
  while (!Serial); // Czekaj na otwarcie portu szeregowego

  Serial.println("Start programu - Odbiornik LoRa");

  // Ustawienie pinów dla modułu LoRa
  LoRa.setPins(NSS_PIN, NRESET_PIN, DIO0_PIN);

  // Inicjalizacja modułu LoRa na tej samej częstotliwości co nadajnik
  // Pamiętaj, aby użyć tej samej częstotliwości, co w kodzie nadajnika!
  // Domyślnie w poprzednim przykładzie nadajnika było 868E6.
  long frequency = 868E6; // Europa
  // Inne popularne: 433E6 (Europa, Azja), 915E6 (Ameryka Pn./Pd., Australia)

  if (!LoRa.begin(frequency)) {
    Serial.println("Błąd inicjalizacji modułu LoRa!");
    while (1); // Zatrzymaj program w przypadku błędu
  }
  Serial.print("Moduł LoRa zainicjalizowany pomyślnie na częstotliwości: ");
  Serial.println(frequency);

  // --- WAŻNE: Ustawienia parametrów transmisji ---
  // Poniższe parametry MUSZĄ być takie same jak w nadajniku,
  // aby urządzenia mogły się komunikować.
  // Jeśli zmieniałeś je w nadajniku, zmień je również tutaj.

  // LoRa.setSpreadingFactor(7);       // Domyślnie: 7. Zakres: 6-12.
  // LoRa.setSignalBandwidth(125E3);   // Domyślnie: 125E3. Inne np: 62.5E3, 250E3.
  // LoRa.setCodingRate4(5);         // Domyślnie: 5 (4/5). Zakres: 5-8.
  // LoRa.setPreambleLength(8);        // Domyślnie: 8.
  // LoRa.setSyncWord(0x12);           // Domyślnie: 0x12. Zakres: 0x00-0xFF.

  // Możesz również włączyć CRC (Cyclic Redundancy Check) dla lepszej integralności danych
  // LoRa.enableCrc(); // Domyślnie CRC jest wyłączone w tej bibliotece dla trybu явnego nagłówka (explicit header mode)
                      // Jeśli nadajnik wysyła z CRC, odbiornik też musi go oczekiwać.
                      // Domyślnie nadajnik z poprzedniego przykładu nie używał CRC.

  // Ustawienie modułu w tryb ciągłego nasłuchu
  // Można też użyć LoRa.onReceive(onReceiveCallback); dla obsługi opartej na przerwaniach.
  // Dla prostoty, tutaj będziemy odpytywać w pętli loop().
  Serial.println("Oczekiwanie na pakiety LoRa...");
}

void loop() {
  // Spróbuj sparsować przychodzący pakiet
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    // Odebrano pakiet
    Serial.print("Odebrano pakiet '");
    BlinkLED();
    // Odczytaj pakiet
    String receivedText = "";
    while (LoRa.available()) {
      receivedText += (char)LoRa.read();
    }
    Serial.print(receivedText);
    Serial.println("'");

    // Wyświetl siłę sygnału (RSSI) i stosunek sygnału do szumu (SNR)
    Serial.print("RSSI: ");
    Serial.print(LoRa.packetRssi());
    Serial.println(" dBm");

    Serial.print("SNR: ");
    Serial.print(LoRa.packetSnr());
    Serial.println(" dB");

    Serial.println("---"); // Separator
  }
  // Małe opóźnienie, aby nie obciążać zbytnio procesora,
  // chociaż w przypadku odpytywania, częste sprawdzanie jest pożądane.
  // delay(10);
}

/*
// Opcjonalnie: Funkcja callback dla obsługi opartej na przerwaniach (bardziej zaawansowane)
// Aby z niej skorzystać, odkomentuj ją oraz linię LoRa.onReceive(onReceiveCallback); w setup()
// i zakomentuj/usuń kod odpytujący LoRa.parsePacket() w loop().
// Pamiętaj, że funkcje callback wywoływane z przerwań powinny być jak najkrótsze.
// Nie używaj w nich długich operacji jak Serial.print() czy delay().
void onReceiveCallback(int packetSize) {
  if (packetSize == 0) return; // Jeśli nie ma pakietu, wyjdź

  // Odczytaj pakiet
  String receivedText = "";
  while (LoRa.available()) {
    receivedText += (char)LoRa.read();
  }

  // Tutaj możesz ustawić flagę lub zapisać dane do bufora,
  // a następnie przetworzyć je w głównej pętli loop().
  // Na przykład:
  // global_received_data = receivedText;
  // global_packet_received_flag = true;

  // Dla celów demonstracyjnych, jeśli chcesz szybko coś zobaczyć (niezalecane w produkcji):
  // Serial.print("Odebrano (callback): '");
  // Serial.print(receivedText);
  // Serial.println("'");
  // Serial.print("RSSI: ");
  // Serial.println(LoRa.packetRssi());
}
*/
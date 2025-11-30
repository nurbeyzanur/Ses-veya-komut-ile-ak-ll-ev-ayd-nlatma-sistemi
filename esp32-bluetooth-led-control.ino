#include "BluetoothSerial.h"

// Bluetooth Serial nesnesini oluşturma
BluetoothSerial SerialBT;

// Yapılandırma ve sabitler
#define VOICE_TIMEOUT 5000 // 5 saniye ses tanıma zaman aşımı
#define LDR_INTERVAL 5000  // LDR okuma aralığı (5 saniye)

// PWM konfigürasyonu
const int freq = 5000;
const int resolution = 8; // 0-255 arası çözünürlük

// Mekan yapısı
struct Room {
  const char* name;
  // RGB LED pinleri
  int redPin;
  int greenPin;
  int bluePin;
  // PWM kanalları
  int redChannel;
  int greenChannel;
  int blueChannel;
  // Renk değerleri
  int red;
  int green;
  int blue;
  // Parlaklık
  int brightness;
  // Durum
  bool active;
};

// Mekan tanımlamaları
Room mutfak = {"Mutfak", 5, 4, 2, 0, 1, 2, 0, 0, 0, 255, false};
Room oturma = {"Oturma Odası", 14, 12, 13, 3, 4, 5, 0, 0, 0, 255, false};
Room salon = {"Salon", 18, 19, 21, 6, 7, 8, 0, 0, 0, 255, false};

// LDR pin
const int ldrPin = 34; // Işık sensörü analog giriş pini
unsigned long lastLdrRead = 0;
int ldrValue = 0;

// Komut işleme
String receivedCommand = "";
boolean newCommand = false;

// Ses tanıma için
unsigned long lastVoiceTime = 0;
boolean voiceCommandMode = false;

// Renk değerleri (RGB formatında)
const int COLOR_RED[] = {255, 0, 0};
const int COLOR_GREEN[] = {0, 255, 0};
const int COLOR_BLUE[] = {0, 0, 255};
const int COLOR_YELLOW[] = {255, 255, 0};
const int COLOR_PURPLE[] = {255, 0, 255};
const int COLOR_CYAN[] = {0, 255, 255};
const int COLOR_WHITE[] = {255, 255, 255};
const int COLOR_OFF[] = {0, 0, 0};

void setup() {
  // Seri haberleşmeyi başlat
  Serial.begin(115200);

  // Bluetooth'u başlat
  SerialBT.begin("ESP32_Ev_Kontrolu"); // Bluetooth ismi
  Serial.println("Bluetooth ses kontrolü aktif, cihaz keşfedilebilir durumda");

  // Mekanları yapılandır
  setupRoom(&mutfak);
  setupRoom(&oturma);
  setupRoom(&salon);

  // LDR pini giriş olarak ayarla
  pinMode(ldrPin, INPUT);

  // Başlangıçta tüm LED'leri kapat
  updateRoomLEDs(&mutfak, 0, 0, 0);
  updateRoomLEDs(&oturma, 0, 0, 0);
  updateRoomLEDs(&salon, 0, 0, 0);

  // Başlangıç mesajını gönder
  SerialBT.println("ESP32 Ev Kontrol Sistemi hazır. 'Dinle' komutunu söyleyin veya gönderin.");
  showHelp();
}

void loop() {
  // Bluetooth'dan veri geldi mi kontrol et
  if (SerialBT.available()) {
    receivedCommand = SerialBT.readStringUntil('\n');
    receivedCommand.trim(); // Boşlukları ve yeni satır karakterlerini temizle
    newCommand = true;

    // Gelen komutu seri monitöre yazdır
    Serial.print("Alınan komut: ");
    Serial.println(receivedCommand);

    // Ses kontrolü modunu kontrol et
    if (receivedCommand.equalsIgnoreCase("dinle") || 
        receivedCommand.equalsIgnoreCase("listen")) {
      voiceCommandMode = true;
      lastVoiceTime = millis();
      SerialBT.println("Sizi dinliyorum... (5 saniye)");
      blinkAllLEDs(); // Dinleme moduna geçtiğini belirtmek için LED'leri yakıp söndür
    }
  }

  // Ses komut modu aktif mi kontrol et
  if (voiceCommandMode) {
    // Zaman aşımı kontrolü
    if (millis() - lastVoiceTime > VOICE_TIMEOUT) {
      voiceCommandMode = false;
      SerialBT.println("Ses dinleme süresi doldu. Tekrar dinlemek için 'Dinle' deyin.");
    }

    // Ses komutu geldi mi?
    if (newCommand && !receivedCommand.equalsIgnoreCase("dinle") && 
        !receivedCommand.equalsIgnoreCase("listen")) {
      processCommand(receivedCommand);
      lastVoiceTime = millis(); // Komut geldiğinde süreyi yenile
    }
  } else {
    // Normal mod - doğrudan komutları işle
    if (newCommand) {
      processCommand(receivedCommand);
    }
  }

  // LDR değerini düzenli olarak oku ve bildir
  if (millis() - lastLdrRead > LDR_INTERVAL) {
    ldrValue = analogRead(ldrPin);
    lastLdrRead = millis();
    Serial.print("LDR değeri: ");
    Serial.println(ldrValue);
  }

  newCommand = false;
}

// Mekanı yapılandırma
void setupRoom(Room* room) {
  // PWM kanallarını konfigüre et
  ledcSetup(room->redChannel, freq, resolution);
  ledcSetup(room->greenChannel, freq, resolution);
  ledcSetup(room->blueChannel, freq, resolution);
  
  // Pinleri kanallara bağla
  ledcAttachPin(room->redPin, room->redChannel);
  ledcAttachPin(room->greenPin, room->greenChannel);
  ledcAttachPin(room->bluePin, room->blueChannel);
}

// Komut işleme
void processCommand(String command) {
  command.toLowerCase(); // Komutu küçük harfe çevir (büyük/küçük harf duyarlılığını kaldırmak için)

  // Yardım komutu
  if (command.indexOf("yardım") >= 0 || command.indexOf("help") >= 0 ||
      command.indexOf("komut") >= 0) {
    showHelp();
    return;
  }

  // LDR değerini oku ve bildir
  if (command.indexOf("ldr") >= 0 || command.indexOf("ölç") >= 0 || 
      command.indexOf("ışık ölç") >= 0 || command.indexOf("ortam") >= 0) {
    readAndReportLDR();
    return;
  }

  // Parlaklık komutları
  if (command.indexOf("azalt") >= 0 || command.indexOf("düşür") >= 0 || 
      command.indexOf("yükselt") >= 0 || command.indexOf("artır") >= 0 || 
      command.indexOf("parlaklık") >= 0) {
    processBrightnessCommand(command);
    return;
  }

  // Mekan ve renk kontrolü
  String mekan = "";
  String renk = "";

  // Mekan belirleme
  if (command.indexOf("mutfak") >= 0) {
    mekan = "mutfak";
  } else if (command.indexOf("oturma") >= 0 || command.indexOf("oda") >= 0) {
    mekan = "oturma";
  } else if (command.indexOf("salon") >= 0) {
    mekan = "salon";
  } else if (command.indexOf("hepsi") >= 0 || command.indexOf("tüm") >= 0 || command.indexOf("all") >= 0) {
    mekan = "hepsi";
  }

  // Kapatma kontrolü
  if (command.indexOf("kapat") >= 0 || command.indexOf("off") >= 0 || command.indexOf("söndür") >= 0) {
    switchOff(mekan);
    return;
  }

  // Renk belirleme
  if (command.indexOf("kırmızı") >= 0 || command.indexOf("kirmizi") >= 0 || command.indexOf("red") >= 0) {
    renk = "kirmizi";
  } else if (command.indexOf("yeşil") >= 0 || command.indexOf("yesil") >= 0 || command.indexOf("green") >= 0) {
    renk = "yesil";
  } else if (command.indexOf("mavi") >= 0 || command.indexOf("blue") >= 0) {
    renk = "mavi";
  } else if (command.indexOf("sarı") >= 0 || command.indexOf("sari") >= 0 || command.indexOf("yellow") >= 0) {
    renk = "sari";
  } else if (command.indexOf("mor") >= 0 || command.indexOf("purple") >= 0) {
    renk = "mor";
  } else if (command.indexOf("turkuaz") >= 0 || command.indexOf("cyan") >= 0) {
    renk = "turkuaz";
  } else if (command.indexOf("beyaz") >= 0 || command.indexOf("white") >= 0) {
    renk = "beyaz";
  }

  // Mekan ve renk bilgisi varsa uygula
  if (mekan != "" && renk != "") {
    setColor(mekan, renk);
  } else if (mekan != "" && (command.indexOf("aç") >= 0 || command.indexOf("on") >= 0 || command.indexOf("yak") >= 0)) {
    // Sadece mekan bilgisi varsa ve açma komutu geldi ise son ayarlanan renkle aç
    switchOn(mekan);
  } else if (renk != "") {
    // Sadece renk bilgisi varsa aktif olan mekanlara uygula
    applyColorToActiveLights(renk);
  } else if (!command.equalsIgnoreCase("dinle") && !command.equalsIgnoreCase("listen")) {
    SerialBT.println("Anlaşılamadı: '" + command + "'. Komutlar için 'yardım' yazın.");
  }
}

// Parlaklık komutlarını işler
void processBrightnessCommand(String command) {
  String mekan = "";

  // Hangi mekana uygulanacağını belirle
  if (command.indexOf("mutfak") >= 0) {
    mekan = "mutfak";
  } else if (command.indexOf("oturma") >= 0 || command.indexOf("oda") >= 0) {
    mekan = "oturma";
  } else if (command.indexOf("salon") >= 0) {
    mekan = "salon";
  } else {
    mekan = "hepsi"; // Mekan belirtilmemişse tümüne uygula
  }

  // Parlaklık azaltma
  if (command.indexOf("azalt") >= 0 || command.indexOf("düşür") >= 0) {
    adjustBrightness(mekan, -64);
  }
  // Parlaklık yükseltme
  else if (command.indexOf("yükselt") >= 0 || command.indexOf("artır") >= 0) {
    adjustBrightness(mekan, 64);
  }
}

// LDR değerini oku ve bildir
void readAndReportLDR() {
  ldrValue = analogRead(ldrPin);
  SerialBT.print("LDR değeri: ");
  SerialBT.println(ldrValue);
  // 0-4095 aralığını 0-100 aralığına dönüştür
  int percent = map(ldrValue, 0, 4095, 0, 100);
  SerialBT.print("Ortam ışığı seviyesi: %");
  SerialBT.println(percent);
}

// Belirtilen mekana belirtilen rengi uygular
void setColor(String mekan, String renk) {
  int r = 0, g = 0, b = 0;

  // Renk değerlerini belirle
  if (renk == "kirmizi") {
    r = COLOR_RED[0]; g = COLOR_RED[1]; b = COLOR_RED[2];
  } else if (renk == "yesil") {
    r = COLOR_GREEN[0]; g = COLOR_GREEN[1]; b = COLOR_GREEN[2];
  } else if (renk == "mavi") {
    r = COLOR_BLUE[0]; g = COLOR_BLUE[1]; b = COLOR_BLUE[2];
  } else if (renk == "sari") {
    r = COLOR_YELLOW[0]; g = COLOR_YELLOW[1]; b = COLOR_YELLOW[2];
  } else if (renk == "mor") {
    r = COLOR_PURPLE[0]; g = COLOR_PURPLE[1]; b = COLOR_PURPLE[2];
  } else if (renk == "turkuaz") {
    r = COLOR_CYAN[0]; g = COLOR_CYAN[1]; b = COLOR_CYAN[2];
  } else if (renk == "beyaz") {
    r = COLOR_WHITE[0]; g = COLOR_WHITE[1]; b = COLOR_WHITE[2];
  }

  // Mekan(lar)a uygula
  if (mekan == "mutfak") {
    updateRoomLEDs(&mutfak, r, g, b);
    mutfak.active = true;
    SerialBT.println("Mutfak " + renk + " rengine ayarlandı");
  } else if (mekan == "oturma") {
    updateRoomLEDs(&oturma, r, g, b);
    oturma.active = true;
    SerialBT.println("Oturma odası " + renk + " rengine ayarlandı");
  } else if (mekan == "salon") {
    updateRoomLEDs(&salon, r, g, b);
    salon.active = true;
    SerialBT.println("Salon " + renk + " rengine ayarlandı");
  } else if (mekan == "hepsi") {
    updateRoomLEDs(&mutfak, r, g, b);
    updateRoomLEDs(&oturma, r, g, b);
    updateRoomLEDs(&salon, r, g, b);
    mutfak.active = true;
    oturma.active = true;
    salon.active = true;
    SerialBT.println("Tüm mekanlar " + renk + " rengine ayarlandı");
  }
}

// Aktif olan mekanlara renk uygular
void applyColorToActiveLights(String renk) {
  boolean anyActive = false;

  // Aktif mekan var mı kontrol et
  if (mutfak.active || oturma.active || salon.active) {
    anyActive = true;
  } else {
    // Hiçbir mekan aktif değilse hepsini aktif et
    mutfak.active = true;
    oturma.active = true;
    salon.active = true;
    anyActive = true;
  }

  if (anyActive) {
    // Aktif olan mekanlara rengi uygula
    if (mutfak.active) setColor("mutfak", renk);
    if (oturma.active) setColor("oturma", renk);
    if (salon.active) setColor("salon", renk);
  }
}

// Belirtilen mekanı kapat
void switchOff(String mekan) {
  if (mekan == "mutfak") {
    updateRoomLEDs(&mutfak, 0, 0, 0);
    mutfak.active = false;
    SerialBT.println("Mutfak ışıkları kapatıldı");
  } else if (mekan == "oturma") {
    updateRoomLEDs(&oturma, 0, 0, 0);
    oturma.active = false;
    SerialBT.println("Oturma odası ışıkları kapatıldı");
  } else if (mekan == "salon") {
    updateRoomLEDs(&salon, 0, 0, 0);
    salon.active = false;
    SerialBT.println("Salon ışıkları kapatıldı");
  } else if (mekan == "hepsi") {
    updateRoomLEDs(&mutfak, 0, 0, 0);
    updateRoomLEDs(&oturma, 0, 0, 0);
    updateRoomLEDs(&salon, 0, 0, 0);
    mutfak.active = false;
    oturma.active = false;
    salon.active = false;
    SerialBT.println("Tüm ışıklar kapatıldı");
  } else {
    SerialBT.println("Kapatılacak mekan belirtilmedi");
  }
}

// Belirtilen mekanı aç (son kullanılan renkle)
void switchOn(String mekan) {
  if (mekan == "mutfak") {
    updateRoomLEDs(&mutfak, mutfak.red, mutfak.green, mutfak.blue);
    mutfak.active = true;
    SerialBT.println("Mutfak ışıkları açıldı");
  } else if (mekan == "oturma") {
    updateRoomLEDs(&oturma, oturma.red, oturma.green, oturma.blue);
    oturma.active = true;
    SerialBT.println("Oturma odası ışıkları açıldı");
  } else if (mekan == "salon") {
    updateRoomLEDs(&salon, salon.red, salon.green, salon.blue);
    salon.active = true;
    SerialBT.println("Salon ışıkları açıldı");
  } else if (mekan == "hepsi") {
    updateRoomLEDs(&mutfak, mutfak.red, mutfak.green, mutfak.blue);
    updateRoomLEDs(&oturma, oturma.red, oturma.green, oturma.blue);
    updateRoomLEDs(&salon, salon.red, salon.green, salon.blue);
    mutfak.active = true;
    oturma.active = true;
    salon.active = true;
    SerialBT.println("Tüm ışıklar açıldı");
  } else {
    SerialBT.println("Açılacak mekan belirtilmedi");
  }
}

// Parlaklık ayarı
void adjustBrightness(String mekan, int amount) {
  if (mekan == "mutfak" || mekan == "hepsi") {
    mutfak.brightness = constrain(mutfak.brightness + amount, 0, 255);
    updateRoomLEDs(&mutfak, mutfak.red, mutfak.green, mutfak.blue);
    SerialBT.print("Mutfak parlaklığı: %");
    SerialBT.println(map(mutfak.brightness, 0, 255, 0, 100));
  }

  if (mekan == "oturma" || mekan == "hepsi") {
    oturma.brightness = constrain(oturma.brightness + amount, 0, 255);
    updateRoomLEDs(&oturma, oturma.red, oturma.green, oturma.blue);
    SerialBT.print("Oturma odası parlaklığı: %");
    SerialBT.println(map(oturma.brightness, 0, 255, 0, 100));
  }

  if (mekan == "salon" || mekan == "hepsi") {
    salon.brightness = constrain(salon.brightness + amount, 0, 255);
    updateRoomLEDs(&salon, salon.red, salon.green, salon.blue);
    SerialBT.print("Salon parlaklığı: %");
    SerialBT.println(map(salon.brightness, 0, 255, 0, 100));
  }
}

// Mekanın LED'lerini günceller
void updateRoomLEDs(Room* room, int r, int g, int b) {
  // RGB değerlerini kaydet
  room->red = r;
  room->green = g;
  room->blue = b;

  // Parlaklık faktörünü uygula
  int adjustedR = map(r, 0, 255, 0, room->brightness);
  int adjustedG = map(g, 0, 255, 0, room->brightness);
  int adjustedB = map(b, 0, 255, 0, room->brightness);

  // PWM ile LED'leri güncelle
  ledcWrite(room->redChannel, adjustedR);
  ledcWrite(room->greenChannel, adjustedG);
  ledcWrite(room->blueChannel, adjustedB);
}

// Yardım bilgilerini göster
void showHelp() {
  SerialBT.println("\n--- ESP32 Ev Kontrol Komutları ---");
  SerialBT.println("1. 'dinle' - Ses dinleme modunu aktifleştirir");
  SerialBT.println("2. Mekan komutları: 'mutfak', 'oturma odası', 'salon', 'hepsi'");
  SerialBT.println("3. Renk komutları: 'kırmızı', 'yeşil', 'mavi', 'sarı', 'mor', 'turkuaz', 'beyaz'");
  SerialBT.println("4. Örnek kullanım: 'mutfak kırmızı' - Mutfak ışıklarını kırmızı yapar");
  SerialBT.println("5. Açma/Kapama: 'mutfak aç', 'salon kapat', 'hepsi kapat'");
  SerialBT.println("6. Parlaklık: 'mutfak parlaklık azalt', 'salon parlaklık artır'");
  SerialBT.println("7. LDR: 'ışık ölç' - Ortam ışığı değerini ölçer ve bildirir");
  SerialBT.println("8. 'yardım' - Bu yardım mesajını gösterir");
}

// Dinleme moduna geçtiğini belirtmek için LED'leri yakıp söndür
void blinkAllLEDs() {
  // Mevcut durumları kaydet
  int savedMutfakR = mutfak.red;
  int savedMutfakG = mutfak.green;
  int savedMutfakB = mutfak.blue;

  int savedOturmaR = oturma.red;
  int savedOturmaG = oturma.green;
  int savedOturmaB = oturma.blue;

  int savedSalonR = salon.red;
  int savedSalonG = salon.green;
  int savedSalonB = salon.blue;

  // Yanıp sönme efekti
  for(int i=0; i<3; i++) {
    // Hepsini beyaz yap
    updateRoomLEDs(&mutfak, 255, 255, 255);
    updateRoomLEDs(&oturma, 255, 255, 255);
    updateRoomLEDs(&salon, 255, 255, 255);
    delay(150);

    // Hepsini kapat
    updateRoomLEDs(&mutfak, 0, 0, 0);
    updateRoomLEDs(&oturma, 0, 0, 0);
    updateRoomLEDs(&salon, 0, 0, 0);
    delay(150);
  }

  // Eski duruma geri dön
  updateRoomLEDs(&mutfak, savedMutfakR, savedMutfakG, savedMutfakB);
  updateRoomLEDs(&oturma, savedOturmaR, savedOturmaG, savedOturmaB);
  updateRoomLEDs(&salon, savedSalonR, savedSalonG, savedSalonB);
}
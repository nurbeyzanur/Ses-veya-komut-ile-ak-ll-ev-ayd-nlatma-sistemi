# 🏠 Sesli veya Sözlü Komut ile Led Kontrolü

ESP32 tabanlı Bluetooth üzerinden sesli ve yazılı komutlarla RGB LED kontrolü sağlayan akıllı ev aydınlatma sistemi.

## 📋 İçindekiler
- [Özellikler](#-özellikler)
- [Gereksinimler](#-gereksinimler)
- [Donanım Bağlantıları](#-donanım-bağlantıları)
- [Kullanım](#-kullanım)
- [Komut Listesi](#-komut-listesi)
- [Proje Görselleri](#-proje-görselleri)

## ✨ Özellikler

- 🎙️ **Sesli Komut Desteği**: Bluetooth üzerinden sesli komutlarla kontrol
- 🎨 **RGB LED Kontrolü**: 7 farklı renk seçeneği (Kırmızı, Yeşil, Mavi, Sarı, Mor, Turkuaz, Beyaz)
- 🏘️ **Çoklu Mekan Yönetimi**: 3 farklı mekan için bağımsız kontrol (Mutfak, Oturma Odası, Salon)
- 💡 **Parlaklık Ayarı**: Her mekan için ayrı ayrı parlaklık kontrolü
- 🌞 **Ortam Işığı Sensörü**: LDR ile ortam ışık seviyesi ölçümü
- 📱 **Bluetooth Kontrol**: Android/iOS uygulamaları üzerinden kontrol

## 🔧 Gereksinimler

### Donanım
- ESP32 Development Board
- 3x RGB LED (Ortak Katot)
- LDR (Işık Bağımlı Direnç)
- 10kΩ Direnç (LDR için pull-down)
- 3x 220Ω Direnç (Her LED pini için)
- Breadboard ve Jumper kablolar

### Yazılım
- [Arduino IDE](https://www.arduino.cc/en/software) (1.8.x veya üzeri)
- ESP32 Board Desteği
- BluetoothSerial Kütüphanesi (ESP32 ile birlikte gelir)

## 🔌 Donanım Bağlantıları

### Mutfak (RGB LED 1)
| ESP32 Pin | Bileşen | Renk |
|-----------|---------|------|
| GPIO 5 | Red | Kırmızı |
| GPIO 4 | Green | Yeşil |
| GPIO 2 | Blue | Mavi |
| GND | Common Cathode | - |

### Oturma Odası (RGB LED 2)
| ESP32 Pin | Bileşen | Renk |
|-----------|---------|------|
| GPIO 14 | Red | Kırmızı |
| GPIO 12 | Green | Yeşil |
| GPIO 13 | Blue | Mavi |
| GND | Common Cathode | - |

### Salon (RGB LED 3)
| ESP32 Pin | Bileşen | Renk |
|-----------|---------|------|
| GPIO 18 | Red | Kırmızı |
| GPIO 19 | Green | Yeşil |
| GPIO 21 | Blue | Mavi |
| GND | Common Cathode | - |

### LDR (Işık Sensörü)
| ESP32 Pin | Bileşen |
|-----------|---------|
| GPIO 34 | LDR (bir uç) |
| GND | LDR (diğer uç - 10kΩ direnç ile) |
| 3.3V | 10kΩ direnç |

## 📱 Kullanım

### Bluetooth Bağlantısı

1. ESP32'yi bilgisayarınıza veya güç kaynağına bağlayın
2. Telefon/tablet'inizde Bluetooth ayarlarını açın
3. "ESP32_Ev_Kontrolu" cihazını bulun ve bağlanın
4. Bluetooth terminal uygulaması kullanarak komut gönderin

**Önerilen Uygulamalar:**
- Android: [Serial Bluetooth Terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal)
- iOS: [BLE Terminal](https://apps.apple.com/app/ble-terminal/id1398703795)

### Sesli Komut Kullanımı

1. "dinle" yazın veya söyleyin
2. LED'ler yanıp sönerek dinleme moduna geçildiğini gösterir
3. 5 saniye içinde komutunuzu verin
4. Örnek: "mutfak kırmızı"

## 🎮 Komut Listesi

### Temel Komutlar

| Komut | Açıklama |
|-------|----------|
| `dinle` | Sesli komut modunu aktifleştirir (5 saniye) |
| `yardım` | Komut listesini gösterir |
| `ışık ölç` | LDR ile ortam ışık seviyesini ölçer |

### Mekan Komutları

**Mekanlara Renk Atama:**
```
mutfak kırmızı
oturma odası yeşil
salon mavi
hepsi beyaz
```

**Açma/Kapama:**
```
mutfak aç
salon kapat
hepsi kapat
```

### Renk Seçenekleri

- 🔴 Kırmızı / Red
- 🟢 Yeşil / Green
- 🔵 Mavi / Blue
- 🟡 Sarı / Yellow
- 🟣 Mor / Purple
- 🩵 Turkuaz / Cyan
- ⚪ Beyaz / White

### Parlaklık Kontrolü

```
mutfak parlaklık azalt
salon parlaklık artır
parlaklık yükselt      # Tüm mekanlara uygulanır
```

### Örnek Kullanım Senaryoları

1. **Tüm ışıkları beyaz yap:**
   ```
   hepsi beyaz
   ```

2. **Mutfağı kırmızı yap, parlaklığı azalt:**
   ```
   mutfak kırmızı
   mutfak parlaklık azalt
   ```

3. **Ortam ışığını ölç:**
   ```
   ışık ölç
   ```

4. **Sesli komutla salon kontrolü:**
   ```
   dinle
   salon mavi
   ```

## 🖼️ Proje Görselleri


## 🔍 Teknik Detaylar

### PWM Konfigürasyonu
- **Frekans**: 5000 Hz
- **Çözünürlük**: 8-bit (0-255)
- **Kanal Sayısı**: 9 (her LED için 3 kanal)

### LDR Okuma
- **Pin**: GPIO 34 (Analog)
- **Aralık**: 0-4095 (12-bit ADC)
- **Okuma Periyodu**: 5 saniye

### Zaman Aşımları
- **Sesli Komut Modu**: 5 saniye

## 🐛 Sorun Giderme

**Bluetooth bağlantısı kurulamıyor:**
- ESP32'nin güç aldığından emin olun
- Seri monitörde "Bluetooth ses kontrolü aktif" mesajını kontrol edin
- Diğer cihazlardan bağlantıyı kesin

**LED'ler yanmıyor:**
- Bağlantıları kontrol edin
- Dirençlerin doğru değerde olduğundan emin olun
- Ortak katot (GND) bağlantısını kontrol edin

**Komutlar çalışmıyor:**
- Komutları küçük harfle yazın
- Yardım komutuyla mevcut komutları görüntüleyin
- Seri monitörde gelen komutları kontrol edin


## 👤 Geliştirici

**Beyza Nur Damar**
- GitHub: [@beyzanurdamar](https://github.com/nurbeyzanur)
- LinkedIn: [Beyza Nur Damar](https://www.linkedin.com/in/Beyzanur Damar)


⭐ Bu projeyi beğendiyseniz yıldız vermeyi unutmayın!

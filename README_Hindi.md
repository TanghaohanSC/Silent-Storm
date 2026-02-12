[English](README.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm Trailer](Silent_Storm.png)](https://www.youtube.com/watch?v=ugRAaC7K_1I)

कंप्यूटर गेम [Silent Storm](https://en.wikipedia.org/wiki/Silent_Storm) एक टर्न-बेस्ड टैक्टिकल RPG है, जिसे [Nival Interactive](http://nival.com/) ने विकसित किया और 2003 में जारी किया।

यह गेम अभी भी [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) और [GOG.com](https://www.gog.com/game/silent_storm_gold) पर उपलब्ध है।

2026 में, गेम का सोर्स कोड एक [special license](LICENSE.md) के तहत प्रकाशित किया गया, जो व्यावसायिक उपयोग को प्रतिबंधित करता है, लेकिन गेम की कम्युनिटी, शिक्षा और शोध के लिए पूरी तरह खुला है। इसका उपयोग करने से पहले कृपया [license agreement](LICENSE.md) के नियमों को ध्यान से पढ़ें।

## टेक स्टैक

- **इंजन**: मालिकाना 3D इंजन, मुख्य रूप से C++ में लिखा गया
- **स्क्रिप्टिंग भाषा**: Lua 4.0
- **एनीमेशन**: कस्टम एनीमेशन सिस्टम
- **वीडियो**: Bink Video Technology ⚠️ *व्यावसायिक लाइसेंस — शामिल नहीं है*
- **साउंड**: FMOD साउंड सिस्टम ⚠️ *व्यावसायिक लाइसेंस — शामिल नहीं है*
- **ग्राफिक्स**: DirectX 8

## इस रिपॉज़िटरी में क्या है

- `Soft/` — सोर्स कोड और डेवलपमेंट टूल्स
- `Complete/` — गेम डेटा और संसाधन
- `Data/` — गेम कॉन्फ़िगरेशन डेटा
- `Tools/` — डेवलपमेंट और बिल्ड टूल्स
- `bin/` — कम्पाइल किए गए एक्सीक्यूटेबल्स
- `cfg/` — कॉन्फ़िगरेशन फ़ाइलें
- `Versions/Current` — डेवलपर संस्करण

---

# गेम चलाना

## बेसिक लॉन्च

1. `bin/` डायरेक्टरी पर जाएँ।
2. गेम का एक्सीक्यूटेबल `Game.exe` चलाएँ।

डिफ़ॉल्ट रूप से गेम 800x600 रेज़ोल्यूशन के साथ विंडो मोड में चलता है। विंडो मोड के लिए Display Properties/Settings/Colors में कलर डेप्थ 32-बिट (true color) पर सेट होनी चाहिए। गेम को फुलस्क्रीन मोड में चलाने के लिए `-fullscreen` पैरामीटर का उपयोग करें। रेज़ोल्यूशन बदलने के लिए 640x480, 1024x768 या 1280x1024 के लिए क्रमशः `-640`, `-1024` या `-1280` पैरामीटर का उपयोग करें।

⚠️ आधुनिक ऑपरेटिंग सिस्टम पर गेम चलाने में समस्याएँ हैं। यदि आप कोई समाधान ढूँढ़ते हैं, तो कृपया GitHub Issues के माध्यम से हमें बताएँ।

---

# मैप एडिटर और डेवलपमेंट टूल्स

## मैप एडिटर

- **स्थान**: `bin/MapEdit.exe`

---

# प्रोजेक्ट बिल्ड करना

## बिल्ड के लिए आवश्यकताएँ

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

### ⚠️ अतिरिक्त टूल्स, जो सोर्स कोड में शामिल नहीं हैं:

- **FMOD Audio System**
- **Bink Video Technology**

### 📋 थर्ड-पार्टी कॉम्पोनेंट लाइसेंस:

- **STLport** — BSD टाइप लाइसेंस
- **zlib** — zlib लाइसेंस
- **Lua 4.0** — MIT टाइप लाइसेंस
- **Ogg Vorbis** — BSD टाइप लाइसेंस

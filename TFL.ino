#define CRC12_POLY 0x180D      // x^12 + x^11 + x^3 + x^2 + x + 1
#define CRC16_POLY 0x8005      // x^16 + x^15 + x^2 + 1
#define CRC_CCITT_POLY 0x1021  // x^16 + x^12 + x^5 + 1
#define CRC3_POLY 0x07         // x^3 + 1
#define CRC4_POLY 0x13         // x^4 + x + 1
#define CRC5_POLY 0x25         // x^5 + x^2 + 1
#define CRC6_POLY 0x43         // x^6 + x + 1

#define LED_PIN 11    // Definir el pin del LED
#define PHOTO_PIN A5  // Pin donde se conecta el fototransistor

int option;

String receivedData = "";
unsigned int selectPolynomial(int option);
String charToBinary(char ch);
unsigned int binaryToHex(const String& binary);
int polyDegree(unsigned int poly);
int getCharsPerFrame(int degree);
String calculateCRC(const String& frame, unsigned int poly);
void showBinaryOnLED(const String& binaryStr);

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("Ingrese una palabra:");
  while (Serial.available() == 0);
  String word = Serial.readString();
  
  // Solicitar método CRC
  Serial.println("1. CRC-12");
  Serial.println("2. CRC-16");
  Serial.println("3. CRC-CCITT");
  Serial.println("4. CRC-3");
  Serial.println("5. CRC-4");
  Serial.println("6. CRC-5");
  Serial.println("7. CRC-6");

  while (Serial.available() == 0);
  option = Serial.parseInt();


  // Calcular y mostrar CRC por tramas basadas en el grado del polinomio
  unsigned int poly = selectPolynomial(option);
  int degree = polyDegree(poly);
  int charsPerFrame = getCharsPerFrame(degree);  // Calcular caracteres por trama basado en el grado



  String finalCRC = "";

  for (size_t i = 0; i < word.length(); i += charsPerFrame) {
    String extendedFrame = "";

    for (size_t j = 0; j < charsPerFrame && (i + j) < word.length(); j++) {
      extendedFrame += word[i + j];
    }

    // Si la trama no es completamente llena, ajustamos la longitud
    if (extendedFrame.length() < charsPerFrame) {
      // Si es la última trama, completa con caracteres de relleno (como un espacio o algo similar si es necesario)
      while (extendedFrame.length() < charsPerFrame) {
        extendedFrame += " ";  // Usamos espacio como relleno
      }
    }

    // Construir la representación binaria de la trama
    String binaryExtendedFrame = "";
    for (size_t j = 0; j < extendedFrame.length(); j++) {
      binaryExtendedFrame += charToBinary(extendedFrame[j]);
    }

    // Agregar ceros según el grado del polinomio
    for (int k = 0; k < degree; k++) {
      binaryExtendedFrame += "0";  // Agregar ceros al final
    }

    // Calcular CRC
    String crc = calculateCRC(binaryExtendedFrame, poly);
    String finalCRC = binaryExtendedFrame.substring(0, binaryExtendedFrame.length() - degree) + crc;

    Serial.print("Trama: ");
    Serial.print(extendedFrame);
    Serial.print(" (Extendido: ");
    Serial.print(binaryExtendedFrame);
    Serial.print(") Residuo: ");
    Serial.println(crc);
    Serial.print("Trama resultante: ");
    Serial.println(finalCRC);
    showBinaryOnLED(finalCRC);

}
  processReceivedData(receivedData, poly, degree, charsPerFrame);

  // Mostrar el CRC resultante completo a través del LED
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  // El código no requiere un loop en este caso
}

unsigned int selectPolynomial(int option) {
  switch (option) {
    case 1: return CRC12_POLY;
    case 2: return CRC16_POLY;
    case 3: return CRC_CCITT_POLY;
    case 4: return CRC3_POLY;
    case 5: return CRC4_POLY;
    case 6: return CRC5_POLY;
    case 7: return CRC6_POLY;
    default: return 0;
  }
}

String charToBinary(char ch) {
  String binary = "";
  for (int i = 7; i >= 0; i--) {
    binary += (ch & (1 << i)) ? '1' : '0';
  }
  return binary;
}

unsigned int binaryToHex(const String& binary) {
  unsigned int hexValue = 0;
  for (size_t i = 0; i < binary.length(); i++) {
    hexValue = (hexValue << 1) | (binary[i] == '1' ? 1 : 0);
  }
  return hexValue;
}

int polyDegree(unsigned int poly) {
  int degree = 0;
  while (poly) {
    poly >>= 1;  // Desplazar a la derecha
    degree++;    // Aumentar el contador
  }
  return degree - 1;  // Restar 1 para obtener el grado
}

int getCharsPerFrame(int degree) {
  // Divide según el tamaño del polinomio
  if (degree <= 8) return 1;
  else return (degree + 7) / 8; // Redondeo para grados mayores
}

String calculateCRC(const String& frame, unsigned int poly) {
  unsigned int remainder = 0;
  for (size_t i = 0; i < frame.length(); i++) {
    remainder <<= 1;
    remainder |= (frame[i] == '1' ? 1 : 0);
    if (remainder & (1 << polyDegree(poly))) remainder ^= poly;
  }
  String crc = "";
  for (int i = polyDegree(poly) - 1; i >= 0; i--) {
    crc += ((remainder >> i) & 1) ? "1" : "0";
  }
  return crc;
}

void showBinaryOnLED(const String& binaryStr) {
  // Mostrar cada bit del resultado en el LED
  for (size_t i = 0; i < binaryStr.length(); i++) {
    // Encender o apagar el LED según el bit
    digitalWrite(LED_PIN, (binaryStr[i] == '1') ? HIGH : LOW);
    delay(10);  // Esperar un tiempo antes de mostrar el siguiente bit
    // Leer el valor del fototransistor durante el parpadeo del LED
    delay(5);                                 
    int sensorValue = analogRead(PHOTO_PIN);  // Leer el valor del fototransistor
    if (sensorValue < 1021) {
      receivedData += "1";  // Si el valor es menor que 1021, es un 1
    } else {
      receivedData += "0";  // Si es mayor, es un 0
    }
  }
}

void processReceivedData(String receivedData, unsigned int poly, int degree, int charsPerFrame) {
  String binaryFrame = receivedData;

  String finalMessage = "";

  // Dividir la trama en fragmentos de acuerdo con los caracteres por trama
  for (size_t i = 0; i < binaryFrame.length(); i += charsPerFrame * 8 + degree) {


    String dataFrame = "";
    String crcFrame = "";

    // Extraer los datos (hasta charsPerFrame caracteres) y el CRC (degree bits)
    for (size_t j = 0; j < charsPerFrame * 8 && (i + j) < binaryFrame.length(); j++) {
      dataFrame += binaryFrame[i + j];
    }

    for (size_t j = 0; j < degree && (i + charsPerFrame * 8 + j) < binaryFrame.length(); j++) {
      crcFrame += binaryFrame[i + charsPerFrame * 8 + j];
    }

    // Realizar la operación XOR con el polinomio
    String extendedFrame = dataFrame + crcFrame;
    String crcCalculated = calculateCRC(extendedFrame, poly);
    String zeros = "";
    for (size_t i = 0; i < crcFrame.length(); i++) {
      zeros += "0";  // Agregar un '0' por cada iteración
    }

    crcCalculated.trim();
    zeros.trim();
    // Comparar el CRC calculado con el CRC recibido
    if (crcCalculated == zeros) {

      // El CRC es correcto, mostrar el carácter original
      String originalChar = "";
      for (size_t k = 0; k < dataFrame.length(); k += 8) {
        String charBinary = dataFrame.substring(k, k + 8);
        originalChar += (char)binaryToHex(charBinary);
      }
      finalMessage += originalChar;
    } else {
      // El CRC no es válido, mostrar un "?"
      finalMessage += "?";
    }
  }

  // Mostrar el mensaje final
  Serial.print("Mensaje recibido: ");
  Serial.println(finalMessage);
}

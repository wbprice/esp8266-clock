
#include <ESP8266WiFi.h>
#include <time.h>

const char* ssid = "...........";
const char* password = "...........";

int timezone = -4; // GMT-4
int dst = 0;

#define SHIFT_DATA 2
#define SHIFT_CLK 4
#define SHIFT_LATCH 5

void setup() {
  Serial.begin(115200);

  // Get initial time from WIFI
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  configTime(timezone * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");

  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  Serial.setDebugOutput(true);
}

int getHexForPosition(int pos) {
  /*
   * Only 4 bits are used on the second shift register, meaning the first half of each byte
   * is going to be 1111xxxx;
   * 
   * @param pos - the position from the left to display a number
   * @return a hex value describing which position to put a number at
   */
  switch (pos) {
    case 0: 
      return 0xF1; // 11110001;
    case 1:
      return 0xF2; // 11110010;
    case 2:
      return 0xF4; // 11110100;
    default:
      return 0xF8; // 11111000;
  }
}

 /*
    Seven Segment Truth Table (common anode)
    +--------+---+---+---+---+---+---+---+
    | number | a | b | c | d | e | f | g |
    +--------+---+---+---+---+---+---+---+
    | 0      | 0 | 0 | 0 | 0 | 0 | 0 | 1 |
    | 1      | 1 | 0 | 0 | 1 | 1 | 1 | 1 |
    | 2      | 0 | 0 | 1 | 0 | 0 | 1 | 0 |
    | 3      | 0 | 0 | 0 | 0 | 1 | 1 | 0 |
    | 4      | 1 | 0 | 0 | 1 | 1 | 0 | 0 |
    | 5      | 0 | 1 | 0 | 0 | 1 | 0 | 0 |
    | 6      | 0 | 1 | 0 | 0 | 0 | 0 | 0 |
    | 7      | 0 | 0 | 0 | 1 | 1 | 1 | 1 |
    | 8      | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
    | 9      | 0 | 0 | 0 | 1 | 1 | 0 | 0 |
    +--------+---+---+---+---+---+---+---+
 */

int getHexForNumber(int number) {
  /*
   * Gets a number and returns a hex that tells
   * the 7 segement display what segments need
   * to be high or low.
   * 
   * See Common Anode Truth Table.
   * The table should be read from right to left (g -> a).  
   * The first digit represents the decimal pin (alwasy off) so,
   * 0 => 11000000 => 0xCO
   * 
   * @param number - the number to display
   * @return a hex value describing which pins to bring high or low
   */
  
 
  switch (number) {
    case 0:
      return 0xC0; // 11000000
    case 1:
      return 0xF9; // 11111001
    case 2:
      return 0xA4; // 10100100
    case 3:
      return 0xB0; // 10110000
    case 4:
      return 0x99; // 10011001
    case 5:
      return 0x92; // 10010010
    case 6:
      return 0x82; // 10000010
    case 7:
      return 0xF8; // 11111000
    case 8:
      return 0x00; // 00000000
    default:
      return 0x98; // 10011000
  }
}

void displayNumber(int pos, int number) {
  /*
   * Takes position and number arguments and displays 
   * the given number at the given position on the 4-digit 
   * 7-segment display.
   * 
   * @param pos - position to display number at
   * @param number - what number to display
   */
  
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, getHexForPosition(pos));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, getHexForNumber(number));

  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

void loop() {
  time_t now = time(nullptr);
  struct tm *tmp = gmtime(&now);

  int hours = tmp->tm_hour;
  int mins = tmp->tm_min;

  int thousands = hours / 10;
  int hundred = hours - thousands * 10;
  int tens = mins / 10;
  int ones = mins - tens * 10;

  // Display the current time
  displayNumber(0, thousands);
  displayNumber(1, hundred);
  displayNumber(2, tens);
  displayNumber(3, ones);
}
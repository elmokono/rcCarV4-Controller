#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

/*
---------------------------
| MISO | SCK  | CE  | GND |
| #    | MOSI | CNS | VCC |
|                         |
|        (ANTENNA)        |
---------------------------
MISO = SPI RX - receiver (not used)
MOSI = SPI TX - transmitter 
SCK = SPI Clock
CE = Custom PIN on RP2040
CNS = Custom PIN on RP2040
*/

#define GAS_PIN 26
#define STEER_PIN 27
#define LEDS_PIN 1
#define CE_PIN 22
#define CS_PIN 21
//pico 19 mosi
//pico 18 clock
//pico 16 miso (not used)

RF24 radio(CE_PIN, CS_PIN); // CE, CSN
const byte address[6] = "00010";

int minGas = 1023;
int maxGas = 0;
int minSteer = 1023;
int maxSteer = 0;

void setup()
{
  delay(2000);
  Serial.begin(9600);
  if (!radio.begin())
  {
    while (true)
    {
      Serial.println("failed to initialize radio");
      delay(1000);
    }
  }

  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(124);
  radio.openWritingPipe(address);
  radio.setAutoAck(true);
  radio.stopListening();
  Serial.println("radio init ok");

  //pinMode(STEER_PIN, INPUT_PULLUP);
  //pinMode(GAS_PIN, INPUT_PULLUP);
  pinMode(LEDS_PIN, INPUT_PULLUP);  

  Serial.println("controller init ok");
}

void loop()
{
  int gas = analogRead(GAS_PIN); // 0-1023
  int steer = analogRead(STEER_PIN); // 0-1023

  //Serial.print(gas);Serial.print(" ");Serial.println(steer);

  if (gas < minGas && gas >= 0)
    minGas = gas;
  if (gas > maxGas && gas <= 1023)
    maxGas = gas;
  
  if (steer < minSteer && steer >= 0)
    minSteer = steer;
  if (steer > maxSteer && steer <= 1023)
    maxSteer = steer;

  PinStatus leds = digitalRead(LEDS_PIN);

  uint8_t buffer[4];
  buffer[0] = 0x0;
  buffer[1] = static_cast<uint8_t>(map(gas, minGas, maxGas, 255, 0)); //long to uint8_t
  buffer[2] = static_cast<uint8_t>(map(steer, minSteer, maxSteer, 0, 255)); //long to uint8_t
  buffer[3] = leds == LOW ? 0 : 255;

  Serial.print("command sent ");
  Serial.print(buffer[0]);Serial.print(" ");
  Serial.print(buffer[1]);Serial.print(" ");
  Serial.print(buffer[2]);Serial.print(" ");
  Serial.print(buffer[3]);Serial.println();

  radio.write(&buffer, sizeof(buffer));
  delay(25);
}
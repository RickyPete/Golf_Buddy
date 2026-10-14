#include <SPI.h>
#include "epd4in2_V2.h"
#include "imagedata.h"
#include "epdpaint.h"
#include "Skull.h"

#define COLORED     0
#define UNCOLORED   1

void setup() {
  Serial.begin(9600);
  Epd epd;

  if (epd.Init() != 0) {
    Serial.print("e-Paper init failed");
    return;
  }

  epd.Clear();

  Serial.print("Displaying skull image\r\n");
  epd.Display(gImage_Skull);

  epd.Sleep();
}

void loop() {
}

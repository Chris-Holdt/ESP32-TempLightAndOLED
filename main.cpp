/*
* @Author: Chris Holdt
* @Date:   2017-12-31 16:27:37
* @Last Modified by:   Christopher
* @Last Modified time: 2017-12-31 17:31:02
*/

#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

// Pins
#define THERMISTORPIN 25
#define PHOTORESISTORPIN 27

// Series resistor value
// NOTE same value used for thermistor and photo resistor
#define SERIESRESISTOR 10000

// Number of samples to average
#define SAMPLERATE 5
// Nominal resistance at 25C
#define THERMISTORNOMINAL 10000
// Nominal temperature in degrees
#define TEMPERATURENOMINAL 25
// Beta coefficient
#define BCOEFFICIENT 3380

int refreshCount;

void setup() {
	refreshCount = 0;

  Serial.begin(9600);
  Serial.println("");
  Serial.println("Intialising OLED");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20,30);
  display.print("OLED Initialised");
  display.display();
  delay(1000);
  display.clearDisplay();
}

/**
 * Read the thermistor and convert its resistance level to kelvin then celsius.
 * As this is only accurate to 1 degree celsius we just return an int
 * @return {int} Temperature in Celsius
 */
int getTemp() {
	double thermalSamples[SAMPLERATE];
	double average, kelvin, resistance, celsius;
	int i;

	// Collect SAMPLERATE (default 5) samples
	for (i=0; i<SAMPLERATE; i++) {
		thermalSamples[i] = analogRead(THERMISTORPIN);
		delay(10);
	}
	
	// Calculate the average value of the samples
	average = 0;
	for (i=0; i<SAMPLERATE; i++) {
		average += thermalSamples[i];
	}
	average /= SAMPLERATE;

	// Convert to resistance
	resistance = (4095 / average) - 1;

	/*
	 * Use Steinhart equation (simplified B parameter equation) to convert resistance to kelvin
	 * B param eq: T = 1/( 1/To + 1/B * ln(R/Ro) )
	 * T  = Temperature in Kelvin
	 * R  = Resistance measured
	 * Ro = Resistance at nominal temperature
	 * B  = Coefficent of the thermistor
	 * To = Nominal temperature in kelvin
	 */
	kelvin = 1.0 / ( 1.0/(TEMPERATURENOMINAL+273.15) + (1/3950) * log(resistance/THERMISTORNOMINAL) );
	// Convert Kelvin to Celsius
	celsius = kelvin - 273.15;

	// Send the value back to be displayed
	return celsius;
}

/**
 * Get the LDR resistance in terms of %
 * 0   = No, or very little, light
 * 100 = Very Bright
 * @return {int} Value between 0 and 100 where 0 is dark
 */
int getLight() {
	int ldrSamples[SAMPLERATE];
	int i, light, average;

	// Get a new sample every 10 milliseconds, get SAMPLERATE samples (default 5)
	for (i=0; i<SAMPLERATE; i++) {
		ldrSamples[i] = analogRead(PHOTORESISTORPIN);
		delay(10);
	}

	// Calculate the average
	average = 0;
	for (i=0; i<SAMPLERATE; i++) {
		average += ldrSamples[i];
	}
	average /= SAMPLERATE;

	// Invert
	average = 4095 - average;

	// Map light to a value between 0-100
	light = map(average, 0, 4095, 0, 100);

	return light;
}

void loop() {
  int temp, photoVal;

  photoVal = getLight();  
	temp = getTemp();

  // Display thermal reading
  display.setCursor(20, 20);
  display.print("Temp: ");
  display.print(temp);
  display.println(" C");

  // Display light reading
  display.setCursor(20, 30);
  display.print("Light: ");
  display.println(photoVal);

  // Display update count
  display.setCursor(20, 40);
  display.print("Refreshes: ");
  display.println(refreshCount);

  // Display photo values
  display.display();

  // Clear the display for new values after 2 seconds
  delay(2000);
  display.clearDisplay();

  // Increment refresh counter
  refreshCount++;
}

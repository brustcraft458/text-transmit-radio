//Libraries:
#include <TEA5767.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);

//Constants:
TEA5767 Radio; // Pinout SLC and SDA - A5 and A4


//Variables:
double old_frequency;
double frequency;
unsigned long current_millis = millis();
int inByte;
int delayStep = 0;
int delayHighStep = 0;
int freqGroup = 0;
int analogSound = 0;

String binaryStr = "";
String textBinaryResult = "";
int binaryValue = -1;
int binaryLast = -1;
int binaryD0Count = 0;
int binaryD1Count = 0;
int binaryDNCount = 0;
int analogLimitD0[2] = {0};
int analogLimitD1[2] = {0};

bool showBinarySerial = false;
bool showBinaryLcd = false;
//bool showTextBinaryLcd = false;

const int lampGreenPin = 5;
const int lampRedPin = 4;
const int freqButtonPin = 2;
const int textbinButtonPin = 3;
const int buzzerPin = 6;

void setup() {
    // Init Pin
    pinMode(lampGreenPin, OUTPUT);
    pinMode(lampRedPin, OUTPUT);
    pinMode(freqButtonPin, INPUT);
    
    // LCd Init
    lcd.init();
    lcd.backlight();
    lcd.setCursor(2,0);
    
    //Init
    Serial.begin(9600);
    Radio.init();
    
    frequency = 107.3;
    old_frequency = frequency;
    Radio.set_frequency(frequency);

    // Init
    Serial.println("ANALOG ");
    Serial.print("D0-min D0-max ");
    Serial.println("D1-min D1-max ");
}

void loop() {
    // Audio Read
    analogSound = analogRead(A2);
    analogSound = map(analogSound, 0, 1023, 20, 20000); // range: 20 Hz to 20 kHz
    
    // Show Analog Signal
    Serial.print(analogSound);
    Serial.print(",");

    // Binary
    binaryValue = convertToBinary(analogSound);
    binaryProcces(binaryValue);
    binaryPrint(binaryValue);

    // Step
    delayStep++;
    delayHighStep++;
    
    if (delayStep > 25) {
        radioControl();
        delayStep = 0;
    }

    if (delayHighStep > 150) {
        serialCommandControl();
        delayHighStep = 0;
    }

    // End
    Serial.println(" ");
    delay(5);
}

void binaryPrint(int binValue) {
    int binD0Val = 0;
    int binD1Val = 0;
    switch (binValue) {
    case 0:
        binD0Val = 50;
        digitalWrite(lampGreenPin, LOW);
        digitalWrite(lampRedPin, HIGH);
        break;
    case 1:
        binD1Val = 50;
        digitalWrite(lampRedPin, LOW);
        digitalWrite(lampGreenPin, HIGH);
        break;
    
    default:
        digitalWrite(lampRedPin, LOW);
        digitalWrite(lampGreenPin, LOW);
        break;
    }
    
    if (showBinarySerial) {
        Serial.print(analogLimitD0[0] + binD0Val);
        Serial.print(",");
        Serial.print(analogLimitD0[1] + binD0Val);
        Serial.print(",");
        Serial.print(analogLimitD1[0] + binD1Val);
        Serial.print(",");
        Serial.print(analogLimitD1[1] + binD1Val);
        Serial.print(",");
    }
}

int convertToBinary(int value) {
    if (value >= analogLimitD1[0] && value <= analogLimitD1[1]) {
        binaryDNCount = 0;
        binaryD1Count++;
    } else if (value >= analogLimitD0[0] && value <= analogLimitD0[1]) {
        binaryDNCount = 0;
        binaryD0Count++;
    } else {
        if (binaryDNCount > 6) {
            binaryD0Count = 0;
            binaryD1Count = 0;
        }

        binaryDNCount++;
    }

    // Final binary
    if (binaryD0Count > 8 || binaryD1Count > 8) {
        if (binaryD0Count >= binaryD1Count) {
            return 0;
        } else {
            return 1;
        }
    } else {
        return -1;
    }
}

void binaryProcces(int binValue) {
    if (binaryDNCount > 50) {
        int length = binaryStr.length();
        if (length >= 8) {
            binaryToTextAppend(textBinaryResult, binaryStr);

            // Text
            if (showBinaryLcd) {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Text: ");
                lcd.setCursor(0, 1);
                lcd.print(textBinaryResult);
            }
        }

        binaryStr = "";
        return;
    }
    if (binValue == -1) {
        binaryLast = -1;
        return;
    }
    if (binaryLast == binValue) {
        return;
    }

    binaryLast = binValue;
    binaryStr += String(binValue);

    // Display
    if (showBinaryLcd) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Bin: ");
        lcd.setCursor(0, 1);
        lcd.print(binaryStr);
    }
}

void binaryToTextAppend(String &textResult, String binStr) {
    int charValue = 0;

    // Binary string to integer
    charValue = strtol(binStr.c_str(), NULL, 2);

    // Integer to ASCII character
    textResult += (char)charValue;
}

bool serialCommandControl() {
    if (!Serial.available() > 0) {return false;}

    String command = Serial.readStringUntil('\n');

    // Parse the command
    char cmd[20];
    char select[20];
    int binary;
    int min;
    int max;

    int numberOfValues = sscanf(command.c_str(), "%19s %19s %d %d", &cmd, &select, &min, &max);
    //if (numberOfValues != 3) {return false;}

    if (strcmp(cmd, "showbin") == 0) {
        if (strcmp(select, "serial") == 0) {
            toggleBool(showBinarySerial);
        } else if (strcmp(select, "lcd") == 0) {
            //showTextBinaryLcd = false;
            textBinaryResult = "";
            toggleBool(showBinaryLcd);
        }
    } else if (strcmp(cmd, "setbin") == 0) {
        if (strcmp(select, "d0") == 0) {
            analogLimitD0[0] = min;
            analogLimitD0[1] = max;
        } else if (strcmp(select, "d1") == 0) {
            analogLimitD1[0] = min;
            analogLimitD1[1] = max;
        }
    }

    return true;
}

void radioControl() {
    // Volume Frequency
    float volumValue = analogRead(A7); // Read the analog input

    // Change Freq Group
    if (digitalRead(freqButtonPin) == HIGH) {
        if (freqGroup >= 3) {
            freqGroup = 0;
        } else {
            freqGroup++;
        }
    } else if (digitalRead(textbinButtonPin) == HIGH) {
        lcd.clear();
        toggleBool(showBinaryLcd);
        textBinaryResult = "";
    }

    // Frequency Group
    float freqMin, freqMax;
    switch (freqGroup) {
    case 0:
        freqMin = 76.0;
        freqMax = 83.0;
        break;
    case 1:
        freqMin = 83.1;
        freqMax = 90.0;
        break;
    case 2:
        freqMin = 90.1;
        freqMax = 97.0;
    break;
    case 3:
        freqMin = 97.1;
        freqMax = 108.0;
        break;
    default:
        break;
    }

    // Change Frequency
    frequency = customMap(volumValue, 0, 1022, freqMin, freqMax); // step 8, min 76.0, max 108.0
    if (frequency == old_frequency) {
        return;
    }
    
    // Set Freq
    old_frequency = frequency;
    Radio.set_frequency(frequency);

    // Blink
    digitalWrite(lampGreenPin, HIGH);
    digitalWrite(lampRedPin, HIGH);
    
    // LCD Display
    lcd.clear();
    lcd.setCursor(0, 0); // Set kursor di baris pertama
    lcd.print("Current freq: ");
    lcd.setCursor(0, 1);
    lcd.print(frequency);
    lcd.print("MHz");
        
    // End
    //Serial.println("");
    digitalWrite(lampGreenPin, LOW);
    digitalWrite(lampRedPin, LOW);
}

void toggleBool(bool &boolValue) {
    if (boolValue) {
        boolValue = false;
    } else {
        boolValue = true;
    }
}

float customMap(float inputValue, float inMin, float inMax, float outMin, float outMax) {
    float result = (inputValue - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
    return round(result * pow(10, 1)) / pow(10, 1);
}

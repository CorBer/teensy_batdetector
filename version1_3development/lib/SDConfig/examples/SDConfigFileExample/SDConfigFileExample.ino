#include <SD.h>
#include <SDConfig.h>

/*
 * Example use of the SDConfig library.
 * This sketch reads the configuration file from the SD card,
 * then prints the configuration and prints a hello message
 * at a rate given in the configuration file.
 */

/*
 * Set up:
 * 1) copy the example.cfg file to the SD card.
 * 2) Upload the sketch
 * 3) Open the Serial Monitor at 9600 baud.
 * Observe how the serial monitor shows the settings,
 * the greeting, and the hello message printed
 * with the timing given in the configuration file.
 */

int pinSelectSD = 4; // SD shield Chip Select pin.
char configFile[] = "example.cfg"; // filename

/*
 *   hello = the "hello world" string, allocated using malloc().
 *   doDelay = if true, delay wait in loop().
 *     if false, don't delay.
 *   wait = time (milliseconds) to wait after printing hello.
 */

char *hello = 0;
boolean doDelay = false;
int wait = 0;

boolean readConfiguration();

void setup() { 
  Serial.begin(9600);
  pinMode(pinSelectSD, OUTPUT);

  // Setup the SD card 
  Serial.println("Calling SD.begin()...");
  if (!SD.begin(pinSelectSD)) { 
    Serial.println("SD.begin() failed. Check: ");
    Serial.println("  card insertion,");
    Serial.println("  SD shield I/O pins and chip select,");
    Serial.println("  card formatting.");
    return;
  }
  Serial.println("...succeeded.");
  // Read our configuration from the SD card file.
}

void loop() { 
  /*
   * If we didn't read the configuration, do nothing.
   */
  if (readConfiguration() == false ) { 
    return;
  }
  /*
   * print the hello message,
   * then wait the configured time.
   */
  if (hello) { 
    Serial.println(hello);
    if (doDelay == true) { 
      delay(wait);
    }
  }
}
/*
 * Read our settings from our SD configuration file.
 * Returns true if successful, false if it failed.
 */
boolean readConfiguration() { 
  /*
   * Length of the longest line expected in the config file.
   * The larger this number, the more memory is used
   * to read the file.
   * You probably won't need to change this number.
   */
  int maxLineLength = 127;
   SDConfig cfg;
  // Open the configuration file.
  if (!cfg.begin(configFile, maxLineLength)) { 
    Serial.print("Failed to open configuration file: ");
    Serial.println(configFile);
    return false;
  }
  // Read each setting from the file.
  while (cfg.readNextSetting()) { 
    // Put a nameIs() block here for each setting you have.
    // doDelay
    if (cfg.nameIs("doDelay")) { 
      
      doDelay = cfg.getBooleanValue();
      Serial.print("Read doDelay: ");
      if (doDelay) { 
        Serial.println("true");
      } else { 
        Serial.println("false");
      }
    // wait integer
    } else if (cfg.nameIs("wait")) { 
      wait = cfg.getIntValue();
      Serial.print("Read wait: ");
      Serial.println(wait);
    // hello string (char *)
    } else if (cfg.nameIs("hello")) { 
      // Dynamically allocate a copy of the string.
      hello = cfg.copyValue();
      Serial.print("Read hello: ");
      Serial.println(hello);
    } else { 
      // report unrecognized names.
      Serial.print("Unknown name in config: ");
      Serial.println(cfg.getName());
    }
  }
  // clean up
  cfg.end();
  return true;
}
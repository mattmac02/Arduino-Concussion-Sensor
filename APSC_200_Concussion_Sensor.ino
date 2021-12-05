#include <WiFiNINA.h>

#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

//#include <WiFiNINA.h>
#define fsrpin A0
char ssid[] = "Matt";             //  your network SSID (name) between the " "
char pass[] = "matthew123";      // your network password between the " "
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;      //connection status
WiFiServer server(80);            //server socket
WiFiClient client = server.available();
int fsrPin = 0;     // the FSR and 10K pulldown are connected to a0
int fsrReading;     // the analog reading from the FSR resistor divider
int fsrVoltage;     // the analog reading converted to voltage
unsigned long fsrResistance;  // The voltage converted to resistance
unsigned long fsrConductance;
long fsrForce;       // Finally, the resistance converted to force
long highestForce;
unsigned long time;
void setup() {
  Serial.begin(9600);
  while (!Serial);
  enable_WiFi();
  connect_WiFi();
  server.begin();
  printWifiStatus();
}
void loop() {
  client = server.available();
  if (client) {
    printWEB();
  }

}
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
void enable_WiFi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }
}
void connect_WiFi() {
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
}
void printWEB() {
  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            //create the buttons
            client.print("<HTML>");
            client.print("<HEAD>");
            client.print("<meta http-equiv=\"refresh\" content=\"0.1\">");
            client.print("<TITLE />FSR Readings</title>");
            client.print("</HEAD>");
            client.print("<Body>");
            client.print("</Body>");
            client.print("</HTML>");

            delay(1);

            time = millis();



            fsrReading = analogRead(fsrPin);

            while (fsrReading != 0) {

              // highestForce = 0;
              // analog voltage reading ranges from about 0 to 1023 which maps to 0V to 5V (= 5000mV)
              fsrVoltage = map(fsrReading, 0, 1023, 0, 5000);
              if (fsrVoltage == 0) {
                client.println("No Contact");
              } else {
                // The voltage = Vcc * R / (R + FSR) where R = 10K and Vcc = 5V
                // so FSR = ((Vcc - V) * R) / V
                fsrResistance = 5000 - fsrVoltage;     // fsrVoltage is in millivolts so 5V = 5000mV
                fsrResistance *= 10000;                // 10K resistor
                fsrResistance /= fsrVoltage;

                fsrConductance = 1000000;           // we measure in micromhos so
                fsrConductance /= fsrResistance;

                // Use the two FSR guide graphs to approximate the force
                if (fsrConductance <= 1000) {
                  fsrForce = fsrConductance / 80;
                  client.print("<p style=font-size:20px;background-color:#E6E8E6;margin-left:34%;width:30%;border-radius:0.45rem;padding:1rem;text-align:center>Force Experienced: <br>");
                  client.println(fsrForce);
                  client.print(" N<br></p>");
                } else {
                  fsrForce = fsrConductance - 1000;
                  fsrForce /= 30;
                  client.print("<p style=font-size:20px;background-color:#E6E8E6;margin-left:34%;width:30%;border-radius:0.45rem;padding:1rem;text-align:center>Force Experienced: <br>");
                  client.println(fsrForce);
                  client.print(" N<br></p>");
                }
              }

              if (fsrForce > highestForce) {
                highestForce = fsrForce;
              }
              fsrReading = 0;
            }

            if (highestForce >= 15) {
              client.print("<div style=background-color:#343434;padding:4rem;border-radius:.45rem;margin-left:25%;width:40%>");
              client.print("<p style=font-size:28px;background-color:#E6E8E6;font-weight:bold;border-radius:.45rem;margin-left:30%;padding:2rem;width:30%;text-align:center>Highest Force: ");
              client.println(highestForce);
              client.print(" N<br></p>");
            }
            if (highestForce >= 45) {
              client.print("<p style=font-size:22px;background-color:#ff4747;margin-top:.5rem;font-weight:bold;border-radius:0.45rem;padding-left:1rem;text-align:center;padding-right:1rem;padding-top:2rem;padding-bottom:2rem;>Player enduring high-risk contact!<br>Take off the field for medical assessment.</p>");
              client.print("</div>");
            }

            client.print("<p style=font-size:18px;background-color:#E6E8E6;font-weight:bold;width:30%;margin-left:34%;border-radius:0.45rem;padding:1rem;text-align:center>(Threshold for highest force recording is set to 15 Newtons)</p>");



            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
  }
  // close the connection:
  client.stop();
  Serial.println("client disconnected");
}

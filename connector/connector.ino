#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>

const char *ssid = "MainHomeNet";
const char *password = "piramida321";
const unsigned int udpPort = 5555; //local port to listen for UDP packets

struct Controller
{
    IPAddress ip;
    unsigned int port;
};

WiFiUDP Udp;
WiFiClient client;
Controller controller;

void setup()
{
    Serial.begin(115200);
    delay(10);
    // prepare GPIO2
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, 1); //Switch led off

    //Connect to WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());

    Serial.println("\nStarting connection to server...");
    //if you get a connection, report back via serial:
    Udp.begin(udpPort);

    controller = findController(WiFi.localIP());

    delay(100);
}

void loop()
{

    if (!client.connected())
    {
        Serial.println("Connecting to controller:");
        Serial.print(controller.ip);
        Serial.println(controller.port);

        if (client.connect(controller.ip, controller.port))
        {
            Serial.println("connected");
            client.println("OK");
        }
    }

    if (client.available() > 0)
    {
        String data = client.readString();
        Serial.print("Recieved string: ");
        Serial.println(data);
        client.println("OK");
    }

    delay(3000);
}

//Search controller server
Controller findController(IPAddress curIP)
{
    Serial.println("Searching server...");

    bool isServer = false;
    IPAddress multicastIp = IPAddress(curIP[0], curIP[1], curIP[2], 255);
    Controller cntr;
    while (!isServer)
    {
        //Sending broadcast message
        Serial.println("blink lighthouse");

        //send hello world to server
        //data will be sent to server
        char buffer[50] = "board_1";
        Udp.beginPacket(multicastIp, udpPort);
        Udp.write(buffer);
        Udp.endPacket();
        memset(buffer, 0, 50);
        //processing incoming packet, must be called before reading the buffer
        Udp.parsePacket();
        //receive response from server, it will be HELLO WORLD
        if (Udp.read(buffer, 50) > 0)
        {
            Serial.print("Controller found: ");
            Serial.print(Udp.remoteIP());
            Serial.print(":");
            Serial.println(Udp.remotePort());
            cntr.ip = Udp.remoteIP();
            cntr.port = Udp.remotePort();
            isServer = true;
        }
        //Wait for 1 second
        delay(1000);
    }

    return cntr;
}
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <Servo.h>

//network
const char *ssid = "MainHomeNet";
const char *password = "piramida321";
const unsigned int udpPort = 5555; //local port to listen for UDP packets

//Servo
/**
A-	1	5 or D1
A+	3	0 or D3
B-	2	4 or D2
B+	4	2 or D4
*/
int STEERING_PIN = D5;    //Arduino 9 or GPIO5
int ACCELERATOR_PIN = D6; //Aduino 8 or GPIO4

int STEERING_MID = 1500;
int ACCELERATOR_MID = 1500;
int acceleratorVal = 0;
int steeringVal = 0;

Servo wheelServo;
Servo acceleratorServo;

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

    digitalWrite(LED_BUILTIN, 0); //Switch led off

    wheelServo.attach(STEERING_PIN);
    acceleratorServo.attach(ACCELERATOR_PIN);

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
    connectToClient();
    delay(100);
}

void connectToClient()
{
    if (!client.connected())
    {
        Serial.println("Connecting to controller:");
        Serial.print(controller.ip);
        Serial.print(':');
        Serial.println(controller.port);

        if (client.connect(controller.ip, controller.port))
        {
            Serial.println("connected");
            client.print("Im ready");
        }
    }
}

void loop()
{
    if (client.available() > 0)
    {
        String command = client.readStringUntil(';');
        command.trim();

        Serial.print("Command: ");
        Serial.println(command);
        //Serial.print("Recieved string: ");
        if (command.length() > 0)
        {
            client.print(command);
            processCommand(command);
        }
    }

    if (!client.connected())
    {
        reset();
        Serial.println("Client disconnected");
        Serial.println("connecting...");
        Serial.println(controller.ip);
        Serial.println(controller.port);
        connectToClient();
        delay(1000);
    }
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
            //Parse TCP:TP:port
            String data = buffer;

            String ip = getValue(data, ':', 1);
            String port = getValue(data, ':', 2);

            Serial.print("Controller found: ");
            Serial.print(ip);
            Serial.print(":");
            Serial.println(port);
            Serial.println(buffer);

            IPAddress ip_addr;
            ip_addr.fromString(ip);
            cntr.ip = ip_addr;
            cntr.port = port.toInt();
            isServer = true;
        }
        //Wait for 1 second
        delay(1000);
    }

    return cntr;
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (data.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void processCommand(String command)
{
    //wheelServo.writeMicroseconds(angle);
    if (command == "state")
    {
        printState();
        return;
    }

    if (command == "brake")
    {
        brake();
        return;
    }

    int pos = command.indexOf(':');
    if (pos != 0)
    {
        String subCommand = getValue(command, ':', 0);
        String value = getValue(command, ':', 1);
        processSubCommand(subCommand, value);
    }
}

void processSubCommand(String subCommand, String value)
{
    Serial.print("Sub command: ");
    Serial.print(subCommand);
    Serial.print(" = ");
    Serial.println(value);

    if (subCommand == "AM")
    {
        ACCELERATOR_MID = value.toInt();
    }
    if (subCommand == "SM")
    {
        STEERING_MID = value.toInt();
    }
    if (subCommand == "A")
    {
        setAccelerator(value.toInt());
    }
    if (subCommand == "S")
    {
        setSteering(value.toInt());
    }
}

void brake()
{
    //If car moving forward
    if (acceleratorVal > ACCELERATOR_MID)
    {
        acceleratorVal = ACCELERATOR_MID - 300;
    }
    //If car moving backward
    if (acceleratorVal < ACCELERATOR_MID)
    {
        acceleratorVal = ACCELERATOR_MID + 100;
    }

    acceleratorServo.writeMicroseconds(acceleratorVal);
    delayMicroseconds(500);
    //Reset accelerator val
    acceleratorVal = ACCELERATOR_MID;
    acceleratorServo.writeMicroseconds(acceleratorVal);
}

void reset()
{
    brake();
    setSteering(STEERING_MID);
}

void setAccelerator(int value)
{
    acceleratorVal = value;
    acceleratorServo.writeMicroseconds(acceleratorVal);
}

void setSteering(int value)
{
    steeringVal = value;
    wheelServo.writeMicroseconds(steeringVal);
}

void printState()
{
    Serial.println("Current state:");
    Serial.print("ACCELERATOR_MID: ");
    Serial.println(ACCELERATOR_MID);
    Serial.print("STEERING_MID: ");
    Serial.println(STEERING_MID);
    Serial.print("steering value: ");
    Serial.println(steeringVal);
    Serial.print("accelerator value: ");
    Serial.println(acceleratorVal);

    client.print("ACCELERATOR_MID: ");
    client.print(ACCELERATOR_MID);
    client.print("STEERING_MID: ");
    client.print(STEERING_MID);
    client.print("steering value: ");
    client.print(steeringVal);
    client.print("accelerator value: ");
    client.print(acceleratorVal);
}

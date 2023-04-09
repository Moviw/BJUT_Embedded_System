#define TRIGGER_PIN 2
#define ECHO_PIN 3
#define RED_LED 7
#define GREEN_LED 5
#define BLUE_LED 4

double sum = 0;
int count = 0;
void setup()
{
    Serial.begin(9600);
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);
}

void loop()
{
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);

    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH);
    int distance = duration / 2 / 29.15;
    sum += distance;
    if (count % 3 == 2)
    {
        sum = sum / 3;

        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");
        if (sum >= 10 and sum < 20)
        {
            digitalWrite(RED_LED, HIGH);
            digitalWrite(GREEN_LED, HIGH);
        }
        if (sum < 10)
        {
            digitalWrite(RED_LED, HIGH);
        }
        sum = 0;
    }
    delay(500);
}

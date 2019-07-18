//temp agua
#include <OneWire.h> 
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 11 
OneWire oneWire(ONE_WIRE_BUS);  
DallasTemperature sensors(&oneWire);

//Variaveis do comprimento de onda
int s0=3,s1=4,s2=5,s3=6;
int out=2;
int flag=0;
float frequencyR,frequencyG,frequencyB,tempAgua,tempAr,humidade,gasPPM,gasDig,solubilidadePPM;
byte counter=0;
byte countR=0,countG=0,countB=0;

//Salinidade
#define TdsSensorPin A0
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;

//Gas
#define MQ_analog A1
#define MQ_dig 12
int valor_analog;
int valor_dig;

//Temperatura e Humidade
#include "DHT.h"
#define DHTPIN 8     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);


void setup() {
  // put your setup code here, to run once:
  // start serial port 
 Serial.begin(9600); 
 sensors.begin();

 pinMode(s0,OUTPUT);
 pinMode(s1,OUTPUT);
 pinMode(s2,OUTPUT);
 pinMode(s3,OUTPUT);

 pinMode(TdsSensorPin,INPUT);

 pinMode(MQ_analog, INPUT);
 pinMode(MQ_dig, INPUT);

 dht.begin();
}

void TCS()
 {
 flag=0;
 digitalWrite(s1,HIGH);
 digitalWrite(s0,HIGH);
 digitalWrite(s2,LOW);
 digitalWrite(s3,LOW);
 attachInterrupt(0, ISR_INTO, CHANGE);
 timer0_init();

 }
void ISR_INTO()
 {
 counter++;
 }
 void timer0_init(void)
 {
  TCCR2A=0x00;
  TCCR2B=0x07;   //the clock frequency source 1024 points
  TCNT2= 100;    //10 ms overflow again
  TIMSK2 = 0x01; //allow interrupt
 }
 int i=0;
 ISR(TIMER2_OVF_vect)//the timer 2, 10ms interrupt overflow again. Internal overflow interrupt executive function
{
    TCNT2=100;
    flag++;
 if(flag==1)
  {
    countR=counter;
    frequencyR = map(countR, 590,750,0,255);
  }
  else if(flag==2)
   {
    countG=counter;
    frequencyG = map(countG, 495,585,0,255);
   }
   else if(flag==3)
    {
    countB=counter;
    frequencyB = map(countB, 380,494,0,255);
    }
    else if(flag==4)
     {
     flag=0;
     }
       counter=0;
}

int getMedianNum(int bArray[], int iFilterLen)
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++)
      {
      for (i = 0; i < iFilterLen - j - 1; i++)
          {
        if (bTab[i] > bTab[i++])
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i++];
        bTab[i+1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];}
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
      

      
}
void PRT()
  {
    Serial.print(tempAgua);
    Serial.print(",");
    Serial.print(tempAr);
    Serial.print(",");
    Serial.print(humidade);
    Serial.print(",");
    Serial.print(gasPPM);
    Serial.print(",");
    Serial.print(solubilidadePPM);
    Serial.print(",");
    Serial.println(frequencyR);
  }

void SOL(){
  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT)
         analogBufferIndex = 0;
   }
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
      analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
      tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
      solubilidadePPM=tdsValue;
   }
  }

void loop() {
  TCS();
  SOL();

  gasPPM = analogRead(MQ_analog); 
  gasDig = digitalRead(MQ_dig);
  humidade = dht.readHumidity();
  tempAr = dht.readTemperature();

 //TempAgua
 sensors.requestTemperatures(); // Send the command to get temperature readings 
 tempAgua=sensors.getTempCByIndex(0); // Why "byIndex"? 
 tempAgua=tempAr;
 delay(1000);
 PRT(); 
}

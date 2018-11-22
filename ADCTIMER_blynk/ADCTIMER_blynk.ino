/*


27/08/2018

Programa completo para la recogida de datos desde los sensores y envio al mpu para procesado y lanzar a app blynk

Usando linkit smart 7688 duo, mcu atmega32u4 mpu mediatek 7688

Mejoras:
-Varias pasadas para el calculo del cos fi, luego hacer la media

*/

#define BLYNK_PRINT Serial

#include <TimeLib.h>
#include <Bridge.h>
#include <BlynkSimpleYun.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "48224d6f4f9049fabff6f65b3a266f9e";

// Bridge widget on virtual pin 1
WidgetBridge bridge1(V1);


BLYNK_CONNECTED() {
  bridge1.setAuthToken("ec0bfcf4ac5e4529814fb4ff2d9364a9"); // Place the AuthToken of the second hardware here
}


//8/10/2018  timer 1 dispara cada 170us
int flagdebug=0;
//unsigned long c[80];
//unsigned long v[80];

//int ct=0;

//uint16_t compareRegister = 160;//120us
uint16_t compareRegister = 190;//120us
//uint16_t compareRegister = 60;//120us
//unsigned long t1;



//flag aux para sincronizar el main loop con la interrupcion
volatile boolean flag = false, flag1=false;

//-------------constantes de programa----------------
const float pi=3.14;


//resolucion del adc
const float res=3.3/1023.0;
//frecuencia angular de la onda de red
const float w=2*pi*50;
//conversion de rad a grado 
const float radagrado=360.0/(2*pi);
//factor de reduccion usado para la medida de la tension, trafo mas div resistivo 
//mini trafo const float factor=228.4/0.25;
//trafo carril din
const float factor=225.8/0.93;
const float sct=30.0;
//constantes para tomar como referencia por los offset usados para la medida de corriente y tension 
int cteI=559;
int cteV=310;
const int offset=503;
//usado para el calculo del coseno de fi, usando tecnica de cruces por cero
int detect=0;
float cosfi,senfi;

uint16_t voltage;
unsigned long t1,t2,t3,t4=0;
long dift=0;

//aray para las muestras de corriente, solo para las pruebas experimentales
float current[160],voltagge[160];
float auxx[50];
int auxxx;
//long times[160],times2[160];
int i=0;

//variables de apoyo para la deteccion de cruces por cero
float aux=0.0,auxaI=0.0,auxaV=0.0;
//variable que recoge lectura siguiente programada, canal 0 corriente, canal 1 tension
int echa=0;
int cont=0;
int analogval;

//para ir haciendo operaciones intermedias para el calculo final del rms
float sum1=0,sum2=0.0;
//contadores para llevar cuenta de medidas de tension y corriente
int l=0, j=0;
int aux1=0;

//array para almacenar cruces por cero de subida y bajada de onda de tension y de corriente
unsigned long cpcs[2],cpcb[2],cpcsV[2],cpcbV[2];
//indices asociados 
int s=0,b=0,sV=0,bV=0;

float S,P,Q;
float Irms=0.0,Vrms=0.0;
int cont2=0;




//----------------------------------------------funciones inicializacion hardware atmega32u4
// inicializo ADC trigered compare match timer 1
//cuando el timer 1 venza se dispara la conversion y tengo hasta que vuelva a disparar para realizar la conversion que toque 
void adc_init() {
  //con esta configuracion tiempo de conversion es 16 us
  // Voltage Reference 2.56v
  //vcc 01
  //ref aref 00
  ADMUX |= (0 << REFS1) | (1 << REFS0);
  // ADC Channel A0 (ahora mismo recoge dato del sensor de temperatura interno del chip atmega32u4)
  ADMUX |= (0 << MUX5) | (0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);
  // Prescaler (1/8), 
  ADCSRA=0;
  ADCSRA |= (0 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  // ADC Auto Trigger Source - Timer1B Compare Interrupt
  ADCSRB |= (1 << ADTS2) | (0 << ADTS1) | (1 << ADTS0);
  //ADCSRB |= (0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0);
  // Enable ADC
  ADCSRA |= (1 << ADEN);
  // Enable Auto Trigger
  ADCSRA |= (1 << ADATE);
  //enbale adc interrupt 
  ADCSRA |= (1<< ADIE);
  //sei();
  //ADCSRA |=(1<<ADSC);
}


// incializo Timer1 Compare Interrupt with a given compare count number
void timer1_init(uint16_t compareMatchRegister) {
  // Reset Timer Count
  TCCR1A = 0;
  // Mode = CTC, Prescaler = 1
  TCCR1B = (1 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10);
//TCCR1B = (1 << WGM12) | (0 << CS12) | (1 << CS11) | (1 << CS10);
  // Initialize the counter
  TCNT1 = 0;
  // Timer compare value
  OCR1A = compareMatchRegister;
  OCR1B = compareMatchRegister;
  // Initialize the Timer1_Compare_A interrupt
 // TIMSK1 = (1 << OCIE1A);
  //TIMSK1 = (1 << OCIE1B)|(1<<OCIE1A);
  TIMSK1 = (1 << OCIE1B);
  // Enable global interrupts
  //sei();
  //t1=micros();
  
}

ISR(TIMER1_COMPB_vect) {
//  Serial.print("ok");
//  Serial.println(micros()-t3);
//  t3=micros();

  //v[ct]=micros();
  
}


// rutina de interrupcion asociada a la finalizacion de la conversion que estaba en curso
ISR(ADC_vect){
  
//  v[ct]=micros()-v[ct];
//  if (ct<=78) ct++;
//  else ct=0;
  
 
  //if (i==0) t2=micros();

  /*if (i==196){
    t1=micros();
  }*/
  //159
  if (i<=159){
  // Done reading
  //flag= 1;
  //conv lista adc0
  //adc0 lectura de corriente
  if (echa==0){
  
  ADMUX |= (0 << MUX5) | (0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (1 << MUX0);
  echa=1;
  //t1=micros();
  auxxx=ADC;
  aux=(float)(auxxx-offset)*res*sct;
  //Serial.println(ADC-cteI);
  sum1+=aux*aux;
  current[j]=aux;
  //c[j]=micros();
  j++;

  //detectar los pasos por cero
  //subida
  if((auxaI<0.0 && aux>0.0)){
    cpcs[s]=micros();
    //cpcs[1][s]=i;
    s++;
    
  }

  //bajada
  else if(auxaI>0.0 && aux<0.0){
    cpcb[b]=micros();
    //cpcb[1][b]=i;
    b++;
  }
  
  auxaI=aux;
  
  }

  //adc1 para la lectura de voltaje
  else {
    //ADMUX &= (1 << REFS1) | (1 << REFS0)|(0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);
    ADMUX &= (0 << REFS1) | (1 << REFS0)|(0 << MUX5) | (0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);
    echa=0;
    auxxx=ADC;
    aux=(float)(auxxx-offset)*res*factor;//voltaje asociado
    
    sum2+=aux*aux;
    voltagge[l]=aux;
    
    l++;


  if((auxaV<0.0 && aux>0.0)){
    cpcsV[sV]=micros();
    //cpcs[1][s]=i;
    sV++;
    
  }
  else if(auxaV>0.0 && aux<0.0){
    cpcbV[bV]=micros();
    //cpcb[1][b]=i;
    bV++;
  }
  
  auxaV=aux;
  
    
  }
  
  }
  
  
  //fin de ciclo de lectura desde los sensores
  else {
    //Serial.println(i);
    //Serial.println(micros()-t2);
    //t3=micros();
//    TCCR1A=0;
//    TCCR1B=0;
//    TIMSK1 = (0 << OCIE1B);
    ADCSRA &= (0 << ADEN);
    flag=1;
    /*Serial.println("ok adc:");
    Serial.println(t3-t2);
    Serial.println(j);
    Serial.println(l);
    */
  }

  i++;

  //t3=micros();
  //Serial.println("----:");
  
  /*if (i==101){
  t3=micros();
  }*/
  //flag1=1;
  
  // Not needed because free-running mode is enabled.
  // Set ADSC in ADCSRA (0x7A) to start another ADC conversion
  // ADCSRA |= B01000000;
}






void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, IPAddress(89,39,22,36), 8080);
  //cli();
  // Initialize ADC
  adc_init();
  // Initialize the timer
  timer1_init(compareRegister);
  //timer2_init(compareRegister2);
  //hanilito las interrupciones
  sei();
  Serial.println("arrancamos");
  //t3=micros();
  //t1=micros();
  
}

void loop() {
 Blynk.run();
 
 if (flag){
   Serial.println(micros()-t4);
  
 
   //debug
    for (int i=0;i<j;i++){
      Serial.println("voltaje:");
      Serial.println(current[i]);
      Serial.println("corriente:");
      Serial.println(voltagge[i]);
//      if (i>=1){
//        Serial.println(c[i]-c[i-1]);
//        Serial.print("time:");
//        Serial.println(v[i]);
//      }
      //Serial.println(c[i]);
      //Serial.println(voltagge[i]);
      //Serial.println(j);
      //Serial.println(auxx[i]);
      //Serial.println(times[i]);
      //Serial.println(times2[i]);
      
    }
    for(i=0;i<s;i++){
      //Serial.println(cpcs[i]);
    }
    for(i=0;i<b;i++){
      //Serial.println(cpcb[i]);
    }




    //operaciones finales para envio al mpu
    //calculo para resultado final para las medidas
    Irms=sqrt(sum1/j);
    Serial.print("Irms:");
    Serial.println(Irms);
    Vrms=sqrt(sum2/l);
    Serial.print("Vrms:");
    Serial.println(Vrms);

    
    //primera pasada para detectar el fdp, en cpc,flanco de sibida.
    for (i=0;i<s&&!detect;i++){
      for (j=0;j<sV&&!detect;j++){
        dift=abs(long(cpcs[i])-long(cpcsV[j]));
        //dift=abs(0-512);
        
        //si la diferencia de cruces es menor en tiempo que medio ciclo de red, procedemos al calculo del desfase 
        if (dift<5000){
          cosfi=cos(w*dift*1000000*radagrado);
          senfi=sin(w*dift*1000000*radagrado);
          detect=1;
          Serial.println(dift);
        }
      }
    }

    //primera pasada para detectar el fdp, en cpc, flanco de bajada
    for (i=0;i<b&&!detect;i++){
      for (j=0;j<bV&&!detect;j++){
        dift=abs(long(cpcb[i])-long(cpcbV[j]));
        //si la diferencia de cruces es menor en tiempo que medio ciclo de red, procedemos al calculo del desfase 
        if (dift<5000){
          cosfi=cos(w*dift*1000000*radagrado);
          senfi=sin(w*dift*1000000*radagrado);
          detect=1;
          Serial.println(dift);
        }
      }
    }
    Serial.println("detect:");
    Serial.println(detect);
    Serial.println(cosfi);
    Serial.println(s);
    Serial.println(b);
    Serial.println(bV);
    Serial.println(sV);
    //puedo hacer una segunda pasada esta vez con el flanco de bajada 


    //envio de datos a la raspi via comunicacion serie 
    //tengo que enviar Irms,Vrms,P, cosfi, t podemos cogerlo en la raspi.
    //P,Q,cosfi
    float S,P,senfi;
    typedef union {
    float val;
    uint8_t bytes[4];
    } floatval;

    floatval Pu,Qu,cosfiu;
    S=Irms*Vrms;
    P=S*cosfi;
    Q=S*senfi;

    //envio datos a la rpi para que los procese y los cuelge a la app
    //P, cosfi, Q
    bridge1.virtualWrite(V11, 1023.0,125.5,32.2);
    //time_t t=now();
    //Serial.println(t);
    Pu.val=P;
    Qu.val=Q;
    cosfiu.val=cosfi;

    /*Serial.write(Pu.bytes,4);
    Serial.write(Qu.bytes,4);
    Serial.write(cosfiu.bytes,4);
   */
    

    


    
    //reseteamos variables para un nuevo ciclo de recogida de datos mas envio
    detect=0;
    flag=0;
    sum1=0.0;
    sum2=0.0;
    auxaI=0.0;
    auxaV=0.0;
    i=0;
    j=0;
    l=0;
    s=0;
    sV=0;
    b=0;
    bV=0;
    
    delay(5000);
    //inhabilito las int
    t4=micros();
    cli();
    adc_init();
    // Initialize the timer
    timer1_init(compareRegister);
    sei();
    Serial.println("nuevo ciclo");
  


    
  }
  //30s y de nuevo periodo de lectura 
  //supongo que durante esos 30 s el consumo ha sido el de la lectura que se viene a continuacion
  

  
  
}

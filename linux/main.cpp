/*
falta:
-guardar en memoria variables criticas
-

*/

#include <iostream>
#include <string>
#include <stdio.h>      /* puts, printf */
#include <time.h>       /* time_t, struct tm, time, localtime */

using namespace std;

using std::string;
using std::getline;
/* Comment this out to disable prints and save space */
#define BLYNK_PRINT stdout

#ifdef RASPBERRY
 #include <BlynkApiWiringPi.h>
#else
 #include <BlynkApiLinux.h>
#endif
#include <BlynkSocket.h>
#include <BlynkOptionsParser.h>

static BlynkTransportSocket _blynkTransport;
BlynkSocket Blynk(_blynkTransport);
#include <BlynkWidgets.h>

//variables globales
BlynkParamAllocated items(128); // list length, in bytes


int i=0;
int com,tar,pmax;
int ok[3];
void horper(void);
float precios[2][7];
int flag=0,flag2=0,flagdatos=1;//--------------------------flagdatos a 0
int vi=-1;
float precio[2];
float kwhcont=0,kwh=0;
const int ncom=2;

int contpmax=0;
float kwmedia=0,kw=0;
int c=0;


string nomcom[ncom];

//siempre al conectar recojo datos del servidor, BD
BLYNK_CONNECTED(){
  Blynk.virtualWrite(V0,1);

  
}

BLYNK_APP_CONNECTED(){

}


//v0 asociar a la solicitud de datos del servidor que tiene la base de datos
BLYNK_WRITE(V0)
{ size_t l;
  

  
  
  //printf("\n");
  
  //printf(param.asStr());
  
  //printf("\n");
  

  string data= param.asStr();
  string data2;
  char test[]="hola";
  int pos1,pos2;
  l=data.length();
  if (l>10){
    //primero voy a extraer el numero de comercializadoras que tengo
    pos1=data.find("*");
    data2=data.substr(pos1-1,1);
    int ncom=stoi(data2);
    cout<<ncom<<endl;
    cout<<data2<<endl;
    //substraigo numero comercializadora
    data=data.substr(pos1+1);
    for (int i=0;i<ncom;i++){
    for (int j=0;j<8;j++){
      pos1=data.find(";");
      //en este caso pos1 actua como long
      data2=data.substr(0,pos1);
      //en este caso pos1 actua como indice
      data=data.substr(pos1+1);
      cout<<data2<<endl;
      if (j==0){
        nomcom[i]=data2;
        items.add(data2.c_str());
        string val="0";
        val= val + "€";
        Blynk.virtualWrite(V12, "add", i , data2.c_str() , val.c_str());
      } 
      if (j!=0){
        precios[i][j-1]=stof(data2);
      }

    }
    }
    Blynk.setProperty(V4, "labels", items);
    
       for (int i=0;i<ncom;i++){
      for (int j=0;j<7;j++){
        cout<<precios[i][j]<<endl;
      }
    }

  }
  
}
    
    //por defecto el primer item es el 1 no el cero
    BLYNK_WRITE(V4)
    {
      com=param.asInt()-1;
      ok[0]=1;
      cout<<com<<endl;
      //ok++;

    }

    BLYNK_WRITE(V5)
    {
      tar=param.asInt()-1;
      ok[1]=1;
      //ok++;
      cout<<tar<<endl;
    }


    BLYNK_WRITE(V6)
    {
      pmax=param.asInt();
      ok[2]=1;
      //ok++;
      cout<<pmax<<endl;

    }

    //bridge
    BLYNK_WRITE(V11)
    { //bridge 
      //kwh 
      //cosfi
      float p,q,cosfi;
      time_t t;
      //float pmax=param[0].asFloat();
      p=param[0].asFloat();
      q=param[1].asFloat();
      cosfi=param[2].asFloat();
      t=time(0);
      cout<<p<<endl;
      cout<<q<<endl;
      cout<<cosfi<<endl;
      cout<<t<<endl;
      //pmax=param[1].asFloat();
      //cout<<pmax<<endl;
      flagdatos=1;
      //necesito el valor de kwh  

    }




 
    /*
    pos1 = data.find("*");
    data = data.substr(pos1);
    cout <<data;
    */



    /*pos1 = data.find("*"); 
    //cadena lista con los datos 
    int pos2 = data.find(";");
    string datam = data.substr (pos+1,pos2-pos-1);

    cout<<l;
    printf("entro");
    cout<<datam;*/
  
 

  
  //printf(webhookdata);

  //webhookdata.ignore(256,'*');
  //getline(webhookdata,string data,'*');
  //cout<<webhookdata;
  /*
  for(int i=0;i<webhookdata.lenght();i++){
    if(webhookdata[i]=="*"){
      j=0;
    }

  }*/


/*
BLYNK_WRITE(V1) {
  int value = param.asInt();
  printf("ok");
  if (value == 1) {
    //Serial.println("Item 1 selected");
  } else if (value == 2) {
    // If item 2 is selected, change menu items...
    BlynkParamAllocated items(128); // list length, in bytes
    items.add("Elias");
    items.add("Xele");
    items.add("Sergio");
    Blynk.setProperty(V1, "labels", items);

    // You can also use it like this:
    //Blynk.setProperty(V1, "labels", "item 1", "item 2", "item 3");

  } else {
    //Serial.println("Unknown item selected");
  }
  cout<<value;
}
*/

//esta funcion va llevando el recuento de precios no solo de tu comercializadora y tu tarifa sino otras comercializadoras con tarifa del mismo plan
void horper(){
  float aux;
  time_t rawtime;
  struct tm * timeinfo;

  time (&rawtime);
  timeinfo = localtime (&rawtime);
  //printf ("Current local time and date: %s", asctime(timeinfo));
  int  mes=timeinfo->tm_mon;
  int  hor=timeinfo->tm_hour;
  int  dia=timeinfo->tm_mday;
  mes++;//0-11
  //int  min=timeinfo->tm_min;
  //cout<<mes<<endl<<hor<<":"<<min<<endl;

  //si estamos a dia 1 empieza un nuevo periodo de facturacion 
  if (dia==1 && !flag){
    flag=1;
    //actualizo precio total
    for (int j=0;j<ncom;j++){
      precio[j]=precios[com][0]*30*pmax;
    }

    contpmax=0;
    kwmedia=0;
    c=0;
    
    kwhcont=0;

  }
  if (dia!=1 && flag){
    flag=0;
  }

  //horario
  if (mes>3 && mes<11){
    //verano
    vi=0;
    cout<<"verano"<<endl;
  }
  else{
    vi=1;//invierno
    cout<<"invierno"<<endl;
  } 

  switch(tar){
    case 0:{
      //sin discriminacion horaria
      cout<<"sin discriminacion horaria"<<endl;
      for (int j=0;j<ncom;j++){
        precio[j]=precio[j]+precios[j][1]*kwh;
      }
      //aux=precios[com][1];
    }

    case 1:{
      //DH
      if (vi==0){
        cout<<"verano"<<endl;
        if(hor>=13 && hor<23)//cout<<"punta"<<endl;
          {
            for (int j=0;j<ncom;j++){
              precio[j]=precio[j]+precios[j][2]*kwh;
            }
          }
          //aux=precios[com][2];
        else{
            for (int j=0;j<ncom;j++){
              cout<<precios[j][3]*kwh<<endl;
              
              precio[j]=precio[j]+precios[j][3]*kwh;
            }          
        } //aux=precios[com][3];//cout<<"valle"<<endl;//precio[com][3];
      }
      else{
        cout<<"invierno"<<endl;
        if(hor>=12 && hor<22)//cout<<"punta"<<endl;
         {
            for (int j=0;j<ncom;j++){
              precio[j]=precio[j]+precios[j][2]*kwh;
            }          
         }
          //aux=precios[com][2];
        else //aux=precios[com][3];//cout<<"valle"<<endl; //precio[com][3];
        {
            for (int j=0;j<ncom;j++){
              precio[j]=precio[j]+precios[j][3]*kwh;
            }          
        }
      }
    }

    case 2:{
      //DHS
      if (vi==0){
        cout<<"verano"<<endl;
        if(hor>=13 && hor<23){
          cout<<"punta"<<endl;
          //aux=precios[com][4];
            for (int j=0;j<ncom;j++){
              precio[j]=precio[j]+precios[j][4]*kwh;
            }
        }
          

        else if(hor>=1 && hor<7){
          cout<<"sv"<<endl;
            for (int j=0;j<ncom;j++){
              precio[j]=precio[j]+precios[j][6]*kwh;
            }
          //aux=precios[com][6];
        }
        
        else{
          cout<<"v"<<endl;
            for (int j=0;j<ncom;j++){
              precio[j]=precio[j]+precios[j][5]*kwh;
            }
          //aux=precios[com][5];
        } 
      }
      else{
        cout<<"invierno"<<endl;
        if(hor>=13 && hor<23){
          cout<<"punta"<<endl;
          //aux=precios[com][4];
            for (int j=0;j<ncom;j++){
              precio[j]=precio[j]+precios[j][4]*kwh;
            }
        }
          

        else if(hor>=1 && hor<7){
          cout<<"sv"<<endl;
            for (int j=0;j<ncom;j++){
              precio[j]=precio[j]+precios[j][6]*kwh;
            }
          //aux=precios[com][6];
        }
        
        else{
          cout<<"v"<<endl;
            for (int j=0;j<ncom;j++){
              precio[j]=precio[j]+precios[j][5]*kwh;
            }
          //aux=precios[com][5];
        } 
      }
    }
    }
    string val;
    for (int j=0;j<ncom;j++){
      val=to_string(precio[j]);
      int p=val.find(".");
      val=val.substr(0,p+3);
      cout<<p<<endl;
      cout<<nomcom[j]<<endl;
      val=val + "€";
      Blynk.virtualWrite(V12, "update", j, nomcom[j].c_str(),   val.c_str());
    }

    //solo actualizo el precio asociado a mi comercializadora
    Blynk.virtualWrite(V7,precio[com]);

    Blynk.virtualWrite(V9,precio[0]);
    //return(aux);
}





void setup()
{
  printf("ok");
  Blynk.virtualWrite(V0, "89.39.22.36/mysql2.php");
  // You can perform HTTPS requests even if your hardware alone can't handle SSL
  // Blynk  can also fetch much bigger messages,
  // if hardware has enough RAM (set BLYNK_MAX_READBYTES to 4096)
  //Blynk.virtualWrite(V0, "https://api.sunrise-sunset.org/json?lat=50.4495484&lng=30.5253873&date=2016-10-01");
}

void loop()
{
  kw=rand() % 10 + 1;
  kwh=rand() % 10 + 1;
  Blynk.run();
  //para el mes en el que estoy necesito saber si horario verano e invierno
  //solo si tengo discriminacion horaria??
  //el periodo del dia en el que estoy 


  
  //si tengo todos los datos necesarios del usuario
  if(ok[0] && ok[1] && ok[2]){
    //nueva confifuracion 
    //aqui tendria que calcular el precio fijo por potencia
    //considero 30 dias el mes 
    for(int j=0;j<ncom;j++){
      precio[j]=precios[j][0]*30*pmax;
      cout<<"precio por potencia al mes:";
      cout<<precio[j]<<endl;
    }
    
    Blynk.virtualWrite(V8,precio[com]);

    kwhcont=0;
    contpmax=0;
    kwmedia=0;
    c=0;
    flag2=1;
    ok[0]=0;
    ok[1]=0;
    ok[2]=0;
  }


  if (flag2 && flagdatos){
    /*float aux=horper();
    cout<<aux<<endl;
    precio=precio+aux*kwh;
    cout<<precio<<endl;
    
    Blynk.virtualWrite(V7,precio);*/
    //calculo de precios en t real
    horper();
    kwhcont=kwhcont+kwh;
    Blynk.virtualWrite(V1,kwhcont);
    //recuento de la potencia media del mes
    //contador de picos de consumo y recojo el momento del dia
    if(kw>=pmax)contpmax++;
    kwmedia=kwmedia+kw;
    c++;
    Blynk.virtualWrite(V10,contpmax);
    Blynk.virtualWrite(V11,kwmedia/c);
    cout<<kw<<endl;
    flagdatos=0;
  }
  delay(5000);
  cout<<"nuevo periodo"<<endl;
  //Blynk.virtualWrite(V0,HIGH);
  //Blynk.virtualWrite(V0,1);

}

int main()
{

  string mensaje;
  mensaje= "hola";
  //printf(mensaje);
  cout<<mensaje<<endl;
  Blynk.begin("ec0bfcf4ac5e4529814fb4ff2d9364a9", "89.39.22.36", 25);
  setup();
  
  while(true) {
    loop();

  }

  return 0;
}

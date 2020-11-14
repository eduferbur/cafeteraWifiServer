/*   MODIFICACIONES 
 V3_Reconexión de wifi: TESTED 2019/08/11
 V2_Rebote de botón: TESTED y funcionando 2019/08/11    %% lo último modificado

PROBLEMAS CORREGIDOS
* CAFETERA_V2
*   1.1. EL BOTÓN ES NC, A LA PLACA LE CUESTA ARRANCAR AL ESTAR SIEMPRE PULSADO EN EL INICIO
*   1.2. EL BOTON NC ES MUY SENSIBLE Y SE ACTIVA CASI SOLO
*   1.3. SI DEJAS PULSADO EL BOTÓN, VA CAMBIANDO DE PULSADO, NO PULSADO CONTINUAMENTE. 
* CAFETERA_V3
*   2.1. LA WIFI CUANDO SE DESCONECTA NO VUELVE A CONECTARSE, Y SE DESCONECTA EN UNAS 10HORAS
/////////////////////////////////////////////////////////////////////////////////

 Placa: Doit ESP32 DevKit V1
 
 ESP32 House Automation
 https://www.youtube.com/watch?v=-9D-vtZ-wl4

 learnelectronics
 14 SEPT 2017
 
 www.youtube.com/c/learnelectronics
 arduino0169@gmail.com
 
 */

#include <WiFi.h>
 
const char* ssid     = "MiFibra-133E-24G";
const char* password = "ociFo9TT";
int in_pulsador=12, out_relay=13;
unsigned long inicio, fin, transcurrido;
unsigned long tempOFF=12*60*1000;  //1seg=1000; 13min = 13*60*1000=780000
bool pulsador, coffeeON;    //cambiado de int a bool %%

int abortwifi=0; 
bool flag_wifi_error=0;

int reconnect_ini=0, reconnect_end, reconnect_transc, reconnect_OFF=20*60*1000;
bool reconectar;
bool se_entera_de_desconexion=0;
 
WiFiServer server(80);
 
void setup()
{
    Serial.begin(115200);
//    pinMode(5, OUTPUT);      // set the LED pin mode
    pinMode(in_pulsador,INPUT);  //pulsador
    pinMode(out_relay,OUTPUT);  //relay

//Inicialización de valores:
    digitalWrite(out_relay,HIGH);  //relay APAGADO AL INICIO
    //Temporizador cafetera
    inicio=0;
    fin=0;
    
    //Temporizador reconexión a Wifi
    reconnect_ini=0;
    reconnect_end=0;
    reconectar=0;
 
    delay(10);
 
    // We start by connecting to a WiFi network
 /*
    //   Serial.println();
    //   Serial.println();
    //   Serial.print("Connecting to ");
    //   Serial.println(ssid);
 */
    WiFi.begin(ssid, password);

    abortwifi=0;  //flag por si falla la conexión
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        //   Serial.print(".");
          abortwifi++;

        if (abortwifi>25)  {
          //   Serial.print("No pudo conectar");
          goto here;
        }
        
    }
/*    
if (WiFi.status() == WL_CONNECTED) {
//   Serial.print("IP address: ");
//   Serial.print( WiFi.localIP());

}  

*/

server.begin();
 here:
   delay (10);
   abortwifi=0;
   
}
 
 
void loop(){
 WiFiClient client = server.available();   // listen for incoming clients


//////Lee la entrada del botón y evita rebotes/////////////////////////////
//////////////////////////////////////////////////////////  
      if (digitalRead(in_pulsador) == HIGH)     //Pregunta si el pulsador está pulsador
      {
          pulsador = 1;     //La variable cambia de valor
          delay(150);      //evita el pequeño rebote que pueda formarse
      }
              
      if (digitalRead(in_pulsador) == LOW && pulsador == 1)
          pulsador = 0;    //La variable vuelve a su valor original
///////////////////////////////////////////////////////////////////////////////////////        

coffeeON = !digitalRead(out_relay);   //el relé tiene lógica negativa

  if (coffeeON) 
    fin=millis();

  if (!coffeeON)
    inicio = 0;

  

    transcurrido=fin-inicio;
  // //   Serial.println(transcurrido);
  // //   Serial.println(tempOFF);
  
  
 ////////MODO MANUAL: LA CAFETERA SE ENCIENDE Y APAGA CON EL BOTÓN,
 ////////PERO SE APAGA POR TERMPORIZADOR SI LLEGA AL tempOFF
 Manual:

   if (pulsador && !coffeeON) {                   //encendemos cafetera
  
   // //   Serial.println("TURN ON por boton");
    digitalWrite(out_relay, LOW);               // GET /H turns the LED on
    inicio=millis();
    
    delay(100);
    while (digitalRead(in_pulsador) == HIGH);
    delay(100);
  
   }
  
   if (coffeeON && (pulsador || transcurrido>tempOFF))  {     //APAGAMOS cafetera
                       
  //  //   Serial.println("TURN OFF por boton");
    digitalWrite(out_relay, HIGH);              
    inicio=0;
    
    delay(100);
    while (digitalRead(in_pulsador) == HIGH);
    delay(100);
   }
////////////////////    FIN DE MANUAL  ////////////////// 


///  WIFI. Si la conexión inicial falló, el sistema intenta volver a conectarse cada "reconnect_OFF" minutos
 Wifi:

if (WiFi.status()!= WL_CONNECTED && !se_entera_de_desconexion) {   //Si está desconectada, comienzo a contar para reconectarme
reconnect_ini=millis();
se_entera_de_desconexion=1;
reconectar=1;
////   Serial.println("se ha enterado ahora que está desconectado");
}

if (WiFi.status() == WL_CONNECTED && se_entera_de_desconexion)
se_entera_de_desconexion=0;


 if (WiFi.status() != WL_CONNECTED  && reconectar)  {
 ////   Serial.println("Nuevo intento de reconexión:");
 reconectar=0;

    WiFi.begin(ssid, password);

    abortwifi=0;  //flag por si falla la conexión
    
    while (!flag_wifi_error || coffeeON) {
        delay(400);
      //  //   Serial.print("re.");
          abortwifi++;

        if (abortwifi>20)  {
       //   //   Serial.print("... No pudo conectar ...");
          flag_wifi_error=1;
          reconectar=0;  
          reconnect_ini=millis();
          goto here_dos;
          
        }
        
    }


 here_dos:
     delay (10);
     
     if (WiFi.status() == WL_CONNECTED)
 //    //   Serial.println(WiFi.localIP());  
    
    server.begin();
  }

/* AQUÍ TERMINA LA RECONEXIÓN DE LA WIFI */

////////////////////////////////////////////////
/*TEMPORIZADOR PARA RECONECTAR WIFI EN CASO DE FALLO*/
//int reconnect_ini=0, reconnect_end, reconnect_transc, reconnect_OFF;

  if (flag_wifi_error) 
    reconnect_end=millis();

  reconnect_transc= reconnect_end - reconnect_ini;

  if(WiFi.status()!= WL_CONNECTED && reconnect_transc>reconnect_OFF){   //si tiempo transcurrido > reconec, intento reconectarme
  reconectar=1;
  flag_wifi_error=0;
  //   //   Serial.println("reconectar=1");
  }
  
  if (WiFi.status()== WL_CONNECTED)
  {
    reconnect_ini=0;
    reconectar=0;
    flag_wifi_error=0;
  }



    

/* CREAMOS EL CLIENTE WEB */
 
  if (client) {                             // if you get a client,
    //   Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
//        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
 
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
 
            // the content of the HTTP response follows the header:
            client.print("<br>");
            client.print("<br>");
            client.print("COFFEE ON -->  <a href=\"/L\">HERE</a> <br>");
            client.print("<br>");
            client.print("<br>");
            client.print("COFFEE OFF -->  <a href=\"/H\">HERE</a> <br>");
            if (coffeeON)
            client.print("SE ESTÁ HACIENDO UN CAFELITO <br>");

            if (!coffeeON)
            client.print("CAFETERA APAGADA <br>");
           
 
 /*           client.print("Click <a href=\"/I\">here</a> to turn WALKWAY on.<br>");
            client.print("Click <a href=\"/J\">here</a> to turn WALKWAY off.<br>");
 
            client.print("Click <a href=\"/O\">here</a> to turn BACK DOOR on.<br>");
            client.print("Click <a href=\"/P\">here</a> to turn BACK DOOR off.<br>");
           
 
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
*/           break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
 
        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /L")) {
           digitalWrite(out_relay, LOW);               // GET /H turns the LED on
           inicio=millis();
        }

        if (currentLine.endsWith("GET /H")) {
           digitalWrite(out_relay, HIGH);              
           inicio=0;
        }

/*        if (currentLine.endsWith("GET /I")) {
          digitalWrite(12, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /J")) {
          digitalWrite(12, LOW);                // GET /L turns the LED off
        }
 
        if (currentLine.endsWith("GET /O")) {
          digitalWrite(13, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /P")) {
          digitalWrite(13, LOW);                // GET /L turns the LED off
*/      

        }
      }
    }
   
    // close the connection:
    client.stop();

   
  }

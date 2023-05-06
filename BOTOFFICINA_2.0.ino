#include <WiFi.h> //Wifi library
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <String.h>
#include "esp_wpa2.h" //wpa2 library for connections to Enterprise networks


#define Lunghezza 150
int lista_persone[Lunghezza +1];
int ValoreSoglia=3000;
int OfficinaAperta;
const int pinMisura = 35;
int misuraLuce = 0;
int riconnessione = 0;
byte accesoBot=0;

unsigned long ControlloNotifiche = 0;//questa parte serve per controllare ogni 60 sec se si é disconnessa la connessione 
unsigned long intervallo = 10000;





#define WIFI_SSID "provaprova"
#define WIFI_PASSWORD "provaprova"
#define ONBOARD_LED  2
#define BOT_TOKEN "6165276369:AAE-4va7jYLiSaqpsaJA0mprez4WQ4UAo-w"
#define EAP_ANONYMOUS_IDENTITY "d.nascivera" //anonymous@example.com, or you can use also nickname@example.com
#define EAP_IDENTITY "d.nascivera" //nickname@example.com, at some organizations should work nickname only without realm, but it is not recommended
#define EAP_PASSWORD "31052001Davide" //password for eduroam account

//SSID NAME
const char* ssid = "Unipisa"; // eduroam SSID

const unsigned long BOT_MTBS = 1000; // mean time between scan messages
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done


String eliminaNumero(int kill)
{
    String Messaggio;
    for(int i=0; i<=Lunghezza;i++)
    {
      if(lista_persone[i]==kill)
        {
          for(int j=i; j<Lunghezza;j++)
            {
              lista_persone[j]=lista_persone[j+1];
            }
            lista_persone[Lunghezza]=0;
            Messaggio = "Sei stato eliminato dalla lista nera (niente sambucone per lei \xF0\x9F\x9A\xAB)";
            return Messaggio;
            //break;  
        }
    }
    Messaggio = "Brah devi essere nella lista prima di paccare \xF0\x9F\x9A\xB6";
    return Messaggio;
}

String AggiungiNumero(int nuovo)
{
    String risposta;
    for(int i=0; i<=Lunghezza;i++)
    {
      if(lista_persone[i]==nuovo)
        {
          risposta = "Sei giá stato aggiunto alla lista aggiornamenti champ \xF0\x9F\x9B\x80";
          return risposta;
        }
        else if(lista_persone[i]==0)
        {
          lista_persone[i]=nuovo;
          risposta = "Sarai avvisato quando aprirà/chiuderà l'officina (e quando la roma farà un goal) \xF0\x9F\x9A\x80";
          return risposta;
        }
    }  
    risposta = "Errore troppe persone nella lista";
    return risposta;
}
int ControllaLunghezza()
{
  for(int i=0; i<=Lunghezza;i++)
  {
    if(lista_persone[i]==0){
      return i;
    }  
  }
}
void OfficinaAperto()
{
   String welcome = "L'officina ha aperto capo \xE2\x9C\x85\xF0\x9F\x9A\x96 \n/BastaNotifiche";
   for(int i=0; i<=Lunghezza;i++)
   {
     if(lista_persone[i]==0) break;
     bot.sendMessage(String(lista_persone[i]), welcome, "Markdown");
     accesoBot=1;
   }
}

void OfficinaChiusa()
{
      String welcome = "L'officina ha chiuso capo \xE2\x9D\x8C\xF0\x9F\x9A\xB7 \n/BastaNotifiche";
      for(int i=0; i<=Lunghezza;i++)
      {
        if(lista_persone[i]==0) break;
        bot.sendMessage(String(lista_persone[i]), welcome, "Markdown");
        accesoBot=0;
      }
}


void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);                                                 

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    Serial.println(bot.messages[i].chat_id);

    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";
    String Carattere =text.substring(0, 1);
    
    if(Carattere=="r")
    {
      String Prova =text.substring(1);
      ValoreSoglia = Prova.toInt();
      bot.sendMessage(chat_id, text.substring(1), "Markdown");
    }
    
    if(text == "/nerd")
    {
      String Messaggio = "Il bot funziona leggendo una fotoresistenza banalmente utilizzando un analogRead. Solitamente quando accendono le luci il valore sta sui 4k \n\n";
      Messaggio += "Valore attuale = ";
      Messaggio += String(misuraLuce);
      Messaggio += "\nValore Soglia = ";
      Messaggio += String(ValoreSoglia);
      Messaggio += "\nPersone iscritte = ";
      Messaggio += String(ControllaLunghezza());
      Messaggio += "\nRicconnesso = ";
      Messaggio += String(riconnessione);
      Messaggio += "\nFreeHeap = ";
      Messaggio += String(ESP.getFreeHeap());

      bot.sendMessage(chat_id, Messaggio, "Markdown");
    }
    
    if (text == "/BastaNotifiche")
    {     
      String messaggio = eliminaNumero(chat_id.toInt());
      messaggio += "\n/TienimiAggiornato";
      String keyboardJson = "[[\"/start\", \"/TienimiAggiornato\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, messaggio, "Markdown",keyboardJson, true);
    }
    if (text == "/prova")
    {
      String messaggio = "Cambiato lo stato";
      accesoBot=0;
      bot.sendMessage(chat_id, messaggio, "Markdown");
    }
    
    if (text == "/status")
    {
      String messaggio = "Current status:  ";
      messaggio += String(misuraLuce);
      //String NuovaProva = String(prova);
      //String welcome = "Ciao questo è il bot dell'officina dell'università di Pisa!\n se stai leggendo questo messaggio significa che l'officina è aperta! ";
      bot.sendMessage(chat_id, messaggio, "Markdown");
    }
      
    if (text == "/TienimiAggiornato")
    {
      String messaggio = AggiungiNumero(chat_id.toInt());
      messaggio += "\n/BastaNotifiche";
      String keyboardJson = "[[\"/start\", \"/BastaNotifiche\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, messaggio, "Markdown",keyboardJson, true);
    }

    if (text == "/dirigenza")
    {
      String messaggio = "https://youtu.be/dQw4w9WgXcQ";
      bot.sendMessage(chat_id, messaggio, "Markdown");
    }
    
    if (text == "/ae")
    {
      String test_photo_url = "https://www.intimamente.shop/wp-content/uploads/2022/04/TRAPUNTINO-NAPOLI-2.jpg";
      bot.sendPhoto(chat_id, test_photo_url, "Daje Roma Daje, Fozza Napoli");
    }
    if (text == "Tuono")
    {
      String test_photo_url = "https://i.ibb.co/p2XWRsk/Immagine-Whats-App-2023-05-06-ore-00-04-26.jpg";
      bot.sendPhoto(chat_id, test_photo_url, "Vulva dieci");
    }
    if (text == "/start")
    {
      String welcome;
      
      if(OfficinaAperta==0){
        welcome = "Officina Chiusa \xE2\x9D\x8C";
      }
      else if (OfficinaAperta=1){
        welcome = "Officina Aperta \xE2\x9C\x85";
      }
      welcome += "\n /TienimiAggiornato \n";
      String keyboardJson = "[[\"/start\", \"/TienimiAggiornato\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, welcome, "Markdown",keyboardJson, true);
      
    }
  }
}



void setup() {
  Serial.begin(9600);
  delay(10);
  pinMode(ONBOARD_LED,OUTPUT);
  Serial.print(F("Connecting to network: "));
  Serial.println(ssid);
  WiFi.disconnect(true);  //disconnect from WiFi to set new WiFi connection
  WiFi.mode(WIFI_STA); //init wifi mode
  
  //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_ANONYMOUS_IDENTITY, EAP_IDENTITY, EAP_PASSWORD); //WITHOUT CERTIFICATE - WORKING WITH EXCEPTION ON RADIUS SERVER
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println("");
  Serial.println(F("WiFi is connected!"));
  Serial.println(F("IP address set: "));
  Serial.println(WiFi.localIP()); //print LAN IP




  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
  
}



void loop() {
  yield();
  misuraLuce = analogRead(pinMisura);
  
  //Serial.println(misuraLuce);

  
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    
    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
  
  if(misuraLuce<ValoreSoglia)
  {
    if(OfficinaAperta==1) OfficinaChiusa();
    OfficinaAperta=0;
    digitalWrite(ONBOARD_LED,LOW);
  }
  else
  {
    if(OfficinaAperta==0)   OfficinaAperto();
    OfficinaAperta=1;
    digitalWrite(ONBOARD_LED,HIGH);
  }

  unsigned long currentMillis = millis();

  if (currentMillis - ControlloNotifiche >=intervallo) 
  {
    if(WiFi.status() != WL_CONNECTED)
    {
      //Serial.print(millis());
      //Serial.println("Reconnecting to WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
      riconnessione ++;
    }
    if(OfficinaAperta!=accesoBot)
    {
      if(OfficinaAperta==0) OfficinaChiusa();
      if(OfficinaAperta==1) OfficinaAperto();
      accesoBot=OfficinaAperta;
    }
    ControlloNotifiche = currentMillis;
  }
  

}

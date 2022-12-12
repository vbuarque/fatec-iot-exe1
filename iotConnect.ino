#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"

TaskHandle_t tTaskControllMinuteTime;

SemaphoreHandle_t mutex;

char *ntpServer = "pool.ntp.org";
struct tm timeinfo;

char *ssid = "WIFI IOT";
char *pwd = "Fatec@123";

String serverName = "http://postman-echo.com/";

bool sharedData = false;

void setup()
{
  Serial.begin(115200);

  mutex = xSemaphoreCreateMutex();
  if (mutex == NULL)
  {
    Serial.println("Error - Mutex not created");
  }
  
  WiFi.begin(ssid, pwd);
  connectWifi();
  
  configTime(0, 0, ntpServer);  
  
  xTaskCreatePinnedToCore(
    verifyTime,   // task function
    "VerifyTime", // task name
    10000,        // Task stack size
    NULL,         // task parameters
    1,            // task priority
    &tTaskControllMinuteTime,      // task handle
    1           // task core (core 1=loop)
  );
}

void loop()
{  
  
}

// Sincronizar a data e hora com o servidor NTP 
void verifyTime(void *pvParameters)
{
  while (true)
  {
    xSemaphoreTake(mutex, portMAX_DELAY);

    // sincronização da data e hora com o servidor NTP
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Acesso ao ntp falhou");
    }

    // transmitindo dados a cada 5 minutos
    if (timeinfo.tm_min % 5 == 0) {
      if (sharedData == false) {
        shareData();
      }     
    } else {
      sharedData = false;
    }  
    xSemaphoreGive(mutex); 
  }    
}

// Conectando ao WiFi
void connectWifi()
{
  Serial.println("Conectando...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" ");
  Serial.print("Conectado ao WiFi com o IP: ");
  Serial.println(WiFi.localIP());
}

// Transmitindo os dados para o servidor
void shareData()
{
  if (WiFi.status() == WL_CONNECTED)
  {   
    WiFiClient client;
    HTTPClient http_post;
    
    String url = serverName + "post";
    http_post.begin(client, url);
    
    // configurando o header da requisição
    http_post.addHeader("Content-Type", "application/json");
    String data = "{\"Message\": \"Realizando o teste de requisição\", \"Username\": \"Vinicius Buarque\", \"Description\": \"Estudante da fatec sjc\"}";

    int httpCode = http_post.POST(data);
    if (httpCode > 0)
    {
      Serial.println(httpCode);
      String payload = http_post.getString();
      Serial.print("Response of server: ");
      Serial.println(payload);

      sharedData = true;
    }
    else
    {
      sharedData = false;
      Serial.println("Http error");
    }
  }
  else
  {
    Serial.println("WIFI connection not established");
    connectWifi();
  }    
}
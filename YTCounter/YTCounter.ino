#define YTHOST  "www.youtube.com"
#define YTTYPE  "user"
#define YTUSER  "cengizermis1"

#define COUNT   8 
#define NUMMAX  4
#define ROTATE  90
#define ARROW   140

#define BTNPin  D2
#define CSPin   D0
#define CLKPin  D5
#define DINPin  D7

#include "Fonts.h"
#include "LedMatrix.h"
#include "OTA.h"

uint32_t convToInt(const char *txt)
{
  uint32_t val = 0;
  for (uint32_t i = 0; i < strlen(txt); i++)
    if (isdigit(txt[i])) val = val * 10 + (txt[i] & 0xF);
  return val;
}

uint32_t getYTSubs(const char *channelId, uint32_t *pSubs, uint32_t *pViews)
{
  if (!pSubs || !pViews) return -2;

  WiFiClientSecure client;

  Serial.print("Connecting to ");
  Serial.print(YTHOST);

  if (!client.connect(YTHOST, 443))
  {
    Serial.println(" (connection failed)");
    return -1;
  }

  client.print(String("GET /") + YTTYPE + "/" + YTUSER + "/about HTTP/1.1\r\n" + "Host: " + YTHOST + "\r\nConnection: close\r\n\r\n");

  int8_t repeatCounter = 10;

  while (!client.available() && repeatCounter--)
  {
    Serial.println(" (connected)");
    String YT = String("") + char(ARROW) + " YT " + char(ARROW);
    blinkStringCenter(YT.c_str(), font5x7, 500, 3);
  }

  uint32_t idxS, idxE, statsFound = 0;
  *pSubs = *pViews = 0;

  while (client.connected() && client.available())
  {
    String line = client.readStringUntil('\n');
    if (statsFound == 0)
    {
      statsFound = (line.indexOf("about-stats") > 0);
    }
    else
    {
      idxS = line.indexOf("<b>");
      idxE = line.indexOf("</b>");
      String val = line.substring(idxS + 3, idxE);
      if (!*pSubs)
        *pSubs = convToInt(val.c_str());
      else
      {
        *pViews = convToInt(val.c_str());
        break;
      }
    }
  }
  client.stop();
  return 0;
}

void setup()
{
  pinMode(BTNPin, INPUT_PULLUP);

  initMatrix();

  Serial.begin(115200);

  
  Serial.println();
  Serial.println();
  Serial.println("Booting . . .");

  WiFiSetup();
  OTASetup();

  Serial.println();
}

void loop()
{
  uint32_t subs, views, cnt = 0;
  String lb1, lb2, yt1, yt2;

  lb1 = "ABONE";
  lb2 = "İZLEME";
  yt1 = "0";
  yt2 = "0";

  while (1)
  {
    OTALoop();

    if (!cnt--)
    {
      cnt = COUNT - 1;
      if (getYTSubs(YTHOST, &subs, &views) == 0)
      {
        yt1 = String(formatNumber(subs));
        yt2 = String(formatNumber(views));
      }
    }
    
    scrollStringCenter(lb1.c_str(), font5x7, 20);
    blinkStringCenter(lb1.c_str(), font5x7, 750, 3);
    delay(1000);
    scrollStringCenter(yt1.c_str(), font3x7, 20);
    delay(1000);
    blinkStringCenter(yt1.c_str(), font3x7, 750, 3);
    delay(2000);
    
    scrollStringCenter(lb2.c_str(), font5x7, 20);
    blinkStringCenter(lb2.c_str(), font5x7, 750, 3);
    delay(1000);
    scrollStringCenter(yt2.c_str(), font3x7, 20);
    delay(1000);
    blinkStringCenter(yt2.c_str(), font3x7, 750, 3);
    delay(2000);
  }
}

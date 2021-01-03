/*
cd ~/Arduino/libraries
git clone https://github.com/lewisxhe/CoinMarketCapApi2
git clone -b 6.x https://github.com/bblanchon/ArduinoJson.git
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "CoinMarketCapApi.h"
#include "xrp.h"
#include "Free_Fonts.h" // Include the header file attached to this sketch
//---------------------------------------//

// Edit this for your network
#define wifi_ssid "SSID"
#define wifi_password "PASSWORD"

// The new api requires a developer key, so you must apply for a key to use
// https://pro.coinmarketcap.com/account
#define APIKEY "APIKEY"

//---------------------------------------//
WiFiClientSecure client;
CoinMarketCapApi api(client, APIKEY);

// CoinMarketCap's limit is "no more than 10 per minute"
// Make sure to factor in if you are requesting more than one coin.
unsigned long api_mtbs = 300000; //mean time between api requests
unsigned long api_due_time = 0;
// SPI TFT settings edit UserSetup !!!
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI(); // size 320x240

// begin setup
void setup()
{
    Serial.begin(115200);
    Serial.println(F("Boot Ticker"));

    tft.setRotation(3); //rotate 90 degree
    tft.fillScreen(TFT_BLACK);

    uint16_t time = millis();
    time = millis() - time;
    tft.init();
    tft.fillScreen(TFT_BLACK);
    tft.setSwapBytes(true);
    tft.pushImage(109, 69, xrpWidth, xrpHeight, xrp);
    delay(3000);

    // Starup
    Serial.println(wifi_ssid);
    WiFi.begin(wifi_ssid, wifi_password);
    tft.drawCentreString("Connecting to WIFI", 160, 170, 2);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    tft.setTextColor(TFT_GREEN);
    tft.drawCentreString("WiFi connected", 160, 183, 2);
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay(1000);

    tft.setTextColor(TFT_WHITE);
    tft.fillRect(70, 170, 190, 16, TFT_BLACK);
    tft.drawCentreString("XRP - PHP", 160, 170, 2);
}

void printTickerData(String ticker)
{
    Serial.println("---------------------------------");
    Serial.println("Getting ticker data for " + ticker);

    //For the new API, you can use the currency ID or abbreviated name, such as
    //Bitcoin, you can view the letter after Circulating Supply at https://coinmarketcap.com/, it is BTC

    CMCTickerResponse response = api.GetTickerInfo(ticker, "PHP");
    if (response.error == "")
    {
        Serial.print("ID: ");
        Serial.println(response.id);
        Serial.print("Name: ");
        Serial.println(response.name);
        Serial.print("Symbol: ");
        Serial.println(response.symbol);

        Serial.print("Rank: ");
        Serial.println(response.cmc_rank);

        Serial.print("Price: ");
        Serial.println(response.price);

        Serial.print("24h Volume: ");
        Serial.println(response.volume_24h);
        Serial.print("Market Cap: ");
        Serial.println(response.market_cap);

        Serial.print("Circulating Supply: ");
        Serial.println(response.circulating_supply);
        Serial.print("Total Supply: ");
        Serial.println(response.total_supply);

        Serial.print("Percent Change 1h: ");
        Serial.println(response.percent_change_1h);
        Serial.print("Percent Change 24h: ");
        Serial.println(response.percent_change_24h);
        Serial.print("Percent Change 7d: ");
        Serial.println(response.percent_change_7d);
        Serial.print("Last Updated: ");
        Serial.println(response.last_updated);

        // price
        tft.setTextColor(TFT_WHITE);
        tft.setFreeFont(FF24);
        tft.setTextDatum(MC_DATUM);

        tft.fillRect(70, 30, 190, 40, TFT_BLACK);
        if (response.percent_change_1h < 0)
        {
            tft.setTextColor(TFT_RED);
        }
        if (response.percent_change_1h > 0)
        {
            tft.setTextColor(TFT_GREEN);
        }
        //tft.drawCentreString(String(response.price), 160, 30, 6);
        tft.drawString(String(response.price), 160, 45, GFXFF);
        tft.setTextDatum(TL_DATUM);

        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.fillRect(0, 225, 320, 15, TFT_BLACK);
        tft.drawString("Rank [" + String(response.cmc_rank) + "]", 5, 225, 2);

        // hours change
        if (response.percent_change_1h < 0)
        {
            tft.setTextColor(TFT_RED);
        }
        if (response.percent_change_1h > 0)
        {
            tft.setTextColor(TFT_GREEN);
        }
        tft.drawString("1h [" + String(response.percent_change_1h) + "%]", 65, 225, 2);

        // 24 hours change
        if (response.percent_change_24h < 0)
        {
            tft.setTextColor(TFT_RED);
        }
        if (response.percent_change_24h > 0)
        {
            tft.setTextColor(TFT_GREEN);
        }
        tft.drawString("24h [" + String(response.percent_change_24h) + "%]", 145, 225, 2);

        // 7d days change
        if (response.percent_change_7d < 0)
        {
            tft.setTextColor(TFT_RED);
        }
        if (response.percent_change_7d > 0)
        {
            tft.setTextColor(TFT_GREEN);
        }
        tft.drawString("7d [" + String(response.percent_change_7d) + "%]", 235, 225, 2);
    }
    else
    {
        Serial.print("Error getting data: ");
        Serial.println(response.error);
        tft.setTextColor(TFT_RED);
        tft.drawCentreString("API error", 160, 0, 2);
    }
    Serial.println("---------------------------------");
}

float RSSI = 0.0;

void loop()
{
    unsigned long timeNow = millis();
    tft.setTextColor(TFT_WHITE);
    tft.fillRect(70, 186, 190, 16, TFT_BLACK);
    tft.drawCentreString(String(timeNow) + " > " + String(api_due_time), 160, 183, 2);

    if ((timeNow > api_due_time))
    {
        // int signal bars
        Serial.print("WiFi Signal strength: ");
        Serial.print(WiFi.RSSI());

        int bars;
        RSSI = WiFi.RSSI();

        if (RSSI >= -55)
        {
            bars = 5;
            Serial.println(" 5 bars");
        }
        else if (RSSI < -55 & RSSI >= -65)
        {
            bars = 4;
            Serial.println(" 4 bars");
        }
        else if (RSSI < -65 & RSSI >= -70)
        {
            bars = 3;
            Serial.println(" 3 bars");
        }
        else if (RSSI < -70 & RSSI >= -78)
        {
            bars = 2;
            Serial.println(" 2 bars");
        }
        else if (RSSI < -78 & RSSI >= -82)
        {
            bars = 1;
            Serial.println(" 1 bars");
        }
        else
        {
            bars = 0;
            Serial.println(" 0 bars");
        }

        tft.fillRect(280, 0, 40, 32, TFT_BLACK);
        // signal bars
        for (int b = 0; b <= bars; b++)
        {
            tft.fillRect(280 + (b * 6), 23 - (b * 4), 5, b * 4, TFT_WHITE);
        }
        printTickerData("XRP");
        api_due_time = timeNow + api_mtbs;
    }
    delay(1000);
}

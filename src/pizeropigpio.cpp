//============================================================================
// Name        : pizeropigpio.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <unistd.h>
#include <pigpiod_if2.h>

bool startSCD30(int pi, int handle);
bool getSCD30Status(int pi, int handle);
bool getSCD30Readings(int pi, int handle);

// #define PIN 6

#define MILLISEC 1000
#define PAUSE 5000

float fCO2, fTemperature, fRH;

using namespace std;

int main()
{
	cout << "SCD30 Test Program" << endl;

    int hI2C;

    int pi = pigpio_start(NULL, NULL);

    if (pi < 0)
    {
       cout << "Pigpio start failed" << endl;
    }
    else
    {
        cout << "GPIO init OK" << endl;

        hI2C = i2c_open(pi,  1, 0x61, 0);

        if (hI2C < 0)
        {
            cout << "I2C bad handle: " << hI2C << endl;
        }
        else
        {
            // start sensor
            startSCD30(pi, hI2C);
            usleep(100 * MILLISEC);

            while (true)
            {
                if (getSCD30Status(pi, hI2C))
                {
                    usleep(PAUSE);

                    if (getSCD30Readings(pi, hI2C))
                    {
                        cout << fCO2 << "," << fTemperature << "," << fRH << endl;
                    }
                }

                usleep(500 * MILLISEC);
            }
        }
    }
	return 0;
}



bool startSCD30(int pi, int handle)
{
    char startCmd[] = {0x00, 0x10, 0x00, 0x00};

    return (i2c_write_device(pi, handle, startCmd, 4) == 0);
}

bool getSCD30Status(int pi, int handle)
{
    char statusCmd[] = {0x02, 0x02};
    char datain[3] = { 0 };

    if (i2c_write_device(pi, handle, statusCmd, 2) != 0) return false;

    usleep(PAUSE);

    if (i2c_read_device(pi, handle, datain, 3) != 3) return false;

    return (datain[1] == 1);
}

bool getSCD30Readings(int pi, int handle)
{
    char readCmd[] = {0x03, 0x00};
    char dataBuffer[18] = { 0 };
    uint8_t co2Bytes[4] = { 0 };
    uint8_t tempBytes[4] = { 0 };
    uint8_t rhBytes[4] = { 0 };
    int expected = 18;

    // send reading request
    if (i2c_write_device(pi, handle,  readCmd, 2) != 0) return false;

    // wait 5ms
    usleep(PAUSE);

    // read back data
    if (i2c_read_device(pi, handle, dataBuffer, expected) != expected) return false;

    // convert to floats
    co2Bytes[0] = dataBuffer[4];
    co2Bytes[1] = dataBuffer[3];
    co2Bytes[2] = dataBuffer[1];
    co2Bytes[3] = dataBuffer[0];

    tempBytes[0] = dataBuffer[10];
    tempBytes[1] = dataBuffer[9];
    tempBytes[2] = dataBuffer[7];
    tempBytes[3] = dataBuffer[6];

    rhBytes[0] = dataBuffer[16];
    rhBytes[1] = dataBuffer[15];
    rhBytes[2] = dataBuffer[13];
    rhBytes[3] = dataBuffer[12];

    fCO2 = *(float*)co2Bytes;

    fTemperature = *(float*)tempBytes;

    fRH = *(float*)rhBytes;

    return true;
}

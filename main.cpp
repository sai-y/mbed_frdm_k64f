//gps.cpp
//for use with Adafruit Ultimate GPS
//Reads in and parses GPS data
 
#include "mbed.h"
#include "MBed_Adafruit_GPS.h"
#include "SDFileSystem.h"

Serial * gps_Serial;
Serial pc (USBTX, USBRX);
AnalogIn ain(A0);

SDFileSystem sd(PTE3, PTE1, PTE2, PTE4, "sd");
int main() {
    
    pc.baud(115200); //sets virtual COM serial communication to high rate; this is to allow more time to be spent on GPS retrieval
    
    gps_Serial = new Serial(PTD3, PTD2); //serial object for use w/ GPS
    Adafruit_GPS myGPS(gps_Serial); //object of Adafruit's GPS class
    char c; //when read via Adafruit_GPS::read(), the class returns single character stored here
    Timer refresh_Timer; //sets up a timer for use in loop; how often do we print GPS info?
    const int refresh_Time = 2000; //refresh time in ms
    
    myGPS.begin(9600);  //sets baud rate for GPS communication; note this may be changed via Adafruit_GPS::sendCommand(char *)
                        //a list of GPS commands is available at http://www.adafruit.com/datasheets/PMTK_A08.pdf
    
    myGPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //these commands are defined in MBed_Adafruit_GPS.h; a link is provided there for command creation
    myGPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    myGPS.sendCommand(PGCMD_ANTENNA);
    
    pc.printf("Connection established at 115200 baud...\n");
    
    wait(1);
    
    refresh_Timer.start();  //starts the clock on the timer
    
    while(true){
        c = myGPS.read();   //queries the GPS
        
        if (c) { pc.printf("%c", c); } //this line will echo the GPS data if not paused
        
        //check if we recieved a new message from GPS, if so, attempt to parse it,
        if ( myGPS.newNMEAreceived() ) {
            if ( !myGPS.parse(myGPS.lastNMEA()) ) {
                continue;   
            }    
        }
        
        //check if enough time has passed to warrant printing GPS info to screen
        //note if refresh_Time is too low or pc.baud is too low, GPS data may be lost during printing
        if (refresh_Timer.read_ms() >= refresh_Time) {
            refresh_Timer.reset();
            pc.printf("Time: %d:%d:%d.%u\n", myGPS.hour, myGPS.minute, myGPS.seconds, myGPS.milliseconds);   
            pc.printf("Date: %d/%d/20%d\n", myGPS.day, myGPS.month, myGPS.year);
            pc.printf("Fix: %d\n", (int) myGPS.fix);
            pc.printf("Quality: %d\n", (int) myGPS.fixquality);
            
            if (myGPS.fix) {
                pc.printf("Location: %5.2f%c, %5.2f%c\n", myGPS.latitude, myGPS.lat, myGPS.longitude, myGPS.lon);
                int degree_value = (int) myGPS.latitude/100;
                double decimal_value = (double) ((myGPS.latitude / 100-degree_value)*100);
                float decimal_degree = degree_value + (decimal_value/60); 
                
                degree_value = (int) myGPS.longitude/100;
                decimal_value = (double) ((myGPS.longitude / 100 - degree_value)*100);
                float decimal_longitude = degree_value + (decimal_value/60);
                pc.printf("Decimal latitude: %5.5f %c\n", decimal_degree, myGPS.lat);
                pc.printf("Decimal longitude: %5.5f %c\n", decimal_longitude, myGPS.lon);
                pc.printf("Speed: %5.2f knots\n", myGPS.speed);
                pc.printf("Angle: %5.2f\n", myGPS.angle);
                pc.printf("Altitude: %5.2f\n", myGPS.altitude);
                pc.printf("Satellites: %d\n", myGPS.satellites);
                mkdir("/sd/mydir", 0777);
                pc.printf("Temperature: %3.3f degrees Celsius\n",ain.read()*330);
                FILE *fp = fopen("/sd/mydir/gps_data.txt", "a+");
                if(fp == NULL) {
                    error("Could not open file for write\n");
                }
                fprintf(fp, "%d/%d/20%d,%d:%d:%d.%u,%5.5f %c, %5.5f %c, %3.3f Celsius\n",myGPS.day, myGPS.month, myGPS.year,myGPS.hour, myGPS.minute, myGPS.seconds, myGPS.milliseconds,decimal_degree,myGPS.lat,decimal_longitude,myGPS.lon,ain.read()*330);
                fclose(fp);
               
            }
            
        }
    }
}
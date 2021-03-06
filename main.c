#include "stdio.h"
#include "stdlib.h"
#include <stdbool.h>
#include <stdint.h>

bool msec_2_Flag = 0;
bool first_pXY_ready =0;                //this will be set after 32 samples have been taken 

#define adc_postVGA channel_AN0

int32_t sdy =60;                        // initial guess for the standard deviation of y
uint16_t rawPostVGA = 0;
  
//correlation_function_X is the function to which we are checking the correlation to Y_data
//this could be a known voice audio clip for example, or whatever signal you are searching for.
uint8_t correlation_function_X[32] = {120,125,120,115,120,125,120,115,120,125,120,115,120,125,120,115,120,125,120,115,120,125,120,115,120,125,120,115,120,125,120,115}; 
uint16_t SDX = 4;                       //standard deviation of x - SD[X]
int16_t EX = 120;                       //expected value of correlation function - E[X]


int16_t Y_data[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  // array of 32 samples
uint8_t Yptr = 0;                       //points to current y data sample
int16_t EY = 0;                         //expected value of y, E[Y]
int32_t EYY = 0;                        //expected value of y^2, E[Y^2]
int32_t EXY =0;                         //expected value of x*y, E[XY]
int16_t Y_accumulate =0;
int32_t YY_accumulate = 0;
int32_t XY_accumulate = 0;
int32_t numerator = 0;                  // E[XY] - E[X]E[Y]
int32_t denominator =1;                 // standard deviation of x * standatrd deviation of y = sqrt(E[X^2]- [E[X]]^2) * sqrt(Y[X^2]- [E[Y]]^2)
int32_t pXY = 0;                        //initialize the correlation coefficient


void main(void)
{
    // initialize the device
    msec_2_Flag = 0;   
    SYSTEM_Initialize();
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
 
    while (1){
 
        if(msec_2_Flag==1){              //this is set by an external timer interrupt routine (once every 2 msec in this case)         
                    
            //Aquire the new sample:   
            rawPostVGA = ADC_GetConversion(adc_postVGA);       
            LATCbits.LATC0  = 0;
            TRISCbits.TRISC0 = 0;
            
            //RC0 pin used to measure the calculation time 
            PORTCbits.RC0 = 1;   
            //start the correlation coefficient calculation here         

            Y_accumulate = Y_accumulate - (uint32_t)Y_data[Yptr] + rawPostVGA;

            EY = (uint16_t)(Y_accumulate>>5);

            uint32_t yyold = (int32_t)Y_data[Yptr]*Y_data[Yptr];
            uint32_t yynew = (int32_t)rawPostVGA*rawPostVGA;
            
            YY_accumulate = (int32_t)YY_accumulate - yyold +yynew;
            EYY = (int32_t)(YY_accumulate>>5);

            Y_data[Yptr] = rawPostVGA;
            XY_accumulate =0;
            uint8_t iy = Yptr;
            for (uint8_t ix = 0 ; ix<32; ix++){
                uint32_t xy = Y_data[iy] * (uint32_t)correlation_function_X[ix];
                XY_accumulate = XY_accumulate + xy;
                iy = iy+1;
                if (iy>=32){
                    iy=0;
                }
            }
            EXY = (int32_t) (XY_accumulate>>5);

            Yptr =Yptr+1; 
            if (Yptr >=32) {
                Yptr =0;         
            }
            int32_t exey = (int32_t)EY*EX;
            int32_t exy_exey = EXY - exey;
            numerator = (int32_t)100*exy_exey;            //this will make the range of pXY -100 to +100 rather than -1 to +1

            if ( (EY*EY) < EYY){
                int32_t eyey= (int32_t)EY*EY;
                int32_t eyy_eyey = EYY-(int32_t)EY*EY;
                
                //Here I start the babylonian method for calculating the square root. 
                //The result happens to be the standard deviation of y in this case.
                int32_t initial_sqrt_estimate = sdy;

                int32_t second_estimate = (int32_t)initial_sqrt_estimate + eyy_eyey/ initial_sqrt_estimate;
                second_estimate = (int32_t)second_estimate>>1;

                int32_t third_estimate = (int32_t)second_estimate + eyy_eyey/ second_estimate;
                third_estimate = (int32_t)third_estimate>>1; 

                int32_t forth_estimate = (int32_t)third_estimate + eyy_eyey/ third_estimate;
                sdy = (int32_t)forth_estimate>>1;       //final square root estimate
                if (sdy ==0){
                    sdy =1;
                }
                denominator = (int32_t)SDX*sdy;
            }
            else {
              //printf("can't calculate SDY");
            }
            
            pXY = (int32_t)numerator/denominator;   //correlation coefficient
                  
            //this is the end correlation coefficient calculation  
            PORTCbits.RC0 = 0;  //RC0 pin used to measure the calculation time          
            
            if (Yptr >=31){
              first_pXY_ready = 1;                  //first valid pXY is ready
            }
            
            if (first_pXY_ready){                   //start printing the correlation coefficient as soon as the first valid calc is ready
              printf ("%ld,", pXY);   
            }
                                                                                    
            msec_2_Flag=0;                         // this should alway be reset before it gets set again in the the interrupt routine               
        }                   
    }    
}

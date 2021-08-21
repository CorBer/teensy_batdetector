#ifndef _ILI9341_T3_EXT_H
#define _ILI9341_T3_EXT_H

#include "ILI9341_t3.h" //baseclass

class ILI9341_t3_ext: public ILI9341_t3 {
  public:
    ILI9341_t3_ext(uint8_t _CS, uint8_t _DC, uint8_t _RST, uint8_t _MOSI, uint8_t _SCLK, uint8_t _MISO); // constructor
    void setScrollarea(uint16_t top, uint16_t bottom);
  private:
};

//constructor points directly to default ILI9341_t3 
ILI9341_t3_ext::ILI9341_t3_ext(uint8_t _CS, uint8_t _DC, uint8_t _RST, uint8_t _MOSI, uint8_t _SCLK, uint8_t _MISO):
    ILI9341_t3(_CS, _DC, _RST, _MOSI, _SCLK, _MISO)
    {
      }

//additional routine to scroll pixels in an area
void ILI9341_t3_ext::setScrollarea(uint16_t top, uint16_t bottom)
{
	beginSPITransaction(_clock);
	writecommand_cont(0x33);
	writedata16_last(top); //section
	writedata16_last(ILI9341_TFTHEIGHT-top-bottom); //difference
	writedata16_last(bottom); //section 
	endSPITransaction();
}


#endif

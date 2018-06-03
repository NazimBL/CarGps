/////////////////////////////////////////////////////////////////////////
////                          EMNT_USB.h                    ////
////                                                                 ////
/////////////////////////////////////////////////////////////////////////

#IFNDEF __USB_DESCRIPTORS__
#DEFINE __USB_DESCRIPTORS__

#include <usb.h>

//////////////////////////////////////////////////////////////////
///
///   start config descriptor
///   right now we only support one configuration descriptor.
///   the config, interface, class, and endpoint goes into this array.
///
//////////////////////////////////////////////////////////////////

   #DEFINE USB_TOTAL_CONFIG_LEN      32 //config+interface+class+endpoint

   //configuration descriptor
   char const USB_CONFIG_DESC[] = 
   {
   //config_descriptor for config index 1
         USB_DESC_CONFIG_LEN,     //length of descriptor size
         USB_DESC_CONFIG_TYPE,         //constant CONFIGURATION (0x02)
         USB_TOTAL_CONFIG_LEN,0,  //size of all data returned for this config
         1,      //number of interfaces this device supports
         0x01,                //identifier for this configuration.  (IF we had more than one configurations)
         0x00,                //index of string descriptor for this configuration
         0xC0,                //bit 6=1 if self powered, bit 5=1 if supports remote wakeup (we don't), bits 0-4 reserved and bit7=1
         0x32,                //maximum bus power required (maximum milliamperes/2)  (0x32 = 100mA)

   //interface descriptor 0 alt 0
         USB_DESC_INTERFACE_LEN,  //length of descriptor
         USB_DESC_INTERFACE_TYPE,      //constant INTERFACE (0x04)
         0x00,                //number defining this interface (IF we had more than one interface)
         0x00,                //alternate setting
         2,       //number of endpoints, not counting endpoint 0.
         0xFF,                //class code, FF = vendor defined
         0xFF,                //subclass code, FF = vendor
         0xFF,                //protocol code, FF = vendor
         0x00,                //index of string descriptor for interface

   //endpoint descriptor
         USB_DESC_ENDPOINT_LEN, //length of descriptor
         USB_DESC_ENDPOINT_TYPE,     //constant ENDPOINT (0x05)
         0x81,              //endpoint number and direction (0x81 = EP1 IN)
         0x02,              //transfer type supported (0 is control, 1 is iso, 2 is bulk, 3 is interrupt)
         USB_EP1_TX_SIZE,0x00,         //maximum packet size supported
         0x01,              //polling interval in ms. (for interrupt transfers ONLY)

   //endpoint descriptor
         USB_DESC_ENDPOINT_LEN, //length of descriptor
         USB_DESC_ENDPOINT_TYPE,     //constant ENDPOINT (0x05)
         0x01,              //endpoint number and direction (0x01 = EP1 OUT)
         0x02,              //transfer type supported (0 is control, 1 is iso, 2 is bulk, 3 is interrupt)
         USB_EP1_RX_SIZE,0x00,         //maximum packet size supported
         0x01,              //polling interval in ms. (for interrupt transfers ONLY)

  };

   //****** BEGIN CONFIG DESCRIPTOR LOOKUP TABLES ********
   //since we can't make pointers to constants in certain pic16s, this is an offset table to find
   //  a specific descriptor in the above table.

   //NOTE: DO TO A LIMITATION OF THE CCS CODE, ALL HID INTERFACES MUST START AT 0 AND BE SEQUENTIAL
   //      FOR EXAMPLE, IF YOU HAVE 2 HID INTERFACES THEY MUST BE INTERFACE 0 AND INTERFACE 1
   #define USB_NUM_HID_INTERFACES   0

   //the maximum number of interfaces seen on any config
   //for example, if config 1 has 1 interface and config 2 has 2 interfaces you must define this as 2
   #define USB_MAX_NUM_INTERFACES   1

   //define how many interfaces there are per config.  [0] is the first config, etc.
   const char USB_NUM_INTERFACES[USB_NUM_CONFIGURATIONS]={1};

   #if (sizeof(USB_CONFIG_DESC) != USB_TOTAL_CONFIG_LEN)
      #error USB_TOTAL_CONFIG_LEN not defined correctly
   #endif


//////////////////////////////////////////////////////////////////
///
///   start device descriptors
///
//////////////////////////////////////////////////////////////////

   //device descriptor
   char const USB_DEVICE_DESC[] =
   {
         USB_DESC_DEVICE_LEN,          //the length of this report
         0x01,                //constant DEVICE (0x01)
         0x10,0x01,           //usb version in bcd
         0x00,                //class code (if 0, interface defines class.  FF is vendor defined)
         0x00,                //subclass code
         0x00,                //protocol code
         USB_MAX_EP0_PACKET_LENGTH,   //max packet size for endpoint 0. (SLOW SPEED SPECIFIES 8)
         0x77,0x07,           //vendor id (0x0777 is EMNT)
         0x84,0x50,           //product id
         0x02,0x00,           //device release number
         0x01,                //index of string description of manufacturer. therefore we point to string_1 array (see below)
         0x02,                //index of string descriptor of the product
         0x00,                //index of string descriptor of serial number
         USB_NUM_CONFIGURATIONS   //number of possible configurations
   };


//////////////////////////////////////////////////////////////////
///
///   start string descriptors
///   String 0 is a special language string, and must be defined.  People in U.S.A. can leave this alone.
///
///   You must define the length else get_next_string_character() will not see the string
///   Current code only supports 10 strings (0 thru 9)
///
//////////////////////////////////////////////////////////////////

//the offset of the starting location of each string.
//offset[0] is the start of string 0, offset[1] is the start of string 1, etc.
const char USB_STRING_DESC_OFFSET[]={0,4,14};

#define USB_STRING_DESC_COUNT sizeof(USB_STRING_DESC_OFFSET)

char const USB_STRING_DESC[]={
   //string 0
         4, //length of string index
         USB_DESC_STRING_TYPE, //descriptor type 0x03 (STRING)
         0x09,0x04,   //Microsoft Defined for US-English
   //string 1 --> la compañia del producto ???
         10, //length of string index
         USB_DESC_STRING_TYPE, //descriptor type 0x03 (STRING)
         'E',0,
         'M',0,
         'N',0,
       'T',0,
   //string 2 --> nombre del dispositivo
         40, //length of string index
         USB_DESC_STRING_TYPE, //descriptor type 0x03 (STRING)
       'E',0,
         'M',0,
         'N',0,
       'T',0,
       ' ',0,
         'G',0,
         'P',0,
         'S',0,
         '/',0,
         'G',0,
         'S',0,
         'M',0,
         ' ',0,
       'S',0,
       'y',0,
         's',0,
       't',0,
       'e',0,
         'm',0
};


#ENDIF

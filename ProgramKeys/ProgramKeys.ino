 // Version to send to github to share with others.
 
 /**
 * FULL VIDEO SUPPORT: https://www.youtube.com/c/ChristineFarion 
 * @CMoz
 * MAY-2025
 * 
 * Original beautiful code & styling by Volos Projects: https://github.com/VolosR/ProgrammerKeyboard
 * 
 * This code is part of a project that uses the T-Keyboard-S3-Pro hardware
 * and the Arduino framework. It includes functionality for handling
 * keyboard inputs, rotary encoder interactions, and displaying information
 * on a connected LCD screen.
 *
 * CHANGES MADE:
 *
 * 1. The code is structured to handle different option sets, allowing users to
 * switch between different configurations and display relevant information
 * on the LCD screen.
 *
 * 2. Modify the `keyboardText` array to change the text sent
 * by each button in the different option sets.
 *
 * 3. Pressing the rotary encoder is used to cycle through the option sets, and the
 * display updates accordingly. Sets are tracked and option set is displayed.
 *
 * 4. The code includes functionality to show a rainbow effect on the LEDs
 * or set all LEDs to a specific color using HSV values.
 *
 * 5. There are variables at the top of the code for you to customize it easily.
 *
 */

#include <Arduino.h>
#include "T-Keyboard-S3-Pro_Drive.h"
#include "pin_config.h"
#include "Arduino_GFX_Library.h"
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "USBHIDConsumerControl.h"
#include "Arduino_DriveBus_Library.h"
#include "AGENCY.h"
#include "AGENCY12.h"
#include "AGENCY9.h"

#define KEY_HID_R 'r'
#define IIC_MAIN_DEVICE_ADDRESS 0x01 // 主设备的IIC地址，默认为0x01
#define IIC_LCD_CS_DEVICE_DELAY 20   // 选定屏幕等待时间

// for the rotary encoder programming and choices for new screens
int optionSet = 0;             // Tracks which option set is active (0–4)
const int NUM_OPTION_SETS = 5; // Total number of option sets

bool selectionMode = false;
int screenIndex = 0;

int pressed[4] = {0};
int press[4] = {0};
unsigned short bck = 0x00A5;

unsigned short dotsCol[4] = {0xFAEA, 0xFDE5, 0x2647, 0xBA3A};

unsigned long startupTime = 0;
const unsigned long STARTUP_GRACE_PERIOD = 2000; // 2 seconds grace period

// ************************** --> CUSTOMIZE YOUR KEYBOARD TEXT FROM HERE <-- ************************************************* //

// Labels at the TOP, a lighter color - very Small
String lbls3[5][4] = {
    {"CMOZ", "AI", "PROMPT", "HELPER"},
    {"Optn2-1", "Optn2-2", "Optn2-3", "Optn2-4"},
    {"Optn3-1", "Optn3-2", "Optn3-3", "Optn3-4"},
    {"Optn4-1", "Optn4-2", "Optn4-3", "Optn4-4"},
    {"Optn5-1", "Optn5-2", "Optn5-3", "Optn5-4"}};

    uint16_t label3TXTClr = 0x122D;             // <-- 0x122D default text color for these labels

// Main content CENTER area on the OLED screen - Large
String lbls[5][4] = {
    {",tags", "#tags", "smry", "desc"},
    {"Option2-1", "Option2-2", "Option2-3", "Option2-4"},
    {"Option3-1", "Option3-2", "Option3-3", "Option3-4"},
    {"Option4-1", "Option4-2", "Option4-3", "Option4-4"},
    {"Option5-1", "Option5-2", "Option5-3", "Option5-4"}};

    uint16_t labelTXTClr = WHITE;               // <-- WHITE change the Default text color for these labels on the OLED screen

// Content on the BOTTOM area on the OLED screen - Medium
String lbls2[5][4] = {
    {"tags with comma", "hashtags with #", "write a summary", "write description"},
    {"Option2-1", "Option2-2", "Option2-3", "Option2-4"},
    {"Option3-1", "Option3-2", "Option3-3", "Option3-4"},
    {"Option4-1", "Option4-2", "Option4-3", "Option4-4"},
    {"Option5-1", "Option5-2", "Option5-3", "Option5-4"}};

    uint16_t label2TXTClr = 0x445A;             // <-- 0x445A change the Default text color for these labels on the OLED screen

// *** MODIFY text that will be sent by pressing keys
// 2D arrays (5 option sets × 4 buttons)
String keyboardText[5][4] = {
    // Option Set 1
    {
        "Option1 Button1: Custom text for button 1 in option set 1",
        "Option1 Button2: Custom text for button 1 in option set 1",
        "Option1 Button3: Custom text for button 1 in option set 1",
        "Option1 Button4: Custom text for button 1 in option set 1"},
    // Option Set 2
    {
        "Option2 Button1: Custom text for button 1 in option set 2",
        "Option2 Button2: Custom text for button 2 in option set 2",
        "Option2 Button3: Custom text for button 3 in option set 2",
        "Option2 Button4: Custom text for button 4 in option set 2"},
    // Option Set 3
    {
        "Option3 Button1: Custom text for button 1 in option set 3",
        "Option3 Button2: Custom text for button 2 in option set 3",
        "Option3 Button3: Custom text for button 3 in option set 3",
        "Option3 Button4: Custom text for button 4 in option set 3"},
    // Option Set 4
    {
        "Option4 Button1: Custom text for button 1 in option set 4",
        "Option4 Button2: Custom text for button 2 in option set 4",
        "Option4 Button3: Custom text for button 3 in option set 4",
        "Option4 Button4: Custom text for button 4 in option set 4"},
    // Option Set 5
    {
        "Option5 Button1: Custom text for button 1 in option set 5",
        "Option5 Button2: Custom text for button 2 in option set 5",
        "Option5 Button3: Custom text for button 3 in option set 5",
        "Option5 Button4: Custom text for button 4 in option set 5"}};

// ***** --> CUSTOMIZE YOUR OLED / TEXT COLOURS HERE <-- ***** //

uint8_t brightness = 50;                    // <-- change the Default brightness set for RGB LEDs

uint16_t setTxtClr = PINK;                  // <--- "set:x" text colour - top righthand corner of the screen 0x122D
uint16_t pressedTEXTClr = 0xA61C;           // <---- "PRESSED" text colour 0xA61C default
uint16_t numOfPressesTexClr = ORANGE;       // <---- the number of presses text colour ORANGE default

// Background colors for each option set and each screen (5 option sets × 4 screens)
unsigned short screenColors[5][4] = {
  // Option Set 1 (screens 1-4)
  {BLACK, 0x00A5, 0x00A5, 0x00A5},  
  
  // Option Set 2 (screens 1-4)
  {0x2A69, BLACK, 0x2A69, 0x2A69},  
  
  // Option Set 3 (screens 1-4)
  {0x7800, 0x7800, BLACK, 0x7800},  
  
  // Option Set 4 (screens 1-4)
  {0x03E0, 0x03E0, 0x03E0, BLACK},  
  
  // Option Set 5 (screens 1-4)
  {RED, 0x780F, 0x780F, 0x780F}   
};

enum KNOB_State
{
    KNOB_NULL,
    KNOB_INCREMENT,
    KNOB_DECREMENT,
};

int8_t KNOB_Data = 0;
bool KNOB_Trigger_Flag = false;
uint8_t KNOB_State_Flag = KNOB_State::KNOB_NULL;

//  0B000000[KNOB_DATA_A][KNOB_DATA_B]
uint8_t KNOB_Previous_Logical = 0B00000000;
uint8_t IIC_Master_Receive_Data;

size_t IIC_Bus_CycleTime = 0;
size_t KNOB_CycleTime = 0;

uint8_t KEY1_Lock = 0;
uint8_t KEY2_Lock = 0;
uint8_t KEY3_Lock = 0;
uint8_t KEY4_Lock = 0;
uint8_t KEY5_Lock = 0;

std::vector<unsigned char> IIC_Device_ID_Registry_Scan;
std::vector<unsigned char> vector_temp;
std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);
std::vector<T_Keyboard_S3_Pro_Device_KEY> KEY_Trigger;
USBHIDKeyboard Keyboard;
USBHIDConsumerControl ConsumerControl;

/*LCD*/
bool IIC_Device_ID_State = false;
std::vector<unsigned char> IIC_Device_ID_Scan;
std::vector<unsigned char> IIC_Device_ID_Registry;
Arduino_DataBus *bus = new Arduino_HWSPI(
    LCD_DC /* DC */, -1 /* CS */, LCD_SCLK /* SCK */, LCD_MOSI /* MOSI */, -1 /* MISO */);

Arduino_GFX *gfx = new Arduino_GC9107(
    bus, -1 /* RST */, 0 /* rotation */, true /* IPS */,
    LCD_WIDTH /* width */, LCD_HEIGHT /* height */,
    2 /* col offset 1 */, 1 /* row offset 1 */, 0 /* col_offset2 */, 0 /* row_offset2 */);

Arduino_Canvas *sprite = new Arduino_Canvas(128, 128, gfx); // Width: 100, Height: 50

void Iocn_Show(std::vector<unsigned char> device_id, int chosen); // *** IMPORTANT *** Added forward declaration for platformIO

// Function to show a rainbow effect on the LEDs
void showRainbowEffect(uint8_t device_id, uint8_t brightness) {
    // Set LED brightness (0-100)
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, brightness);

    // Set LED to test mode 1 (built-in rainbow effect)
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000011);

    // Wait a moment for the effect to be visible
    delay(2000);

    // Return to normal mode with lower brightness
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 10);
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001);
}

// Function to set all LEDs to a specific HSV color
void setAllLEDsToColor(uint8_t device_id, uint16_t hue, uint8_t saturation, uint8_t brightness) {
    // Set LED to free mode for custom colors
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000010);

    // Set LED brightness
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, brightness);

    // Set hue (0-360)
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_COLOR_HUE_H, (uint8_t)(hue >> 8));
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_COLOR_HUE_L, (uint8_t)hue);

    // Set saturation (0-255)
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_COLOR_STATURATION, saturation);

    // Clear previous LED selection
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B10000000);

    // Select all LEDs
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B00111111);
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_CONTROL_2, 0B11111111);

    // Display the selected LEDs
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B01000000);
}

void KNOB_Logical_Scan_Loop(void) {
    uint8_t KNOB_Logical_Scan = 0B00000000;

    if (digitalRead(KNOB_DATA_A) == 1) {
        KNOB_Logical_Scan |= 0B00000010;
    } else {
        KNOB_Logical_Scan &= 0B11111101;
    }

    if (digitalRead(KNOB_DATA_B) == 1) {
        KNOB_Logical_Scan |= 0B00000001;
    } else {
        KNOB_Logical_Scan &= 0B11111110;
    }

    if (KNOB_Previous_Logical != KNOB_Logical_Scan) {
        if (KNOB_Logical_Scan == 0B00000000 || KNOB_Logical_Scan == 0B00000011)
        {
            KNOB_Previous_Logical = KNOB_Logical_Scan;
            KNOB_Trigger_Flag = true;
        } else {
            if (KNOB_Logical_Scan == 0B00000010) {
                switch (KNOB_Previous_Logical) {
                case 0B00000000:
                    KNOB_State_Flag = KNOB_State::KNOB_INCREMENT;
                    break;
                case 0B00000011:
                    KNOB_State_Flag = KNOB_State::KNOB_DECREMENT;
                    break;

                default:
                    break;
                }
            }
            if (KNOB_Logical_Scan == 0B00000001) {
                switch (KNOB_Previous_Logical) {
                case 0B00000000:
                    KNOB_State_Flag = KNOB_State::KNOB_DECREMENT;
                    break;
                case 0B00000011:
                    KNOB_State_Flag = KNOB_State::KNOB_INCREMENT;
                    break;

                default:
                    break;
                }
            }
        }
        delay(10);
        Serial.print("A: ");
        Serial.print(digitalRead(KNOB_DATA_A));
        Serial.print(" B: ");
        Serial.println(digitalRead(KNOB_DATA_B));
        Serial.print("KNOB_State_Flag: ");
        Serial.println(KNOB_State_Flag);
    }
}

void IIC_KEY_Read_Loop(void) {
    if (IIC_Device_ID_Registry_Scan.size() > 0)  {

        for (int i = 0; i < IIC_Device_ID_Registry_Scan.size(); i++) {
            IIC_Bus->IIC_ReadC8_Data(IIC_Device_ID_Registry_Scan[i], T_KEYBOARD_S3_PRO_RD_KEY_TRIGGER,
                                     &IIC_Master_Receive_Data, 1);

            T_Keyboard_S3_Pro_Device_KEY key_trigger_temp;
            key_trigger_temp.ID = IIC_Device_ID_Registry_Scan[i];
            key_trigger_temp.Trigger_Data = IIC_Master_Receive_Data;

            KEY_Trigger.push_back(key_trigger_temp);
        }
    }
}

void IIC_KEY_Trigger_Loop(void) {
    if (millis() - startupTime < STARTUP_GRACE_PERIOD) {
        // During grace period, just clear any key triggers without processing them
        while (KEY_Trigger.size() > 0) {
            KEY_Trigger.erase(KEY_Trigger.begin());
        }
        return; // Skip the rest of the function during grace period
    }

    if (KEY_Trigger.size() > 0) {
        switch (KEY_Trigger[0].ID) {
        case 0x01:
            if (((KEY_Trigger[0].Trigger_Data & 0B00010000) >> 4) == 1) {
                if (KEY1_Lock == 0) {
                    pressed[0]++;
                    press[0]++;
                    if (press[0] == 8)
                        press[0] = 0;
                    Iocn_Show(vector_temp, 0);

                    // Show test pattern 2 (another built-in effect)
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 50);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000100);
                    delay(500);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 10);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001);

                    // Use the current option set to determine which text to send
                    Keyboard.sendString(keyboardText[optionSet][0].c_str());
                    delay(50); // Short stability delay
                }
                KEY1_Lock = 1;
            } else {
                KEY1_Lock = 0; // loop auto release
            }

            if (((KEY_Trigger[0].Trigger_Data & 0B00001000) >> 3) == 1) {
                if (KEY2_Lock == 0) {
                    pressed[1]++;
                    press[1]++;
                    if (press[1] == 8)
                        press[1] = 0;
                    Iocn_Show(vector_temp, 1);

                    // Show test pattern 2 (another built-in effect)
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 50);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000100);
                    delay(500);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 10);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001);

                    // Use the current option set to determine which text to send
                    Keyboard.sendString(keyboardText[optionSet][1].c_str());
                    delay(50); // Short stability delay
                }
                KEY2_Lock = 1;
            } else {
                KEY2_Lock = 0;
            }

            if (((KEY_Trigger[0].Trigger_Data & 0B00000100) >> 2) == 1) {
                if (KEY3_Lock == 0) {
                    pressed[2]++;
                    press[2]++;
                    if (press[2] == 8)
                        press[2] = 0;
                    Iocn_Show(vector_temp, 2);

                    // Show test pattern 2 (another built-in effect)
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 50);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000100);
                    delay(500);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 10);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001);

                    // Use the current option set to determine which text to send
                    Keyboard.sendString(keyboardText[optionSet][2].c_str());
                    delay(50); // Short stability delay
                }

                KEY3_Lock = 1;
            } else {
                KEY3_Lock = 0;
            }

            if (((KEY_Trigger[0].Trigger_Data & 0B00000010) >> 1) == 1) {
                if (KEY4_Lock == 0) {
                    pressed[3]++;
                    press[3]++;
                    if (press[3] == 8)
                        press[3] = 0;
                    Iocn_Show(vector_temp, 3);

                    // Show test pattern 2 (another built-in effect)
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 50);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000100);
                    delay(500);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 10);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001);

                    // Use the current option set to determine which text to send
                    Keyboard.sendString(keyboardText[optionSet][3].c_str());
                    delay(50); // Short stability delay
                }
                KEY4_Lock = 1;
            } else {
                KEY4_Lock = 0;
            }

            if ((KEY_Trigger[0].Trigger_Data & 0B00000001) == 1) {
                if (KEY5_Lock == 0) {
                    // Rotary encoder button press - cycle through option sets
                    optionSet = (optionSet + 1) % NUM_OPTION_SETS;
                    Serial.printf("Option Set: %d\n", optionSet + 1);

                    // Show rainbow effect when KEY5 (rotary encoder) is pressed
                    // showRainbowEffect(IIC_MAIN_DEVICE_ADDRESS, 50);

                    // Show test pattern 2 (another built-in effect)
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 50);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000100);
                    delay(500);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 10);
                    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001);

                    Iocn_Show(vector_temp, 5);
                }
                KEY5_Lock = 1;
            } else {
                KEY5_Lock = 0;
            }

            break;

        default:
            Keyboard.release(KEY1_Lock);
            Keyboard.release(KEY2_Lock);
            Keyboard.release(KEY3_Lock);
            Keyboard.release(KEY4_Lock);
            break;
        }
        KEY_Trigger.erase(KEY_Trigger.begin());
    }
}

void KNOB_Trigger_Loop(void) {
    KNOB_Logical_Scan_Loop();

    if (KNOB_Trigger_Flag == true) {
        KNOB_Trigger_Flag = false;

        switch (KNOB_State_Flag) {
        case KNOB_State::KNOB_INCREMENT:
            KNOB_Data++;
            Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);
            ConsumerControl.press(CONSUMER_CONTROL_VOLUME_INCREMENT);
            ConsumerControl.release();
            delay(5);
            break;
        case KNOB_State::KNOB_DECREMENT:
            KNOB_Data--;
            Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);
            ConsumerControl.press(CONSUMER_CONTROL_VOLUME_DECREMENT);
            ConsumerControl.release();
            delay(5);
            break;

        default:
            break;
        }
    } else {
        ConsumerControl.release();
    }
}

void Task1(void *pvParameters) {

    while (1) {
        IIC_Bus->IIC_Device_7Bit_Scan(&IIC_Device_ID_Registry_Scan);
        if (millis() > IIC_Bus_CycleTime) {
            if (IIC_Bus->IIC_Device_7Bit_Scan(&IIC_Device_ID_Scan) == true) {
                if (IIC_Device_ID_Scan.size() != IIC_Device_ID_Registry.size()) {
                    IIC_Device_ID_State = true;
                }
            }
            IIC_Bus_CycleTime = millis() + 10; // 10ms
        }
        delay(1);
    }
}

void Select_Screen_All(std::vector<unsigned char> device_id, bool select) {
    if (select == true) {
        for (int i = 0; i < device_id.size(); i++) {
            IIC_Bus->IIC_WriteC8D8(device_id[i],
                                   T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
            delay(IIC_LCD_CS_DEVICE_DELAY);
        }
    } else {
        for (int i = 0; i < device_id.size(); i++) {
            IIC_Bus->IIC_WriteC8D8(device_id[i],
                                   T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
            delay(IIC_LCD_CS_DEVICE_DELAY);
        }
    }
}

void Iocn_Show(std::vector<unsigned char> device_id, int chosen) {
    Select_Screen_All(device_id, true);

    // variable chosen dictate which display will be updated if chosen =0 first display will be updated
    //  if chosen =2 second one will be updated, if chosen == 5 , all display will be updated

    if (chosen == 5)
        gfx->fillScreen(BLACK);

    // UPDATING ONLY FIRST DISPLAY
    if (chosen == 0 || chosen == 5) {
        for (int i = 0; i < device_id.size(); i++) {
            IIC_Bus->IIC_WriteC8D8(device_id[i],
                                   T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00010000); // 选定屏幕1 LCD_1
            delay(IIC_LCD_CS_DEVICE_DELAY);
        }
        
        sprite->fillScreen(screenColors[optionSet][0]);
        
        for (int i = 0; i < 3; i++)
            sprite->fillCircle(18 + (i * 12), 9, 4, dotsCol[i]);
        
        // for keeping track of the set
        sprite->setCursor(85, 6);
        sprite->setTextColor(setTxtClr); 
        sprite->print(" Set:");
        sprite->println(optionSet + 1);

        sprite->setFont(&AGENCYR30pt7b);
        sprite->setTextColor(labelTXTClr);
        sprite->setCursor(1, 92);
        sprite->println(lbls[optionSet][0]); // sprite->println(lbls3[optionSet][3]); // optionSet: 0-4, 3: fourth display

        sprite->setFont(&AGENCYR12pt7b);
        sprite->setTextColor(pressedTEXTClr);
        sprite->setCursor(12, 38);
        sprite->println("PRESSED: ");

        sprite->setTextColor(numOfPressesTexClr);
        sprite->setCursor(86, 38);
        sprite->println(String(pressed[0]));

        sprite->setFont(&AGENCYR10pt7b);
        sprite->setTextColor(label2TXTClr);
        sprite->setCursor(12, 122);
        sprite->println(lbls2[optionSet][0]);

        // sprite->setFont(NULL);
        sprite->setFont((const GFXfont *)nullptr);

        sprite->setCursor(50, 6);
        sprite->setTextColor(label3TXTClr); 
        sprite->println(lbls3[optionSet][0]);

        for (int i = 0; i < 8; i++)
            if (i <= press[0])
                sprite->fillRect(118, 94 - (i * 6), 6, 3, dotsCol[0]);
            else
                sprite->fillRect(118, 94 - (i * 6), 6, 3, 0x122D);

        sprite->flush();
        delay(5);
    }

    // UPDATING SECOND DISPLAY
    if (chosen == 1 || chosen == 5) {
        for (int i = 0; i < device_id.size(); i++) {
            IIC_Bus->IIC_WriteC8D8(device_id[i],
                                   T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00001000); // 选定屏幕2 LCD_2
            delay(IIC_LCD_CS_DEVICE_DELAY);
        }
        
        sprite->fillScreen(screenColors[optionSet][1]);

        for (int i = 0; i < 3; i++)
            sprite->fillCircle(18 + (i * 12), 9, 4, dotsCol[i]);

        sprite->setFont(&AGENCYR30pt7b);
        sprite->setTextColor(labelTXTClr);
        sprite->setCursor(1, 92);
        sprite->println(lbls[optionSet][1]);

        sprite->setFont(&AGENCYR12pt7b);
        sprite->setTextColor(pressedTEXTClr);
        sprite->setCursor(12, 38);
        sprite->println("PRESSED: ");

        sprite->setTextColor(numOfPressesTexClr);
        sprite->setCursor(86, 38);
        sprite->println(String(pressed[1]));

        sprite->setFont(&AGENCYR10pt7b);
        sprite->setTextColor(label2TXTClr);
        sprite->setCursor(12, 122);
        sprite->println(lbls2[optionSet][1]);

        // sprite->setFont(NULL);
        sprite->setFont((const GFXfont *)nullptr);

        sprite->setCursor(50, 6);
        sprite->setTextColor(label3TXTClr);
        sprite->println(lbls3[optionSet][1]);

        for (int i = 0; i < 8; i++)
            if (i <= press[1])
                sprite->fillRect(118, 94 - (i * 6), 6, 3, dotsCol[1]);
            else
                sprite->fillRect(118, 94 - (i * 6), 6, 3, 0x122D);

        sprite->flush();
        delay(5);
    }

    // UPDATING THIRD DISPLAY
    if (chosen == 2 || chosen == 5) {
        for (int i = 0; i < device_id.size(); i++) {
            IIC_Bus->IIC_WriteC8D8(device_id[i],
                                   T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000100); // 选定屏幕3 LCD_3
            delay(IIC_LCD_CS_DEVICE_DELAY);
        }
        
        sprite->fillScreen(screenColors[optionSet][2]);

        for (int i = 0; i < 3; i++)
            sprite->fillCircle(18 + (i * 12), 9, 4, dotsCol[i]);

        sprite->setFont(&AGENCYR30pt7b);
        sprite->setTextColor(labelTXTClr);
        sprite->setCursor(1, 92);
        sprite->println(lbls[optionSet][2]);

        sprite->setFont(&AGENCYR12pt7b);
        sprite->setTextColor(pressedTEXTClr);
        sprite->setCursor(12, 38);
        sprite->println("PRESSED: ");

        sprite->setTextColor(numOfPressesTexClr);
        sprite->setCursor(86, 38);
        sprite->println(String(pressed[2]));

        sprite->setFont(&AGENCYR10pt7b);
        sprite->setTextColor(label2TXTClr);
        sprite->setCursor(12, 122);
        sprite->println(lbls2[optionSet][2]);

        // sprite->setFont(NULL);
        sprite->setFont((const GFXfont *)nullptr);

        sprite->setCursor(50, 6);
        sprite->setTextColor(label3TXTClr);
        sprite->println(lbls3[optionSet][2]);

        for (int i = 0; i < 8; i++)
            if (i <= press[2])
                sprite->fillRect(118, 94 - (i * 6), 6, 3, dotsCol[2]);
            else
                sprite->fillRect(118, 94 - (i * 6), 6, 3, 0x122D);

        sprite->flush();
        delay(5);
    }

    // UPDATING LAST DISPLAY
    if (chosen == 3 || chosen == 5) {
        for (int i = 0; i < device_id.size(); i++) {
            IIC_Bus->IIC_WriteC8D8(device_id[i],
                                   T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000010); // 选定屏幕4 LCD_4
            delay(IIC_LCD_CS_DEVICE_DELAY);
        }

        sprite->fillScreen(screenColors[optionSet][3]);

        for (int i = 0; i < 3; i++)
            sprite->fillCircle(18 + (i * 12), 9, 4, dotsCol[i]);

        sprite->setFont(&AGENCYR30pt7b);
        sprite->setTextColor(labelTXTClr);
        sprite->setCursor(1, 92);
        sprite->println(lbls[optionSet][3]);

        sprite->setFont(&AGENCYR12pt7b);
        sprite->setTextColor(pressedTEXTClr);
        sprite->setCursor(12, 38);
        sprite->println("PRESSED: ");

        sprite->setTextColor(numOfPressesTexClr);
        sprite->setCursor(86, 38);
        sprite->println(String(pressed[3]));

        sprite->setFont(&AGENCYR10pt7b);
        sprite->setTextColor(label2TXTClr);
        sprite->setCursor(12, 122);
        sprite->println(lbls2[optionSet][3]);

        // sprite->setFont(NULL);
        sprite->setFont((const GFXfont *)nullptr);

        sprite->setCursor(50, 6);
        sprite->setTextColor(label3TXTClr);
        sprite->println(lbls3[optionSet][3]);

        for (int i = 0; i < 8; i++)
            if (i <= press[3])
                sprite->fillRect(118, 94 - (i * 6), 6, 3, dotsCol[3]);
            else
                sprite->fillRect(118, 94 - (i * 6), 6, 3, 0x122D);

        sprite->flush();
        delay(5);
    }
    Select_Screen_All(device_id, false);
}

void LCD_Initialization(std::vector<unsigned char> device_id)
{
    Select_Screen_All(device_id, true);
    gfx->begin(4500000);
    gfx->setTextWrap(false);
    gfx->fillScreen(BLACK);
    delay(500);

    Select_Screen_All(device_id, false);
}

void Print_IIC_Info(std::vector<unsigned char> device_id)
{
    Select_Screen_All(device_id, true);
    gfx->fillScreen(WHITE);
    gfx->setCursor(15, 30);
    gfx->setTextColor(PURPLE);
    gfx->printf("IIC Info");
    Select_Screen_All(device_id, false);
}

void resetLEDs(uint8_t device_id)
{
    // Set LEDs to normal mode with low brightness
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 10);
    IIC_Bus->IIC_WriteC8D8(device_id, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("T-Keyboard-S3-Pro Drive Test Start");
    pinMode(KNOB_DATA_A, INPUT_PULLUP);
    pinMode(KNOB_DATA_B, INPUT_PULLUP);

    while (IIC_Bus->begin() == false) {
        Serial.println("IIC_Bus initialization fail");
        delay(2000);
    }
    Serial.println("IIC_Bus initialization successfully");
    xTaskCreatePinnedToCore(Task1, "Task1", 10000, NULL, 1, NULL, 1);
    delay(100);
    // Initialize LEDs with a low brightness
    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 10);
    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001);

    Keyboard.begin();
    ConsumerControl.begin();
    USB.begin();
    /*LCD*/
    pinMode(LCD_RST, OUTPUT);
    digitalWrite(LCD_RST, HIGH);
    delay(100);
    digitalWrite(LCD_RST, LOW);
    delay(100);
    digitalWrite(LCD_RST, HIGH);
    delay(100);
    analogWrite(LCD_BL, 150);

    while (1) {
        bool temp = false;
        if (IIC_Device_ID_State == true)
        {
            delay(100);

            Serial.printf("Find IIC ID: %#X\n", IIC_Device_ID_Scan[0]);

            if (IIC_Device_ID_Scan[0] == IIC_MAIN_DEVICE_ADDRESS)
            { // 只初始化主设备，其他设备稍后测试

                IIC_Device_ID_Registry.push_back(IIC_Device_ID_Scan[0]);

                std::vector<unsigned char> vector_temp;
                vector_temp.push_back(IIC_MAIN_DEVICE_ADDRESS);

                LCD_Initialization(vector_temp);

                temp = true;
            }
        }
        else
        {
            temp = false;
        }

        if (temp == true)
        {
            Serial.println("IIC_Bus select LCD_CS successful");
            break;
        }
        else
        {
            Serial.println("IIC ID not found");
            delay(1000);
        }
    }

    if (IIC_Bus->IIC_ReadC8_Delay_Data(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_RD_DRIVE_FIRMWARE_VERSION, 20, &IIC_Master_Receive_Data, 1) == true)
    {
        Serial.printf("STM32 dirve firmware version: %#X \n", IIC_Master_Receive_Data);
    }
    sprite->begin();
    vector_temp.push_back(IIC_MAIN_DEVICE_ADDRESS);
    Print_IIC_Info(vector_temp);

    // Reset optionSet to ensure it starts at 0
    optionSet = 0;

    // Clear any pending key triggers that might have accumulated during startup
    while (KEY_Trigger.size() > 0) {
        KEY_Trigger.erase(KEY_Trigger.begin());
    }

    // Reset all key locks
    KEY1_Lock = 0;
    KEY2_Lock = 0;
    KEY3_Lock = 0;
    KEY4_Lock = 0;
    KEY5_Lock = 0;

    // Add a delay to stabilize
    delay(500);

    // Record the current time for startup grace period
    startupTime = millis();

    // Update the display with the correct initial option set
    Iocn_Show(vector_temp, 5);
}

void loop() {
    IIC_KEY_Read_Loop();
    IIC_KEY_Trigger_Loop();
    if (millis() > KNOB_CycleTime)
    {
        KNOB_Logical_Scan_Loop();
        KNOB_CycleTime = millis() + 20; // 20ms
    }
    KNOB_Trigger_Loop();
    delay(50);
}

/*
// Basic colors
BLACK       // 0x0000
NAVY        // 0x000F
DARKGREEN   // 0x03E0
DARKCYAN    // 0x03EF
MAROON      // 0x7800
PURPLE      // 0x780F
OLIVE       // 0x7BE0
LIGHTGREY   // 0xC618
DARKGREY    // 0x7BEF
BLUE        // 0x001F
GREEN       // 0x07E0
CYAN        // 0x07FF
RED         // 0xF800
MAGENTA     // 0xF81F
YELLOW      // 0xFFE0
WHITE       // 0xFFFF
ORANGE      // 0xFD20
GREENYELLOW // 0xAFE5
PINK        // 0xF81F
To creat a custom color, you can use the following formula:
// color = (red << 11) | (green << 5) | blue;
// where red, green, and blue are 5-bit values (0-31).
// Some custom colors
0x00A5  // Dark blue/teal
0xA61C  // Reddish
0x445A  // Grayish blue
0x122D  // Dark blue
0xFAEA  // Light pink/peach
0xFDE5  // Light pink
0x2647  // Dark blue
0xBA3A  // Reddish orange
*/

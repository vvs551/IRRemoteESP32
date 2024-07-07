#ifndef IRREMOTE_H
#define IRREMOTE_H

#include "Arduino.h"
#include <vector>

class IRRemote {
public:
    IRRemote(uint8_t irPin = 10);
    ~IRRemote();
    int checkRemote();

private:
    static void IRAM_ATTR isr_IR();
    static void error(uint32_t intval_l, uint32_t intval_h, uint8_t pulsecounter);
    static void setIRresult(uint8_t userCode_a, uint8_t userCode_b, uint8_t dataCode_a, uint8_t dataCode_b);
    static void rcCounter(uint8_t rc);
    static int getButton(uint16_t ir_cmd);

    static int16_t ir_cmd_a;
    static int16_t ir_cmd_b;
    static int16_t ir_adr_a;
    static int16_t ir_adr_b;
    static uint8_t ir_rc;
    static uint8_t ir_addressCode;
    static uint8_t g_ir_pin;
    static uint32_t ir_intval_l;
    static uint32_t ir_intval_h;
    static int16_t ir_pulsecounter;
    static  boolean  m_f_error;

    uint32_t m_t0;
    uint32_t m_ir_num;
    int8_t   m_ir_pin;
    uint8_t  m_ir_resultstr[10];
    int8_t   m_key;

    static const int ir_button[20];
};

#endif // IRREMOTE_H

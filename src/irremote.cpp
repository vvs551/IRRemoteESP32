#include "IRRemote.h"

// Static member initialization
DRAM_ATTR int16_t IRRemote::ir_cmd_a = -1;
DRAM_ATTR int16_t IRRemote::ir_cmd_b = -1;
DRAM_ATTR int16_t IRRemote::ir_adr_a = 0;
DRAM_ATTR int16_t IRRemote::ir_adr_b = 0;
uint8_t IRRemote::ir_rc = 0;
uint8_t IRRemote::ir_addressCode = 0;
uint8_t IRRemote::g_ir_pin = 0;
uint32_t IRRemote::ir_intval_l = 0;
uint32_t IRRemote::ir_intval_h = 0;
int16_t IRRemote::ir_pulsecounter = 0;
bool IRRemote::m_f_error = false;

const int IRRemote::ir_button[20] = {0x19, 0x45, 0x46, 0x47, 0x44, 0x40, 0x43, 0x07, 0x15, 0x09, 0x16, 0x0d, 0x1c, 0x18, 0x5a, 0x52, 0x08, 0x00, 0x00, 0x00};

IRRemote::IRRemote(uint8_t irPin) : m_ir_pin(irPin), m_t0(0), m_ir_num(0), m_key(-1) {
    memset(m_ir_resultstr, 0, sizeof(m_ir_resultstr));

    Serial.begin(115200);

    ir_adr_b = -1;
    m_t0 = 0;
    ir_cmd_b = -1;
    m_f_error = false;

    if (m_ir_pin >= 0) {
        g_ir_pin = m_ir_pin;
        pinMode(m_ir_pin, INPUT_PULLUP);
        attachInterrupt(m_ir_pin, IRRemote::isr_IR, CHANGE);
    }
}

IRRemote::~IRRemote() {
    if (m_ir_pin >= 0) {
        detachInterrupt(m_ir_pin);
    }
}

int IRRemote::checkRemote() {
    int button = -1;
    if (ir_cmd_a != -01) {
        if (ir_cmd_a + ir_cmd_b == 0xFF) {
            button = getButton(ir_cmd_a);
            ir_cmd_a = -01;
        }
    }
    return button;
}

void IRAM_ATTR IRRemote::isr_IR() {
    uint16_t userCode = 0;
    uint16_t dataCode = 0;
    int32_t t1=0, intval_h=0, intval_l = 0;

    static uint8_t pulsecounter=0;
    static uint32_t t0 = 0;
    static uint32_t ir_value=0;
    static uint64_t bit = 0x00000001;

    static boolean f_AGC = 0;
    static boolean f_LP = 0;
    static boolean f_BURST  = 0;
    static boolean f_P = 0;
    static boolean f_RC = false;
    static uint8_t RC_cnt = 0;

    t1 = micros();
    if(!digitalRead(g_ir_pin)) intval_h = t1 - t0;
    else                       intval_l = t1 - t0;
    t0 = t1;

    if(intval_l >= 8000 && intval_l < 10000) {
        f_AGC = true;
        f_LP = false;
        return;
    }

    if(f_AGC && !f_LP) {
        if((intval_h >= 3500)&&(intval_h <= 5500)) {
            f_LP = true;
            f_AGC = false;
            pulsecounter = 0;
            ir_value = 0;
            bit = 0x00000001;
            RC_cnt = 0;
            return;
        }
    }

    if(f_LP) {
        if((intval_h > 400) && (intval_h < 750)) {
            if(f_BURST) {
                f_BURST = false;
                bit <<= 1;
                pulsecounter++;
                return;
            }
        }

        if((intval_h > 1500) && (intval_h < 1700)) {
            if(f_BURST) {
                f_BURST = false;
                ir_value += bit;
                bit <<= 1;
                pulsecounter++;
            }
            return;
        }

        if((intval_l > 400) && (intval_l < 750)) {
            f_BURST = true;
            if(pulsecounter < 32) return;

            userCode =  ir_value & 0x0000FFFF;
            dataCode = (ir_value & 0xFFFF0000) >> 16;
            uint8_t a, b, c, d;
            a = (userCode & 0x00FF);
            b = (userCode & 0xFF00) >> 8;
            d = (dataCode & 0xFF00) >> 8;
            c = (dataCode & 0x00FF);
            setIRresult(a, b, c, d);

            pulsecounter++;
            f_LP = false;
            return;
        }
    }

    if(f_AGC) {
        if((intval_h > 1700) && (intval_h < 2800)) {
            f_P = true;
            return;
        }
        if(f_P) {
            if((intval_l > 400) && (intval_l < 750)) {
                f_AGC = false;
                f_RC = true;
            }
        }
    }
    if(f_RC) {
        f_RC = false;
        RC_cnt++;
        rcCounter(RC_cnt);
        return;
    }

    if(intval_l) error(intval_l, intval_h, pulsecounter);
}

void IRRemote::error(uint32_t intval_l, uint32_t intval_h, uint8_t pulsecounter) {
    ir_intval_l = intval_l;
    ir_intval_h = intval_h;
    ir_pulsecounter = pulsecounter;
    m_f_error = true;
}

void IRRemote::setIRresult(uint8_t userCode_a, uint8_t userCode_b, uint8_t dataCode_a, uint8_t dataCode_b) {
    ir_cmd_a = dataCode_a;
    ir_cmd_b = dataCode_b;
    ir_adr_a = userCode_a;
    ir_adr_b = userCode_b;
}

void IRRemote::rcCounter(uint8_t rc) {
    ir_rc = rc;
}

int IRRemote::getButton(uint16_t ir_cmd) {
    int n = sizeof(ir_button) / sizeof(ir_button[0]);
    int targetIndex = 0;
    for (auto b : ir_button){
      if(b == ir_cmd)
        break;
      targetIndex++;
    }
    return (targetIndex >= n) ? -1 : targetIndex;
}

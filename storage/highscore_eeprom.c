#include <mega128.h>
#include "../config/board_config.h"
#include "highscore_eeprom.h"

/* Two 13-byte slots; no compiler-dependent struct layout.
   [0..1] magic, [2] version, [3..4] sequence LE,
   [5..10] three scores LE, [11] CRC-8, [12] commit marker. */
#define HS_RECORD_SIZE 13u
#define HS_DATA_SIZE 12u
#define HS_MAGIC_0 0x53u
#define HS_MAGIC_1 0x47u
#define HS_VERSION 1u
#define HS_COMMIT 0xA5u

eeprom unsigned char hs_slot_a[HS_RECORD_SIZE];
eeprom unsigned char hs_slot_b[HS_RECORD_SIZE];

static unsigned int best_scores[3]={0,0,0};
static unsigned int sequence=0;
static unsigned char active_slot=2; /* 2 means no valid record. */

static unsigned char eeprom_byte(unsigned char slot, unsigned char index)
{
    if (slot==0) return hs_slot_a[index];
    return hs_slot_b[index];
}

static void eeprom_put(unsigned char slot, unsigned char index,
                       unsigned char value)
{
    if (slot==0) hs_slot_a[index]=value;
    else hs_slot_b[index]=value;
}

static unsigned char crc8_add(unsigned char crc, unsigned char value)
{
    unsigned char bit_index;
    crc ^= value;
    for (bit_index=0; bit_index<8; ++bit_index) {
        if (crc&0x80u) crc=(unsigned char)((crc<<1)^0x07u);
        else crc=(unsigned char)(crc<<1);
    }
    return crc;
}

static unsigned char read_record(unsigned char slot, unsigned int *scores,
                                 unsigned int *record_sequence)
{
    unsigned char i,crc=0;
    unsigned int value;
    if (eeprom_byte(slot,12)!=HS_COMMIT) return 0;
    if (eeprom_byte(slot,0)!=HS_MAGIC_0 ||
        eeprom_byte(slot,1)!=HS_MAGIC_1 ||
        eeprom_byte(slot,2)!=HS_VERSION) return 0;
    for (i=0; i<11; ++i) crc=crc8_add(crc,eeprom_byte(slot,i));
    if (crc!=eeprom_byte(slot,11)) return 0;
    *record_sequence=(unsigned int)eeprom_byte(slot,3) |
                     ((unsigned int)eeprom_byte(slot,4)<<8);
    for (i=0; i<3; ++i) {
        value=(unsigned int)eeprom_byte(slot,(unsigned char)(5+2*i)) |
              ((unsigned int)eeprom_byte(slot,(unsigned char)(6+2*i))<<8);
        if (value>GAME_MAX_SCORE || value%10u!=0) return 0;
        scores[i]=value;
    }
    return 1;
}

void highscore_init(void)
{
    unsigned int a[3],b[3],seq_a=0,seq_b=0;
    unsigned char valid_a,valid_b,i;
    valid_a=read_record(0,a,&seq_a);
    valid_b=read_record(1,b,&seq_b);
    if (valid_a && (!valid_b ||
        ((unsigned int)(seq_b-seq_a)&0xffffu)>=0x8000u || seq_a==seq_b)) {
        for (i=0; i<3; ++i) best_scores[i]=a[i];
        sequence=seq_a;
        active_slot=0;
    } else if (valid_b) {
        for (i=0; i<3; ++i) best_scores[i]=b[i];
        sequence=seq_b;
        active_slot=1;
    } else {
        for (i=0; i<3; ++i) best_scores[i]=0;
        sequence=0;
        active_slot=2;
    }
}

unsigned int highscore_get(GameLevel level)
{
    if (level>GAME_LEVEL_HARD) return 0;
    return best_scores[(unsigned char)level];
}

HighscoreSaveResult highscore_record_result(GameLevel level,
                                            unsigned int score)
{
    unsigned char data[HS_DATA_SIZE];
    unsigned char slot,i,crc=0;
    unsigned int next_sequence,verify_scores[3],verify_sequence,old_score;
    if (level>GAME_LEVEL_HARD || score>GAME_MAX_SCORE ||
        score%10u!=0) return HIGHSCORE_SAVE_FAILED;
    if (score<=best_scores[(unsigned char)level]) return HIGHSCORE_UNCHANGED;

    old_score=best_scores[(unsigned char)level];
    best_scores[(unsigned char)level]=score;
    next_sequence=(sequence==65535u) ? 0u : (unsigned int)(sequence+1u);
    slot=(active_slot==0) ? 1 : 0;
    data[0]=HS_MAGIC_0;
    data[1]=HS_MAGIC_1;
    data[2]=HS_VERSION;
    data[3]=(unsigned char)next_sequence;
    data[4]=(unsigned char)(next_sequence>>8);
    for (i=0; i<3; ++i) {
        data[5+2*i]=(unsigned char)best_scores[i];
        data[6+2*i]=(unsigned char)(best_scores[i]>>8);
    }
    for (i=0; i<11; ++i) crc=crc8_add(crc,data[i]);
    data[11]=crc;

    /* Preserve the other valid slot until the new record is committed. */
    eeprom_put(slot,12,0);
    for (i=0; i<HS_DATA_SIZE; ++i) eeprom_put(slot,i,data[i]);
    eeprom_put(slot,12,HS_COMMIT);
    if (!read_record(slot,verify_scores,&verify_sequence) ||
        verify_sequence!=next_sequence) {
        best_scores[(unsigned char)level]=old_score;
        return HIGHSCORE_SAVE_FAILED;
    }
    for (i=0; i<3; ++i)
        if (verify_scores[i]!=best_scores[i]) {
            best_scores[(unsigned char)level]=old_score;
            return HIGHSCORE_SAVE_FAILED;
        }
    active_slot=slot;
    sequence=next_sequence;
    return HIGHSCORE_SAVED;
}

#include <mega128.h>
#include "../config/board_config.h"
#include "highscore_eeprom.h"

/* Keep the original v1 declarations and order. Verify their actual addresses
   in the CodeVisionAVR memory map before programming an existing EEPROM. */
#define V1_SIZE 13u
#define V2_SIZE 25u
#define MAGIC0 0x53u
#define MAGIC1 0x47u
#define COMMIT 0xA5u
eeprom unsigned char hs_slot_a[V1_SIZE];
eeprom unsigned char hs_slot_b[V1_SIZE];
#ifdef HOST_TEST
eeprom unsigned char hs_v2_a[V2_SIZE];
eeprom unsigned char hs_v2_b[V2_SIZE];
#else
eeprom unsigned char hs_v2_a[V2_SIZE] @0x0100;
eeprom unsigned char hs_v2_b[V2_SIZE] @0x0120;
#endif

static unsigned int best_scores[9];
static unsigned int sequence;
static unsigned char active_slot=2;

static unsigned char read_byte(unsigned char version,unsigned char slot,unsigned char index)
{
    if (version==1u) return slot==0u ? hs_slot_a[index] : hs_slot_b[index];
    return slot==0u ? hs_v2_a[index] : hs_v2_b[index];
}

static void write_v2(unsigned char slot,unsigned char index,unsigned char value)
{
    if (slot==0u) hs_v2_a[index]=value;
    else hs_v2_b[index]=value;
}

static unsigned char crc_add(unsigned char crc,unsigned char value)
{
    unsigned char bit_index;
    crc^=value;
    for (bit_index=0;bit_index<8u;++bit_index)
        crc=(unsigned char)((crc<<1)^((crc&0x80u) ? 0x07u : 0u));
    return crc;
}

static unsigned char read_record(unsigned char version,unsigned char slot,
                                 unsigned int *scores,unsigned int *seq)
{
    unsigned char i,crc=0,count=(version==1u) ? 3u : 9u;
    unsigned char crc_pos=(version==1u) ? 11u : 23u;
    unsigned int value,limit;
    if (read_byte(version,slot,(unsigned char)(crc_pos+1u))!=COMMIT ||
        read_byte(version,slot,0)!=MAGIC0 ||
        read_byte(version,slot,1)!=MAGIC1 ||
        read_byte(version,slot,2)!=version) return 0;
    for (i=0;i<crc_pos;++i) crc=crc_add(crc,read_byte(version,slot,i));
    if (crc!=read_byte(version,slot,crc_pos)) return 0;
    *seq=(unsigned int)read_byte(version,slot,3) |
         ((unsigned int)read_byte(version,slot,4)<<8);
    for (i=0;i<count;++i) {
        value=(unsigned int)read_byte(version,slot,(unsigned char)(5u+2u*i)) |
              ((unsigned int)read_byte(version,slot,(unsigned char)(6u+2u*i))<<8);
        limit=(version==1u) ? GAME_MAX_SCORE :
              (unsigned int)(game_map_free_cells_for((MapId)(i%3u))-3u)*10u;
        if (value>limit || value%10u!=0u) return 0;
        scores[i]=value;
    }
    return 1;
}

void highscore_init(void)
{
    unsigned int a[9],b[9],sa=0,sb=0;
    unsigned char va,vb,i,version;
    for (i=0;i<9u;++i) best_scores[i]=0;
    sequence=0; active_slot=2;
    for (version=2u;version>0u;--version) {
        va=read_record(version,0,a,&sa);
        vb=read_record(version,1,b,&sb);
        if (!va && !vb) continue;
        if (va && (!vb || ((unsigned int)(sa-sb)&0xffffu)<0x8000u)) {
            active_slot=(version==2u) ? 0u : 2u;
            sequence=(version==2u) ? sa : 0u;
        } else {
            for (i=0;i<((version==1u) ? 3u : 9u);++i) a[i]=b[i];
            active_slot=(version==2u) ? 1u : 2u;
            sequence=(version==2u) ? sb : 0u;
        }
        if (version==2u) for (i=0;i<9u;++i) best_scores[i]=a[i];
        else for (i=0;i<3u;++i) best_scores[(unsigned char)(i*3u)]=a[i];
        return;
    }
}

unsigned int highscore_get(GameLevel level,MapId map)
{
    if ((unsigned int)level>=3u || (unsigned int)map>=MAP_COUNT) return 0;
    return best_scores[(unsigned char)((unsigned char)level*3u+(unsigned char)map)];
}

HighscoreSaveResult highscore_record_result(GameLevel level,MapId map,
                                             unsigned int score)
{
    unsigned char data[24],slot,i,crc=0,index;
    unsigned int next_seq,verify[9],verify_seq,old_score,limit;
    if ((unsigned int)level>=3u || (unsigned int)map>=MAP_COUNT)
        return HIGHSCORE_SAVE_FAILED;
    limit=(unsigned int)(game_map_free_cells_for(map)-3u)*10u;
    if (score>limit || score%10u!=0u) return HIGHSCORE_SAVE_FAILED;
    index=(unsigned char)((unsigned char)level*3u+(unsigned char)map);
    if (score<=best_scores[index]) return HIGHSCORE_UNCHANGED;
    old_score=best_scores[index]; best_scores[index]=score;
    next_seq=(sequence==65535u) ? 0u : (unsigned int)(sequence+1u);
    slot=(active_slot==0u) ? 1u : 0u;
    data[0]=MAGIC0; data[1]=MAGIC1; data[2]=2u;
    data[3]=(unsigned char)next_seq; data[4]=(unsigned char)(next_seq>>8);
    for (i=0;i<9u;++i) {
        data[5u+2u*i]=(unsigned char)best_scores[i];
        data[6u+2u*i]=(unsigned char)(best_scores[i]>>8);
    }
    for (i=0;i<23u;++i) crc=crc_add(crc,data[i]);
    data[23]=crc;
    write_v2(slot,24,0);
    for (i=0;i<24u;++i) write_v2(slot,i,data[i]);
    write_v2(slot,24,COMMIT);
    if (!read_record(2,slot,verify,&verify_seq) || verify_seq!=next_seq) {
        best_scores[index]=old_score; return HIGHSCORE_SAVE_FAILED;
    }
    for (i=0;i<9u;++i) if (verify[i]!=best_scores[i]) {
        best_scores[index]=old_score; return HIGHSCORE_SAVE_FAILED;
    }
    active_slot=slot; sequence=next_seq;
    return HIGHSCORE_SAVED;
}

# Lưu và đọc kỷ lục bằng EEPROM

## 1. Ví dụ dễ hình dung

Giả sử kỷ lục NORMAL–CLASSIC đang là 80 điểm. Người chơi kết thúc ván với 120 điểm:

| Thời điểm | Điểm ván | Kỷ lục trong RAM | EEPROM |
|---|---:|---:|---|
| Trước ván | 0 | 80 | Có bản ghi kỷ lục 80 |
| Đang chơi được 120 | 120 | Vẫn 80 | Chưa ghi chỉ vì vừa ăn điểm |
| GAME OVER hoặc WIN | 120 | Cập nhật thành 120 nếu lưu thành công | Tạo bản ghi mới ở slot còn lại |
| Khởi động lại với EEPROM còn dữ liệu | Ván mới bắt đầu từ 0 | Đọc lại 120 | Giữ bản ghi đã lưu |

Nếu ván chỉ được 60 hoặc đúng 80, không tạo kỷ lục mới. Sáu tổ hợp NORMAL/HARD × CLASSIC/PILLARS/LABYRINTH giữ điểm riêng, nên 120 của NORMAL–CLASSIC không trở thành kỷ lục HARD–PILLARS.

```text
Slot A đang hợp lệ, sequence 7, kỷ lục 80
   → tạo bản ghi mới ở slot B, sequence 8, kỷ lục 120
   → đánh dấu B hoàn tất và đọc kiểm tra
   → B trở thành slot đang dùng
```

Nhấn 9 chơi lại chỉ reset điểm của ván, không xóa kỷ lục EEPROM. Khả năng đọc lại sau nạp/reset còn phụ thuộc dữ liệu EEPROM có được giữ nguyên bởi công cụ nạp hoặc mô phỏng hay không.

## 2. Cách chương trình thực hiện

Phần lưu trữ nằm trong [storage/highscore_eeprom.c](../../storage/highscore_eeprom.c), API ở [storage/highscore_eeprom.h](../../storage/highscore_eeprom.h). [main.c](../../main.c) gọi đọc lúc khởi động và xử lý kết quả lúc ván kết thúc.

### 2.1. Phân biệt điểm ván, bản sao RAM và bản ghi EEPROM

| Dữ liệu | Nơi lưu | Vai trò |
|---|---|---|
| `score` của rắn | RAM, `game.c` | Điểm ván đang chơi |
| `best_scores[36]` | RAM, nhóm EEPROM | Bản sao các kỷ lục đã đọc, phục vụ truy vấn nhanh |
| `sequence` | RAM | Số thứ tự của bản ghi v4 hiện được chọn |
| `active_slot` | RAM | 0: A, 1: B, 2: chưa có slot v4 đang dùng |
| `hs_v4_a`, `hs_v4_b` | EEPROM | Hai bản ghi bền vững để luân phiên cập nhật |

`eeprom` là từ khóa riêng của CodeVisionAVR để đặt dữ liệu ở EEPROM; `best_scores` không có từ khóa này nên chỉ là RAM. Khi khởi động, phải đọc bản ghi hợp lệ để tạo lại bản sao RAM.

Mảng có 36 ô vì bảo toàn bố cục dữ liệu các phiên bản/nhóm cũ. Gameplay hiện chỉ dùng 6 ô đang hoạt động, không có 36 lựa chọn trên menu.

### 2.2. Ánh xạ Level/Map vào chỉ số kỷ lục

```c
unsigned int highscore_get(GameLevel level,MapId map,GameMode mode)
{
    if ((level!=GAME_LEVEL_NORMAL && level!=GAME_LEVEL_HARD) || (unsigned int)map>=MAP_COUNT ||
        (unsigned int)mode>=MODE_COUNT) return 0;
    return best_scores[(unsigned char)((mode==MODE_SURVIVAL ? 27u : 0u)+
        (unsigned char)level*3u+(unsigned char)map)];
}
```

Hàm kiểm tra Level chỉ là NORMAL=1 hoặc HARD=2, Map nhỏ hơn 3 và Mode thuộc API lưu trữ. Với nhóm gameplay có vật cản, chỉ số là:

```text
index = 27 + level*3 + map
```

| Tổ hợp hiện tại | Phép tính | Chỉ số trong `best_scores[]` |
|---|---|---:|
| NORMAL–CLASSIC | `27+1*3+0` | 30 |
| NORMAL–PILLARS | `27+1*3+1` | 31 |
| NORMAL–LABYRINTH | `27+1*3+2` | 32 |
| HARD–CLASSIC | `27+2*3+0` | 33 |
| HARD–PILLARS | `27+2*3+1` | 34 |
| HARD–LABYRINTH | `27+2*3+2` | 35 |

Tên API `MODE_SURVIVAL` có giá trị enum 1 nhưng được ánh xạ vào nhóm lưu từ offset 27, tương ứng nhóm thứ tư của bố cục v4. Không được lấy trực tiếp `mode*9` rồi thay công thức này.

API vẫn đọc/ghi nhóm STANDARD từ offset 0 để tương thích. Gameplay hiện tại gọi nhóm SURVIVAL cho cả NORMAL/HARD; các nhóm cũ không tự biến thành lựa chọn menu mới.

### 2.3. Bố cục bản ghi v4 dài 79 byte

| Offset trong một slot | Số byte | Nội dung |
|---|---:|---|
| 0 | 1 | `MAGIC0=0x53` |
| 1 | 1 | `MAGIC1=0x47` |
| 2 | 1 | Phiên bản 4 |
| 3–4 | 2 | Sequence: byte thấp trước, cao sau |
| 5–76 | 72 | 36 điểm, mỗi điểm 2 byte |
| 77 | 1 | CRC của các byte 0–76 |
| 78 | 1 | Dấu hoàn tất `COMMIT=0xA5` |

Với ô điểm thứ `i`, byte thấp ở `5+2*i`, byte cao ở `6+2*i`. `data[78]` trong hàm ghi chỉ chứa 78 byte đầu, gồm cả CRC; byte COMMIT được ghi riêng ở EEPROM để thực hiện sau cùng.

Ví dụ index 30 nằm tại offset 65 và 66. Điểm 120 có giá trị hex `0x0078`, được lưu thành `0x78`, `0x00`. Khi đọc, ghép bằng `low | (high<<8)`.

### 2.4. Hai slot v4 và vùng lưu các phiên bản cũ

| Phiên bản | Điểm trong bản ghi | Kích thước mỗi slot | Địa chỉ khai báo trên CodeVision |
|---|---:|---:|---|
| v1 | 3 | 13 byte | Hai mảng đầu, địa chỉ thực tế xem memory map |
| v2 | 9 | 25 byte | A `0x0100`, B `0x0120` |
| v3 | 27 | 61 byte | A `0x0200`, B `0x0240` |
| v4 | 36 | 79 byte | A `0x0300`, B `0x0350` |

V4 A chiếm `0x0300–0x034E`, B chiếm `0x0350–0x039E`, không đè lên nhau. Các vùng v2/v3 được đặt riêng để việc ghi v4 không ghi đè bản ghi cũ.

Địa chỉ v1 không được cố định bằng `@...` trong source, nên tài liệu không giả định nó nằm tại một địa chỉ cụ thể. Nhánh `HOST_TEST` bỏ các địa chỉ cố định để dùng mảng giả lập trong kiểm thử host; đó không phải cách khai báo khi nạp lên MCU.

### 2.5. CRC kiểm tra nội dung bản ghi

```c
static unsigned char crc_add(unsigned char crc,unsigned char value)
{
    unsigned char bit_index;
    crc^=value;
    for (bit_index=0;bit_index<8u;++bit_index)
        crc=(unsigned char)((crc<<1)^((crc&0x80u) ? 0x07u : 0u));
    return crc;
}
```

CRC bắt đầu từ 0. Với mỗi byte dữ liệu, hàm XOR byte đó vào CRC rồi dịch 8 lần. Nếu bit cao trước dịch là 1 thì XOR thêm `0x07`; phép ép về `unsigned char` giữ kết quả trong 8 bit.

Khi ghi v4, chương trình chạy hàm này trên 77 byte đầu rồi đặt kết quả vào byte 77. Khi đọc, nó tính lại từ cùng vùng dữ liệu và so sánh với CRC lưu.

CRC giúp phát hiện bản ghi có dữ liệu không khớp. Nó không thay cho dấu hoàn tất; một bản ghi phải qua cả COMMIT, magic, version, CRC và các kiểm tra điểm mới được dùng.

### 2.6. Đọc một slot bằng `read_record()`

```c
static unsigned char read_record(unsigned char version,unsigned char slot,
                                 unsigned int *scores,unsigned int *seq)
{
    unsigned char i,crc=0,count=(version==1u) ? 3u :
                               ((version==2u) ? 9u :
                               ((version==3u) ? 27u : 36u));
    unsigned char crc_pos=(version==1u) ? 11u :
                          ((version==2u) ? 23u :
                          ((version==3u) ? 59u : 77u));
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
        if (version==4u)
            limit=score_limit_v4((MapId)(i%3u),(unsigned char)(i/9u));
        else {
            limit=(version==1u) ? GAME_MAX_SCORE :
                  (unsigned int)(game_map_free_cells_for((MapId)(i%3u))-3u)*10u;
            if (version==3u && i>=9u) limit=(unsigned int)(limit*2u);
        }
        if (value>limit || value%10u!=0u) return 0;
        scores[i]=value;
    }
    return 1;
}
```

Trình tự đọc:

1. Chọn số ô điểm và vị trí CRC theo phiên bản: v1/v2/v3/v4 dùng 3/9/27/36 điểm.
2. Kiểm tra byte sau CRC có bằng `0xA5`, hai magic đúng và version khớp.
3. Tính lại CRC, loại slot nếu không khớp.
4. Ghép hai byte sequence vào `*seq`.
5. Lần lượt ghép hai byte của từng điểm vào `scores[i]`.
6. Yêu cầu điểm là bội số của 10 và không vượt giới hạn hợp lệ cho nhóm/map đó.
7. Chỉ khi tất cả đều đạt mới trả về 1.

Hai tham số `scores` và `seq` nhận địa chỉ vùng kết quả của bên gọi. Nếu slot bị từ chối giữa chừng, kết quả tạm không được coi là một bản ghi hợp lệ chỉ vì một số điểm đã đọc được.

Giới hạn v4 dùng số mồi tối đa dựa trên số ô map trừ 3 đốt ban đầu, nhân 50 là điểm cao nhất mỗi mồi. CLASSIC cho ngưỡng kiểm tra `109*50=5450`. Đây là ngưỡng kiểm tra dữ liệu, không phải điểm bắt buộc phải đạt để WIN. Các nhóm đã nghỉ còn được xét theo luật cũ để không loại nhầm bản ghi tương thích.

### 2.7. Khởi động: tìm phiên bản hợp lệ trước rồi chọn slot

```c
void highscore_init(void)
{
    unsigned int a[36],b[36],sa=0,sb=0;
    unsigned char va,vb,i,version;
    for (i=0;i<36u;++i) best_scores[i]=0;
    sequence=0; active_slot=2;
    for (version=4u;version>0u;--version) {
        va=read_record(version,0,a,&sa);
        vb=read_record(version,1,b,&sb);
        if (!va && !vb) continue;
        if (va && (!vb || ((unsigned int)(sa-sb)&0xffffu)<0x8000u)) {
            active_slot=(version==4u) ? 0u : 2u;
            sequence=(version==4u) ? sa : 0u;
        } else {
            for (i=0;i<((version==1u) ? 3u :
                        ((version==2u) ? 9u :
                        ((version==3u) ? 27u : 36u)));++i) a[i]=b[i];
            active_slot=(version==4u) ? 1u : 2u;
            sequence=(version==4u) ? sb : 0u;
        }
        if (version==4u) for (i=0;i<36u;++i) best_scores[i]=a[i];
        else if (version==3u) for (i=0;i<27u;++i) best_scores[i]=a[i];
        else if (version==2u) for (i=0;i<9u;++i) best_scores[i]=a[i];
        else for (i=0;i<3u;++i) best_scores[(unsigned char)(i*3u)]=a[i];
        return;
    }
}
```

Hàm đặt bản sao RAM về 0, sequence về 0 và `active_slot=2`. Sau đó thử từ v4 xuống v1; mỗi phiên bản đọc cả A lẫn B.

- Cả hai slot hỏng: thử phiên bản thấp hơn.
- Chỉ một slot hợp lệ: chọn slot đó.
- Cả hai hợp lệ: so sequence theo vòng 16 bit.
- Không có phiên bản hợp lệ: giữ các kỷ lục bằng 0.

Ưu tiên phiên bản xảy ra trước so sequence: có v4 hợp lệ thì không lấy v3 chỉ vì sequence v3 cao hơn. Đọc khởi động không tự ghi một bản v4 mới chỉ để chuyển định dạng.

### 2.8. So sequence khi bộ đếm quay vòng

Code kiểm tra hiệu hai sequence trong 16 bit, so với `0x8000`. Hiệu nhỏ hơn nửa vòng được dùng để nhận biết bản A mới hơn hoặc cùng sequence; cùng sequence thì chọn A.

Ví dụ A có 7, B có 8: hiệu A−B sau giới hạn 16 bit là 65535, nên B mới hơn. Nếu bản cũ là 65535 và bản mới là 0, hiệu 0−65535 trong vòng 16 bit là 1, nên 0 được nhận là mới hơn.

Cách so sánh theo vòng giả định các bản ghi cần phân biệt không cách nhau từ nửa vòng trở lên. Sequence chỉ tăng khi có lần ghi kỷ lục mới thành công, không tăng ở mọi vòng lặp hoặc mọi lần ăn.

### 2.9. Bảo toàn dữ liệu cũ khi đọc

| Bản ghi được chọn | Cách đưa vào bản sao RAM |
|---|---|
| v4 | Chép đủ 36 ô |
| v3 | Chép 27 ô đầu, nhóm còn lại giữ 0 |
| v2 | Chép 9 ô đầu |
| v1 | Ba điểm theo Level được đặt vào các chỉ số 0, 3, 6 |

Nếu dữ liệu đến từ v1/v2/v3, `active_slot` vẫn là 2 và sequence ghi v4 bắt đầu lại từ 0. Khi có kỷ lục mới cần lưu, chương trình sẽ tạo v4 ở A với sequence 1 và mang theo các ô cũ đã đọc trong RAM.

Điểm STANDARD cũ không được tự chuyển sang sáu ô SURVIVAL 30–35. Các nhóm giữ đúng ý nghĩa lưu trữ của chúng. NORMAL=1 và HARD=2 vẫn giữ ID cũ dù menu không còn EASY.

### 2.10. Khi nào được gọi lưu kết quả?

`main.c` gọi `highscore_record_result()` khi đang ở màn chơi, rắn WIN/GAME OVER và `result_saved` chưa bật. Nó truyền Level, Map, `MODE_SURVIVAL` và điểm cuối ván.

Sau đó đặt `result_saved=1` để không gọi lại ở mọi vòng lặp. Trong code hiện tại, `main.c` không kiểm tra giá trị trả về để thử lại khi lưu lỗi. Cờ này đánh dấu đã xử lý lượt kết quả, không phải bằng chứng EEPROM luôn ghi thành công.

Điểm đang chơi chỉ nằm trong `score`; không ghi EEPROM sau từng lần ăn. Cách này tránh những lần ghi không cần thiết khi điểm chưa vượt kỷ lục hoặc ván còn đang chạy.

### 2.11. Chỉ tạo bản ghi mới cho điểm hợp lệ và cao hơn

Toàn bộ hàm ghi:

```c
HighscoreSaveResult highscore_record_result(GameLevel level,MapId map,
                                             GameMode mode,
                                             unsigned int score)
{
    unsigned char data[78],slot,i,crc=0,index;
    unsigned int next_seq,verify[36],verify_seq,old_score,limit;
    if ((level!=GAME_LEVEL_NORMAL && level!=GAME_LEVEL_HARD) || (unsigned int)map>=MAP_COUNT ||
        (unsigned int)mode>=MODE_COUNT)
        return HIGHSCORE_SAVE_FAILED;
    limit=score_limit_v4(map,(unsigned char)(mode==MODE_SURVIVAL ? 3u : 0u));
    if (score>limit || score%10u!=0u) return HIGHSCORE_SAVE_FAILED;
    index=(unsigned char)((mode==MODE_SURVIVAL ? 27u : 0u)+
                          (unsigned char)level*3u+(unsigned char)map);
    if (score<=best_scores[index]) return HIGHSCORE_UNCHANGED;
    old_score=best_scores[index]; best_scores[index]=score;
    next_seq=(sequence==65535u) ? 0u : (unsigned int)(sequence+1u);
    slot=(active_slot==0u) ? 1u : 0u;
    data[0]=MAGIC0; data[1]=MAGIC1; data[2]=4u;
    data[3]=(unsigned char)next_seq; data[4]=(unsigned char)(next_seq>>8);
    for (i=0;i<36u;++i) {
        data[5u+2u*i]=(unsigned char)best_scores[i];
        data[6u+2u*i]=(unsigned char)(best_scores[i]>>8);
    }
    for (i=0;i<77u;++i) crc=crc_add(crc,data[i]);
    data[77]=crc;
    write_v4(slot,78,0);
    for (i=0;i<78u;++i) write_v4(slot,i,data[i]);
    write_v4(slot,78,COMMIT);
    if (!read_record(4,slot,verify,&verify_seq) || verify_seq!=next_seq) {
        best_scores[index]=old_score; return HIGHSCORE_SAVE_FAILED;
    }
    for (i=0;i<36u;++i) if (verify[i]!=best_scores[i]) {
        best_scores[index]=old_score; return HIGHSCORE_SAVE_FAILED;
    }
    active_slot=slot; sequence=next_seq;
    return HIGHSCORE_SAVED;
}
```

Các bước đầu kiểm tra Level/Map/Mode, giới hạn điểm và tính bội số của 10. Dữ liệu không hợp lệ trả về FAILED trước khi ghi.

Sau khi tính chỉ số, nếu `score<=best_scores[index]` thì trả UNCHANGED. Ví dụ kỷ lục 80, ván được 80 cũng không tạo bản ghi mới.

Nếu có kỷ lục mới, hàm giữ `old_score`, cập nhật tạm bản sao RAM rồi chọn `next_seq` và slot đích. A đang dùng thì ghi B; B đang dùng thì ghi A; chưa có v4 đang dùng thì ghi A.

### 2.12. Đóng gói cả 36 điểm, không chỉ một điểm vừa thay đổi

Hàm gán magic, version, sequence rồi duyệt đủ 36 ô RAM để tạo vùng `data[]`. Byte thấp và cao được lấy riêng để định dạng không phụ thuộc cách sao chép cấu trúc trong RAM.

Việc ghi toàn bộ bản ghi làm mỗi slot hợp lệ chứa trọn bộ kỷ lục. Khi chọn lại A/B lúc khởi động, chương trình không cần ghép một nửa dữ liệu của A với một nửa của B. Các ô nhóm cũ trong bản sao RAM cũng được mang theo.

Ví dụ minh họa: mọi điểm khác bằng 0, riêng NORMAL–CLASSIC index 30 bằng 120, sequence 8:

```text
Offset 0–4:    53 47 04 08 00
Offset 65–66:  78 00      (120 điểm)
Offset 77:     18      (CRC được tính từ byte 0–76)
Offset 78:     A5         (COMMIT)
```

Giá trị CRC trên phụ thuộc toàn bộ các byte, nên thay một điểm hoặc sequence sẽ phải tính lại.

### 2.13. Ghi COMMIT sau cùng rồi đọc kiểm tra

Thứ tự ghi là:

```text
Chọn slot đích khác slot đang dùng
  → ghi COMMIT của slot đích = 0, đánh dấu chưa hoàn tất
  → ghi 78 byte dữ liệu và CRC
  → ghi COMMIT = 0xA5 sau cùng
  → read_record() kiểm tra lại
  → so sequence và đủ 36 điểm với dữ liệu mong muốn
  → cập nhật active_slot và sequence nếu tất cả đúng
```

Nếu việc ghi dừng giữa chừng trước COMMIT, slot đích chưa được chấp nhận; slot cũ vẫn còn. Thiết kế hai slot và dấu cuối giảm khả năng dùng một bản ghi đang ghi dở sau khởi động lại.

Chỉ sau khi đọc lại khớp, hàm trả SAVED. Nếu đọc/so sánh thất bại, nó trả FAILED và phục hồi `best_scores[index]=old_score`; `active_slot` và sequence RAM chưa được chuyển sang bản mới.

### 2.14. Kết quả API và hiển thị kỷ lục

| Kết quả | Hành vi |
|---|---|
| `HIGHSCORE_UNCHANGED` | Điểm không vượt kỷ lục; không ghi |
| `HIGHSCORE_SAVED` | Đã ghi và xác minh bản mới |
| `HIGHSCORE_SAVE_FAILED` | Dữ liệu không hợp lệ hoặc xác minh thất bại |

Menu Level truy vấn kỷ lục CLASSIC cho Level đang trỏ tới. Menu Map truy vấn đúng tổ hợp Level đã xác nhận và Map đang trỏ tới. Các truy vấn dùng `best_scores[]` trong RAM, không đọc lại toàn EEPROM mỗi lần đổi con trỏ.

Restart không gọi hàm xóa EEPROM. Code hiện cũng không có lựa chọn menu để xóa toàn bộ kỷ lục. Lưu trữ và điểm ván mới là hai vòng đời khác nhau: điểm ván về 0, bản ghi tốt vẫn giữ cho các lần đọc sau.

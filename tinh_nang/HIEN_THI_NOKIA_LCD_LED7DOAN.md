# Hiển thị trên Nokia 5110, LCD 16×2 và LED 7 đoạn

## 1. Ví dụ dễ hình dung

Giả sử đang chơi HARD–PILLARS, điểm 80 và đã có 2 thanh chắn:

| Thiết bị | Nội dung đang hiển thị |
|---|---|
| Nokia 5110 | Tường map, hai thanh chắn, thân rắn và mồi |
| LED 7 đoạn | `0080` |
| LCD 16×2 | `HARD PILLARS` và `OBST:02 PLAYING` |

Phím 5 được bỏ qua khi đang chơi. Khi GAME OVER, Nokia/LCD chuyển sang kết quả và điểm cuối, còn LED giữ điểm đó.

```text
Dữ liệu game
  ├─ tọa độ thân/mồi/tường/thanh ──> Nokia 5110
  ├─ số điểm ─────────────────────> bộ chữ số ──> quét LED 7 đoạn
  └─ Level/Map/trạng thái ─────────> bộ đệm LCD ──> ghi từng ký tự
```

Ba thiết bị chia sẻ cùng dữ liệu ván nhưng dùng cách cập nhật khác nhau. Cập nhật hình vẽ, ghi chuỗi LCD và quét LED không phải cùng một thao tác phần cứng.

## 2. Cách chương trình thực hiện

Các driver là [drivers/display.c](../drivers/display.c), [drivers/lcd1602.c](../drivers/lcd1602.c), [drivers/led7seg.c](../drivers/led7seg.c). [game/game.c](../game/game.c) chuẩn bị hình vẽ, còn [main.c](../main.c) gọi phục vụ theo trạng thái và đồng hồ.

### 2.1. Phân chia trách nhiệm

| Phần | Trách nhiệm |
|---|---|
| `game_render()` và các hàm menu | Chọn nội dung cần xuất lên Nokia |
| Driver `display` | Chuyển lời gọi vẽ/text thành API GLCD CodeVision |
| Driver `lcd1602` | Chuẩn bị 32 ký tự mong muốn, ghi dần phần khác với dữ liệu đã xuất |
| Driver `led7seg` | Tách số thành 4 chữ số, xuất mẫu segment và chọn digit |
| `main.c` | Điều phối cờ vẽ, cập nhật trạng thái, quét LED và phục vụ LCD |

Điểm khi đang chơi được hiện trên LED 7 đoạn. LCD đang chơi hiện Level, Map, số thanh và PLAYING. Nokia đang chơi tập trung vùng đồ họa; điểm được in trên Nokia ở màn kết quả.

### 2.2. Các chân và cấu hình thư viện

Nokia dùng các tín hiệu PG0=DC, PG1=DIN, PG2=CLK, PG3=RST, PG4=CE/CS. Chân đèn nền PD7 được cấu hình bởi source. Các chân giao tiếp GLCD phải được thiết lập trong project CodeVision cho Nokia/PCD8544; chỉ đặt macro kích thước không tự tạo cấu hình backend thư viện.

LCD ở project hiện tại dùng PA0=RS, PA1=R/W, PA2=E, PA3..PA6=D4..D7. `alcd.h` và `glcd.h` là thư viện CodeVision được dùng qua driver, không phải các file mock của bộ kiểm thử host.

LED dùng nhiều bit rải trên PORTA, PORTD, PORTF:

| Chức năng | Chân |
|---|---|
| Digit 1, 2, 3, 4 | PF0, PF3, PF4, PF6 |
| Segment A, B, C | PF1, PF5, PA7 |
| Segment D, E, F, G | PD1, PD2, PF2, PD3 |
| DP | PF7, được giữ tắt |

Các mask trong [config/board_config.h](../config/board_config.h) mô tả đúng các bit này. Mức cấu hình hiện tại là chọn digit bằng 1 và bật segment bằng 0 cho LED common-anode trực tiếp.

### 2.3. Khởi tạo Nokia và font

```c
unsigned char display_init(void)
{
    GLCDINIT_t init_data;

    /* PD7 is shared by Nokia-BL and LCD-BL on the supplied schematic. */
    NOKIA_BL_DDR=1;
    NOKIA_BL=1;

    init_data.font=font5x7;
    init_data.readxmem=0;
    init_data.writexmem=0;
    init_data.temp_coef=0;
    init_data.bias=2;
    init_data.vlcd=NOKIA_CONTRAST;

    return (unsigned char)glcd_init(&init_data);
}
```

`GLCDINIT_t` chứa font và tham số PCD8544. `font5x7` là font mặc định; `temp_coef=0`, `bias=2`, `vlcd=NOKIA_CONTRAST` với cấu hình hiện tại là `0x40`. Hai con trỏ bộ nhớ ngoài được đặt 0 vì không cung cấp hàm đọc/ghi bộ nhớ ngoài.

`NOKIA_BL_DDR=1` đặt chân đèn nền làm output rồi bật PD7. Theo mạch đang dùng, đường này dùng chung cho đèn nền Nokia và LCD.

`glcd_init()` trả kết quả qua `display_init()`. Trong nhánh game bình thường, nếu kết quả là thất bại, `main.c` đứng ở vòng chờ khởi tạo, chưa đi vào luồng game; đó không phải trạng thái GAME OVER do va chạm.

### 2.4. Đổi từ ô game sang pixel

Màn hình Nokia rộng 84, cao 48 pixel. Ô game có kích thước 6 pixel nên có 14 cột và 8 hàng:

```text
pixel_x = cell_x * 6
pixel_y = cell_y * 6
```

| Đối tượng | Cách vẽ |
|---|---|
| Đốt rắn | Hình tô đặc `5×5` pixel tại góc ô |
| Tường cố định | Khung `6×6` pixel |
| Thanh chắn 3 ô | Hình tô đặc `18×6` pixel |
| Mồi | Mẫu điểm ảnh tròn/vuông/sao theo loại |

Ví dụ đốt ở `(8,4)` bắt đầu tại pixel `(48,24)`. Thân được vẽ từng ô riêng nên vẫn hiển thị đúng khi các đốt nằm ở hai cạnh sau xuyên biên.

### 2.5. Thứ tự vẽ một màn chơi

```c
void game_render(void)
{
    unsigned char i,x,y;
    SnakePoint point;
    SnakeState state=snake_state();
    display_clear();
    if (state==SNAKE_GAME_OVER) {
        display_text(0,4,over_text);
        render_final_score();
        return;
    }
    if (state==SNAKE_WIN) {
        display_text(0,4,win_text);
        render_final_score();
        return;
    }
    for (y=0; y<GRID_ROWS; ++y) for (x=0; x<GRID_COLS; ++x)
        if (game_map_is_blocked(x,y))
            display_rect((unsigned char)(x*CELL_SIZE),
                         (unsigned char)(y*CELL_SIZE),CELL_SIZE,CELL_SIZE);
    for (i=0;i<dynamic_obstacle_get_count();++i) {
        point=dynamic_obstacle_point(i);
        display_fill_rect((unsigned char)(point.x*CELL_SIZE),
                          (unsigned char)(point.y*CELL_SIZE),
                          DYNAMIC_OBSTACLE_LENGTH*CELL_SIZE,CELL_SIZE);
    }
    point=snake_food();
    render_fruit(point,snake_get_food_type());
    for (i=0; i<snake_length(); ++i) {
        point=snake_segment(i);
        display_fill_rect((unsigned char)(point.x*CELL_SIZE),
                          (unsigned char)(point.y*CELL_SIZE),
                          CELL_SIZE-1,CELL_SIZE-1);
    }
}
```

Hàm xóa màn hình trước. Nếu GAME OVER hoặc WIN, nó vẽ thông báo và điểm rồi trả về ngay, không vẽ tiếp map.

Khi RUNNING, thứ tự vẽ là tường cố định → thanh chắn → mồi → các đốt rắn. Dữ liệu logic đã kiểm tra các đối tượng không được chồng lên nhau ở những trường hợp đặt mồi/vật cản hợp lệ.

Không có một hình con rắn riêng được kéo dài hoặc di chuyển trên màn hình độc lập. Hình vẽ được tạo lại từ các tọa độ sau khi xử lý bước đi.

### 2.6. Giới hạn vùng vẽ và phân biệt chuỗi Flash/RAM

`display_rect()` và `display_fill_rect()` loại kích thước bằng 0, tọa độ bắt đầu ngoài màn hình, rồi cắt phần vượt cạnh. Khi gọi thư viện, góc cuối dùng `x+width-1`, `y+height-1` vì hai tọa độ góc là các điểm bao gồm trong hình.

Chuỗi hằng `flash char` dùng `display_text()` → `glcd_outtextxyf()`. Chuỗi RAM như điểm được ghép vào mảng ký tự dùng `display_text_ram()` → `glcd_outtextxy()`. Trên AVR, phân biệt nơi lưu chuỗi là cần thiết vì Flash và RAM dùng cơ chế truy cập khác nhau.

### 2.7. Khi nào `main.c` yêu cầu vẽ lại?

Cờ `redraw` được bật khi đổi lựa chọn menu, vào ván, bước di chuyển làm thay đổi trạng thái/hình ảnh, Restart hoặc sinh được thanh chắn.

Cuối lượt, `main.c` tạm blank LED, gọi hàm menu hoặc game phù hợp rồi resume LED. Nó không vẽ toàn Nokia ở mọi vòng lặp khi không có thay đổi.

Các lệnh GLCD là lời gọi đồng bộ trong vòng chính. Trong lúc đang vẽ, các lượt phục vụ khác của vòng chính phải chờ đến khi lệnh vẽ hoàn tất; ngắt Timer1 vẫn chỉ tăng tick, không tự quét LED.

### 2.8. Tách điểm thành bốn chữ số

```c
void led7seg_set_number(unsigned int number)
{
    if (number>9999u) number=9999u;
    digits[0]=(unsigned char)(number/1000u);
    digits[1]=(unsigned char)((number/100u)%10u);
    digits[2]=(unsigned char)((number/10u)%10u);
    digits[3]=(unsigned char)(number%10u);
    enabled=1;
}
```

Ví dụ số 80 tạo `digits=[0,0,8,0]`. Các phép `/1000`, `/100%10`, `/10%10`, `%10` lấy lần lượt hàng nghìn, trăm, chục, đơn vị. Giá trị trên 9999 chỉ được chặn khi chuẩn bị chữ số, không sửa điểm trong game.

`led7seg_set_number()` cập nhật bộ chữ số và bật cờ `enabled`; nó không đứng chờ để hiện lần lượt cả bốn digit. Việc đó được thực hiện ở các lần quét sau.

### 2.9. Từ chữ số sang các segment

Bảng `digit_pattern[]` lưu mẫu với thứ tự bit A/B/C/D/E/F/G. Ví dụ số 0 có mẫu `0x3f`, bật A..F và tắt G; số 8 có `0x7f`, bật cả bảy đoạn.

Các bit trong mẫu là ý nghĩa “đoạn cần sáng”, chưa phải mức điện áp trên mọi chân. Driver đổi mẫu sang các bit rải trên ba port:

```c
static void segment_out(unsigned char pattern)
{
    unsigned char f_on=0;
    unsigned char d_on=0;
    unsigned char a_on=0;

    if (pattern&0x01) f_on |= LED7_SEG_A_MASK;
    if (pattern&0x02) f_on |= LED7_SEG_B_MASK;
    if (pattern&0x04) a_on |= LED7_SEG_C_MASK;
    if (pattern&0x08) d_on |= LED7_SEG_D_MASK;
    if (pattern&0x10) d_on |= LED7_SEG_E_MASK;
    if (pattern&0x20) f_on |= LED7_SEG_F_MASK;
    if (pattern&0x40) d_on |= LED7_SEG_G_MASK;

    /* ALCD owns PA0..PA6; only PA7 is changed here. */
    DDRA |= LED7_SEG_C_MASK;
    if (LED7_SEGMENT_ON_LEVEL) {
        PORTF=(PORTF & (unsigned char)~LED7_SEG_F_PORT_MASK) | f_on;
        PORTD=(PORTD & (unsigned char)~LED7_SEG_D_PORT_MASK) | d_on;
        PORTA=(PORTA & (unsigned char)~LED7_SEG_C_MASK) | a_on;
    } else {
        PORTF=(PORTF & (unsigned char)~LED7_SEG_F_PORT_MASK) |
              ((unsigned char)~f_on & LED7_SEG_F_PORT_MASK);
        PORTD=(PORTD & (unsigned char)~LED7_SEG_D_PORT_MASK) |
              ((unsigned char)~d_on & LED7_SEG_D_PORT_MASK);
        PORTA=(PORTA & (unsigned char)~LED7_SEG_C_MASK) |
              ((unsigned char)~a_on & LED7_SEG_C_MASK);
    }
}
```

`f_on`, `d_on`, `a_on` gộp các bit cần bật của từng port. Với mức segment-on bằng 0, driver đảo các bit thuộc segment để xuất thấp cho đoạn sáng, cao cho đoạn tắt.

Phép `(PORT & ~mask) | new_bits` chỉ sửa các bit trong mask và giữ các chân khác. Ví dụ phần LED trên PORTD không sửa PD0 của keypad hoặc PD7 của đèn nền. Trên PORTA, driver LED chỉ sửa PA7, giữ PA0..PA6 cho LCD. DP không được thêm vào mẫu sáng nên được giữ tắt theo mức cấu hình.

### 2.10. Quét luân phiên bốn digit

```c
void led7seg_scan(unsigned long now_ms)
{
    if (!enabled) return;
    if (scan_started && (unsigned long)(now_ms-last_scan_ms)<LED7_SCAN_MS) return;
    digit_off_all();
    if (scan_started) current_digit=(unsigned char)((current_digit+1u)&3u);
    else {
        scan_started=1;
        current_digit=0;
    }
    segment_out(digit_pattern[digits[current_digit]]);
    digit_on(current_digit);
    last_scan_ms=now_ms;
}
```

Mỗi lần đến chu kỳ `LED7_SCAN_MS=2` ms:

1. Tắt tất cả digit trước khi đổi mẫu, giảm việc mẫu của digit cũ xuất nhầm ở digit mới.
2. Tăng `current_digit`, quay vòng bằng `&3u`; lần đầu chọn digit 0.
3. Xuất mẫu của `digits[current_digit]`.
4. Bật đúng digit tương ứng.
5. Ghi lại `last_scan_ms`.

Nếu phục vụ đều, 4 digit mất khoảng 8 ms cho một vòng, tương ứng khoảng 125 vòng/giây. Đây là chu kỳ danh định từ cấu hình, không phải tần số đã đo; vẽ GLCD hoặc ghi LCD trong vòng chính có thể kéo dài khoảng cách giữa các lần quét.

Hàm không sinh bù nhiều lần quét khi bị trễ. Nó chuyển một digit ở một lần gọi đủ thời gian.

### 2.11. Tạm tắt digit, khôi phục hiển thị và Clear khác nhau thế nào?

| Hàm | Hành vi |
|---|---|
| `led7seg_blank()` | Tắt digit tạm thời, giữ dữ liệu chữ số và cờ hoạt động |
| `led7seg_resume()` | Nếu đã quét và còn enabled, xuất lại mẫu và bật digit hiện tại |
| `led7seg_clear()` | Đặt enabled=0, tắt digit và xuất mẫu không sáng |

Trong game, blank/resume bao quanh các thao tác GLCD hoặc LCD. Chúng tránh để một digit cứ sáng suốt khoảng thao tác dài, và giúp tái xuất mẫu sau khi phục vụ ngoại vi có dùng chung port.

Chỉ gọi blank không xóa điểm hoặc tắt bộ quét vĩnh viễn. Khi muốn thể hiện điểm 0 ở menu, chương trình gọi `led7seg_set_number(0)`.

### 2.12. LCD dùng hai bộ đệm 32 ký tự

| Biến | Ý nghĩa |
|---|---|
| `desired[32]` | Nội dung chương trình muốn LCD hiển thị |
| `shown[32]` | Nội dung driver đã ghi nhận là đã gửi |
| `next_position` | Điểm bắt đầu tìm ký tự cần cập nhật tiếp theo |

Dòng 1 là chỉ số 0–15, dòng 2 là 16–31. `shown` là bản theo dõi của phần mềm, không phải kết quả đọc lại toàn màn LCD từ phần cứng.

`lcd1602_init()` gọi `lcd_init(16)`, đặt desired thành dấu cách và shown thành 0. Các nội dung khác nhau sẽ được ghi trong những lượt phục vụ đầu.

`spaces()` xóa nội dung mong muốn về 32 dấu cách trước khi ghép một trạng thái mới, giúp chữ dài ở màn cũ không còn đuôi ký tự khi chuyển sang chuỗi ngắn hơn.

### 2.13. Ghép nội dung theo trạng thái

`copy_flash()` chép chuỗi từ Flash vào desired, dừng trước khi vượt độ dài dòng hoặc cuối mảng. `put_four_digits()` ghép một số thành bốn ký tự ASCII, không dùng `printf()` để định dạng.

Trong ván, `lcd1602_show_obstacles()` ghép Level/Map ở dòng trên, `OBST:00 PLAYING` ở dòng dưới, rồi điền số thanh tại offset 21 và 22, tức cột 5 và 6 của dòng 2.

Menu Level hiện `SELECT LEVEL` và Level đang chọn. Menu Map hiện `SELECT MAP` và Map đang chọn. Màn cuối hiện thắng/thua và điểm.

Driver còn có hàm `lcd1602_show_playing()` dùng chuỗi `STATUS: PLAYING` cho kiểm tra hiển thị. Luồng game hiện tại đi qua `lcd_for_state()` và `show_obstacles()`, nên nội dung đang chơi thực tế là dạng có số thanh chắn.

### 2.14. Chỉ ghi ký tự LCD thay đổi, một ký tự mỗi lượt

```c
void lcd1602_service(void)
{
    unsigned char n,index;
    for (n=0; n<32; ++n) {
        index=(unsigned char)((next_position+n)&31u);
        if (shown[index]!=desired[index]) {
            lcd_gotoxy((unsigned char)(index&15u),(unsigned char)(index>>4));
            lcd_putchar(desired[index]);
            shown[index]=desired[index];
            next_position=(unsigned char)((index+1u)&31u);
            return;
        }
    }
}
```

`lcd1602_is_dirty()` duyệt 32 vị trí để biết còn khác nhau giữa shown và desired hay không. Nếu có, `main.c` blank LED, gọi service rồi resume LED.

Trong service:

1. Xét lần lượt tối đa 32 vị trí, bắt đầu từ `next_position`.
2. `(next_position+n)&31u` quay vòng trong mảng 32 phần tử.
3. Gặp một ký tự khác thì đổi chỉ số sang cột `index&15u` và dòng `index>>4`.
4. Gửi một ký tự bằng `lcd_putchar()`.
5. Cập nhật shown, chuyển điểm bắt đầu sang vị trí sau rồi trả về.

Một lần gọi chỉ ghi tối đa một ký tự. Những lần lặp sau tiếp tục làm phần còn thiếu. Ví dụ số thanh 01 sang 02 hoặc chuyển sang màn GAME OVER chỉ cần ghi các vị trí khác, không buộc xóa toàn LCD vật lý rồi ghi lại 32 ký tự mỗi lần.

Nếu trạng thái mới tới khi cập nhật cũ chưa xong, desired nhận nội dung mới; service tiếp tục tìm điểm khác với nội dung mong muốn hiện tại. Đây là cập nhật hội tụ theo bộ đệm, không phải hàng đợi từng màn hình cũ.

### 2.15. Luồng chung của một lần cập nhật game

| Sự kiện | Nokia | LED 7 đoạn | LCD |
|---|---|---|---|
| Bước đi không ăn | Vẽ vị trí rắn mới | Giữ chữ số | Giữ thông tin nếu không đổi |
| Ăn mồi | Vẽ thân dài thêm và mồi mới; HARD có thể đổi đầu | Nhận điểm mới | Không cần hiện điểm đang chơi |
| Sinh thanh thành công | Vẽ thêm thanh | Giữ điểm | Đổi số thanh |
| WIN/GAME OVER | Thay bằng màn kết quả | Giữ điểm cuối | Hiện kết quả và điểm |
| Restart | Vẽ ván mới | Về 0000 | Hiện trạng thái ván mới |

Toàn bộ cách xuất dữ liệu dựa trên biến game và cờ cập nhật. Việc một màn hình chưa được vẽ lại ngay không tự làm thay đổi trạng thái logic của rắn; xử lý gameplay và chuẩn bị hiển thị là hai bước được gọi theo thứ tự trong vòng chính.

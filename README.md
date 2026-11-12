# Snake Game ATmega128 — kiểm định giai đoạn 6

Firmware Embedded C cho **CodeVisionAVR**. Nokia 5110 hiển thị game và menu; LED 7 đoạn CA hiển thị điểm; LCD 16×2 hiển thị cấp độ, trạng thái và kỷ lục. Source được sửa trên Ubuntu; build HEX bằng CodeVisionAVR trên Windows, thử trong Proteus rồi mới nạp board thật. Người dùng báo hệ thống đã có kết quả mô phỏng Proteus; workspace này không có log Build All hoặc mô phỏng để đối chiếu. Kết quả kiểm định tại đây nằm trong [báo cáo cuối](docs/final_validation_report.md).

## Cấu trúc source

| Module | Vai trò |
|---|---|
| `main.c` | `APP_STATE_MENU`/`APP_STATE_GAME`, khởi tạo và lập lịch không chặn |
| `game/game_menu.c/.h` | Cấp độ đang chọn, phím menu, chu kỳ di chuyển, vẽ menu Nokia |
| `game/snake_game.c/.h` | Rắn, thức ăn, va chạm, điểm và trạng thái gameplay; không đổi |
| `game/game_render.c/.h` | Vẽ gameplay và lời nhắc `9:RESTART`, `1:MENU` khi kết thúc |
| `storage/highscore_eeprom.c/.h` | Kỷ lục ba cấp độ, kiểm tra và ghi EEPROM nội |
| `drivers/keypad.c/.h` | Thêm `KEY_MENU` cho phím 1; debounce và các phím cũ giữ nguyên |
| `drivers/lcd1602.c/.h` | Wrapper `alcd.h`; menu, level, trạng thái, score; chỉ ghi ký tự thay đổi |
| `drivers/display.c/.h`, `led7seg.c/.h`, `timer.c/.h` | GLCD, LED CA, Timer1 1 ms; giữ thuật toán Prompt 4 |
| `config/board_config.h` | Pin mapping, macro test và ba chu kỳ di chuyển |

## Menu, tốc độ và phím

Sau reset, ứng dụng vào menu với EASY được chọn; Snake chưa khởi tạo để chạy. Nokia hiện tên game, ba lựa chọn có dấu `>` và lời nhắc. Font 5×7 chiếm tối đa 6 pixel ngang/ký tự: dòng dài nhất `2/8:MOVE 5:OK` có 13 ký tự, rộng 78 pixel; các dòng ở y=0,10,18,26,40 nằm trong 84×48. Màn hình chỉ được vẽ lại khi chọn khác hoặc chuyển trạng thái.

| Cấp độ | Chu kỳ di chuyển |
|---|---:|
| EASY | 500 ms |
| NORMAL | 300 ms |
| HARD | 150 ms |

`game_get_move_period()` là nguồn duy nhất cho chu kỳ. Timer1 vẫn tick 1 ms; main dùng `(unsigned long)(now_ms-last_move_ms)>=move_period_ms` nên so sánh chịu được tràn tick 32-bit. Không dùng delay để di chuyển. Mốc thời gian được đặt lại khi Start, Resume và Restart; không có bước dồn từ thời gian đứng ở menu hoặc Pause. `timer_get_ms()` đọc bộ đếm ISR một cách nguyên tử.

| Trạng thái | Phím |
|---|---|
| Menu | 2 lên, 8 xuống (xoay vòng), 5 chọn và Start |
| Running | 2/4/6/8 đổi hướng, 5 Pause, 9 Restart |
| Paused | 5 Resume, 9 Restart |
| Game Over/Win | 9 Restart cùng cấp độ, 1 về menu |

Menu là trạng thái cấp ứng dụng; `snake_game.c` vẫn chỉ quản lý RUNNING, PAUSED, GAME_OVER và WIN. Khi quay về menu, lựa chọn mặc định là cấp độ vừa chơi. Phím 1 được phát qua `KeyEvent` hiện có nên một lần nhấn chỉ được xử lý một lần. Khi vào menu, LED hiện `0000`; khi chơi, LED lấy score trực tiếp từ `snake_get_score()` và giữ điểm cuối ở Game Over/Win.

LCD menu hiện `SELECT LEVEL` và `<LEVEL> BEST:dddd`; chuỗi dài nhất `NORMAL BEST:0080` vừa đúng 16 ký tự. Gameplay hiện `LEVEL: EASY/NORMAL/HARD` và `STATUS: PLAYING` hoặc `GAME PAUSED`; khi kết thúc hiện `GAME OVER`/`YOU WIN!` và `SCORE: dddd`. Wrapper dùng `lcd_init(16)`, `lcd_gotoxy`, `lcd_putchar`; hai mảng 32 byte lưu nội dung mong muốn/đã hiện, chỉ một ký tự khác biệt được ghi trong mỗi vòng lặp. Trước mỗi lần ghi ALCD, main tắt các digit LED rồi khôi phục; không gọi LCD hoặc LED từ Timer ISR.

## EEPROM High Score

CodeVisionAVR hỗ trợ biến toàn cục với thuộc tính `eeprom`; mã dùng `eeprom unsigned char hs_slot_a[13]` và `hs_slot_b[13]`, truy cập từng byte bằng chỉ số. Cú pháp này được mô tả trong [CodeVisionAVR User Manual, mục Accessing the EEPROM](https://www.thierry-lequeu.fr/data/CodeVisionAVR-3-20-User-Manual.pdf). Không dùng API AVR-GCC hay struct phụ thuộc layout compiler.

Mỗi bản ghi 13 byte: magic `53 47`, version `01`, sequence 16-bit little-endian, ba score 16-bit little-endian (EASY/NORMAL/HARD), CRC-8 của 11 byte đầu và byte commit `A5`. Giá trị hợp lệ phải là bội số 10 trong khoảng 0–1090. Khi khởi động, đọc hai slot, kiểm tra marker/version/CRC/score và chọn sequence mới hơn; nếu cả hai sai thì dùng ba kỷ lục bằng 0 **mà không ghi EEPROM**. Sequence có so sánh modulo 16-bit.

Chỉ khi ván chuyển sang Game Over/Win và score vượt kỷ lục cấp độ đó, mã ghi slot còn lại: xóa commit, ghi 12 byte dữ liệu, ghi commit cuối, rồi đọc lại xác minh. Không ghi sau từng bước, khi Pause, hay liên tục trong màn hình kết thúc. Nếu ghi thất bại, cache RAM trở lại kỷ lục trước đó. Cách ghi hai slot giúp giữ bản ghi cũ khi mất nguồn giữa lúc ghi bản mới; không bảo đảm an toàn tuyệt đối trước lỗi ở cả hai slot hoặc lỗi ghi không được phát hiện. Một lần ghi EEPROM có thể tạm ngừng quét LED trong lúc ván đã kết thúc.

Khi cập nhật firmware, **không tự động nạp file `.eep`** của bản build mới nếu muốn giữ kỷ lục: CodeVisionAVR tạo `.eep` cho dữ liệu khởi tạo EEPROM, còn `.hex` là firmware Flash. Hai mảng EEPROM trong source không có giá trị khởi tạo; hãy kiểm tra nội dung `.eep` thực tế và các lựa chọn erase/program EEPROM của programmer hoặc Proteus trước khi nạp. Không được suy ra rằng chỉ nạp `.hex` thì EEPROM chắc chắn được giữ nếu thao tác nạp có chip erase.

### Lưu ý cú pháp CodeVisionAVR

`bit` là từ khóa riêng của CVAVR, không được dùng làm tên biến cục bộ. Hàm CRC dùng biến đếm `bit_index` kiểu `unsigned char`. Các cờ logic khác cũng dùng `unsigned char`; hai mảng `eeprom unsigned char` là biến toàn cục, không dùng struct hay API AVR-GCC. Khi sửa module EEPROM, kiểm tra tên biến trùng các từ khóa CVAVR trước khi Build All.

## Pin mapping giữ nguyên

Theo `docs/so_do_chan_mach.jpeg`: Nokia DC/DIN/CLK/RST/CS = PG0/PG1/PG2/PG3/PG4, BL = PD7; keypad ROW1/2/3 = PC6/PE1/PD0, COL1/2/3 = PC4/PC5/PE0; LCD RS/RW/E/D4/D5/D6/D7 = PA0/PA1/PA2/PA3/PA4/PA5/PA6. Cụm LED **7SEG-CA** dùng Digit 1–4 = PF0/PF3/PF4/PF6; A/B/C/D/E/F/G/DP = PF1/PF5/PA7/PD1/PD2/PF2/PD3/PF7. PA7 dùng chung PORTA với LCD; không đổi wiring. Mức kích hoạt LED và hành vi ghi PORTA của `alcd.h` phụ thuộc cấu hình/board thực tế, đã được người dùng kiểm thử trong Prompt 4.

| Ngoại vi | Tín hiệu → GPIO |
|---|---|
| Nokia 5110 | DC→PG0, DIN→PG1, CLK→PG2, RST→PG3, CS→PG4, BL→PD7 |
| Keypad 3×3 | ROW1→PC6, ROW2→PE1, ROW3→PD0; COL1→PC4, COL2→PC5, COL3→PE0 |
| LCD 16×2 | RS→PA0, RW→PA1, E→PA2, D4→PA3, D5→PA4, D6→PA5, D7→PA6 |
| LED 7SEG-CA | Digit 1/2/3/4→PF0/PF3/PF4/PF6; A/B/C/D/E/F/G/DP→PF1/PF5/PA7/PD1/PD2/PF2/PD3/PF7 |

Keypad PE0/PE1 còn là PDI/PDO khi ATmega128 được nạp nối tiếp; kiểm tra mạch ISP và ngoại vi nối chung trước khi nạp. PF4–PF7 liên quan JTAG; kiểm tra JTAGEN/JTD nếu LED có digit/segment không hoạt động. Theo [datasheet ATmega128](https://ww1.microchip.com/downloads/en/DeviceDoc/doc2467.pdf), fuse EESAVE quyết định EEPROM có được giữ qua chip erase hay không; chỉ thay đổi fuse sau khi đã đọc cấu hình board thực tế.

## CodeVisionAVR và Proteus

1. Mở project CVAVR đang chạy Prompt 4, giữ ATmega128, clock, cấu hình GLCD/ALCD và Timer1. Thêm **hai source mới** `game/game_menu.c` và `storage/highscore_eeprom.c`; tổng cộng **10 file `.c`**: `main.c`, `drivers/display.c`, `drivers/keypad.c`, `drivers/timer.c`, `drivers/led7seg.c`, `drivers/lcd1602.c`, `game/snake_game.c`, `game/game_render.c`, `game/game_menu.c`, `storage/highscore_eeprom.c`. Không thêm header như source. Các include dùng đường dẫn tương đối từ từng file.
2. Đặt `KEYPAD_TEST_MODE`, `LED7SEG_TEST_MODE`, `LCD1602_TEST_MODE`, `LED7_LCD_TEST_MODE` bằng `0` để chạy game. Giữ ALCD 4-bit: RS=PA0, RW=PA1, E=PA2, D4–D7=PA3–PA6. Kiểm tra phiên bản `alcd.h` đang cài và cấu hình từng chân trước build.
3. Chọn **Project → Build All**. Kiểm tra không có lỗi compiler/linker, phân loại warning, ghi lại Flash/SRAM usage và đường dẫn file Intel HEX mà project xuất ra. Chỉ lấy HEX sau khi build CVAVR thành công; lần kiểm định này không chạy được CVAVR.
4. Trong Proteus, mở schematic gốc, chọn ATmega128, gán HEX vào **Program File**, xác nhận **Clock Frequency** khớp project rồi chạy. Không thay đổi sơ đồ chân.

### Checklist Proteus

1. Reset: Nokia hiện menu, EASY được chọn, LED `0000`, LCD `SELECT LEVEL`/`EASY BEST:0000`, rắn chưa chạy.
2. Phím 8 chuyển EASY→NORMAL→HARD→EASY; phím 2 đi ngược; phím 5 bắt đầu. Giữ phím không làm menu nhảy nhiều lần.
3. EASY/NORMAL/HARD lần lượt di chuyển mỗi 500/300/150 ms theo thời gian mô phỏng; LCD hiện đúng level. Kiểm tra các phím điều hướng, thức ăn, va chạm, Pause/Resume và LED score.
4. Sau Game Over/Win: LCD hiện điểm cuối, Nokia hiện `9:RESTART` và `1:MENU`. Phím 9 giữ level và high score, reset score/rắn/mốc tick. Phím 1 về menu với level vừa chơi và high score tương ứng.
5. Tạo kỷ lục EASY 50, rồi ván EASY 30 (không ghi), rồi EASY 80 (ghi); NORMAL/HARD không đổi. Reset MCU và kiểm tra giá trị được nạp từ EEPROM; kiểm tra cả trường hợp xóa EEPROM và bản ghi CRC sai. Phân biệt reset MCU với dừng/nạp lại mô phỏng vì Proteus có thể xử lý EEPROM khác board thật.
6. Quan sát LCD, Nokia, LED cùng lúc, đặc biệt PA7 và độ nhấp nháy khi render/ghi EEPROM. Kiểm tra lại mọi chức năng Prompt 1–4.

## Kiểm tra logic bằng host

Chạy `sh tests/run_host_validation.sh` trên Ubuntu để kiểm tra logic Snake, menu, định dạng LCD và bản ghi EEPROM bằng mock. Thư mục `tests/mock/` chỉ dành cho host; **không thêm các file test hoặc mock vào project CodeVisionAVR**. Lệnh này không tạo firmware ATmega128 và không thay thế Build All hay Proteus.

## Bring-up board thực

1. **Nguồn:** đo VCC/AVCC/GND của MCU, nguồn LCD và Nokia, mức logic ở chân Nokia; xác minh giới hạn của module Nokia thực tế và mạch chuyển mức trước khi cấp tín hiệu từ MCU. Chip PCD8544 gốc có dải VDD logic 2,7–3,3 V theo [datasheet PCD8544](https://cdn-shop.adafruit.com/datasheets/pcd8544.pdf); không suy ra module thực tế chịu được 5 V nếu chưa kiểm tra mạch trên module. Đo điện trở hạn dòng, dòng từng đoạn/digit và mạch đệm 7SEG-CA.
2. **Clock/reset:** đo thạch anh hoặc nguồn clock, đối chiếu fuse với clock trong project CVAVR. Với 16 MHz và prescaler 64, Timer1 CTC cần OCR1A=249 để tạo tick 1 ms. Không tự thay fuse clock khi chưa xác minh board.
3. **ISP:** kiểm tra sơ đồ header/programmer, nguồn chung và RESET; đối chiếu đường nạp ATmega128 (PDI/PE0, PDO/PE1, SCK/PB1) theo datasheet, đọc device signature, lưu cấu hình fuse hiện tại, nạp HEX và verify Flash. Kiểm tra chip erase/EESAVE và sao lưu EEPROM trước khi nạp bản mới nếu cần giữ kỷ lục. Không giả định MOSI/MISO PB2/PB3 của SPI chạy thường là đường dữ liệu nạp nối tiếp ATmega128.
4. **Ngoại vi:** thử lần lượt Nokia, keypad, LCD 16×2, LED CA bằng các macro test sẵn có; sau đó `LED7_LCD_TEST_MODE=1`, rồi đặt tất cả macro test về `0` và nạp firmware game. Kiểm tra tốc độ ba level, phím, EEPROM qua reset và qua mất nguồn thật.

## Tài nguyên và giới hạn

ATmega128 có Flash 128 KB, SRAM 4 KB, EEPROM 4 KB theo [Microchip](https://www.microchip.com/en-us/product/ATmega128). Các mảng xác định trực tiếp gồm rắn 112×2=224 byte, LCD hai dòng đệm 32+32=64 byte, LED 4 byte và cache kỷ lục ba số 16-bit=6 byte: **298 byte**, chưa kể biến trạng thái, stack và bộ nhớ thư viện GLCD/ALCD. Không có framebuffer Nokia thứ hai trong source. Cần lấy số Flash/SRAM và thông tin stack từ báo cáo CVAVR; không suy ra tổng sử dụng từ các mảng này.

Driver LED chuyển digit khoảng mỗi 2 ms, tương ứng khoảng 125 Hz mỗi digit khi vòng lặp chính không bị chặn. Render GLCD và ghi EEPROM khi kết thúc có thể làm trống LED ngắn hạn; mức nhấp nháy và ghosting phải đo trên Proteus/board. Chưa đo mức điện áp và dòng trên board thật, cũng chưa kiểm chứng phiên bản `alcd.h` trong Windows tác động tới PA7 như thế nào.

## Trạng thái kiểm tra

Đã chạy bộ kiểm tra host C89 trong `tests/` cho khởi tạo/di chuyển/va chạm/Win, menu, định dạng LCD, EEPROM theo level, bản ghi hỏng và sequence quay vòng. Đã kiểm tra cấu trúc DOCX và GPIO trên source/sơ đồ. Đây **không phải** build CodeVisionAVR hoặc mô phỏng Proteus. Ma trận 16 nhóm test, trạng thái PASS/NOT RUN/BLOCKED và rủi ro tồn tại được ghi trong [báo cáo cuối](docs/final_validation_report.md).

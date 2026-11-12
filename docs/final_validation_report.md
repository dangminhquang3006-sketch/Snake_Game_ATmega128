# Báo cáo kiểm định cuối — Snake Game ATmega128

**Ngày kiểm định source:** 2026-11-11 (America/New_York).  
**Mốc Git:** `4781c5591ba212240570e74a320b235dc7843ed7` cộng các thay đổi chưa commit của Prompt 6. Đây chưa phải một bản firmware đã build và chưa có số phiên bản HEX.  
**Công cụ tại đây:** đọc source/DOCX, GCC trên máy chủ để chạy test logic với mock, kiểm tra ZIP/XML DOCX. Không có CodeVisionAVR, Proteus hoặc board ATmega128 trong môi trường thực thi này.  
**Nguồn trạng thái trước Prompt 6:** người dùng báo hệ thống đã có kết quả mô phỏng Proteus; chưa có log hoặc ảnh kết quả của bản source Prompt 6 để xác minh độc lập.

## 1. Tổng quan và module

Firmware gồm `main.c` điều phối `APP_STATE_MENU` và `APP_STATE_GAME`; `game/game_menu.c` quản lý EASY/NORMAL/HARD; `game/snake_game.c` quản lý RUNNING/PAUSED/GAME_OVER/WIN, rắn và điểm; `game/game_render.c` vẽ Nokia; `drivers/display.c`, `keypad.c`, `timer.c`, `led7seg.c`, `lcd1602.c` điều khiển ngoại vi; `storage/highscore_eeprom.c` lưu kỷ lục theo cấp độ. `config/board_config.h` chứa mapping và chu kỳ. Các header tương ứng vẫn giữ API có một nơi khai báo, một nơi định nghĩa. `tests/` là kiểm tra host, không thuộc project CVAVR.

Các chức năng có trong source: menu ba cấp độ; 500/300/150 ms/ô; hướng, chống quay 180° và chống dội phím; thức ăn và tăng 10 điểm/mồi; Pause/Resume; va chạm, Win, Restart, Back to Menu; LED CA hiển thị score; LCD 16×2 hiển thị level/trạng thái/BEST; EEPROM giữ ba kỷ lục. Đây là danh sách tính năng được **triển khai**, chưa phải xác nhận toàn bộ trên board thật.

## 2. Lỗi và thay đổi Prompt 6

| Phát hiện | Ảnh hưởng | Thay đổi | Bằng chứng |
|---|---|---|---|
| `GameLevel` không hợp lệ dạng âm hoặc giá trị lớn có thể lọt qua kiểm tra `level>GAME_LEVEL_HARD` trong API lưu trữ. | Chỉ số mảng `best_scores[]` có thể vượt biên nếu caller truyền enum hỏng. | `highscore_get()` và `highscore_record_result()` kiểm tra `(unsigned int)level>=3u` trước khi lập chỉ số; `game_menu_init()` đưa level không hợp lệ về EASY. | Test host truyền `-1` và `256`: PASS. |
| Chưa có bộ kiểm tra logic lưu trong repo cho lần kiểm định cuối. | Khó lặp lại kiểm tra mảng rắn/EEPROM/LCD sau khi sửa source. | Thêm `tests/run_host_validation.sh`, ba file test và hai mock header chỉ dành cho host. | Chạy script: 3/3 nhóm PASS. |

Không đổi thuật toán Snake, cấu hình Timer1, GPIO keypad, thư viện GLCD/ALCD hoặc driver quét LED vì chưa phát hiện lỗi mới có bằng chứng. Release mặc định khi cả bốn macro `*_TEST_MODE` bằng 0; các vòng lặp test bị loại bởi `#if` khi build release. Header đã chặn bật đồng thời nhiều chế độ test.

## 3. Kiểm tra logic, state và thời gian

Test host dùng chính `snake_game.c`: ba đốt ban đầu (7,4), (6,4), (5,4); food hợp lệ; cấm quay 180° và chỉ một turn/tick; Pause chặn bước; ăn tăng một đốt và 10 điểm; va tường/cắn thân; đi vào ô đuôi vừa rời; Win ở 112 đốt và 1090 điểm. Hàm `snake_step()` trả 0 trong PAUSED/GAME_OVER/WIN. Test Win dùng trạng thái gần đầy lưới dựng trong harness; không phải ván chơi bình thường trên Proteus.

Main chỉ gọi `snake_step()` khi `APP_STATE_GAME` và `SNAKE_RUNNING`; sau Start, Resume, Restart đều đặt lại `last_move_ms`. Ghi High Score nằm trong nhánh `snake_step()` vừa chuyển sang GAME_OVER/WIN; sau đó không còn bước nên không ghi lặp. Phím 1 chỉ chuyển từ trạng thái kết thúc về Menu. Các đường state này đã **rà soát source**, chưa chạy kiểm thử end-to-end trên MCU trong lần kiểm định này.

Timer1 CTC dùng prescaler 64, `OCR1A = F_CPU/64000 - 1`; tại F_CPU=16 MHz kết quả là 249 và tick lý thuyết 1 ms. ISR chỉ tăng `volatile unsigned long system_ms`. `timer_get_ms()` lưu SREG, tắt ngắt ngắn, sao chép 32-bit và khôi phục SREG. Main so sánh hiệu `unsigned long` cho chuyển động, keypad và LED; công thức chịu được tràn 32-bit nếu khoảng đo nhỏ hơn chu kỳ tràn. Tần số CPU/fuse thực tế và chu kỳ tick trên Proteus/board **chưa đo tại đây**.

## 4. GPIO và hiển thị

| Thiết bị | Tín hiệu và GPIO trong source/sơ đồ |
|---|---|
| Nokia 5110 | DC PG0, DIN PG1, CLK PG2, RST PG3, CS PG4, BL PD7 |
| Keypad | ROW1 PC6, ROW2 PE1, ROW3 PD0; COL1 PC4, COL2 PC5, COL3 PE0 |
| LCD 16×2 | RS PA0, RW PA1, E PA2, D4 PA3, D5 PA4, D6 PA5, D7 PA6 |
| LED 7SEG-CA | Digit 1/2/3/4 PF0/PF3/PF4/PF6; A/B/C/D/E/F/G/DP PF1/PF5/PA7/PD1/PD2/PF2/PD3/PF7 |

Các GPIO trên đã đối chiếu với `docs/so_do_chan_mach.jpeg` và mapping hiện tại. PA7 thuộc cùng PORTA với LCD, dù không trùng bit; main tắt digit khi gọi ALCD và khôi phục sau đó. PE0/PE1 dùng cho keypad cũng là PDI/PDO trong chế độ nạp nối tiếp ATmega128, cần kiểm tra mạch ghép và header ISP của board. PD7 đồng thời ghi nhãn Nokia-BL/LCD-BL trong sơ đồ; firmware điều khiển chân này qua `display_init()`. JTAG có thể ảnh hưởng PF4–PF7 nếu fuse/cấu hình không phù hợp. Chưa đo mức điện, cực tính, nhiễu hoặc ghosting trên board.

LED chuyển digit mỗi `LED7_SCAN_MS=2` ms, chu kỳ bốn digit lý thuyết 8 ms, xấp xỉ 125 Hz/digit khi main không bị chặn. Quét trong main, không có GPIO LED trong ISR. Render GLCD, một lần `lcd1602_service()` và ghi EEPROM lúc kết thúc có thể làm gián đoạn quét; chỉ đo bằng Proteus/board mới kết luận được nhấp nháy. LCD có hai bộ đệm 32 ký tự và chỉ ghi một ký tự khác biệt mỗi lượt service; chuỗi dài nhất `NORMAL BEST:0080` vừa 16 ký tự.

## 5. EEPROM và tài nguyên

Hai slot 13 byte trong EEPROM nội chứa magic, version, sequence 16-bit, ba score 16-bit, CRC-8 và commit marker. Khi khởi động, firmware chỉ đọc và chọn slot hợp lệ mới hơn; dữ liệu trống/hỏng cho cache 0 mà không ghi. Khi kết thúc và lập kỷ lục, ghi slot còn lại, commit cuối, rồi đọc xác minh; điểm thấp hơn không ghi. Test host PASS cho ba cấp độ, bản ghi hỏng, commit mất, sequence 65535→0 và level không hợp lệ. Persistence sau reset hoặc chip erase trên Proteus/board **chưa đo tại đây**. Khi nạp lại firmware, cần kiểm tra file `.eep`, thao tác chip erase và fuse EESAVE; không nạp `.eep` vô ý.

ATmega128 có 128 KB Flash, 4 KB SRAM, 4 KB EEPROM theo [Microchip](https://www.microchip.com/en-us/product/ATmega128). Các **mảng dữ liệu xác định** trên target gồm rắn 112×2=224 byte, LCD desired/shown 64 byte, LED digits 4 byte, cache kỷ lục 3×2=6 byte: **298 byte**. Ngoài ra còn biến trạng thái/tick, stack và bộ nhớ riêng của thư viện GLCD/ALCD; không có framebuffer Nokia thứ hai trong source project. Không có số Flash/SRAM/stack từ CVAVR. Phải ghi báo cáo build thực tế và kiểm tra mức stack trước khi nghiệm thu; 298 byte không phải tổng RAM.

## 6. Kết quả đã thực hiện

Quy ước: **PASS** là lệnh test đã chạy và trả kết quả đạt; **FAIL** là đã chạy và lỗi; **NOT RUN** là chưa chạy; **BLOCKED** là chưa chạy do thiếu công cụ/board.

| Kiểm tra | Trạng thái | Bằng chứng và giới hạn |
|---|---|---|
| Host Snake: init, turn, food, pause, va chạm, ô đuôi, Win | PASS | `sh tests/run_host_validation.sh` chạy `tests/host_snake.c` với assert; dùng C compiler trên Ubuntu, không mô phỏng phần cứng. |
| Host menu/EEPROM: level, CRC, slot, dữ liệu lỗi, sequence, index biên | PASS | Cùng script, `tests/host_eeprom.c`; EEPROM là mảng mock trong RAM. |
| Host LCD: 16 ký tự, menu, pause, kết thúc và xóa ký tự cũ | PASS | Cùng script, `tests/host_lcd.c`; ALCD được mock. |
| `git diff --check`, cấu trúc ZIP/XML DOCX và dữ liệu ảnh | PASS | Đã chạy lệnh kiểm tra cục bộ sau khi cập nhật tài liệu. |
| Build All bằng CodeVisionAVR | BLOCKED | Không có compiler Windows/CVAVR tại đây; chưa có HEX/Flash/RAM report cho Prompt 6. |
| Mô phỏng Proteus của đúng source Prompt 6 | BLOCKED | Không có Proteus trong môi trường này. |
| Nạp và đo trên board thật | BLOCKED | Không có board/programmer/thiết bị đo trong môi trường này. |

Các test host không chứng minh tương thích cú pháp CVAVR cho toàn bộ firmware hoặc thời gian thực tế của mạch. Không có test nào được đánh dấu FAIL sau khi sửa guard level; các kiểm tra phần cứng chưa thể kết luận đạt.

## 7. Ma trận 16 nhóm test nghiệm thu

| ID | Bước và tiêu chí trên CVAVR/Proteus/board | Trạng thái tại đây |
|---|---|---|
| 01 Power-on | Reset MCU; Nokia menu, LCD `SELECT LEVEL`, LED `0000`, BEST đọc EEPROM | BLOCKED |
| 02 Menu | Phím 2/8 xoay vòng một lựa chọn/lần nhấn; 5 Start; rắn chưa chạy khi ở Menu | BLOCKED |
| 03 EASY | Đo khoảng cách hai bước ≈500 ms theo thời gian mô phỏng; LCD EASY | BLOCKED |
| 04 NORMAL | Đo khoảng cách hai bước ≈300 ms; LCD NORMAL | BLOCKED |
| 05 HARD | Đo khoảng cách hai bước ≈150 ms; LCD HARD, LED không nhấp nháy rõ | BLOCKED |
| 06 Điều khiển | 2/4/6/8, chặn quay 180°, nhấn nhanh trước tick không nhận hai hướng | BLOCKED |
| 07 Thức ăn | Food trong lưới, không trùng thân; ăn dài +1, điểm +10, food mới hợp lệ | BLOCKED |
| 08 Va chạm | Tường/thân → Game Over; ô đuôi vừa rời khi không ăn vẫn hợp lệ | BLOCKED |
| 09 Pause | Phím 5 dừng/tiếp tục, không có bước dồn; LED/LCD ổn định | BLOCKED |
| 10 Game Over | Giữ score, Nokia/LCD đúng, cập nhật kỷ lục tối đa một lần | BLOCKED |
| 11 Restart | Phím 9, score 0, rắn 3 đốt, level/BEST giữ nguyên | BLOCKED |
| 12 Back to Menu | Phím 1 sau kết thúc, chọn level mới, không chạy dữ liệu ván cũ | BLOCKED |
| 13 Win | Dài 112, score 1090, không sinh food mới, phím 9/1 hoạt động | BLOCKED |
| 14 EEPROM | Ván thấp hơn không ghi; ván cao hơn lưu; reset đọc lại; kiểm tra `.eep`/chip erase | BLOCKED |
| 15 Stress | Ít nhất 20 vòng Restart/Menu/Pause và nhiều ván HARD; không treo/nhấp nháy bất thường | BLOCKED |
| 16 Build regression | Build All 10 source, không lỗi linker/compiler; phân loại warning, ghi Flash/SRAM/stack | BLOCKED |

Test 06–08, 13 đã có phần logic tương ứng PASS trong host test; trạng thái nhóm nghiệm thu vẫn BLOCKED vì chưa chạy ngoại vi/timing Proteus. Test 14 có phần định dạng bản ghi PASS trên RAM mock; tính bền EEPROM thật vẫn BLOCKED.

## 8. Quy trình hoàn tất kiểm định thủ công

1. **CodeVisionAVR:** mở project ATmega128 đã dùng, kiểm tra clock và mapping `glcd.h`/`alcd.h`. Thêm đúng 10 file `.c`: `main.c`, `drivers/display.c`, `keypad.c`, `timer.c`, `led7seg.c`, `lcd1602.c`, `game/snake_game.c`, `game_render.c`, `game_menu.c`, `storage/highscore_eeprom.c`. Không thêm `tests/` hoặc `tests/mock/` vào project/include path. Đặt cả bốn `*_TEST_MODE=0`, chạy Build All, xem lỗi/warning, ghi đường dẫn HEX và Flash/SRAM usage. Giữ bản build và log cho nghiệm thu.
2. **Proteus:** nạp đúng HEX vừa build vào ATmega128 của schematic hiện có; đối chiếu clock, chạy lần lượt ma trận 01–15. Dùng thời gian mô phỏng/logic analyzer thay vì FPS để đo 500/300/150 ms. Theo dõi PA7 khi LCD ghi và PF0/PF3/PF4/PF6 khi quét digit. Với EEPROM, phân biệt reset MCU, dừng/chạy lại simulation và nạp lại firmware; kiểm tra cách Proteus khởi tạo EEPROM image ở mỗi tình huống.
3. **Board:** trước khi cấp nguồn, đo nguồn MCU/Nokia/LCD, giới hạn mức logic PCD8544, điện trở hạn dòng LED và các mạch đệm; xác minh clock, RESET, fuse/JTAG. Kiểm tra pin header ISP theo board; ATmega128 dùng PDI/PE0, PDO/PE1, SCK/PB1 khi nạp nối tiếp, các đường PE0/PE1 đang dùng keypad. Đọc signature, sao lưu EEPROM/fuse, nạp HEX, verify rồi thử lần lượt Nokia → keypad → LCD → LED → kết hợp → firmware game → EEPROM qua mất nguồn. Không thay fuse hoặc đấu 5 V vào Nokia khi chưa xác minh module thật.

## 9. Rủi ro và mức sẵn sàng

- **Rủi ro chưa đo:** điện áp logic Nokia trên board, cấu hình fuse clock/JTAG/EESAVE, cách `alcd.h` thao tác PA7, độ nhấp nháy LED khi GLCD render/EEPROM ghi, sức chứa Flash/SRAM/stack của thư viện CVAVR.
- **Rủi ro EEPROM:** hai slot giữ bản ghi cũ khi slot mới chưa commit, nhưng không chống được lỗi ở cả hai slot hay mất nguồn vượt ngoài giả định; programmer hoặc Proteus có thể nạp lại `.eep`/erase EEPROM.
- **Kết luận:** logic host và cấu trúc source đã qua các kiểm tra nêu trên. Firmware **sẵn sàng để Build All và chạy ma trận Proteus**, chưa đủ bằng chứng để kết luận sẵn sàng nạp/chạy an toàn trên board thật. Sau khi có HEX, log CVAVR, kết quả Proteus và phép đo nguồn/clock, cập nhật các ô BLOCKED bằng kết quả thực tế trước nghiệm thu.

**Nguồn phần cứng tham chiếu:** [ATmega128/L datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/doc2467.pdf), [PCD8544 datasheet](https://cdn-shop.adafruit.com/datasheets/pcd8544.pdf), `docs/so_do_chan_mach.jpeg`.

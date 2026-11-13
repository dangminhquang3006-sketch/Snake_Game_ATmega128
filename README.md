# Snake Game ATmega128 — Prompt 8

Firmware Embedded C cho CodeVisionAVR trên ATmega128. Sửa source bằng VS Code trên Ubuntu; Build All và tạo HEX bằng CodeVisionAVR trên Windows; kiểm tra Proteus trước khi nạp board. Workspace này chưa có kết quả Build All hoặc mô phỏng của source Prompt 8.

## Game Mode và menu

Luồng: **SELECT LEVEL → SELECT MAP → SELECT MODE → GAME**. Trong mỗi menu, phím 2/8 chuyển lựa chọn vòng, phím 5 xác nhận; tại SELECT MODE, phím 1 quay lại SELECT MAP. Sau GAME_OVER, WIN hoặc TIME_UP, phím 9 Restart cùng Level, Map, Mode; phím 1 về SELECT LEVEL. Level EASY/NORMAL/HARD vẫn đi mỗi 500/300/150 ms. Map CLASSIC/PILLARS/LABYRINTH vẫn có 112/104/100 ô chơi được.

| Mode | Thức ăn thường | Bonus Food | Giới hạn lượt |
|---|---|---|---|
| STANDARD | +10 điểm, +1 đốt | Không | Không |
| BONUS | +10 điểm, +1 đốt | +30 điểm, không tăng đốt | Không |
| TIME_ATTACK | +10 điểm, +1 đốt | +30 điểm, không tăng đốt | 60 giây RUNNING |

Bonus xuất hiện ở mốc mỗi ba thức ăn thường, tồn tại 5000 ms thời gian RUNNING, tối đa một bonus. Mốc đạt khi bonus còn hoạt động được bỏ qua, không cộng dồn. Vị trí được chọn bằng hai lượt duyệt ô hợp lệ, loại tường, rắn và thức ăn thường; nếu không còn ô thì bỏ qua. Nếu bonus chiếm ô trống cuối cần cho thức ăn thường, bonus biến mất để thức ăn thường được đặt. Khi Pause, đồng hồ lượt và bonus đều dừng; sau Resume tiếp tục từ giá trị còn lại. Bonus hết hạn ở đầu vòng cập nhật sẽ bị xóa trước khi xét bước di chuyển. Khi đồng hồ về 0, TIME_UP được xử lý trước bước di chuyển; điểm cuối được giữ và xét kỷ lục.

Nokia vẽ rắn đặc 5×5, thức ăn thường đặc 3×3, bonus khung rỗng 5×5 và vật cản viền 6×6. Màn SELECT MODE hiển thị đủ `STANDARD`, `BONUS`, `TIME ATTACK` trong 84×48; dòng dài nhất 13 ký tự ×6=78 pixel. LCD hiển thị kỷ lục của Mode đang chọn. TIME_ATTACK hiển thị `TIME: 01:00` đến `TIME: 00:00`, chỉ thay đổi nội dung khi số giây hiển thị đổi; khi hết giờ hiện `TIME UP!` và score. LED 7 đoạn CA luôn hiển thị score.

## Source

| Module | Vai trò |
|---|---|
| `main.c` | AppState bốn màn hình; cập nhật elapsed trước bước rắn; lưu kết quả một lần |
| `game/game_menu.c/.h` | Menu Level, Map, Mode |
| `game/game_map.c/.h` | Bitmask map Flash, xác thực và truy vấn vật cản |
| `game/game_mode.c/.h` | Mode hiện tại, countdown 60 giây và cấu hình thời gian/điểm Bonus |
| `game/bonus_food.c/.h` | Vị trí, trạng thái và thời gian còn lại của Bonus |
| `game/snake_game.c/.h` | Rắn, thức ăn, va chạm, điểm, WIN và TIME_UP |
| `game/game_render.c/.h` | Nokia gameplay và màn hình kết thúc |
| `storage/highscore_eeprom.c/.h` | 27 kỷ lục Mode × Level × Map; đọc v1/v2, ghi v3 |
| `drivers/lcd1602.c/.h` | Hiển thị menu Mode, countdown, TIME_UP bằng `alcd.h` |

Driver `display`, `keypad`, `timer`, `led7seg` và pin mapping không đổi. Timer1 vẫn tạo tick 1 ms; `timer_get_ms()` đọc biến ISR 32 bit nguyên tử. Main lấy hiệu `unsigned long` để chịu tràn tick, cộng thời gian chỉ khi `SNAKE_RUNNING`; không dùng delay chặn hoặc LCD/GLCD/EEPROM trong ISR. Không có framebuffer Nokia thứ hai.

## EEPROM v1 → v2 → v3

| Phiên bản | Dữ liệu | Slot A / B | Kích thước |
|---|---|---|---:|
| v1 | 3 Level của CLASSIC + STANDARD | Hai biến `hs_slot_a/b` cũ, địa chỉ do linker cấp | 13 byte/slot |
| v2 | 9 Level × Map của STANDARD | `0x0100` / `0x0120` | 25 byte/slot |
| v3 | 27 Mode × Level × Map | `0x0200` / `0x0240` | 61 byte/slot |

V3 dùng `index=mode*9+level*3+map`. Mỗi record gồm magic `53 47`, version `03`, sequence 16 bit LE, 27 score 16 bit LE, CRC-8 tại byte 59 và commit `A5` tại byte 60. Khi khởi động, ưu tiên v3 hợp lệ mới nhất; nếu chưa có v3, 9 score v2 được đưa vào STANDARD; nếu chỉ có v1, ba score được đưa vào STANDARD + CLASSIC. BONUS/TIME_ATTACK chưa có dữ liệu bắt đầu bằng 0 trong RAM. Không ghi EEPROM khi khởi động. Khi kết thúc lượt, chỉ score cao hơn mới được ghi vào slot v3 không hoạt động: xóa commit, ghi dữ liệu và CRC, ghi commit cuối, đọc lại xác minh. V1 và v2 không bị ghi bởi firmware mới.

**Trước khi nạp lên board có kỷ lục:** xác nhận địa chỉ thực tế của hai biến v1 trong EEPROM memory map của đúng project CodeVisionAVR, bảo đảm chúng không chồng `0x0100–0x0138` hoặc `0x0200–0x027C`; source v1 không gán địa chỉ cố định. Sao lưu EEPROM và kiểm tra `.eep`, chip erase, fuse EESAVE. Cú pháp địa chỉ `eeprom ... @0x0200` thuộc [CodeVisionAVR User Manual](https://www.thierry-lequeu.fr/data/CodeVisionAVR-3-20-User-Manual.pdf); cần Build All với phiên bản compiler trên Windows để xác nhận thực tế.

## Build All và kiểm thử Proteus

1. Mở project CVAVR ATmega128 đang chạy Prompt 7. Thêm **hai source mới** `game/game_mode.c`, `game/bonus_food.c`; tổng cộng 13 file `.c` gồm `main.c`, 5 driver (`display`, `keypad`, `timer`, `led7seg`, `lcd1602`), 6 game (`snake_game`, `game_render`, `game_menu`, `game_map`, `game_mode`, `bonus_food`) và `storage/highscore_eeprom.c`. Không thêm `tests/` hoặc mock vào project. Giữ clock, GLCD PCD8544, font `font5x7` và ALCD 4 bit như cũ.
2. Đặt bốn macro chế độ test ngoại vi bằng 0. Chạy **Project → Build All**; kiểm tra lỗi/warning, EEPROM memory map, SRAM/Flash usage và đường dẫn HEX. Chỉ dùng HEX được tạo từ build thành công.
3. Trong Proteus, mở schematic gốc, gán HEX mới vào Program File của ATmega128, kiểm tra Clock Frequency. Thử đủ ba Mode trên mỗi Map/Level: menu 2/8/5/1; STANDARD không bonus; BONUS xuất hiện sau ba mồi, +30 không tăng đốt, hết hạn sau 5 giây RUNNING; Pause trên 5 giây vẫn giữ bonus. TIME_ATTACK bắt đầu 01:00, dừng khi Pause, TIME_UP ở 00:00 trước bước rắn, Restart đặt lại 01:00.
4. Kiểm tra va chạm tường/thân/vật cản, WIN theo map, LCD, LED và hồi quy Prompt 1–7. Kiểm tra EEPROM với image v1 rồi v2, kỷ lục v3 trên nhiều tổ hợp, reset MCU, CRC lỗi và commit chưa hoàn tất. Phân biệt reset MCU với nạp lại image EEPROM trong Proteus.

`sh tests/run_host_validation.sh` chạy các bài kiểm tra C89 bằng mock trên Ubuntu. Kết quả host không chứng minh firmware đã biên dịch bằng CodeVisionAVR hoặc chạy Proteus/board. Báo cáo [giai đoạn 6](docs/final_validation_report.md) là mốc kiểm định của bản base.

## Phần cứng giữ nguyên

Nokia DC/DIN/CLK/RST/CS=PG0/PG1/PG2/PG3/PG4, BL=PD7. Keypad ROW1/2/3=PC6/PE1/PD0, COL1/2/3=PC4/PC5/PE0. LCD RS/RW/E/D4–D7=PA0/PA1/PA2/PA3–PA6. LED CA digit 1–4=PF0/PF3/PF4/PF6; segment A/B/C/D/E/F/G/DP=PF1/PF5/PA7/PD1/PD2/PF2/PD3/PF7. Clock 16 MHz là cấu hình dự kiến; đối chiếu project, Proteus và board.

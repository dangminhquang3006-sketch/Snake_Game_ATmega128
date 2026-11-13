# Snake Game ATmega128 — Prompt 9

Firmware Embedded C cho CodeVisionAVR. Source được sửa trên Ubuntu; Build All và tạo HEX trên Windows bằng CodeVisionAVR; sau đó kiểm tra Proteus trước khi nạp board. Workspace này chưa có kết quả Build All hoặc mô phỏng của đúng source Prompt 9.

## Game Mode và điều khiển

Luồng menu: **SELECT LEVEL → SELECT MAP → SELECT MODE → GAME**. Phím 2/8 đổi lựa chọn, 5 xác nhận; phím 1 tại menu Mode trở về Map. Sau GAME_OVER, WIN hoặc TIME_UP, 9 Restart giữ Level/Map/Mode, 1 về Level. EASY/NORMAL/HARD vẫn di chuyển mỗi 500/300/150 ms. CLASSIC/PILLARS/LABYRINTH vẫn giữ layout tường cố định và lần lượt có 112/104/100 ô ban đầu.

| Mode | Luật chính |
|---|---|
| STANDARD | Thức ăn +10 điểm, +1 đốt; không Bonus, không giới hạn giờ |
| BONUS | Như STANDARD; Bonus +30 điểm, không tăng đốt, tồn tại 5 giây RUNNING |
| TIME_ATTACK | Có Bonus; 60 giây RUNNING, hết giờ chuyển TIME_UP |
| SURVIVAL | Thức ăn +10 điểm, +1 đốt; không Bonus; thử thêm một vật cản mỗi 20 giây RUNNING, tối đa 8 |

Timer1 vẫn tạo tick 1 ms. `timer_get_ms()` đọc tick ISR 32 bit nguyên tử; main dùng hiệu `unsigned long` để chịu tràn. Pause dừng thời gian Bonus, Time Attack và lịch sinh vật cản; Resume không dồn bước. Trong SURVIVAL, bước rắn được xử lý trước, rồi mới thử sinh vật cản nếu game còn RUNNING. Restart xóa tất cả vật cản động và đặt lại lịch 20 giây.

## Dynamic Obstacles

[dynamic_obstacle.c](game/dynamic_obstacle.c) lưu vật cản bằng **8 bitmask 16 bit** (16 byte SRAM); tối đa 8 bit được bật. Khi đến mốc 20 giây RUNNING, module thử tối đa 112 ô theo thứ tự xoay từ chỉ số giả ngẫu nhiên của Snake; mỗi lần cập nhật thêm tối đa một vật cản, nếu không có ứng viên thì bỏ qua mốc đó.

Ứng viên bị loại nếu ngoài lưới, trùng tường cố định/vật cản động/rắn/thức ăn thường/Bonus, cách đầu rắn Manhattan ≤2 hoặc trùng ô đầu sẽ đi tới theo hướng đã xếp hàng. BFS từ đầu rắn dùng hàng đợi tĩnh 112 byte và bitmask visited 16 byte; ứng viên chỉ được nhận nếu mọi ô không thuộc tường vẫn liên thông và thức ăn nằm trong vùng đó. BFS bỏ qua thân rắn vì thân di chuyển; phép kiểm tra này **không bảo đảm** người chơi luôn có đường thoát ở mọi tình huống. BFS chỉ chạy khi xét ứng viên sinh vật cản, không ở ISR hay mỗi frame.

Rắn kiểm tra va chạm tường động trước khi sửa mảng thân. Thuật toán thức ăn hai lượt duyệt loại cả vật cản tĩnh và động. Số ô chơi được hiện tại bằng số ô trống của map gốc trừ số vật cản động; WIN dựa trên số này sau khi ăn, không dùng hằng 112. Vì ứng viên không được đặt lên rắn hoặc thức ăn, việc thêm vật cản không tự tạo WIN giả.

Nokia vẽ tường tĩnh bằng viền 6×6, vật cản động bằng dấu X 5×5, thức ăn thường đặc 3×3, Bonus khung 5×5, rắn đặc 5×5. Menu Mode có bốn dòng ở y=8/18/28/38; dòng dài nhất `> TIME ATTACK` rộng 78 pixel, nằm trong 84×48. LCD SURVIVAL hiển thị `SURVIVAL HARD` / `OBST:03 PLAYING` hoặc `SURVIVAL PAUSED` / `OBSTACLES: 03`; LED CA vẫn hiển thị điểm. Không có framebuffer Nokia thứ hai.

## EEPROM v1 → v4

| Phiên bản | Kỷ lục | Vùng slot A / B | Kích thước |
|---|---:|---|---:|
| v1 | 3 Level | Hai biến `hs_slot_a/b` cũ, địa chỉ do linker cấp | 13 byte/slot |
| v2 | 9 Level × Map | `0x0100` / `0x0120` | 25 byte/slot |
| v3 | 27 Mode × Level × Map | `0x0200` / `0x0240` | 61 byte/slot |
| v4 | 36 Mode × Level × Map | `0x0300` / `0x0350` | 79 byte/slot |

Giá trị Mode cũ giữ nguyên: STANDARD=0, BONUS=1, TIME_ATTACK=2; SURVIVAL=3. Chỉ số cache và record v4: `mode*9 + level*3 + map`, nên 27 kỷ lục v3 nằm ở 27 vị trí đầu. Record v4 có magic `53 47`, version `04`, sequence LE, 36 score LE, CRC-8 byte 77 và commit `A5` byte 78. Khởi động ưu tiên v4 hợp lệ mới nhất; nếu chưa có, đọc v3/v2/v1 và ánh xạ như các phiên bản trước, đặt 9 kỷ lục SURVIVAL bằng 0. Khởi động không ghi EEPROM. Chỉ kỷ lục mới lúc GAME_OVER, WIN hoặc TIME_UP mới được ghi vào slot v4 không hoạt động, commit ghi cuối và đọc lại xác minh. V1/v2/v3 không bị ghi.

**Trước khi nạp lên board có kỷ lục:** kiểm tra EEPROM memory map của đúng project CodeVisionAVR, đặc biệt địa chỉ thực tế của hai biến v1 không có `@`. Xác nhận không chồng v2 (`0x0100–0x0138`), v3 (`0x0200–0x027C`) hoặc v4 (`0x0300–0x039E`). Sao lưu EEPROM; kiểm tra `.eep`, chip erase và EESAVE. Địa chỉ v4 dùng cú pháp `eeprom ... @0x0300` của CodeVisionAVR, nhưng phải xác nhận bằng Build All trên máy Windows đang dùng.

## Cấu trúc source và Build All

| Module | Trách nhiệm |
|---|---|
| `main.c` | AppState, thời gian RUNNING, thứ tự di chuyển rồi sinh vật cản, lưu kỷ lục một lần |
| `game/game_menu.c/.h` | Level, Map và bốn Mode |
| `game/game_map.c/.h` | Layout tường cố định trong Flash |
| `game/game_mode.c/.h` | Mode hiện tại, countdown Time Attack và hằng số thời gian |
| `game/dynamic_obstacle.c/.h` | Lịch 20 giây, bitmask tường động, vùng an toàn, BFS |
| `game/snake_game.c/.h` | Rắn, thức ăn, Bonus, va chạm và WIN |
| `game/game_render.c/.h` | Nokia và ký hiệu vật cản động |
| `storage/highscore_eeprom.c/.h` | Migration v1–v3 và 36 kỷ lục v4 |
| `drivers/lcd1602.c/.h` | Dòng SURVIVAL/số vật cản; chỉ ghi ký tự đổi |

Driver GPIO/GLCD/keypad/LED/Timer1 và pin mapping giữ nguyên. Mở project CodeVisionAVR ATmega128 của Prompt 8, thêm **một source mới** `game/dynamic_obstacle.c` (tổng 14 file `.c`). Giữ clock, cấu hình `glcd.h` PCD8544/font `font5x7` và `alcd.h` 4 bit. Không thêm `tests/` hoặc mock vào project. Đặt mọi macro test ngoại vi về 0, chạy **Project → Build All**, kiểm tra lỗi/warning, EEPROM memory map, SRAM/Flash/stack và đường dẫn Intel HEX. Không dùng HEX cũ nếu build mới chưa đạt.

## Kiểm thử Proteus

1. Mở schematic gốc; gán HEX mới vào Program File ATmega128; kiểm tra Clock Frequency khớp CVAVR. Không đổi dây hoặc chân GPIO.
2. Chọn SURVIVAL trên từng Map và Level. Xác nhận lúc Start không có tường động; sau 20/40/60 giây RUNNING có tối đa một lần thử sinh mỗi mốc, không quá 8. Pause trước mốc, chờ rồi Resume: lịch vẫn tiếp tục từ thời gian RUNNING trước đó.
3. Kiểm tra tường không đè rắn/thức ăn, không ở gần đầu hoặc ngay phía trước, không chia cắt vùng ô trống; đâm tường động phải GAME_OVER. Kiểm tra thức ăn không nằm trên tường và WIN dùng sức chứa đã giảm.
4. Restart xóa tường và đặt lại 20 giây; menu Mode vẫn có STANDARD/BONUS/TIME_ATTACK, các Mode cũ không có tường động. Kiểm tra Nokia, LCD và LED cùng lúc, nhất là HARD + LABYRINTH + SURVIVAL.
5. Sao lưu image EEPROM. Thử đọc v3 có 27 kỷ lục, xác nhận SURVIVAL=0; tạo kỷ lục mới ở các Mode, reset MCU, kiểm tra giữ dữ liệu. Thử một slot CRC lỗi/commit chưa hoàn tất. Phân biệt reset MCU với việc Proteus nạp lại EEPROM image.

`sh tests/run_host_validation.sh` kiểm tra logic C89 bằng mock. Test host không tạo HEX và không thay thế CodeVisionAVR, Proteus hay board thực. GPIO: Nokia PG0–PG4/PD7; keypad PC6/PE1/PD0 và PC4/PC5/PE0; LCD PA0–PA6; LED CA PF0/PF3/PF4/PF6 và các segment theo [board_config.h](config/board_config.h). Clock 16 MHz vẫn cần đối chiếu project và board.

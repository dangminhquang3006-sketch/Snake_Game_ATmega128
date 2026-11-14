# Snake Game ATmega128 — bản chuẩn bị nghiệm thu (Prompt 13)

Firmware Embedded C cho ATmega128 và CodeVisionAVR. Source được sửa trên Ubuntu; Build All và tạo HEX trên Windows bằng CodeVisionAVR; sau đó kiểm tra Proteus trước khi nạp board. Bản source Prompt 13 đã qua kiểm tra host; Build All, Proteus và board thật của đúng bản này chưa được xác minh trong workspace. Kết quả chi tiết ở [báo cáo nghiệm thu mở rộng](docs/final_extended_validation_report.md).

## Tổng quan và cấu hình release

Game dùng Nokia 5110 84×48 (lưới 14×8), keypad 3×3, LCD 16×2, LED 7 đoạn CA, Timer1 tick 1 ms và EEPROM nội. Luồng chọn là Level → Map → Mode → Game. EASY/NORMAL/HARD di chuyển lần lượt 500/300/150 ms/ô. CLASSIC không có tường tĩnh, PILLARS và LABYRINTH có layout riêng. STANDARD chơi thường; BONUS thêm mồi +30; TIME_ATTACK giới hạn 60 giây RUNNING; SURVIVAL thử thêm vật cản mỗi 20 giây RUNNING, tối đa 8.

Phím **2/8** đổi lựa chọn trong menu, **5** xác nhận; **1** quay lại màn trước khi menu hỗ trợ. Khi chơi: **2/4/6/8** đổi hướng, **5** Pause/Resume, **9** Restart. Khi GAME_OVER/WIN/TIME_UP: **9** chơi lại cùng Level/Map/Mode, **1** về menu Level. Bốn macro `KEYPAD_TEST_MODE`, `LED7SEG_TEST_MODE`, `LCD1602_TEST_MODE`, `LED7_LCD_TEST_MODE` mặc định bằng 0 trong `board_config.h`; kiểm tra cả cấu hình macro của project CVAVR để bảo đảm không ghi đè giá trị này khi build release.

**Build trên Windows:** Mở project CodeVisionAVR ATmega128 hiện có; xác nhận đủ 14 source `.c`: `main.c`; `drivers/display.c`, `keypad.c`, `timer.c`, `led7seg.c`, `lcd1602.c`; `game/snake_game.c`, `game_render.c`, `game_menu.c`, `game_map.c`, `game_mode.c`, `bonus_food.c`, `dynamic_obstacle.c`; `storage/highscore_eeprom.c`. Giữ cấu hình PCD8544 trong `glcd.h`, font `font5x7`, LCD 4-bit qua `alcd.h`, include path tương ứng và clock thực của project. Chạy **Project → Build All**, xem Errors/Warnings, báo cáo Flash/SRAM/stack, EEPROM memory map và đường dẫn HEX mới. Không thêm `tests/` hoặc mock vào project firmware.

**Mô phỏng Proteus:** Mở schematic gốc, chọn ATmega128, gán HEX vừa build vào Program File và đối chiếu Clock Frequency. Thử menu, ba loại Fruit, bốn cạnh ở từng Level, Reverse ở HARD, cả ba Map và bốn Mode, Pause/Restart, Nokia/LCD/LED, TIME_UP, vật cản ở mốc 20/40/60 giây RUNNING và kỷ lục sau reset MCU. Kiểm tra EEPROM image được giữ khi reset; phân biệt reset với thao tác nạp lại image hoặc chip erase. Ma trận 36 tổ hợp và các ca chuyên sâu có trong báo cáo nghiệm thu.

**Chuẩn bị board thật:** Kiểm tra nguồn, clock/thạch anh, điện áp và mức logic Nokia, chân GPIO theo schematic, ISP programmer, signature chip, fuse và EESAVE. Sao lưu EEPROM và fuse trước khi nạp; kiểm tra file `.eep` và chế độ chip erase, sau đó nạp và verify HEX. Không thay đổi fuse chỉ dựa trên giả định. Board thật chưa được kiểm thử trong workspace.

**Giới hạn xác minh:** Test host không kiểm tra cú pháp đặc thù CVAVR, thời gian ISR, độ sáng/độ tương phản Nokia, nhiễu GPIO, tốc độ LED multiplexing hay độ bền EEPROM trên thiết bị thật. Các số Flash/SRAM/stack phải lấy từ báo cáo Build All của bản release. EEPROM v4 vẫn lưu 36 kỷ lục; dữ liệu cũ được giữ dù luật tính điểm và độ khó ở NORMAL/HARD đã thay đổi. Chưa phân loại kỷ lục theo phiên bản luật và không tự reset EEPROM.

## Reverse Snake (Prompt 12)

HARD ăn **CIRCLE/SQUARE/STAR** sẽ cộng 10/20/50 điểm, tăng đúng một đốt, rồi đảo thứ tự mảng `segments` tại chỗ nếu ván còn RUNNING. Đầu mới là đuôi cũ; chiều dài, tập tọa độ và điểm không đổi do thao tác đảo. Hướng mới được suy ra từ đầu và đốt cổ sau khi đảo, kể cả khi hai đốt nằm ở hai cạnh đối diện của lưới Wrap Around. `next_direction` được đồng bộ và `turn_queued` xóa; không có bước di chuyển thứ hai trong cùng tick. Nếu vừa WIN thì cập nhật điểm và kết thúc, không đảo.

EASY/NORMAL không đảo. BONUS FOOD chỉ cộng 30 điểm, không tăng đốt và không kích hoạt Reverse. Trong BONUS/TIME_ATTACK, ba loại thức ăn thường vẫn kích hoạt Reverse ở HARD; countdown và tuổi thọ Bonus không reset. Trong SURVIVAL, vật cản và lịch 20 giây được giữ, các lần Safe Spawn sau đó truy vấn vị trí đầu mới. Reverse không tự lái rắn tránh vật cản ở bước tiếp theo. Pause giữ nguyên đầu/hướng; Restart đưa rắn về ba đốt và RIGHT, giữ Level/Map/Mode. EEPROM v4 và 36 kỷ lục giữ nguyên; kỷ lục HARD cũ vẫn tồn tại dù điều kiện chơi đã đổi.

**Kiểm thử Proteus Prompt 12:** Build All project ATmega128 hiện có và nạp HEX mới vào schematic, kiểm tra Clock Frequency. Thử HARD ăn từng Fruit, thân chữ L, rắn qua bốn cạnh, nhiều lần Reverse liên tiếp và phím đổi hướng sau Reverse. Thử Bonus không Reverse; Pause/Resume, Restart, WIN, GAME_OVER, TIME_UP; thử trên CLASSIC/PILLARS/LABYRINTH và STANDARD/BONUS/TIME_ATTACK/SURVIVAL. Theo dõi Nokia, LCD, LED và kỷ lục sau reset MCU. Các thử nghiệm Proteus/board cho đúng source này chưa chạy trong workspace; bộ kiểm tra host C89 đã chạy bằng `sh tests/run_host_validation.sh`.

## Wrap Around (Prompt 11)

EASY giữ luật va chạm biên. NORMAL và HARD xuyên cả bốn cạnh: phải `(13,y)→(0,y)`, trái `(0,y)→(13,y)`, trên `(x,0)→(x,7)`, dưới `(x,7)→(x,0)`. Mỗi tick vẫn đi một ô; hướng và tốc độ 500/300/150 ms không đổi. `main.c` chuyển Level đã chọn cho Snake lúc bắt đầu ván; Restart giữ Level. HARD có thêm Reverse Snake theo mục Prompt 12 ở trên.

Sau khi đổi tọa độ, Snake vẫn kiểm tra tường tĩnh, vật cản động và thân trước khi cập nhật mảng. Đi vào ô đuôi vừa rời được phép nếu không tăng chiều dài. Thức ăn CIRCLE/SQUARE/STAR và Bonus ở cạnh đối diện vẫn được xử lý theo luật cũ. Renderer vẽ từng đốt tại tọa độ ô, nên rắn có thể hiển thị ở cả hai cạnh mà không vẽ đường nối hoặc thêm framebuffer.

Trong SURVIVAL, Safe Spawn dùng khoảng cách Manhattan ngắn nhất qua biên ở NORMAL/HARD; ô đầu sắp tới được lấy từ cùng hàm tính vị trí đã xuyên biên. BFS xem hai cạnh đối diện là kề nhau khi Level cho phép; EASY giữ BFS cũ. Không đổi layout CLASSIC/PILLARS/LABYRINTH, Timer1, GPIO hay EEPROM v4. Kỷ lục NORMAL/HARD cũ được giữ nguyên dù luật di chuyển mới có thể khiến điểm cũ và mới khác điều kiện đạt được.

**Kiểm thử Proteus Prompt 11:** Build All trên CodeVisionAVR, gán HEX mới cho ATmega128 và đối chiếu clock. Ở EASY, thử đi ra cả bốn cạnh để thấy GAME_OVER. Ở NORMAL/HARD, thử cả bốn cạnh và xác nhận đầu xuất hiện đúng phía đối diện, vẫn cùng hướng và không tiến hai ô. Thử ô đích có thân/vật cản, ô đuôi vừa rời, từng loại Multi-Fruit, Bonus, Pause/Resume, Restart, TIME_ATTACK và SURVIVAL. Theo dõi LCD, LED và EEPROM sau reset MCU. Các kiểm thử Proteus này chưa chạy trong workspace; `sh tests/run_host_validation.sh` đã chạy đạt trên host.

## Multi-Fruit (Prompt 10)

Mỗi lần sinh thức ăn thường, PRNG LFSR hiện có chọn một giá trị 0–99: 0–59 là **CIRCLE** (+10), 60–94 là **SQUARE** (+20), 95–99 là **STAR** (+50). Mọi loại đều tăng đúng một đốt, xuất hiện trong cả ba Level và bốn Mode; tốc độ 500/300/150 ms giữ nguyên. Vị trí tiếp tục được chọn từ các ô không có tường cố định, vật cản động, rắn hoặc Bonus đang hoạt động. Phép `%100` trên LFSR 16 bit có sai lệch rất nhỏ vì chu kỳ 65535 không chia hết cho 100; kiểm thử host đo tần suất trên 30000 lần chọn. Seed ban đầu cố định và được trộn với thời điểm phím bắt đầu từ Timer; trong Proteus, thao tác phím giống nhau có thể cho chuỗi giống nhau.

CIRCLE là viền tròn 5×5, SQUARE là khối đặc 3×3, STAR là hình sao 5×5; Bonus vẫn là khung vuông 5×5, +30 và không tăng chiều dài. Ăn STAR vẫn tính là một thức ăn thường cho bộ đếm kích hoạt Bonus sau mỗi ba lần ăn trong BONUS/TIME_ATTACK. SURVIVAL không có Bonus. NORMAL đã có Wrap Around; HARD có cả Wrap Around và Reverse Snake. Phân loại High Score theo phiên bản luật chưa được triển khai trong bản nghiệm thu này.

EEPROM giữ layout v4 và 36 tổ hợp. Bản ghi v1/v2/v3 tiếp tục được đọc theo giới hạn điểm cũ; bản ghi v4 dùng giới hạn bảo thủ theo số ô map: tối đa 50 điểm cho mỗi lần tăng đốt, cộng tối đa 30 điểm cho mỗi ba thức ăn thường ở BONUS/TIME_ATTACK. Ví dụ CLASSIC tối đa 5450 điểm thường, hoặc 6530 khi có Bonus. Điểm cũ vẫn được giữ dù luật tính điểm thay đổi; không tự reset EEPROM. LED 7 đoạn, LCD và màn hình kết thúc Nokia bão hòa ở 9999; điểm trong RAM dùng `unsigned int` và phép cộng có bảo vệ tràn.

**Kiểm thử Proteus Prompt 10:** Build All project ATmega128 hiện có, dùng HEX mới và kiểm tra Clock Frequency. Thử từng Fruit ở EASY/NORMAL/HARD trên CLASSIC/PILLARS/LABYRINTH, xác nhận hình/điểm/tăng một đốt; trong BONUS/TIME_ATTACK kiểm tra STAR và Bonus độc lập, Pause và 60 giây; trong SURVIVAL kiểm tra thức ăn không nằm trên tường động. Restart phải giữ Level/Map/Mode và kỷ lục. Tạo điểm trên 1090, reset MCU và kiểm tra EEPROM v4 đọc lại. Các thử nghiệm Proteus này **chưa chạy** trong workspace; kiểm thử host C89 đã chạy bằng `sh tests/run_host_validation.sh`.

## Game Mode và điều khiển

Luồng menu: **SELECT LEVEL → SELECT MAP → SELECT MODE → GAME**. Phím 2/8 đổi lựa chọn, 5 xác nhận; phím 1 tại menu Mode trở về Map. Sau GAME_OVER, WIN hoặc TIME_UP, 9 Restart giữ Level/Map/Mode, 1 về Level. EASY/NORMAL/HARD vẫn di chuyển mỗi 500/300/150 ms. CLASSIC/PILLARS/LABYRINTH vẫn giữ layout tường cố định và lần lượt có 112/104/100 ô ban đầu.

| Mode | Luật chính |
|---|---|
| STANDARD | Multi-Fruit +10/+20/+50 điểm, +1 đốt; không Bonus, không giới hạn giờ |
| BONUS | Multi-Fruit; Bonus +30 điểm, không tăng đốt, tồn tại 5 giây RUNNING |
| TIME_ATTACK | Có Bonus; 60 giây RUNNING, hết giờ chuyển TIME_UP |
| SURVIVAL | Multi-Fruit, +1 đốt; không Bonus; thử thêm một vật cản mỗi 20 giây RUNNING, tối đa 8 |

Timer1 vẫn tạo tick 1 ms. `timer_get_ms()` đọc tick ISR 32 bit nguyên tử; main dùng hiệu `unsigned long` để chịu tràn. Pause dừng thời gian Bonus, Time Attack và lịch sinh vật cản; Resume không dồn bước. Trong SURVIVAL, bước rắn được xử lý trước, rồi mới thử sinh vật cản nếu game còn RUNNING. Restart xóa tất cả vật cản động và đặt lại lịch 20 giây.

## Dynamic Obstacles

[dynamic_obstacle.c](game/dynamic_obstacle.c) lưu vật cản bằng **8 bitmask 16 bit** (16 byte SRAM); tối đa 8 bit được bật. Khi đến mốc 20 giây RUNNING, module thử tối đa 112 ô theo thứ tự xoay từ chỉ số giả ngẫu nhiên của Snake; mỗi lần cập nhật thêm tối đa một vật cản, nếu không có ứng viên thì bỏ qua mốc đó.

Ứng viên bị loại nếu ngoài lưới, trùng tường cố định/vật cản động/rắn/thức ăn thường/Bonus, cách đầu rắn Manhattan ≤2 hoặc trùng ô đầu sẽ đi tới theo hướng đã xếp hàng. BFS từ đầu rắn dùng hàng đợi tĩnh 112 byte và bitmask visited 16 byte; ứng viên chỉ được nhận nếu mọi ô không thuộc tường vẫn liên thông và thức ăn nằm trong vùng đó. BFS bỏ qua thân rắn vì thân di chuyển; phép kiểm tra này **không bảo đảm** người chơi luôn có đường thoát ở mọi tình huống. BFS chỉ chạy khi xét ứng viên sinh vật cản, không ở ISR hay mỗi frame.

Rắn kiểm tra va chạm tường động trước khi sửa mảng thân. Thuật toán thức ăn hai lượt duyệt loại cả vật cản tĩnh và động. Số ô chơi được hiện tại bằng số ô trống của map gốc trừ số vật cản động; WIN dựa trên số này sau khi ăn, không dùng hằng 112. Vì ứng viên không được đặt lên rắn hoặc thức ăn, việc thêm vật cản không tự tạo WIN giả.

Nokia vẽ tường tĩnh bằng viền 6×6, vật cản động bằng dấu X 5×5, ba thức ăn thường theo hình CIRCLE/SQUARE/STAR, Bonus khung 5×5, rắn đặc 5×5. Menu Mode có bốn dòng ở y=8/18/28/38; dòng dài nhất `> TIME ATTACK` rộng 78 pixel, nằm trong 84×48. LCD SURVIVAL hiển thị `SURVIVAL HARD` / `OBST:03 PLAYING` hoặc `SURVIVAL PAUSED` / `OBSTACLES: 03`; LED CA vẫn hiển thị điểm. Không có framebuffer Nokia thứ hai.

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

Driver GPIO/GLCD/keypad/LED/Timer1 và pin mapping giữ nguyên. Project CodeVisionAVR hiện có cần đủ **14 source `.c`** như danh sách ở đầu README; Prompt 13 không thêm source firmware mới. Giữ clock, cấu hình `glcd.h` PCD8544/font `font5x7` và `alcd.h` 4 bit. Không thêm `tests/` hoặc mock vào project. Đặt mọi macro test ngoại vi về 0, chạy **Project → Build All**, kiểm tra lỗi/warning, EEPROM memory map, SRAM/Flash/stack và đường dẫn Intel HEX. Không dùng HEX cũ nếu build mới chưa đạt.

## Kiểm thử Proteus

1. Mở schematic gốc; gán HEX mới vào Program File ATmega128; kiểm tra Clock Frequency khớp CVAVR. Không đổi dây hoặc chân GPIO.
2. Chọn SURVIVAL trên từng Map và Level. Xác nhận lúc Start không có tường động; sau 20/40/60 giây RUNNING có tối đa một lần thử sinh mỗi mốc, không quá 8. Pause trước mốc, chờ rồi Resume: lịch vẫn tiếp tục từ thời gian RUNNING trước đó.
3. Kiểm tra tường không đè rắn/thức ăn, không ở gần đầu hoặc ngay phía trước, không chia cắt vùng ô trống; đâm tường động phải GAME_OVER. Kiểm tra thức ăn không nằm trên tường và WIN dùng sức chứa đã giảm.
4. Restart xóa tường và đặt lại 20 giây; menu Mode vẫn có STANDARD/BONUS/TIME_ATTACK, các Mode cũ không có tường động. Kiểm tra Nokia, LCD và LED cùng lúc, nhất là HARD + LABYRINTH + SURVIVAL.
5. Sao lưu image EEPROM. Thử đọc v3 có 27 kỷ lục, xác nhận SURVIVAL=0; tạo kỷ lục mới ở các Mode, reset MCU, kiểm tra giữ dữ liệu. Thử một slot CRC lỗi/commit chưa hoàn tất. Phân biệt reset MCU với việc Proteus nạp lại EEPROM image.

`sh tests/run_host_validation.sh` kiểm tra logic C89 bằng mock. Test host không tạo HEX và không thay thế CodeVisionAVR, Proteus hay board thực. GPIO: Nokia PG0–PG4/PD7; keypad PC6/PE1/PD0 và PC4/PC5/PE0; LCD PA0–PA6; LED CA PF0/PF3/PF4/PF6 và các segment theo [board_config.h](config/board_config.h). Clock 16 MHz vẫn cần đối chiếu project và board.

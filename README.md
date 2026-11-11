# Snake Game ATmega128 — giai đoạn 4

Firmware Embedded C cho **CodeVisionAVR**. Game hiện có rắn, thức ăn, điểm, Pause/Resume, Game Over, Win và Restart; giai đoạn 4 bổ sung LED 7 đoạn CA hiển thị điểm và LCD 16×2 hiển thị cấp độ/trạng thái. Chưa có EEPROM hoặc menu chọn cấp độ.

## Phần cứng và source

Giữ nguyên cấu hình Nokia PCD8544 đã kiểm thử: `glcd.h`, `font5x7`, 84×48 pixel; DC=PG0, DIN=PG1, CLK=PG2, RST=PG3, CS=PG4, BL=PD7. Keypad theo ảnh `docs/so_do_chan_ban_phim.jpeg`: ROW-1=PC6/41, ROW-2=PE1/3, ROW-3=PD0/25; COL-1=PC4/39, COL-2=PC5/40, COL-3=PE0/2. Ba cột là OUTPUT; ba hàng là INPUT có pull-up nội. Driver quét từng cột LOW, chống dội 20 ms và phát một sự kiện nhấn mới. Ảnh schematic tổng thể cũ ghi COL-1/COL-2 khác ảnh bàn phím; kết quả kiểm thử keypad trên Proteus theo thông tin người dùng là nguồn xác nhận cho mapping hiện tại. Không thay đổi chân trong giai đoạn này.

| File | Trách nhiệm |
|---|---|
| `main.c` | Khởi tạo, đọc phím, lập lịch bước 500 ms, vẽ khi trạng thái thay đổi |
| `config/board_config.h` | Chân GPIO và kích thước 14×8, bước lưới 6 pixel |
| `drivers/display.c/.h` | Bao bọc GLCD và font hiện có; không có framebuffer thứ hai |
| `drivers/keypad.c/.h` | Quét phím; thêm `KEY_RESTART` cho phím 9 |
| `drivers/timer.c/.h` | Timer1 CTC, tick 1 ms và đọc thời gian nguyên tử; không thay đổi |
| `drivers/led7seg.c/.h` | Quét 4 digit của cụm 7SEG-CA bằng GPIO, hiển thị điểm 0000–9999 |
| `drivers/lcd1602.c/.h` | Wrapper `alcd.h`, ghi tối đa một ký tự mỗi lần service |
| `game/snake_game.c/.h` | Toàn bộ dữ liệu và quy tắc trò chơi, độc lập GLCD/GPIO |
| `game/game_render.c/.h` | Vẽ rắn, thức ăn, Pause, Game Over, Win và điểm cuối |

## Logic trò chơi

Lưới có **14×8 = 112 ô**. `SnakePoint` gồm hai tọa độ 8-bit. Mảng tĩnh chứa tối đa 112 đốt (224 byte tọa độ), biến chiều dài chỉ ra phần đang dùng. Rắn bắt đầu với ba đốt ở (7,4), (6,4), (5,4), hướng RIGHT và điểm 0. Mỗi đốt được tô 5×5 pixel trên bước 6 pixel; thức ăn là ô 3×3 pixel đặt lệch (1,1) trong ô logic.

Thức ăn được chọn bằng **hai lượt duyệt ô trống**: đếm ô chưa bị rắn chiếm, lấy một chỉ số từ LFSR 16-bit, rồi duyệt lại để chọn ô tương ứng. Không có vòng thử ngẫu nhiên vô hạn hay mảng ô trống phụ. LFSR là số **giả ngẫu nhiên**; sau reset Proteus có thể lặp lại vị trí thức ăn đầu tiên. Thời điểm nhấn phím được trộn vào trạng thái để các lần Restart có khả năng khác nhau, không bảo đảm entropy thật.

Mỗi bước: áp dụng tối đa một hướng đã được chấp nhận; tính đầu mới bằng số nguyên có dấu; kiểm tra biên; xác định có ăn mồi; kiểm tra thân **kể cả đuôi nếu ăn**, **bỏ qua đuôi cũ nếu không ăn**; sau đó mới dịch mảng. Khi ăn, dịch từ vị trí `length` về 1 để giữ đuôi cũ, tăng đúng một đốt và cộng 10 điểm. Điểm tối đa của ván trên lưới này là 1090, phù hợp `unsigned int` của CVAVR. Nếu dài 112 đốt, chuyển `SNAKE_WIN` và không sinh thức ăn nữa.

Các trạng thái: `SNAKE_RUNNING`, `SNAKE_PAUSED`, `SNAKE_GAME_OVER`, `SNAKE_WIN`. Va tường hoặc thân chuyển Game Over và giữ điểm. Phím 5 Pause/Resume; phím 9 Restart được nhận cả khi đang chạy, Pause, Game Over hoặc Win. Restart đặt lại rắn/điểm/thức ăn/hướng và mốc thời gian, nên không di chuyển dồn bước. Giữ phím 9 không Restart liên tục nhờ sự kiện nhấn mới của keypad. Khi kết thúc, Nokia hiện `GAME OVER` hoặc `YOU WIN`, `SCORE: 0000` và `PRESS 9`; màn hình chơi dùng toàn bộ lưới nên không hiện điểm liên tục.

## CodeVisionAVR và Proteus

1. Trên Windows 11, mở project CodeVisionAVR **đang chạy tốt**, chọn ATmega128 và giữ clock/cấu hình GLCD/Timer1 hiện tại. Project cần tám source: `main.c`, `drivers/display.c`, `drivers/keypad.c`, `drivers/timer.c`, **`drivers/led7seg.c`, `drivers/lcd1602.c`**, `game/snake_game.c`, `game/game_render.c`. Không thêm header như source và không thêm Nokia driver cũ. Các include hiện dùng đường dẫn tương đối từ file nguồn.
2. `KEYPAD_TEST_MODE=0` (mặc định) để chạy game. Đặt thành `1` trong `board_config.h` hoặc project defines để kiểm tra 9 phím độc lập; sau đó build lại. Khi bật test mode, Nokia hiện phím, hàng/cột và số sự kiện nhấn.
3. Trong **Project → Configure → C Compiler → Libraries → Alphanumeric LCD**, bật `alcd.h`, chọn 16 ký tự/dòng và gán `RS=PA0`, `RW/RD=PA1`, `E/EN=PA2`, `D4=PA3`, `D5=PA4`, `D6=PA5`, `D7=PA6`. Các API `lcd_init(16)`, `lcd_gotoxy`, `lcd_putchar` đã được đối chiếu với [manual CodeVisionAVR do HP InfoTech viết](https://www.thierry-lequeu.fr/data/CodeVisionAVR-3-20-User-Manual.pdf); cần kiểm tra phiên bản CVAVR đang cài có cấu hình được từng chân này, và kiểm tra `alcd.h` có ghi toàn PORTA/DDRA làm thay đổi PA7 hay không.
4. Chạy **Project → Build All**. Chỉ khi CVAVR báo thành công mới lấy file Intel HEX từ đường dẫn output của project. Mã giai đoạn 4 chưa được biên dịch bằng CVAVR trong môi trường này.
5. Trong Proteus, mở schematic gốc, gán HEX vào **Program File** của ATmega128, kiểm tra **Clock Frequency** khớp project, rồi chạy mô phỏng. Không cần sửa wiring.

### Checklist mô phỏng

1. Khởi động: ba đốt đúng vị trí, một thức ăn 3×3 không trùng thân, điểm logic bằng 0. Rắn tự đi mỗi 500 ms.
2. Phím 2/4/6/8 đổi hướng hợp lệ; nhấn hướng ngược trực tiếp hoặc hai hướng nhanh trước một tick không làm quay 180°. Phím 5 Pause/Resume, không có bước dồn sau Resume.
3. Dẫn đầu tới thức ăn: mỗi lần ăn dài thêm đúng một đốt, thức ăn cũ mất và thức ăn mới ở ô trống. Sau ba lần ăn: sáu đốt, điểm logic 30 và LED hiện `0030`.
4. Đâm tường hoặc cắn thân: hiện `GAME OVER`, giữ điểm và dừng. Đi vào ô **đuôi vừa rời** khi không ăn phải hợp lệ; khi ăn thì đuôi vẫn được tính là chiếm chỗ.
5. Nhấn 9 khi chạy/Pause/Game Over/Win: rắn về ba đốt, điểm 0, hướng RIGHT, thức ăn hợp lệ và đợi đủ 500 ms trước bước đầu. Giữ phím 9 không lặp Restart.
6. Điều kiện Win ở 112 ô khó đạt bằng tay: dùng test harness logic kiểm soát dữ liệu hoặc một build thử riêng gần đầy lưới. Mong đợi `YOU WIN`, điểm cuối và không sinh thêm thức ăn.
7. Hồi quy: Nokia, keypad, Timer1, khe 1 pixel giữa đốt, và các GPIO vẫn hoạt động như trước.

## Kết quả kiểm tra và giới hạn

Người dùng xác nhận Prompt 1–3 đã chạy trên CVAVR/Proteus. **Thay đổi Prompt 4 chưa được build bằng CodeVisionAVR, chưa mô phỏng Proteus và chưa chạy trên board thực**. Cần ghi Flash/SRAM usage từ báo cáo build trước khi nạp board. Kết quả kiểm tra tĩnh của mã mới không thay thế được các bước đó.

## Tích hợp LED 7 đoạn và LCD 16×2

Các net label dưới đây được đối chiếu với **sơ đồ chân chính thức** `docs/so_do_chan_mach.jpeg`; cực tính điện thực tế của mạch chọn digit/segment vẫn cần xác nhận trên Proteus. Snake chỉ điều khiển cụm **7SEG-CA**, không dùng 7SEG-CC.

| 7SEG-CA | GPIO (chân vật lý) | 7SEG-CA | GPIO (chân vật lý) |
|---|---|---|---|
| Digit 1 | PF0 (61) | A | PF1 (60) |
| Digit 2 | PF3 (58) | B | PF5 (56) |
| Digit 3 | PF4 (57) | C | PA7 (44) |
| Digit 4 | PF6 (55) | D | PD1 (26) |
| DP | PF7 (54) | E | PD2 (27) |
| F | PF2 (59) | G | PD3 (28) |

PF6 là Digit 4 và PF7 là DP theo net label. LCD dùng RS=PA0 (51), RW=PA1 (50), E=PA2 (49), D4=PA3 (48), D5=PA4 (47), D6=PA5 (46), D7=PA6 (45). Không có chân trùng với Nokia/keypad; **PA7 và LCD cùng dùng Port A**. Driver LED dùng mặt nạ trên `PORTA`, `PORTD`, `PORTF` và `DDRx`; không dùng macro `PORTF.0` gây lỗi parser CVAVR. Trong main, các digit được tắt trước mỗi lần ghi `alcd.h`; driver khôi phục PA7 sau ghi. Vì quét LED chỉ diễn ra ở main, không có ISR LED chen giữa thao tác của ALCD. Chưa có bằng chứng phiên bản `alcd.h` đang cài bảo toàn PA7; kiểm tra đoạn C và LCD cùng lúc trên Proteus. Nếu thấy nhiễu, kiểm tra `PORTA`/`DDRA` bằng logic analyzer, đối chiếu file cấu hình thư viện CVAVR và điều chỉnh phần mềm theo wiring hiện tại. PF4–PF7 còn liên quan JTAG trên ATmega128; nếu LED tương ứng không sáng, kiểm tra fuse JTAGEN/cấu hình JTD trước khi kết luận sai mapping.

Mặc định `LED7_DIGIT_ON_LEVEL=1`, `LED7_SEGMENT_ON_LEVEL=0` theo điều khiển CA trực tiếp. Đây là **giả thuyết mức kích hoạt cần xác minh** vì ảnh không cho thấy đầy đủ tầng transistor/đệm. Nếu các digit hoặc segment bị đảo, sửa đúng macro mức kích hoạt trong `board_config.h` sau khi đo tín hiệu, không đổi GPIO. `LED7_SCAN_MS=2` chuyển một digit mỗi 2 ms, tương ứng khoảng 125 Hz cho mỗi digit khi vòng lặp không bị chặn. Việc render Nokia và ghi từng ký tự LCD có thể làm tăng khoảng cách giữa hai lần quét; cần nhìn trực tiếp trên Proteus và board để đánh giá nhấp nháy. Mã không cấu hình lại Timer1 và không ghi LCD/GLCD trong ISR.

### Chế độ kiểm thử

Chỉ bật **một** macro test tại một thời điểm trong `config/board_config.h` hoặc project defines; build lại sau mỗi lần đổi:

1. `LED7SEG_TEST_MODE=1`: LED lần lượt hiện `0000`, `1234`, `5678`, `9999`, mỗi mẫu 2 giây. Kiểm tra thứ tự digit, cực tính, đoạn C/DP, nhấp nháy và ghosting. Chế độ này không khởi tạo GLCD/LCD/game.
2. `LCD1602_TEST_MODE=1`: LCD lần lượt hiện `SNAKE GAME`/`STATUS: READY`, `LEVEL: EASY`/`STATUS: PLAYING`, `SNAKE GAME`/`GAME PAUSED`, `GAME OVER`/`SCORE: 0050`, mỗi mẫu 2 giây. Nokia hiện `LCD TEST`. Kiểm tra 2 dòng và ký tự cũ được xóa bằng dấu cách.
3. `LED7_LCD_TEST_MODE=1`: LCD giữ `SNAKE GAME`/`STATUS: READY`, LED luân phiên `1234`, `5678`, `9090`, mỗi mẫu 2 giây. Kiểm tra segment C ở PA7 và các ký tự LCD trong khi LED quét; dùng logic analyzer tại PA7, E và chân digit nếu có nhiễu.
4. Đặt cả ba macro trên về `0` để chạy game và kiểm tra hai ngoại vi đồng thời. LED hiện score `0000` lúc khởi động/Restart, `0010` sau một lần ăn, giữ nguyên khi Pause/Game Over/Win. LCD hiện EASY/PLAYING, `GAME PAUSED` khi tạm dừng, hoặc trạng thái kết thúc cùng điểm số.

Không bật `KEYPAD_TEST_MODE` đồng thời với hai macro trên. Điểm LED được giới hạn ở `9999` để không tràn hiển thị; game hiện chỉ có tối đa 1090 điểm. Các chuỗi LCD nằm trong Flash; bộ đệm trạng thái LCD dùng 64 byte SRAM và bộ đệm LED dùng 4 byte, ngoài dữ liệu game/GLCD hiện có.

# Snake Game ATmega128 — giai đoạn 3

Firmware Embedded C cho **CodeVisionAVR**. Giai đoạn này có rắn tự di chuyển, bàn phím, thức ăn, tăng chiều dài, điểm số trong logic, Pause/Resume, Game Over, Win và Restart. Chưa điều khiển LED 7 đoạn, LCD 16×2, EEPROM hoặc menu cấp độ.

## Phần cứng và source

Giữ nguyên cấu hình Nokia PCD8544 đã kiểm thử: `glcd.h`, `font5x7`, 84×48 pixel; DC=PG0, DIN=PG1, CLK=PG2, RST=PG3, CS=PG4, BL=PD7. Keypad theo ảnh `docs/so_do_chan_ban_phim.jpeg`: ROW-1=PC6/41, ROW-2=PE1/3, ROW-3=PD0/25; COL-1=PC4/39, COL-2=PC5/40, COL-3=PE0/2. Ba cột là OUTPUT; ba hàng là INPUT có pull-up nội. Driver quét từng cột LOW, chống dội 20 ms và phát một sự kiện nhấn mới. Ảnh schematic tổng thể cũ ghi COL-1/COL-2 khác ảnh bàn phím; kết quả kiểm thử keypad trên Proteus theo thông tin người dùng là nguồn xác nhận cho mapping hiện tại. Không thay đổi chân trong giai đoạn này.

| File | Trách nhiệm |
|---|---|
| `main.c` | Khởi tạo, đọc phím, lập lịch bước 500 ms, vẽ khi trạng thái thay đổi |
| `config/board_config.h` | Chân GPIO và kích thước 14×8, bước lưới 6 pixel |
| `drivers/display.c/.h` | Bao bọc GLCD và font hiện có; không có framebuffer thứ hai |
| `drivers/keypad.c/.h` | Quét phím; thêm `KEY_RESTART` cho phím 9 |
| `drivers/timer.c/.h` | Timer1 CTC, tick 1 ms và đọc thời gian nguyên tử; không thay đổi |
| `game/snake_game.c/.h` | Toàn bộ dữ liệu và quy tắc trò chơi, độc lập GLCD/GPIO |
| `game/game_render.c/.h` | Vẽ rắn, thức ăn, Pause, Game Over, Win và điểm cuối |

## Logic trò chơi

Lưới có **14×8 = 112 ô**. `SnakePoint` gồm hai tọa độ 8-bit. Mảng tĩnh chứa tối đa 112 đốt (224 byte tọa độ), biến chiều dài chỉ ra phần đang dùng. Rắn bắt đầu với ba đốt ở (7,4), (6,4), (5,4), hướng RIGHT và điểm 0. Mỗi đốt được tô 5×5 pixel trên bước 6 pixel; thức ăn là ô 3×3 pixel đặt lệch (1,1) trong ô logic.

Thức ăn được chọn bằng **hai lượt duyệt ô trống**: đếm ô chưa bị rắn chiếm, lấy một chỉ số từ LFSR 16-bit, rồi duyệt lại để chọn ô tương ứng. Không có vòng thử ngẫu nhiên vô hạn hay mảng ô trống phụ. LFSR là số **giả ngẫu nhiên**; sau reset Proteus có thể lặp lại vị trí thức ăn đầu tiên. Thời điểm nhấn phím được trộn vào trạng thái để các lần Restart có khả năng khác nhau, không bảo đảm entropy thật.

Mỗi bước: áp dụng tối đa một hướng đã được chấp nhận; tính đầu mới bằng số nguyên có dấu; kiểm tra biên; xác định có ăn mồi; kiểm tra thân **kể cả đuôi nếu ăn**, **bỏ qua đuôi cũ nếu không ăn**; sau đó mới dịch mảng. Khi ăn, dịch từ vị trí `length` về 1 để giữ đuôi cũ, tăng đúng một đốt và cộng 10 điểm. Điểm tối đa của ván trên lưới này là 1090, phù hợp `unsigned int` của CVAVR. Nếu dài 112 đốt, chuyển `SNAKE_WIN` và không sinh thức ăn nữa.

Các trạng thái: `SNAKE_RUNNING`, `SNAKE_PAUSED`, `SNAKE_GAME_OVER`, `SNAKE_WIN`. Va tường hoặc thân chuyển Game Over và giữ điểm. Phím 5 Pause/Resume; phím 9 Restart được nhận cả khi đang chạy, Pause, Game Over hoặc Win. Restart đặt lại rắn/điểm/thức ăn/hướng và mốc thời gian, nên không di chuyển dồn bước. Giữ phím 9 không Restart liên tục nhờ sự kiện nhấn mới của keypad. Khi kết thúc, Nokia hiện `GAME OVER` hoặc `YOU WIN`, `SCORE: 0000` và `PRESS 9`; màn hình chơi dùng toàn bộ lưới nên không hiện điểm liên tục.

## CodeVisionAVR và Proteus

1. Trên Windows 11, mở project CodeVisionAVR **đang chạy tốt**, chọn ATmega128 và giữ clock/cấu hình GLCD/Timer1 hiện tại. Project cần sáu source: `main.c`, `drivers/display.c`, `drivers/keypad.c`, `drivers/timer.c`, `game/snake_game.c`, `game/game_render.c`. Không thêm header như source và không thêm Nokia driver cũ. Các include hiện dùng đường dẫn tương đối từ file nguồn.
2. `KEYPAD_TEST_MODE=0` (mặc định) để chạy game. Đặt thành `1` trong `board_config.h` hoặc project defines để kiểm tra 9 phím độc lập; sau đó build lại. Khi bật test mode, Nokia hiện phím, hàng/cột và số sự kiện nhấn.
3. Chạy **Project → Build All**. Chỉ khi CVAVR báo thành công mới lấy file Intel HEX từ đường dẫn output của project. Mã giai đoạn 3 chưa được biên dịch bằng CVAVR trong môi trường này.
4. Trong Proteus, mở schematic gốc, gán HEX vào **Program File** của ATmega128, kiểm tra **Clock Frequency** khớp project, rồi chạy mô phỏng. Không cần sửa wiring.

### Checklist mô phỏng

1. Khởi động: ba đốt đúng vị trí, một thức ăn 3×3 không trùng thân, điểm logic bằng 0. Rắn tự đi mỗi 500 ms.
2. Phím 2/4/6/8 đổi hướng hợp lệ; nhấn hướng ngược trực tiếp hoặc hai hướng nhanh trước một tick không làm quay 180°. Phím 5 Pause/Resume, không có bước dồn sau Resume.
3. Dẫn đầu tới thức ăn: mỗi lần ăn dài thêm đúng một đốt, thức ăn cũ mất và thức ăn mới ở ô trống. Sau ba lần ăn: sáu đốt, điểm logic 30. Điểm chỉ hiện trên màn hình kết thúc trong giai đoạn này.
4. Đâm tường hoặc cắn thân: hiện `GAME OVER`, giữ điểm và dừng. Đi vào ô **đuôi vừa rời** khi không ăn phải hợp lệ; khi ăn thì đuôi vẫn được tính là chiếm chỗ.
5. Nhấn 9 khi chạy/Pause/Game Over/Win: rắn về ba đốt, điểm 0, hướng RIGHT, thức ăn hợp lệ và đợi đủ 500 ms trước bước đầu. Giữ phím 9 không lặp Restart.
6. Điều kiện Win ở 112 ô khó đạt bằng tay: dùng test harness logic kiểm soát dữ liệu hoặc một build thử riêng gần đầy lưới. Mong đợi `YOU WIN`, điểm cuối và không sinh thêm thức ăn.
7. Hồi quy: Nokia, keypad, Timer1, khe 1 pixel giữa đốt, và các GPIO vẫn hoạt động như trước.

## Kết quả kiểm tra và giới hạn

Đã rà soát toàn bộ source và DOCX kiến trúc; phần đặc tả thức ăn, ô đuôi, Game Over, Win và pin keypad trong DOCX phù hợp giai đoạn này nên không sửa file Word. Đã kiểm tra logic bằng chương trình thử tạm trên máy chủ: khởi tạo/thức ăn hợp lệ, tăng trưởng/điểm, tường, cắn thân, ô đuôi vừa rời, Restart và Win 112 ô. Đây **không phải** bản build firmware CodeVisionAVR. Người dùng đã báo Prompt 1/2 chạy trên CVAVR/Proteus; **thay đổi Prompt 3 chưa được build hoặc mô phỏng** tại đây. Chưa kiểm tra board thực và chưa xác minh chất lượng ngẫu nhiên trên phần cứng.

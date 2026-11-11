# Snake Game ATmega128 — giai đoạn 1

Firmware Embedded C cho CodeVisionAVR. Giai đoạn này chỉ kiểm tra Nokia 5110; các driver game, keypad, Timer1, LED 7 đoạn, LCD 16×2 và EEPROM chưa được triển khai.

## Tài liệu đã xem

- `docs/so_do_chan.jpeg`: sơ đồ chân ATmega128 in trên tài liệu board. Có thể đọc tên net và chân.
- `docs/mach_mo_phong.png`: ảnh chụp schematic Proteus ở mức tổng thể. Xác nhận bố trí và nhãn Nokia; ảnh không đủ nét để kiểm tra mọi dây, cực tính LED hoặc cấu hình clock.
- `docs/mach_that.jpeg`: ảnh board trong tài liệu, không cho biết chi tiết điện của từng net.
- `docs/Mo_ta_de_tai_Snake_Game_ATmega128.docx`: đã trích và đọc nội dung Word; tài liệu nêu chức năng, không cung cấp GPIO chi tiết.

Không có file project Proteus có thể mở và kiểm tra netlist trực tiếp. `VERIFIED` bên dưới nghĩa là đọc được từ sơ đồ chân/nhãn trong ảnh cung cấp, **chưa phải** đã đo hoặc chạy mô phỏng.

## GPIO mapping

| Ngoại vi | Tín hiệu | GPIO | Trạng thái | Bằng chứng/ghi chú |
|---|---|---|---|---|
| Nokia 5110 | DC | PC0 | VERIFIED | `so_do_chan.jpeg`, pin 33 |
| Nokia 5110 | DIN | PG1 | VERIFIED | pin 34 |
| Nokia 5110 | CLK | PG2 | VERIFIED | pin 43 |
| Nokia 5110 | RST | PG3 | VERIFIED | pin 18; **khác bảng dự kiến của đề bài** |
| Nokia 5110 | CE/CS | PG4 | VERIFIED | pin 19; **khác bảng dự kiến của đề bài** |
| Nokia 5110 | BL | PD7 | VERIFIED | pin 32; net chung LCD-BL |
| Keypad 3×3 | ROW-1 | PC5 | VERIFIED | pin 40 |
| Keypad 3×3 | ROW-2 | PE1 | VERIFIED | pin 3; net chung BLUE-RX |
| Keypad 3×3 | ROW-3 | PD0 | VERIFIED | pin 25; net chung ENC-R |
| Keypad 3×3 | COL-1 | PC3 | VERIFIED | pin 38 |
| Keypad 3×3 | COL-2 | PC4 | VERIFIED | pin 39 |
| Keypad 3×3 | COL-3 | PE0 | VERIFIED | pin 2; net chung BLUE-TX |
| LED 7 đoạn CA | A | PF1 | VERIFIED | pin 60 |
| LED 7 đoạn CA | B | PF5 | VERIFIED | pin 56 |
| LED 7 đoạn CA | C | PA7 | VERIFIED | pin 44; net `7SLED-C` |
| LED 7 đoạn CA | D | PD1 | VERIFIED | pin 26 |
| LED 7 đoạn CA | E | PD2 | VERIFIED | pin 27 |
| LED 7 đoạn CA | F | PF2 | VERIFIED | pin 59 |
| LED 7 đoạn CA | G | PD3 | VERIFIED | pin 28 |
| LED 7 đoạn CA | DP | PF7 | VERIFIED | pin 54 |
| LED 7 đoạn CA | digit 1 | PF0 | VERIFIED | pin 61 |
| LED 7 đoạn CA | digit 2 | PF3 | VERIFIED | pin 58 |
| LED 7 đoạn CA | digit 3 | PF4 | VERIFIED | pin 57 |
| LED 7 đoạn CA | digit 4 | PF6 | VERIFIED | pin 55 |
| LED 7 đoạn CA | cực tính/chọn digit | — | UNVERIFIED | Cần mở project Proteus và đối chiếu loại CA, điện trở, mạch kích. |
| LCD 16×2 | RS | PA0 | VERIFIED | pin 51 |
| LCD 16×2 | RW | PA1 | VERIFIED | pin 50 |
| LCD 16×2 | E | PA2 | VERIFIED | pin 49 |
| LCD 16×2 | D4–D7 | PA3–PA6 | VERIFIED | pin 48–45 |
| LCD 16×2 | BL | PD7 | VERIFIED | pin 32; dùng chung Nokia-BL |
| LCD 16×2 | điện áp/contrast | — | UNVERIFIED | Ảnh schematic tổng thể chưa đủ để xác minh hoàn toàn. |

Các net chung có thể ảnh hưởng giai đoạn sau: PD7 điều khiển cả hai backlight; PA7 là `7SLED-C` trong khi LCD dùng PA0–PA6; PE0/PE1 dùng chung nhãn Bluetooth; PD0 dùng chung encoder. Trước khi kích hoạt các ngoại vi này phải kiểm tra mạch thật và netlist. Tần số CPU 16 MHz, mức nguồn Nokia, cực tính BL, thứ tự phím, và cài đặt fuse/clock vẫn **UNVERIFIED**. Chỉ Nokia GPIO đã được đưa vào mã nguồn.

## File nguồn

- `main.c`: mẫu kiểm tra tĩnh, hiện `SNAKE GAME`, ba ô rắn và một ô thức ăn.
- `config/board_config.h`: kích thước LCD, GPIO Nokia và giá trị contrast ban đầu.
- `drivers/nokia5110.h`, `drivers/nokia5110.c`: software SPI, reset/khởi tạo PCD8544, framebuffer 504 byte, pixel, hình chữ nhật, chữ hoa A–Z/dấu cách và cập nhật toàn màn hình. Chưa hỗ trợ font đầy đủ.

Driver chỉ dùng vòng đợi ngắn trong **khởi tạo reset**, không dùng delay để điều khiển game. `NOKIA_CONTRAST` là giá trị khởi đầu, có thể cần chỉnh theo LCD thực. BL được đặt mức cao theo giả định kích hoạt mức cao; cực tính chưa xác minh, nên nếu thiếu đèn nền hãy kiểm tra schematic trước khi đổi mức.

## Tạo và build bằng CodeVisionAVR trên Windows 11

1. Chia sẻ/copy nguyên thư mục project sang Windows. Trong CodeVisionAVR, chọn **File → New → Project**. Có thể dùng Project Wizard để tạo project trống; không để wizard tạo mã thay thế `main.c`.
2. Trong cấu hình project chọn **ATmega128**, đặt **Clock** đúng tần số đang cấu hình trong thuộc tính ATmega128 của Proteus và board thực. 16 MHz chỉ là giá trị dự kiến; kiểm tra crystal/fuse trước. Không bật clock divide nếu không tương ứng phần cứng.
3. Trong danh sách **Source Files**, thêm `main.c` và `drivers/nokia5110.c`. Không thêm header như source cần biên dịch.
4. Thêm `config` và `drivers` vào **Include Directories** của C Compiler. Nếu phiên bản CVAVR dùng đường dẫn tương đối theo thư mục project, dùng `config` và `drivers`; nếu project đặt nơi khác, dùng đường dẫn tuyệt đối tới các thư mục này.
5. Chọn định dạng output **Intel HEX**, tối ưu mặc định hoặc mức tối ưu thông thường; giữ chip target ATmega128. Đảm bảo `<mega128.h>` từ thư viện CodeVisionAVR được tìm thấy. Không dùng compiler AVR-GCC.
6. Chạy **Project → Build All** (tên menu có thể khác theo phiên bản). Xem cửa sổ thông báo để xử lý lỗi/cảnh báo. Tìm `.hex` tại đường dẫn output hiển thị trong cửa sổ build hoặc Project settings, thường dưới thư mục output của project. Không đoán tên file hay coi build thành công trước khi thấy thông báo của CVAVR.

## Kiểm tra Proteus

1. Mở schematic gốc trong Proteus 8, phóng lớn và xác nhận trực tiếp Nokia `DC=PC0`, `DIN=PG1`, `CLK=PG2`, `RST=PG3`, `CE=PG4`, `BL=PD7`. Đặc biệt kiểm tra sự khác biệt RST/CE so với bảng dự kiến.
2. Mở thuộc tính ATmega128, gán **Program File** tới `.hex` vừa build; đặt **Clock Frequency** khớp project CVAVR và mạch thực.
3. Chạy mô phỏng. Mong đợi dòng `SNAKE GAME` ở phía trên, ba ô vuông liền nhau ở giữa và một ô nhỏ bên phải.
4. Nếu màn hình trống: kiểm tra nguồn/GND và điện áp phù hợp PCD8544; xem RST lên mức cao sau reset, CE xuống thấp trong lúc truyền, DIN và CLK có xung, DC thấp cho lệnh/cao cho dữ liệu. Kiểm tra thứ tự lệnh `0x21 → Vop → 0x04 → 0x14 → 0x20 → 0x0C`, thử chỉnh `NOKIA_CONTRAST` trong giới hạn phù hợp phần cứng rồi build lại. Nếu cạnh CLK quá nhanh trên board thật, thêm vài lệnh `nop` theo cú pháp CVAVR đã xác minh hoặc đo bằng logic analyzer trước khi sửa. Kiểm tra riêng BL; màn hình có thể có hình dù đèn nền tắt.

## Trạng thái xác minh

Đã đọc các tài liệu hiện có và kiểm tra mã nguồn về cấu trúc, include, mapping và giới hạn framebuffer. Chưa có CodeVisionAVR hay Proteus trong môi trường Ubuntu hiện tại, do đó **chưa biên dịch, chưa tạo HEX, chưa mô phỏng và chưa thử trên board**. Người dùng cần thực hiện build/mô phỏng theo các bước trên và cung cấp log hoặc ảnh nếu gặp lỗi. Giai đoạn tiếp theo: xác minh keypad 3×3 trên schematic/board, rồi triển khai quét phím và Timer1.

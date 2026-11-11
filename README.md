# Snake Game ATmega128 — refactor hiển thị giai đoạn đầu

Firmware Embedded C cho CodeVisionAVR. Hiện chỉ có chương trình kiểm tra Nokia 5110; chưa có game, ALCD, keypad, LED 7 đoạn, Timer1 hoặc EEPROM.

## Tài liệu và phạm vi xác minh

Đã đọc `docs/Kien_truc_va_tinh_nang_Snake_Game_ATmega128_cap_nhat_thu_vien.docx` và xem `so_do_chan.jpeg`, `mach_mo_phong.png`, `mach_that.jpeg`. Không có file Proteus gốc/netlist trong workspace. DOCX cũ `Mo_ta_de_tai_Snake_Game_ATmega128.docx` đã bị xóa trong working tree trước lần refactor này; thay đổi đó được giữ nguyên.

Đã đối chiếu [hướng dẫn CodeVisionAVR V3.20 do HP InfoTech viết](https://www.thierry-lequeu.fr/data/CodeVisionAVR-3-20-User-Manual.pdf) và [trang sản phẩm của nhà phân phối chính thức](https://www.codevision.be/). Tài liệu xác nhận `glcd.h` hỗ trợ PCD8544, `font5x7.h`, `glcd_init`, `glcd_clear`, `glcd_outtextxyf`, `glcd_rectangle`, `glcd_bar`, `glcd_putpixel`; từng tín hiệu GLCD có thể gán vào bit I/O riêng trong cấu hình project. `alcd.h` hỗ trợ LCD ký tự 2×16 và cấu hình tín hiệu riêng trong IDE. **Chưa kiểm tra header hoặc phiên bản CVAVR cài trên máy Windows của người dùng.** Cần đối chiếu trực tiếp trên máy đó trước khi coi project đã build được.

## Mapping từ ảnh sơ đồ chân

`VERIFIED` nghĩa là đọc được chân/nét nối từ ảnh; chưa xác nhận bằng netlist hoặc chạy board.

| Ngoại vi | Tín hiệu | ATmega128 | Trạng thái |
|---|---|---|---|
| Nokia 5110 | DC | **PG0** | VERIFIED |
| Nokia 5110 | DIN | PG1 | VERIFIED |
| Nokia 5110 | CLK | PG2 | VERIFIED |
| Nokia 5110 | RST | PG3 | VERIFIED |
| Nokia 5110 | CE/CS | PG4 | VERIFIED |
| Nokia 5110 | BL | PD7 | VERIFIED; chung LCD-BL |
| LCD 16×2 | RS/RW/E | PA0/PA1/PA2 | VERIFIED |
| LCD 16×2 | D4/D5/D6/D7 | PA3/PA4/PA5/PA6 | VERIFIED |
| Keypad | ROW-1/2/3 | PC5/PE1/PD0 | VERIFIED cho net; thứ tự phím UNVERIFIED |
| Keypad | COL-1/2/3 | PC3/PC4/PE0 | VERIFIED cho net; thứ tự phím UNVERIFIED |
| LED CA | A/B/C/D/E/F/G/DP | PF1/PF5/PA7/PD1/PD2/PF2/PD3/PF7 | VERIFIED cho net; cực tính UNVERIFIED |
| LED CA | digit 1/2/3/4 | PF0/PF3/PF4/PF6 | VERIFIED cho net; cực tính UNVERIFIED |

PG0 là chân vật lý **33** trên ảnh. Mapping PC0 trong phiên bản `board_config.h` cũ là lỗi đã sửa. PD7 chung hai đèn nền; PE0/PE1 chung nhãn Bluetooth; PD0 chung encoder. Clock 16 MHz, điện áp Nokia và mức kích hoạt BL vẫn cần xác minh trên schematic/board. Chưa cấu hình các ngoại vi ngoài Nokia.

## Cấu trúc nguồn

- `main.c`: mẫu hiển thị tĩnh `SNAKE GAME`, ba ô thân rắn và một ô thức ăn.
- `config/board_config.h`: kích thước GLCD, giá trị contrast khởi đầu, chân backlight; ghi rõ pin mapping GLCD phải thiết lập trong project CVAVR.
- `drivers/display.c`, `drivers/display.h`: adapter mỏng dùng GLCD của CVAVR, không tạo framebuffer riêng. API `display_init()` trả 0 nếu GLCD khởi tạo thất bại.
- `game/`: để dành cho logic và render ở giai đoạn sau.

Driver Nokia software SPI cũ đã bị loại khỏi source; **không thêm `nokia5110.c` vào project**. `display.c` hiện chỉ bọc GLCD. ALCD sẽ được thêm vào cùng module khi bắt đầu kiểm thử LCD 16×2.

## Cấu hình CodeVisionAVR trên Windows 11

1. Clone/pull source trên Windows; mở CodeVisionAVR, tạo project mới, chọn **ATmega128**. Không dùng AVR-GCC/Arduino. Chọn CPU clock đúng với thuộc tính ATmega128 trong Proteus và oscillator/fuse của board. 16 MHz chỉ là dự kiến.
2. Thêm **`main.c` và `drivers/display.c`** vào Source Files. Thêm `config/` và `drivers/` vào C Compiler include paths. Chọn output Intel HEX.
3. Mở **Project → Configure → C Compiler → Libraries → Graphic Display**. Chọn **PCD8544, 84×48**, bật hỗ trợ GLCD nếu có ô chọn. Trong Connections, gán `D/C=PG0`, `DIN/DATA=PG1`, `CLK=PG2`, `RST=PG3`, `CE/CS=PG4`. Tên từng tín hiệu có thể khác giữa các phiên bản; đối chiếu vai trò điện trước khi lưu. Backlight `PD7` do `display.c` điều khiển, không đưa vào năm tín hiệu giao tiếp GLCD.
4. Kiểm tra `glcd.h`, `font5x7.h` và cấu trúc `GLCDINIT_t` trong thư mục `INC` của đúng bản CVAVR. Mã này dùng các trường `font`, `readxmem`, `writexmem`, `temp_coef`, `bias`, `vlcd` theo manual V3.20. Nếu phiên bản cài đặt khác, đối chiếu header cùng example PCD8544 đi kèm trước khi sửa mã; không đoán API.
5. Chạy **Project → Build All**. Chỉ sau khi báo build thành công, lấy đường dẫn `.hex` trong cửa sổ build hoặc project settings. Chưa có `.prj` tự tạo trong repo.

Khi triển khai LCD 16×2 ở giai đoạn khác: dùng `alcd.h`, vào **Libraries → Alphanumeric LCD**, chọn 16 ký tự/dòng, giao tiếp 4-bit và gán `RS=PA0`, `RW=PA1`, `E=PA2`, `D4=PA3`, `D5=PA4`, `D6=PA5`, `D7=PA6`. Chưa bật hoặc gọi ALCD trong bài kiểm tra hiện tại.

## Proteus và chẩn đoán

Mở schematic gốc, xác nhận trực tiếp net `NOKIA-DC` tại PG0/pin 33 và bốn chân PG1–PG4. Trong thuộc tính ATmega128, chọn **Program File** là HEX vừa build và đặt **Clock Frequency** khớp cấu hình CVAVR. Chạy mô phỏng: kỳ vọng chữ `SNAKE GAME` ở trên, ba ô lớn liên tiếp và một ô nhỏ bên phải.

Nếu màn hình trống, kiểm tra nguồn/GND, mức điện áp PCD8544, RST, CE, DIN, CLK, D/C bằng probe/logic analyzer; kiểm tra cấu hình GLCD của project và `vlcd` contrast. Nếu có hình nhưng thiếu đèn nền, kiểm tra cực tính BL và đường PD7 chung trước khi đổi mức xuất. Kiểm tra cả thông báo lỗi hoặc giới hạn license GLCD trong bản CVAVR đang dùng.

## Trạng thái và bước sau

Đã đọc tài liệu, kiểm tra source và cấu hình pin, đối chiếu API trên manual của HP InfoTech. **Chưa biên dịch CVAVR, chưa tạo HEX, chưa chạy Proteus hoặc board thực.** Sau bài kiểm tra Nokia: kiểm tra LCD 16×2 qua ALCD; xác minh thứ tự phím và cực tính LED CA từ Proteus/board; sau đó mới triển khai keypad, LED multiplex và Timer1.

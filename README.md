# Snake Game ATmega128 — giai đoạn 2: kiểm thử chuyển động

Firmware Embedded C cho CodeVisionAVR. Giai đoạn này kiểm tra rắn dài ba đốt tự chạy trên Nokia, đổi hướng bằng keypad và Pause/Resume. Chưa có thức ăn, điểm số, LED 7 đoạn, LCD 16×2, EEPROM hoặc menu.

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
| Keypad | ROW-1/2/3 | PC6/PE1/PD0 | VERIFIED cho net; thứ tự phím UNVERIFIED |
| Keypad | COL-1/2/3 | PC3/PC4/PE0 | VERIFIED cho net; thứ tự phím UNVERIFIED |
| LED CA | A/B/C/D/E/F/G/DP | PF1/PF5/PA7/PD1/PD2/PF2/PD3/PF7 | VERIFIED cho net; cực tính UNVERIFIED |
| LED CA | digit 1/2/3/4 | PF0/PF3/PF4/PF6 | VERIFIED cho net; cực tính UNVERIFIED |

PG0 là chân vật lý **33** trên ảnh. Ảnh cho thấy `ROW-1=PC6`, `ROW-2=PE1`, `COL-2=PC4`, `COL-3=PE0`, khác bảng keypad trong nhiệm vụ 02; mã dùng các net trong ảnh. PD7 chung hai đèn nền; PE0/PE1 chung nhãn Bluetooth; PD0 chung encoder; PC6 chung SERVO-2. Clock 16 MHz và thứ tự đánh số phím thực tế vẫn cần kiểm tra trong Proteus. Keypad giả định phím vật lý theo bố trí 1–9 thông thường **chỉ để thử nghiệm**; nếu test sai, sửa `event_from_position()` trong `drivers/keypad.c`, không đổi chân GPIO.

## Cấu trúc nguồn

- `main.c`: khởi tạo GLCD/keypad/Timer1 và điều phối bằng system tick không chặn.
- `config/board_config.h`: kích thước GLCD, giá trị contrast khởi đầu, chân backlight; ghi rõ pin mapping GLCD phải thiết lập trong project CVAVR.
- `drivers/display.c`, `drivers/display.h`: adapter mỏng dùng GLCD của CVAVR, không tạo framebuffer riêng. API `display_init()` trả 0 nếu GLCD khởi tạo thất bại.
- `drivers/timer.c/.h`: Timer1 CTC, tick 1 ms, đọc `unsigned long` nguyên tử.
- `drivers/keypad.c/.h`: quét 3 hàng × 3 cột, pull-up nội, debounce 20 ms, sự kiện nhấn mới; nhiều phím cùng lúc bị bỏ qua cho đến khi nhả hết.
- `game/snake_game.c/.h`: logic rắn ba đốt, hướng đi và dừng khi chạm biên; không truy cập GPIO.
- `game/game_render.c/.h`: vẽ rắn và thông báo PAUSE/STOP qua display API.

Driver Nokia software SPI cũ đã bị loại khỏi source; **không thêm `nokia5110.c` vào project**. `display.c` hiện chỉ bọc GLCD. ALCD sẽ được thêm vào cùng module khi bắt đầu kiểm thử LCD 16×2.

## Cấu hình CodeVisionAVR trên Windows 11

1. Clone/pull source trên Windows; mở CodeVisionAVR, tạo project mới, chọn **ATmega128**. Không dùng AVR-GCC/Arduino. Chọn CPU clock đúng với thuộc tính ATmega128 trong Proteus và oscillator/fuse của board. 16 MHz chỉ là dự kiến.
2. Giữ cấu hình GLCD đã chạy ở giai đoạn 1. Thêm **`main.c`, `drivers/display.c`, `drivers/timer.c`, `drivers/keypad.c`, `game/snake_game.c`, `game/game_render.c`** vào Source Files. Thêm `config/`, `drivers/`, `game/` vào C Compiler include paths. Chọn output Intel HEX. Không thêm `nokia5110.c` cũ.
3. Mở **Project → Configure → C Compiler → Libraries → Graphic Display**. Chọn **PCD8544, 84×48**, bật hỗ trợ GLCD nếu có ô chọn. Trong Connections, gán `D/C=PG0`, `DIN/DATA=PG1`, `CLK=PG2`, `RST=PG3`, `CE/CS=PG4`. Tên từng tín hiệu có thể khác giữa các phiên bản; đối chiếu vai trò điện trước khi lưu. Backlight `PD7` do `display.c` điều khiển, không đưa vào năm tín hiệu giao tiếp GLCD.
4. Kiểm tra `glcd.h`, `font5x7.h` và cấu trúc `GLCDINIT_t` trong thư mục `INC` của đúng bản CVAVR. Mã này dùng các trường `font`, `readxmem`, `writexmem`, `temp_coef`, `bias`, `vlcd` theo manual V3.20. Nếu phiên bản cài đặt khác, đối chiếu header cùng example PCD8544 đi kèm trước khi sửa mã; không đoán API.
5. Xác nhận project CVAVR định nghĩa `_MCU_CLOCK_FREQUENCY_` đúng clock đang chạy. Timer1 dùng prescaler 64 và `OCR1A = F_CPU / (64 × 1000) − 1`; với 16 MHz là **249**. Mã yêu cầu chia chính xác thành tick 1 ms, không thay đổi clock project. Timer1 CTC và Compare Match A chỉ do `timer.c` cấu hình; không bật Timer1 lần nữa bằng CodeWizard. GLCD giai đoạn 1 không cấu hình Timer1 trong source, nhưng cần đối chiếu cấu hình library/project đang dùng.
6. Chạy **Project → Build All**. Chỉ sau khi báo build thành công, lấy đường dẫn `.hex` trong cửa sổ build hoặc project settings. Chưa có `.prj` tự tạo trong repo.

Khi triển khai LCD 16×2 ở giai đoạn khác: dùng `alcd.h`, vào **Libraries → Alphanumeric LCD**, chọn 16 ký tự/dòng, giao tiếp 4-bit và gán `RS=PA0`, `RW=PA1`, `E=PA2`, `D4=PA3`, `D5=PA4`, `D6=PA5`, `D7=PA6`. Chưa bật hoặc gọi ALCD trong bài kiểm tra hiện tại.

## Proteus và chẩn đoán

Mở schematic gốc, xác nhận Nokia PG0–PG4 và keypad PC6/PE1/PD0, PC3/PC4/PE0. Trong thuộc tính ATmega128, chọn **Program File** là HEX vừa build và đặt **Clock Frequency** khớp cấu hình CVAVR. Chạy mô phỏng:

1. Khởi tạo: rắn ba đốt ở ô (7,4), (6,4), (5,4) của lưới 14×8, mỗi ô 6×6 pixel.
2. Không nhấn phím: đầu rắn tiến phải một ô mỗi 500 ms; chạm biên thì dừng với chữ `STOP`.
3. Thử riêng từng vị trí phím 1–9 và ghi lại vị trí thực trên schematic/board. Với bố trí 1–9 thông thường: 2 lên, 8 xuống, 4 trái, 6 phải, 5 Pause/Resume. Cần khởi động lại mô phỏng sau khi rắn đã dừng.
4. Khi đang đi phải, nhấn 4: không quay đầu. Nhấn nhanh hai hướng trước cùng một bước: chỉ nhận hướng hợp lệ đầu tiên.
5. Giữ phím hoặc bấm nhiều phím cùng lúc: không có sự kiện lặp hoặc hướng sai; sau khi nhả tất cả có thể bấm tiếp.
6. Nhấn 5 để Pause, rắn đứng yên; nhấn lại để Resume, đợi đủ 500 ms rồi mới tiến một ô.
7. Kiểm tra Nokia vẫn hiển thị và các chân PG0–PG4, PD7 không bị keypad/Timer1 điều khiển.

Nếu màn hình trống, kiểm tra nguồn/GND, mức điện áp PCD8544, RST, CE, DIN, CLK, D/C bằng probe/logic analyzer; kiểm tra cấu hình GLCD của project và `vlcd` contrast. Nếu có hình nhưng thiếu đèn nền, kiểm tra cực tính BL và đường PD7 chung trước khi đổi mức xuất. Kiểm tra cả thông báo lỗi hoặc giới hạn license GLCD trong bản CVAVR đang dùng.

## Trạng thái và bước sau

Người dùng báo đã build và mô phỏng thành công **phiên bản giai đoạn 1**. Với thay đổi giai đoạn 2, **chưa biên dịch CVAVR, chưa tạo HEX mới, chưa chạy Proteus hoặc board thực** trong môi trường này. Cần xác nhận clock project, vector Timer1 và thứ tự phím vật lý khi build/mô phỏng. Giai đoạn tiếp theo mới xét thức ăn, điểm số và các ngoại vi còn lại.

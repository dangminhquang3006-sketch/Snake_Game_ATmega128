# Báo cáo kiểm định mở rộng — Prompt 13

## 1. Phạm vi và phiên bản

Source audit bắt đầu từ commit `1d5df423d400078c0d137f595a3c43334fb86aaa` trên `main`; các thay đổi tài liệu và test trong working tree chưa được commit. Mục tiêu là chuẩn bị nghiệm thu firmware ATmega128, không bổ sung gameplay. Module firmware: `main.c`, `config/board_config.h`, năm driver (`display`, `keypad`, `timer`, `led7seg`, `lcd1602`), bảy module game (`snake_game`, `game_render`, `game_menu`, `game_map`, `game_mode`, `bonus_food`, `dynamic_obstacle`) và `storage/highscore_eeprom`.

**Quy ước:** PASS = đã chạy và đạt; FAIL = đã chạy và không đạt; NOT RUN = chưa thực hiện; BLOCKED = thiếu công cụ/phần cứng trong môi trường này. Dấu `*` chỉ kiểm tra smoke host: chọn Map/Mode/Level, khởi tạo và ăn Fruit đầu tiên; không đại diện cho mô phỏng đầy đủ.

## 2. Ma trận 36 tổ hợp

| Level | Map | Mode | Host smoke | Proteus đầy đủ |
|---|---|---|---|---|
| EASY | CLASSIC | STANDARD | PASS* | NOT RUN |
| EASY | CLASSIC | BONUS | PASS* | NOT RUN |
| EASY | CLASSIC | TIME_ATTACK | PASS* | NOT RUN |
| EASY | CLASSIC | SURVIVAL | PASS* | NOT RUN |
| EASY | PILLARS | STANDARD | PASS* | NOT RUN |
| EASY | PILLARS | BONUS | PASS* | NOT RUN |
| EASY | PILLARS | TIME_ATTACK | PASS* | NOT RUN |
| EASY | PILLARS | SURVIVAL | PASS* | NOT RUN |
| EASY | LABYRINTH | STANDARD | PASS* | NOT RUN |
| EASY | LABYRINTH | BONUS | PASS* | NOT RUN |
| EASY | LABYRINTH | TIME_ATTACK | PASS* | NOT RUN |
| EASY | LABYRINTH | SURVIVAL | PASS* | NOT RUN |
| NORMAL | CLASSIC | STANDARD | PASS* | NOT RUN |
| NORMAL | CLASSIC | BONUS | PASS* | NOT RUN |
| NORMAL | CLASSIC | TIME_ATTACK | PASS* | NOT RUN |
| NORMAL | CLASSIC | SURVIVAL | PASS* | NOT RUN |
| NORMAL | PILLARS | STANDARD | PASS* | NOT RUN |
| NORMAL | PILLARS | BONUS | PASS* | NOT RUN |
| NORMAL | PILLARS | TIME_ATTACK | PASS* | NOT RUN |
| NORMAL | PILLARS | SURVIVAL | PASS* | NOT RUN |
| NORMAL | LABYRINTH | STANDARD | PASS* | NOT RUN |
| NORMAL | LABYRINTH | BONUS | PASS* | NOT RUN |
| NORMAL | LABYRINTH | TIME_ATTACK | PASS* | NOT RUN |
| NORMAL | LABYRINTH | SURVIVAL | PASS* | NOT RUN |
| HARD | CLASSIC | STANDARD | PASS* | NOT RUN |
| HARD | CLASSIC | BONUS | PASS* | NOT RUN |
| HARD | CLASSIC | TIME_ATTACK | PASS* | NOT RUN |
| HARD | CLASSIC | SURVIVAL | PASS* | NOT RUN |
| HARD | PILLARS | STANDARD | PASS* | NOT RUN |
| HARD | PILLARS | BONUS | PASS* | NOT RUN |
| HARD | PILLARS | TIME_ATTACK | PASS* | NOT RUN |
| HARD | PILLARS | SURVIVAL | PASS* | NOT RUN |
| HARD | LABYRINTH | STANDARD | PASS* | NOT RUN |
| HARD | LABYRINTH | BONUS | PASS* | NOT RUN |
| HARD | LABYRINTH | TIME_ATTACK | PASS* | NOT RUN |
| HARD | LABYRINTH | SURVIVAL | PASS* | NOT RUN |

Host smoke PASS* cho 36/36 tổ hợp qua `tests/host_snake.c`. Mỗi hàng vẫn cần kiểm tra menu, chu kỳ thực, keypad, hiển thị, va chạm, Restart và ngoại vi trong Proteus. Không có hàng nào được đánh dấu PASS Proteus.

## 3. Kiểm tra chuyên sâu

| Nhóm | Kết quả | Bằng chứng / giới hạn |
|---|---|---|
| Multi-Fruit | PASS host | Ranh giới 0,59,60,94,95,99; 30.000 lần chọn PRNG; +10/+20/+50 và tăng một đốt; không đo hình thực trên Nokia. |
| Wrap Around | PASS host | Bốn cạnh ở NORMAL/HARD, EASY Game Over, va chạm thân, ô đuôi vừa rời, Fruit/Bonus qua biên. Static obstacle ngay ô đích qua biên không có trong layout cố định hiện tại; kiểm tra tường thường và vật cản động riêng. |
| Reverse Snake | PASS host | Ba Fruit, nhiều lần đảo, thân chữ L, bốn cặp cổ/đầu qua biên, WIN không đảo, Bonus không đảo, Pause/Restart, mảng tối đa 112 đốt. Chưa kiểm thử hình chữ U trên Proteus. |
| Map & obstacles | PASS host | Layout CLASSIC 112, PILLARS 104, LABYRINTH 100 ô; sinh thức ăn không trùng tường/rắn; BFS từ chối cầu bịt kín, xét cạnh Wrap. BFS xét tường và bỏ qua thân di động nên không bảo đảm luôn sống sót. |
| Time Attack | PASS host logic | Countdown 60 giây, TIME_UP và trạng thái dừng; thứ tự sự kiện main đã rà soát tĩnh. Chưa đo sai số Timer1/ISR trên MCU. |
| Survival | PASS host logic | Lịch 20 giây, tối đa 8 vật cản, Pause/Restart, Safe Spawn và đầu mới sau Reverse. Chưa đo thời gian BFS trên ATmega128. |
| EEPROM | PASS host mock | Migration v1/v2/v3→v4, 36 kỷ lục, CRC, commit, sequence qua 65535→0 và fallback một slot lỗi; điểm >1090 được đọc lại. Persistence Proteus/board và linker EEPROM map chưa xác minh. |
| LCD | PASS host mock | Chuỗi hiển thị Level/Mode/Status/Score. Nokia và LED thực chưa kiểm thử trong bản này. |
| GPIO/peripheral | NOT RUN | Mapping đối chiếu source/tài liệu; mức điện, cực tính và nhiễu cần Proteus/board. |
| Timer1 1 ms | NOT RUN thực tế | Công thức `F_CPU/64/1000-1`, CTC, ISR chỉ tăng `volatile unsigned long`; đọc tick có lưu/khôi phục SREG và ngắt tạm. Phải xác nhận `_MCU_CLOCK_FREQUENCY_`, clock Proteus và jitter. |
| AddressSanitizer/UBSan | PASS host | Chạy cho logic Snake và EEPROM với `ASAN_OPTIONS=detect_leaks=0`; LeakSanitizer không dùng được trong môi trường này. |
| CodeVisionAVR Build All | BLOCKED | Không có compiler CVAVR trong Ubuntu workspace; chưa có HEX hoặc báo cáo tài nguyên của bản này. |
| Proteus 8 | BLOCKED | Không có phiên mô phỏng Proteus cho đúng source Prompt 13. |
| Board ATmega128 | BLOCKED | Chưa có nạp/verify hoặc đo phần cứng trong môi trường này. |

## 4. Rà soát Timer, EEPROM và thứ tự sự kiện

Main xử lý thời gian RUNNING trước input, sau đó bước di chuyển, rồi thử sinh vật cản động; TIME_UP kết thúc trước movement cùng lượt. Bonus hết hạn trước bước di chuyển cùng lượt. Pause ngăn cập nhật thời gian ở các vòng lặp sau và Resume đặt lại mốc di chuyển. Với clock giả định 16 MHz, công thức Timer1 cho OCR1A = 249 và 1 ms/tick; phải đối chiếu clock của đúng project CVAVR và Proteus. EEPROM chỉ được gọi khi kết thúc ván với cờ `result_saved`; module chỉ ghi slot v4 khi score vượt kỷ lục, commit byte ghi cuối, đọc lại xác minh. Reset MCU và nạp lại EEPROM image là hai phép thử khác nhau.

## 5. Tài nguyên và cấu hình release

Dữ liệu biết từ source: `segments` 112×2 = 224 byte SRAM; vật cản động có mask 16 byte, queue BFS 112 byte, visited 16 byte; cache kỷ lục 36×2 = 72 byte; LCD desired+shown 64 byte; LED digits 4 byte. Đây là kích thước các mảng riêng lẻ, **không phải** tổng SRAM hoặc stack. Font, map và bảng LED khai báo `flash`; GLCD library, stack, biến khác và mã thư viện phải được đo bằng báo cáo CodeVisionAVR. `highscore_init` có hai mảng tạm 36 score trên stack, `highscore_record_result` có buffer 78 byte và mảng verify 36 score; kiểm tra stack trên bản build thật.

Bốn macro test ngoại vi mặc định 0 trong `board_config.h`; cần kiểm tra tùy chọn project không định nghĩa lại. Không có `malloc`, recursion, AVR-GCC, vòng delay điều khiển game hoặc EEPROM/GLCD trong ISR. Không tạo framebuffer thứ hai. File `.c` firmware cần có trong project: 14; Prompt 13 không thêm source firmware.

## 6. Lỗi và thay đổi trong đợt này

Không phát hiện lỗi gameplay có bằng chứng tái hiện từ bộ kiểm tra hiện có. Phát hiện khoảng trống kiểm thử: chưa có lượt smoke tự động cho toàn bộ 36 tổ hợp và chưa thử sequence EEPROM qua 65535→0; đã bổ sung vào `tests/host_snake.c` và `tests/host_eeprom.c`, chạy đạt. README cũ còn hướng dẫn thêm riêng `dynamic_obstacle.c` như một bước của Prompt 8 và nhắc phân loại kỷ lục ở Prompt 13; đã sửa thành danh sách 14 source hiện tại và ghi rõ phân loại theo phiên bản luật chưa triển khai. Không đổi GPIO, Timer1, driver hay EEPROM layout.

## 7. Việc phải hoàn tất trước release

1. Trong CodeVisionAVR: xác nhận chip ATmega128, clock thực, `glcd.h` PCD8544/font `font5x7`, `alcd.h` 4 bit, đủ 14 source `.c`, bốn macro test = 0; **Project → Build All**. Lưu Errors/Warnings, Flash/SRAM/stack, EEPROM allocation và đường dẫn HEX mới. Xác nhận địa chỉ hai slot v1 không chồng vùng v2/v3/v4.
2. Trong Proteus: gán HEX mới cho ATmega128, đối chiếu clock; thực hiện 36 hàng ma trận và các ca chuyên sâu ở mục 3. Kiểm tra màn Nokia/LCD/LED/keypad, timeout, Pause, Restart, Back to Menu, High Score qua reset MCU và CRC/slot lỗi trên bản sao EEPROM image.
3. Trên board: xác minh nguồn, mức logic Nokia, clock/fuse, GPIO, ISP/signature; sao lưu EEPROM/fuse, xem EESAVE/chip erase và `.eep`, nạp và verify HEX; thử ngoại vi rồi game. Không tự sửa fuse hoặc xóa EEPROM.

**Kết luận:** source là **release candidate cho bước Build All và Proteus**, chưa đủ bằng chứng để tuyên bố firmware release đã nghiệm thu trên CVAVR/Proteus/board. Các ô NOT RUN/BLOCKED phải được cập nhật bằng log và kết quả thực tế trước khi chốt release.

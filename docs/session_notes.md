# Ghi chú bàn giao cho phiên làm việc sau

Cập nhật ngày 2026-11-22, múi giờ Asia/Bangkok. Đọc file này cùng [README](../README.md) trước khi sửa hoặc kiểm tra tiếp.

## Vị trí cần mở

| Thành phần | Đường dẫn |
|---|---|
| Source trên WSL | `/mnt/d/nhung/Snake_Game_ATmega128/` |
| Source trên Windows | `D:\nhung\Snake_Game_ATmega128\` |
| Project CodeVision | `D:\nhung\demo.prj` |
| Project tích hợp IDE | `D:\nhung\demo.cproj` |
| HEX đã kiểm tra | `D:\nhung\Debug\Exe\demo.hex` |
| Listing và memory map | `D:\nhung\Debug\List\demo.lst`, `demo.map` |
| Mạch Proteus đã tìm và kiểm tra | `C:\Users\Admin\Desktop\nhung\board_2025 (1).pdsprj` |
| Mạch trên WSL | `/mnt/c/Users/Admin/Desktop/nhung/board_2025 (1).pdsprj` |

`demo.prj`, `demo.cproj` và output build nằm ngoài thư mục Git của source. Khi chuyển máy hoặc lưu thay đổi project, nhớ mang theo chúng. File `D:\nhung\demo.c` là chương trình mẫu khác có hàm main riêng, không thêm vào game.

Có một file `D:\Hoc_tap_Sinh_Vien\KTMT\board.pdsprj`, nhưng đó là mạch Raspberry Pi, không phải mạch ATmega128 đã kiểm tra. Không dùng nhầm file này.

## Cấu trúc và luật hiện tại

Bản 8 file gốc chỉ còn NORMAL 300 ms/ô và HARD 250 ms/ô, mặc định NORMAL. Đã bỏ bước chọn STANDARD/SURVIVAL; chọn Level → Map là vào game. Cả hai đều thử sinh rào chắn mỗi 10 giây RUNNING trên cả 3 Map, tổng **6 tổ hợp**. Bản rút gọn trong `export/codevision/` chưa cập nhật các thay đổi này.

- Đã bỏ BONUS, TIME_ATTACK, Bonus Food và trạng thái TIME_UP. Còn 2 Level (NORMAL/HARD) và 3 Map, tổng 6 tổ hợp; không có lựa chọn Mode riêng.
- Sáu implementation gameplay đã gộp trực tiếp vào `game/game.c`; giữ header API riêng. Tổng source firmware còn 8 file `.c`, xem danh sách đầy đủ trong README. Không khôi phục sáu source cũ vào danh sách build.
- `board_config.h` hiện đặt NORMAL=300, **HARD=250 ms/ô**. Tài liệu trước đây ghi HARD=150; dùng source hiện tại làm căn cứ. Chu kỳ thực tế chưa được đo trên MCU.
- Cả NORMAL và HARD thử sinh một thanh ngang 3 ô mỗi 10 giây RUNNING, tối đa 8 thanh; Pause dừng thời gian. Không đổi sang lịch 20 giây theo tài liệu cũ.
- EEPROM v4 giữ 36 ô để tương thích dữ liệu cũ. Gameplay dùng 6 kỷ lục NORMAL/HARD × Map ở nhóm SURVIVAL cũ (offset 27); NORMAL=1, HARD=2 giữ nguyên ID. API storage vẫn đọc/ghi nhóm STANDARD cũ phục vụ tương thích và kiểm thử, không đưa nhóm này vào menu. Không xóa hoặc ánh xạ kỷ lục STANDARD sang luật rào chắn.
- Commit nguồn của lần bàn giao trước khi bỏ EASY: `ad8108d` — `refactor(game): consolidate gameplay into game.c and update build docs`. Mốc commit này chưa bao gồm thay đổi bỏ EASY; không tự commit các chỉnh sửa hiện tại.

## Bằng chứng build trước khi bỏ EASY

Đã tìm thấy artifact CodeVision sau khi gộp; không phải chỉ có kết quả test host. Phiên kiểm tra không tự chạy lại một lượt Build All sạch, mà đọc artifact và log hiện có:

- HEX sửa lần cuối lúc **17:46:08 ngày 2026-11-22 (+07:00)**.
- SHA-256: `16bf3c86a925fa4706534ecca1dba51c0a20da9edb6075bd6daa10b8328bfe11`.
- `demo.prj` ghi Debug, ATmega128, clock **8 MHz**, build 11, `No errors`, 3 warning compiler.
- `demo.lst` ghi assembly complete, 0 errors, **31 warnings**. Có cảnh báo `.DEF` register và `.db` alignment; chưa phân loại mức ảnh hưởng, không được ghi là build không warning.
- Log ghi Flash 18.140 byte, global RAM 1.131 byte, data stack cấu hình 1.024 byte, ước lượng sử dụng 228 byte. Đây là số của artifact hiện có, phải cập nhật sau mỗi build mới.
- Dòng EEPROM usage trong log compiler ghi 26 byte, nhưng bảng assembler ghi vùng dùng 356 byte; source có các slot đặt địa chỉ cố định. Cần đối chiếu memory map trước khi kết luận về vùng EEPROM.

Khi source hoặc HEX thay đổi, các số và hash trên chỉ còn là mốc lịch sử; xác minh lại bản đang nạp.

## Kết quả quan sát trong Proteus trước khi bỏ EASY

Người dùng báo bấm nút không chạy dù Nokia đang ở menu Level. Đã truy cập cửa sổ Proteus đang mở, đối chiếu với file mạch trên Desktop và quan sát:

- Thuộc tính lưu trong file mạch: `PROGRAM=D:\nhung\Debug\Exe\demo.hex`, `CKSEL=4` (Int.RC 8 MHz theo định nghĩa linh kiện). Khớp clock 8 MHz của project.
- Nokia hiện menu Level, Map/Mode có phản hồi. Đã thấy menu STANDARD/SURVIVAL.
- Đã vào được ván **HARD + PILLARS**, sau màn chọn STANDARD; LCD hiện `HARD PILLARS` và `STATUS: PLAYING`.
- Qua hai lần chụp, vị trí rắn thay đổi: đã quan sát được chuyển động trong game.
- CPU load trên thanh trạng thái tại các lần quan sát khoảng **20–45%**, thời gian mô phỏng tiếp tục tăng. Chưa có bằng chứng máy quá tải ở thời điểm đó; không kết luận về mọi lần chạy khác.
- Đã phóng to keypad và thử nhấn/thả vùng thanh ngang của nút giữa. Vùng bấm nhỏ có thể gây khó thao tác; chưa chứng minh đây là nguyên nhân duy nhất của hiện tượng người dùng báo.

Đây là kiểm tra smoke có giới hạn, **không phải nghiệm thu toàn bộ Proteus**. Chưa xác nhận riêng Pause/Resume, Restart, toàn bộ hướng, Survival, thời gian thực, EEPROM qua reset hoặc cả 18 tổ hợp. Không có sửa source firmware trong lượt điều tra nút bấm. Các ảnh chụp và script kiểm tra tạm đã được xóa, không có video/log tín hiệu lưu để tái hiện độc lập.

## Nếu lần sau lại không nhận nút

1. Xác nhận đúng file mạch, đúng Program File và timestamp HEX; dùng clock 8 MHz khớp project. Nếu build mới, dừng rồi chạy lại mô phỏng với HEX mới.
2. Phóng to cụm keypad 3×3, thử giữ rồi thả thanh ngang của nút giữa (phím 5). Các nút BT1/BT2/BT3 bên trái là cụm khác. Luồng hiện tại chỉ cần xác nhận Level → Map; chọn map xong là vào game.
3. Quan sát đồng hồ mô phỏng có tăng; code chống dội 20 ms theo tick MCU. Đo thời gian theo mô phỏng, không chỉ thời gian chờ ngoài đời. Không quy lỗi cho máy yếu khi chưa kiểm tra CPU load/thời gian mô phỏng.
4. Đối chiếu mapping: ROW1=PC6, ROW2=PE1, ROW3=PD0; COL1=PC4, COL2=PC5, COL3=PE0. Kiểm tra mức logic thay đổi khi nhấn, đặc biệt các net dùng chung với ngoại vi khác.
5. Nếu lỗi tái hiện, dùng chế độ `KEYPAD_TEST_MODE=1` có sẵn, ba macro test khác=0, rồi Build All/nạp HEX test. Kiểm tra hiển thị KEY/ROW/COL/COUNT. Chưa thực hiện bước test ngoại vi này trong lượt điều tra. Kết thúc phải trả cả bốn macro về 0 và build lại game.
6. Kiểm tra `system_ms`/ISR Timer1 bằng debugger nếu không nhận sự kiện: bộ quét keypad bỏ qua lượt quét khi tick không đổi. Không sửa chân hoặc giảm debounce tùy tiện trước khi có bằng chứng.

## Kiểm thử và việc còn lại

Bản gộp luật rào chắn vào NORMAL/HARD đã đạt lại 4 nhóm host. Kiểm tra cú pháp toàn firmware và 4 chế độ test ngoại vi cũng đạt qua bản ghép tạm trong thư mục test; không ghi lại các file trong `export/codevision/`. Chưa Build All bằng CodeVisionAVR hoặc chạy Proteus cho thay đổi bỏ EASY và bỏ bước chọn Mode.

`sh tests/run_host_validation.sh` đã chạy lại và đạt 4/4 nhóm: Snake (6 tổ hợp), Survival (3 Map × 2 Level × 32 seed), EEPROM mock và LCD mock. Trước khi bỏ EASY, một harness tạm dùng source keypad với GPIO giả lập cũng đạt mapping 9 vị trí, chống dội và giữ/thả phím; không thay cho đo GPIO thật và không được thêm vào repo.

Lần tiếp theo nên ưu tiên tái hiện lỗi nút nếu còn, xác minh Pause/Resume và điều hướng, sau đó đo rào chắn mỗi 10 giây, kiểm tra EEPROM qua reset và hoàn thiện ma trận 6 tổ hợp của bản hiện tại. Board thật chưa kiểm tra. Giữ nguyên thay đổi người dùng trong các file `.cci/.hci`; chúng là dữ liệu IDE sinh ra, không phải firmware source.
